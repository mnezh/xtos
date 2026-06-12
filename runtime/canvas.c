#include "draw.h"
#include "int60.h"
#include "screen.h"
#include "canvas.h"

#define CANVAS_TEXT_BUFFER_SIZE 256

char *XtosCopyFarString(char *dest, const char XTOS_FAR *source, u16 max);

#ifdef XTOS_BUILD_RUNTIME
const Font *RuntimeFontById(FontId id);
#endif

static char canvas_text_buffer[CANVAS_TEXT_BUFFER_SIZE];

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

void RuntimeCanvasTextId(u16 x, u16 y, FontId font_id,
                         enum CanvasColorRole role,
                         const char XTOS_FAR *text)
{
    const char *near_text;

    near_text = XtosCopyFarString(canvas_text_buffer, text,
                                  CANVAS_TEXT_BUFFER_SIZE);
#ifdef XTOS_BUILD_RUNTIME
    DrawText(x, y, RuntimeFontById(font_id), canvas_role(role), near_text);
#else
    DrawText(x, y, FontGet(font_id), canvas_role(role), near_text);
#endif
}

u16 RuntimeCanvasTextWidth(const Font *font, const char *text)
{
    return FontTextWidth(font, text);
}

u16 RuntimeCanvasTextWidthId(FontId font_id, const char XTOS_FAR *text)
{
    const char *near_text;

    near_text = XtosCopyFarString(canvas_text_buffer, text,
                                  CANVAS_TEXT_BUFFER_SIZE);
#ifdef XTOS_BUILD_RUNTIME
    return FontTextWidth(RuntimeFontById(font_id), near_text);
#else
    return FontTextWidth(FontGet(font_id), near_text);
#endif
}

void RuntimeCanvasPresent(void)
{
}
