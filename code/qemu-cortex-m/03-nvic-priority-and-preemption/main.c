/* Example 03 - NVIC priorities, preemption, and the two traps everyone
 * falls into.
 *
 * Runs on QEMU: `make run`
 *
 * Trap 1: lower number = higher priority. Priority 0 preempts priority 1.
 * Trap 2: only the TOP N bits of the 8-bit priority field are implemented.
 *         On this part that is 3 bits, so priorities 0x00 and 0x1F are the
 *         SAME priority. Writing 0,1,2,3 and expecting four distinct levels
 *         is a real and very common bug.
 */
#include "lm3s6965.h"
#include "uart.h"
#include "semihost.h"

static volatile uint32_t g_systick_count;
static volatile uint32_t g_low_prio_entered;
static volatile uint32_t g_preempted_at;
static volatile int      g_in_low_prio;

/* How many priority bits does this core actually implement? Write 0xFF
 * to a priority register and read back: the zeros at the bottom are the
 * bits that do not exist. Do this at runtime instead of trusting a
 * header - it takes four lines and it is always right. */
static uint32_t nvic_priority_bits(void)
{
    uint8_t saved = NVIC_IPR(IRQ_UART0);
    NVIC_IPR(IRQ_UART0) = 0xFFu;
    uint8_t readback = NVIC_IPR(IRQ_UART0);
    NVIC_IPR(IRQ_UART0) = saved;

    uint32_t bits = 0u;
    while (readback & 0x80u) { bits++; readback = (uint8_t)(readback << 1); }
    return bits;
}

/* SysTick: HIGH priority (0). Preempts the UART handler below. */
void SysTick_Handler(void)
{
    g_systick_count++;
    if (g_in_low_prio && g_preempted_at == 0u) {
        g_preempted_at = g_systick_count;
    }
}

/* UART0: LOW priority (0xE0 = level 7 of 8 on a 3-bit field).
 * Deliberately slow, so SysTick has time to preempt it. */
void UART0_Handler(void)
{
    UART0_ICR = 0xFFFFu;                 /* clear the source FIRST, or you
                                          * re-enter the moment you return */
    g_low_prio_entered++;
    g_in_low_prio = 1;

    uint32_t before = g_systick_count;
    /* Busy work long enough to span several SysTick periods. */
    for (volatile uint32_t i = 0u; i < 3000000u; i++) { }
    uint32_t after = g_systick_count;

    g_in_low_prio = 0;

    uart_puts("  [UART0 ISR] ran while SysTick advanced ");
    uart_put_u32(after - before);
    uart_puts(" ticks -> it preempted me\n");
}

int main(void)
{
    uart_init(115200u);
    uart_puts("\n=== NVIC priority and preemption ===\n\n");

    uint32_t bits = nvic_priority_bits();
    uart_puts("Implemented priority bits on this core: ");
    uart_put_u32(bits);
    uart_puts("  -> ");
    uart_put_u32(1u << bits);
    uart_puts(" distinct levels\n");
    if (bits < 8u) {
        uart_puts("So priority values 0x00 and ");
        uart_put_hex32((1u << (8u - bits)) - 1u);
        uart_puts(" collapse to the SAME level.\n");
    }
    uart_puts("NOTE: real LM3S6965 silicon implements 3 bits (8 levels).\n"
              "QEMU 8.x reports 8 bits here. Treat the count as a runtime\n"
              "probe you should re-run on the actual part, not as a fact\n"
              "you can carry between an emulator and hardware.\n\n");

    /* SysTick priority lives in SCB_SHPR3 bits [31:24], not in NVIC_IPR. */
    SCB_SHPR3 = (SCB_SHPR3 & 0x00FFFFFFu) | (0x00u << 24);  /* highest */
    NVIC_IPR(IRQ_UART0) = 0xE0u;                            /* lowest  */

    SYSTICK_LOAD = (SYSTEM_CLOCK_HZ / 1000u) - 1u;
    SYSTICK_VAL  = 0u;
    SYSTICK_CTRL = SYSTICK_CTRL_CLKSOURCE | SYSTICK_CTRL_TICKINT
                 | SYSTICK_CTRL_ENABLE;

    UART0_IM  = UART_IM_RXIM | UART_IM_RTIM;
    NVIC_ISER0 = (1u << IRQ_UART0);

    uart_puts("SysTick priority 0 (high), UART0 priority 0xE0 (low).\n");
    uart_puts("Type a character to fire the low-priority UART ISR;\n");
    uart_puts("SysTick will preempt it mid-flight.\n\n");

    /* Self-trigger so the example is deterministic in CI: pend the UART
     * IRQ by hand rather than waiting for a keystroke. */
    REG32(0xE000E200u) = (1u << IRQ_UART0);   /* NVIC_ISPR0 */

    while (g_low_prio_entered == 0u) { wait_for_interrupt(); }
    while (g_in_low_prio) { }

    uart_puts("\nResult: the low-priority handler was entered ");
    uart_put_u32(g_low_prio_entered);
    uart_puts(" time(s)\n");
    if (g_preempted_at != 0u) {
        uart_puts("and SysTick preempted it at tick ");
        uart_put_u32(g_preempted_at);
        uart_puts(" - nested exceptions confirmed.\n");
    }

    uart_puts("\nThings to take away:\n");
    uart_puts("  * clear the peripheral flag at the TOP of the ISR\n");
    uart_puts("  * core exceptions use SHPR, external IRQs use NVIC_IPR\n");
    uart_puts("  * always measure the implemented priority bits\n");

    semihost_exit(0);
    return 0;
}
