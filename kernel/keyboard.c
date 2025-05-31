// --------------------------------------------------------------------------------------
// Copyright (c) 2014-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: keyboard.c - Driver for PS/2 keyboard
// --------------------------------------------------------------------------------------

#include <kernel.h>

enum {
    PS2_PORT_DATA = 0x60,
};

enum {
    KBD_DEBUG = 0,
};

static const unsigned char krn_keyboard_map_default[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 254, 0,
    '-', 252, 0, 253, '+', 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const unsigned char krn_keyboard_map_shift[] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 254, 0,
    '-', 252, 0, 253, '+', 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static void
krn_keyboard_handle_intr(isr_stack_st *isr_stack __attribute__((unused)))
{
    static uint8_t shift = 0;

    uint8_t scan;
    int evtype;

    scan = inb(PS2_PORT_DATA);
    evtype = scan & 0x80 ? EVENT_KEY_UP : EVENT_KEY_DOWN,
    scan = scan & 0x7f;

    if (scan >= sizeof(krn_keyboard_map_default)) {
        return;
    }

    event_st ev = {
        .type = evtype,
        .key_code = scan,
        .key_char = shift ? krn_keyboard_map_shift[scan] : krn_keyboard_map_default[scan],
    };

    if (KBD_DEBUG) {
        krn_debug_printf("keyboard event: %s code=%02X char=%02X (%c)\n",
            ev.type == EVENT_KEY_UP ? "up" : "down",
            ev.key_code,
            ev.key_char,
            ev.key_char ? ev.key_char : ' '
        );
    }

    if (ev.key_code == 0x2a || ev.key_code == 0x36) {
        shift = (ev.type == EVENT_KEY_UP) ? 0 : 1;
    } else {
        (void)krn_event_ipush(ev);
    }
}

void
krn_keyboard_init(void)
{
    krn_interrupt_set_handler(0x21, krn_keyboard_handle_intr);
}
