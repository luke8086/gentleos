/*
 * Copyright (c) 2014-2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: rand.c - Random number generator
 */

#include <kernel.h>

static uint16_t rand_seed = 1;

global void
rand_add_entropy(uint16_t seed)
{
    rand_seed ^= seed;

    if (!rand_seed) {
        ++rand_seed;
    }
}

global uint16_t
rand(void)
{
    /* See https://en.wikipedia.org/wiki/Xorshift */
    uint16_t x = rand_seed;

    x ^= x << 7;
    x ^= x >> 9;
    x ^= x << 8;

    rand_seed = x;

    return x;
}
