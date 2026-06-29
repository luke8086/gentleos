/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: setup.c - Settings app
 */

#include <gui.h>

static const char *labels[] = {
    "Date:        -  -",
    "Time:      :  :",
    "Colors:",
    "Invert:",
    NULL,
};

enum {
    FONT_WIDTH = 5,
    FONT_HEIGHT = 8,

    ROW_HEIGHT = FONT_HEIGHT + 5,
    ROW_COUNT = (sizeof(labels) / sizeof(labels[0])) - 1,

    PADDING_Y = 10,
    PADDING_X = 15,

    WINDOW_WIDTH = 160,
    WINDOW_HEIGHT = PADDING_Y + ROW_HEIGHT * ROW_COUNT + PADDING_Y,

    CONTENT_X = PADDING_X,
    CONTENT_Y = PADDING_Y,
};

enum {
    FIELD_YEAR,
    FIELD_MONTH,
    FIELD_DAY,
    FIELD_HOUR,
    FIELD_MINUTE,
    FIELD_SECOND,
    FIELD_THEME,
    FIELD_COLORS,
};

static const char *hints[] = {
    "Current year",
    "Current month",
    "Current day",
    "Current hour",
    "Current minute",
    "Current second",
    "Color theme (VGA+)",
    "Inverted colors",
};

static window_st window;
static int cursor;

/*
 * Row 0: "Date:  YYYY-MM-DD"
 * Row 1: "Time:  HH:MM:SS"
 */

 static struct {
    uint8_t row;
    uint8_t col;
    uint8_t len;
    int min;
    int max;
    int val;
} fields[] = {
    { 0,  9,  4, 2000, 2099 },
    { 0, 14,  2,    1,   12 },
    { 0, 17,  2,    1,   31 },
    { 1,  9,  2,    0,   23 },
    { 1, 12,  2,    0,   59 },
    { 1, 15,  2,    0,   59 },
    { 2,  9, 11,    0, VGA_THEME_COUNT - 1 },
    { 3,  9, 11,    0,    1 },
};

#define FIELD_COUNT (sizeof(fields) / sizeof(fields[0]))

static void
draw_field(int n)
{
    rect_st rect;
    char buf[16];

    int val = fields[n].val;
    int is_current = (n == cursor);

    uint8_t fg = is_current ? gui_color_bg : gui_color_fg;
    uint8_t bg = is_current ? gui_color_fg : gui_color_bg;

    rect.x = CONTENT_X + fields[n].col * FONT_WIDTH;
    rect.y = CONTENT_Y + fields[n].row * ROW_HEIGHT + (ROW_HEIGHT - FONT_HEIGHT) / 2 - 1;
    rect.width = fields[n].len * FONT_WIDTH;
    rect.height = FONT_HEIGHT + 1;

    if (n == FIELD_COLORS) {
        snprintf(buf, sizeof(buf), val == 1 ? "Enable" : "Disable");
    } else if (n == FIELD_THEME) {
        snprintf(buf, sizeof(buf), "%s", krn_vga_themes[val].name);
    } else if (n == FIELD_YEAR) {
        snprintf(buf, sizeof(buf), "%04d", val);
    } else {
        snprintf(buf, sizeof(buf), "%02d", val);
    }

    gui_surface_draw_rect(&window.origin, &rect, bg);
    gui_surface_draw_str(&window.origin, rect.x, rect.y + 1, NULL, buf, fg, bg);
    gui_surface_mark_dirty(&window.origin, &rect);
}

static void
draw_all(void)
{
    int i;
    rect_st r;

    gui_window_draw(&window, gui_color_bg, 1);

    gui_surface_draw_str_lines(&window.origin, CONTENT_X,
        CONTENT_Y + (ROW_HEIGHT - FONT_HEIGHT) / 2, ROW_HEIGHT - FONT_HEIGHT,
        NULL, labels, gui_color_fg, gui_color_bg);

    for (i = 0; i < FIELD_COUNT; ++i) {
        draw_field(i);
    }

    gui_rect_init(&r, 0, 0, window.size.width, window.size.height);
    gui_surface_mark_dirty(&window.origin, &r);
}

static void
move_cursor(int dn)
{
    int prev_cursor = cursor;

    cursor = cursor + dn;
    cursor = MIN(cursor, FIELD_COUNT - 1);
    cursor = MAX(cursor, 0);

    gui_status_set("Edit: %s", hints[cursor]);

    if (prev_cursor == cursor) {
        return;
    }

    draw_field(prev_cursor);
    draw_field(cursor);
}

static void
edit_field(int dv)
{
    int min_val = fields[cursor].min;
    int max_val = fields[cursor].max;
    int prev_val = fields[cursor].val;
    int new_val = MAX(min_val, MIN(max_val, prev_val + dv));

    if (prev_val != new_val) {
        fields[cursor].val = new_val;
        draw_field(cursor);
    }
}

static void
load_fields(void)
{
    time_st time;

    time_get(&time);

    fields[FIELD_YEAR].val = time.year;
    fields[FIELD_MONTH].val = time.month;
    fields[FIELD_DAY].val = time.day;
    fields[FIELD_HOUR].val = time.hour;
    fields[FIELD_MINUTE].val = time.minute;
    fields[FIELD_SECOND].val = time.second;
    fields[FIELD_THEME].val = krn_vga_current_theme;
    fields[FIELD_COLORS].val = gui_colors_inverted;
}

static void
save_fields(void)
{
    time_st t;

    t.year = fields[FIELD_YEAR].val;
    t.month = fields[FIELD_MONTH].val;
    t.day = fields[FIELD_DAY].val;
    t.hour = fields[FIELD_HOUR].val;
    t.minute = fields[FIELD_MINUTE].val;
    t.second = fields[FIELD_SECOND].val;;
    time_set(&t, 1);

    if (fields[FIELD_THEME].val != krn_vga_current_theme) {
        krn_vga_set_theme(fields[FIELD_THEME].val);
    }

    gui_set_colors_inverted(fields[FIELD_COLORS].val);
    gui_status_set("Settings saved");
}

static void
on_key_down(uint8_t key_code, uint8_t key_mods)
{
    switch (key_code) {
    case KEY_UP: move_cursor(-1); return;
    case KEY_DOWN: move_cursor(1); return;
    case KEY_PGUP: edit_field(1); return;
    case KEY_PGDN: edit_field(-1); return;
    case KEY_S: save_fields(); return;
    }
}

static void
on_show(void)
{
    cursor = 0;
    load_fields();
    draw_all();
    move_cursor(0);

    gui_status_set_br("PgUp/PgDn: Edit  S: Save");
}

static void
on_init(void)
{
    gui_window_init(&window, WINDOW_WIDTH, WINDOW_HEIGHT);

    app_setup.on_show = on_show;
    app_setup.on_key_down = on_key_down;
}

global app_st app_setup = {
    "Setup",
    &icon_setup,
    on_init,
};
