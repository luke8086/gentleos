/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: sokoban.c - Sokoban game
 */

#include <gui.h>

typedef struct {
	int unused;
} app_state_st;

/*
static app_state_st *app_state = (app_state_st *)gui_app_shared_buffer;
*/

static void
on_key_down(uint8_t key_code, uint8_t key_mods)
{
}

static void
on_show(void)
{
}

static void
on_init(void)
{
    ASSERT(sizeof(app_state_st) <= sizeof(gui_app_shared_buffer));

    app_sokoban.on_show = on_show;
    app_sokoban.on_key_down = on_key_down;
}

global app_st app_sokoban = {
    "Sokoban",
    &icon_sokoban,
    on_init,
};
