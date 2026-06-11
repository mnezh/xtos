#ifndef XTOS_UI_INVALIDATION_H
#define XTOS_UI_INVALIDATION_H

#include "../types.h"

typedef struct Rect {
    u16 left;
    u16 top;
    u16 right;
    u16 bottom;
} Rect;

void InvalidateRect(const Rect *rect);
void InvalidateAll(void);
u8 InvalidationIsDirty(void);
void InvalidationGet(Rect *rect);
void InvalidationClear(void);
u8 RectIntersects(const Rect *a, const Rect *b);

#endif
