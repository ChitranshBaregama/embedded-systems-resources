/* SPDX-License-Identifier: MIT */
#include "rtc_math.h"
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned long checks;
#define CHECK(x) do { ++checks; if (!(x)) { \
    fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); return 1; \
} } while (0)

static bool same_date(const rtc_date *a, const rtc_date *b)
{
    return a->year == b->year && a->month == b->month && a->day == b->day &&
        a->hour == b->hour && a->minute == b->minute && a->second == b->second;
}

int main(int argc, char **argv)
{
    bool emit = argc == 2 && strcmp(argv[1], "--oracle") == 0;
    for (unsigned raw = 0; raw < 256u; ++raw) {
        unsigned out = 777u;
        bool valid = raw / 16u <= 9u && raw % 16u <= 9u;
        CHECK(rtc_bcd_decode((uint8_t)raw, 99u, &out) == valid);
        CHECK(out == (valid ? 10u * (raw / 16u) + raw % 16u : 777u));
    }
    for (unsigned value = 0; value <= 99u; ++value) {
        uint8_t raw = 0;
        unsigned decoded = 100u;
        CHECK(rtc_bcd_encode(value, &raw));
        CHECK(rtc_bcd_decode(raw, 99u, &decoded));
        CHECK(decoded == value);
    }
    unsigned output = 777u;
    uint8_t byte = 0x55;
    CHECK(!rtc_bcd_decode(0x60, 59u, &output) && output == 777u);
    CHECK(!rtc_bcd_decode(0x00, 100u, &output));
    CHECK(!rtc_bcd_decode(0x00, 99u, NULL));
    CHECK(!rtc_bcd_encode(100u, &byte) && byte == 0x55);
    CHECK(!rtc_bcd_encode(0u, NULL));
    CHECK(rtc_is_leap(2000u));
    CHECK(!rtc_is_leap(2100u));
    CHECK(!rtc_is_leap(2200u));
    CHECK(!rtc_is_leap(2300u));
    CHECK(rtc_is_leap(2400u));
    CHECK(rtc_month_days(2024u, 0u) == 0u);
    CHECK(rtc_month_days(2024u, 13u) == 0u);
    rtc_date invalid[] = {
        {1999,12,31,0,0,0}, {2400,1,1,0,0,0}, {2024,0,1,0,0,0},
        {2024,13,1,0,0,0}, {2024,1,0,0,0,0}, {2024,4,31,0,0,0},
        {2100,2,29,0,0,0}, {2023,2,29,0,0,0}, {2024,1,1,24,0,0},
        {2024,1,1,0,60,0}, {2024,1,1,0,0,60}, {UINT_MAX,1,1,0,0,0}
    };
    uint64_t seconds = 123u;
    for (size_t i = 0; i < sizeof invalid / sizeof invalid[0]; ++i) {
        CHECK(!rtc_date_valid(&invalid[i]));
        CHECK(!rtc_to_epoch2000(&invalid[i], &seconds) && seconds == 123u);
    }
    CHECK(!rtc_date_valid(NULL));
    CHECK(!rtc_to_epoch2000(NULL, &seconds));
    rtc_date start = {2000,1,1,0,0,0}, decoded = start;
    CHECK(!rtc_to_epoch2000(&start, NULL));
    CHECK(rtc_to_epoch2000(&start, &seconds) && seconds == 0u);
    CHECK(!rtc_from_epoch2000(RTC_EPOCH2000_LIMIT, &decoded));
    CHECK(same_date(&decoded, &start));
    CHECK(!rtc_from_epoch2000(UINT64_MAX, &decoded));
    CHECK(same_date(&decoded, &start));
    CHECK(!rtc_from_epoch2000(0, NULL));
    rtc_date next = {2001,1,1,0,0,0};
    CHECK(rtc_to_epoch2000(&next, &seconds) && seconds == UINT64_C(31622400));
    rtc_date century = {2100,3,1,0,0,0};
    CHECK(rtc_to_epoch2000(&century, &seconds) && seconds == UINT64_C(3160857600));
    uint64_t days = 0;
    for (unsigned y = 2000u; y <= 2399u; ++y) {
        for (unsigned m = 1u; m <= 12u; ++m) {
            for (unsigned day = 1u; day <= rtc_month_days(y, m); ++day) {
                rtc_date d = {y,m,day,0,0,0};
                CHECK(rtc_to_epoch2000(&d, &seconds));
                CHECK(seconds == days * UINT64_C(86400));
                CHECK(rtc_from_epoch2000(seconds, &decoded));
                CHECK(same_date(&d, &decoded));
                if (emit) printf("%u,%u,%u,%" PRIu64 "\n", y,m,day,seconds);
                d.hour = 23u; d.minute = 59u; d.second = 59u;
                CHECK(rtc_to_epoch2000(&d, &seconds));
                CHECK(seconds == days * UINT64_C(86400) + 86399u);
                CHECK(rtc_from_epoch2000(seconds, &decoded));
                CHECK(same_date(&d, &decoded));
                d.hour = 12u; d.minute = 34u; d.second = 56u;
                CHECK(rtc_to_epoch2000(&d, &seconds));
                CHECK(seconds == days * UINT64_C(86400) + 45296u);
                CHECK(rtc_from_epoch2000(seconds, &decoded));
                CHECK(same_date(&d, &decoded));
                ++days;
            }
        }
    }
    CHECK(days == UINT64_C(146097));
    CHECK(rtc_from_epoch2000(RTC_EPOCH2000_LIMIT - 1u, &decoded));
    rtc_date last = {2399,12,31,23,59,59};
    CHECK(same_date(&decoded, &last));
    fprintf(stderr, "PASS: %lu checks; %" PRIu64 " calendar days\n", checks, days);
    return 0;
}
