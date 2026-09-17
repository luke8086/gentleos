/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: initrd.c - Support for a simple initial RAM disk
 */

#include <kernel.h>

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t count;
} initrd_header_st;

#define INITRD_MAGIC 0x32445249UL /* "IRD2" */
#define INITRD_MAX_SIZE 0x10000L

enum {
    INITRD_VERSION = 2,
    INITRD_MAX_COUNT = (INITRD_MAX_SIZE - sizeof(initrd_header_st)) / sizeof(file_st),
};

static const char initrd_dos_path[] = "gentleos.dat";

global uint16_t krn_initrd_files_count;
global file_st far *krn_initrd_files;

static uint32_t
initrd_dos_load(void)
{
    static char buf[512];
    uint16_t far *psp_first_free_seg = MK_FP(krn_main_segment, 0x02);
    uint32_t total = 0;
    regs_st regs;
    uint16_t handle, n;

    if ((uint32_t)krn_initrd_segment + 0x1000 > *psp_first_free_seg) {
        krn_debug_printf("not enough memory\n");
        return 0;
    }

    regs.h.ah = 0x3d;
    regs.h.al = 0x00;
    regs.x.dx = (uint16_t)(uint32_t)initrd_dos_path;
    intr(0x21, &regs);

    if (regs.x.flags & 0x0001) {
        krn_debug_printf("not found\n");
        return 0;
    }

    handle = regs.x.ax;

    for (;;) {
        regs.h.ah = 0x3f;
        regs.x.bx = handle;
        regs.x.cx = sizeof(buf);
        regs.x.dx = (uint16_t)(uint32_t)buf;
        intr(0x21, &regs);

        if (regs.x.flags & 0x0001) {
            krn_debug_printf("read error\n");
            total = 0;
            break;
        }

        n = regs.x.ax;

        if (n == 0) {
            break;
        }

        if (total + n > INITRD_MAX_SIZE) {
            krn_debug_printf("too big\n");
            total = 0;
            break;
        }

        memcpy_far(MK_FP(krn_initrd_segment, (uint16_t)total), buf, n);
        total += n;
    }

    regs.h.ah = 0x3e;
    regs.x.bx = handle;
    intr(0x21, &regs);

    return total;
}

global void
krn_initrd_init(void)
{
    initrd_header_st far *header;
    file_st far *files;
    uint32_t image_size = INITRD_MAX_SIZE;
    uint16_t i, count, table_size;

    krn_debug_printf("Initializing initrd... ");

    header = MK_FP(krn_initrd_segment, 0);
    files = MK_FP(krn_initrd_segment, (uint16_t)sizeof(initrd_header_st));

    if (krn_is_dos()) {
        image_size = initrd_dos_load();
    }

    if (image_size == 0) {
        return;
    }

    if (image_size < sizeof(initrd_header_st) || header->magic != INITRD_MAGIC) {
        krn_debug_printf("not found\n");
        return;
    }

    if (header->version != INITRD_VERSION) {
        krn_debug_printf("incompatible version\n");
        return;
    }

    if (header->count > INITRD_MAX_COUNT) {
        krn_debug_printf("invalid format\n");
        return;
    }

    count = (uint16_t)header->count;
    table_size = count * (uint16_t)sizeof(file_st);

    if (sizeof(initrd_header_st) + table_size > image_size) {
        krn_debug_printf("invalid format\n");
        return;
    }

    for (i = 0; i < count; ++i) {
        if (files[i].u.offset > image_size || files[i].size > image_size - files[i].u.offset) {
            krn_debug_printf("invalid format\n");
            return;
        }

        files[i].u.addr = MK_FP(krn_initrd_segment, (uint16_t)files[i].u.offset);
        files[i].name[sizeof(files[i].name) - 1] = 0;
    }

    krn_debug_printf("found %u files\n", count);

    krn_initrd_files_count = count;
    krn_initrd_files = files;
}
