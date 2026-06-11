#include "xtos/ui/form.h"
#include "xtos/ui/invalidation.h"

static u16 label_width(Label *label)
{
    u16 width;
    const char *text;

    if (label == 0 || label->font == 0) {
        return 1;
    }

    width = 1;
    text = label->text;

    while (text != 0 && *text != 0) {
        width = (u16)(width + label->font->width);
        ++text;
    }

    return width;
}

void LabelInit(Label *label, u16 x, u16 y, const Font *font,
               const char *text, enum CanvasColorRole color)
{
    label->x = x;
    label->y = y;
    label->font = font;
    label->text = text;
    label->color = color;
    label->dirty_width = label_width(label);
}

void LabelSetText(Label *label, const char *text)
{
    u16 old_width;
    u16 new_width;

    old_width = label_width(label);
    LabelInvalidate(label);
    label->text = text;
    new_width = label_width(label);
    label->dirty_width = old_width > new_width ? old_width : new_width;
    LabelInvalidate(label);
}

void LabelDraw(Label *label)
{
    if (label == 0 || label->text == 0 || label->font == 0) {
        return;
    }

    CanvasText(label->x, label->y, label->font, label->color, label->text);
}

void LabelDrawInRect(Label *label, const Rect *rect)
{
    Rect bounds;

    if (label == 0 || label->text == 0 || label->font == 0) {
        return;
    }

    bounds.left = label->x;
    bounds.top = label->y;
    bounds.right = (u16)(label->x + label->dirty_width);
    bounds.bottom = (u16)(label->y + label->font->height);

    if (!RectIntersects(&bounds, rect)) {
        return;
    }

    CanvasClearRect(bounds.left, bounds.top, bounds.right, bounds.bottom);
    LabelDraw(label);
    label->dirty_width = label_width(label);
}

void LabelInvalidate(Label *label)
{
    Rect rect;
    u16 width;

    if (label == 0 || label->font == 0) {
        return;
    }

    width = label_width(label);
    if (label->dirty_width < width) {
        label->dirty_width = width;
    }

    rect.left = label->x;
    rect.top = label->y;
    rect.right = (u16)(label->x + label->dirty_width);
    rect.bottom = (u16)(label->y + label->font->height);
    InvalidateRect(&rect);
}
