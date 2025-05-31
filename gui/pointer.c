// --------------------------------------------------------------------------------------
// Copyright (c) 2025-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: pointer.c - Mouse pointer
// --------------------------------------------------------------------------------------

#include <gui.h>

static struct {
    int x;
    int y;
    int saved;
} pointer_state;

static uint8_t saved_surface_pixels[11 * 15] = { 0 };

static surface_st saved_surface = {
    .size = { .width = 11, .height = 15, },
    .pitch = 11,
    .pixels = saved_surface_pixels,
};

void
gui_pointer_hide(void)
{
    if (!pointer_state.saved) {
        return;
    }

    rect_st src_rect = {
        .x = 0,
        .y = 0,
        .width = saved_surface.size.width,
        .height = saved_surface.size.height,
    };

    gui_surface_copy(gui_fb_surface, pointer_state.x,
        pointer_state.y, &saved_surface, src_rect);
}

void
gui_pointer_draw(void)
{
    rect_st src_rect = {
        .x = pointer_state.x,
        .y = pointer_state.y,
        .width = saved_surface.size.width,
        .height = saved_surface.size.height,
    };

    gui_surface_copy(&saved_surface, 0, 0, gui_fb_surface, src_rect);
    pointer_state.saved = 1;

    gui_surface_draw_bitmap(gui_fb_surface, pointer_state.x,
        pointer_state.y, &bitmap_pointer);
}

void
gui_pointer_move(uint16_t x, uint16_t y)
{
    gui_pointer_hide();

    pointer_state.x = x;
    pointer_state.y = y;

    gui_pointer_draw();
}

void
gui_pointer_init(void)
{
    pointer_state.x = krn_core_mboot_info->fb_width / 2;
    pointer_state.y = krn_core_mboot_info->fb_height / 2;
}
