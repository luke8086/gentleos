/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: player.c - Music player app
 */

#include <gui.h>

enum {
    FONT_WIDTH = 5,
    FONT_HEIGHT = 8,

    PADDING = 8,

    CONTENT_X = PADDING,
    CONTENT_Y = PADDING,

    TITLE_Y = CONTENT_Y,
    TITLE_HEIGHT = FONT_HEIGHT,

    PROGRESS_Y = TITLE_Y + TITLE_HEIGHT + 5,
    PROGRESS_HEIGHT = 9,

    TIME_Y = PROGRESS_Y + PROGRESS_HEIGHT + 5,
    TIME_HEIGHT = FONT_HEIGHT,

    GRID_COLS = 1,
    GRID_ROWS = 8,
    GRID_CELL_WIDTH = 286,
    GRID_CELL_HEIGHT = FONT_HEIGHT + 2,
    GRID_WIDTH = GRID_WIDTH_SPACED(GRID_CELL_WIDTH, GRID_COLS),
    GRID_HEIGHT = GRID_HEIGHT_SPACED(GRID_CELL_HEIGHT, GRID_ROWS),
    GRID_X = CONTENT_X + 1,
    GRID_Y = TIME_Y + TIME_HEIGHT + PADDING + 1,

    CONTENT_WIDTH = GRID_CELL_WIDTH + 2,

    WINDOW_WIDTH = CONTENT_X + CONTENT_WIDTH + PADDING,
    WINDOW_HEIGHT = GRID_Y + GRID_HEIGHT + 1 + PADDING,

    TICK_FREQUENCY = SONG_TICK_FREQUENCY,
    REFRESH_TICKS = TICK_FREQUENCY * 25 / 100, /* 0.25s */

    SONG_MAX_COUNT = 16,
};

enum {
    PLAY_STATE_STOPPED,
    PLAY_STATE_PLAYING,
    PLAY_STATE_PAUSED,
};

typedef struct {
    window_st window;
    grid_st grid;

    file_st far *songs[SONG_MAX_COUNT];
    uint32_t song_ticks[SONG_MAX_COUNT];
    int song_count;

    int sel_index;
    int active_index;
    int scroll_top;
    int play_state;

    uint32_t last_elapsed_secs;
    uint32_t last_total_secs;
    int last_fill;
} app_state_st;

static app_state_st *app_state = (app_state_st *)gui_app_shared_buffer;

static const char *
get_song_name(int index)
{
    app_state_st *a = app_state;
    static char name[sizeof(a->songs[0]->name)];

    if (index < 0 || index >= a->song_count) {
        return "-";
    }

    memcpy_far(name, a->songs[index]->name, sizeof(name));

    return name;
}

static const note_st far *
get_song_notes(int index)
{
    app_state_st *a = app_state;
    static note_st stop_note = { 0, 0 };

    if (index < 0 || index >= a->song_count) {
        return &stop_note;
    }

    return a->songs[index]->u.addr;
}

static int
get_shown_index(void)
{
    app_state_st *a = app_state;

    return a->active_index >= 0 ? a->active_index : a->sel_index;
}

static uint32_t
get_shown_ticks(void)
{
    app_state_st *a = app_state;

    return a->song_count > 0 ? a->song_ticks[get_shown_index()] : 0;
}

static int
get_play_state(const speaker_state_st *st)
{
    speaker_state_st local_st;

    if (!st) {
        krn_speaker_get_state(&local_st);
        st = &local_st;
    }

    if (st->song_owner != &app_player) {
        return PLAY_STATE_STOPPED;
    }

    switch (st->state) {
    case SPEAKER_STATE_PLAYING: return PLAY_STATE_PLAYING;
    case SPEAKER_STATE_PAUSED: return PLAY_STATE_PAUSED;
    default: return PLAY_STATE_STOPPED;
    }
}

