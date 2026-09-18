/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: vga.c - Driver for CGA/VGA cards
 */

#include <kernel.h>

global const vga_theme_st krn_vga_themes[VGA_THEME_COUNT] = {
    { 0xcccccc, 0x000000, "Gray/Black" },
    { 0x67a353, 0x000000, "Green/Black" },
    { 0x00b0e8, 0x000000, "Blue/Black" },
    { 0x55ffff, 0x002041, "Cyan/Blue" },
};

global int krn_vga_current_theme = DEFAULT_VGA_THEME;

static void
krn_vga_set_color(uint8_t idx, uint32_t rgb)
{
    uint8_t dac_index;
    regs_st regs;
	uint16_t *rgb_words = (uint16_t *)&rgb;
	uint16_t rgb_hi = rgb_words[1];
	uint16_t rgb_lo = rgb_words[0];
    uint8_t r6 = (uint8_t)((rgb_hi >> 2) & 0x3F);
    uint8_t g6 = (uint8_t)((rgb_lo >> 10) & 0x3F);
    uint8_t b6 = (uint8_t)((rgb_lo >> 2) & 0x3F);

    regs.h.ah = 0x10;
    regs.h.al = 0x07;
    regs.h.bl = idx;
    krn_intr(0x10, &regs);
    dac_index = regs.h.bh;

    regs.h.ah = 0x10;
    regs.h.al = 0x10;
    regs.x.bx = dac_index;
    regs.h.dh = r6;
    regs.h.ch = g6;
    regs.h.cl = b6;

    krn_intr(0x10, &regs);
}

global void
krn_vga_set_theme(int n)
{
    if (n < 0 || n >= VGA_THEME_COUNT) {
        return;
    }

    krn_debug_printf("Setting color theme... ");

    krn_vga_set_color(0, krn_vga_themes[n].bg_color);
    krn_vga_set_color(3, krn_vga_themes[n].fg_color);
    krn_vga_current_theme = n;

    krn_debug_printf("ok\n");
}

global void
krn_vga_init(void)
{
    regs_st regs;

    krn_debug_printf("Initializing video... ");

    krn_debug_text_mode_enabled = 0;

    regs.h.ah = 0x00;
    regs.h.al = 0x04;
    krn_intr(0x10, &regs);

    krn_debug_printf("ok\n");

#if DEFAULT_VGA_THEME
    krn_vga_set_theme(DEFAULT_VGA_THEME);
#endif
}

global void
krn_vga_deinit(void)
{
    regs_st regs;

    regs.h.ah = 0x00;
    regs.h.al = 0x03;
    krn_intr(0x10, &regs);

    krn_debug_text_mode_enabled = 1;
}
