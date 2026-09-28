/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: freecell.c - FreeCell game
 */

#include <gui.h>

enum {
    CARD_WIDTH = 30,
    CARD_HEIGHT = 24,

    CARD_COUNT = 52,
    HOLD_COUNT = 4,
    FOUND_COUNT = 4,
    COLUMN_COUNT = 8,

    GAP_Y = 6,
    GAP_X = 6,

    COLUMN_CARDS_STEP = 10,
    COLUMN_CARDS_MAX = 21,

    WINDOW_WIDTH = COLUMN_COUNT * CARD_WIDTH + (COLUMN_COUNT + 1) * GAP_X,
    WINDOW_HEIGHT = GUI_HEIGHT - 2 * STATUS_HEIGHT,

    HOLDS_Y = GAP_Y,
    COLUMNS_Y = HOLDS_Y + CARD_HEIGHT + GAP_Y,
    COLUMNS_H = WINDOW_HEIGHT - COLUMNS_Y - GAP_Y,

    PILE_HOLDS = 1,
    PILE_FOUNDS = 2,
    PILE_COLUMNS = 3,

    STATE_DEFAULT = 0,
    STATE_ENTER_MOVE_COUNT = 1,
    STATE_WON = 2,
    STATE_AUTO_PENDING = 3,

    AUTO_MOVE_HIGHLIGHT_TICKS = MSECS_TO_TICKS(100),
    AUTO_MOVE_EXECUTE_TICKS = MSECS_TO_TICKS(300),
};

typedef struct {
    window_st window;

    card_t holds_cards[HOLD_COUNT];
    card_pile_st holds[HOLD_COUNT];

    card_t founds_cards[FOUND_COUNT];
    card_pile_st founds[FOUND_COUNT];

    card_t columns_cards[COLUMN_COUNT][COLUMN_CARDS_MAX];
    card_pile_st columns[COLUMN_COUNT];

    card_game_st game;
    int state;
    int ticks_waited;
} app_state_st;

static app_state_st *app_state = (app_state_st *)gui_app_shared_buffer;

static card_pile_st *
get_pile(int type, int idx)
{
    app_state_st *a = app_state;

    switch (type) {
    case PILE_HOLDS: return &a->holds[idx];
    case PILE_FOUNDS: return &a->founds[idx];
    case PILE_COLUMNS: return &a->columns[idx];
    default: return NULL;
    }
}

static int
remaining_cards(void)
{
    app_state_st *a = app_state;
    int i;
    int ret = 0;

    for (i = 0; i < HOLD_COUNT; ++i) {
        ret += a->holds[i].count;
    }

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
    int i, col;

    card_deck_init(deck, CARD_COUNT);
    card_deck_shuffle(deck, CARD_COUNT);

    for (i = 0; i < HOLD_COUNT; ++i) {
        a->holds[i].count = 0;
    }

    for (i = 0; i < FOUND_COUNT; ++i) {
        a->founds[i].count = 0;
    }

    for (i = 0; i < COLUMN_COUNT; ++i) {
        a->columns[i].count = 0;
    }

    for (i = 0; i < CARD_COUNT; ++i) {
        col = i % COLUMN_COUNT;
        card_pile_push(&a->columns[col], deck[i]);
    }

    a->game.cur_move.src = NULL;
    a->game.cur_pile = &a->columns[0];

    a->state = STATE_DEFAULT;
}


static void
draw_piles(void)
{
    app_state_st *a = app_state;
    int i;

    for (i = 0; i < HOLD_COUNT; ++i) {
        card_pile_draw(&a->game, &a->holds[i]);
    }

    for (i = 0; i < FOUND_COUNT; ++i) {
        card_pile_draw(&a->game, &a->founds[i]);
    }

    for (i = 0; i < COLUMN_COUNT; ++i) {
        card_pile_draw(&a->game, &a->columns[i]);
    }
}

static void
move_cursor(int dx, int dy)
{
    app_state_st *a = app_state;
    int new_type, new_idx, max_idx;

    card_cursor_draw(&a->game, 0);

    if (dy < 0) {
        new_type = PILE_HOLDS;
    } else if (dy > 0) {
        new_type = PILE_COLUMNS;
    } else {
        new_type = a->game.cur_pile->type;
    }

    max_idx = (new_type == PILE_COLUMNS ? COLUMN_COUNT :  HOLD_COUNT) - 1;

    new_idx = a->game.cur_pile->index + dx;
    new_idx = MAX(0, new_idx);
    new_idx = MIN(new_idx, max_idx);

    a->game.cur_pile = get_pile(new_type, new_idx);

    card_cursor_draw(&a->game, 1);
}

