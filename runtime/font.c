#include "screen.h"
#include "log.h"
#include "int60.h"
#include "xtos/text.h"
#include "xtos/ui/font.h"
#include "xtos/ui/fonts.h"

#define FONT_ASCII_FIRST 0x20
#define FONT_ASCII_LAST 0x7e
#define FONT_ASCII_GLYPH_BASE 1

#ifdef XTOS_BUILD_RUNTIME
static int text_continuation(unsigned char ch)
{
    return (ch & 0xc0) == 0x80;
}

static int font_next_codepoint(const char **cursor, u16 *codepoint)
{
    const unsigned char *p;
    u16 value;

    if (cursor == 0 || *cursor == 0 || codepoint == 0) {
        return 0;
    }

    p = (const unsigned char *)*cursor;
    if (*p == 0) {
        return 0;
    }

    if (*p < 0x80) {
        *codepoint = *p;
        *cursor = (const char *)(p + 1);
        return 1;
    }

    if ((*p & 0xe0) == 0xc0 && text_continuation(p[1])) {
        value = (u16)(((*p & 0x1f) << 6) | (p[1] & 0x3f));
        if (value >= 0x80) {
            *codepoint = value;
            *cursor = (const char *)(p + 2);
            return 1;
        }
    }

    if ((*p & 0xf0) == 0xe0 && text_continuation(p[1]) &&
        text_continuation(p[2])) {
        value = (u16)(((*p & 0x0f) << 12) |
                      ((p[1] & 0x3f) << 6) |
                      (p[2] & 0x3f));
        if (value >= 0x800) {
            *codepoint = value;
            *cursor = (const char *)(p + 3);
            return 1;
        }
    }

    *codepoint = XTOS_TEXT_REPLACEMENT;
    *cursor = (const char *)(p + 1);
    return 1;
}

static u16 screen_step(u8 width)
{
    return ScreenIs640() ? (u16)((width + 1) >> 1) : width;
}

static int font_ascii_glyph_index(const Font *font, u16 codepoint, u16 *index)
{
    u16 glyph_index;

    if (font == 0 || index == 0 ||
        codepoint < FONT_ASCII_FIRST || codepoint > FONT_ASCII_LAST) {
        return 0;
    }

    glyph_index = (u16)(codepoint - FONT_ASCII_FIRST +
                        FONT_ASCII_GLYPH_BASE);
    if (glyph_index >= font->glyph_count) {
        return 0;
    }

    *index = glyph_index;
    return 1;
}

static int font_glyph_index(const Font *font, u16 codepoint, u16 *index)
{
    u16 i;

    if (font_ascii_glyph_index(font, codepoint, index)) {
        return 1;
    }

    if (font == 0 || font->map == 0 || index == 0) {
        return 0;
    }

    for (i = 0; i < font->glyph_count; ++i) {
        if (font->map[i].codepoint == codepoint) {
            *index = font->map[i].glyph_index;
            return 1;
        }
    }

    if (font->replacement_glyph != 0xffff &&
        font->replacement_glyph < font->glyph_count) {
        *index = font->replacement_glyph;
        return 1;
    }

    return 0;
}

static const Font *runtime_font_by_id(FontId id)
{
    if (id == FONT_SMALL) {
        return &Font4x6;
    }

    if (id == FONT_LARGE) {
        return &Font5x8;
    }

    return &Font5x7;
}

const Font *RuntimeFontById(FontId id)
{
    return runtime_font_by_id(id);
}

u8 RuntimeFontCount(void)
{
    return 3;
}

const char *RuntimeFontName(FontId id)
{
    return runtime_font_by_id(id)->name;
}

u8 RuntimeFontWidth(FontId id)
{
    return runtime_font_by_id(id)->width;
}

u8 RuntimeFontHeight(FontId id)
{
    return runtime_font_by_id(id)->height;
}

u16 RuntimeFontGlyphCount(FontId id)
{
    return runtime_font_by_id(id)->glyph_count;
}

u16 RuntimeFontCodepointAt(FontId id, u16 index)
{
    return FontCodepointAt(runtime_font_by_id(id), index);
}

u8 RuntimeFontGlyphWidthAt(FontId id, u16 index)
{
    return FontGlyphWidthAt(runtime_font_by_id(id), index);
}
#endif

