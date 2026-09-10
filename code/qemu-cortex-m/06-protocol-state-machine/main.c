/* Example 06 - The same parser, on the target.
 *
 * Runs on QEMU: `make run`
 *
 * The file being compiled here is byte-for-byte the file the host tests
 * in code/host-tests/ exercise: code/portable/frame/frame.c. Nothing is
 * reimplemented for the target.
 *
 * That is the whole architectural point. Split firmware into
 *
 *     portable logic   - parsers, CRCs, state machines, scaling, packing
 *     hardware layer   - the registers, and only the registers
 *
 * and the first half becomes testable on a build machine, at speed,
 * under sanitisers, with no board on your desk. The second half stays
 * small enough to review by eye.
 *
 * Type framed bytes into the terminal and it parses those too.
 */
#include "lm3s6965.h"
#include "uart.h"
#include "semihost.h"
#include "frame.h"

static frame_parser_t g_parser;

static void print_payload(const frame_parser_t *p)
{
    uart_puts("  payload [");
    uart_put_u32(p->length);
    uart_puts("] =");
    for (uint8_t i = 0u; i < p->length; i++) {
        uart_puts(" ");
        uart_put_hex8(p->payload[i]);
    }
    uart_puts("\n");
}

static void handle(uint8_t byte)
{
    frame_event_t ev = frame_feed(&g_parser, byte);
    switch (ev) {
    case FRAME_EV_NONE:
        break;
    case FRAME_EV_COMPLETE:
        uart_puts("FRAME OK\n");
        print_payload(&g_parser);
        break;
    case FRAME_EV_ERR_LENGTH:
        uart_puts("REJECTED: length field exceeds buffer "
                  "(this is the overflow that did not happen)\n");
        break;
    case FRAME_EV_ERR_CRC:
        uart_puts("REJECTED: CRC mismatch\n");
        break;
    case FRAME_EV_ERR_FRAMING:
        uart_puts("REJECTED: missing ETX\n");
        break;
    default:
        break;
    }
}

/* RX interrupt: drain the FIFO, run the state machine, leave. Bounded
 * work, no printf inside the ISR on a latency-sensitive design - the
 * uart_puts calls above are here for legibility and would move to a
 * queue in production. */
void UART0_Handler(void)
{
    UART0_ICR = 0xFFFFu;
    while ((UART0_FR & UART_FR_RXFE) == 0u) {
        handle((uint8_t)(UART0_DR & 0xFFu));
    }
}

/* --- built-in self test, so CI has something deterministic to check -- */
static uint16_t crc_of(const uint8_t *payload, uint8_t len)
{
    uint16_t crc = crc16_ccitt_update(0xFFFFu, len);
    for (uint8_t i = 0u; i < len; i++) {
        crc = crc16_ccitt_update(crc, payload[i]);
    }
    return crc;
}

static void inject_frame(const uint8_t *payload, uint8_t len, int corrupt)
{
    uint16_t crc = crc_of(payload, len);
    handle(FRAME_STX);
    handle(len);
    for (uint8_t i = 0u; i < len; i++) {
        handle((uint8_t)(payload[i] ^ ((corrupt && i == 0u) ? 0x01u : 0x00u)));
    }
    handle((uint8_t)(crc & 0xFFu));
    handle((uint8_t)(crc >> 8));
    handle(FRAME_ETX);
}

int main(void)
{
    uart_init(115200u);
    frame_reset(&g_parser);

    uart_puts("\n=== framed protocol state machine ===\n");
    uart_puts("wire format: STX | LEN | PAYLOAD | CRC16_LO | CRC16_HI | ETX\n\n");

    const uint8_t good[] = { 0xDE, 0xAD, 0xBE, 0xEF };

    uart_puts("1. a well-formed frame\n");
    inject_frame(good, 4u, 0);

    uart_puts("\n2. same frame, one bit flipped in the payload\n");
    inject_frame(good, 4u, 1);

    uart_puts("\n3. a frame claiming a payload longer than the buffer\n");
    handle(FRAME_STX);
    handle((uint8_t)(FRAME_MAX_PAYLOAD + 1u));

    uart_puts("\n4. line noise, then a good frame (resynchronisation)\n");
    for (int i = 0; i < 5; i++) { handle(0x55u); }
    inject_frame(good, 4u, 0);

    uart_puts("\n--- counters ---\n");
    uart_puts("frames_ok       = "); uart_put_u32(g_parser.frames_ok);       uart_puts("\n");
    uart_puts("err_crc         = "); uart_put_u32(g_parser.err_crc);         uart_puts("\n");
    uart_puts("err_length      = "); uart_put_u32(g_parser.err_length);      uart_puts("\n");
    uart_puts("err_framing     = "); uart_put_u32(g_parser.err_framing);     uart_puts("\n");
    uart_puts("bytes_discarded = "); uart_put_u32(g_parser.bytes_discarded); uart_puts("\n");

    /* bytes_discarded is 6, not 5, and the extra one is instructive:
     * after the CRC mismatch in test 2 the parser drops straight back to
     * IDLE, so the ETX that followed is discarded as line noise. The
     * first draft of this self-test asserted 5 and failed - which is
     * exactly the sort of off-by-one that a real counter would have
     * hidden if nobody had checked the arithmetic against the state
     * diagram. */
    int ok = (g_parser.frames_ok == 2u)
          && (g_parser.err_crc == 1u)
          && (g_parser.err_length == 1u)
          && (g_parser.err_framing == 0u)
          && (g_parser.bytes_discarded == 6u);

    uart_puts(ok ? "\nself-test PASSED\n" : "\nself-test FAILED\n");

    uart_puts("\nNow interactive - send framed bytes over the UART.\n");
    UART0_IM   = UART_IM_RXIM | UART_IM_RTIM;
    NVIC_ISER0 = (1u << IRQ_UART0);

    semihost_exit(ok ? 0 : 1);
    return 0;
}
