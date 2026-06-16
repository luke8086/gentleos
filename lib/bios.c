/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: bios.c - Wrappers for BIOS functions
 */

#include <lib.h>

static uint8_t
to_bcd(uint8_t v)
{
    return ((v / 10) << 4) | (v % 10);
}

static unsigned
from_bcd(uint16_t v)
{
    return ((v >> 4) & 0x0F) * 10 + (v & 0x0F);
}

global void
bios_putc(char c)
{
    regs_st regs;

    regs.h.ah = 0x0e;
    regs.h.al = c;
    regs.x.bx = 0;

    intr(0x10, &regs);
}

global void
bios_puts(const char *s)
{
    while (*s) {
        if ((*s) == '\n') {
            bios_putc('\r');
        }

        bios_putc(*s++);
    }
}

global uint16_t
bios_getc(void)
{
    regs_st regs;

    regs.h.ah = 0x00;
    intr(0x16, &regs);

    return regs.x.ax;
}

global uint16_t
bios_get_key(void)
{
    regs_st regs;
    key_st key;

    regs.h.ah = 0x01;
    intr(0x16, &regs);

    if (regs.x.flags & 0x40) {
        return 0;
    }

    regs.h.ah = 0x00;
    intr(0x16, &regs);
    key.p.code = regs.h.ah;

    regs.h.ah = 0x02;
    intr(0x16, &regs);
    key.p.mods =
        (KEY_MOD_SHIFT * ((regs.h.al & 0x03) != 0)) |
        (KEY_MOD_CTRL  * ((regs.h.al & 0x04) != 0)) |
        (KEY_MOD_ALT   * ((regs.h.al & 0x08) != 0));

    return key.encoded;
}

global void
bios_uart_init(void)
{
    regs_st regs;

    regs.h.ah = 0x00;
    regs.h.al = 0xe3; /* 8N1, 9600 */
    regs.x.dx = 0;

    intr(0x14, &regs);
}

global void
bios_uart_putc(char c)
{
    regs_st regs;

    regs.h.ah = 0x01;
    regs.h.al = c;
    regs.x.dx = 0;

    intr(0x14, &regs);
}

global void
bios_uart_puts(const char *s)
{
    while (*s) {
        if ((*s) == '\n') {
            bios_uart_putc('\r');
        }

        bios_uart_putc(*s++);
    }
}

global void
bios_get_time(time_st *t)
{
    regs_st regs;

    regs.h.ah = 0x02;

    intr(0x1a, &regs);

    t->hour = from_bcd(regs.h.ch);
    t->minute = from_bcd(regs.h.cl);
    t->second = from_bcd(regs.h.dh);

    /* Temporary workaround for machines not supporting this call */
    t->hour = MIN(23, t->hour);
    t->minute = MIN(59, t->minute);
    t->second = MIN(59, t->second);
}

global void
bios_set_time(uint8_t hour, uint8_t minute, uint8_t second)
{
    regs_st regs;

    regs.h.ah = 0x03;
    regs.h.ch = to_bcd(hour);
    regs.h.cl = to_bcd(minute);
    regs.h.dh = to_bcd(second);
    regs.h.dl = 0;

    intr(0x1a, &regs);
}

global void
bios_get_date(date_st *d)
{
    regs_st regs;

    regs.h.ah = 0x04;

    intr(0x1a, &regs);

    d->year = from_bcd(regs.h.ch) * 100 + from_bcd(regs.h.cl);
    d->month = from_bcd(regs.h.dh);
    d->day = from_bcd(regs.h.dl);

    /* Temporary workaround for machines not supporting this call */
    d->year = MAX(1900, MIN(2100, d->year));
    d->month = MAX(1, MIN(12, d->month));
    d->day = MAX(1, MIN(31, d->month));
}

global void
bios_set_date(uint16_t year, uint8_t month, uint8_t day)
{
    regs_st regs;

    regs.h.ah = 0x05;
    regs.h.ch = to_bcd(year / 100);
    regs.h.cl = to_bcd(year % 100);
    regs.h.dh = to_bcd(month);
    regs.h.dl = to_bcd(day);

    intr(0x1a, &regs);
}

global void
bios_reboot(void)
{
    void (far *reset)(void) = MK_FP(0xFFFF, 0);
    uint8_t far *bda = MK_FP(0x40, 0);

    *(uint16_t far *)(bda + 0x72) = 0x1234; /* Prefer warm boot */

    reset();
}
