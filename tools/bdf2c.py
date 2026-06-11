#!/usr/bin/env python3
import argparse
import html
import re
from pathlib import Path


def ident_from_symbol(symbol):
    out = []
    for ch in symbol:
        if ch.isalnum():
            out.append(ch)
        else:
            out.append("_")
    return "".join(out)


def parse_bdf(path):
    font = {
        "font": path.stem,
        "width": 0,
        "height": 0,
        "ascent": 0,
        "descent": 0,
        "glyphs": {},
    }
    current = None
    in_bitmap = False
    bitmap = []

    for raw in path.read_text(encoding="latin-1").splitlines():
        line = html.unescape(re.sub(r"<[^>]+>", "", raw)).strip()

        if line.startswith("FONT "):
            font["font"] = line[5:].strip()
        elif line.startswith("FONTBOUNDINGBOX "):
            parts = line.split()
            font["width"] = int(parts[1])
            font["height"] = int(parts[2])
        elif line.startswith("FONT_ASCENT "):
            font["ascent"] = int(line.split()[1])
        elif line.startswith("FONT_DESCENT "):
            font["descent"] = int(line.split()[1])
        elif line.startswith("STARTCHAR "):
            current = {
                "encoding": None,
                "advance": font["width"],
                "width": font["width"],
                "height": font["height"],
            }
            bitmap = []
            in_bitmap = False
        elif current is not None and line.startswith("ENCODING "):
            current["encoding"] = int(line.split()[1])
        elif current is not None and line.startswith("DWIDTH "):
            current["advance"] = int(line.split()[1])
        elif current is not None and line.startswith("BBX "):
            parts = line.split()
            current["width"] = int(parts[1])
            current["height"] = int(parts[2])
        elif current is not None and line == "BITMAP":
            in_bitmap = True
            bitmap = []
        elif current is not None and line == "ENDCHAR":
            enc = current["encoding"]
            if enc is not None and 0 <= enc <= 65535:
                font["glyphs"][enc] = {
                    "width": current["width"],
                    "advance": current["advance"],
                    "height": current["height"],
                    "bitmap": bitmap[:],
                }
                font["width"] = max(font["width"], current["width"])
                font["height"] = max(font["height"], current["height"])
            current = None
            in_bitmap = False
        elif in_bitmap:
            bitmap.append(line)

    if font["ascent"] == 0 and font["descent"] == 0:
        font["ascent"] = font["height"]
        font["descent"] = 0

    return font


def glyph_row_bytes(row_text, width):
    row_text = row_text.strip()
    bit_count = len(row_text) * 4
    value = int(row_text, 16) if row_text else 0
    row = []
    byte = 0

    for col in range(width):
        src_bit = bit_count - 1 - col
        if src_bit >= 0 and (value & (1 << src_bit)):
            byte |= 0x80 >> (col & 7)
        if (col & 7) == 7:
            row.append(byte)
            byte = 0

    if width & 7:
        row.append(byte)

    return row


def build_glyph_data(font, wanted_codes=None):
    glyphs = font["glyphs"]
    if wanted_codes is not None:
        glyphs = {code: glyphs[code] for code in wanted_codes if code in glyphs}
    width = font["width"]
    height = font["height"]
    bytes_per_row = (width + 7) // 8
    data = []
    glyph_map = []
    widths = []
    glyph_index = 0

    for code in sorted(glyphs.keys()):
        glyph = glyphs[code]
        rows = glyph["bitmap"] if glyph else []
        glyph_map.append((code, glyph_index))
        widths.append(glyph["advance"])

        for row_index in range(height):
            if row_index < len(rows):
                row = glyph_row_bytes(rows[row_index], width)
            else:
                row = []
            row += [0] * (bytes_per_row - len(row))
            data.extend(row[:bytes_per_row])
        glyph_index += 1

    return bytes_per_row, data, glyph_map, widths


def write_header(path, symbol):
    guard = re.sub(r"[^A-Za-z0-9]", "_", str(path)).upper() + "_"
    path.write_text(
        "#ifndef {guard}\n"
        "#define {guard}\n\n"
        "#include \"xtos/ui/font.h\"\n\n"
        "extern const Font {symbol};\n\n"
        "#endif\n".format(guard=guard, symbol=symbol),
        encoding="ascii",
    )


def replacement_glyph(glyph_map):
    for code, index in glyph_map:
        if code == 0xFFFD:
            return index
    for code, index in glyph_map:
        if code == ord("?"):
            return index
    return 65535


def font_flags(font, widths):
    if widths and any(width != widths[0] for width in widths):
        return "FONT_PROPORTIONAL"
    return "FONT_MONOSPACE"


def write_source(path, header, symbol, display_name, font, data, glyph_map, widths):
    data_name = ident_from_symbol(symbol) + "_Glyphs"
    map_name = ident_from_symbol(symbol) + "_Map"
    widths_name = ident_from_symbol(symbol) + "_Widths"
    lines = []
    lines.append("#include \"{}\"\n".format(header.as_posix()))
    lines.append("static const u8 {}[] = {{".format(data_name))

    for i in range(0, len(data), 12):
        chunk = data[i : i + 12]
        lines.append("    " + ", ".join("0x{:02x}".format(v) for v in chunk) + ",")

    lines.append("};\n")
    lines.append("static const GlyphMap {}[] = {{".format(map_name))

    for code, index in glyph_map:
        lines.append("    {{0x{:04x}, {}}},".format(code, index))

    lines.append("};\n")
    lines.append("static const u8 {}[] = {{".format(widths_name))

    for i in range(0, len(widths), 12):
        chunk = widths[i : i + 12]
        lines.append("    " + ", ".join("{}".format(v) for v in chunk) + ",")

    lines.append("};\n")
    lines.append("const Font {} = {{".format(symbol))
    lines.append("    \"{}\",".format(display_name))
    lines.append("    {}, {},".format(font["width"], font["height"]))
    lines.append("    {}, {},".format(font["ascent"], font["descent"]))
    lines.append("    {}, {},".format(len(glyph_map), replacement_glyph(glyph_map)))
    lines.append("    {},".format(font_flags(font, widths)))
    lines.append("    {}, {}, {}".format(data_name, map_name, widths_name))
    lines.append("};")
    path.write_text("\n".join(lines) + "\n", encoding="ascii")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=Path)
    parser.add_argument("--header", required=True, type=Path)
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--symbol", required=True)
    parser.add_argument("--name", required=True)
    parser.add_argument("--text", default="")
    args = parser.parse_args()

    font = parse_bdf(args.input)
    wanted_codes = None
    if args.text:
        wanted_codes = set(range(32, 127))
        wanted_codes.update(ord(ch) for ch in args.text)
    _bytes_per_row, data, glyph_map, widths = build_glyph_data(font, wanted_codes)
    write_header(args.header, args.symbol)
    write_source(args.source, args.header, args.symbol, args.name, font, data, glyph_map, widths)


if __name__ == "__main__":
    main()
