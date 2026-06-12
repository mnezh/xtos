#include "xtos/event.h"
#include "xtos/ui/form.h"
#include "xtos/ui/invalidation.h"

#define LIST_PADDING_Y 2

static void list_bounds(List *list, Rect *rect)
{
    rect->left = list->x;
    rect->top = list->y;
    rect->right = (u16)(list->x + list->width - 1);
    rect->bottom = (u16)(list->y + LIST_PADDING_Y +
                         list->count * list->row_height +
                         LIST_PADDING_Y - 1);
}

static void ListDrawBoundary(List *list)
{
    Rect bounds;

    if (list == 0) {
        return;
    }

    list_bounds(list, &bounds);

    if (list->focused) {
        CanvasRect(bounds.left, bounds.top, bounds.right, bounds.bottom,
                   CANVAS_PRIMARY_FOREGROUND);
    } else {
        CanvasDottedRect(bounds.left, bounds.top, bounds.right, bounds.bottom,
                         CANVAS_PRIMARY_FOREGROUND);
    }
}

void ListInit(List *list, u16 x, u16 y, u16 width, const Font *font,
              const char * const *items, u8 count)
{
    list->x = x;
    list->y = y;
    list->width = width;
    list->font = font;
    list->items = items;
    list->count = count;
    list->selected = 0;
    list->row_height = (u8)(FontHeight(font) + 2);
    list->focused = 0;
}

void ListDrawRow(List *list, u8 row)
{
    u16 y;

    if (list == 0 || list->font == 0 || list->items == 0 ||
        row >= list->count) {
        return;
    }

    y = (u16)(list->y + LIST_PADDING_Y + row * list->row_height);
    CanvasClearRect((u16)(list->x + 1), y,
                    (u16)(list->x + list->width - 2),
                    (u16)(y + list->row_height - 1));

    if (row == list->selected) {
        CanvasFillRect((u16)(list->x + 1), y,
                       (u16)(list->x + list->width - 2),
                       (u16)(y + list->row_height - 1),
                       CANVAS_INVERTED_BACKGROUND);
        CanvasText((u16)(list->x + 3), (u16)(y + 1), list->font,
                   CANVAS_INVERTED_FOREGROUND, list->items[row]);
    } else {
        CanvasText((u16)(list->x + 3), (u16)(y + 1), list->font,
                   CANVAS_PRIMARY_FOREGROUND, list->items[row]);
    }
}

void ListDraw(List *list)
{
    u8 i;

    if (list == 0) {
        return;
    }

    for (i = 0; i < list->count; ++i) {
        ListDrawRow(list, i);
    }

    ListDrawBoundary(list);
}

void ListDrawInRect(List *list, const Rect *rect)
{
    Rect row_rect;
    Rect bounds;
    u8 i;
    u16 y;

    if (list == 0 || rect == 0) {
        return;
    }

    for (i = 0; i < list->count; ++i) {
        y = (u16)(list->y + LIST_PADDING_Y + i * list->row_height);
        row_rect.left = list->x;
        row_rect.top = y;
        row_rect.right = (u16)(list->x + list->width - 1);
        row_rect.bottom = (u16)(y + list->row_height - 1);

        if (RectIntersects(&row_rect, rect)) {
            ListDrawRow(list, i);
        }
    }

    list_bounds(list, &bounds);

    if (RectIntersects(&bounds, rect)) {
        ListDrawBoundary(list);
    }
}

void ListHandleEvent(List *list, Event *event)
{
    if (list == 0 || event == 0 || event->type != EVENT_KEYDOWN) {
        return;
    }

    if (event->key == XTOS_KEY_UP) {
        if (list->selected > 0) {
            ListInvalidateRow(list, list->selected);
            --list->selected;
            ListInvalidateRow(list, list->selected);
        }
    } else if (event->key == XTOS_KEY_DOWN) {
        if ((u8)(list->selected + 1) < list->count) {
            ListInvalidateRow(list, list->selected);
            ++list->selected;
            ListInvalidateRow(list, list->selected);
        }
    }
}

u8 ListSelected(List *list)
{
    return list->selected;
}

void ListSetSelected(List *list, u8 selected)
{
    if (selected < list->count && selected != list->selected) {
        ListInvalidateRow(list, list->selected);
        list->selected = selected;
        ListInvalidateRow(list, list->selected);
    }
}

void ListSetFocused(List *list, u8 focused)
{
    if (list == 0 || list->focused == focused) {
        return;
    }

    list->focused = focused;
    ListInvalidate(list);
}

void ListInvalidateRow(List *list, u8 row)
{
    Rect rect;
    u16 y;

    if (list == 0 || row >= list->count) {
        return;
    }

    y = (u16)(list->y + LIST_PADDING_Y + row * list->row_height);
    rect.left = list->x;
    rect.top = y;
    rect.right = (u16)(list->x + list->width - 1);
    rect.bottom = (u16)(y + list->row_height - 1);
    InvalidateRect(&rect);
}

void ListInvalidate(List *list)
{
    u8 i;

    if (list == 0) {
        return;
    }

    for (i = 0; i < list->count; ++i) {
        ListInvalidateRow(list, i);
    }
}
