#include "draw_cga.h"

#define CGA_WIDTH_BYTES 80
#define CGA_SCREEN_BYTES 16384
#define CGA_ODD_SCANLINE_OFFSET 0x2000

volatile u8 __far *DrawCgaMemory(void)
{
    return (volatile u8 __far *)0xb8000000UL;
}

u16 DrawCgaLineOffset(u16 y)
{
    return (u16)(((y & 1) ? CGA_ODD_SCANLINE_OFFSET : 0) +
                 ((y >> 1) * CGA_WIDTH_BYTES));
}

static u8 pixel_byte_320(u8 color)
{
    return (u8)((color & 3) * 0x55);
}

void DrawCgaPlotPixel320Row(volatile u8 __far *vram,
                            u16 line_offset, u16 x, u8 color)
{
    u16 offset;
    u8 shift;
    u8 mask;

    offset = (u16)(line_offset + (x >> 2));
    shift = (u8)((3 - (x & 3)) << 1);
    mask = (u8)(0x03 << shift);
    vram[offset] = (u8)((vram[offset] & ~mask) | ((color & 3) << shift));
}

void DrawCgaPlotBit640Row(volatile u8 __far *vram,
                          u16 line_offset, u16 x, u8 bit)
{
    u16 offset;
    u8 mask;

    offset = (u16)(line_offset + (x >> 3));
    mask = (u8)(0x80 >> (x & 7));

    if (bit) {
        vram[offset] |= mask;
    } else {
        vram[offset] &= (u8)~mask;
    }
}

void DrawCgaClear(void)
{
    volatile u8 __far *vram = DrawCgaMemory();
    u8 value;
    u16 i;

    value = ScreenFillByte();

    for (i = 0; i < CGA_SCREEN_BYTES; ++i) {
        vram[i] = value;
    }
}

static void plot_pixel_320(u16 x, u16 y, u8 color)
{
    volatile u8 __far *vram = DrawCgaMemory();

    DrawCgaPlotPixel320Row(vram, DrawCgaLineOffset(y), x, color);
}

static void plot_bit_640(u16 x, u16 y, u8 bit)
{
    volatile u8 __far *vram = DrawCgaMemory();

    DrawCgaPlotBit640Row(vram, DrawCgaLineOffset(y), x, bit);
}

static void plot_vertical_edge(u16 x, u16 y,
                               enum ColorRole role, u8 right_edge)
{
    if (ScreenIs640()) {
        plot_bit_640((u16)((x << 1) + (right_edge ? 1 : 0)),
                     y, ScreenRoleBit(role));
    } else {
        plot_pixel_320(x, y, ScreenRolePixel(role));
    }
}

static void fill_span_320(volatile u8 __far *vram, u16 y,
                          u16 left, u16 right, u8 color, u8 byte_value)
{
    u16 x;
    u16 offset;
    u16 byte_offset;
    u16 count;
    u16 i;

    x = left;
    offset = DrawCgaLineOffset(y);

    while (x <= right && (x & 3) != 0) {
        DrawCgaPlotPixel320Row(vram, offset, x, color);
        ++x;
    }

    if (x > right) {
        return;
    }

    count = (u16)((right - x + 1) >> 2);
    byte_offset = (u16)(offset + (x >> 2));

    for (i = 0; i < count; ++i) {
        vram[(u16)(byte_offset + i)] = byte_value;
    }

    x = (u16)(x + (count << 2));

    while (x <= right) {
        DrawCgaPlotPixel320Row(vram, offset, x, color);
        ++x;
    }
}

static void fill_span_640(volatile u8 __far *vram, u16 y,
                          u16 left, u16 right, u8 bit, u8 byte_value)
{
    u16 x;
    u16 physical_right;
    u16 offset;
    u16 byte_offset;
    u16 count;
    u16 i;

    x = (u16)(left << 1);
    physical_right = (u16)((right << 1) + 1);
    offset = DrawCgaLineOffset(y);

    while (x <= physical_right && (x & 7) != 0) {
        DrawCgaPlotBit640Row(vram, offset, x, bit);
        ++x;
    }

    if (x > physical_right) {
        return;
    }

    count = (u16)((physical_right - x + 1) >> 3);
    byte_offset = (u16)(offset + (x >> 3));

    for (i = 0; i < count; ++i) {
        vram[(u16)(byte_offset + i)] = byte_value;
    }

    x = (u16)(x + (count << 3));

    while (x <= physical_right) {
        DrawCgaPlotBit640Row(vram, offset, x, bit);
        ++x;
    }
}

