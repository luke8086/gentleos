/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: app.c - Support for running apps
 */

#include <gui.h>

global rect_st gui_app_rect = {
    0,
    STATUS_HEIGHT,
    GUI_WIDTH,
    GUI_HEIGHT - STATUS_HEIGHT * 2,
};

global uint8_t gui_app_shared_buffer[1536];

static app_st *gui_app_current;

global void
gui_app_launch(app_st *app)
{
    if (gui_app_current) {
        if (gui_app_current->on_close) {
            gui_app_current->on_close();
        }

        gui_app_current = NULL;
    }

    gui_surface_draw_rect(&GUI_POINT_ZERO, &gui_app_rect, gui_color_bg);
    gui_surface_mark_dirty(&GUI_POINT_ZERO, &gui_app_rect);
    gui_status_set("");
    gui_status_set_br("");

    gui_app_current = app;

    krn_timer_set_frequency(gui_app_current->tick_frequency);

    memset(gui_app_shared_buffer, 0, sizeof(gui_app_shared_buffer));

    if (gui_app_current->on_init) {
        gui_app_current->on_init();
        gui_app_current->on_init = (void(*)(void))NULL;
    }

    ASSERT(!!gui_app_current->on_show);
    gui_app_current->on_show();
}

global void
gui_app_handle_event(event_st *event)
{
    app_st *app = gui_app_current;
    key_st key;

    if (event->type == EVENT_TIMER_TICK) {
        if (app->on_tick) {
            app->on_tick();
        }
    } else if (event->type == EVENT_KEY_DOWN) {
        key.encoded = event->payload;

        if (key.p.code == KEY_ESC) {
            gui_app_launch(&app_launcher);
        } else if (key.p.code == KEY_Q && key.p.mods & KEY_MOD_SHIFT) {
            krn_exit();
        } else if (app->on_key_down) {
            app->on_key_down(key.p.code, key.p.mods);
        }
    } else if (event->type == EVENT_KEY_UP) {
        key.encoded = event->payload;

        if (app->on_key_up) {
            app->on_key_up(key.p.code, key.p.mods);
        }
    }
}
