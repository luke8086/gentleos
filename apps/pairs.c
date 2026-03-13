// --------------------------------------------------------------------------------------
// Copyright (c) 2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: pairs.c - Pair matching / Memory game
// --------------------------------------------------------------------------------------

#include <gui.h>

enum {
    GRID_CELL_WIDTH = 40,
    GRID_CELL_HEIGHT = 40,
    GRID_ROWS = 4,
    GRID_COLS = 5,
    GRID_CELL_COUNT = GRID_ROWS * GRID_COLS,
    GRID_WIDTH = GRID_WIDTH_SPACED(GRID_CELL_WIDTH, GRID_COLS),
    GRID_HEIGHT = GRID_HEIGHT_SPACED(GRID_CELL_HEIGHT, GRID_ROWS),
    GRID_X = 1,
    GRID_Y = TITLE_BAR_HEIGHT,

    WINDOW_WIDTH = GRID_X + GRID_WIDTH + 1,
    WINDOW_HEIGHT = GRID_Y + GRID_HEIGHT + 1,

    PAIR_COUNT = GRID_CELL_COUNT / 2,
    MISMATCH_DELAY = 800,
};

static uint8_t window_pixels[WINDOW_WIDTH * WINDOW_HEIGHT];
static surface_st window_surface;
static window_st window;

static widget_st title_bar;
static widget_st close_button;
static widget_st buttons[GRID_CELL_COUNT];
static widget_st *widgets[GRID_CELL_COUNT + 2];

static grid_st grid;

static bitmap_st *icons[PAIR_COUNT] = {
    &bitmap_icon_pairs_bear,
    &bitmap_icon_pairs_bot,
    &bitmap_icon_pairs_butterfly,
    &bitmap_icon_pairs_cat,
    &bitmap_icon_pairs_elephant,
    &bitmap_icon_pairs_fish,
    &bitmap_icon_pairs_ghost,
    &bitmap_icon_pairs_rabbit,
    &bitmap_icon_pairs_snake,
    &bitmap_icon_pairs_turtle,
};

enum {
    BUTTON_STATE_HIDDEN = 0,
    BUTTON_STATE_REVEALED = 1,
    BUTTON_STATE_MATCHED = 2,
};

static uint8_t button_icons[GRID_CELL_COUNT];
static uint8_t button_states[GRID_CELL_COUNT];

static int first_pick;
static int second_pick;
static int tries;
static int matched_count;
static int waiting;

static void
shuffle_icons(void)
{
    uint8_t deck[GRID_CELL_COUNT];

    for (int i = 0; i < PAIR_COUNT; i++) {
        deck[i * 2] = i;
        deck[i * 2 + 1] = i;
    }

    for (int i = GRID_CELL_COUNT - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        uint8_t tmp = deck[i];
        deck[i] = deck[j];
        deck[j] = tmp;
    }

    for (int i = 0; i < GRID_CELL_COUNT; i++) {
        button_icons[i] = deck[i];
    }
}

static void
draw_button(widget_st *widget)
{
    int idx = widget->tag1;
    uint8_t state = button_states[idx];
    rect_st rect = widget->rect;
    int pressed = widget->window->pressed_widget == widget && !waiting;

    if (state == BUTTON_STATE_HIDDEN && !pressed) {
        gui_surface_draw_rect(window.surface, rect, COLOR_WINDOW);
        gui_surface_draw_h_seg(window.surface, rect.x, rect.y, rect.width, COLOR_WHITE);
        gui_surface_draw_v_seg(window.surface, rect.x, rect.y, rect.height, COLOR_WHITE);
    } else if (state == BUTTON_STATE_HIDDEN && pressed) {
        gui_surface_draw_rect(window.surface, rect, COLOR_WINDOW);
    } else {
        gui_surface_draw_rect(window.surface, rect, COLOR_WINDOW);
        gui_surface_draw_bitmap_centered(window.surface, rect, icons[button_icons[idx]],
            COLOR_TEXT_ACTIVE);
    }

    gui_wm_render_window_region(&window, rect);
}

static void
reveal_icon(int idx)
{
    button_states[idx] = BUTTON_STATE_REVEALED;
    draw_button(&buttons[idx]);
}

static void
hide_icon(int idx)
{
    button_states[idx] = BUTTON_STATE_HIDDEN;
    draw_button(&buttons[idx]);
}

static void
update_status(void)
{
    if (matched_count == PAIR_COUNT) {
        gui_status_set("You won after %d tries! Click to play again", tries);
    } else {
        gui_status_set("Tries: %d", tries);
    }
}

static void
restart_game(void)
{
    shuffle_icons();

    first_pick = -1;
    second_pick = -1;
    tries = 0;
    matched_count = 0;
    waiting = 0;

    for (int i = 0; i < GRID_CELL_COUNT; i++) {
        button_states[i] = BUTTON_STATE_HIDDEN;
        draw_button(&buttons[i]);
    }

    update_status();
}

static void
handle_mismatch_timeout(timeout_payload payload _unsd)
{
    hide_icon(first_pick);
    first_pick = -1;

    hide_icon(second_pick);
    second_pick = -1;

    waiting = 0;

    update_status();
}

static void
handle_cell_pointer_up(widget_st *widget, event_st event _unsd, point_st pos _unsd)
{
    if (matched_count == PAIR_COUNT) {
        restart_game();
        return;
    }

    if (waiting) {
        return;
    }

    int idx = widget->tag1;

    if (button_states[idx] != BUTTON_STATE_HIDDEN) {
        return;
    }

    if (first_pick == -1) {
        first_pick = idx;
        reveal_icon(first_pick);
        return;
    }

    second_pick = idx;
    reveal_icon(second_pick);
    tries++;

    if (button_icons[first_pick] == button_icons[second_pick]) {
        button_states[first_pick] = BUTTON_STATE_MATCHED;
        button_states[second_pick] = BUTTON_STATE_MATCHED;
        first_pick = -1;
        second_pick = -1;
        matched_count++;
    } else {
        waiting = 1;
        gui_timeout_add(MISMATCH_DELAY, handle_mismatch_timeout, NULL);
    }

    update_status();
}

static void
init_window(void)
{
    window_surface.size.width = WINDOW_WIDTH;
    window_surface.size.height = WINDOW_HEIGHT;
    window_surface.pitch = WINDOW_WIDTH;
    window_surface.pixels = window_pixels;

    window.surface = &window_surface;
    window.title = "Pairs";
    window.bg_color = COLOR_BORDER;
    window.widgets = widgets;
    window.widgets_capacity = sizeof(widgets) / sizeof(widgets[0]);

    gui_window_init_frame(&window, &title_bar, &close_button);
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

    for (int i = 0; i < GRID_CELL_COUNT; i++) {
        int col = i % GRID_COLS;
        int row = i / GRID_COLS;

        buttons[i].type = WIDGET_TYPE_BUTTON;
        buttons[i].rect = gui_grid_cell_rect(&grid, col, row);
        buttons[i].tag1 = i;
        buttons[i].draw = draw_button;
        buttons[i].on_pointer_up = handle_cell_pointer_up;
        buttons[i].hide_border = 1;

        gui_window_add_widget(&window, &buttons[i]);
    }
}

static void
show_app(void)
{
    static int initialized = 0;

    if (!initialized) {
        init_window();
        init_grid();
        initialized = 1;
    }

    restart_game();

    (void)gui_wm_add_window(&window);
}

app_st app_pairs = {
    .icon = &bitmap_icon_pairs,
    .show = show_app,
};
