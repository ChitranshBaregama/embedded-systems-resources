/* Tests for the ISO 15765-2 transport layer.
 *
 * The valuable half is the malformed and adversarial input. A diagnostic
 * transport sits behind the OBD connector on a vehicle, so "the length
 * field lied" is not a hypothetical - it is the first thing anyone with
 * a CAN interface tries.
 */
#include "../portable/can/isotp.h"
#include "test.h"
#include <stdlib.h>

/* ---- helpers -------------------------------------------------------- */

/* Run a full sender -> receiver exchange in memory and check the payload
 * arrives byte-for-byte. This is the loop that would otherwise need two
 * ECUs and a CAN interface. */
static bool round_trip(const uint8_t *msg, uint16_t len,
                       uint8_t block_size, uint8_t st_min,
                       uint8_t *out, uint16_t *out_len)
{
    isotp_tx_t tx;
    isotp_rx_t rx;
    uint8_t    frame[ISOTP_CAN_DLC];

    if (!isotp_tx_begin(&tx, msg, len)) { return false; }
    isotp_rx_init(&rx, ISOTP_MAX_PAYLOAD, block_size, st_min);

    for (int guard = 0; guard < 20000; guard++) {
        uint8_t dlc = isotp_tx_next(&tx, frame);
        if (dlc == 0u) {
            if (tx.state == ISOTP_TX_DONE)    { break; }
            if (tx.state == ISOTP_TX_ABORTED) { return false; }
            return false;             /* deadlocked waiting for an FC */
        }

        isotp_event_t ev = isotp_rx_frame(&rx, frame, dlc);

        if (ev == ISOTP_EV_SEND_FC) {
            /* The receiver asked us to slow down; hand its FC back. */
            isotp_tx_flow_control(&tx, rx.fc_frame, ISOTP_CAN_DLC);
        } else if (ev == ISOTP_EV_MESSAGE) {
            for (uint16_t i = 0u; i < rx.received; i++) { out[i] = rx.buf[i]; }
            *out_len = rx.received;
            return true;
        } else if (ev != ISOTP_EV_NONE) {
            return false;
        }
    }
    return false;
}

static void fill(uint8_t *b, uint16_t n)
{
    for (uint16_t i = 0u; i < n; i++) { b[i] = (uint8_t)(i * 7u + 3u); }
}

/* ---- single frame --------------------------------------------------- */

static void test_single_frame_round_trip(void)
{
    uint8_t msg[7], got[ISOTP_MAX_PAYLOAD];
    uint16_t got_len = 0u;
    fill(msg, 7u);
    CHECK(round_trip(msg, 7u, 0u, 0u, got, &got_len));
    CHECK_EQ(got_len, 7);
    CHECK_EQ(memcmp(got, msg, 7u), 0);
}

static void test_single_frame_pci_is_correct(void)
{
    /* A UDS "read DTC" request is 3 bytes. On the wire it must be
     * 03 19 02 FF, padded - PCI nibble 0, length 3. */
    const uint8_t uds[] = { 0x19, 0x02, 0xFF };
    isotp_tx_t tx;
    uint8_t    frame[ISOTP_CAN_DLC];

    CHECK(isotp_tx_begin(&tx, uds, 3u));
    CHECK_EQ(isotp_tx_next(&tx, frame), ISOTP_CAN_DLC);
    CHECK_EQ(frame[0], 0x03);          /* SF, length 3 */
    CHECK_EQ(frame[1], 0x19);
    CHECK_EQ(frame[2], 0x02);
    CHECK_EQ(frame[3], 0xFF);
    CHECK_EQ(tx.state, ISOTP_TX_DONE);
}

/* ---- multi frame ---------------------------------------------------- */

static void test_multi_frame_round_trip(void)
{
    uint8_t msg[100], got[ISOTP_MAX_PAYLOAD];
    uint16_t got_len = 0u;
    fill(msg, 100u);
    CHECK(round_trip(msg, 100u, 0u, 0u, got, &got_len));
    CHECK_EQ(got_len, 100);
    CHECK_EQ(memcmp(got, msg, 100u), 0);
}

