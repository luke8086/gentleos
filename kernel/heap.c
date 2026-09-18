/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: heap.c - Heap allocator
 */

#include <kernel.h>

#define KRN_HEAP_SIZE 0x10000UL

global void far *
krn_heap_alloc(uint16_t size)
{
    static uint32_t krn_heap_current_ofs = 0;

    system_info_st *si = &system_info;
    uint32_t remaining_space = KRN_HEAP_SIZE - krn_heap_current_ofs;
    void far *ret;

    if (size < 0xFFFF) {
        size = (size + 1) & ~1;
    }

    if (size > remaining_space) {
        krn_debug_printf("FATAL: Out of memory\n");
        krn_debug_beep_adv(300, 500, 3);

        halt();
        /* UNREACHABLE */
    }

    ret = MK_FP(si->heap_segment, (uint16_t)krn_heap_current_ofs);
    krn_heap_current_ofs += size;

    memset_far(ret, 0, size);

    return ret;
}