static void
update_status(void)
{
    app_state_st *a = app_state;
    int remaining = remaining_cards();

    if (a->state == STATE_WON) {
        gui_status_set("You Won! Press R to restart");
    } else {
        gui_status_set("Remaining cards: %d", remaining);
    }
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

static void
start_move(void)
{
    app_state_st *a = app_state;

    if (a->game.cur_pile->type != PILE_HOLDS && a->game.cur_pile->type != PILE_COLUMNS) {
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

    for (i = 0; i < HOLD_COUNT; ++i) {
        card = CARD_PILE_TOP(&a->holds[i]);

        if (card != CARD_EMPTY && card_should_auto_promote(card)) {
            set_auto_move(&a->holds[i], &a->founds[CARD_SUIT(card)]);
            return;
        }
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

static int
get_max_valid_sequence_len(card_pile_st *p)
{
    int count, i;
    card_t curr, prev;

    count = p->count;

    if (count == 0) {
        return 0;
    }

    for (i = count - 1; i > 0; --i) {
        curr = p->cards[i];
        prev = p->cards[i - 1];

        if (CARD_RANK(prev) != CARD_RANK(curr) + 1) {
            break;
        }

        if (CARD_COLOR(prev) == CARD_COLOR(curr)) {
            break;
        }
    }

    return count - i;
}

static int
get_max_movable_cards_count(card_pile_st *dst)
{
    app_state_st *a = app_state;
    int i;
    int avail_holds = 0;
    int avail_cols = 0;

    for (i = 0; i < HOLD_COUNT; ++i) {
        if (a->holds[i].count == 0) {
            ++avail_holds;
        }
    }

    for (i = 0; i < COLUMN_COUNT; ++i) {
        if (a->columns[i].count == 0 && dst != &a->columns[i]) {
            ++avail_cols;
        }
    }

    return (1 + avail_holds) * (1 + avail_cols);
}

static int
get_move_count(card_pile_st *src, card_pile_st *dst)
{
    card_t dst_top, src_card;
    int max_seq_len, max_movable_count, n;

    if (src->count == 0 || dst->count == 0) {
        return 0;
    }

    dst_top = CARD_PILE_TOP(dst);
    max_seq_len = get_max_valid_sequence_len(src);
    max_movable_count = get_max_movable_cards_count(dst);

    for (n = MIN(max_seq_len, max_movable_count); n >= 1; --n) {
        src_card = src->cards[src->count - n];

        if (CARD_RANK(dst_top) == CARD_RANK(src_card) + 1 &&
            CARD_COLOR(dst_top) != CARD_COLOR(src_card)) {
            return n;
        }
    }

    return 0;
}

static void
request_move_to_hold(void)
{
    app_state_st *a = app_state;

    if (a->game.cur_move.dst->count > 0) {
        show_error("Cell not empty");
        return;
    }

    a->game.cur_move.count = 1;
    exec_move();
}

static void
request_move_to_found(void)
{
    app_state_st *a = app_state;
    int expected_rank;
    card_t card;
    card_pile_st *found;

    if (a->game.cur_move.src == NULL) {
        start_move();
    }

    if (a->game.cur_move.src == NULL) {
        return;
    }

    card = CARD_SELECTED(&a->game);
    found = &a->founds[CARD_SUIT(card)];
    expected_rank = (found->count == 0) ? 0 : CARD_RANK(CARD_PILE_TOP(found)) + 1;

    if (CARD_RANK(card) != expected_rank) {
        show_error("Invalid move");
        return;
    }

    a->game.cur_move.dst = found;
    a->game.cur_move.count = 1;
    exec_move();
}

static void
request_move_to_nonempty_col(void)
{
    app_state_st *a = app_state;
    card_t dst_top, src_card;

    if (a->game.cur_move.src->type == PILE_HOLDS) {
        src_card = CARD_PILE_TOP(a->game.cur_move.src);
        dst_top = CARD_PILE_TOP(a->game.cur_move.dst);

        if (CARD_RANK(dst_top) != CARD_RANK(src_card) + 1 ||
            CARD_COLOR(dst_top) == CARD_COLOR(src_card)) {
            show_error("Invalid move");
            return;
        }

        a->game.cur_move.count = 1;
    } else if (a->game.cur_move.src->type == PILE_COLUMNS) {
        a->game.cur_move.count = get_move_count(a->game.cur_move.src, a->game.cur_move.dst);

        if (a->game.cur_move.count == 0) {
            show_error("Invalid move");
            return;
        }
    }

    exec_move();
}

static void
request_move_to_empty_col(void)
{
    app_state_st *a = app_state;
    int max_seq_len, max_movable;

    if (a->game.cur_move.src->type == PILE_HOLDS) {
        a->game.cur_move.count = 1;
        exec_move();
        return;
    }

    ASSERT(a->game.cur_move.src->type == PILE_COLUMNS);

    max_seq_len = get_max_valid_sequence_len(a->game.cur_move.src);
    max_movable = MIN(max_seq_len, get_max_movable_cards_count(a->game.cur_move.dst));

    if (max_movable <= 1) {
        a->game.cur_move.count = 1;
        exec_move();
        return;
    }

    a->state = STATE_ENTER_MOVE_COUNT;
    gui_status_set("How many? (0=max, 1-%d)", max_movable);
}

static void
handle_move_count(int key_code)
{
    app_state_st *a = app_state;
    int count, max_seq_len, max_movable;

    a->state = STATE_DEFAULT;

    count = key_number_for_code(key_code);

    if (count < 0) {
        cancel_move();
        return;
    }

    max_seq_len = get_max_valid_sequence_len(a->game.cur_move.src);
    max_movable = MIN(max_seq_len, get_max_movable_cards_count(a->game.cur_move.dst));

    if (count == 0) {
        count = max_movable;
    }

    if (count > max_movable) {
        show_error("Too many cards");
        return;
    }

    a->game.cur_move.count = count;
    exec_move();
}

static void
request_move(void)
{
    app_state_st *a = app_state;

    a->game.cur_move.dst = a->game.cur_pile;

    if (a->game.cur_pile->type == PILE_HOLDS) {
        request_move_to_hold();
    } else if (a->game.cur_pile->count == 0) {
        request_move_to_empty_col();
    } else {
        request_move_to_nonempty_col();
    }
}

static void
handle_space(void)
{
    app_state_st *a = app_state;

    if (a->game.cur_move.src == NULL) {
        start_move();
    } else if (a->game.cur_move.src == a->game.cur_pile) {
        cancel_move();
    } else {
        request_move();
    }
}

static void
restart_game(void)
{
    app_state_st *a = app_state;

    deal_cards();
    draw_piles();
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

    if (a->state == STATE_ENTER_MOVE_COUNT) {
        handle_move_count(key_code);
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
        case KEY_F: request_move_to_found(); return;
        case KEY_R: restart_game(); return;
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

    if (a->ticks_waited > AUTO_MOVE_EXECUTE_TICKS) {
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
    a->game.size = &a->window.size;

    a->game.card_width = CARD_WIDTH;
    a->game.card_height = CARD_HEIGHT;
    a->game.card_step = COLUMN_CARDS_STEP;

    a->game.cur_move.src = NULL;
    a->game.cur_move.dst = NULL;
    a->game.cur_move.count = 0;
    a->game.cur_pile = &a->columns[0];

    for (i = 0; i < HOLD_COUNT; ++i) {
        a->holds[i].type = PILE_HOLDS;
        a->holds[i].index = i;
        a->holds[i].capacity = 1;
        a->holds[i].count = 0;
        a->holds[i].cards = &a->holds_cards[i];
        a->holds[i].is_cascade = 0;
        a->holds[i].replace_on_push = 0;

        gui_rect_init(&a->holds[i].rect,
            i * (CARD_WIDTH + GAP_X), HOLDS_Y, CARD_WIDTH, CARD_HEIGHT);
    }

    for (i = 0; i < FOUND_COUNT; ++i) {
        a->founds[i].type = PILE_FOUNDS;
        a->founds[i].index = i;
        a->founds[i].capacity = 1;
        a->founds[i].count = 0;
        a->founds[i].cards = &a->founds_cards[i];
        a->founds[i].is_cascade = 0;
        a->founds[i].replace_on_push = 1;

        gui_rect_init(&a->founds[i].rect,
            (i + HOLD_COUNT) * (CARD_WIDTH + GAP_X) + 2 * GAP_X, HOLDS_Y,
            CARD_WIDTH, CARD_HEIGHT);
    }

    for (i = 0; i < COLUMN_COUNT; ++i) {
        a->columns[i].type = PILE_COLUMNS;
        a->columns[i].index = i;
        a->columns[i].capacity = COLUMN_CARDS_MAX;
        a->columns[i].count = 0;
        a->columns[i].cards = a->columns_cards[i];
        a->columns[i].is_cascade = 1;
        a->columns[i].replace_on_push = 0;

        gui_rect_init(&a->columns[i].rect,
            i * (CARD_WIDTH + GAP_X) + GAP_X, COLUMNS_Y,
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

    app_freecell.on_show = on_show;
    app_freecell.on_key_down = on_key_down;
    app_freecell.on_tick = on_tick;
}

global app_st app_freecell = {
    "FreeCell",
    &icon_freecell,
    on_init,
};
