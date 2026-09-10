/* CAN bit-timing solver.
 *
 * Pure arithmetic, no hardware - which is the point. Bit timing is where
 * most CAN bring-ups fail, and it is entirely computable in advance. A
 * node whose sample point sits in the wrong place does not fail cleanly:
 * it works on a short bench harness and produces sporadic form errors on
 * a real loom, which is the worst possible failure mode.
 *
 * A CAN bit is divided into an integer number of time quanta (TQ):
 *
 *   |<---------------- one nominal bit time ---------------->|
 *   | SYNC_SEG | PROP_SEG + PHASE_SEG1  |     PHASE_SEG2     |
 *   |   1 TQ   |<------- TSEG1 -------->|<---- TSEG2 ------->|
 *                                       ^
 *                                  sample point
 *
 *   TQ            = BRP / f_can          (BRP = baud rate prescaler)
 *   bit time      = (1 + TSEG1 + TSEG2) * TQ
 *   bitrate       = f_can / (BRP * (1 + TSEG1 + TSEG2))
 *   sample point  = (1 + TSEG1) / (1 + TSEG1 + TSEG2)
 *
 * SYNC_SEG is always exactly 1 TQ. TSEG1 lumps PROP_SEG and PHASE_SEG1
 * together because that is how nearly every controller exposes it.
 *
 * The sample point must be LATE in the bit, because PROP_SEG has to cover
 * the round-trip propagation delay across the whole bus plus both
 * transceivers - during arbitration and the ACK slot a node must see the
 * other end's dominant bit within the same bit time. CiA recommends 87.5%
 * for CANopen and J1939; 75% is common at 1 Mbit/s on short buses.
 */
#ifndef CAN_BITTIMING_H
#define CAN_BITTIMING_H

#include <stdint.h>
#include <stdbool.h>

/* Controller limits. Defaults match ST bxCAN / NXP FlexCAN-class parts;
 * override for a controller with a different register layout. */
typedef struct {
    uint32_t brp_min,   brp_max;
    uint32_t tseg1_min, tseg1_max;
    uint32_t tseg2_min, tseg2_max;
    uint32_t sjw_max;
    uint32_t tq_min,    tq_max;    /* total TQ per bit */
} can_btl_limits_t;

extern const can_btl_limits_t CAN_LIMITS_BXCAN;   /* TSEG1 1..16, TSEG2 1..8, SJW 1..4 */

typedef struct {
    uint32_t brp;            /* prescaler, 1-based                       */
    uint32_t tseg1;          /* TQ, excluding SYNC_SEG                   */
    uint32_t tseg2;          /* TQ                                       */
    uint32_t sjw;            /* resynchronisation jump width, TQ         */
    uint32_t total_tq;       /* 1 + tseg1 + tseg2                        */
    uint32_t bitrate_hz;     /* what you will actually get               */
    uint32_t sample_point_permille;  /* e.g. 875 = 87.5%                 */
    int32_t  bitrate_error_ppm;      /* signed, relative to requested    */
} can_bittiming_t;

/* Find the best timing for a requested bitrate and sample point.
 *
 * Returns false when no exact-bitrate solution exists at all - which is a
 * real and common outcome, and the reason CAN nodes so often run from a
 * crystal rather than an internal RC oscillator. Do NOT ship an
 * approximate bitrate: CAN has no per-frame resynchronisation beyond SJW,
 * and a few thousand ppm of error will bite on long frames.
 *
 * target_sp_permille: 875 is the usual answer. 750 is common at 1 Mbit/s.
 */
bool can_solve_bittiming(uint32_t f_can_hz,
                         uint32_t bitrate_hz,
                         uint32_t target_sp_permille,
                         const can_btl_limits_t *limits,
                         can_bittiming_t *out);

/* Maximum bus length this timing can tolerate, in metres, for a given
 * transceiver loop delay. This is the calculation that decides whether a
 * topology is legal, and almost nobody does it.
 *
 * The result is deliberately CONSERVATIVE: it reserves 1 TQ of
 * PHASE_SEG1 for phase-error correction rather than spending the whole
 * of TSEG1 on propagation. It is also very sensitive to the transceiver
 * loop delay you pass in - a 150 ns transceiver buys you roughly three
 * times the length of a 250 ns one at 1 Mbit/s. Published rules of thumb
 * ("40 m at 1 Mbit/s") assume a fast transceiver and no margin; use this
 * with your actual part's datasheet figure. */
uint32_t can_max_bus_length_m(const can_bittiming_t *t,
                              uint32_t f_can_hz,
                              uint32_t transceiver_loop_delay_ns);

#endif
