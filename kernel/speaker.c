// --------------------------------------------------------------------------------------
// Copyright (c) 2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: speaker.c - Driver for PC speaker
// --------------------------------------------------------------------------------------

#include <kernel.h>

enum {
    PIT_CR2      = 0x42, // PIT counter 2 data port
    PIT_CWR      = 0x43, // PIT control word register
    PPI_PB       = 0x61, // Port B of 8255A-5 PPI
};

void
krn_speaker_stop(void)
{
    uint8_t val = inb(PPI_PB);
    outb(val & ~0x03, PPI_PB);
 }

void
krn_speaker_play(unsigned hz)
{
    // If hz is 0, turn off the speaker
    if (hz == 0) {
        krn_speaker_stop();
        return;
    }

    // Configure counter 2 of PIT to mode 3 (square wave)
    outb(0xB6, PIT_CWR);

    // Set counter 2 to the desired frequency
    uint32_t divisor = 1193180 / hz;
    outb(divisor & 0xFF, PIT_CR2);
    outb((divisor >> 8) & 0xFF, PIT_CR2);

    // Enable speaker by setting bits 0 (speaker enable) and 1 (gate) on port 0x61
    uint8_t val = inb(PPI_PB);
    outb(val | 0x03, PPI_PB);
}
