# Shared build rules for every example in code/qemu-cortex-m/.
# Each example's Makefile sets TARGET and SRCS, then includes this.
#
#   make        build
#   make run    run under QEMU (Ctrl-A then X to quit)
#   make size   show where the flash and RAM went
#   make disasm dump the disassembly
#   make clean

COMMON_DIR := $(dir $(lastword $(MAKEFILE_LIST)))common

CROSS   ?= arm-none-eabi-
CC      := $(CROSS)gcc
OBJCOPY := $(CROSS)objcopy
OBJDUMP := $(CROSS)objdump
SIZE    := $(CROSS)size
QEMU    ?= qemu-system-arm

CPUFLAGS := -mcpu=cortex-m3 -mthumb

# -ffreestanding      : no hosted libc assumptions
# -fno-common         : every tentative definition gets its own symbol
# -fdata/function-sections + --gc-sections : drop what we never call
CFLAGS := $(CPUFLAGS) -std=c11 -O2 -g3 \
          -Wall -Wextra -Werror -Wshadow -Wconversion -Wundef \
          -ffreestanding -fno-common \
          -ffunction-sections -fdata-sections \
          -I$(COMMON_DIR) $(EXTRA_CFLAGS)

LDFLAGS := $(CPUFLAGS) -T$(COMMON_DIR)/lm3s6965.ld \
           -nostdlib -Wl,--gc-sections \
           -Wl,-Map=$(TARGET).map -Wl,--print-memory-usage

SRCS += $(COMMON_DIR)/startup.c $(COMMON_DIR)/uart.c
OBJS := $(notdir $(SRCS:.c=.o))

# VPATH lets an example pull a .c out of code/portable/ without copying
# it. Sharing the source is the point: the host tests compile the very
# same file.
VPATH := $(COMMON_DIR):.:$(sort $(dir $(SRCS)))

.PHONY: all run size disasm clean

all: $(TARGET).elf $(TARGET).bin

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@
	@$(SIZE) $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# -nographic routes the emulated UART0 to this terminal.
# -semihosting-config lets the guest exit QEMU cleanly with a return code.
run: $(TARGET).elf
	$(QEMU) -M lm3s6965evb -cpu cortex-m3 -nographic \
	        -semihosting-config enable=on,target=native \
	        -kernel $<

size: $(TARGET).elf
	$(SIZE) -A -x $<

disasm: $(TARGET).elf
	$(OBJDUMP) -d -S $< | less

clean:
	rm -f *.o *.elf *.bin *.map
