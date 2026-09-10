/* STM32F4 I2C1 master, registers only, no HAL.
 *
 * STATUS: compiles clean for Cortex-M4 with -Wall -Wextra -Werror
 *         -Wconversion. NOT yet run on hardware. Every I2C claim that
 *         depends on analogue behaviour - rise time, clock stretching by
 *         a real target, bus capacitance - is unverified until it is.
 *         Treat this as a reviewed reference implementation, not a
 *         tested one, and see the STATUS note in peripherals/i2c.md.
 *
 * Three things in here are the difference between code that works on a
 * bench and code that survives a field deployment:
 *
 *   1. EVERY wait loop has a timeout. A bare `while (!(SR1 & SB));` is
 *      the single most common way an I2C driver hangs a product forever:
 *      one target holding SDA low is all it takes.
 *   2. The ADDR flag is cleared by the documented read-SR1-then-SR2
 *      sequence, in that order. Doing it any other way leaves the flag
 *      set and the transfer stalls.
 *   3. There is a bus-recovery path. When a target is mid-byte at the
 *      moment the master resets, it can hold SDA low indefinitely; only
 *      clocking it out manually frees the bus.
 */
#include "stm32f4_regs.h"
#include "i2c_master.h"

/* Loop budget, not microseconds: this deliberately does not depend on a
 * timebase, so it works before SysTick is running - which is exactly
 * when you most need to debug a bring-up problem. Calibrate it once for
 * your clock and slowest target. */
#ifndef I2C_TIMEOUT_LOOPS
#define I2C_TIMEOUT_LOOPS 100000u
#endif

#define WAIT_FOR(cond, err)                          \
    do {                                             \
        uint32_t _n = I2C_TIMEOUT_LOOPS;             \
        while (!(cond)) {                            \
            if (--_n == 0u) { return (err); }        \
            if (I2C1->SR1 & I2C_SR1_AF) {            \
                I2C1->SR1 &= ~I2C_SR1_AF;            \
                I2C1->CR1 |= I2C_CR1_STOP;           \
                return I2C_ERR_NACK;                 \
            }                                        \
        }                                            \
    } while (0)

void i2c_init(uint32_t pclk1_hz, uint32_t scl_hz)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    /* PB6 = SCL, PB7 = SDA: alternate function 4, open-drain, pull-up.
     * Open-drain is not optional - I2C is a wired-AND bus and a
     * push-pull driver will fight the target and eventually damage one
     * of them. */
    GPIOB->MODER   = (GPIOB->MODER   & ~(0xFu << 12)) | (0xAu << 12);
    GPIOB->OTYPER |= (3u << 6);                     /* open-drain       */
    GPIOB->OSPEEDR = (GPIOB->OSPEEDR & ~(0xFu << 12)) | (0xFu << 12);
    GPIOB->PUPDR   = (GPIOB->PUPDR   & ~(0xFu << 12)) | (0x5u << 12);
    GPIOB->AFR[0]  = (GPIOB->AFR[0]  & ~(0xFFu << 24)) | (0x44u << 24);

    /* Reset the peripheral before configuring. Skipping this leaves
     * stale state after a warm reset and produces bugs that only appear
     * on the second boot. */
    I2C1->CR1 = I2C_CR1_SWRST;
    I2C1->CR1 = 0u;

    uint32_t pclk_mhz = pclk1_hz / 1000000u;
    I2C1->CR2 = pclk_mhz;                           /* peripheral clock */

    if (scl_hz <= 100000u) {
        /* Standard mode: CCR = pclk / (2 * scl). Minimum 4. */
        uint32_t ccr = pclk1_hz / (2u * scl_hz);
        I2C1->CCR   = (ccr < 4u) ? 4u : ccr;
        /* TRISE = (max rise time / pclk period) + 1; 1000 ns in Sm. */
        I2C1->TRISE = pclk_mhz + 1u;
    } else {
        /* Fast mode, 2:1 duty: CCR = pclk / (3 * scl). Minimum 1. */
        uint32_t ccr = pclk1_hz / (3u * scl_hz);
        I2C1->CCR   = (1u << 15) | ((ccr < 1u) ? 1u : ccr);
        I2C1->TRISE = ((pclk_mhz * 300u) / 1000u) + 1u;  /* 300 ns in Fm */
    }

    I2C1->CR1 = I2C_CR1_PE;
}

static i2c_result_t i2c_start(uint8_t addr7, int reading)
{
    WAIT_FOR((I2C1->SR2 & I2C_SR2_BUSY) == 0u, I2C_ERR_BUS_BUSY);

    I2C1->CR1 |= I2C_CR1_ACK | I2C_CR1_START;
    WAIT_FOR(I2C1->SR1 & I2C_SR1_SB, I2C_ERR_TIMEOUT_START);

    /* Reading SR1 (done by the wait) then writing DR clears SB. */
    I2C1->DR = (uint32_t)((uint8_t)(addr7 << 1) | (reading ? 1u : 0u));
    WAIT_FOR(I2C1->SR1 & I2C_SR1_ADDR, I2C_ERR_NACK);

    /* Clear ADDR: read SR1, then SR2. The order is mandated by the
     * reference manual and the two reads must both happen. */
    (void)I2C1->SR1;
    (void)I2C1->SR2;
    return I2C_OK;
}

