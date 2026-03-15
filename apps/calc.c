// --------------------------------------------------------------------------------------
// Copyright (c) 2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: calc.c - Calculator app
// --------------------------------------------------------------------------------------

#include <gui.h>

enum {
    BUTTON_WIDTH = 36,
    BUTTON_HEIGHT = 36,
    BUTTON_COLS = 4,
    BUTTON_ROWS = 4,
    BUTTONS_COUNT = BUTTON_COLS * BUTTON_ROWS,

    DISPLAY_HEIGHT = BUTTON_HEIGHT,

    GRID_X = 1,
    GRID_Y = TITLE_BAR_HEIGHT + DISPLAY_HEIGHT + 1,
    GRID_WIDTH = GRID_WIDTH_SPACED(BUTTON_WIDTH, BUTTON_COLS),
    GRID_HEIGHT = GRID_HEIGHT_SPACED(BUTTON_HEIGHT, BUTTON_ROWS),

    WINDOW_WIDTH = GRID_X + GRID_WIDTH + 1,
    WINDOW_HEIGHT = GRID_Y + GRID_HEIGHT + 1,

    DISPLAY_X = GRID_X,
    DISPLAY_Y = TITLE_BAR_HEIGHT,
    DISPLAY_WIDTH = GRID_WIDTH,
};

static uint8_t window_pixels[WINDOW_WIDTH * WINDOW_HEIGHT];
static surface_st window_surface;
static window_st window;

static widget_st title_bar;
static widget_st close_button;
static widget_st button_widgets[BUTTONS_COUNT];
static widget_st *widgets[BUTTONS_COUNT + 2];

static grid_st grid;

static const char *button_labels[BUTTONS_COUNT] = {
    "7", "8", "9", "/",
    "4", "5", "6", "*",
    "1", "2", "3", "-",
    "0", "C", "=", "+"
};

typedef int32_t val_t;

#define VAL_MAX INT32_MAX
#define VAL_MIN INT32_MIN

static val_t current_val = 0;
static val_t stored_val = 0;
static val_t last_operand = 0;
static uint8_t current_op = 0;
static uint8_t last_op = 0;
static int new_number = 1;
static int error = 0;

static void
exec_add(void)
{
    val_t result;

    if (__builtin_add_overflow(stored_val, current_val, &result)) {
        error = 1;
        return;
    }

    current_val = result;
}

static void
exec_sub(void)
{
    val_t result;

    if (__builtin_sub_overflow(stored_val, current_val, &result)) {
        error = 1;
        return;
    }

    current_val = result;
}

static void
exec_mul(void)
{
    val_t result;

    if (__builtin_mul_overflow(current_val, stored_val, &result)) {
        error = 1;
        return;
    }

    current_val = result;
}

static void
exec_div(void)
{
    if (current_val == 0 || (stored_val == VAL_MIN && current_val == -1)) {
        error = 1;
        return;
    }

    current_val = stored_val / current_val;
}

static void
exec_current_op(void)
{
    switch (current_op) {
    case '+': exec_add(); break;
    case '-': exec_sub(); break;
    case '*': exec_mul(); break;
    case '/': exec_div(); break;
    }

    current_op = 0;
    new_number = 1;
}

static void
update_display(void)
{
    static char buf[32];

    if (error) {
        snprintf(buf, sizeof(buf), "ERR");
    } else {
        snprintf(buf, sizeof(buf), "%d", current_val);
    }

    rect_st rect = gui_rect_make(DISPLAY_X, DISPLAY_Y, DISPLAY_WIDTH, DISPLAY_HEIGHT);

    gui_surface_draw_rect(window.surface, rect, COLOR_WINDOW);

    font_st *font = font_8x16;
    int text_width = strlen(buf) * font->size.width;
    int text_x = rect.x + rect.width - text_width - 10;
    int text_y = rect.y + (rect.height - font_8x16->size.height) / 2;
    gui_surface_draw_str(window.surface, text_x, text_y, font,
        buf, COLOR_TEXT_ACTIVE, COLOR_WINDOW);

    gui_wm_render_window_region(&window, rect);
}

static void
on_button_press(widget_st *widget, event_st event _unsd, point_st pos _unsd)
{
    gui_button_on_pointer_up(widget, event, pos);

    uint8_t op = widget->label[0];

    if (error && op != 'C') {
        return;
    }

    if (op >= '0' && op <= '9') {
        int val = op - '0';
        if (new_number) {
            current_val = val;
            new_number = 0;
        } else {
            val_t new_val;
            if (__builtin_mul_overflow(current_val, 10, &new_val) ||
                __builtin_add_overflow(new_val, val, &new_val)) {
                return;
            }
            current_val = new_val;
        }
    } else if (op == 'C') {
        current_val = 0;
        stored_val = 0;
        current_op = 0;
        last_op = 0;
        last_operand = 0;
        new_number = 1;
        error = 0;
    } else if (op == '=') {
        if (current_op) {
            last_op = current_op;
            last_operand = current_val;
            exec_current_op();
        } else if (last_op) {
            stored_val = current_val;
            current_val = last_operand;
            current_op = last_op;
            exec_current_op();
        }
    } else if (op == '+' || op == '-' || op == '*' || op == '/') {
        if (current_op && !new_number) {
            exec_current_op();
        }
        stored_val = current_val;
        current_op = op;
        last_op = 0;
        new_number = 1;
    }

    update_display();
}

static void
init_window(void)
{
    window_surface.size.width = WINDOW_WIDTH;
    window_surface.size.height = WINDOW_HEIGHT;
    window_surface.pitch = WINDOW_WIDTH;
    window_surface.pixels = window_pixels;

    window.surface = &window_surface;
    window.title = "Calculator";
    window.bg_color = COLOR_BORDER;
    window.widgets = widgets;
    window.widgets_capacity = sizeof(widgets) / sizeof(widgets[0]);

    gui_window_init_frame(&window, &title_bar, &close_button);
}

static void
init_buttons(void)
{
    grid.cell_width = BUTTON_WIDTH;
    grid.cell_height = BUTTON_HEIGHT;
    grid.cols = BUTTON_COLS;
    grid.rows = BUTTON_ROWS;
    grid.x = GRID_X;
    grid.y = GRID_Y;

    gui_grid_draw_background(&grid, &window, COLOR_BORDER);

    for (int row = 0; row < BUTTON_ROWS; ++row) {
        for (int col = 0; col < BUTTON_COLS; ++col) {
            int idx = row * BUTTON_COLS + col;
            widget_st *button = &button_widgets[idx];

            button->type = WIDGET_TYPE_BUTTON;
            button->rect = gui_grid_cell_rect(&grid, col, row);
            button->hide_border = 1;
            button->window = &window;
            button->label = button_labels[idx];
            button->font = font_8x16;
            button->on_pointer_up = on_button_press;

            gui_window_add_widget(&window, button);
        }
    }
}

static void
show_app(void)
{
    static int initialized = 0;

    if (!initialized) {
        init_window();
        init_buttons();
        initialized = 1;
    }

    gui_wm_add_window(&window);
    update_display();
}

app_st app_calc = {
    .icon = &bitmap_icon_calc,
    .show = show_app,
};