static void test_maximum_length_message(void)
{
    /* 4095 is the ceiling of the 12-bit length field. 585 frames. */
    static uint8_t msg[ISOTP_MAX_PAYLOAD], got[ISOTP_MAX_PAYLOAD];
    uint16_t got_len = 0u;
    fill(msg, ISOTP_MAX_PAYLOAD);
    CHECK(round_trip(msg, ISOTP_MAX_PAYLOAD, 0u, 0u, got, &got_len));
    CHECK_EQ(got_len, ISOTP_MAX_PAYLOAD);
    CHECK_EQ(memcmp(got, msg, ISOTP_MAX_PAYLOAD), 0);
}

static void test_every_length_round_trips(void)
{
    /* Sweep every length. This catches the boundary bugs that live at
     * 7/8 (SF to FF) and at each multiple of 7 in the CF stream, and the
     * sequence-number wrap at 15 -> 0. */
    static uint8_t msg[600], got[ISOTP_MAX_PAYLOAD];
    fill(msg, 600u);
    for (uint16_t len = 1u; len <= 600u; len++) {
        uint16_t got_len = 0u;
        CHECK(round_trip(msg, len, 0u, 0u, got, &got_len));
        CHECK_EQ(got_len, len);
        CHECK_EQ(memcmp(got, msg, len), 0);
    }
}

static void test_sequence_number_wraps_past_15(void)
{
    /* 200 bytes needs 28 consecutive frames, so SN wraps 15 -> 0 twice.
     * Getting this wrong produces a transport that works for short
     * messages and fails on anything over ~110 bytes - a very common
     * and very confusing bug. */
    uint8_t msg[200], got[ISOTP_MAX_PAYLOAD];
    uint16_t got_len = 0u;
    fill(msg, 200u);
    CHECK(round_trip(msg, 200u, 0u, 0u, got, &got_len));
    CHECK_EQ(got_len, 200);
    CHECK_EQ(memcmp(got, msg, 200u), 0);
}

/* ---- flow control --------------------------------------------------- */

static void test_block_size_is_honoured(void)
{
    /* With BS=4 the sender must stop after 4 consecutive frames and wait.
     * Sending on regardless is the single most common ISO-TP bug, and it
     * only shows up against an ECU slow enough to care. */
    const uint8_t msg[60] = {0};
    isotp_tx_t tx;
    uint8_t frame[ISOTP_CAN_DLC];
    const uint8_t fc_cts[3] = { 0x30, 0x04, 0x00 };   /* CTS, BS=4 */

    CHECK(isotp_tx_begin(&tx, msg, 60u));
    CHECK_EQ(isotp_tx_next(&tx, frame), ISOTP_CAN_DLC);    /* FF */
    CHECK_EQ(tx.state, ISOTP_TX_WAIT_FC);
    CHECK_EQ(isotp_tx_next(&tx, frame), 0);                /* blocked */

    isotp_tx_flow_control(&tx, fc_cts, 3u);
    CHECK_EQ(tx.state, ISOTP_TX_SENDING);

    for (int i = 0; i < 4; i++) {
        CHECK_EQ(isotp_tx_next(&tx, frame), ISOTP_CAN_DLC);
    }
    /* Block exhausted - must stop, not continue. */
    CHECK_EQ(isotp_tx_next(&tx, frame), 0);
    CHECK_EQ(tx.state, ISOTP_TX_WAIT_FC);
}

static void test_receiver_emits_fc_every_block(void)
{
    uint8_t msg[100], got[ISOTP_MAX_PAYLOAD];
    uint16_t got_len = 0u;
    fill(msg, 100u);
    /* BS=3 forces repeated flow control. If the two sides disagree about
     * when an FC is due, this deadlocks and round_trip returns false. */
    CHECK(round_trip(msg, 100u, 3u, 20u, got, &got_len));
    CHECK_EQ(got_len, 100);
    CHECK_EQ(memcmp(got, msg, 100u), 0);
}

