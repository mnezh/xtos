#include "xtos/app.h"
#include "xtos/display.h"
#include "xtos/system.h"
#include "xtos/ui/canvas.h"
#include "xtos/ui/font.h"
#include "runtime/client.h"
#include "runtime/log.h"

#define ABI_STRESS_LOOPS 64

static void log_ok(const char *name, int ok)
{
    XTOS_LOG_PREFIX("[TEST]", ok ? name : "resident abi failure");
}

static void SmokeInit(void)
{
    SystemPrefs prefs;
    SystemPrefs loaded;
    const SystemPrefs *current;
    u16 magic;
    u16 version;
    u16 status;
    u16 i;
    const Font *font;
    int ok;
    int loaded_ok;
    int saved_ok;

    XTOS_LOG_PREFIX("[TEST]", "smoke start");

    ok = XtosRuntimeSelfTest(&magic, &version, &status);
    log_ok("resident selftest ok", ok);

    ok = XtosRuntimeStatus(&magic, &version);
    log_ok("resident status ok", ok);

    DisplaySetMode(DISPLAY_MODE_LOW);
    DisplaySetPalette(DISPLAY_PALETTE_WHITE_ON_BLUE);
    ok = DisplayCurrentMode() == DISPLAY_MODE_LOW &&
         DisplayCurrentPalette() == DISPLAY_PALETTE_WHITE_ON_BLUE;
    log_ok("display roundtrip ok", ok);

    loaded_ok = SystemPrefsLoad(&loaded);
    XTOS_LOG_PREFIX_U16("[TEST]", "prefs_load_result", (u16)loaded_ok);
    XTOS_LOG_PREFIX_U16("[TEST]", "prefs_loaded_mode", (u16)loaded.mode);
    XTOS_LOG_PREFIX_U16("[TEST]", "prefs_loaded_palette",
                        (u16)loaded.palette);

    prefs.mode = DISPLAY_MODE_LOW;
    prefs.palette = loaded.palette == DISPLAY_PALETTE_BROWN_ON_BLUE ?
                    DISPLAY_PALETTE_BLUE_ON_WHITE :
                    DISPLAY_PALETTE_BROWN_ON_BLUE;
    SystemPrefsApply(&prefs);
    XTOS_LOG_PREFIX_U16("[TEST]", "prefs_apply_mode", (u16)prefs.mode);
    XTOS_LOG_PREFIX_U16("[TEST]", "prefs_apply_palette",
                        (u16)prefs.palette);
    saved_ok = SystemPrefsSave(&prefs);
    XTOS_LOG_PREFIX_U16("[TEST]", "prefs_save_result", (u16)saved_ok);
    log_ok("prefs save ok", saved_ok);

    loaded_ok = SystemPrefsLoad(&loaded);
    ok = loaded_ok &&
         loaded.mode == prefs.mode &&
         loaded.palette == prefs.palette;
    log_ok("prefs load ok", ok);

    current = SystemPrefsCurrent();
    ok = current != 0 &&
         current->mode == prefs.mode &&
         current->palette == prefs.palette;
    log_ok("prefs current ok", ok);

    ok = 1;
    for (i = 0; i < ABI_STRESS_LOOPS; ++i) {
        if (!XtosRuntimeStatus(&magic, &version) ||
            !XtosRuntimeSelfTest(&magic, &version, &status) ||
            DisplayCurrentMode() != prefs.mode ||
            DisplayCurrentPalette() != prefs.palette) {
            ok = 0;
            break;
        }

        current = SystemPrefsCurrent();
        if (current == 0 ||
            current->mode != prefs.mode ||
            current->palette != prefs.palette) {
            ok = 0;
            break;
        }
    }
    log_ok("resident abi loop ok", ok);

    font = FontGet(FONT_SYSTEM);
    ok = FontCount() >= 3 &&
         FontHeight(font) == 7 &&
         FontGlyphCount(font) > 128 &&
         FontName(font) != 0 &&
         CanvasTextWidth(font, "XTOS") > 0;
    log_ok("resident font metadata ok", ok);
}

static void SmokeDraw(void)
{
    const Font *font;

    font = FontGet(FONT_SYSTEM);
    CanvasClear();
    CanvasRect(12, 12, 307, 187, CANVAS_PRIMARY_FOREGROUND);
    CanvasRect(16, 16, 303, 183, CANVAS_EXTRA_1_ON_BACKGROUND);
    CanvasText(92, 88, font, CANVAS_PRIMARY_FOREGROUND, "XTOS SMOKE TEST");
    CanvasText(100, 102, FontGet(FONT_SMALL), CANVAS_EXTRA_1_ON_BACKGROUND,
               "INT 60h orchestration OK");
    XTOS_LOG_PREFIX("[TEST]", "screenshot written");
    AppQuit();
}

static void SmokeShutdown(void)
{
    XTOS_LOG_PREFIX("[TEST]", "smoke complete");
    DisplayShutdown();
}

int main(void)
{
    static Application SmokeApp;

    SmokeApp.Init = SmokeInit;
    SmokeApp.Draw = SmokeDraw;
    SmokeApp.Shutdown = SmokeShutdown;
    SmokeApp.first_draw_screenshot_path = "smoke.cga";

    return AppRun(&SmokeApp);
}
