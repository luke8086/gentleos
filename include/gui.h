/*
 * Copyright (c) 2025-2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: gui.h - GUI API
 */

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
    int x;
    int y;
    int width;
    int height;
} rect_st;

enum {
    FONT_COUNT = 3,
};

#define FONT_5x8 (&fonts[0])
#define FONT_4x8 (&fonts[1])
#define FONT_4x6 (&fonts[2])

typedef struct {
    size_st size;
    const char *name;
    const uint8_t *pixels;
} font_st;

typedef struct {
    size_st size;
    int pitch;
    const uint8_t *pixels;
} bitmap_st;

struct widget;
typedef struct widget widget_st;

struct widget {
    point_st *origin;
    rect_st rect;

    int tag1;
    int tag2;

    int active;
    int hide_border;

    void (*draw)(widget_st *);

    const char *label;
};

typedef struct {
    point_st origin;
    size_st size;
} window_st;

typedef struct {
    int cell_width;
    int cell_height;
    int cols;
    int rows;
    int x;
    int y;
} grid_st;

typedef struct {
    const char *name;
    bitmap_st *icon;
    void (*on_init)(void);
    void (*on_show)(void);
    void (*on_key_down)(uint8_t, uint8_t);
    void (*on_key_up)(uint8_t, uint8_t);
    void (*on_close)(void);
    void (*on_tick)(void);
} app_st;

enum {
    STATUS_HEIGHT = 16,
};

enum {
    GUI_WIDTH = 320,
    GUI_HEIGHT = 200,
    GUI_FB_PITCH = GUI_WIDTH / 8,
    GUI_FB_PLANE_SIZE = GUI_HEIGHT * GUI_FB_PITCH,
    GUI_VRAM_PITCH = GUI_WIDTH / 4,
};

#define GRID_WIDTH_SPACED(cell_width, cols) ((cell_width) * (cols) + (cols) - 1)
#define GRID_HEIGHT_SPACED(cell_height, rows) ((cell_height) * (rows) + (rows) - 1)

typedef uint8_t card_t;

enum {
    CARD_EMPTY = 0xff,
    CARD_PILE_ALL_FACE_DOWN = 0xff,
};

typedef struct {
    uint8_t type;
    int index;
    rect_st rect;
    uint8_t capacity;
    uint8_t count;
    uint8_t face_up_from;
    card_t *cards;
    unsigned is_cascade : 1;
    unsigned replace_on_push : 1;
} card_pile_st;

typedef struct {
    card_pile_st *src;
    card_pile_st *dst;
    int count;
} card_move_st;

typedef struct {
    point_st *origin;
    size_st *size;

    uint8_t card_width;
    uint8_t card_height;
    uint8_t card_step;

    card_move_st cur_move;
    card_pile_st *cur_pile;
} card_game_st;

#define CARD_RANK(card) ((card) % 13)
#define CARD_SUIT(card) ((card) / 13)
#define CARD_COLOR(card) (CARD_SUIT(card) / 2)
#define CARD_PILE_TOP(p) ((p)->count > 0 ? (p)->cards[(p)->count - 1] : CARD_EMPTY)
#define CARD_PILE_IS_SELECTED(game, pile) ((game)->cur_move.src == (pile))
#define CARD_SELECTED(game) \
    ((game)->cur_move.src ? CARD_PILE_TOP((game)->cur_move.src) : CARD_EMPTY)

#include "p_gui.h"
#include "p_data.h"
#include "p_apps.h"

#endif /* _GUI_H_ */
