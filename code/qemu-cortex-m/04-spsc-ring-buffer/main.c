/* Example 04 - A lock-free SPSC ring buffer driven by a real interrupt.
 *
 * Runs on QEMU: `make run`
 *
 * The SysTick ISR is the producer; main() is the consumer. Both run
 * concurrently on one core with genuine preemption, so this exercises
 * the same hazards a UART RX ISR would - without needing a keyboard,
 * which is what makes it runnable in CI.
 *
 * The interactive UART path is still wired up: type while it runs.
 */
#include "lm3s6965.h"
#include "uart.h"
#include "semihost.h"
#include "ringbuf.h"

static ringbuf_t g_rx;
static volatile uint32_t g_produced;
static volatile uint32_t g_dropped;
static volatile uint32_t g_ticks;

/* Producer context. Note what an ISR must never do: no printf, no
 * malloc, no blocking, no unbounded loop. It moves a byte and leaves. */
void SysTick_Handler(void)
{
    g_ticks++;

    /* Synthesise a repeating pattern so the consumer can verify order. */
    uint8_t byte = (uint8_t)('A' + (g_produced % 26u));
    if (rb_put(&g_rx, byte)) {
        g_produced++;
    } else {
        g_dropped++;         /* overrun: count it, never block in an ISR */
    }
}

/* Real UART RX interrupt, for interactive use. Same producer discipline. */
void UART0_Handler(void)
{
    UART0_ICR = 0xFFFFu;
    while ((UART0_FR & UART_FR_RXFE) == 0u) {
        uint8_t b = (uint8_t)(UART0_DR & 0xFFu);
        if (!rb_put(&g_rx, b)) { g_dropped++; }
    }
}

int main(void)
{
    uart_init(115200u);
    rb_init(&g_rx);

    uart_puts("\n=== lock-free SPSC ring buffer ===\n\n");
    uart_puts("capacity = "); uart_put_u32(RB_CAPACITY);
    uart_puts(" bytes, indices are free-running and masked on access\n\n");

    UART0_IM   = UART_IM_RXIM | UART_IM_RTIM;
    NVIC_ISER0 = (1u << IRQ_UART0);

    SYSTICK_LOAD = (SYSTEM_CLOCK_HZ / 2000u) - 1u;   /* 2 kHz producer */
    SYSTICK_VAL  = 0u;
    SYSTICK_CTRL = SYSTICK_CTRL_CLKSOURCE | SYSTICK_CTRL_TICKINT
                 | SYSTICK_CTRL_ENABLE;

    uint32_t consumed = 0u;
    uint32_t errors   = 0u;
    uint8_t  expected = (uint8_t)'A';
    uint32_t high_water = 0u;

    while (consumed < 260u) {
        uint32_t depth = rb_count(&g_rx);
        if (depth > high_water) { high_water = depth; }

        uint8_t b;
        if (rb_get(&g_rx, &b)) {
            if (b != expected) { errors++; }
            expected = (uint8_t)('A' + ((consumed + 1u) % 26u));
            consumed++;
            if ((consumed % 26u) == 0u) {
                uart_puts("consumed "); uart_put_u32(consumed);
                uart_puts("  depth="); uart_put_u32(depth);
                uart_puts("  dropped="); uart_put_u32(g_dropped);
                uart_puts("\n");
            }
        } else {
            /* Nothing to do: sleep until the next interrupt rather than
             * spinning. On a battery target this is the difference
             * between microamps and milliamps. */
            wait_for_interrupt();
        }
    }

    /* --- phase 2: force an overrun ---------------------------------
     * A ring buffer that never fills teaches you nothing. Stall the
     * consumer and watch the producer discard. Dropping the NEWEST byte
     * (as rb_put does) keeps the oldest data; some designs overwrite the
     * OLDEST instead. Which is correct depends on whether stale data or
     * missing data hurts you more - decide it deliberately, per stream. */
    uart_puts("\n--- phase 2: stalling the consumer to force overrun ---\n");
    uint32_t drops_before = g_dropped;
    uint32_t t0 = g_ticks;
    while ((g_ticks - t0) < 200u) { }      /* consume nothing for 100 ms */

    uart_puts("after stalling: depth="); uart_put_u32(rb_count(&g_rx));
    uart_puts(" dropped="); uart_put_u32(g_dropped - drops_before);
    uart_puts("\n");

    SYSTICK_CTRL = 0u;

    uart_puts("\n--- result ---\n");
    uart_puts("produced   = "); uart_put_u32(g_produced);  uart_puts("\n");
    uart_puts("consumed   = "); uart_put_u32(consumed);    uart_puts("\n");
    uart_puts("dropped    = "); uart_put_u32(g_dropped);   uart_puts("\n");
    uart_puts("high water = "); uart_put_u32(high_water);
    uart_puts(" of "); uart_put_u32(RB_CAPACITY); uart_puts("\n");
    uart_puts("ordering errors = "); uart_put_u32(errors);
    uart_puts(errors == 0u ? "  (sequence intact)\n" : "  *** CORRUPTION ***\n");

    uart_puts("\nWhy no critical section was needed:\n");
    uart_puts("  * exactly one writer per index\n");
    uart_puts("  * 32-bit aligned loads/stores are atomic on Cortex-M\n");
    uart_puts("  * capacity is a power of two, so masking replaces modulo\n");
    uart_puts("Add a second producer and every line above stops being true.\n");

    semihost_exit(errors == 0u ? 0 : 1);
    return 0;
}
