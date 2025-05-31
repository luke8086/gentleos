// --------------------------------------------------------------------------------------
// Copyright (c) 2014-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: rtc.c - Driver for Real Time Clock
// --------------------------------------------------------------------------------------

#include <kernel.h>

enum {
    RTC_PORT_ADDR = 0x70,
    RTC_PORT_DATA = 0x71,
};

static uint16_t
krn_rtc_parse_bcd(uint16_t bcd)
{
    uint16_t ret = 0;

    ret += ((bcd >> 8) & 0x0F) * 100;
    ret += ((bcd >> 4) & 0x0F) * 10;
    ret += ((bcd >> 0) & 0x0F);

    return ret;
}

static uint8_t
krn_rtc_get_reg(uint8_t reg)
{
    outb(reg, RTC_PORT_ADDR);
    return inb(RTC_PORT_DATA);
}

static uint8_t
krn_rtc_is_update_in_progress(void)
{
    return krn_rtc_get_reg(0x0a) & (0x80);
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

int
krn_rtc_are_times_equal(time_st *t1, time_st *t2)
{
    return t1->year == t2->year &&
        t1->month == t2->month &&
        t1->day == t2->day &&
        t1->hour == t2->hour &&
        t1->minute == t2->minute &&
        t1->second == t2->second;
}

void
krn_rtc_get_time(time_st *t)
{
    time_st t1, t2;
    uint8_t reg_b;
    int is_bcd, is_12h, is_pm;

    // Keep reading raw time until we obtain two identical values twice
    // in a row. This prevents getting an inconsistent state in case
    // we try to read it during RTC update
    do {
        while (krn_rtc_is_update_in_progress()) { };
        krn_rtc_read_raw_time(&t1);

        while (krn_rtc_is_update_in_progress()) { };
        krn_rtc_read_raw_time(&t2);
    } while (!krn_rtc_are_times_equal(&t1, &t2));

    // Check status flags
    reg_b = krn_rtc_get_reg(0x0b);
    is_bcd = !(reg_b & 0x04);
    is_12h = !(reg_b & 0x02);
    is_pm = !!(t2.hour & 0x80);

    // Clear the PM bit
    t2.hour = t2.hour & 0x7F;

    // Parse BCD values
    if (is_bcd) {
        t2.second = krn_rtc_parse_bcd(t2.second);
        t2.minute = krn_rtc_parse_bcd(t2.minute);
        t2.hour = krn_rtc_parse_bcd(t2.hour);
        t2.day = krn_rtc_parse_bcd(t2.day);
        t2.month = krn_rtc_parse_bcd(t2.month);
        t2.year = krn_rtc_parse_bcd(t2.year);
    }

    // Convert 12h format to 24h
    if (is_12h && is_pm) {
        t2.hour = (t2.hour + 12) % 24;
    }

    // Calculate full year
    t2.year += (t2.year < 70) ? 2000 : 1900;

    memcpy(t, &t2, sizeof(t2));
}
