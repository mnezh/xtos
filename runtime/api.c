#include "../xtos/display.h"
#include "../xtos/event.h"
#include "../xtos/exec.h"
#include "../xtos/system.h"
#include "../xtos/ui/canvas.h"
#include "../xtos/ui/invalidation.h"
#include "../xtos/ui/runtime_form.h"
#include "int60.h"
#include "log.h"

static u16 app_call(XtosPb XTOS_FAR *pb)
{
    if (pb->opcode != XTOS_OP_GET_EVENT &&
        pb->opcode != XTOS_OP_DISPLAY_CURRENT_MODE &&
        pb->opcode != XTOS_OP_DISPLAY_CURRENT_PALETTE &&
        pb->opcode != XTOS_OP_SYSTEM_PREFS_CURRENT &&
        pb->opcode < XTOS_OP_CANVAS_CLEAR) {
        XTOS_LOG_PREFIX_U16("[APP]", "op", pb->opcode);
    }
    return XtosInt60Call(pb);
}

void CanvasClear(void)
{
    XtosPb pb;

    pb.opcode = XTOS_OP_CANVAS_CLEAR;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = 0;
    pb.int_out = 0;
    pb.addr_in = 0;
    pb.addr_out = 0;
    app_call(&pb);
}

void CanvasClearRect(u16 left, u16 top, u16 right, u16 bottom)
{
    XtosPb pb;
    u16 int_in[4];

    int_in[0] = left;
    int_in[1] = top;
    int_in[2] = right;
    int_in[3] = bottom;
    pb.opcode = XTOS_OP_CANVAS_CLEAR_RECT;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = 0;
    pb.addr_in = 0;
    pb.addr_out = 0;
    app_call(&pb);
}

void CanvasRect(u16 left, u16 top, u16 right, u16 bottom,
                enum CanvasColorRole role)
{
    XtosPb pb;
    u16 int_in[5];

    int_in[0] = left;
    int_in[1] = top;
    int_in[2] = right;
    int_in[3] = bottom;
    int_in[4] = (u16)role;
    pb.opcode = XTOS_OP_CANVAS_RECT;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = 0;
    pb.addr_in = 0;
    pb.addr_out = 0;
    app_call(&pb);
}

void CanvasDottedRect(u16 left, u16 top, u16 right, u16 bottom,
                      enum CanvasColorRole role)
{
    XtosPb pb;
    u16 int_in[5];

    int_in[0] = left;
    int_in[1] = top;
    int_in[2] = right;
    int_in[3] = bottom;
    int_in[4] = (u16)role;
    pb.opcode = XTOS_OP_CANVAS_DOTTED_RECT;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = 0;
    pb.addr_in = 0;
    pb.addr_out = 0;
    app_call(&pb);
}

void CanvasFillRect(u16 left, u16 top, u16 right, u16 bottom,
                    enum CanvasColorRole role)
{
    XtosPb pb;
    u16 int_in[5];

    int_in[0] = left;
    int_in[1] = top;
    int_in[2] = right;
    int_in[3] = bottom;
    int_in[4] = (u16)role;
    pb.opcode = XTOS_OP_CANVAS_FILL_RECT;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = 0;
    pb.addr_in = 0;
    pb.addr_out = 0;
    app_call(&pb);
}

void CanvasText(u16 x, u16 y, const Font *font,
                enum CanvasColorRole role, const char *text)
{
    CanvasTextId(x, y, FontIdOf(font), role, text);
}

void CanvasTextId(u16 x, u16 y, FontId font_id,
                  enum CanvasColorRole role, const char *text)
{
    XtosPb pb;
    u16 int_in[4];

    int_in[0] = x;
    int_in[1] = y;
    int_in[2] = font_id;
    int_in[3] = (u16)role;
    pb.opcode = XTOS_OP_CANVAS_TEXT;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = 0;
    pb.addr_in = (void XTOS_FAR *)text;
    pb.addr_out = 0;
    app_call(&pb);
}

u16 CanvasTextWidth(const Font *font, const char *text)
{
    return CanvasTextWidthId(FontIdOf(font), text);
}

u16 CanvasTextWidthId(FontId font_id, const char *text)
{
    XtosPb pb;
    u16 int_in[1];
    u16 int_out[1];

    int_in[0] = font_id;
    int_out[0] = 0;
    pb.opcode = XTOS_OP_CANVAS_TEXT_WIDTH;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = int_out;
    pb.addr_in = (void XTOS_FAR *)text;
    pb.addr_out = 0;
    app_call(&pb);
    return int_out[0];
}

