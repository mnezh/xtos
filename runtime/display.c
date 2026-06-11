#include "screen.h"
#include "cursor.h"
#include "../xtos/display.h"
#include "../xtos/ui/invalidation.h"

static enum DisplayMode current_display_mode;
static enum DisplayPalette current_display_palette;
static u8 display_mode_set;
static u8 display_palette_set;

static const DisplayModeInfo display_modes[] = {
    { DISPLAY_MODE_LOW, "320x200x4", 320, 200, 4 },
    { DISPLAY_MODE_HIGH, "640x200x2", 640, 200, 2 }
};

static const DisplayPaletteInfo display_palettes[] = {
    { DISPLAY_PALETTE_BLACK_ON_WHITE, "Black on White" },
    { DISPLAY_PALETTE_WHITE_ON_BLACK, "White on Black" },
    { DISPLAY_PALETTE_BLUE_ON_WHITE, "Blue on White" },
    { DISPLAY_PALETTE_WHITE_ON_BLUE, "White on Blue" },
    { DISPLAY_PALETTE_BROWN_ON_BLUE, "Brown on Blue" }
};

static enum ScreenModeId screen_mode_for_display(enum DisplayMode mode)
{
    if (mode == DISPLAY_MODE_HIGH) {
        return CGA_640x200x2;
    }

    return CGA_320x200x4;
}

static enum ScreenPaletteId screen_palette_for_display(enum DisplayPalette palette)
{
    if (palette == DISPLAY_PALETTE_WHITE_ON_BLACK) {
        return WHITE_ON_BLACK;
    }

    if (palette == DISPLAY_PALETTE_BLUE_ON_WHITE) {
        return BLUE_ON_WHITE;
    }

    if (palette == DISPLAY_PALETTE_WHITE_ON_BLUE) {
        return WHITE_ON_BLUE;
    }

    if (palette == DISPLAY_PALETTE_BROWN_ON_BLUE) {
        return BROWN_ON_BLUE;
    }

    return BLACK_ON_WHITE;
}

void DisplaySetMode(enum DisplayMode mode)
{
    if (mode != DISPLAY_MODE_LOW && mode != DISPLAY_MODE_HIGH) {
        mode = DISPLAY_MODE_LOW;
    }

    if (display_mode_set && current_display_mode == mode) {
        return;
    }

    current_display_mode = mode;
    display_mode_set = 1;
    display_palette_set = 0;
    MouseCursorHide();
    ScreenMode(screen_mode_for_display(mode));
    MouseCursorReset();
    InvalidateAll();
}

enum DisplayMode DisplayCurrentMode(void)
{
    return current_display_mode;
}

void DisplaySetPalette(enum DisplayPalette palette)
{
    if (palette != DISPLAY_PALETTE_BLACK_ON_WHITE &&
        palette != DISPLAY_PALETTE_WHITE_ON_BLACK &&
        palette != DISPLAY_PALETTE_BLUE_ON_WHITE &&
        palette != DISPLAY_PALETTE_WHITE_ON_BLUE &&
        palette != DISPLAY_PALETTE_BROWN_ON_BLUE) {
        palette = DISPLAY_PALETTE_BLUE_ON_WHITE;
    }

    if (display_palette_set && current_display_palette == palette) {
        return;
    }

    current_display_palette = palette;
    display_palette_set = 1;
    MouseCursorHide();
    ScreenPalette(screen_palette_for_display(palette));
    InvalidateAll();
}

enum DisplayPalette DisplayCurrentPalette(void)
{
    return current_display_palette;
}

u16 DisplayWidth(void)
{
    return 320;
}

u16 DisplayHeight(void)
{
    return 200;
}

u8 DisplayModeCount(void)
{
    return (u8)(sizeof(display_modes) / sizeof(display_modes[0]));
}

const DisplayModeInfo *DisplayModeInfoAt(u8 index)
{
    if (index >= DisplayModeCount()) {
        return 0;
    }

    return &display_modes[index];
}

u8 DisplayPaletteCount(void)
{
    return (u8)(sizeof(display_palettes) / sizeof(display_palettes[0]));
}

const DisplayPaletteInfo *DisplayPaletteInfoAt(u8 index)
{
    if (index >= DisplayPaletteCount()) {
        return 0;
    }

    return &display_palettes[index];
}

void DisplayShutdown(void)
{
    MouseCursorHide();
    ScreenMode(TEXT_80x25);
    display_mode_set = 0;
    display_palette_set = 0;
}
