/*
 * Copyright (c) 2014-2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: string.c - String handling routines
 */

#include <lib.h>

global void *
memcpy(void *dest, const void *src, size_t n)
{
    uint8_t *srcb = (uint8_t *)src;
    uint8_t *destb = (uint8_t *)dest;
    uint16_t *srcw, *destw;

    for (; n > 0 && ((uintptr_t)destb % 2) != 0; --n) {
        *(destb++) = *(srcb++);
    }

    srcw = (uint16_t *)srcb;
    destw = (uint16_t *)destb;

    for (; n >= sizeof(*destw); n -= sizeof(*destw)) {
        *(destw++) = *(srcw++);
    }

    srcb = (uint8_t *)srcw;
    destb = (uint8_t *)destw;
    for (; n > 0; --n) {
        *(destb++) = *(srcb++);
    }

    return dest;
}

global void far *
memcpy_far(void far *dest, const void far *src, size_t n)
{
    uint8_t far *srcb = (uint8_t far *)src;
    uint8_t far *destb = (uint8_t far *)dest;
    uint16_t far *srcw;
    uint16_t far *destw;

    for (; n > 0 && ((uintptr_t)destb % 2) != 0; --n) {
        *(destb++) = *(srcb++);
    }

    srcw = (uint16_t far *)srcb;
    destw = (uint16_t far *)destb;

    for (; n >= sizeof(*destw); n -= sizeof(*destw)) {
        *(destw++) = *(srcw++);
    }

    srcb = (uint8_t far *)srcw;
    destb = (uint8_t far *)destw;
    for (; n > 0; --n) {
        *(destb++) = *(srcb++);
    }

    return dest;
}

global void *
memset(void *dest, int c, size_t n)
{
    uint8_t *dest8 = (uint8_t *)dest;
    uint8_t c8 = (unsigned char)c;
    uint16_t *dest16, c16;

    for (; n > 0 && ((uintptr_t)dest8 % 2) != 0; --n) {
        *(dest8++) = c8;
    }

    dest16 = (uint16_t *)dest8;
    c16 = c8 | ((uint16_t)c8 << 8);

    for (; n >= sizeof(*dest16); n -= sizeof(*dest16)) {
        *(dest16++) = c16;
    }

    dest8 = (uint8_t *)dest16;
    for (; n > 0; --n) {
        *(dest8++) = c8;
    }

    return dest;
}

global void far *
memset_far(void far *dest, int c, size_t n)
{
    uint8_t far *dest8 = (uint8_t far *)dest;
    uint8_t c8 = (unsigned char)c;
    uint16_t far *dest16;
    uint16_t c16;

    for (; n > 0 && ((uintptr_t)dest8 % 2) != 0; --n) {
        *(dest8++) = c8;
    }

    dest16 = (uint16_t far *)dest8;
    c16 = c8 | ((uint16_t)c8 << 8);

    for (; n >= sizeof(*dest16); n -= sizeof(*dest16)) {
        *(dest16++) = c16;
    }

    dest8 = (uint8_t far *)dest16;
    for (; n > 0; --n) {
        *(dest8++) = c8;
    }

    return dest;
}

global int32_t
strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2)) {
        ++s1;
        ++s2;
    }

    return (*s1 - *s2);
}

global int32_t
strcmp_far(const char far *s1, const char far *s2)
{
    while (*s1 && (*s1 == *s2)) {
        ++s1;
        ++s2;
    }

    return (*s1 - *s2);
}

global size_t
strlen(const char *s1)
{
    size_t ret = 0;

    while (*s1++) {
        ++ret;
    }

    return ret;
}

global char *
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
