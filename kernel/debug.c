/*
 * Copyright (c) 2014-2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: debug.c - Debug routines
 */

#include <kernel.h>

static char buf[128];

global int krn_debug_text_mode_enabled = 1;
global void (*krn_debug_status_cb)(const char *, ...) = (void (*)(const char *, ...))NULL;

global void
krn_debug_printf(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    (void)vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

#if DEBUG_TO_UART
    krn_bios_uart_puts(buf);
#endif

    if (krn_debug_text_mode_enabled) {
        krn_bios_puts(buf);
    }
}

global void
krn_debug_assert(int expr, const char *file, unsigned line)
{
    if (expr) {
        return;
    }

    krn_debug_printf("Fatal: Assertion failed (%s:%u)\n", file, line);

    if (krn_debug_status_cb) {
        krn_debug_status_cb("Assert failed (%s:%u)", file, line);
    }

    halt();
}

global void
krn_debug_beep_adv(unsigned hz, unsigned msecs, unsigned count)
{
    unsigned i;

    for (i = 0; i < count; i++) {
        krn_speaker_play(hz);
        sleep(msecs);
        krn_speaker_stop();
        sleep(msecs);
    }
}

global void
krn_debug_beep(void)
{
    krn_debug_beep_adv(300, 200, 1);
}
