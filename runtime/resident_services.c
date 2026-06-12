#include "cga.h"
#include "display.h"
#include "log.h"
#include "resident_dos.h"
#include "screen.h"
#include "system.h"
#include "../xtos/abi.h"

#define XTOS_CFG_FILE "XTOS.CFG"
#define XTOS_EXEC_PATH_SIZE 64

static enum DisplayMode current_display_mode = DISPLAY_MODE_LOW;
static enum DisplayPalette current_display_palette = DISPLAY_PALETTE_BLUE_ON_WHITE;

static SystemPrefs current_prefs = {
    DISPLAY_MODE_LOW,
    DISPLAY_PALETTE_BLUE_ON_WHITE
};
static char cfg_buffer[64];
static char cfg_write_buffer[18];
static char next_exec_path[XTOS_EXEC_PATH_SIZE];
static char exec_log_buffer[80];
static u8 next_exec_pending;

char *XtosCopyFarString(char *dest, const char XTOS_FAR *source, u16 max);

static void prefs_clamp(SystemPrefs *prefs)
{
    if (prefs->mode != DISPLAY_MODE_LOW &&
        prefs->mode != DISPLAY_MODE_HIGH) {
        prefs->mode = DISPLAY_MODE_LOW;
    }

    if (prefs->palette != DISPLAY_PALETTE_BLACK_ON_WHITE &&
        prefs->palette != DISPLAY_PALETTE_WHITE_ON_BLACK &&
        prefs->palette != DISPLAY_PALETTE_BLUE_ON_WHITE &&
        prefs->palette != DISPLAY_PALETTE_WHITE_ON_BLUE &&
        prefs->palette != DISPLAY_PALETTE_BROWN_ON_BLUE) {
        prefs->palette = DISPLAY_PALETTE_BLUE_ON_WHITE;
    }
}

static u8 decimal_at(const char *buffer, u16 length, const char *key, u16 *value)
{
    u16 i;
    u16 k;

    for (i = 0; i < length; ++i) {
        k = 0;
        while (key[k] != 0 &&
               (u16)(i + k) < length &&
               buffer[i + k] == key[k]) {
            ++k;
        }

        if (key[k] == 0 &&
            (u16)(i + k) < length &&
            buffer[i + k] >= '0' &&
            buffer[i + k] <= '9') {
            *value = (u16)(buffer[i + k] - '0');
            return 1;
        }
    }

    return 0;
}

static enum ScreenModeId screen_mode_for_display(enum DisplayMode mode)
{
    if (mode == DISPLAY_MODE_HIGH) {
        return CGA_640x200x2;
    }

    return CGA_320x200x4;
}

static enum ScreenPaletteId screen_palette_for_display(
    enum DisplayPalette palette)
{
    if (palette == DISPLAY_PALETTE_WHITE_ON_BLACK) {
        return WHITE_ON_BLACK;
    }

    if (palette == DISPLAY_PALETTE_BLUE_ON_WHITE) {
        return BLUE_ON_WHITE;
    }

    if (palette == DISPLAY_PALETTE_WHITE_ON_BLUE) {
        return WHITE_ON_BLUE;
    }

    if (palette == DISPLAY_PALETTE_BROWN_ON_BLUE) {
        return BROWN_ON_BLUE;
    }

    return BLACK_ON_WHITE;
}

void RuntimeDisplaySetMode(enum DisplayMode mode)
{
    if (mode != DISPLAY_MODE_LOW && mode != DISPLAY_MODE_HIGH) {
        mode = DISPLAY_MODE_LOW;
    }

    current_display_mode = mode;
    XTOS_LOG_PREFIX_U16("[RT]", "resident_display_mode", (u16)mode);
    ScreenMode(screen_mode_for_display(mode));
    ScreenPalette(screen_palette_for_display(current_display_palette));
}

enum DisplayMode RuntimeDisplayCurrentMode(void)
{
    return current_display_mode;
}

