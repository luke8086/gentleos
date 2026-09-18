/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: speaker.c - Driver for PC speaker
 */

#include <lib.h>
#include <kernel.h>

enum {
    PIT_CR2      = 0x42, /* PIT counter 2 data port */
    PIT_CWR      = 0x43, /* PIT control word register */
    PPI_PB       = 0x61, /* Port B of 8255A-5 PPI */
};

global void
krn_speaker_stop(void)
{
    uint8_t val = krn_inb(PPI_PB);
    krn_outb(val & ~0x03, PPI_PB);
 }

global void
krn_speaker_play(uint16_t hz)
{
    uint32_t divisor;
    uint8_t val;

    /* If hz is 0, turn off the speaker */
    if (hz == 0) {
        krn_speaker_stop();
        return;
    }

    /* Configure counter 2 of PIT to mode 3 (square wave) */
    krn_outb(0xB6, PIT_CWR);

    /* Set counter 2 to the desired frequency */
    (void)udiv32(&divisor, PIT_FREQUENCY, hz);
    krn_outb((uint16_t)divisor & 0xFF, PIT_CR2);
    krn_outb(((uint16_t)divisor >> 8) & 0xFF, PIT_CR2);

    /* Enable speaker by setting bits 0 (speaker enable) and 1 (gate) on port 0x61 */
    val = krn_inb(PPI_PB);
    krn_outb(val | 0x03, PPI_PB);
}
