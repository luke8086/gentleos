// --------------------------------------------------------------------------------------
// Copyright (c) 2025-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: panel.c - Side panel app
// --------------------------------------------------------------------------------------

#include <gui.h>

static app_st *apps[] = {
    &app_about,
    &app_clock,
    &app_calendar,
    &app_fonts,
    &app_colors,
    &app_patterns,
    &app_sounds,
    &app_snake,
    &app_mines,
    &app_tetris,
};

#define APPS_COUNT (sizeof(apps) / sizeof(apps[0]))

enum {
    WINDOW_WIDTH = PANEL_WIDTH,
    WINDOW_HEIGHT = GUI_HEIGHT,

    APP_BUTTON_MARGIN = 8,
    APP_BUTTON_SIZE = 48,
    APP_BUTTON_STRIDE = APP_BUTTON_SIZE + APP_BUTTON_MARGIN,

    APP_BUTTONS_COUNT = (WINDOW_HEIGHT - STATUS_HEIGHT - APP_BUTTON_MARGIN)
        / APP_BUTTON_STRIDE,

    NAV_WIDTH = PANEL_WIDTH / 2,
    NAV_HEIGHT = STATUS_HEIGHT,
};

static int current_page = 0;

static uint8_t window_pixels[WINDOW_WIDTH * WINDOW_HEIGHT];
static surface_st window_surface;
static window_st window;

static widget_st app_buttons[APP_BUTTONS_COUNT];
static widget_st prev_button;
static widget_st next_button;
static widget_st *widgets[APP_BUTTONS_COUNT + 2];

static void
set_page(int page)
{
    current_page = page;

    rect_st area = { .x = 1, .y = 0, .width = WINDOW_WIDTH - 1,
        .height = WINDOW_HEIGHT - STATUS_HEIGHT };
    gui_surface_draw_rect(window.surface, area, COLOR_WINDOW);

    for (size_t i = 0; i < APP_BUTTONS_COUNT; i++) {
        size_t app_idx = current_page * APP_BUTTONS_COUNT + i;

        if (app_idx >= APPS_COUNT) {
            app_buttons[i].hidden = 0;
            continue;
        }

        app_buttons[i].bitmap_regular = apps[app_idx]->panel_icon_r;
        app_buttons[i].bitmap_pressed = apps[app_idx]->panel_icon_a;
        app_buttons[i].tag1 = app_idx;
        app_buttons[i].hidden = 0;

        gui_widget_draw(&app_buttons[i]);
    }

    gui_wm_render_window_region(&window, gui_window_area(&window));
}

static void
on_button_pointer_up(widget_st *widget, event_st event, point_st pos)
{
    gui_button_on_pointer_up(widget, event, pos);

    if (apps[widget->tag1]) {
        apps[widget->tag1]->show();
    }
}

static void
on_prev_pointer_up(widget_st *widget, event_st event, point_st pos)
{
    gui_button_on_pointer_up(widget, event, pos);

    if (current_page > 0) {
        set_page(current_page - 1);
    }
}

static void
on_next_pointer_up(widget_st *widget, event_st event, point_st pos)
{
    gui_button_on_pointer_up(widget, event, pos);

    int max_page = (APPS_COUNT + APP_BUTTONS_COUNT - 1) / APP_BUTTONS_COUNT - 1;

    if (current_page < max_page) {
        set_page(current_page + 1);
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

    gui_surface_draw_rect(window.surface, gui_window_area(&window), COLOR_WINDOW);
    gui_surface_draw_v_seg(window.surface, 0, 0, WINDOW_HEIGHT - STATUS_HEIGHT + 1,
        COLOR_BORDER);
}

static void
init_app_buttons(void)
{
    for (size_t i = 0; i < APP_BUTTONS_COUNT; i++) {
        app_buttons[i].type = WIDGET_TYPE_BUTTON;
        app_buttons[i].rect.x = APP_BUTTON_MARGIN;
        app_buttons[i].rect.y = APP_BUTTON_MARGIN + (i * APP_BUTTON_STRIDE);
        app_buttons[i].rect.width = APP_BUTTON_SIZE;
        app_buttons[i].rect.height = APP_BUTTON_SIZE;
        app_buttons[i].window = &window;
        app_buttons[i].on_pointer_up = on_button_pointer_up;
        app_buttons[i].hidden = 1;

        gui_window_add_widget(&window, &app_buttons[i]);
    }
}

static void
init_nav_buttons(void)
{
    prev_button.type = WIDGET_TYPE_BUTTON;
    prev_button.rect.x = 0;
    prev_button.rect.y = WINDOW_HEIGHT - NAV_HEIGHT;
    prev_button.rect.width = NAV_WIDTH;
    prev_button.rect.height = NAV_HEIGHT;
    prev_button.window = &window;
    prev_button.label = "<";
    prev_button.hide_border = 1;
    prev_button.on_pointer_up = on_prev_pointer_up;
    gui_window_add_widget(&window, &prev_button);

    next_button.type = WIDGET_TYPE_BUTTON;
    next_button.rect.x = NAV_WIDTH;
    next_button.rect.y = WINDOW_HEIGHT - NAV_HEIGHT;
    next_button.rect.width = NAV_WIDTH;
    next_button.rect.height = NAV_HEIGHT;
    next_button.window = &window;
    next_button.label = ">";
    next_button.hide_border = 1;
    next_button.on_pointer_up = on_next_pointer_up;
    gui_window_add_widget(&window, &next_button);
}

static void
show_panel(void)
{
    static int initialized = 0;

    if (!initialized) {
        init_window();
        init_app_buttons();
        init_nav_buttons();
        set_page(0);
        initialized = 1;
    }

    gui_wm_set_panel_window(&window);
}

app_st app_panel = {
    .show = show_panel,
};
