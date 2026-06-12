#include "xtos/app.h"
#include "xtos/display.h"
#include "xtos/event.h"
#include "xtos/ui/fonts.h"
#include "xtos/ui/form.h"

#define ACTION_RESET 1
#define ACTION_APPROVE 2

static Form MainForm;
static List FruitList;
static List CheeseList;
static Label FruitLabel;
static Label CheeseLabel;
static Label StatusLabel;
static Button ResetButton;
static Button ApproveButton;
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
                 CanvasTextWidth(&Font4x6, text) +
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

    append_text(&cursor, end, fruits[ListSelected(&FruitList)]);
    append_text(&cursor, end, " and ");
    append_text(&cursor, end, cheeses[ListSelected(&CheeseList)]);
    append_text(&cursor, end, ", yum!");
    *cursor = 0;

    LabelSetText(&StatusLabel, status_text);
}

static void ShowcaseInit(void)
{
    u16 approve_right;
    u16 reset_left;

    approve_right = button_right_for(7, "Approve");
    reset_left = (u16)(approve_right + UI_GAP + 1);

    FormInit(&MainForm, "Widget Showcase", &Font5x7);
    LabelInit(&FruitLabel, 9, 6, &Font4x6, "Fruits",
              CANVAS_PRIMARY_FOREGROUND);
    LabelInit(&CheeseLabel, 129, 6, &Font4x6, "Cheese",
              CANVAS_PRIMARY_FOREGROUND);
    LabelInit(&StatusLabel, 7, 176, &Font4x6, "",
              CANVAS_EXTRA_1_ON_BACKGROUND);

    ListInit(&FruitList, 7, 14, 104, &Font4x6, fruits, 4);
    ListInit(&CheeseList, 127, 14, 144, &Font4x6, cheeses, 6);
    ListSetSelected(&FruitList, 0);
    ListSetSelected(&CheeseList, 1);

    ButtonInit(&ApproveButton, ACTION_APPROVE, 7, 158, approve_right, 170,
               &Font4x6, "Approve");
    ButtonInit(&ResetButton, ACTION_RESET, reset_left, 158,
               button_right_for(reset_left, "Reset"), 170,
               &Font4x6, "Reset");

    FormAddLabel(&MainForm, &FruitLabel);
    FormAddLabel(&MainForm, &CheeseLabel);
    FormAddLabel(&MainForm, &StatusLabel);
    FormAddList(&MainForm, &FruitList);
    FormAddList(&MainForm, &CheeseList);
    FormAddButton(&MainForm, &ResetButton);
    FormAddButton(&MainForm, &ApproveButton);
}

static void ShowcaseHandleEvent(Event *event)
{
    int action;

    if (event->type == EVENT_KEYDOWN && event->key == XTOS_KEY_ALT_Q) {
        AppQuit();
        return;
    }

    action = FormHandleEvent(&MainForm, event);

    if (action == ACTION_RESET) {
        LabelSetText(&StatusLabel, "");
    } else if (action == ACTION_APPROVE) {
        update_status_label();
    } else if (action == FORM_ACTION_CLOSE) {
        AppQuit();
    }
}

static void ShowcaseDraw(void)
{
    FormDraw(&MainForm);
}

static void ShowcaseShutdown(void)
{
    DisplayShutdown();
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
