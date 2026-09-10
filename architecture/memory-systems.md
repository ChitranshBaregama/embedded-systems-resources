# CONTINUATION — EMBEDDED MEMORY SYSTEMS
## PARTS XXXVIII – LX

DRAM → MEMORY CONTROLLER → EXTERNAL MEMORY → MULTICORE COHERENCY → ATOMICS →
MEMORY MODEL → COMPILER SEMANTICS → ECC & SAFETY → POWER/RETENTION →
BOOT MEMORY → ARMv8 TRANSLATION → IOMMU → TRUSTZONE → SIDE CHANNELS →
MEASUREMENT → ALLOCATORS → MULTICORE IPC → ANTI-PATTERNS → FIELD MANUAL

Append this directly below PART XXXVII. Same rules, same tag vocabulary, same
[PRINCIPLE] / [VARIES] / [ARM-SPECIFIC] / [VENDOR-SPECIFIC] / [OS-SPECIFIC] /
[INFERENCE] / [COMMON MISTAKE] / [DEBUGGING] / [DESIGN RULE] / [SECURITY] /
[PERFORMANCE] discipline.

---

## DELTA RATIONALE — WHAT IS NEW AND WHY

Parts I–XXXVII build a **single-core, SRAM-centric, CPU-vs-DMA** model of memory.
That model is complete for a Cortex-M with internal SRAM and a DMA engine.
It breaks the moment the system has any of the following:

- volatile memory that is **not** SRAM (DRAM has state, timing and refresh)
- more than one **coherent** master (core-to-core, not just CPU-to-DMA)
- a **compiler** aggressive enough to break the hardware model you built
- a **safety** or **security** requirement on the memory itself
- a **boot sequence** in which the memory does not yet exist
- **measurement** rather than reasoning as the source of truth

Each part below exists only because it is absent above. Explicit overlap map:

| Already covered (I–XXXVII)      | Deliberately NOT repeated                  | New part adds                                     |
|---------------------------------|--------------------------------------------|---------------------------------------------------|
| II–IV SRAM cell → MCU SRAM      | SRAM cell physics, 6T, arrays              | XXXVIII: 1T1C DRAM, destructive read, refresh     |
| VII–X cache first principles    | tag/index/offset, associativity, policies  | XLII: MESI, snooping, false sharing, ACE/CHI      |
| XI coherency (CPU ↔ DMA)        | clean/invalidate, DMA buffer ownership     | XLII: core ↔ core; LI: device isolation           |
| XII barriers (DMB/DSB/ISB)      | barrier definitions                        | XLIV: ordering model, litmus tests, store buffer  |
| XIV–XX MPU/MMU/TLB/paging       | generic translation, page table walk       | L: ARMv8 stages, ASID/VMID, break-before-make     |
| XXI DMA                         | descriptors, transfer types                | LI: SMMU/IOVA, bus-master attribution             |
| XXII bus/interconnect           | AHB/APB/AXI, arbitration                   | XXXIX: scheduling, row-hit policy, turnaround     |
| XXVI linker script              | sections, placement, symbols               | XLIX: relocation, PIC, pre-DRAM stack             |
| XXVII memory security           | XN, TrustZone *concepts*                   | LII: SAU/IDAU/NSC/veneers; LIII: side channels    |
| XXVIII performance engineering  | optimization strategy                      | LIV: PMU/DWT measurement, geometry discovery      |
| XXIX real-time                  | WCET vs average                            | XXXVIII/XXXIX: why DRAM is nondeterministic       |
| XXX debugging catalogue         | 5 symptom-driven problems                  | LVII: architecture-level anti-patterns            |
| V stack and heap                | SP, frames, fragmentation basics           | LV: allocator families, worst-case stack proof    |

[DESIGN RULE] If a new part restates something from I–XXXVII, delete it.
The continuation must be readable *only* by someone who has already finished
the first document.

---

## PART XXXVIII — DRAM FROM CELL TO CHANNEL

The first document taught volatile memory as SRAM: bistable, static, addressable,
deterministic. DRAM is none of those things. Teach it as a **managed resource with
state**, not as "slower RAM".

### 1. The cell

```
        WORD LINE
            │
        ────┴────
            │
        ┌───┴───┐   access transistor
        │       │
        ▼       │
      ┌───┐     │
      │ C │ ────┘        ONE transistor
      └─┬─┘                ONE capacitor
        │
       GND
        │
      BIT LINE ──→ SENSE AMPLIFIER
```

Explain:

- charge storage, not bistability
- why reading **destroys** the value (charge sharing onto the bit line)
- why every read must be followed by a **restore**
- why leakage forces **refresh**
- why the sense amplifier row is also the **row buffer**

[PRINCIPLE] SRAM holds a value. DRAM holds a decaying charge and continuously
re-decides what it was.

### 2. The hierarchy

```
CHANNEL
 └── RANK
      └── BANK GROUP
           └── BANK
                └── ROW (page)
                     └── COLUMN
                          └── BURST
```

Explain what parallelism exists at each level and what serializes.

### 3. The command sequence

```
ACT (row)        ← open row into row buffer   [tRCD]
   ↓
RD / WR (col)    ← burst from row buffer      [CL / CWL]
   ↓
PRE              ← close row, restore charge  [tRP]
   ↓
(bank idle)
```

Three outcomes for every access — teach these as the core DRAM concept:

| Case          | Condition                    | Cost                  |
|---------------|------------------------------|-----------------------|
| Row hit       | requested row already open   | CL only               |
| Row miss      | bank idle, no row open       | tRCD + CL             |
| Row conflict  | different row open in bank   | tRP + tRCD + CL       |

[PERFORMANCE] The same physical address can be 3× more expensive depending
only on what the *previous* access did. This is the single most important
DRAM fact for a real-time engineer.

### 4. Timing parameters

Cover, and make the learner extract each from a real datasheet:

tRCD, tRP, tRAS, tRC, CL, CWL, tWR, tWTR, tRTP, tRRD, tFAW, tRFC, tREFI, tCCD

Teach the difference between **clock cycles** and **nanoseconds**, and why a
"faster" DDR part can have identical absolute latency.

### 5. Refresh

```
tREFI (e.g. 7.8 µs)  ──→  issue REF
                            │
                            └── bank(s) unavailable for tRFC
```

Make the learner calculate:

```
refresh overhead = tRFC / tREFI
```

Then repeat at 85 °C where tREFI typically halves.

[PRINCIPLE] Refresh is a tax paid in latency jitter, and it scales with density
and temperature.

