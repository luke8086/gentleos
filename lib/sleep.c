// --------------------------------------------------------------------------------------
// Copyright (c) 2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: sleep.c - Functions for suspending execution
// --------------------------------------------------------------------------------------

#include <kernel.h>
#include <lib.h>

void
sleep(uint32_t msecs)
{
    uint32_t start = krn_timer_get_msecs();

    while (krn_timer_get_msecs() - start < msecs) {
        cpu_hlt();
    }
}
