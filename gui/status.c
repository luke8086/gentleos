// --------------------------------------------------------------------------------------
// Copyright (c) 2025-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: status.c - Status bar routines
// --------------------------------------------------------------------------------------

#include <gui.h>

enum {
    STATUS_WIDTH = GUI_WIDTH - PANEL_WIDTH,
    STATUS_X = 0,
    STATUS_Y = GUI_HEIGHT - STATUS_HEIGHT,
};

enum {
    FONT_WIDTH = 8,
    FONT_HEIGHT = 16,
};

enum {
    TEXT_X = STATUS_X + FONT_WIDTH,
    TEXT_Y = STATUS_Y + (STATUS_HEIGHT - FONT_HEIGHT) / 2,
    TEXT_MAX_LEN = (STATUS_WIDTH / FONT_WIDTH) - 2,
};

static size_t status_text_len = 0;
static uint8_t status_bg_color = 0;

static void
gui_status_set_bg_color(uint8_t color)
{
    if (color == status_bg_color) {
        return;
    }

    rect_st bg_rect = {
        .x = STATUS_X,
        .y = STATUS_Y + 1,
        .width = STATUS_WIDTH,
        .height = STATUS_HEIGHT - 1,
    };

    gui_fb_draw_start();
    gui_surface_draw_rect(gui_fb_surface, bg_rect, color);
    gui_fb_draw_end();

    status_bg_color = color;
}

static void
gui_status_set_text(const char *text, uint8_t color)
{
    size_t len = strlen(text);
    font_st *font = font_8x16;

    gui_fb_draw_start();

    gui_surface_draw_str(gui_fb_surface, TEXT_X, TEXT_Y, font, text, color,
        status_bg_color);

    // If the new text is shorter than previous, clear the remaining space
    if (len < status_text_len) {
        rect_st clear_rect = {
            .x = TEXT_X + len * font->size.width,
            .y = TEXT_Y,
            .width = (status_text_len - len) * font->size.width,
            .height = font->size.height,
        };

        gui_surface_draw_rect(gui_fb_surface, clear_rect, status_bg_color);
    }

    gui_fb_draw_end();

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
    gui_fb_draw_start();

    gui_surface_draw_h_seg(gui_fb_surface, STATUS_X, STATUS_Y,
        STATUS_WIDTH, COLOR_BORDER);

    gui_status_set("", COLOR_TEXT_ACTIVE);

    gui_fb_draw_end();
}
