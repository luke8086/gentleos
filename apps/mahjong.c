/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: mahjong.c - Mahjong game
 */

#include <gui.h>

enum {
    TILE_W = 22,
    TILE_H = 20,
    TILE_D = 2,

    TILE_TYPE_COUNT = 36,
    TILES_PER_TYPE = 4,
    TILE_EMPTY = 0,
    TILE_COUNT = 144,

    BOARD_LAYERS = 4,
    BOARD_COLS = 14,
    BOARD_ROWS = 8,

    WINDOW_WIDTH = BOARD_COLS * (TILE_W - 1) + 1 + TILE_D,
    WINDOW_HEIGHT = BOARD_ROWS * (TILE_H - 1) + 1 + TILE_D,

    STATE_DEFAULT = 0,
    STATE_WON = 1,
    STATE_STUCK = 2,
};

static bitmap_st *tile_bitmaps[] = {
    NULL,
    &sprite_mj_ci_1, &sprite_mj_ci_2, &sprite_mj_ci_3, &sprite_mj_ci_4,
    &sprite_mj_ci_5, &sprite_mj_ci_6, &sprite_mj_ci_7, &sprite_mj_ci_8, &sprite_mj_ci_9,
    &glyph_mn_bbchick, &sprite_mj_ba_2, &sprite_mj_ba_3, &sprite_mj_ba_4,
    &sprite_mj_ba_5, &sprite_mj_ba_6, &sprite_mj_ba_7, &sprite_mj_ba_8, &sprite_mj_ba_9,
    &glyph_mn_num_1, &glyph_mn_num_2, &glyph_mn_num_3, &glyph_mn_num_4,
    &glyph_mn_num_5, &glyph_mn_num_6, &glyph_mn_num_7, &glyph_mn_num_8, &glyph_mn_num_9,
    &glyph_mn_east, &glyph_mn_south, &glyph_mn_west, &glyph_mn_north,
    &glyph_mn_white, &glyph_mn_issue, &glyph_mn_central,
    &glyph_mn_tulip, &glyph_mn_herb,
};

/* Layer 0: 88, Layer 1: 44, Layer 2: 8, Layer 3: 4 */
static const uint8_t board_layout[BOARD_LAYERS][BOARD_ROWS][BOARD_COLS] = {
    {
        {0,0,0,1,1,1,1,1,1,1,1,0,0,0},
        {0,0,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,1,1,1,1,1,1,1,1,1,1,1,1,0},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {0,1,1,1,1,1,1,1,1,1,1,1,1,0},
        {0,0,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,0,1,1,1,1,1,1,1,1,0,0,0},
    },
    {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,1,1,1,1,1,1,0,0,0,0},
        {0,0,0,1,1,1,1,1,1,1,1,0,0,0},
        {0,0,0,1,1,1,1,1,1,1,1,0,0,0},
        {0,0,0,1,1,1,1,1,1,1,1,0,0,0},
        {0,0,0,1,1,1,1,1,1,1,1,0,0,0},
        {0,0,0,0,1,1,1,1,1,1,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    },
    {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,1,1,1,1,0,0,0,0,0},
        {0,0,0,0,0,1,1,1,1,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    },
    {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,1,1,0,0,0,0,0,0},
        {0,0,0,0,0,0,1,1,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    },
};

typedef struct {
    uint8_t board[BOARD_LAYERS][BOARD_ROWS][BOARD_COLS];
    uint8_t dirty[BOARD_LAYERS][BOARD_ROWS][BOARD_COLS];
} app_heap_st;

typedef struct {
    window_st window;

    int cur_col;
    int cur_row;

    int sel_col;
    int sel_row;
    int sel_layer;

    int remaining_pairs;
    int valid_moves;
    int state;
} app_state_st;

static app_state_st *app_state = (app_state_st *)gui_app_shared_buffer;
static app_heap_st far *app_heap;

static void
update_status(void)
{
    app_state_st *a = app_state;

    if (a->state == STATE_WON) {
        gui_status_set("You Won! Press R to play again");
        gui_status_set_br("");
    } else {
        gui_status_set("Pairs: %d  Moves: %d", a->remaining_pairs, a->valid_moves);
        gui_status_set_br("S: Shuffle  R: Restart");
    }
}

