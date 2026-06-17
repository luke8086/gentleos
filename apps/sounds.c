/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: sounds.c - Sound playing app
 */

#include <gui.h>

enum {
    KEY_W_WIDTH = 17,
    KEY_W_HEIGHT = 80,
    KEY_W_COUNT = 15,

    KEY_B_WIDTH = 11,
    KEY_B_HEIGHT = 50,
    KEY_B_COUNT = 10,

    KEYBOARD_Y = 0,
    KEYBOARD_HEIGHT = (KEY_W_HEIGHT),

    WINDOW_WIDTH = ((KEY_W_COUNT * KEY_W_WIDTH) - (KEY_W_COUNT - 1)),
    WINDOW_HEIGHT = (KEYBOARD_Y + KEYBOARD_HEIGHT),

    TAG_KEY_W = 1,
    TAG_KEY_B = 2,
};

static window_st window;

static widget_st keys_w[KEY_W_COUNT];
static widget_st keys_b[KEY_B_COUNT];
static widget_st *pressed_widget;

static void
draw_key_w(widget_st *widget)
{
    rect_st rect_base;
    uint8_t color = (widget == pressed_widget) ? gui_color_fg : gui_color_bg;

    int octave = widget->tag2 / 7;
    int ofs = widget->tag2 % 7;

    rect_st rect_top;
    rect_st rect_bottom;

    gui_rect_copy(&rect_base, &widget->rect);
    gui_rect_shrink(&rect_base, 1);

    gui_rect_copy(&rect_top, &rect_base);
    if (ofs == 1 || ofs == 2 || ofs == 4 || ofs == 5 || ofs == 6) {
        rect_top.x += KEY_B_WIDTH / 2;
        rect_top.width -= KEY_B_WIDTH / 2;
    }
    if ((ofs == 0 && octave < 2) || ofs == 1 || ofs == 3 || ofs == 4 || ofs == 5) {
        rect_top.width -= KEY_B_WIDTH / 2;
    }
    gui_surface_draw_rect(widget->origin, &rect_top, color);

    gui_rect_copy(&rect_bottom, &rect_base);
    rect_bottom.y += KEY_B_HEIGHT;
    rect_bottom.height -= KEY_B_HEIGHT;
    gui_surface_draw_rect(widget->origin, &rect_bottom, color);

    gui_surface_mark_dirty(widget->origin, &widget->rect);
}

static void
draw_key_b(widget_st *widget)
{
    uint8_t color = (widget == pressed_widget) ? gui_color_bg : gui_color_fg;

    gui_surface_draw_rect(widget->origin, &widget->rect, color);

    gui_surface_mark_dirty(widget->origin, &widget->rect);
}

static unsigned
key_frequency(widget_st *widget)
{
    static unsigned freqs_w[] = { 131, 147, 165, 175, 196, 220, 247 };
    static unsigned freqs_b[] = { 139, 156, 185, 208, 233 };

    int is_w = widget->tag1 == TAG_KEY_W;
    unsigned *freqs = is_w ? freqs_w : freqs_b;
    unsigned octave = is_w ? widget->tag2 / 7 : widget->tag2 / 5;
    unsigned ofs = is_w ? widget->tag2 % 7 : widget->tag2 % 5;

    return freqs[ofs] * (1 << octave);
}

