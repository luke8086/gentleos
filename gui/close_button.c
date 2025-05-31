// --------------------------------------------------------------------------------------
// Copyright (c) 2025-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: close_button.c - Window close button widget
// --------------------------------------------------------------------------------------

#include <gui.h>

static void
gui_close_button_draw(widget_st *widget)
{
    window_st *win = widget->window;

    int is_pressed = (widget == widget->window->pressed_widget) || widget->active;
    int bg_col_normal = win->active ? COLOR_TITLE_BAR_ACTIVE : COLOR_TITLE_BAR_INACTIVE;
    int bg_col = is_pressed ? COLOR_BUTTON_PRESSED : bg_col_normal;
    int fg_col = is_pressed ? COLOR_WINDOW : COLOR_TEXT_ACTIVE;

    rect_st bar_rect = gui_rect_center((rect_st) {
        .width = TITLE_BAR_HEIGHT - 14,
        .height = 2,
    }, widget->rect);

    gui_surface_draw_border(widget->window->surface, widget->rect, COLOR_BORDER);
    gui_surface_draw_rect(win->surface, gui_rect_shrink(widget->rect, 1), bg_col);
    gui_surface_draw_rect(win->surface, bar_rect, fg_col);

    gui_wm_render_window_region(widget->window, widget->rect);
}

static void
gui_close_button_on_pointer_up(widget_st *widget, event_st event, point_st pos)
{
    gui_button_on_pointer_up(widget, event, pos);
    gui_wm_remove_window(widget->window);
}

void
gui_close_button_init(widget_st *button, window_st *window)
{
    button->type = WIDGET_TYPE_BUTTON,
    button->rect.x = window->rect.width - TITLE_BAR_HEIGHT;
    button->rect.y = 0;
    button->rect.width = TITLE_BAR_HEIGHT;
    button->rect.height = TITLE_BAR_HEIGHT;
    button->draw = gui_close_button_draw;
    button->on_pointer_up = gui_close_button_on_pointer_up;
}
