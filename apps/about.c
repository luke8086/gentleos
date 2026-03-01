// --------------------------------------------------------------------------------------
// Copyright (c) 2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: about.c - System info app
// --------------------------------------------------------------------------------------

#include <gui.h>

enum {
    GRID_CELL_WIDTH = 7,
    GRID_CELL_HEIGHT = 15,
    GRID_ROWS = 12,
    GRID_COLS = 33,
    GRID_CELLS_COUNT = (GRID_ROWS * GRID_COLS),
    GRID_WIDTH = GRID_WIDTH_SPACED(GRID_CELL_WIDTH, GRID_COLS),
    GRID_HEIGHT = GRID_HEIGHT_SPACED(GRID_CELL_HEIGHT, GRID_ROWS),
    GRID_X = 1,
    GRID_Y = TITLE_BAR_HEIGHT,

    WINDOW_WIDTH = GRID_X + GRID_WIDTH + 1,
    WINDOW_HEIGHT = GRID_Y + GRID_HEIGHT + 1,

    LABEL_COL = 2,
    VALUE_COL = 11,
    VALUE_LEN = GRID_COLS - VALUE_COL - 2,
};

static uint8_t window_pixels[WINDOW_WIDTH * WINDOW_HEIGHT];
static surface_st window_surface;
static window_st window;

static widget_st title_bar;
static widget_st close_button;
static widget_st *widgets[2];

static grid_st grid;

static void
draw_text_lg(int col, int row, const char *text)
{
    if (row >= GRID_COLS) {
        return;
    }

    rect_st r = gui_grid_cell_rect(&grid, col, row);
    gui_surface_draw_str(window.surface, r.x, r.y, font_8x16,
        text, COLOR_TEXT_ACTIVE, COLOR_WINDOW);
}

static void
draw_text_sm(int col, int row, const char *text)
{
    if (row >= GRID_COLS) {
        return;
    }

    rect_st r = gui_grid_cell_rect(&grid, col, row);
    gui_surface_draw_str(window.surface, r.x, r.y, font_8x8,
        text, COLOR_TEXT_ACTIVE, COLOR_WINDOW);
}

static void
draw_info(void)
{
    rect_st r = gui_grid_rect(&grid);
    mboot_info_st *m = krn_core_mboot_info;
    static char buf[VALUE_LEN + 1];
    int line = 0;
    const char *title = "-=[ GENTLE OS ]=-";

    gui_surface_draw_rect(window.surface, r, window.bg_color);

    line++;
    draw_text_lg((GRID_COLS - strlen(title)) / 2, line++, title);
    line++;

    snprintf(buf, sizeof(buf), "%s", krn_system_get_cpu_vendor());
    draw_text_sm(LABEL_COL, line, "CPU:");
    draw_text_sm(VALUE_COL, line++, buf);

    if (m->flags & 0x04) {
        snprintf(buf, sizeof(buf), "%s", m->boot_loader_name);
        draw_text_sm(LABEL_COL, line, "Boot:");
        draw_text_sm(VALUE_COL, line++, buf);
    }

    snprintf(buf, sizeof(buf), "%dx%dx%d", m->fb_width, m->fb_height, m->fb_bpp);
    draw_text_sm(LABEL_COL, line, "Display:");
    draw_text_sm(VALUE_COL, line++, buf);

    snprintf(buf, sizeof(buf), "%u KB", krn_system_get_total_mem() >> 10);
    draw_text_sm(LABEL_COL, line, "Mem:");
    draw_text_sm(VALUE_COL, line++, buf);

    snprintf(buf, sizeof(buf), "%u KB", krn_system_get_used_mem() >> 10);
    draw_text_sm(LABEL_COL, line, "Used:");
    draw_text_sm(VALUE_COL, line++, buf);

    snprintf(buf, sizeof(buf), "%u KB", krn_system_get_avail_mem() >> 10);
    draw_text_sm(LABEL_COL, line, "Avail:");
    draw_text_sm(VALUE_COL, line++, buf);

    line++;
    draw_text_sm(LABEL_COL, line++, "github.com/luke8086/gentleos");

    gui_wm_render_window_region(&window, r);
}

static void
init_window(void)
{
    window_surface.size.width = WINDOW_WIDTH;
    window_surface.size.height = WINDOW_HEIGHT;
    window_surface.pitch = WINDOW_WIDTH;
    window_surface.pixels = window_pixels;

    window.surface = &window_surface;
    window.title = "About";
    window.bg_color = COLOR_WINDOW;
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

    draw_info();

    gui_wm_add_window(&window);
}

app_st app_about = {
    .panel_icon_r = &bitmap_icon_about_r,
    .panel_icon_a = &bitmap_icon_about_a,
    .show = show_app,
};