static widget_st *
key_for_key_code(int key_code)
{
    widget_st *w = NULL;

    switch (key_code) {
    case KEY_Z: w = &keys_w[0]; break;
    case KEY_X: w = &keys_w[1]; break;
    case KEY_C: w = &keys_w[2]; break;
    case KEY_V: w = &keys_w[3]; break;
    case KEY_B: w = &keys_w[4]; break;
    case KEY_N: w = &keys_w[5]; break;
    case KEY_M: w = &keys_w[6]; break;
    case KEY_COMMA: w = &keys_w[7]; break;
    case KEY_W: w = &keys_w[7]; break;
    case KEY_E: w = &keys_w[8]; break;
    case KEY_R: w = &keys_w[9]; break;
    case KEY_T: w = &keys_w[10]; break;
    case KEY_Y: w = &keys_w[11]; break;
    case KEY_U: w = &keys_w[12]; break;
    case KEY_I: w = &keys_w[13]; break;
    case KEY_O: w = &keys_w[14]; break;
    case KEY_S: w = &keys_b[0]; break;
    case KEY_D: w = &keys_b[1]; break;
    case KEY_G: w = &keys_b[2]; break;
    case KEY_H: w = &keys_b[3]; break;
    case KEY_J: w = &keys_b[4]; break;
    case KEY_3: w = &keys_b[5]; break;
    case KEY_4: w = &keys_b[6]; break;
    case KEY_6: w = &keys_b[7]; break;
    case KEY_7: w = &keys_b[8]; break;
    case KEY_8: w = &keys_b[9]; break;
    }

    return w;
}

static void
on_key_down(uint8_t key_code, uint8_t key_mods)
{
    widget_st *widget = key_for_key_code(key_code);
    widget_st *prev_widget;

    if (!widget) {
        return;
    }

    if (pressed_widget) {
        prev_widget = pressed_widget;
        pressed_widget = NULL;
        prev_widget->draw(prev_widget);
    }

    krn_speaker_play(key_frequency(widget));
    pressed_widget = widget;
    widget->draw(widget);
}

static void
on_key_up(uint8_t key_code, uint8_t key_mods)
{
    widget_st *widget = key_for_key_code(key_code);

    if (!widget || widget != pressed_widget) {
        return;
    }

    pressed_widget = NULL;
    widget->draw(widget);
    krn_speaker_stop();
}

static void
init_keys(void)
{
    int i;
    int octave_no, octave_ofs, key_w_idx;

    for (i = 0; i < KEY_B_COUNT; i++) {
        octave_no = i / 5;
        octave_ofs = i % 5;
        key_w_idx = (octave_no * 7) + octave_ofs + 1 + (octave_ofs > 1 ? 1 : 0);

        keys_b[i].origin = &window.origin;
        keys_b[i].rect.x = (key_w_idx * KEY_W_WIDTH) - key_w_idx - (KEY_B_WIDTH / 2);
        keys_b[i].rect.y = 1;
        keys_b[i].rect.width = KEY_B_WIDTH;
        keys_b[i].rect.height = KEY_B_HEIGHT;
        keys_b[i].draw = draw_key_b;
        keys_b[i].tag1 = TAG_KEY_B;
        keys_b[i].tag2 = i;
    }

    for (i = 0; i < KEY_W_COUNT; i++) {
        keys_w[i].origin = &window.origin;
        keys_w[i].rect.x = (i * KEY_W_WIDTH) - i;
        keys_w[i].rect.y = 0;
        keys_w[i].rect.width = KEY_W_WIDTH;
        keys_w[i].rect.height = KEY_W_HEIGHT;
        keys_w[i].draw = draw_key_w;
        keys_w[i].tag1 = TAG_KEY_W;
        keys_w[i].tag2 = i;
    }
}

static void
on_show(void)
{
    int i;

    gui_window_draw(&window, gui_color_fg, 1);

    for (i = 0; i < KEY_B_COUNT; ++i) {
        keys_b[i].draw(&keys_b[i]);
    }

    for (i = 0; i < KEY_W_COUNT; ++i) {
        keys_w[i].draw(&keys_w[i]);
    }

    if (USE_BIOS_KEYBOARD) {
        gui_status_set("This app is unavailable when USE_BIOS_KEYBOARD is set");
    } else {
        gui_status_set("Z-,: Wh/Lo  S-J: Bl/Lo  W-O: Wh/Hi  3-8: Bl/Hi");
    }
}

static void
on_init(void)
{
    gui_window_init(&window, WINDOW_WIDTH, WINDOW_HEIGHT);

    init_keys();

    app_sounds.on_show = on_show;
    app_sounds.on_key_down = on_key_down;
    app_sounds.on_key_up = on_key_up;
}

global app_st app_sounds = {
    "Sounds",
    &icon_sounds,
    on_init,
};
