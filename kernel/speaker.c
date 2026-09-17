/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: speaker.c - Driver for PC speaker
 */

#include <kernel.h>

enum {
    PIT_CR2      = 0x42, /* PIT counter 2 data port */
    PIT_CWR      = 0x43, /* PIT control word register */
    PPI_PB       = 0x61, /* Port B of 8255A-5 PPI */
};

/* Used instead of silence to prevent glitching in QEMU */
#define REST_PITCH 59659U

static volatile speaker_state_st krn_speaker_state;

static void
krn_speaker_set_freq(uint16_t hz)
{
    uint8_t val;
    uint32_t divisor;

    /* If hz is 0, turn off the speaker */
    if (hz == 0) {
        val = krn_inb(PPI_PB);
        krn_outb(val & ~0x03, PPI_PB);
        return;
    }

    (void)udiv32(&divisor, PIT_FREQUENCY, hz);
    divisor = MIN(divisor, 0xffffUL);

    /* Configure counter 2 of PIT to mode 3 (square wave) */
    krn_outb(0xB6, PIT_CWR);

    /* Set counter 2 to the desired frequency */
    krn_outb((uint16_t)divisor & 0xFF, PIT_CR2);
    krn_outb(((uint16_t)divisor >> 8) & 0xFF, PIT_CR2);

    /* Enable speaker by setting bits 0 (speaker enable) and 1 (gate) on port 0x61 */
    val = krn_inb(PPI_PB);
    krn_outb(val | 0x03, PPI_PB);
}

/* Must be called while locked or in interrupt context */
static void
krn_speaker_start_note(void)
{
    const note_st far *note = krn_speaker_state.note;

    if (note == NULL || note->ticks == 0) {
        /* Keep song, owner and elapsed time for inspection */
        krn_speaker_state.state = SPEAKER_STATE_STOPPED;
        krn_speaker_state.note = NULL;
        krn_speaker_set_freq(0);
        return;
    }

    krn_speaker_state.note_ticks_left = note->ticks;

    krn_speaker_set_freq(note->pitch ? note->pitch : REST_PITCH);
}

global void
krn_speaker_get_state(speaker_state_st *out)
{
    krn_lock_t lock;

    lock = krn_lock();

    memcpy(out, (const void *)&krn_speaker_state, sizeof(*out));

    krn_unlock(lock);
}

global void
krn_speaker_play_song(const note_st far *notes, void *owner)
{
    krn_lock_t lock;

    lock = krn_lock();

    krn_speaker_state.state = SPEAKER_STATE_PLAYING;
    krn_speaker_state.song = notes;
    krn_speaker_state.song_owner = owner;
    krn_speaker_state.song_elapsed_ticks = 0;
    krn_speaker_state.note = krn_speaker_state.song;

    krn_speaker_start_note();

    krn_unlock(lock);
}

global void
krn_speaker_play_freq(uint16_t hz, void *owner)
{
    static note_st notes[2];
    krn_lock_t lock;

    lock = krn_lock();

    notes[0].pitch = hz;
    notes[0].ticks = 0xffff;
    notes[1].pitch = 0;
    notes[1].ticks = 0;

    krn_speaker_play_song(notes, owner);

    krn_unlock(lock);
}

global void
krn_speaker_pause(void *owner)
{
    krn_lock_t lock;

    lock = krn_lock();

    if (krn_speaker_state.song_owner != owner) {
        krn_unlock(lock);
        return;
    }

    if (krn_speaker_state.state == SPEAKER_STATE_PLAYING) {
        krn_speaker_state.state = SPEAKER_STATE_PAUSED;
        krn_speaker_set_freq(REST_PITCH);
    }

    krn_unlock(lock);
}

global void
krn_speaker_resume(void *owner)
{
    krn_lock_t lock;

    lock = krn_lock();

    if (krn_speaker_state.song_owner != owner) {
        krn_unlock(lock);
        return;
    }

    if (krn_speaker_state.state == SPEAKER_STATE_PAUSED) {
        krn_speaker_state.state = SPEAKER_STATE_PLAYING;

        krn_speaker_set_freq(krn_speaker_state.note->pitch
            ? krn_speaker_state.note->pitch : REST_PITCH);
    }

    krn_unlock(lock);
}

global void
krn_speaker_stop(void *owner)
{
    krn_lock_t lock;

    lock = krn_lock();

    if (krn_speaker_state.song_owner != owner) {
        krn_unlock(lock);
        return;
    }

    krn_speaker_state.state = SPEAKER_STATE_STOPPED;
    krn_speaker_state.song = NULL;
    krn_speaker_state.song_owner = NULL;
    krn_speaker_state.song_elapsed_ticks = 0;
    krn_speaker_state.note = NULL;
    krn_speaker_state.note_ticks_left = 0;
    krn_speaker_set_freq(0);

    krn_unlock(lock);
}

/* Must be called in interrupt context */
global void
krn_speaker_on_tick(void)
{
    if (krn_speaker_state.state != SPEAKER_STATE_PLAYING) {
        return;
    }

    ++krn_speaker_state.song_elapsed_ticks;
    --krn_speaker_state.note_ticks_left;

    if (krn_speaker_state.note_ticks_left > 0) {
        return;
    }

    ++krn_speaker_state.note;
    krn_speaker_start_note();
}

global void
krn_speaker_deinit(void)
{
    krn_lock_t lock = krn_lock();

    krn_speaker_stop(krn_speaker_state.song_owner);

    krn_unlock(lock);
}
