/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: 2048.c - 2048 game
 */

#include <gui.h>

enum {
    LINE_SIZE = 4,

    GRID_ROWS = LINE_SIZE,
    GRID_COLS = LINE_SIZE,
    GRID_CELL_WIDTH = 32,
    GRID_CELL_HEIGHT = 32,
    GRID_WIDTH = GRID_WIDTH_SPACED(GRID_CELL_WIDTH, GRID_COLS),
    GRID_HEIGHT = GRID_HEIGHT_SPACED(GRID_CELL_HEIGHT, GRID_ROWS),
    GRID_X = 1,
    GRID_Y = 1,

    WINDOW_WIDTH = GRID_X + GRID_WIDTH + 1,
    WINDOW_HEIGHT = GRID_Y + GRID_HEIGHT + 1,

    CELL_EXP_WIN = 11, /* 2^11 == 2048 */
    FLASH_TICKS = MSECS_TO_TICKS(150),
};

typedef struct {
    uint8_t exp;
    uint8_t flashed;
} cell_st;

typedef struct {
    window_st window;

    grid_st grid;

    cell_st board[GRID_COLS][GRID_ROWS];

    uint16_t score;
    int game_over;
    int game_won;
    int skip_intro;

    unsigned flash_ticks;
} app_state_st;

static uint16_t best_score;
static app_state_st *app_state = (app_state_st *)gui_app_shared_buffer;

static void
update_status(void)
{
    app_state_st *a = app_state;

    if (a->game_won) {
        gui_status_set("You won!  Score: %u  Best: %u", a->score, best_score);
    } else if (a->game_over) {
        gui_status_set("Game over!  Score: %u  Best: %u", a->score, best_score);
    } else {
        gui_status_set("Score: %u  Best: %u", a->score, best_score);
    }
}

static void
add_score(uint16_t score)
{
    app_state_st *a = app_state;

    a->score += score;

    if (a->score > best_score) {
        best_score = a->score;
    }
}

/* Check for an empty cell or two equal neighbours */
static int
has_moves(void)
{
    app_state_st *a = app_state;
    uint8_t cur;
    int col, row;

    for (row = 0; row < GRID_ROWS; ++row) {
        for (col = 0; col < GRID_COLS; ++col) {
            cur = a->board[col][row].exp;

            if (!cur) {
                return 1;
            }

            if (col + 1 < GRID_COLS && a->board[col + 1][row].exp == cur) {
                return 1;
            }

            if (row + 1 < GRID_ROWS && a->board[col][row + 1].exp == cur) {
                return 1;
            }
        }
    }

    return 0;
}

static int
has_won(void)
{
    app_state_st *a = app_state;
    int col, row;

    for (row = 0; row < GRID_ROWS; ++row) {
        for (col = 0; col < GRID_COLS; ++col) {
            if (a->board[col][row].exp >= CELL_EXP_WIN) {
                return 1;
            }
        }
    }

    return 0;
}

static int
count_empty_cells(void)
{
    app_state_st *a = app_state;
    int col, row;
    int ret = 0;

    for (row = 0; row < GRID_ROWS; ++row) {
        for (col = 0; col < GRID_COLS; ++col) {
            if (!a->board[col][row].exp) {
                ++ret;
            }
        }
    }

    return ret;
}

static void
draw_cell(int col, int row)
{
    app_state_st *a = app_state;
    cell_st *cell = &a->board[col][row];
    rect_st rect;
    uint8_t bg, fg;
    char str[8];

    gui_grid_cell_rect(&a->grid, col, row, &rect);

    if (cell->flashed) {
        bg = gui_color_fg;
        fg = gui_color_bg;
    } else {
        bg = gui_color_bg;
        fg = gui_color_fg;
    }

    gui_surface_draw_rect(&a->window.origin, &rect, bg);

    if (cell->exp) {
        snprintf(str, sizeof(str), "%u", 1u << cell->exp);
        gui_surface_draw_str_centered(&a->window.origin, &rect, NULL, str, fg);
    }

    gui_surface_mark_dirty(&a->window.origin, &rect);
}

static void
draw_board(void)
{
    int col, row;

    for (row = 0; row < GRID_ROWS; ++row) {
        for (col = 0; col < GRID_COLS; ++col) {
            draw_cell(col, row);
        }
    }
}

static void
clear_flash(void)
{
    app_state_st *a = app_state;
    cell_st *cell;
    int col, row;

    a->flash_ticks = 0;

    for (row = 0; row < GRID_ROWS; ++row) {
        for (col = 0; col < GRID_COLS; ++col) {
            cell = &a->board[col][row];

            if (cell->flashed) {
                cell->flashed = 0;
                draw_cell(col, row);
            }
        }
    }
}

static void
clear_board(void)
{
    app_state_st *a = app_state;
    int col, row;

    for (row = 0; row < GRID_ROWS; ++row) {
        for (col = 0; col < GRID_COLS; ++col) {
            a->board[col][row].exp = 0;
            a->board[col][row].flashed = 0;
        }
    }

    draw_board();
}

