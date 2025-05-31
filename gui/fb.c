// --------------------------------------------------------------------------------------
// Copyright (c) 2025-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: fb.c - Framebuffer routines
// --------------------------------------------------------------------------------------

#include <gui.h>

static surface_st _gui_fb_surface = { 0 };
surface_st *gui_fb_surface = &_gui_fb_surface;

static int gui_fb_drawing = 0;

void
gui_fb_draw_start(void)
{
    if (gui_fb_drawing == 0) {
        gui_pointer_hide();
    }

    ++gui_fb_drawing;
}

void
gui_fb_draw_end(void)
{
    --gui_fb_drawing;

    if (gui_fb_drawing == 0) {
        gui_pointer_draw();
    }
}

void
gui_fb_init(void)
{
    gui_fb_surface->size.width = krn_core_mboot_info->fb_width;
    gui_fb_surface->size.height = krn_core_mboot_info->fb_height;
    gui_fb_surface->pitch = krn_core_mboot_info->fb_pitch;
    gui_fb_surface->pixels = krn_core_mboot_info->fb_addr;
}
