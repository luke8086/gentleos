#!/usr/bin/env python3

import re
import os
import glob

from PIL import Image

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
    img = Image.open(path).convert("RGB")
    width, height = img.size
    data = img.load()

    rows = []
    for y in range(height):
        row = []
        for x in range(width):
            r, g, b = data[x, y]
            row.append((r << 16) | (g << 8) | b)
        rows.append(row)

    return rows

def load_image(path, palette):
    print(f"Loading image: {path}")
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
        f"    .foreground = 0x00,",
        f"    .alpha = {hex(alpha)},",
        f"    .pixels = (uint8_t *)",
        *pixel_lines,
        f"}};",
        "",
    ]

    return("\n".join(lines))

def main():
    palette = load_palette("misc/vga-256.gpl")
    bitmap_files = sorted(glob.glob("bitmaps/*.bmp"))
    bitmaps = (load_image(x, palette) for x in bitmap_files)

    lines = [
        '#include <gui.h>',
        '',
        *bitmaps,
    ]

    with open("data/data_bitmaps.c", "w") as f:
        f.write("\n".join(lines))

if __name__ == "__main__":
    main()
