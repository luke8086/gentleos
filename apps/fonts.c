// --------------------------------------------------------------------------------------
// Copyright (c) 2025-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: fonts.c - Font browser app
// --------------------------------------------------------------------------------------

#include <gui.h>

enum {
    TOOL_BAR_Y = TITLE_BAR_HEIGHT - 1,
    TOOL_BAR_HEIGHT = 36,

    GRID_CELL_WIDTH = 18,
    GRID_CELL_HEIGHT = 18,
    GRID_ROWS = 16,
    GRID_COLS = 16,
    GRID_CELLS_COUNT = (GRID_ROWS * GRID_COLS),
    GRID_WIDTH = GRID_WIDTH_SPACED(GRID_CELL_WIDTH, GRID_COLS),
    GRID_HEIGHT = GRID_HEIGHT_SPACED(GRID_CELL_HEIGHT, GRID_ROWS),
    GRID_X = 1,
    GRID_Y = TOOL_BAR_Y + TOOL_BAR_HEIGHT,

    WINDOW_WIDTH = GRID_X + GRID_WIDTH + 1,
    WINDOW_HEIGHT = GRID_Y + GRID_HEIGHT + 1,
};

static uint8_t window_pixels[WINDOW_WIDTH * WINDOW_HEIGHT];
static surface_st window_surface;
static window_st window;

static widget_st title_bar;
static widget_st close_button;
static widget_st prev_button;
static widget_st next_button;
static widget_st char_buttons[GRID_CELLS_COUNT];
static widget_st *widgets[GRID_CELLS_COUNT + 4];

static widget_st *active_char_button = NULL;
static size_t current_font = 0;

static grid_st grid;

static void
draw_char_button(widget_st *widget)
{
    char str[2] = { widget->tag2 ? widget->tag2 : ' ', 0 };
    int is_active = widget == active_char_button;

    gui_surface_draw_rect(window.surface, widget->rect,
        is_active ? COLOR_BORDER : COLOR_WINDOW);

    gui_surface_draw_str_centered(
        window.surface,
        widget->rect,
        &fonts[current_font],
        (const char *)str,
        is_active ? COLOR_WINDOW : COLOR_TEXT_ACTIVE,
        is_active ? COLOR_BUTTON_PRESSED : COLOR_WINDOW
    );

    gui_wm_render_window_region(widget->window, widget->rect);
}

static void
draw_all_char_buttons(void)
{
    for (size_t i = 0; i < GRID_CELLS_COUNT; ++i) {
        char_buttons[i].draw(&char_buttons[i]);
    }
}

static void
draw_font_label(void)
{
    rect_st r = {
        .x = TOOL_BAR_HEIGHT - 1,
        .y = TOOL_BAR_Y,
        .width = WINDOW_WIDTH - 2 * TOOL_BAR_HEIGHT + 2,
        .height = TOOL_BAR_HEIGHT,
    };

    gui_surface_draw_border(window.surface, r, COLOR_BORDER);
    gui_surface_draw_rect(window.surface, gui_rect_shrink(r, 1), COLOR_WINDOW);
    gui_surface_draw_str_centered(window.surface, r, font_8x16,
        fonts[current_font].name, COLOR_TEXT_ACTIVE, COLOR_WINDOW);

    gui_wm_render_window_region(&window, r);
}

static void
on_prev_button(widget_st *widget _unsd, event_st event, point_st pos)
{
    gui_button_on_pointer_up(widget, event, pos);

    current_font = (current_font - 1) % FONT_COUNT;

    draw_font_label();
    draw_all_char_buttons();
}

static void
on_next_button(widget_st *widget _unsd, event_st event, point_st pos)
{
    gui_button_on_pointer_up(widget, event, pos);

    current_font = (current_font + 1) % FONT_COUNT;

    draw_font_label();
    draw_all_char_buttons();
}

static void
on_char_button_press(widget_st *widget, event_st event _unsd, point_st pos _unsd)
{
    widget_st *prev_active_char_button = active_char_button;

    active_char_button = widget;

    if (active_char_button == prev_active_char_button) {
        return;
    }

    if (prev_active_char_button) {
        gui_widget_draw(prev_active_char_button);
    }

    gui_widget_draw(active_char_button);
    gui_status_set("hex:%02x dec:%03d", active_char_button->tag2,
        active_char_button->tag2);
}

static void
init_window(void)
{
    window_surface.size.width = WINDOW_WIDTH;
    window_surface.size.height = WINDOW_HEIGHT;
    window_surface.pitch = WINDOW_WIDTH;
    window_surface.pixels = window_pixels;

    window.surface = &window_surface;
    window.title = "Fonts";
    window.bg_color = COLOR_WINDOW;
    window.widgets = widgets;
    window.widgets_capacity = sizeof(widgets) / sizeof(widgets[0]);

    gui_window_init_frame(&window, &title_bar, &close_button);
}

static void
init_buttons(void)
{
    prev_button.type = WIDGET_TYPE_BUTTON;
    prev_button.rect.x = 0;
    prev_button.rect.y = TOOL_BAR_Y;
    prev_button.rect.width = TOOL_BAR_HEIGHT;
    prev_button.rect.height = TOOL_BAR_HEIGHT;
    prev_button.label = "<";
    prev_button.on_pointer_up = on_prev_button;

    next_button.type = WIDGET_TYPE_BUTTON;
    next_button.rect.x = WINDOW_WIDTH - TOOL_BAR_HEIGHT;
    next_button.rect.y = TOOL_BAR_Y;
    next_button.rect.width = TOOL_BAR_HEIGHT;
    next_button.rect.height = TOOL_BAR_HEIGHT;
    next_button.label = ">";
    next_button.on_pointer_up = on_next_button;

    gui_window_add_widget(&window, &prev_button);
    gui_window_add_widget(&window, &next_button);
}

static void
init_char_buttons(void)
{
    grid.cell_width = GRID_CELL_WIDTH;
    grid.cell_height = GRID_CELL_HEIGHT;
    grid.cols = GRID_COLS;
    grid.rows = GRID_ROWS;
    grid.x = GRID_X;
    grid.y = GRID_Y;

    gui_grid_draw_background(&grid, &window, COLOR_BORDER);

    for (size_t i = 0; i < GRID_CELLS_COUNT; ++i) {
        int col = i % grid.cols;
        int row = i / grid.cols;

        char_buttons[i].type = WIDGET_TYPE_BUTTON;
        char_buttons[i].rect = gui_grid_cell_rect(&grid, col, row);
        char_buttons[i].tag2 = i;
        char_buttons[i].window = &window;
        char_buttons[i].draw = draw_char_button;
        char_buttons[i].on_pointer_down = on_char_button_press;
        char_buttons[i].press_on_move_in = 1;

        gui_window_add_widget(&window, &char_buttons[i]);
    }
}

static void
show_app(void)
{
    static int initialized = 0;

    if (!initialized) {
        init_window();
        init_buttons();
        init_char_buttons();
        draw_font_label();
        initialized = 1;
    }

    gui_wm_add_window(&window);
}

app_st app_fonts = {
    .panel_icon_r = &bitmap_icon_fonts_r,
    .panel_icon_a = &bitmap_icon_fonts_a,
    .show = show_app,
};
