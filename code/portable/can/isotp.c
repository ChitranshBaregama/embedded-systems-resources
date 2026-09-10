#include "isotp.h"
#include <string.h>

/* ------------------------------------------------------------------ */
/* Receive                                                             */
/* ------------------------------------------------------------------ */

void isotp_rx_init(isotp_rx_t *rx, uint16_t buffer_limit,
                   uint8_t block_size, uint8_t st_min)
{
    memset(rx, 0, sizeof *rx);
    rx->state        = ISOTP_RX_IDLE;
    rx->buffer_limit = (buffer_limit > ISOTP_MAX_PAYLOAD)
                     ? ISOTP_MAX_PAYLOAD : buffer_limit;
    rx->block_size   = block_size;
    rx->st_min       = st_min;
}

static void build_fc(isotp_rx_t *rx, uint8_t flow_status)
{
    rx->fc_frame[0] = (uint8_t)((ISOTP_PCI_FC << 4) | (flow_status & 0x0Fu));
    rx->fc_frame[1] = rx->block_size;
    rx->fc_frame[2] = rx->st_min;
    /* Padding. Some ECUs reject a short FC frame outright, and 0xCC /
     * 0xAA / 0x00 are all seen in the wild. Fixed 8-byte frames with
     * 0xCC padding is the safest default for classical CAN. */
    for (unsigned i = 3u; i < ISOTP_CAN_DLC; i++) {
        rx->fc_frame[i] = 0xCCu;
    }
}

static void rx_abort(isotp_rx_t *rx)
{
    rx->state     = ISOTP_RX_IDLE;
    rx->total_len = 0u;
    rx->received  = 0u;
    rx->next_sn   = 0u;
    rx->cf_since_fc = 0u;
}

