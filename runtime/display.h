#ifndef RUNTIME_DISPLAY_H
#define RUNTIME_DISPLAY_H

#include "../xtos/display.h"

void RuntimeDisplaySetMode(enum DisplayMode mode);
enum DisplayMode RuntimeDisplayCurrentMode(void);
void RuntimeDisplaySetPalette(enum DisplayPalette palette);
enum DisplayPalette RuntimeDisplayCurrentPalette(void);

#endif
