// --------------------------------------------------------------------------------------
// Copyright (c) 2025-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: surface.c - Surface drawing routines
// --------------------------------------------------------------------------------------

#include <gui.h>

void
gui_surface_copy(surface_st *dst_sf, int dst_x, int dst_y,
    surface_st *src_sf, rect_st src_rect)
{
    if (dst_x + src_rect.width > dst_sf->size.width) {
        src_rect.width = dst_sf->size.width - dst_x;
        src_rect.width = MAX(src_rect.width, 0);
    }

    if (dst_y + src_rect.height > dst_sf->size.height) {
        src_rect.height = dst_sf->size.height - dst_y;
        src_rect.height = MAX(src_rect.height, 0);
    }

    if (dst_y < 0 || dst_y + src_rect.height > dst_sf->size.height) {
        krn_debug_printf("err: dst_y: %d, src_rect.height: %d, dst_sf->size.height: %d\n",
            dst_y, src_rect.height, dst_sf->size.height);
        return;
    }

    if (dst_x < 0 || dst_x + src_rect.width > dst_sf->size.width) {
        krn_debug_printf("err: dst_x: %d, src_rect.width: %d, dst_sf->size.width: %d\n",
            dst_x, src_rect.width, dst_sf->size.width);
        return;
    }

    for (int i = 0; i < src_rect.height; ++i) {
        memcpy(
            dst_sf->pixels + (dst_y + i) * dst_sf->pitch + dst_x,
            src_sf->pixels + (src_rect.y + i) * src_sf->pitch + src_rect.x,
            src_rect.width
        );
    }
}

void
gui_surface_draw_h_seg(surface_st *surface, int x, int y, int w, uint8_t color)
{
    memset(surface->pixels + y * surface->pitch + x, color, w);
}

void
gui_surface_draw_v_seg(surface_st *surface, int x, int y, int h, uint8_t color)
{
    for (int i = 0; i < h; i++) {
        surface->pixels[(y + i) * surface->pitch + x] = color;
    }
}

void
gui_surface_draw_border(surface_st *surface, rect_st r, uint8_t color)
{
    gui_surface_draw_h_seg(surface, r.x, r.y, r.width, color);
    gui_surface_draw_h_seg(surface, r.x, r.y + r.height - 1, r.width, color);
    gui_surface_draw_v_seg(surface, r.x, r.y, r.height, color);
    gui_surface_draw_v_seg(surface, r.x + r.width - 1, r.y, r.height, color);
}

void
gui_surface_draw_rect(surface_st *surface, rect_st r, uint8_t color)
{
    for (int i = 0; i < r.height; i++) {
        gui_surface_draw_h_seg(surface, r.x, r.y + i, r.width, color);
    }
}

void
gui_surface_draw_char(surface_st *surface, uint16_t x, uint16_t y,
    font_st *font, uint8_t ch, uint8_t fg, uint8_t bg)
{
    const uint8_t *glyph;
    uint8_t active;
    size_t npixel;
    int i, j;

    if (!ch) {
        ch = ' ';
    }

    glyph = font->pixels + (ch * font->size.height);

    for (j = 0; j < font->size.height; ++j) {
        for (i = 0; i < font->size.width; ++i) {
            active = (glyph[j] & (1 << (7 - i)));
            npixel = (y + j) * surface->pitch + (x + i);
            surface->pixels[npixel] = fg * !!active + bg * !active;
        }
    }
}

void
gui_surface_draw_str(surface_st *surface, uint16_t x, uint16_t y,
    font_st *font, const char *s, uint8_t fg, uint8_t bg)
{
    for (int i = 0; s[i]; i++) {
        gui_surface_draw_char(surface, x + i * font->size.width, y, font, s[i], fg, bg);
    }
}

void
gui_surface_draw_str_centered(surface_st *surface, rect_st rect,
    font_st *font, const char *s, uint8_t fg, uint8_t bg)
{
    int text_width = strlen(s) * font->size.width;

    int x = rect.x + (rect.width - text_width) / 2;
    int y = rect.y + (rect.height - font->size.height) / 2;

    gui_surface_draw_str(surface, x, y, font, s, fg, bg);
}

void
gui_surface_draw_bitmap(surface_st *surface, int dst_x, int dst_y, bitmap_st *bitmap)
{
    char alpha = (char)bitmap->alpha;

    rect_st src_rect = {
        .x = 0,
        .y = 0,
        .size = bitmap->size,
    };

    if (dst_x + src_rect.width > surface->size.width) {
        src_rect.width = surface->size.width - dst_x;
        src_rect.width = src_rect.width < 0 ? 0 : src_rect.width;
    }

    if (dst_y + src_rect.height > surface->size.height) {
        src_rect.height = surface->size.height - dst_y;
        src_rect.height = src_rect.height < 0 ? 0 : src_rect.height;
    }

    for (uint16_t i = 0; i < src_rect.height; i++) {
        for (uint16_t j = 0; j < src_rect.width; j++) {
            if (bitmap->pixels[i * bitmap->size.width + j] == alpha) {
                continue;
            }

            size_t src_pixel_no = i * bitmap->size.width + j;
            size_t dst_pixel_no = (dst_x + j) + (dst_y + i) * surface->pitch;

            surface->pixels[dst_pixel_no] = bitmap->pixels[src_pixel_no];
        }
    }
}

void
gui_surface_draw_bitmap_centered(surface_st *surface, rect_st rect, bitmap_st *b)
{
    int x = rect.x + (rect.width - b->size.width) / 2;
    int y = rect.y + (rect.height - b->size.height) / 2;

    gui_surface_draw_bitmap(surface, x, y, b);
}

