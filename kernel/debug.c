// --------------------------------------------------------------------------------------
// Copyright (c) 2014-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: debug.c - Debug routines
// --------------------------------------------------------------------------------------

#include <kernel.h>

void
krn_debug_printf(const char *fmt, ...)
{
    int count;
    static char buf[4096];

    va_list args;

    va_start(args, fmt);
    count = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    for (int i = 0; i < count; i++) {
        outb(buf[i], 0xe9);
    }
}

static void
krn_debug_dump_multiboot_mmap(uint64_t len, uint64_t paddr)
{
    mboot_mmap_entry_st *mmap_start, *mmap_end, *e;
    uint64_t addr1, addr2;

    mmap_start  = (mboot_mmap_entry_st*)paddr;
    mmap_end = (mboot_mmap_entry_st*)(paddr + len);

    krn_debug_printf("bios memory map:\n");

    for (e = mmap_start; e < mmap_end; ++e) {
        addr1 = e->addr;
        addr2 = e->addr + e->len - 1;
        krn_debug_printf(
            "  %01x - %08x%08x - %08x%08x\n",
            (uint32_t)e->type,
            (uint32_t)(addr1 >> 32), (uint32_t)addr1,
            (uint32_t)(addr2 >> 32), (uint32_t)addr2
        );
    }
}

void
krn_debug_dump_multiboot_info(void)
{
    mboot_info_st *m = krn_core_mboot_info;

    if (m->flags & 0x04) {
        krn_debug_printf("bootloader: %s\n", m->boot_loader_name);
    }

    if (m->flags & 0x800) {
        krn_debug_printf(
            "video: %08x %dx%dx%d\n",
            (uint32_t)m->fb_addr,
            m->fb_width, m->fb_height, m->fb_bpp
        );
    }

    if (m->flags & 0x40) {
        krn_debug_dump_multiboot_mmap(m->mmap_length, (uint64_t) m->mmap_addr);
    }
}

void
krn_debug_dump_kernel_location(void)
{
    uint32_t start = (uint32_t) &krn_link_start;
    uint32_t end = (uint32_t) &krn_link_end;
    uint32_t size = (end - start) >> 10;

    krn_debug_printf("kernel location: %08x - %08x (%dKB)\n", start, end, size);
}
