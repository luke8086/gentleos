/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: file.c - Operations on files
 */

#include <kernel.h>

global uint16_t
file_count(void)
{
    return krn_initrd_files_count;
}

global file_st far *
file_get(uint16_t index)
{
    if (index >= krn_initrd_files_count) {
        return NULL;
    }

    return &krn_initrd_files[index];
}
