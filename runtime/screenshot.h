#ifndef RUNTIME_SCREENSHOT_H
#define RUNTIME_SCREENSHOT_H

#include "../xtos/types.h"

int RuntimeScreenshotCga(const char *path);
u8 RuntimeCgaReadByte(u16 offset);

#endif
