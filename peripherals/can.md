# CAN — Controller Area Network

> A two-wire differential bus where every node hears every message, nobody
> has an address, priority is decided bit by bit while transmitting, and a
> node that misbehaves eventually removes itself from the bus.

**Contents**
[Cheat sheet](#1-cheat-sheet) ·
[How it works](#2-how-it-actually-works) ·
[Arbitration](#arbitration--the-idea-that-makes-can-can) ·
[Frame format](#frame-format) ·
[Bit stuffing](#bit-stuffing) ·
[Registers](#3-register-level-walkthrough) ·
[Code](#4-code) ·
[Captures](#5-captures) ·
[Debugging](#6-debugging-checklist) ·
[Q&A](#7-questions-i-should-be-able-to-answer) ·
[Sources](#8-sources) ·
[Electricals](#9-electrical-characteristics) ·
[Bit timing](#10-bit-timing-and-the-sample-point) ·
[Error handling](#11-error-handling-and-fault-confinement) ·
[Driver architecture](#12-driver-architecture) ·
[Filters](#13-acceptance-filters-and-mailboxes) ·
[CAN FD](#14-can-fd) ·
[Higher layers](#15-higher-layers-j1939-canopen-uds) ·
[Real-time analysis](#16-real-time-response-time-analysis) ·
[Board-level failures](#17-board-level-failure-modes) ·
[CAN vs the others](#18-can-vs-i2c-spi-uart) ·
[Testing](#19-testing-a-can-driver)

---

## 1. Cheat sheet

| | |
| :--- | :--- |
| **Topology** | Linear bus, 120 Ω termination at **both** ends, short stubs |
| **Wires** | CAN_H, CAN_L — differential pair, plus a common ground reference |
| **Bit rate** | Classical CAN up to 1 Mbit/s; CAN FD data phase to 5–8 Mbit/s |
| **Addressing** | **None.** Messages carry an identifier, not a destination |
| **Identifier** | 11-bit (CAN 2.0A) or 29-bit (CAN 2.0B extended) |
| **Payload** | 0–8 bytes classical, 0–64 bytes CAN FD |
| **Arbitration** | Bitwise, non-destructive. Lowest numeric ID wins. No frame is lost |
| **Bit encoding** | NRZ with bit stuffing (5 identical bits → 1 opposite inserted) |
| **Error check** | CRC-15, bit monitoring, form check, stuff check, ACK check |
| **Confinement** | Error-active → error-passive at 128 → bus-off at 256 |
| **Standards** | ISO 11898-1 (protocol), -2 (high-speed PHY), -3 (fault-tolerant low-speed) |

**Dominant and recessive — the whole protocol rests on this:**

| Level | Logical bit | CAN_H | CAN_L | Differential |
| :--- | :--- | :--- | :--- | :--- |
| **Dominant** | **0** | ≈3.5 V | ≈1.5 V | ≈2.0 V |
| **Recessive** | **1** | ≈2.5 V | ≈2.5 V | ≈0 V |

The bus is a **wired-AND**: if any node drives dominant, the bus is dominant.
A recessive bit is not driven at all — it is the termination resistors pulling
the pair together. Every clever thing CAN does falls out of that one property.

**Five things that catch people out:**

1. **Lower ID = higher priority.** ID 0x001 beats 0x7FF. The winner does not
   even know arbitration happened.
2. **A single node alone on a bus cannot transmit successfully.** No other
   node means no ACK, so it retries forever and eventually goes bus-off. Use
   loopback mode, or put a second node on the bus.
3. **Termination is 120 Ω at each *end*, not at each node.** Two resistors
   total, giving 60 Ω across the pair. One resistor, three resistors, or one
   at the middle all produce reflections.
4. **The sample point must be late in the bit** — typically 87.5%. It has to
   be, because during arbitration a node must see the other end's dominant
   bit within the same bit time.
5. **A node with the wrong bit rate destroys the bus for everyone**, because
   it error-frames every message it fails to decode. This looks like "the bus
   is broken" rather than "one node is misconfigured".

---

## 2. How it actually works

CAN was designed for a car in 1986, and every design decision follows from
that: a shared pair of wires, no master, no addressing, no negotiation, and
graceful degradation when a node fails. It is a **broadcast** bus. A node
transmits a message with an identifier; every other node receives it and
decides for itself whether it cares.

That inversion is the hardest thing to unlearn coming from I²C or SPI. An ID
is not "who this is for". It is closer to a topic name — `0x0C1` might mean
"engine RPM", and whoever needs engine RPM listens for `0x0C1`. Two nodes
transmitting the same ID with different content is a system design fault, not
a protocol event.

### Arbitration — the idea that makes CAN CAN

Every node may start transmitting when the bus is idle. While transmitting the
identifier, each node **monitors the bus** and compares what it reads back
against what it sent:

- Sent recessive, read recessive → still winning, carry on.
- Sent dominant, read dominant → still winning, carry on.
- **Sent recessive, read dominant → lost arbitration.** Stop transmitting
  immediately, switch to receiving, retry when the bus is next idle.

Because dominant wins on a wired-AND bus, the node whose identifier has the
first dominant bit where others have recessive takes the bus. A dominant bit
is a 0, so **the numerically lowest identifier wins**.

```
       SOF   10  9   8   7   6   5   4   3   2   1   0   RTR
Node A  0    0   0   0   1   1   0   1  ...                      ID 0x0D
Node B  0    0   0   0   1   0   ← loses here, goes quiet        ID 0x0B... no:
Node C  0    0   0   1   ← loses here                            ID 0x1xx
bus     0    0   0   0   1   0   ...
                             ^
                    Node A sent recessive, read dominant -> A loses to B
```

Two properties fall out, and both are why CAN survived into safety-critical
systems:

- **Non-destructive.** The winning frame is not corrupted and not retried. No
  collision, no backoff, no wasted bandwidth. Ethernet's CSMA/CD throws away
  both frames; CAN throws away neither.
- **Deterministic priority.** The highest-priority message pending anywhere on
  the bus always goes next. That makes worst-case latency *analysable* — see
  [§16](#16-real-time-response-time-analysis).

The cost is the constraint that shapes everything else: **a node must see the
whole bus settle within one bit time**, because it compares its own bit
against the bus while sending it. That is why the sample point is late, why
bit rate trades against bus length, and why 1 Mbit/s CAN cannot run 500 m.

### Frame format

A standard (11-bit) data frame:

```
 ┌───┬──────────────┬───┬───┬───┬──────┬─────────────┬────────┬───┬───┬───────┐
 │SOF│ Identifier   │RTR│IDE│ r0│ DLC  │  Data 0-8B  │ CRC-15 │DEL│ACK│  EOF  │
 │ 1 │     11       │ 1 │ 1 │ 1 │  4   │   0-64 bits │   15   │ 1 │2  │   7   │
 └───┴──────────────┴───┴───┴───┴──────┴─────────────┴────────┴───┴───┴───────┘
   └──── arbitration field ────┘└─ control ─┘                    └ACK slot+del┘
```

| Field | Meaning |
| :--- | :--- |
| **SOF** | Start of frame, one dominant bit. Synchronises every receiver |
| **Identifier** | 11 bits, transmitted MSB first — which is what makes bitwise arbitration work |
| **RTR** | Remote transmission request. Dominant = data frame. A remote frame asks another node to transmit; largely deprecated, and forbidden in CAN FD |
| **IDE** | Identifier extension. Dominant = 11-bit, recessive = 29-bit. A recessive IDE loses arbitration to a dominant one, so **standard IDs always beat extended IDs** with the same first 11 bits |
| **DLC** | Data length code, 4 bits. Values 9–15 all mean 8 bytes in classical CAN |
| **CRC-15** | Polynomial `0x4599`, over everything from SOF to the end of data |
| **ACK slot** | The transmitter sends **recessive**; *any* node that received the frame without error drives it **dominant**. The transmitter reads it back to know somebody heard |
| **EOF** | 7 recessive bits, then 3 bits of inter-frame space |

The ACK mechanism is worth dwelling on. It is not an acknowledgement from the
*intended* recipient — CAN has no intended recipient. It means "at least one
node on this bus received a structurally valid frame". A node alone on a bus
never sees its ACK slot pulled dominant, treats that as an error, and retries.
That is bring-up trap number one.

Extended (29-bit) frames insert SRR and IDE after the 11-bit base ID, then 18
more identifier bits, then RTR. Same rules, longer arbitration field, slightly
worse bus efficiency.

### Bit stuffing

CAN is NRZ-coded with no separate clock. A long run of identical bits gives
receivers nothing to resynchronise on, so the transmitter inserts a bit of
opposite polarity after **five consecutive identical bits**. Receivers remove
them. Stuffing applies from SOF through the CRC sequence — not to the CRC
delimiter, ACK field or EOF, which is what makes an error frame (six identical
bits) detectable as a deliberate stuff-rule violation.

Two consequences that matter in practice:

- **Frame length is data-dependent.** A worst-case 8-byte standard frame is
  about 132 bits including stuff bits, not 111. Bandwidth calculations that
  ignore stuffing are optimistic by up to 20%, and worst-case latency analysis
  that ignores it is simply wrong.
- **Identifier choice affects timing.** An ID with long runs of identical bits
  attracts more stuff bits. It is a small effect, but on a heavily loaded
  J1939 bus it is measurable.

---

## 3. Register-level walkthrough

Concretely, ST bxCAN — the structure is the same on almost every controller
even where the names differ.

**1. Enter initialisation mode.** The controller must be out of normal
operation before its bit timing can be written.

```c
CAN1->MCR |= CAN_MCR_INRQ;                    /* request init            */
while (!(CAN1->MSR & CAN_MSR_INAK)) { }       /* wait for acknowledge    */
```

Do this with a timeout. If `INAK` never sets, the peripheral clock is off —
see [`patterns/driver-bring-up.md`](../patterns/driver-bring-up.md) phase 1.

**2. Configure bit timing.** One register, and the single most important write
in the whole driver:

```c
CAN1->BTR = ((sjw   - 1u) << 24)
          | ((tseg2 - 1u) << 20)
          | ((tseg1 - 1u) << 16)
          |  (brp   - 1u);
```

Note every field is **encoded minus one**. Writing the raw TQ counts is a
classic off-by-one that yields a bit rate slightly wrong in a way that works
on a two-node bench and fails on a loaded bus. Compute the values with
[`code/portable/can/can_bittiming.c`](../code/portable/can/can_bittiming.c)
rather than copying a table from a forum post.

**3. Set mode bits.**

```c
CAN1->MCR |= CAN_MCR_ABOM;    /* automatic bus-off recovery - see §11 */
CAN1->MCR &= ~CAN_MCR_NART;   /* automatic retransmission ON          */
```

`ABOM` is a real design decision, not a default to accept blindly. See
[§11](#11-error-handling-and-fault-confinement).

**4. Configure acceptance filters** *before* leaving init mode — see
[§13](#13-acceptance-filters-and-mailboxes).

**5. Leave initialisation mode and wait for synchronisation.**

```c
CAN1->MCR &= ~CAN_MCR_INRQ;
while (CAN1->MSR & CAN_MSR_INAK) { }
```

The controller now waits for 11 consecutive recessive bits before it considers
itself synchronised to the bus. On a busy bus that is immediate; on an idle
one it is also immediate; on a bus being hammered by a misconfigured node it
may never happen, which is itself a useful diagnostic.

**Transmit** is: find a free mailbox in `TSR`, write ID, DLC and data, set
`TXRQ`. **Receive** is: an interrupt fires, read the FIFO's ID/DLC/data, then
release the FIFO — forgetting the release is why "I only ever get three
messages".

---

## 4. Code

Runnable and tested, in this repository:

| File | What it is | Verified how |
| :--- | :--- | :--- |
| [`code/portable/can/can_bittiming.c`](../code/portable/can/can_bittiming.c) | Bit-timing solver: exact bit rate, sample point placement, SJW selection, maximum bus length | **8 host tests, 502 assertions**, ASan + UBSan. Also compiles clean for Cortex-M4 |
| [`code/portable/can/isotp.c`](../code/portable/can/isotp.c) | ISO 15765-2 transport — the layer UDS diagnostics ride on. Segmentation, flow control, block size, sequence numbers, malformed-input rejection | **18 host tests, 1.2M assertions** including a 300k-frame fuzz pass |

```bash
cd code/host-tests && make        # runs both, ~3 seconds
```

**Why these two and not a register driver.** Bit timing and transport
segmentation are pure arithmetic and pure state machines — they are wrong in
ways a compiler cannot catch and a bench test will not reveal, and they are
exactly the parts that *can* be tested exhaustively on a laptop. The register
poking is thin by comparison, and it needs a transceiver and a second node to
mean anything.

What the bit-timing solver produces:

```
f_can      bitrate     BRP  TSEG1  TSEG2  SJW    TQ   sample   bus_m
16000000   125000        8     13      2    2    16    87.5%     550
36000000   500000        9      6      1    1     8    87.5%      75
48000000   500000        6     13      2    2    16    87.5%     100
80000000   1000000       4     14      5    4    20    75.0%      15
```

It refuses to return an approximate bit rate. `10 MHz @ 300 kbit/s` gives no
solution, because no integer prescaler produces it — and that is the correct
answer, not a defect. It is also why a CAN node runs from a crystal rather
than an internal RC oscillator.

> [!NOTE]
> **Still outstanding: a register-level driver on real silicon.** No QEMU
> machine emulates a CAN controller and transceiver faithfully enough to be
> worth pretending, so `bxCAN` register sequencing, error-counter behaviour,
> bus-off recovery timing and everything analogue are unverified here. That
> needs two nodes, two transceivers and a scope.

---

## 5. Captures

**Not yet taken** — needs a two-node bench and a logic analyzer with CAN
decode, plus a scope for the analogue items.

- [ ] **A complete standard data frame**, differential, with the fields
      annotated — SOF through EOF
- [ ] **Arbitration in progress** — two nodes starting simultaneously, the
      loser going recessive mid-identifier
- [ ] **CAN_H and CAN_L separately** at a dominant bit, confirming ≈3.5 V and
      ≈1.5 V and a ≈2 V differential
- [ ] **Bit stuffing** — a payload of `0x00` bytes, showing the inserted
      recessive bits
- [ ] **The ACK slot pulled dominant** by another node, and then the same
      frame with the second node powered down — the retry storm
- [ ] **An error frame** — six dominant bits violating the stuff rule
- [ ] **Missing termination** — the same frame with 2×120 Ω, 1×120 Ω and none,
      showing the reflections grow
- [ ] **A node at the wrong bit rate** joining the bus and error-framing
      everything

---

## 6. Debugging checklist

Symptom-first, so it is usable at 1am.

### Nothing transmits; TX error counter climbs to 128 then bus-off

- **Is there a second node?** No ACK means no successful transmission, ever.
  Put another node on the bus, or use loopback / silent-loopback mode.
- **Termination.** Measure resistance across CAN_H–CAN_L with everything
  powered off: **60 Ω is correct.** 120 Ω means one terminator is missing;
  40 Ω means three are fitted; open circuit means none.
- **Bit rate mismatch** between the two nodes. Both must agree exactly.
- **CAN_H and CAN_L swapped.** The bus will not work and nothing will be
  damaged; it is a two-minute check.
- **Transceiver standby pin.** Most transceivers have an `S`/`STB`/`RS` pin
  that must be pulled to the correct rail. Left floating, the transceiver sits
  in a low-power mode and the controller sees a permanently recessive bus.

### Bus works, then dies after a few seconds

- A node going **bus-off** and auto-recovering in a loop. Read the error
  counters — that is what they are for.
- **Ground offset** between nodes powered from different supplies. CAN is
  differential but not ground-independent; the common-mode range is roughly
  −2 V to +7 V and exceeding it corrupts everything.
- **Transmit buffer overrun** — a periodic task queueing faster than the bus
  drains, eventually filling every mailbox.

### Receives some IDs, not others

- **Acceptance filters.** The default filter configuration on many parts
  rejects *everything*; a filter that appears to work often matches only by
  accident. Set an accept-all filter first, confirm traffic, then narrow it.
- **FIFO not released** after reading, so only the first few messages arrive.
- **Filter register format.** bxCAN's `FxR1`/`FxR2` are shifted left by 5 (or
  3 for extended). Writing the raw ID silently matches nothing.

### Occasional errors on a bus that mostly works

- **Sample point too early.** Works on a short harness, fails on a long one.
  Recompute it — see [§10](#10-bit-timing-and-the-sample-point).
- **Stub length.** Drops from the main bus should be under about 30 cm at
  500 kbit/s. A long stub is a transmission-line stub and it reflects.
- **A node with slightly wrong oscillator tolerance.** CAN tolerates roughly
  ±0.5% with a well-placed sample point, and much less with a bad one.
- **Electrically noisy neighbour** — an unsuppressed relay or motor coupling
  into the loom.

### Everything on the bus breaks when one node is powered on

Almost always that node is misconfigured — wrong bit rate, wrong sample point
— and is error-framing valid traffic. Unplug it and the bus recovers, which is
the diagnostic. This is CAN's one genuinely nasty failure mode: a single bad
node degrades the whole bus rather than only itself.

---

## 7. Questions I should be able to answer

1. Why does the numerically lowest identifier win arbitration, and why is the
   arbitration non-destructive?
2. Why must the identifier be transmitted MSB first for arbitration to work?
3. A node is alone on a CAN bus and transmits. Trace exactly what happens to
   its transmit error counter and why.
4. What does the ACK bit actually tell the transmitter? What does it *not*
   tell it?
5. Why must the sample point be late in the bit time, and what physically
   limits how late it can be?
6. Derive the relationship between bit rate and maximum bus length.
7. What is bit stuffing for, and how does an error frame exploit the stuff
   rule to be unambiguously detectable?
8. Explain error-active, error-passive and bus-off. What are the counter
   thresholds and what changes behaviourally at each?
9. Why is automatic bus-off recovery (`ABOM`) not always the right choice?
10. Two nodes transmit the same identifier at the same instant with different
    data. What happens, and why is this a system design fault?
11. Why does a standard-ID frame win arbitration against an extended-ID frame
    sharing its first 11 bits?
12. How does CAN FD's bit-rate switching work, and why is arbitration still
    done at the slow rate?
13. Why does ISO-TP need flow control at all, given that CAN already has
    error detection and retransmission?
14. Given a set of periodic messages with known IDs and periods, how would you
    compute the worst-case response time of the lowest-priority one?
15. Why is 60 Ω the resistance you should measure across a healthy bus?
16. A CAN bus works at 125 kbit/s and fails at 500 kbit/s on the same harness.
    Give three candidate causes.

---

## 8. Sources

- **ISO 11898-1:2015** — data link layer and physical signalling. The
  normative protocol definition, including CAN FD.
- **ISO 11898-2:2016** — high-speed medium access unit. Termination,
  common-mode range, transceiver characteristics.
- **ISO 15765-2:2016** — ISO-TP. The transport layer implemented in
  [`code/portable/can/isotp.c`](../code/portable/can/isotp.c).
- **ISO 14229-1** — UDS, the diagnostic services that ride on ISO-TP.
- **Bosch CAN Specification 2.0** (1991) — the original, still the clearest
  statement of arbitration and fault confinement.
- **CiA 301** — CANopen application layer. Also the source of the 87.5%
  sample-point recommendation.
- **SAE J1939** — the heavy-vehicle higher layer: 29-bit IDs, PGNs, address
  claiming.
- **ST RM0090 §32** — bxCAN, used as the concrete controller throughout.
- **NXP AN11238 / TJA1042 datasheet** — transceiver behaviour, loop delay
  figures for the bus-length calculation.

---

## 9. Electrical characteristics

| Parameter | ISO 11898-2 high speed |
| :--- | :--- |
| Differential dominant | 1.5 V min, 2.0 V typical, 3.0 V max |
| Differential recessive | −0.5 V to +0.05 V |
| Common-mode range | −2 V to +7 V (transceiver dependent; some to ±12 V) |
| Termination | 120 Ω ±10% at each bus end |
| Characteristic impedance | 120 Ω twisted pair |
| Bus capacitance | ≤ ~30 pF per node |
| Transceiver loop delay | 120–255 ns typical — matters for bus length |

**Why 120 Ω twice and not once.** The pair is a transmission line. An
unterminated end reflects the edge back down the bus, and the reflection
arrives at other nodes as a false level near the sample point. Terminating
both ends absorbs the energy at both. A resistor in the *middle* of the bus
terminates neither end and is the worst of all worlds.

**Why 60 Ω is the number you measure.** Two 120 Ω resistors in parallel. This
is the single most useful CAN measurement: power everything down, put a meter
across CAN_H and CAN_L, and read it.

**Why a ground reference is still needed.** Differential signalling rejects
common-mode noise *within the receiver's common-mode range*. Two nodes on
separate supplies with a few volts of ground offset push the pair outside
that range and the receiver stops resolving dominant from recessive. Long
booms, trailers and separately-grounded machines are where this bites.

**Low-speed fault-tolerant CAN (ISO 11898-3)** is a different animal: up to
125 kbit/s, terminated per node rather than per bus end, and able to keep
running single-wire when CAN_H or CAN_L is open or shorted. Body electronics
uses it. Do not mix the two on one segment.

---

## 10. Bit timing and the sample point

A CAN bit is divided into an integer number of **time quanta**:

```
|<------------------ one nominal bit time ------------------>|
| SYNC_SEG |  PROP_SEG  |  PHASE_SEG1  |      PHASE_SEG2      |
|   1 TQ   |<-------- TSEG1 ---------->|<----- TSEG2 -------->|
                                       ^
                                  sample point
```

```
TQ           = BRP / f_can
bit time     = (1 + TSEG1 + TSEG2) × TQ
bit rate     = f_can / (BRP × (1 + TSEG1 + TSEG2))
sample point = (1 + TSEG1) / (1 + TSEG1 + TSEG2)
```

**SYNC_SEG** is always exactly 1 TQ, and is where a recessive-to-dominant edge
is expected to fall.

**PROP_SEG** must cover the round-trip propagation delay: signal out to the
furthest node, that node's dominant bit back again, plus two transceiver loop
delays. This is the arbitration constraint made physical.

**PHASE_SEG1 and PHASE_SEG2** absorb phase error. On resynchronisation,
PHASE_SEG1 is lengthened or PHASE_SEG2 shortened by up to **SJW** time quanta.
SJW must never exceed PHASE_SEG2, or a resynchronisation can shorten the bit
below zero.

**Why 87.5%.** Late enough to satisfy the propagation constraint on a
realistic bus, early enough to leave PHASE_SEG2 room to shrink for
resynchronisation. CiA recommends it for CANopen and J1939; AUTOSAR stacks
generally default to it. 75% is the convention at 1 Mbit/s, where the bus is
short and resynchronisation headroom matters more than propagation budget.

**Bit rate against bus length** — the physical trade-off, from
`can_max_bus_length_m()` with a 250 ns transceiver:

| Bit rate | Max bus length (conservative) | Rule of thumb usually quoted |
| ---: | ---: | ---: |
| 125 kbit/s | ~550 m | 500 m |
| 250 kbit/s | ~250 m | 250 m |
| 500 kbit/s | ~100 m | 100 m |
| 1 Mbit/s | ~15 m | 40 m |

The 1 Mbit/s row is where the calculation and the folklore diverge, and the
difference is instructive: the 40 m figure assumes a fast transceiver and no
margin, while the function reserves 1 TQ of PHASE_SEG1 for phase error. A
150 ns transceiver roughly triples the number. **Use your part's actual loop
delay**, not a table.

Compute all of this rather than copying it:
[`code/portable/can/can_bittiming.c`](../code/portable/can/can_bittiming.c).

---

## 11. Error handling and fault confinement

CAN's most under-appreciated feature is that it removes broken nodes from the
bus automatically. Five error checks run continuously:

| Check | Detects |
| :--- | :--- |
| **Bit monitoring** | Transmitted level ≠ level read back (outside arbitration and the ACK slot) |
| **Bit stuffing** | Six consecutive identical bits where stuffing applies |
| **CRC** | Corrupted frame contents |
| **Form** | A fixed-format field holding the wrong value |
| **ACK** | Nobody pulled the ACK slot dominant |

On detecting an error a node transmits an **error frame** — six consecutive
dominant bits, a deliberate stuff-rule violation that every other node sees
and reacts to. The frame is destroyed for everyone, and the transmitter
retries. This is what makes a single misconfigured node so destructive.

### The error counters

Every node maintains a **transmit error counter (TEC)** and a **receive error
counter (REC)**. Roughly: +8 on a transmit error, +1 on a receive error, −1 on
each success. The asymmetry is deliberate — a node that keeps failing to
*transmit* is far more likely to be the faulty one than a node that fails to
receive.

| State | Condition | Behaviour |
| :--- | :--- | :--- |
| **Error active** | TEC and REC < 128 | Normal. Signals errors with **dominant** error flags, which disturb the bus |
| **Error passive** | either ≥ 128 | Still participates, but signals with **recessive** error flags, which do not disturb others. Also waits an extra 8 bits before transmitting again |
| **Bus off** | TEC ≥ 256 | Disconnected. Transmits nothing, ACKs nothing |

The escalation is the point: a degrading node first stops being able to spoil
other people's frames, then stops participating entirely. The bus survives.

Recovery from bus-off requires **128 occurrences of 11 consecutive recessive
bits** — that is, a demonstrably idle, healthy bus.

### `ABOM` is a design decision

Automatic bus-off recovery makes the controller re-enter the bus as soon as
that condition is met.

- **Automatic recovery** is right where availability matters more than
  diagnosis and the fault is likely transient — a connector glitch during
  vibration.
- **Manual recovery** is right where you want the fault *recorded*. Auto-
  recovery hides a node that goes bus-off every ten seconds behind a bus that
  "mostly works", and that is a genuinely miserable field fault to chase.

Either way: **count bus-off events and expose the count.** A node that has
gone bus-off 4,000 times since power-on is telling you something no
intermittent bench test ever will.

---

## 12. Driver architecture

```
   application  ──────────────────────────────────────────
                 signals, engineering units, timeouts
        │
   higher layer ──────────────────────────────────────────
                 J1939 / CANopen / UDS over ISO-TP
        │                                    (code/portable/can/isotp.c)
   transport   ───────────────────────────────────────────
                 segmentation, flow control
        │
   driver      ───────────────────────────────────────────
                 mailboxes, FIFOs, filters, error counters
        │
   controller  ───────────────────────────────────────────
                 bit timing, arbitration, CRC, confinement
        │
   transceiver ───────────────────────────────────────────
                 differential PHY, dominant/recessive
```

Design rules that survive contact with a real vehicle:

**Never block in the transmit path.** A CAN transmit can be delayed
indefinitely by higher-priority traffic. Queue the message and return; let the
mailbox-empty interrupt drain the queue.

**Priority-ordered transmit queue.** Most controllers have three mailboxes.
With more than three pending messages, a FIFO queue *inverts priority* — a
low-priority message occupies a mailbox while a high-priority one waits in
software. Order the software queue by identifier, or the determinism you chose
CAN for is gone.

**Receive into a ring buffer from the ISR, parse in a task.** The ISR reads
the frame and releases the FIFO. Nothing else. Parsing, unit conversion and
application logic run outside it.
([`code/portable/ringbuf/`](../code/portable/ringbuf/) is exactly this queue.)

**Expose the error counters.** `TEC`, `REC`, last error code, bus-off count,
FIFO overrun count. Six values, and they turn "the CAN is playing up" into a
diagnosis.

**Separate the transport from the driver.** ISO-TP has no business knowing
about mailboxes, which is why the implementation here is a pure state machine
and is testable on a laptop.

---

## 13. Acceptance filters and mailboxes

Every node sees every frame. On a busy J1939 bus that is thousands of frames a
second, and waking the CPU for each one is not viable. Acceptance filters do
the rejection in hardware.

**Mask mode** — match selected bits:

```
filter accepts frame  ⟺  (received_id & mask) == (filter_id & mask)
```

A mask bit of 1 means "this bit must match"; 0 means "don't care". Mask
`0x7FF` accepts exactly one ID; mask `0x000` accepts everything.

**List mode** — a small set of exact identifiers. Better when you want four
unrelated IDs and a mask would have to let in a hundred.

Three practical points:

1. **Start with accept-everything**, confirm traffic arrives, *then* narrow.
   A filter that silently matches nothing is indistinguishable from a dead bus,
   and it is the more likely of the two.
2. **Watch the register format.** bxCAN's filter registers hold the ID shifted
   left by 5 for standard frames (3 for extended, plus the IDE and RTR bits).
   Writing the unshifted ID matches nothing at all.
3. **Design identifiers so masks are useful.** If all your engine messages
   share a high-bit prefix, one mask filter catches the lot. Identifier
   allocation is a system design decision, and doing it well makes the
   filtering fall out for free.

---

## 14. CAN FD

CAN FD (ISO 11898-1:2015) keeps arbitration exactly as it is and changes what
happens afterwards.

| | Classical CAN | CAN FD |
| :--- | :--- | :--- |
| Payload | 0–8 bytes | 0–8, 12, 16, 20, 24, 32, 48, 64 |
| Data-phase rate | same as arbitration | switched, typically 2–8 Mbit/s |
| CRC | 15-bit | 17-bit (≤16 B) or 21-bit |
| Stuff-bit counting | — | explicit stuff count, CRC-protected |
| RTR | yes | replaced by RRS, always dominant |

**Why arbitration stays slow.** Arbitration requires a node to see the whole
bus within one bit time. That is a physical constraint set by the length of
the wire, and no amount of protocol cleverness removes it. So CAN FD arbitrates
at 500 kbit/s, and once exactly one transmitter remains — nobody else is
driving the bus, so nothing has to be compared — it sets **BRS** and switches
to the fast rate for the data and CRC. It switches back before the ACK slot,
because ACK needs everyone again.

That is a genuinely elegant piece of engineering: the fast phase is exactly
the interval during which the bus is a point-to-point link.

**Practical consequences:**

- A classical CAN controller on the same bus **cannot** tolerate FD frames —
  it sees a protocol violation and error-frames them, destroying the bus.
  Mixed segments need FD-tolerant classical controllers, or a gateway.
- Two bit-timing configurations to get right, not one, and the fast one has
  much less margin. The data-phase sample point is where FD bring-ups fail.
- Transceiver ringing during the fast phase becomes a real constraint, which
  is why FD transceivers specify symmetry parameters that classical ones do
  not.
- 64-byte payloads make ISO-TP far more efficient: an SF carries up to 62
  bytes instead of 7, and the escape-length encoding changes accordingly.

---

## 15. Higher layers: J1939, CANopen, UDS

CAN itself defines no message meanings. Everything above is convention.

**SAE J1939** — heavy vehicles, agriculture, marine. 29-bit identifiers
structured into priority, PGN (parameter group number) and source address.
250 kbit/s classically, 500 kbit/s increasingly. Includes address claiming, so
nodes negotiate source addresses at startup. Transport for >8 bytes is
J1939/21 (BAM and CMDT), *not* ISO-TP — a distinction that catches people who
learned diagnostics first.

**CANopen** — industrial automation. Object dictionary, PDOs for real-time
process data, SDOs for configuration, NMT for state control, and a predefined
connection set mapping node IDs onto identifiers. CiA 301 is the core.

**UDS (ISO 14229) over ISO-TP** — diagnostics, and the one most relevant to
firmware work. Every diagnostic session, DTC read, and flash reprogramming
routine is UDS requests and responses carried by ISO-TP. Typically request on
`0x7E0` and response on `0x7E8` for a single ECU, or `0x7DF` for functional
broadcast.

A minimal UDS exchange, end to end:

```
Tester -> ECU   0x7E0   02 10 03 CC CC CC CC CC      DiagnosticSessionControl
                        │  │  └── sub-function: extended session
                        │  └───── SID 0x10
                        └──────── ISO-TP SF, 2 bytes

ECU -> Tester   0x7E8   06 50 03 00 32 01 F4 CC      positive response
                           └── SID + 0x40
```

That `02` and `06` are ISO-TP PCI bytes — the transport in
[`code/portable/can/isotp.c`](../code/portable/can/isotp.c) is what produces
and consumes them. A "read DTCs" request that returns fifty fault codes is a
multi-frame message, and then flow control, block size and STmin all matter.

**AUTOSAR** wraps this stack as CanIf, CanTp, Dcm and Com. The concepts are
the ones above; the configuration tooling is the learning curve.

---

## 16. Real-time response time analysis

The reason CAN persists in safety-critical systems is that worst-case latency
is *computable*. For a message *m*:

```
R_m  =  w_m  +  C_m
```

where `C_m` is its own transmission time and `w_m` is queueing delay:

```
w_m  =  B_m  +  Σ  ⌈ (w_m + τ_bit) / T_j ⌉ × C_j
             j∈hp(m)
```

- **`B_m`** — blocking by a lower-priority message already transmitting. CAN
  frames are not preemptible once started, so this is the longest frame on the
  bus, about 135 bit times for a stuffed 8-byte standard frame.
- **`hp(m)`** — every message with a *lower identifier*, each of which can
  preempt *m* once per its period `T_j`.
- Solve by fixed-point iteration; if it converges below the deadline, the
  message meets it.

Two things this makes obvious that intuition does not:

- **Identifier allocation is real-time design**, not naming. Assigning
  identifiers alphabetically, or by subsystem, or in the order features were
  added, is assigning deadlines by accident.
- **The lowest-priority message on a heavily loaded bus may never get
  through.** Not "may be slow" — may be starved indefinitely, because a
  higher-priority message is always pending. Bus load above about 30–40% is
  where this starts to matter for the tail.

This analysis is standard in automotive design (Tindell, Burns and Wellings,
1995, and its later corrections). Being able to sketch it is a strong signal
in an automotive interview.

---

## 17. Board-level failure modes

| Symptom | Cause |
| :--- | :--- |
| Measured 120 Ω across the bus | One terminator missing |
| Measured 40 Ω | Three terminators — commonly a dev board with one fitted, plugged into an already-terminated bus |
| Works on the bench, fails in the vehicle | Stub too long, or ground offset between separately-supplied nodes |
| Intermittent errors under vibration | Connector or crimp. CAN's error detection makes these *visible* rather than silent, which is a feature |
| One node's presence breaks the bus | That node is misconfigured and error-framing valid traffic |
| Bus dominant permanently | A transceiver failed with TXD stuck low, or a short between CAN_L and ground. Some transceivers have a TXD-dominant timeout for exactly this |
| Everything recessive, no traffic | Transceiver standby pin floating, or no supply to the transceiver |
| Errors only at high bus load | Sample point, oscillator tolerance, or reflections that only matter with back-to-back frames |

**Termination on development boards** is the single most common bench mistake:
most CAN dev boards fit a 120 Ω resistor by default, so two of them wired
together are correctly terminated — and adding a third node, or plugging into
a vehicle bus that is already terminated, is not.

---

## 18. CAN vs I2C, SPI, UART

| | CAN | I²C | SPI | UART |
| :--- | :--- | :--- | :--- | :--- |
| Wires | 2 + gnd | 2 | 4+ | 2 |
| Topology | Multi-drop bus | Multi-drop bus | Star, one CS per device | Point to point |
| Addressing | None — message IDs | 7/10-bit device address | Chip select | None |
| Multi-master | Yes, arbitrated | Yes, arbitrated | No | No |
| Arbitration cost | **Non-destructive** | Non-destructive | n/a | n/a |
| Max distance | ~40 m at 1 Mbit/s, 500 m at 125 k | ~1 m | ~0.3 m | ~15 m (RS-232), 1200 m (RS-485) |
| Error detection | CRC-15, 5 checks, fault confinement | ACK only | **None** | Parity, framing |
| Determinism | Analysable worst case | No | No | No |
| Complexity | High — needs a controller | Low | Very low | Very low |

The honest summary: **CAN costs more silicon and more design effort than the
other three combined, and buys error detection, fault confinement, multi-drop
over tens of metres in a hostile electrical environment, and provable
worst-case latency.** Inside one PCB that is a bad trade, which is why nobody
puts a temperature sensor on CAN. Across a vehicle it is the only one of the
four that works at all.

Closest relative is RS-485 ([`uart.md` §15](uart.md#15-rs-485)): the same
differential multi-drop physical idea, with none of the arbitration, error
confinement or determinism.

---

## 19. Testing a CAN driver

**Without hardware** — more than people assume:

- **Bit timing.** Pure arithmetic. Test that the bit rate is exact, the sample
  point lands where you asked, SJW never exceeds PHASE_SEG2, and impossible
  requests are rejected rather than approximated.
  ([8 tests here.](../code/host-tests/test_can_bittiming.c))
- **Transport segmentation.** A pure state machine. Test every payload length,
  the sequence-number wrap past 15, block-size handling, and every malformed
  input you can construct.
  ([18 tests here](../code/host-tests/test_isotp.c), including a 300k-frame
  fuzz pass under AddressSanitizer.)
- **Application decoding.** Signal packing, scaling, endianness, sign
  extension — all pure functions. A DBC-derived decoder should be tested
  against known frames on the host, not on a bench.

**Needs one node:** loopback mode. Most controllers offer loopback (internal)
and silent-loopback (internal, TX pin held recessive). Enough to prove filter
configuration, mailbox handling and ISR wiring without a transceiver.

**Needs two nodes:** ACK behaviour, arbitration, error counters, bus-off and
recovery, filters against real traffic. A USB-CAN adapter and `can-utils`
under Linux is the cheapest possible second node — `candump`, `cansend`, and
`cangen` for load testing.

**Needs a scope:** everything analogue. Differential levels, reflections from
termination faults, stub effects, transceiver loop delay, common-mode offset.

**A test worth building deliberately:** a **bus-off injector**. Deliberately
drive one node to bus-off — a shorted bus, or a node at the wrong bit rate —
and confirm the rest of the system degrades the way you designed it to.
Bus-off handling is written once, never exercised, and then relied upon in the
field. That is precisely the code that should be tested on purpose.

---

## Related in this repository

- [`code/portable/can/`](../code/portable/can/) — the bit-timing solver and
  ISO-TP stack, with their tests
- [`patterns/driver-bring-up.md`](../patterns/driver-bring-up.md) — the phased
  procedure; CAN's "needs a second node" trap is phase 3 in its purest form
- [`patterns/state-machines.md`](../patterns/state-machines.md) — ISO-TP is a
  worked example of the run-to-completion pattern
- [`peripherals/uart.md` §15](uart.md#15-rs-485) — RS-485, CAN's nearest
  relative
- [`architecture/interrupts-and-nvic.md`](../architecture/interrupts-and-nvic.md)
  — the receive ISR discipline this document assumes
