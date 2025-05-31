// --------------------------------------------------------------------------------------
// Copyright (c) 2014-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: timer.c - Driver for PIT 8254
// --------------------------------------------------------------------------------------

#include <kernel.h>

enum {
    PIT_CR0 = 0x40,
    PIT_CWR = 0x43,
};

volatile static uint32_t timer_msecs = 0;

static void
krn_timer_handle_intr(isr_stack_st *isr_stack __attribute__((unused)))
{
    timer_msecs += 50;

    event_st event = {
        .type = EVENT_TIMER_TICK,
        .timer_msecs = timer_msecs,
    };

    (void)krn_event_ipush(event);
}

void
krn_timer_init(void)
{
    uint8_t hz = 20;
    uint32_t div = 1193180 / hz;

    // Set Counter 0, write both LSB and MSB, use mode 3, binary counter
    outb(0x36, PIT_CWR);

    // Write LSB and MSB for counter 0
    outb((uint8_t)((div >> 0) & 0xFF), PIT_CR0);
    outb((uint8_t)((div >> 8) & 0xFF), PIT_CR0);

    krn_interrupt_set_handler(0x20, krn_timer_handle_intr);
}
