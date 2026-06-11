#include "event.h"
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

    if (app == 0) {
        return 1;
    }

    app_running = 1;
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

            if (app->HandleEvent != 0) {
                app->HandleEvent(&event);
            }

        }

        if (app_running && InvalidationIsDirty() && app->Draw != 0) {
            MouseCursorHide();
            app->Draw();
            InvalidationClear();
            if (RuntimeMousePresent()) {
                MouseCursorShow();
            }
        }
    }

    if (app->Shutdown != 0) {
        app->Shutdown();
    }

    return 0;
}
