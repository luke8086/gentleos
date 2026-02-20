// --------------------------------------------------------------------------------------
// Copyright (c) 2025-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: colors.c - Color palette app
// --------------------------------------------------------------------------------------

#include <gui.h>

enum {
    GRID_CELL_WIDTH = 18,
    GRID_CELL_HEIGHT = 18,
    GRID_ROWS = 16,
    GRID_COLS = 16,
    GRID_CELLS_COUNT = (GRID_ROWS * GRID_COLS),
    GRID_WIDTH = GRID_WIDTH_SPACED(GRID_CELL_WIDTH, GRID_COLS),
    GRID_HEIGHT = GRID_HEIGHT_SPACED(GRID_CELL_HEIGHT, GRID_ROWS),
    GRID_X = 0,
    GRID_Y = (TITLE_BAR_HEIGHT - 1),

    WINDOW_WIDTH = GRID_X + GRID_WIDTH + 1,
    WINDOW_HEIGHT = GRID_Y + GRID_HEIGHT + 1,
};

static uint8_t window_surface_pixels[WINDOW_WIDTH * WINDOW_HEIGHT];
static surface_st window_surface;
static window_st window;

static widget_st color_buttons[GRID_CELLS_COUNT];
static widget_st *active_color_button = &color_buttons[0];

static widget_st close_button;
static widget_st title_bar;

static widget_st *widgets[GRID_CELLS_COUNT + 2];

static grid_st grid;

static void
on_color_button_press(widget_st *widget, event_st event _unsd, point_st pos _unsd)
{
    widget_st *prev_active_color_button = active_color_button;

    active_color_button = widget;

    if (active_color_button == prev_active_color_button) {
        return;
    }

    if (prev_active_color_button) {
        gui_widget_draw(prev_active_color_button);
    }

    gui_widget_draw(active_color_button);
    gui_status_set("hex:%02x dec:%03d", active_color_button->tag2,
        active_color_button->tag2);
}

static void
draw_color_button(widget_st *widget)
{
    rect_st rect = widget->rect;

    if (widget == active_color_button) {
        gui_surface_draw_rect(widget->window->surface, rect, COLOR_BORDER);
        gui_surface_draw_rect(widget->window->surface, gui_rect_shrink(rect, 1),
            widget->tag2);
    } else {
        gui_surface_draw_rect(widget->window->surface, rect, widget->tag2);
    }

    gui_wm_render_window_region(widget->window, rect);
}

static void
init_window(void)
{
    window_surface.size.width = WINDOW_WIDTH;
    window_surface.size.height = WINDOW_HEIGHT;
    window_surface.pitch = WINDOW_WIDTH;
    window_surface.pixels = window_surface_pixels;

    window.surface = &window_surface;
    window.title = "Colors";
    window.bg_color = COLOR_BLACK;
    window.widgets = widgets;
    window.widgets_capacity = sizeof(widgets) / sizeof(widgets[0]);

    gui_window_init_frame(&window, &title_bar, &close_button);
}

static void
init_color_buttons(void)
{
    grid.cell_width = GRID_CELL_WIDTH;
    grid.cell_height = GRID_CELL_HEIGHT;
    grid.cols = GRID_COLS;
    grid.rows = GRID_ROWS;
    grid.x = GRID_X;
    grid.y = GRID_Y;

    for (size_t i = 0; i < GRID_CELLS_COUNT; i++) {
        int col = i % grid.cols;
        int row = i / grid.cols;

        color_buttons[i].rect = gui_grid_cell_rect(&grid, col, row);
        color_buttons[i].tag2 = i;
        color_buttons[i].window = &window;
        color_buttons[i].draw = draw_color_button;
        color_buttons[i].on_pointer_down = on_color_button_press;
        color_buttons[i].press_on_move_in = 1;

        gui_window_add_widget(&window, &color_buttons[i]);
    };
}

static void
show_app(void)
{
    static int initialized = 0;

    if (!initialized) {
        init_window();
        init_color_buttons();
        initialized = 1;
    }

    gui_wm_add_window(&window);
}

app_st app_colors = {
    .panel_icon_r = &bitmap_icon_colors_r,
    .panel_icon_a = &bitmap_icon_colors_a,
    .show = show_app,
};
