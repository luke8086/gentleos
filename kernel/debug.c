/*
 * Copyright (c) 2014-2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: debug.c - Debug routines
 */

#include <kernel.h>

#define DEBUG_TO_UART 0

static char buf[128];

global int krn_debug_text_mode_enabled = 1;
global void (*krn_debug_status_cb)(const char *, ...) = (void (*)(const char *, ...))NULL;

static void
krn_debug_puts(const char *s)
{
#if DEBUG_TO_UART
    krn_bios_uart_puts(s);
#endif

    if (krn_debug_text_mode_enabled) {
        krn_bios_puts(s);
    }
}

global void
krn_debug_printf(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    (void)vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    krn_debug_puts(buf);
}

global void
krn_debug_fatal(unsigned beeps, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    (void)vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    krn_debug_puts(buf);
    krn_debug_puts("\n");

    if (krn_debug_status_cb) {
        krn_debug_status_cb("%s", buf);
    }

    krn_debug_beep_adv(300, 200, beeps);

    halt();
}

global void
krn_debug_assert(int expr, const char *file, unsigned line)
{
    if (expr) {
        return;
    }

    krn_debug_fatal(2, "Fatal: Assertion failed (%s:%u)", file, line);
}

global void
krn_debug_beep_adv(unsigned hz, unsigned msecs, unsigned count)
{
    static int beep_owner;
    unsigned i;

    if (!system_info.timer_available) {
        return;
    }

    for (i = 0; i < count; i++) {
        krn_speaker_play_freq(hz, &beep_owner);
        sleep(msecs);
        krn_speaker_stop(&beep_owner);
        sleep(msecs);
    }
}

global void
krn_debug_beep(void)
{
    krn_debug_beep_adv(300, 200, 1);
}
