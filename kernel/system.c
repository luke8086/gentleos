// --------------------------------------------------------------------------------------
// Copyright (c) 2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: system.c - System information
// --------------------------------------------------------------------------------------

#include <kernel.h>

const char *
krn_system_get_cpu_vendor(void)
{
    static char buf[13];

    if (!cpu_has_cpuid()) {
        return "Unknown";
    }

    uint32_t ebx, ecx, edx;
    cpu_cpuid(0, &ebx, &ecx, &edx);
    *(uint32_t *)(buf + 0) = ebx;
    *(uint32_t *)(buf + 4) = edx;
    *(uint32_t *)(buf + 8) = ecx;
    buf[12] = '\0';

    return buf;
}

uint32_t
krn_system_get_total_mem(void)
{
    mboot_info_st *m = krn_core_mboot_info;
    uint32_t total = 0;

    if (!(m->flags & 0x40)) {
        return 0;
    }

    mboot_mmap_entry_st *start = m->mmap_addr;
    mboot_mmap_entry_st *end = (mboot_mmap_entry_st *)((uint32_t)start + m->mmap_length);

    for (mboot_mmap_entry_st *e = start; e < end; ++e) {
        if (e->type == 1) {
            total += (uint32_t)(e->len);
        }
    }

    return total;
}

uint32_t
krn_system_get_used_mem(void)
{
    return (uint32_t)&krn_link_end - (uint32_t)&krn_link_start;
}

uint32_t
krn_system_get_avail_mem(void)
{
    mboot_info_st *m = krn_core_mboot_info;
    uint32_t kernel_addr = (uint32_t)&krn_link_start;
    uint32_t kernel_size = krn_system_get_used_mem();

    if (!(m->flags & 0x40)) {
        return 0;
    }

    mboot_mmap_entry_st *start = m->mmap_addr;
    mboot_mmap_entry_st *end = (mboot_mmap_entry_st *)((uint32_t)start + m->mmap_length);

    for (mboot_mmap_entry_st *e = start; e < end; ++e) {
        if (e->type == 1 && kernel_addr >= e->addr &&
            kernel_addr < e->addr + e->len) {
            return (uint32_t)(e->len) - kernel_size;
        }
    }

    return 0;
}
