#ifndef I2C_MASTER_H
#define I2C_MASTER_H
#include <stdint.h>

typedef enum {
    I2C_OK = 0,
    I2C_ERR_BUS_BUSY,
    I2C_ERR_TIMEOUT_START,
    I2C_ERR_NACK,
    I2C_ERR_TIMEOUT_TX,
    I2C_ERR_TIMEOUT_RX,
    I2C_ERR_PARAM
} i2c_result_t;

void         i2c_init(uint32_t pclk1_hz, uint32_t scl_hz);
i2c_result_t i2c_write(uint8_t addr7, const uint8_t *data, uint32_t len);
i2c_result_t i2c_read(uint8_t addr7, uint8_t *data, uint32_t len);
i2c_result_t i2c_write_read(uint8_t addr7, const uint8_t *tx, uint32_t tx_len,
                            uint8_t *rx, uint32_t rx_len);
void         i2c_bus_recover(void);
const char  *i2c_strerror(i2c_result_t r);

#endif
