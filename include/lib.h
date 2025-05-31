// --------------------------------------------------------------------------------------
// Copyright (c) 2014-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: lib.h - Basic standard library
// --------------------------------------------------------------------------------------

#ifndef _LIB_H_
#define _LIB_H_

#include <stdarg.h>
#include <stdint.h>

#include <config.h>

#define NULL ((void *)0)

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define _unsd __attribute__((unused))

#define RETURN_IF_ALREADY_CALLED        \
    static int _already_called = 0;     \
    if (_already_called) {              \
        return;                         \
    }                                   \
    _already_called = 1;

typedef uint32_t size_t;
typedef int32_t ssize_t;

// lib/cpu.s
uint32_t cpu_get_eflags(void);
void cpu_set_eflags(uint32_t eflags);
void cpu_cli(void);
void cpu_hlt(void);
uint8_t inb(uint16_t port);
void outb(uint8_t value, uint16_t port);

#include "proto_lib.h"

#endif // _LIB_H_
