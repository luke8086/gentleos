/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: calendar.c - Calendar app
 */

#include <gui.h>

enum {
    GRID_CELL_WIDTH = 32,
    GRID_CELL_HEIGHT = 16,

    TOOL_BAR_Y = 0,
    TOOL_BAR_HEIGHT = GRID_CELL_HEIGHT + 2,

    WEEK_BAR_Y = (TOOL_BAR_Y + TOOL_BAR_HEIGHT - 1),
    WEEK_BAR_HEIGHT = 16,

    GRID_ROWS = 6,
    GRID_COLS = 7,
    GRID_WIDTH = GRID_WIDTH_SPACED(GRID_CELL_WIDTH, GRID_COLS),
    GRID_HEIGHT = GRID_HEIGHT_SPACED(GRID_CELL_HEIGHT, GRID_ROWS),
    GRID_X = 1,
    GRID_Y = WEEK_BAR_Y + WEEK_BAR_HEIGHT,

    WINDOW_WIDTH = GRID_X + GRID_WIDTH + 1,
    WINDOW_HEIGHT = GRID_Y + GRID_HEIGHT + 1,
};

enum {
    MIN_YEAR = 1900,
    MAX_YEAR = 2099,
};

typedef struct {
    window_st window;
    grid_st grid;

    int cur_month;
    int cur_year;
    int cur_day;

    int sel_month;
    int sel_year;
} app_state_st;

static app_state_st *app_state = (app_state_st *)gui_app_shared_buffer;

static void
draw_month_label(void)
{
    app_state_st *a = app_state;
    const char *month_name = TIME_MONTH_NAMES_SHORT[a->sel_month - 1];
    char buf[16];
    rect_st rect;

    rect.x = 0;
    rect.y = TOOL_BAR_Y;
    rect.width = WINDOW_WIDTH;
    rect.height = TOOL_BAR_HEIGHT;

    snprintf(buf, sizeof(buf), "%s %d", month_name, a->sel_year);

    gui_surface_draw_border(&a->window.origin, &rect, gui_color_fg);
    gui_surface_draw_str_centered(&a->window.origin, &rect, NULL, buf,
        gui_color_fg, gui_color_bg);
    gui_surface_mark_dirty(&a->window.origin, &rect);
}

static void
draw_day_cell(int col, int row, int day)
{
    app_state_st *a = app_state;
    int fg, bg;
    char buf[3];
    rect_st rect;

    gui_grid_cell_rect(&a->grid, col, row, &rect);

    if (day == 0) {
        gui_surface_draw_rect(&a->window.origin, &rect, gui_color_bg);
        gui_surface_mark_dirty(&a->window.origin, &rect);
        return;
    }

    if (day == a->cur_day && a->sel_month == a->cur_month && a->sel_year == a->cur_year) {
        fg = gui_color_bg;
        bg = gui_color_fg;
    } else {
        fg = gui_color_fg;
        bg = gui_color_bg;
    }

    gui_surface_draw_rect(&a->window.origin, &rect, bg);

    snprintf(buf, sizeof(buf), "%d", day);
    gui_surface_draw_str_centered(&a->window.origin, &rect, NULL, buf, fg, bg);

    gui_surface_mark_dirty(&a->window.origin, &rect);
}

static void
draw_selected_month(void)
{
    app_state_st *a = app_state;
    int day_of_week = time_get_day_of_week(1, a->sel_month, a->sel_year);
    int num_days = time_get_days_in_month(a->sel_month, a->sel_year);
    int col, row, day;

    for (row = 0; row < GRID_ROWS; ++row) {
        for (col = 0; col < GRID_COLS; ++col) {
            day = row * GRID_COLS + col - day_of_week + 1;
            draw_day_cell(col, row, (day >= 1 && day <= num_days) ? day : 0);
        }
    }

    draw_month_label();
}

static void
draw_week_bar(void)
{
    app_state_st *a = app_state;
    int y;
    rect_st rect;

    for (y = 0; y < 7; y++) {
        rect.x = y * (GRID_CELL_WIDTH + 2) - y;
        rect.y = WEEK_BAR_Y;
        rect.width = GRID_CELL_WIDTH + 2;
        rect.height = WEEK_BAR_HEIGHT;

        gui_surface_draw_border(&a->window.origin, &rect, gui_color_fg);
        gui_surface_draw_str_centered(&a->window.origin, &rect, NULL,
            TIME_DAY_NAMES_SHORT[y], gui_color_fg, gui_color_bg);
    }
}

static void
set_prev_month(void)
{
    app_state_st *a = app_state;

    if (a->sel_month > 1) {
        a->sel_month -= 1;
    } else if (a->sel_year > MIN_YEAR) {
        a->sel_year -= 1;
        a->sel_month = 12;
    } else {
        return;
    }

    draw_selected_month();
}

static void
set_next_month(void)
{
    app_state_st *a = app_state;

    if (a->sel_month < 12) {
        a->sel_month += 1;
    } else if (a->sel_year < MAX_YEAR) {
        a->sel_year += 1;
        a->sel_month = 1;
    } else {
        return;
    }

    draw_selected_month();
}

static void
on_key_up(uint8_t key_code, uint8_t key_mods)
{
    switch (key_code) {
    case KEY_PGUP: set_next_month(); break;
    case KEY_PGDN: set_prev_month(); break;
    }
}

static void
init_grid(void)
{
    app_state_st *a = app_state;

    a->grid.cell_width = GRID_CELL_WIDTH;
    a->grid.cell_height = GRID_CELL_HEIGHT;
    a->grid.cols = GRID_COLS;
    a->grid.rows = GRID_ROWS;
    a->grid.x = GRID_X;
    a->grid.y = GRID_Y;
}

static void
init_current_date(void)
{
    app_state_st *a = app_state;
    time_st t;
    time_get(&t);

    a->cur_month = t.month;
    a->cur_year = t.year;
    a->cur_day = t.day;

    a->sel_month = a->cur_month;
    a->sel_year = a->cur_year;
}

static void
on_show(void)
{
    app_state_st *a = app_state;

    gui_window_init(&a->window, WINDOW_WIDTH, WINDOW_HEIGHT);
    init_grid();
    init_current_date();

    gui_window_draw(&a->window, gui_color_bg, 1);
    draw_week_bar();
    draw_selected_month();
    gui_status_set_br("PgUp/PgDn: Select month");
}

static void
on_init(void)
{
    ASSERT(sizeof(app_state_st) <= sizeof(gui_app_shared_buffer));

    app_calendar.on_show = on_show;
    app_calendar.on_key_up = on_key_up;
}

global app_st app_calendar = {
    "Calendar",
    &icon_calendar,
    on_init,
};