### 6. The DRAM family tree

Compare, without repeating the SLC/MLC discussion from the flash document:

- SDR / DDR / DDR2 / DDR3 / DDR4 / DDR5
- LPDDR2 / LPDDR3 / LPDDR4(X) / LPDDR5
- PSRAM (DRAM cell, SRAM interface, self-refresh hidden)
- HyperRAM / Xccela (PSRAM over an 8-bit DDR bus)
- On-die vs on-package vs discrete vs DIMM

[VARIES] MCU-class systems usually see PSRAM/HyperRAM or SDR/DDR2.
MPU-class systems see DDR3/DDR4/LPDDR4. The cell physics is identical.

### 7. Determinism

Explain precisely why DRAM breaks the real-time model built in PART XXIX:

```
Same instruction, same address, different latency because of:
 ├── row buffer state
 ├── refresh collision
 ├── bank conflict from another master
 ├── read/write turnaround on the shared bus
 ├── controller reordering
 └── temperature-driven refresh rate
```

[DESIGN RULE] If a code path has a hard deadline, it does not execute from DRAM.

---

## PART XXXIX — MEMORY CONTROLLER ENGINEERING

Part XXII taught the interconnect. This part teaches the component that turns
**bus transactions** into **DRAM commands** — and that is where most of the
performance and most of the jitter lives.

```
AXI requests
     ↓
┌────────────────────────────┐
│  Transaction queue          │
│  ┌──────────────────────┐   │
│  │ address mapping       │   │  ← chooses row/bank/col bits
│  ├──────────────────────┤   │
│  │ scheduler (reorder)   │   │  ← FR-FCFS, age, QoS
│  ├──────────────────────┤   │
│  │ timing/bank state FSM │   │  ← enforces tRCD/tRP/tFAW
│  ├──────────────────────┤   │
│  │ refresh engine        │   │
│  └──────────────────────┘   │
└────────────┬───────────────┘
             ↓
          DRAM PHY
```

Cover:

- **address mapping**: which physical address bits select row, bank, column,
  and why interleaving banks across low bits improves streaming while
  interleaving rows across low bits destroys it
- **FR-FCFS** scheduling: row hits are promoted ahead of older requests
- **write buffering** and read/write **turnaround** penalty
- **bus turnaround** on a shared DQ bus
- **refresh postponement** (pull-in / push-out credits)
- **QoS**: latency-sensitive vs bandwidth-hungry masters (CPU vs display vs GPU)
- **starvation** and age-based promotion

Quantitative work:

```
peak bandwidth  = data width × transfers/s
effective BW    = peak × row-hit efficiency × (1 − refresh overhead)
                       × (1 − turnaround overhead)
```

Make the learner compute effective bandwidth for a realistic mixed workload
and explain why 60–70 % of peak is a normal result.

[COMMON MISTAKE] Quoting peak DRAM bandwidth in a design document.

[DEBUGGING] A display or camera pipeline that tears only when another master is
active is almost always an arbitration/QoS problem, not a driver bug.

---

## PART XL — DRAM BRING-UP, TRAINING AND SIGNAL INTEGRITY

This is the part that does not appear in textbooks and appears constantly in real
projects. Treat it as the DRAM equivalent of flash bring-up.

```
CONTROLLER (digital)
      ↓
     PHY
  ├── DQ (data)
  ├── DQS (data strobe)
  ├── CK / CK#
  ├── CA / ADDR / CMD
  ├── ODT
  └── ZQ
      ↓
   DRAM DIE
```

### Training steps

```
Power/clock init
    ↓
Mode register write (MR0..MRn)
    ↓
ZQ calibration            ← output driver / ODT impedance
    ↓
Write leveling            ← align DQS to CK at the die (fly-by skew)
    ↓
Read gate training        ← find the DQS preamble window
    ↓
Read/write DQ deskew      ← per-bit centering in the data eye
    ↓
VREF training             ← DDR4/LPDDR4 receiver reference level
    ↓
Stability verification
```

Explain **where training runs**: boot ROM, first-stage loader, or SPL — before
any DRAM is usable, therefore from ROM/SRAM/TCM only. Explain the option of
storing training results in flash to shorten boot, and the risk of doing so
across temperature.

### Signal integrity concepts to teach

- data eye, setup/hold margin, jitter
- fly-by topology and why write leveling exists
- termination (ODT), reflections, stub length
- length matching per byte lane, not per net
- crosstalk, SSO/ground bounce
- power delivery to the DRAM rail during heavy write bursts

### Failure signatures table

| Symptom                                   | First suspicion                       |
|-------------------------------------------|---------------------------------------|
| Works cold, fails hot                     | refresh rate / margin, VREF drift     |
| Works at low clock only                   | training margin, SI, termination      |
| Single bit always wrong                   | one DQ line: solder, deskew, routing  |
| Random rare corruption under load         | power delivery, crosstalk, QoS/timing |
| Fails only after long uptime              | refresh, retention, thermal           |
| Fails only with DMA/display active        | SI under SSO, or arbitration timing   |

### Lab

Sweep frequency and voltage, run a memory stress pattern, and plot the shmoo of
pass/fail. Repeat at temperature extremes. This is the memory-systems equivalent
of the flash characterisation matrix.

[PRINCIPLE] DRAM either works or works *until it matters*. Untested margin is
not margin.

---

## PART XLI — EXTERNAL MEMORY INTERFACES

Part XXV connected flash and SRAM. This part covers the controllers that attach
**foreign memory** to the address space.

```
                ┌── Async SRAM / NOR  (FMC/FSMC/EMIF/EMC/WEIM)
                │
BUS MATRIX ─────┼── SDRAM / DDR       (SDRAM ctrl / MMDC / DDRC)
                │
                ├── PSRAM / HyperRAM  (OSPI / FlexSPI / HyperBus)
                │
                └── Serial NOR XIP    (QSPI / OSPI / FlexSPI)
```

Cover:

- multiplexed vs non-multiplexed address/data buses
- chip select, address setup/hold, NWE/NOE pulse widths, wait states, NWAIT
- how to convert **datasheet AC timing** into **controller register fields**
  (do this as a full worked example — it is the same skill as flash timeout
  derivation in the previous document, applied to a bus)
- burst vs single access modes
- **memory-mapped mode vs command mode**, and switching between them
- XIP from external NOR and execution from external PSRAM
- what happens to an XIP fetch when the same device receives a write command

[PRINCIPLE] Being memory-mapped tells you the *addressing*, not the *attributes*,
not the *latency*, and not the *coherency*.

