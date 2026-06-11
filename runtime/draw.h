#ifndef RUNTIME_DRAW_H
#define RUNTIME_DRAW_H

#include "screen.h"
#include "types.h"
#include "xtos/ui/font.h"

void DrawClear(void);
void DrawClearRect(u16 left, u16 top, u16 right, u16 bottom);
void DrawRect(u16 left, u16 top, u16 right, u16 bottom,
              enum ColorRole role);
void DrawDottedRect(u16 left, u16 top, u16 right, u16 bottom,
                    enum ColorRole role);
void DrawBar(u16 left, u16 top, u16 right, u16 bottom,
             enum ColorRole role);
void DrawText(u16 x, u16 y, const Font *font,
              enum ColorRole role, const char *text);

#endif
