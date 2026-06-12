#include "../xtos/display.h"
#include "../xtos/event.h"
#include "../xtos/system.h"
#include "../xtos/ui/canvas.h"
#include "int60.h"
#include "log.h"

static u16 app_call(XtosPb XTOS_FAR *pb)
{
    if (pb->opcode != XTOS_OP_GET_EVENT &&
        pb->opcode != XTOS_OP_DISPLAY_CURRENT_MODE &&
        pb->opcode != XTOS_OP_DISPLAY_CURRENT_PALETTE &&
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
    XtosPb pb;
    u16 int_in[3];

    int_in[0] = x;
    int_in[1] = y;
    int_in[2] = (u16)role;
    pb.opcode = XTOS_OP_CANVAS_TEXT;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = 0;
    pb.addr_in = (void XTOS_FAR *)font;
    pb.addr_out = (void XTOS_FAR *)text;
    app_call(&pb);
}

u16 CanvasTextWidth(const Font *font, const char *text)
{
    XtosPb pb;
    u16 int_out[1];

    int_out[0] = 0;
    pb.opcode = XTOS_OP_CANVAS_TEXT_WIDTH;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = 0;
    pb.int_out = int_out;
    pb.addr_in = (void XTOS_FAR *)font;
    pb.addr_out = (void XTOS_FAR *)text;
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
    return (int)int_out[0];
}

const SystemPrefs *SystemPrefsCurrent(void)
{
    XtosPb pb;
    const SystemPrefs *prefs;

    prefs = 0;
    pb.opcode = XTOS_OP_SYSTEM_PREFS_CURRENT;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = 0;
    pb.int_out = 0;
    pb.addr_in = 0;
    pb.addr_out = (void XTOS_FAR *)&prefs;
    app_call(&pb);
    return prefs;
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
