// --------------------------------------------------------------------------------------
// Copyright (c) 2025-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: panel.c - Side panel app
// --------------------------------------------------------------------------------------

#include <gui.h>

static app_st *apps[] = {
    &app_clock,
    &app_calendar,
    &app_fonts,
    &app_colors,
    &app_sounds,
    &app_snake
};

enum {
    WINDOW_WIDTH = PANEL_WIDTH,
    WINDOW_HEIGHT = GUI_HEIGHT,

    BUTTONS_COUNT = sizeof(apps) / sizeof(apps[0]),
};

static uint8_t window_pixels[WINDOW_WIDTH * WINDOW_HEIGHT];
static surface_st window_surface;
static window_st window;

static widget_st buttons[BUTTONS_COUNT];
static widget_st *widgets[BUTTONS_COUNT];

static void
on_button_pointer_up(widget_st *widget, event_st event, point_st pos)
{
    gui_button_on_pointer_up(widget, event, pos);

    if (apps[widget->tag1]) {
        apps[widget->tag1]->show();
    }
}

static void
init_window(void)
{
    window_surface.size.width = WINDOW_WIDTH;
    window_surface.size.height = WINDOW_HEIGHT;
    window_surface.pitch = WINDOW_WIDTH;
    window_surface.pixels = window_pixels;

    window.rect.x = GUI_WIDTH - WINDOW_WIDTH;
    window.rect.y = 0;
    window.rect.width = WINDOW_WIDTH;
    window.rect.height = WINDOW_HEIGHT;
    window.surface = &window_surface;
    window.widgets = widgets;
    window.widgets_capacity = sizeof(widgets) / sizeof(widgets[0]);
    window.visible = 1;

    gui_surface_draw_v_seg(window.surface, 0, 0, WINDOW_HEIGHT, COLOR_BORDER);

    rect_st content_rect = {
        .x = 1,
        .y = 0,
        .width = WINDOW_WIDTH - 1,
        .height = WINDOW_HEIGHT,
    };
    gui_surface_draw_rect(window.surface, content_rect, COLOR_WINDOW);
}

static void
init_buttons(void)
{
    for (size_t i = 0; i < sizeof(apps) / sizeof(apps[0]); i++) {
        buttons[i].type = WIDGET_TYPE_BUTTON;
        buttons[i].rect.x = 8;
        buttons[i].rect.y = 8 + (i * 56);
        buttons[i].rect.width = 48;
        buttons[i].rect.height = 48;
        buttons[i].window = &window;
        buttons[i].bitmap_regular = apps[i]->panel_icon_r;
        buttons[i].bitmap_pressed = apps[i]->panel_icon_a;
        buttons[i].on_pointer_up = on_button_pointer_up;
        buttons[i].tag1 = i;

        gui_window_add_widget(&window, &buttons[i]);
    }
}

static void
show_panel(void)
{
    static int initialized = 0;

    if (!initialized) {
        init_window();
        init_buttons();
        initialized = 1;
    }

    gui_wm_set_panel_window(&window);
}

app_st app_panel = {
    .show = show_panel,
};
