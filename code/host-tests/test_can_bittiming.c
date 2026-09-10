/* Tests for the CAN bit-timing solver.
 *
 * The interesting assertions are not "it returns something" but:
 *   - the bitrate is EXACT, never approximate
 *   - the sample point lands where CiA says it should
 *   - impossible combinations are reported as impossible rather than
 *     silently approximated
 */
#include "../portable/can/can_bittiming.h"
#include "test.h"

static const can_btl_limits_t *L = &CAN_LIMITS_BXCAN;

static void test_500k_at_36mhz(void)
{
    /* The canonical STM32F4 case: APB1 at 36 MHz, 500 kbit/s, 87.5%.
     * 36e6 / 500e3 = 72 = BRP * total_tq. BRP=9, total_tq=8 gives
     * a sample point of exactly 7/8 = 87.5%. */
    can_bittiming_t t;
    CHECK(can_solve_bittiming(36000000u, 500000u, 875u, L, &t));
    CHECK_EQ(t.bitrate_hz, 500000);
    CHECK_EQ(t.sample_point_permille, 875);
    CHECK_EQ(t.brp * t.total_tq, 72);
    CHECK_EQ(1u + t.tseg1 + t.tseg2, t.total_tq);
    CHECK(t.sjw >= 1u && t.sjw <= 4u);
    CHECK(t.sjw <= t.tseg2);
}

static void test_250k_at_36mhz(void)
{
    can_bittiming_t t;
    CHECK(can_solve_bittiming(36000000u, 250000u, 875u, L, &t));
    CHECK_EQ(t.bitrate_hz, 250000);
    CHECK_EQ(t.sample_point_permille, 875);
}

static void test_125k_at_48mhz(void)
{
    can_bittiming_t t;
    CHECK(can_solve_bittiming(48000000u, 125000u, 875u, L, &t));
    CHECK_EQ(t.bitrate_hz, 125000);
    CHECK(t.sample_point_permille >= 850u && t.sample_point_permille <= 900u);
}

static void test_1m_at_80mhz_prefers_75_percent(void)
{
    /* At 1 Mbit/s the bus is short and PROP_SEG matters less, so 75% is
     * the conventional choice - it leaves more of the bit after the
     * sample point for resynchronisation. */
    can_bittiming_t t;
    CHECK(can_solve_bittiming(80000000u, 1000000u, 750u, L, &t));
    CHECK_EQ(t.bitrate_hz, 1000000);
    CHECK(t.sample_point_permille >= 700u && t.sample_point_permille <= 800u);
}

static void test_bitrate_is_always_exact(void)
{
    /* Sweep every standard rate against every common peripheral clock.
     * Whenever a solution is returned, it must be EXACT - the solver is
     * never allowed to hand back an approximation. */
    const uint32_t clocks[]   = { 8000000u, 16000000u, 24000000u, 36000000u,
                                  40000000u, 42000000u, 48000000u, 80000000u };
    const uint32_t bitrates[] = { 10000u, 20000u, 50000u, 100000u, 125000u,
                                  250000u, 500000u, 800000u, 1000000u };
    int solved = 0;
    for (unsigned c = 0; c < sizeof clocks / sizeof clocks[0]; c++) {
        for (unsigned b = 0; b < sizeof bitrates / sizeof bitrates[0]; b++) {
            can_bittiming_t t;
            if (can_solve_bittiming(clocks[c], bitrates[b], 875u, L, &t)) {
                CHECK_EQ(t.bitrate_hz, bitrates[b]);
                CHECK_EQ(1u + t.tseg1 + t.tseg2, t.total_tq);
                CHECK_EQ(clocks[c], t.brp * t.total_tq * t.bitrate_hz);
                CHECK(t.tseg1 >= 1u && t.tseg1 <= 16u);
                CHECK(t.tseg2 >= 1u && t.tseg2 <= 8u);
                CHECK(t.total_tq >= 8u && t.total_tq <= 25u);
                solved++;
            }
        }
    }
    CHECK(solved > 40);          /* most combinations should be solvable */
}

