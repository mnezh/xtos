#ifndef XTOS_EVENT_H
#define XTOS_EVENT_H

#include "types.h"

#define XTOS_KEY_ENTER 0x000d
#define XTOS_KEY_ESCAPE 0x001b
#define XTOS_KEY_TAB 0x0009
#define XTOS_KEY_ALT_Q 0x1000
#define XTOS_KEY_ALT_S 0x1f00
#define XTOS_KEY_UP 0x4800
#define XTOS_KEY_DOWN 0x5000

#define XTOS_MOUSE_LEFT 0x0001
#define XTOS_MOUSE_RIGHT 0x0002

enum EventType {
    EVENT_NONE,
    EVENT_KEYDOWN,
    EVENT_KEYUP,
    EVENT_MOUSE_MOVE,
    EVENT_MOUSE_DOWN,
    EVENT_MOUSE_UP,
    EVENT_TICK,
    EVENT_QUIT
};

#define EVENT_MOUSEMOVE EVENT_MOUSE_MOVE
#define EVENT_MOUSEDOWN EVENT_MOUSE_DOWN
#define EVENT_MOUSEUP EVENT_MOUSE_UP

typedef struct Event {
    enum EventType type;
    u16 key;
    u16 x;
    u16 y;
    u16 buttons;
} Event;

int GetEvent(Event *event);

#endif
