// --------------------------------------------------------------------------------------
// Copyright (c) 2014-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: mouse.c - Driver for PS/2 mouse
// --------------------------------------------------------------------------------------

#include <kernel.h>

enum {
    PS2_PORT_DATA = 0x60,
    PS2_PORT_STATUS = 0x64,
    PS2_PORT_COMMAND = 0x64,

    PS2_CMD_ENABLE_AUX = 0xA8,
    PS2_CMD_READ_CONFIG = 0x20,
    PS2_CMD_WRITE_CONFIG = 0x60,
    PS2_CMD_SEND_AUX = 0xD4,
    PS2_CMD_SET_DEFAULT = 0xF6,
    PS2_CMD_ENABLE_REPORTING = 0xF4,
};

static struct {
    int16_t x;
    int16_t y;
    uint16_t btn_left;
    uint16_t btn_right;
} mouse_state;

static void
krn_mouse_handle_packet(int8_t a, int8_t b, int8_t c)
{
    int32_t fb_width = krn_core_mboot_info->fb_width;
    int32_t fb_height = krn_core_mboot_info->fb_height;

    int32_t current_x = mouse_state.x + b;
    int32_t current_y = mouse_state.y - c;

    current_x = current_x > fb_width ? fb_width : current_x;
    current_x = current_x < 0 ? 0 : current_x;

    current_y = current_y > fb_height ? fb_height : current_y;
    current_y = current_y < 0 ? 0 : current_y;

    uint8_t btn_left = a & 0x01 ? 1 : 0;
    uint8_t btn_right = a & 0x02 ? 1 : 0;

    event_st event = {
        .type = EVENT_POINTER_MOVE,
        .pointer_x = current_x,
        .pointer_y = current_y,
    };

    if (btn_left && !mouse_state.btn_left) {
        event.type = EVENT_POINTER_DOWN;
    } else if (!btn_left && mouse_state.btn_left) {
        event.type = EVENT_POINTER_UP;
    } else if (btn_right && !mouse_state.btn_right) {
        event.type = EVENT_POINTER_ALT;
    }

    if (event.type == EVENT_POINTER_MOVE &&
        event.pointer_x == mouse_state.x &&
        event.pointer_y == mouse_state.y) {

        return;
    }

    mouse_state.x = current_x;
    mouse_state.y = current_y;
    mouse_state.btn_left = btn_left;
    mouse_state.btn_right = btn_right;

    (void)krn_event_ipush(event);
}

static void
krn_mouse_handle_intr(isr_stack_st *isr_stack _unsd)
{
    static uint8_t mouse_cycle = 0;
    static int8_t mouse_byte[3];

    uint8_t mouse_data = inb(PS2_PORT_DATA);

    // Synchronize incoming data
    if (mouse_cycle == 0 && (mouse_data & 8) == 0) {
        return;
    }

    mouse_byte[mouse_cycle++] = mouse_data;

    if (mouse_cycle == 3) {
        mouse_cycle = 0;
        krn_mouse_handle_packet(mouse_byte[0], mouse_byte[1], mouse_byte[2]);
    }
}

static void
krn_mouse_putc(uint8_t val, uint16_t port)
{
    // Wait for output buffer to be empty, assume the loop will take enough time
    for (volatile int i = 0; i < 1000000; ++i) {
        if ((inb(PS2_PORT_STATUS) & 2) == 0) {
            break;
        }
    }

    outb(val, port);
}

static uint8_t
krn_mouse_getc(uint16_t port)
{
    // Wait for input buffer to be empty, assume the loop will take enough time
    for (volatile int i = 0; i < 1000000; ++i) {
        if ((inb(PS2_PORT_STATUS) & 1) == 0) {
            break;
        }
    }

    return inb(port);
}

void
krn_mouse_init(void)
{
    // Set initial coordinates to the center of the screen
    mouse_state.x = krn_core_mboot_info->fb_width / 2;
    mouse_state.y = krn_core_mboot_info->fb_height / 2;

    // Activate the second PS/2 port
    krn_mouse_putc(PS2_CMD_ENABLE_AUX, PS2_PORT_COMMAND);
    krn_mouse_putc(PS2_CMD_READ_CONFIG, PS2_PORT_COMMAND);
    uint8_t status = krn_mouse_getc(PS2_PORT_DATA);
    krn_mouse_putc(PS2_CMD_WRITE_CONFIG, PS2_PORT_COMMAND);
    krn_mouse_putc(status | 0x02, PS2_PORT_DATA);

    // Send the default configuration to the mouse
    krn_mouse_putc(PS2_CMD_SEND_AUX, PS2_PORT_COMMAND);
    krn_mouse_putc(PS2_CMD_SET_DEFAULT, PS2_PORT_DATA);
    (void)krn_mouse_getc(PS2_PORT_DATA);

    // Enable mouse interrupts
    krn_mouse_putc(PS2_CMD_SEND_AUX, PS2_PORT_COMMAND);
    krn_mouse_putc(PS2_CMD_ENABLE_REPORTING, PS2_PORT_DATA);
    (void)krn_mouse_getc(PS2_PORT_DATA);

    // Enable interrupt handler
    krn_interrupt_set_handler(0x2c, krn_mouse_handle_intr);
}