static void test_impossible_combination_is_rejected(void)
{
    can_bittiming_t t;

    /* f_can / bitrate = 7. That must equal BRP * total_tq, and total_tq
     * has a hardware floor of 8, so no legal split exists. The right
     * answer is "no", not "close enough".
     *
     * Worth noting how easy it is to guess wrong here: 7 MHz at
     * 500 kbit/s gives a ratio of 14, which IS solvable (BRP=1,
     * 14 TQ) - the first draft of this test asserted otherwise and
     * failed. Ratio arithmetic beats intuition. */
    CHECK(!can_solve_bittiming(3500000u, 500000u, 875u, L, &t));

    /* Non-integer ratio: 10 MHz / 300 kbit/s = 33.33. No prescaler can
     * fix a clock that is not a multiple of the bitrate. This is exactly
     * why CAN nodes run from a crystal, not an internal RC. */
    CHECK(!can_solve_bittiming(10000000u, 300000u, 875u, L, &t));

    /* Bitrate above the classical CAN ceiling for this clock: 8 MHz
     * would need 4 TQ per bit, below the hardware minimum. */
    CHECK(!can_solve_bittiming(8000000u, 2000000u, 875u, L, &t));
    /* Degenerate inputs. */
    CHECK(!can_solve_bittiming(0u, 500000u, 875u, L, &t));
    CHECK(!can_solve_bittiming(36000000u, 0u, 875u, L, &t));
}

static void test_sjw_never_exceeds_phase_seg2(void)
{
    /* SJW > TSEG2 is a configuration the hardware will accept and that
     * corrupts the bit time when a resynchronisation shortens PHASE_SEG2
     * below zero. The solver must never emit it. */
    const uint32_t clocks[] = { 16000000u, 36000000u, 48000000u, 80000000u };
    const uint32_t rates[]  = { 125000u, 250000u, 500000u, 1000000u };
    for (unsigned c = 0; c < 4; c++) {
        for (unsigned r = 0; r < 4; r++) {
            can_bittiming_t t;
            if (can_solve_bittiming(clocks[c], rates[r], 875u, L, &t)) {
                CHECK(t.sjw <= t.tseg2);
                CHECK(t.sjw <= t.tseg1);
                CHECK(t.sjw >= 1u);
            }
        }
    }
}

static void test_bus_length_shrinks_as_bitrate_rises(void)
{
    /* The physical law behind every CAN topology rule: the faster the
     * bus, the shorter it must be, because PROP_SEG is a fraction of a
     * shrinking bit time. */
    can_bittiming_t slow, fast;
    CHECK(can_solve_bittiming(36000000u, 125000u,  875u, L, &slow));
    CHECK(can_solve_bittiming(36000000u, 1000000u, 750u, L, &fast));

    uint32_t len_slow = can_max_bus_length_m(&slow, 36000000u, 250u);
    uint32_t len_fast = can_max_bus_length_m(&fast, 36000000u, 250u);

    CHECK(len_slow > len_fast);
    CHECK(len_fast > 0u);
    /* Sanity against the published rules of thumb: ~500 m at 125 kbit/s,
     * ~40 m at 1 Mbit/s. Order of magnitude, not exact. */
    CHECK(len_slow > 200u);
    CHECK(len_fast < 100u);
}

int main(void)
{
    printf("\ncan bit timing\n");
    RUN(test_500k_at_36mhz);
    RUN(test_250k_at_36mhz);
    RUN(test_125k_at_48mhz);
    RUN(test_1m_at_80mhz_prefers_75_percent);
    RUN(test_bitrate_is_always_exact);
    RUN(test_impossible_combination_is_rejected);
    RUN(test_sjw_never_exceeds_phase_seg2);
    RUN(test_bus_length_shrinks_as_bitrate_rises);
    TEST_SUMMARY();
}