[COMMON MISTAKE] Placing a DMA descriptor or a critical ISR in external memory
because "it's in the address map".

[DESIGN RULE] Any externally-attached memory needs three answers before use:
attributes, worst-case latency, and behaviour when the controller is reconfigured.

---

## PART XLII — MULTICORE CACHE COHERENCY

Part XI solved exactly one coherency problem: CPU cache vs DMA, resolved by
software maintenance. This part solves the other one: **two caches holding the
same line**, resolved by hardware protocol.

```
   CORE 0            CORE 1
     │                 │
  L1 D$             L1 D$
     │                 │
     └──── SNOOP ──────┘
            │
      COHERENT INTERCONNECT (SCU / CCI / CMN)
            │
           L2/L3
            │
          MEMORY
```

### Protocol

Teach MESI as a state machine per cache line:

```
        ┌─────────┐  remote read   ┌─────────┐
        │MODIFIED │───────────────▶│ SHARED  │
        └────┬────┘                └────┬────┘
             │ write-back               │ local write
             ▼                          ▼
        ┌─────────┐   evict       ┌─────────┐
        │EXCLUSIVE│──────────────▶│ INVALID │
        └─────────┘                └─────────┘
```

Cover: M, E, S, I; the O state in MOESI; F in MESIF; snoop filters; directory
vs snooping; inclusive vs exclusive cache hierarchies.

### Coherent vs non-coherent masters

```
CPU cluster ──── ACE      (full coherency, snoopable)
GPU         ──── ACE-Lite (I/O coherent, can snoop but is not snooped)
Legacy DMA  ──── AXI      (not coherent — software maintenance required)
```

[COMMON MISTAKE] Assuming "this SoC is coherent" applies to every master.
Coherency is a property of a **port**, not of a chip.

### False sharing

```
CACHE LINE (64 B)
┌───────────────┬───────────────┐
│ core0_counter │ core1_counter │
└───────────────┴───────────────┘
       ▲               ▲
    core 0          core 1     → line ping-pongs on every write
```

Have the learner measure it and then fix it with alignment/padding, and compute
the cost:

```
slowdown ≈ (coherency transaction latency) / (local write latency)
```

[PERFORMANCE] False sharing is invisible in source code and obvious in PMU counters.

---

## PART XLIII — ATOMICS, EXCLUSIVES AND LOCK-FREE MEMORY

Neither document has yet explained how a *single* memory location is safely
modified by two agents. Teach it as memory-system behaviour, not as an API.

### Load-linked / store-conditional

```
LDREX  addr   ──▶ set exclusive monitor for the ERG containing addr
   ...
STREX  addr   ──▶ succeeds only if monitor still set
                  fails on: context switch, another exclusive,
                            eviction, any write in the same ERG
```

Cover:

- local vs global exclusive monitors
- **ERG (Exclusive Reservation Granule)** — typically ≥ 8 bytes, often a cache
  line; two *unrelated* variables in the same ERG can cause livelock
- required retry loop, and bounded-retry design
- CLREX and context-switch semantics

[ARM-SPECIFIC] ARMv8.1-A LSE atomics (LDADD, SWP, CAS, CASAL) remove the retry
loop entirely and scale far better under contention.

### Decision table

| Mechanism            | Scope             | ISR-safe | Multicore-safe | Cost         |
|----------------------|-------------------|----------|----------------|--------------|
| Disable interrupts   | one core          | yes      | no             | latency spike|
| Bit-band write       | one word, one core| yes      | no             | very low     |
| LDREX/STREX          | system            | yes      | yes            | retry risk   |
| LSE atomic           | system            | yes      | yes            | low          |
| Spinlock             | system            | careful  | yes            | blocking     |
| Mutex                | tasks             | no       | yes            | scheduler    |

### Lock-free structures

- SPSC ring buffer: exactly which loads/stores need which ordering, and why a
  full barrier is unnecessary
- the ABA problem and generation counters
- when lock-free is worse than a mutex (contention, priority, debuggability)

### C11 mapping

```
memory_order_relaxed  → no barrier, atomicity only
memory_order_acquire  → no later access may be observed before  (DMB ISHLD)
memory_order_release  → no earlier access may be observed after (DMB ISH)
memory_order_seq_cst  → total order across all threads
```

[COMMON MISTAKE] `volatile` used as an atomic. `volatile` guarantees the access
happens; it guarantees nothing about atomicity or ordering.

---

## PART XLIV — THE ARCHITECTURAL MEMORY MODEL

Part XII introduced DMB/DSB/ISB as instructions. This part explains the **model
they implement**, which is what actually determines correctness.

### Two independent reorderers

```
SOURCE ORDER
     │
     ▼   compiler reordering ── defeated by: barriers, atomics, volatile-ish
COMPILED ORDER
     │
     ▼   CPU reordering ────── defeated by: DMB/DSB, acquire/release
OBSERVED ORDER
```

[PRINCIPLE] Fixing one of these does not fix the other. Most "impossible" bugs
are one of the two being fixed and the other not.

### Structural causes

- store buffer (write is retired before it is visible)
- write combining / merging
- speculative and out-of-order loads
- multi-copy atomicity, and why two observers may disagree on order

### Litmus tests

Teach these four explicitly, with the "can this outcome happen?" question:

```
MESSAGE PASSING           STORE BUFFERING
T0: data=1                T0: x=1
    flag=1                    r0=y
T1: r0=flag               T1: y=1
    r1=data                   r1=x
Can r0==1 && r1==0 ?      Can r0==0 && r1==0 ?
```

```
LOAD BUFFERING            IRIW (independent reads, independent writes)
T0: r0=x ; y=1            T0: x=1     T2: r0=x ; r1=y
T1: r1=y ; x=1            T1: y=1     T3: r2=y ; r3=x
```

Then show the minimal fix for each, and prove that a full barrier is usually
stronger than required.

### Dependencies

- address dependency (naturally ordered on ARM)
- data dependency
- control dependency (NOT ordering — needs a barrier or CSDB)

[SECURITY] Control dependencies not being ordering is precisely the hole that
speculative side channels exploit (see PART LIII).

[DESIGN RULE] Write down the required ordering as a sentence before choosing an
instruction: "the flag store must not be observable before the payload stores."

---

## PART XLV — COMPILER AND ABI MEMORY SEMANTICS

The learner now has a correct hardware model. This part explains why correct
hardware reasoning still produces broken firmware.

### The C abstract machine

Explain that C does not describe your memory system. It describes an abstract
machine, and the compiler may do anything unobservable within that machine.

