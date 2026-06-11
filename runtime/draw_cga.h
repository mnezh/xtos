#ifndef RUNTIME_DRAW_CGA_H
#define RUNTIME_DRAW_CGA_H

#include "screen.h"
#include "types.h"

volatile u8 __far *DrawCgaMemory(void);
u16 DrawCgaLineOffset(u16 y);
void DrawCgaPlotPixel320Row(volatile u8 __far *vram,
                            u16 line_offset, u16 x, u8 color);
void DrawCgaPlotBit640Row(volatile u8 __far *vram,
                          u16 line_offset, u16 x, u8 bit);

void DrawCgaClear(void);
void DrawCgaClearRect(u16 left, u16 top, u16 right, u16 bottom);
void DrawCgaRect(u16 left, u16 top, u16 right, u16 bottom,
                 enum ColorRole role);
void DrawCgaDottedRect(u16 left, u16 top, u16 right, u16 bottom,
                       enum ColorRole role);
void DrawCgaBar(u16 left, u16 top, u16 right, u16 bottom,
                enum ColorRole role);

#endif
