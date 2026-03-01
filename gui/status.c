// --------------------------------------------------------------------------------------
// Copyright (c) 2025-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: status.c - Status bar routines
// --------------------------------------------------------------------------------------

#include <gui.h>

enum {
    STATUS_WIDTH = GUI_WIDTH - PANEL_WIDTH,
};

enum {
    FONT_WIDTH = 8,
    FONT_HEIGHT = 16,
};

enum {
    TEXT_X = FONT_WIDTH,
    TEXT_Y = (STATUS_HEIGHT - FONT_HEIGHT) / 2,
    TEXT_MAX_LEN = (STATUS_WIDTH / FONT_WIDTH) - 2,
};

static uint8_t window_pixels[STATUS_WIDTH * STATUS_HEIGHT];
static surface_st window_surface;
static window_st window;

static size_t status_text_len = 0;
static uint8_t status_bg_color = 0;

static void
gui_status_set_bg_color(uint8_t color)
{
    if (color == status_bg_color) {
        return;
    }

    rect_st bg_rect = {
        .x = 0,
        .y = 1,
        .width = STATUS_WIDTH,
        .height = STATUS_HEIGHT - 1,
    };

    gui_surface_draw_rect(window.surface, bg_rect, color);
    gui_wm_render_window_region(&window, bg_rect);

    status_bg_color = color;
}

static void
gui_status_set_text(const char *text, uint8_t color)
{
    size_t len = strlen(text);
    font_st *font = font_8x16;

    gui_surface_draw_str(window.surface, TEXT_X, TEXT_Y, font, text, color,
        status_bg_color);

    // If the new text is shorter than previous, clear the remaining space
    if (len < status_text_len) {
        rect_st clear_rect = {
            .x = TEXT_X + len * font->size.width,
            .y = TEXT_Y,
            .width = (status_text_len - len) * font->size.width,
            .height = font->size.height,
        };

        gui_surface_draw_rect(window.surface, clear_rect, status_bg_color);
    }

    rect_st text_rect = {
        .x = TEXT_X,
        .y = TEXT_Y,
        .width = STATUS_WIDTH - TEXT_X * 2,
        .height = font->size.height,
    };

    gui_wm_render_window_region(&window, text_rect);

    status_text_len = len;
}

void
gui_status_set(const char *fmt, ...)
{
    static char buf[TEXT_MAX_LEN + 1];

    va_list args;

    va_start(args, fmt);
    (void) vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    gui_status_set_bg_color(COLOR_WINDOW);
    gui_status_set_text(buf, COLOR_TEXT_ACTIVE);
}


void
gui_status_set_alert(const char *fmt, ...)
{
    static char buf[TEXT_MAX_LEN + 1];

    va_list args;

    va_start(args, fmt);
    (void) vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    gui_status_set_bg_color(COLOR_RED);
    gui_status_set_text(buf, COLOR_WHITE);
}

void
gui_status_init(void)
{
    window_surface.size.width = STATUS_WIDTH;
    window_surface.size.height = STATUS_HEIGHT;
    window_surface.pitch = STATUS_WIDTH;
    window_surface.pixels = window_pixels;

    window.rect.x = 0;
    window.rect.y = GUI_HEIGHT - STATUS_HEIGHT;
    window.rect.width = STATUS_WIDTH;
    window.rect.height = STATUS_HEIGHT;
    window.surface = &window_surface;
    window.visible = 1;

    gui_surface_draw_h_seg(window.surface, 0, 0, STATUS_WIDTH, COLOR_BORDER);

    gui_status_set("", COLOR_TEXT_ACTIVE);

    gui_wm_set_status_window(&window);
}