Cover:

- **strict aliasing**: two pointers of incompatible type may be assumed not to
  alias; type-punning through casts is UB; the `memcpy` idiom is the portable fix
- `restrict` and what the optimizer does with it
- **dead store elimination** removing a `memset` that clears a key
  → `explicit_bzero` / `memset_s` / opaque barrier [SECURITY]
- **struct layout**: padding, alignment, `packed` and the unaligned-access trap
  it creates on strict-alignment targets
- bitfield allocation order is implementation-defined — never overlay a bitfield
  struct on a hardware register or a wire format
- endianness, byte-swap intrinsics, network vs host order
- `volatile`: guarantees the access is emitted, in program order **relative to
  other volatile accesses only**, with no atomicity and no CPU barrier
- inline asm memory clobbers
- LTO removing "obviously used" symbols and reordering across translation units
- section attributes, alignment attributes, placement of `.ramfunc`

### ABI

- stack alignment requirements (8-byte AAPCS at public interfaces)
- argument passing and where large structures actually live
- callee/caller saved registers and what an ISR must preserve
- what the ABI implies about the *minimum* stack frame you can rely on

### Worked demonstrations

Give three code fragments that are correct under the hardware model of
PARTS I–XLIV and still fail, and identify the compiler rule responsible.

[COMMON MISTAKE] "It works at -O0" is a diagnosis, not a fix.

---

## PART XLVI — MEMORY ECC, SOFT ERRORS AND FUNCTIONAL SAFETY

The flash document covered flash ECC. Volatile memory ECC is a different problem
with different failure modes.

### Soft errors

```
neutron / alpha particle
        ↓
  charge deposition
        ↓
  cell state flips           ← SRAM: no charge loss, just a flipped bit
        ↓
  silent data corruption
```

Cover: SER, FIT rate, FIT/Mbit, altitude and process dependence, single-bit
upset vs multi-bit upset, why smaller geometries increase MBU probability.

Make the learner calculate:

```
device FIT = FIT/Mbit × Mbit × derating
MTBF       = 10^9 / FIT   hours
```

### Protection schemes

| Scheme       | Detect | Correct | Overhead | Typical use          |
|--------------|--------|---------|----------|----------------------|
| Parity       | 1 bit  | none    | ~1/8     | I-cache, TCM         |
| SECDED       | 2 bits | 1 bit   | ~1/4     | system SRAM, DRAM    |
| SEC-DAEC     | more   | adjacent| higher   | safety MCUs          |
| ECC + scrub  | —      | —       | +time    | long-uptime systems  |

### The practical traps

- **initialization**: ECC RAM read before it is ever written raises an
  uncorrectable error — startup code must write the entire ECC region
  [COMMON MISTAKE] this breaks `.noinit`, retained buffers, and stack painting
- **partial-word writes** on ECC memory become read-modify-write; a byte write
  to uninitialized ECC memory faults
- **scrubbing**: background vs demand, and why scrub interval must be shorter
  than the accumulation time to a double-bit error
- error injection registers for testing the handler
- correctable-error counters as a **predictive maintenance** signal

### Safety architecture

```
FAULT
  ↓
DETECTION      ← ECC, parity, lockstep compare, MBIST, watchdog
  ↓
CLASSIFICATION ← correctable / uncorrectable / transient / permanent
  ↓
REACTION       ← retry, isolate, degrade, reset, safe state
  ↓
RECORD         ← persistent diagnostic (ties to the flash document)
```

Cover lockstep (DCLS/TCLS), delay lines and compare units, MBIST at startup vs
runtime, diagnostic coverage and latent-fault metrics, and why memory is usually
the largest contributor to a hardware failure-rate budget.

---

## PART XLVII — MEMORY TEST ALGORITHMS

A separate, small, high-value part. Every serious product tests its own RAM.

### Fault models

```
stuck-at            cell locked to 0 or 1
transition          cell can go 0→1 but not 1→0
coupling            writing cell A changes cell B
address decoder     two addresses map to one cell / one address to two
retention           cell loses state over time
```

### Algorithms

| Algorithm     | Ops   | Detects                                | Cost |
|---------------|-------|----------------------------------------|------|
| Checkerboard  | O(n)  | gross shorts, some coupling            | low  |
| Walking 1/0   | O(n²) | address decoder, coupling              | huge |
| MATS+         | 5n    | stuck-at, some address faults          | low  |
| March C-      | 10n   | stuck-at, transition, coupling, AF     | mid  |
| March SS      | 22n   | + more coupling classes                | high |
| Address-in-addr| 2n   | address decoder faults specifically    | low  |

Teach March C- notation explicitly:

```
⇕(w0) ⇑(r0,w1) ⇑(r1,w0) ⇓(r0,w1) ⇓(r1,w0) ⇕(r0)
 │      │                                     │
 init   ascending march             descending, final read
```

### The hard practical problem

```
You must test the RAM that contains:
 ├── your stack
 ├── your variables
 └── your test routine itself
```

Teach the solutions: run from ROM/TCM with a register-only test, test in halves
and relocate the stack, test non-destructively at runtime (save/test/restore per
word with interrupts masked), and partition POST vs periodic runtime testing.

[DESIGN RULE] Decide per region: tested destructively at boot, tested
non-destructively at runtime, or protected by ECC and never marched.

---

## PART XLVIII — MEMORY POWER MANAGEMENT AND RETENTION

Part IV mentioned retention SRAM. This part makes it an engineering discipline.

```
VDD
 ├── Core domain        ← off in deep sleep
 ├── SRAM bank 0..n     ← individually retainable
 ├── Retention domain   ← low-voltage state retention
 └── Backup domain      ← RTC + backup RAM, separate supply
```

### What survives what

Build this matrix explicitly for a target device — it is one of the highest-value
tables an embedded engineer can own:

| Event             | CPU regs | SRAM | Retention RAM | Backup RAM | Cache | TLB/MMU | Peripherals |
|-------------------|----------|------|---------------|------------|-------|---------|-------------|
| Software reset    |          |      |               |            |       |         |             |
| Watchdog reset    |          |      |               |            |       |         |             |
| Brownout (BOR)    |          |      |               |            |       |         |             |
| Power-on (POR)    |          |      |               |            |       |         |             |
| Sleep             |          |      |               |            |       |         |             |
| Stop / retention  |          |      |               |            |       |         |             |
| Standby           |          |      |               |            |       |         |             |
| Shutdown          |          |      |               |            |       |         |             |

### Engineering content

