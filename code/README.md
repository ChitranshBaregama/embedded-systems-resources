# Code

Every program in this directory builds and runs on a normal Linux or macOS
machine. **No development board is required.** The Cortex-M examples run
under QEMU; the logic they exercise is also compiled and tested natively.

That matters for a reason beyond convenience: it means the claims in the
written notes are executable. If a document says the startup code copies
`.data` out of flash, there is a program here that prints the addresses and
proves it.

---

## Layout

| Directory | What it is | Needs hardware? |
| :--- | :--- | :--- |
| [`qemu-cortex-m/`](qemu-cortex-m/) | Bare-metal Cortex-M3 programs — vector table, linker script, drivers, fault handlers | No, QEMU |
| [`portable/`](portable/) | Hardware-independent logic: protocol parser, CRC, ring buffer, CAN bit timing, ISO-TP transport | No |
| [`host-tests/`](host-tests/) | 45 unit tests for `portable/`, run under AddressSanitizer and UBSan | No |
| [`stm32f4/`](stm32f4/) | Register-accurate I²C and SPI masters. Compile-verified only | Yes, to run |

The split between `portable/` and the hardware layer is the architectural
argument this directory is making. `code/portable/frame/frame.c` is compiled
**unmodified** by both the target build and the host test suite. Firmware
written this way can be tested at a million iterations per second on a build
machine; firmware that mixes register writes into its parser cannot be tested
at all without a board.

---

## Prerequisites

```bash
# Debian / Ubuntu
sudo apt install gcc-arm-none-eabi qemu-system-arm build-essential

# macOS
brew install --cask gcc-arm-embedded
brew install qemu
```

Verify:

```bash
arm-none-eabi-gcc --version
qemu-system-arm --version
```

---

## Running everything

```bash
cd code/host-tests   && make        # 45 host unit tests, ~4 seconds
cd code/qemu-cortex-m && ./run-all.sh  # all six target examples
```

Expected output of `run-all.sh`:

```
01-startup-and-memory-map          ok
02-systick-timebase                ok
03-nvic-priority-and-preemption    ok
04-spsc-ring-buffer                ok
05-hardfault-decoder               ok
06-protocol-state-machine          ok

all examples passed
```

Running a single example:

```bash
cd code/qemu-cortex-m/02-systick-timebase
make          # build
make run      # run under QEMU  (Ctrl-A then X to quit)
make size     # section sizes — where the flash and RAM went
make disasm   # annotated disassembly
```

---

## The portable layer

Two of these are worth calling out because they are the pieces most likely to
be wrong in a way nothing on a bench will show you.

### [`portable/can/can_bittiming.c`](portable/can/can_bittiming.c)

Solves CAN bit timing: given a peripheral clock, a bit rate and a target
sample point, produce BRP/TSEG1/TSEG2/SJW — and refuse, rather than
approximate, when no exact solution exists. Also computes the maximum legal
bus length for a given transceiver loop delay.

**8 tests, 502 assertions.** The valuable ones assert that the bit rate is
always exact, that SJW never exceeds PHASE_SEG2, and that impossible requests
come back as "no".

### [`portable/can/isotp.c`](portable/can/isotp.c)

ISO 15765-2 — the transport layer under UDS diagnostics. Segmentation, flow
control, block size, STmin, sequence numbering, and rejection of every
malformed input the wire can produce.

**18 tests, 1.2M assertions**, including a 300k-frame fuzz pass. The tests
that matter are the adversarial ones: a first frame declaring 4,000 bytes to a
64-byte buffer, a single frame claiming more payload than its own DLC, a
consecutive frame with no first frame, an out-of-order sequence number. This
code would sit behind an OBD connector, so "the length field lied" is the
first thing anyone tries.

---

## The examples

### [`01-startup-and-memory-map`](qemu-cortex-m/01-startup-and-memory-map/)

What `Reset_Handler` actually does, demonstrated rather than described. Prints
the runtime address of a `.data` variable, a `.bss` variable, a `.rodata`
string, a stack local, and the linker symbols that bound each region — then
verifies that the copy loop and the zero loop did their jobs.

Read this alongside the linker script in `common/lm3s6965.ld`. Between them
they answer "why is my global garbage before `main`" permanently.

### [`02-systick-timebase`](qemu-cortex-m/02-systick-timebase/)

SysTick configured from the ARM architecture reference alone — no vendor HAL.
Covers the 24-bit reload limit, why the tick counter must be `volatile`, and
overflow-safe delay arithmetic (`now - start >= n`, never `now >= start + n`).