void CanvasPresent(void)
{
    XtosPb pb;

    pb.opcode = XTOS_OP_CANVAS_PRESENT;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = 0;
    pb.int_out = 0;
    pb.addr_in = 0;
    pb.addr_out = 0;
    app_call(&pb);
}

int XtosScreenshotCga(const char *path)
{
    XtosPb pb;

    pb.opcode = XTOS_OP_SCREENSHOT_CGA;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = 0;
    pb.int_out = 0;
    pb.addr_in = (void XTOS_FAR *)path;
    pb.addr_out = 0;
    app_call(&pb);
    return pb.result == XTOS_RESULT_OK;
}

int ExecRequest(const char *path)
{
    XtosPb pb;

    if (path == 0 || path[0] == 0) {
        return 0;
    }

    pb.opcode = XTOS_OP_EXEC_REQUEST;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = 0;
    pb.int_out = 0;
    pb.addr_in = (void XTOS_FAR *)path;
    pb.addr_out = 0;
    app_call(&pb);
    return pb.result == XTOS_RESULT_OK;
}

void DisplaySetMode(enum DisplayMode mode)
{
    XtosPb pb;
    u16 int_in[1];

    int_in[0] = (u16)mode;
    pb.opcode = XTOS_OP_DISPLAY_SET_MODE;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = 0;
    pb.addr_in = 0;
    pb.addr_out = 0;
    app_call(&pb);
}

enum DisplayMode DisplayCurrentMode(void)
{
    XtosPb pb;
    u16 int_out[1];

    int_out[0] = 0;
    pb.opcode = XTOS_OP_DISPLAY_CURRENT_MODE;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = 0;
    pb.int_out = int_out;
    pb.addr_in = 0;
    pb.addr_out = 0;
    app_call(&pb);
    return (enum DisplayMode)int_out[0];
}

void DisplaySetPalette(enum DisplayPalette palette)
{
    XtosPb pb;
    u16 int_in[1];

    int_in[0] = (u16)palette;
    pb.opcode = XTOS_OP_DISPLAY_SET_PALETTE;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = 0;
    pb.addr_in = 0;
    pb.addr_out = 0;
    app_call(&pb);
}

enum DisplayPalette DisplayCurrentPalette(void)
{
    XtosPb pb;
    u16 int_out[1];

    int_out[0] = 0;
    pb.opcode = XTOS_OP_DISPLAY_CURRENT_PALETTE;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = 0;
    pb.int_out = int_out;
    pb.addr_in = 0;
    pb.addr_out = 0;
    app_call(&pb);
    return (enum DisplayPalette)int_out[0];
}

int GetEvent(Event *event)
{
    XtosPb pb;
    u16 int_out[1];

    if (event == 0) {
        return 0;
    }

    int_out[0] = 0;
    pb.opcode = XTOS_OP_GET_EVENT;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = 0;
    pb.int_out = int_out;
    pb.addr_in = 0;
    pb.addr_out = (void XTOS_FAR *)event;
    app_call(&pb);
    return (int)int_out[0];
}

int SystemPrefsLoad(SystemPrefs *prefs)
{
    XtosPb pb;
    u16 int_out[1];

    if (prefs == 0) {
        return 0;
    }

    int_out[0] = 0;
    pb.opcode = XTOS_OP_SYSTEM_PREFS_LOAD;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = 0;
    pb.int_out = int_out;
    pb.addr_in = 0;
    pb.addr_out = (void XTOS_FAR *)prefs;
    app_call(&pb);
    return (int)int_out[0];
}

int SystemPrefsSave(const SystemPrefs *prefs)
{
    XtosPb pb;
    u16 int_out[1];

    if (prefs == 0) {
        return 0;
    }

    int_out[0] = 0;
    pb.opcode = XTOS_OP_SYSTEM_PREFS_SAVE;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = 0;
    pb.int_out = int_out;
    pb.addr_in = (void XTOS_FAR *)prefs;
    pb.addr_out = 0;
    app_call(&pb);
    XTOS_LOG_PREFIX_U16("[APP]", "prefs_save_pb_result", pb.result);
    XTOS_LOG_PREFIX_U16("[APP]", "prefs_save_result", int_out[0]);
    return (int)int_out[0];
}