- retention voltage vs operating voltage; state-retention flip-flops
- leakage current per KB retained → **battery-life calculation**
- wake-up latency as a function of what must be restored
- `.noinit` sections, magic numbers, and CRC-validated retained structures
- caches and TLBs are lost across most low-power states: restoration sequence
- DRAM self-refresh entry/exit, and re-training on exit
- what the bootloader must assume when it cannot tell POR from wake

[DESIGN RULE] Never trust retained RAM without a magic value **and** a checksum.
Retention is a probability, not a guarantee.

[COMMON MISTAKE] Placing a DMA buffer or a stack in a bank that the power manager
turns off in the low-power state the application actually uses.

---

## PART XLIX — BOOT-TIME MEMORY ARCHITECTURE

Part XXVI taught the linker script. This part answers the question the linker
script cannot: **what does memory look like before memory works?**

```
RESET
  ↓
BOOT ROM            ← immutable, on-die, executes from ROM
  ↓  (stack? where?)
FIRST STAGE / SPL   ← runs from internal SRAM, TCM, or cache-as-RAM
  ↓  initializes DRAM (PART XL)
SECOND STAGE        ← runs from DRAM, relocates itself
  ↓
OS / APPLICATION
```

Cover:

- **the pre-RAM stack problem** and its three answers: on-die SRAM, TCM, or
  *cache-as-RAM* (lock the cache in no-eviction mode and use it as scratch)
- **address remap**: the same physical memory appearing at 0x00000000 and at its
  natural address; boot pin / OTP selecting what is mapped at reset
- XIP-then-relocate, and why the relocation code must not execute from the
  region it is overwriting
- **position independence**: PIC/PIE, GOT, `_start` relocation loops,
  why a bootloader that may be linked to two different slots must be PIC
- image handoff: reserved memory regions, device-tree `/memory` and
  `reserved-memory`, and passing a memory map forward
- what the MMU/MPU state is at each stage boundary

### MMU/cache enable sequence [ARM-SPECIFIC]

```
1. Build translation tables in memory
2. Clean D-cache (tables must be visible to the table walker)
3. Set MAIR / TCR / TTBR
4. DSB                      ← ensure system register writes complete
5. Invalidate TLB, I-cache, BP
6. Set SCTLR.M / .C / .I
7. ISB                      ← ensure subsequent fetches use the new config
```

Have the learner explain why each barrier is there and what fails without it.

[SECURITY] The set of memory that is locked, hidden or made execute-never
*before* the first non-immutable code runs is the entire basis of a root of trust.
This connects directly to the secure-boot chapter of the flash document.

---

## PART L — ARMv8-A TRANSLATION IN DETAIL

Parts XV–XX taught translation generically. This part teaches the real thing,
because the learner will read ARM documentation, not a textbook.

```
                Virtual Address (48-bit)
                          │
        ┌─────────────────┴─────────────────┐
     TTBR0 (low VA, user)          TTBR1 (high VA, kernel)
        └─────────────────┬─────────────────┘
                          ▼
        L0 → L1 → L2 → L3 descriptors
                          │
             ┌────────────┴────────────┐
        block descriptor          page descriptor
        (1 GB / 2 MB)                 (4 KB)
                          ▼
                Intermediate Physical Address
                          │
                 Stage 2 (hypervisor)
                          ▼
                   Physical Address
```

Cover:

- exception levels EL0–EL3 and which translation regime applies at each
- TTBR0/TTBR1 split and TCR fields (T0SZ/T1SZ, granule select)
- granule sizes 4 KB / 16 KB / 64 KB and the resulting level structure
- **stage 1 vs stage 2**, IPA, and why a hypervisor needs both
- MAIR + AttrIndx: attributes are indirected, not encoded in the descriptor
- shareability: non-shareable / inner / outer, and what a domain actually means
- access flag, hardware AF/DBM update vs software management
- **ASID** and **VMID**: why a context switch is not a full TLB flush
- TLBI variants and scope (VA, ASID, VMALL, IS/OS)
- **break-before-make**: you may not change a live translation in place
- PAN, UAO, WXN, and hardware-enforced W^X

[PRINCIPLE] Mapping one physical region twice with different memory attributes
produces a **mismatched alias** and is architecturally UNPREDICTABLE. This is the
single most common cause of "impossible" coherency bugs on Cortex-A.

[INFERENCE] Everything in this part has a Cortex-M analogue that is simply
degenerate: one regime, no translation, attributes from MPU regions instead of
descriptors. Present it that way to reinforce the invariant model.

---

## PART LI — IOMMU / SMMU AND BUS-MASTER ISOLATION

Part XXI treated DMA as a performance and coherency problem. This part treats it
as an **addressing and trust** problem.

```
                 CPU              DEVICE
                  │                 │
                 MMU              SMMU        ← same job, different master
                  │                 │
            Physical Address   Physical Address
                  └────────┬────────┘
                      INTERCONNECT
```

Cover:

- why an unconstrained DMA master is equivalent to a CPU with no MMU
- StreamID, contexts, per-device translation tables
- IOVA vs physical address; DMA-API `map`/`unmap`/`sync` semantics [OS-SPECIFIC]
- bounce buffers and why a driver may not DMA to an arbitrary buffer
- scatter-gather lists as page-granular translation
- device isolation, and containment of a compromised or buggy peripheral

### MCU-class equivalents

Do not let the learner conclude this is a Linux-only topic:

```
MCU bus master isolation:
 ├── DMA channel security attribution (secure / non-secure master)
 ├── privileged / unprivileged master attribution
 ├── peripheral firewall / resource isolation units
 ├── memory region attribution checkers on the bus, not in the CPU
 └── per-master MPU-like filters in the interconnect
```

[SECURITY] The CPU MPU protects the CPU. It does not protect memory from any
other master. Isolation must be enforced at the interconnect.

[DESIGN RULE] For every master in the system, write down: what it can reach,
who programmed it, and what happens if that programming is wrong.

---

## PART LII — TRUSTZONE MEMORY ARCHITECTURE

Part XXVII mentioned TrustZone. This part makes it concrete, because memory
partitioning *is* TrustZone.

### Cortex-M (TrustZone-M / ARMv8-M)

```
ADDRESS SPACE
 ├── Secure          (S)     ← only secure state may access
 ├── Non-Secure      (NS)    ← either state
 └── Non-Secure Callable (NSC) ← entry points only

    SAU  (software-defined)  ┐
                             ├─→ final security attribution
    IDAU (silicon-defined)   ┘
```

Cover:

- SAU regions, IDAU, and how the two combine (most-secure wins)
- **secure gateway**: SG instruction in an NSC region, then BXNS/BLXNS to return
- veneers and why a direct call into secure code fails
- banked stacks: MSP_S / PSP_S / MSP_NS / PSP_NS, and the limit registers
  MSPLIM/PSPLIM
- **stack sealing** and the return-address integrity problem
- secure faults: SecureFault, and the fault status bits worth memorising
- **pointer validation**: `TT`/`TTA` instructions and
  `cmse_check_address_range` — validating that an NS-supplied pointer really is
  NS-accessible before the secure side dereferences it
- lazy floating-point state preservation and the leakage issue it created

[SECURITY] The dominant vulnerability class in TrustZone-M firmware is a secure
function dereferencing a non-secure pointer without attribution checking.
Confused-deputy, not cryptography.

### Cortex-A (TrustZone-A)

- NS bit as a 33rd address bit, propagated on the bus
- TZASC / TZC carving secure DRAM regions
- EL3 / secure monitor, SMC calls
- shared non-secure memory for TEE communication, and why it must be
  validated and copied, never used in place

[COMMON MISTAKE] Assuming secure memory is encrypted. Secure memory is
*partitioned*. Encryption is a separate mechanism (see the flash document).

---

## PART LIII — MEMORY SIDE CHANNELS, SPECULATION AND PHYSICAL ATTACKS

Everything so far assumed the only observable results are architectural.
This part removes that assumption.

```
ARCHITECTURAL STATE   ← what the ISA says is visible
        +
MICROARCHITECTURAL STATE  ← caches, TLBs, predictors, buffers
        ↓
        leaks through TIMING
```

### Cache timing attacks

```
FLUSH+RELOAD          PRIME+PROBE           EVICT+TIME
flush shared line     fill the set          evict target
victim runs           victim runs           time victim
time reload           time re-access        infer usage
```

Explain each in terms of the cache model already built (sets, ways, lines,
eviction) — no new cache theory required.

### Speculation

- Spectre v1: bounds-check bypass; the control dependency from PART XLIV
- Spectre v2: branch target injection
- Meltdown-class: fault-deferred loads leaving cache traces
- mitigations: index masking, CSDB / speculation barriers, hardened loads

[VARIES] Most classic Cortex-M cores do not speculate and have no cache in
many parts — say so honestly rather than implying universal risk. State clearly
which classes of device are affected.

### Constant-time programming

```
FORBIDDEN with secret data:
 ├── secret-dependent branches
 ├── secret-dependent memory indices
 ├── secret-dependent loop counts
 └── early-exit comparisons (memcmp on a MAC)
```

### Physical memory attacks

- cold boot / DRAM remanence, and the cooling trick
- RowHammer: repeated row activation disturbing neighbours — an *integrity*
  failure of the DRAM model taught in PART XXXVIII
- bus probing, interposers, and why external memory is a trust boundary
- memory encryption and integrity trees as the countermeasure

[DESIGN RULE] Draw the trust boundary at the package edge. Anything crossing it
is confidential only if encrypted and authentic only if integrity-protected.

---

## PART LIV — MEASURING THE MEMORY SYSTEM

Part XXVIII taught optimization. This part teaches **evidence**. No memory
performance claim is acceptable without a measurement method.

### Instruments

```
Cortex-M:  DWT cycle counter, DWT comparators/watchpoints, ITM, ETM/MTB trace
Cortex-A:  PMU event counters, cycle counter, ETM/CoreSight, perf
System:    interconnect performance monitors, DRAM controller counters
```

Key event classes to teach: cycles, instructions, L1D refill, L1I refill,
L2 refill, TLB refill, bus access, memory stall cycles.

### Derived quantities

```
AMAT = hit_time + miss_rate × miss_penalty

CPI_memory = memory_stall_cycles / instructions

bandwidth = bytes_transferred / elapsed_time
```

### Discovering geometry experimentally

One of the best labs in the whole document:

```
for stride in 1..N:
    pointer-chase a buffer of size S with that stride
    plot time per access
```

```
time/access
   │            ┌─────  DRAM
   │        ┌───┘
   │    ┌───┘  L2
   │ ───┘  L1
   └──────────────────── working-set size
```

From the plot, the learner **derives** cache line size, cache sizes,
associativity and TLB reach without reading a datasheet — and then checks the
datasheet to confirm. This is the memory-systems equivalent of characterising a
flash part.

### Measurement discipline

- pointer-chase for latency; streaming for bandwidth; never confuse them
- warm-up, cache priming, and what a cold measurement actually means
- observer effect: instrumentation that changes what it measures
- report distributions and maxima, not averages, for anything real-time
- statistical stability: repeat count, outlier handling, thermal drift

[PRINCIPLE] An unmeasured optimization is a guess with extra confidence.

---

## PART LV — ALLOCATOR ENGINEERING AND WORST-CASE MEMORY

Part V introduced stack and heap. This part makes memory usage **provable**.

### Allocator families

| Allocator     | Alloc time   | Fragmentation | Deterministic | Typical use        |
|---------------|--------------|---------------|---------------|--------------------|
| Bump / arena  | O(1)         | none (no free)| yes           | init-time only     |
| Pool / slab   | O(1)         | internal only | yes           | fixed-size objects |
| Buddy         | O(log n)     | internal      | bounded       | page allocation    |
| First/best fit| O(n)         | external      | no            | general purpose    |
| TLSF          | O(1)         | bounded       | yes           | real-time heaps    |

Explain TLSF's two-level segregated free list and why it achieves O(1) with a
bounded fragmentation factor — the reason it appears in real-time systems.

### Fragmentation

```
internal = allocated_block − requested_size
external = free_total − largest_free_block
```

Prove that a workload can fail to allocate with 60 % of memory free.

### Hardening

- allocator metadata is adjacent to user data → heap overflow rewrites metadata
- guard bytes, canaries, poisoning on free, quarantine, out-of-line metadata
- MPU-backed guard regions between heap and stack

### Worst-case stack analysis

```
WCS = max over all paths of Σ frame sizes
      + deepest ISR nesting
      + ABI alignment padding
      + compiler temporaries at the chosen -O level
```

Cover: call-graph analysis, recursion prohibition, function pointers as analysis
blockers, `-fstack-usage`, stack painting + high-water measurement, MSPLIM/PSPLIM
or MPU guard region as the runtime backstop.

[DESIGN RULE] A safety-relevant system needs a stack bound derived from the call
graph **and** a hardware guard. Neither alone is sufficient.

