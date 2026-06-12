#include "cursor.h"
#include "int60.h"
#include "mouse.h"
#include "event.h"
#include "xtos/event.h"

#ifdef XTOS_BUILD_RUNTIME
#define INPUT_DISPLAY_WIDTH 320
#define INPUT_DISPLAY_HEIGHT 200

static u8 mouse_present;
static u8 mouse_initialized;
static u16 last_x;
static u16 last_y;
static u16 last_buttons;

static u16 normalize_x(u16 driver_x)
{
    if (driver_x >= INPUT_DISPLAY_WIDTH) {
        driver_x = (u16)(driver_x >> 1);
    }

    if (driver_x >= INPUT_DISPLAY_WIDTH) {
        driver_x = (u16)(INPUT_DISPLAY_WIDTH - 1);
    }

    return driver_x;
}

static u16 normalize_y(u16 driver_y)
{
    if (driver_y >= INPUT_DISPLAY_HEIGHT) {
        driver_y = (u16)(INPUT_DISPLAY_HEIGHT - 1);
    }

    return driver_y;
}

void RuntimeMouseInit(void)
{
    u16 x;
    u16 y;
    u16 buttons;

    mouse_present = MouseReset() != 0;
    mouse_initialized = 1;
    last_x = 0;
    last_y = 0;
    last_buttons = 0;
    MouseCursorReset();

    if (!mouse_present) {
        return;
    }

    MouseRead(&x, &y, &buttons);
    last_x = normalize_x(x);
    last_y = normalize_y(y);
    last_buttons = buttons;
    MouseCursorSetPosition(last_x, last_y);
}

u8 RuntimeMousePresent(void)
{
    return mouse_present;
}

void RuntimePumpMouse(void)
{
    Event event;
    u16 x;
    u16 y;
    u16 buttons;
    u16 changed;
    u16 logical_x;
    u16 logical_y;

    if (!mouse_initialized) {
        RuntimeMouseInit();
    }

    if (!mouse_present) {
        return;
    }

    MouseRead(&x, &y, &buttons);
    logical_x = normalize_x(x);
    logical_y = normalize_y(y);

    if (logical_x != last_x || logical_y != last_y) {
        last_x = logical_x;
        last_y = logical_y;
        MouseCursorSetPosition(last_x, last_y);

        event.type = EVENT_MOUSE_MOVE;
        event.key = 0;
        event.x = last_x;
        event.y = last_y;
        event.buttons = buttons;
        EventPost(&event);
    }

    changed = (u16)(buttons ^ last_buttons);

    if (changed & XTOS_MOUSE_LEFT) {
        event.type = (buttons & XTOS_MOUSE_LEFT) ?
                     EVENT_MOUSE_DOWN : EVENT_MOUSE_UP;
        event.key = 0;
        event.x = last_x;
        event.y = last_y;
        event.buttons = buttons;
        EventPost(&event);
    }

    last_buttons = buttons;
}
#else
static void mouse_call(u16 opcode, u16 *out)
{
    XtosPb pb;
    u16 int_out[1];

    int_out[0] = 0;
    pb.opcode = opcode;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = 0;
    pb.int_out = out != 0 ? int_out : 0;
    pb.addr_in = 0;
    pb.addr_out = 0;
    XtosInt60Call(&pb);

    if (out != 0) {
        *out = int_out[0];
    }
}

void RuntimeMouseInit(void)
{
    mouse_call(XTOS_OP_MOUSE_INIT, 0);
}

u8 RuntimeMousePresent(void)
{
    u16 present;

    present = 0;
    mouse_call(XTOS_OP_MOUSE_PRESENT, &present);
    return (u8)present;
}

void RuntimePumpMouse(void)
{
    mouse_call(XTOS_OP_PUMP_EVENTS, 0);
}
#endif
