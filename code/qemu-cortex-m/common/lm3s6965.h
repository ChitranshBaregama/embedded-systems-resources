/* Minimal register map for the TI Stellaris LM3S6965 (Cortex-M3).
 *
 * Deliberately hand-written rather than vendor-generated: the point of
 * these examples is that you can see every address you touch.
 *
 * Pattern to note: every register is `volatile uint32_t *` at a fixed
 * address. `volatile` is what stops the compiler caching a status read
 * outside a polling loop - the single most common bare-metal C bug.
 */
#ifndef LM3S6965_H
#define LM3S6965_H

#include <stdint.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

/* ---- System control ------------------------------------------------ */
#define SYSCTL_RCGC1    REG32(0x400FE104u)  /* UART/timer clock gating   */
#define SYSCTL_RCGC2    REG32(0x400FE108u)  /* GPIO clock gating         */
#define RCGC1_UART0     (1u << 0)
#define RCGC2_GPIOA     (1u << 0)
#define RCGC2_GPIOF     (1u << 5)

/* ---- UART0 (PL011-compatible) -------------------------------------- */
#define UART0_BASE      0x4000C000u
#define UART0_DR        REG32(UART0_BASE + 0x000u)
#define UART0_FR        REG32(UART0_BASE + 0x018u)
#define UART0_IBRD      REG32(UART0_BASE + 0x024u)
#define UART0_FBRD      REG32(UART0_BASE + 0x028u)
#define UART0_LCRH      REG32(UART0_BASE + 0x02Cu)
#define UART0_CTL       REG32(UART0_BASE + 0x030u)
#define UART0_IM        REG32(UART0_BASE + 0x038u)
#define UART0_MIS       REG32(UART0_BASE + 0x040u)
#define UART0_ICR       REG32(UART0_BASE + 0x044u)

#define UART_FR_RXFE    (1u << 4)   /* receive FIFO empty                */
#define UART_FR_TXFF    (1u << 5)   /* transmit FIFO full                */
#define UART_FR_BUSY    (1u << 3)
#define UART_LCRH_WLEN8 (3u << 5)
#define UART_LCRH_FEN   (1u << 4)
#define UART_CTL_UARTEN (1u << 0)
#define UART_CTL_TXE    (1u << 8)
#define UART_CTL_RXE    (1u << 9)
#define UART_IM_RXIM    (1u << 4)
#define UART_IM_RTIM    (1u << 6)

/* ---- GPIO port F (LED on the real EVB) ----------------------------- */
#define GPIOF_BASE      0x40025000u
#define GPIOF_DATA_BITS(mask) REG32(GPIOF_BASE + ((mask) << 2))
#define GPIOF_DIR       REG32(GPIOF_BASE + 0x400u)
#define GPIOF_DEN       REG32(GPIOF_BASE + 0x51Cu)

/* ---- Cortex-M core peripherals (same on every Cortex-M) ------------ */
#define SYSTICK_CTRL    REG32(0xE000E010u)
#define SYSTICK_LOAD    REG32(0xE000E014u)
#define SYSTICK_VAL     REG32(0xE000E018u)
#define SYSTICK_CTRL_ENABLE    (1u << 0)
#define SYSTICK_CTRL_TICKINT   (1u << 1)
#define SYSTICK_CTRL_CLKSOURCE (1u << 2)

#define NVIC_ISER0      REG32(0xE000E100u)
#define NVIC_ICER0      REG32(0xE000E180u)
#define NVIC_IPR(n)     (*(volatile uint8_t *)(0xE000E400u + (n)))
#define SCB_AIRCR       REG32(0xE000ED0Cu)
#define SCB_SHPR3       REG32(0xE000ED20u)  /* PendSV / SysTick priority */

#define IRQ_UART0       5u

/* QEMU's lm3s6965evb runs the core at 12 MHz by default. */
#define SYSTEM_CLOCK_HZ 12000000u

/* ---- Critical sections --------------------------------------------- */
static inline uint32_t irq_save(void)
{
    uint32_t primask;
    __asm__ volatile ("mrs %0, primask" : "=r"(primask));
    __asm__ volatile ("cpsid i" ::: "memory");
    return primask;
}

static inline void irq_restore(uint32_t primask)
{
    if ((primask & 1u) == 0u) {
        __asm__ volatile ("cpsie i" ::: "memory");
    }
}

static inline void wait_for_interrupt(void)
{
    __asm__ volatile ("wfi" ::: "memory");
}

#endif /* LM3S6965_H */