---

## PART LVI — INTER-PROCESSOR SHARED MEMORY

Modern SoCs are heterogeneous. This part covers memory between *dissimilar*
cores — a case the SMP coherency of PART XLII does not solve.

```
        Cortex-A (Linux)            Cortex-M (RTOS)
              │                            │
        sees 0x80000000            sees 0x38000000
              └───────────┬────────────────┘
                   SAME PHYSICAL RAM
```

[COMMON MISTAKE] Putting a pointer in a shared structure. Addresses are not
portable across cores — share offsets, or translate explicitly.

Cover:

- AMP vs SMP; big.LITTLE vs A+M heterogeneous
- hardware mailboxes, IPC interrupts, hardware semaphores/spinlock IP
- rpmsg / virtio ring buffers over shared memory; OpenAMP lifecycle
- required memory attributes: non-cacheable shared region, or cacheable with
  explicit maintenance on both sides (and both sides must agree)
- remote core lifecycle: firmware load into shared RAM, release from reset,
  crash detection, recovery, and what happens to shared buffers on remote reset
- reserved-memory carveouts so the OS never allocates from the shared region

Have the learner draw the **dual memory map** for one real SoC, listing every
region with both cores' addresses and both cores' cache attributes.

---

## PART LVII — MEMORY ARCHITECTURE ANTI-PATTERN CATALOGUE

Part XXX debugged symptoms. This part attacks the **design decisions** that
create those symptoms. Give each entry as: anti-pattern → symptom → root cause →
detection → fix.

| # | Anti-pattern                                              | Typical symptom                          |
|---|-----------------------------------------------------------|------------------------------------------|
| 1 | DMA buffer not cache-line aligned / sized                 | neighbouring variable corrupted          |
| 2 | CPU data sharing a cache line with a DMA buffer           | rare, load-dependent corruption          |
| 3 | Buffer in DTCM that the DMA master cannot reach           | transfer silently does nothing / faults  |
| 4 | Peripheral region marked cacheable or normal              | missed events, stale status registers    |
| 5 | Two mappings of one region with different attributes      | unpredictable, works until it doesn't    |
| 6 | Stack in a bank powered down in the used low-power mode   | wake-up crash, corrupted context         |
| 7 | Vector table in flash while flash is being erased         | brick on update interruption             |
| 8 | Descriptors in write-back memory, never cleaned           | DMA reads stale descriptors              |
| 9 | Stack and heap in one MPU region                          | overflow undetected until corruption     |
|10 | ECC RAM region read before written                        | uncorrectable fault at boot              |
|11 | Hard-deadline ISR executing from DRAM or external NOR     | rare deadline miss under load            |
|12 | Shared struct containing pointers across two cores        | garbage on the remote core               |
|13 | Bitfield struct overlaid on a hardware register           | works on one toolchain only              |
|14 | Retained RAM trusted without magic + CRC                  | corrupt state survives and propagates    |
|15 | Barrier omitted after a config register write             | intermittent, optimization-dependent     |
|16 | Non-secure pointer dereferenced in secure code            | full secure-world compromise             |
|17 | Memory sized from typical rather than worst-case usage    | field failures at high load only         |
|18 | Peak DRAM bandwidth used as the design budget             | pipeline underruns in production         |

Then run a **design review exercise**: present a deliberately bad memory
architecture containing at least eight of these, and require the learner to find
every one, classify severity, and propose fixes — mirroring the flash design
review.

---

## PART LVIII — QUANTITATIVE EXERCISES (DELTA ONLY)

At least 40 problems, none overlapping PART XXXIII. Every solution shows reasoning.

**DRAM and controller**
1. Compute refresh overhead from tRFC and tREFI, then repeat at 85 °C.
2. Compute access latency for row hit, row miss and row conflict.
3. Compute effective bandwidth given a row-hit rate and turnaround penalty.
4. Given an address-mapping scheme, determine whether a 2D image traversal
   produces row hits or conflicts; redesign the stride.
5. Size a frame buffer bandwidth budget for a display and check it against DDR.
6. Compute worst-case latency for a hard-real-time access to DRAM.

**Coherency and atomics**
7. Compute the cost of false sharing given coherency transaction latency.
8. Determine how many cache lines a shared structure must be padded to.
9. Given an ERG size, decide whether two locks can share a cache line.
10. Compute retry probability for LDREX/STREX under N-core contention.

**ECC and safety**
11. Convert FIT/Mbit to device FIT and MTBF.
12. Compute the maximum safe scrub interval for a target double-bit-error rate.
13. Compute ECC storage overhead for several word widths.

**Power and retention**
14. Compute battery life for a given retained SRAM size and leakage current.
15. Trade retained size against wake-up energy and choose an optimum.

**Translation**
16. Compute page table size for 4 KB / 16 KB / 64 KB granules at 48-bit VA.
17. Compute TLB reach for each granule and entry count.
18. Compute the number of table-walk memory accesses for a stage-1+stage-2 miss.

**Allocators and stack**
19. Compute worst-case stack depth from a call graph with nested interrupts.
20. Demonstrate an allocation failure with 60 % of the heap free.
21. Compute internal fragmentation for a pool allocator across a size mix.

**Measurement**
22. Derive AMAT from measured PMU counters.
23. Derive cache line size and cache size from a stride-sweep table (given data).
24. Compute memory-stall CPI contribution and predict speedup from a fix.

Extend each family until the total reaches 40+.

---

## PART LIX — HANDS-ON LABS (DELTA ONLY)

At least 15 labs, none overlapping PART XXXIV. Same structure as before:
Objective / Theory / Hardware / Software / Procedure / Expected result /
Failure modes / Debugging / Measurements / Engineering lesson.

1. **Cache geometry discovery** — stride sweep; derive line size, L1/L2 size,
   associativity and TLB reach; verify against the datasheet.
2. **DRAM shmoo** — sweep frequency and voltage with a stress pattern; plot the
   pass/fail region; repeat hot and cold.
3. **Row-hit experiment** — two access patterns over the same buffer; measure the
   bandwidth difference; explain it from the address mapping.
4. **False sharing** — two threads incrementing adjacent counters; measure;
   fix by padding; measure again.
5. **Exclusive monitor** — force STREX failure; observe ERG effects by moving a
   second variable in and out of the same granule.
6. **Litmus test** — implement message-passing on two cores without barriers;
   observe the violation; add acquire/release; prove the fix.
7. **Strict aliasing** — build a fragment that breaks only at -O2; identify the
   rule; fix it three different ways.
