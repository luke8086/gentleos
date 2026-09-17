/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: song.c - Operations on songs
 */

#include <lib.h>

global uint32_t
song_get_total_ticks(const note_st far *notes)
{
    uint32_t ticks = 0;

    while (notes->ticks != 0) {
        ticks += notes->ticks;
        ++notes;
    }

    return ticks;
}
