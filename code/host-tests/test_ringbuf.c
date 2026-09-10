/* Unit tests for the SPSC ring buffer.
 *
 * Single-threaded here on purpose: these tests pin down the INDEX
 * ARITHMETIC, which is where ring buffers actually go wrong - the
 * full/empty ambiguity, the wrap, the off-by-one at capacity.
 *
 * Concurrency is verified separately, on the target, by example 04,
 * where a real interrupt preempts a real consumer. Host tests cannot
 * prove ISR-safety and should not pretend to.
 */
#include "../portable/ringbuf/ringbuf.h"
#include "test.h"

static void test_starts_empty(void)
{
    ringbuf_t rb; rb_init(&rb);
    CHECK(rb_empty(&rb));
    CHECK(!rb_full(&rb));
    CHECK_EQ(rb_count(&rb), 0);
}

static void test_put_get_roundtrip(void)
{
    ringbuf_t rb; rb_init(&rb);
    uint8_t out = 0u;
    CHECK(rb_put(&rb, 0xA5u));
    CHECK_EQ(rb_count(&rb), 1);
    CHECK(rb_get(&rb, &out));
    CHECK_EQ(out, 0xA5);
    CHECK(rb_empty(&rb));
}

static void test_fifo_order(void)
{
    ringbuf_t rb; rb_init(&rb);
    for (uint32_t i = 0u; i < 10u; i++) { CHECK(rb_put(&rb, (uint8_t)i)); }
    for (uint32_t i = 0u; i < 10u; i++) {
        uint8_t out = 0u;
        CHECK(rb_get(&rb, &out));
        CHECK_EQ(out, i);
    }
}

static void test_fills_to_exactly_capacity(void)
{
    /* The bug this catches: implementations that mask the stored indices
     * can only hold CAPACITY-1 items and quietly lose a slot. Free-
     * running indices hold all CAPACITY. */
    ringbuf_t rb; rb_init(&rb);
    for (uint32_t i = 0u; i < RB_CAPACITY; i++) {
        CHECK(rb_put(&rb, (uint8_t)i));
    }
    CHECK(rb_full(&rb));
    CHECK_EQ(rb_count(&rb), RB_CAPACITY);
    CHECK(!rb_put(&rb, 0xFFu));           /* refuses, does not overwrite */
    CHECK_EQ(rb_count(&rb), RB_CAPACITY);
}

static void test_get_on_empty_fails_cleanly(void)
{
    ringbuf_t rb; rb_init(&rb);
    uint8_t out = 0x5Au;
    CHECK(!rb_get(&rb, &out));
    CHECK_EQ(out, 0x5A);                  /* output left untouched */
}

static void test_wraps_correctly(void)
{
    ringbuf_t rb; rb_init(&rb);
    uint8_t out = 0u;
    /* Drive it several times round the buffer. */
    for (uint32_t i = 0u; i < RB_CAPACITY * 5u; i++) {
        CHECK(rb_put(&rb, (uint8_t)(i & 0xFFu)));
        CHECK(rb_get(&rb, &out));
        CHECK_EQ(out, (uint8_t)(i & 0xFFu));
    }
    CHECK(rb_empty(&rb));
}

static void test_index_overflow_at_2_32(void)
{
    /* The one nobody tests, and the one that bites after three weeks of
     * uptime. Free-running indices WILL wrap past 2^32. Unsigned
     * subtraction is defined to wrap, so `head - tail` stays correct
     * across the boundary - as long as you never write `head > tail`. */
    ringbuf_t rb; rb_init(&rb);
    rb.head = 0xFFFFFFFEu;
    rb.tail = 0xFFFFFFFEu;
    CHECK(rb_empty(&rb));

    uint8_t out = 0u;
    for (uint32_t i = 0u; i < 8u; i++) {
        CHECK(rb_put(&rb, (uint8_t)(0x10u + i)));
    }
    CHECK_EQ(rb_count(&rb), 8);           /* head has wrapped past zero */
    CHECK(rb.head < rb.tail);             /* and is now numerically less */
    for (uint32_t i = 0u; i < 8u; i++) {
        CHECK(rb_get(&rb, &out));
        CHECK_EQ(out, 0x10u + i);         /* data still in order */
    }
    CHECK(rb_empty(&rb));
}

static void test_interleaved_partial_drain(void)
{
    ringbuf_t rb; rb_init(&rb);
    uint8_t out = 0u;
    uint32_t next_in = 0u, next_out = 0u;
    for (int round = 0; round < 1000; round++) {
        uint32_t burst = (uint32_t)(round % 7) + 1u;
        for (uint32_t i = 0u; i < burst; i++) {
            if (rb_put(&rb, (uint8_t)(next_in & 0xFFu))) { next_in++; }
        }
        uint32_t drain = (uint32_t)(round % 5) + 1u;
        for (uint32_t i = 0u; i < drain; i++) {
            if (rb_get(&rb, &out)) {
                CHECK_EQ(out, (uint8_t)(next_out & 0xFFu));
                next_out++;
            }
        }
        CHECK(rb_count(&rb) <= RB_CAPACITY);
    }
}

int main(void)
{
    printf("\nspsc ring buffer\n");
    RUN(test_starts_empty);
    RUN(test_put_get_roundtrip);
    RUN(test_fifo_order);
    RUN(test_fills_to_exactly_capacity);
    RUN(test_get_on_empty_fails_cleanly);
    RUN(test_wraps_correctly);
    RUN(test_index_overflow_at_2_32);
    RUN(test_interleaved_partial_drain);
    TEST_SUMMARY();
}
