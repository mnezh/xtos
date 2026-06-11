#ifndef RUNTIME_DRAW_TEXT_H
#define RUNTIME_DRAW_TEXT_H

#include "screen.h"
#include "types.h"
#include "xtos/ui/font.h"

void DrawTextInternal(u16 x, u16 y, const Font *font,
                      enum ColorRole role, const char *text);

#endif
