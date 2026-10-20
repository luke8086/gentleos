/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: sokoban.c - Sokoban game
 */

#include <gui.h>

typedef struct {
    uint16_t count;
    /* Followed by {count} levels, each starting with a sokoban_level_st */
} sokoban_levels_st;

typedef struct {
    uint8_t cols;
    uint8_t rows;
    /* Followed by {cols * rows} tiles, row by row */
} sokoban_level_st;

enum {
    TILE_SIZE = 9,
    BOARD_MAX_COLS = 30,
    BOARD_MAX_ROWS = 18,
};

enum {
    TILE_WALL = 0x01,
    TILE_GOAL = 0x02,
    TILE_BOX = 0x04,
    TILE_MAN = 0x08,
};

typedef struct {
    window_st window;

    const sokoban_levels_st far *levels;

    uint8_t board[BOARD_MAX_ROWS][BOARD_MAX_COLS];
    int cols;
    int rows;
    int cur_x;
    int cur_y;
    unsigned moves;
    int solved;
} app_state_st;

static int cur_level = 0;
static app_state_st *app_state = (app_state_st *)gui_app_shared_buffer;

static void
update_status(void)
{
    app_state_st *a = app_state;
    int count = a->levels->count;

    if (a->solved) {
        gui_status_set("Level %d/%d solved in %u moves!", cur_level + 1, count, a->moves);
    } else {
        gui_status_set("Level %d/%d  Moves: %u", cur_level + 1, count, a->moves);
    }
}

static void
update_help(void)
{
    app_state_st *a = app_state;

    if (a->solved) {
        gui_status_set_br("Enter: Next level");
    } else {
        gui_status_set_br("R: Restart  PgUp/PgDn: Level");
    }
}


static int
is_solved(void)
{
    app_state_st *a = app_state;
    int x, y;

    for (y = 0; y < a->rows; ++y) {
        for (x = 0; x < a->cols; ++x) {
            if ((a->board[y][x] & (TILE_BOX | TILE_GOAL)) == TILE_BOX) {
                return 0;
            }
        }
    }

    return 1;
}

