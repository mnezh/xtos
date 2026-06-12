#include "canvas.h"
#include "display.h"
#include "event.h"
#include "int60.h"
#include "log.h"
#include "screenshot.h"
#include "system.h"

#define DISPATCH_TEXT_BUFFER_SIZE 256

static char dispatch_text_buffer[DISPATCH_TEXT_BUFFER_SIZE];

char *XtosCopyFarString(char *dest, const char XTOS_FAR *source, u16 max);

static const char *copy_far_string(const char XTOS_FAR *text)
{
    if (text == 0) {
        return 0;
    }

    return XtosCopyFarString(dispatch_text_buffer, text,
                             DISPATCH_TEXT_BUFFER_SIZE);
}

u16 XtosInt60Dispatch(XtosPb XTOS_FAR *pb)
{
    Event event;
    Event XTOS_FAR *event_out;
    u16 status;
    SystemPrefs XTOS_FAR *prefs_out;
    const SystemPrefs XTOS_FAR *prefs_in;
    const SystemPrefs *prefs_current;
    SystemPrefs prefs_copy;

    if (pb == 0) {
        return XTOS_RESULT_BAD_PARAMETER;
    }

    status = XTOS_RESULT_OK;

    switch (pb->opcode) {
    case XTOS_OP_PING:
        if (pb->int_out != 0) {
            pb->int_out[0] = XTOS_RUNTIME_MAGIC;
            pb->int_out[1] = XTOS_ABI_VERSION;
        }
        XTOS_LOG_PREFIX("[INT60]", "ping");
        break;

    case XTOS_OP_LOG:
        if (pb->addr_in == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        XTOS_LOG_PREFIX("[APP]",
                        copy_far_string((const char XTOS_FAR *)pb->addr_in));
        break;

    case XTOS_OP_SCREENSHOT_CGA:
        if (!RuntimeScreenshotCga(
                copy_far_string((const char XTOS_FAR *)pb->addr_in))) {
            status = XTOS_RESULT_IO_ERROR;
        }
        break;

    case XTOS_OP_DISPLAY_SET_MODE:
        if (pb->int_in == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        status = XtosInt60CallResident(pb);
        if (pb->result == XTOS_RESULT_OK) {
            XTOS_LOG_PREFIX_U16("[RT]", "resident_display_mode",
                                pb->int_in[0]);
            RuntimeDisplaySetMode((enum DisplayMode)pb->int_in[0]);
        }
        break;

    case XTOS_OP_DISPLAY_CURRENT_MODE:
        if (pb->int_out == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        status = XtosInt60CallResident(pb);
        break;

    case XTOS_OP_DISPLAY_SET_PALETTE:
        if (pb->int_in == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        status = XtosInt60CallResident(pb);
        if (pb->result == XTOS_RESULT_OK) {
            XTOS_LOG_PREFIX_U16("[RT]", "resident_display_palette",
                                pb->int_in[0]);
            RuntimeDisplaySetPalette((enum DisplayPalette)pb->int_in[0]);
        }
        break;

    case XTOS_OP_DISPLAY_CURRENT_PALETTE:
        if (pb->int_out == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        status = XtosInt60CallResident(pb);
        break;

    case XTOS_OP_GET_EVENT:
        if (pb->addr_out == 0 || pb->int_out == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        pb->int_out[0] = (u16)EventGet(&event);
        if (pb->int_out[0] != 0) {
            event_out = (Event XTOS_FAR *)pb->addr_out;
            *event_out = event;
        }
        break;

    case XTOS_OP_SYSTEM_PREFS_LOAD:
        if (pb->addr_out == 0 || pb->int_out == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        status = XtosInt60CallResident(pb);
        if (pb->result == XTOS_RESULT_OK) {
            XTOS_LOG_PREFIX("[RT]", "resident_prefs_load");
        }
        break;

    case XTOS_OP_SYSTEM_PREFS_SAVE:
        if (pb->addr_in == 0 || pb->int_out == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        status = XtosInt60CallResident(pb);
        if (pb->result == XTOS_RESULT_OK) {
            XTOS_LOG_PREFIX("[RT]", "resident_prefs_save");
        }
        break;

    case XTOS_OP_SYSTEM_PREFS_CURRENT:
        if (pb->addr_out == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        status = XtosInt60CallResident(pb);
        if (status != XTOS_RESULT_OK || pb->result != XTOS_RESULT_OK) {
            prefs_out = (SystemPrefs XTOS_FAR *)pb->addr_out;
            prefs_current = RuntimeSystemPrefsCurrent();
            *prefs_out = *prefs_current;
        }
        break;

    case XTOS_OP_SYSTEM_PREFS_APPLY:
        if (pb->addr_in == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        status = XtosInt60CallResident(pb);
        if (pb->result == XTOS_RESULT_OK) {
            XTOS_LOG_PREFIX("[RT]", "resident_prefs_apply");
            prefs_in = (const SystemPrefs XTOS_FAR *)pb->addr_in;
            prefs_copy = *prefs_in;
            RuntimeSystemPrefsApply(&prefs_copy);
        }
        break;

    case XTOS_OP_CANVAS_CLEAR:
        status = XtosInt60CallResident(pb);
        break;

    case XTOS_OP_CANVAS_CLEAR_RECT:
        if (pb->int_in == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        status = XtosInt60CallResident(pb);
        break;

    case XTOS_OP_CANVAS_RECT:
        if (pb->int_in == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        status = XtosInt60CallResident(pb);
        break;

    case XTOS_OP_CANVAS_DOTTED_RECT:
        if (pb->int_in == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        status = XtosInt60CallResident(pb);
        break;

    case XTOS_OP_CANVAS_FILL_RECT:
        if (pb->int_in == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        status = XtosInt60CallResident(pb);
        break;

    case XTOS_OP_CANVAS_TEXT:
        if (pb->int_in == 0 || pb->addr_in == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        status = XtosInt60CallResident(pb);
        break;

    case XTOS_OP_CANVAS_TEXT_WIDTH:
        if (pb->int_in == 0 || pb->int_out == 0 || pb->addr_in == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        status = XtosInt60CallResident(pb);
        break;

    case XTOS_OP_CANVAS_PRESENT:
        status = XtosInt60CallResident(pb);
        break;

    case XTOS_OP_FONT_COUNT:
    case XTOS_OP_FONT_NAME:
    case XTOS_OP_FONT_WIDTH:
    case XTOS_OP_FONT_HEIGHT:
    case XTOS_OP_FONT_GLYPH_COUNT:
    case XTOS_OP_FONT_CODEPOINT_AT:
    case XTOS_OP_FONT_GLYPH_WIDTH_AT:
        status = XtosInt60CallResident(pb);
        break;

    case XTOS_OP_RESTORE_TEXT_MODE:
        DisplayShutdown();
        XTOS_LOG_PREFIX("[INT60]", "restore_text_mode");
        break;

    case XTOS_OP_RUNTIME_STATUS:
        if (pb->int_out == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        pb->int_out[0] = XTOS_RUNTIME_MAGIC;
        pb->int_out[1] = XTOS_ABI_VERSION;
        break;

    case XTOS_OP_SELFTEST:
        if (pb->int_out == 0) {
            status = XTOS_RESULT_BAD_PARAMETER;
            break;
        }
        pb->int_out[0] = XTOS_RUNTIME_MAGIC;
        pb->int_out[1] = XTOS_ABI_VERSION;
        pb->int_out[2] = XTOS_RUNTIME_STATUS_READY;
        break;

    default:
        status = XTOS_RESULT_UNKNOWN_OPCODE;
        XTOS_LOG_PREFIX_U16("[INT60]", "unknown", pb->opcode);
        break;
    }

    pb->result = status;
    return status;
}