static void test_flow_control_wait_does_not_advance(void)
{
    const uint8_t msg[60] = {0};
    isotp_tx_t tx;
    uint8_t frame[ISOTP_CAN_DLC];
    const uint8_t fc_wait[3] = { 0x31, 0x00, 0x00 };   /* FS = WAIT */

    CHECK(isotp_tx_begin(&tx, msg, 60u));
    isotp_tx_next(&tx, frame);                          /* FF */
    isotp_tx_flow_control(&tx, fc_wait, 3u);
    CHECK_EQ(tx.state, ISOTP_TX_WAIT_FC);
    CHECK_EQ(tx.wait_count, 1);
    CHECK_EQ(isotp_tx_next(&tx, frame), 0);             /* still blocked */
}

static void test_overflow_aborts_the_sender(void)
{
    const uint8_t msg[60] = {0};
    isotp_tx_t tx;
    uint8_t frame[ISOTP_CAN_DLC];
    const uint8_t fc_ovflw[3] = { 0x32, 0x00, 0x00 };

    CHECK(isotp_tx_begin(&tx, msg, 60u));
    isotp_tx_next(&tx, frame);
    CHECK_EQ(isotp_tx_flow_control(&tx, fc_ovflw, 3u), ISOTP_EV_ERR_OVERFLOW);
    CHECK_EQ(tx.state, ISOTP_TX_ABORTED);
    CHECK_EQ(isotp_tx_next(&tx, frame), 0);
}

/* ---- malformed input ------------------------------------------------ */

static void test_receiver_rejects_oversize_declared_length(void)
{
    /* The attack: declare 4000 bytes to a receiver with a 64-byte buffer.
     * The correct response is FS=OVFLW on the bus, not a memcpy. */
    isotp_rx_t rx;
    isotp_rx_init(&rx, 64u, 0u, 0u);
    const uint8_t ff[8] = { 0x1F, 0xA0, 1,2,3,4,5,6 };  /* len = 0xFA0 = 4000 */
    CHECK_EQ(isotp_rx_frame(&rx, ff, 8u), ISOTP_EV_ERR_OVERFLOW);
    CHECK_EQ(rx.state, ISOTP_RX_IDLE);
    CHECK_EQ(rx.fc_frame[0] & 0x0F, ISOTP_FS_OVFLW);
    CHECK_EQ(rx.err_overflow, 1);
}

static void test_receiver_rejects_sf_longer_than_its_frame(void)
{
    /* SF claiming 7 payload bytes inside a 3-byte CAN frame. Trusting
     * the claimed length over the DLC reads 5 bytes past the frame. */
    isotp_rx_t rx;
    isotp_rx_init(&rx, 4095u, 0u, 0u);
    const uint8_t sf[3] = { 0x07, 0xAA, 0xBB };
    CHECK_EQ(isotp_rx_frame(&rx, sf, 3u), ISOTP_EV_ERR_LENGTH);
    CHECK_EQ(rx.err_length, 1);
}

static void test_receiver_rejects_zero_length_sf(void)
{
    isotp_rx_t rx;
    isotp_rx_init(&rx, 4095u, 0u, 0u);
    const uint8_t sf[8] = { 0x00, 0,0,0,0,0,0,0 };
    CHECK_EQ(isotp_rx_frame(&rx, sf, 8u), ISOTP_EV_ERR_LENGTH);
}

static void test_receiver_rejects_ff_declaring_under_8_bytes(void)
{
    /* Anything under 8 bytes must be a single frame. A short FF is
     * malformed, and some stacks mis-handle it into a negative
     * "remaining" count. */
    isotp_rx_t rx;
    isotp_rx_init(&rx, 4095u, 0u, 0u);
    const uint8_t ff[8] = { 0x10, 0x03, 1,2,3,4,5,6 };
    CHECK_EQ(isotp_rx_frame(&rx, ff, 8u), ISOTP_EV_ERR_LENGTH);
}

static void test_consecutive_frame_without_first_frame(void)
{
    isotp_rx_t rx;
    isotp_rx_init(&rx, 4095u, 0u, 0u);
    const uint8_t cf[8] = { 0x21, 1,2,3,4,5,6,7 };
    CHECK_EQ(isotp_rx_frame(&rx, cf, 8u), ISOTP_EV_ERR_UNEXPECTED);
    CHECK_EQ(rx.err_unexpected, 1);
}

