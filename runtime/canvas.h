#ifndef RUNTIME_CANVAS_H
#define RUNTIME_CANVAS_H

#include "../xtos/abi.h"
#include "../xtos/ui/canvas.h"

void RuntimeCanvasClear(void);
void RuntimeCanvasClearRect(u16 left, u16 top, u16 right, u16 bottom);
void RuntimeCanvasRect(u16 left, u16 top, u16 right, u16 bottom,
                       enum CanvasColorRole role);
void RuntimeCanvasDottedRect(u16 left, u16 top, u16 right, u16 bottom,
                             enum CanvasColorRole role);
void RuntimeCanvasFillRect(u16 left, u16 top, u16 right, u16 bottom,
                           enum CanvasColorRole role);
void RuntimeCanvasText(u16 x, u16 y, const Font *font,
                       enum CanvasColorRole role, const char *text);
void RuntimeCanvasTextId(u16 x, u16 y, FontId font_id,
                         enum CanvasColorRole role,
                         const char XTOS_FAR *text);
u16 RuntimeCanvasTextWidth(const Font *font, const char *text);
u16 RuntimeCanvasTextWidthId(FontId font_id, const char XTOS_FAR *text);
void RuntimeCanvasPresent(void);

#endif