static void
draw_tile(int x, int y)
{
    app_state_st *a = app_state;
    uint8_t tile = a->board[y][x];
    bitmap_st *sprite = NULL;
    rect_st rect;

    if (tile & TILE_WALL) {
        sprite = &sprite_sb_wall;
    } else if (tile & TILE_MAN) {
        sprite = &sprite_sb_man;
    } else if (tile & TILE_BOX) {
        sprite = (tile & TILE_GOAL) ? &sprite_sb_box_goal : &sprite_sb_box;
    } else if (tile & TILE_GOAL) {
        sprite = &sprite_sb_goal;
    }

    gui_rect_init(&rect, x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
    gui_surface_draw_rect(&a->window.origin, &rect, gui_color_bg);

    if (sprite) {
        gui_surface_draw_bitmap(&a->window.origin, rect.x, rect.y, sprite, gui_color_fg);
    }

    if (tile & TILE_WALL && (x == 0 || (x > 0 && !(a->board[y][x-1] & TILE_WALL)))) {
        gui_surface_draw_v_seg(&a->window.origin, rect.x, rect.y + 5, 4, gui_color_fg);
    }

    gui_surface_mark_dirty(&a->window.origin, &rect);
}

static void
draw_board(void)
{
    app_state_st *a = app_state;
    int x, y;

    gui_surface_draw_rect(&GUI_POINT_ZERO, &gui_app_rect, gui_color_bg);
    gui_surface_mark_dirty(&GUI_POINT_ZERO, &gui_app_rect);

    for (y = 0; y < a->rows; ++y) {
        for (x = 0; x < a->cols; ++x) {
            if (a->board[y][x]) {
                draw_tile(x, y);
            }
        }
    }
}

static const sokoban_level_st far *
find_level(int index)
{
    app_state_st *a = app_state;
    const sokoban_level_st far *level;
    const uint8_t far *tiles;

    if (!a->levels || index < 0 || index >= a->levels->count) {
        return NULL;
    }

    level = (const sokoban_level_st far *)(a->levels + 1);

    while (index) {
        tiles = (const uint8_t far *)(level + 1);
        level = (const sokoban_level_st far *)(tiles + level->cols * level->rows);
        --index;
    }

    return level;
}

static void
load_level(int index)
{
    app_state_st *a = app_state;
    const sokoban_level_st far *level;
    const uint8_t far *tiles;
    int x, y;

    gui_status_set_urgent("Loading...");

    cur_level = index;
    level = find_level(cur_level);
    ASSERT(level && level->cols <= BOARD_MAX_COLS && level->rows <= BOARD_MAX_ROWS);

    tiles = (const uint8_t far *)(level + 1);

    a->cols = level->cols;
    a->rows = level->rows;
    a->moves = 0;
    a->solved = 0;

    for (y = 0; y < a->rows; ++y) {
        for (x = 0; x < a->cols; ++x) {
            a->board[y][x] = *tiles;

            if (a->board[y][x] & TILE_MAN) {
                a->cur_x = x;
                a->cur_y = y;
            }

            ++tiles;
        }
    }

    gui_window_init(&a->window, a->cols * TILE_SIZE, a->rows * TILE_SIZE);
    draw_board();
    update_status();
    update_help();
}

static void
switch_level(int delta)
{
    app_state_st *a = app_state;
    int count = a->levels->count;

    load_level((cur_level + count + delta) % count);
}

/* Levels are assumed to be enclosed by walls, so no bounds checks here */
static void
move_man(int dx, int dy)
{
    app_state_st *a = app_state;
    int next_x = a->cur_x + dx;
    int next_y = a->cur_y + dy;
    int n_next_x = next_x + dx;
    int n_next_y = next_y + dy;
    uint8_t next_tile = a->board[next_y][next_x];

    if (next_tile & TILE_WALL) {
        return;
    }

    if (next_tile & TILE_BOX) {
        if (a->board[n_next_y][n_next_x] & (TILE_WALL | TILE_BOX)) {
            return;
        }

        a->board[next_y][next_x] &= (uint8_t)~TILE_BOX;
        a->board[n_next_y][n_next_x] |= TILE_BOX;
        draw_tile(n_next_x, n_next_y);
    }

    a->board[a->cur_y][a->cur_x] &= (uint8_t)~TILE_MAN;
    a->board[next_y][next_x] |= TILE_MAN;

    draw_tile(a->cur_x, a->cur_y);
    draw_tile(next_x, next_y);

    a->cur_x = next_x;
    a->cur_y = next_y;
    ++a->moves;

    if ((next_tile & TILE_BOX) && is_solved()) {
        a->solved = 1;
        update_help();
    }

    update_status();
}

static void
on_key_down(uint8_t key_code, uint8_t key_mods)
{
    app_state_st *a = app_state;

    if (!a->levels) {
        return;
    }

    switch (key_code) {
    case KEY_PGUP: switch_level(-1); return;
    case KEY_PGDN: switch_level(1); return;
    case KEY_R: load_level(cur_level); return;
    }

    if (a->solved) {
        if (key_code == KEY_ENTER) {
            switch_level(1);
        }

        return;
    }

    switch (key_code) {
    case KEY_UP: move_man(0, -1); return;
    case KEY_DOWN: move_man(0, 1); return;
    case KEY_LEFT: move_man(-1, 0); return;
    case KEY_RIGHT: move_man(1, 0); return;
    }
}

static void
on_show(void)
{
    app_state_st *a = app_state;
    file_st far *file;
    sokoban_levels_st far *levels;

    file = file_lookup("LEVELS.SBN");
    levels = file ? (sokoban_levels_st far *)file->u.addr : 0;

    if (!levels || !levels->count) {
        gui_status_set("No levels found");
        return;
    }

    a->levels = file->u.addr;

    load_level(cur_level);
}

static void
on_init(void)
{
    ASSERT(sizeof(app_state_st) <= sizeof(gui_app_shared_buffer));

    app_sokoban.on_show = on_show;
    app_sokoban.on_key_down = on_key_down;
}

global app_st app_sokoban = {
    "Sokoban",
    &icon_sokoban,
    on_init,
};
