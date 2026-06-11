#ifndef XTOS_UI_CANVAS_H
#define XTOS_UI_CANVAS_H

#include "../types.h"
#include "font.h"

enum CanvasColorRole {
    CANVAS_PRIMARY_BACKGROUND,
    CANVAS_PRIMARY_FOREGROUND,
    CANVAS_EXTRA_1_ON_BACKGROUND,
    CANVAS_EXTRA_2_ON_BACKGROUND,
    CANVAS_INVERTED_BACKGROUND,
    CANVAS_INVERTED_FOREGROUND,
    CANVAS_INVERTED_EXTRA_1,
    CANVAS_INVERTED_EXTRA_2
};

void CanvasClear(void);
void CanvasClearRect(u16 left, u16 top, u16 right, u16 bottom);
void CanvasRect(u16 left, u16 top, u16 right, u16 bottom,
                enum CanvasColorRole role);
void CanvasDottedRect(u16 left, u16 top, u16 right, u16 bottom,
                      enum CanvasColorRole role);
void CanvasFillRect(u16 left, u16 top, u16 right, u16 bottom,
                    enum CanvasColorRole role);
void CanvasText(u16 x, u16 y, const Font *font,
                enum CanvasColorRole role, const char *text);
u16 CanvasTextWidth(const Font *font, const char *text);
void CanvasPresent(void);

#endif
