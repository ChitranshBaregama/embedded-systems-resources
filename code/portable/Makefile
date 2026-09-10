# Cross-compile every portable module for a real Cortex-M target.
#
# The host tests prove the LOGIC. This proves the same source also builds
# clean for the part it is meant to run on - at -Werror -Wconversion,
# freestanding, no libc. Without this step "portable" is an assertion
# rather than a fact.
#
#   make        cross-compile for Cortex-M4
#   make m0     cross-compile for Cortex-M0+ (no hardware divide, stricter)
#   make clean

CROSS  ?= arm-none-eabi-
CC     := $(CROSS)gcc
CPU    ?= -mcpu=cortex-m4 -mthumb

# Deferred (=) not immediate (:=), so the `m0` target's CPU override is
# actually picked up. With := the flags are baked in at parse time and the
# m0 build silently compiles for Cortex-M4 while claiming otherwise - which
# is exactly what the first version of this file did.
CFLAGS = $(CPU) -std=c11 -O2 -g3 \
          -Wall -Wextra -Werror -Wconversion -Wshadow -Wundef \
          -ffreestanding -fno-common

SRCS := frame/frame.c can/can_bittiming.c can/isotp.c
OBJS := $(SRCS:.c=.o)

# Header-only modules still need a compile check.
HDR_CHECK := ringbuf_check.o

.PHONY: all m0 clean

all: $(OBJS) $(HDR_CHECK)
	@echo "portable layer compiles clean for $(CPU)"

m0: CPU := -mcpu=cortex-m0plus -mthumb
m0: clean all

%.o: %.c
	$(CC) $(CFLAGS) -I$(dir $<) -c $< -o $@

# ringbuf.h is header-only; compile a translation unit that instantiates
# every static inline so the code is actually generated and checked.
ringbuf_check.o: ringbuf/ringbuf.h
	@printf '#include "ringbuf.h"\nvoid rb_check(ringbuf_t *r, uint8_t *o);\nvoid rb_check(ringbuf_t *r, uint8_t *o){ rb_init(r); (void)rb_put(r,1); (void)rb_get(r,o); (void)rb_count(r); (void)rb_full(r); (void)rb_empty(r); }\n' > /tmp/rb_check.c
	$(CC) $(CFLAGS) -Iringbuf -c /tmp/rb_check.c -o $@

clean:
	rm -f $(OBJS) $(HDR_CHECK)
