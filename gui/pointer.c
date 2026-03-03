// --------------------------------------------------------------------------------------
// Copyright (c) 2025-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: pointer.c - Mouse pointer
// --------------------------------------------------------------------------------------

#include <gui.h>

static rect_st gui_pointer_rect;

void
gui_pointer_draw(void)
{
#if GUI_PLANAR_MODE
    gui_planar_draw_bitmap(gui_pointer_rect.x, gui_pointer_rect.y, &bitmap_pointer);
#else
    gui_surface_draw_bitmap(gui_fb_vram_surface, gui_pointer_rect.x,
        gui_pointer_rect.y, &bitmap_pointer);
#endif
}

void
gui_pointer_move(uint16_t x, uint16_t y)
{
    gui_fb_mark_dirty(gui_pointer_rect);

    gui_pointer_rect.x = x;
    gui_pointer_rect.y = y;
}

void
gui_pointer_init(void)
{
    gui_pointer_rect.x = krn_core_mboot_info->fb_width / 2;
    gui_pointer_rect.y = krn_core_mboot_info->fb_height / 2;
    gui_pointer_rect.width = bitmap_pointer.size.width;
    gui_pointer_rect.height = bitmap_pointer.size.height;
}
