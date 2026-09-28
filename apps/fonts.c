/*
 * Copyright (c) 2025-2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: fonts.c - Font browser app
 */

#include <gui.h>

enum {
    TOOL_BAR_Y = 0,
    TOOL_BAR_HEIGHT = 24,

    GRID_CELL_WIDTH = 13,
    GRID_CELL_HEIGHT = 14,
    GRID_ROWS = 8,
    GRID_COLS = 16,
    GRID_CELLS_COUNT = (GRID_ROWS * GRID_COLS),
    GRID_WIDTH = GRID_WIDTH_SPACED(GRID_CELL_WIDTH, GRID_COLS),
    GRID_HEIGHT = GRID_HEIGHT_SPACED(GRID_CELL_HEIGHT, GRID_ROWS),
    GRID_X = 1,
    GRID_Y = TOOL_BAR_Y + TOOL_BAR_HEIGHT,

    WINDOW_WIDTH = GRID_X + GRID_WIDTH + 1,
    WINDOW_HEIGHT = GRID_Y + GRID_HEIGHT + 1,
};

static window_st window;
static grid_st grid;

static unsigned current_font;
static unsigned current_col;
static unsigned current_row;

static void
update_status(void)
{
    int n = current_row * grid.cols + current_col;
    gui_status_set("Hex:%02x  Dec:%03d", n, n);
}

static void
draw_cell(int col, int row)
{
    char str[2];
    rect_st r;
    int is_active = col == current_col && row == current_row;

    str[0] = row * grid.cols + col;
    str[1] = 0;

    gui_grid_cell_rect(&grid, col, row, &r);

    gui_surface_draw_rect(&window.origin, &r, is_active ? gui_color_fg : gui_color_bg);

    gui_surface_draw_str_centered(
        &window.origin,
        &r,
        &fonts[current_font],
        (const char *)str,
        is_active ? gui_color_bg : gui_color_fg,
        is_active ? gui_color_fg : gui_color_bg
    );

    gui_surface_mark_dirty(&window.origin, &r);
}

static void
draw_all_cells(void)
{
    int row, col;

    for (row = 0; row < grid.rows; ++row) {
        for (col = 0; col < grid.cols; ++col) {
            draw_cell(col, row);
        }
    }
}

static void
draw_font_label(void)
{
    rect_st r;
    rect_st shrunken;

    gui_rect_init(&r, 0, TOOL_BAR_Y, WINDOW_WIDTH, TOOL_BAR_HEIGHT);

    gui_surface_draw_border(&window.origin, &r, gui_color_fg);
    gui_rect_copy(&shrunken, &r);
    gui_rect_shrink(&shrunken, 1);
    gui_surface_draw_rect(&window.origin, &shrunken, gui_color_bg);
    gui_surface_draw_str_centered(&window.origin, &r, NULL,
        fonts[current_font].name, gui_color_fg, gui_color_bg);

    gui_surface_mark_dirty(&window.origin, &r);
}

static void
set_prev_font(void)
{
    if (current_font == 0) {
        return;
    }

    --current_font;
    draw_font_label();
    draw_all_cells();
}

static void
set_next_font(void)
{
    if (current_font >= (FONT_COUNT - 1)) {
        return;
    }

    ++current_font;
    draw_font_label();
    draw_all_cells();
}

static void
update_current_cell(int dx, int dy)
{
    int prev_col = current_col;
    int prev_row = current_row;

    current_col = (current_col + dx) % grid.cols;
    current_row = (current_row + dy) % grid.rows;

    draw_cell(prev_col, prev_row);
    draw_cell(current_col, current_row);

    update_status();
}

static void
on_key_down(uint8_t key_code, uint8_t key_mods)
{
    switch (key_code) {
        case KEY_LEFT:  update_current_cell(-1, 0); return;
        case KEY_RIGHT: update_current_cell(1, 0); return;
        case KEY_UP:    update_current_cell(0, -1); return;
        case KEY_DOWN:  update_current_cell(0, 1); return;
        case KEY_PGUP:  set_prev_font(); return;
        case KEY_PGDN:  set_next_font(); return;
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
    gui_window_draw(&window, gui_color_fg, 1);

    current_font = 0;
    current_col = 0;
    current_row = 0;

    draw_font_label();
    draw_all_cells();
    gui_status_set_br("PgUp/PgDn: Select font");

    update_status();
}

static void
on_init(void)
{
    gui_window_init(&window, WINDOW_WIDTH, WINDOW_HEIGHT);

    init_grid();

    app_fonts.on_show = on_show;
    app_fonts.on_key_down = on_key_down;
}

global app_st app_fonts = {
    "Fonts",
    &icon_fonts,
    on_init,
};
