#ifndef UART_H
#define UART_H
#include <stdint.h>

void uart_init(uint32_t baud);
void uart_putc(char c);
void uart_puts(const char *s);
void uart_put_u32(uint32_t v);
void uart_put_hex32(uint32_t v);
void uart_put_hex8(uint8_t v);
int  uart_getc_nonblocking(void);   /* -1 when the RX FIFO is empty */

#endif