static void
get_ticks_duration(uint32_t ticks, unsigned *mins, unsigned *secs)
{
    uint32_t total_secs;

    (void)udiv32(&total_secs, ticks, TICK_FREQUENCY);

    total_secs = MIN(total_secs, 99UL * 60 + 59);

    *mins = (unsigned)total_secs / 60;
    *secs = (unsigned)total_secs % 60;
}

static void
update_status(void)
{
    app_state_st *a = app_state;

    if (a->song_count == 0) {
        gui_status_set("No songs found");
        return;
    }

    switch (a->play_state) {
    case PLAY_STATE_PLAYING: gui_status_set("Playing"); break;
    case PLAY_STATE_PAUSED: gui_status_set("Paused"); break;
    default: gui_status_set("Stopped"); break;
    }
}

static void
update_status_br(void)
{
    app_state_st *a = app_state;

    switch (a->play_state) {
    case PLAY_STATE_PLAYING: gui_status_set_br("P: Pause"); break;
    case PLAY_STATE_PAUSED: gui_status_set_br("P: Resume"); break;
    default: gui_status_set_br(""); break;
    }
}

static void
draw_title(void)
{
    app_state_st *a = app_state;
    rect_st rect;

    gui_rect_init(&rect, CONTENT_X, TITLE_Y, CONTENT_WIDTH, TITLE_HEIGHT);

    gui_surface_draw_rect(&a->window.origin, &rect, gui_color_bg);
    gui_surface_draw_str_centered(&a->window.origin, &rect, NULL,
        get_song_name(get_shown_index()), gui_color_fg, gui_color_bg);

    gui_surface_mark_dirty(&a->window.origin, &rect);
}

static void
draw_time(uint32_t elapsed_ticks)
{
    app_state_st *a = app_state;
    rect_st rect;
    unsigned elapsed_mins, elapsed_secs;
    unsigned total_mins, total_secs;
    char time[16];

    get_ticks_duration(elapsed_ticks, &elapsed_mins, &elapsed_secs);
    get_ticks_duration(get_shown_ticks(), &total_mins, &total_secs);

    snprintf(time, sizeof(time), "%02u:%02u / %02u:%02u",
        elapsed_mins, elapsed_secs, total_mins, total_secs);

    gui_rect_init(&rect, CONTENT_X, TIME_Y, CONTENT_WIDTH, TIME_HEIGHT);

    gui_surface_draw_rect(&a->window.origin, &rect, gui_color_bg);
    gui_surface_draw_str_centered(&a->window.origin, &rect, NULL, time,
        gui_color_fg, gui_color_bg);

    gui_surface_mark_dirty(&a->window.origin, &rect);
}

static void
draw_progress(uint32_t elapsed_ticks)
{
    app_state_st *a = app_state;
    rect_st rect;
    uint32_t total_ticks = get_shown_ticks();
    uint32_t ticks_per_pixel, fill = 0;

    gui_rect_init(&rect, CONTENT_X, PROGRESS_Y, CONTENT_WIDTH, PROGRESS_HEIGHT);
    gui_rect_shrink(&rect, 2);

    if (total_ticks > 0) {
        (void)udiv32(&ticks_per_pixel, total_ticks + rect.width / 2, rect.width);
        ticks_per_pixel = MAX(ticks_per_pixel, 1);
        (void)udiv32(&fill, MIN(elapsed_ticks, total_ticks), ticks_per_pixel);
        fill = MIN(fill, (uint32_t)rect.width);
    }

    if ((int)fill == a->last_fill) {
        return;
    }

    a->last_fill = (int)fill;

    gui_rect_init(&rect, CONTENT_X, PROGRESS_Y, CONTENT_WIDTH, PROGRESS_HEIGHT);
    gui_surface_draw_border(&a->window.origin, &rect, gui_color_fg);
    gui_surface_mark_dirty(&a->window.origin, &rect);

    gui_rect_shrink(&rect, 2);
    gui_surface_draw_rect(&a->window.origin, &rect, gui_color_bg);

    rect.width = (int)fill;
    gui_surface_draw_rect(&a->window.origin, &rect, gui_color_fg);
}

