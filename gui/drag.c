// --------------------------------------------------------------------------------------
// Copyright (c) 2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: drag.c - Routines for dragging windows
// --------------------------------------------------------------------------------------

#include <gui.h>

static window_st *drag_window = NULL;

static rect_st drag_outline_rect;
static int drag_outline_drawn = 0;

static rect_st drag_origin_rect;
static int drag_origin_x;
static int drag_origin_y;

static int drag_current_x;
static int drag_current_y;

static rect_st
drag_target_rect(void)
{
    point_st dpos = {
        .x = drag_current_x - drag_origin_x,
        .y = drag_current_y - drag_origin_y,
    };

    rect_st r = gui_rect_translate(drag_origin_rect, dpos);
    return gui_rect_limit(r, gui_wm_container);
}

void
gui_drag_start(window_st *window, event_st event)
{
    drag_origin_x = drag_current_x = event.pointer_x;
    drag_origin_y = drag_current_y = event.pointer_y;
    drag_origin_rect = window->rect;
    drag_window = window;
    drag_outline_drawn = 0;
}

void
gui_drag_move(event_st event)
{
    if (!drag_window) {
        return;
    }

    drag_current_x = event.pointer_x;
    drag_current_y = event.pointer_y;
}

void
gui_drag_end(void)
{
    if (!drag_window) {
        return;
    }

    rect_st final_rect = drag_target_rect();
    drag_window->rect = final_rect;
    drag_window = NULL;

    gui_wm_render_desktop_region(drag_origin_rect, NULL);
    gui_wm_render_desktop_region(final_rect, NULL);
}

void
gui_drag_draw_outline(void)
{
    if (!drag_window) {
        return;
    }

    drag_outline_rect = drag_target_rect();
    gui_fb_draw_outline(drag_outline_rect);
    drag_outline_drawn = 1;
}

void
gui_drag_clear_outline(void)
{
    if (!drag_outline_drawn) {
        return;
    }

#if GUI_PLANAR_MODE
    gui_planar_xor_corners(drag_outline_rect);
#else
    gui_fb_mark_dirty(drag_outline_rect);
#endif

    drag_outline_drawn = 0;
}
