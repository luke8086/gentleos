/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: mines.c - Minesweeper game
 */

#include <gui.h>

static void reveal_cell(int col, int row);

enum {
    GRID_CELL_WIDTH = 13,
    GRID_CELL_HEIGHT = 13,
    GRID_ROWS = 10,
    GRID_COLS = 16,
    GRID_CELL_COUNT = GRID_ROWS * GRID_COLS,
    GRID_WIDTH = GRID_WIDTH_SPACED(GRID_CELL_WIDTH, GRID_COLS),
    GRID_HEIGHT = GRID_HEIGHT_SPACED(GRID_CELL_HEIGHT, GRID_ROWS),
    GRID_X = 1,
    GRID_Y = 1,

    WINDOW_WIDTH = GRID_X + GRID_WIDTH + 1,
    WINDOW_HEIGHT = GRID_Y + GRID_HEIGHT + 1,

    MINE_COUNT = 24,
};

static window_st window;
static grid_st grid;

enum {
    CELL_STATE_HIDDEN = 0,
    CELL_STATE_REVEALED = 1,
    CELL_STATE_FLAGGED = 2,
};

enum {
    CELL_TYPE_EMPTY = 0,
    CELL_TYPE_MINE = 9,
};

enum {
    GAME_STATE_PLAYING = 0,
    GAME_STATE_WON = 1,
    GAME_STATE_LOST = 2,
};

static uint8_t cell_state[GRID_COLS][GRID_ROWS];
static uint8_t cell_type[GRID_COLS][GRID_ROWS];
static unsigned current_col;
static unsigned current_row;

static size_t
count_cells_by_state(uint8_t state)
{
    int x, y;
    size_t count = 0;

    for (y = 0; y < GRID_ROWS; ++y) {
        for (x = 0; x < GRID_COLS; ++x) {
            if (cell_state[x][y] == state) {
                count++;
            }
        }
    }

    return count;
}

static size_t
count_adjacent_mines(int col, int row)
{
    int dx, dy, nx, ny;
    size_t count = 0;

    for (dy = -1; dy <= 1; ++dy) {
        for (dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) {
                continue;
            }

            nx = col + dx;
            ny = row + dy;

            if (nx >= 0 && nx < GRID_COLS && ny >= 0 && ny < GRID_ROWS) {
                if (cell_type[nx][ny] == CELL_TYPE_MINE) {
                    ++count;
                }
            }
        }
    }

    return count;
}

static void
draw_cursor(int col, int row, uint8_t color)
{
    rect_st rect;

    gui_grid_cell_rect(&grid, col, row, &rect);
    gui_surface_draw_border(&window.origin, &rect, color);
    gui_surface_mark_dirty(&window.origin, &rect);
}