static void fill_rect_320(volatile u8 __far *vram,
                          u16 left, u16 top, u16 right, u16 bottom,
                          u8 color, u8 byte_value)
{
    u16 y;

    for (y = top; y <= bottom; ++y) {
        fill_span_320(vram, y, left, right, color, byte_value);
    }
}

static void fill_rect_640(volatile u8 __far *vram,
                          u16 left, u16 top, u16 right, u16 bottom,
                          u8 bit, u8 byte_value)
{
    u16 y;

    for (y = top; y <= bottom; ++y) {
        fill_span_640(vram, y, left, right, bit, byte_value);
    }
}

static void draw_horizontal_line(u16 left, u16 right,
                                 u16 y, enum ColorRole role)
{
    volatile u8 __far *vram = DrawCgaMemory();
    u8 value;

    if (ScreenIs640()) {
        value = ScreenRoleBit(role);
        fill_span_640(vram, y, left, right, value,
                      value ? 0xff : 0x00);
    } else {
        value = ScreenRolePixel(role);
        fill_span_320(vram, y, left, right, value,
                      pixel_byte_320(value));
    }
}

static void draw_vertical_edge(u16 x, u16 top, u16 bottom,
                               enum ColorRole role, u8 right_edge)
{
    u16 y;

    for (y = top; y <= bottom; ++y) {
        plot_vertical_edge(x, y, role, right_edge);
    }
}

static void dotted_horizontal_320(volatile u8 __far *vram, u16 y,
                                  u16 left, u16 right, u8 color)
{
    u16 offset;
    u16 byte_x;
    u16 first_byte;
    u16 last_byte;
    u8 bg;
    u8 slot;
    u8 mask;
    u8 value;
    u16 x;

    offset = DrawCgaLineOffset(y);
    first_byte = (u16)(left >> 2);
    last_byte = (u16)(right >> 2);
    bg = ScreenRolePixel(COLOR_PRIMARY_BACKGROUND);

    for (byte_x = first_byte; byte_x <= last_byte; ++byte_x) {
        mask = 0;
        value = 0;

        for (slot = 0; slot < 4; ++slot) {
            x = (u16)((byte_x << 2) + slot);

            if (x >= left && x <= right) {
                mask |= (u8)(0x03 << ((3 - slot) << 1));

                if (((u16)(x - left) & 1) == 0) {
                    value |= (u8)((color & 3) << ((3 - slot) << 1));
                } else {
                    value |= (u8)((bg & 3) << ((3 - slot) << 1));
                }
            }
        }

        vram[(u16)(offset + byte_x)] =
            (u8)((vram[(u16)(offset + byte_x)] & ~mask) | value);
    }
}

static void dotted_point_320(volatile u8 __far *vram, u16 x, u16 y,
                             u8 color, u8 enabled)
{
    u16 offset;
    u8 bg;
    u8 shift;
    u8 mask;
    u8 value;

    offset = (u16)(DrawCgaLineOffset(y) + (x >> 2));
    bg = ScreenRolePixel(COLOR_PRIMARY_BACKGROUND);
    shift = (u8)((3 - (x & 3)) << 1);
    mask = (u8)(0x03 << shift);
    value = (u8)(((enabled ? color : bg) & 3) << shift);
    vram[offset] = (u8)((vram[offset] & ~mask) | value);
}

static void dotted_horizontal_640(volatile u8 __far *vram, u16 y,
                                  u16 left, u16 right, u8 bit)
{
    u16 offset;
    u16 byte_x;
    u16 first_byte;
    u16 last_byte;
    u16 physical_left;
    u16 physical_right;
    u16 physical_x;
    u16 logical_x;
    u8 bg;
    u8 slot;
    u8 mask;
    u8 value;

    offset = DrawCgaLineOffset(y);
    physical_left = (u16)(left << 1);
    physical_right = (u16)((right << 1) + 1);
    first_byte = (u16)(physical_left >> 3);
    last_byte = (u16)(physical_right >> 3);
    bg = ScreenRoleBit(COLOR_PRIMARY_BACKGROUND);

    for (byte_x = first_byte; byte_x <= last_byte; ++byte_x) {
        mask = 0;
        value = 0;

        for (slot = 0; slot < 8; ++slot) {
            physical_x = (u16)((byte_x << 3) + slot);

            if (physical_x >= physical_left && physical_x <= physical_right) {
                logical_x = (u16)(physical_x >> 1);
                mask |= (u8)(0x80 >> slot);

                if (((u16)(logical_x - left) & 1) == 0) {
                    value |= bit ? (u8)(0x80 >> slot) : 0;
                } else {
                    value |= bg ? (u8)(0x80 >> slot) : 0;
                }
            }
        }

        vram[(u16)(offset + byte_x)] =
            (u8)((vram[(u16)(offset + byte_x)] & ~mask) | value);
    }
}

