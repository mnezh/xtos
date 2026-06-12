#include "xtos/app.h"
#include "xtos/event.h"
#include "xtos/exec.h"
#include "xtos/ui/canvas.h"
#include "xtos/ui/font.h"
#include "xtos/ui/form.h"

#define ACTION_CONTROL 1
#define ACTION_SHOWCASE 2
#define ACTION_FONT 3
#define ACTION_EXIT 4

static Form MainForm;
static Label TitleLabel;
static Button ControlButton;
static Button ShowcaseButton;
static Button FontButton;
static Button ExitButton;

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
    FormInit(&MainForm, "XTOS Launcher", FontGet(FONT_SYSTEM));
    LabelInit(&TitleLabel, 9, 8, FontGet(FONT_SMALL), "Choose an app",
              CANVAS_PRIMARY_FOREGROUND);

    ButtonInit(&ControlButton, ACTION_CONTROL, 9, 28,
               button_right_for(9, "Control Panel"), 40,
               FontGet(FONT_SMALL), "Control Panel");
    ButtonInit(&ShowcaseButton, ACTION_SHOWCASE, 9, 48,
               button_right_for(9, "Showcase"), 60,
               FontGet(FONT_SMALL), "Showcase");
    ButtonInit(&FontButton, ACTION_FONT, 9, 68,
               button_right_for(9, "Font Viewer"), 80,
               FontGet(FONT_SMALL), "Font Viewer");
    ButtonInit(&ExitButton, ACTION_EXIT, 9, 158,
               button_right_for(9, "Exit XTOS"), 170,
               FontGet(FONT_SMALL), "Exit XTOS");

    FormAddLabel(&MainForm, &TitleLabel);
    FormAddButton(&MainForm, &ControlButton);
    FormAddButton(&MainForm, &ShowcaseButton);
    FormAddButton(&MainForm, &FontButton);
    FormAddButton(&MainForm, &ExitButton);
}

static void LauncherHandleEvent(Event *event)
{
    int action;

    if (event->type == EVENT_KEYDOWN && event->key == XTOS_KEY_ALT_Q) {
        AppQuit();
        return;
    }

    action = FormHandleEvent(&MainForm, event);

    if (action == ACTION_CONTROL) {
        launch_and_quit("control.exe");
    } else if (action == ACTION_SHOWCASE) {
        launch_and_quit("showcase.exe");
    } else if (action == ACTION_FONT) {
        launch_and_quit("font.exe");
    } else if (action == ACTION_EXIT || action == FORM_ACTION_CLOSE) {
        AppQuit();
    }
}

static void LauncherDraw(void)
{
    FormDraw(&MainForm);
}

int main(void)
{
    static Application LauncherApp;

    LauncherApp.Init = LauncherInit;
    LauncherApp.HandleEvent = LauncherHandleEvent;
    LauncherApp.Draw = LauncherDraw;
    LauncherApp.Shutdown = 0;
    LauncherApp.first_draw_screenshot_path = "launcher.cga";

    return AppRun(&LauncherApp);
}
