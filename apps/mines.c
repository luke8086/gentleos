// --------------------------------------------------------------------------------------
// Copyright (c) 2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: mines.c - Minesweeper game
// --------------------------------------------------------------------------------------

#include <gui.h>

static void reveal_cell(int col, int row);

enum {
    GRID_CELL_WIDTH = 17,
    GRID_CELL_HEIGHT = 17,
    GRID_ROWS = 9,
    GRID_COLS = 9,
    GRID_CELL_COUNT = GRID_ROWS * GRID_COLS,
    GRID_WIDTH = GRID_WIDTH_SPACED(GRID_CELL_WIDTH, GRID_COLS),
    GRID_HEIGHT = GRID_HEIGHT_SPACED(GRID_CELL_HEIGHT, GRID_ROWS),
    GRID_X = 1,
    GRID_Y = TITLE_BAR_HEIGHT,

    WINDOW_WIDTH = GRID_X + GRID_WIDTH + 1,
    WINDOW_HEIGHT = GRID_Y + GRID_HEIGHT + 1,

    MINE_COUNT = 10,
};

static uint8_t window_pixels[WINDOW_WIDTH * WINDOW_HEIGHT];
static surface_st window_surface;
static window_st window;

static widget_st title_bar;
static widget_st close_button;
static widget_st cell_widgets[GRID_CELL_COUNT];
static widget_st *widgets[GRID_CELL_COUNT + 2];

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

