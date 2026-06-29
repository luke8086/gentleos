/*
 * Copyright (c) 2025-2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: clock.c - Clock app
 */

#include <gui.h>

enum {
    GRID_CELL_WIDTH = 8,
    GRID_CELL_HEIGHT = 8,
    GRID_COLS = 27,
    GRID_ROWS = 5,
    GRID_X = 1,
    GRID_Y = 1,
    GRID_WIDTH = GRID_WIDTH_SPACED(GRID_CELL_WIDTH, GRID_COLS),
    GRID_HEIGHT = GRID_HEIGHT_SPACED(GRID_CELL_HEIGHT, GRID_ROWS),

    WINDOW_WIDTH = GRID_X + GRID_WIDTH + 1,
    WINDOW_HEIGHT = GRID_Y + GRID_HEIGHT + 1,
};

static window_st window;

static grid_st grid;

static int last_second;
static int last_day;

static void
draw_cell(int x, int y, int active)
{
    rect_st r;
    uint8_t color = active ? gui_color_fg : gui_color_bg;
    gui_grid_cell_rect(&grid, x, y, &r);
    gui_surface_draw_rect(&window.origin, &r, color);
    gui_surface_mark_dirty(&window.origin, &r);
}

static void
draw_digit(int x, int y, int digit)
{
    static const uint16_t digit_pixels[10] = {
        0xf6de, 0x592e, 0xe7ce, 0xe79e, 0xb792,
        0xf39e, 0xf3de, 0xe492, 0xf7de, 0xf79e,
    };
    int i;
    uint8_t active;

    for (i = 0; i < 15; ++i) {
        active = !!(digit_pixels[digit] & (1 << (15 - i)));
        draw_cell(x + i % 3, y + i / 3, active);
    }
}

static void
draw_time(void)
{
    time_st time;

    time_get(&time);

    if (time.second == last_second) {
        return;
    }

    last_second = time.second;

    draw_digit(0, 0, time.hour / 10);
    draw_digit(4, 0, time.hour % 10);
    draw_cell(8, 1, 1);
    draw_cell(8, 3, 1);
    draw_digit(10, 0, time.minute / 10);
    draw_digit(14, 0, time.minute % 10);
    draw_cell(18, 1, 1);
    draw_cell(18, 3, 1);
    draw_digit(20, 0, time.second / 10);
    draw_digit(24, 0, time.second % 10);
}

static void
update_status(void)
{
    time_st time;
    int day_of_week;

    time_get(&time);

    if (time.day == last_day) {
        return;
    }

    last_day = time.day;
    day_of_week = time_get_day_of_week(time.day, time.month, time.year);

    gui_status_set_br("%04d-%02d-%02d (%s)",
        time.year, time.month, time.day,
        TIME_DAY_NAMES_LONG[day_of_week]
    );
}

static void
on_timeout(void)
{
    draw_time();
    update_status();
}

static void
on_tick(void)
{
    static unsigned count = 0;

    ++count;

    if (count >= 5) {
        count = 0;
        on_timeout();
    }
}

static void
init_grid(void)
{
    grid.cell_width = GRID_CELL_WIDTH;
    grid.cell_height = GRID_CELL_HEIGHT;
    grid.cols = GRID_COLS;
    grid.rows = GRID_ROWS;
    grid.x = GRID_X;
    grid.y = GRID_Y;
}

static void
on_show(void)
{
    last_day = 0xff;
    last_second = 0xff;

    draw_time();
    update_status();
}

static void
on_init(void)
{
    gui_window_init(&window, WINDOW_WIDTH, WINDOW_HEIGHT);

    init_grid();

    app_clock.on_show = on_show;
    app_clock.on_tick = on_tick;
}

global app_st app_clock = {
    "Clock",
    &icon_clock,
    on_init,
};
