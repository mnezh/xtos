#include "xtos/display.h"
#include "xtos/ui/invalidation.h"

static Rect dirty_rect;
static u8 dirty;

static void rect_union(Rect *dst, const Rect *src)
{
    if (src->left < dst->left) {
        dst->left = src->left;
    }

    if (src->top < dst->top) {
        dst->top = src->top;
    }

    if (src->right > dst->right) {
        dst->right = src->right;
    }

    if (src->bottom > dst->bottom) {
        dst->bottom = src->bottom;
    }
}

void InvalidateRect(const Rect *rect)
{
    if (rect == 0) {
        return;
    }

    if (!dirty) {
        dirty_rect = *rect;
        dirty = 1;
        return;
    }

    rect_union(&dirty_rect, rect);
}

void InvalidateAll(void)
{
    dirty_rect.left = 0;
    dirty_rect.top = 0;
    dirty_rect.right = (u16)(DisplayWidth() - 1);
    dirty_rect.bottom = (u16)(DisplayHeight() - 1);
    dirty = 1;
}

u8 InvalidationIsDirty(void)
{
    return dirty;
}

void InvalidationGet(Rect *rect)
{
    if (rect != 0) {
        *rect = dirty_rect;
    }
}

void InvalidationClear(void)
{
    dirty = 0;
}

u8 RectIntersects(const Rect *a, const Rect *b)
{
    if (a == 0 || b == 0) {
        return 0;
    }

    if (a->right < b->left || b->right < a->left) {
        return 0;
    }

    if (a->bottom < b->top || b->bottom < a->top) {
        return 0;
    }

    return 1;
}
