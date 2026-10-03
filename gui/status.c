/*
 * Copyright (c) 2025-2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: status.c - Status bar routines
 */

#include <gui.h>

enum {
    STATUS_WIDTH = GUI_WIDTH,
};

enum {
    FONT_WIDTH = 5,
    FONT_HEIGHT = 8,
};

enum {
    TEXT_X = FONT_WIDTH,
    TEXT_Y = (STATUS_HEIGHT - FONT_HEIGHT) / 2,
    TEXT_MAX_LEN = (STATUS_WIDTH / FONT_WIDTH) - 2,
};

enum {
    CORNER_TL = 0,
    CORNER_TR = 1,
    CORNER_BL = 2,
    CORNER_BR = 3,
};

static uint16_t status_text_len[4];
static char status_text_buf[TEXT_MAX_LEN + 1];
static const char *github_link = "luke8086/gentleos";

static void
gui_status_set_text(int corner, const char *text)
{
    point_st origin = { 0, 0 };
    rect_st clear_rect, text_rect;
    uint16_t len = strlen(text);
    font_st *font = FONT_DEFAULT;
    int width = len * font->size.width;
    int prev_width = status_text_len[corner] * font->size.width;
    int x = TEXT_X;
    int prev_x = TEXT_X;

    if (corner == CORNER_BL || corner == CORNER_BR) {
        origin.y = GUI_HEIGHT - STATUS_HEIGHT;
    }

    if (corner == CORNER_TR || corner == CORNER_BR) {
        x = STATUS_WIDTH - TEXT_X - width;
        prev_x = STATUS_WIDTH - TEXT_X - prev_width;
    }

    gui_surface_draw_str(&origin, x, TEXT_Y, font, text, gui_color_fg);

    /* If the new text is shorter than previous, clear the remaining space */
    if (len < status_text_len[corner]) {
        gui_rect_init(&clear_rect,
            (corner == CORNER_TR || corner == CORNER_BR) ? prev_x : x + width,
            TEXT_Y,
            prev_width - width,
            font->size.height
        );

        gui_surface_draw_rect(&origin, &clear_rect, gui_color_bg);
    }

    gui_rect_init(&text_rect, TEXT_X, 0, STATUS_WIDTH - TEXT_X * 2, STATUS_HEIGHT);
    gui_surface_mark_dirty(&origin, &text_rect);

    status_text_len[corner] = len;
}

global void
gui_status_set_tl(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    (void) vsnprintf(status_text_buf, sizeof(status_text_buf), fmt, args);
    va_end(args);

    gui_status_set_text(CORNER_TL, status_text_buf);
}

global void
gui_status_set_tr(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    (void) vsnprintf(status_text_buf, sizeof(status_text_buf), fmt, args);
    va_end(args);

    gui_status_set_text(CORNER_TR, status_text_buf);
}

global void
gui_status_set(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    (void) vsnprintf(status_text_buf, sizeof(status_text_buf), fmt, args);
    va_end(args);

    gui_status_set_text(CORNER_BL, status_text_buf);
}

global void
gui_status_set_urgent(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    (void) vsnprintf(status_text_buf, sizeof(status_text_buf), fmt, args);
    va_end(args);

    gui_status_set_text(CORNER_BL, status_text_buf);
    gui_surface_flush();
}

global void
gui_status_set_br(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    (void) vsnprintf(status_text_buf, sizeof(status_text_buf), fmt, args);
    va_end(args);

    gui_status_set_text(CORNER_BR, status_text_buf);
}

global void
gui_status_init(void)
{
    point_st origin = { 0, 0 };

    gui_surface_draw_h_seg(&origin, 0, STATUS_HEIGHT - 1, STATUS_WIDTH, gui_color_fg);
    gui_surface_draw_h_seg(&origin, 0, GUI_HEIGHT - STATUS_HEIGHT, STATUS_WIDTH, gui_color_fg);
    gui_surface_draw_bitmap(&origin,
        STATUS_WIDTH - 2 * TEXT_X - (int)strlen(github_link) * FONT_WIDTH - sprite_github.size.width,
        1, &sprite_github, gui_color_fg);

    gui_status_set("");
    gui_status_set_tl("");
    gui_status_set_tr(github_link);
}