void RuntimeDisplaySetPalette(enum DisplayPalette palette)
{
    if (palette != DISPLAY_PALETTE_BLACK_ON_WHITE &&
        palette != DISPLAY_PALETTE_WHITE_ON_BLACK &&
        palette != DISPLAY_PALETTE_BLUE_ON_WHITE &&
        palette != DISPLAY_PALETTE_WHITE_ON_BLUE &&
        palette != DISPLAY_PALETTE_BROWN_ON_BLUE) {
        palette = DISPLAY_PALETTE_BLUE_ON_WHITE;
    }

    current_display_palette = palette;
    XTOS_LOG_PREFIX_U16("[RT]", "resident_display_palette", (u16)palette);
    ScreenPalette(screen_palette_for_display(palette));
}

enum DisplayPalette RuntimeDisplayCurrentPalette(void)
{
    return current_display_palette;
}

int RuntimeSystemPrefsLoad(SystemPrefs *prefs)
{
    int handle;
    u16 bytes;
    u16 value;
    u8 loaded;

    if (prefs == 0) {
        return 0;
    }

    prefs->mode = DISPLAY_MODE_LOW;
    prefs->palette = DISPLAY_PALETTE_BLUE_ON_WHITE;
    loaded = 0;

    handle = ResidentDosOpenRead(XTOS_CFG_FILE);
    if (handle >= 0) {
        bytes = ResidentDosRead((u16)handle, cfg_buffer, sizeof(cfg_buffer));
        ResidentDosClose((u16)handle);

        if (bytes != 0xffff) {
            if (decimal_at(cfg_buffer, bytes, "MODE=", &value)) {
                prefs->mode = (enum DisplayMode)value;
                loaded = 1;
            }
            if (decimal_at(cfg_buffer, bytes, "PALETTE=", &value)) {
                prefs->palette = (enum DisplayPalette)value;
                loaded = 1;
            }
        }
    }

    prefs_clamp(prefs);
    current_prefs = *prefs;
    XTOS_LOG_PREFIX("[RT]", loaded ? "resident_prefs_load" :
                    "resident_prefs_load_default");
    return loaded;
}

int RuntimeSystemPrefsSave(const SystemPrefs *prefs)
{
    int handle;
    SystemPrefs safe_prefs;

    if (prefs == 0) {
        return 0;
    }

    safe_prefs = *prefs;
    prefs_clamp(&safe_prefs);
    XTOS_LOG_PREFIX_U16("[RT]", "resident_prefs_save_mode",
                        (u16)safe_prefs.mode);
    XTOS_LOG_PREFIX_U16("[RT]", "resident_prefs_save_palette",
                        (u16)safe_prefs.palette);
    cfg_write_buffer[0] = 'M';
    cfg_write_buffer[1] = 'O';
    cfg_write_buffer[2] = 'D';
    cfg_write_buffer[3] = 'E';
    cfg_write_buffer[4] = '=';
    cfg_write_buffer[5] = (char)('0' + safe_prefs.mode);
    cfg_write_buffer[6] = '\r';
    cfg_write_buffer[7] = '\n';
    cfg_write_buffer[8] = 'P';
    cfg_write_buffer[9] = 'A';
    cfg_write_buffer[10] = 'L';
    cfg_write_buffer[11] = 'E';
    cfg_write_buffer[12] = 'T';
    cfg_write_buffer[13] = 'T';
    cfg_write_buffer[14] = 'E';
    cfg_write_buffer[15] = '=';
    cfg_write_buffer[16] = (char)('0' + safe_prefs.palette);
    cfg_write_buffer[17] = '\n';

    handle = ResidentDosCreate(XTOS_CFG_FILE);
    if (handle < 0) {
        XTOS_LOG_PREFIX("[RT]", "resident_prefs_save_create_fail");
        return 0;
    }

    if (ResidentDosWrite((u16)handle, cfg_write_buffer,
                         sizeof(cfg_write_buffer)) == 0xffff) {
        ResidentDosClose((u16)handle);
        XTOS_LOG_PREFIX("[RT]", "resident_prefs_save_write_fail");
        return 0;
    }

    ResidentDosClose((u16)handle);
    current_prefs = safe_prefs;
    XTOS_LOG_PREFIX("[RT]", "resident_prefs_save");
    return 1;
}