static int
topmost_layer_at(int col, int row)
{
    app_heap_st far *h = app_heap;
    int layer;

    for (layer = BOARD_LAYERS - 1; layer >= 0; layer--) {
        if (h->board[layer][row][col] != TILE_EMPTY) {
            return layer;
        }
    }

    return -1;
}

static int
is_tile_free(int layer, int col, int row)
{
    app_heap_st far *h = app_heap;
    int left_empty, right_empty;

    if (h->board[layer][row][col] == TILE_EMPTY) {
        return 0;
    }

    if (layer != topmost_layer_at(col, row)) {
        return 0;
    }

    left_empty = (col == 0 || h->board[layer][row][col - 1] == TILE_EMPTY);
    right_empty = (col == BOARD_COLS - 1 || h->board[layer][row][col + 1] == TILE_EMPTY);

    return left_empty || right_empty;
}

static int
count_valid_moves(void)
{
    app_heap_st far *h = app_heap;
    uint8_t free_counts[TILE_TYPE_COUNT + 1];
    int layer, col, row;
    uint8_t type;
    int total = 0;

    memset(free_counts, 0, sizeof(free_counts));

    for (layer = 0; layer < BOARD_LAYERS; ++layer) {
        for (row = 0; row < BOARD_ROWS; ++row) {
            for (col = 0; col < BOARD_COLS; ++col) {
                if (is_tile_free(layer, col, row)) {
                    type = h->board[layer][row][col];
                    ++free_counts[type];
                }
            }
        }
    }

    for (type = 1; type <= TILE_TYPE_COUNT; ++type) {
        total += free_counts[type] / 2;
    }

    return total;
}

static void
draw_tile(int layer, int col, int row)
{
    app_state_st *a = app_state;
    app_heap_st far *h = app_heap;
    uint8_t type = h->board[layer][row][col];
    int x = col * (TILE_W - 1) - layer * TILE_D;
    int y = row * (TILE_H - 1) - layer * TILE_D;
    int is_cursor;
    int is_selected;
    uint8_t face_color;
    uint8_t glyph_color;
    rect_st rect;
    point_st *origin = &a->window.origin;
    int has_right;
    int has_bottom;
    int has_diag;

    gui_rect_init(&rect, x, y, TILE_W + TILE_D, TILE_H + TILE_D);

    if (type == TILE_EMPTY) {
        gui_surface_draw_rect(origin, &rect, gui_color_bg);
        gui_surface_mark_dirty(origin, &rect);
        return;
    }

    is_cursor = (col == a->cur_col && row == a->cur_row && layer == topmost_layer_at(col, row));
    is_selected = (col == a->sel_col && row == a->sel_row && layer == a->sel_layer);

    face_color = is_selected ? gui_color_fg : gui_color_bg;
    glyph_color = is_selected ? gui_color_bg : gui_color_fg;

    has_right = (col < BOARD_COLS - 1 && h->board[layer][row][col + 1] != TILE_EMPTY);
    has_bottom = (row < BOARD_ROWS - 1 && h->board[layer][row + 1][col] != TILE_EMPTY);
    has_diag = (col < BOARD_COLS - 1 && row < BOARD_ROWS - 1 &&
        h->board[layer][row + 1][col + 1] != TILE_EMPTY);

    gui_rect_init(&rect, x, y, TILE_W, TILE_H);
    gui_surface_draw_rect(origin, &rect, face_color);
    gui_surface_draw_border(origin, &rect, gui_color_fg);
    gui_surface_draw_bitmap_centered(origin, &a->window.size,
        &rect, tile_bitmaps[type], glyph_color);

    if (is_cursor) {
        gui_rect_init(&rect, x + 2, y + 2, TILE_W - 4, TILE_H - 4);
        gui_surface_draw_border(origin, &rect, glyph_color);
    }

    if (!has_right) {
        gui_rect_init(&rect, x + TILE_W, y + 2, 2, TILE_H - 2);
        gui_surface_draw_rect(origin, &rect, gui_color_fg);
        gui_surface_draw_pixel(origin, x + TILE_W, y + 1, gui_color_fg);
    }

    if (!has_bottom) {
        gui_rect_init(&rect, x + 2, y + TILE_H, TILE_W - 2, 2);
        gui_surface_draw_rect(origin, &rect, gui_color_fg);
        gui_surface_draw_pixel(origin, x + 1, y + TILE_H, gui_color_fg);
    }

    if (!has_diag) {
        gui_rect_init(&rect, x + TILE_W, y + TILE_H, 2, 2);
        gui_surface_draw_rect(origin, &rect, gui_color_fg);
    }

    gui_rect_init(&rect, x, y, TILE_W + TILE_D, TILE_H + TILE_D);
    gui_surface_mark_dirty(origin, &rect);
}

