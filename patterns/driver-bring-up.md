# Driver Bring-Up — A Repeatable Procedure

> How to go from "here is a peripheral you have never used" to "here is a
> driver I trust", without the twenty minutes of staring that usually starts it.

**Contents**
[Cheat sheet](#1-cheat-sheet) ·
[Why people freeze](#2-why-people-freeze) ·
[The seven phases](#3-the-seven-phases) ·
[Reading a datasheet](#4-reading-a-datasheet-for-bring-up) ·
[First light](#5-phase-3-first-light) ·
[Bisecting a dead bus](#6-bisecting-a-dead-bus) ·
[Hardening](#7-phase-6-hardening) ·
[The bring-up log](#8-the-bring-up-log) ·
[Worked example](#9-worked-example-an-unfamiliar-sensor) ·
[Questions](#10-questions-i-should-be-able-to-answer) ·
[Sources](#11-sources)

---

## 1. Cheat sheet

**The order is always the same, and it is not the order of the datasheet.**

| # | Phase | You are proving | Stop when |
| :--- | :--- | :--- | :--- |
| 0 | Scope | What "working" means | You can state the acceptance test in one sentence |
| 1 | Power and clock | The peripheral is alive | A register readback is non-zero and matches reset value |
| 2 | Pins | Signals leave the chip | A pin toggles on a scope, or a loopback returns bytes |
| 3 | First light | One transaction completes | One byte moves, once, blocking, no interrupts |
| 4 | The real transaction | The device answers | A known register reads its documented reset value |
| 5 | Structure | It is a driver, not a script | Blocking calls, an error enum, no globals |
| 6 | Hardening | It survives the field | Timeouts, recovery, counters, no infinite loop anywhere |
| 7 | Verification | You can prove it | Host tests on the logic, a capture on the wire |

**Four rules that do most of the work:**

1. **Never skip phase 1.** More bring-up time is lost to an ungated peripheral
   clock than to any other single cause. A peripheral with no clock does not
   fault — it reads back zeros and ignores writes, which looks exactly like
   bad configuration.
2. **Change one thing at a time.** If you configure eight registers and it does
   not work, you have eight suspects and no information.
3. **Prove the layer below before debugging the layer above.** A "broken I²C
   driver" is usually a missing pull-up.
4. **Blocking first, always.** Get one byte through with a polled, blocking,
   interrupt-free call. Interrupts and DMA are optimisations; adding them
   before the transaction works means debugging two unknowns at once.

---

## 2. Why people freeze

Being handed an unfamiliar peripheral is a specific kind of stuck, and it has
a specific cause: **the datasheet is organised for reference, and you need a
procedure.** Chapter 1 is a block diagram, chapter 2 is 40 pages of register
definitions, and nowhere does it say "do these six things in this order".

So the instinct is to read it all first. That fails, because a peripheral
chapter is only comprehensible once you have made something happen with it.

The fix is to invert it: **run a fixed procedure and consult the datasheet as
each phase demands it.** The procedure below never changes. Only the register
names do. Once you have done it three times, an unfamiliar peripheral stops
being a blank page and becomes a form to fill in — and that is the entire
difference between an engineer who can bring up a new part and one who can
only maintain a driver someone else wrote.

---

## 3. The seven phases

### Phase 0 — Scope it before you touch anything

Write down, in one sentence, what "done" means. Not "get the sensor working"
— something you can execute:

> *Read the WHO_AM_I register of the LIS3DH over SPI1 and get 0x33, then read
> 100 consecutive X/Y/Z samples at 50 Hz with no bus errors.*

This matters more than it sounds. Without it, bring-up expands until you have
implemented interrupts, FIFO mode and power management for a part you have not
yet successfully read one byte from.

Also collect, and put in one place:

- The peripheral chapter of the MCU reference manual (not the datasheet — the
  datasheet has pinouts and electricals, the reference manual has registers)
- The device datasheet, if there is an external device
- The board schematic, or at minimum: which pins, which alternate function,
  which pull-ups are fitted
- The errata sheet. Read it now, not after two days of debugging.

### Phase 1 — Power and clock

Almost every MCU gates peripheral clocks off at reset to save power. A gated
peripheral is not "off" in any visible way: writes are discarded, reads return
zero, and nothing faults.

```c
RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

/* Read it back. On some parts there is a delay of a few cycles before the
 * clock is actually running, and the very next register write is lost. This
 * readback both flushes the write and proves it landed. */
(void)RCC->APB1ENR;
```

**Prove it before moving on.** Read a register with a documented non-zero
reset value and confirm you get that value. If everything reads zero, the
clock is not on — stop, and fix that. Nothing downstream can work.

On a part with multiple power domains, confirm the domain is powered too.

### Phase 2 — Pins

Three separate things must all be right, and they fail independently:

| | Failure mode |
| :--- | :--- |
| **Mux / alternate function** | Pin stays a GPIO; the peripheral drives nothing |
| **Direction and drive type** | Push-pull on an open-drain bus will fight the other device |
| **Pull-up / pull-down / speed** | Edges too slow, or a floating line reads as noise |

**Prove it with a scope or a loopback, not by reading the code again.**

- UART: tie TX to RX and send a byte. If it comes back, the pins and the
  peripheral are both fine and you have eliminated half the system.
- SPI: tie MOSI to MISO and do the same.
- I²C: measure SCL and SDA idle voltage. Both must sit at VDD. If either sits
  low, the bus is held — see [bisecting a dead bus](#6-bisecting-a-dead-bus).
- Anything: configure the pin as a plain GPIO first and toggle it. If it does
  not toggle, the problem is below the peripheral entirely.

### Phase 3 — First light

See [§5](#5-phase-3-first-light).

### Phase 4 — The real transaction

Now do the thing that has a *known correct answer*. Not a write — a **read of
a register whose reset value is documented.**

This is the single most valuable step in bring-up, because it is the first
moment you get a signal that distinguishes "working" from "appears to work":

- WHO_AM_I / device ID / chip ID registers exist on almost every sensor and
  external peripheral precisely for this purpose
- A wrong value tells you something specific — 0x00 usually means nothing
  responded, 0xFF usually means the bus is floating, a plausible-but-wrong
  value usually means a mode or endianness mismatch
- A *write*, by contrast, tells you nothing at all until you read it back

If the device ID reads correctly, the bus works, the addressing works, the
mode works, and the timing is close enough. Everything after this is
application logic.

### Phase 5 — Make it a driver

The script that read the device ID is not a driver. Turn it into one before
adding features, because retrofitting structure onto working code is much
harder than it sounds and nobody ever gets round to it.

The minimum shape:

```c
typedef enum { DEV_OK = 0, DEV_ERR_BUS, DEV_ERR_ID, DEV_ERR_TIMEOUT } dev_result_t;

dev_result_t dev_init(void);
dev_result_t dev_read_reg(uint8_t reg, uint8_t *out);
dev_result_t dev_write_reg(uint8_t reg, uint8_t val);
const char  *dev_strerror(dev_result_t r);
```

Rules that pay for themselves immediately:

- **Every function returns a status.** `void` functions on a bus that can fail
  are how errors get discarded silently.
- **No `printf` inside the driver.** Return a code; let the caller decide.
- **No global state that is not in a context struct** — the moment there are
  two of the device, a global costs you a rewrite.
- **`dev_strerror`.** Three lines of code that turn every future bug report
  from "it returned 3" into "NACK — wrong address?".

### Phase 6 — Hardening

See [§7](#7-phase-6-hardening).

### Phase 7 — Verification

Split the driver so that the part that can be tested, is:

- **Register sequencing and protocol logic** — put it behind a thin accessor
  and test it on the host against a mock register file
- **Framing, CRC, packing, scaling arithmetic** — pure functions; test them
  natively, under sanitisers, with malformed input
- **The actual register writes** — capture them on a logic analyzer once, and
  keep the capture

This repository does exactly that split in
[`code/portable/`](../code/portable/) versus
[`code/stm32f4/`](../code/stm32f4/) — the first is compiled unmodified by both
the target build and the [host tests](../code/host-tests/), the second is
compile-verified only and says so.

---

## 4. Reading a datasheet for bring-up

You do not read it front to back. You read it in this order, and you stop as
soon as you have what the current phase needs.

| Read this | When | Looking for |
| :--- | :--- | :--- |
| Block diagram | Phase 0 | What clocks it, what it connects to, how many instances |
| Clock / reset chapter | Phase 1 | The enable bit, the reset bit, any required delay |
| Pin / AF tables | Phase 2 | Which pins, which AF number, drive requirements |
| "Functional description" intro | Phase 3 | The minimum enable sequence, usually 5–10 lines of prose |
| Register map | Phase 3–4 | Only the registers that sequence names |
| Timing / electrical tables | Phase 4 | Divisor arithmetic, setup and hold, max clock |
| Interrupt / DMA sections | After phase 4 | Not before |
| Errata | Phase 0 and again at every stall | Anything mentioning your peripheral |

**The single highest-value paragraph** in any peripheral chapter is the one
that begins something like *"to configure the peripheral, proceed as
follows"*. It is usually buried between the block diagram and the register
map, it is usually short, and it is usually the exact sequence you need.
Search the PDF for "proceed as follows", "the following sequence", "must be
configured before", and "software must".

**Also search for the word "must".** Every occurrence is a constraint someone
found the hard way.

---

## 5. Phase 3 — First light

The goal is one transaction. Not a driver, not an API, not a loop — one
transaction, in `main`, with everything hostile removed.

```c
int main(void)
{
    clock_init();           /* system clock first, so timing is knowable */
    debug_uart_init();      /* a way to see anything at all             */
    puts("alive\n");        /* prove THAT works before trusting it      */

    peripheral_clock_enable();
    puts("clock on\n");

    gpio_configure_pins();
    puts("pins configured\n");

    peripheral_configure();
    printf("SR after config = %08lX\n", (unsigned long)PERIPH->SR);

    uint8_t id = 0;
    int rc = read_one_register(WHO_AM_I, &id);
    printf("rc=%d id=%02X (expected %02X)\n", rc, id, WHO_AM_I_EXPECTED);

    for (;;) { }
}
```

Things that are **deliberately absent** and must stay absent until this works:

- Interrupts. A driver that half-works and an ISR that half-works are
  indistinguishable from either one working alone.
- DMA. Same argument, plus cache coherency if you are on an M7.
- An RTOS task. A scheduler adds preemption to a system you do not yet trust.
- Retries. A retry loop hides the first failure, which is the informative one.
- Timeouts *inside* the transaction — for this one phase only. You want it to
  hang, so you can attach a debugger and read the status register at the
  moment it is stuck. Timeouts go in at phase 6, and they are mandatory then.

**Get a print path working first.** Whatever it is — UART, SWO, a single LED
blinked in a pattern, a GPIO on a scope — establish it before the peripheral,
and verify it independently. Debugging a silent board with an unverified print
path is debugging two things at once.

---

## 6. Bisecting a dead bus

When phase 3 or 4 produces nothing, resist the urge to re-read your
configuration code. Bisect the physical system instead. Each of these
questions eliminates roughly half of what is left.

**1. Is the MCU running at all?**
Toggle a GPIO in a tight loop. No toggle → clock configuration, watchdog,
brownout, or you are not actually flashing the part you think you are.

**2. Is the peripheral clocked?**
Read a register with a known non-zero reset value. All zeros → clock gate.

**3. Do the pins move?**
Scope them. Nothing moving → alternate function, or the peripheral is not
enabled, or you are scoping the wrong pin (check the schematic, not the
silkscreen).

**4. Does the signal reach the device?**
Scope at the *device* pin, not the MCU pin. A series resistor, a level shifter
or a dry joint lives in between.

**5. Does the device respond at all?**
- I²C: run an address scan across 0x08–0x77 and see what ACKs. If nothing
  ACKs, check the address you assumed — datasheets quote 7-bit and 8-bit
  addresses interchangeably and getting it wrong by a shift is the single
  most common I²C mistake.
- SPI: check that CS actually goes low. Then check the mode. A mode mismatch
  produces plausible-looking garbage, not silence.
- UART: check the baud rate at both ends, then check it with a scope by
  measuring one bit width.

**6. Is the answer being misread?**
Data on the wire but wrong in software → byte order, bit order (LSB-first vs
MSB-first), a stale RX flag never cleared, or reading the data register before
the "data ready" flag is set.

### Symptom table

| Symptom | Look here first |
| :--- | :--- |
| Every register reads 0x00000000 | Peripheral clock gated |
| Every register reads 0xFFFFFFFF | Bus fault swallowed, or reading an unmapped address |
| Reads work, writes do nothing | Write-protection / lock register, or a read-only alias |
| Works once, then never again | A flag not cleared; an error state that latches (I²C `AF`, SPI `OVR`, UART `ORE`) |
| Works with the debugger attached, fails standalone | A race the halt hides, or a `printf` that was providing the delay |
| Works at low speed, fails fast | Rise time, wait states, missing prescaler on a divided clock |
| Works cold, fails after minutes | Buffer overrun, counter wrap, or thermal |
| Data off by one byte, consistently | Stale RX flag, or a shift register read that was never cleared |
| First byte right, rest garbage | Bus speed, or an ISR taking too long and dropping bytes |

---

## 7. Phase 6 — Hardening

Everything up to phase 5 makes it work. This is what makes it survive.

**Bound every wait.** No exceptions.

```c
#define WAIT_FOR(cond, err)                   \
    do {                                      \
        uint32_t _n = TIMEOUT_LOOPS;          \
        while (!(cond)) {                     \
            if (--_n == 0u) return (err);     \
        }                                     \
    } while (0)
```

A single `while (!(SR & FLAG));` is enough to hang a product permanently.
Prefer a loop budget over a microsecond timeout for bring-up code, because it
works before the timebase exists — which is exactly when you need it.

**Add a recovery path.** For every bus, ask: *what state can the peer be in
that my reset does not clear?* On I²C the answer is well known — a target
holding SDA low needs nine manual clocks to unwedge, which the peripheral
cannot do for you (see
[`code/stm32f4/i2c_master.c`](../code/stm32f4/i2c_master.c)). On SPI it is a
device left mid-transaction by a truncated CS. On UART it is a framing error
that latches until the flag is read.

**Count everything.** Four bytes each, and they turn "the link is flaky" from
an argument into a measurement:

```c
uint32_t transfers_ok, err_timeout, err_nack, err_crc, bytes_discarded;
```

**Decide what a failure means.** Retry, degrade, or fault — but decide it
explicitly, per call site, and write down why. A silent retry loop around a
disconnected sensor produces a system that looks healthy and reports stale
data forever.

**Handle the not-present case.** If the device is optional, the driver must
return cleanly when it is absent, not hang. Test it by physically removing
the device.

---

## 8. The bring-up log

Keep one file per peripheral, in the repository, updated as you go. Not
polished — dated entries.

```
2026-03-14  LIS3DH over SPI1
  - APB2ENR SPI1EN set; CR1 reads 0x0000 after reset, as documented
  - PA5/6/7 to AF5, PA4 GPIO out for CS. Verified with scope: SCK moves.
  - WHO_AM_I returned 0x00. CS was never going low - MODER for PA4 was
    being overwritten by the AF setup line below it. Ordering bug.
  - Fixed: WHO_AM_I = 0x33. Correct.
  - Mode 3 works; mode 0 also appears to work at 1 MHz. Datasheet says
    both are supported (fig 9). Using mode 3.
  - At 10 MHz, reads corrupt. Board has long flying leads. Left at 5 MHz;
    revisit on a real PCB.
```

Three reasons this is worth the two minutes:

1. **You will bring up the same part again**, on a different project, in
   eighteen months, having forgotten all of it.
2. **The failures are the valuable part.** "WHO_AM_I returned 0x00 because CS
   never went low" is a debugging pattern you will reuse; "it works" is not.
3. **It is the raw material for the write-up.** Every peripheral document in
   this repository started as one of these.

---

## 9. Worked example: an unfamiliar sensor

You are handed a board and a part number. Nothing else.

**Phase 0.** Find the datasheet. Acceptance test: *read the device ID
register and get the documented value.* Check the schematic for which bus,
which pins, which address or CS. Check whether I²C pull-ups are actually
fitted — on a prototype they often are not.

**Phase 1.** Enable the bus peripheral clock. Read `CR1` and confirm it
matches the documented reset value. It does. Two minutes.

**Phase 2.** Configure pins. Scope SCL: nothing. The AF number was wrong —
the pin table has two entries for this pin and I took the first. Fix; SCL now
clocks. Ten minutes, and without the scope it would have been an hour.

**Phase 3.** Blocking single-byte read of the ID register in `main`. Returns
0x00. The bus is clocking, so something is not answering.

**Bisect.** Run an I²C address scan. Device ACKs at 0x19, not the 0x32 in my
code — the datasheet quoted the 8-bit address and the peripheral wants 7-bit.
Shift right by one. ID reads correctly.

**Phase 4.** Read the ID a thousand times in a loop. One thousand successes,
no errors. Now the bus is trusted.

**Phase 5.** Wrap it: `dev_init`, `dev_read_reg`, `dev_write_reg`, an error
enum, `dev_strerror`. Configure the part properly and read real samples.

**Phase 6.** Add timeouts to every wait. Add a bus-recovery call on
`ERR_BUS_BUSY`. Add counters. Unplug the sensor mid-run and confirm the
system reports a sensor fault instead of hanging.

**Phase 7.** Move the sample-decoding arithmetic — two's complement, the
scaling to mg, the axis remap — into a pure function and test it on the host
with boundary values. Capture one complete transaction on the logic analyzer
and save it.

**Log it.** Especially the AF-number mistake and the 7-bit/8-bit address
shift, because you will make both again.

---

## 10. Questions I should be able to answer

1. A peripheral's registers all read zero. What is the first thing you check,
   and why does it not fault?
2. Why get one blocking transaction working before adding interrupts, when
   the final driver will be interrupt-driven anyway?
3. Why is reading a device ID register a better first transaction than
   writing a configuration register?
4. What does 0x00 tell you that 0xFF does not, when a read comes back wrong?
5. I²C: SDA is stuck low and the peripheral reports the bus busy. Why will
   resetting the peripheral not fix it, and what will?
6. A driver works with the debugger attached and fails when running free.
   Name three causes.
7. Why is a loop-count timeout preferable to a microsecond timeout during
   bring-up?
8. You configure eight registers, it does not work. What did you do wrong
   before you wrote a single incorrect value?
9. Which parts of a driver can be unit-tested without hardware, and how would
   you restructure a driver that currently cannot be?
10. A sensor reads correctly at 1 MHz and returns garbage at 10 MHz on the
    same board. Give three candidate causes and how you would separate them.
11. What is the difference between the MCU datasheet and the reference
    manual, and which one has the register map?
12. Why does the procedure say to read the errata in phase 0 rather than when
    you get stuck?

---

## 11. Sources

- ARM, *ARMv7-M Architecture Reference Manual* (DDI 0403) — exception model,
  memory ordering, the parts that are the same on every Cortex-M
- ST, *RM0090 Reference Manual, STM32F405/415/407/417/427/437/429/439* —
  used throughout as the concrete example; the peripheral chapters follow the
  "proceed as follows" structure described in §4
- ST, *AN4989: STM32 microcontroller debug toolbox* — practical bring-up and
  debugging techniques
- NXP, *AN10216: I2C manual* — bus recovery, the nine-clock unwedge
- Any peripheral errata sheet. Genuinely: read it first.

## Related in this repository

- [`peripherals/`](../peripherals/) — per-bus reference, each with a
  symptom-first debugging checklist in §6
- [`code/stm32f4/i2c_master.c`](../code/stm32f4/i2c_master.c) — a driver
  written to the phase-6 standard: timeouts on every wait, error enum,
  `strerror`, bus recovery
- [`code/qemu-cortex-m/05-hardfault-decoder/`](../code/qemu-cortex-m/05-hardfault-decoder/) —
  when bring-up produces a fault rather than silence
