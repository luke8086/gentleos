/*
 * Copyright (c) 2025-2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: gui.c - GUI main function
 */

#include <gui.h>

global int gui_colors_inverted;
global uint8_t gui_color_bg;
global uint8_t gui_color_fg;

global void
gui_set_colors_inverted(int inverted)
{
    gui_color_bg = inverted ? 0x0f : 0x00;
    gui_color_fg = inverted ? 0x00 : 0x0f;

    if (inverted == gui_colors_inverted) {
        return;
    }

    gui_colors_inverted = inverted;
    gui_surface_invert();
}

global void
gui_main(void)
{
    event_st event;

    gui_surface_init();
    gui_colors_inverted = (krn_flags & KRN_FLAG_COLORS_INVERTED) != 0;
    gui_set_colors_inverted(gui_colors_inverted);
    gui_surface_clear();
    gui_status_init();
    gui_app_launch(&app_launcher);
    gui_surface_flush();

    krn_debug_status_cb = gui_status_set_urgent;

    while (1) {
        krn_event_wait(&event);

        gui_app_handle_event(&event);

        if (krn_event_count() == 0) {
            gui_surface_flush();
        }
    }
}
