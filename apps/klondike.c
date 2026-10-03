/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: klondike.c - Klondike game
 */

#include <gui.h>

enum {
    CARD_WIDTH = 30,
    CARD_HEIGHT = 24,

    CARD_COUNT = 52,
    FOUND_COUNT = 4,
    COLUMN_COUNT = 7,

    GAP_Y = 6,
    GAP_X = 6,

    COLUMN_CARDS_STEP = 10,
    COLUMN_CARDS_MAX = 24,

    WINDOW_WIDTH = COLUMN_COUNT * CARD_WIDTH + (COLUMN_COUNT + 1) * GAP_X,
    WINDOW_HEIGHT = GUI_HEIGHT - 2 * STATUS_HEIGHT,

    TOP_PILES_Y = GAP_Y,
    COLUMNS_Y = TOP_PILES_Y + CARD_HEIGHT + GAP_Y,
    COLUMNS_H = WINDOW_HEIGHT - COLUMNS_Y - GAP_Y,

    PILE_STOCK = 1,
    PILE_WASTE = 2,
    PILE_FOUNDS = 3,
    PILE_COLUMNS = 4,

    STATE_DEFAULT = 0,
    STATE_WON = 1,
    STATE_AUTO_PENDING = 2,

    AUTO_MOVE_HIGHLIGHT_TICKS = MSECS_TO_TICKS(100),
    AUTO_MOVE_EXECUTE_TICKS = MSECS_TO_TICKS(300),
};

typedef struct {
    window_st window;

    card_t stock_cards[CARD_COUNT];
    card_pile_st stock;

    card_t waste_cards[CARD_COUNT];
    card_pile_st waste;

    card_t founds_cards[FOUND_COUNT];
    card_pile_st founds[FOUND_COUNT];

    card_t columns_cards[COLUMN_COUNT][COLUMN_CARDS_MAX];
    card_pile_st columns[COLUMN_COUNT];

    card_game_st game;
    int state;
    int ticks_waited;
} app_state_st;

static app_state_st *app_state = (app_state_st *)gui_app_shared_buffer;

static void
draw_all_piles(void)
{
    app_state_st *a = app_state;
    int i;

    card_pile_draw(&a->game, &a->stock);
    card_pile_draw(&a->game, &a->waste);

    for (i = 0; i < FOUND_COUNT; ++i) {
        card_pile_draw(&a->game, &a->founds[i]);
    }

    for (i = 0; i < COLUMN_COUNT; ++i) {
        card_pile_draw(&a->game, &a->columns[i]);
    }
}

static int
remaining_cards(void)
{
    app_state_st *a = app_state;
    int i, ret;

    ret = a->stock.count + a->waste.count;

    for (i = 0; i < COLUMN_COUNT; ++i) {
        ret += a->columns[i].count;
    }

    return ret;
}

static void
deal_cards(void)
{
    app_state_st *a = app_state;
    card_t deck[CARD_COUNT];
    int i, j, k;

    card_deck_init(deck, CARD_COUNT);
    card_deck_shuffle(deck, CARD_COUNT);

    a->stock.count = 0;
    a->waste.count = 0;

    for (i = 0; i < FOUND_COUNT; ++i) {
        a->founds[i].count = 0;
    }

    for (i = 0; i < COLUMN_COUNT; ++i) {
        a->columns[i].count = 0;
        a->columns[i].face_up_from = 0;
    }

    k = 0;
    for (i = 0; i < COLUMN_COUNT; ++i) {
        for (j = 0; j <= i; ++j) {
            card_pile_push(&a->columns[i], deck[k++]);
        }
        a->columns[i].face_up_from = i;
    }

    while (k < CARD_COUNT) {
        card_pile_push(&a->stock, deck[k++]);
    }

    a->game.cur_move.src = NULL;
    a->state = STATE_DEFAULT;
}

static void
update_status(void)
{
    app_state_st *a = app_state;

    if (a->state == STATE_WON) {
        gui_status_set("You Won! Press R to restart");
        return;
    }

    gui_status_set("Remaining cards: %d", remaining_cards());
}

static void
check_win(void)
{
    app_state_st *a = app_state;
    int i;

    for (i = 0; i < FOUND_COUNT; ++i) {
        if (a->founds[i].count == 0 || CARD_RANK(CARD_PILE_TOP(&a->founds[i])) != 12) {
            return;
        }
    }

    a->state = STATE_WON;
    update_status();
}

