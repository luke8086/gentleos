// --------------------------------------------------------------------------------------
// Copyright (c) 2025-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: gui.c - GUI main function
// --------------------------------------------------------------------------------------

#include <gui.h>

void
gui_main(void)
{
    event_st event;
    window_st *pressed_window = NULL;

    gui_fb_init();
    gui_pointer_init();
    gui_wm_init();

    while (1) {
        if (krn_event_count() == 0) {
            continue;
        }

        if (krn_event_pop(&event) != 0) {
            continue;
        }

        if (event.type == EVENT_TIMER_TICK) {
            gui_timeout_on_tick(event);
            continue;
        }

        if (event.type == EVENT_POINTER_DOWN) {
            gui_pointer_move(event.pointer_x, event.pointer_y);

            window_st *w = gui_wm_find_window(event.pointer_x, event.pointer_y);

            if (w) {
                pressed_window = w;
                gui_wm_raise_window(w);
                gui_window_on_pointer_down(w, event);
            }

            continue;
        }

        if (event.type == EVENT_POINTER_MOVE) {
            gui_pointer_move(event.pointer_x, event.pointer_y);

            if (pressed_window) {
                gui_window_on_pointer_move(pressed_window, event);
            }

            continue;
        }

        if (event.type == EVENT_POINTER_UP) {
            gui_pointer_move(event.pointer_x, event.pointer_y);

            if (pressed_window) {
                gui_window_on_pointer_up(pressed_window, event);
            }

            pressed_window = NULL;

            continue;
        }

        if (event.type == EVENT_POINTER_ALT) {
            gui_pointer_move(event.pointer_x, event.pointer_y);

            window_st *w = gui_wm_find_window(event.pointer_x, event.pointer_y);

            if (w && !pressed_window) {
                gui_window_on_pointer_alt(w, event);
            }

            continue;
        }

        if (event.type == EVENT_KEY_DOWN) {
            window_st *w = gui_wm_top_window();

            if (w && w->on_key_down) {
                w->on_key_down(w, event);
            }

            continue;
        }

        if (event.type == EVENT_KEY_UP) {
            window_st *w = gui_wm_top_window();

            if (w && w->on_key_up) {
                w->on_key_up(w, event);
            }

            continue;
        }
    }
}
