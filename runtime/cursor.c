#include "cursor.h"
#include "draw.h"
#include "screen.h"
#include "xtos/display.h"

#define CGA_WIDTH_BYTES 80
#define CGA_ODD_SCANLINE_OFFSET 0x2000
#define CURSOR_WIDTH 8
#define CURSOR_HEIGHT 8
#define CURSOR_MAX_BYTES_PER_ROW 4

static const u8 cursor_primary_bits[CURSOR_HEIGHT] = {
    0x80,
    0xc0,
    0xa0,
    0x90,
    0x88,
    0xc0,
    0xa0,
    0x10
};

static const u8 cursor_inverted_bits[CURSOR_HEIGHT] = {
    0x40,
    0x20,
    0x50,
    0x68,
    0x74,
    0x20,
    0x40,
    0x00
};

static u8 saved_bytes[CURSOR_HEIGHT][CURSOR_MAX_BYTES_PER_ROW];
static u8 saved_counts[CURSOR_HEIGHT];
static u16 saved_offsets[CURSOR_HEIGHT];
static u16 cursor_x;
static u16 cursor_y;
static u8 cursor_visible;
static u8 cursor_saved;

static volatile u8 __far *cga(void)
{
    return (volatile u8 __far *)0xb8000000UL;
}

static u16 cga_line_offset(u16 y)
{
    return (u16)(((y & 1) ? CGA_ODD_SCANLINE_OFFSET : 0) +
                 ((y >> 1) * CGA_WIDTH_BYTES));
}

static void cursor_byte_span(u16 x, u16 *byte_left, u16 *byte_right)
{
    u16 physical_left;
    u16 physical_right;

    if (ScreenIs640()) {
        physical_left = (u16)(x << 1);
        physical_right = (u16)(((x + CURSOR_WIDTH - 1) << 1) + 1);
        *byte_left = (u16)(physical_left >> 3);
        *byte_right = (u16)(physical_right >> 3);
    } else {
        *byte_left = (u16)(x >> 2);
        *byte_right = (u16)((x + CURSOR_WIDTH - 1) >> 2);
    }
}

static void save_background(void)
{
    volatile u8 __far *vram;
    u16 row;
    u16 byte_left;
    u16 byte_right;
    u16 count;
    u16 i;

    vram = cga();
    cursor_byte_span(cursor_x, &byte_left, &byte_right);
    count = (u16)(byte_right - byte_left + 1);

    if (count > CURSOR_MAX_BYTES_PER_ROW) {
        count = CURSOR_MAX_BYTES_PER_ROW;
    }

    for (row = 0; row < CURSOR_HEIGHT; ++row) {
        if ((u16)(cursor_y + row) >= DisplayHeight()) {
            saved_counts[row] = 0;
        } else {
            saved_offsets[row] = (u16)(cga_line_offset((u16)(cursor_y + row)) +
                                       byte_left);
            saved_counts[row] = (u8)count;

            for (i = 0; i < count; ++i) {
                saved_bytes[row][i] = vram[(u16)(saved_offsets[row] + i)];
            }
        }
    }

    cursor_saved = 1;
}

static void restore_background(void)
{
    volatile u8 __far *vram;
    u16 row;
    u16 i;

    if (!cursor_saved) {
        return;
    }

    vram = cga();

    for (row = 0; row < CURSOR_HEIGHT; ++row) {
        for (i = 0; i < saved_counts[row]; ++i) {
            vram[(u16)(saved_offsets[row] + i)] = saved_bytes[row][i];
        }
    }

    cursor_saved = 0;
}

static void draw_cursor(void)
{
    u16 row;
    u16 col;

    for (row = 0; row < CURSOR_HEIGHT; ++row) {
        if ((u16)(cursor_y + row) >= DisplayHeight()) {
            return;
        }

        for (col = 0; col < CURSOR_WIDTH; ++col) {
            if ((u16)(cursor_x + col) < DisplayWidth()) {
                if (cursor_primary_bits[row] & (0x80 >> col)) {
                    DrawBar((u16)(cursor_x + col), (u16)(cursor_y + row),
                            (u16)(cursor_x + col), (u16)(cursor_y + row),
                            COLOR_PRIMARY_FOREGROUND);
                } else if (cursor_inverted_bits[row] & (0x80 >> col)) {
                    DrawBar((u16)(cursor_x + col), (u16)(cursor_y + row),
                            (u16)(cursor_x + col), (u16)(cursor_y + row),
                            COLOR_INVERTED_FOREGROUND);
                }
            }
        }
    }
}

void MouseCursorSetPosition(u16 x, u16 y)
{
    if (x > (u16)(DisplayWidth() - CURSOR_WIDTH)) {
        x = (u16)(DisplayWidth() - CURSOR_WIDTH);
    }

    if (y > (u16)(DisplayHeight() - CURSOR_HEIGHT)) {
        y = (u16)(DisplayHeight() - CURSOR_HEIGHT);
    }

    if (cursor_visible) {
        restore_background();
        cursor_x = x;
        cursor_y = y;
        save_background();
        draw_cursor();
    } else {
        cursor_x = x;
        cursor_y = y;
    }
}

void MouseCursorShow(void)
{
    if (cursor_visible) {
        return;
    }

    save_background();
    draw_cursor();
    cursor_visible = 1;
}

void MouseCursorHide(void)
{
    if (!cursor_visible) {
        return;
    }

    restore_background();
    cursor_visible = 0;
}

void MouseCursorReset(void)
{
    cursor_visible = 0;
    cursor_saved = 0;
}
