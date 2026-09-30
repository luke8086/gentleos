/*
 * Copyright (c) 2025-2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: snake.c - Snake game
 */

#include <gui.h>

enum {
    GRID_CELL_WIDTH = 6,
    GRID_CELL_HEIGHT = 6,
    GRID_ROWS = 16,
    GRID_COLS = 28,
    GRID_COUNT = GRID_ROWS * GRID_COLS,
    GRID_WIDTH = GRID_WIDTH_SPACED(GRID_CELL_WIDTH, GRID_COLS),
    GRID_HEIGHT = GRID_HEIGHT_SPACED(GRID_CELL_HEIGHT, GRID_ROWS),
    GRID_X = 1,
    GRID_Y = 1,

    WINDOW_WIDTH = GRID_X + GRID_WIDTH + 1,
    WINDOW_HEIGHT = GRID_Y + GRID_HEIGHT + 1,

    MOVE_TICKS = MSECS_TO_TICKS(150),
};

enum {
    CELL_FLOOR = 0,
    CELL_WALL = 1,
    CELL_SNAKE = 2,
    CELL_FRUIT = 3,
};

enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
};

typedef struct {
    int8_t x, y;
} coords_st;

typedef struct {
    window_st window;
    grid_st grid;

    uint8_t cell_colors[4];
    uint8_t cells[GRID_COLS][GRID_ROWS];

    struct {
        coords_st far *head;
        coords_st far *tail;
        int grow;
    } body;

    int prev_dir;
    int next_dir;

    int score;
    int game_over;
    int game_paused;
} app_state_st;

static int best_score;
static coords_st far *body_coords;
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

    gui_status_set("%sScore: %d  Best: %d", msg, a->score, best_score);
}

static void
draw_cell(int x, int y, uint8_t cell_type)
{
    app_state_st *a = app_state;
    rect_st r;

    a->cells[x][y] = cell_type;

    gui_grid_cell_rect(&a->grid, x, y, &r);
    gui_surface_draw_rect(&a->window.origin, &r, a->cell_colors[cell_type]);
    gui_surface_mark_dirty(&a->window.origin, &r);
}

static void
draw_region(int x, int y, int w, int h, uint8_t cell_type)
{
    int i, j;

    for (j = 0; j < h; ++j) {
        for (i = 0; i < w; ++i) {
            draw_cell(x + i, y + j, cell_type);
        }
    }
}

static void
draw_board(void)
{
    draw_region(0, 0, GRID_COLS, GRID_ROWS, CELL_FLOOR);
}

static void
add_fruit(void) {
    app_state_st *a = app_state;
    coords_st c;

    do {
        c.x = rand() % GRID_COLS;
        c.y = rand() % GRID_ROWS;
    } while (a->cells[c.x][c.y] != CELL_FLOOR);

    draw_cell(c.x, c.y, CELL_FRUIT);
}

static coords_st
move_head(coords_st head)
{
    app_state_st *a = app_state;

    switch (a->next_dir) {
    case DIR_UP:    head.y--; break;
    case DIR_DOWN:  head.y++; break;
    case DIR_LEFT:  head.x--; break;
    case DIR_RIGHT: head.x++; break;
    }

    return head;
}

static void
move_snake(coords_st next_head)
{
    app_state_st *a = app_state;
    coords_st far *c;

    if (a->body.grow) {
        ++a->body.tail;
        --a->body.grow;
    } else {
        draw_cell(a->body.tail->x, a->body.tail->y, CELL_FLOOR);
    }

    for (c = a->body.tail; c != a->body.head; --c) {
        *c = *(c - 1);
    }

    *(a->body.head) = next_head;

    draw_cell(next_head.x, next_head.y, CELL_SNAKE);
}

static void
end_game(void)
{
    app_state_st *a = app_state;

    a->game_over = 1;

    if (a->score > best_score) {
        best_score = a->score;
    }

    update_status();
}

static void
restart_game(void)
{
    app_state_st *a = app_state;

    a->score = 0;
    a->game_over = 0;
    a->game_paused = 0;

    body_coords[0].x = GRID_COLS / 2;
    body_coords[0].y = GRID_ROWS / 2;
    a->body.head = a->body.tail = body_coords;
    a->body.grow = 7;

    a->prev_dir = DIR_RIGHT;
    a->next_dir = DIR_RIGHT;

    draw_board();
    add_fruit();
    update_status();
}

static void
on_timeout(void)
{
    app_state_st *a = app_state;
    coords_st next_head;
    uint8_t next_block;

    if (a->game_over || a->game_paused) {
        return;
    }

    next_head = move_head(*a->body.head);

    if (next_head.x < 0 || next_head.x >= GRID_COLS ||
        next_head.y < 0 || next_head.y >= GRID_ROWS) {
        end_game();
        return;
    }

    next_block = a->cells[next_head.x][next_head.y];

    if (next_block != CELL_FRUIT && next_block != CELL_FLOOR) {
        end_game();
        return;
    }

    if (next_block == CELL_FRUIT) {
        a->body.grow += 2;
        a->score += 5;
        update_status();
    }

    move_snake(next_head);

    if (next_block == CELL_FRUIT) {
        add_fruit();
    }

    a->prev_dir = a->next_dir;
}

static void
on_tick(void)
{
    static unsigned count = 0;

    ++count;

    if (count >= MOVE_TICKS) {
        on_timeout();
        count = 0;
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

    if (key_code == KEY_UP && a->prev_dir != DIR_DOWN) a->next_dir = DIR_UP;
    else if (key_code == KEY_DOWN && a->prev_dir != DIR_UP) a->next_dir = DIR_DOWN;
    else if (key_code == KEY_LEFT && a->prev_dir != DIR_RIGHT) a->next_dir = DIR_LEFT;
    else if (key_code == KEY_RIGHT && a->prev_dir != DIR_LEFT) a->next_dir = DIR_RIGHT;
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

    a->cell_colors[CELL_FLOOR] = gui_color_bg;
    a->cell_colors[CELL_WALL] = gui_color_fg;
    a->cell_colors[CELL_SNAKE] = gui_color_fg;
    a->cell_colors[CELL_FRUIT] = gui_color_fg;

    gui_window_draw(&a->window, gui_color_bg, 1);
    gui_status_set_br("P: Pause/Resume");
    restart_game();
}

static void
on_init(void)
{
    ASSERT(sizeof(app_state_st) <= sizeof(gui_app_shared_buffer));

    body_coords = heap_alloc(GRID_COUNT * sizeof(coords_st));

    app_snake.on_show = on_show;
    app_snake.on_tick = on_tick;
    app_snake.on_key_down = on_key_down;
}

global app_st app_snake = {
    "Snake",
    &icon_snake,
    on_init,
};
