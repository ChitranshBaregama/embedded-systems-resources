/* A byte-at-a-time framed-protocol receiver.
 *
 * Shape:  STX | LEN | PAYLOAD[LEN] | CRC16_LO | CRC16_HI | ETX
 *
 * Three properties that make this the pattern to reach for, and the
 * reason it is written as a pure function of (state, byte):
 *
 *   1. It never blocks and never allocates, so it is safe to call from
 *      an ISR, from a poll loop, or from a host unit test.
 *   2. It cannot overrun: LEN is bounds-checked before a single payload
 *      byte is stored.
 *   3. It has no dependency on the transport, so the identical code runs
 *      on the target over UART and on a build machine against a byte
 *      array. That is what makes protocol code testable at all.
 *
 * The same skeleton covers DLMS HDLC framing, Modbus RTU, SLIP, and
 * every in-house serial protocol you will ever be handed.
 */
#ifndef FRAME_H
#define FRAME_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define FRAME_STX          0x02u
#define FRAME_ETX          0x03u
#define FRAME_MAX_PAYLOAD  64u

typedef enum {
    FRAME_ST_IDLE = 0,     /* hunting for STX                  */
    FRAME_ST_LEN,          /* next byte is the length          */
    FRAME_ST_PAYLOAD,      /* collecting LEN bytes             */
    FRAME_ST_CRC_LO,
    FRAME_ST_CRC_HI,
    FRAME_ST_ETX,
    FRAME_ST_COUNT
} frame_state_t;

typedef enum {
    FRAME_EV_NONE = 0,     /* byte consumed, nothing to report */
    FRAME_EV_COMPLETE,     /* a valid frame is in `payload`    */
    FRAME_EV_ERR_LENGTH,   /* LEN exceeded FRAME_MAX_PAYLOAD   */
    FRAME_EV_ERR_CRC,      /* CRC mismatch                     */
    FRAME_EV_ERR_FRAMING   /* expected ETX, got something else */
} frame_event_t;

typedef struct {
    frame_state_t state;
    uint8_t       payload[FRAME_MAX_PAYLOAD];
    uint8_t       length;
    uint8_t       index;
    uint16_t      crc_received;
    uint16_t      crc_computed;

    /* Diagnostics. Counters like these cost four bytes each and save
     * hours: without them "the link is flaky" is unfalsifiable. */
    uint32_t      frames_ok;
    uint32_t      err_length;
    uint32_t      err_crc;
    uint32_t      err_framing;
    uint32_t      bytes_discarded;
} frame_parser_t;

void          frame_reset(frame_parser_t *p);
frame_event_t frame_feed(frame_parser_t *p, uint8_t byte);
uint16_t      crc16_ccitt(const uint8_t *data, size_t len);
uint16_t      crc16_ccitt_update(uint16_t crc, uint8_t byte);
const char   *frame_state_name(frame_state_t s);
const char   *frame_event_name(frame_event_t e);

#endif /* FRAME_H */
