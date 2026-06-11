#include "cga.h"
#include "screen.h"

struct ScreenPaletteDef {
    u8 bios_background;
    u8 bios_foreground_320;
    u8 bios_foreground_640;
    u8 pixel[COLOR_ROLE_COUNT];
    u8 bit[COLOR_ROLE_COUNT];
};

static const struct ScreenPaletteDef palettes[SCREEN_PALETTE_COUNT] = {
    {
        0, 1, 15, /* Black on white. */
        { 3, 0, 1, 2, 0, 3, 1, 2 },
        { 1, 0, 0, 0, 0, 1, 1, 1 }
    },
    {
        0, 1, 15, /* White on black. */
        { 0, 3, 1, 2, 3, 0, 1, 2 },
        { 0, 1, 1, 1, 1, 0, 0, 0 }
    },
    {
        1, 1, 15, /* Blue on white. */
        { 3, 0, 1, 2, 0, 3, 1, 2 },
        { 1, 0, 0, 0, 0, 1, 1, 1 }
    },
    {
        1, 1, 15, /* White on blue. */
        { 0, 3, 1, 2, 3, 0, 1, 2 },
        { 0, 1, 1, 1, 1, 0, 0, 0 }
    },
    {
        1, 0, 6,  /* Brown on blue. */
        { 0, 3, 1, 2, 3, 0, 1, 2 },
        { 0, 1, 1, 1, 1, 0, 0, 0 }
    }
};

static enum ScreenModeId current_mode = TEXT_80x25;
static enum ScreenPaletteId current_palette = BLACK_ON_WHITE;
static const struct ScreenPaletteDef *active_palette = &palettes[0];

static void apply_cga_colors(void)
{
    CgaSetColors(active_palette->bios_background,
                 current_mode == CGA_640x200x2 ?
                 active_palette->bios_foreground_640 :
                 active_palette->bios_foreground_320);
}

void ScreenMode(enum ScreenModeId mode)
{
    current_mode = mode;
    CgaSetVideoMode(mode);

    if (mode != TEXT_80x25) {
        apply_cga_colors();
    }
}

enum ScreenModeId ScreenCurrentMode(void)
{
    return current_mode;
}

void ScreenPalette(enum ScreenPaletteId palette)
{
    current_palette = palette;
    active_palette = &palettes[palette];

    if (current_mode != TEXT_80x25) {
        apply_cga_colors();
    }
}

enum ScreenPaletteId ScreenCurrentPalette(void)
{
    return current_palette;
}

u8 ScreenIs640(void)
{
    return current_mode == CGA_640x200x2;
}

u8 ScreenRolePixel(enum ColorRole role)
{
    return active_palette->pixel[role];
}

u8 ScreenRoleBit(enum ColorRole role)
{
    return active_palette->bit[role];
}

u8 ScreenFillByte(void)
{
    if (current_mode == CGA_640x200x2) {
        return active_palette->bit[COLOR_PRIMARY_BACKGROUND] ? 0xff : 0x00;
    }

    return (u8)(active_palette->pixel[COLOR_PRIMARY_BACKGROUND] * 0x55);
}
