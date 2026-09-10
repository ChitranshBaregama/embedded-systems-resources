/* STM32F4 SPI1 master, registers only, no HAL.
 *
 * STATUS: compiles clean for Cortex-M4 with -Wall -Wextra -Werror
 *         -Wconversion. NOT yet run on hardware. See the STATUS note in
 *         peripherals/spi.md.
 *
 * SPI is simple enough that the bugs cluster in three places, and all
 * three are handled explicitly below:
 *
 *   1. SPI is a SHIFT REGISTER, not a transmitter. Every byte you send
 *      produces a byte back. Ignoring the received byte leaves RXNE set
 *      and the next read returns stale data - which looks exactly like
 *      "the sensor is off by one".
 *   2. TXE going high does NOT mean the transfer finished. It means the
 *      TX buffer is free. Deasserting CS on TXE truncates the last byte.
 *      Wait for BSY to clear.
 *   3. The baud prescaler is a power of two only. You cannot ask for
 *      "8 MHz"; you get the fastest divider that does not exceed it.
 */
#include "stm32f4_regs.h"
#include "spi_master.h"

#define SPI_TIMEOUT_LOOPS 100000u

void spi_init(uint8_t mode, uint32_t pclk2_hz, uint32_t sck_hz_max)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    /* PA5 SCK, PA6 MISO, PA7 MOSI -> AF5.  PA4 as plain GPIO chip select,
     * driven in software: hardware NSS on the STM32 is more trouble than
     * it is worth as soon as you have more than one device on the bus. */
    GPIOA->MODER   = (GPIOA->MODER   & ~(0xFFu << 8))  | (0xAAu << 10);
    GPIOA->MODER   = (GPIOA->MODER   & ~(0x3u  << 8))  | (0x1u  << 8);
    GPIOA->OSPEEDR |= (0xFFu << 8);
    GPIOA->AFR[0]  = (GPIOA->AFR[0]  & ~(0xFFFu << 20)) | (0x555u << 20);
    GPIOA->BSRR    = (1u << 4);                      /* CS idles high */

    /* Prescaler is 2^(BR+1). Pick the smallest divider whose resulting
     * clock is <= the device's maximum - never the nearest, because
     * "nearest" can be 10% over and the part will latch garbage. */
    uint32_t br = 0u;
    uint32_t div = 2u;
    while ((pclk2_hz / div) > sck_hz_max && br < 7u) {
        br++;
        div <<= 1;
    }

    SPI1->CR1 = 0u;
    SPI1->CR1 = SPI_CR1_MSTR
              | SPI_CR1_SSM | SPI_CR1_SSI      /* software slave management */
              | (br << SPI_CR1_BR_Pos)
              | ((mode & 1u) ? SPI_CR1_CPHA : 0u)
              | ((mode & 2u) ? SPI_CR1_CPOL : 0u);
    SPI1->CR2 = 0u;
    SPI1->CR1 |= SPI_CR1_SPE;
}

void spi_cs_assert(void)  { GPIOA->BSRR = (1u << (4 + 16)); }

void spi_cs_release(void)
{
    /* Never deassert CS until BSY clears, or the final byte is cut off
     * mid-shift and the device sees a short frame. */
    (void)spi_wait_idle();
    GPIOA->BSRR = (1u << 4);
}

bool spi_wait_idle(void)
{
    uint32_t n = SPI_TIMEOUT_LOOPS;
    while (SPI1->SR & SPI_SR_BSY) {
        if (--n == 0u) { return false; }
    }
    return true;
}

uint8_t spi_transfer(uint8_t tx)
{
    uint32_t n = SPI_TIMEOUT_LOOPS;
    while (!(SPI1->SR & SPI_SR_TXE)) {
        if (--n == 0u) { return 0xFFu; }
    }
    SPI1->DR = tx;

    n = SPI_TIMEOUT_LOOPS;
    while (!(SPI1->SR & SPI_SR_RXNE)) {
        if (--n == 0u) { return 0xFFu; }
    }
    /* Reading DR is what clears RXNE. Skipping this read is bug (1). */
    return (uint8_t)(SPI1->DR & 0xFFu);
}

void spi_transfer_buf(const uint8_t *tx, uint8_t *rx, uint32_t len)
{
    /* Full duplex: pass NULL for whichever direction you do not care
     * about. 0xFF is the conventional filler because it leaves MOSI
     * high, which most devices treat as idle. */
    for (uint32_t i = 0u; i < len; i++) {
        uint8_t out = (tx != 0) ? tx[i] : 0xFFu;
        uint8_t in  = spi_transfer(out);
        if (rx != 0) { rx[i] = in; }
    }
}
