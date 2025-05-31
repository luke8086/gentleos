// --------------------------------------------------------------------------------------
// Copyright (c) 2025-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: button.c - Button widget
// --------------------------------------------------------------------------------------

#include <gui.h>

void
gui_button_on_pointer_down(widget_st *widget, event_st event _unsd, point_st pos _unsd)
{
    gui_widget_draw(widget);
}

void
gui_button_on_pointer_up(widget_st *widget, event_st event _unsd, point_st pos _unsd)
{
    gui_widget_draw(widget);
}

void
gui_button_on_pointer_out(widget_st *widget, event_st event _unsd, point_st pos _unsd)
{
    widget->window->pressed_widget = NULL;
    gui_widget_draw(widget);
}

void
gui_button_draw(widget_st *widget)
{
    rect_st rect = widget->rect;

    int is_pressed = (widget == widget->window->pressed_widget) || widget->active;

    if (!widget->hide_border) {
        gui_surface_draw_border(widget->window->surface, rect, COLOR_BORDER);
        rect = gui_rect_shrink(rect, 1);
    }

    gui_surface_draw_rect(
        widget->window->surface,
        rect,
        is_pressed ? COLOR_BUTTON_PRESSED : COLOR_WINDOW
    );

    if (widget->bitmap_regular && widget->bitmap_pressed) {
        gui_surface_draw_bitmap_centered(
            widget->window->surface,
            rect,
            is_pressed ? widget->bitmap_pressed : widget->bitmap_regular
        );
    } else if (widget->label) {
        gui_surface_draw_str_centered(
            widget->window->surface,
            rect,
            widget->font ? widget->font : font_8x16,
            widget->label,
            is_pressed ? COLOR_WINDOW : COLOR_TEXT_ACTIVE,
            is_pressed ? COLOR_BUTTON_PRESSED : COLOR_WINDOW
        );
    }

    gui_wm_render_window_region(widget->window, rect);
}
