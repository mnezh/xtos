#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xtos/system.h"

#define XTOS_CFG_FILE "XTOS.CFG"

static SystemPrefs current_prefs = {
    DISPLAY_MODE_LOW,
    DISPLAY_PALETTE_BLUE_ON_WHITE
};

static void prefs_defaults(SystemPrefs *prefs)
{
    prefs->mode = DISPLAY_MODE_LOW;
    prefs->palette = DISPLAY_PALETTE_BLUE_ON_WHITE;
}

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

static int line_value(const char *line, const char *key, int *value)
{
    unsigned int len;

    len = (unsigned int)strlen(key);

    if (strncmp(line, key, len) != 0 || line[len] != '=') {
        return 0;
    }

    *value = atoi(line + len + 1);
    return 1;
}

int SystemPrefsLoad(SystemPrefs *prefs)
{
    FILE *file;
    char line[32];
    int value;
    int loaded;

    if (prefs == 0) {
        return 0;
    }

    prefs_defaults(prefs);
    loaded = 0;
    file = fopen(XTOS_CFG_FILE, "r");

    if (file == 0) {
        current_prefs = *prefs;
        return 0;
    }

    while (fgets(line, sizeof(line), file) != 0) {
        if (line_value(line, "MODE", &value)) {
            prefs->mode = (enum DisplayMode)value;
            loaded = 1;
        } else if (line_value(line, "PALETTE", &value)) {
            prefs->palette = (enum DisplayPalette)value;
            loaded = 1;
        }
    }

    fclose(file);
    prefs_clamp(prefs);
    current_prefs = *prefs;
    return loaded;
}

int SystemPrefsSave(const SystemPrefs *prefs)
{
    FILE *file;
    SystemPrefs safe_prefs;

    if (prefs == 0) {
        return 0;
    }

    safe_prefs = *prefs;
    prefs_clamp(&safe_prefs);
    file = fopen(XTOS_CFG_FILE, "w");

    if (file == 0) {
        return 0;
    }

    fprintf(file, "MODE=%u\n", (unsigned int)safe_prefs.mode);
    fprintf(file, "PALETTE=%u\n", (unsigned int)safe_prefs.palette);
    fclose(file);
    current_prefs = safe_prefs;
    return 1;
}

const SystemPrefs *SystemPrefsCurrent(void)
{
    return &current_prefs;
}

void SystemPrefsApply(const SystemPrefs *prefs)
{
    SystemPrefs safe_prefs;

    if (prefs == 0) {
        return;
    }

    safe_prefs = *prefs;
    prefs_clamp(&safe_prefs);

    /* Mode changes reset CGA hardware colors, so apply palette after mode. */
    DisplaySetMode(safe_prefs.mode);
    DisplaySetPalette(safe_prefs.palette);
    current_prefs = safe_prefs;
}