static void
draw_row(int index)
{
    app_state_st *a = app_state;
    rect_st rect;
    unsigned mins, secs;
    char duration[8];
    int row = index - a->scroll_top;
    uint8_t fg, bg;
    uint16_t text_x, text_y, duration_x;

    if (row < 0 || row >= a->grid.rows) {
        return;
    }

    gui_grid_cell_rect(&a->grid, 0, row, &rect);

    if (index >= a->song_count) {
        gui_surface_draw_rect(&a->window.origin, &rect, gui_color_bg);
        gui_surface_mark_dirty(&a->window.origin, &rect);
        return;
    }

    fg = (index == a->sel_index) ? gui_color_bg : gui_color_fg;
    bg = (index == a->sel_index) ? gui_color_fg : gui_color_bg;

    get_ticks_duration(a->song_ticks[index], &mins, &secs);
    snprintf(duration, sizeof(duration), "%02u:%02u", mins, secs);

    text_x = rect.x + 3;
    text_y = rect.y + (GRID_CELL_HEIGHT - FONT_HEIGHT) / 2;
    duration_x = rect.x + rect.width - 3 - (uint16_t)strlen(duration) * FONT_WIDTH;

    gui_surface_draw_rect(&a->window.origin, &rect, bg);
    gui_surface_draw_str(&a->window.origin, text_x, text_y, NULL, get_song_name(index), fg, bg);
    gui_surface_draw_str(&a->window.origin, duration_x, text_y, NULL, duration, fg, bg);
    gui_surface_mark_dirty(&a->window.origin, &rect);
}

static void
draw_list(void)
{
    app_state_st *a = app_state;
    rect_st rect;
    int row;

    gui_rect_init(&rect, GRID_X - 1, GRID_Y - 1, GRID_WIDTH + 2, GRID_HEIGHT + 2);
    gui_surface_draw_border(&a->window.origin, &rect, gui_color_fg);
    gui_surface_mark_dirty(&a->window.origin, &rect);

    for (row = 0; row < a->grid.rows; ++row) {
        draw_row(a->scroll_top + row);
    }
}

static void
sync_playback_state(const speaker_state_st *st, int refresh_status)
{
    app_state_st *a = app_state;
    speaker_state_st local_st;
    uint32_t total_ticks = get_shown_ticks();
    uint32_t elapsed_ticks = 0;
    uint32_t elapsed_secs, total_secs;

    if (!st) {
        krn_speaker_get_state(&local_st);
        st = &local_st;
    }

    a->play_state = get_play_state(st);

    if (a->play_state != PLAY_STATE_STOPPED) {
        elapsed_ticks = MIN(st->song_elapsed_ticks, total_ticks);
    }

    (void)udiv32(&elapsed_secs, elapsed_ticks, TICK_FREQUENCY);
    (void)udiv32(&total_secs, total_ticks, TICK_FREQUENCY);

    if (elapsed_secs != a->last_elapsed_secs || total_secs != a->last_total_secs) {
        draw_time(elapsed_ticks);

        a->last_elapsed_secs = elapsed_secs;
        a->last_total_secs = total_secs;
    }

    draw_progress(elapsed_ticks);

    if (refresh_status) {
        update_status();
        update_status_br();
    }
}

static void
select_song(int index)
{
    app_state_st *a = app_state;
    int prev_index = a->sel_index;

    if (a->song_count == 0) {
        return;
    }

    index = (index + a->song_count) % a->song_count;
    a->sel_index = index;

    if (index < a->scroll_top) {
        a->scroll_top = index;
        draw_list();
    } else if (index >= a->scroll_top + a->grid.rows) {
        a->scroll_top = index - a->grid.rows + 1;
        draw_list();
    } else {
        draw_row(prev_index);
        draw_row(index);
    }

    if (a->active_index < 0) {
        draw_title();
        sync_playback_state(NULL, 0);
    }
}

