/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: launcher.c - App launcher
 */

#include <gui.h>

enum {
    GRID_COLS = 5,
    GRID_ROWS = 3,

    APP_BUTTON_H_MARGIN = 12,
    APP_BUTTON_V_MARGIN = 8,
    APP_BUTTON_WIDTH = 42,
    APP_BUTTON_HEIGHT = 42,
    APP_BUTTON_H_STRIDE = APP_BUTTON_WIDTH + APP_BUTTON_H_MARGIN,
    APP_BUTTON_V_STRIDE = APP_BUTTON_HEIGHT + APP_BUTTON_V_MARGIN,

    GRID_X = 1 + APP_BUTTON_H_MARGIN,
    GRID_Y = 1 + APP_BUTTON_V_MARGIN,
    GRID_WIDTH = GRID_COLS * APP_BUTTON_H_STRIDE - APP_BUTTON_H_MARGIN,
    GRID_HEIGHT = GRID_ROWS * APP_BUTTON_V_STRIDE - APP_BUTTON_V_MARGIN,

    WINDOW_WIDTH = GRID_X + GRID_WIDTH + APP_BUTTON_H_MARGIN + 1,
    WINDOW_HEIGHT = GRID_Y + GRID_HEIGHT + APP_BUTTON_V_MARGIN + 1,
};

static app_st *apps[] = {
    &app_clock,
    &app_calendar,
    &app_calc,
    &app_fonts,
    &app_keys,
    &app_sounds,
    &app_mines,
    &app_pairs,
    &app_mahjong,
    &app_snake,
    &app_tetris,
    &app_freecell,
    &app_klondike,
    &app_blackjack,
    &app_setup,
};

#define APPS_COUNT (sizeof(apps) / sizeof(apps[0]))

static window_st window;
static int current_col;
static int current_row;

static void
cell_rect_init(rect_st *out, int col, int row)
{
    out->x = GRID_X + col * APP_BUTTON_H_STRIDE;
    out->y = GRID_Y + row * APP_BUTTON_V_STRIDE;
    out->width = APP_BUTTON_WIDTH;
    out->height = APP_BUTTON_HEIGHT;
}

static app_st *
get_current_app(void)
{
    int i = current_row * GRID_COLS + current_col;

    return apps[i];
}

static void
launch_current_app(void)
{
    app_st *app = get_current_app();

    gui_status_set_tl("GentleOS > %s", app->name);
    gui_run_app(app);
}

static void
update_status_bl(void)
{
    app_st *app = get_current_app();
    gui_status_set("Launch: %s", app->name);
}

static void
update_status_br(void)
{
    time_st t;

    time_get(&t);

    gui_status_set_br("%04u-%02u-%02u %02u:%02u",
        t.year, t.month, t.day, t.hour, t.minute);
}

static void
on_tick(void)
{
    static unsigned count = 0;

    ++count;

    if (count >= 30) {
        update_status_br();
        count = 0;
    }
}

static void
draw_cursor(int col, int row, uint8_t color)
{
    rect_st rect;

    cell_rect_init(&rect, col, row);
    gui_rect_shrink(&rect, 2);
    gui_surface_draw_border(&window.origin, &rect, color);
    gui_surface_mark_dirty(&window.origin, &rect);
}

static void
draw_cell(int col, int row)
{
    rect_st rect;
    int i = row * GRID_COLS + col;

    cell_rect_init(&rect, col, row);
    gui_surface_draw_rect(&window.origin, &rect, gui_color_bg);
    gui_surface_draw_border(&window.origin, &rect, gui_color_fg);

    if (i < APPS_COUNT && apps[i] && apps[i]->icon) {
        gui_surface_draw_bitmap_centered(&window.origin, &window.size, &rect,
            apps[i]->icon, gui_color_fg);
    }

    if (col == current_col && row == current_row) {
        draw_cursor(col, row, gui_color_fg);
    }

    gui_surface_mark_dirty(&window.origin, &rect);
}

static void
draw_all_cells(void)
{
    int col, row;

    for (row = 0; row < GRID_ROWS; ++row) {
        for (col = 0; col < GRID_COLS; ++col) {
            if (row * GRID_COLS + col < APPS_COUNT) {
                draw_cell(col, row);
            }
        }
    }
}

static void
move_cursor(int dx, int dy)
{
    int prev_col = current_col;
    int prev_row = current_row;

    int new_col = MAX(0, MIN(GRID_COLS - 1, current_col + dx));
    int new_row = MAX(0, MIN(GRID_ROWS - 1, current_row + dy));

    if ((new_row * GRID_COLS + new_col) >= APPS_COUNT) {
        return;
    }

    current_col = new_col;
    current_row = new_row;

    draw_cursor(prev_col, prev_row, gui_color_bg);
    draw_cursor(current_col, current_row, gui_color_fg);

    update_status_bl();
}

static void
on_key_down(uint8_t key_code, uint8_t key_mods)
{
    switch (key_code) {
        case KEY_LEFT: move_cursor(-1, 0); return;
        case KEY_RIGHT: move_cursor(1, 0); return;
        case KEY_UP: move_cursor(0, -1); return;
        case KEY_DOWN: move_cursor(0, 1); return;
        case KEY_SPACE:
        case KEY_ENTER: launch_current_app(); return;
    }
}

static void
on_show(void)
{
    gui_window_draw(&window, gui_color_bg, 0);
    draw_all_cells();

    gui_status_set_tl("GentleOS");
    update_status_bl();

    update_status_br();
}

static void
on_init(void)
{
    gui_window_init(&window, WINDOW_WIDTH, WINDOW_HEIGHT);

    app_launcher.on_show = on_show;
    app_launcher.on_key_down = on_key_down;
    app_launcher.on_tick = on_tick;
}

global app_st app_launcher = {
    "Launcher",
	0,
    on_init,
};
