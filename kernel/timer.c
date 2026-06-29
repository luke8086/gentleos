/*
 * Copyright (c) 2014-2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: timer.c - Driver for PIT 8254
 */

#include "lib.h"
#include <kernel.h>

enum {
    PIT_CR0 = 0x40,
    PIT_CWR = 0x43,
    TIMER_HZ = 20,
};

static isr_st saved_isr_handler;
extern void *krn_isr_timer;

volatile static uint32_t timer_msecs;

global void
krn_timer_handle_intr(void)
{
    event_st event;

    timer_msecs += (1000 / TIMER_HZ);

    event.type = EVENT_TIMER_TICK;
    event.payload = timer_msecs;

    (void)krn_event_ipush(&event);

    outb(0x20, 0x20);
}

global uint32_t
krn_timer_get_msecs(void)
{
    return timer_msecs;
}

global uint16_t
krn_timer_get_counter_0(void)
{
    uint8_t lo, hi;

    /* Latch counter 0 and read the snapshot */
    outb(0x00, PIT_CWR);
    lo = inb(PIT_CR0);
    hi = inb(PIT_CR0);

    return ((uint16_t)hi << 8) | lo;
}

global void
krn_timer_init(void)
{
    uint16_t div = 1193180 / TIMER_HZ;

    krn_debug_printf("Initializing timer... ");

    /* Set Counter 0, write both LSB and MSB, use mode 3, binary counter */
    outb(0x36, PIT_CWR);

    /* Write LSB and MSB for counter 0 */
    outb((uint8_t)((div >> 0) & 0xFF), PIT_CR0);
    outb((uint8_t)((div >> 8) & 0xFF), PIT_CR0);

    krn_get_isr(0x08, &saved_isr_handler);
    krn_set_isr(0x08, krn_main_segment, (uint16_t)(uint32_t)&krn_isr_timer);

    krn_debug_printf("ok\n");
}

global void
krn_timer_deinit(void)
{
    krn_set_isr(0x08, saved_isr_handler.seg, saved_isr_handler.ofs);
}
