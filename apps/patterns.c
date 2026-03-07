// --------------------------------------------------------------------------------------
// Copyright (c) 2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: patterns.c - Background pattern app
// --------------------------------------------------------------------------------------

#include <gui.h>

enum {
    PATTERN_COLS = 4,
    PATTERN_ROWS = 2,
    PATTERN_CELL_WIDTH = 49,
    PATTERN_CELL_HEIGHT = 49,
    PATTERN_COUNT = (PATTERN_COLS * PATTERN_ROWS),
    PATTERN_GRID_X = 1,
    PATTERN_GRID_Y = TITLE_BAR_HEIGHT,
    PATTERN_GRID_WIDTH = GRID_WIDTH_SPACED(PATTERN_CELL_WIDTH, PATTERN_COLS),
    PATTERN_GRID_HEIGHT = GRID_HEIGHT_SPACED(PATTERN_CELL_HEIGHT, PATTERN_ROWS),

    COLOR_COLS = 8,
    COLOR_ROWS = 2,
    COLOR_CELL_WIDTH = 24,
    COLOR_CELL_HEIGHT = 18,
    COLOR_COUNT = (COLOR_COLS * COLOR_ROWS),
    COLOR_GRID_WIDTH = GRID_WIDTH_SPACED(COLOR_CELL_WIDTH, COLOR_COLS),
    COLOR_GRID_HEIGHT = GRID_HEIGHT_SPACED(COLOR_CELL_HEIGHT, COLOR_ROWS),

    COLOR1_GRID_X = 1,
    COLOR1_GRID_Y = PATTERN_GRID_Y + PATTERN_GRID_HEIGHT + 1,

    COLOR2_GRID_X = 1,
    COLOR2_GRID_Y = COLOR1_GRID_Y + COLOR_GRID_HEIGHT + 1,

    WINDOW_WIDTH = COLOR_GRID_WIDTH + 2,
    WINDOW_HEIGHT = COLOR2_GRID_Y + COLOR_GRID_HEIGHT + 1,

    WIDGETS_COUNT = PATTERN_COUNT + COLOR_COUNT + COLOR_COUNT + 2,
};

static uint8_t window_pixels[WINDOW_WIDTH * WINDOW_HEIGHT];
static surface_st window_surface;
static window_st window;

static widget_st title_bar;
static widget_st close_button;

static widget_st pattern_buttons[PATTERN_COUNT];
static widget_st color1_buttons[COLOR_COUNT];
static widget_st color2_buttons[COLOR_COUNT];

static widget_st *widgets[WIDGETS_COUNT];

static grid_st pattern_grid;
static grid_st color1_grid;
static grid_st color2_grid;

static widget_st *active_pattern_button;
static widget_st *active_color1_button;
static widget_st *active_color2_button;

static bitmap_st *patterns[] = {
    NULL,
    &bitmap_pattern_1,
    &bitmap_pattern_2,
    &bitmap_pattern_3,
    &bitmap_pattern_4,
    &bitmap_pattern_5,
    &bitmap_pattern_6,
    &bitmap_pattern_7,
};

static void
draw_pattern_button(widget_st *widget)
{
    rect_st rect = widget->rect;
    surface_st *sf = widget->window->surface;
    int is_active = (widget == active_pattern_button);

    if (is_active) {
        gui_surface_draw_rect(sf, rect, COLOR_BORDER);
        rect = gui_rect_shrink(rect, 1);
    }

    int idx = widget->tag1;

    if (widget->tag1 == 0) {
        gui_surface_draw_rect(sf, rect, COLOR_WINDOW);
    } else {
        gui_surface_draw_pattern(sf, rect, patterns[idx], COLOR_WINDOW, COLOR_BLACK);
    }

    gui_wm_render_window_region(widget->window, widget->rect);
}

static void
on_pattern_button_press(widget_st *widget, event_st event _unsd, point_st pos _unsd)
{
    widget_st *prev = active_pattern_button;
    active_pattern_button = widget;

    gui_wm_bg_pattern = patterns[widget->tag1];

    if (prev && prev != widget) {
        gui_widget_draw(prev);
    }

    gui_widget_draw(widget);

    gui_wm_render_desktop_region(gui_wm_container, NULL);
}

static void
draw_color_button(widget_st *widget)
{
    rect_st rect = widget->rect;
    surface_st *sf = widget->window->surface;

    if (widget == active_color1_button || widget == active_color2_button) {
        gui_surface_draw_rect(sf, rect, COLOR_BORDER);
        rect = gui_rect_shrink(rect, 1);
    }

    gui_surface_draw_rect(sf, rect, widget->tag2);

    gui_wm_render_window_region(widget->window, widget->rect);
}

