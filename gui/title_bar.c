// --------------------------------------------------------------------------------------
// Copyright (c) 2025-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: title_bar.c - Window title bar
// --------------------------------------------------------------------------------------

#include <gui.h>

static void
gui_title_bar_on_pointer_down(widget_st *widget, event_st event, point_st pos _unsd)
{
    window_st *window = widget->window;

    window->drag_active = 1;
    window->drag_start.x = event.pointer_x;
    window->drag_start.y = event.pointer_y;
}

static void
gui_title_bar_on_pointer_move(widget_st *widget, event_st event, point_st pos _unsd)
{
    window_st *window = widget->window;

    if (!window->drag_active) {
        return;
    }

    rect_st saved_rect = window->rect;
    rect_st hdiff, vdiff;

    point_st dpos = {
        .x = event.pointer_x - window->drag_start.x,
        .y = event.pointer_y - window->drag_start.y,
    };

    window->rect = gui_rect_translate(window->rect, dpos);
    window->rect = gui_rect_limit(window->rect, gui_wm_container);

    gui_rect_translate_diff(saved_rect, window->rect, &hdiff, &vdiff);

    gui_wm_render_desktop_region(hdiff, NULL);
    gui_wm_render_desktop_region(vdiff, NULL);
    gui_wm_render_desktop_region(window->rect, window);

    window->drag_start.x = event.pointer_x;
    window->drag_start.y = event.pointer_y;
}

static void
gui_title_bar_on_pointer_up(widget_st *widget, event_st event _unsd, point_st pos _unsd)
{
    window_st *window = widget->window;

    if (window->drag_active) {
        window->drag_active = 0;
    }
}

static void
gui_title_bar_draw(widget_st *widget)
{
    char title[64];
    snprintf(title, sizeof(title), "   %s", widget->window->title);

    window_st *win = widget->window;
    int bg_color = win->active ? COLOR_TITLE_BAR_ACTIVE : COLOR_TITLE_BAR_INACTIVE;

    gui_surface_draw_border(win->surface, widget->rect, COLOR_BORDER);
    gui_surface_draw_rect(win->surface, gui_rect_shrink(widget->rect, 1), bg_color);
    gui_surface_draw_str_centered(win->surface, widget->rect,
        font_8x16, title, COLOR_TEXT_ACTIVE, bg_color);

    gui_wm_render_window_region(widget->window, widget->rect);
}

void
gui_title_bar_init(widget_st *bar, window_st *window)
{
    bar->rect = (rect_st) {
        .x = 0,
        .y = 0,
        .width = window->surface->size.width - TITLE_BAR_HEIGHT + 1,
        .height = TITLE_BAR_HEIGHT,
    };

    bar->press_sticky = 1;

    bar->draw = gui_title_bar_draw;
    bar->on_pointer_down = gui_title_bar_on_pointer_down;
    bar->on_pointer_move = gui_title_bar_on_pointer_move;
    bar->on_pointer_up = gui_title_bar_on_pointer_up;
}
