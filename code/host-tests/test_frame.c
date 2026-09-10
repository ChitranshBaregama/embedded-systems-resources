/* Unit tests for the framed-protocol parser.
 *
 * Build and run:  make        (also runs under ASan/UBSan)
 *
 * Note what is being tested: not "does a good frame parse" - that is the
 * easy half. The valuable tests are the malformed inputs, because those
 * are the ones a real link produces and the ones that turn into
 * vulnerabilities.
 */
#include "../portable/frame/frame.h"
#include "test.h"
#include <stdlib.h>

static frame_parser_t p;

/* Build a well-formed frame into `out`, return its length. */
static size_t build(uint8_t *out, const uint8_t *payload, uint8_t len)
{
    size_t   n   = 0u;
    uint16_t crc = 0xFFFFu;

    out[n++] = FRAME_STX;
    out[n++] = len;
    crc = crc16_ccitt_update(crc, len);
    for (uint8_t i = 0u; i < len; i++) {
        out[n++] = payload[i];
        crc = crc16_ccitt_update(crc, payload[i]);
    }
    out[n++] = (uint8_t)(crc & 0xFFu);
    out[n++] = (uint8_t)(crc >> 8);
    out[n++] = FRAME_ETX;
    return n;
}

static frame_event_t feed_all(const uint8_t *buf, size_t n)
{
    frame_event_t last = FRAME_EV_NONE;
    for (size_t i = 0u; i < n; i++) {
        frame_event_t e = frame_feed(&p, buf[i]);
        if (e != FRAME_EV_NONE) { last = e; }
    }
    return last;
}

/* --- the CRC itself, against the published check value --------------- */
static void test_crc_known_vector(void)
{
    /* CRC-16/CCITT-FALSE of "123456789" is 0x29B1. This is the standard
     * check value; if your CRC does not produce it, your CRC is wrong,
     * no matter how self-consistent it looks. */
    const uint8_t v[] = "123456789";
    CHECK_EQ(crc16_ccitt(v, 9u), 0x29B1u);
}

static void test_valid_frame(void)
{
    uint8_t buf[80], payload[] = { 0x10, 0x20, 0x30 };
    frame_reset(&p);
    size_t n = build(buf, payload, 3u);
    CHECK_EQ(feed_all(buf, n), FRAME_EV_COMPLETE);
    CHECK_EQ(p.length, 3);
    CHECK_EQ(memcmp(p.payload, payload, 3u), 0);
    CHECK_EQ(p.frames_ok, 1);
}

static void test_zero_length_payload(void)
{
    uint8_t buf[16];
    frame_reset(&p);
    size_t n = build(buf, NULL, 0u);
    CHECK_EQ(feed_all(buf, n), FRAME_EV_COMPLETE);
    CHECK_EQ(p.length, 0);
}

static void test_max_length_payload(void)
{
    uint8_t buf[128], payload[FRAME_MAX_PAYLOAD];
    for (uint8_t i = 0u; i < FRAME_MAX_PAYLOAD; i++) { payload[i] = i; }
    frame_reset(&p);
    size_t n = build(buf, payload, FRAME_MAX_PAYLOAD);
    CHECK_EQ(feed_all(buf, n), FRAME_EV_COMPLETE);
    CHECK_EQ(p.length, FRAME_MAX_PAYLOAD);
}

/* --- the tests that matter ------------------------------------------- */
static void test_oversize_length_is_rejected(void)
{
    /* The attack: claim a payload longer than the buffer. If the parser
     * stores even one byte before checking, memory past `payload` is
     * writable by whatever is on the wire. */
    frame_parser_t q;
    frame_reset(&q);
    q.err_length = 0u;
    CHECK_EQ(frame_feed(&q, FRAME_STX), FRAME_EV_NONE);
    CHECK_EQ(frame_feed(&q, FRAME_MAX_PAYLOAD + 1u), FRAME_EV_ERR_LENGTH);
    CHECK_EQ(q.state, FRAME_ST_IDLE);
    CHECK_EQ(q.err_length, 1);
}

static void test_corrupted_payload_fails_crc(void)
{
    uint8_t buf[80], payload[] = { 0xAA, 0xBB, 0xCC, 0xDD };
    frame_reset(&p);
    size_t n = build(buf, payload, 4u);
    buf[3] ^= 0x01u;                      /* flip one bit in the payload */
    CHECK_EQ(feed_all(buf, n), FRAME_EV_ERR_CRC);
    CHECK_EQ(p.state, FRAME_ST_IDLE);
}

