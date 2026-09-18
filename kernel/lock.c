/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: lock.c - Support for protecting code from being interrupted
 */

#include <kernel.h>

/* Note: Locks may be nested but need to be released in reverse order */
global krn_lock_t
krn_lock(void)
{
    krn_lock_t ret;

    ret = krn_cpu_get_flags();
    krn_cpu_cli();

    return ret;
}

global void
krn_unlock(krn_lock_t lock)
{
    krn_cpu_set_flags(lock);
}
