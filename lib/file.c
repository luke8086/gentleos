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
    system_info_st *si = &system_info;

    return si->initrd_files_count;
}

global file_st far *
file_get(uint16_t index)
{
    system_info_st *si = &system_info;

    if (index >= si->initrd_files_count) {
        return NULL;
    }

    return &si->initrd_files[index];
}