static void
on_color1_button_press(widget_st *widget, event_st event _unsd, point_st pos _unsd)
{
    widget_st *prev = active_color1_button;
    active_color1_button = widget;

    gui_wm_bg_color1 = widget->tag2;

    if (prev && prev != widget) {
        gui_widget_draw(prev);
    }

    gui_widget_draw(widget);

    gui_wm_render_desktop_region(gui_wm_container, NULL);
}

static void
on_color2_button_press(widget_st *widget, event_st event _unsd, point_st pos _unsd)
{
    widget_st *prev = active_color2_button;
    active_color2_button = widget;

    gui_wm_bg_color2 = widget->tag2;

    if (prev && prev != widget) {
        gui_widget_draw(prev);
    }

    gui_widget_draw(widget);

    gui_wm_render_desktop_region(gui_wm_container, NULL);
}

static void
init_window(void)
{
    window_surface.size.width = WINDOW_WIDTH;
    window_surface.size.height = WINDOW_HEIGHT;
    window_surface.pitch = WINDOW_WIDTH;
    window_surface.pixels = window_pixels;

    window.surface = &window_surface;
    window.title = "Patterns";
    window.bg_color = COLOR_BLACK;
    window.widgets = widgets;
    window.widgets_capacity = sizeof(widgets) / sizeof(widgets[0]);

    gui_window_init_frame(&window, &title_bar, &close_button);
}

static void
init_pattern_buttons(void)
{
    pattern_grid.cell_width = PATTERN_CELL_WIDTH;
    pattern_grid.cell_height = PATTERN_CELL_HEIGHT;
    pattern_grid.cols = PATTERN_COLS;
    pattern_grid.rows = PATTERN_ROWS;
    pattern_grid.x = PATTERN_GRID_X;
    pattern_grid.y = PATTERN_GRID_Y;

    for (int i = 0; i < PATTERN_COUNT; i++) {
        int col = i % PATTERN_COLS;
        int row = i / PATTERN_COLS;

        pattern_buttons[i].rect = gui_grid_cell_rect(&pattern_grid, col, row);
        pattern_buttons[i].tag1 = i;
        pattern_buttons[i].window = &window;
        pattern_buttons[i].draw = draw_pattern_button;
        pattern_buttons[i].on_pointer_down = on_pattern_button_press;
        pattern_buttons[i].press_on_move_in = 1;

        gui_window_add_widget(&window, &pattern_buttons[i]);
    }
}

static void
init_color_buttons(grid_st *grid, widget_st *buttons, int grid_x, int grid_y,
    void (*on_press)(widget_st *, event_st, point_st))
{
    grid->cell_width = COLOR_CELL_WIDTH;
    grid->cell_height = COLOR_CELL_HEIGHT;
    grid->cols = COLOR_COLS;
    grid->rows = COLOR_ROWS;
    grid->x = grid_x;
    grid->y = grid_y;

    for (int i = 0; i < COLOR_COUNT; i++) {
        int col = i % COLOR_COLS;
        int row = i / COLOR_COLS;

        buttons[i].rect = gui_grid_cell_rect(grid, col, row);
        buttons[i].tag2 = i;
        buttons[i].window = &window;
        buttons[i].draw = draw_color_button;
        buttons[i].on_pointer_down = on_press;
        buttons[i].press_on_move_in = 1;

        gui_window_add_widget(&window, &buttons[i]);
    }
}

static void
select_active_buttons(void)
{
    for (int i = 0; i < PATTERN_COUNT; i++) {
        if (gui_wm_bg_pattern == patterns[i]) {
            active_pattern_button = &pattern_buttons[i];
            break;
        }
    }

    for (int i = 0; i < COLOR_COUNT; i++) {
        if (gui_wm_bg_color1 == i) {
            active_color1_button = &color1_buttons[i];
        }

        if (gui_wm_bg_color2 == i) {
            active_color2_button = &color2_buttons[i];
        }
    }
}

static void
show_app(void)
{
    static int initialized = 0;

    if (!initialized) {
        init_window();
        select_active_buttons();
        init_pattern_buttons();
        init_color_buttons(&color1_grid, color1_buttons, COLOR1_GRID_X, COLOR1_GRID_Y,
            on_color1_button_press);
        init_color_buttons(&color2_grid, color2_buttons, COLOR2_GRID_X, COLOR2_GRID_Y,
            on_color2_button_press);
        initialized = 1;
    }

    gui_wm_add_window(&window);
}

app_st app_patterns = {
    .panel_icon_r = &bitmap_icon_patterns_r,
    .panel_icon_a = &bitmap_icon_patterns_a,
    .show = show_app,
};
