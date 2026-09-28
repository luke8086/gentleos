/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: keys.c - Keyboard map
 */

#include <gui.h>

typedef struct {
    uint16_t code;
    uint8_t width;
    const char *label;
    int x;
    int y;
    int pressed;
} cell_st;

enum {
    CELL_H = 15,
    CELL_W = 15,
};

static cell_st cells[] = {
    { KEY_ESC,      CELL_W, "Esc" },
    { KEY_F1,       CELL_W, "F1" },
    { KEY_F2,       CELL_W, "F2" },
    { KEY_F3,       CELL_W, "F3" },
    { KEY_F4,       CELL_W, "F4" },
    { KEY_F5,       CELL_W, "F5" },
    { KEY_F6,       CELL_W, "F6" },
    { KEY_F7,       CELL_W, "F7" },
    { KEY_F8,       CELL_W, "F8" },
    { KEY_F9,       CELL_W, "F9" },
    { KEY_F10,      CELL_W, "F10" },
    { KEY_F11,      CELL_W, "F11" },
    { KEY_F12,      CELL_W, "F12" },
    { KEY_BKTICK,   CELL_W, "`" },
    { KEY_1,        CELL_W, "1" },
    { KEY_2,        CELL_W, "2" },
    { KEY_3,        CELL_W, "3" },
    { KEY_4,        CELL_W, "4" },
    { KEY_5,        CELL_W, "5" },
    { KEY_6,        CELL_W, "6" },
    { KEY_7,        CELL_W, "7" },
    { KEY_8,        CELL_W, "8" },
    { KEY_9,        CELL_W, "9" },
    { KEY_0,        CELL_W, "0" },
    { KEY_MINUS,    CELL_W, "-" },
    { KEY_EQUAL,    CELL_W, "=" },
    { KEY_BKSP,     34,     "Bksp" },
    { KEY_TAB,      23,     "Tab" },
    { KEY_Q,        CELL_W, "Q" },
    { KEY_W,        CELL_W, "W" },
    { KEY_E,        CELL_W, "E" },
    { KEY_R,        CELL_W, "R" },
    { KEY_T,        CELL_W, "T" },
    { KEY_Y,        CELL_W, "Y" },
    { KEY_U,        CELL_W, "U" },
    { KEY_I,        CELL_W, "I" },
    { KEY_O,        CELL_W, "O" },
    { KEY_P,        CELL_W, "P" },
    { KEY_LBRCKT,   CELL_W, "[" },
    { KEY_RBRCKT,   CELL_W, "]" },
    { KEY_BKSLASH,  26,     "\\" },
    { KEY_CAPS,     27,     "Caps" },
    { KEY_A,        CELL_W, "A" },
    { KEY_S,        CELL_W, "S" },
    { KEY_D,        CELL_W, "D" },
    { KEY_F,        CELL_W, "F" },
    { KEY_G,        CELL_W, "G" },
    { KEY_H,        CELL_W, "H" },
    { KEY_J,        CELL_W, "J" },
    { KEY_K,        CELL_W, "K" },
    { KEY_L,        CELL_W, "L" },
    { KEY_SEMICOL,  CELL_W, ";" },
    { KEY_QUOTE,    CELL_W, "'" },
    { KEY_ENTER,    37,     "Enter" },
    { KEY_LSHIFT,   35,     "Shift" },
    { KEY_Z,        CELL_W, "Z" },
    { KEY_X,        CELL_W, "X" },
    { KEY_C,        CELL_W, "C" },
    { KEY_V,        CELL_W, "V" },
    { KEY_B,        CELL_W, "B" },
    { KEY_N,        CELL_W, "N" },
    { KEY_M,        CELL_W, "M" },
    { KEY_COMMA,    CELL_W, "," },
    { KEY_PERIOD,   CELL_W, "." },
    { KEY_SLASH,    CELL_W, "/" },
    { KEY_RSHIFT,   44,     "Shift" },
    { KEY_CTRL,     25,     "Ctrl" },
    { KEY_ALT,      26,     "Alt" },
    { KEY_SPACE,    127,    "Space" },
    { KEY_RALT,     26,     "Alt" },
    { KEY_RCTRL,    25,     "Ctrl" },
    { KEY_INS,      CELL_W, "Ins" },
    { KEY_HOME,     CELL_W, "Hom" },
    { KEY_PGUP,     CELL_W, "PgU" },
    { KEY_DEL,      CELL_W, "Del" },
    { KEY_END,      CELL_W, "End" },
    { KEY_PGDN,     CELL_W, "PgD" },
    { KEY_UP,       CELL_W, "^" },
    { KEY_LEFT,     CELL_W, "<" },
    { KEY_DOWN,     CELL_W, "v" },
    { KEY_RIGHT,    CELL_W, ">" },
};

