/* SPDX-License-Identifier: MIT
 * Teaching helpers: Gregorian 2000..2399, ordinary 86400-second days.
 * Not Unix time; no leap seconds, timezone conversion, or hardware access.
 */
#ifndef RTC_MATH_H
#define RTC_MATH_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    unsigned year, month, day, hour, minute, second;
} rtc_date;

static inline bool rtc_is_leap(unsigned year)
{
    return year % 4u == 0u && (year % 100u != 0u || year % 400u == 0u);
}

static inline unsigned rtc_month_days(unsigned year, unsigned month)
{
    static const unsigned char days[12] =
        {31,28,31,30,31,30,31,31,30,31,30,31};
    if (month < 1u || month > 12u) return 0u;
    return (unsigned)days[month-1u] +
        ((month == 2u && rtc_is_leap(year)) ? 1u : 0u);
}

static inline bool rtc_date_valid(const rtc_date *d)
{
    return d != NULL && d->year >= 2000u && d->year <= 2399u &&
        d->month >= 1u && d->month <= 12u && d->day >= 1u &&
        d->day <= rtc_month_days(d->year, d->month) &&
        d->hour < 24u && d->minute < 60u && d->second < 60u;
}

/* Callers extract device-specific flag bits before using this function.
 * Output remains unchanged on failure; max_value must be <= 99.
 */
static inline bool rtc_bcd_decode(uint8_t raw, unsigned max_value, unsigned *out)
{
    unsigned hi = (unsigned)(raw >> 4), lo = (unsigned)(raw & 15u);
    if (out == NULL || max_value > 99u || hi > 9u || lo > 9u) return false;
    unsigned value = 10u * hi + lo;
    if (value > max_value) return false;
    *out = value;
    return true;
}

static inline bool rtc_bcd_encode(unsigned value, uint8_t *out)
{
    if (out == NULL || value > 99u) return false;
    *out = (uint8_t)(((value / 10u) << 4) | (value % 10u));
    return true;
}

static inline bool rtc_to_epoch2000(const rtc_date *d, uint64_t *out)
{
    if (out == NULL || !rtc_date_valid(d)) return false;
    uint64_t days = 0;
    for (unsigned y = 2000u; y < d->year; ++y)
        days += rtc_is_leap(y) ? 366u : 365u;
    for (unsigned m = 1u; m < d->month; ++m)
        days += rtc_month_days(d->year, m);
    days += d->day - 1u;
    *out = days * UINT64_C(86400) + (uint64_t)d->hour * 3600u +
        (uint64_t)d->minute * 60u + d->second;
    return true;
}

/* 146097 days in the Gregorian cycle 2000..2399. */
#define RTC_EPOCH2000_LIMIT (UINT64_C(146097) * UINT64_C(86400))

static inline bool rtc_from_epoch2000(uint64_t seconds, rtc_date *out)
{
    if (out == NULL || seconds >= RTC_EPOCH2000_LIMIT) return false;
    rtc_date d = {2000u, 1u, 1u, 0u, 0u, 0u};
    uint64_t days = seconds / UINT64_C(86400);
    uint32_t within = (uint32_t)(seconds % UINT64_C(86400));
    for (;;) {
        unsigned n = rtc_is_leap(d.year) ? 366u : 365u;
        if (days < n) break;
        days -= n;
        ++d.year;
    }
    while (days >= rtc_month_days(d.year, d.month)) {
        days -= rtc_month_days(d.year, d.month);
        ++d.month;
    }
    d.day = (unsigned)days + 1u;
    d.hour = (unsigned)(within / UINT32_C(3600));
    d.minute = (unsigned)((within % UINT32_C(3600)) / UINT32_C(60));
    d.second = (unsigned)(within % UINT32_C(60));
    *out = d;
    return true;
}
#endif
