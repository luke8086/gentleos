// --------------------------------------------------------------------------------------
// Copyright (c) 2025-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: rect.c - Operations on rectangles
// --------------------------------------------------------------------------------------

#include <gui.h>

rect_st
gui_rect_make(int x, int y, int width, int height)
{
    return (rect_st) {
        .x = x,
        .y = y,
        .width = width,
        .height = height,
    };
}

rect_st
gui_rect_translate(rect_st r, point_st v)
{
    r.x += v.x;
    r.y += v.y;
    return r;
}

rect_st
gui_rect_translate_back(rect_st r, point_st v)
{
    r.x -= v.x;
    r.y -= v.y;
    return r;
}

int
gui_rect_contains_point(rect_st r, point_st p)
{
    return p.x >= r.x && p.x < r.x + r.width && p.y >= r.y && p.y < r.y + r.height;
}

rect_st
gui_rect_center(rect_st r, rect_st container)
{
    r.x = container.x + (container.width - r.width) / 2;
    r.x = r.x < container.x ? container.x : r.x;

    r.y = container.y + (container.height - r.height) / 2;
    r.y = r.y < container.y ? container.y : r.y;

    return r;
}

rect_st
gui_rect_limit(rect_st r, rect_st container)
{
    if (r.x < container.x) {
        r.x = container.x;
    }

    if (r.y < container.y) {
        r.y = container.y;
    }

    if (r.x + r.width > container.x + container.width) {
        r.x = container.x + container.width - r.width;
    }

    if (r.y + r.height > container.y + container.height) {
        r.y = container.y + container.height - r.height;
    }

    return r;
}

rect_st
gui_rect_shrink(rect_st r, int amount)
{
    r.x += amount;
    r.y += amount;

    r.width -= amount * 2;
    r.width = MAX(r.width, 0);

    r.height -= amount * 2;
    r.height = MAX(r.height, 0);

    return r;
}

rect_st
gui_rect_clip(rect_st r, rect_st clipper)
{
    if (r.x < clipper.x) {
        r.width -= clipper.x - r.x;
        r.width = r.width > 0 ? r.width : 0;
        r.x = clipper.x;
    }

    if (r.y < clipper.y) {
        r.height -= clipper.y - r.y;
        r.height = r.height > 0 ? r.height : 0;
        r.y = clipper.y;
    }

    if (r.x + r.width > clipper.x + clipper.width) {
        r.width = clipper.x + clipper.width - r.x;
    }

    if (r.y + r.height > clipper.y + clipper.height) {
        r.height = clipper.y + clipper.height - r.y;
    }

    if (r.width < 0 || r.height < 0) {
        r.width = 0;
        r.height = 0;
    }

    return r;
}

//
// Calculate the difference between two rectangles which
// only differ in position. The result is two rectangles,
// one for the horizontal difference and one for the vertical difference
//
// This function is used to select a part of the screen that needs
// to be redrawn after a window is moved
//
void
gui_rect_translate_diff(rect_st r1, rect_st r2, rect_st *hdiff, rect_st *vdiff)
{
    if (r1.x > r2.x) {
        hdiff->x = r2.x + r2.width;
        hdiff->width = r1.x - r2.x;
    } else {
        hdiff->x = r1.x;
        hdiff->width = r2.x - r1.x;
    }

    hdiff->y = MIN(r1.y, r2.y);
    hdiff->height = MAX(r1.y + r1.height, r2.y + r2.height) - hdiff->y;

    if (r1.y > r2.y) {
        vdiff->y = r2.y + r2.height;
        vdiff->height = r1.y - r2.y;
    } else {
        vdiff->y = r1.y;
        vdiff->height = r2.y - r1.y;
    }

    vdiff->x = MIN(r1.x, r2.x);
    vdiff->width = MAX(r1.x + r1.width, r2.x + r2.width) - vdiff->x;
}

const char *
gui_rect_format(rect_st r)
{
    static char buf[100];

    snprintf(buf, sizeof(buf), "<x: %d, y: %d, w: %d, h: %d>",
        r.pos.x, r.pos.y, r.size.width, r.size.height);

    return buf;
}
