/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: pairs.c - Pair matching / Memory game
 */

#include <gui.h>

enum {
    GRID_CELL_WIDTH = 24,
    GRID_CELL_HEIGHT = 24,
    GRID_ROWS = 5,
    GRID_COLS = 8,
    GRID_CELL_COUNT = GRID_ROWS * GRID_COLS,
    GRID_WIDTH = GRID_WIDTH_SPACED(GRID_CELL_WIDTH, GRID_COLS),
    GRID_HEIGHT = GRID_HEIGHT_SPACED(GRID_CELL_HEIGHT, GRID_ROWS),
    GRID_X = 1,
    GRID_Y = 1,

    WINDOW_WIDTH = GRID_X + GRID_WIDTH + 1,
    WINDOW_HEIGHT = GRID_Y + GRID_HEIGHT + 1,

    PAIR_COUNT = GRID_CELL_COUNT / 2,

    MISMATCH_TICKS = MSECS_TO_TICKS(1000),
};

static bitmap_st *icons[PAIR_COUNT] = {
    &glyph_mn_beaver,
    &glyph_mn_cactus,
    &glyph_mn_dolphin,
    &glyph_mn_drmcamel,
    &glyph_mn_elephant,
    &glyph_mn_flamingo,
    &glyph_mn_horsefac,
    &glyph_mn_monkey,
    &glyph_mn_mushroom,
    &glyph_mn_octopus,
    &glyph_mn_palmtree,
    &glyph_mn_pandafac,
    &glyph_mn_pumpkin,
    &glyph_mn_rabbit,
    &glyph_mn_robotfac,
    &glyph_mn_sloth,
    &glyph_mn_snail,
    &glyph_mn_tigerfac,
    &glyph_mn_trex,
    &glyph_mn_tulip,
};

enum {
    CELL_STATE_HIDDEN = 0,
    CELL_STATE_REVEALED = 1,
    CELL_STATE_MATCHED = 2,
};

typedef struct {
    window_st window;
    grid_st grid;

    uint8_t cell_icons[GRID_CELL_COUNT];
    uint8_t cell_states[GRID_CELL_COUNT];

    int current_col;
    int current_row;

    int first_pick;
    int second_pick;
    int tries;
    int matched_count;
    int waiting;
} app_state_st;

static app_state_st *app_state = (app_state_st *)gui_app_shared_buffer;

static void
shuffle_icons(void)
{
    app_state_st *a = app_state;
    uint8_t deck[GRID_CELL_COUNT];
    int i, j;
    uint8_t tmp;

    for (i = 0; i < PAIR_COUNT; i++) {
        deck[i * 2] = i;
        deck[i * 2 + 1] = i;
    }

    for (i = GRID_CELL_COUNT - 1; i > 0; i--) {
        j = rand() % (i + 1);
        tmp = deck[i];
        deck[i] = deck[j];
        deck[j] = tmp;
    }

    for (i = 0; i < GRID_CELL_COUNT; i++) {
        a->cell_icons[i] = deck[i];
    }
}

static void
draw_cursor(int col, int row, uint8_t color)
{
    app_state_st *a = app_state;
    rect_st rect;

    gui_grid_cell_rect(&a->grid, col, row, &rect);
    gui_rect_shrink(&rect, 1);
    gui_surface_draw_border(&a->window.origin, &rect, color);
    gui_surface_mark_dirty(&a->window.origin, &rect);
}

static void
draw_cell(int col, int row)
{
    app_state_st *a = app_state;
    int idx = row * GRID_COLS + col;
    uint8_t state = a->cell_states[idx];
    rect_st rect;

    gui_grid_cell_rect(&a->grid, col, row, &rect);
    gui_surface_draw_rect(&a->window.origin, &rect, gui_color_bg);

    if (state == CELL_STATE_REVEALED || state == CELL_STATE_MATCHED) {
        gui_surface_draw_bitmap_centered(&a->window.origin, &a->window.size, &rect,
            icons[a->cell_icons[idx]], gui_color_fg);
    }

    if (col == a->current_col && row == a->current_row) {
        draw_cursor(col, row, gui_color_fg);
    }

    gui_surface_mark_dirty(&a->window.origin, &rect);
}