#ifndef XTOS_BUILD_RUNTIME
static const Font app_font_handles[3] = {
    { "FONT_SYSTEM", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { "FONT_SMALL", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { "FONT_LARGE", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

static char app_font_names[3][24];

static FontId app_font_id(const Font *font)
{
    if (font == &app_font_handles[FONT_SMALL]) {
        return FONT_SMALL;
    }

    if (font == &app_font_handles[FONT_LARGE]) {
        return FONT_LARGE;
    }

    return FONT_SYSTEM;
}

static u16 font_call(XtosPb XTOS_FAR *pb)
{
    return XtosInt60Call(pb);
}

static u16 font_query_word(FontId id, u16 opcode, u16 index)
{
    XtosPb pb;
    u16 int_in[2];
    u16 int_out[1];

    int_in[0] = id;
    int_in[1] = index;
    int_out[0] = 0;
    pb.opcode = opcode;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = int_out;
    pb.addr_in = 0;
    pb.addr_out = 0;
    font_call(&pb);
    return int_out[0];
}
#endif

u16 FontGlyphCount(const Font *font)
{
#ifndef XTOS_BUILD_RUNTIME
    return font_query_word(app_font_id(font), XTOS_OP_FONT_GLYPH_COUNT, 0);
#else
    if (font == 0) {
        return 0;
    }

    return font->glyph_count;
#endif
}

u16 FontCodepointAt(const Font *font, u16 index)
{
#ifndef XTOS_BUILD_RUNTIME
    return font_query_word(app_font_id(font), XTOS_OP_FONT_CODEPOINT_AT,
                           index);
#else
    if (font != 0 &&
        index >= FONT_ASCII_GLYPH_BASE &&
        index <= (u16)(FONT_ASCII_LAST - FONT_ASCII_FIRST +
                       FONT_ASCII_GLYPH_BASE) &&
        index < font->glyph_count) {
        return (u16)(index - FONT_ASCII_GLYPH_BASE + FONT_ASCII_FIRST);
    }

    if (font == 0 || font->map == 0 || index >= font->glyph_count) {
        return 0;
    }

    return font->map[index].codepoint;
#endif
}

u8 FontGlyphWidthAt(const Font *font, u16 index)
{
#ifndef XTOS_BUILD_RUNTIME
    return (u8)font_query_word(app_font_id(font), XTOS_OP_FONT_GLYPH_WIDTH_AT,
                               index);
#else
    if (font == 0 || index >= font->glyph_count) {
        return 0;
    }

    if (font->widths == 0) {
        return font->width;
    }

    return font->widths[index];
#endif
}

u8 FontWidth(const Font *font)
{
#ifndef XTOS_BUILD_RUNTIME
    return (u8)font_query_word(app_font_id(font), XTOS_OP_FONT_WIDTH, 0);
#else
    return font != 0 ? font->width : 0;
#endif
}

u8 FontHeight(const Font *font)
{
#ifndef XTOS_BUILD_RUNTIME
    return (u8)font_query_word(app_font_id(font), XTOS_OP_FONT_HEIGHT, 0);
#else
    return font != 0 ? font->height : 0;
#endif
}

u8 FontCount(void)
{
#ifndef XTOS_BUILD_RUNTIME
    return (u8)font_query_word(FONT_SYSTEM, XTOS_OP_FONT_COUNT, 0);
#else
    return RuntimeFontCount();
#endif
}

const char *FontName(const Font *font)
{
#ifndef XTOS_BUILD_RUNTIME
    XtosPb pb;
    u16 int_in[2];
    FontId id;

    id = app_font_id(font);
    int_in[0] = id;
    int_in[1] = sizeof(app_font_names[0]);
    app_font_names[id][0] = 0;
    pb.opcode = XTOS_OP_FONT_NAME;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = 0;
    pb.addr_in = 0;
    pb.addr_out = (void XTOS_FAR *)app_font_names[id];
    font_call(&pb);
    return app_font_names[id];
#else
    return font != 0 ? font->name : "";
#endif
}

FontId FontIdOf(const Font *font)
{
#ifndef XTOS_BUILD_RUNTIME
    return app_font_id(font);
#else
    if (font == &Font4x6) {
        return FONT_SMALL;
    }

    if (font == &Font5x8) {
        return FONT_LARGE;
    }

    return FONT_SYSTEM;
#endif
}

u16 FontTextWidth(const Font *font, const char *text)
{
#ifndef XTOS_BUILD_RUNTIME
    XtosPb pb;
    u16 int_in[1];
    u16 int_out[1];

    int_in[0] = FontIdOf(font);
    int_out[0] = 0;
    pb.opcode = XTOS_OP_CANVAS_TEXT_WIDTH;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = int_out;
    pb.addr_in = (void XTOS_FAR *)text;
    pb.addr_out = 0;
    font_call(&pb);
    return int_out[0];
#else
    u16 width;
    u16 codepoint;
    u16 glyph_index;

    if (font == 0 || text == 0) {
        return 0;
    }

    width = 0;

    while (font_next_codepoint(&text, &codepoint)) {
        if (font_glyph_index(font, codepoint, &glyph_index)) {
            width = (u16)(width + screen_step(FontGlyphWidthAt(font,
                                                               glyph_index)));
        }
    }

    return width;
#endif
}

const Font *FontGet(FontId id)
{
#ifndef XTOS_BUILD_RUNTIME
    if (id > FONT_LARGE) {
        id = FONT_SYSTEM;
    }

    return &app_font_handles[id];
#else
    if (id == FONT_SMALL) {
        return &Font4x6;
    }

    if (id == FONT_LARGE) {
        return &Font5x8;
    }

    return &Font5x7;
#endif
}
