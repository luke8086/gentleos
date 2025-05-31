// --------------------------------------------------------------------------------------
// Copyright (c) 2025-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: gui.h - GUI API
// --------------------------------------------------------------------------------------

#ifndef _GUI_H_
#define _GUI_H_

#include <kernel.h>

typedef struct {
    int x;
    int y;
} point_st;

typedef struct {
    int width;
    int height;
} size_st;

typedef struct {
    union {
        point_st pos;
        struct {
            int x;
            int y;
        };
    };

    union {
        size_st size;
        struct {
            int width;
            int height;
        };
    };
} rect_st;

enum {
    FONT_COUNT = 2,
};

typedef struct {
    size_st size;
    const char *name;
    const uint8_t *pixels;
} font_st;

typedef struct {
    size_st size;
    int alpha;
    const uint8_t *pixels;
} bitmap_st;

typedef struct {
    size_st size;
    int pitch;
    uint8_t *pixels;
} surface_st;

enum {
    WIDGET_TYPE_UNKNOWN = 0,
    WIDGET_TYPE_BUTTON = 1,
};

struct window;
typedef struct window window_st;

struct widget;
typedef struct widget widget_st;

struct widget {
    window_st *window;
    rect_st rect;

    int type;
    int tag1;
    int tag2;

    int active;
    int pressed;
    int press_on_move_in;
    int press_sticky;
    int press_fixed;
    int hide_border;

    void (*draw)(widget_st *);
    void (*on_pointer_down)(widget_st *, event_st event, point_st pos);
    void (*on_pointer_up)(widget_st *, event_st event, point_st pos);
    void (*on_pointer_out)(widget_st *, event_st event, point_st pos);
    void (*on_pointer_move)(widget_st *, event_st event, point_st pos);
    void (*on_pointer_alt)(widget_st *, event_st event, point_st pos);

    const char *label;
    font_st *font;

    bitmap_st *bitmap_regular;
    bitmap_st *bitmap_pressed;
};

struct window {
    rect_st rect;
    surface_st *surface;

    int visible;
    int active;

    const char *title;
    uint8_t bg_color;

    widget_st **widgets;
    size_t widgets_count;
    size_t widgets_capacity;
    widget_st *pressed_widget;

    int drag_active;
    point_st drag_start;

    void (*render_region)(window_st *, rect_st reg);
    void (*on_pointer)(window_st *, event_st event);
    void (*on_key_down)(window_st *, event_st event);
    void (*on_key_up)(window_st *, event_st event);
    void (*on_active_change)(window_st *);
};

typedef struct {
    int cell_width;
    int cell_height;
    int cols;
    int rows;
    int x;
    int y;
} grid_st;

typedef struct {
    bitmap_st *panel_icon_r;
    bitmap_st *panel_icon_a;

    void (*show)(void);
} app_st;

typedef void *timeout_payload;
typedef void (*timeout_callback_fn)(timeout_payload);

enum {
    COLOR_BLACK = 0x00,
    COLOR_WHITE = 0x0f,
    COLOR_RED = 0x04,
    COLOR_TITLE_BAR_ACTIVE = 0x2b,
    COLOR_TITLE_BAR_INACTIVE = 0x19,
    COLOR_WINDOW = 0x1a,
    COLOR_WINDOW_DARKER = 0x19,
    COLOR_BORDER = 0x14,
    COLOR_TEXT_ACTIVE = 0x00,
    COLOR_BUTTON_PRESSED = 0x00,
};

enum {
    KEY_UP = 0x48,
    KEY_DOWN = 0x50,
    KEY_LEFT = 0x4b,
    KEY_RIGHT = 0x4d,
};

enum {
    TITLE_BAR_HEIGHT = 24,
    PANEL_WIDTH = 64,
    STATUS_HEIGHT = 24,
};

#define GRID_WIDTH_SPACED(cell_width, cols) ((cell_width) * (cols) + (cols) - 1)
#define GRID_HEIGHT_SPACED(cell_height, rows) ((cell_height) * (rows) + (rows) - 1)

#include "proto_gui.h"
#include "proto_data.h"
#include "proto_apps.h"

#endif // _GUI_H_
