#include "keyboard.h"
#include "mouse.h"
#include "event.h"
#include "../xtos/event.h"

static Event event_queue[EVENT_QUEUE_SIZE];
static u8 event_head;
static u8 event_tail;

static u8 event_queue_empty(void)
{
    return event_head == event_tail;
}

static u8 event_queue_next(u8 index)
{
    return (u8)((index + 1) % EVENT_QUEUE_SIZE);
}

void EventPost(const Event *event)
{
    u8 next_tail;

    next_tail = event_queue_next(event_tail);

    if (next_tail == event_head) {
        return;
    }

    event_queue[event_tail] = *event;
    event_tail = next_tail;
}

int EventGet(Event *event)
{
    if (event_queue_empty()) {
        return 0;
    }

    *event = event_queue[event_head];
    event_head = event_queue_next(event_head);
    return 1;
}

static void pump_keyboard(void)
{
    Event event;

    if (!KeyboardHasKey()) {
        return;
    }

    event.type = EVENT_KEYDOWN;
    event.key = KeyboardRead();
    event.x = 0;
    event.y = 0;
    event.buttons = 0;
    EventPost(&event);
}

void RuntimePumpEvents(void)
{
    pump_keyboard();
    RuntimePumpMouse();
}