static void
set_random_cell(void)
{
    app_state_st *a = app_state;
    int col, row, empty_index, empty_count;

    empty_count = count_empty_cells();

    if (!empty_count) {
        return;
    }

    empty_index = rand() % empty_count;

    for (row = 0; row < GRID_ROWS; ++row) {
        for (col = 0; col < GRID_COLS; ++col) {
            if (a->board[col][row].exp) {
                continue;
            }

            if (empty_index > 0) {
                --empty_index;
                continue;
            }

            /* 1 in 10 times start with 4 */
            a->board[col][row].exp = (rand() % 10) ? 1 : 2;
            draw_cell(col, row);

            return;
        }
    }
}

/* Shift back all cells after given index */
static int
shift_line(cell_st *line, int i)
{
    int shifted = 0;

    for (; i + 1 < LINE_SIZE; ++i) {
        line[i] = line[i + 1];

        if (line[i].exp) {
            shifted = 1;
        }
    }

    line[i].exp = 0;
    line[i].flashed = 0;

    return shifted;
}

/* Slide all cells towards the start of the line, merging adjacent equal cells */
static void
slide_line(cell_st *line)
{
    int i = 0;
    int shifted;
    cell_st *cur, *next;

    while (i < LINE_SIZE - 1) {
        cur = &line[i];
        next = &line[i + 1];

        if (!cur->exp) {
            /* Empty current: shift the subsequent ones and check current again */
            shifted = shift_line(line, i);

            if (!shifted) {
                return;
            }
        } else if (!next->exp) {
            /* Empty next: shift the subsequent ones and check current again */
            shifted = shift_line(line, i + 1);

            if (!shifted) {
                return;
            }
        } else if (cur->exp == next->exp) {
            /* Equal next: merge current, set next to 0, advance to next */
            ++cur->exp;
            cur->flashed = 1;
            next->exp = 0;

            add_score(1u << cur->exp);

            ++i;
        } else {
            /* Non-equal next: advance to next */
            ++i;
        }
    }
}

static int
slide_board(int dc, int dr)
{
    app_state_st *a = app_state;
    cell_st line[LINE_SIZE];
    cell_st *new_cell, *old_cell;
    int i, j, col, row, start_col, start_row;
    int changed = 0;

    for (i = 0; i < LINE_SIZE; ++i) {
        start_col = dc ? ((dc > 0) ? LINE_SIZE - 1 : 0) : i;
        start_row = dr ? ((dr > 0) ? LINE_SIZE - 1 : 0) : i;

        for (j = 0; j < LINE_SIZE; ++j) {
            col = start_col - j * dc;
            row = start_row - j * dr;

            line[j] = a->board[col][row];
        }

        slide_line(line);

        for (j = 0; j < LINE_SIZE; ++j) {
            col = start_col - j * dc;
            row = start_row - j * dr;
            old_cell = &a->board[col][row];
            new_cell = &line[j];

            if (new_cell->exp != old_cell->exp || new_cell->flashed) {
                old_cell->exp = new_cell->exp;
                old_cell->flashed = new_cell->flashed;
                draw_cell(col, row);
                changed = 1;
            }
        }
    }

    return changed;
}

static void
restart_game(void)
{
    app_state_st *a = app_state;

    a->game_over = 0;
    a->game_won = 0;
    a->score = 0;
    a->flash_ticks = 0;

    clear_board();
    set_random_cell();
    set_random_cell();

    update_status();
}

static void
make_move(int col_step, int row_step)
{
    app_state_st *a = app_state;

    clear_flash();

    if (!slide_board(col_step, row_step)) {
        return;
    }

    a->flash_ticks = FLASH_TICKS;

    set_random_cell();

    a->game_won = has_won();
    a->game_over = !has_moves();

    update_status();
}

static void
on_tick(void)
{
    app_state_st *a = app_state;

    if (!a->flash_ticks) {
        return;
    }

    --a->flash_ticks;

    if (!a->flash_ticks) {
        clear_flash();
    }
}

static void
on_key_down(uint8_t key_code, uint8_t key_mods)
{
    app_state_st *a = app_state;

    if (key_code == KEY_R) {
        restart_game();
        return;
    }

    if (a->game_over || a->game_won) {
        return;
    }

    switch (key_code) {
    case KEY_LEFT: make_move(-1, 0); break;
    case KEY_RIGHT: make_move(1, 0); break;
    case KEY_UP: make_move(0, -1); break;
    case KEY_DOWN: make_move(0, 1); break;
    default: break;
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

    gui_window_draw(&a->window, gui_color_fg, 1);
    gui_status_set_br("R: Restart");

    restart_game();

    if (!a->skip_intro) {
        gui_status_set("Slide and merge tiles to reach 2048");
        a->skip_intro = 1;
    }
}

static void
on_init(void)
{
    ASSERT(sizeof(app_state_st) <= sizeof(gui_app_shared_buffer));

    app_2048.on_show = on_show;
    app_2048.on_key_down = on_key_down;
    app_2048.on_tick = on_tick;
}

global app_st app_2048 = {
    "2048",
    &icon_2048,
    on_init,
};
