/* Blocking UART0 output. Enough to make an embedded target printf-able,
 * with no newlib, no syscalls, and no heap. */
#include "lm3s6965.h"
#include "uart.h"

void uart_init(uint32_t baud)
{
    SYSCTL_RCGC1 |= RCGC1_UART0;
    SYSCTL_RCGC2 |= RCGC2_GPIOA;

    UART0_CTL = 0u;                       /* disable while reconfiguring */

    /* Baud divisor is fixed-point: 16.6 format.
     * divisor = clk / (16 * baud); IBRD = int, FBRD = frac * 64 + 0.5 */
    uint32_t div64 = (SYSTEM_CLOCK_HZ * 4u) / baud;  /* = 64*clk/(16*baud) */
    UART0_IBRD = div64 / 64u;
    UART0_FBRD = div64 % 64u;

    UART0_LCRH = UART_LCRH_WLEN8 | UART_LCRH_FEN;   /* 8N1, FIFOs on     */
    UART0_CTL  = UART_CTL_UARTEN | UART_CTL_TXE | UART_CTL_RXE;
}

void uart_putc(char c)
{
    while (UART0_FR & UART_FR_TXFF) { }   /* volatile read: never hoisted */
    UART0_DR = (uint32_t)c;
}

void uart_puts(const char *s)
{
    while (*s) {
        if (*s == '\n') {
            uart_putc('\r');
        }
        uart_putc(*s++);
    }
}

int uart_getc_nonblocking(void)
{
    if (UART0_FR & UART_FR_RXFE) {
        return -1;
    }
    return (int)(UART0_DR & 0xFFu);
}

void uart_put_u32(uint32_t v)
{
    char buf[11];
    int  i = 10;
    buf[10] = '\0';
    if (v == 0u) { uart_putc('0'); return; }
    while (v && i > 0) {
        buf[--i] = (char)('0' + (v % 10u));
        v /= 10u;
    }
    uart_puts(&buf[i]);
}

void uart_put_hex32(uint32_t v)
{
    static const char digits[] = "0123456789ABCDEF";
    uart_puts("0x");
    for (int shift = 28; shift >= 0; shift -= 4) {
        uart_putc(digits[(v >> shift) & 0xFu]);
    }
}

void uart_put_hex8(uint8_t v)
{
    static const char digits[] = "0123456789ABCDEF";
    uart_putc(digits[(v >> 4) & 0xFu]);
    uart_putc(digits[v & 0xFu]);
}