const SystemPrefs *SystemPrefsCurrent(void)
{
    XtosPb pb;
    static SystemPrefs prefs;

    pb.opcode = XTOS_OP_SYSTEM_PREFS_CURRENT;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = 0;
    pb.int_out = 0;
    pb.addr_in = 0;
    pb.addr_out = (void XTOS_FAR *)&prefs;
    app_call(&pb);
    return &prefs;
}

void SystemPrefsApply(const SystemPrefs *prefs)
{
    XtosPb pb;

    if (prefs == 0) {
        return;
    }

    pb.opcode = XTOS_OP_SYSTEM_PREFS_APPLY;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = 0;
    pb.int_out = 0;
    pb.addr_in = (void XTOS_FAR *)prefs;
    pb.addr_out = 0;
    app_call(&pb);
}

FormId RtFormCreate(const char *title, FontId font_id)
{
    XtosPb pb;
    u16 int_in[1];
    u16 int_out[1];

    if (title == 0) {
        return RT_FORM_INVALID;
    }

    int_in[0] = font_id;
    int_out[0] = RT_FORM_INVALID;
    pb.opcode = XTOS_OP_FORM_CREATE;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = int_out;
    pb.addr_in = (void XTOS_FAR *)title;
    pb.addr_out = 0;
    app_call(&pb);
    return pb.result == XTOS_RESULT_OK ? int_out[0] : RT_FORM_INVALID;
}

int RtFormDestroy(FormId form_id)
{
    XtosPb pb;
    u16 int_in[1];
    u16 int_out[1];

    int_in[0] = form_id;
    int_out[0] = 0;
    pb.opcode = XTOS_OP_FORM_DESTROY;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = int_out;
    pb.addr_in = 0;
    pb.addr_out = 0;
    app_call(&pb);
    return pb.result == XTOS_RESULT_OK && int_out[0] != 0;
}

int RtFormAddLabel(FormId form_id, ControlId control_id,
                   u16 x, u16 y, FontId font_id,
                   enum CanvasColorRole color, const char *text)
{
    XtosPb pb;
    u16 int_in[7];
    u16 int_out[1];

    if (text == 0) {
        return 0;
    }

    int_in[0] = form_id;
    int_in[1] = control_id;
    int_in[2] = x;
    int_in[3] = y;
    int_in[4] = font_id;
    int_in[5] = (u16)color;
    int_out[0] = 0;
    pb.opcode = XTOS_OP_FORM_ADD_LABEL;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = int_out;
    pb.addr_in = (void XTOS_FAR *)text;
    pb.addr_out = 0;
    app_call(&pb);
    return pb.result == XTOS_RESULT_OK && int_out[0] != 0;
}

int RtFormAddButton(FormId form_id, ControlId control_id,
                    u16 left, u16 top, u16 right, u16 bottom,
                    FontId font_id, const char *text)
{
    XtosPb pb;
    u16 int_in[7];
    u16 int_out[1];

    if (text == 0) {
        return 0;
    }

    int_in[0] = form_id;
    int_in[1] = control_id;
    int_in[2] = left;
    int_in[3] = top;
    int_in[4] = right;
    int_in[5] = bottom;
    int_in[6] = font_id;
    int_out[0] = 0;
    pb.opcode = XTOS_OP_FORM_ADD_BUTTON;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = int_out;
    pb.addr_in = (void XTOS_FAR *)text;
    pb.addr_out = 0;
    app_call(&pb);
    return pb.result == XTOS_RESULT_OK && int_out[0] != 0;
}

static int rt_form_set_list_item(FormId form_id, ControlId control_id,
                                 u8 index, const char *text)
{
    XtosPb pb;
    u16 int_in[3];
    u16 int_out[1];

    if (text == 0) {
        return 0;
    }

    int_in[0] = form_id;
    int_in[1] = control_id;
    int_in[2] = index;
    int_out[0] = 0;
    pb.opcode = XTOS_OP_FORM_SET_LIST_ITEM;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = int_out;
    pb.addr_in = (void XTOS_FAR *)text;
    pb.addr_out = 0;
    app_call(&pb);
    return pb.result == XTOS_RESULT_OK && int_out[0] != 0;
}

