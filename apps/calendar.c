// --------------------------------------------------------------------------------------
// Copyright (c) 2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: calendar.c - Calendar app
// --------------------------------------------------------------------------------------

#include <gui.h>

enum {
    GRID_CELL_WIDTH = 36,
    GRID_CELL_HEIGHT = 36,

    TOOL_BAR_Y = TITLE_BAR_HEIGHT - 1,
    TOOL_BAR_HEIGHT = GRID_CELL_HEIGHT + 2,

    WEEK_BAR_Y = (TOOL_BAR_Y + TOOL_BAR_HEIGHT - 1),
    WEEK_BAR_HEIGHT = 24,

    GRID_ROWS = 6,
    GRID_COLS = 7,
    GRID_CELLS_COUNT = GRID_ROWS * GRID_COLS,
    GRID_WIDTH = GRID_WIDTH_SPACED(GRID_CELL_WIDTH, GRID_COLS),
    GRID_HEIGHT = GRID_HEIGHT_SPACED(GRID_CELL_HEIGHT, GRID_ROWS),
    GRID_X = 1,
    GRID_Y = WEEK_BAR_Y + WEEK_BAR_HEIGHT,

    WINDOW_WIDTH = GRID_X + GRID_WIDTH + 1,
    WINDOW_HEIGHT = GRID_Y + GRID_HEIGHT + 1,
};

static widget_st title_bar;
static widget_st close_button;
static widget_st prev_button;
static widget_st next_button;
static widget_st day_buttons[GRID_CELLS_COUNT];
static widget_st *widgets[GRID_CELLS_COUNT + 4];

static uint8_t window_pixels[WINDOW_WIDTH * WINDOW_HEIGHT];
static surface_st window_surface;
static window_st window;

enum {
    MIN_YEAR = 1900,
    MAX_YEAR = 2099,
};

static int current_month = 0;
static int current_year = 0;
static int current_day = 0;

static int selected_month = 0;
static int selected_year = 0;

static grid_st grid;

static int
get_day_of_week(int day, int month, int year)
{
    static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };

    if (month < 3) {
        --year;
    }

    return (year + year / 4 - year / 100 + year / 400 + t[month - 1] + day) % 7;
}

static int
get_num_days_in_month(int month, int year)
{
    static int days_in_month[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    int is_leap = (year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0));

    return (is_leap && month == 2) ? 29 : days_in_month[month];
}

