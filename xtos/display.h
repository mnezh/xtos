#ifndef XTOS_DISPLAY_H
#define XTOS_DISPLAY_H

#include "types.h"

enum DisplayMode {
    DISPLAY_MODE_LOW,
    DISPLAY_MODE_HIGH
};

enum DisplayPalette {
    DISPLAY_PALETTE_BLACK_ON_WHITE,
    DISPLAY_PALETTE_WHITE_ON_BLACK,
    DISPLAY_PALETTE_WHITE_ON_BLUE,
    DISPLAY_PALETTE_BROWN_ON_BLUE,
    DISPLAY_PALETTE_BLUE_ON_WHITE,
    DISPLAY_PALETTE_COUNT
};

typedef struct DisplayModeInfo {
    enum DisplayMode id;
    const char *name;
    u16 width;
    u16 height;
    u8 colors;
} DisplayModeInfo;

typedef struct DisplayPaletteInfo {
    enum DisplayPalette id;
    const char *name;
} DisplayPaletteInfo;

void DisplaySetMode(enum DisplayMode mode);
enum DisplayMode DisplayCurrentMode(void);

void DisplaySetPalette(enum DisplayPalette palette);
enum DisplayPalette DisplayCurrentPalette(void);

u16 DisplayWidth(void);
u16 DisplayHeight(void);
u8 DisplayModeCount(void);
const DisplayModeInfo *DisplayModeInfoAt(u8 index);
u8 DisplayPaletteCount(void);
const DisplayPaletteInfo *DisplayPaletteInfoAt(u8 index);
void DisplayShutdown(void);

#endif
