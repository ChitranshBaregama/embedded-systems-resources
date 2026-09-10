#include "can_bittiming.h"

const can_btl_limits_t CAN_LIMITS_BXCAN = {
    .brp_min = 1u,   .brp_max   = 1024u,
    .tseg1_min = 1u, .tseg1_max = 16u,
    .tseg2_min = 1u, .tseg2_max = 8u,
    .sjw_max = 4u,
    .tq_min = 8u,    .tq_max    = 25u,
};

static uint32_t abs_diff(uint32_t a, uint32_t b)
{
    return (a > b) ? (a - b) : (b - a);
}

bool can_solve_bittiming(uint32_t f_can_hz,
                         uint32_t bitrate_hz,
                         uint32_t target_sp_permille,
                         const can_btl_limits_t *lim,
                         can_bittiming_t *out)
{
    if (f_can_hz == 0u || bitrate_hz == 0u || lim == 0 || out == 0) {
        return false;
    }

    bool     found     = false;
    uint32_t best_sp_err = 0xFFFFFFFFu;
    uint32_t best_tq     = 0u;

    for (uint32_t brp = lim->brp_min; brp <= lim->brp_max; brp++) {
        uint32_t divisor = brp * bitrate_hz;
        if (divisor == 0u || (f_can_hz % divisor) != 0u) {
            /* Only exact solutions. An inexact bitrate is not a
             * "close enough" - it is a bus that works until it does not. */
            continue;
        }
        uint32_t total_tq = f_can_hz / divisor;
        if (total_tq < lim->tq_min || total_tq > lim->tq_max) {
            continue;
        }

        /* Choose the TSEG1/TSEG2 split that lands the sample point
         * closest to target. sample_point = (1 + tseg1) / total_tq. */
        for (uint32_t tseg1 = lim->tseg1_min; tseg1 <= lim->tseg1_max; tseg1++) {
            if ((1u + tseg1) >= total_tq) { break; }
            uint32_t tseg2 = total_tq - 1u - tseg1;
            if (tseg2 < lim->tseg2_min || tseg2 > lim->tseg2_max) {
                continue;
            }
            /* TSEG2 must be at least as long as SJW, and at least 2 TQ if
             * you want any resynchronisation headroom at all. */
            uint32_t sp = ((1u + tseg1) * 1000u) / total_tq;
            uint32_t err = abs_diff(sp, target_sp_permille);

            /* Rank: sample-point accuracy first, then MORE time quanta.
             * More TQ means finer resynchronisation granularity and more
             * tolerance of oscillator drift, so at equal accuracy the
             * longer bit is strictly better. */
            bool better = (err < best_sp_err)
                       || (err == best_sp_err && total_tq > best_tq);
            if (!better) { continue; }

            uint32_t sjw = (tseg2 < lim->sjw_max) ? tseg2 : lim->sjw_max;
            if (sjw > tseg1) { sjw = tseg1; }
            if (sjw == 0u)   { sjw = 1u; }

            out->brp        = brp;
            out->tseg1      = tseg1;
            out->tseg2      = tseg2;
            out->sjw        = sjw;
            out->total_tq   = total_tq;
            out->bitrate_hz = f_can_hz / (brp * total_tq);
            out->sample_point_permille = sp;
            out->bitrate_error_ppm     = 0;   /* exact, by construction */

            best_sp_err = err;
            best_tq     = total_tq;
            found       = true;
        }
    }

    return found;
}

uint32_t can_max_bus_length_m(const can_bittiming_t *t,
                              uint32_t f_can_hz,
                              uint32_t transceiver_loop_delay_ns)
{
    if (t == 0 || f_can_hz == 0u || t->brp == 0u) { return 0u; }

    /* PROP_SEG must cover the round trip: signal out to the far node and
     * that node's dominant bit back again, plus two transceiver delays.
     *
     *   prop_seg >= 2 * (t_bus + t_transceiver)
     *
     * TSEG1 = PROP_SEG + PHASE_SEG1, and PHASE_SEG1 must keep at least
     * 1 TQ for phase error correction, so the propagation budget is
     * (TSEG1 - 1) TQ. Signal propagation in twisted pair is about
     * 5 ns/m (roughly 2/3 c).
     */
    uint64_t tq_ns = ((uint64_t)t->brp * 1000000000ull) / f_can_hz;
    if (t->tseg1 < 2u) { return 0u; }

    uint64_t prop_ns = (uint64_t)(t->tseg1 - 1u) * tq_ns;
    uint64_t budget  = prop_ns / 2u;
    if (budget <= transceiver_loop_delay_ns) { return 0u; }

    uint64_t bus_ns = budget - transceiver_loop_delay_ns;
    return (uint32_t)(bus_ns / 5u);       /* 5 ns per metre */
}