static void
draw_empty_cursor(int col, int row, uint8_t color)
{
    app_state_st *a = app_state;
    /* At this size it won't draw over adjacent tiles up to 4th layer */
    int size = 4;
    int x = col * (TILE_W - 1) + (TILE_W - size) / 2;
    int y = row * (TILE_H - 1) + (TILE_H - size) / 2;
    rect_st rect;

    gui_rect_init(&rect, x, y, size, size);
    gui_surface_draw_rect(&a->window.origin, &rect, color);
    gui_surface_mark_dirty(&a->window.origin, &rect);
}

static void
draw_dirty_tiles(void)
{
    app_heap_st far *h = app_heap;
    int layer, row, col;

    /* Clear empty tiles */
    for (layer = 0; layer < BOARD_LAYERS; ++layer) {
        for (row = 0; row < BOARD_ROWS; ++row) {
            for (col = 0; col < BOARD_COLS; ++col) {
                if (!h->dirty[layer][row][col] || h->board[layer][row][col] != TILE_EMPTY) {
                    continue;
                }

                draw_tile(layer, col, row);
                h->dirty[layer][row][col] = 0;
            }
        }
    }

    /* Draw present tiles */
    for (layer = 0; layer < BOARD_LAYERS; ++layer) {
        for (row = 0; row < BOARD_ROWS; ++row) {
            for (col = 0; col < BOARD_COLS; ++col) {
                if (!h->dirty[layer][row][col]) {
                    continue;
                }

                draw_tile(layer, col, row);
                h->dirty[layer][row][col] = 0;
            }
        }
    }
}

static void
mark_tile_dirty(int layer, int col, int row)
{
    app_heap_st far *h = app_heap;
    static const int8_t dc[3] = {1, 0, 1};
    static const int8_t dr[3] = {0, 1, 1};
    int i, nl, nc, nr;

    if (col >= BOARD_COLS || row >= BOARD_ROWS || h->board[layer][row][col] == TILE_EMPTY) {
        return;
    }

    h->dirty[layer][row][col] = 1;

    /*
     * Recursively mark as dirty the bottom, right and bottom-right tiles
     * in higher layers, since they're drawn over the original cell
     */
    for (i = 0; i < 3; ++i) {
        nc = col + dc[i];
        nr = row + dr[i];

        if (nc >= BOARD_COLS || nr >= BOARD_ROWS) {
            continue;
        }

        for (nl = layer + 1; nl < BOARD_LAYERS; ++nl) {
            mark_tile_dirty(nl, nc, nr);
        }
    }
}

static void
redraw_board(void)
{
    app_state_st *a = app_state;
    app_heap_st far *h = app_heap;
    int layer, row, col;
    rect_st rect;

    gui_rect_init(&rect, 0, 0, a->window.size.width, a->window.size.height);
    gui_surface_draw_rect(&a->window.origin, &rect, gui_color_bg);
    gui_surface_mark_dirty(&a->window.origin, &rect);

    for (layer = 0; layer < BOARD_LAYERS; ++layer) {
        for (row = 0; row < BOARD_ROWS; ++row) {
            for (col = 0; col < BOARD_COLS; ++col) {
                h->dirty[layer][row][col] = (h->board[layer][row][col] != TILE_EMPTY);
            }
        }
    }

    draw_dirty_tiles();

    if (topmost_layer_at(a->cur_col, a->cur_row) < 0) {
        draw_empty_cursor(a->cur_col, a->cur_row, gui_color_fg);
    }
}