isotp_event_t isotp_rx_frame(isotp_rx_t *rx, const uint8_t *data, uint8_t dlc)
{
    if (data == 0 || dlc == 0u || dlc > ISOTP_CAN_DLC) {
        rx->err_length++;
        return ISOTP_EV_ERR_LENGTH;
    }

    uint8_t pci = (uint8_t)(data[0] >> 4);

    switch (pci) {

    /* ---- single frame ------------------------------------------- */
    case ISOTP_PCI_SF: {
        uint8_t len = data[0] & 0x0Fu;
        /* A zero-length SF is malformed, and a length that does not fit
         * in the frame we actually received is the classic truncation
         * attack: trust the DLC, not the claimed length. */
        if (len == 0u || len > 7u || (len + 1u) > dlc) {
            rx->err_length++;
            rx_abort(rx);
            return ISOTP_EV_ERR_LENGTH;
        }
        if (len > rx->buffer_limit) {
            rx->err_overflow++;
            rx_abort(rx);
            return ISOTP_EV_ERR_OVERFLOW;
        }
        memcpy(rx->buf, &data[1], len);
        rx->total_len = len;
        rx->received  = len;
        rx->state     = ISOTP_RX_IDLE;
        rx->messages_ok++;
        return ISOTP_EV_MESSAGE;
    }

    /* ---- first frame -------------------------------------------- */
    case ISOTP_PCI_FF: {
        if (dlc < ISOTP_CAN_DLC) {
            /* A FF must always be a full 8-byte frame. Anything shorter
             * cannot carry the 6 payload bytes it implies. */
            rx->err_length++;
            rx_abort(rx);
            return ISOTP_EV_ERR_LENGTH;
        }
        uint16_t len = (uint16_t)((((uint16_t)(data[0] & 0x0Fu)) << 8) | data[1]);
        if (len < 8u) {
            /* Anything under 8 bytes must have been a single frame.
             * A short FF is malformed and some stacks mis-handle it. */
            rx->err_length++;
            rx_abort(rx);
            return ISOTP_EV_ERR_LENGTH;
        }
        if (len > rx->buffer_limit) {
            /* Say so on the bus rather than silently truncating. This is
             * exactly what FS=OVFLW exists for, and it is the difference
             * between a clean diagnostic rejection and a buffer overrun. */
            rx->err_overflow++;
            build_fc(rx, ISOTP_FS_OVFLW);
            rx_abort(rx);
            return ISOTP_EV_ERR_OVERFLOW;
        }

        memcpy(rx->buf, &data[2], 6u);
        rx->total_len   = len;
        rx->received    = 6u;
        rx->next_sn     = 1u;             /* first CF is always SN = 1 */
        rx->cf_since_fc = 0u;
        rx->state       = ISOTP_RX_WAIT_CF;
        build_fc(rx, ISOTP_FS_CTS);
        return ISOTP_EV_SEND_FC;
    }

    /* ---- consecutive frame -------------------------------------- */
    case ISOTP_PCI_CF: {
        if (rx->state != ISOTP_RX_WAIT_CF) {
            /* A CF with no FF. Common after a reset, or when two
             * senders share an address. Drop it; do not guess. */
            rx->err_unexpected++;
            return ISOTP_EV_ERR_UNEXPECTED;
        }
        uint8_t sn = data[0] & 0x0Fu;
        if (sn != rx->next_sn) {
            /* Sequence numbers wrap 0..15, so a single lost frame is
             * detectable but a loss of exactly 16 is not - which is why
             * the transport also has timeouts. */
            rx->err_sequence++;
            rx_abort(rx);
            return ISOTP_EV_ERR_SEQUENCE;
        }

        uint16_t remaining = (uint16_t)(rx->total_len - rx->received);
        uint16_t avail     = (uint16_t)(dlc - 1u);
        uint16_t take      = (remaining < avail) ? remaining : avail;

        memcpy(&rx->buf[rx->received], &data[1], take);
        rx->received = (uint16_t)(rx->received + take);
        rx->next_sn  = (uint8_t)((rx->next_sn + 1u) & 0x0Fu);
        rx->cf_since_fc++;

        if (rx->received >= rx->total_len) {
            rx->state = ISOTP_RX_IDLE;
            rx->messages_ok++;
            return ISOTP_EV_MESSAGE;
        }

        if (rx->block_size != 0u && rx->cf_since_fc >= rx->block_size) {
            rx->cf_since_fc = 0u;
            build_fc(rx, ISOTP_FS_CTS);
            return ISOTP_EV_SEND_FC;
        }
        return ISOTP_EV_NONE;
    }

    /* ---- flow control ------------------------------------------- */
    case ISOTP_PCI_FC:
        /* An FC arriving on the receive path is the peer's, not ours.
         * The caller routes it to isotp_tx_flow_control(). */
        return ISOTP_EV_NONE;

    default:
        rx->err_length++;
        return ISOTP_EV_ERR_LENGTH;
    }
}

/* ------------------------------------------------------------------ */
/* Transmit                                                            */
/* ------------------------------------------------------------------ */

bool isotp_tx_begin(isotp_tx_t *tx, const uint8_t *payload, uint16_t len)
{
    if (tx == 0 || payload == 0 || len == 0u || len > ISOTP_MAX_PAYLOAD) {
        return false;
    }
    memset(tx, 0, sizeof *tx);
    tx->payload   = payload;
    tx->total_len = len;
    tx->sent      = 0u;
    tx->next_sn   = 1u;
    tx->state     = ISOTP_TX_IDLE;
    return true;
}