static void
draw_month_label(void)
{
    static const char *month_names[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    char buf[16];
    snprintf(buf, sizeof(buf), "%s %d", month_names[selected_month - 1], selected_year);

    rect_st rect = {
        .x = TOOL_BAR_HEIGHT - 1,
        .y = TOOL_BAR_Y,
        .width = WINDOW_WIDTH - (2 * TOOL_BAR_HEIGHT) + 2,
        .height = TOOL_BAR_HEIGHT,
    };

    gui_surface_draw_border(window.surface, rect, COLOR_BORDER);
    gui_surface_draw_str_centered(window.surface, rect, font_8x16, buf,
        COLOR_TEXT_ACTIVE, COLOR_WINDOW);
    gui_wm_render_window_region(&window, rect);
}

static void
draw_day_button(widget_st *widget)
{
    int day = widget->tag1;
    int num_days = get_num_days_in_month(selected_month, selected_year);

    int is_in_month = day >= 0 && day < num_days;
    int is_current = (day == current_day - 1 && selected_month == current_month
        && selected_year == current_year);
    int is_pressed = (widget == widget->window->pressed_widget) || widget->active;

    if (!is_in_month) {
        gui_surface_draw_rect(widget->window->surface, widget->rect, COLOR_WINDOW);
        gui_wm_render_window_region(widget->window, widget->rect);
        return;
    }

    int fg = is_pressed ? COLOR_WINDOW : COLOR_TEXT_ACTIVE;
    int bg = is_pressed
        ? COLOR_BUTTON_PRESSED
        : (is_current ? COLOR_TITLE_BAR_ACTIVE : COLOR_WINDOW);

    gui_surface_draw_rect(widget->window->surface, widget->rect, bg);

    char buf[3];
    snprintf(buf, sizeof(buf), "%d", day + 1);
    gui_surface_draw_str_centered(
        widget->window->surface,
        widget->rect,
        widget->font ? widget->font : font_8x16,
        buf,
        fg,
        bg
    );

    gui_wm_render_window_region(widget->window, widget->rect);
}

static void
draw_selected_month(void)
{
    int day_of_week = get_day_of_week(1, selected_month, selected_year);

    for (size_t i = 0; i < GRID_CELLS_COUNT; ++i) {
        day_buttons[i].tag1 = i - day_of_week;
        gui_widget_draw(&day_buttons[i]);
    }

    draw_month_label();
}

static void
draw_week_bar(void)
{
    static const char *day_names[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

    for (int y = 0; y < 7; y++) {
        rect_st rect = {
            .x = y * (GRID_CELL_WIDTH + 2) - y,
            .y = WEEK_BAR_Y,
            .width = GRID_CELL_WIDTH + 2,
            .height = WEEK_BAR_HEIGHT,
        };

        gui_surface_draw_border(window.surface, rect, COLOR_BORDER);
        gui_surface_draw_str_centered(window.surface, rect, font_8x16,
            day_names[y], COLOR_TEXT_ACTIVE, COLOR_WINDOW);
    }
}

static void
on_prev_button(widget_st *widget _unsd, event_st event,
    point_st pos)
{
    gui_button_on_pointer_up(widget, event, pos);

    if (selected_month > 1) {
        selected_month -= 1;
    } else if (selected_year > MIN_YEAR) {
        selected_year -= 1;
        selected_month = 12;
    } else {
        return;
    }

    draw_selected_month();
}

static void
on_next_button(widget_st *widget _unsd, event_st event,
    point_st pos)
{
    gui_button_on_pointer_up(widget, event, pos);

    if (selected_month < 12) {
        selected_month += 1;
    } else if (selected_year < MAX_YEAR) {
        selected_year += 1;
        selected_month = 1;
    } else {
        return;
    }

    draw_selected_month();
}

static void
init_window(void)
{
    window_surface.size.width = WINDOW_WIDTH;
    window_surface.size.height = WINDOW_HEIGHT;
    window_surface.pitch = WINDOW_WIDTH;
    window_surface.pixels = window_pixels;

    window.surface = &window_surface;
    window.title = "Calendar";
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
init_day_buttons(void)
{
    grid.cell_width = GRID_CELL_WIDTH;
    grid.cell_height = GRID_CELL_HEIGHT;
    grid.cols = GRID_COLS;
    grid.rows = GRID_ROWS;
    grid.x = GRID_X;
    grid.y = GRID_Y;

    gui_grid_draw_background(&grid, &window, COLOR_BORDER);

    for (size_t i = 0; i < GRID_CELLS_COUNT; ++i) {
        int col = i % GRID_COLS;
        int row = i / GRID_COLS;

        day_buttons[i].type = WIDGET_TYPE_BUTTON;
        day_buttons[i].rect = gui_grid_cell_rect(&grid, col, row);
        day_buttons[i].draw = draw_day_button;
        day_buttons[i].font = font_8x16;
        day_buttons[i].press_on_move_in = 1;

        gui_window_add_widget(&window, &day_buttons[i]);
    }
}

static void
init_current_date(void)
{
    time_st t;
    krn_rtc_get_time(&t);

    current_month = t.month;
    current_year = t.year;
    current_day = t.day;

    selected_month = current_month;
    selected_year = current_year;
}

static void
show_app(void)
{
    static int initialized = 0;

    if (!initialized) {
        init_window();
        init_buttons();
        init_day_buttons();
        init_current_date();

        draw_week_bar();
        draw_selected_month();

        initialized = 1;
    }

    (void)gui_wm_add_window(&window);
}


app_st app_calendar = {
    .panel_icon_r = &bitmap_icon_calendar_r,
    .panel_icon_a = &bitmap_icon_calendar_a,

    .show = show_app,
};
