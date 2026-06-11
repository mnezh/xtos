#include "draw.h"
#include "draw_cga.h"
#include "draw_text.h"

void DrawClear(void)
{
    DrawCgaClear();
}

void DrawClearRect(u16 left, u16 top, u16 right, u16 bottom)
{
    DrawCgaClearRect(left, top, right, bottom);
}

void DrawRect(u16 left, u16 top, u16 right, u16 bottom,
              enum ColorRole role)
{
    DrawCgaRect(left, top, right, bottom, role);
}

void DrawDottedRect(u16 left, u16 top, u16 right, u16 bottom,
                    enum ColorRole role)
{
    DrawCgaDottedRect(left, top, right, bottom, role);
}

void DrawBar(u16 left, u16 top, u16 right, u16 bottom,
             enum ColorRole role)
{
    DrawCgaBar(left, top, right, bottom, role);
}

void DrawText(u16 x, u16 y, const Font *font,
              enum ColorRole role, const char *text)
{
    DrawTextInternal(x, y, font, role, text);
}
