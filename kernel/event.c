/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: event.c - Main event queue
 */

#include <kernel.h>

#pragma disable_message(201)

enum {
    EVENT_QUEUE_SIZE = 32,
    EVENT_QUEUE_DEBUG = 0,
};

static struct {
    uint16_t head;
    uint16_t tail;
    event_st events[EVENT_QUEUE_SIZE];
} krn_event_queue = { 0 };

static const char *
krn_event_format_queue(void)
{
    static char buf[64];

    snprintf(buf, sizeof(buf),
        "event_queue<%02d->%02d>",
        krn_event_queue.tail,
        krn_event_queue.head
    );

    return buf;
}

static const char *
krn_event_format(event_st *ev)
{
    static char buf[64];
    size_t n = sizeof(buf);

    if (ev->type == EVENT_KEY_DOWN) {
        snprintf(buf, n, "key_down<%x>", (uint16_t)ev->payload);
    } else if (ev->type == EVENT_KEY_UP) {
        snprintf(buf, n, "key_up<%x>", (uint16_t)ev->payload);
    } else if (ev->type == EVENT_TIMER_TICK) {
        snprintf(buf, n, "timer_tick<%lu>", ev->payload);
    } else {
        snprintf(buf, n, "unknown<%d>", ev->type);
    }

    return buf;
}

global void
krn_event_wait(event_st *out)
{
    while (1) {
        if (USE_BIOS_KEYBOARD) {
            krn_keyboard_handle_bios();
        }

        if (krn_event_count() > 0 && krn_event_pop(out) == 0) {
            break;
        }

        cpu_hlt();
    }
}

global int
krn_event_ipush(event_st *event)
{
    uint16_t next_head = (krn_event_queue.head + 1) % EVENT_QUEUE_SIZE;

    if (event->type == EVENT_TIMER_TICK &&
        krn_event_queue.head != krn_event_queue.tail &&
        krn_event_queue.events[krn_event_queue.head].type == EVENT_TIMER_TICK) {

        krn_event_queue.events[krn_event_queue.head].payload = event->payload;

        if (EVENT_QUEUE_DEBUG) {
            krn_debug_printf("%s: squashed %s\n", krn_event_format_queue(),
                krn_event_format(event));
        }

        return 0;
    }

    if (next_head == krn_event_queue.tail) {
        if (EVENT_QUEUE_DEBUG) {
            krn_debug_printf("%s: failed to push %s\n", krn_event_format_queue(),
                krn_event_format(event));
        }

        return -1;
    }

    memcpy(&krn_event_queue.events[krn_event_queue.head], event, sizeof(*event));

    krn_event_queue.head = next_head;

    if (EVENT_QUEUE_DEBUG && event->type != EVENT_TIMER_TICK) {
        krn_debug_printf("%s: pushed %s\n", krn_event_format_queue(),
            krn_event_format(event));
    }

    return 0;
}

global int
krn_event_push(event_st *event)
{
    uint16_t flags;
    int ret;

    flags = cpu_get_flags();
    cpu_cli();

    ret = krn_event_ipush(event);

    cpu_set_flags(flags);

    return ret;
}


global int
krn_event_pop(event_st *event)
{
    uint16_t flags;

    flags = cpu_get_flags();
    cpu_cli();

    if (krn_event_queue.head == krn_event_queue.tail) {
        if (EVENT_QUEUE_DEBUG) {
            krn_debug_printf("%s: pop failed\n", krn_event_format_queue());
        }

        cpu_set_flags(flags);
        return -1;
    }

    memcpy(event, &krn_event_queue.events[krn_event_queue.tail], sizeof(*event));

    krn_event_queue.tail = (krn_event_queue.tail + 1) % EVENT_QUEUE_SIZE;

    if (EVENT_QUEUE_DEBUG && event->type != EVENT_TIMER_TICK) {
        krn_debug_printf("%s: popped %s\n", krn_event_format_queue(),
            krn_event_format(event));
    }

    cpu_set_flags(flags);
    return 0;
}

global uint16_t
krn_event_count(void)
{
    uint16_t flags;
    uint16_t ret;

    flags = cpu_get_flags();
    cpu_cli();

    ret = (krn_event_queue.head - krn_event_queue.tail) % EVENT_QUEUE_SIZE;

    cpu_set_flags(flags);
    return ret;
}

