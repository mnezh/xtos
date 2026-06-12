#include "draw.h"
#include "screen.h"
#include "canvas.h"

static enum ColorRole canvas_role(enum CanvasColorRole role)
{
    return (enum ColorRole)role;
}

void RuntimeCanvasClear(void)
{
    DrawClear();
}

void RuntimeCanvasClearRect(u16 left, u16 top, u16 right, u16 bottom)
{
    DrawClearRect(left, top, right, bottom);
}

void RuntimeCanvasRect(u16 left, u16 top, u16 right, u16 bottom,
                       enum CanvasColorRole role)
{
    DrawRect(left, top, right, bottom, canvas_role(role));
}

void RuntimeCanvasDottedRect(u16 left, u16 top, u16 right, u16 bottom,
                             enum CanvasColorRole role)
{
    DrawDottedRect(left, top, right, bottom, canvas_role(role));
}

void RuntimeCanvasFillRect(u16 left, u16 top, u16 right, u16 bottom,
                           enum CanvasColorRole role)
{
    DrawBar(left, top, right, bottom, canvas_role(role));
}

void RuntimeCanvasText(u16 x, u16 y, const Font *font,
                       enum CanvasColorRole role, const char *text)
{
    DrawText(x, y, font, canvas_role(role), text);
}

u16 RuntimeCanvasTextWidth(const Font *font, const char *text)
{
    return FontTextWidth(font, text);
}

void RuntimeCanvasPresent(void)
{
}
