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

int TextEncodeUtf8(u16 codepoint, char *out, u8 out_size)
{
    if (out == 0 || out_size == 0) {
        return 0;
    }

    if (codepoint < 0x80) {
        if (out_size < 2) {
            out[0] = 0;
            return 0;
        }

        out[0] = (char)codepoint;
        out[1] = 0;
        return 1;
    }

    if (codepoint < 0x800) {
        if (out_size < 3) {
            out[0] = 0;
            return 0;
        }

        out[0] = (char)(0xc0 | (codepoint >> 6));
        out[1] = (char)(0x80 | (codepoint & 0x3f));
        out[2] = 0;
        return 2;
    }

    if (out_size < 4) {
        out[0] = 0;
        return 0;
    }

    out[0] = (char)(0xe0 | (codepoint >> 12));
    out[1] = (char)(0x80 | ((codepoint >> 6) & 0x3f));
    out[2] = (char)(0x80 | (codepoint & 0x3f));
    out[3] = 0;
    return 3;
}