static void
draw_cell_by_idx(int idx)
{
    draw_cell(idx % GRID_COLS, idx / GRID_COLS);
}

static void
reveal_icon(int idx)
{
    app_state_st *a = app_state;

    a->cell_states[idx] = CELL_STATE_REVEALED;
    draw_cell_by_idx(idx);
}

static void
hide_icon(int idx)
{
    app_state_st *a = app_state;

    a->cell_states[idx] = CELL_STATE_HIDDEN;
    draw_cell_by_idx(idx);
}

static void
update_status(void)
{
    app_state_st *a = app_state;
    int remaining = PAIR_COUNT - a->matched_count;

    if (a->matched_count == PAIR_COUNT) {
        gui_status_set("You won after %d tries! Press R to play again", a->tries);
    } else {
        gui_status_set("Remaining pairs: %d  Tries: %d", remaining, a->tries);
    }
}

static void
restart_game(void)
{
    app_state_st *a = app_state;
    int i;

    shuffle_icons();

    a->first_pick = -1;
    a->second_pick = -1;
    a->tries = 0;
    a->matched_count = 0;
    a->waiting = 0;

    for (i = 0; i < GRID_CELL_COUNT; i++) {
        a->cell_states[i] = CELL_STATE_HIDDEN;
        draw_cell_by_idx(i);
    }

    update_status();
}

static void
on_tick(void)
{
    app_state_st *a = app_state;

    if (!a->waiting) {
        return;
    }

    if (--a->waiting) {
        return;
    }

    hide_icon(a->first_pick);
    a->first_pick = -1;

    hide_icon(a->second_pick);
    a->second_pick = -1;

    a->waiting = 0;

    update_status();
}

static void
move_cursor(int dx, int dy)
{
    app_state_st *a = app_state;
    int prev_col = a->current_col;
    int prev_row = a->current_row;

    a->current_col = MAX(0, MIN(GRID_COLS - 1, a->current_col + dx));
    a->current_row = MAX(0, MIN(GRID_ROWS - 1, a->current_row + dy));

    draw_cursor(prev_col, prev_row, gui_color_bg);
    draw_cursor(a->current_col, a->current_row, gui_color_fg);
}

static void
on_enter(void)
{
    app_state_st *a = app_state;
    int idx = a->current_row * GRID_COLS + a->current_col;

    if (a->waiting) {
        return;
    }

    if (a->cell_states[idx] != CELL_STATE_HIDDEN) {
        return;
    }

    if (a->first_pick == -1) {
        a->first_pick = idx;
        reveal_icon(a->first_pick);
        return;
    }

    a->second_pick = idx;
    reveal_icon(a->second_pick);
    a->tries++;

    if (a->cell_icons[a->first_pick] == a->cell_icons[a->second_pick]) {
        a->cell_states[a->first_pick] = CELL_STATE_MATCHED;
        a->cell_states[a->second_pick] = CELL_STATE_MATCHED;
        a->first_pick = -1;
        a->second_pick = -1;
        a->matched_count++;
    } else {
        a->waiting = MISMATCH_TICKS;
    }

    update_status();
}

static void
on_key_down(uint8_t key_code, uint8_t key_mods)
{
    app_state_st *a = app_state;

    if (a->matched_count == PAIR_COUNT) {
        if (key_code == KEY_R) {
            restart_game();
            return;
        }

        return;
    }

    switch (key_code) {
        case KEY_LEFT: move_cursor(-1, 0); return;
        case KEY_RIGHT: move_cursor(1, 0); return;
        case KEY_UP: move_cursor(0, -1); return;
        case KEY_DOWN: move_cursor(0, 1); return;
        case KEY_SPACE:
        case KEY_ENTER: on_enter(); return;
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
    gui_status_set_br("Spc: Reveal");
    restart_game();
}

static void
on_init(void)
{
    ASSERT(sizeof(app_state_st) <= sizeof(gui_app_shared_buffer));

    app_pairs.on_show = on_show;
    app_pairs.on_tick = on_tick;
    app_pairs.on_key_down = on_key_down;
}

global app_st app_pairs = {
    "Pairs",
    &icon_pairs,
    on_init,
};