static void
remove_tile(int layer, int col, int row)
{
    app_state_st *a = app_state;
    app_heap_st far *h = app_heap;
    int nl, nc, nr;

    h->board[layer][row][col] = TILE_EMPTY;
    h->dirty[layer][row][col] = 1;

    /* All adjacent tiles need to be redrawn */
    for (nl = 0; nl < BOARD_LAYERS; ++nl) {
        for (nr = row - 1; nr <= row + 1; ++nr) {
            for (nc = col - 1; nc <= col + 1; ++nc) {
                if (nc < 0 || nc >= BOARD_COLS || nr < 0 || nr >= BOARD_ROWS) {
                    continue;
                }

                mark_tile_dirty(nl, nc, nr);
            }
        }
    }

    draw_dirty_tiles();

    if (a->cur_col == col && a->cur_row == row && topmost_layer_at(col, row) < 0) {
        draw_empty_cursor(col, row, gui_color_fg);
    }
}

static void
shuffle_tiles(void)
{
    app_state_st *a = app_state;
    app_heap_st far *h = app_heap;
    uint8_t deck[TILE_COUNT];
    uint8_t tmp;
    int count = 0;
    int layer, col, row, i, j;

    if (a->state == STATE_WON) {
        return;
    }

    for (layer = 0; layer < BOARD_LAYERS; ++layer) {
        for (row = 0; row < BOARD_ROWS; ++row) {
            for (col = 0; col < BOARD_COLS; ++col) {
                if (h->board[layer][row][col] != TILE_EMPTY) {
                    deck[count++] = h->board[layer][row][col];
                }
            }
        }
    }

    for (i = count - 1; i > 0; --i) {
        j = rand() % (i + 1);
        tmp = deck[i];
        deck[i] = deck[j];
        deck[j] = tmp;
    }

    i = 0;
    for (layer = 0; layer < BOARD_LAYERS; ++layer) {
        for (row = 0; row < BOARD_ROWS; ++row) {
            for (col = 0; col < BOARD_COLS; ++col) {
                if (h->board[layer][row][col] != TILE_EMPTY) {
                    h->board[layer][row][col] = deck[i++];
                }
            }
        }
    }

    a->sel_col = -1;
    a->valid_moves = count_valid_moves();
    a->state = a->valid_moves > 0 ? STATE_DEFAULT : STATE_STUCK;

    redraw_board();
    update_status();
}

static void
init_tiles(void)
{
    app_heap_st far *h = app_heap;
    int layer, col, row, i = 0;

    for (layer = 0; layer < BOARD_LAYERS; layer++) {
        for (row = 0; row < BOARD_ROWS; row++) {
            for (col = 0; col < BOARD_COLS; col++) {
                if (board_layout[layer][row][col]) {
                    h->board[layer][row][col] = i / TILES_PER_TYPE + 1;
                    ++i;
                }
            }
        }
    }

    shuffle_tiles();
}

