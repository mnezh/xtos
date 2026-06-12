#ifndef XTOS_UI_FONT_H
#define XTOS_UI_FONT_H

#include "../types.h"

#define FONT_MONOSPACE 1
#define FONT_PROPORTIONAL 2

typedef u16 FontId;

enum FontIdValue {
    FONT_SYSTEM,
    FONT_SMALL,
    FONT_LARGE
};

typedef struct GlyphMap {
    u16 codepoint;
    u16 glyph_index;
} GlyphMap;

typedef struct Font {
    const char *name;
    u8 width;
    u8 height;
    u8 ascent;
    u8 descent;
    u16 glyph_count;
    u16 replacement_glyph;
    u8 flags;
    const u8 *glyphs;
    const GlyphMap *map;
    const u8 *widths;
} Font;

u16 FontGlyphCount(const Font *font);
u16 FontCodepointAt(const Font *font, u16 index);
u8 FontGlyphWidthAt(const Font *font, u16 index);
u8 FontWidth(const Font *font);
u8 FontHeight(const Font *font);
u8 FontCount(void);
const char *FontName(const Font *font);
FontId FontIdOf(const Font *font);
u16 FontTextWidth(const Font *font, const char *text);
const Font *FontGet(FontId id);

#ifdef XTOS_DEBUG
void FontDebugSnapshotSelect(const Font *font);
void FontDebugCheckSelect(const Font *font);
#endif

#endif
