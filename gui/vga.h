// --------------------------------------------------------------------------------------
// Copyright (c) 2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: vga.h - Inline routines for programming the VGA
// --------------------------------------------------------------------------------------

#include <kernel.h>

static inline void
gui_vga_set_write_planes(uint8_t plane_mask)
{
    outw((plane_mask << 8) | 0x02, 0x3C4);
}

static inline void
gui_vga_set_bit_mask(uint8_t mask)
{
    outw((mask << 8) | 0x08, 0x3CE);
}

static inline void
gui_vga_set_write_mode(uint8_t mode)
{
    outw((mode << 8) | 0x05, 0x3CE);
}

static inline void
gui_vga_set_logic_op(uint8_t op)
{
    outw((op << 8) | 0x03, 0x3CE);
}

static inline void
gui_vga_latch_write(volatile uint8_t *addr, uint8_t val)
{
    (void)*addr;
    *addr = val;
}