static void
play_song(int index)
{
    app_state_st *a = app_state;

    if (index < 0 || index >= a->song_count) {
        return;
    }

    a->active_index = index;
    krn_speaker_play_song(get_song_notes(index), &app_player);

    draw_title();
    sync_playback_state(NULL, 1);
}

static void
stop_song(void)
{
    app_state_st *a = app_state;

    krn_speaker_stop(&app_player);
    a->active_index = -1;

    draw_title();
    sync_playback_state(NULL, 1);
}

static void
pause_song(void)
{
    krn_speaker_pause(&app_player);
    sync_playback_state(NULL, 1);
}

static void
resume_song(void)
{
    krn_speaker_resume(&app_player);
    sync_playback_state(NULL, 1);
}

static void
advance_song(void)
{
    app_state_st *a = app_state;

    if (a->active_index + 1 >= a->song_count) {
        stop_song();
        return;
    }

    play_song(a->active_index + 1);
}

static void
on_key_down(uint8_t key_code, uint8_t key_mods)
{
    app_state_st *a = app_state;

    switch (key_code) {
    case KEY_UP:
        if (a->sel_index > 0) {
            select_song(a->sel_index - 1);
        }
        return;

    case KEY_DOWN:
        if (a->sel_index + 1 < a->song_count) {
            select_song(a->sel_index + 1);
        }
        return;

    case KEY_ENTER:
        play_song(a->sel_index);
        return;

    case KEY_P:
        switch (get_play_state(NULL)) {
        case PLAY_STATE_PLAYING: pause_song(); break;
        case PLAY_STATE_PAUSED: resume_song(); break;
        }
        return;
    }
}

static void
on_tick(void)
{
    static unsigned tick_count = 0;
    app_state_st *a = app_state;
    speaker_state_st st;

    if (a->active_index < 0) {
        return;
    }

    krn_speaker_get_state(&st);

    if (st.song_owner != &app_player) {
        stop_song();
        return;
    }

    if (st.state == SPEAKER_STATE_STOPPED) {
        advance_song();
        return;
    }

    if (st.state == SPEAKER_STATE_PLAYING && ++tick_count >= REFRESH_TICKS) {
        tick_count = 0;
        sync_playback_state(&st, 0);
    }
}

static void
init_grid(void)
{
    app_state_st *a = app_state;

    a->grid.cell_width = GRID_CELL_WIDTH;
    a->grid.cell_height = GRID_CELL_HEIGHT;
    a->grid.cols = GRID_COLS;
    a->grid.rows = GRID_ROWS;
    a->grid.x = GRID_X;
    a->grid.y = GRID_Y;
}

static void
init_songs(void)
{
    app_state_st *a = app_state;
    file_st far *file;
    uint16_t i, count = file_count();

    for (i = 0; i < count && a->song_count < SONG_MAX_COUNT; ++i) {
        file = file_get(i);

        if (file->type == FILE_TYPE_SONG) {
            a->song_ticks[a->song_count] = song_get_total_ticks(file->u.addr);
            a->songs[a->song_count] = file;
            ++a->song_count;
        }
    }
}

static void
on_show(void)
{
    app_state_st *a = app_state;

    gui_window_init(&a->window, WINDOW_WIDTH, WINDOW_HEIGHT);
    init_grid();
    init_songs();

    a->active_index = -1;
    a->last_elapsed_secs = 0xFFFFFFFF;
    a->last_total_secs = 0xFFFFFFFF;
    a->last_fill = -1;

    gui_window_draw(&a->window, gui_color_bg, 1);

    draw_list();
    draw_title();
    sync_playback_state(NULL, 1);
}

static void
on_close(void)
{
    krn_speaker_stop(&app_player);
}

static void
on_init(void)
{
    ASSERT(sizeof(app_state_st) <= sizeof(gui_app_shared_buffer));

    app_player.on_show = on_show;
    app_player.on_key_down = on_key_down;
    app_player.on_tick = on_tick;
    app_player.on_close = on_close;
}

global app_st app_player = {
    "Player",
    &icon_player,
    TICK_FREQUENCY,
    on_init,
};
