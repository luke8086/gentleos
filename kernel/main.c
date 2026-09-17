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

global system_info_st system_info;

global isr_st far *krn_ivt = MK_FP(0, 0);

static void
krn_check_load(void)
{
    krn_debug_printf("Checking kernel load... ");

    if (krn_magic_number != 0xf0cacc1a) {
        krn_debug_printf("fail\n");
        halt();
        /* UNREACHABLE */
    }

    krn_debug_printf("ok (flags: %02x)\n", krn_flags);
}

global void
krn_main(void)
{
    krn_bios_uart_init();
    krn_debug_printf("\n");
    krn_check_load();
    krn_mem_init();
    krn_initrd_init();
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

global int
krn_is_dos(void)
{
    uint16_t *psp = 0;

    return *psp == 0x20cd;
}

global void
krn_exit(void)
{
    regs_st regs;

    if (!krn_is_dos()) {
        return;
    }

    krn_vga_deinit();
    krn_speaker_deinit();
    krn_timer_deinit();
    krn_keyboard_deinit();

    krn_intr(0x20, &regs);
}

global void
krn_set_isr(uint8_t no, uint16_t seg, uint16_t ofs)
{
    krn_lock_t lock = krn_lock();

    krn_ivt[no].seg = seg;
    krn_ivt[no].ofs = ofs;

    krn_unlock(lock);
}

global void
krn_get_isr(uint8_t no, isr_st *dst)
{
    dst->seg = krn_ivt[no].seg;
    dst->ofs = krn_ivt[no].ofs;
}
