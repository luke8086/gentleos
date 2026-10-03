/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: gallery.c - Image gallery app
 */

#include <gui.h>

enum {
    IMAGE_MAX_COUNT = 64,
};

typedef struct {
    rect_st viewport;

    file_st far *images[IMAGE_MAX_COUNT];

    int image_count;
    int cur_index;
    int inverted;
} app_state_st;

static app_state_st *app_state = (app_state_st *)gui_app_shared_buffer;

static void
show_image(int index)
{
    app_state_st *a = app_state;
    file_st far *file = a->images[index];
    const bitmap_file_st far *bitmap = file->u.addr;
    const uint8_t far *pixels = (const uint8_t far *)(bitmap + 1);
    char name[sizeof(file->name)];
    uint8_t bg_col = a->inverted ? 0x00 : 0x0f;
    uint8_t fg_col = a->inverted ? 0x0f : 0x00;
    window_st window;

    gui_status_set_urgent("Loading...");

    gui_surface_draw_rect(&GUI_POINT_ZERO, &a->viewport, bg_col);

    gui_window_init(&window, bitmap->width, bitmap->height);
    gui_surface_draw_bitmap_far(&window.origin, 0, 0,
        &window.size, bitmap->pitch, pixels, fg_col);

    gui_surface_mark_dirty(&GUI_POINT_ZERO, &a->viewport);

    a->cur_index = index;

    memcpy_far(name, file->name, sizeof(name));
    gui_status_set("%s (%d/%d)", name, index + 1, a->image_count);
}

static void
on_key_down(uint8_t key_code, uint8_t key_mods)
{
    app_state_st *a = app_state;

    if (a->image_count == 0) {
        return;
    }

    switch (key_code) {
    case KEY_PGUP:
        show_image((a->cur_index + a->image_count - 1) % a->image_count);
        return;

    case KEY_PGDN:
        show_image((a->cur_index + 1) % a->image_count);
        return;

    case KEY_I:
        a->inverted = !a->inverted;
        show_image(a->cur_index);
        return;
    }
}

static void
init_images(void)
{
    app_state_st *a = app_state;
    file_st far *file;
    const bitmap_file_st far *bitmap;
    uint16_t i, count = file_count();

    for (i = 0; i < count && a->image_count < IMAGE_MAX_COUNT; ++i) {
        file = file_get(i);

        if (file->type != FILE_TYPE_BITMAP) {
            continue;
        }

        bitmap = file->u.addr;

        if (bitmap->width <= a->viewport.width && bitmap->height <= a->viewport.height) {
            a->images[a->image_count++] = file;
        }
    }
}

static void
on_show(void)
{
    app_state_st *a = app_state;

    gui_rect_copy(&a->viewport, &gui_app_rect);
    a->viewport.y += 1;
    a->viewport.height -= 2;

    init_images();

    if (a->image_count == 0) {
        gui_status_set("No images found");
    } else {
        gui_status_set_br("I: Invert  PgUp/PgDn: Browse");
        show_image(0);
    }
}

static void
on_init(void)
{
    ASSERT(sizeof(app_state_st) <= sizeof(gui_app_shared_buffer));

    app_gallery.on_show = on_show;
    app_gallery.on_key_down = on_key_down;
}

global app_st app_gallery = {
    "Gallery",
    &icon_gallery,
    on_init,
};
