#!/usr/bin/env python3
#
# Copyright (c) 2026 luke8086
# Distributed under the terms of GPL-2 License.
#
# File: mksbn.py - Convert Sokoban levels from text to binary
#

# /// script
# requires-python = ">=3.10"
# dependencies = [
# ]
# ///

import argparse
import re
import struct
from pathlib import Path

LEVEL_MAX_WIDTH = 30
LEVEL_MAX_HEIGHT = 18

TILE_WALL = 0x01
TILE_GOAL = 0x02
TILE_BOX = 0x04
TILE_MAN = 0x08

TILES = {
    " ": 0,
    "#": TILE_WALL,
    ".": TILE_GOAL,
    "$": TILE_BOX,
    "*": TILE_BOX | TILE_GOAL,
    "@": TILE_MAN,
    "+": TILE_MAN | TILE_GOAL,
}

ROW_REX = re.compile(r"[ #.$*@+]+")


def die(msg):
    raise SystemExit(msg)


def read_levels(path):
    levels = []
    rows = None

    for line in path.read_text().splitlines():
        line = line.rstrip()

        if not ROW_REX.fullmatch(line):
            rows = None
            continue

        if rows is None:
            rows = []
            levels.append(rows)

        rows.append(line)

    return levels


def convert_levels(levels):
    data = bytearray(struct.pack("<H", len(levels)))

    for number, rows in enumerate(levels, 1):
        height = len(rows)
        width = max(len(row) for row in rows)

        level_data = bytearray([width, height])
        for row in rows:
            level_data += bytes(TILES[ch] for ch in row.ljust(width))

        if width > LEVEL_MAX_WIDTH or height > LEVEL_MAX_HEIGHT:
            die(f"Level {number} is too big ({width}x{height})")

        data += level_data
        print(f"{number}: {width}x{height}")

    return data


def run(in_path, out_path):
    levels = read_levels(in_path)
    data = convert_levels(levels)
    out_path.write_bytes(data)

    print(f"{out_path}: {len(levels)} levels, {len(data)} bytes")


def main():
    parser = argparse.ArgumentParser(
        description="Convert Sokoban levels from text to binary")
    parser.add_argument("-i", dest="input", type=Path, required=True, metavar="FILE",
        help="input file")
    parser.add_argument("-o", dest="output", type=Path, required=True, metavar="FILE",
        help="output file")
    args = parser.parse_args()

    try:
        run(args.input, args.output)
    except OSError as exc:
        die(f"Error: {exc}")


if __name__ == "__main__":
    main()
