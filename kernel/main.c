/*
 * Copyright (c) 2014-2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: main.c - Kernel main function
 */

#include "lib.h"
#include <kernel.h>
#include <gui.h>

extern uint32_t krn_magic_number;

global isr_st far *krn_ivt = MK_FP(0, 0);

#if ENABLE_TESTS
extern void tests_run(void);
#endif

static void
krn_check_load(void)
{
    krn_debug_printf("Checking kernel load... ");

    if (krn_magic_number != 0xf0cacc1a) {
        krn_debug_printf("fail\n");
        halt();
        /* UNREACHABLE */
    }

    krn_debug_printf("ok\n");
}

global void
krn_main(void)
{
    bios_uart_init();
    krn_debug_printf("\n");
    krn_check_load();
    krn_mem_init();

#if ENABLE_TESTS
    tests_run();
#endif

    krn_heap_init();
    krn_keyboard_init();
    krn_timer_init();
    krn_rtc_init();

    krn_debug_printf("Starting GUI...\n");
    sleep(2000);

    krn_vga_init();
    gui_main();

    halt();
    /* UNREACHABLE */
}

global void
krn_exit(void)
{
    regs_st regs;
    uint16_t *psp = 0;

    /* Do nothing if not under DOS */
    if (*psp != 0x20cd) {
        return;
    }

    krn_vga_deinit();
    krn_timer_deinit();
    krn_keyboard_deinit();

    intr(0x20, &regs);
}

global void
krn_set_isr(uint8_t no, uint16_t seg, uint16_t ofs)
{
    uint16_t flags = cpu_get_flags();

    krn_ivt[no].seg = seg;
    krn_ivt[no].ofs = ofs;

    cpu_set_flags(flags);
}

global void
krn_get_isr(uint8_t no, isr_st *dst)
{
    dst->seg = krn_ivt[no].seg;
    dst->ofs = krn_ivt[no].ofs;
}
