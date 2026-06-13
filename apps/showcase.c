#include "xtos/app.h"
#include "xtos/event.h"
#include "xtos/ui/canvas.h"
#include "xtos/ui/font.h"
#include "xtos/ui/layout.h"
#include "xtos/ui/runtime_form.h"

#define ACTION_RESET 1
#define ACTION_APPROVE 2
#define CONTROL_FRUIT_LIST 10
#define CONTROL_CHEESE_LIST 11
#define CONTROL_STATUS_LABEL 12
#define CONTROL_FRUIT_LABEL 13
#define CONTROL_CHEESE_LABEL 14

static FormId MainForm;
static char status_text[80];

static const char *fruits[] = {
    "Apple",
    "Orange",
    "Pear",
    "Grapes"
};

static const char *cheeses[] = {
    "Brie",
    "Pecorino",
    "Roquefort",
    "Gouda",
    "Адыгейский",
    "Hermelín"
};

static u16 button_right_for(u16 left, const char *text)
{
    return (u16)(left + UI_BUTTON_PAD_X +
                 CanvasTextWidth(FontGet(FONT_SMALL), text) +
                 UI_BUTTON_PAD_X - 1);
}

static void append_text(char **cursor, const char *end, const char *text)
{
    while (*text != 0 && *cursor < end) {
        **cursor = *text;
        ++*cursor;
        ++text;
    }
}

static void update_status_label(void)
{
    char *cursor;
    const char *end;

    cursor = status_text;
    end = status_text + sizeof(status_text) - 1;

    append_text(&cursor, end,
                fruits[RtFormListSelected(MainForm, CONTROL_FRUIT_LIST)]);
    append_text(&cursor, end, " and ");
    append_text(&cursor, end,
                cheeses[RtFormListSelected(MainForm, CONTROL_CHEESE_LIST)]);
    append_text(&cursor, end, ", yum!");
    *cursor = 0;

    RtFormSetLabelText(MainForm, CONTROL_STATUS_LABEL, status_text);
}

static void ShowcaseInit(void)
{
    u16 approve_right;
    u16 reset_left;

    approve_right = button_right_for(7, "Approve");
    reset_left = (u16)(approve_right + UI_GAP + 1);

    MainForm = RtFormCreate("Widget Showcase", FONT_SYSTEM);
    if (MainForm == RT_FORM_INVALID) {
        AppQuit();
        return;
    }

    RtFormAddLabel(MainForm, CONTROL_FRUIT_LABEL, 9, 6, FONT_SMALL,
                   CANVAS_PRIMARY_FOREGROUND, "Fruits");
    RtFormAddLabel(MainForm, CONTROL_CHEESE_LABEL, 129, 6, FONT_SMALL,
                   CANVAS_PRIMARY_FOREGROUND, "Cheese");
    RtFormAddLabel(MainForm, CONTROL_STATUS_LABEL, 7, 176, FONT_SMALL,
                   CANVAS_EXTRA_1_ON_BACKGROUND, "");

    RtFormAddList(MainForm, CONTROL_FRUIT_LIST, 7, 14, 104, FONT_SMALL,
                  fruits, 4);
    RtFormAddList(MainForm, CONTROL_CHEESE_LIST, 127, 14, 144, FONT_SMALL,
                  cheeses, 6);
    RtFormListSetSelected(MainForm, CONTROL_FRUIT_LIST, 0);
    RtFormListSetSelected(MainForm, CONTROL_CHEESE_LIST, 1);

    RtFormAddButton(MainForm, ACTION_APPROVE, 7, 158, approve_right, 170,
                    FONT_SMALL, "Approve");
    RtFormAddButton(MainForm, ACTION_RESET, reset_left, 158,
                    button_right_for(reset_left, "Reset"), 170,
                    FONT_SMALL, "Reset");
}

static void ShowcaseHandleEvent(Event *event)
{
    FormAction action;

    if (event->type == EVENT_KEYDOWN && event->key == XTOS_KEY_ALT_Q) {
        AppQuit();
        return;
    }

    RtFormDispatch(MainForm, event, &action);

    if (action.type == RT_FORM_ACTION_BUTTON &&
        action.control_id == ACTION_RESET) {
        RtFormSetLabelText(MainForm, CONTROL_STATUS_LABEL, "");
    } else if (action.type == RT_FORM_ACTION_BUTTON &&
               action.control_id == ACTION_APPROVE) {
        update_status_label();
    } else if (action.type == RT_FORM_ACTION_CLOSE) {
        AppQuit();
    }
}

static void ShowcaseDraw(void)
{
    RtFormDraw(MainForm);
}

static void ShowcaseShutdown(void)
{
    RtFormDestroy(MainForm);
}

int main(void)
{
    static Application ShowcaseApp;

    ShowcaseApp.Init = ShowcaseInit;
    ShowcaseApp.HandleEvent = ShowcaseHandleEvent;
    ShowcaseApp.Draw = ShowcaseDraw;
    ShowcaseApp.Shutdown = ShowcaseShutdown;
    ShowcaseApp.first_draw_screenshot_path = "showcase.cga";

    return AppRun(&ShowcaseApp);
}
