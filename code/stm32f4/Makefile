# Register-accurate STM32F4 drivers.
#
# These are COMPILE-ONLY: there is no QEMU machine that emulates the
# STM32F4 I2C and SPI peripherals faithfully enough to be worth
# pretending. `make` proves they build clean at -Werror -Wconversion for
# a real Cortex-M4; running them needs a board.
#
# That is a smaller claim than the qemu-cortex-m examples make, and it is
# labelled as such in the headers and in the docs that link here.

CROSS  ?= arm-none-eabi-
CC     := $(CROSS)gcc
CFLAGS := -mcpu=cortex-m4 -mthumb -std=c11 -O2 -g3 \
          -Wall -Wextra -Werror -Wconversion -Wshadow -Wundef \
          -ffreestanding -fno-common

OBJS := i2c_master.o spi_master.o

.PHONY: all clean
all: $(OBJS)
	@echo "stm32f4 drivers compile clean (not hardware-verified)"

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o