uint8_t isotp_tx_next(isotp_tx_t *tx, uint8_t out[ISOTP_CAN_DLC])
{
    if (tx == 0 || out == 0) { return 0u; }

    switch (tx->state) {

    case ISOTP_TX_IDLE:
        if (tx->total_len <= 7u) {
            /* Single frame. Pad to 8 bytes: many ECUs and most CAN-FD
             * gateways expect fixed-length frames. */
            memset(out, 0xCC, ISOTP_CAN_DLC);
            out[0] = (uint8_t)((ISOTP_PCI_SF << 4) | (tx->total_len & 0x0Fu));
            memcpy(&out[1], tx->payload, tx->total_len);
            tx->sent  = tx->total_len;
            tx->state = ISOTP_TX_DONE;
            return ISOTP_CAN_DLC;
        }
        /* First frame: 12-bit length, then 6 payload bytes. */
        out[0] = (uint8_t)((ISOTP_PCI_FF << 4) | ((tx->total_len >> 8) & 0x0Fu));
        out[1] = (uint8_t)(tx->total_len & 0xFFu);
        memcpy(&out[2], tx->payload, 6u);
        tx->sent  = 6u;
        tx->state = ISOTP_TX_WAIT_FC;      /* must not send more until FC */
        return ISOTP_CAN_DLC;

    case ISOTP_TX_SENDING: {
        if (tx->block_size != 0u && tx->cf_in_block >= tx->block_size) {
            /* Block finished. Stop and wait for the next FC - sending on
             * regardless is the most common ISO-TP implementation bug and
             * it only shows up against a slow ECU. */
            tx->state = ISOTP_TX_WAIT_FC;
            return 0u;
        }
        uint16_t remaining = (uint16_t)(tx->total_len - tx->sent);
        uint8_t  take      = (remaining > 7u) ? 7u : (uint8_t)remaining;

        memset(out, 0xCC, ISOTP_CAN_DLC);
        out[0] = (uint8_t)((ISOTP_PCI_CF << 4) | (tx->next_sn & 0x0Fu));
        memcpy(&out[1], &tx->payload[tx->sent], take);

        tx->sent    = (uint16_t)(tx->sent + take);
        tx->next_sn = (uint8_t)((tx->next_sn + 1u) & 0x0Fu);
        tx->cf_in_block++;

        if (tx->sent >= tx->total_len) {
            tx->state = ISOTP_TX_DONE;
        }
        return ISOTP_CAN_DLC;
    }

    case ISOTP_TX_WAIT_FC:
    case ISOTP_TX_DONE:
    case ISOTP_TX_ABORTED:
    default:
        return 0u;
    }
}

isotp_event_t isotp_tx_flow_control(isotp_tx_t *tx, const uint8_t *data, uint8_t dlc)
{
    if (tx == 0 || data == 0 || dlc < 3u) {
        return ISOTP_EV_ERR_LENGTH;
    }
    if ((data[0] >> 4) != ISOTP_PCI_FC) {
        return ISOTP_EV_ERR_UNEXPECTED;
    }
    if (tx->state != ISOTP_TX_WAIT_FC && tx->state != ISOTP_TX_SENDING) {
        return ISOTP_EV_ERR_UNEXPECTED;
    }

    switch (data[0] & 0x0Fu) {
    case ISOTP_FS_CTS:
        tx->block_size  = data[1];
        tx->st_min      = data[2];
        tx->cf_in_block = 0u;
        tx->state       = ISOTP_TX_SENDING;
        return ISOTP_EV_NONE;

    case ISOTP_FS_WAIT:
        /* The receiver is busy. Stay put and wait for another FC.
         * A real stack bounds this: N_WFTmax (typically 0 or a small
         * number) caps how many WAITs you will accept before aborting,
         * otherwise a stuck ECU holds your transmitter forever. */
        tx->wait_count++;
        tx->state = ISOTP_TX_WAIT_FC;
        return ISOTP_EV_NONE;

    case ISOTP_FS_OVFLW:
        /* The receiver told us its buffer is too small. Abort cleanly:
         * continuing would overrun a buffer on the other node. */
        tx->state = ISOTP_TX_ABORTED;
        return ISOTP_EV_ERR_OVERFLOW;

    default:
        tx->state = ISOTP_TX_ABORTED;
        return ISOTP_EV_ERR_UNEXPECTED;
    }
}

const char *isotp_event_name(isotp_event_t e)
{
    switch (e) {
    case ISOTP_EV_NONE:           return "-";
    case ISOTP_EV_MESSAGE:        return "MESSAGE";
    case ISOTP_EV_SEND_FC:        return "SEND_FC";
    case ISOTP_EV_ERR_SEQUENCE:   return "ERR_SEQUENCE";
    case ISOTP_EV_ERR_OVERFLOW:   return "ERR_OVERFLOW";
    case ISOTP_EV_ERR_LENGTH:     return "ERR_LENGTH";
    case ISOTP_EV_ERR_UNEXPECTED: return "ERR_UNEXPECTED";
    default:                      return "?";
    }
}