static void
select_tile(void)
{
    app_state_st *a = app_state;
    app_heap_st far *h = app_heap;
    int layer;
    int prev_col, prev_row, prev_layer;

    if (a->state != STATE_DEFAULT) {
        return;
    }

    layer = topmost_layer_at(a->cur_col, a->cur_row);

    if (layer < 0 || !is_tile_free(layer, a->cur_col, a->cur_row)) {
        return;
    }

    /* Selecting already selected tile */
    if (a->sel_col == a->cur_col && a->sel_row == a->cur_row && a->sel_layer == layer) {
        a->sel_col = -1;
        mark_tile_dirty(layer, a->cur_col, a->cur_row);
        draw_dirty_tiles();
        update_status();
        return;
    }

    /* First pick */
    if (a->sel_col == -1) {
        a->sel_col = a->cur_col;
        a->sel_row = a->cur_row;
        a->sel_layer = layer;
        mark_tile_dirty(layer, a->cur_col, a->cur_row);
        draw_dirty_tiles();
        update_status();
        return;
    }

    /* Second pick - no match */
    if (h->board[layer][a->cur_row][a->cur_col] != h->board[a->sel_layer][a->sel_row][a->sel_col]) {
        prev_col = a->sel_col;
        prev_row = a->sel_row;
        prev_layer = a->sel_layer;
        a->sel_col = -1;
        mark_tile_dirty(prev_layer, prev_col, prev_row);
        draw_dirty_tiles();
        gui_status_set("No match");
        return;
    }

    /* Second pick - match */
    prev_col = a->sel_col;
    prev_row = a->sel_row;
    prev_layer = a->sel_layer;
    a->sel_col = -1;
    --a->remaining_pairs;

    remove_tile(layer, a->cur_col, a->cur_row);
    remove_tile(prev_layer, prev_col, prev_row);

    a->valid_moves = count_valid_moves();

    if (a->remaining_pairs == 0) {
        a->state = STATE_WON;
    } else if (a->valid_moves == 0) {
        a->state = STATE_STUCK;
    }

    update_status();
}

static void
update_cursor(int dx, int dy)
{
    app_state_st *a = app_state;
    int old_col = a->cur_col;
    int old_row = a->cur_row;
    int old_layer = topmost_layer_at(old_col, old_row);
    int new_layer;

    a->cur_col = MAX(0, MIN(BOARD_COLS - 1, a->cur_col + dx));
    a->cur_row = MAX(0, MIN(BOARD_ROWS - 1, a->cur_row + dy));

    if (old_col == a->cur_col && old_row == a->cur_row) {
        return;
    }

    new_layer = topmost_layer_at(a->cur_col, a->cur_row);

    if (old_layer >= 0) {
        mark_tile_dirty(old_layer, old_col, old_row);
    } else {
        draw_empty_cursor(old_col, old_row, gui_color_bg);
    }

    if (new_layer >= 0) {
        mark_tile_dirty(new_layer, a->cur_col, a->cur_row);
    } else {
        draw_empty_cursor(a->cur_col, a->cur_row, gui_color_fg);
    }

    if (old_layer >= 0 || new_layer >= 0) {
        draw_dirty_tiles();
    }
}

static void
restart_game(void)
{
    app_state_st *a = app_state;

    a->cur_col = BOARD_COLS / 2;
    a->cur_row = BOARD_ROWS / 2;
    a->sel_col = -1;
    a->remaining_pairs = TILE_COUNT / 2;
    a->state = STATE_DEFAULT;

    init_tiles();
}

static void
on_key_down(uint8_t key_code, uint8_t key_mods)
{
    app_state_st *a = app_state;

    if (key_code == KEY_R) {
        restart_game();
        return;
    }

    if (a->state == STATE_WON) {
        return;
    }

    switch (key_code) {
        case KEY_S: shuffle_tiles(); return;
        case KEY_LEFT: update_cursor(-1, 0); return;
        case KEY_RIGHT: update_cursor(1, 0); return;
        case KEY_UP: update_cursor(0, -1); return;
        case KEY_DOWN: update_cursor(0, 1); return;
        case KEY_SPACE:
        case KEY_ENTER: select_tile(); return;
    }
}

static void
on_show(void)
{
    app_state_st *a = app_state;
    app_heap_st far *h = app_heap;

    memset_far(h, 0, sizeof(app_heap_st));

    gui_window_init(&a->window, WINDOW_WIDTH, WINDOW_HEIGHT);

    restart_game();
}

static void
on_init(void)
{
    ASSERT(sizeof(app_state_st) <= sizeof(gui_app_shared_buffer));

    app_heap = heap_alloc(sizeof(app_heap_st));

    app_mahjong.on_show = on_show;
    app_mahjong.on_key_down = on_key_down;
}

global app_st app_mahjong = {
    "Mahjong",
    &glyph_mn_east_icn,
    on_init,
};
