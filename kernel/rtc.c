/*
 * Copyright (c) 2014-2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: rtc.c - Driver for Real Time Clock
 */

#include <kernel.h>

static void
krn_rtc_get_dos_time(time_st *t)
{
    regs_st regs;

    regs.h.ah = 0x2a;
    krn_intr(0x21, &regs);

    t->year = regs.x.cx;
    t->month = regs.h.dh;
    t->day = regs.h.dl;

    regs.h.ah = 0x2c;
    krn_intr(0x21, &regs);

    t->hour = regs.h.ch;
    t->minute = regs.h.cl;
    t->second = regs.h.dh;
}

global void
krn_rtc_init(void)
{
    int avail;
    time_st t;

    krn_debug_printf("Reading clock from %s... ", krn_is_dos() ? "DOS" : "BIOS");

    if (krn_is_dos()) {
        krn_rtc_get_dos_time(&t);
        avail = 1;
    } else {
        avail = krn_bios_get_time(&t);
    }

    if (avail && t.year > 2000) {
        time_set(&t);
        krn_debug_printf("available and set\n");
        return;
    }

    time_init(&t, 2026, 9, 1, 12, 0, 0);
    time_set(&t);

    krn_debug_printf(avail ? "available but unset" : "unavailable");
    krn_debug_printf(", using default time\n");
}
