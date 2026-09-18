/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: mem.c - Utilities for kernel memory
 */

#include <kernel.h>

extern uint8_t krn_marker_text_start;
extern uint8_t krn_marker_text_end;
extern uint8_t krn_marker_data_start;
extern uint8_t krn_marker_data_end;
extern uint8_t krn_marker_bss_start;
extern uint8_t krn_marker_bss_end;

global uint16_t krn_heap_segment;
global uint16_t krn_initrd_segment;

static void
krn_mem_dump_layout(void)
{
    uint16_t text_start = (uint16_t)(uint32_t)&krn_marker_text_start;
    uint16_t text_end = (uint16_t)(uint32_t)&krn_marker_text_end;
    uint16_t data_start = (uint16_t)(uint32_t)&krn_marker_data_start;
    uint16_t data_end = (uint16_t)(uint32_t)&krn_marker_data_end;
    uint16_t bss_start = (uint16_t)(uint32_t)&krn_marker_bss_start;
    uint16_t bss_end = (uint16_t)(uint32_t)&krn_marker_bss_end;
    uint16_t data_size = data_end - data_start;
    uint16_t text_size = text_end - text_start;
    uint16_t bss_size = bss_end - bss_start;

    krn_debug_printf("Main segment: %04x\n", krn_main_segment);
    krn_debug_printf("- Code:  %04x-%04x (%u B)\n", text_start, text_end, text_size);
    krn_debug_printf("- Data:  %04x-%04x (%u B)\n", data_start, data_end, data_size);
    krn_debug_printf("- BSS:   %04x-%04x (%u B)\n", bss_start, bss_end, bss_size);
    krn_debug_printf("- Total: %u B\n", bss_end - text_start);
    krn_debug_printf("Heap segment: %04x\n", krn_heap_segment);
    krn_debug_printf("Initrd segment: %04x\n", krn_initrd_segment);
}

global void
krn_mem_init(void)
{
    memset(&krn_marker_bss_start, 0, &krn_marker_bss_end - &krn_marker_bss_start);

    krn_heap_segment = (krn_main_segment + 0x1000 + 0xfff) & 0xf000;
    krn_initrd_segment = krn_heap_segment + 0x1000;

    krn_mem_dump_layout();
}