const SystemPrefs *RuntimeSystemPrefsCurrent(void)
{
    return &current_prefs;
}

void RuntimeSystemPrefsApply(const SystemPrefs *prefs)
{
    SystemPrefs safe_prefs;

    if (prefs == 0) {
        return;
    }

    safe_prefs = *prefs;
    prefs_clamp(&safe_prefs);
    RuntimeDisplaySetMode(safe_prefs.mode);
    RuntimeDisplaySetPalette(safe_prefs.palette);
    current_prefs = safe_prefs;
    XTOS_LOG_PREFIX("[RT]", "resident_prefs_apply");
}

int RuntimeExecRequest(const char *path)
{
    u16 i;
    u16 len;
    u16 pos;

    if (path == 0 || path[0] == 0) {
        XTOS_LOG_PREFIX("[EXEC]", "request invalid empty");
        return 0;
    }

    len = 0;
    while (path[len] != 0) {
        ++len;

        if (len >= XTOS_EXEC_PATH_SIZE) {
            XTOS_LOG_PREFIX("[EXEC]", "request invalid too-long");
            return 0;
        }
    }

    for (i = 0; i <= len; ++i) {
        next_exec_path[i] = path[i];
    }

    next_exec_pending = 1;
    pos = 0;
    exec_log_buffer[pos++] = 'r';
    exec_log_buffer[pos++] = 'e';
    exec_log_buffer[pos++] = 'q';
    exec_log_buffer[pos++] = 'u';
    exec_log_buffer[pos++] = 'e';
    exec_log_buffer[pos++] = 's';
    exec_log_buffer[pos++] = 't';
    exec_log_buffer[pos++] = ' ';
    exec_log_buffer[pos++] = 'p';
    exec_log_buffer[pos++] = 'a';
    exec_log_buffer[pos++] = 't';
    exec_log_buffer[pos++] = 'h';
    exec_log_buffer[pos++] = '=';
    for (i = 0; next_exec_path[i] != 0 &&
         pos < (u16)(sizeof(exec_log_buffer) - 1); ++i) {
        exec_log_buffer[pos++] = next_exec_path[i];
    }
    exec_log_buffer[pos] = 0;
    XTOS_LOG_PREFIX("[EXEC]", exec_log_buffer);
    return 1;
}

int RuntimeExecRequestFar(const char XTOS_FAR *path)
{
    char path_copy[XTOS_EXEC_PATH_SIZE];
    u16 len;

    if (path == 0) {
        XTOS_LOG_PREFIX("[EXEC]", "request invalid null");
        return 0;
    }

    len = 0;
    while (path[len] != 0) {
        ++len;

        if (len >= XTOS_EXEC_PATH_SIZE) {
            XTOS_LOG_PREFIX("[EXEC]", "request invalid too-long");
            return 0;
        }
    }

    XtosCopyFarString(path_copy, path, XTOS_EXEC_PATH_SIZE);
    return RuntimeExecRequest(path_copy);
}

int RuntimeExecNext(char *path, u16 path_size)
{
    u16 i;

    if (path == 0 || path_size == 0) {
        return 0;
    }

    path[0] = 0;

    if (!next_exec_pending) {
        return 0;
    }

    for (i = 0; (u16)(i + 1) < path_size && next_exec_path[i] != 0; ++i) {
        path[i] = next_exec_path[i];
    }

    path[i] = 0;
    return path[0] != 0;
}

void RuntimeExecClear(void)
{
    next_exec_path[0] = 0;
    next_exec_pending = 0;
    XTOS_LOG_PREFIX("[RT]", "exec_clear");
}
