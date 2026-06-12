#include <stdio.h>

#include "event.h"
#include "client.h"
#include "int60.h"
#include "log.h"
#include "cursor.h"
#include "mouse.h"
#include "../xtos/app.h"
#include "../xtos/system.h"
#include "../xtos/ui/invalidation.h"

static u8 app_running;

void AppQuit(void)
{
    app_running = 0;
}

int AppRun(Application *app)
{
    Event event;
    SystemPrefs prefs;
    u8 first_draw_screenshot_done;

    if (app == 0) {
        return 1;
    }

    if (!XtosRuntimePresent()) {
        puts("This is a XTOS application and cannot run in DOS");
        return 1;
    }

    XtosRuntimeLog("application detected XTOS runtime");
    app_running = 1;
    first_draw_screenshot_done = 0;
    XtosInt60Install();
    XTOS_LOG_PREFIX("[RT]", "app start");
    SystemPrefsLoad(&prefs);
    SystemPrefsApply(&prefs);

    if (app->Init != 0) {
        app->Init();
    }

    RuntimeMouseInit();
    InvalidateAll();

    while (app_running) {
        RuntimePumpEvents();

        while (app_running && GetEvent(&event)) {
            if (event.type == EVENT_QUIT) {
                app_running = 0;
                break;
            }

            if (event.type == EVENT_KEYDOWN &&
                event.key == XTOS_KEY_ALT_S) {
                XtosScreenshotCga(0);
                continue;
            }

            if (app->HandleEvent != 0) {
                app->HandleEvent(&event);
            }

        }

        if (app_running && InvalidationIsDirty() && app->Draw != 0) {
            MouseCursorHide();
            app->Draw();
            InvalidationClear();
            if (!first_draw_screenshot_done &&
                app->first_draw_screenshot_path != 0) {
                XtosScreenshotCga(app->first_draw_screenshot_path);
                first_draw_screenshot_done = 1;
            }
            if (RuntimeMousePresent()) {
                MouseCursorShow();
            }
        }
    }

    if (app->Shutdown != 0) {
        app->Shutdown();
    }

    XTOS_LOG_PREFIX("[RT]", "app shutdown");
    XtosInt60Restore();
    return 0;
}
