// --------------------------------------------------------------------------------------
// Copyright (c) 2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: sounds.c - Sound playing app
// --------------------------------------------------------------------------------------

#include <gui.h>

enum {
    KEY_W_WIDTH = 30,
    KEY_W_HEIGHT = 130,
    KEY_W_COUNT = 15,

    KEY_B_WIDTH = 17,
    KEY_B_HEIGHT = 80,
    KEY_B_COUNT = 10,

    KEYBOARD_Y = (TITLE_BAR_HEIGHT - 1),
    KEYBOARD_HEIGHT = (KEY_W_HEIGHT),

    WINDOW_WIDTH = ((KEY_W_COUNT * KEY_W_WIDTH) - (KEY_W_COUNT - 1)),
    WINDOW_HEIGHT = (KEYBOARD_Y + KEYBOARD_HEIGHT),

    TAG_KEY_W = 1,
    TAG_KEY_B = 2,
};

static uint8_t window_pixels[WINDOW_WIDTH * WINDOW_HEIGHT];
static surface_st window_surface;
static window_st window;

static widget_st keys_w[KEY_W_COUNT];
static widget_st keys_b[KEY_B_COUNT];
static widget_st title_bar;
static widget_st close_button;
static widget_st *widgets[KEY_W_COUNT + KEY_B_COUNT + 2];

static void
draw_key_w(widget_st *widget)
{
    rect_st rect_base = gui_rect_shrink(widget->rect, 1);
    uint8_t color = (widget == widget->window->pressed_widget)
        ? COLOR_TITLE_BAR_ACTIVE : COLOR_WINDOW;

    int octave = widget->tag2 / 7;
    int ofs = widget->tag2 % 7;

    rect_st rect_top = rect_base;
    if (ofs == 1 || ofs == 2 || ofs == 4 || ofs == 5 || ofs == 6) {
        rect_top.x += KEY_B_WIDTH / 2;
        rect_top.width -= KEY_B_WIDTH / 2;
    }
    if ((ofs == 0 && octave < 2) || ofs == 1 || ofs == 3 || ofs == 4 || ofs == 5) {
        rect_top.width -= KEY_B_WIDTH / 2;
    }
    gui_surface_draw_rect(widget->window->surface, rect_top, color);

    rect_st rect_bottom = rect_base;
    rect_bottom.y += KEY_B_HEIGHT;
    rect_bottom.height -= KEY_B_HEIGHT;
    gui_surface_draw_rect(widget->window->surface, rect_bottom, color);

    gui_wm_render_window_region(widget->window, widget->rect);
}

static void
draw_key_b(widget_st *widget)
{
    uint8_t color = (widget == widget->window->pressed_widget)
        ? COLOR_TITLE_BAR_ACTIVE : COLOR_BLACK;

    gui_surface_draw_rect(widget->window->surface, widget->rect, color);

    gui_wm_render_window_region(widget->window, widget->rect);
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

static void
on_key_pointer_down(widget_st *widget, event_st event, point_st pos)
{
    krn_speaker_play(key_frequency(widget));

    gui_button_on_pointer_down(widget, event, pos);
}

static void
on_key_pointer_up(widget_st *widget, event_st event, point_st pos)
{
    krn_speaker_stop();

    gui_button_on_pointer_up(widget, event, pos);
}

static void
on_key_pointer_out(widget_st *widget, event_st event, point_st pos)
{
    krn_speaker_stop();

    gui_button_on_pointer_out(widget, event, pos);
}

static void
init_window(void)
{
    window_surface.size.width = WINDOW_WIDTH;
    window_surface.size.height = WINDOW_HEIGHT;
    window_surface.pitch = WINDOW_WIDTH;
    window_surface.pixels = window_pixels;

    window.surface = &window_surface;
    window.title = "Sounds";
    window.bg_color = COLOR_BORDER;
    window.widgets = widgets;
    window.widgets_capacity = sizeof(widgets) / sizeof(widgets[0]);

    gui_window_init_frame(&window, &title_bar, &close_button);
}

static void
init_keys(void)
{
    for (int i = 0; i < KEY_B_COUNT; i++) {
        int octave_no = i / 5;
        int octave_ofs = i % 5;
        int key_w_idx = (octave_no * 7) + octave_ofs + 1 + (octave_ofs > 1 ? 1 : 0);

        keys_b[i].type = WIDGET_TYPE_BUTTON;
        keys_b[i].rect.x = (key_w_idx * KEY_W_WIDTH) - key_w_idx - (KEY_B_WIDTH / 2);
        keys_b[i].rect.y = TITLE_BAR_HEIGHT;
        keys_b[i].rect.width = KEY_B_WIDTH;
        keys_b[i].rect.height = KEY_B_HEIGHT;
        keys_b[i].draw = draw_key_b;
        keys_b[i].tag1 = TAG_KEY_B;
        keys_b[i].tag2 = i;
        keys_b[i].press_on_move_in = 1;
        keys_b[i].on_pointer_down = on_key_pointer_down;
        keys_b[i].on_pointer_up = on_key_pointer_up;
        keys_b[i].on_pointer_out = on_key_pointer_out;
        gui_window_add_widget(&window, &keys_b[i]);
    }

    for (int i = 0; i < KEY_W_COUNT; i++) {
        keys_w[i].type = WIDGET_TYPE_BUTTON;
        keys_w[i].rect.x = (i * KEY_W_WIDTH) - i;
        keys_w[i].rect.y = TITLE_BAR_HEIGHT - 1;
        keys_w[i].rect.width = KEY_W_WIDTH;
        keys_w[i].rect.height = KEY_W_HEIGHT;
        keys_w[i].draw = draw_key_w;
        keys_w[i].tag1 = TAG_KEY_W;
        keys_w[i].tag2 = i;
        keys_w[i].press_on_move_in = 1;
        keys_w[i].on_pointer_down = on_key_pointer_down;
        keys_w[i].on_pointer_up = on_key_pointer_up;
        keys_w[i].on_pointer_out = on_key_pointer_out;
        gui_window_add_widget(&window, &keys_w[i]);
    }
}

static void
show_app(void)
{
    static int initialized = 0;

    if (!initialized) {
        init_window();
        init_keys();
        initialized = 1;
    }

    gui_wm_add_window(&window);
}

app_st app_sounds = {
    .panel_icon_r = &bitmap_icon_sounds_r,
    .panel_icon_a = &bitmap_icon_sounds_a,
    .show = show_app,
};
