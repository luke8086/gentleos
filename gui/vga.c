// --------------------------------------------------------------------------------------
// Copyright (c) 2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: vga.c - Routines for programming the VGA
// --------------------------------------------------------------------------------------

#include <gui.h>
#include "vga.h"

void
gui_vga_set_color(int index, uint32_t rgb)
{
    uint8_t dac_index = index;

#if GUI_PLANAR_MODE
    // In planar mode, resolve the real DAC index via the attribute controller
    inb(0x3DA);
    outb(index | 0x20, 0x3C0);
    dac_index = inb(0x3C1);
#endif

    outb(dac_index, 0x3C8);
    outb((rgb >> 18) & 0x3F, 0x3C9);
    outb((rgb >> 10) & 0x3F, 0x3C9);
    outb((rgb >>  2) & 0x3F, 0x3C9);
}

void
gui_vga_init(void)
{
    gui_vga_set_color(0x07, 0xb0b0b0);
    gui_vga_set_color(0x09, 0x3366aa);
    gui_vga_set_color(0x0e, 0xffcc00);

#if GUI_PLANAR_MODE
    gui_vga_set_write_mode(0);
    gui_vga_set_bit_mask(0xFF);
#endif
}
