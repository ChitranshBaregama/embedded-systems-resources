TARGET := 05-hardfault

# Pick which fault to trigger:  make FAULT=3 run
#   1 undefined instruction   2 bad function pointer   3 divide by zero
#   4 unaligned access        5 stack overflow         6 unmapped address
FAULT ?= 1
EXTRA_CFLAGS += -DWHICH_FAULT=$(FAULT)

SRCS := main.c
include ../common.mk
