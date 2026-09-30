/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: card.c - Generic code for implementing card games
 */

#include <gui.h>

static bitmap_st *card_rank_bmp[] = {
    &card_rank_a,
    &card_rank_2,
    &card_rank_3,
    &card_rank_4,
    &card_rank_5,
    &card_rank_6,
    &card_rank_7,
    &card_rank_8,
    &card_rank_9,
    &card_rank_10,
    &card_rank_j,
    &card_rank_q,
    &card_rank_k,
};

static bitmap_st *card_suit_bmp[] = {
    &sprite_heart_2,
    &sprite_diamnd_2,
    &sprite_club_2,
    &sprite_spade_2,
};

global void
card_deck_init(card_t *deck, int n)
{
    int i;

    for (i = 0; i < n; ++i) {
        deck[i] = i;
    }
}

global void
card_deck_shuffle(card_t *deck, int n)
{
    card_t tmp;
    int i, j;

    for (i = n - 1; i > 0; --i) {
        j = rand() % (i + 1);
        tmp = deck[i];
        deck[i] = deck[j];
        deck[j] = tmp;
    }
}

global int
card_pile_cascade_step(card_game_st *game, card_pile_st *p)
{
    int step, max_step;

    step = game->card_step;

    if (p->count > 1) {
        max_step = (p->rect.height - game->card_height) / (p->count - 1);
        step = MIN(step, max_step);
        step = MAX(step, 1);
    }

    return step;
}

global int
card_pile_top_y(card_game_st *game, card_pile_st *p)
{
    if (!p->is_cascade || p->count <= 1) {
        return p->rect.y;
    }

    return p->rect.y + (p->count - 1) * card_pile_cascade_step(game, p);
}

global card_t
card_pile_pop(card_pile_st *p)
{
    ASSERT(p->count > 0);
    return p->cards[--p->count];
}

global void
card_pile_push(card_pile_st *p, card_t c)
{
    if (p->replace_on_push) {
        p->cards[0] = c;
        p->count = 1;
        return;
    }

    ASSERT(p->count < p->capacity);
    p->cards[p->count++] = c;
}

global void
card_pile_uncover_top(card_pile_st *pile)
{
    if (pile->count > 0 && pile->face_up_from > pile->count - 1) {
        pile->face_up_from = pile->count - 1;
    }
}

global void
card_draw(card_game_st *game, int x, int y, card_t card, int is_sel)
{
    uint8_t fg = is_sel ? gui_color_bg : gui_color_fg;
    uint8_t bg = is_sel ? gui_color_fg : gui_color_bg;
    int rank = CARD_RANK(card);
    int suit = CARD_SUIT(card);
    rect_st r;

    gui_rect_init(&r, x, y, game->card_width, game->card_height);
    gui_surface_draw_rect(game->origin, &r, bg);
    gui_surface_draw_border(game->origin, &r, gui_color_fg);
    gui_surface_mark_dirty(game->origin, &r);

    if (card == CARD_EMPTY) {
        return;
    }

    gui_surface_draw_bitmap(game->origin, game->size,
        x + 3, y + 3, card_rank_bmp[rank], fg);

    gui_surface_draw_bitmap(game->origin, game->size,
        x + game->card_width - 8, y + 3, card_suit_bmp[suit], fg);

    gui_surface_mark_dirty(game->origin, &r);
}

global void
card_stub_draw(card_game_st *game, int x, int y, int height, card_t card)
{
    int rank = CARD_RANK(card);
    int suit = CARD_SUIT(card);
    rect_st r;

    gui_rect_init(&r, x, y, game->card_width, height);
    gui_surface_draw_rect(game->origin, &r, gui_color_bg);
    gui_surface_draw_border(game->origin, &r, gui_color_fg);

    gui_surface_draw_bitmap(game->origin, game->size,
        x + 3, y + 3, card_rank_bmp[rank], gui_color_fg);

    gui_surface_draw_bitmap(game->origin, game->size,
        x + game->card_width - 8, y + 3, card_suit_bmp[suit], gui_color_fg);

    gui_surface_mark_dirty(game->origin, &r);
}

