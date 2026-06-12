#include "screen.h"
#include "xtos/text.h"
#include "xtos/ui/font.h"
#include "xtos/ui/fonts.h"

static u16 screen_step(u8 width)
{
    return ScreenIs640() ? (u16)((width + 1) >> 1) : width;
}

static int font_glyph_index(const Font *font, u16 codepoint, u16 *index)
{
    u16 i;

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

u16 FontGlyphCount(const Font *font)
{
    if (font == 0) {
        return 0;
    }

    return font->glyph_count;
}

u16 FontCodepointAt(const Font *font, u16 index)
{
    if (font == 0 || font->map == 0 || index >= font->glyph_count) {
        return 0;
    }

    return font->map[index].codepoint;
}

u8 FontGlyphWidthAt(const Font *font, u16 index)
{
    if (font == 0 || index >= font->glyph_count) {
        return 0;
    }

    if (font->widths == 0) {
        return font->width;
    }

    return font->widths[index];
}

u16 FontTextWidth(const Font *font, const char *text)
{
    u16 width;
    u16 codepoint;
    u16 glyph_index;

    if (font == 0 || text == 0) {
        return 0;
    }

    width = 0;

    while (TextNextCodepoint(&text, &codepoint)) {
        if (font_glyph_index(font, codepoint, &glyph_index)) {
            width = (u16)(width + screen_step(FontGlyphWidthAt(font,
                                                               glyph_index)));
        }
    }

    return width;
}

const Font *FontGet(enum FontId id)
{
    if (id == FONT_SMALL) {
        return &Font4x6;
    }

    if (id == FONT_LARGE) {
        return &Font5x8;
    }

    return &Font5x7;
}
