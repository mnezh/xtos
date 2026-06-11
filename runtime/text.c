#include "xtos/text.h"

static int continuation(unsigned char ch)
{
    return (ch & 0xc0) == 0x80;
}

int TextNextCodepoint(const char **cursor, u16 *codepoint)
{
    const unsigned char *p;
    u16 value;

    if (cursor == 0 || *cursor == 0 || codepoint == 0) {
        return 0;
    }

    p = (const unsigned char *)*cursor;

    if (*p == 0) {
        return 0;
    }

    if (*p < 0x80) {
        *codepoint = *p;
        *cursor = (const char *)(p + 1);
        return 1;
    }

    if ((*p & 0xe0) == 0xc0 && continuation(p[1])) {
        value = (u16)(((*p & 0x1f) << 6) | (p[1] & 0x3f));

        if (value >= 0x80) {
            *codepoint = value;
            *cursor = (const char *)(p + 2);
            return 1;
        }
    }

    if ((*p & 0xf0) == 0xe0 && continuation(p[1]) &&
        continuation(p[2])) {
        value = (u16)(((*p & 0x0f) << 12) |
                      ((p[1] & 0x3f) << 6) |
                      (p[2] & 0x3f));

        if (value >= 0x800) {
            *codepoint = value;
            *cursor = (const char *)(p + 3);
            return 1;
        }
    }

    *codepoint = XTOS_TEXT_REPLACEMENT;
    *cursor = (const char *)(p + 1);
    return 1;
}
