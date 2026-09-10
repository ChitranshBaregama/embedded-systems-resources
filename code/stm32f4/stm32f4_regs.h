/* Hand-written register definitions for STM32F4 I2C1 and SPI1.
 *
 * Deliberately not CMSIS. Typing these out of the reference manual once
 * is how the memory map stops being magic - and it is the exercise an
 * interviewer is really setting when they ask you to "bring up I2C".
 *
 * Source: RM0090 rev 19, sections 6 (RCC), 8 (GPIO), 27 (I2C), 28 (SPI).
 */
#ifndef STM32F4_REGS_H
#define STM32F4_REGS_H

#include <stdint.h>

#define __IO volatile

typedef struct {
    __IO uint32_t CR1, CR2, OAR1, OAR2, DR, SR1, SR2, CCR, TRISE, FLTR;
} I2C_TypeDef;

typedef struct {
    __IO uint32_t CR1, CR2, SR, DR, CRCPR, RXCRCR, TXCRCR, I2SCFGR, I2SPR;
} SPI_TypeDef;

typedef struct {
    __IO uint32_t MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFR[2];
} GPIO_TypeDef;

typedef struct {
    __IO uint32_t CR, PLLCFGR, CFGR, CIR, AHB1RSTR, AHB2RSTR, AHB3RSTR,
                  RESERVED0, APB1RSTR, APB2RSTR, RESERVED1[2],
                  AHB1ENR, AHB2ENR, AHB3ENR, RESERVED2, APB1ENR, APB2ENR;
} RCC_TypeDef;

#define PERIPH_BASE   0x40000000u
#define APB1PERIPH    (PERIPH_BASE + 0x00000000u)
#define APB2PERIPH    (PERIPH_BASE + 0x00010000u)
#define AHB1PERIPH    (PERIPH_BASE + 0x00020000u)

#define I2C1   ((I2C_TypeDef  *)(APB1PERIPH + 0x5400u))
#define SPI1   ((SPI_TypeDef  *)(APB2PERIPH + 0x3000u))
#define GPIOA  ((GPIO_TypeDef *)(AHB1PERIPH + 0x0000u))
#define GPIOB  ((GPIO_TypeDef *)(AHB1PERIPH + 0x0400u))
#define RCC    ((RCC_TypeDef  *)(AHB1PERIPH + 0x3800u))

/* --- RCC --- */
#define RCC_AHB1ENR_GPIOAEN  (1u << 0)
#define RCC_AHB1ENR_GPIOBEN  (1u << 1)
#define RCC_APB1ENR_I2C1EN   (1u << 21)
#define RCC_APB2ENR_SPI1EN   (1u << 12)

/* --- I2C CR1 --- */
#define I2C_CR1_PE      (1u << 0)
#define I2C_CR1_START   (1u << 8)
#define I2C_CR1_STOP    (1u << 9)
#define I2C_CR1_ACK     (1u << 10)
#define I2C_CR1_SWRST   (1u << 15)
/* --- I2C SR1 --- */
#define I2C_SR1_SB      (1u << 0)
#define I2C_SR1_ADDR    (1u << 1)
#define I2C_SR1_BTF     (1u << 2)
#define I2C_SR1_RXNE    (1u << 6)
#define I2C_SR1_TXE     (1u << 7)
#define I2C_SR1_BERR    (1u << 8)
#define I2C_SR1_ARLO    (1u << 9)
#define I2C_SR1_AF      (1u << 10)
#define I2C_SR1_OVR     (1u << 11)
/* --- I2C SR2 --- */
#define I2C_SR2_MSL     (1u << 0)
#define I2C_SR2_BUSY    (1u << 1)

/* --- SPI --- */
#define SPI_CR1_CPHA    (1u << 0)
#define SPI_CR1_CPOL    (1u << 1)
#define SPI_CR1_MSTR    (1u << 2)
#define SPI_CR1_BR_Pos  3
#define SPI_CR1_SPE     (1u << 6)
#define SPI_CR1_LSBFIRST (1u << 7)
#define SPI_CR1_SSI     (1u << 8)
#define SPI_CR1_SSM     (1u << 9)
#define SPI_SR_RXNE     (1u << 0)
#define SPI_SR_TXE      (1u << 1)
#define SPI_SR_OVR      (1u << 6)
#define SPI_SR_BSY      (1u << 7)

#endif
