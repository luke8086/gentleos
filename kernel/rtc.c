/*
 * Copyright (c) 2014-2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: rtc.c - Driver for Real Time Clock
 */

#include <kernel.h>

enum {
    RTC_PORT_ADDR = 0x70,
    RTC_PORT_DATA = 0x71,
    RTC_MAX_GET_ATTEMPTS = 100,
    RTC_MAX_UIP_ATTEMPTS = 100,
};

static uint16_t
from_bcd(uint16_t bcd)
{
    uint16_t ret = 0;

    ret += ((bcd >> 8) & 0x0F) * 100;
    ret += ((bcd >> 4) & 0x0F) * 10;
    ret += ((bcd >> 0) & 0x0F);

    return ret;
}

static uint8_t
to_bcd(uint8_t val)
{
    return ((val / 10) << 4) | (val % 10);
}

static int
valid_bcd(uint8_t val, uint8_t max)
{
    return (val & 0x0F) <= 9 && (val >> 4) <= 9 && val <= max;
}

static uint8_t
krn_rtc_get_reg(uint8_t reg)
{
    outb(reg, RTC_PORT_ADDR);
    return inb(RTC_PORT_DATA);
}

static void
krn_rtc_set_reg(uint8_t reg, uint8_t val)
{
    outb(reg, RTC_PORT_ADDR);
    outb(val, RTC_PORT_DATA);
}

static uint8_t
krn_rtc_is_updating(void)
{
    return krn_rtc_get_reg(0x0a) & (0x80);
}

static void
krn_rtc_wait_while_updating(void)
{
    size_t attempt = 0;

    while (krn_rtc_is_updating() && attempt < RTC_MAX_UIP_ATTEMPTS) {
        ++attempt;
    };
}

static void
krn_rtc_read_raw_time(time_st *t)
{
    t->second = krn_rtc_get_reg(0x00);
    t->minute = krn_rtc_get_reg(0x02);
    t->hour = krn_rtc_get_reg(0x04);
    t->day = krn_rtc_get_reg(0x07);
    t->month = krn_rtc_get_reg(0x08);
    t->year = krn_rtc_get_reg(0x09);
}

global int
krn_rtc_get_time(time_st *t)
{
    time_st t1, t2;
    uint8_t reg_b;
    int is_bcd, is_12h, is_pm;
    size_t attempt = 0;

    /* Check if status register A is set */
    if (krn_rtc_get_reg(0x0a) == 0xFF) {
        time_clear(t);
        return 0;
    }

    /*
     * Keep reading raw time until we obtain two identical values twice
     * in a row. This prevents getting an inconsistent state in case
     * we try to read it during CLOCK update
     */
    do {
        krn_rtc_wait_while_updating();
        krn_rtc_read_raw_time(&t1);

        krn_rtc_wait_while_updating();
        krn_rtc_read_raw_time(&t2);

        ++attempt;
    } while (!time_equals(&t1, &t2) && attempt < RTC_MAX_GET_ATTEMPTS);

    if (attempt == RTC_MAX_GET_ATTEMPTS) {
        time_clear(t);
        return 0;
    }

    /* Check status flags */
    reg_b = krn_rtc_get_reg(0x0b);
    is_bcd = !(reg_b & 0x04);
    is_12h = !(reg_b & 0x02);
    is_pm = !!(t2.hour & 0x80);

    /* Clear the PM bit */
    t2.hour = t2.hour & 0x7F;

    /* Validate BCD values */
    if (is_bcd &&
        (!valid_bcd(t2.second, 0x59)
        || !valid_bcd(t2.minute, 0x59)
        || !valid_bcd(t2.hour & 0x7F, 0x23)
        || !valid_bcd(t2.day, 0x31)
        || t2.day < 0x01
        || !valid_bcd(t2.month, 0x12)
        || t2.month < 0x01)
    ) {

        time_clear(t);
        return 0;
    }

    /* Parse BCD values */
    if (is_bcd) {
        t2.second = from_bcd(t2.second);
        t2.minute = from_bcd(t2.minute);
        t2.hour = from_bcd(t2.hour);
        t2.day = from_bcd(t2.day);
        t2.month = from_bcd(t2.month);
        t2.year = from_bcd(t2.year);
    }

    /* Convert 12h format to 24h */
    if (is_12h && is_pm) {
        t2.hour = (t2.hour + 12) % 24;
    }

    /* Calculate full year */
    t2.year += (t2.year < 70) ? 2000 : 1900;

    /* Just in case, ensure the returned values are sane */
    t2.year = MAX(1800, MIN(2200, t2.year));
    t2.month = MAX(1, MIN(12, t2.month));
    t2.day = MAX(1, MIN(31, t2.day));
    t2.hour = MIN(23, t2.hour);
    t2.minute = MIN(59, t2.minute);
    t2.second = MIN(59, t2.second);

    time_copy(t, &t2);
    return 1;
}

global void
krn_rtc_set_time(time_st *t)
{
    uint8_t reg_b;

    /* Update HW clock only if status register A is valid */
    if (krn_rtc_get_reg(0x0a) == 0xFF) {
        return;
    }

    /* Force BCD (clear DM) + 24h (set bit 1), preserve other bits */
    reg_b = krn_rtc_get_reg(0x0b);
    reg_b = (reg_b | 0x02) & ~0x04;

    /* Inhibit updates while writing */
    krn_rtc_set_reg(0x0b, reg_b | 0x80);

    krn_rtc_set_reg(0x00, to_bcd(t->second));
    krn_rtc_set_reg(0x02, to_bcd(t->minute));
    krn_rtc_set_reg(0x04, to_bcd(t->hour));
    krn_rtc_set_reg(0x07, to_bcd(t->day));
    krn_rtc_set_reg(0x08, to_bcd(t->month));
    krn_rtc_set_reg(0x09, to_bcd(t->year % 100));

    /* Resume updates */
    krn_rtc_set_reg(0x0b, reg_b);
}

global void
krn_rtc_init(void)
{
    int avail;
    time_st t;

    krn_debug_printf("Detecting RTC... ");

    avail = krn_rtc_get_time(&t);

    if (avail && t.year > 2000) {
        time_set(&t, 0);
        krn_debug_printf("available and set\n");
        return;
    }

    time_init(&t, DEFAULT_YEAR, DEFAULT_MONTH, DEFAULT_DAY,
        DEFAULT_HOUR, DEFAULT_MINUTE, 0);
    time_set(&t, 1);

    krn_debug_printf(avail ? "available but unset" : "unavailable");
    krn_debug_printf(", using default time\n");
}
