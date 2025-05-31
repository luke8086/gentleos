// --------------------------------------------------------------------------------------
// Copyright (c) 2025-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: window.c - Window routines
// --------------------------------------------------------------------------------------

#include <gui.h>

rect_st
gui_window_area(window_st *window)
{
    return (rect_st) {
        .x = 0,
        .y = 0,
        .width = window->rect.width,
        .height = window->rect.height,
    };
}

void
gui_window_init_frame(window_st *window, widget_st *title_bar, widget_st *close_button)
{
    window->rect = (rect_st) {
        .x = 0,
        .y = 0,
        .width = window->surface->size.width,
        .height = window->surface->size.height,
    };
    window->rect = gui_rect_center(window->rect, gui_wm_container);

    gui_surface_draw_border(window->surface, gui_window_area(window), COLOR_BORDER);

    rect_st content_area = {
        .x = 1,
        .y = TITLE_BAR_HEIGHT,
        .width = window->surface->size.width - 2,
        .height = window->surface->size.height - TITLE_BAR_HEIGHT - 1,
    };
    gui_surface_draw_rect(window->surface, content_area, window->bg_color);

    gui_title_bar_init(title_bar, window);
    gui_window_add_widget(window, title_bar);

    gui_close_button_init(close_button, window);
    gui_window_add_widget(window, close_button);
}

int
gui_window_add_widget(window_st *window, widget_st *widget)
{
    if (window->widgets_count >= window->widgets_capacity) {
        return -1;
    }

    widget->window = window;
    window->widgets[window->widgets_count++] = widget;

    gui_widget_draw(widget);

    return 0;
}

widget_st *
gui_window_find_widget_at(window_st *window, point_st pos)
{
    for (size_t i = 0; i < window->widgets_count; i++) {
        if (gui_rect_contains_point(window->widgets[i]->rect, pos)) {
            return window->widgets[i];
        }
    }

    return NULL;
}

void
gui_window_on_pointer_out(window_st *window, event_st event, point_st pos)
{
    widget_st *pressed_widget = window->pressed_widget;

    if (pressed_widget) {
        window->pressed_widget = NULL;

        if (pressed_widget->on_pointer_out) {
            pressed_widget->on_pointer_out(pressed_widget, event, pos);
        } else if (pressed_widget->type == WIDGET_TYPE_BUTTON) {
            gui_button_on_pointer_out(pressed_widget, event, pos);
        }
    }
}

void
gui_window_on_pointer_down(window_st *window, event_st event)
{
    point_st pos = {
        .x = event.pointer_x - window->rect.x,
        .y = event.pointer_y - window->rect.y,
    };

    // Just in case, deactivate previously pressed widget
    gui_window_on_pointer_out(window, event, pos);

    widget_st *pointed_widget = gui_window_find_widget_at(window, pos);

    if (pointed_widget) {
        window->pressed_widget = pointed_widget;

        if (pointed_widget->on_pointer_down) {
            pointed_widget->on_pointer_down(pointed_widget, event, pos);
        } else if (pointed_widget->type == WIDGET_TYPE_BUTTON) {
            gui_button_on_pointer_down(pointed_widget, event, pos);
        }
    }
}

void
gui_window_on_pointer_move(window_st *window, event_st event)
{
    point_st pos = {
        .x = event.pointer_x - window->rect.x,
        .y = event.pointer_y - window->rect.y,
    };

    widget_st *pressed_widget = window->pressed_widget;
    widget_st *pointed_widget = gui_window_find_widget_at(window, pos);

    if (!pressed_widget && !pointed_widget) {
        return;
    }

    if (pressed_widget &&
        ((pressed_widget == pointed_widget) || pressed_widget->press_sticky)) {

        if (pressed_widget->on_pointer_move) {
            pressed_widget->on_pointer_move(pressed_widget, event, pos);
        }

        return;
    }

    // If a non-sticky widget is pressed, deactivate it
    gui_window_on_pointer_out(window, event, pos);

    if (pointed_widget && pointed_widget->press_on_move_in) {
        window->pressed_widget = pointed_widget;

        if (pointed_widget->on_pointer_down) {
            pointed_widget->on_pointer_down(pointed_widget, event, pos);
        } else if (pointed_widget->type == WIDGET_TYPE_BUTTON) {
            gui_button_on_pointer_down(pointed_widget, event, pos);
        }
    }
}

void
gui_window_on_pointer_up(window_st *window, event_st event)
{
    point_st pos = {
        .x = event.pointer_x - window->rect.x,
        .y = event.pointer_y - window->rect.y,
    };

    widget_st *pressed_widget = window->pressed_widget;

    // If widget was pressed, deactivate it and handle pointer up
    if (pressed_widget) {
        window->pressed_widget = NULL;

        if (pressed_widget->on_pointer_up) {
            pressed_widget->on_pointer_up(pressed_widget, event, pos);
        } else if (pressed_widget->type == WIDGET_TYPE_BUTTON) {
            gui_button_on_pointer_up(pressed_widget, event, pos);
        }
    }
}

void
gui_window_on_pointer_alt(window_st *window, event_st event)
{
    point_st pos = {
        .x = event.pointer_x - window->rect.x,
        .y = event.pointer_y - window->rect.y,
    };

    if (window->pressed_widget) {
        return;
    }

    widget_st *pointed_widget = gui_window_find_widget_at(window, pos);

    if (pointed_widget) {
        if (pointed_widget->on_pointer_alt) {
            pointed_widget->on_pointer_alt(pointed_widget, event, pos);
        }
    }
}

void
gui_window_on_active_change(window_st *window)
{
    for (size_t i = 0; i < window->widgets_count; i++) {
        if (window->widgets[i]->rect.y == 0) {
            gui_widget_draw(window->widgets[i]);
        }
    }

    if (window->on_active_change) {
        window->on_active_change(window);
    }
}
