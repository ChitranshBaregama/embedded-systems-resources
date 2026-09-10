#ifndef SPI_MASTER_H
#define SPI_MASTER_H
#include <stdint.h>
#include <stdbool.h>

/* CPOL/CPHA as a single mode number, the way every datasheet quotes it.
 *   mode 0: CPOL=0 CPHA=0   sample on rising, idle low   (most common)
 *   mode 1: CPOL=0 CPHA=1   sample on falling, idle low
 *   mode 2: CPOL=1 CPHA=0   sample on falling, idle high
 *   mode 3: CPOL=1 CPHA=1   sample on rising, idle high  (also common)
 */
void    spi_init(uint8_t mode, uint32_t pclk2_hz, uint32_t sck_hz_max);
uint8_t spi_transfer(uint8_t tx);
void    spi_transfer_buf(const uint8_t *tx, uint8_t *rx, uint32_t len);
void    spi_cs_assert(void);
void    spi_cs_release(void);
bool    spi_wait_idle(void);

#endif