static void
draw_cell(int col, int row)
{
    uint8_t state = cell_state[col][row];
    uint8_t type = cell_type[col][row];
    rect_st rect, num_rect, dot_rect;
    char num_str[2];

    num_str[0] = 0;
    num_str[1] = 0;

    gui_grid_cell_rect(&grid, col, row, &rect);
    gui_surface_draw_rect(&window.origin, &rect, gui_color_bg);

    if (state == CELL_STATE_FLAGGED) {
        gui_surface_draw_bitmap_centered(&window.origin, &window.size, &rect, &sprite_flag,
            gui_color_fg);
    } else if (state == CELL_STATE_REVEALED && type == CELL_TYPE_MINE) {
        gui_surface_draw_bitmap_centered(&window.origin, &window.size, &rect, &sprite_mine,
            gui_color_fg);
    } else if (state == CELL_STATE_REVEALED && type == CELL_TYPE_EMPTY) {
        gui_rect_init(&dot_rect, rect.x + rect.width / 2 - 1, rect.y + rect.height / 2, 2, 1);
        gui_surface_draw_rect(&window.origin, &dot_rect, gui_color_fg);
    } else if (state == CELL_STATE_REVEALED) {
        num_str[0] = '0' + type;

        gui_rect_init(&num_rect, rect.x + 1, rect.y + 1,
            rect.width - 1, rect.height - 1);

        gui_surface_draw_str_centered(&window.origin, &num_rect, NULL,
            num_str, gui_color_fg, gui_color_bg);
    }

    if (row == current_row && col == current_col) {
        draw_cursor(col, row, gui_color_fg);
    }

    gui_surface_mark_dirty(&window.origin, &rect);
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
update_cell(int col, int row, uint8_t type, uint8_t state)
{
    cell_type[col][row] = type;
    cell_state[col][row] = state;
    draw_cell(col, row);
}

static void
update_all_mines(uint8_t state)
{
    int row, col;

    for (row = 0; row < GRID_ROWS; ++row) {
        for (col = 0; col < GRID_COLS; ++col) {
            if (cell_type[col][row] == CELL_TYPE_MINE) {
                update_cell(col, row, CELL_TYPE_MINE, state);
            }
        }
    }
}

static void
clear_cells(void)
{
    int row, col;

    for (row = 0; row < GRID_ROWS; ++row) {
        for (col = 0; col < GRID_COLS; ++col) {
            update_cell(col, row, CELL_TYPE_EMPTY, CELL_STATE_HIDDEN);
        }
    }
}

static void
place_mines(int except_col, int except_row)
{
    int remaining = MINE_COUNT;
    int col, row;

    while (remaining > 0) {
        col = rand() % GRID_COLS;
        row = rand() % GRID_ROWS;

        if (col == except_col && row == except_row) {
            continue;
        }

        if (cell_type[col][row] != CELL_TYPE_MINE) {
            cell_type[col][row] = CELL_TYPE_MINE;
            --remaining;
        }
    }
}

static int
get_game_state(void)
{
    int row, col;

    for (row = 0; row < GRID_ROWS; ++row) {
        for (col = 0; col < GRID_COLS; ++col) {
            if (cell_type[col][row] == CELL_TYPE_MINE &&
                cell_state[col][row] == CELL_STATE_REVEALED) {
                return GAME_STATE_LOST;
            }
        }
    }

    if (count_cells_by_state(CELL_STATE_REVEALED) == (GRID_CELL_COUNT - MINE_COUNT)) {
        return GAME_STATE_WON;
    }

    return GAME_STATE_PLAYING;
}

static void
update_status(void)
{
    int state = get_game_state();

    if (state == GAME_STATE_LOST) {
        gui_status_set("Game Over! Press R to play again");
        gui_status_set_br("");
    } else if (state == GAME_STATE_WON) {
        gui_status_set("You Won! Press R to play again");
        gui_status_set_br("");
    } else {
        size_t flagged_count = count_cells_by_state(CELL_STATE_FLAGGED);
        size_t remaining = MINE_COUNT > flagged_count ? MINE_COUNT - flagged_count : 0;

        gui_status_set("Remaining mines: %u", remaining);
        gui_status_set_br("F: Flag   Spc: Reveal");
    }

}

static void
restart_game(void)
{
    clear_cells();
    update_status();
}

static void
reveal_cell(int col, int row)
{
    int dx, dy;
    int adj_mine_count;

    if (col < 0 || col >= GRID_COLS || row < 0 || row >= GRID_ROWS) {
        return;
    }

    if (cell_state[col][row] != CELL_STATE_HIDDEN) {
        return;
    }

    adj_mine_count = count_adjacent_mines(col, row);
    update_cell(col, row, adj_mine_count, CELL_STATE_REVEALED);

    if (adj_mine_count > 0) {
        return;
    }

    for (dy = -1; dy <= 1; dy++) {
        for (dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) {
                continue;
            }

            reveal_cell(col + dx, row + dy);
        }
    }
}

static void
press_cell(int col, int row)
{
    if (get_game_state() != GAME_STATE_PLAYING) {
        return;
    }

    if (col < 0 || col >= GRID_COLS || row < 0 || row >= GRID_ROWS) {
        return;
    }

    if (cell_state[col][row] != CELL_STATE_HIDDEN) {
        return;
    }

    if (count_cells_by_state(CELL_STATE_REVEALED) == 0) {
        place_mines(col, row);
    }

    if (cell_type[col][row] == CELL_TYPE_MINE) {
        update_all_mines(CELL_STATE_REVEALED);
        update_status();
        return;
    };

    reveal_cell(col, row);

    if (get_game_state() == GAME_STATE_WON) {
        update_all_mines(CELL_STATE_FLAGGED);
    }

    update_status();
}

static void
flag_cell(int col, int row)
{
    if (get_game_state() != GAME_STATE_PLAYING) {
        return;
    }

    if (cell_state[col][row] == CELL_STATE_HIDDEN) {
        update_cell(col, row, cell_type[col][row], CELL_STATE_FLAGGED);
        update_status();
    } else if (cell_state[col][row] == CELL_STATE_FLAGGED) {
        update_cell(col, row, cell_type[col][row], CELL_STATE_HIDDEN);
        update_status();
    }
}

static void
move_cursor(int dx, int dy)
{
    int prev_col = current_col;
    int prev_row = current_row;

    current_col = (current_col + dx + grid.cols) % grid.cols;
    current_row = (current_row + dy + grid.rows) % grid.rows;

    draw_cursor(prev_col, prev_row, gui_color_bg);
    draw_cursor(current_col, current_row, gui_color_fg);
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
        case KEY_ENTER: press_cell(current_col, current_row); return;
        case KEY_F: flag_cell(current_col, current_row); return;
    }

    if (key_code == KEY_R && get_game_state() != GAME_STATE_PLAYING) {
        restart_game();
        return;
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
    current_row = 0;
    current_col = 0;

    gui_window_draw(&window, gui_color_fg, 1);
    draw_all_cells();
    restart_game();
}

static void
on_init(void)
{
    gui_window_init(&window, WINDOW_WIDTH, WINDOW_HEIGHT);

    init_grid();

    app_mines.on_show = on_show;
    app_mines.on_key_down = on_key_down;
}

global app_st app_mines = {
    "Mines",
    &icon_mines,
    on_init,
};
