#include "canvas.h"
#include "display.h"
#include "event.h"
#include "int60.h"
#include "log.h"
#include "screenshot.h"
#include "system.h"

u16 XtosInt60Dispatch(XtosPb XTOS_FAR *pb)
{
    u16 status;
    const SystemPrefs XTOS_FAR **prefs_out;

    if (pb == 0) {
        return XTOS_RESULT_BAD_PARAMETER;
    }

    status = XTOS_RESULT_OK;

    switch (pb->opcode) {
    case XTOS_OP_PING:
        break;

    case XTOS_OP_LOG:
        if (pb->addr_in == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        XTOS_LOG_PREFIX("[APP]", (const char *)pb->addr_in);
        break;

    case XTOS_OP_SCREENSHOT_CGA:
        if (!RuntimeScreenshotCga((const char *)pb->addr_in)) {
            status = XTOS_RESULT_IO_ERROR;
        }
        break;

    case XTOS_OP_DISPLAY_SET_MODE:
        if (pb->int_in == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        RuntimeDisplaySetMode((enum DisplayMode)pb->int_in[0]);
        break;

    case XTOS_OP_DISPLAY_CURRENT_MODE:
        if (pb->int_out == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        pb->int_out[0] = (u16)RuntimeDisplayCurrentMode();
        break;

    case XTOS_OP_DISPLAY_SET_PALETTE:
        if (pb->int_in == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        RuntimeDisplaySetPalette((enum DisplayPalette)pb->int_in[0]);
        break;

    case XTOS_OP_DISPLAY_CURRENT_PALETTE:
        if (pb->int_out == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        pb->int_out[0] = (u16)RuntimeDisplayCurrentPalette();
        break;

    case XTOS_OP_GET_EVENT:
        if (pb->addr_out == 0 || pb->int_out == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        pb->int_out[0] = (u16)EventGet((Event *)pb->addr_out);
        break;

    case XTOS_OP_SYSTEM_PREFS_LOAD:
        if (pb->addr_out == 0 || pb->int_out == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        pb->int_out[0] = (u16)RuntimeSystemPrefsLoad((SystemPrefs *)pb->addr_out);
        break;

    case XTOS_OP_SYSTEM_PREFS_SAVE:
        if (pb->addr_in == 0 || pb->int_out == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        pb->int_out[0] = (u16)RuntimeSystemPrefsSave((const SystemPrefs *)pb->addr_in);
        break;

    case XTOS_OP_SYSTEM_PREFS_CURRENT:
        if (pb->addr_out == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        prefs_out = (const SystemPrefs XTOS_FAR **)pb->addr_out;
        *prefs_out = RuntimeSystemPrefsCurrent();
        break;

    case XTOS_OP_SYSTEM_PREFS_APPLY:
        if (pb->addr_in == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        RuntimeSystemPrefsApply((const SystemPrefs *)pb->addr_in);
        break;

    case XTOS_OP_CANVAS_CLEAR:
        RuntimeCanvasClear();
        break;

    case XTOS_OP_CANVAS_CLEAR_RECT:
        if (pb->int_in == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        RuntimeCanvasClearRect(pb->int_in[0], pb->int_in[1],
                               pb->int_in[2], pb->int_in[3]);
        break;

    case XTOS_OP_CANVAS_RECT:
        if (pb->int_in == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        RuntimeCanvasRect(pb->int_in[0], pb->int_in[1],
                          pb->int_in[2], pb->int_in[3],
                          (enum CanvasColorRole)pb->int_in[4]);
        break;

    case XTOS_OP_CANVAS_DOTTED_RECT:
        if (pb->int_in == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        RuntimeCanvasDottedRect(pb->int_in[0], pb->int_in[1],
                                pb->int_in[2], pb->int_in[3],
                                (enum CanvasColorRole)pb->int_in[4]);
        break;

    case XTOS_OP_CANVAS_FILL_RECT:
        if (pb->int_in == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        RuntimeCanvasFillRect(pb->int_in[0], pb->int_in[1],
                              pb->int_in[2], pb->int_in[3],
                              (enum CanvasColorRole)pb->int_in[4]);
        break;

    case XTOS_OP_CANVAS_TEXT:
        if (pb->int_in == 0 || pb->addr_in == 0 || pb->addr_out == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        RuntimeCanvasText(pb->int_in[0], pb->int_in[1],
                          (const Font *)pb->addr_in,
                          (enum CanvasColorRole)pb->int_in[2],
                          (const char *)pb->addr_out);
        break;

    case XTOS_OP_CANVAS_TEXT_WIDTH:
        if (pb->int_out == 0 || pb->addr_in == 0 || pb->addr_out == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        pb->int_out[0] =
            RuntimeCanvasTextWidth((const Font *)pb->addr_in,
                                   (const char *)pb->addr_out);
        break;

    case XTOS_OP_CANVAS_PRESENT:
        RuntimeCanvasPresent();
        break;

    default:
        status = XTOS_RESULT_UNKNOWN_OPCODE;
        XTOS_LOG_PREFIX_U16("[INT60]", "unknown", pb->opcode);
        break;
    }

    pb->result = status;
    return status;
}
