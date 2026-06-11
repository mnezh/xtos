#ifndef RUNTIME_EVENT_H
#define RUNTIME_EVENT_H

#include "../xtos/event.h"

#define EVENT_QUEUE_SIZE 8

void EventPost(const Event *event);
int EventGet(Event *event);
void RuntimePumpEvents(void);

#endif