static size_t
count_cells_by_state(uint8_t state)
{
    size_t count = 0;

    for (int y = 0; y < GRID_ROWS; ++y) {
        for (int x = 0; x < GRID_COLS; ++x) {
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
    size_t count = 0;

    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) {
                continue;
            }

            int nx = col + dx;
            int ny = row + dy;

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
draw_cell(widget_st *widget)
{
    int idx = widget->tag1;
    int col = idx % GRID_COLS;
    int row = idx / GRID_COLS;
    uint8_t state = cell_state[col][row];
    uint8_t type = cell_type[col][row];
    rect_st rect = widget->rect;
    char num_str[2] = { 0, 0 };

    if (state == CELL_STATE_HIDDEN) {
        gui_surface_draw_rect(window.surface, rect, COLOR_WINDOW_DARKER);
    } else if (state == CELL_STATE_FLAGGED) {
        gui_surface_draw_rect(window.surface, rect, COLOR_WINDOW_DARKER);
        gui_surface_draw_bitmap_centered(window.surface, rect, &bitmap_sprite_flag);
    } else if (state == CELL_STATE_REVEALED && type == CELL_TYPE_MINE) {
        gui_surface_draw_rect(window.surface, rect, COLOR_WINDOW);
        gui_surface_draw_bitmap_centered(window.surface, rect, &bitmap_sprite_mine);
    } else if (state == CELL_STATE_REVEALED && type == CELL_TYPE_EMPTY) {
        gui_surface_draw_rect(window.surface, rect, COLOR_WINDOW);
    } else if (state == CELL_STATE_REVEALED) {
        num_str[0] = '0' + type;
        rect_st num_rect = gui_rect_make(rect.x + 1, rect.y + 1,
            rect.width - 1, rect.height - 1);

        gui_surface_draw_rect(window.surface, rect, COLOR_WINDOW);
        gui_surface_draw_str_centered(window.surface, num_rect, font_8x8,
            num_str, COLOR_TEXT_ACTIVE, COLOR_WINDOW);
    }

    gui_wm_render_window_region(&window, rect);
}

static void
update_cell(int col, int row, uint8_t type, uint8_t state)
{
    cell_type[col][row] = type;
    cell_state[col][row] = state;
    draw_cell(&cell_widgets[row * GRID_COLS + col]);
}

static void
update_all_mines(uint8_t state)
{
    for (int row = 0; row < GRID_ROWS; ++row) {
        for (int col = 0; col < GRID_COLS; ++col) {
            if (cell_type[col][row] == CELL_TYPE_MINE) {
                update_cell(col, row, CELL_TYPE_MINE, state);
            }
        }
    }
}

static void
clear_cells(void)
{
    for (int row = 0; row < GRID_ROWS; ++row) {
        for (int col = 0; col < GRID_COLS; ++col) {
            update_cell(col, row, CELL_TYPE_EMPTY, CELL_STATE_HIDDEN);
        }
    }
}

static void
place_mines(int except_col, int except_row)
{
    int remaining = MINE_COUNT;

    while (remaining > 0) {
        int col = rand() % GRID_COLS;
        int row = rand() % GRID_ROWS;

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
    for (int row = 0; row < GRID_ROWS; ++row) {
        for (int col = 0; col < GRID_COLS; ++col) {
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
        gui_status_set("Game Over! Press any cell to start a new game");
    } else if (state == GAME_STATE_WON) {
        gui_status_set("You Win! Press any cell to start a new game");
    } else {
        size_t flagged_count = count_cells_by_state(CELL_STATE_FLAGGED);
        size_t remaining = MINE_COUNT > flagged_count ? MINE_COUNT - flagged_count : 0;

        gui_status_set("Remaining mines: %u", remaining);
    }
}

static void
restart_game(void)
{
    clear_cells();
    update_status();
}

static void
reveal_adjacent_cells(int col, int row)
{
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) {
                continue;
            }

            reveal_cell(col + dx, row + dy);
        }
    }
}

static void
reveal_cell(int col, int row)
{
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

    int adjacent_mine_count = count_adjacent_mines(col, row);
    update_cell(col, row, adjacent_mine_count, CELL_STATE_REVEALED);

    if (adjacent_mine_count == 0) {
        reveal_adjacent_cells(col, row);
    }

    if (get_game_state() == GAME_STATE_WON) {
        update_all_mines(CELL_STATE_FLAGGED);
    }

    update_status();
}

static void
handle_cell_pointer_up(widget_st *widget, event_st event _unsd, point_st pos _unsd)
{
    if (get_game_state() != GAME_STATE_PLAYING) {
        restart_game();
        return;
    }

    int idx = widget->tag1;
    int col = idx % GRID_COLS;
    int row = idx / GRID_COLS;

    if (cell_state[col][row] != CELL_STATE_HIDDEN) {
        return;
    }

    reveal_cell(col, row);
}

static void
handle_cell_pointer_alt(widget_st *widget, event_st event _unsd, point_st pos _unsd)
{
    if (get_game_state() != GAME_STATE_PLAYING) {
        return;
    }

    int idx = widget->tag1;
    int col = idx % GRID_COLS;
    int row = idx / GRID_COLS;

    if (cell_state[col][row] == CELL_STATE_HIDDEN) {
        update_cell(col, row, cell_type[col][row], CELL_STATE_FLAGGED);
        update_status();
    } else if (cell_state[col][row] == CELL_STATE_FLAGGED) {
        update_cell(col, row, cell_type[col][row], CELL_STATE_HIDDEN);
        update_status();
    }
}

static void
init_window(void)
{
    window_surface.size.width = WINDOW_WIDTH;
    window_surface.size.height = WINDOW_HEIGHT;
    window_surface.pitch = WINDOW_WIDTH;
    window_surface.pixels = window_pixels;

    window.surface = &window_surface;
    window.title = "Mines";
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

        cell_widgets[i].type = WIDGET_TYPE_BUTTON;
        cell_widgets[i].rect = gui_grid_cell_rect(&grid, col, row);
        cell_widgets[i].draw = draw_cell;
        cell_widgets[i].tag1 = i;
        cell_widgets[i].on_pointer_up = handle_cell_pointer_up;
        cell_widgets[i].on_pointer_alt = handle_cell_pointer_alt;

        gui_window_add_widget(&window, &cell_widgets[i]);
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

app_st app_mines = {
    .panel_icon_r = &bitmap_icon_mines_r,
    .panel_icon_a = &bitmap_icon_mines_a,
    .show = show_app,
};

