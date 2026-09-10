#include "frame.h"

/* CRC-16/CCITT-FALSE: poly 0x1021, init 0xFFFF, no reflection, no final
 * XOR. Written bitwise rather than with a 512-byte table because on a
 * part with 2 KB of flash to spare the table is the wrong trade. Swap in
 * a table when you measure that you need to - not before. */
uint16_t crc16_ccitt_update(uint16_t crc, uint8_t byte)
{
    crc ^= (uint16_t)((uint16_t)byte << 8);
    for (int i = 0; i < 8; i++) {
        crc = (uint16_t)((crc & 0x8000u) ? ((uint16_t)(crc << 1) ^ 0x1021u)
                                         : (uint16_t)(crc << 1));
    }
    return crc;
}

uint16_t crc16_ccitt(const uint8_t *data, size_t len)
{
    uint16_t crc = 0xFFFFu;
    for (size_t i = 0u; i < len; i++) {
        crc = crc16_ccitt_update(crc, data[i]);
    }
    return crc;
}

void frame_reset(frame_parser_t *p)
{
    p->state        = FRAME_ST_IDLE;
    p->length       = 0u;
    p->index        = 0u;
    p->crc_received = 0u;
    p->crc_computed = 0xFFFFu;
}

/* One byte in, one event out. No loops, no I/O, bounded work per call -
 * so worst-case execution time is a constant you can actually state. */
frame_event_t frame_feed(frame_parser_t *p, uint8_t byte)
{
    switch (p->state) {

    case FRAME_ST_IDLE:
        if (byte == FRAME_STX) {
            p->length       = 0u;
            p->index        = 0u;
            p->crc_computed = 0xFFFFu;
            p->state        = FRAME_ST_LEN;
        } else {
            /* Resynchronisation: silently drop anything before STX, but
             * COUNT it. A rising discard count is how you spot a baud
             * mismatch or a partially-powered peer. */
            p->bytes_discarded++;
        }
        return FRAME_EV_NONE;

    case FRAME_ST_LEN:
        if (byte > FRAME_MAX_PAYLOAD) {
            /* Bounds check BEFORE trusting the length. Omitting this is
             * the single most common remotely-triggerable buffer
             * overflow in embedded protocol code. */
            p->err_length++;
            p->state = FRAME_ST_IDLE;
            return FRAME_EV_ERR_LENGTH;
        }
        p->length       = byte;
        p->crc_computed = crc16_ccitt_update(p->crc_computed, byte);
        p->state        = (byte == 0u) ? FRAME_ST_CRC_LO : FRAME_ST_PAYLOAD;
        return FRAME_EV_NONE;

    case FRAME_ST_PAYLOAD:
        p->payload[p->index++] = byte;
        p->crc_computed = crc16_ccitt_update(p->crc_computed, byte);
        if (p->index >= p->length) {
            p->state = FRAME_ST_CRC_LO;
        }
        return FRAME_EV_NONE;

    case FRAME_ST_CRC_LO:
        p->crc_received = byte;
        p->state        = FRAME_ST_CRC_HI;
        return FRAME_EV_NONE;

    case FRAME_ST_CRC_HI:
        p->crc_received |= (uint16_t)((uint16_t)byte << 8);
        if (p->crc_received != p->crc_computed) {
            p->err_crc++;
            p->state = FRAME_ST_IDLE;
            return FRAME_EV_ERR_CRC;
        }
        p->state = FRAME_ST_ETX;
        return FRAME_EV_NONE;

    case FRAME_ST_ETX:
        p->state = FRAME_ST_IDLE;
        if (byte != FRAME_ETX) {
            p->err_framing++;
            return FRAME_EV_ERR_FRAMING;
        }
        p->frames_ok++;
        return FRAME_EV_COMPLETE;

    default:
        /* Unreachable by construction. Recovering rather than asserting
         * is the right call in shipped firmware: a corrupted state
         * variable should cost you one frame, not the product. */
        frame_reset(p);
        return FRAME_EV_NONE;
    }
}

const char *frame_state_name(frame_state_t s)
{
    switch (s) {
    case FRAME_ST_IDLE:    return "IDLE";
    case FRAME_ST_LEN:     return "LEN";
    case FRAME_ST_PAYLOAD: return "PAYLOAD";
    case FRAME_ST_CRC_LO:  return "CRC_LO";
    case FRAME_ST_CRC_HI:  return "CRC_HI";
    case FRAME_ST_ETX:     return "ETX";
    default:               return "?";
    }
}

const char *frame_event_name(frame_event_t e)
{
    switch (e) {
    case FRAME_EV_NONE:        return "-";
    case FRAME_EV_COMPLETE:    return "COMPLETE";
    case FRAME_EV_ERR_LENGTH:  return "ERR_LENGTH";
    case FRAME_EV_ERR_CRC:     return "ERR_CRC";
    case FRAME_EV_ERR_FRAMING: return "ERR_FRAMING";
    default:                   return "?";
    }
}