static int
get_max_valid_sequence_len(card_pile_st *p)
{
    int i;
    card_t curr, prev;

    if (p->count == 0) {
        return 0;
    }

    for (i = p->count - 1; i > p->face_up_from; --i) {
        curr = p->cards[i];
        prev = p->cards[i - 1];

        if (CARD_RANK(prev) != CARD_RANK(curr) + 1) {
            break;
        }
        if (CARD_COLOR(prev) == CARD_COLOR(curr)) {
            break;
        }
    }

    return p->count - i;
}

static int
card_should_auto_promote(card_t card)
{
    app_state_st *a = app_state;
    int rank = CARD_RANK(card);
    int suit = CARD_SUIT(card);
    int color = CARD_COLOR(card);
    int i;

    if (a->founds[suit].count == 0) {
        if (rank != 0) {
            return 0;
        }
    } else if (rank != CARD_RANK(CARD_PILE_TOP(&a->founds[suit])) + 1) {
        return 0;
    }

    if (rank <= 1) {
        return 1;
    }

    for (i = 0; i < FOUND_COUNT; ++i) {
        if (CARD_COLOR(i * 13) == color) {
            continue;
        }

        if (a->founds[i].count == 0 || CARD_RANK(CARD_PILE_TOP(&a->founds[i])) < rank - 1) {
            return 0;
        }
    }

    return 1;
}

static void
set_auto_move(card_pile_st *src, card_pile_st *dst)
{
    app_state_st *a = app_state;

    a->game.cur_move.src = src;
    a->game.cur_move.dst = dst;
    a->game.cur_move.count = 1;
    a->state = STATE_AUTO_PENDING;
    a->ticks_waited = 0;
}

static void
check_auto_move(void)
{
    app_state_st *a = app_state;
    int i;
    card_t card;

    card = CARD_PILE_TOP(&a->waste);

    if (card != CARD_EMPTY && card_should_auto_promote(card)) {
        set_auto_move(&a->waste, &a->founds[CARD_SUIT(card)]);
        return;
    }

    for (i = 0; i < COLUMN_COUNT; ++i) {
        card = CARD_PILE_TOP(&a->columns[i]);

        if (card != CARD_EMPTY && card_should_auto_promote(card)) {
            set_auto_move(&a->columns[i], &a->founds[CARD_SUIT(card)]);
            return;
        }
    }
}

static void
exec_move(void)
{
    app_state_st *a = app_state;

    card_game_exec_cur_move(&a->game);
    update_status();
    check_win();

    if (a->state != STATE_WON) {
        check_auto_move();
    }
}

static void
start_move(void)
{
    app_state_st *a = app_state;

    if (a->game.cur_pile->type != PILE_WASTE && a->game.cur_pile->type != PILE_COLUMNS) {
        return;
    }

    if (a->game.cur_pile->count == 0) {
        return;
    }

    a->game.cur_move.src = a->game.cur_pile;
    card_pile_draw(&a->game, a->game.cur_pile);
    update_status();
}

static void
cancel_move(void)
{
    app_state_st *a = app_state;
    card_pile_st *old = a->game.cur_move.src;

    a->game.cur_move.src = NULL;

    if (old != NULL) {
        card_pile_draw(&a->game, old);
    }

    update_status();
}

static void
show_error(const char *msg)
{
    cancel_move();
    gui_status_set("%s", msg);
}

static int
can_move_to_column(card_pile_st *col, card_t src_bottom)
{
    card_t dst_top;

    if (col->count == 0) {
        return CARD_RANK(src_bottom) == 12;
    }

    dst_top = CARD_PILE_TOP(col);

    if (CARD_RANK(dst_top) != CARD_RANK(src_bottom) + 1) {
        return 0;
    }

    if (CARD_COLOR(dst_top) == CARD_COLOR(src_bottom)) {
        return 0;
    }

    return 1;
}

