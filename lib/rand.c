/*
 * Copyright (c) 2014-2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: rand.c - Random number generator
 */

#include <kernel.h>

static uint16_t rand_seed = 1;

global void
rand_init(void)
{
	time_st t;

	time_get(&t);

    rand_seed = ((uint16_t)t.second << 8) | t.minute;
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
