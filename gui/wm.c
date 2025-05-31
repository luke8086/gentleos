// --------------------------------------------------------------------------------------
// Copyright (c) 2025-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: wm.c - Window manager
// --------------------------------------------------------------------------------------

#include <gui.h>

enum {
    WINDOWS_COUNT_MAX = 6,
};

rect_st gui_wm_container = { 0 };

static window_st *gui_wm_panel_window = NULL;
static window_st *gui_wm_windows[WINDOWS_COUNT_MAX];

void
gui_wm_toggle_window_active(window_st *w, int active)
{
    if (w->active == active) {
        return;
    }

    w->active = active;

    if (active) {
        gui_status_set("");
    }

    gui_window_on_active_change(w);
}

void
gui_wm_raise_window(struct window *w)
{
    unsigned i;

    for (i = 0; i < WINDOWS_COUNT_MAX; ++i) {
        if (gui_wm_windows[i] == w) {
            break;
        }
    }

    if (i == WINDOWS_COUNT_MAX) {
        return;
    }

    if (i > 0) {
        for (; i > 0; --i) {
            gui_wm_windows[i] = gui_wm_windows[i - 1];
            gui_wm_toggle_window_active(gui_wm_windows[i], 0);
        }
    }

    gui_wm_windows[0] = w;
    gui_wm_toggle_window_active(w, 1);

    gui_wm_render_desktop_region(w->rect, w);
}

int
gui_wm_add_window(struct window *w)
{
    unsigned i;

    for (i = 0; i < WINDOWS_COUNT_MAX; ++i) {
        if (gui_wm_windows[i] == w) {
            return 0;
        }

        if (gui_wm_windows[i] == NULL) {
            gui_wm_windows[i] = w;
            w->visible = 1;
            gui_wm_raise_window(w);
            return 1;
        }
    }

    gui_status_set("Error: Too many windows");

    return 0;
}

void
gui_wm_remove_window(struct window *w)
{
    unsigned i;

    for (i = 0; i < WINDOWS_COUNT_MAX; ++i) {
        if (gui_wm_windows[i] == w) {
            w->visible = 0;
            gui_wm_windows[i] = NULL;
            gui_wm_toggle_window_active(w, 0);

            break;
        }
    }

    for (; i < WINDOWS_COUNT_MAX; ++i) {
        gui_wm_windows[i] = (i + 1 < WINDOWS_COUNT_MAX) ? gui_wm_windows[i + 1] : NULL;
    }

    if (gui_wm_windows[0]) {
        gui_wm_toggle_window_active(gui_wm_windows[0], 1);
    }

    gui_wm_render_desktop_region(w->rect, NULL);
}

static void
gui_wm_render_wallpaper(rect_st rect)
{
    gui_fb_draw_start();
    gui_surface_draw_rect(gui_fb_surface, rect, 0x7c);
    gui_fb_draw_end();
}

void
gui_wm_render_window_surface(window_st *window, rect_st desktop_reg)
{
    desktop_reg = gui_rect_clip(desktop_reg, window->rect);
    rect_st window_reg = gui_rect_translate_back(desktop_reg, window->rect.pos);

    gui_fb_draw_start();
    gui_surface_copy(gui_fb_surface, desktop_reg.x, desktop_reg.y,
        window->surface, window_reg);
    gui_fb_draw_end();

}

// Re-render a specified region of the desktop to the screen,
// by rendering that region in all windows from the bottom up,
// starting from a specified bottom window
void
gui_wm_render_desktop_region(rect_st rect, window_st *bottom_window)
{
    window_st *w;
    int started = (bottom_window == NULL);

    if (!bottom_window) {
        gui_wm_render_wallpaper(rect);
    }

    for (int i = WINDOWS_COUNT_MAX - 1; i >= 0; --i) {
        w = gui_wm_windows[i];

        if (w && w == bottom_window) {
            started = 1;
        }

        if (w && started) {
            gui_wm_render_window_surface(w, rect);
        }
    }
}

void
gui_wm_render_window_region(window_st *window, rect_st window_reg)
{
    rect_st desktop_reg;

    if (!window->visible) {
        return;
    }

    desktop_reg = gui_rect_translate(window_reg, window->rect.pos);

    if (window == gui_wm_panel_window) {
        gui_wm_render_window_surface(window, desktop_reg);
    } else {
        gui_wm_render_desktop_region(desktop_reg, window);
    }
}

window_st *
gui_wm_find_window(uint16_t x, uint16_t y)
{
    point_st p = { .x = x, .y = y };

    if (gui_wm_panel_window && gui_rect_contains_point(gui_wm_panel_window->rect, p)) {
        return gui_wm_panel_window;
    }

    for (size_t i = 0; i < WINDOWS_COUNT_MAX; ++i) {
        window_st *w = gui_wm_windows[i];

        if (!w) {
            break;
        }

        if (gui_rect_contains_point(w->rect, p)) {
            return w;
        }
    }

    return NULL;
}

window_st *
gui_wm_top_window(void)
{
    return gui_wm_windows[0];
}

void
gui_wm_set_panel_window(window_st *w)
{
    gui_wm_panel_window = w;
    gui_wm_render_window_region(w, gui_window_area(w));
}

void
gui_wm_init(void)
{
    gui_wm_container.width = GUI_WIDTH - PANEL_WIDTH;
    gui_wm_container.height = GUI_HEIGHT - STATUS_HEIGHT;
    gui_wm_render_wallpaper(gui_wm_container);

    gui_status_init();

    app_panel.show();
}
