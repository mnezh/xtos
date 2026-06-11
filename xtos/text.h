#ifndef XTOS_TEXT_H
#define XTOS_TEXT_H

#include "types.h"

#define XTOS_TEXT_REPLACEMENT 0xfffd

int TextNextCodepoint(const char **cursor, u16 *codepoint);

#endif