8. **Secret zeroization** — show the compiler deleting a `memset`; verify in the
   disassembly; fix with an explicit-zero primitive.
9. **ECC error injection** — inject a correctable and an uncorrectable error;
   write the handler; log the diagnostic persistently.
10. **March C-** — implement it for one SRAM bank while running from a different
    memory; then implement a non-destructive runtime variant.
11. **Retention survey** — measure what survives each reset and low-power mode;
    fill in the PART XLVIII matrix from experiment, not documentation.
12. **Cache-as-RAM** — bring up a stack before RAM exists (or emulate the
    constraint) and document the sequence.
13. **MMU enable** — build translation tables and enable the MMU with correct
    barriers; then remove each barrier in turn and record what breaks.
14. **Mismatched alias** — map one region twice with different attributes;
    demonstrate the incoherency; explain why the architecture forbids it.
15. **TrustZone pointer validation** — implement a secure entry point, pass it a
    malicious non-secure pointer, observe the compromise, then add attribution
    checking and re-test.
16. **Dual-core shared memory** — set up a shared ring buffer between two cores
    with different address views; get the attributes right; then deliberately get
    them wrong and observe the failure.
17. **Cache timing channel** — measure access-time difference between cached and
    evicted lines; demonstrate that timing distinguishes them.

---

## PART LX — THE MEMORY ENGINEER'S FIELD MANUAL

The practical culmination, parallel to the flash field manual.

### Chapter A — First 60 minutes with an unknown SoC

```
1. Find the memory map           → every region, size, base
2. Classify each region          → volatile / non-volatile / peripheral / reserved
3. Find the caches               → levels, sizes, line size, policy, maintenance ops
4. Find the tightly-coupled memory → sizes, which masters can reach it
5. Find the protection unit      → MPU regions or MMU regime, who configures it
6. List every bus master         → CPU(s), DMA(s), display, network, accelerators
7. For each master               → which regions it can reach, coherent or not
8. Find the low-power modes      → what is retained in each
9. Find the reset sources        → what is preserved across each
10. Find the errata              → memory errata are disproportionately severe
```

Output: a one-page memory architecture diagram, produced from documentation only.

### Chapter B — Build the memory map from documentation alone
Then verify it experimentally by probing region behaviour, and list every place
the documentation was wrong or incomplete.

### Chapter C — Bring up external memory
Interface configuration, timing derivation, training, stress test, margin sweep.

### Chapter D — Bring up a correct DMA path
Attributes, alignment, ownership protocol, maintenance or coherent port,
descriptor placement, and a deliberate corruption test to prove it is right.

### Chapter E — Configure protection from zero
MPU/MMU regions, XN enforcement, stack guard, peripheral isolation, and a fault
handler that reports address, access type, and faulting context usefully.

### Chapter F — Debug a memory corruption
```
Symptom
  ↓
Is it deterministic?      → build/UB vs timing/hardware
  ↓
Who else can write it?    → DMA, other core, ISR, stack, MPU gap
  ↓
Watchpoint on the address
  ↓
Identify the master
  ↓
Attribute the mechanism
  ↓
Prove the fix by reproducing and then failing to reproduce
```

### Chapter G — Diagnose a performance cliff
Measure before hypothesizing: counters, working set, alignment, attributes,
contention, row-hit rate. Then fix one variable at a time.

### Chapter H — Harden a memory architecture
Trust boundaries, master attribution, isolation enforcement point, secret
handling, side-channel scope, and what an attacker with each capability achieves.

### Chapter I — Memory design review
Present a deliberately flawed architecture (PART LVII catalogue) and require a
full written review with severity ranking.

### Chapter J — Certification project

Give a completely unknown SoC with datasheet, reference manual, architecture
manual and errata. The learner must independently produce:

```
                      UNKNOWN SoC
                           │
          ┌────────────────┴────────────────┐
          ▼                                 ▼
    DOCUMENTATION                       HARDWARE
          │                                 │
          ▼                                 ▼
    MEMORY MAP + ATTRIBUTES          MEASURED GEOMETRY
          │                                 │
          ▼                                 ▼
    MASTER/ACCESS MATRIX             LATENCY/BANDWIDTH DATA
          │                                 │
          └────────────────┬────────────────┘
                           ▼
                  LINKER + STARTUP DESIGN
                           │
          ┌────────────────┼────────────────┐
          ▼                ▼                ▼
    CACHE/DMA POLICY  PROTECTION MODEL  POWER/RETENTION PLAN
          │                │                │
          └────────────────┼────────────────┘
                           ▼
                 ISOLATION & TRUST MODEL
                           │
                           ▼
              MEASURED, REVIEWED, DOCUMENTED
                  MEMORY ARCHITECTURE
```

Deliverables: the architecture document, the linker script, the startup code, the
protection configuration, the measurement report, and a written justification for
every attribute chosen.

---

## MYTHS — DELTA ONLY

Add to PART XXXVII without repeating it:

- ❌ "DRAM is just slower SRAM."
- ❌ "Peak bandwidth is achievable bandwidth."
- ❌ "The SoC is cache-coherent." (coherency is per-port, not per-chip)
- ❌ "volatile makes it atomic." / "volatile is a memory barrier."
- ❌ "A barrier in the compiler is a barrier in the CPU."
- ❌ "If it works at -O0 the hardware is fine."
- ❌ "ECC memory cannot produce faults." (uninitialized ECC RAM will)
- ❌ "Retained RAM is guaranteed across that low-power mode."
- ❌ "The MPU protects memory." (it protects the CPU's view of memory)
- ❌ "Secure memory is encrypted memory."
- ❌ "Only servers care about side channels."
- ❌ "Physical addresses are the same on every core."
- ❌ "Two mappings of the same RAM are equivalent."
- ❌ "Aligning the buffer is enough." (size must also be line-aligned)
- ❌ "Worst-case stack usage can be measured by running the tests."
- ❌ "Memory tests are for factory only."

For each: state the belief, explain the mechanism that makes it false, and give
the failure it produces in the field.

---

## CLOSING PRINCIPLE

The first document answered: *where does this address go?*

This continuation answers three further questions that separate an engineer from
a user of memory:

```
WHEN does it get there?     → DRAM state, scheduling, contention, refresh
WHO ELSE can get there?     → other cores, other masters, other security states
HOW DO I PROVE IT?          → measurement, testing, characterisation, review
```

[PRINCIPLE] Vendor terminology changes; architecture doesn't.
Add to that: **timing, trust and evidence are engineering, not documentation.**