static void
request_move_to_column(void)
{
    app_state_st *a = app_state;
    card_pile_st *src = a->game.cur_move.src;
    card_pile_st *dst = a->game.cur_move.dst;
    int count, max_count;
    card_t src_bottom;

    if (src->type == PILE_WASTE) {
        src_bottom = CARD_PILE_TOP(src);

        if (!can_move_to_column(dst, src_bottom)) {
            show_error("Invalid move");
            return;
        }

        a->game.cur_move.count = 1;
        exec_move();
        return;
    } else if (src->type == PILE_COLUMNS) {
        max_count = get_max_valid_sequence_len(src);

        for (count = max_count; count >= 1; --count) {
            src_bottom = src->cards[src->count - count];

            if (can_move_to_column(dst, src_bottom)) {
                a->game.cur_move.count = count;
                exec_move();
                return;
            }
        }
    }

    show_error("Invalid move");
}

static int
can_move_to_found(card_pile_st *found, card_t card)
{
    if (CARD_SUIT(card) != found->index) {
        return 0;
    }

    if (found->count == 0) {
        return CARD_RANK(card) == 0;
    }

    return CARD_RANK(CARD_PILE_TOP(found)) + 1 == CARD_RANK(card);
}

static void
request_move(void)
{
    app_state_st *a = app_state;

    a->game.cur_move.dst = a->game.cur_pile;

    if (a->game.cur_pile->type == PILE_COLUMNS) {
        request_move_to_column();
    } else {
        show_error("Invalid move");
    }
}

static void
request_promote_to_found(void)
{
    app_state_st *a = app_state;
    card_t card;
    card_pile_st *found;

    if (a->game.cur_move.src == NULL) {
        start_move();
    }

    if (a->game.cur_move.src == NULL) {
        return;
    }

    card = CARD_PILE_TOP(a->game.cur_move.src);
    if (card == CARD_EMPTY) {
        cancel_move();
        return;
    }

    found = &a->founds[CARD_SUIT(card)];
    if (!can_move_to_found(found, card)) {
        show_error("Invalid move");
        return;
    }

    a->game.cur_move.dst = found;
    a->game.cur_move.count = 1;
    exec_move();
}

static void
draw_card_from_stock(void)
{
    app_state_st *a = app_state;

    if (a->stock.count > 0) {
        card_pile_push(&a->waste, card_pile_pop(&a->stock));
    } else if (a->waste.count > 0) {
        while (a->waste.count > 0) {
            card_pile_push(&a->stock, card_pile_pop(&a->waste));
        }
    }

    card_pile_draw(&a->game, &a->stock);
    card_pile_draw(&a->game, &a->waste);
    update_status();
    check_auto_move();
}

static void
handle_space(void)
{
    app_state_st *a = app_state;

    if (a->game.cur_pile->type == PILE_STOCK) {
        if (a->game.cur_move.src == NULL) {
            draw_card_from_stock();
        } else {
            cancel_move();
        }

        return;
    }

    if (a->game.cur_move.src == NULL) {
        start_move();
    } else if (a->game.cur_move.src == a->game.cur_pile) {
        cancel_move();
    } else {
        request_move();
    }
}

static void
move_cursor(int dx, int dy)
{
    app_state_st *a = app_state;
    card_pile_st *p = a->game.cur_pile;
    int col;

    card_cursor_draw(&a->game, 0);

    if (p->type == PILE_COLUMNS) {
        if (dy < 0) {
            a->game.cur_pile = (p->index == 0) ? &a->stock : &a->waste;
        } else if (dx != 0) {
            col = MAX(0, MIN(COLUMN_COUNT - 1, p->index + dx));
            a->game.cur_pile = &a->columns[col];
        }
    } else if (dy > 0) {
        a->game.cur_pile = &a->columns[(p->type == PILE_STOCK) ? 0 : 1];
    } else if (dx > 0 && p->type == PILE_STOCK) {
        a->game.cur_pile = &a->waste;
    } else if (dx < 0 && p->type == PILE_WASTE) {
        a->game.cur_pile = &a->stock;
    }

    card_cursor_draw(&a->game, 1);
}

static void
restart_game(void)
{
    app_state_st *a = app_state;

    deal_cards();
    draw_all_piles();
    card_cursor_draw(&a->game, 1);
    update_status();
}

