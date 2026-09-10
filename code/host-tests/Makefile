# Host unit tests. No hardware, no emulator - just cc and a few seconds.
#
#   make          build and run everything under ASan + UBSan
#   make fast     no sanitisers, for tight edit/run loops
#   make clean
#
# Sanitisers are ON by default deliberately. A buffer overrun in a
# protocol parser is the bug class that becomes a CVE; catching it here
# costs nothing.

CC      ?= gcc
WARN    := -Wall -Wextra -Wconversion -Wshadow -Wundef -Wpointer-arith
SAN     := -fsanitize=address,undefined -fno-omit-frame-pointer
CFLAGS  := -std=c11 -O1 -g $(WARN)
PORTABLE := ../portable

TESTS := test_frame test_ringbuf

# Note: run-% targets are deliberately NOT in .PHONY. GNU make skips
# implicit and pattern rules for anything declared phony, so listing them
# there silently turns `make` into a no-op.
.PHONY: all fast clean

all: $(TESTS)
	@for t in $(TESTS); do ./$$t || exit 1; done
	@echo ""
	@echo "all host tests passed"

fast: CFLAGS := -std=c11 -O2 $(WARN)
fast: SAN :=
fast: all

test_frame: test_frame.c $(PORTABLE)/frame/frame.c test.h
	$(CC) $(CFLAGS) $(SAN) -I. $< $(PORTABLE)/frame/frame.c -o $@

test_ringbuf: test_ringbuf.c $(PORTABLE)/ringbuf/ringbuf.h test.h
	$(CC) $(CFLAGS) $(SAN) -I. $< -o $@

clean:
	rm -f $(TESTS)