int RtFormAddList(FormId form_id, ControlId control_id,
                  u16 x, u16 y, u16 width, FontId font_id,
                  const char * const *items, u8 count)
{
    XtosPb pb;
    u16 int_in[7];
    u16 int_out[1];
    u8 i;

    if (items == 0 || count == 0) {
        return 0;
    }

    int_in[0] = form_id;
    int_in[1] = control_id;
    int_in[2] = x;
    int_in[3] = y;
    int_in[4] = width;
    int_in[5] = font_id;
    int_in[6] = count;
    int_out[0] = 0;
    pb.opcode = XTOS_OP_FORM_ADD_LIST;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = int_out;
    pb.addr_in = 0;
    pb.addr_out = 0;
    app_call(&pb);

    if (pb.result != XTOS_RESULT_OK || int_out[0] == 0) {
        return 0;
    }

    for (i = 0; i < count; ++i) {
        if (!rt_form_set_list_item(form_id, control_id, i, items[i])) {
            return 0;
        }
    }

    return 1;
}

u8 RtFormListSelected(FormId form_id, ControlId control_id)
{
    XtosPb pb;
    u16 int_in[2];
    u16 int_out[2];

    int_in[0] = form_id;
    int_in[1] = control_id;
    int_out[0] = 0;
    int_out[1] = 0;
    pb.opcode = XTOS_OP_FORM_LIST_SELECTED;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = int_out;
    pb.addr_in = 0;
    pb.addr_out = 0;
    app_call(&pb);
    return (u8)int_out[1];
}

int RtFormListSetSelected(FormId form_id, ControlId control_id, u8 selected)
{
    XtosPb pb;
    u16 int_in[3];
    u16 int_out[1];

    int_in[0] = form_id;
    int_in[1] = control_id;
    int_in[2] = selected;
    int_out[0] = 0;
    pb.opcode = XTOS_OP_FORM_LIST_SET_SELECTED;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = int_out;
    pb.addr_in = 0;
    pb.addr_out = 0;
    app_call(&pb);
    if (pb.result == XTOS_RESULT_OK && int_out[0] != 0) {
        InvalidateAll();
        return 1;
    }
    return 0;
}

int RtFormSetLabelText(FormId form_id, ControlId control_id,
                       const char *text)
{
    XtosPb pb;
    u16 int_in[2];
    u16 int_out[1];

    if (text == 0) {
        return 0;
    }

    int_in[0] = form_id;
    int_in[1] = control_id;
    int_out[0] = 0;
    pb.opcode = XTOS_OP_FORM_SET_LABEL_TEXT;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = int_out;
    pb.addr_in = (void XTOS_FAR *)text;
    pb.addr_out = 0;
    app_call(&pb);
    if (pb.result == XTOS_RESULT_OK && int_out[0] != 0) {
        InvalidateAll();
        return 1;
    }
    return 0;
}

int RtFormInvalidate(FormId form_id)
{
    XtosPb pb;
    u16 int_in[1];
    u16 int_out[1];

    int_in[0] = form_id;
    int_out[0] = 0;
    pb.opcode = XTOS_OP_FORM_INVALIDATE;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = int_out;
    pb.addr_in = 0;
    pb.addr_out = 0;
    app_call(&pb);
    if (pb.result == XTOS_RESULT_OK && int_out[0] != 0) {
        InvalidateAll();
        return 1;
    }
    return 0;
}

int RtFormDraw(FormId form_id)
{
    XtosPb pb;
    u16 int_in[1];
    u16 int_out[1];

    int_in[0] = form_id;
    int_out[0] = 0;
    pb.opcode = XTOS_OP_FORM_DRAW;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = int_out;
    pb.addr_in = 0;
    pb.addr_out = 0;
    app_call(&pb);
    return pb.result == XTOS_RESULT_OK && int_out[0] != 0;
}

int RtFormDispatch(FormId form_id, const Event *event, FormAction *action)
{
    XtosPb pb;
    u16 int_in[1];
    u16 int_out[1];

    if (event == 0 || action == 0) {
        return 0;
    }

    action->type = RT_FORM_ACTION_NONE;
    action->form_id = RT_FORM_INVALID;
    action->control_id = 0;
    action->value = 0;
    int_in[0] = form_id;
    int_out[0] = 0;
    pb.opcode = XTOS_OP_FORM_DISPATCH;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = int_out;
    pb.addr_in = (void XTOS_FAR *)event;
    pb.addr_out = (void XTOS_FAR *)action;
    app_call(&pb);
    if (pb.result == XTOS_RESULT_OK && int_out[0] != 0) {
        InvalidateAll();
        return 1;
    }
    return 0;
}
