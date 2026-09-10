/* Example 02 - SysTick as a timebase, and why the tick counter is volatile.
 *
 * Runs on QEMU: `make run`
 *
 * SysTick is the one timer that exists on every Cortex-M, at the same
 * address, independent of vendor. If you can set it up from the ARM
 * ARM alone you can bring up a timebase on any Cortex-M part.
 */
#include "lm3s6965.h"
#include "uart.h"
#include "semihost.h"

/* Written by the ISR, read by main. Without `volatile` the compiler is
 * entitled to hoist the read out of the wait loop below and spin forever
 * on a stale register copy - at -O2 it will actually do it. */
static volatile uint32_t g_ticks;

void SysTick_Handler(void)
{
    g_ticks++;
}

/* Reload is 24-bit. period = (RELOAD + 1) / clk, so RELOAD = clk/hz - 1.
 * At 12 MHz the slowest tick you can get is ~1.4 Hz - a real constraint
 * people hit and misdiagnose as "SysTick is broken". */
static void systick_init(uint32_t hz)
{
    uint32_t reload = (SYSTEM_CLOCK_HZ / hz) - 1u;
    if (reload > 0x00FFFFFFu) {
        uart_puts("reload exceeds 24 bits - pick a faster tick\n");
        semihost_exit(1);
    }
    SYSTICK_LOAD = reload;
    SYSTICK_VAL  = 0u;                    /* writing any value clears it */
    SYSTICK_CTRL = SYSTICK_CTRL_CLKSOURCE /* core clock, not /8          */
                 | SYSTICK_CTRL_TICKINT   /* raise the exception         */
                 | SYSTICK_CTRL_ENABLE;
}

/* Overflow-safe delay. Note the subtraction: `now - start >= n` keeps
 * working across the 2^32 wrap, whereas `now >= start + n` does not. */
static void delay_ticks(uint32_t n)
{
    uint32_t start = g_ticks;
    while ((g_ticks - start) < n) {
        wait_for_interrupt();   /* sleep instead of burning the core */
    }
}

int main(void)
{
    uart_init(115200u);
    uart_puts("\n=== SysTick timebase ===\n");

    systick_init(1000u);                  /* 1 kHz -> 1 ms per tick */
    uart_puts("RELOAD = ");
    uart_put_u32(SYSTICK_LOAD);
    uart_puts("  (");
    uart_put_u32(SYSTEM_CLOCK_HZ / 1000u);
    uart_puts(" core cycles per tick)\n\n");

    for (uint32_t i = 1u; i <= 5u; i++) {
        delay_ticks(100u);                /* 100 ms */
        uart_puts("t = ");
        uart_put_u32(g_ticks);
        uart_puts(" ms   iteration ");
        uart_put_u32(i);
        uart_puts("\n");
    }

    /* COUNTFLAG (bit 16) is read-to-clear on real silicon: the first read
     * after a wrap returns 1, the next returns 0. QEMU 8.2 does not model
     * that clear-on-read, so both reads below come back identical.
     *
     * Keep this in the example on purpose. Emulators are approximations,
     * and knowing which corners yours gets wrong is part of the skill.
     * Anything involving read-to-clear, side-effect-on-read, or analogue
     * timing needs real hardware to verify. */
    uint32_t c1 = SYSTICK_CTRL;
    uint32_t c2 = SYSTICK_CTRL;
    uart_puts("\nCOUNTFLAG (bit 16) read twice: ");
    uart_put_hex32(c1); uart_puts(" then "); uart_put_hex32(c2);
    uart_puts("\n");
    if ((c1 & (1u << 16)) && (c2 & (1u << 16))) {
        uart_puts("Both reads show COUNTFLAG set -> this host does NOT model\n"
                  "read-to-clear. On real silicon the second read would be 0.\n");
    } else if (c1 & (1u << 16)) {
        uart_puts("Cleared on read, as the hardware spec requires.\n");
    }
    uart_puts("Either way the rule stands: never poll COUNTFLAG from two\n"
              "places, because the first reader consumes the flag.\n");

    semihost_exit(0);
    return 0;
}
