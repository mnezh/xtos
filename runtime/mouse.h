#ifndef RUNTIME_MOUSE_H
#define RUNTIME_MOUSE_H

#include "types.h"

u16 MouseReset(void);
u16 MouseRead(u16 *x, u16 *y, u16 *buttons);

void RuntimeMouseInit(void);
u8 RuntimeMousePresent(void);
void RuntimePumpMouse(void);

#endif
