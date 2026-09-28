/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: tetris.c - Tetris game
 */

#include <gui.h>

enum {
    GRID_CELL_WIDTH = 6,
    GRID_CELL_HEIGHT = 6,
    GRID_ROWS = 20,
    GRID_COLS = 10,
    GRID_WIDTH = GRID_WIDTH_SPACED(GRID_CELL_WIDTH, GRID_COLS),
    GRID_HEIGHT = GRID_HEIGHT_SPACED(GRID_CELL_HEIGHT, GRID_ROWS),
    GRID_X = 1,
    GRID_Y = 1,

    WINDOW_WIDTH = GRID_X + GRID_WIDTH + 1,
    WINDOW_HEIGHT = GRID_Y + GRID_HEIGHT + 1,

    DROP_TICKS = MSECS_TO_TICKS(400),
};

static uint16_t pieces[7][4] = {
    { 0x4444, 0x0f00, 0x4444, 0x0f00 }, /* I */
    { 0x44c0, 0x8e00, 0x6440, 0x0e20 }, /* J */
    { 0x4460, 0x0e80, 0xc440, 0x2e00 }, /* L */
    { 0x0cc0, 0x0cc0, 0x0cc0, 0x0cc0 }, /* O */
    { 0x06c0, 0x4620, 0x06c0, 0x4620 }, /* S */
    { 0x4e00, 0x4640, 0x0e40, 0x4c40 }, /* T */
    { 0x0c60, 0x2640, 0x0c60, 0x2640 }, /* Z */
};

typedef struct {
    window_st window;
    grid_st grid;

    uint8_t board[GRID_ROWS][GRID_COLS];

    int cur_piece;
    int cur_rot;
    int cur_col;
    int cur_row;

    int game_over;
    int game_paused;

    uint16_t score;
} app_state_st;

static uint16_t best_score;
static app_state_st *app_state = (app_state_st *)gui_app_shared_buffer;

static void
update_status(void)
{
    app_state_st *a = app_state;
    const char *msg = "";

    if (a->game_over) {
        msg = "Game Over!  |  ";
    } else if (a->game_paused) {
        msg = "Paused  |  ";
    }

    gui_status_set("%sScore: %u  Best: %u", msg, a->score, best_score);
}

static void
update_score(int ds)
{
    app_state_st *a = app_state;

    a->score += ds;
    update_status();
}

static void
draw_cell(int row, int col, int active)
{
    app_state_st *a = app_state;
    rect_st cell;
    gui_grid_cell_rect(&a->grid, col, row, &cell);
    gui_surface_draw_rect(&a->window.origin, &cell, active ? gui_color_fg : gui_color_bg);
    gui_surface_mark_dirty(&a->window.origin, &cell);
}

static int
is_piece_valid(int piece_idx, int row, int col, int rot)
{
    app_state_st *a = app_state;
    uint16_t piece = pieces[piece_idx][rot];
    int dx, dy, x, y;

    for (dy = 0; dy < 4; ++dy) {
        for (dx = 0; dx < 4; ++dx) {
            if (!(piece & (0x8000 >> (dy * 4 + dx)))) {
                continue;
            }

            x = col + dx;
            y = row + dy;

            if (x < 0 || x >= GRID_COLS || y < 0 || y >= GRID_ROWS) {
                return 0;
            }

            if (a->board[y][x]) {
                return 0;
            }
        }
    }

    return 1;
}

static void
draw_current_piece(int visible)
{
    app_state_st *a = app_state;
    uint16_t piece = pieces[a->cur_piece][a->cur_rot];
    int dx, dy, col, row;

    for (dy = 0; dy < 4; ++dy) {
        for (dx = 0; dx < 4; ++dx) {
            if (!(piece & (0x8000 >> (dy * 4 + dx)))) {
                continue;
            }

            col = a->cur_col + dx;
            row = a->cur_row + dy;

            if (col >= 0 && col < GRID_COLS && row >= 0 && row < GRID_ROWS) {
                draw_cell(row, col, visible);
            }
        }
    }
}

static void
lock_current_piece(void)
{
    app_state_st *a = app_state;
    uint16_t piece = pieces[a->cur_piece][a->cur_rot];
    int dx, dy, col, row;

    for (dy = 0; dy < 4; ++dy) {
        for (dx = 0; dx < 4; ++dx) {
            if (!(piece & (0x8000 >> (dy * 4 + dx)))) {
                continue;
            }

            col = a->cur_col + dx;
            row = a->cur_row + dy;

            if (col >= 0 && col < GRID_COLS && row >= 0 && row < GRID_ROWS) {
                a->board[row][col] = 1;
            }
        }
    }

    update_score(5);
}

