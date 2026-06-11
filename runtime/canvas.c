#include "draw.h"
#include "screen.h"
#include "../xtos/ui/canvas.h"

static enum ColorRole canvas_role(enum CanvasColorRole role)
{
    return (enum ColorRole)role;
}

void CanvasClear(void)
{
    DrawClear();
}

void CanvasClearRect(u16 left, u16 top, u16 right, u16 bottom)
{
    DrawClearRect(left, top, right, bottom);
}

void CanvasRect(u16 left, u16 top, u16 right, u16 bottom,
                enum CanvasColorRole role)
{
    DrawRect(left, top, right, bottom, canvas_role(role));
}

void CanvasDottedRect(u16 left, u16 top, u16 right, u16 bottom,
                      enum CanvasColorRole role)
{
    DrawDottedRect(left, top, right, bottom, canvas_role(role));
}

void CanvasFillRect(u16 left, u16 top, u16 right, u16 bottom,
                    enum CanvasColorRole role)
{
    DrawBar(left, top, right, bottom, canvas_role(role));
}

void CanvasText(u16 x, u16 y, const Font *font,
                enum CanvasColorRole role, const char *text)
{
    DrawText(x, y, font, canvas_role(role), text);
}

u16 CanvasTextWidth(const Font *font, const char *text)
{
    return FontTextWidth(font, text);
}

void CanvasPresent(void)
{
}