i2c_result_t i2c_write(uint8_t addr7, const uint8_t *data, uint32_t len)
{
    i2c_result_t r = i2c_start(addr7, 0);
    if (r != I2C_OK) { return r; }

    for (uint32_t i = 0u; i < len; i++) {
        WAIT_FOR(I2C1->SR1 & I2C_SR1_TXE, I2C_ERR_TIMEOUT_TX);
        I2C1->DR = data[i];
    }

    /* Wait for BTF, not just TXE. TXE means the shift register took the
     * byte; BTF means it has actually gone out on the wire. Issuing STOP
     * on TXE truncates the last byte - a classic and very confusing bug
     * because the logic analyzer shows N-1 bytes and the code shows N. */
    WAIT_FOR(I2C1->SR1 & I2C_SR1_BTF, I2C_ERR_TIMEOUT_TX);
    I2C1->CR1 |= I2C_CR1_STOP;
    return I2C_OK;
}

i2c_result_t i2c_read(uint8_t addr7, uint8_t *data, uint32_t len)
{
    if (len == 0u) { return I2C_ERR_PARAM; }

    i2c_result_t r = i2c_start(addr7, 1);
    if (r != I2C_OK) { return r; }

    for (uint32_t i = 0u; i < len; i++) {
        if (i == (len - 1u)) {
            /* NACK the final byte BEFORE reading it, then queue STOP.
             * ACKing the last byte tells the target to send another one,
             * and the bus locks up waiting for a clock that never comes. */
            I2C1->CR1 &= ~I2C_CR1_ACK;
            I2C1->CR1 |= I2C_CR1_STOP;
        }
        WAIT_FOR(I2C1->SR1 & I2C_SR1_RXNE, I2C_ERR_TIMEOUT_RX);
        data[i] = (uint8_t)(I2C1->DR & 0xFFu);
    }

    I2C1->CR1 |= I2C_CR1_ACK;      /* restore for the next transfer */
    return I2C_OK;
}

i2c_result_t i2c_write_read(uint8_t addr7, const uint8_t *tx, uint32_t tx_len,
                            uint8_t *rx, uint32_t rx_len)
{
    /* Register read = write the register index, REPEATED START, read.
     * A STOP between the two lets another master interleave and the
     * target may serve your read from a different register pointer. */
    i2c_result_t r = i2c_start(addr7, 0);
    if (r != I2C_OK) { return r; }

    for (uint32_t i = 0u; i < tx_len; i++) {
        WAIT_FOR(I2C1->SR1 & I2C_SR1_TXE, I2C_ERR_TIMEOUT_TX);
        I2C1->DR = tx[i];
    }
    WAIT_FOR(I2C1->SR1 & I2C_SR1_BTF, I2C_ERR_TIMEOUT_TX);

    return i2c_read(addr7, rx, rx_len);   /* issues the repeated START */
}

/* Bus recovery. Not optional in a product.
 *
 * If the master resets while a target is driving a data bit low, the
 * target never sees the rest of the byte and holds SDA down forever.
 * The peripheral cannot fix this - it sees a busy bus and refuses to
 * start. The only way out is to release the pins to GPIO and clock SCL
 * by hand until the target finishes its byte and lets go.
 */
void i2c_bus_recover(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    /* PB6/PB7 to open-drain GPIO output, both released high. */
    GPIOB->MODER  = (GPIOB->MODER & ~(0xFu << 12)) | (0x5u << 12);
    GPIOB->OTYPER |= (3u << 6);
    GPIOB->BSRR    = (3u << 6);

    for (int i = 0; i < 9; i++) {          /* 9 = 8 data bits + ACK */
        GPIOB->BSRR = (1u << (6 + 16));    /* SCL low  */
        for (volatile int d = 0; d < 200; d++) { }
        GPIOB->BSRR = (1u << 6);           /* SCL high */
        for (volatile int d = 0; d < 200; d++) { }
        if (GPIOB->IDR & (1u << 7)) { break; }   /* target released SDA */
    }

    /* Manual STOP: SDA low while SCL high, then SDA high. */
    GPIOB->BSRR = (1u << (7 + 16));
    for (volatile int d = 0; d < 200; d++) { }
    GPIOB->BSRR = (1u << 7);

    I2C1->CR1 = I2C_CR1_SWRST;
    I2C1->CR1 = 0u;
}

const char *i2c_strerror(i2c_result_t r)
{
    switch (r) {
    case I2C_OK:                return "ok";
    case I2C_ERR_BUS_BUSY:      return "bus busy - try i2c_bus_recover()";
    case I2C_ERR_TIMEOUT_START: return "no START - check pull-ups and clocks";
    case I2C_ERR_NACK:          return "address or data NACK - wrong address?";
    case I2C_ERR_TIMEOUT_TX:    return "TX timeout - target clock-stretching?";
    case I2C_ERR_TIMEOUT_RX:    return "RX timeout";
    case I2C_ERR_PARAM:         return "bad argument";
    default:                    return "?";
    }
}
