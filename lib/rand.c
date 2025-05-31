// --------------------------------------------------------------------------------------
// Copyright (c) 2014-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: rand.c - Random number generator
// --------------------------------------------------------------------------------------

#include <kernel.h>

static uint32_t rand_seed = 1;

void
rand_init(void)
{
	time_st t;

	krn_rtc_get_time(&t);

    rand_seed = (t.second << 24) | (t.minute << 16) | (t.hour << 8) | t.day;
}

uint32_t
rand(void)
{
    // See https://en.wikipedia.org/wiki/Xorshift
	uint32_t x = rand_seed;

	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;

	rand_seed = x;

	return x;
}