static void test_out_of_order_sequence_number(void)
{
    isotp_rx_t rx;
    isotp_rx_init(&rx, 4095u, 0u, 0u);
    const uint8_t ff[8]  = { 0x10, 0x14, 1,2,3,4,5,6 };   /* 20 bytes */
    const uint8_t cf3[8] = { 0x23, 1,2,3,4,5,6,7 };       /* SN 3, expected 1 */
    CHECK_EQ(isotp_rx_frame(&rx, ff, 8u), ISOTP_EV_SEND_FC);
    CHECK_EQ(isotp_rx_frame(&rx, cf3, 8u), ISOTP_EV_ERR_SEQUENCE);
    CHECK_EQ(rx.state, ISOTP_RX_IDLE);
    CHECK_EQ(rx.err_sequence, 1);
}

static void test_recovers_after_a_bad_message(void)
{
    /* One corrupted exchange must not poison the next good one. */
    isotp_rx_t rx;
    isotp_rx_init(&rx, 4095u, 0u, 0u);
    const uint8_t ff[8]  = { 0x10, 0x14, 1,2,3,4,5,6 };
    const uint8_t bad[8] = { 0x25, 1,2,3,4,5,6,7 };
    isotp_rx_frame(&rx, ff, 8u);
    isotp_rx_frame(&rx, bad, 8u);

    const uint8_t sf[8] = { 0x02, 0xAA, 0xBB, 0,0,0,0,0 };
    CHECK_EQ(isotp_rx_frame(&rx, sf, 8u), ISOTP_EV_MESSAGE);
    CHECK_EQ(rx.received, 2);
    CHECK_EQ(rx.buf[0], 0xAA);
}

static void test_fuzz_receiver_never_overruns(void)
{
    /* 300k random frames straight into the receiver. The assertion is
     * not about output - it is that `received` never exceeds the buffer
     * and the state stays inside the enum. Under ASan this is a real
     * memory-safety result for the reachable input space. */
    isotp_rx_t rx;
    isotp_rx_init(&rx, ISOTP_MAX_PAYLOAD, 8u, 0u);
    srand(20260910u);
    for (int i = 0; i < 300000; i++) {
        uint8_t f[ISOTP_CAN_DLC];
        uint8_t dlc = (uint8_t)(1 + (rand() % 8));
        for (int b = 0; b < 8; b++) { f[b] = (uint8_t)(rand() & 0xFF); }
        (void)isotp_rx_frame(&rx, f, dlc);
        CHECK(rx.state < ISOTP_RX_COUNT);
        CHECK(rx.received <= ISOTP_MAX_PAYLOAD);
        CHECK(rx.total_len <= ISOTP_MAX_PAYLOAD);
        CHECK(rx.received <= rx.total_len || rx.state == ISOTP_RX_IDLE);
    }
}

int main(void)
{
    printf("\nisotp (ISO 15765-2)\n");
    RUN(test_single_frame_round_trip);
    RUN(test_single_frame_pci_is_correct);
    RUN(test_multi_frame_round_trip);
    RUN(test_maximum_length_message);
    RUN(test_every_length_round_trips);
    RUN(test_sequence_number_wraps_past_15);
    RUN(test_block_size_is_honoured);
    RUN(test_receiver_emits_fc_every_block);
    RUN(test_flow_control_wait_does_not_advance);
    RUN(test_overflow_aborts_the_sender);
    RUN(test_receiver_rejects_oversize_declared_length);
    RUN(test_receiver_rejects_sf_longer_than_its_frame);
    RUN(test_receiver_rejects_zero_length_sf);
    RUN(test_receiver_rejects_ff_declaring_under_8_bytes);
    RUN(test_consecutive_frame_without_first_frame);
    RUN(test_out_of_order_sequence_number);
    RUN(test_recovers_after_a_bad_message);
    RUN(test_fuzz_receiver_never_overruns);
    TEST_SUMMARY();
}
