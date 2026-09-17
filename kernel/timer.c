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
};

static isr_st saved_isr_handler;
extern void *krn_isr_timer;

volatile static uint32_t timer_msecs;
static uint16_t timer_msecs_per_tick;

global void
krn_timer_handle_intr(void)
{
    event_st event;

    timer_msecs += timer_msecs_per_tick;

    krn_speaker_on_tick();

    event.type = EVENT_TIMER_TICK;
    event.payload = timer_msecs;

    (void)krn_event_ipush(&event);

    krn_outb(0x20, 0x20);
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
    krn_outb(0x00, PIT_CWR);
    lo = krn_inb(PIT_CR0);
    hi = krn_inb(PIT_CR0);

    return ((uint16_t)hi << 8) | lo;
}

static void
krn_timer_set_counter_0(uint16_t div)
{
    /* Set Counter 0, write both LSB and MSB, use mode 3 (square wave), binary counter */
    krn_outb(0x36, PIT_CWR);

    /* Write LSB and MSB for counter 0 */
    krn_outb((uint8_t)((div >> 0) & 0xFF), PIT_CR0);
    krn_outb((uint8_t)((div >> 8) & 0xFF), PIT_CR0);
}

/* Note: The divisor must fit in 16 bits and a tick must last a whole number of msecs */
global void
krn_timer_set_frequency(uint16_t hz)
{
    krn_lock_t lock;
    uint32_t div;

    ASSERT(hz >= 19 && hz <= 1000);

    (void)udiv32(&div, PIT_FREQUENCY, hz);

    lock = krn_lock();

    timer_msecs_per_tick = 1000 / hz;
    krn_timer_set_counter_0((uint16_t)div);

    krn_unlock(lock);
}

global void
krn_timer_init(void)
{
    system_info_st *si = &system_info;

    krn_debug_printf("Initializing timer... ");

    krn_timer_set_frequency(DEFAULT_TICK_FREQUENCY);

    krn_get_isr(0x08, &saved_isr_handler);
    krn_set_isr(0x08, si->main_segment, (uint16_t)(uint32_t)&krn_isr_timer);

    krn_debug_printf("ok\n");
}

global void
krn_timer_deinit(void)
{
    krn_timer_set_counter_0(0);
    krn_set_isr(0x08, saved_isr_handler.seg, saved_isr_handler.ofs);
}
