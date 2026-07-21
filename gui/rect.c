/*
 * Copyright (c) 2025-2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: rect.c - Operations on rectangles
 */

#include <gui.h>

global const point_st GUI_POINT_ZERO = { 0, 0 };
global const rect_st GUI_RECT_SCREEN = { 0, 0, GUI_WIDTH, GUI_HEIGHT };

global void
gui_rect_copy(rect_st *dst, const rect_st *src)
{
    dst->x = src->x;
    dst->y = src->y;
    dst->width = src->width;
    dst->height = src->height;
}

global void
gui_point_copy(point_st *dst, const point_st *src)
{
    dst->x = src->x;
    dst->y = src->y;
}

global void
gui_size_copy(size_st *dst, const size_st *src)
{
    dst->width = src->width;
    dst->height = src->height;
}

global int
gui_rect_is_empty(const rect_st *r)
{
    return r->width <= 0 || r->height <= 0;
}

global uint16_t
gui_rect_area(const rect_st *r)
{
    return (uint16_t)r->width * (uint16_t)r->height;
}

global int
gui_rect_touches(const rect_st *a, const rect_st *b)
{
    return 1
        && (a->x <= b->x + b->width)
        && (b->x <= a->x + a->width)
        && (a->y <= b->y + b->height)
        && (b->y <= a->y + a->height);
}

global void
gui_rect_init(rect_st *out, int x, int y, int width, int height)
{
    out->x = x;
    out->y = y;
    out->width = width;
    out->height = height;
}

global void
gui_rect_translate(rect_st *r, const point_st *v)
{
    r->x += v->x;
    r->y += v->y;
}

global void
gui_rect_center(rect_st *r, const rect_st *container)
{
    r->x = container->x + (container->width - r->width) / 2;
    r->x = r->x < container->x ? container->x : r->x;

    r->y = container->y + (container->height - r->height) / 2;
    r->y = r->y < container->y ? container->y : r->y;
}

global void
gui_rect_shrink(rect_st *r, int amount)
{
    r->x += amount;
    r->y += amount;

    r->width -= amount * 2;
    r->width = MAX(r->width, 0);

    r->height -= amount * 2;
    r->height = MAX(r->height, 0);
}

global void
gui_rect_enclose(rect_st *a, const rect_st *b)
{
    int x2, y2;

    if (gui_rect_is_empty(a)) {
        gui_rect_copy(a, b);
        return;
    }

    if (gui_rect_is_empty(b)) {
        return;
    }

    x2 = MAX((a->x + a->width), (b->x + b->width));
    y2 = MAX((a->y + a->height), (b->y + b->height));

    a->x = MIN(a->x, b->x);
    a->y = MIN(a->y, b->y);
    a->width = x2 - a->x;
    a->height = y2 - a->y;
}

global void
gui_rect_clip(rect_st *r, const rect_st *clipper)
{
    if (r->x < clipper->x) {
        r->width -= clipper->x - r->x;
        r->width = r->width > 0 ? r->width : 0;
        r->x = clipper->x;
    }

    if (r->y < clipper->y) {
        r->height -= clipper->y - r->y;
        r->height = r->height > 0 ? r->height : 0;
        r->y = clipper->y;
    }

    if (r->x + r->width > clipper->x + clipper->width) {
        r->width = clipper->x + clipper->width - r->x;
    }

    if (r->y + r->height > clipper->y + clipper->height) {
        r->height = clipper->y + clipper->height - r->y;
    }

    if (r->width < 0 || r->height < 0) {
        r->width = 0;
        r->height = 0;
    }
}

global const char *
gui_rect_format(const rect_st *r)
{
    static char buf[100];

    snprintf(buf, sizeof(buf), "<x: %d, y: %d, w: %d, h: %d>",
        r->x, r->y, r->width, r->height);

    return buf;
}