static int
move_current_piece(int dy, int dx, int dr)
{
    app_state_st *a = app_state;
    int row = a->cur_row + dy;
    int col = a->cur_col + dx;
    int rot = (a->cur_rot + dr) % 4;

    if (!is_piece_valid(a->cur_piece, row, col, rot)) {
        return 0;
    }

    draw_current_piece(0);
    a->cur_col = col;
    a->cur_row = row;
    a->cur_rot = rot;
    draw_current_piece(1);

    return 1;
}

static void
drop_current_piece(void)
{
    app_state_st *a = app_state;

    draw_current_piece(0);

    while (is_piece_valid(a->cur_piece, a->cur_row + 1, a->cur_col, a->cur_rot)) {
        ++a->cur_row;
    }

    draw_current_piece(1);
}

static int
is_row_full(int row)
{
    app_state_st *a = app_state;
    int col;

    for (col = 0; col < GRID_COLS; ++col) {
        if (!a->board[row][col]) {
            return 0;
        }
    }

    return 1;
}

static void
clear_rows(void)
{
    app_state_st *a = app_state;
    int row, row_to_shift, row_to_draw, col;

    for (row = GRID_ROWS - 1; row >= 0; --row) {
        if (!is_row_full(row)) {
            continue;
        }

        for (row_to_shift = row; row_to_shift > 0; --row_to_shift) {
            for (col = 0; col < GRID_COLS; ++col) {
                a->board[row_to_shift][col] = a->board[row_to_shift - 1][col];
            }
        }

        for (col = 0; col < GRID_COLS; ++col) {
            a->board[0][col] = 0;
        }

        for (row_to_draw = 0; row_to_draw <= row; ++row_to_draw) {
            for (col = 0; col < GRID_COLS; ++col) {
                draw_cell(row_to_draw, col, a->board[row_to_draw][col]);
            }
        }

        update_score(20);
        ++row;
    }
}

static void
spawn_piece(void)
{
    app_state_st *a = app_state;

    a->cur_piece = rand() % 7;
    a->cur_rot = 0;
    a->cur_col = GRID_COLS / 2 - 1;
    a->cur_row = 0;

    if (!is_piece_valid(a->cur_piece, a->cur_row, a->cur_col, a->cur_rot)) {
        a->game_over = 1;

        if (a->score > best_score) {
            best_score = a->score;
        }

        update_status();
        return;
    }

    draw_current_piece(1);
}

static void
restart_game(void)
{
    app_state_st *a = app_state;
    int row, col;

    a->game_over = 0;
    a->game_paused = 0;
    a->score = 0;

    for (row = 0; row < GRID_ROWS; ++row) {
        for (col = 0; col < GRID_COLS; ++col) {
            a->board[row][col] = 0;
            draw_cell(row, col, 0);
        }
    }

    spawn_piece();
    update_status();
}

static void
on_tick(void) {
    app_state_st *a = app_state;
    static unsigned count = 0;

    if ((++count) < DROP_TICKS) {
        return;
    }

    count = 0;

    if (a->game_over || a->game_paused) {
        return;
    }

    if (!move_current_piece(1, 0, 0)) {
        lock_current_piece();
        clear_rows();
        spawn_piece();
    }
}

static void
on_key_down(uint8_t key_code, uint8_t key_mods)
{
    app_state_st *a = app_state;

    if (a->game_over) {
        restart_game();
        return;
    }

    if (key_code == KEY_P) {
        a->game_paused = !a->game_paused;
        update_status();
        return;
    }

    if (a->game_paused) {
        return;
    }

    switch (key_code) {
    case KEY_LEFT: move_current_piece(0, -1, 0); return;
    case KEY_RIGHT: move_current_piece(0, 1, 0); return;
    case KEY_DOWN: move_current_piece(1, 0, 0); return;
    case KEY_UP: move_current_piece(0, 0, 1); return;
    case KEY_SPACE: drop_current_piece(); return;
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
on_show(void)
{
    app_state_st *a = app_state;

    gui_window_init(&a->window, WINDOW_WIDTH, WINDOW_HEIGHT);
    init_grid();

    gui_window_draw(&a->window, gui_color_bg, 1);
    gui_status_set_br("P: Pause/Resume");
    restart_game();
}

static void
on_init(void)
{
    ASSERT(sizeof(app_state_st) <= sizeof(gui_app_shared_buffer));

    app_tetris.on_show = on_show;
    app_tetris.on_tick = on_tick;
    app_tetris.on_key_down = on_key_down;
}

global app_st app_tetris = {
    "Tetris",
    &icon_tetris,
    on_init,
};
