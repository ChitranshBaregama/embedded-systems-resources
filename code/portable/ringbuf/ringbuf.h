/* Single-producer / single-consumer byte ring buffer.
 *
 * The whole point: with exactly one writer of `head` and one writer of
 * `tail`, and a power-of-two capacity, you need NO critical section on
 * a Cortex-M. Aligned 32-bit loads and stores are already atomic, so
 * the ISR and main thread can never tear each other's index.
 *
 * Break either assumption - two producers, or a non-power-of-two size
 * with a modulo - and you need locking again. This is the distinction
 * interviewers are probing when they ask "do you need a mutex here?".
 */
#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define RB_CAPACITY 64u                     /* MUST be a power of two */
#if (RB_CAPACITY & (RB_CAPACITY - 1u)) != 0u
#  error "RB_CAPACITY must be a power of two"
#endif

typedef struct {
    uint8_t  buf[RB_CAPACITY];
    volatile uint32_t head;   /* written by producer only */
    volatile uint32_t tail;   /* written by consumer only */
} ringbuf_t;

static inline void rb_init(ringbuf_t *rb)
{
    rb->head = 0u;
    rb->tail = 0u;
}

/* Free-running indices, masked only on access. This is what lets
 * `head - tail` be the count with no ambiguity between "full" and
 * "empty" - the classic bug when you mask the indices themselves and
 * lose a slot to tell the two apart. */
static inline uint32_t rb_count(const ringbuf_t *rb)
{
    return rb->head - rb->tail;             /* wraps correctly at 2^32 */
}

static inline bool rb_empty(const ringbuf_t *rb) { return rb_count(rb) == 0u; }
static inline bool rb_full(const ringbuf_t *rb)  { return rb_count(rb) >= RB_CAPACITY; }

/* PRODUCER side. Call from exactly one context (typically the ISR). */
static inline bool rb_put(ringbuf_t *rb, uint8_t b)
{
    uint32_t head = rb->head;
    if ((head - rb->tail) >= RB_CAPACITY) {
        return false;                        /* full: drop, never block */
    }
    rb->buf[head & (RB_CAPACITY - 1u)] = b;

    /* Publish the data BEFORE the index. On Cortex-M0/M3/M4 a compiler
     * barrier is enough for ISR-vs-thread; on a multicore or
     * write-buffered system you would need a real DMB here. */
    __atomic_signal_fence(__ATOMIC_RELEASE);
    rb->head = head + 1u;
    return true;
}

/* CONSUMER side. Call from exactly one context (typically main). */
static inline bool rb_get(ringbuf_t *rb, uint8_t *out)
{
    uint32_t tail = rb->tail;
    if (rb->head == tail) {
        return false;                        /* empty */
    }
    *out = rb->buf[tail & (RB_CAPACITY - 1u)];
    __atomic_signal_fence(__ATOMIC_ACQUIRE);
    rb->tail = tail + 1u;
    return true;
}

#endif /* RINGBUF_H */
