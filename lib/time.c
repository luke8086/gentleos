/*
 * Copyright (c) 2026 luke8086
 * Distributed under the terms of GPL-2 License
 *
 * File: time.c - Date / time library
 */

#include <kernel.h>

static time_st time_base;
static uint32_t time_base_msecs;

global const char *TIME_MONTH_NAMES_SHORT[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

global const char *TIME_DAY_NAMES_SHORT[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

global const char *TIME_DAY_NAMES_LONG[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

global int
time_get_day_of_week(int day, int month, int year)
{
    static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };

    if (month < 3) {
        --year;
    }

    return (year + year / 4 - year / 100 + year / 400 + t[month - 1] + day) % 7;
}

global int
time_get_days_in_month(int month, int year)
{
    static int days_in_month[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    int is_leap = (year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0));

    return (is_leap && month == 2) ? 29 : days_in_month[month];
}

global int
time_equals(time_st *t1, time_st *t2)
{
    return t1->year == t2->year &&
        t1->month == t2->month &&
        t1->day == t2->day &&
        t1->hour == t2->hour &&
        t1->minute == t2->minute &&
        t1->second == t2->second;
}

global void
time_init(time_st *t, uint16_t year, uint8_t month, uint8_t day,
    uint8_t hour, uint8_t minute, uint8_t second)
{
    t->year = year;
    t->month = month;
    t->day = day;
    t->hour = hour;
    t->minute = minute;
    t->second = second;
}

global void
time_clear(time_st *t)
{
    time_init(t, 1, 1, 1, 0, 0, 0);
}

global void
time_copy(time_st *dst, time_st *src)
{
    memcpy(dst, src, sizeof(time_st));
}

global void
time_add_seconds(time_st *t, uint32_t secs)
{
    uint32_t carry, q, r;

    carry = (uint32_t)t->second + secs;
    (void)umod32(&r, carry, 60);
    (void)udiv32(&q, carry, 60);
    t->second = (uint8_t)r;

    carry = (uint32_t)t->minute + q;
    (void)umod32(&r, carry, 60);
    (void)udiv32(&q, carry, 60);
    t->minute = (uint8_t)r;

    carry = (uint32_t)t->hour + q;
    (void)umod32(&r, carry, 24);
    (void)udiv32(&q, carry, 24);
    t->hour = (uint8_t)r;

    while (q) {
        q -= 1;
        t->day += 1;

        if (t->day <= time_get_days_in_month(t->month, t->year)) {
            continue;
        }

        t->day = 1;
        t->month += 1;

        if (t->month <= 12) {
            continue;
        }

        t->month = 1;
        t->year += 1;
    }
}

global void
time_get(time_st *t)
{
    uint32_t elapsed_msecs, elapsed_secs;

    elapsed_msecs = krn_timer_get_msecs() - time_base_msecs;
    (void)udiv32(&elapsed_secs, elapsed_msecs, 1000);

    time_copy(t, &time_base);
    time_add_seconds(t, elapsed_secs);
}

global void
time_set(time_st *t, int set_rtc)
{
    time_copy(&time_base, t);
    time_base_msecs = krn_timer_get_msecs();

    if (set_rtc) {
        krn_rtc_set_time(t);
    }
}
