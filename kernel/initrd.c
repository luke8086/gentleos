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

global uint16_t krn_initrd_files_count;
global file_st far *krn_initrd_files;

global void
krn_initrd_init(void)
{
    initrd_header_st far *header;
    file_st far *files;
    uint16_t i, count;

    krn_debug_printf("Initializing initrd... ");

    header = MK_FP(krn_initrd_segment, 0);
    files = MK_FP(krn_initrd_segment, (uint16_t)sizeof(initrd_header_st));

    if (krn_is_dos()) {
        memset_far(header, 0, sizeof(*header));
    }

    if (header->magic != INITRD_MAGIC) {
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

    for (i = 0; i < count; ++i) {
        if (files[i].u.offset > INITRD_MAX_SIZE || files[i].size > INITRD_MAX_SIZE - files[i].u.offset) {
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
