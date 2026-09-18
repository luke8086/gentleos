/*
 * Copyright (c) 2014-2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: kernel.h - Kernel API
 */

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include <lib.h>

typedef struct {
    uint16_t ofs;
    uint16_t seg;
} isr_st;

typedef uint16_t krn_lock_t;

enum {
    EVENT_UNKNOWN = 0,
    EVENT_KEY_DOWN = 5,
    EVENT_KEY_UP = 6,
    EVENT_TIMER_TICK = 7,
};

typedef struct {
    uint8_t type;
    uint32_t payload;
} event_st;

#define VGA_THEME_COUNT 4

typedef struct {
    uint32_t fg_color;
    uint32_t bg_color;
    const char *name;
} vga_theme_st;

extern void *krn_link_start;
extern void *krn_link_end;
extern uint16_t krn_main_segment;
extern uint16_t krn_flags;

enum {
    KRN_FLAG_COLORS_INVERTED = 1 << 1,
};

/* kernel/cpu.s */
typedef union {
    struct {
        uint16_t ax, bx, cx, dx;
        uint16_t bp, di, si, flags;
    } x;

    struct {
        uint8_t al, ah;
        uint8_t bl, bh;
        uint8_t cl, ch;
        uint8_t dl, dh;
    } h;
} regs_st;

uint16_t krn_cpu_get_flags(void);
void krn_cpu_set_flags(uint16_t flags);
void krn_cpu_cli(void);
void krn_cpu_hlt(void);
uint8_t krn_inb(uint16_t port);
void krn_outb(uint8_t value, uint16_t port);
void krn_intr(int, regs_st *);

#include "p_kernel.h"

#endif /* _KERNEL_H_ */
