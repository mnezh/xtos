#include "xtos/app.h"
#include "xtos/event.h"
#include "xtos/exec.h"
#include "xtos/ui/canvas.h"
#include "xtos/ui/font.h"
#include "xtos/ui/layout.h"
#include "xtos/ui/runtime_form.h"

#define ACTION_CONTROL 1
#define ACTION_SHOWCASE 2
#define ACTION_FONT 3
#define ACTION_EXIT 4

static FormId MainForm;

static u16 button_right_for(u16 left, const char *text)
{
    return (u16)(left + UI_BUTTON_PAD_X +
                 CanvasTextWidth(FontGet(FONT_SMALL), text) +
                 UI_BUTTON_PAD_X - 1);
}

static void launch_and_quit(const char *path)
{
    if (ExecRequest(path)) {
        AppQuit();
    }
}

static void LauncherInit(void)
{
    MainForm = RtFormCreate("XTOS Launcher", FONT_SYSTEM);
    if (MainForm == RT_FORM_INVALID) {
        AppQuit();
        return;
    }

    RtFormAddLabel(MainForm, 10, 9, 8, FONT_SMALL,
                   CANVAS_PRIMARY_FOREGROUND, "Choose an app");
    RtFormAddButton(MainForm, ACTION_CONTROL, 9, 28,
                    button_right_for(9, "Control Panel"), 40,
                    FONT_SMALL, "Control Panel");
    RtFormAddButton(MainForm, ACTION_SHOWCASE, 9, 48,
                    button_right_for(9, "Showcase"), 60,
                    FONT_SMALL, "Showcase");
    RtFormAddButton(MainForm, ACTION_FONT, 9, 68,
                    button_right_for(9, "Font Viewer"), 80,
                    FONT_SMALL, "Font Viewer");
    RtFormAddButton(MainForm, ACTION_EXIT, 9, 158,
                    button_right_for(9, "Exit XTOS"), 170,
                    FONT_SMALL, "Exit XTOS");
}

static void LauncherHandleEvent(Event *event)
{
    FormAction action;

    if (event->type == EVENT_KEYDOWN && event->key == XTOS_KEY_ALT_Q) {
        AppQuit();
        return;
    }

    RtFormDispatch(MainForm, event, &action);

    if (action.type == RT_FORM_ACTION_BUTTON &&
        action.control_id == ACTION_CONTROL) {
        launch_and_quit("control.exe");
    } else if (action.type == RT_FORM_ACTION_BUTTON &&
               action.control_id == ACTION_SHOWCASE) {
        launch_and_quit("showcase.exe");
    } else if (action.type == RT_FORM_ACTION_BUTTON &&
               action.control_id == ACTION_FONT) {
        launch_and_quit("font.exe");
    } else if ((action.type == RT_FORM_ACTION_BUTTON &&
                action.control_id == ACTION_EXIT) ||
               action.type == RT_FORM_ACTION_CLOSE) {
        AppQuit();
    }
}

static void LauncherDraw(void)
{
    RtFormDraw(MainForm);
}

static void LauncherShutdown(void)
{
    RtFormDestroy(MainForm);
}

int main(void)
{
    static Application LauncherApp;

    LauncherApp.Init = LauncherInit;
    LauncherApp.HandleEvent = LauncherHandleEvent;
    LauncherApp.Draw = LauncherDraw;
    LauncherApp.Shutdown = LauncherShutdown;
    LauncherApp.first_draw_screenshot_path = "launcher.cga";

    return AppRun(&LauncherApp);
}