static void dotted_point_640(volatile u8 __far *vram, u16 x, u16 y,
                             u8 bit, u8 enabled)
{
    u16 offset;
    u16 physical_x;
    u8 bg;
    u8 mask;
    u8 value;

    physical_x = (u16)(x << 1);
    offset = (u16)(DrawCgaLineOffset(y) + (physical_x >> 3));
    bg = ScreenRoleBit(COLOR_PRIMARY_BACKGROUND);
    mask = (u8)(0xc0 >> (physical_x & 7));
    value = (enabled ? bit : bg) ? mask : 0;
    vram[offset] = (u8)((vram[offset] & ~mask) | value);
}

void DrawCgaClearRect(u16 left, u16 top, u16 right, u16 bottom)
{
    volatile u8 __far *vram = DrawCgaMemory();
    u8 value;

    if (right < left || bottom < top) {
        return;
    }

    /* Dedicated pane clearing path. A 320-mode interior byte clears four
       pixels, and a 640-mode byte clears four logical pixels (eight bits).
       Only unaligned edges still need read/modify/write masking. */
    if (ScreenIs640()) {
        value = ScreenRoleBit(COLOR_PRIMARY_BACKGROUND);
        fill_rect_640(vram, left, top, right, bottom, value,
                      ScreenFillByte());
    } else {
        value = ScreenRolePixel(COLOR_PRIMARY_BACKGROUND);
        fill_rect_320(vram, left, top, right, bottom, value,
                      ScreenFillByte());
    }
}

void DrawCgaBar(u16 left, u16 top, u16 right, u16 bottom,
                enum ColorRole role)
{
    volatile u8 __far *vram = DrawCgaMemory();
    u8 value;

    if (right < left || bottom < top) {
        return;
    }

    /* Filled rectangles are the main UI primitive. The renderer handles the
       left/right partial bytes with masks, then writes the aligned middle as
       whole CGA bytes to avoid per-pixel offset calculation and VRAM RMW. */
    if (ScreenIs640()) {
        value = ScreenRoleBit(role);
        fill_rect_640(vram, left, top, right, bottom, value,
                      value ? 0xff : 0x00);
    } else {
        value = ScreenRolePixel(role);
        fill_rect_320(vram, left, top, right, bottom, value,
                      pixel_byte_320(value));
    }
}

void DrawCgaRect(u16 left, u16 top, u16 right, u16 bottom,
                 enum ColorRole role)
{
    draw_horizontal_line(left, right, top, role);
    draw_horizontal_line(left, right, bottom, role);
    draw_vertical_edge(left, top, bottom, role, 0);
    draw_vertical_edge(right, top, bottom, role, 1);
}

void DrawCgaDottedRect(u16 left, u16 top, u16 right, u16 bottom,
                       enum ColorRole role)
{
    volatile u8 __far *vram = DrawCgaMemory();
    u16 y;
    u8 value;
    u8 enabled;

    if (right < left || bottom < top) {
        return;
    }

    if (ScreenIs640()) {
        value = ScreenRoleBit(role);
        dotted_horizontal_640(vram, top, left, right, value);
        dotted_horizontal_640(vram, bottom, left, right, value);

        for (y = (u16)(top + 1); y < bottom; ++y) {
            enabled = (u8)(((u16)(y - top) & 1) == 0);
            dotted_point_640(vram, left, y, value, enabled);
            dotted_point_640(vram, right, y, value, enabled);
        }
    } else {
        value = ScreenRolePixel(role);
        dotted_horizontal_320(vram, top, left, right, value);
        dotted_horizontal_320(vram, bottom, left, right, value);

        for (y = (u16)(top + 1); y < bottom; ++y) {
            enabled = (u8)(((u16)(y - top) & 1) == 0);
            dotted_point_320(vram, left, y, value, enabled);
            dotted_point_320(vram, right, y, value, enabled);
        }
    }
}
