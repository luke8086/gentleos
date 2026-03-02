// --------------------------------------------------------------------------------------
// Copyright (c) 2014-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: string.c - String handling routines
// --------------------------------------------------------------------------------------

#include <lib.h>

void *
memcpy(void *dest, const void *src, size_t n)
{
    uint8_t *srcb = (uint8_t *)src;
    uint8_t *destb = (uint8_t *)dest;

    for (; n > 0 && ((uintptr_t)destb % 4) != 0; --n) {
        *(destb++) = *(srcb++);
    }

    uint32_t *srcq = (uint32_t *)srcb;
    uint32_t *destq = (uint32_t *)destb;

    for (; n >= sizeof(*destq); n -= sizeof(*destq)) {
        *(destq++) = *(srcq++);
    }

    srcb = (uint8_t *)srcq;
    destb = (uint8_t *)destq;
    for (; n > 0; --n) {
        *(destb++) = *(srcb++);
    }

    return dest;
}

void *
memset(void *dest, int c, size_t n)
{
    uint8_t *dest8 = (uint8_t *)dest;
    uint8_t c8 = (unsigned char)c;

    for (; n > 0 && ((uintptr_t)dest8 % 4) != 0; --n) {
        *(dest8++) = c8;
    }

    uint32_t *dest32 = (uint32_t *)dest8;
    uint32_t c32 = c8 | (c8 << 8) | (c8 << 16) | (c8 << 24);

    for (; n >= sizeof(*dest32); n -= sizeof(*dest32)) {
        *(dest32++) = c32;
    }

    dest8 = (uint8_t *)dest32;
    for (; n > 0; --n) {
        *(dest8++) = c8;
    }

    return dest;
}

int32_t
strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2)) {
        ++s1;
        ++s2;
    }

    return (*s1 - *s2);
}

size_t
strlen(const char *s1)
{
    size_t ret = 0;

    while (*s1++) {
        ++ret;
    }

    return ret;
}

char *
strncpy(char *dest, const char *src, size_t n)
{
    size_t i;

    for (i = 0; i < n && src[i] != '\0'; ++i) {
        dest[i] = src[i];
    }

    while (i < n) {
        dest[i++] = '\0';
    }

    return dest;
}
