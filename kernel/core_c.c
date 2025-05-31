// --------------------------------------------------------------------------------------
// Copyright (c) 2014-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: core_c.c - Main entry point in C
// --------------------------------------------------------------------------------------

#include <kernel.h>

enum {
    MBOOT_FLAGS = 0x07, // page align & mem info & video mode
    MBOOT_MAGIC = 0x1BADB002,
    MBOOT_CKSUM = -(MBOOT_MAGIC + MBOOT_FLAGS),
};

__attribute__((section(".multiboot"))) __attribute__((aligned(4)))
uint32_t krn_core_mboot_header[] = {
    MBOOT_MAGIC,
    MBOOT_FLAGS,
    MBOOT_CKSUM,
    0, 0, 0, 0, 0,
    0,
    GUI_WIDTH,
    GUI_HEIGHT,
    8,
};

mboot_info_st *krn_core_mboot_info;

void
krn_core_c_main(void)
{
    krn_main();

    while (1);
}

__attribute__((force_align_arg_pointer)) void
krn_core_c_isr_handle(isr_stack_st *isr_stack)
{
    krn_interrupt_handle(isr_stack);
}