#define CELL_COUNT (sizeof(cells) / sizeof(cells[0]))

static window_st window;
static int last_key_code;

static void
init_cells(void)
{
    int i;

    window.size.width = 0;
    window.size.height = 0;

    for (i = 0; i < CELL_COUNT; ++i) {
        cells[i].x = 0;
        cells[i].y = 0;
        cells[i].pressed = 0;
    }

    cells[1].x = 27;
    cells[5].x = 98;
    cells[9].x = 169;
    cells[13].y = CELL_H + 5;
    cells[27].y = CELL_H * 2 + 5;
    cells[41].y = CELL_H * 3 + 5;
    cells[54].y = CELL_H * 4 + 5;
    cells[66].y = CELL_H * 5 + 5;
    cells[71].x = 240;
    cells[71].y = CELL_H + 5;
    cells[74].x = 240;
    cells[74].y = CELL_H * 2 + 5;
    cells[77].x = 240 + CELL_W;
    cells[77].y = CELL_H * 4 + 5;
    cells[78].x = 240;
    cells[78].y = CELL_H * 5 + 5;

    for (i = 1; i < CELL_COUNT; ++i) {
        if (!cells[i].x && !cells[i].y) {
            cells[i].x = cells[i - 1].x + cells[i - 1].width;
            cells[i].y = cells[i - 1].y;
        }

        if (cells[i].x + cells[i].width > window.size.width) {
            window.size.width = cells[i].x + cells[i].width;
        }

        if (cells[i].y + CELL_H > window.size.height) {
            window.size.height = cells[i].y + CELL_H;
        }
    }
}

static void
draw_cell(cell_st *key, int pressed)
{
    rect_st rect;
    uint8_t fg = pressed ? gui_color_bg : gui_color_fg;
    uint8_t bg = pressed ? gui_color_fg : gui_color_bg;

    gui_rect_init(&rect, key->x, key->y, key->width + 1, CELL_H + 1);
    gui_surface_draw_rect(&window.origin, &rect, bg);

    if (!pressed) {
        gui_surface_draw_border(&window.origin, &rect, gui_color_fg);
    }

    gui_surface_draw_str_centered(&window.origin, &rect, &fonts[1], key->label, fg, bg);
    gui_surface_mark_dirty(&window.origin, &rect);
}

static void
draw_keyboard(void)
{
    int i;

    for (i = 0; i < CELL_COUNT; ++i) {
        draw_cell(&cells[i], 0);
    }
}

static cell_st *
find_key(uint16_t code)
{
    int i;

    for (i = 0; i < CELL_COUNT; ++i) {
        if (cells[i].code == code) {
            return &cells[i];
        }
    }

    return NULL;
}

static void
update_cell(uint16_t code, int escaped, int pressed)
{
    cell_st *key = NULL;

    if (escaped) {
        key = find_key(code | 0xe000);
    }

    if (!key) {
        key = find_key(code);
    }

    if (key && key->pressed != pressed) {
        draw_cell(key, pressed);
        key->pressed = pressed;
    }
}

static void
on_key_down(uint8_t key_code, uint8_t key_mods)
{
    int escaped = !!(key_mods & KEY_MOD_ESC);
    char key_char = key_char_for_code(key_code, key_mods);
    char key_char_to_print = MAX(key_char, ' ');


    update_cell(key_code, escaped, 1);

    if (key_code != last_key_code) {
        gui_status_set("Last key:%02X  Mods:%02X  Char:%02X (%c)",
            key_code, key_mods, key_char, key_char_to_print);
    }
}

static void
on_key_up(uint8_t key_code, uint8_t key_mods)
{
    int escaped = !!(key_mods & KEY_MOD_ESC);

    update_cell(key_code, escaped, 0);
}

static void
on_show(void)
{
    draw_keyboard();
}

static void
on_init(void)
{
    init_cells();

    gui_window_init(&window, window.size.width, window.size.height);

    app_keys.on_show = on_show;
    app_keys.on_key_down = on_key_down;
    app_keys.on_key_up = on_key_up;
}

global app_st app_keys = {
    "Keys",
    &icon_keys,
    on_init,
};
