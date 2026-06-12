#include "xtos/app.h"
#include "xtos/display.h"
#include "xtos/event.h"
#include "xtos/system.h"
#include "xtos/ui/canvas.h"
#include "xtos/ui/font.h"
#include "xtos/ui/form.h"

#define MAX_MODE_ITEMS 4
#define MAX_PALETTE_ITEMS 8
#define ACTION_APPLY 1
#define ACTION_SAVE 2

static Form MainForm;
static List ModeList;
static List PaletteList;
static Label ModeLabel;
static Label PaletteLabel;
static Label StatusLabel;
static Button ApplyButton;
static Button SaveButton;

static const char *mode_items[MAX_MODE_ITEMS];
static const char *palette_items[MAX_PALETTE_ITEMS];
static const char *status_text;

static u16 button_right_for(u16 left, const char *text)
{
    return (u16)(left + UI_BUTTON_PAD_X +
                 CanvasTextWidth(FontGet(FONT_SMALL), text) +
                 UI_BUTTON_PAD_X - 1);
}

static void load_display_items(void)
{
    u8 i;
    const DisplayModeInfo *mode;
    const DisplayPaletteInfo *palette;

    for (i = 0; i < DisplayModeCount() && i < MAX_MODE_ITEMS; ++i) {
        mode = DisplayModeInfoAt(i);
        mode_items[i] = mode != 0 ? mode->name : "";
    }

    for (i = 0; i < DisplayPaletteCount() && i < MAX_PALETTE_ITEMS; ++i) {
        palette = DisplayPaletteInfoAt(i);
        palette_items[i] = palette != 0 ? palette->name : "";
    }
}

static u8 mode_index_for(enum DisplayMode mode_id)
{
    u8 i;
    const DisplayModeInfo *mode;

    for (i = 0; i < DisplayModeCount(); ++i) {
        mode = DisplayModeInfoAt(i);

        if (mode != 0 && mode->id == mode_id) {
            return i;
        }
    }

    return 0;
}

static u8 palette_index_for(enum DisplayPalette palette_id)
{
    u8 i;
    const DisplayPaletteInfo *palette;

    for (i = 0; i < DisplayPaletteCount(); ++i) {
        palette = DisplayPaletteInfoAt(i);

        if (palette != 0 && palette->id == palette_id) {
            return i;
        }
    }

    return 0;
}

static void selected_prefs(SystemPrefs *prefs)
{
    const DisplayModeInfo *mode;
    const DisplayPaletteInfo *palette;

    mode = DisplayModeInfoAt(ListSelected(&ModeList));
    palette = DisplayPaletteInfoAt(ListSelected(&PaletteList));

    prefs->mode = mode != 0 ? mode->id : DISPLAY_MODE_LOW;
    prefs->palette = palette != 0 ? palette->id :
                     DISPLAY_PALETTE_BLUE_ON_WHITE;
}

static void set_status(const char *text)
{
    status_text = text;
    LabelSetText(&StatusLabel, status_text);
}

static void ControlPanelInit(void)
{
    const SystemPrefs *prefs;
    u16 apply_right;
    u16 save_left;

    load_display_items();
    prefs = SystemPrefsCurrent();
    apply_right = button_right_for(7, "Apply");
    save_left = (u16)(apply_right + UI_GAP + 1);

    FormInit(&MainForm, "Control Panel", FontGet(FONT_SYSTEM));
    LabelInit(&ModeLabel, 9, 6, FontGet(FONT_SMALL), "Display Mode",
              CANVAS_PRIMARY_FOREGROUND);
    LabelInit(&PaletteLabel, 129, 6, FontGet(FONT_SMALL), "Palette",
              CANVAS_PRIMARY_FOREGROUND);
    LabelInit(&StatusLabel, 7, 176, FontGet(FONT_SMALL), "",
              CANVAS_EXTRA_1_ON_BACKGROUND);
    ButtonInit(&ApplyButton, ACTION_APPLY, 7, 158, apply_right, 170,
               FontGet(FONT_SMALL), "Apply");
    ButtonInit(&SaveButton, ACTION_SAVE, save_left, 158,
               button_right_for(save_left, "Save"), 170,
               FontGet(FONT_SMALL), "Save");

    ListInit(&ModeList, 7, 14, 104, FontGet(FONT_SMALL), mode_items,
             DisplayModeCount());
    ListInit(&PaletteList, 127, 14, 144, FontGet(FONT_SMALL), palette_items,
             DisplayPaletteCount());
    ListSetSelected(&ModeList, mode_index_for(prefs->mode));
    ListSetSelected(&PaletteList, palette_index_for(prefs->palette));

    FormAddLabel(&MainForm, &ModeLabel);
    FormAddLabel(&MainForm, &PaletteLabel);
    FormAddLabel(&MainForm, &StatusLabel);
    FormAddList(&MainForm, &ModeList);
    FormAddList(&MainForm, &PaletteList);
    FormAddButton(&MainForm, &ApplyButton);
    FormAddButton(&MainForm, &SaveButton);
    set_status("Current settings loaded");
}

static void apply_selected(void)
{
    SystemPrefs prefs;

    selected_prefs(&prefs);
    SystemPrefsApply(&prefs);
    FormInvalidateAll(&MainForm);
    set_status("Settings applied");
}

static void save_selected(void)
{
    SystemPrefs prefs;

    selected_prefs(&prefs);
    SystemPrefsApply(&prefs);
    FormInvalidateAll(&MainForm);

    if (SystemPrefsSave(&prefs)) {
        set_status("Settings saved to XTOS.CFG");
    } else {
        set_status("Could not save XTOS.CFG");
    }
}

static void ControlPanelHandleEvent(Event *event)
{
    int action;

    if (event->type == EVENT_KEYDOWN && event->key == XTOS_KEY_ALT_Q) {
        AppQuit();
    } else if (event->type == EVENT_KEYDOWN &&
               (event->key == 's' || event->key == 'S')) {
        save_selected();
    } else {
        action = FormHandleEvent(&MainForm, event);

        if (action == ACTION_APPLY) {
            apply_selected();
        } else if (action == ACTION_SAVE) {
            save_selected();
        } else if (action == FORM_ACTION_CLOSE) {
            AppQuit();
        }
    }
}

static void ControlPanelDraw(void)
{
    FormDraw(&MainForm);
}

static void ControlPanelShutdown(void)
{
    DisplayShutdown();
}

int main(void)
{
    static Application ControlPanelApp;

    ControlPanelApp.Init = ControlPanelInit;
    ControlPanelApp.HandleEvent = ControlPanelHandleEvent;
    ControlPanelApp.Draw = ControlPanelDraw;
    ControlPanelApp.Shutdown = ControlPanelShutdown;
    ControlPanelApp.first_draw_screenshot_path = "control.cga";

    return AppRun(&ControlPanelApp);
}
