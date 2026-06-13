#include "xtos/app.h"
#include "xtos/display.h"
#include "xtos/event.h"
#include "xtos/system.h"
#include "xtos/ui/canvas.h"
#include "xtos/ui/font.h"
#include "xtos/ui/layout.h"
#include "xtos/ui/runtime_form.h"

#define MAX_MODE_ITEMS 4
#define MAX_PALETTE_ITEMS 8

#define CONTROL_MODE_LIST 1
#define CONTROL_PALETTE_LIST 2
#define CONTROL_STATUS_LABEL 3
#define CONTROL_APPLY_BUTTON 4
#define CONTROL_SAVE_BUTTON 5
#define CONTROL_MODE_LABEL 6
#define CONTROL_PALETTE_LABEL 7

static FormId MainForm;

static const char *mode_items[MAX_MODE_ITEMS];
static const char *palette_items[MAX_PALETTE_ITEMS];

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

    mode = DisplayModeInfoAt(RtFormListSelected(MainForm, CONTROL_MODE_LIST));
    palette = DisplayPaletteInfoAt(RtFormListSelected(MainForm,
                                                      CONTROL_PALETTE_LIST));

    prefs->mode = mode != 0 ? mode->id : DISPLAY_MODE_LOW;
    prefs->palette = palette != 0 ? palette->id :
                     DISPLAY_PALETTE_BLUE_ON_WHITE;
}

static void set_status(const char *text)
{
    RtFormSetLabelText(MainForm, CONTROL_STATUS_LABEL, text);
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

    MainForm = RtFormCreate("Control Panel", FONT_SYSTEM);
    if (MainForm == RT_FORM_INVALID) {
        AppQuit();
        return;
    }

    RtFormAddLabel(MainForm, CONTROL_MODE_LABEL, 9, 6, FONT_SMALL,
                   CANVAS_PRIMARY_FOREGROUND, "Display Mode");
    RtFormAddLabel(MainForm, CONTROL_PALETTE_LABEL, 129, 6, FONT_SMALL,
                   CANVAS_PRIMARY_FOREGROUND, "Palette");
    RtFormAddLabel(MainForm, CONTROL_STATUS_LABEL, 7, 176, FONT_SMALL,
                   CANVAS_EXTRA_1_ON_BACKGROUND, "");

    RtFormAddList(MainForm, CONTROL_MODE_LIST, 7, 14, 104, FONT_SMALL,
                  mode_items, DisplayModeCount());
    RtFormAddList(MainForm, CONTROL_PALETTE_LIST, 127, 14, 144, FONT_SMALL,
                  palette_items, DisplayPaletteCount());
    RtFormListSetSelected(MainForm, CONTROL_MODE_LIST,
                          mode_index_for(prefs->mode));
    RtFormListSetSelected(MainForm, CONTROL_PALETTE_LIST,
                          palette_index_for(prefs->palette));

    RtFormAddButton(MainForm, CONTROL_APPLY_BUTTON, 7, 158,
                    apply_right, 170, FONT_SMALL, "Apply");
    RtFormAddButton(MainForm, CONTROL_SAVE_BUTTON, save_left, 158,
                    button_right_for(save_left, "Save"), 170,
                    FONT_SMALL, "Save");
    set_status("Current settings loaded");
}

static void apply_selected(void)
{
    SystemPrefs prefs;

    selected_prefs(&prefs);
    SystemPrefsApply(&prefs);
    RtFormInvalidate(MainForm);
    set_status("Settings applied");
}

static void save_selected(void)
{
    SystemPrefs prefs;

    selected_prefs(&prefs);
    SystemPrefsApply(&prefs);
    RtFormInvalidate(MainForm);

    if (SystemPrefsSave(&prefs)) {
        set_status("Settings saved to XTOS.CFG");
    } else {
        set_status("Could not save XTOS.CFG");
    }
}

static void ControlPanelHandleEvent(Event *event)
{
    FormAction action;

    if (event->type == EVENT_KEYDOWN && event->key == XTOS_KEY_ALT_Q) {
        AppQuit();
    } else if (event->type == EVENT_KEYDOWN &&
               (event->key == 's' || event->key == 'S')) {
        save_selected();
    } else {
        RtFormDispatch(MainForm, event, &action);

        if (action.type == RT_FORM_ACTION_BUTTON &&
            action.control_id == CONTROL_APPLY_BUTTON) {
            apply_selected();
        } else if (action.type == RT_FORM_ACTION_BUTTON &&
                   action.control_id == CONTROL_SAVE_BUTTON) {
            save_selected();
        } else if (action.type == RT_FORM_ACTION_CLOSE) {
            AppQuit();
        }
    }
}

static void ControlPanelDraw(void)
{
    RtFormDraw(MainForm);
}

static void ControlPanelShutdown(void)
{
    RtFormDestroy(MainForm);
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