Also demonstrates `COUNTFLAG`'s read-to-clear behaviour, including the way it
misleads you if anything slow happens between the two reads.

### [`03-nvic-priority-and-preemption`](qemu-cortex-m/03-nvic-priority-and-preemption/)

Two interrupts at different priorities, with the high-priority one observably
preempting the low-priority one mid-execution.

Probes the number of **implemented** priority bits at runtime rather than
trusting a header. Most Cortex-M parts implement 3 or 4 of the 8 bits, so
priority values 0x00 and 0x1F can be the same level — a bug that looks like
"the NVIC is ignoring my priorities".

### [`04-spsc-ring-buffer`](qemu-cortex-m/04-spsc-ring-buffer/)

A lock-free single-producer/single-consumer queue with a real ISR as the
producer and `main` as the consumer, on one core, with genuine preemption.

Then it deliberately stalls the consumer to force an overrun, so the drop
path is exercised rather than assumed. The four properties that make the
lock-free version safe — one writer per index, aligned 32-bit atomicity,
power-of-two capacity, free-running indices — are stated in the header and
each one is load-bearing.

### [`05-hardfault-decoder`](qemu-cortex-m/05-hardfault-decoder/)

A fault handler that recovers the stacked exception frame, prints the
faulting PC, and decodes CFSR/HFSR into English. Six selectable faults:

```bash
make FAULT=1 run   # undefined instruction   -> UNDEFINSTR
make FAULT=2 run   # function pointer, LSB=0 -> INVSTATE
make FAULT=3 run   # divide by zero          -> DIVBYZERO
make FAULT=4 run   # unaligned load          -> UNALIGNED
make FAULT=5 run   # stack overflow          -> see note below
make FAULT=6 run   # unmapped address        -> see note below
```

Being able to write this handler from memory is worth more in an interview
than any amount of describing it, because the default `while(1)` fault
handler is where most firmware debugging dies.

### [`06-protocol-state-machine`](qemu-cortex-m/06-protocol-state-machine/)

The framed-protocol parser from `portable/frame/`, running on the target,
fed by the UART RX interrupt. Includes a built-in self-test that injects a
good frame, a corrupted frame, an over-long length field, and line noise —
so CI has something deterministic to assert on.

---

## What QEMU gets wrong

Worth knowing before you trust an emulator result. Observed with QEMU 8.2 on
`-M lm3s6965evb`:

| Behaviour | On real silicon | Under QEMU |
| :--- | :--- | :--- |
| Implemented NVIC priority bits | 3 on LM3S6965 | reports 8 |
| Read from unmapped address | BusFault, `BFAR` valid | silently returns data |
| Stack overflow past SRAM | BusFault (`STKERR`) or corruption | runs on, no fault |
| Instruction timing | cycle-accurate to the part | approximate; loops finish fast |

Everything else in these examples — vector table dispatch, `.data`/`.bss`
init, SysTick, NVIC preemption and nesting, UsageFault decoding, UART FIFO
behaviour — matches the hardware.

The general rule: **anything with a side effect on read, an analogue
characteristic, or a timing dependency needs a real board.** Logic, arithmetic
and control flow do not.

---

## Notes on the build

- `-Wall -Wextra -Werror -Wconversion -Wshadow` on every target. `-Wconversion`
  in particular catches the implicit narrowing that silently corrupts register
  writes.
- `-ffunction-sections -fdata-sections` with `--gc-sections`, so unused code
  is dropped and the reported size is honest.
- `-Wl,--print-memory-usage` prints flash and SRAM consumption on every link.
  Watching that number move as you edit is the cheapest way to build intuition
  about what your code costs.
- No newlib, no `printf`, no heap. `-nostdlib` throughout.

---

## A note on the build order

`code/qemu-cortex-m/common.mk` uses `vpath %.c`, not `VPATH`, and the
difference is load-bearing. `VPATH` makes make search those directories for
*any* target, objects included — so once `make -C code/portable` has left
Cortex-M4 objects beside their sources, a stale `portable/frame/frame.o`
satisfies an example's `frame.o` prerequisite, make skips the rebuild, and the
Cortex-M3 link then fails looking for an object that was never built locally.

Order-dependent, silent until it is not. CI now builds `code/portable` before
running the examples specifically to keep reproducing that condition.

---

## Adding an example

```makefile
TARGET := 07-my-example
SRCS   := main.c
include ../common.mk
```

Then add it to the table above and to `ROADMAP.md`. `run-all.sh` picks up any
directory starting with a digit automatically.
