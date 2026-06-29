/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: bios.c - Wrappers for BIOS functions
 */

#include <lib.h>

global void
bios_putc(char c)
{
    regs_st regs;

    regs.h.ah = 0x0e;
    regs.h.al = c;
    regs.x.bx = 0;

    intr(0x10, &regs);
}

global void
bios_puts(const char *s)
{
    while (*s) {
        if ((*s) == '\n') {
            bios_putc('\r');
        }

        bios_putc(*s++);
    }
}

global uint16_t
bios_getc(void)
{
    regs_st regs;

    regs.h.ah = 0x00;
    intr(0x16, &regs);

    return regs.x.ax;
}

global uint16_t
bios_get_key(void)
{
    regs_st regs;
    key_st key;

    regs.h.ah = 0x01;
    intr(0x16, &regs);

    if (regs.x.flags & 0x40) {
        return 0;
    }

    regs.h.ah = 0x00;
    intr(0x16, &regs);
    key.p.code = regs.h.ah;

    regs.h.ah = 0x02;
    intr(0x16, &regs);
    key.p.mods =
        (KEY_MOD_SHIFT * ((regs.h.al & 0x03) != 0)) |
        (KEY_MOD_CTRL  * ((regs.h.al & 0x04) != 0)) |
        (KEY_MOD_ALT   * ((regs.h.al & 0x08) != 0));

    return key.encoded;
}

global void
bios_uart_init(void)
{
    regs_st regs;

    regs.h.ah = 0x00;
    regs.h.al = 0xe3; /* 8N1, 9600 */
    regs.x.dx = 0;

    intr(0x14, &regs);
}

global void
bios_uart_putc(char c)
{
    regs_st regs;

    regs.h.ah = 0x01;
    regs.h.al = c;
    regs.x.dx = 0;

    intr(0x14, &regs);
}

global void
bios_uart_puts(const char *s)
{
    while (*s) {
        if ((*s) == '\n') {
            bios_uart_putc('\r');
        }

        bios_uart_putc(*s++);
    }
}

global void
bios_reboot(void)
{
    void (far *reset)(void) = MK_FP(0xFFFF, 0);
    uint8_t far *bda = MK_FP(0x40, 0);

    *(uint16_t far *)(bda + 0x72) = 0x1234; /* Prefer warm boot */

    reset();
}
