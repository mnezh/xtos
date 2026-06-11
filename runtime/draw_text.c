#include "draw_cga.h"
#include "draw_text.h"
#include "xtos/text.h"

static u8 font_bytes_per_row(const Font *font)
{
    return (u8)((font->width + 7) >> 3);
}

static u16 glyph_step(u8 width)
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

static u8 font_glyph_width(const Font *font, u16 glyph_index)
{
    if (font->widths == 0) {
        return font->width;
    }

    return font->widths[glyph_index];
}

static void draw_glyph_320(volatile u8 __far *vram,
                           u16 x, u16 y, const Font *font,
                           u8 bytes_per_row, u8 color, u16 ch)
{
    const u8 *glyph;
    u16 glyph_index;
    u8 row;
    u8 col;
    u8 bits;
    u16 line_offset;

    if (!font_glyph_index(font, ch, &glyph_index)) {
        return;
    }

    glyph = font->glyphs +
            (glyph_index * font->height * bytes_per_row);

    /* Font glyphs are row-major bitmaps. Current built-in fonts are <= 8
       pixels wide, so this hot path loads one glyph byte per row instead of
       re-reading it for every column. A future renderer can merge this byte
       directly into VRAM when x is byte-aligned. */
    if (bytes_per_row == 1) {
        for (row = 0; row < font->height; ++row) {
            bits = glyph[row];
            line_offset = DrawCgaLineOffset((u16)(y + row));

            for (col = 0; col < font->width; ++col) {
                if (bits & (0x80 >> (col & 7))) {
                    DrawCgaPlotPixel320Row(vram, line_offset,
                                           (u16)(x + col), color);
                }
            }
        }

        return;
    }

    for (row = 0; row < font->height; ++row) {
        line_offset = DrawCgaLineOffset((u16)(y + row));

        for (col = 0; col < font->width; ++col) {
            bits = glyph[(u16)row * bytes_per_row + (col >> 3)];

            if (bits & (0x80 >> (col & 7))) {
                DrawCgaPlotPixel320Row(vram, line_offset,
                                       (u16)(x + col), color);
            }
        }
    }
}

static void draw_glyph_640(volatile u8 __far *vram,
                           u16 x, u16 y, const Font *font,
                           u8 bytes_per_row, u8 bit, u16 ch)
{
    const u8 *glyph;
    u16 glyph_index;
    u8 row;
    u8 col;
    u8 bits;
    u16 line_offset;
    u16 physical_x;

    if (!font_glyph_index(font, ch, &glyph_index)) {
        return;
    }

    glyph = font->glyphs +
            (glyph_index * font->height * bytes_per_row);
    physical_x = (u16)(x << 1);

    /* In 640 mode text columns are physical CGA bits, not doubled logical
       pixels. This keeps small bitmap fonts sharp while avoiding the former
       ScreenIs640()/ScreenRoleBit() branch on every lit glyph pixel. */
    if (bytes_per_row == 1) {
        for (row = 0; row < font->height; ++row) {
            bits = glyph[row];
            line_offset = DrawCgaLineOffset((u16)(y + row));

            for (col = 0; col < font->width; ++col) {
                if (bits & (0x80 >> (col & 7))) {
                    DrawCgaPlotBit640Row(vram, line_offset,
                                         (u16)(physical_x + col), bit);
                }
            }
        }

        return;
    }

    for (row = 0; row < font->height; ++row) {
        line_offset = DrawCgaLineOffset((u16)(y + row));

        for (col = 0; col < font->width; ++col) {
            bits = glyph[(u16)row * bytes_per_row + (col >> 3)];

            if (bits & (0x80 >> (col & 7))) {
                DrawCgaPlotBit640Row(vram, line_offset,
                                     (u16)(physical_x + col), bit);
            }
        }
    }
}

void DrawTextInternal(u16 x, u16 y, const Font *font,
                      enum ColorRole role, const char *text)
{
    volatile u8 __far *vram;
    u16 cursor_x;
    u8 bytes_per_row;
    u8 value;
    u16 ch;
    u16 glyph_index;

    if (font == 0 || text == 0) {
        return;
    }

    vram = DrawCgaMemory();
    cursor_x = x;
    bytes_per_row = font_bytes_per_row(font);

    if (ScreenIs640()) {
        value = ScreenRoleBit(role);

        while (TextNextCodepoint(&text, &ch)) {
            if (font_glyph_index(font, ch, &glyph_index)) {
                draw_glyph_640(vram, cursor_x, y, font, bytes_per_row,
                               value, ch);
                cursor_x = (u16)(cursor_x +
                                  glyph_step(font_glyph_width(font,
                                                              glyph_index)));
            }
        }

        return;
    }

    value = ScreenRolePixel(role);

    while (TextNextCodepoint(&text, &ch)) {
        if (font_glyph_index(font, ch, &glyph_index)) {
            draw_glyph_320(vram, cursor_x, y, font, bytes_per_row,
                           value, ch);
            cursor_x = (u16)(cursor_x + glyph_step(font_glyph_width(font,
                                                                    glyph_index)));
        }
    }
}
