#!/usr/bin/env python3
"""Convert a raw XTOS CGA dump to BMP or PNG.

XTOS.CGA is a 16 KiB dump of B800:0000 in CGA graphics mode.
"""

import argparse
import struct
import zlib

CGA_SIZE = 16384
WIDTH_320 = 320
WIDTH_640 = 640
HEIGHT = 200
BYTES_PER_LINE = 80
ODD_FIELD_OFFSET = 8192

PALETTE_320 = [
    (0x00, 0x00, 0x00),
    (0x55, 0xff, 0xff),
    (0xff, 0x55, 0xff),
    (0xff, 0xff, 0xff),
]

PALETTE_640 = [
    (0x00, 0x00, 0x00),
    (0xff, 0xff, 0xff),
]


def cga_line_offset(y):
    field = ODD_FIELD_OFFSET if (y & 1) else 0
    return field + (y >> 1) * BYTES_PER_LINE


def decode_320(data):
    pixels = []
    for y in range(HEIGHT):
        line = []
        offset = cga_line_offset(y)
        for byte in data[offset:offset + BYTES_PER_LINE]:
            line.append(PALETTE_320[(byte >> 6) & 0x03])
            line.append(PALETTE_320[(byte >> 4) & 0x03])
            line.append(PALETTE_320[(byte >> 2) & 0x03])
            line.append(PALETTE_320[byte & 0x03])
        pixels.append(line)
    return WIDTH_320, pixels


def decode_640(data):
    pixels = []
    for y in range(HEIGHT):
        line = []
        offset = cga_line_offset(y)
        for byte in data[offset:offset + BYTES_PER_LINE]:
            mask = 0x80
            while mask:
                line.append(PALETTE_640[1 if (byte & mask) else 0])
                mask >>= 1
        pixels.append(line)
    return WIDTH_640, pixels


def write_bmp(path, width, pixels):
    row_size = (width * 3 + 3) & ~3
    pixel_bytes = row_size * HEIGHT
    header_size = 14 + 40

    with open(path, "wb") as out:
        out.write(b"BM")
        out.write(struct.pack("<IHHI", header_size + pixel_bytes, 0, 0,
                              header_size))
        out.write(struct.pack("<IiiHHIIiiII", 40, width, HEIGHT, 1, 24, 0,
                              pixel_bytes, 0, 0, 0, 0))

        padding = b"\x00" * (row_size - width * 3)
        for line in reversed(pixels):
            for r, g, b in line:
                out.write(bytes((b, g, r)))
            out.write(padding)


def png_chunk(kind, payload):
    return (struct.pack(">I", len(payload)) + kind + payload +
            struct.pack(">I", zlib.crc32(kind + payload) & 0xffffffff))


def write_png(path, width, pixels):
    raw = bytearray()
    for line in pixels:
        raw.append(0)
        for r, g, b in line:
            raw.extend((r, g, b))

    with open(path, "wb") as out:
        out.write(b"\x89PNG\r\n\x1a\n")
        out.write(png_chunk(b"IHDR",
                            struct.pack(">IIBBBBB", width, HEIGHT, 8, 2,
                                        0, 0, 0)))
        out.write(png_chunk(b"IDAT", zlib.compress(bytes(raw), 9)))
        out.write(png_chunk(b"IEND", b""))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("input")
    parser.add_argument("output")
    parser.add_argument("--mode", choices=("320", "640"), default="320")
    args = parser.parse_args()

    with open(args.input, "rb") as inp:
        data = inp.read()

    if len(data) != CGA_SIZE:
        raise SystemExit("expected 16384-byte CGA dump")

    if args.mode == "640":
        width, pixels = decode_640(data)
    else:
        width, pixels = decode_320(data)

    lower_output = args.output.lower()
    if lower_output.endswith(".png"):
        write_png(args.output, width, pixels)
    elif lower_output.endswith(".bmp"):
        write_bmp(args.output, width, pixels)
    else:
        raise SystemExit("output must end in .bmp or .png")


if __name__ == "__main__":
    main()
