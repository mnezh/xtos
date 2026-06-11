#ifndef RUNTIME_SCREEN_H
#define RUNTIME_SCREEN_H

#include "types.h"

enum ScreenModeId {
    TEXT_80x25 = 0x03,
    CGA_320x200x4 = 0x04,
    CGA_640x200x2 = 0x06
};

enum ScreenPaletteId {
    BLACK_ON_WHITE,
    WHITE_ON_BLACK,
    BLUE_ON_WHITE,
    WHITE_ON_BLUE,
    BROWN_ON_BLUE,
    SCREEN_PALETTE_COUNT
};

enum ColorRole {
    COLOR_PRIMARY_BACKGROUND,
    COLOR_PRIMARY_FOREGROUND,
    COLOR_EXTRA_1_ON_BACKGROUND,
    COLOR_EXTRA_2_ON_BACKGROUND,
    COLOR_INVERTED_BACKGROUND,
    COLOR_INVERTED_FOREGROUND,
    COLOR_INVERTED_EXTRA_1,
    COLOR_INVERTED_EXTRA_2,
    COLOR_ROLE_COUNT
};

void ScreenMode(enum ScreenModeId mode);
enum ScreenModeId ScreenCurrentMode(void);

void ScreenPalette(enum ScreenPaletteId palette);
enum ScreenPaletteId ScreenCurrentPalette(void);

u8 ScreenIs640(void);
u8 ScreenRolePixel(enum ColorRole role);
u8 ScreenRoleBit(enum ColorRole role);
u8 ScreenFillByte(void);

#endif