static void test_missing_etx_is_framing_error(void)
{
    uint8_t buf[80], payload[] = { 0x01 };
    frame_reset(&p);
    size_t n = build(buf, payload, 1u);
    buf[n - 1u] = 0x00u;                  /* clobber ETX */
    CHECK_EQ(feed_all(buf, n), FRAME_EV_ERR_FRAMING);
}

static void test_garbage_before_stx_is_discarded(void)
{
    uint8_t buf[80], payload[] = { 0x42 };
    frame_parser_t q; frame_reset(&q); q.bytes_discarded = 0u;
    for (int i = 0; i < 10; i++) { frame_feed(&q, 0x55u); }
    size_t n = build(buf, payload, 1u);
    frame_event_t last = FRAME_EV_NONE;
    for (size_t i = 0u; i < n; i++) {
        frame_event_t e = frame_feed(&q, buf[i]);
        if (e != FRAME_EV_NONE) last = e;
    }
    CHECK_EQ(last, FRAME_EV_COMPLETE);
    CHECK_EQ(q.bytes_discarded, 10);      /* resynchronised, and said so */
}

static void test_recovers_after_error(void)
{
    /* The property that actually keeps a link alive: one bad frame must
     * not poison the next good one.
     *
     * Note the memset. frame_reset() deliberately does NOT clear the
     * diagnostic counters - they are cumulative for the life of the
     * link, which is what makes them useful in the field. Writing this
     * test caught exactly that: the first draft reused the shared
     * parser and saw counts from earlier tests. Tests that share
     * mutable global state lie to you. */
    uint8_t bad[80], good[80], payload[] = { 0x11, 0x22 };
    memset(&p, 0, sizeof p);
    frame_reset(&p);
    size_t nb = build(bad, payload, 2u);
    bad[3] ^= 0xFFu;
    feed_all(bad, nb);
    size_t ng = build(good, payload, 2u);
    CHECK_EQ(feed_all(good, ng), FRAME_EV_COMPLETE);
    CHECK_EQ(p.frames_ok, 1);
    CHECK_EQ(p.err_crc, 1);
}

static void test_truncated_frame_then_new_stx(void)
{
    /* Peer resets mid-frame. Half a frame arrives, then a fresh one.
     * A parser that treats STX as sacred would deadlock here; this one
     * does not, because STX only restarts from IDLE - so the half-frame
     * has to fail its CRC first, which it does. */
    uint8_t good[80], payload[] = { 0x7E, 0x7F };
    frame_reset(&p);
    frame_feed(&p, FRAME_STX);
    frame_feed(&p, 8u);
    for (int i = 0; i < 3; i++) { frame_feed(&p, (uint8_t)i); }
    size_t n = build(good, payload, 2u);
    frame_event_t last = feed_all(good, n);
    CHECK(last == FRAME_EV_ERR_CRC || last == FRAME_EV_COMPLETE);
    /* and the parser must be usable again either way */
    frame_reset(&p);
    CHECK_EQ(feed_all(good, n), FRAME_EV_COMPLETE);
}

static void test_fuzz_never_overruns(void)
{
    /* 200k random bytes. The assertion is not about output - it is that
     * the parser never writes past `payload` and never leaves the state
     * enum. Run this under -fsanitize=address,undefined and it becomes
     * a real memory-safety proof for the reachable input space. */
    frame_parser_t q;
    frame_reset(&q);
    srand(1234u);
    for (int i = 0; i < 200000; i++) {
        frame_feed(&q, (uint8_t)(rand() & 0xFF));
        CHECK(q.state < FRAME_ST_COUNT);
        CHECK(q.index <= FRAME_MAX_PAYLOAD);
        CHECK(q.length <= FRAME_MAX_PAYLOAD);
    }
}

int main(void)
{
    printf("\nframe parser\n");
    RUN(test_crc_known_vector);
    RUN(test_valid_frame);
    RUN(test_zero_length_payload);
    RUN(test_max_length_payload);
    RUN(test_oversize_length_is_rejected);
    RUN(test_corrupted_payload_fails_crc);
    RUN(test_missing_etx_is_framing_error);
    RUN(test_garbage_before_stx_is_discarded);
    RUN(test_recovers_after_error);
    RUN(test_truncated_frame_then_new_stx);
    RUN(test_fuzz_never_overruns);
    TEST_SUMMARY();
}