static void
on_key_down(uint8_t key_code, uint8_t key_mods)
{
    app_state_st *a = app_state;

    if (a->state == STATE_AUTO_PENDING) {
        return;
    }

    if (a->state == STATE_WON) {
        if (key_code == KEY_R) {
            restart_game();
        }
        return;
    }

    switch (key_code) {
        case KEY_LEFT: move_cursor(-1, 0); return;
        case KEY_RIGHT: move_cursor(1, 0); return;
        case KEY_UP: move_cursor(0, -1); return;
        case KEY_DOWN: move_cursor(0, 1); return;
        case KEY_SPACE: handle_space(); return;
        case KEY_F: request_promote_to_found(); return;
        case KEY_R: restart_game(); return;
        case KEY_ESC: cancel_move(); return;
    }
}

static void
on_tick(void)
{
    app_state_st *a = app_state;

    if (a->state != STATE_AUTO_PENDING) {
        return;
    }

    ++a->ticks_waited;

    if (a->ticks_waited == AUTO_MOVE_HIGHLIGHT_TICKS) {
        card_pile_draw(&a->game, a->game.cur_move.src);
        return;
    }

    if (a->ticks_waited >= AUTO_MOVE_EXECUTE_TICKS) {
        a->state = STATE_DEFAULT;
        exec_move();
    }
}

static void
init_game(void)
{
    app_state_st *a = app_state;
    int i;

    a->game.origin = &a->window.origin;

    a->game.card_width = CARD_WIDTH;
    a->game.card_height = CARD_HEIGHT;
    a->game.card_step = COLUMN_CARDS_STEP;

    a->game.cur_pile = &a->columns[0];

    a->stock.type = PILE_STOCK;
    a->stock.index = 0;
    a->stock.capacity = CARD_COUNT;
    a->stock.count = 0;
    a->stock.face_up_from = CARD_PILE_ALL_FACE_DOWN;
    a->stock.cards = a->stock_cards;
    a->stock.is_cascade = 0;
    a->stock.replace_on_push = 0;
    gui_rect_init(&a->stock.rect, GAP_X, TOP_PILES_Y, CARD_WIDTH, CARD_HEIGHT);

    a->waste.type = PILE_WASTE;
    a->waste.index = 0;
    a->waste.capacity = CARD_COUNT;
    a->waste.count = 0;
    a->waste.cards = a->waste_cards;
    a->waste.is_cascade = 0;
    a->waste.replace_on_push = 0;
    gui_rect_init(&a->waste.rect,
        GAP_X + (CARD_WIDTH + GAP_X), TOP_PILES_Y, CARD_WIDTH, CARD_HEIGHT);

    for (i = 0; i < FOUND_COUNT; ++i) {
        a->founds[i].type = PILE_FOUNDS;
        a->founds[i].index = i;
        a->founds[i].capacity = 1;
        a->founds[i].count = 0;
        a->founds[i].cards = &a->founds_cards[i];
        a->founds[i].is_cascade = 0;
        a->founds[i].replace_on_push = 1;

        gui_rect_init(&a->founds[i].rect,
            GAP_X + (i + 3) * (CARD_WIDTH + GAP_X), TOP_PILES_Y,
            CARD_WIDTH, CARD_HEIGHT);
    }

    for (i = 0; i < COLUMN_COUNT; ++i) {
        a->columns[i].type = PILE_COLUMNS;
        a->columns[i].index = i;
        a->columns[i].capacity = COLUMN_CARDS_MAX;
        a->columns[i].count = 0;
        a->columns[i].face_up_from = 0;
        a->columns[i].cards = a->columns_cards[i];
        a->columns[i].is_cascade = 1;
        a->columns[i].replace_on_push = 0;

        gui_rect_init(&a->columns[i].rect,
            GAP_X + i * (CARD_WIDTH + GAP_X), COLUMNS_Y,
            CARD_WIDTH, COLUMNS_H);
    }
}

static void
on_show(void)
{
    app_state_st *a = app_state;

    gui_window_init(&a->window, WINDOW_WIDTH, WINDOW_HEIGHT);
    init_game();

    gui_status_set_br("F: Promote  R: Restart");
    restart_game();
}

static void
on_init(void)
{
    ASSERT(sizeof(app_state_st) <= sizeof(gui_app_shared_buffer));

    app_klondike.on_show = on_show;
    app_klondike.on_key_down = on_key_down;
    app_klondike.on_tick = on_tick;
}

global app_st app_klondike = {
    "Klondike",
    &icon_klondike,
    on_init,
};
