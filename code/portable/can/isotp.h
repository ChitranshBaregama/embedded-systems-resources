/* ISO 15765-2 (ISO-TP) transport layer for CAN.
 *
 * This is the layer that carries UDS diagnostics (ISO 14229), and it is
 * the thing that lets an 8-byte CAN frame deliver a 4095-byte message.
 * If you are moving into automotive firmware, this state machine is worth
 * knowing cold - it sits under every diagnostic session, every flash
 * reprogramming routine and every DTC read.
 *
 * Four PDU types, identified by the high nibble of the first data byte
 * (the "PCI"):
 *
 *   SF  0x0L            single frame, L = 1..7 bytes of payload
 *   FF  0x1L LL         first frame, 12-bit length 8..4095
 *   CF  0x2S            consecutive frame, S = sequence number, wraps 0..15
 *   FC  0x3F BS STmin   flow control from the receiver
 *
 * The flow-control handshake is the part people get wrong:
 *
 *   sender                        receiver
 *     |------------- FF --------->|
 *     |<------------ FC ----------|   FS=CTS, BS=n, STmin=t
 *     |------ CF 1 .. CF n ------>|
 *     |<------------ FC ----------|   another FC after every BS frames
 *     |------ CF n+1 .. ---------->|
 *
 * BS = 0 means "send everything, do not wait for another FC".
 * STmin is the receiver telling the sender how slowly to go, because the
 * receiver's ISR cannot keep up with back-to-back frames at 500 kbit/s.
 * Ignoring STmin is the classic cause of a flash routine that works on
 * the bench and drops frames in the vehicle.
 *
 * Everything here is a pure function of (state, frame). No CAN driver, no
 * timers, no allocation - so it runs unmodified under a host test suite,
 * which is the only practical way to cover the malformed-input cases that
 * matter.
 */
#ifndef ISOTP_H
#define ISOTP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define ISOTP_MAX_PAYLOAD     4095u   /* 12-bit length field, classical CAN */
#define ISOTP_CAN_DLC         8u

/* PCI type codes, high nibble of byte 0 */
#define ISOTP_PCI_SF          0x0u
#define ISOTP_PCI_FF          0x1u
#define ISOTP_PCI_CF          0x2u
#define ISOTP_PCI_FC          0x3u

/* Flow status, low nibble of an FC frame */
#define ISOTP_FS_CTS          0x0u   /* continue to send      */
#define ISOTP_FS_WAIT         0x1u   /* wait for another FC   */
#define ISOTP_FS_OVFLW        0x2u   /* buffer too small; abort */

typedef enum {
    ISOTP_RX_IDLE = 0,
    ISOTP_RX_WAIT_CF,          /* FF seen, collecting consecutive frames */
    ISOTP_RX_COUNT
} isotp_rx_state_t;

typedef enum {
    ISOTP_EV_NONE = 0,
    ISOTP_EV_MESSAGE,          /* a complete message is in the buffer     */
    ISOTP_EV_SEND_FC,          /* caller must transmit fc_frame           */
    ISOTP_EV_ERR_SEQUENCE,     /* consecutive frame out of order          */
    ISOTP_EV_ERR_OVERFLOW,     /* declared length exceeds our buffer      */
    ISOTP_EV_ERR_LENGTH,       /* malformed PCI / bad DLC                 */
    ISOTP_EV_ERR_UNEXPECTED    /* a CF with no FF, etc.                   */
} isotp_event_t;

typedef struct {
    isotp_rx_state_t state;

    uint8_t  buf[ISOTP_MAX_PAYLOAD];
    uint16_t total_len;        /* declared by the FF                      */
    uint16_t received;         /* how much is in buf                      */
    uint8_t  next_sn;          /* expected sequence number, 0..15          */
    uint16_t cf_since_fc;      /* for block-size accounting                */

    /* Receiver policy, set by the caller before use. */
    uint16_t buffer_limit;     /* refuse messages longer than this         */
    uint8_t  block_size;       /* BS we advertise; 0 = no more FCs needed   */
    uint8_t  st_min;           /* STmin we advertise                        */

    /* Populated when the event is ISOTP_EV_SEND_FC. */
    uint8_t  fc_frame[ISOTP_CAN_DLC];

    /* Diagnostics. Cheap, and the difference between "the bus is flaky"
     * and a number you can act on. */
    uint32_t messages_ok;
    uint32_t err_sequence;
    uint32_t err_overflow;
    uint32_t err_length;
    uint32_t err_unexpected;
} isotp_rx_t;

void          isotp_rx_init(isotp_rx_t *rx, uint16_t buffer_limit,
                            uint8_t block_size, uint8_t st_min);
isotp_event_t isotp_rx_frame(isotp_rx_t *rx, const uint8_t *data, uint8_t dlc);

/* ---- transmit side ------------------------------------------------- */

typedef enum {
    ISOTP_TX_IDLE = 0,
    ISOTP_TX_WAIT_FC,          /* FF sent, waiting for the receiver's FC  */
    ISOTP_TX_SENDING,          /* cleared to send consecutive frames      */
    ISOTP_TX_DONE,
    ISOTP_TX_ABORTED
} isotp_tx_state_t;

typedef struct {
    isotp_tx_state_t state;
    const uint8_t   *payload;
    uint16_t         total_len;
    uint16_t         sent;
    uint8_t          next_sn;
    uint8_t          block_size;      /* from the receiver's FC           */
    uint8_t          st_min;          /* from the receiver's FC           */
    uint16_t         cf_in_block;
    uint32_t         wait_count;      /* how many FS=WAIT we have taken   */
} isotp_tx_t;

/* Begin sending. Returns false if the payload is too long to segment. */
bool isotp_tx_begin(isotp_tx_t *tx, const uint8_t *payload, uint16_t len);

/* Produce the next frame to put on the bus.
 * Returns the DLC to send, or 0 when there is nothing to send right now
 * (either finished, or blocked waiting for a flow-control frame). */
uint8_t isotp_tx_next(isotp_tx_t *tx, uint8_t out[ISOTP_CAN_DLC]);

/* Feed a received flow-control frame back into the sender. */
isotp_event_t isotp_tx_flow_control(isotp_tx_t *tx, const uint8_t *data, uint8_t dlc);

const char *isotp_event_name(isotp_event_t e);

#endif
