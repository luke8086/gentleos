#!/usr/bin/env python3

import re
import os
import glob

def load_palette(path):
    ignored = ['GIMP Palette', 'Name:', 'Columns:', '#']
    split_rex = r'\s+'
    ret = {}

    for line in open(path, "r"):
        if any(line.startswith(x) for x in ignored):
            continue

        cols = re.split(split_rex, line.strip())

        if len(cols) != 4:
            raise ValueError(f"Invalid line: {line}")

        r = int(cols[0])
        g = int(cols[1])
        b = int(cols[2])
        rgb = (r << 16) | (g << 8) | b
        idx = int(cols[3].replace('$', ''), 16)

        if not rgb in ret:
            ret[rgb] = idx

    return ret

def load_pixels(path):
    with open(path, "rb") as f:
        assert f.read(2) == b'BM', "Not a BMP file"
        f.read(4) # size
        f.read(4) # reserved
        bf_off_bits = int.from_bytes(f.read(4), 'little')

        bi_size = int.from_bytes(f.read(4), 'little')
        assert bi_size == 40, f"Unsupported DIB header size in {path}"

        width = int.from_bytes(f.read(4), 'little', signed=True)
        height = int.from_bytes(f.read(4), 'little', signed=True)
        planes = int.from_bytes(f.read(2), 'little')
        bits_per_pixel = int.from_bytes(f.read(2), 'little')
        compression = int.from_bytes(f.read(4), 'little')
        f.read(12) # Skip biSizeImage, biXPelsPerMeter, biYPelsPerMeter
        f.read(8) # Skip biClrUsed, biClrImportant

        assert planes == 1 and bits_per_pixel == 24 and compression == 0, "Only 24‑bit uncompressed BMP supported"

        row_padding = (4 - (width * 3) % 4) % 4 # rows are 4‑byte aligned
        top_down = height < 0 # BMP stores rows bottom‑to‑top; if height <0 it’s top‑to‑bottom

        f.seek(bf_off_bits)

        rows = []
        for _ in range(abs(height)):
            row = []
            for _ in range(width):
                b, g, r = f.read(3)
                row.append((r << 16) | (g << 8) | b)
            f.read(row_padding) # discard padding
            rows.append(row)

        if not top_down:
            rows.reverse()

        return rows

def load_bitmap(path, palette):
    print(f"Loading bitmap: {path}")
    name = os.path.splitext(os.path.basename(path))[0]
    alpha = int(0x56)
    pixels = load_pixels(path)
    width = len(pixels[0])
    height = len(pixels)

    pixel_lines = []

    for row in pixels:
        pixel_str = ""
        for x in row:
            try:
                pixel_str += f"\\x{palette[x]:02x}"
            except KeyError as e:
                raise Exception(f"Missing color: {hex(x)}")
        pixel_lines.append(f'        "{pixel_str}" \\')

    lines = [
        f"bitmap_st bitmap_{name} = {{",
        f"    .size = {{ .width = {width}, .height = {height} }},",
        f"    .alpha = {hex(alpha)},",
        f"    .pixels = (uint8_t *)",
        *pixel_lines,
        f"}};",
        "",
    ]

    return("\n".join(lines))

def main():
    palette = load_palette("misc/vga-256.gpl")
    bitmap_files = glob.glob("bitmaps/*.bmp")
    bitmaps = (load_bitmap(x, palette) for x in bitmap_files)

    lines = [
        '#include <gui.h>',
        '',
        *bitmaps,
    ]

    with open("data/data_bitmaps.c", "w") as f:
        f.write("\n".join(lines))

if __name__ == "__main__":
    main()
