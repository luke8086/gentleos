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
    GRID_CELLS_COUNT = GRID_ROWS * GRID_COLS,
    GRID_WIDTH = GRID_WIDTH_SPACED(GRID_CELL_WIDTH, GRID_COLS),
    GRID_HEIGHT = GRID_HEIGHT_SPACED(GRID_CELL_HEIGHT, GRID_ROWS),
    GRID_X = 1,
    GRID_Y = WEEK_BAR_Y + WEEK_BAR_HEIGHT,

    WINDOW_WIDTH = GRID_X + GRID_WIDTH + 1,
    WINDOW_HEIGHT = GRID_Y + GRID_HEIGHT + 1,
};

static widget_st day_buttons[GRID_CELLS_COUNT];
static window_st window;

enum {
    MIN_YEAR = 1900,
    MAX_YEAR = 2099,
};

static int current_month;
static int current_year;
static int current_day;

static int selected_month;
static int selected_year;

static grid_st grid;

static void
draw_month_label(void)
{
    const char *month_name = TIME_MONTH_NAMES_SHORT[selected_month - 1];
    char buf[16];
    rect_st rect;

    rect.x = 0;
    rect.y = TOOL_BAR_Y;
    rect.width = WINDOW_WIDTH;
    rect.height = TOOL_BAR_HEIGHT;

    snprintf(buf, sizeof(buf), "%s %d", month_name, selected_year);

    gui_surface_draw_border(&window.origin, &rect, gui_color_fg);
    gui_surface_draw_str_centered(&window.origin, &rect, NULL, buf,
        gui_color_fg, gui_color_bg);
    gui_surface_mark_dirty(&window.origin, &rect);
}

static void
draw_day_button(widget_st *widget)
{
    int day = widget->tag1;
    int num_days = time_get_days_in_month(selected_month, selected_year);
    int is_in_month = day >= 0 && day < num_days;
    int is_current = (day == current_day - 1 && selected_month == current_month
        && selected_year == current_year);
    int fg, bg;
    char buf[3];

    if (!is_in_month) {
        gui_surface_draw_rect(widget->origin, &widget->rect, gui_color_bg);
        gui_surface_mark_dirty(widget->origin, &widget->rect);
        return;
    }

    fg = is_current ? gui_color_bg : gui_color_fg;
    bg = is_current ? gui_color_fg : gui_color_bg;

    gui_surface_draw_rect(widget->origin, &widget->rect, bg);

    snprintf(buf, sizeof(buf), "%d", day + 1);
    gui_surface_draw_str_centered(widget->origin, &widget->rect, NULL,
        buf, fg, bg);

    gui_surface_mark_dirty(widget->origin, &widget->rect);
}

static void
draw_selected_month(void)
{
    int day_of_week = time_get_day_of_week(1, selected_month, selected_year);
    size_t i;

    for (i = 0; i < GRID_CELLS_COUNT; ++i) {
        day_buttons[i].tag1 = i - day_of_week;
        day_buttons[i].draw(&day_buttons[i]);
    }

    draw_month_label();
}

static void
draw_week_bar(void)
{
    int y;
    rect_st rect;

    for (y = 0; y < 7; y++) {
        rect.x = y * (GRID_CELL_WIDTH + 2) - y;
        rect.y = WEEK_BAR_Y;
        rect.width = GRID_CELL_WIDTH + 2;
        rect.height = WEEK_BAR_HEIGHT;

        gui_surface_draw_border(&window.origin, &rect, gui_color_fg);
        gui_surface_draw_str_centered(&window.origin, &rect, NULL,
            TIME_DAY_NAMES_SHORT[y], gui_color_fg, gui_color_bg);
    }
}

static void
set_prev_month(void)
{
    if (selected_month > 1) {
        selected_month -= 1;
    } else if (selected_year > MIN_YEAR) {
        selected_year -= 1;
        selected_month = 12;
    } else {
        return;
    }

    draw_selected_month();
}

static void
set_next_month(void)
{
    if (selected_month < 12) {
        selected_month += 1;
    } else if (selected_year < MAX_YEAR) {
        selected_year += 1;
        selected_month = 1;
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
init_day_buttons(void)
{
    uint16_t i;
    int col, row;

    grid.cell_width = GRID_CELL_WIDTH;
    grid.cell_height = GRID_CELL_HEIGHT;
    grid.cols = GRID_COLS;
    grid.rows = GRID_ROWS;
    grid.x = GRID_X;
    grid.y = GRID_Y;

    for (i = 0; i < GRID_CELLS_COUNT; ++i) {
        col = i % GRID_COLS;
        row = i / GRID_COLS;

        day_buttons[i].origin = &window.origin;
        gui_grid_cell_rect(&grid, col, row, &day_buttons[i].rect);
        day_buttons[i].draw = draw_day_button;
    }
}

static void
init_current_date(void)
{
    time_st t;
    time_get(&t);

    current_month = t.month;
    current_year = t.year;
    current_day = t.day;

    selected_month = current_month;
    selected_year = current_year;
}

static void
on_show(void)
{
    int i;

    gui_window_draw(&window, gui_color_bg, 1);

    for (i = 0; i < GRID_CELLS_COUNT; ++i) {
        day_buttons[i].draw(&day_buttons[i]);
    }

    draw_week_bar();
    draw_selected_month();
    gui_status_set_br("PgUp/PgDn: Select month");
}

static void
on_init(void)
{
    gui_window_init(&window, WINDOW_WIDTH, WINDOW_HEIGHT);

    init_day_buttons();
    init_current_date();

    app_calendar.on_show = on_show;
    app_calendar.on_key_up = on_key_up;
}

global app_st app_calendar = {
    "Calendar",
    &icon_calendar,
    on_init,
};
