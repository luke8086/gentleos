// --------------------------------------------------------------------------------------
// Copyright (c) 2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: drag.c - Routines for dragging windows
// --------------------------------------------------------------------------------------

#include <gui.h>

static window_st *drag_window = NULL;
static int drag_start_x;
static int drag_start_y;

void
gui_drag_start(window_st *window, event_st event)
{
    drag_start_x = event.pointer_x;
    drag_start_y = event.pointer_y;
    drag_window = window;
}

void
gui_drag_move(event_st event)
{
    if (!drag_window) {
        return;
    }

    rect_st saved_rect = drag_window->rect;
    rect_st hdiff, vdiff;

    point_st dpos = {
        .x = event.pointer_x - drag_start_x,
        .y = event.pointer_y - drag_start_y,
    };

    drag_window->rect = gui_rect_translate(drag_window->rect, dpos);
    drag_window->rect = gui_rect_limit(drag_window->rect, gui_wm_container);

    gui_rect_translate_diff(saved_rect, drag_window->rect, &hdiff, &vdiff);

    gui_wm_render_desktop_region(hdiff, NULL);
    gui_wm_render_desktop_region(vdiff, NULL);
    gui_wm_render_desktop_region(drag_window->rect, drag_window);

    drag_start_x = event.pointer_x;
    drag_start_y = event.pointer_y;
}

void
gui_drag_end(void)
{
    drag_window = NULL;
}