global void
card_back_draw(card_game_st *game, int x, int y)
{
    rect_st rect, rect_inner;

    gui_rect_init(&rect, x, y, game->card_width, game->card_height);
    gui_surface_draw_rect(game->origin, &rect, gui_color_bg);
    gui_surface_draw_border(game->origin, &rect, gui_color_fg);

    gui_rect_copy(&rect_inner, &rect);
    gui_rect_shrink(&rect_inner, 2);
    gui_surface_draw_dots_pattern(game->origin, &rect_inner);

    gui_surface_mark_dirty(game->origin, &rect);
}

global void
card_back_stub_draw(card_game_st *game, int x, int y, int height)
{
    rect_st rect, rect_inner;

    gui_rect_init(&rect, x, y, game->card_width, height);
    gui_surface_draw_rect(game->origin, &rect, gui_color_bg);
    gui_surface_draw_border(game->origin, &rect, gui_color_fg);

    if (height > 4) {
        gui_rect_copy(&rect_inner, &rect);
        gui_rect_shrink(&rect_inner, 2);
        gui_surface_draw_dots_pattern(game->origin, &rect_inner);
    }

    gui_surface_mark_dirty(game->origin, &rect);
}

global void
card_pile_draw(card_game_st *game, card_pile_st *p)
{
    int x = p->rect.x;
    int top_y = card_pile_top_y(game, p);
    card_t top_card = CARD_PILE_TOP(p);
    int is_sel = top_card != CARD_EMPTY && CARD_PILE_IS_SELECTED(game, p);
    int is_top_face_down = (p->count > 0 && p->face_up_from > p->count - 1);
    int y, i, step;

    gui_surface_draw_rect(game->origin, &p->rect, gui_color_bg);

    if (p->is_cascade && p->count > 1) {
        step = card_pile_cascade_step(game, p);
        for (i = 0; i < p->count - 1; ++i) {
            y = p->rect.y + i * step;
            if (i < p->face_up_from) {
                card_back_stub_draw(game, x, y, step + 1);
            } else {
                card_stub_draw(game, x, y, step + 1, p->cards[i]);
            }
        }
    }

    if (is_top_face_down) {
        card_back_draw(game, x, top_y);
    } else {
        card_draw(game, x, top_y, top_card, is_sel);
    }

    if (game->cur_pile == p) {
        card_cursor_draw(game, 1);
    }

    gui_surface_mark_dirty(game->origin, &p->rect);
}

global void
card_cursor_draw(card_game_st *game, int visible)
{
    int x = game->cur_pile->rect.x;
    int y = card_pile_top_y(game, game->cur_pile);
    int is_sel = CARD_PILE_IS_SELECTED(game, game->cur_pile);
    uint8_t color = (visible ^ is_sel) ? gui_color_fg : gui_color_bg;
    rect_st r;

    gui_rect_init(&r, x + 1, y + 1, game->card_width - 2, game->card_height - 2);
    gui_surface_draw_border(game->origin, &r, color);
    gui_surface_mark_dirty(game->origin, &r);
}

global void
card_game_exec_cur_move(card_game_st *game)
{
    int i;
    int count = game->cur_move.count;
    card_pile_st *src = game->cur_move.src;
    card_pile_st *dst = game->cur_move.dst;

    game->cur_move.src = NULL;

    ASSERT(src != NULL);
    ASSERT(dst != NULL);

    ASSERT(count <= src->count);
    ASSERT(dst->replace_on_push || dst->count + count <= dst->capacity);

    for (i = 0; i < count; ++i) {
        card_pile_push(dst, src->cards[src->count - count + i]);
    }

    src->count -= count;

    if (src->is_cascade) {
        card_pile_uncover_top(src);
    }

    card_pile_draw(game, src);
    card_pile_draw(game, dst);
}

