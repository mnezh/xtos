#include "xtos/ui/form.h"

static void draw_button_outline(Button *button, enum CanvasColorRole role)
{
    u16 left;
    u16 top;
    u16 right;
    u16 bottom;

    left = button->left;
    top = button->top;
    right = button->right;
    bottom = button->bottom;

    CanvasFillRect((u16)(left + 2), top, (u16)(right - 2), top, role);
    CanvasFillRect((u16)(left + 1), (u16)(top + 1),
                   (u16)(left + 1), (u16)(top + 1), role);
    CanvasFillRect((u16)(right - 1), (u16)(top + 1),
                   (u16)(right - 1), (u16)(top + 1), role);
    CanvasFillRect(left, (u16)(top + 2), left, (u16)(bottom - 2), role);
    CanvasFillRect(right, (u16)(top + 2), right, (u16)(bottom - 2), role);
    CanvasFillRect((u16)(left + 1), (u16)(bottom - 1),
                   (u16)(left + 1), (u16)(bottom - 1), role);
    CanvasFillRect((u16)(right - 1), (u16)(bottom - 1),
                   (u16)(right - 1), (u16)(bottom - 1), role);
    CanvasFillRect((u16)(left + 2), bottom, (u16)(right - 2), bottom, role);
}

static void fill_button_shape(Button *button, enum CanvasColorRole role)
{
    u16 left;
    u16 top;
    u16 right;
    u16 bottom;

    left = button->left;
    top = button->top;
    right = button->right;
    bottom = button->bottom;

    CanvasFillRect((u16)(left + 2), top, (u16)(right - 2), top, role);
    CanvasFillRect((u16)(left + 1), (u16)(top + 1),
                   (u16)(right - 1), (u16)(top + 1), role);
    CanvasFillRect(left, (u16)(top + 2), right,
                   (u16)(bottom - 2), role);
    CanvasFillRect((u16)(left + 1), (u16)(bottom - 1),
                   (u16)(right - 1), (u16)(bottom - 1), role);
    CanvasFillRect((u16)(left + 2), bottom, (u16)(right - 2),
                   bottom, role);
}

void ButtonInit(Button *button, u8 id, u16 left, u16 top,
                u16 right, u16 bottom, const Font *font,
                const char *text)
{
    button->left = left;
    button->top = top;
    button->right = right;
    button->bottom = bottom;
    button->font = font;
    button->text = text;
    button->id = id;
    button->focused = 0;
    button->pressed = 0;
}

void ButtonSetText(Button *button, const char *text)
{
    if (button == 0) {
        return;
    }

    ButtonInvalidate(button);
    button->text = text;
    ButtonInvalidate(button);
}

void ButtonDraw(Button *button)
{
    u16 text_x;
    u16 text_y;
    u16 text_right;
    u16 underline_y;
    enum CanvasColorRole text_role;

    if (button == 0 || button->font == 0 || button->text == 0) {
        return;
    }

    CanvasClearRect(button->left, button->top,
                    button->right, button->bottom);

    if (button->pressed) {
        fill_button_shape(button, CANVAS_PRIMARY_FOREGROUND);
        text_x = (u16)(button->left + 4);
        text_y = (u16)(button->top + 3);
        text_role = CANVAS_PRIMARY_BACKGROUND;
    } else if (button->focused) {
        draw_button_outline(button, CANVAS_PRIMARY_FOREGROUND);
        text_x = (u16)(button->left + 4);
        text_y = (u16)(button->top + 3);
        text_role = CANVAS_PRIMARY_FOREGROUND;
    } else {
        draw_button_outline(button, CANVAS_PRIMARY_FOREGROUND);
        text_x = (u16)(button->left + 4);
        text_y = (u16)(button->top + 3);
        text_role = CANVAS_PRIMARY_FOREGROUND;
    }

    CanvasText(text_x, text_y, button->font, text_role, button->text);

    if (button->focused && !button->pressed) {
        text_right = (u16)(text_x + CanvasTextWidth(button->font,
                                                   button->text) - 1);

        if (text_right > (u16)(button->right - 3)) {
            text_right = (u16)(button->right - 3);
        }

        underline_y = (u16)(text_y + button->font->height);

        if (underline_y < button->bottom) {
            CanvasFillRect(text_x, underline_y, text_right, underline_y,
                           CANVAS_PRIMARY_FOREGROUND);
        }
    }
}

void ButtonDrawInRect(Button *button, const Rect *rect)
{
    Rect bounds;

    if (button == 0 || rect == 0) {
        return;
    }

    bounds.left = button->left;
    bounds.top = button->top;
    bounds.right = button->right;
    bounds.bottom = button->bottom;

    if (RectIntersects(&bounds, rect)) {
        ButtonDraw(button);
    }
}

void ButtonSetFocused(Button *button, u8 focused)
{
    if (button == 0 || button->focused == focused) {
        return;
    }

    button->focused = focused;
    ButtonInvalidate(button);
}

void ButtonSetPressed(Button *button, u8 pressed)
{
    if (button == 0 || button->pressed == pressed) {
        return;
    }

    button->pressed = pressed;
    ButtonInvalidate(button);
}

void ButtonInvalidate(Button *button)
{
    Rect rect;

    if (button == 0) {
        return;
    }

    rect.left = button->left;
    rect.top = button->top;
    rect.right = button->right;
    rect.bottom = button->bottom;
    InvalidateRect(&rect);
}
