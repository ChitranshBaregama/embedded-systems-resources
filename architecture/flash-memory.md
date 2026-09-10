# FLASH MEMORY AS AN ENGINEERING DISCIPLINE
## A vendor-independent curriculum: from the silicon cell to the production firmware update

**Compiled 31 August 2026**

**Design goal of this document:** teach the *invariant* structure of flash memory — the physics, the architecture, the failure modes, the design patterns — so that you can walk up to *any* microcontroller reference manual, from any vendor, and know exactly which questions to ask and what the answers imply.

This is deliberately **not** a manual for one chip. Wherever a real device is mentioned, it is used only as *evidence that a concept has variants*, never as the definition of the concept.

---

## 0.1 HOW THIS DOCUMENT IS BUILT

### The three-label rule

| Label | Meaning |
|---|---|
| **[PRINCIPLE]** | Holds across essentially all flash implementations. Safe to reason from. |
| **[VARIES]** | Real implementations differ. You must look it up for your part. The document tells you the *range* and *why* it varies. |
| **[INFERENCE]** | My engineering judgement or design opinion. Argue with it. |

Almost everything painful in flash engineering lives in the **[VARIES]** category. The single most valuable skill this document can give you is the reflex to notice *which category a fact belongs to* before you rely on it.

### The two things never asserted here

1. **Numbers.** No endurance figure, retention figure, timing figure, or address in this document should ever be copied into your design. They appear only as *worked examples with invented values*, clearly marked. Real values come from the datasheet for your exact part number, under stated temperature and voltage conditions.
2. **Register names.** Generic register models are named `FLASH_CMD`, `FLASH_STATUS`, etc. These are pedagogical fictions. No real chip uses them. That is the point: you learn the *shape* of a flash controller, then map it onto whatever names your vendor chose.

### A note on your uploaded document

You referenced an attached Renesas RL78/I1C flash chapter. **No file was attached** to this conversation — the upload area was empty. Nothing here depends on it. If you attach it later, the device-specific layer can be added on top of this conceptual foundation, which is the correct order anyway.

---

## 0.2 THE KNOWLEDGE MAP

Every chapter attaches to this tree. Return here when you lose the thread.

```
FLASH MEMORY
│
├── 1. PHYSICS ..................................... Part I
│      stored charge · threshold voltage · tunnelling
│      floating gate vs charge trap · sensing
│      retention · disturb · wear-out
│
├── 2. ARRAY ORGANISATION ......................... Part I
│      NOR (random read, XIP) vs NAND (page/block)
│      bits per cell: SLC / MLC / TLC / QLC
│      why embedded flash ≠ SSD flash
│
├── 3. THE MACHINE ................................ Part II
│      regions: code / data / info / config / OTP
│      banks · sectors · flash word · ECC word
│      controller = command state machine
│      read path: wait states, prefetch, cache
│      the three execution models (XIP / copy / hybrid)
│
├── 4. THE OPERATIONS ............................. Part II
│      read   — sensing + bus transaction
│      program — the one-way bit rule, granularity
│      erase  — why granularity is asymmetric
│      endurance / retention as an engineering budget
│
├── 5. PERSISTENT DATA ............................ Part III
│      EEPROM emulation · records · CRC · sequence
│      wear levelling · garbage collection
│      power-fail atomicity · torn writes · commit
│
├── 6. FIRMWARE & BOOT ............................ Part IV
│      reset → vector table → linker → image
│      bootloader · image header · validation
│      RAM-resident code · interrupts · watchdog
│      update topologies: single / dual / A-B / swap
│
├── 7. INTEGRITY & SECURITY ....................... Part V
│      ECC: parity → Hamming → SEC-DED
│      protection ≠ cryptography ≠ secure boot
│      config/option regions · debug lock · RoT
│
├── 8. BEYOND THE MCU ............................. Part VI
│      SPI / QSPI / OSPI external NOR
│      NAND / eMMC / UFS · FTL
│      MCU boot vs MPU boot chain
│
└── 9. PRACTICE ................................... Part VII
       datasheet protocol · debugging · production
       capstone architecture · labs · exam
```

---

## 0.3 THE TWENTY QUESTIONS

This is the deliverable. Everything else is scaffolding to make these questions meaningful.

When you meet an unfamiliar MCU, answer these **before writing a line of code**:

**Geometry**
1. Smallest **erase** unit?
2. Smallest **program** unit, and its alignment rule?
3. Can a location be programmed twice without erasing? Under what constraint?
4. What is the **erased state**, and is it deterministic?

**Budget**
5. Endurance — at what temperature, for what retention?
6. Retention — at what temperature, after how many cycles?
7. Worst-case erase time? Worst-case program time?

**Concurrency**
8. Can I **read** flash while writing flash? Same bank? Other bank?
9. Can I **execute** while programming? From where?
10. What happens to **interrupts** during an operation?

**Failure**
11. What happens if **power fails** mid-erase or mid-program?
12. Is there **ECC**? What granularity? What happens on an uncorrectable error?
13. What are the **voltage and clock** constraints during programming?

**System**
14. Where does the CPU fetch its **initial SP and reset vector**?
15. Where does the **vector table** live after relocation?
16. What **protects the bootloader** from the application?
17. What **config/option region** exists, and what bricks the part if corrupted?
18. What **programming interfaces** exist (debug port, serial loader, vendor tool)?
19. What does the **errata** say about the flash controller?

**The one that matters**
20. **What am I assuming that the datasheet does not guarantee?**

---

---


---

## CONTENTS

**PART I — FOUNDATIONS**

- Chapter 1 — What Non-Volatile Memory Actually Is
- Chapter 2 — The Physics: How a Transistor Remembers
- Chapter 3 — Array Organisation: NOR, NAND, and Bits per Cell

**PART II — THE MACHINE**

- Chapter 4 — The MCU Flash Subsystem
- Chapter 5 — The Flash Controller as a State Machine
- Chapter 6 — Reading, Wait States, and Execution
- Chapter 7 — Programming
- Chapter 8 — Erasing
- Chapter 9 — Endurance, Retention, and the Lifetime Budget

**PART III — PERSISTENT DATA**

- Chapter 10 — EEPROM Emulation
- Chapter 11 — Wear Levelling and Garbage Collection
- Chapter 12 — Power Failure

**PART IV — FIRMWARE, BOOT, AND UPDATE**

- Chapter 13 — Reset, Vector Tables, and the Linker
- Chapter 14 — Bootloader Architecture
- Chapter 15 — Firmware Images and Validation
- Chapter 16 — RAM-Resident Code, Interrupts, and the RTOS
- Chapter 17 — Firmware Update Topologies

**PART V — INTEGRITY AND SECURITY**

- Chapter 18 — Error Correcting Codes
- Chapter 19 — Security and Configuration Regions

**PART VI — BEYOND THE MICROCONTROLLER**

- Chapter 20 — External Flash, Serial Interfaces, and Storage Patterns
- Chapter 21 — MCU Flash versus MPU Storage

**PART VII — PRACTICE**

- Chapter 22 — Reading a Datasheet, and the Variation Axes
- Chapter 23 — Debugging Flash Failures
- Chapter 24 — Production, Environment, and Safety
- Chapter 25 — Capstone: A Complete Storage Architecture

**APPENDIX A — THE LAB PROGRAMME**


**APPENDIX B — THE FLASH ENGINEER CHEAT SHEET**


**APPENDIX C — THE EXAMINATION**


**APPENDIX D — ANSWER KEY**


**APPENDIX E — GLOSSARY**


**APPENDIX F — ON SOURCES**


---

# PART I — FOUNDATIONS

---

# Chapter 1 — What Non-Volatile Memory Actually Is

## 1.1 The definition, stated precisely

**Non-volatile** means: the stored state survives removal of supply power.

That is the *only* thing the word promises. It does not promise:

- that the state survives **forever** (it does not — retention is finite and temperature-dependent)
- that the state can be **rewritten** (OTP and mask ROM cannot)
- that rewriting is **cheap** (flash erase is slow and destructive to the cell)
- that rewriting is **unlimited** (endurance is finite)
- that the memory is **byte-writable** (most flash is not, in the way you expect)

**[PRINCIPLE]** Every non-volatile technology is a trade among five axes: *retention, endurance, write granularity, write energy/time, and cost per bit.* No technology wins all five. Flash won the market because it wins **cost per bit** decisively while being merely adequate on the rest.

Internalise that sentence. Almost every irritating property of flash — erase-before-write, large erase blocks, wear limits, the need for ECC, the need for bootloaders that survive power loss — is a *consequence of optimising for cost per bit*.

## 1.2 The memory taxonomy

Two independent axes: **volatility** and **writability**.

```
                     VOLATILE                    NON-VOLATILE
                 ┌──────────────┐            ┌──────────────────────┐
 Read-only       │      —       │            │  Mask ROM            │
                 │              │            │  OTP / eFuse         │
                 ├──────────────┤            ├──────────────────────┤
 Read-write      │  SRAM        │            │  EEPROM              │
 (cheap writes)  │  DRAM        │            │  FRAM  MRAM          │
                 │              │            │  battery-backed SRAM │
                 ├──────────────┤            ├──────────────────────┤
 Read-write      │      —       │            │  FLASH (NOR / NAND)  │
 (expensive,                                 │                      │
  block-erased)                              └──────────────────────┘
```

### 1.2.1 The comparison table

| Technology | Volatile? | Write granularity | Erase needed? | Endurance (order) | Typical embedded role |
|---|---|---|---|---|---|
| **SRAM** | Yes | Byte/word | No | Unlimited | Working memory, stacks, buffers |
| **DRAM** | Yes (needs refresh) | Byte/word | No | Unlimited | Bulk RAM on application processors |
| **Mask ROM** | No | Not writable | — | 0 writes | Boot ROM burned at fabrication |
| **OTP / eFuse** | No | Bit (one direction, once) | Impossible | 1 write per bit | Device ID, keys, lifecycle, trim |
| **EEPROM** | No | Byte | Internally, per byte | High (≥10⁵ typical) | Small config; increasingly displaced |
| **NOR flash** | No | Word / page | Yes, per sector | Moderate | Code storage, XIP, boot |
| **NAND flash** | No | Page | Yes, per block | Lower per cell | Bulk storage, filesystems |
| **FRAM** | No | Byte | No | Very high | Fast, frequent-write NVM |
| **MRAM/MRAM-class** | No | Byte/word | No | Very high | Emerging unified memory |
| **Battery-backed SRAM** | No*, while battery lives | Byte | No | Unlimited | RTC domain, tamper-sensitive state |

**[VARIES]** Every endurance column entry above is an order-of-magnitude sketch. Actual figures span decades between parts and are datasheet parameters.

### 1.2.2 The distinctions that engineers actually confuse

**Flash vs EEPROM.** Classical EEPROM erases at byte granularity — the erase circuitry is replicated per byte, which costs die area. Flash amortises erase circuitry over a large block. That is the entire difference in one sentence: **flash is EEPROM with the erase granularity traded away for density**. This is why "EEPROM emulation on flash" (Chapter 10) is such a universal embedded topic — you are buying back, in software, the property that was sold off in silicon.

**Flash vs FRAM.** FRAM stores state as a ferroelectric polarisation, not as trapped charge. There is no erase step and no high-voltage programming; a write is roughly as cheap as a RAM write. The consequences are architectural, not incremental: an FRAM-based MCU does not need EEPROM emulation, does not need wear levelling for ordinary config data, and does not have "erase-in-progress" hazards. It also typically has lower density and different read-disturb behaviour. **[INFERENCE]** If your product's dominant pain is *frequent small persistent writes*, changing memory technology is often a better answer than a cleverer wear-levelling layer.

**Flash vs OTP/eFuse.** OTP is not "flash you can only write once" — it is usually a physically different structure (an antifuse or a metal/poly fuse) whose change is irreversible by construction. That irreversibility is the *feature*: it is what makes "secure boot enable" and "debug permanently disabled" trustworthy. A bit that can be un-set is a bit an attacker might un-set.

**Flash vs battery-backed SRAM.** BBSRAM has unlimited endurance and byte writes, which makes it superb for high-rate counters and tamper state. It fails when the battery fails, and it is not a substitute for firmware storage. **[INFERENCE]** In metering, industrial, and automotive designs, a common and good pattern is: *hot, high-rate state in BBSRAM or FRAM; consolidated snapshots into flash at low rate.* You get endurance where you need it and durability where you need that.

## 1.3 Where non-volatile memory sits in a system

```
   ┌──────────────────────────────────────────────────────┐
   │                      CPU CORE                        │
   └──────────┬───────────────────────────┬───────────────┘
              │ instruction fetch         │ data access
              ▼                           ▼
   ┌──────────────────────────────────────────────────────┐
   │                    BUS FABRIC                        │
   └───┬─────────────┬──────────────┬─────────────┬───────┘
       │             │              │             │
       ▼             ▼              ▼             ▼
   ┌────────┐   ┌─────────┐   ┌───────────┐  ┌──────────┐
   │  SRAM  │   │  FLASH  │   │PERIPHERALS│  │ EXTERNAL │
   │        │   │ I/F +   │   │           │  │ MEMORY   │
   │        │   │CONTROLLER│  │           │  │ I/F      │
   └────────┘   └────┬────┘   └───────────┘  └────┬─────┘
                     ▼                            ▼
                ┌─────────┐                  ┌──────────┐
                │  FLASH  │                  │ EXTERNAL │
                │  ARRAY  │                  │  FLASH   │
                └─────────┘                  └──────────┘
```

**[PRINCIPLE]** Note the asymmetry: SRAM hangs off the bus almost directly, while flash always sits behind *two* layers — a read interface (fast path, wait states, prefetch, cache) and a controller (slow path, program/erase state machine). Nearly every surprising behaviour in flash programming comes from the fact that **the fast read path and the slow modify path share one physical array**.

That single sentence explains: why erase blocks reads, why code must sometimes run from RAM, why cache must be invalidated after an update, and why "read-while-write" is a marketing feature rather than a default.

## 1.4 Chapter exercises

1. Your design writes a 4-byte counter 20 times per second and must survive power loss. Rank SRAM, BBSRAM, FRAM, NOR flash, EEPROM for this job, and justify the ranking on the five-axis trade.
2. Explain in one sentence why an eFuse is a better place for a "secure boot enabled" flag than a flash byte.
3. A colleague says "we'll just use the internal EEPROM." The MCU has none, only flash. What are you actually being asked to build?

---

# Chapter 2 — The Physics: How a Transistor Remembers

You do not need to design flash cells. You need this chapter because **every** system-level property in the rest of the document — erase granularity, endurance, retention, ECC, disturb, temperature sensitivity, the erase-before-write rule — is a direct consequence of the physics. Skip it and you will be memorising rules instead of deriving them.

## 2.1 Stage 1 — Intuition

A flash cell is a transistor with a **hidden pocket of trapped electrons** inside its gate stack.

- Put electrons in the pocket → the transistor becomes *harder* to switch on.
- Take electrons out → it becomes *easier* to switch on.
- The pocket is electrically isolated, so the electrons stay for years without power.
- To read: apply a fixed voltage and ask "did it switch on?" The answer encodes the bit.

That is genuinely the whole idea. Everything else is engineering around three inconvenient truths: *getting charge in and out requires abusive voltages*, *the isolation is not perfect*, and *the abuse damages the isolation a little every time*.

## 2.2 Stage 2 — The device

### 2.2.1 Baseline: the MOSFET

An n-channel MOSFET conducts between source and drain when the control-gate voltage exceeds the **threshold voltage**, V<sub>t</sub>. V<sub>t</sub> is a property of the device's construction and of any charge sitting between the gate and the channel.

```
        CONTROL GATE
   ═══════════════════════
        gate oxide
   ───────────────────────
   [n+]     channel    [n+]
  SOURCE               DRAIN
            substrate
```

### 2.2.2 The flash cell: insert a charge store

Between the control gate and the channel, add an electrically isolated storage layer:

```
        CONTROL GATE
   ═══════════════════════
       inter-poly / blocking oxide
   ┌───────────────────────┐
   │  CHARGE STORAGE LAYER │  ← floating gate (conductive)
   └───────────────────────┘     or charge-trap layer (insulating, e.g. nitride)
       tunnel oxide  (very thin)
   ───────────────────────
   [n+]     channel    [n+]
```

Electrons parked in the storage layer partially screen the control gate from the channel. The transistor now needs a *higher* control-gate voltage to turn on: **V<sub>t</sub> has shifted up**.

**Reading = measuring V<sub>t</sub>, indirectly, by applying a fixed read voltage and sensing current.**

### 2.2.3 Floating gate vs charge trap — a real distinction, not a synonym

**[PRINCIPLE]** These are two different technologies. Do not conflate them.

| | **Floating gate (FG)** | **Charge trap (CT)** |
|---|---|---|
| Storage layer | Conductive polysilicon | Insulating dielectric (commonly silicon nitride) |
| Charge behaviour | Electrons are mobile within the layer, forming a shared pool | Electrons are immobilised in discrete traps |
| Single tunnel-oxide defect | Can drain the **entire** stored charge — catastrophic for that cell | Leaks only charge near the defect — graceful |
| Scaling | Harder; cell-to-cell coupling grows as cells shrink | Better suited to aggressive scaling and 3D stacking |
| Where you meet it | Much classic NOR and planar NAND; many embedded MCU flash macros | Modern 3D NAND; some modern embedded macros |

**[INFERENCE]** For firmware work, the practical consequence is about *failure shape*: FG cells tend toward abrupt, whole-cell loss; CT cells tend toward gradual V<sub>t</sub> drift. This is one reason ECC strategy and refresh policy differ between memory generations. It is also why you should never assume "a worn flash bit fails to zero" or any other tidy model — **ask the ECC, not your intuition**.

## 2.3 Stage 3 — Moving charge: the two mechanisms

Getting electrons across a good insulator requires help.

### 2.3.1 Fowler–Nordheim (F-N) tunnelling

Apply a strong electric field across the thin tunnel oxide. The field bends the oxide's energy barrier into a triangle thin enough for electrons to **tunnel** through quantum-mechanically.

- Field-driven, low current, relatively energy-efficient per bit
- Used for **erase** in essentially all flash, and for **program** in NAND and in many embedded macros
- Requires high voltage across a very thin oxide — the direct cause of oxide wear-out

### 2.3.2 Channel hot-electron injection (CHE / HCI)

Run a substantial current through the channel with a high drain voltage. Some electrons acquire enough kinetic energy ("hot") to jump the oxide barrier into the storage layer.

- Current-driven, fast per cell, but power-hungry
- Historically used for **program** in NOR flash
- The high current is one reason NOR programming is bit-parallel-limited and why programming a large image is not instantaneous

**[VARIES]** Which mechanism a given macro uses for program and for erase, and in which direction V<sub>t</sub> moves, is a design choice of the memory vendor. **Do not assume.** Some macros erase to all-ones; some to all-zeros; some (with ECC, see §2.7) have no architecturally meaningful "erased pattern" at all.

### 2.3.3 Where does the high voltage come from?

Your MCU runs on, say, a low single-digit supply. Tunnelling needs much more.

**Charge pumps.** An on-chip switched-capacitor network multiplies the supply up to the programming/erase rail, then regulates it. This has consequences you will meet in the lab:

**[PRINCIPLE]**
- Flash program/erase draws **markedly more current** than idle operation, in bursts. Your power budget and decoupling must account for it.
- Charge-pump output depends on the input supply. Below some supply threshold the pump cannot reach the required rail — hence **minimum programming voltage** specs, often stricter than the minimum operating voltage.
- Pumps need time to ramp. This is part of why a program operation has fixed setup overhead independent of data size.
- Pump behaviour is temperature-sensitive, which is part of why programming is sometimes restricted over the full temperature range.

## 2.4 Stage 4 — The array and how a bit is actually read

Cells are not addressed individually by wires; they sit in a matrix.

```
                 BIT LINES (columns, carry sensed current)
                 BL0      BL1      BL2      BL3
                  │        │        │        │
  WL0  ───────────┼──[C]───┼──[C]───┼──[C]───┼──[C]──
                  │        │        │        │
  WL1  ───────────┼──[C]───┼──[C]───┼──[C]───┼──[C]──
                  │        │        │        │
  WL2  ───────────┼──[C]───┼──[C]───┼──[C]───┼──[C]──
                  │        │        │        │
  (word lines = rows, drive control gates)
                  ▼        ▼        ▼        ▼
              ┌───────────────────────────────────┐
              │        SENSE AMPLIFIERS           │
              │   (compare against reference)     │
              └───────────────┬───────────────────┘
                              ▼
                        raw data bits
```

- **Word line (WL)** — drives the control gates of a row. Selecting a word line applies the read voltage to every cell in that row.
- **Bit line (BL)** — a column. Whether current flows on it reveals the selected cell's state.
- **Source line (SL)** — the common return path.
- **Sense amplifier** — the analogue heart of the read. It compares the selected cell's conduction against a **reference** (a reference cell or a current reference) and produces a clean digital 1 or 0.

**[PRINCIPLE]** *A flash read is an analogue measurement converted to digital.* It has a settling time. That settling time, plus decode and bus overhead, is the flash access time — and it is why fast CPUs need wait states (Chapter 6).

## 2.5 Threshold-voltage distributions — the picture to hold in your head

No two cells are identical. Programming a population of cells produces a *distribution* of V<sub>t</sub>, not a single value.

```
 number
 of cells
    ▲
    │   ┌───┐                          ┌───┐
    │   │ E │                          │ P │
    │   │   │                          │   │
    │  ┌┘   └┐                        ┌┘   └┐
    │ ─┘     └────────┬───────────────┘     └──►  Vt
                      │
                 READ REFERENCE
              ("margin window" on both sides)
```

E = erased population. P = programmed population. The gap between them is the **read margin window**.

**[PRINCIPLE]** Everything that degrades flash is best understood as *the distributions moving or widening until they touch the read reference*:

| Mechanism | Effect on the picture |
|---|---|
| **Retention loss** | Charge slowly leaks; the programmed distribution drifts back toward erased. Accelerated by temperature. |
| **Endurance wear** | Repeated tunnelling damages the oxide and traps charge in it; distributions shift and widen; the window narrows. |
| **Read disturb** | Reading a row applies voltage to unselected cells; over enormous read counts their V<sub>t</sub> creeps. |
| **Program disturb** | Programming one cell exposes neighbours on shared lines to partial fields. |
| **Temperature** | Shifts the electrical operating point at read time *and* accelerates leakage over life. |

**[INFERENCE]** This is the single most useful mental model in the whole subject. When you later read "endurance 100k cycles with 10-year retention at 55 °C," you should now hear: *"after 100k cycles the window has narrowed by a characterised amount, and the residual window is sufficient that the distributions will not cross the reference for 10 years at 55 °C."* Endurance and retention are not two independent numbers — **they are two coordinates on one trade curve.** Cycle a part harder and its retention shortens. That is why datasheet endurance is always quoted *with* a retention condition, and why quoting one without the other is meaningless.

## 2.6 The asymmetry that defines flash programming

**[PRINCIPLE]** In a typical flash macro, the two directions of V<sub>t</sub> movement are not symmetric:

- Moving cells **one way** can be done selectively, per addressed unit, using the program mechanism.
- Moving cells **the other way** is done in bulk, over a whole sector or block, using the erase mechanism — because the erase mechanism operates through structures (shared wells, shared source lines) that are inherently shared across many cells.

This is the physical root of the two rules that dominate firmware design:

1. **Erase granularity is much coarser than program granularity.**
2. **Program can only move bits in one direction; recovering the other direction requires an erase.**

**[VARIES]** *Which* logical direction is "free" — whether the erased state reads as all-ones and programming clears bits, or the reverse — is device-specific, and on ECC-protected macros the question can be meaningless (see below). Always check.

## 2.7 A modern complication: ECC destroys the naive bit model

Many contemporary embedded flash macros store an **ECC code alongside every flash word** and compute it in hardware on write (Chapter 18 covers ECC properly).

The consequences are architectural, and they surprise experienced engineers:

**[PRINCIPLE]**
- The **write unit becomes the ECC word**, not the byte. You cannot program 1 byte and later program the adjacent byte in the same ECC word, because the second write would need to change ECC bits that are already programmed.
- Therefore many ECC-protected macros enforce **write-once-per-word-between-erases**. The old "you can always clear more bits later" trick stops working.
- The **erased-state read value may not be deterministic**, because the check bits for an erased data pattern may not correspond to the erased check-bit pattern. Some devices explicitly document that reading erased flash returns an unspecified value, and provide a dedicated **blank-check** command instead.
- Some devices expose the array through **two address aliases**: one that returns ECC-corrected data, and one that returns raw uncorrected data for diagnostics.

**[INFERENCE]** This single feature invalidates a large amount of folk wisdom and old example code. If your target has ECC on flash, treat "I'll just clear one more status bit in place" as *forbidden until proven otherwise*. Design your record formats so that each state transition writes a **whole fresh ECC word** (Chapter 10).

## 2.8 Three views of one operation

As promised in the teaching plan, here is the template applied to **erase**:

| View | Erase is… |
|---|---|
| **Beginner** | Resetting a region of flash back to its blank state so it can be written again. |
| **Engineer** | A controller-sequenced operation over a fixed-size sector or block, taking milliseconds, during which the array is unavailable for normal reads and after which a verify pass confirms every cell moved. |
| **Silicon** | Application of a high field, generated by on-chip charge pumps, across the tunnel oxides of an entire shared-well region, moving the V<sub>t</sub> distribution of all those cells past the erase verify level — while consuming a quantum of the oxide's finite lifetime. |

## 2.9 Chapter exercises

1. Draw the V<sub>t</sub> distribution picture and mark where a cell sits after (a) erase, (b) program, (c) 10 years at high temperature, (d) 100,000 cycles. Explain why (c) and (d) interact.
2. Why does a flash macro with hardware ECC often forbid programming the same word twice between erases? Answer in terms of the check bits.
3. Your board browns out to a low voltage but the CPU keeps running. Why might a flash program operation fail *silently* rather than throwing an error?
4. Explain why a datasheet quoting "1,000,000 cycles" without a retention condition is not usable information.

---

# Chapter 3 — Array Organisation: NOR, NAND, and Bits per Cell

## 3.1 The organising question

Cells must be wired into a matrix. There are two dominant ways to wire them, and the choice determines almost everything at system level.

## 3.2 NOR organisation

Each cell connects **directly** to a bit line (roughly in parallel, like the pull-downs of a NOR gate — hence the name).

```
   BL
   │
   ├──[cell]── SL      ← each cell independently pulls the bit line
   ├──[cell]── SL
   ├──[cell]── SL
```

**Consequences [PRINCIPLE]:**

| Property | Why it follows from the wiring |
|---|---|
| **True random access** | Any cell can be sensed independently; no need to traverse neighbours. |
| **Low read latency** | One decode + one sense. Suitable for instruction fetch. |
| **Execute-in-place (XIP)** | Because random reads are fast and byte/word addressable, the CPU can fetch instructions directly. |
| **Fine program granularity** | Word or page programming is natural. |
| **Poor density** | Every cell needs its own bit-line contact — large cell area. |
| **Slow, coarse erase** | Erase sectors are large (typically kilobytes to tens of kilobytes) and take milliseconds. |
| **Lower capacity, higher cost/bit** | Direct result of the area cost. |

**Role:** code storage, boot storage, embedded MCU flash, external XIP flash.

## 3.3 NAND organisation

Cells are wired in **series strings** (like a NAND gate's stack), with select transistors at each end.

```
   BL
   │
  [SEL]
   │
  [cell] ─ WL0
  [cell] ─ WL1
  [cell] ─ WL2
     ⋮        ← a string of many cells shares one bit-line contact
  [cell] ─ WLn
   │
  [SEL]
   │
   SL
```

**Consequences [PRINCIPLE]:**

| Property | Why it follows from the wiring |
|---|---|
| **Very high density** | One bit-line contact amortised over a whole string → small effective cell area. |
| **No true random access** | To read one cell you must drive all other cells in the string into pass-through and sense the whole string. |
| **Page-granular reads** | The natural read unit is a whole page (thousands of bytes) into a page register. |
| **High read latency, high throughput** | First byte is slow; streaming is fast. |
| **Page program, block erase** | Program a page; erase a block of many pages. |
| **ECC is mandatory, not optional** | Raw bit error rate is high by design; the controller must correct. |
| **Bad blocks exist from the factory** | Perfect yield is not economic at this density; blocks are marked bad and must be skipped. |
| **Not XIP-capable in the normal sense** | You cannot fetch instructions randomly from a page-oriented device with millisecond-class first-access latency. |

**Role:** bulk storage — filesystems, media, OS images, logs.

## 3.4 The comparison that matters at design time

| Axis | NOR | NAND |
|---|---|---|
| Random read latency | Low | High (page load), then fast streaming |
| Sequential throughput | Moderate | High |
| Program granularity | Word / small page | Page |
| Erase granularity | Sector (small–medium) | Block (large) |
| Erase time | Milliseconds | Milliseconds, but per much larger block |
| ECC | Often internal to the MCU macro; sometimes none on small parts | **Always required**, often multi-bit |
| Factory defects | Not expected in usable array | **Expected** — bad-block table required |
| Cost per bit | High | Low |
| XIP | Yes | No (needs shadowing to RAM) |
| Typical capacity in embedded | KB → tens of MB | Hundreds of MB → many GB |

**[PRINCIPLE]** The rule of thumb that actually holds: **NOR is for code you execute; NAND is for data you stream.** Systems that need both usually have both, or use a managed NAND device that hides the complexity.

## 3.5 Managed vs raw NAND — a critical distinction

**Raw NAND** exposes pages, blocks, bad blocks, and error correction responsibilities to your software. You must implement (or license) a **Flash Translation Layer**: bad-block management, ECC, wear levelling, logical-to-physical mapping, power-fail-safe metadata.

**Managed NAND** (the eMMC/UFS/SD family, and "managed" SPI NAND) puts a controller *inside the package* that does all of that and presents a clean block-device interface.

**[INFERENCE]** For most product teams, raw NAND is a mistake unless you have a strong reason and a strong storage engineer. The FTL is where the hard bugs live, and vendors have spent decades on theirs. Choosing managed NAND converts a research project into a component selection.

## 3.6 The blurring of boundaries

Modern reality is messier than the clean NOR/NAND split:

- **Serial NOR with high-speed interfaces** (quad/octal, DDR) achieves throughput once associated with parallel buses, while retaining XIP.
- **Serial NAND** exists with SPI-like interfaces, trading NAND density into small pin counts, but still requiring ECC and bad-block handling.
- **MCU internal flash with a read accelerator and cache** behaves, from the CPU's viewpoint, closer to a slow SRAM than to a "memory device."
- **Managed NAND** presents a block interface that hides NAND entirely — until you care about power-loss behaviour, at which point it stops hiding.

**[PRINCIPLE]** The useful invariant across all of it: *somewhere in the stack, something is doing erase-before-write, wear management, and error correction.* Your job as an architect is to know **where** that something lives — in silicon, in the packaged controller, in a library, or in code you must write.

## 3.7 Bits per cell: SLC / MLC / TLC / QLC

Instead of two V<sub>t</sub> populations, use more.

```
 SLC  (1 bit/cell)   [E]────────────────────────[P]        1 reference
 MLC  (2 bits/cell)  [11]────[10]────[00]────[01]          3 references
 TLC  (3 bits/cell)  8 populations                          7 references
 QLC  (4 bits/cell)  16 populations                        15 references
```

The same physical V<sub>t</sub> range is subdivided further.

**[PRINCIPLE]** Every added bit per cell:

| Effect | Reason |
|---|---|
| Increases density / lowers cost per bit | More bits in the same silicon |
| **Narrows margins** | The available V<sub>t</sub> window is split into more, tighter bands |
| **Reduces endurance** | Narrow bands tolerate less oxide degradation before overlapping |
| **Reduces retention** | Less drift is required to cross a reference |
| **Increases programming time and complexity** | Placing a cell in a narrow band requires iterative program-and-verify steps |
| **Increases ECC strength required** | More raw errors to correct |
| **Increases read-disturb sensitivity** | Smaller creep is enough to cause a misread |
| **Increases write amplification** | More complex internal data movement and refresh |

### 3.7.1 Why embedded MCU flash is a different animal

**[PRINCIPLE]** MCU internal flash is normally **SLC-class NOR-style embedded flash**, optimised for:

- deterministic, low-latency random read (it holds the code the CPU is executing)
- reliability over industrial or automotive temperature ranges
- long retention, often ten years or more
- modest capacity (kilobytes to a few megabytes)

Whereas SSD/phone NAND is optimised for cost per bit at enormous capacity, and accepts high raw error rates, heavy ECC, background refresh, and complex FTLs to compensate.

**[INFERENCE]** This is why intuitions imported from the SSD world mislead embedded engineers, and vice versa. "Flash wears out after a few thousand writes" is a TLC/QLC NAND statement. "Flash is byte-addressable and executes code" is an embedded NOR statement. Both are true; neither is universal. Always ask which flash you are talking about.

## 3.8 Chapter exercises

1. Explain, from the wiring diagrams alone, why NAND cannot support execute-in-place while NOR can.
2. A vendor advertises a serial NAND at one-fifth the price per bit of serial NOR. List five engineering costs you would incur by switching. (Aim for costs that are *software* costs.)
3. Why does raw bit error rate rise as bits-per-cell increases, even with identical process technology?
4. Your MCU has 256 KB of internal flash and you need to log 2 GB of sensor data per month. Sketch the memory architecture and justify the technology choice for each element.

---

---

# PART II — THE MACHINE

---

# Chapter 4 — The MCU Flash Subsystem

## 4.1 The universal block diagram

**[PRINCIPLE]** Almost every MCU with embedded flash has this structure, whatever the vendor calls the boxes:

```
   ┌──────────────────────────────────────────────────────────┐
   │                        CPU CORE                          │
   └──────────┬──────────────────────────────┬────────────────┘
              │ instruction fetch            │ data load/store
              ▼                              ▼
   ┌──────────────────────────────────────────────────────────┐
   │                     BUS FABRIC / MATRIX                  │  ← DMA also arbitrates here
   └──────────┬──────────────────────────────┬────────────────┘
              │  FAST READ PATH              │  SLOW CONTROL PATH
              ▼                              ▼
   ┌──────────────────────────┐   ┌──────────────────────────┐
   │   FLASH READ INTERFACE   │   │    FLASH CONTROLLER      │
   │  ┌────────────────────┐  │   │  ┌────────────────────┐  │
   │  │ wait-state logic   │  │   │  │ command registers  │  │
   │  │ prefetch buffer    │  │   │  │ address/data regs  │  │
   │  │ instruction cache  │  │   │  │ status/error flags │  │
   │  │ ECC decoder        │  │   │  │ protection logic   │  │
   │  └────────────────────┘  │   │  │ timing generator   │  │
   └────────────┬─────────────┘   │  │ charge-pump ctrl   │  │
                │                 │  │ ECC encoder        │  │
                │                 │  └─────────┬──────────┘  │
                │                 └────────────┼─────────────┘
                │                              │
                ▼                              ▼
   ┌──────────────────────────────────────────────────────────┐
   │                      FLASH ARRAY                         │
   │   bank 0  [ sector | sector | sector | … ]                │
   │   bank 1  [ sector | sector | sector | … ]                │
   │           rows / word lines / bit lines / sense amps      │
   └──────────────────────────────────────────────────────────┘
```

**[PRINCIPLE]** Two paths, one array. This is the source of nearly every hazard:

- The **read path** is optimised for latency and is essentially transparent — the CPU just reads addresses.
- The **control path** is a peripheral you talk to with registers, and it *takes the array away from the read path* while it works.

Ask, for your device: **what happens to the read path while the control path is busy?** The answer is one of the four architectures in §6.6, and it determines whether you need RAM-resident code.

## 4.2 The region taxonomy

**[PRINCIPLE]** MCU flash is almost never one homogeneous block. It is partitioned in silicon into regions with different properties. The *names* vary wildly between vendors; the *roles* are remarkably consistent:

| Generic role | Common vendor names you'll meet | Characteristics |
|---|---|---|
| **Main / code flash** | main, program flash, P-flash, code flash, MAIN | Largest region; holds executable code and constants; coarse erase; may be split into banks |
| **Data flash** | data flash, D-flash, FlexNVM, EEPROM region, info | Optimised for data: often finer erase granularity, higher endurance, and *sometimes* independently accessible while code executes |
| **Information / factory** | info memory, engineering flash, factory row | Holds trim, calibration, unique ID, written at manufacture; often partly locked |
| **Configuration / option** | option bytes, config words, NONMAIN, fuse row, boot config | Holds boot policy, protection, watchdog/BOR defaults, debug policy. **Highest-risk region in the device.** |
| **Boot / bootloader region** | boot cluster, boot block, boot sector | A protected region containing first-stage code; often has special swap/protection hardware |
| **OTP** | OTP, eFuse, one-time programmable | Irreversible; identity, keys, lifecycle state |

### 4.2.1 The three properties that differ between regions

When you read a reference manual, extract these per region:

1. **Erase granularity** — often finer in data flash than main flash
2. **Endurance** — often much higher in data flash than main flash
3. **Accessibility during operations** — data flash is sometimes on a *separate macro*, allowing code execution from main flash while data flash is written. This is the single most valuable feature for persistent-data-heavy products.

**[VARIES]** Whether data flash is a separate macro or just a differently-labelled part of the same array is device-specific and has enormous design consequences. Find out early.

## 4.3 Reading a memory map

**[PRINCIPLE]** A memory map is a table of *address ranges → what responds at those addresses*. Here is a generic, symbolic one. **No real addresses — deliberately.**

```
ADDRESS RANGE            REGION                    NOTES
─────────────────────────────────────────────────────────────────────────
CODE_BASE + 0x0000       Vector table              Initial SP + reset vector
CODE_BASE + 0x0???       Bootloader                Write-protected in production
CODE_BASE + BL_SIZE      Application slot A        Relocated vector table here
   ⋮                     Application slot B        (if A/B update is used)
   ⋮                     Read-only constants
CODE_END                 ── end of main flash ──
─────────────────────────────────────────────────────────────────────────
CODE_BASE_ALIAS          Same array, alternate view  e.g. uncorrected/raw ECC
─────────────────────────────────────────────────────────────────────────
DATA_FLASH_BASE          Data flash / EEPROM region  Finer erase, higher endurance
DATA_FLASH_END
─────────────────────────────────────────────────────────────────────────
INFO_BASE                Factory/calibration         Partly locked
CONFIG_BASE              Option/config region        ⚠ corrupting this can brick
OTP_BASE                 One-time programmable       Irreversible
─────────────────────────────────────────────────────────────────────────
ROM_BASE                 Vendor boot ROM             Mask ROM; serial loader
─────────────────────────────────────────────────────────────────────────
SRAM_BASE                SRAM                        .data, .bss, stack, heap
                                                     + RAM-resident flash routines
─────────────────────────────────────────────────────────────────────────
PERIPH_BASE              Peripheral registers        Includes FLASH CONTROLLER
─────────────────────────────────────────────────────────────────────────
EXT_MEM_BASE             External memory window      QSPI/OSPI memory-mapped region
```

### 4.3.1 Six things to notice on any real memory map

1. **Aliases.** The same physical array may appear at more than one address, with different semantics (cached/uncached, ECC-corrected/raw, secure/non-secure). Reading the "wrong" alias is a classic bug.
2. **The flash controller is a peripheral.** It lives in peripheral space, *not* in flash space. You write registers there to modify the array you are executing from. Contemplate that for a moment — it is the whole hazard in one observation.
3. **Boot ROM is separate.** Vendor boot code is usually mask ROM, not flash, and is what runs *before* your reset vector on many parts.
4. **The config region is small and lethal.** It is often a single sector. Erasing it without rewriting valid contents is a leading cause of dead boards.
5. **Gaps are not free real estate.** Reserved ranges may alias, fault, or be reserved for larger parts in the family.
6. **The map is per-part, not per-family.** Two parts with the same family name routinely differ in flash size, bank count, and region boundaries.

**[INFERENCE]** Build the habit of transcribing the memory map into your project as a single header of named constants, derived from the reference manual, with the document revision in a comment. Every magic address in your code is a future bug.

## 4.4 Banks

A **bank** is an independently-controllable sub-array with its own (or arbitrable) control logic.

**[PRINCIPLE]** Banks exist for two reasons:

1. **Concurrency** — the ability to read (and therefore execute) from bank 0 while bank 1 is being erased or programmed. This is "read-while-write."
2. **Swap** — the ability to remap which bank appears at the boot address, enabling atomic A/B firmware switching (Chapter 17).

**[VARIES]** Bank count, whether read-while-write is supported at all, whether it works across arbitrary bank pairs, and whether address swap exists — all device-specific, and often differ *within* a family based on flash size. Devices exist where:

- there is one bank and any operation stalls the CPU entirely;
- there are two banks and cross-bank execution is supported;
- there are two banks with hardware address swap for firmware update;
- a device *with* the flexible data-flash feature *loses* the bank-swap feature, or vice versa, because they share silicon resources.

That last case is real and catches people. **Check that the features you are relying on coexist on the exact part number you are buying.**

## 4.5 The flash word and the ECC word

**[PRINCIPLE]** Three different "widths" matter and are frequently confused:

| Width | What it is | Typical relationship |
|---|---|---|
| **Bus width** | How many bits the CPU reads per transaction | 32-bit on most modern MCUs |
| **Flash word / program unit** | Smallest quantity the controller can program in one operation | Often *wider* than the bus — e.g. 64 or 128 data bits |
| **ECC word** | The data span protected by one set of check bits | Usually equal to the flash word |

**[PRINCIPLE]** The programming granularity you must respect in software is the **flash word**, and the alignment you must respect is *its* alignment — not the bus width, and definitely not `sizeof(uint32_t)`.

**[INFERENCE]** In practice this means your NVM record structures should be sized as a multiple of the flash word, and your `flash_program()` API should take a length that is a multiple of the flash word and reject anything else. Padding a 20-byte structure to 24 or 32 bytes is not waste — it is correctness.

## 4.6 Chapter exercises

1. Your MCU exposes flash at two address ranges, one described as returning corrected data and one raw. Write down two legitimate uses for the raw alias and one bug it would cause if used by mistake.
2. Explain why the flash controller being a peripheral (not part of the flash array) is the root cause of the "executing from flash while erasing flash" hazard.
3. A part number in a family has FlexNVM-style configurable data flash; another has bank swap. Explain, architecturally, why a vendor might make these mutually exclusive.
4. You must store a 20-byte calibration record. The flash word is 8 bytes. What size do you make the record, and why?

---

# Chapter 5 — The Flash Controller as a State Machine

## 5.1 The mental model

**[PRINCIPLE]** Strip away vendor branding and every flash controller is the same thing:

> **A command-driven state machine that owns a high-voltage analogue sequencer, guarded by an unlock mechanism, that reports progress and errors through a status register.**

You will meet three interface styles, and they are all this same machine:

| Style | How you issue a command | Where you'll see it |
|---|---|---|
| **Register-command** | Write opcode + address + data into dedicated registers, then set a "go" bit | Common on modern 32-bit MCUs |
| **Mode-bit + dummy write** | Set a mode bit (e.g. "erase enable"), then perform a *write to the target address* to trigger it | Common on older/simpler architectures |
| **Library / firmware-mediated** | Call a vendor-supplied library that hides the registers, often because the real sequencer is in on-chip ROM | Common where the sequencer is proprietary or the sequence is timing-critical |

**[INFERENCE]** The third style irritates engineers who want register-level control, but it exists for a good reason: the program/erase sequence is often *not* a simple register poke — it is a timed, voltage-dependent, verify-and-retry algorithm, and vendors would rather ship it correct than document it. When your vendor supplies a flash library, use it. Reverse-engineering the sequencer is a great way to build a product that fails at -40 °C.

## 5.2 The generic register model

The following is a **teaching fiction**. No real device uses these names. Learn the *roles*, then map.

```c
/* GENERIC EXAMPLE — pedagogical, not any real device */

typedef struct {
    volatile uint32_t KEY;      /* unlock: write magic value to enable modification */
    volatile uint32_t CTRL;     /* mode/config: clock divider, protection, IRQ enables */
    volatile uint32_t CMD;      /* command opcode + "go" bit                          */
    volatile uint32_t ADDR;     /* target address within the array                    */
    volatile uint32_t DATA[2];  /* program data, width = flash word                   */
    volatile uint32_t STATUS;   /* BUSY, DONE, and error flags                        */
    volatile uint32_t ERRCLR;   /* write-1-to-clear error flags                       */
    volatile uint32_t PROTECT;  /* per-sector or per-region write protection          */
} flash_ctrl_t;
```

### 5.2.1 Register by register — purpose, fields, reset state, side effects

**KEY / unlock register**

- *Purpose:* prevent a wild pointer or runaway code from erasing the device.
- *Typical field:* a fixed magic constant that must be written before any modifying command.
- *Reset state:* locked.
- *Side effects:* **writing the wrong value is often not a no-op.** On some architectures it triggers a reset or latches a violation flag. This is intentional — a wrong key is evidence of a bug or an attack.
- *Design rule:* your driver should lock the controller again the instant the operation completes. Leaving flash unlocked between operations is a real defect that turns a stack overflow into a bricked unit.

**CTRL / mode register**

- *Purpose:* configure how the sequencer runs.
- *Typical fields:* **clock source and divider** (the flash timing generator often needs a frequency inside a narrow band), interrupt enables, ECC behaviour, cache/prefetch enables.
- *Reset state:* a safe default that may not match your final clock configuration.
- *Side effect that bites people:* **if you change the system clock and forget the flash timing configuration, program/erase becomes out of spec.** The read path usually has wait states you remembered to configure; the *program* path has a timing input you probably forgot. Symptoms: programming works at low clock during bring-up and fails after you enable the PLL.

**CMD / command register**

- *Purpose:* select the operation and start it.
- *Typical opcodes:* program word, erase sector, erase bank, mass erase, blank check, verify, read-with-margin, protection/config commands.
- *Side effect:* writing the "go" bit is the point of no return. On many devices the CPU is **stalled** from that instant if it is fetching from the affected array.

**ADDR / address register**

- *Purpose:* select the target.
- *Field constraint:* **alignment**. The low bits are often ignored or must be zero. An unaligned address either faults, silently truncates, or programs the wrong place — all three behaviours exist in the wild.
- *Trap:* the address expected may be a **physical offset within the array**, not the CPU-visible address. Confusing the two writes to the wrong sector. Always check whether the controller wants `0x0000_1000` or `CODE_BASE + 0x1000`.

**DATA registers**

- *Purpose:* hold the payload for a program operation, one flash word wide.
- *Trap:* you must fill *all* of them, even if you only care about one byte. Leaving stale contents in the upper half programs garbage.

**STATUS register**

- *Purpose:* progress and error reporting. The heart of a correct driver.
- *Typical flags:*

| Flag | Meaning | What your driver must do |
|---|---|---|
| `BUSY` | Sequencer running | Poll until clear, **with a timeout** |
| `DONE` | Command completed | Check *and then check error flags* — done ≠ succeeded |
| `PROT_ERR` | Write/erase protection violation | Report; do not retry blindly |
| `ALIGN_ERR` / `ACCESS_ERR` | Illegal address or sequence | This is a software bug; fail loudly |
| `VERIFY_FAIL` | Cells did not reach target state | Possible wear-out or voltage problem |
| `VOLT_ERR` | Supply out of range during operation | Data integrity is suspect; recovery path |
| `ECC_SEC` / `ECC_DED` | Corrected / uncorrectable error on read | See Chapter 18 |

- **[PRINCIPLE]** *Error flags are usually sticky and must be explicitly cleared before the next command.* A driver that does not clear them will misattribute an old failure to a new operation — or, worse, will have its new command rejected because the controller refuses to start with errors pending.

**PROTECT register**

- *Purpose:* mark regions as non-erasable/non-programmable.
- *Two flavours:* **volatile** (reset each boot; set by the bootloader) and **non-volatile** (stored in the config region; survives reset).
- **[INFERENCE]** Use both. Non-volatile protection on the bootloader region is your defence against a buggy application. Volatile protection, set early in boot, is your defence against a buggy bootloader. Defence in depth costs nothing here.

## 5.3 The canonical command sequence

**[PRINCIPLE]** Whatever the register names, the sequence is always this shape:

```
   ┌─────────────────────────────────────────────┐
   │ 1. Verify preconditions                     │
   │    - supply voltage adequate                │
   │    - clock configured for flash timing      │
   │    - address aligned and in range           │
   │    - target region not protected            │
   └───────────────────┬─────────────────────────┘
                       ▼
   ┌─────────────────────────────────────────────┐
   │ 2. Wait for BUSY clear; clear stale errors  │
   └───────────────────┬─────────────────────────┘
                       ▼
   ┌─────────────────────────────────────────────┐
   │ 3. Enter critical section                   │
   │    - disable interrupts OR relocate ISRs    │
   │    - service watchdog / extend timeout      │
   │    - if required: jump to RAM-resident code │
   └───────────────────┬─────────────────────────┘
                       ▼
   ┌─────────────────────────────────────────────┐
   │ 4. Unlock                                   │
   └───────────────────┬─────────────────────────┘
                       ▼
   ┌─────────────────────────────────────────────┐
   │ 5. Load ADDR, DATA; write CMD + GO          │
   └───────────────────┬─────────────────────────┘
                       ▼
   ┌─────────────────────────────────────────────┐
   │ 6. Poll BUSY with a hard timeout            │
   │    (or wait for completion interrupt)       │
   └───────────────────┬─────────────────────────┘
                       ▼
   ┌─────────────────────────────────────────────┐
   │ 7. Read STATUS. Check EVERY error flag.     │
   └───────────────────┬─────────────────────────┘
                       ▼
   ┌─────────────────────────────────────────────┐
   │ 8. Re-lock; invalidate cache/prefetch       │
   │    Exit critical section                    │
   └───────────────────┬─────────────────────────┘
                       ▼
   ┌─────────────────────────────────────────────┐
   │ 9. VERIFY by reading back / blank-check     │
   └─────────────────────────────────────────────┘
```

**[INFERENCE]** Steps 1, 7, 8 and 9 are the ones that get skipped in example code and cause field failures. Step 8's cache invalidation in particular: you erased a sector, the cache still holds the old contents, your verify read returns the *pre-erase* data, and your code concludes the erase failed. Or worse — it concludes the erase succeeded when reading back freshly programmed data that is actually still in the write buffer's shadow.

## 5.4 A generic driver skeleton

```c
/* GENERIC EXAMPLE — illustrates structure, not any real device */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    FLASH_OK = 0,
    FLASH_ERR_PARAM,        /* alignment, range, null pointer            */
    FLASH_ERR_PROTECTED,    /* region is write/erase protected           */
    FLASH_ERR_TIMEOUT,      /* BUSY never cleared — controller wedged    */
    FLASH_ERR_VOLTAGE,      /* supply out of programming range           */
    FLASH_ERR_VERIFY,       /* readback did not match                    */
    FLASH_ERR_ECC,          /* uncorrectable error on read               */
    FLASH_ERR_HARDWARE      /* controller reported an error flag         */
} flash_status_t;

/* Device-specific geometry, supplied by the port layer. */
typedef struct {
    uint32_t base;              /* CPU address of region start      */
    uint32_t size;              /* bytes                            */
    uint32_t sector_size;       /* minimum erase unit               */
    uint32_t program_unit;      /* minimum program unit (flash word)*/
    uint32_t erase_time_max_us; /* worst case, from datasheet       */
    bool     ecc_enabled;
    bool     write_once_per_word; /* true if ECC forbids re-writing */
} flash_geometry_t;

flash_status_t flash_erase_sector(uint32_t addr);
flash_status_t flash_program(uint32_t addr, const void *data, size_t len);
flash_status_t flash_read(uint32_t addr, void *dst, size_t len);
flash_status_t flash_verify(uint32_t addr, const void *expect, size_t len);
flash_status_t flash_blank_check(uint32_t addr, size_t len);
```

**[INFERENCE]** Four design decisions embedded in that header, all deliberate:

1. **Every function returns a status.** A `void flash_write()` is a design defect. Flash *fails*, routinely, for reasons outside your control (voltage, wear, protection). Code that cannot express failure cannot handle it.
2. **`flash_read()` exists** even though flash is memory-mapped and you could just dereference a pointer. It exists because on an ECC part a read can *fail*, and because on an external-flash part it is not memory-mapped at all. The abstraction lets you port.
3. **`flash_blank_check()` exists** because on ECC parts you cannot determine emptiness by comparing against `0xFF`.
4. **Geometry is data, not `#define`s.** The same driver then serves a family whose members differ in sector size.

## 5.5 What the three interface styles look like side by side

```
  REGISTER-COMMAND STYLE            MODE-BIT STYLE                LIBRARY STYLE
  ──────────────────────            ──────────────                ─────────────
  unlock(KEY)                       unlock(KEY)                   fl_open();
  ADDR = target;                    CTRL |= ERASE_MODE;           fl_erase(block);
  DATA[0..n] = payload;             *(volatile T*)target = 0;     while (fl_busy())
  CMD = OP_PROGRAM | GO;            /* the dummy write is the        fl_handler();
  while (STATUS & BUSY) ;              trigger */                 status = fl_result();
  check(STATUS);                    while (STATUS & BUSY) ;       fl_close();
  lock();                           CTRL &= ~ERASE_MODE;
                                    lock();
```

**[PRINCIPLE]** All three do the same nine steps of §5.3. If you can see that, you can read any vendor's flash chapter.

## 5.6 Chapter exercises

1. Your driver programs correctly at 8 MHz during bring-up and fails intermittently after enabling a 120 MHz PLL. Give three candidate root causes and the register you would inspect for each.
2. Why must error flags be cleared *before* issuing a command rather than only after?
3. Write pseudocode for a `flash_wait_busy()` that cannot hang forever, and explain how you would choose the timeout value from the datasheet.
4. Explain why `if (*(uint32_t*)addr == 0xFFFFFFFF)` is an unsafe blank check on an ECC-protected device.

---

# Chapter 6 — Reading, Wait States, and Execution

## 6.1 Level 1 — What the C statement does

```c
uint32_t x = *(volatile uint32_t *)ADDRESS;
```

At source level: "load 32 bits from ADDRESS." That is the whole story a beginner needs. It is also the story that hides four layers of machinery.

## 6.2 Level 2 — The transaction

```
  CPU issues load, address = ADDRESS
        │
        ▼
  Bus fabric decodes address → routes to flash read interface
        │
        ▼
  Read interface checks: is this line already in cache/prefetch buffer?
        │
        ├── HIT  ──► return data in ~1 cycle. Done.
        │
        └── MISS ──► drive word line, precharge bit lines
                          │
                          ▼
                    sense amplifiers settle  ◄── ANALOGUE. Takes real time.
                          │
                          ▼
                    ECC decode (if present) ◄── may correct, may flag
                          │
                          ▼
                    fill cache line / prefetch buffer
                          │
                          ▼
                    return requested word to CPU
```

**[PRINCIPLE]** The sense-amplifier settling time is a physical constant of the memory macro, expressed in **nanoseconds**. The CPU's demand is expressed in **cycles**. Those two units do not track each other — and that mismatch is the entire wait-state story.

## 6.3 Level 3 — Wait states

**[PRINCIPLE]** If flash access takes t<sub>ACC</sub> nanoseconds and the CPU clock period is 1/f<sub>CPU</sub>, the number of extra cycles the CPU must wait is:

```
   wait_states  =  ceil( t_ACC × f_CPU )  −  1
```

**Worked example [INFERENCE — invented numbers]**

Suppose a flash macro has t<sub>ACC</sub> = 25 ns.

| f<sub>CPU</sub> | Clock period | Cycles needed | Wait states |
|---|---|---|---|
| 16 MHz | 62.5 ns | 0.40 → 1 | 0 |
| 40 MHz | 25.0 ns | 1.00 → 1 | 0 |
| 80 MHz | 12.5 ns | 2.00 → 2 | 1 |
| 120 MHz | 8.33 ns | 3.00 → 3 | 2 |
| 200 MHz | 5.00 ns | 5.00 → 5 | 4 |

**This is why raising the clock requires raising wait states.** The flash did not get slower; the cycles got shorter.

### 6.3.1 The two orderings that matter

**[PRINCIPLE]** When changing frequency, the order of operations is not symmetric:

```
   INCREASING clock:   set wait states FIRST, then raise clock.
   DECREASING clock:   lower clock FIRST, then reduce wait states.
```

Get it backwards and you have a window in which the CPU fetches instructions faster than the flash can supply them. The result is corrupted instruction fetch — which manifests as a hard fault at a random address, or worse, as silently executing garbage.

**[INFERENCE]** This is one of the highest-value five-line rules in embedded systems. Put it in your clock-configuration function as a comment. Every vendor's clock init does it correctly; every hand-rolled one eventually does it wrong.

### 6.3.2 Why wait states do not cost what you fear

A naive reading says "4 wait states means my 200 MHz CPU runs at 40 MHz." That is wrong, because of the next section.

## 6.4 Prefetch, buffers, and cache

**[PRINCIPLE]** Flash read latency is high, but flash read *width* is cheap. Memory macros are typically much wider than the bus — one array access can return several instructions at once. Architectures exploit this:

| Mechanism | What it does | What it helps | What defeats it |
|---|---|---|---|
| **Wide read + line buffer** | One array access fills a buffer of several words | Sequential code | Branches |
| **Prefetch** | Speculatively fetch the *next* line while the CPU consumes the current one | Straight-line code, loops | Unpredictable branches |
| **Instruction cache** | Retains recently used lines | Loops, hot functions | Large working sets, first-time code |
| **Branch/loop buffers** | Small caches specialised for tight loops | Interrupt handlers, DSP loops | Long functions |
| **Data cache** (less common on small MCUs) | Caches constant reads | Lookup tables in flash | Writes, coherency issues |

**[PRINCIPLE]** With prefetch and cache, average instruction-fetch cost approaches one cycle for sequential and looping code, even with several wait states. The cost reappears on **branches and cache misses** — which is why flash-executing code has *less deterministic* timing than RAM-executing code.

**[INFERENCE]** Real-time consequence: if you have a hard deadline in an ISR, "it usually takes 200 cycles" is not a specification. On a flash-XIP part with cache, worst case is what matters, and worst case is a cold miss with full wait states on every fetch. Two standard mitigations: place the critical handler in RAM, or lock it into cache if your part supports that.

### 6.4.1 The coherency hazard

**[PRINCIPLE]** Cache and prefetch buffers hold *copies* of flash contents. When you erase or program flash, the copies become stale. The flash controller usually does **not** invalidate them for you.

Symptoms of forgetting this:

- Verify-after-write reads the **old** data and reports failure on a successful write
- Verify-after-erase reads the **old** data and reports failure on a successful erase
- A firmware update appears to succeed, then executes the previous image
- Behaviour changes depending on whether the debugger is attached (because the debugger's reads may bypass or perturb the cache)

**Rule [PRINCIPLE]: after any program or erase, invalidate the instruction cache, data cache, and prefetch buffers covering that region — before reading it back.**

## 6.5 Execute-in-place, stated properly

**XIP** means: *the CPU fetches instructions directly from non-volatile memory, without first copying them to RAM.*

**[PRINCIPLE]** Three prerequisites:
1. The memory is **randomly addressable** at word granularity.
2. It appears in the CPU's **address space** (directly, or through a memory-mapped controller window).
3. Its **latency is tolerable** with the available wait states, buffers, and cache.

This is why NOR-class memory supports XIP and NAND does not.

### 6.5.1 Internal XIP vs external XIP

| | **Internal flash XIP** | **External NOR XIP (QSPI/OSPI)** |
|---|---|---|
| Address space | Native, always present at reset | A window created by a controller that must be **configured first** |
| Available at reset? | Yes | **No** — something must configure the interface before any instruction can be fetched |
| Who configures it? | Nothing to configure | Boot ROM, using configuration data read from the flash itself |
| Latency | Tens of ns | Higher; serial protocol + command overhead |
| Cache dependency | Helpful | **Effectively mandatory** — without cache, performance collapses |
| Program/erase while executing | Sometimes, bank-dependent | **Never on the same device** without special support — see below |

**[PRINCIPLE]** The bootstrap paradox of external XIP: *to execute from external flash, you must configure the interface; the configuration lives in the external flash; so something must read it before the interface is configured.* Every architecture solves this the same way: a **boot ROM** reads a fixed-format configuration structure from a fixed offset in the external device using a slow, universally-supported default command, then configures the fast interface, then jumps.

**[INFERENCE]** This is why "my board doesn't boot from QSPI" is nearly always one of four things: the configuration structure is at the wrong offset, its contents don't match the actual flash part, the boot-mode pins/fuses select a different source, or the flash needs a mode-enable sequence (e.g. entering quad mode) that the configuration didn't specify.

### 6.5.2 The XIP self-modification hazard

**[PRINCIPLE]** You cannot generally read a flash device while it is executing an erase or program operation. If your instructions are coming from that device, "cannot read" means "cannot fetch." The CPU dies.

Therefore, updating firmware on an external-XIP system requires that the updater code **not** live in the device being updated. Standard solutions:

- Copy the updater into internal SRAM and run it there
- Copy the whole application into RAM at boot and run from RAM permanently
- Use two external devices, or a device with true read-while-write across banks
- Use a device with internal flash for the bootloader and external flash for the application

## 6.6 The four execution architectures

**[PRINCIPLE]** When you ask "can I execute while programming?", the answer is one of exactly four:

```
 ┌─────────────────────────────────────────────────────────────────┐
 │ ARCHITECTURE A — Stall                                          │
 │ Single array. Any program/erase stalls the CPU until complete.  │
 │ The CPU literally stops fetching. Interrupts cannot be serviced.│
 │ → Erase time IS your interrupt latency. Watchdog must survive it│
 └─────────────────────────────────────────────────────────────────┘

 ┌─────────────────────────────────────────────────────────────────┐
 │ ARCHITECTURE B — Cross-bank execution                           │
 │ Multiple banks. Code in bank 0 keeps running while bank 1 is    │
 │ modified. Interrupts work IF the vectors and every ISR reachable│
 │ during the operation live in the other bank.                    │
 │ → The linker script becomes a safety-critical artefact.         │
 └─────────────────────────────────────────────────────────────────┘

 ┌─────────────────────────────────────────────────────────────────┐
 │ ARCHITECTURE C — RAM-resident routine                           │
 │ Single array, but the CPU is not stalled — it simply cannot     │
 │ fetch from flash. The programming routine, its callees, the     │
 │ vector table and all live ISRs are copied to and run from SRAM. │
 │ → Most flexible, most work, most subtle bugs.                   │
 └─────────────────────────────────────────────────────────────────┘

 ┌─────────────────────────────────────────────────────────────────┐
 │ ARCHITECTURE D — Separate macro (true concurrent read-while-    │
 │ write)                                                          │
 │ Data flash is a physically separate macro with its own          │
 │ sequencer. Code executes from code flash entirely unaffected.   │
 │ → The best case for data-logging products. Note: usually still  │
 │   forbidden to operate BOTH macros simultaneously.              │
 └─────────────────────────────────────────────────────────────────┘
```

**[VARIES]** Which one your device is — and it may be different for code flash than for data flash — is the single most consequential fact in your flash bring-up. Find it before you design your NVM layer, not after.

**[INFERENCE]** Design implication table:

| Architecture | Erase during normal operation? | RTOS impact | Design pattern |
|---|---|---|---|
| A | Only when you can afford a full stall | Scheduler frozen; all deadlines missed | Batch writes into a maintenance window |
| B | Yes, with careful linking | Only bank-1-resident ISRs affected | Partition code by bank in the linker |
| C | Yes, with RAM-resident code | ISRs must be RAM-resident or disabled | The classic bootloader pattern |
| D | Yes, freely | Minimal | Log continuously; ordinary task can do it |

## 6.7 Chapter exercises

1. A macro has t<sub>ACC</sub> = 30 ns. Compute required wait states at 24, 48, 96 and 168 MHz.
2. Explain, in terms of the read path, why the same function can take 400 cycles the first time and 90 cycles the second time.
3. You are told your part is "Architecture B." Write the three linker-script constraints this imposes on your firmware-update code.
4. Your verify-after-erase fails, but reading the same address with a debugger shows a correctly erased sector. Diagnose.
5. Explain the bootstrap paradox of external XIP and how boot ROM resolves it.

---

---

# Chapter 7 — Programming

## 7.1 Why "writing" is the wrong word

In RAM, a write is a store: you name an address, you supply a value, the value is there. It is symmetric, instantaneous, and unlimited.

In flash, **programming is a physical process performed by a sequencer, in a fixed granularity, in one direction only, with a finite budget, that can fail.** Using the word "write" imports four assumptions that are all false. Say *program*, and you will think more clearly.

## 7.2 The one-way bit rule

**[PRINCIPLE]** In a typical flash macro:

- The **erase** operation drives an entire sector to one uniform logical state.
- The **program** operation can move individual bits **away from** the erased state, but cannot move them back.

Conventionally (but **[VARIES]** — check your device):

```
   erased state:      1 1 1 1 1 1 1 1
   program 0xF0:      1 1 1 1 0 0 0 0     ← allowed: 1 → 0
   program 0xFF now?  1 1 1 1 0 0 0 0     ← NO EFFECT: 0 → 1 requires erase
   program 0xC0 now:  1 1 0 0 0 0 0 0     ← allowed: more 1s → 0
```

**[PRINCIPLE]** So a second program to an already-programmed location is not necessarily illegal — it can only *add* bits in the programmable direction. This is the basis of the classic "status flag" trick used in EEPROM emulation:

```
   0xFF  →  0xFE  →  0xFC  →  0xF0      /* four state transitions, no erase */
   free      in-use   valid    obsolete
```

### 7.2.1 …and why that trick may be forbidden on your part

**[PRINCIPLE]** If the macro has hardware ECC over a flash word, reprogramming any byte in that word requires recomputing check bits — and the old check bits are already programmed. The controller therefore typically **forbids programming a word twice between erases**, or leaves the result undefined.

**[INFERENCE]** So the multi-state-flag trick divides the world in two:

| Device class | Multi-write-per-word | Record design |
|---|---|---|
| **No ECC, multi-write allowed** | Yes | Flags can be updated in place; compact records |
| **ECC, write-once-per-word** | No | Each state transition must write a **fresh, whole flash word** |

**The write-once design is strictly safer and portable to both.** Design for write-once and you never have to revisit it. See Chapter 10.

## 7.3 Granularity and alignment

**[PRINCIPLE]** Three constraints, all separate:

1. **Program unit** — the flash word. You program N bytes at a time, where N is fixed by silicon.
2. **Alignment** — the target address must be a multiple of the program unit (or sometimes of a larger "page" or "row").
3. **Ordering** — some macros require programming within a row to proceed in a defined order, or forbid re-visiting a row.

**[VARIES]** Program units observed across real devices span byte, word, double-word, "phrase," page, and row. Some devices have *different* program units for code flash and data flash on the same die.

### 7.3.1 Why the API looks restrictive

This is why flash APIs look like:

```c
flash_status_t flash_program_word(uint32_t addr, uint64_t data);
flash_status_t flash_program_page(uint32_t addr, const void *buf);
flash_status_t flash_erase_sector(uint32_t addr);
```

and never like:

```c
void flash_write_anything_anywhere(uint32_t addr, const void *buf, size_t len);
```

**[INFERENCE]** When you *do* build a general `flash_program(addr, buf, len)` convenience wrapper — and you should — it must:

- reject unaligned `addr`
- reject `len` that is not a multiple of the program unit, **or** pad internally with the erased pattern and document that it does
- never silently perform a read-modify-write, because doing so requires an erase, and an erase is a destructive operation the caller did not authorise

The last point is a real API design principle: **a "write" function that can erase is a trap.** If read-modify-write semantics are needed, that belongs in a higher storage layer with its own name, its own power-fail protocol, and its own documentation.

## 7.4 The program-and-verify loop inside the silicon

**[PRINCIPLE]** A program operation is not a single pulse. Typical internal sequence:

```
   for attempt in 1..N:
        apply program pulse (duration, amplitude set by sequencer)
        apply program-verify read at the verify reference level
        if all target cells passed:
             report success
        adjust pulse parameters
   report VERIFY_FAIL
```

Consequences you can observe from firmware:

- Programming time is **data-dependent and part-dependent**; datasheets quote typical and maximum.
- Programming time **increases as the part wears**, because more pulses are needed.
- A `VERIFY_FAIL` is meaningful information: it says the cells could not be moved. Causes: end of life, out-of-range voltage, out-of-range temperature, or the location was not erased first.

## 7.5 Preconditions your driver must enforce

**[PRINCIPLE]** Before issuing any program command:

| Precondition | Why | Failure mode if skipped |
|---|---|---|
| Supply voltage within programming range | Charge pumps need headroom | Silent partial programming; unstable cells that read correctly today and fail in a year |
| Flash timing/clock configured | Sequencer pulse widths derive from it | Out-of-spec pulses; marginal cells |
| Target erased (or write-once rule respected) | One-way bit rule | Verify failure or corrupted data |
| Address aligned, length a multiple of program unit | Hardware constraint | Access error, or writing the wrong location |
| Region not protected | Protection logic | Protection violation flag |
| Interrupts and watchdog handled | Timing hazard | See Chapter 16 |
| Cache/prefetch will be invalidated afterwards | Coherency | False verify results |

**[INFERENCE]** The voltage one deserves emphasis because it is invisible. A brown-out that stays *above* the CPU's minimum but *below* the flash programming minimum produces the nastiest class of flash bug: programming appears to succeed, the readback verify passes because the cells are marginally in the right state, and the data decays to an incorrect value weeks later in the field. **If your product programs flash in the field, you should gate flash writes on a brown-out/supply-supervisor threshold set to the flash programming minimum, not the CPU minimum.**

## 7.6 Three views of programming

| View | Programming is… |
|---|---|
| **Beginner** | Storing data into flash so it survives power-off. |
| **Engineer** | A controller command that moves an aligned, fixed-size unit of cells one direction only, taking microseconds to milliseconds, requiring adequate supply and clock, returning a status that must be checked, and requiring cache invalidation before readback. |
| **Silicon** | An iterative sequence of high-voltage pulses and verify reads that shifts the V<sub>t</sub> distribution of the addressed cells past the program-verify level, consuming a quantum of tunnel-oxide lifetime. |

## 7.7 Chapter exercises

1. Your device has an 8-byte program unit with ECC. Design the on-flash layout of a record that must transition through four states, without ever re-programming a word.
2. Explain why `flash_program()` should not silently erase.
3. A unit programs successfully at 25 °C on the bench and fails verification at -40 °C in the chamber. List four candidate causes.
4. Why does programming time increase over a part's life?

---

# Chapter 8 — Erasing

## 8.1 The asymmetry restated

**[PRINCIPLE]** Erase granularity is coarser than program granularity — usually by a factor of hundreds or thousands — because the erase mechanism operates through structures shared across many cells (a common well, a common source line). Making erase finer would mean replicating those structures, which costs area, which raises cost per bit, which defeats the reason flash exists.

**[PRINCIPLE]** The vocabulary is not standardised. Map the names onto the concepts:

| Concept | Names you will meet |
|---|---|
| Smallest erasable unit | sector, page, block, segment, row, cluster |
| A group of them | block, bank, region |
| Erase everything | mass erase, chip erase, bulk erase |
| Erase everything + reset config | factory reset, full erase |

**[INFERENCE]** Because "page" means *erase unit* on one vendor and *program unit* on another, and "block" means *erase unit* on one and *group of erase units* on another, **never carry these words between datasheets.** In your own code, define your own unambiguous terms — `FLASH_ERASE_UNIT` and `FLASH_PROGRAM_UNIT` — and map the vendor's words onto them once, in one header.

## 8.2 The cost model of an erase

**[PRINCIPLE]** Three costs, and engineers usually only count the first:

| Cost | Magnitude | Consequence |
|---|---|---|
| **Time** | Milliseconds to hundreds of milliseconds | Blocks the CPU or a bank; watchdog and real-time hazard |
| **Endurance** | One cycle from a finite budget for that sector | Lifetime arithmetic (Chapter 10) |
| **Data destruction** | The entire sector's contents | Requires a save/restore or a design that avoids it |

The third cost is the one that shapes architecture.

## 8.3 The read-modify-write-erase trap

**Scenario.** You have a 4 KB sector. It contains a 64-byte configuration structure and 4 KB − 64 bytes of other data. You need to change **one byte** of the configuration.

**The naive implementation:**

```
   1. Read the whole 4 KB sector into RAM        (needs 4 KB of RAM you may not have)
   2. Modify one byte in RAM
   3. Erase the sector                           (~ tens of ms; all data now gone)
   4. Program the 4 KB back                      (~ several ms)
```

**[PRINCIPLE]** Between steps 3 and 4 there is a window in which **the data exists only in volatile RAM.** A power loss there destroys the entire sector's contents, not just the byte you were changing. You have converted a one-byte update into a total data loss risk.

Costs of this design:

- 4 KB of RAM to buffer the sector
- One full erase cycle to change one byte — **a 4096× write amplification**
- A power-loss window of tens of milliseconds
- The whole sector's endurance consumed at the rate of the *most frequently changed* item in it

**[INFERENCE]** Two independent fixes, and you generally want both:

1. **Never mix update rates in one sector.** Put a hot counter in its own sector, away from cold calibration data. Sector layout is a rate-partitioning problem.
2. **Never do read-modify-write-erase for data you update at runtime.** Use append-only records instead (Chapter 10). Erase becomes a rare, background reclamation event rather than part of every update.

## 8.4 Worst-case erase time and the watchdog

**[PRINCIPLE]** Datasheets quote typical and maximum erase times. **You must design against the maximum, at the worst-case temperature and end-of-life wear**, not the typical you measured on a fresh part at room temperature.

Three interactions:

```
   ARCHITECTURE A (CPU stalled)
       ┌──────────── erase, worst case ────────────┐
   ────┤ CPU frozen. No ISRs. No watchdog service. ├────
       └───────────────────────────────────────────┘
       Requirement: watchdog timeout > max erase time + margin,
                    OR a hardware windowed watchdog that
                    tolerates the stall, OR erase in smaller units.

   ARCHITECTURE C (RAM-resident)
       ┌──────────── erase, worst case ────────────┐
   ────┤ CPU runs from RAM. ISRs OK if RAM-resident├────
       └───────────────────────────────────────────┘
       Requirement: a RAM-resident watchdog-service path,
                    or interrupts disabled and the erase split.
```

**[INFERENCE]** The standard mistakes:

- Sizing the watchdog against *typical* erase time. Works for a year, then a cold unit at end of life resets mid-erase.
- Servicing the watchdog *before* the erase and assuming that is enough. It is not if the erase can exceed the remaining window.
- Erasing a whole bank in one command because it is "faster." It is faster in total but produces a far longer single blocking operation. **Erasing sector by sector, servicing the watchdog between sectors, is almost always the better engineering choice** even though it is slower — because it bounds the blocking interval.

## 8.5 Suspend and resume

**[VARIES]** Some devices support **erase suspend**: the sequencer pauses so the CPU can read the array, then resumes. This is valuable for real-time systems and for external SPI NOR.

Caveats that apply almost universally where it exists:

- The suspended sector itself usually cannot be read
- There is a minimum time between resume and the next suspend, or the erase never completes ("suspend starvation")
- Erase time is extended by the suspension overhead
- Not all commands are permitted while suspended

**[INFERENCE]** Erase suspend is a real-time tool, not a correctness tool. If your design *needs* suspend to meet deadlines, consider whether erasing in a maintenance window would be simpler and more robust.

## 8.6 Chapter exercises

1. A sector is 8 KB and holds twelve independent settings. One setting changes hourly; the other eleven change once at commissioning. Explain the endurance consequence and redesign the layout.
2. Compute the write amplification of changing 4 bytes in a 2 KB sector by read-modify-write-erase.
3. Your watchdog timeout is 100 ms and worst-case sector erase is 90 ms at 125 °C. State three reasons this is unsafe and three fixes.
4. Explain why erasing one sector at a time is often better engineering than a bank erase, despite being slower overall.

---

# Chapter 9 — Endurance, Retention, and the Lifetime Budget

## 9.1 Definitions, precisely

**[PRINCIPLE]**

- **Endurance** — the number of erase/program cycles a unit can undergo *while still meeting its retention specification*. Quoted per erase unit, not per byte.
- **Retention** — how long data remains readable after the last program, *at a stated temperature*, *after a stated number of cycles*.

They are two coordinates on one curve, not two independent numbers (§2.5). A datasheet line reading "*10⁵ cycles, 10-year retention at 55 °C*" is a single statement, not two.

**[VARIES]** Real endurance figures span from roughly 10³ to well beyond 10⁶ depending on technology, region (data flash is usually far higher than code flash), temperature grade, and how the vendor defines the test. **Never carry a number between parts.**

## 9.2 The lifetime formula

**[PRINCIPLE]**

```
                       endurance_cycles  ×  wear_leveling_factor
   lifetime  =  ────────────────────────────────────────────────
                            erase_cycles_per_unit_time
```

Three levers. Note carefully: the numerator has **erase** cycles and the denominator has **erase** operations. Programs that do not cause an erase are free. This is the entire justification for append-only design.

### 9.2.1 Worked example — the naive design

**[INFERENCE — invented but representative numbers]**

Assume: one sector, endurance 10,000 cycles, and a design that erases-and-rewrites that sector on every save.

```
   Save rate            Erases/year          Lifetime
   ───────────────────────────────────────────────────────────────
   1 per second         31,536,000           10,000/31,536,000 yr
                                             ≈ 0.0000317 yr
                                             ≈ 2.8 hours          ✗✗✗
   1 per minute            525,600           ≈ 6.9 days           ✗✗
   1 per hour                8,760           ≈ 1.14 years         ✗
   1 per day                    365          ≈ 27 years           ✓
   1 per week                    52          ≈ 192 years          ✓✓
```

**[PRINCIPLE]** This table is why `writeFlashEverySecond()` is not a performance problem — it is a *product destruction* problem. The device does not slow down; it stops retaining data, silently, after a few hours.

**[INFERENCE]** Note also what the table says about *acceptable* designs: anything at or below roughly one erase per day is comfortable on a 10k-cycle part. The entire art of flash storage design is **converting a high logical write rate into a low physical erase rate.** Three techniques do that, and they compose:

| Technique | Mechanism | Typical gain |
|---|---|---|
| **Append-only records** | Many programs per erase | (sector_size / record_size) × |
| **Wear levelling** | Spread erases over N sectors | N × |
| **Batching/buffering** | Coalesce many logical updates into one physical write | (batch factor) × |

### 9.2.2 Worked example — the same requirement, engineered

Requirement: persist a 32-byte record on every change; changes occur once per second; the part offers 10,000 cycles.

**Design 1 — one sector, erase every save**
```
   erases/year = 31,536,000        →  lifetime ≈ 2.8 hours
```

**Design 2 — one 4 KB sector, append-only records**
```
   records per sector = 4096 / 32 = 128
   erases/year = 31,536,000 / 128 = 246,375
   lifetime = 10,000 / 246,375 ≈ 0.04 yr ≈ 15 days
```
Better by 128×. Still unusable.

**Design 3 — four 4 KB sectors, append-only + rotation**
```
   effective capacity = 4 × 128 = 512 records per full rotation
   erases per sector per year = 31,536,000 / 512 = 61,594
   lifetime = 10,000 / 61,594 ≈ 0.16 yr ≈ 59 days
```
Better by 512×. Still unusable. **[INFERENCE] This is the important lesson: at one write per second, no amount of flash cleverness saves you.** The physics is not on your side. You must change the *requirement*.

**Design 4 — batching: persist only every 60 s, or on change-of-significance, or on brown-out detect**
```
   logical rate: 1/s (unchanged, from the application's view)
   physical rate: 1/60 s
   with 4 sectors × 128 records:
   erases per sector per year = 525,600 / 512 = 1,027
   lifetime = 10,000 / 1,027 ≈ 9.7 years                        ✓
```

**Design 5 — the architectural answer**
```
   Hot value lives in RAM (or FRAM, or battery-backed RAM).
   Flash receives a snapshot on: significant change, periodic
   checkpoint, graceful shutdown, and brown-out interrupt.
   Physical erase rate: a few per day.
   Lifetime: decades.                                            ✓✓
```

**[INFERENCE]** Design 5 is what production metering, automotive, and industrial products actually do. The pattern has a name worth remembering: **write-back caching with a power-fail flush.** The brown-out interrupt is the critical piece — it is what makes it safe to keep the authoritative value in volatile memory, because you get a guaranteed opportunity to persist it. Check that your supply's hold-up time (bulk capacitance ÷ load current) exceeds the worst-case program time for your snapshot, with margin. That is a hardware requirement derived from a software architecture, and it must be in your schematic review.

## 9.3 Retention and temperature

**[PRINCIPLE]** Charge loss through an insulator is a thermally activated process. Its rate rises steeply and non-linearly with temperature — an Arrhenius relationship. The engineering consequences:

- Retention specified at, say, 55 °C is **not** the retention you get at 105 °C. It is dramatically shorter.
- A part that meets ten-year retention in a consumer application may need de-rating in an automotive under-hood application.
- **Cycling reduces retention.** Retention is specified *at* an endurance count; at end of life, retention is shorter than at beginning of life.
- Vendors qualify with accelerated high-temperature bake tests, then extrapolate. That extrapolation is a model, not a measurement of your part at your temperature.

**[INFERENCE]** Two practical rules:

1. **If your product stores data for years at elevated temperature, ask the vendor for the retention-versus-temperature-versus-cycles data, not just the headline number.** For automotive and industrial parts this data usually exists in a qualification report or app note.
2. **Consider periodic refresh** for very long-lived data: read the record, verify its CRC, and if the part supports it, use a margin-read to detect cells that are still correct but weak — then rewrite. This trades a small number of erase cycles for a large retention improvement. It is standard practice in long-life industrial products.

## 9.4 Disturb

**[PRINCIPLE]** Operating on one cell applies sub-threshold stress to others sharing its lines.

| Type | Cause | Where it matters |
|---|---|---|
| **Read disturb** | Read voltage on the word line of a row stresses unselected cells | Very high read counts on the same region — e.g. a lookup table read millions of times, or a page read constantly |
| **Program disturb** | Programming one cell exposes neighbours to partial fields | Dense arrays; repeated partial-page programming |
| **Erase disturb** | Erase fields affecting adjacent structures | Design-level; usually managed in silicon |

**[VARIES]** Whether disturb is a concern you must manage in firmware is device- and technology-dependent. It is a first-order design concern in raw NAND and a rarely-observed effect in most embedded NOR-class macros — but "rarely observed" is not "impossible," and the standard mitigation (CRC every record, refresh on weak reads) costs you almost nothing.

## 9.5 Guaranteed vs typical

**[PRINCIPLE]** Datasheets distinguish:

- **Min/Max (guaranteed)** — the vendor commits to this across the specified process, voltage, and temperature range. Design against these.
- **Typ (typical)** — a representative value, often at 25 °C and nominal voltage, sometimes not even production-tested. **Never design against typ.**

**[INFERENCE]** The classic failure: measuring erase time on your bench (typical, 25 °C, fresh part), sizing your watchdog and your OTA progress timeout against that, and shipping. The field returns come from cold units at end of life, where the real number is the datasheet maximum you ignored.

## 9.6 Chapter exercises

1. A part has 100,000 cycles on data flash. You must log one 16-byte event per minute for 15 years. Design a layout (sector count, record size, rotation) and prove the lifetime arithmetic.
2. Explain why "10⁶ cycles" without a retention and temperature condition is not a specification.
3. Your supply has 470 µF of bulk capacitance and the load draws 40 mA at 3.3 V, with a brown-out interrupt at 3.0 V and flash minimum at 2.7 V. Estimate the hold-up time and decide whether you can safely program 256 bytes on power-fail. State your assumptions.
4. Why does retention shorten as cycle count increases? Answer using the V<sub>t</sub> distribution picture.

---

---

# PART III — PERSISTENT DATA

---

# Chapter 10 — EEPROM Emulation

## 10.1 The problem statement

You have flash. Your application wants EEPROM semantics:

```c
config_write(KEY_CALIBRATION, &value, sizeof value);   /* small, arbitrary, frequent */
config_read (KEY_CALIBRATION, &value, sizeof value);
```

Flash cannot do that natively. The gap:

| Application wants | Flash provides |
|---|---|
| Update a small item in place | Program one direction only, in fixed units |
| Update frequently | Finite erase endurance |
| Atomic update | Multi-step, interruptible sequence |
| Update any item independently | Erase destroys a whole sector |

**EEPROM emulation** is the software layer that closes this gap. It is one of the two or three most important pieces of code in an embedded product, and it is very often written badly.

## 10.2 The core idea: append, don't overwrite

**[PRINCIPLE]** Instead of storing "the current value of item X at a fixed address," store **a log of writes**, and define the current value as *the newest valid record for X*.

```
 Sector A (active)
 ┌────────────────────────────────────────────────────────────┐
 │ [hdr][id=1  seq=1  len=4  data  crc]   ← superseded        │
 │ [hdr][id=2  seq=2  len=8  data  crc]                       │
 │ [hdr][id=1  seq=3  len=4  data  crc]   ← superseded        │
 │ [hdr][id=1  seq=4  len=4  data  crc]   ← CURRENT value of 1│
 │ [hdr][id=3  seq=5  len=16 data crc]                        │
 │ ................ free (erased) .............               │
 └────────────────────────────────────────────────────────────┘
```

Why this wins:

- **Writes never erase.** Erase happens only when the sector fills — once per *sector_size/record_size* writes.
- **Power-fail is naturally safe.** The old record is untouched until the new one is complete. A torn new record simply fails its CRC and is ignored; the previous value remains current.
- **No RAM buffer of the whole sector is needed.**
- **Wear is spread automatically** across the sector, and across sectors with rotation.

**[PRINCIPLE]** The append-only log is not a clever trick; it is the *correct* data structure for a medium with these physics. Databases reached the same conclusion independently — this is a write-ahead log.

## 10.3 Record design

### 10.3.1 The fields, and why each exists

```c
/* GENERIC EXAMPLE — portable design targeting the strictest case:
   hardware ECC, write-once-per-flash-word. */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define NVM_MAGIC          0x4E564D31u   /* "NVM1" — identifies a record start */
#define NVM_FLASH_WORD     8u            /* [VARIES] device program unit       */
#define NVM_ALIGN(x)       (((x) + (NVM_FLASH_WORD - 1u)) & ~(NVM_FLASH_WORD - 1u))

typedef struct {
    uint32_t magic;        /* record marker; distinguishes data from erased space   */
    uint16_t id;           /* which logical item this record carries                */
    uint16_t length;       /* payload bytes                                         */
    uint32_t sequence;     /* monotonic; larger = newer                             */
    uint32_t header_crc;   /* CRC over the four fields above                        */
    uint32_t payload_crc;  /* CRC over the payload bytes                            */
    /* payload follows, padded to NVM_FLASH_WORD                                    */
} nvm_record_header_t;
```

| Field | Why it must exist |
|---|---|
| `magic` | Distinguishes a real record from erased space *and* from arbitrary leftover data. Without it you cannot reliably find where the log ends. |
| `id` | Which setting this is. Turns a log into a key-value store. |
| `length` | Lets you skip to the next record without knowing the schema, and lets a single item change size across firmware versions. |
| `sequence` | Establishes ordering. **This is what makes "newest wins" well-defined**, and it is what survives a wrapped log where physical order is misleading. |
| `header_crc` | Lets you trust `length` before you use it. **Critical:** if `length` is corrupt and you trust it, you walk off into garbage. Validate the header independently, *before* using any field in it. |
| `payload_crc` | Detects a torn write, a bit flip, or a retention failure in the data. |

**[INFERENCE]** The two-CRC design (header and payload separately) is the detail that separates a robust implementation from a fragile one. A single CRC over the whole record forces you to know `length` before you can verify `length`. That circularity is exactly where power-fail corruption turns into an infinite loop or a wild pointer.

### 10.3.2 Layout rules

**[PRINCIPLE]**

1. `sizeof(header) + padded_payload` must be a multiple of the flash word.
2. The header must itself be a multiple of the flash word so the payload starts aligned.
3. Do not use bitfields. Do not rely on the compiler's struct padding. Serialise explicitly, or use `static_assert` on the size.
4. Pick an endianness and state it. If a tool on a PC will ever read this flash, this matters.
5. Include a **schema version** — either in the record or in a sector header — so a future firmware can interpret old records.

```c
_Static_assert(sizeof(nvm_record_header_t) % NVM_FLASH_WORD == 0,
               "record header must be a multiple of the flash program unit");
```

## 10.4 Sector states and rotation

**[PRINCIPLE]** With two or more sectors you can compact without ever having a moment where the data exists nowhere.

```
   State machine per sector:

        ERASED ──────► ACTIVE ──────► FULL ──────► OBSOLETE ──────► ERASED
          ▲          (receiving       (no room)   (live records      │
          │           appends)                     copied out)       │
          └──────────────────────────────────────────────────────────┘
```

The sector header carries the state. On an ECC/write-once device, the state must be encoded so that each transition writes a **fresh** flash word — so keep a small array of state-marker words rather than reprogramming one:

```c
/* GENERIC EXAMPLE — sector header, write-once-safe */
typedef struct {
    uint32_t magic;          /* sector magic                          */
    uint32_t schema_version;
    uint32_t erase_count;    /* for wear statistics and levelling     */
    uint32_t header_crc;
} nvm_sector_header_t;       /* padded to flash word                  */

/* State is recorded by appending a marker word, not by rewriting one. */
typedef struct {
    uint32_t marker;         /* NVM_STATE_ACTIVE / _FULL / _OBSOLETE  */
    uint32_t marker_crc;
} nvm_state_marker_t;        /* padded to flash word                  */
```

**[INFERENCE]** Reading the sector state then means "read the header, then scan the marker slots and take the last valid one." Slightly more code, and it works on every device including the strict ECC ones. The alternative — one byte you keep re-programming — works on some parts and silently corrupts on others. Choose the portable one.

## 10.5 The complete algorithms

### 10.5.1 Mount (run at boot)

```c
/* GENERIC EXAMPLE */

typedef struct {
    uint32_t sector_addr[NVM_SECTOR_COUNT];
    uint32_t sector_size;
    int      active_index;      /* which sector is currently ACTIVE   */
    uint32_t write_offset;      /* next free offset in active sector  */
    uint32_t next_sequence;     /* next sequence number to issue      */
} nvm_ctx_t;

flash_status_t nvm_mount(nvm_ctx_t *ctx)
{
    int active = -1;
    uint32_t best_seq = 0;

    /* 1. Determine each sector's state from its header + markers. */
    for (int i = 0; i < NVM_SECTOR_COUNT; i++) {
        nvm_sector_state_t st = nvm_read_sector_state(ctx, i);

        if (st == NVM_STATE_ACTIVE) {
            if (active >= 0) {
                /* Two ACTIVE sectors: a compaction was interrupted.
                   Recover by choosing the one with the higher max
                   sequence number and demoting the other.        */
                active = nvm_resolve_dual_active(ctx, active, i);
            } else {
                active = i;
            }
        } else if (st == NVM_STATE_FULL) {
            /* A compaction was interrupted before the target became
               ACTIVE, or before the source became OBSOLETE.       */
            nvm_note_pending_compaction(ctx, i);
        }
    }

    /* 2. No active sector at all: first boot, or the last erase
          completed but the new header write did not.             */
    if (active < 0) {
        active = nvm_initialise_fresh_sector(ctx);
        if (active < 0) return FLASH_ERR_HARDWARE;
    }
    ctx->active_index = active;

    /* 3. Scan the active sector to find the write frontier and the
          highest sequence number seen anywhere.                   */
    uint32_t off = NVM_SECTOR_HEADER_SIZE;
    while (off + sizeof(nvm_record_header_t) <= ctx->sector_size) {
        nvm_record_header_t h;
        (void)flash_read(ctx->sector_addr[active] + off, &h, sizeof h);

        if (h.magic != NVM_MAGIC) break;                 /* end of log */
        if (crc32(&h, offsetof(nvm_record_header_t, header_crc))
                != h.header_crc) {
            /* Torn header. Everything after it is untrustworthy;
               treat this as the frontier.                          */
            break;
        }
        if (h.sequence > best_seq) best_seq = h.sequence;

        off += NVM_ALIGN(sizeof h + h.length);
    }

    ctx->write_offset   = off;
    ctx->next_sequence  = best_seq + 1u;

    /* 4. Finish any compaction that was interrupted. */
    return nvm_complete_pending_compaction(ctx);
}
```

**[INFERENCE]** Four properties of that function are the whole point, and they are what most hand-written implementations lack:

1. It **never trusts a length field it has not CRC-verified.**
2. It treats a **torn record as the end of the log**, not as an error requiring intervention.
3. It **detects and resolves interrupted compaction** — the state that only ever happens if you lose power at exactly the wrong moment, which in a fleet of a million devices happens constantly.
4. It derives `next_sequence` from the data, so sequence numbers survive any reset.

### 10.5.2 Write

```c
flash_status_t nvm_write(nvm_ctx_t *ctx, uint16_t id,
                         const void *data, uint16_t len)
{
    const uint32_t need = NVM_ALIGN(sizeof(nvm_record_header_t) + len);

    if (data == NULL || len == 0u) return FLASH_ERR_PARAM;
    if (need > ctx->sector_size - NVM_SECTOR_HEADER_SIZE)
        return FLASH_ERR_PARAM;                 /* item can never fit */

    /* Compact if there is no room. */
    if (ctx->write_offset + need > ctx->sector_size) {
        flash_status_t s = nvm_compact(ctx);
        if (s != FLASH_OK) return s;
    }

    /* Build the record in RAM, complete, before touching flash. */
    uint8_t buf[NVM_MAX_RECORD];
    nvm_record_header_t *h = (nvm_record_header_t *)buf;

    memset(buf, 0xFF, need);                    /* pad with erased pattern */
    h->magic       = NVM_MAGIC;
    h->id          = id;
    h->length      = len;
    h->sequence    = ctx->next_sequence;
    h->payload_crc = crc32(data, len);
    h->header_crc  = crc32(h, offsetof(nvm_record_header_t, header_crc));
    memcpy(buf + sizeof *h, data, len);

    /* Program payload-then-header would be safer still on devices that
       allow it; on write-once-per-word devices the header is the first
       word, so we rely on the CRCs instead.                            */
    flash_status_t s = flash_program(ctx->sector_addr[ctx->active_index]
                                     + ctx->write_offset, buf, need);
    if (s != FLASH_OK) return s;

    s = flash_verify(ctx->sector_addr[ctx->active_index]
                     + ctx->write_offset, buf, need);
    if (s != FLASH_OK) return s;

    ctx->write_offset += need;
    ctx->next_sequence++;
    return FLASH_OK;
}
```

### 10.5.3 Read

```c
flash_status_t nvm_read(nvm_ctx_t *ctx, uint16_t id,
                        void *out, uint16_t *len)
{
    uint32_t best_seq = 0;
    uint32_t best_off = 0;
    bool     found    = false;

    uint32_t off  = NVM_SECTOR_HEADER_SIZE;
    uint32_t base = ctx->sector_addr[ctx->active_index];

    while (off < ctx->write_offset) {
        nvm_record_header_t h;
        (void)flash_read(base + off, &h, sizeof h);

        if (h.magic != NVM_MAGIC) break;
        if (crc32(&h, offsetof(nvm_record_header_t, header_crc))
                != h.header_crc) break;

        if (h.id == id && h.sequence >= best_seq) {
            /* Validate the payload before accepting this as the winner:
               a newer but corrupt record must NOT hide an older good one. */
            uint8_t tmp[NVM_MAX_PAYLOAD];
            (void)flash_read(base + off + sizeof h, tmp, h.length);
            if (crc32(tmp, h.length) == h.payload_crc) {
                best_seq = h.sequence;
                best_off = off;
                found    = true;
            }
        }
        off += NVM_ALIGN(sizeof h + h.length);
    }

    if (!found) return FLASH_ERR_PARAM;         /* no valid record */

    nvm_record_header_t h;
    (void)flash_read(base + best_off, &h, sizeof h);
    if (*len < h.length) return FLASH_ERR_PARAM;
    (void)flash_read(base + best_off + sizeof h, out, h.length);
    *len = h.length;
    return FLASH_OK;
}
```

**[INFERENCE]** The subtle line is the payload-CRC check *inside* the "is this newer?" test. A naive implementation picks the highest sequence number and then validates it — and if that record is corrupt, returns failure, even though a perfectly good older copy exists two records back. **A newer corrupt record must never shadow an older valid one.** This single behaviour is the difference between a device that survives power loss and one that loses its calibration.

### 10.5.4 Compaction (garbage collection)

```c
flash_status_t nvm_compact(nvm_ctx_t *ctx)
{
    int src = ctx->active_index;
    int dst = nvm_pick_target_sector(ctx);      /* wear-levelling choice */

    /* 1. Ensure the destination is erased and has a valid header. */
    if (nvm_prepare_sector(ctx, dst) != FLASH_OK) return FLASH_ERR_HARDWARE;

    /* 2. Copy the newest valid record for each live id.
          Order matters: mark the source FULL first, so that a crash
          during copying is detectable and repeatable.               */
    nvm_set_sector_state(ctx, src, NVM_STATE_FULL);

    for (each distinct id in src, newest valid record only) {
        append_record_to(dst, id, payload, ctx->next_sequence++);
    }

    /* 3. Commit: the destination becomes ACTIVE.
          This single word write is the atomic commit point.         */
    nvm_set_sector_state(ctx, dst, NVM_STATE_ACTIVE);

    /* 4. Only now may the source be retired and erased. */
    nvm_set_sector_state(ctx, src, NVM_STATE_OBSOLETE);
    (void)flash_erase_sector(ctx->sector_addr[src]);
    nvm_write_sector_header(ctx, src);          /* erase_count + 1 */

    ctx->active_index  = dst;
    ctx->write_offset  = nvm_scan_frontier(ctx, dst);
    return FLASH_OK;
}
```

**[PRINCIPLE]** The ordering in steps 2–4 is the whole correctness argument. Walk through a power loss at every point:

| Power lost at | State on reboot | Recovery |
|---|---|---|
| Before step 2 | src ACTIVE, dst ERASED | Nothing happened. Continue. |
| During step 2 | src FULL, dst has partial copy, no ACTIVE | Mount sees FULL + no ACTIVE → **redo the compaction from src**. The partial dst is erased and redone. Safe: src is intact. |
| Between 3 and 4 | Two sectors claim relevance: dst ACTIVE, src FULL | Mount prefers ACTIVE; src is retired. Safe. |
| During step 4 erase | dst ACTIVE, src partially erased | Mount uses dst; finishes erasing src. Safe. |

**There is no window in which the data exists nowhere.** That is the definition of a correct commit protocol, and it is achieved by making a *single word write* — the destination's ACTIVE marker — the point at which authority transfers.

## 10.6 Hardware-assisted alternatives

**[VARIES]** Some MCUs provide EEPROM emulation partly or wholly in hardware: a small RAM window backed automatically by a flash region, with a state machine that handles the record management, compaction, and wear levelling for you. Others ship a vendor **library** that implements the algorithms above against their controller.

**[INFERENCE]** Decision guidance:

| Situation | Choice |
|---|---|
| Vendor provides a qualified EEPROM emulation library | **Use it.** It is tested across temperature and validated for power-fail. Your version will not be, for at least a year. |
| Vendor provides a hardware EEE mechanism | Use it, but *understand its partitioning* — it usually consumes flash irreversibly at a configuration step |
| No vendor support, simple needs | Implement the above; it is a few hundred lines |
| No vendor support, complex needs (files, large data) | Consider a proven flash filesystem (Chapter 20) |

The one case where writing your own is clearly right: when you need to control the exact power-fail semantics and prove them in a fault-injection test campaign, and the vendor library's behaviour is not specified to that level.

## 10.7 Chapter exercises

1. Implement `nvm_resolve_dual_active()`. What information do you use to decide, and what do you do with the loser?
2. Why is `header_crc` separate from `payload_crc`? Construct the specific failure that a single combined CRC would cause.
3. Modify the record format to support deletion of an item. (Hint: you cannot erase, so what does "deleted" look like in an append-only log?)
4. Your device has a 256-byte data flash erase unit and you need 40 items of up to 32 bytes. Design the sector layout and compute the compaction frequency.

---

# Chapter 11 — Wear Levelling and Garbage Collection

## 11.1 What wear levelling actually is

**[PRINCIPLE]** Wear levelling is the practice of ensuring that erase cycles are distributed roughly evenly across physical erase units, so that no single unit reaches end of life while the rest of the array is nearly unused.

Without it, the lifetime of your storage equals the lifetime of its **most-erased sector**, not its average.

Two flavours:

| Type | Mechanism | Cost |
|---|---|---|
| **Dynamic** | Levels wear only across *actively rewritten* data | Cheap; but static data pins its sectors forever, so those sectors never wear while others wear out |
| **Static** | Periodically relocates *cold* data too, freeing low-wear sectors into the rotation | More erases in total, but far better worst-case distribution |

**[INFERENCE]** Dynamic levelling is adequate when nearly all your stored data is hot. Static levelling matters when you have a mix — e.g. calibration constants written once at manufacture sitting in the same pool as an event log. In that case, dynamic levelling alone will cycle a small subset of sectors to death while the calibration sectors sit at erase count 1.

## 11.2 The design exercises

### Exercise A — the baseline

*A device saves a 32-byte configuration whenever it changes. One 4 KB sector. Endurance 10,000 cycles.*

**Design A1 — erase-and-rewrite per save**
```
   erases per save      = 1
   saves per lifetime   = 10,000
```
If configuration changes once per hour: `10,000 h ≈ 417 days ≈ 1.1 years`. Unacceptable for a ten-year product.

**Design A2 — append-only, one sector**
```
   usable space   = 4096 − sector_header (say 32) = 4064 B
   record size    = 32 B payload + 24 B header = 56 → aligned to 56 or 64
   records/sector = 4064 / 64 = 63
   erases per save = 1/63
   saves per lifetime = 10,000 × 63 = 630,000
```
At one change per hour: `630,000 h ≈ 71 years`. **A 63× improvement from a data-structure change alone, with no extra flash.**

### Exercise B — adding sectors

**Design B1 — two sectors**
```
   Compaction needs a spare sector, so with 2 sectors only 1 is
   holding live data at a time.
   Erases are shared between 2 sectors → each sees half.
   saves per lifetime ≈ 10,000 × 63 × 2 = 1,260,000
```

**Design B2 — four sectors**
```
   saves per lifetime ≈ 10,000 × 63 × 4 = 2,520,000
```

**[PRINCIPLE]** The scaling is linear in sector count and linear in records-per-sector. Both are cheap. **The single highest-leverage change is almost always switching from rewrite to append**, because that factor is `sector_size / record_size`, which is typically 50–500×.

### Exercise C — the honest version

The clean arithmetic above assumes:

- Every erase is perfectly distributed. **In reality, rotation is round-robin only if you make it so** — a naive "use the next erased sector" policy can favour a subset.
- Compaction copies live data, which consumes space and adds writes. If you have K live items each of size R, every compaction writes K×R bytes of overhead. With a nearly-full working set, `records_per_sector` in the formula must be replaced by `(records_per_sector − K)` — the *net* new records absorbed per erase.
- Metadata (sector headers, state markers) consumes space.

**[INFERENCE]** The corrected model:

```
                       endurance × sector_count × (records_per_sector − live_items)
   saves_per_lifetime ≈ ───────────────────────────────────────────────────────────
                                              1
```

Note the failure mode this exposes: **as `live_items` approaches `records_per_sector`, the denominator of useful work collapses and write amplification explodes.** A store that is 90% full does far more erases per logical write than one that is 40% full. This is exactly the SSD "over-provisioning" phenomenon, appearing at MCU scale.

**Design rule [INFERENCE]: size your NVM pool so that live data occupies well under half of one sector's capacity.** If you have 20 items totalling 400 bytes, do not give the store 1 KB. Give it four 4 KB sectors. Flash is cheaper than field failures.

## 11.3 Choosing the next sector

```c
/* GENERIC EXAMPLE — static wear levelling with erase-count bias */

int nvm_pick_target_sector(nvm_ctx_t *ctx)
{
    int      best  = -1;
    uint32_t least = UINT32_MAX;

    for (int i = 0; i < NVM_SECTOR_COUNT; i++) {
        if (i == ctx->active_index) continue;
        uint32_t ec = nvm_read_erase_count(ctx, i);
        if (ec < least) { least = ec; best = i; }
    }
    return best;
}
```

The `erase_count` lives in the sector header and is rewritten (with the header) each time the sector is prepared. **[INFERENCE]** This is enough for static levelling *if* cold data participates: periodically — say every N compactions, or when the spread between max and min erase count exceeds a threshold — force a compaction that relocates the coldest sector's contents, putting that low-wear sector back into circulation.

```c
bool nvm_should_force_static_leveling(nvm_ctx_t *ctx)
{
    uint32_t min = UINT32_MAX, max = 0;
    for (int i = 0; i < NVM_SECTOR_COUNT; i++) {
        uint32_t ec = nvm_read_erase_count(ctx, i);
        if (ec < min) min = ec;
        if (ec > max) max = ec;
    }
    return (max - min) > NVM_WEAR_SPREAD_THRESHOLD;
}
```

## 11.4 When garbage collection should run

**[PRINCIPLE]** Compaction is the expensive operation: an erase plus a copy of all live data. Running it lazily (only when the store is full) means it happens at an unpredictable moment — potentially in the middle of a time-critical operation, or during a brown-out.

**[INFERENCE]** Better policies:

| Policy | Description | Suits |
|---|---|---|
| **Lazy** | Compact when full | Simple systems; acceptable if you can tolerate the pause |
| **Threshold** | Compact when free space drops below X% | Most systems. Gives you slack to defer if busy. |
| **Scheduled** | Compact during a known idle window | Systems with a duty cycle (a meter reading once a minute has 59 free seconds) |
| **Opportunistic** | Compact when free space is low **and** the system is idle **and** supply is healthy | Best. Combines the above. |

**[INFERENCE]** In an RTOS design, express this as: the NVM task compacts when `free < threshold && system_idle && supply_ok`, and *never* inside a write call unless the store is genuinely full. That way the common-case write latency is a few hundred microseconds, and the rare compaction happens when you can afford it.

## 11.5 Chapter exercises

1. With 8 sectors of 2 KB, 64-byte records, 25 live items, and 100,000-cycle endurance: compute saves-per-lifetime with the honest model. Then recompute with 50 live items and explain the difference.
2. Why does dynamic wear levelling fail when calibration data shares a pool with an event log?
3. Design a policy that guarantees compaction never runs while the device is executing a critical measurement. What must the write API return when the store is full and compaction is deferred?
4. Explain the analogy between this chapter and SSD over-provisioning.

---

# Chapter 12 — Power Failure

## 12.1 Why this is the chapter that separates products from projects

Every flash operation is a multi-step, millisecond-scale process. Power loss is asynchronous and unpredictable. In a fleet of a hundred thousand devices, "the improbable moment" occurs daily.

**[PRINCIPLE]** You do not prevent power loss during flash operations. **You design so that every possible interruption point leaves the system in a recoverable state.** The design question is never "what if power fails?" It is "for each of these N interruption points, what does the next boot see, and what does it do?"

## 12.2 The taxonomy of interrupted operations

```
   ┌─────────────────────────────────────────────────────────────┐
   │ INTERRUPTED ERASE                                            │
   │ Cells partially moved. The sector reads as neither erased nor│
   │ its old contents. On ECC parts, reads may return ECC faults. │
   │ RECOVERY: the sector must be erased again. It is not         │
   │ trustworthy until a full erase completes and blank-check     │
   │ passes. Data in it is gone — this must be acceptable by      │
   │ design, i.e. nothing may be uniquely stored there.           │
   └─────────────────────────────────────────────────────────────┘

   ┌─────────────────────────────────────────────────────────────┐
   │ INTERRUPTED PROGRAM ("torn write")                           │
   │ Some words programmed, some not, possibly one word partially │
   │ with marginal cell states.                                   │
   │ RECOVERY: detected by CRC. The record is discarded. The      │
   │ PREVIOUS record remains valid.                               │
   │ ⚠ Marginal cells may read correctly today and fail later.    │
   │   This is why you must ERASE and rewrite the region rather   │
   │   than "program over" a torn record.                         │
   └─────────────────────────────────────────────────────────────┘

   ┌─────────────────────────────────────────────────────────────┐
   │ INTERRUPTED MULTI-STEP TRANSACTION                           │
   │ E.g. compaction: source marked FULL, destination partially   │
   │ written, no ACTIVE sector.                                   │
   │ RECOVERY: state machine + idempotent redo. See §10.5.4.      │
   └─────────────────────────────────────────────────────────────┘

   ┌─────────────────────────────────────────────────────────────┐
   │ INTERRUPTED FIRMWARE UPDATE                                  │
   │ Application partially erased or partially written.           │
   │ RECOVERY: the bootloader must detect an invalid image and    │
   │ not jump to it. Requires that the bootloader itself was      │
   │ never at risk. See Chapters 14-15.                           │
   └─────────────────────────────────────────────────────────────┘

   ┌─────────────────────────────────────────────────────────────┐
   │ INTERRUPTED CONFIG/OPTION REGION UPDATE  ⚠ WORST CASE        │
   │ Boot policy, protection bits, or debug settings half-written.│
   │ RECOVERY: often NONE. The device may not boot and may not be │
   │ reprogrammable. Some parts define a failure-handling policy; │
   │ many do not.                                                 │
   │ ⚠ NEVER update config regions in the field unless the device │
   │   documents a safe recovery path.                            │
   └─────────────────────────────────────────────────────────────┘
```

**[INFERENCE]** That last box is worth a policy decision at architecture time: **config/option region writes belong in the factory, not in the field.** If a feature requires changing boot configuration at runtime, push back hard. The failure mode is an unreturnable, unrecoverable brick, and it cannot be fixed by a better bootloader because the bootloader is what stops running.

## 12.3 The commit protocol, generalised

**[PRINCIPLE]** Every power-safe flash update has this shape:

```
   1. PREPARE   — write the new data somewhere that is not yet
                  authoritative. Any interruption here is invisible.

   2. VALIDATE  — verify the new data is complete and correct
                  (CRC / hash / readback). Still not authoritative.

   3. COMMIT    — perform ONE atomic operation that transfers
                  authority from old to new.

   4. CLEAN UP  — reclaim the old data. Any interruption here is
                  harmless because authority has already moved.
```

**[PRINCIPLE]** The entire correctness of the scheme rests on step 3 being **atomic**: it must either happen or not happen, with no partial state that could be misread as either.

### 12.3.1 What can serve as an atomic commit?

| Mechanism | Atomicity argument | Availability |
|---|---|---|
| **Single flash-word program** | The smallest indivisible unit the hardware offers. A torn word fails its CRC or ECC, and is therefore *not* a valid commit marker — so it reads as "not committed." | Universal |
| **Sequence number** | Higher sequence wins. A torn new record is invalid, so the old one still wins. No explicit commit needed. | Universal — this is why append-only is so robust |
| **Hardware bank/boot swap** | A dedicated hardware bit or bank remap, designed by the vendor to be atomic across reset | **[VARIES]** — device-specific |
| **Two redundant copies + CRC + sequence** | At least one copy is always valid, because you never write both simultaneously | Universal |

**[INFERENCE]** Note the important subtlety in row 1: a single-word program is *not* atomic in the sense that the word cannot be partially programmed — it can. It is atomic in the sense that **a partially-programmed word is detectably invalid**, so it never reads as a successful commit. That distinction is what makes the whole scheme work, and it is why the commit word must be CRC-protected or ECC-protected. A raw commit flag with no integrity check is not an atomic commit; it is a race.

## 12.4 Redundant configuration — the two-copy pattern

For a small, critical structure (calibration, device identity, boot policy) that must *always* be readable:

```
   ┌──────────────────────┐      ┌──────────────────────┐
   │  COPY A              │      │  COPY B              │
   │  magic               │      │  magic               │
   │  sequence = N        │      │  sequence = N+1      │
   │  payload             │      │  payload             │
   │  crc                 │      │  crc                 │
   └──────────────────────┘      └──────────────────────┘
        (separate erase units — this is mandatory)
```

**Update procedure:**
```
   1. Determine which copy is currently newest and valid.
   2. Erase the OTHER copy's sector.
   3. Program the new data with sequence = newest + 1.
   4. Verify. If verification fails, the old copy still governs.
```

**Read procedure:**
```
   Read both. Discard any that fail magic or CRC.
   If none valid → unprovisioned or catastrophic loss → recovery path.
   If one valid   → use it; schedule a repair of the other.
   If both valid  → use the higher sequence number.
```

**[PRINCIPLE]** They must be in **separate erase units**. Two copies in one sector are not redundant — one erase destroys both. This is the most common way this pattern is implemented wrongly.

**[INFERENCE]** Also handle sequence-number wraparound. With a 32-bit counter and a realistic update rate you will never wrap, but write the comparison as `(int32_t)(a - b) > 0` rather than `a > b` anyway. It costs nothing and removes a whole class of latent bug.

## 12.5 Hardware you need to make this work

**[PRINCIPLE]** Software cannot solve power failure alone. The hardware contract:

| Element | Requirement | Why |
|---|---|---|
| **Brown-out detector** | Threshold at or above the flash **programming** minimum, not the CPU minimum | Prevents starting an operation that cannot complete correctly |
| **Bulk capacitance** | Hold-up time ≥ worst-case single flash operation + margin | Lets an in-flight operation finish |
| **Early power-fail warning** | An interrupt at a threshold *above* the brown-out reset | Gives you a window to flush and to *refuse to start* new operations |
| **Supply monitoring in software** | Check before every erase/program | Turns an unpredictable corruption into a clean, reported failure |

**Hold-up time estimate [GENERAL]:**
```
   t_holdup ≈ C × (V_start − V_min) / I_load
```

**Worked example [INFERENCE — invented numbers]:**
```
   C        = 470 µF
   V_start  = 3.0 V   (power-fail interrupt threshold)
   V_min    = 2.7 V   (flash programming minimum)
   I_load   = 40 mA

   t_holdup ≈ 470e-6 × 0.3 / 0.040  =  3.5 ms
```
If your worst-case program of one flash word is 100 µs, you can program roughly 35 words — perhaps 280 bytes — before the rail collapses. **That is your power-fail flush budget.** Design the emergency-save payload to fit inside it, with margin, and *measure* it on real hardware rather than trusting the arithmetic: real loads are not constant, and regulator dropout behaviour matters.

## 12.6 The fault-injection discipline

**[INFERENCE]** A power-fail-safe design that has not been tested by actually removing power is a hypothesis, not a design. The test method that finds real bugs:

```
   Automated rig:
     - relay or MOSFET in the supply line, under test-computer control
     - device under test running a workload that continuously writes NVM
     - a randomised delay before each power cut, spanning 0…T_max
       where T_max exceeds the longest flash operation
     - after each cut: restore power, wait for boot, query the device
       for data integrity, and log the result
     - run for tens of thousands of cycles
```

What you are looking for:
- Any boot that does not complete
- Any data item that is neither the old value nor the new value
- Any store that becomes permanently unmountable
- Any monotonic counter that goes backwards
- Steadily growing erase counts (indicating a compaction loop)

**[INFERENCE]** The cuts that find bugs are concentrated in narrow windows — during the commit word, at the erase boundary, during compaction. Randomised timing eventually hits them; biased timing (deliberately cutting power a fixed delay after a write begins, swept across the operation duration) finds them faster. If you have one week to validate NVM, spend it here rather than on code review.

## 12.7 Chapter exercises

1. For each of the five interrupted-operation types, state what the next boot observes and what your code does about it.
2. Explain why a plain "committed = 1" flag with no CRC is not a valid atomic commit.
3. Why must redundant copies live in separate erase units?
4. Design the power-fail flush for a system whose hold-up budget is 2 ms and whose flash programs 8 bytes in 60 µs. What is the maximum payload, and what do you leave out?
5. Sketch the fault-injection rig you would build, and the five assertions you would check after each power cut.

---

---

# PART IV — FIRMWARE, BOOT, AND UPDATE

---

# Chapter 13 — Reset, Vector Tables, and the Linker

## 13.1 What the CPU does at reset

**[PRINCIPLE]** Every architecture answers two questions at reset:

1. **Where is the initial program counter?**
2. **Where is the initial stack pointer?** (on architectures where hardware sets it)

**[VARIES]** The mechanism differs by architecture family:

| Mechanism | Description |
|---|---|
| **Vector table at a fixed address** | Hardware reads the first entries of a table at a known location; typically word 0 = initial SP, word 1 = reset handler address. Common on modern 32-bit cores. |
| **Fixed reset address** | The PC is simply set to a constant; a jump instruction there routes to your code. Common on 8/16-bit architectures. |
| **Boot ROM first** | Vendor ROM runs first, examines boot-mode pins/fuses and configuration data, then transfers control. Common on parts with external boot storage. |
| **Reset vector in a configuration region** | The address is a programmable option word, not a fixed location. |

**[INFERENCE]** Answering "what actually executes first on this part?" is question 14 of the twenty, and on many modern MCUs the honest answer is "vendor boot ROM you cannot see, which then decides whether to run your code at all." That ROM's decision logic — driven by boot-mode pins, fuses, and the configuration region — is where a large fraction of "my board is bricked" reports originate.

## 13.2 The vector table

```
   ADDRESS                CONTENTS
   ─────────────────────────────────────────────────
   VECTOR_BASE + 0x00     Initial stack pointer          ← loaded by hardware
   VECTOR_BASE + 0x04     Reset handler address          ← loaded by hardware
   VECTOR_BASE + 0x08     Fault / NMI handler
   VECTOR_BASE + 0x0C     Fault handler
        ⋮                 (architectural exceptions)
   VECTOR_BASE + 0x40     IRQ 0 handler
   VECTOR_BASE + 0x44     IRQ 1 handler
        ⋮                 (device interrupts)
```

**[PRINCIPLE]** Two facts about the vector table dominate bootloader design:

1. **It lives in flash.** Therefore it is in the region you may want to erase, and it is subject to ECC and to the read/write hazards of Chapters 6–8.
2. **There is only one active vector table at a time.** If the bootloader and the application each have one, something must switch between them.

### 13.2.1 Vector relocation

**[VARIES]** Architectures offer different relocation mechanisms:

| Mechanism | How it works | Notes |
|---|---|---|
| **Relocation register** (e.g. a vector table offset register) | Software writes the new table's base address; the core uses it for subsequent exceptions | Cleanest. Requires the register to exist — **not all cores have one** |
| **RAM vector table** | The table is copied to RAM and the core is pointed there, or the hardware always uses a fixed RAM location | Allows runtime handler changes; costs RAM |
| **Memory remap** | A hardware bit remaps which physical memory appears at the vector address | Common on parts with multiple boot sources |
| **Trampoline table** | The fixed table contains jumps to a second, relocatable table | Software workaround when no hardware support exists; adds latency to every interrupt |

**[INFERENCE]** Check this early. "This core has no vector relocation register" changes your bootloader architecture fundamentally: you must either use a trampoline (adding cycles to every ISR), place the shared vector table in a region neither image erases, or accept that the bootloader owns all interrupt handling.

## 13.3 The linker: how firmware gets its address

**[PRINCIPLE]** The linker's job is to assign every symbol an address and to describe where each section is **stored** versus where it must **be** at runtime.

### 13.3.1 The vocabulary

| Term | Meaning |
|---|---|
| **VMA** (virtual/execution address) | Where a section must be when the code runs |
| **LMA** (load address) | Where the section's initial contents are *stored* in the image |
| `.text` | Executable code. VMA = LMA = flash. |
| `.rodata` | Constants. VMA = LMA = flash (read directly). |
| `.data` | Initialised variables. **VMA = RAM, LMA = flash.** Startup code copies flash→RAM. |
| `.bss` | Zero-initialised variables. VMA = RAM, **no LMA** — startup code zeroes it. |
| `.isr_vector` | The vector table. Must be placed at the address the hardware expects. |
| `.ramfunc` / `.fast` | Code that must execute from RAM. **VMA = RAM, LMA = flash.** Copied at startup. |

**[PRINCIPLE]** The VMA/LMA distinction is the single concept that makes startup code comprehensible. `.data` has to live in flash (it must survive power-off) but has to be writable (so it must be in RAM). The linker records both addresses; the startup code performs the copy.

### 13.3.2 A representative linker script

```ld
/* GENERIC EXAMPLE — GNU ld syntax, symbolic addresses */

MEMORY
{
    /* The bootloader occupies the start of flash and is protected. */
    BOOT   (rx)  : ORIGIN = FLASH_BASE,             LENGTH = BOOT_SIZE
    /* The application begins after it. */
    FLASH  (rx)  : ORIGIN = FLASH_BASE + BOOT_SIZE, LENGTH = APP_SIZE
    RAM    (rwx) : ORIGIN = RAM_BASE,               LENGTH = RAM_SIZE
}

ENTRY(Reset_Handler)

SECTIONS
{
    /* The vector table must be the first thing in the application. */
    .isr_vector : ALIGN(VECTOR_ALIGNMENT)
    {
        __app_start = .;
        KEEP(*(.isr_vector))
    } > FLASH

    .text :
    {
        *(.text*)
        *(.glue_7) *(.glue_7t)
        . = ALIGN(4);
    } > FLASH

    .rodata :
    {
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    /* Code that must run from RAM: stored in flash, executed in RAM. */
    .ramfunc : ALIGN(4)
    {
        __ramfunc_start__ = .;
        *(.ramfunc*)
        . = ALIGN(4);
        __ramfunc_end__ = .;
    } > RAM AT> FLASH              /* VMA = RAM, LMA = FLASH */
    __ramfunc_load__ = LOADADDR(.ramfunc);

    /* Initialised data: same VMA/LMA split. */
    .data : ALIGN(4)
    {
        __data_start__ = .;
        *(.data*)
        . = ALIGN(4);
        __data_end__ = .;
    } > RAM AT> FLASH
    __data_load__ = LOADADDR(.data);

    .bss (NOLOAD) : ALIGN(4)
    {
        __bss_start__ = .;
        *(.bss*) *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > RAM

    /* Image footer: leave room for the integrity metadata the
       post-build tool will insert. Its placement must be
       deterministic so the bootloader can find it.               */
    .image_trailer ALIGN(4) :
    {
        __image_end__ = .;
        . += IMAGE_TRAILER_SIZE;
    } > FLASH

    __app_size = __image_end__ - __app_start;
}
```

### 13.3.3 The startup code that makes it work

```c
/* GENERIC EXAMPLE */

extern uint32_t __data_start__, __data_end__, __data_load__;
extern uint32_t __bss_start__,  __bss_end__;
extern uint32_t __ramfunc_start__, __ramfunc_end__, __ramfunc_load__;

void Reset_Handler(void)
{
    /* 1. Configure flash wait states BEFORE raising the clock.       */
    flash_set_wait_states_for(TARGET_CLOCK_HZ);
    clock_init(TARGET_CLOCK_HZ);

    /* 2. Copy .ramfunc from its LMA in flash to its VMA in RAM.      */
    copy_section(&__ramfunc_load__, &__ramfunc_start__, &__ramfunc_end__);

    /* 3. Copy .data from flash (LMA) to RAM (VMA).                   */
    copy_section(&__data_load__, &__data_start__, &__data_end__);

    /* 4. Zero .bss.                                                  */
    zero_section(&__bss_start__, &__bss_end__);

    /* 5. Point the core at this image's vector table (if supported). */
    vector_table_relocate((uint32_t)&__app_start);

    /* 6. C runtime init, then main.                                  */
    __libc_init_array();
    main();
    for (;;) { }
}
```

**[INFERENCE]** Step 1 before step 2 is not stylistic. Steps 2–4 execute *from flash* at the new clock rate. If wait states are wrong, the copy loop itself fetches corrupted instructions.

## 13.4 What changes when you move the application

**[PRINCIPLE]** Relocating an application from address X to address Y touches **six** artefacts. Missing any one produces a distinct, confusing symptom:

| Artefact | Change required | Symptom if missed |
|---|---|---|
| **Linker script** | `ORIGIN` of the application region | Image built for the wrong place; jumps to nothing |
| **Vector table location** | Table now at the new base | Interrupts vector to the bootloader's handlers, or to garbage |
| **Vector relocation call** | Must point at the new base *after* the jump | Interrupts work in the bootloader and break in the application |
| **Bootloader jump target** | Reads SP and PC from the new base | Bootloader jumps to the old address |
| **Debugger configuration** | Load offset / symbol offset | Breakpoints in the wrong place; source view desynchronised |
| **Image generation & signing** | Header offsets, CRC/hash range, flash programming address | Verification fails, or verifies the wrong bytes |

**[INFERENCE]** The remedy is to make the offset a **single build-time parameter** consumed by all six: linker script, startup, bootloader header, post-build tool, debugger script, and production programmer configuration. Hard-coding it in more than one place guarantees they will diverge on the day someone changes the bootloader size.

## 13.5 Chapter exercises

1. Explain the difference between VMA and LMA using `.data` and `.ramfunc` as examples.
2. Your application's interrupts work when it is loaded standalone by a debugger and fail when it is launched by the bootloader. Name the two most likely causes.
3. Why must wait states be configured before the `.data` copy loop runs?
4. List the six artefacts affected by changing the application base address, and describe the symptom of missing each.

---

# Chapter 14 — Bootloader Architecture

## 14.1 What a bootloader is for

**[PRINCIPLE]** A bootloader exists to answer one question at every power-on:

> *Which image should run, and is it safe to run it?*

Everything else — receiving updates, erasing, programming — is secondary. A bootloader that programs images beautifully but cannot correctly refuse a corrupt one is worse than useless, because it turns a recoverable situation into a brick.

## 14.2 The boot chain

```
   ┌──────────────────────────────────────────────────────────────┐
   │  RESET                                                       │
   └──────────────────────────┬───────────────────────────────────┘
                              ▼
   ┌──────────────────────────────────────────────────────────────┐
   │  [VARIES] VENDOR BOOT ROM (mask ROM, immutable)              │
   │   - reads boot mode: pins, fuses, config region              │
   │   - may offer a serial/USB recovery loader                   │
   │   - may authenticate the next stage                          │
   │   - transfers control                                        │
   └──────────────────────────┬───────────────────────────────────┘
                              ▼
   ┌──────────────────────────────────────────────────────────────┐
   │  YOUR BOOTLOADER (flash, write-protected)                    │
   │   - minimal hardware init                                    │
   │   - read boot control block                                  │
   │   - validate candidate image(s)                              │
   │   - decide, or enter update mode                             │
   │   - hand over                                                │
   └──────────────────────────┬───────────────────────────────────┘
                              ▼
   ┌──────────────────────────────────────────────────────────────┐
   │  APPLICATION                                                 │
   └──────────────────────────────────────────────────────────────┘
```

## 14.3 The boot decision

```
                    ┌─────────────┐
                    │    BOOT     │
                    └──────┬──────┘
                           ▼
                ┌──────────────────────┐
                │ Forced update mode?  │  (button, pin, host command,
                │                      │   magic value in RAM/NVM)
                └───┬──────────────┬───┘
                  yes             no
                   │               │
                   ▼               ▼
            ┌────────────┐   ┌───────────────────────┐
            │ UPDATE     │   │ Read boot control     │
            │ MODE       │   │ block → preferred slot│
            └────────────┘   └───────────┬───────────┘
                                         ▼
                             ┌───────────────────────┐
                             │ Validate preferred    │
                             │  magic / size / range │
                             │  CRC or hash          │
                             │  signature (if secure)│
                             │  version ≥ min        │
                             └───┬───────────────┬───┘
                               valid          invalid
                                 │               │
                                 ▼               ▼
                    ┌────────────────────┐  ┌────────────────────┐
                    │ boot-attempt count │  │ Validate the OTHER │
                    │ exceeded?          │  │ slot               │
                    └───┬────────────┬───┘  └───┬────────────┬───┘
                       yes          no        valid       invalid
                        │            │          │             │
                        ▼            ▼          ▼             ▼
                 ┌───────────┐  ┌────────┐ ┌────────┐  ┌────────────┐
                 │ mark bad, │  │ RUN IT │ │ RUN IT │  │ RECOVERY   │
                 │ try other │  │        │ │        │  │ MODE       │
                 └───────────┘  └────────┘ └────────┘  └────────────┘
```

**[INFERENCE]** Four decisions in that diagram are the ones people get wrong:

1. **Forced update must be checked first**, and must not depend on any flash contents being valid. It is your last line of defence when everything else is corrupt. A GPIO or a host command over a fixed-configuration UART.
2. **Validation happens before every boot, not only after an update.** Retention failure, ECC degradation, and partial erases from a previous crash all produce images that were valid yesterday and are not today.
3. **A boot-attempt counter is mandatory for any A/B system.** An image can pass CRC and still crash on boot — a bug, not a corruption. Without a counter, you have an infinite reboot loop. The counter is incremented *before* the jump and cleared by the application once it has proven itself healthy.
4. **Recovery mode must exist and must be reachable.** "Both images are invalid" is a state that will occur. If your answer is "return the unit," you have designed a product with a field-failure mode instead of a fault-tolerant one.

## 14.4 The handover

**[PRINCIPLE]** Transferring control is more than a function call. The bootloader must leave the machine in a state the application can assume.

```c
/* GENERIC EXAMPLE — the shape is universal; details are architecture-specific */

typedef void (*app_entry_t)(void);

static void jump_to_application(uint32_t app_base)
{
    /* 1. Stop everything that generates events. */
    disable_all_interrupts();
    disable_all_peripheral_interrupts();
    stop_systick_and_timers();
    disable_dma();

    /* 2. Undo what the bootloader configured, so the application
          starts from a known state and its own init is not confused
          by a peripheral that is already half-configured.          */
    deinit_communication_peripherals();
    reset_clock_to_boot_default();      /* or document what you leave on */

    /* 3. Flush and disable caches; ensure no stale instruction
          fetches from the region we just programmed.               */
    flush_and_disable_caches();
    invalidate_prefetch();

    /* 4. Retarget the vector table (if the core supports it).      */
    vector_table_relocate(app_base);

    /* 5. Load the application's stack pointer from its vector table,
          then jump to its reset handler.                           */
    uint32_t app_sp    = *(volatile uint32_t *)(app_base + 0);
    uint32_t app_entry = *(volatile uint32_t *)(app_base + 4);

    set_main_stack_pointer(app_sp);
    memory_barrier();
    ((app_entry_t)app_entry)();

    /* Never returns. */
    for (;;) { }
}
```

**[INFERENCE]** Each step corresponds to a real class of field bug:

| Skipped step | Symptom |
|---|---|
| Interrupts not disabled | A pending interrupt fires mid-handover, vectoring through a half-relocated table |
| Peripherals not de-initialised | Application init hangs waiting for a peripheral the bootloader left busy; or a DMA writes into the application's RAM |
| Clock left at bootloader's setting | Application assumes reset defaults; its timing is wrong, or wait states no longer match |
| Cache not flushed | The CPU executes the *old* application from cache |
| Vector table not relocated | Interrupts go to the bootloader's handlers, which reference variables the application has since overwritten |
| Stack pointer not set | The application's first push corrupts the bootloader's stack region |

**[INFERENCE]** The most robust handover on many parts is not a jump at all: **write the boot decision to a persistent location and perform a software reset.** The hardware then does all the de-initialisation for you, deterministically, and the bootloader reads the decision on the next pass and jumps immediately. It costs one reset cycle and eliminates the entire table above. On any product where boot time is not critical, take it.

## 14.5 Protecting the bootloader

**[PRINCIPLE]** The bootloader is the recovery mechanism. If it can be destroyed, there is no recovery. Layered defences:

| Layer | Mechanism | Defeats |
|---|---|---|
| 1 | **Non-volatile write protection** on the bootloader sectors, set in production | Application bugs, wild pointers, malicious application code |
| 2 | **Volatile protection** set by the bootloader early in boot | Bootloader-stage bugs after the protection window |
| 3 | **Range checks in the flash driver** — refuse any address inside the boot region | Ordinary programming errors, caught early with a clear error |
| 4 | **Never update the bootloader in the field** | The entire class of "bricked during bootloader update" |
| 5 | If layer 4 is impossible: **hardware swap** or a two-stage bootloader (see below) | — |

**[INFERENCE]** Layer 4 deserves emphasis. Updating a bootloader in the field is the single most dangerous operation an embedded product can perform, because it is the one operation where a failure removes the ability to retry. Architect so that the bootloader never needs updating: keep it small, keep its protocol versioned and forward-compatible, and put all volatile logic in the application.

### 14.5.1 When you must update the bootloader

**[VARIES]** Two hardware mechanisms exist on various devices:

**Boot-area swap.** The device has two boot regions and a hardware flag selecting which one appears at the boot address. Procedure: program the new bootloader into the *inactive* region while the active one still runs, verify it, then flip the flag. The flip is atomic and survives reset — which is exactly the atomic-commit property of §12.3.

**Bank/address swap.** A larger-granularity version of the same idea applied to whole banks, used mainly for application A/B update but usable for a combined image.

**[INFERENCE]** Where neither exists, the software equivalent is a **two-stage bootloader**: a tiny, never-updated stage 1 whose only job is to validate and launch one of two copies of stage 2. Stage 1 is a few hundred bytes, is written once, is easy to review exhaustively, and is protected forever. Stage 2 carries all the update logic and can be replaced safely because stage 1 will fall back if it is bad.

## 14.6 Chapter exercises

1. List every step of the handover and the symptom of skipping it.
2. Argue for and against the "write decision + software reset" handover style.
3. Design a two-stage bootloader for a part with no boot-swap hardware. What exactly is in stage 1, and how large is it?
4. An A/B system with CRC validation still ends up in a reboot loop. What was missing?

---

# Chapter 15 — Firmware Images and Validation

## 15.1 What a firmware binary contains

| Format | Contains | Suitable for |
|---|---|---|
| **ELF** | Sections, symbols, debug info, load addresses, relocations | Debugging; the build's source of truth |
| **`.hex` / `.s19`** | Text records: address + data + per-record checksum. Address information is present. | Programming tools; supports non-contiguous regions |
| **`.bin`** | Raw bytes. **No address, no length, no version, no integrity.** | Flashing to a known offset; the OTA payload |

**[PRINCIPLE]** A `.bin` is not a firmware image; it is the *contents* of one. It carries nothing that lets a bootloader answer "is this complete, is this for this product, is this newer than what I have, and is it authentic?" Those questions require metadata, and metadata means a **header**.

## 15.2 The image header

```c
/* GENERIC EXAMPLE */

#define IMAGE_MAGIC     0x494D4731u    /* "IMG1" */

typedef struct {
    uint32_t magic;             /* identifies this as an image header      */
    uint16_t header_version;    /* format version of THIS structure        */
    uint16_t header_size;       /* bytes; lets a newer header be skipped   */

    uint32_t device_id;         /* which product/hardware this is for      */
    uint32_t image_version;     /* monotonic; used for rollback policy     */
    uint32_t min_version;       /* anti-rollback floor                     */

    uint32_t image_size;        /* payload bytes covered by hash/CRC       */
    uint32_t load_address;      /* where it must be programmed             */
    uint32_t entry_point;       /* optional; usually derived from vectors  */

    uint32_t flags;             /* encrypted, compressed, delta, ...       */
    uint32_t build_timestamp;

    uint8_t  payload_hash[32];  /* SHA-256 over image_size bytes           */
    uint8_t  signature[64];     /* over the header (minus this field)      */
                                /*  + payload_hash                         */
    uint32_t header_crc;        /* integrity of the header itself          */
} image_header_t;
```

### 15.2.1 Every field justified

| Field | Why it must exist | What breaks without it |
|---|---|---|
| `magic` | Distinguishes an image from erased flash or from unrelated data | Bootloader jumps into blank flash or into a data partition |
| `header_version` / `header_size` | Lets an old bootloader safely skip fields it does not understand | Your bootloader, which you swore never to update, cannot parse next year's images |
| `device_id` | Prevents flashing the wrong product's firmware | A support call, or a destroyed unit if peripherals differ |
| `image_version` | Ordering; update policy; telemetry | Cannot tell newer from older; cannot detect a downgrade |
| `min_version` | Anti-rollback floor | An attacker installs an old, vulnerable-but-validly-signed image |
| `image_size` | Bounds the integrity check | You hash the wrong number of bytes; or you read past the partition |
| `load_address` | Verifies the image matches the slot | Programming a slot-A image into slot B; vectors point to the wrong place |
| `payload_hash` | Detects any corruption, wherever introduced | Corrupt images boot |
| `signature` | Establishes *authenticity*, not just integrity | Anyone can craft a valid image (see §15.4) |
| `header_crc` | Lets you trust the header before using its fields | You act on a corrupt `image_size` and read out of bounds |

**[INFERENCE]** `header_crc` is the same principle as the two-CRC record design of Chapter 10, and for the same reason: **you must be able to validate the metadata before you rely on it.** A bootloader that reads `image_size` from an unvalidated header and then hashes that many bytes has a remote out-of-bounds read triggered by a corrupt flash sector.

### 15.2.2 Where to put the header

**[VARIES]** Three placements, each with a trade-off:

| Placement | Advantage | Disadvantage |
|---|---|---|
| **Prepended** (before the vector table) | Simple to find and parse | Shifts the application's base address; the vector table is no longer at the slot start, which some hardware requires |
| **Appended** (trailer after the payload) | Application layout unchanged | Requires knowing the image length to find it, or a fixed slot-relative offset |
| **Separate metadata sector** | Independent of the image; can be updated atomically without touching the image | Costs a sector; two things to keep consistent |

**[INFERENCE]** For A/B systems, the separate-metadata approach is usually cleanest: a small **boot control block** holds, for each slot, the header and the state (empty / candidate / testing / confirmed / bad). Updating slot state then does not require touching the image at all, and the boot control block itself can be made redundant using the two-copy pattern of §12.4.

## 15.3 Validation, layer by layer

**[PRINCIPLE]** These are four **different** properties. Meeting one does not imply the others.

```
   ┌──────────────────────────────────────────────────────────────┐
   │ 1. PLAUSIBILITY  — magic correct? size within the slot?      │
   │                    load_address matches this slot?           │
   │                    device_id matches this hardware?          │
   │    Costs microseconds. Rejects almost all garbage.           │
   └──────────────────────────────────────────────────────────────┘
   ┌──────────────────────────────────────────────────────────────┐
   │ 2. INTEGRITY     — CRC or hash over the payload.             │
   │    Detects: torn writes, bit rot, transmission errors.       │
   │    Does NOT detect: a deliberately crafted image.            │
   └──────────────────────────────────────────────────────────────┘
   ┌──────────────────────────────────────────────────────────────┐
   │ 3. AUTHENTICITY  — signature verified against a public key   │
   │                    held in immutable storage.                │
   │    Detects: an image not produced by you.                    │
   │    Requires: secure key storage + secure boot (Ch 19).       │
   └──────────────────────────────────────────────────────────────┘
   ┌──────────────────────────────────────────────────────────────┐
   │ 4. FRESHNESS     — version ≥ the anti-rollback floor.        │
   │    Detects: a genuine but obsolete, vulnerable image.        │
   │    Requires: a monotonic counter that cannot be rolled back  │
   │              (OTP, or a monotonic NVM counter).              │
   └──────────────────────────────────────────────────────────────┘
```

**[INFERENCE]** The classic architectural error is stopping at layer 2 and calling it secure. **A CRC proves the image is intact, not that it is yours.** Anyone who can write to your flash can compute a correct CRC. If your threat model includes an attacker with physical access or with control of the update channel, you need layers 3 and 4, and layer 3 needs a hardware root of trust or it is theatre.

## 15.4 CRC versus hash versus signature

| Tool | Detects | Does not detect | Cost |
|---|---|---|---|
| **CRC-32** | Random corruption, burst errors | Deliberate modification (trivially forgeable) | Microseconds; a few hundred bytes of code |
| **Cryptographic hash (e.g. SHA-256)** | Any modification, if the hash itself is trustworthy | Modification where the attacker also replaces the hash | Milliseconds; a few KB of code |
| **Digital signature** | Any modification by anyone without the private key | Compromise of the private key; a flawed key storage | Tens of ms; a few KB plus key storage |

**[INFERENCE]** Pragmatic layering that fits most products:

- **CRC-32** on every boot: fast enough to run unconditionally at every power-on, catching bit rot and torn writes.
- **Signature verification** when an image is *first accepted* (after download, before marking it bootable), and thereafter only if the threat model requires per-boot verification.

Per-boot signature verification is the stronger posture and is required in some regulated domains. Per-boot CRC plus at-acceptance signature is a reasonable engineering compromise where boot time matters. **State which one you chose and why, in your design document** — this is exactly the kind of decision that an auditor will ask about.

## 15.5 Chapter exercises

1. Why is a `.bin` file insufficient for a production update? List five missing properties.
2. Construct an attack that a CRC-only bootloader permits and a signature-checking one does not.
3. Explain why `header_crc` must be validated before `image_size` is used.
4. Design a boot control block for a two-slot system, including its own redundancy.

---

# Chapter 16 — RAM-Resident Code, Interrupts, and the RTOS

## 16.1 The problem restated

From Chapter 6: on Architectures A and C, code cannot be fetched from the array while it is being modified. Therefore, if the operation must proceed while the CPU stays alive, **everything the CPU might execute during the operation must live somewhere else.**

"Everything" is a longer list than people expect.

## 16.2 The complete closure

**[PRINCIPLE]** Enumerate the closure of what may execute during a flash operation:

```
   ┌────────────────────────────────────────────────────────────┐
   │  MUST BE IN RAM (or another bank)                          │
   ├────────────────────────────────────────────────────────────┤
   │  □ The flash operation routine itself                      │
   │  □ Every function it calls — including compiler-generated  │
   │    helpers (division, memcpy, memset, floating point)      │
   │  □ Any const lookup tables it reads                        │
   │  □ The vector table, IF interrupts remain enabled          │
   │  □ Every ISR that can fire during the operation            │
   │  □ Everything those ISRs call                              │
   │  □ The RTOS context switcher and tick handler, if enabled  │
   │  □ The watchdog service routine, if it must run            │
   └────────────────────────────────────────────────────────────┘
```

**[INFERENCE]** Items 2 and 3 are where this goes wrong in practice. You mark your function `__ramfunc`, it calls `memcpy()`, the linker places `memcpy` in flash, and the CPU dies mid-erase with no diagnostic whatsoever. The failure is silent and reproduces only under timing you cannot control.

**Mitigations, in order of reliability:**

1. **Disable interrupts entirely** during the operation. Eliminates most of the list. Requires that the maximum operation time is acceptable as interrupt latency — compute this, do not assume it.
2. **Write the RAM-resident routine in a self-contained style**: no library calls, no `const` tables, simple loops, `volatile` pointer accesses only.
3. **Verify placement in the map file.** Grep the linker map for every symbol in the closure and assert it is in the RAM region. **Automate this as a build-time check** — a script that fails the build if a `.ramfunc` symbol references a flash-resident symbol.
4. **Verify at runtime** with a debugger: set a breakpoint inside the routine, read the PC, confirm it is in RAM.

## 16.3 The interrupt question

**[PRINCIPLE]** Three viable policies. Choose deliberately.

| Policy | How | Cost | When appropriate |
|---|---|---|---|
| **A. Disable all interrupts** | Global disable around the operation | Interrupt latency = worst-case operation time | Bootloaders; systems with no hard real-time constraint during update |
| **B. RAM-resident vector table + RAM-resident ISRs** | Copy vectors and critical handlers to RAM; relocate | RAM cost; discipline in linking | Systems that must keep servicing something during long erases |
| **C. Other-bank execution** | Vectors and ISRs linked into the bank not being modified | Requires Architecture B/D hardware; linker complexity | Devices with real read-while-write |

**[INFERENCE]** Policy A is correct far more often than engineers expect. Ask the real question: *does anything genuinely have to happen during the erase?* If your erase is 30 ms and your tightest deadline is 100 ms, disable interrupts and move on. Policies B and C should be a considered response to a measured requirement, not a default. They roughly triple the complexity of the update path — which is the path where bugs are least recoverable.

**[PRINCIPLE]** Whichever policy you choose: **DMA is part of the question too.** A DMA channel reading a lookup table from flash during an erase will fault, stall, or return corrupt data. Enumerate active DMA channels and suspend those that touch the affected region.

## 16.4 The watchdog

**[PRINCIPLE]** The interaction is straightforward to state and easy to get wrong:

```
   Requirement:  worst_case_operation_time  <  remaining_watchdog_window
```

with "worst case" meaning **datasheet maximum, at worst-case temperature, at end-of-life wear** — not what you measured.

Strategies:

| Strategy | Description | Trade-off |
|---|---|---|
| **Split the work** | Erase one sector, service the watchdog, erase the next | Best. Bounds the blocking interval. Requires the operation to be divisible. |
| **Service immediately before** | Kick the dog, then start the operation | Only sufficient if `max_op_time < full_window`. Fails on a windowed watchdog that also has a *minimum* interval. |
| **Widen the window during update** | Reconfigure the watchdog for the update session | **[VARIES]** Many watchdogs cannot be reconfigured after being enabled — often deliberately, for safety. Check. |
| **RAM-resident service routine** | Service from within the RAM-resident code | Adds to the closure of §16.2 |

**[INFERENCE]** Watch for the **windowed** watchdog specifically: it requires servicing not too late *and* not too early. A design that services the dog immediately before and immediately after a fast operation can violate the minimum interval and trigger a reset — a genuinely baffling bug the first time you meet it.

## 16.5 Flash in an RTOS

**[PRINCIPLE]** Application tasks must not write flash directly. Reasons:

1. **Mutual exclusion.** Two tasks issuing flash commands concurrently corrupts the controller state machine. This is not a data race you can reason about; the hardware has one command register.
2. **Latency.** An erase blocks for milliseconds. If it happens inside a high-priority task, everything below it misses its deadlines. If it happens with interrupts disabled, *everything* misses.
3. **Policy.** Compaction timing, wear levelling, and power-fail behaviour are global properties. They cannot be managed correctly by a caller who only knows about its own data.
4. **Ordering.** Power-fail safety depends on operation ordering. Concurrent callers destroy the ordering guarantees.

### 16.5.1 The standard architecture

```
   ┌──────────┐  ┌──────────┐  ┌──────────┐
   │  Task A  │  │  Task B  │  │  Task C  │
   └────┬─────┘  └────┬─────┘  └────┬─────┘
        │ nvm_set()   │             │        ← non-blocking; enqueue
        └─────────────┼─────────────┘
                      ▼
            ┌───────────────────┐
            │   REQUEST QUEUE   │  (bounded; back-pressure on full)
            └─────────┬─────────┘
                      ▼
            ┌───────────────────────────────┐
            │        NVM TASK               │  single owner of the driver
            │  - serialises all operations  │
            │  - coalesces duplicate keys   │  ← batching = endurance
            │  - decides when to compact    │
            │  - checks supply before write │
            │  - owns the critical section  │
            └─────────┬─────────────────────┘
                      ▼
            ┌───────────────────┐
            │   FLASH DRIVER    │
            └─────────┬─────────┘
                      ▼
            ┌───────────────────┐
            │ FLASH CONTROLLER  │
            └───────────────────┘
```

**[INFERENCE]** The coalescing step is where the endurance win lives, and it is nearly free. If task A updates key 7 forty times in a second and the NVM task drains the queue once per second, only the last value is written. From the application's point of view the API is unchanged; from the flash's point of view the write rate dropped 40×. Chapter 9's Design 5 falls out of this architecture automatically.

**[INFERENCE]** Two API details worth getting right:

- `nvm_set()` should be **non-blocking** and should return a status that can express "queue full." Blocking a real-time task on flash is the thing this architecture exists to prevent.
- Provide an explicit `nvm_flush()` that blocks until pending writes are durable, for use at shutdown and before deliberate resets. Without it, "I saved the setting and then rebooted and it was gone" is a legitimate bug report.

## 16.6 Chapter exercises

1. Enumerate the full closure of code that must be RAM-resident for your current project's flash routine.
2. Write the build-time check that fails if a `.ramfunc` symbol calls a flash-resident symbol. What tool output do you parse?
3. A windowed watchdog resets the device during an update, but only sometimes. Explain the mechanism.
4. Design the queue and coalescing policy for the NVM task, including what happens when the queue is full.

---

---

# Chapter 17 — Firmware Update Topologies

## 17.1 The design space

**[PRINCIPLE]** Every firmware update scheme is a point in a space defined by four questions:

1. **Where does the new image live while it is being received?** (in place / staging slot / external memory / RAM)
2. **What makes the switch atomic?** (bank swap / pointer in NVM / physical copy + commit marker)
3. **What happens if the new image is bad?** (nothing — bricked / fall back / retry other slot)
4. **Who pays the flash cost?** (1×, 2×, or 2×+staging of the application size)

## 17.2 The topologies

### 17.2.1 Single-slot, in-place

```
   ┌────────────┬──────────────────────────────────┐
   │ Bootloader │        Application               │
   └────────────┴──────────────────────────────────┘
                 ↑
                 erased and reprogrammed in place
```

**Flow:** enter bootloader → erase application → receive and program → verify → boot.

| | |
|---|---|
| **Flash cost** | 1× application |
| **Atomic?** | No. There is a long window where no valid application exists. |
| **Power-fail** | Device stays in the bootloader. **Recoverable only if the bootloader can receive an update on its own.** |
| **Requires** | An update channel available to the bootloader (wired UART/USB/CAN, or a radio the bootloader can drive) |
| **Suits** | Cost-sensitive devices with physical access for recovery; wired industrial devices |
| **Fails** | Any remote device where a stranded unit means a truck roll |

**[INFERENCE]** This topology is not "bad." It is the correct answer when flash is the dominant cost and a technician can always reach the device. It is the wrong answer for anything deployed remotely, because the recovery path requires the same channel that just failed.

### 17.2.2 Dual-slot (A/B) with copy

```
   ┌────────────┬───────────────────┬───────────────────┬──────┐
   │ Bootloader │      Slot A       │      Slot B       │ Meta │
   └────────────┴───────────────────┴───────────────────┴──────┘
                    running              receiving
```

**Flow:** running from A → receive into B → verify B → write "boot B" to metadata (**the atomic commit**) → reset → boot B. A remains as fallback.

| | |
|---|---|
| **Flash cost** | 2× application + metadata |
| **Atomic?** | **Yes** — the metadata write is the commit point |
| **Power-fail** | At any point before the commit, A still boots. After, B boots. No window without a valid image. |
| **Rollback** | Trivial: rewrite the metadata pointer |
| **Suits** | Almost every connected product |

**[INFERENCE]** This is the default recommendation for anything with adequate flash. The cost is one extra copy of the application; the benefit is that no single power loss, corrupt download, or bad image can strand the device. If you can afford the flash, take it and stop deliberating.

**Critical detail:** slot B's image must be *position-correct*. Either build two images (one linked for A, one for B), or design the application to be position-independent, or use hardware address swap so both slots appear at the same address when active. Building two images doubles your release artefacts and your test matrix — a real cost that surprises teams.

### 17.2.3 Hardware bank/address swap

```
   Physical bank 0 ──┐        ┌── appears at boot address
                     ├─ SWAP ─┤
   Physical bank 1 ──┘        └── appears at the alternate address
```

**[VARIES]** Where hardware supports remapping which bank appears at the boot address, both images can be **built for the same address**. This removes the dual-build problem entirely.

**Flow:** running from the bank currently mapped to boot → program the other bank → verify → set the swap flag (**atomic, survives reset**) → reset → the other bank now boots.

| | |
|---|---|
| **Flash cost** | 2× application (usually the whole device split in two) |
| **Atomic?** | Yes, in hardware |
| **Advantage** | **One build artefact.** No position-independence requirement. |
| **Constraint** | Requires a device with this feature; often halves usable flash; may be mutually exclusive with other features |

### 17.2.4 External staging

```
   ┌────────────┬───────────────────┐        ┌──────────────────┐
   │ Bootloader │    Application    │        │  EXTERNAL FLASH  │
   └────────────┴───────────────────┘        │  candidate image │
        internal flash                       │  + rollback copy │
                                             └──────────────────┘
```

**Flow:** download into external flash → verify → bootloader copies external → internal → verify → boot.

| | |
|---|---|
| **Internal flash cost** | 1× application |
| **Atomic?** | The *download* is safe; the *copy* is a single-slot in-place update with the same window |
| **Mitigation** | Keep a **golden image** in external flash so the copy can always be retried, and retry automatically on the next boot |
| **Suits** | Devices where internal flash is tight and an external part is already present |
| **Extra cost** | An external memory, its pins, its driver, and its own power-fail behaviour |

### 17.2.5 Delta / differential update

Transmit only the difference between the installed version and the target.

| | |
|---|---|
| **Benefit** | Drastically smaller downloads — matters enormously on cellular, LoRa, or metered links |
| **Costs** | A patch engine on the device; scratch space to apply the patch; **the server must know exactly which version each device runs**; a patch chain that must be exercised for every source→target pair |
| **Risk** | The test matrix grows with the square of the number of deployed versions |

**[INFERENCE]** Delta update is an optimisation, and it should be treated as one: implement full-image A/B first, get it correct and proven in the field, then add delta as a bandwidth optimisation on top — with full-image update retained as the always-available fallback. Teams that start with delta usually end up rebuilding it.

## 17.3 The update state machine

**[PRINCIPLE]** Regardless of topology, this state machine is what you must implement, and every state must be persistent across reset:

```
        ┌──────┐
        │ IDLE │◄────────────────────────────────┐
        └──┬───┘                                 │
           │ update available                    │
           ▼                                     │
     ┌───────────┐  transfer error / timeout     │
     │DOWNLOADING├───────────────────────────────┤
     └─────┬─────┘                               │
           │ transfer complete                   │
           ▼                                     │
     ┌───────────┐  hash / signature / version   │
     │ VERIFYING ├───────── fail ────────────────┤
     └─────┬─────┘                               │
           │ pass                                │
           ▼                                     │
     ┌───────────┐                               │
     │  PENDING  │  ← metadata says "boot new"   │
     └─────┬─────┘     THIS IS THE COMMIT        │
           │ reset                               │
           ▼                                     │
     ┌───────────┐  boot-attempt counter         │
     │  TESTING  ├───── exceeded ────────┐       │
     └─────┬─────┘                       │       │
           │ application calls           ▼       │
           │ confirm_image_ok()   ┌────────────┐ │
           ▼                      │ ROLLING    │ │
     ┌───────────┐                │ BACK       ├─┘
     │ CONFIRMED │                └────────────┘
     └───────────┘
```

**[INFERENCE]** The `TESTING` state is what most home-grown updaters omit, and it is the one that saves you. Consider: a new image passes signature verification perfectly and then hard-faults on boot because of a bug in the new code. Integrity checking cannot detect this — the image is exactly what you built. Only a boot-attempt counter plus an application-level "I am healthy" confirmation can.

**What "healthy" should mean [INFERENCE]:** not merely "reached `main()`." A useful confirmation is issued after the application has completed its own self-test: peripherals initialised, NVM mounted, the communication link established, and — for connected devices — a successful contact with the server. The last one matters because the classic un-recoverable OTA failure is an image that boots fine but has broken networking, so it can never receive a fix. Confirming only after the link is up makes that failure self-healing.

## 17.4 Anti-rollback

**[PRINCIPLE]** Rollback protection means: *a genuine, correctly-signed, but obsolete image must be refused.* This matters because an old image may contain a patched vulnerability, and an attacker who can install any signed image can install the vulnerable one.

Requirements:

1. A **monotonic version** in the image header.
2. A **minimum acceptable version** stored where it cannot be decreased — OTP, a fuse counter, or a monotonic NVM counter protected against rollback.
3. Bootloader logic that refuses `image_version < stored_min_version`.
4. A policy for **when** the floor advances.

**[INFERENCE]** Point 4 is a genuine product decision with no universally correct answer:

- Advancing the floor **immediately on install** gives the strongest security and eliminates your ability to roll back a bad release.
- Advancing it **only when explicitly instructed** by a signed command preserves rollback for operational recovery but leaves a window.
- Advancing it **after the new image has been confirmed healthy for N days** is a reasonable compromise many products use.

Whichever you choose, note that OTP-based counters are consumed permanently — you have a finite number of floor advances for the life of the device. Budget them. A design that burns a fuse on every release will exhaust its fuses in a few years.

## 17.5 Sizing the flash

**Worked example [INFERENCE — invented numbers]**

Given: application 384 KB, bootloader 32 KB, metadata 8 KB, sector size 8 KB.

| Topology | Calculation | Total | Round to sectors |
|---|---|---|---|
| Single-slot | 32 + 384 + 8 | 424 KB | 424 KB |
| A/B internal | 32 + 384 + 384 + 8 (×2 redundant meta = 16) | 816 KB | **1 MB part** |
| A/B with bank swap | 32 + 384 per bank, both banks | 832 KB | **1 MB part**, dual-bank |
| External staging | internal 424 KB; external ≥ 384 KB (+ golden copy = 768 KB) | 424 KB + 1 MB ext | — |
| A/B + golden recovery | 32 + 384 + 384 + 384 + 16 | 1200 KB | **2 MB part** |

**[INFERENCE]** Three observations worth carrying into a design review:

1. **A/B roughly doubles your flash requirement.** Decide this at part-selection time, not after the firmware has grown. Retrofitting A/B onto a device that is 90% full is a project, not a change.
2. **Metadata should be redundant** (§12.4) and therefore occupies at least two erase units, not one.
3. **Round up to whole sectors, then add headroom.** Firmware grows monotonically over a product's life. A part chosen with 5% headroom will be the reason a feature is cancelled in year three.

## 17.6 Chapter exercises

1. For each topology, state exactly what happens if power is lost at three different points, and what boots afterwards.
2. Why does hardware bank swap eliminate the dual-build problem, and what does it cost?
3. Design the "healthy" criterion for a battery-powered cellular sensor. Justify each condition.
4. Your OTP rollback counter has 32 bits available in a one-way fuse array. Design an advancement policy for a 15-year product life.

---

# PART V — INTEGRITY AND SECURITY

---

# Chapter 18 — Error Correcting Codes

## 18.1 Why flash needs ECC

**[PRINCIPLE]** From Chapter 2: cell states drift. Retention loss, disturb, wear, and marginal programming all move V<sub>t</sub> toward the read reference. ECC converts a *slow analogue degradation* into a *managed digital failure*: errors are corrected while they are few, and reported before they become silent corruption.

**[VARIES]** Whether your device has ECC on flash — and on SRAM, and at what strength — is device-specific. Small, low-cost parts often have none. Safety-oriented and larger parts usually do.

## 18.2 From parity to SEC-DED

### 18.2.1 Parity — 1 check bit

Append one bit making the total number of 1s even (or odd).

- **Detects:** any odd number of bit errors (so: any single error)
- **Corrects:** nothing
- **Misses:** any even number of errors

### 18.2.2 Hamming codes — single error correction (SEC)

Add `r` check bits to `k` data bits such that `2^r ≥ k + r + 1`. Each check bit covers a different subset of positions; the pattern of failing checks — the **syndrome** — is the binary index of the failing bit.

```
   Example: 4 data bits (d1..d4), 3 check bits (p1..p3)

   Position:  1   2   3   4   5   6   7
   Content:   p1  p2  d1  p3  d2  d3  d4

   p1 covers positions 1,3,5,7
   p2 covers positions 2,3,6,7
   p3 covers positions 4,5,6,7

   On read, recompute each parity. The three results, read as a
   binary number, give the position of the erroneous bit.
   Syndrome 000 → no error.  Syndrome 101 → bit 5 is wrong; flip it.
```

- **Corrects:** any single-bit error
- **Detects:** single errors
- **Fails dangerously:** a double error produces a *valid-looking* non-zero syndrome, so the decoder "corrects" the wrong bit — turning 2 errors into 3. This is why plain SEC is not used alone.

### 18.2.3 SEC-DED — the embedded standard

Add one more overall parity bit. Now:

| Syndrome | Overall parity | Conclusion |
|---|---|---|
| Zero | Correct | No error |
| Non-zero | Incorrect | **Single error** — correctable; flip the indicated bit |
| Non-zero | Correct | **Double error** — detected, **not correctable** |
| Zero | Incorrect | Error in the overall parity bit itself |

**[PRINCIPLE]** SEC-DED — **S**ingle **E**rror **C**orrection, **D**ouble **E**rror **D**etection — is the overwhelmingly common choice for embedded flash and SRAM. It corrects the common case and refuses to guess in the dangerous case.

### 18.2.4 Overhead

```
   data bits    check bits (SEC-DED)     overhead
   ─────────────────────────────────────────────────
      8              5                     62%
     16              6                     38%
     32              7                     22%
     64              8                     12.5%
    128              9                      7%
```

**[PRINCIPLE]** This table explains a design decision you will meet constantly: **ECC-protected embedded flash usually has a wide flash word** — commonly 64 data bits plus 8 check bits, or similar. Narrower words would waste too much area on check bits. And a wide flash word is precisely what forces the coarse program granularity and the write-once-per-word rule of Chapter 7. *The ECC overhead table is the root cause of your programming API's restrictions.*

## 18.3 The read path with ECC

```
   Flash array (data bits + check bits, read together)
        │
        ▼
   ┌──────────────────────────────────────────┐
   │            ECC DECODER                   │
   │  compute syndrome from data + check bits │
   └───────────────┬──────────────────────────┘
                   ▼
        ┌──────────────────────┐
        │  syndrome == 0 ?     │
        └───┬──────────────┬───┘
          yes             no
           │               │
           ▼               ▼
    ┌────────────┐   ┌──────────────────────────┐
    │ pass data  │   │ single-bit correctable?  │
    │ to CPU     │   └───┬──────────────────┬───┘
    └────────────┘     yes                 no
                        │                   │
                        ▼                   ▼
              ┌──────────────────┐  ┌──────────────────────┐
              │ CORRECT the bit  │  │ UNCORRECTABLE        │
              │ pass to CPU      │  │ - flag status        │
              │ set SEC flag     │  │ - raise interrupt/   │
              │ optionally IRQ   │  │   fault / NMI        │
              └──────────────────┘  │ - data is NOT valid  │
                                    └──────────────────────┘
```

**[PRINCIPLE]** Two behaviours you must know for your part:

1. **On a corrected error:** does the CPU get the corrected data transparently? (Almost always yes.) Is a flag set? Can it raise an interrupt? **Corrected errors are the early-warning signal of a failing device.** A product that never checks them discards its most valuable reliability telemetry.
2. **On an uncorrectable error:** what does the CPU receive? Options in the wild include a bus fault, an NMI, a dedicated interrupt, corrupt data plus a sticky flag, or a lockup. **This determines whether your fault handler can do anything useful.**

**[INFERENCE]** Design rule: enable ECC error reporting, log corrected-error counts to NVM, and expose them in your diagnostics. A device reporting rising correctable-error counts in a particular flash region is telling you it is approaching end of life *while it is still working*. That is a maintenance opportunity, and almost nobody collects it.

## 18.4 Handling an uncorrectable error

**[INFERENCE]** A fault handler that just resets is a wasted opportunity, and on a genuinely degraded flash location it produces a reset loop. A useful handler:

```c
/* GENERIC EXAMPLE */
void flash_ecc_uncorrectable_handler(uint32_t fault_addr)
{
    /* 1. Record it somewhere that survives reset — but NOT in a
          region that may itself be the faulted one.               */
    persist_fault_record(FAULT_ECC_UNCORRECTABLE, fault_addr);

    /* 2. Classify by region and respond appropriately. */
    if (in_region(fault_addr, REGION_NVM_DATA)) {
        /* Data: discard the record, fall back to a redundant copy
           or a default, erase and rewrite the affected unit.      */
        nvm_quarantine_unit(fault_addr);
        return;                       /* recoverable — keep running */
    }

    if (in_region(fault_addr, REGION_APPLICATION)) {
        /* Code: the running image is corrupt. Mark the slot bad
           and reboot so the bootloader selects the other slot.    */
        boot_control_mark_current_slot_bad();
        system_reset();
    }

    if (in_region(fault_addr, REGION_BOOTLOADER)) {
        /* Worst case. Enter the most degraded safe state the
           product defines and signal for service.                */
        enter_safe_state();
    }
}
```

**[INFERENCE]** Note the deliberate difference in response by region. An ECC fault in a log record is a nuisance; the same fault in the running application is a reason to fail over to slot B; in the bootloader it is a service event. Treating all three identically — as most default fault handlers do — either over-reacts to a trivial problem or under-reacts to a serious one.

## 18.5 ECC and blank flash

**[PRINCIPLE]** Restating §2.7 because it causes real bugs: on some ECC-protected devices, reading erased flash produces an **unspecified** value, or even raises an ECC error, because the erased check-bit pattern is not the valid code word for the erased data pattern.

Consequences:

- `if (*p == 0xFFFFFFFF)` is not a valid emptiness test.
- Use the **blank-check command** the controller provides.
- Reading a large erased region "to see what's there" may generate a storm of ECC faults.
- A bootloader that scans unwritten slots must handle this.

**[VARIES]** Some devices provide a second address alias that returns raw, undecoded data specifically so that diagnostics and blank checks can bypass the decoder. Find out whether yours does.

## 18.6 Chapter exercises

1. Work through the Hamming example: given data `1011`, compute the check bits; then flip bit 5 and show the syndrome identifies it.
2. Explain why SEC alone is dangerous and what the extra parity bit in SEC-DED buys you.
3. Using the overhead table, explain why ECC-protected flash tends to have a wide program unit — and connect that to the write-once-per-word rule.
4. Write the region-classification logic for an ECC fault handler for a product with a bootloader, two application slots, and an NVM pool.

---

# Chapter 19 — Security and Configuration Regions

## 19.1 Five different things people call "security"

**[PRINCIPLE]** These are distinct mechanisms with distinct threat models. Conflating them is the most common security design error in embedded systems.

| # | Mechanism | Answers | Defeated by |
|---|---|---|---|
| 1 | **Flash protection** (write/erase protect) | "Can this region be modified by software running on this chip?" | Anything that runs before the protection is applied; physical attack; a config-region rewrite |
| 2 | **Debug protection** (debug lock, access levels) | "Can an external tool read or control this chip?" | Glitching; unlock-with-mass-erase paths; leftover test interfaces |
| 3 | **Cryptographic integrity/authenticity** | "Was this data produced by someone holding the key?" | Key compromise; a verifier that can be bypassed |
| 4 | **Secure boot** | "Is the code about to execute authentic and unmodified?" | A root of trust that is itself modifiable; a chain with an unverified link |
| 5 | **Hardware root of trust** | "What do we anchor all of the above to?" | Physical attack on the anchor; a mutable anchor |

**[PRINCIPLE]** **Setting a read-protection bit does not create a secure system.** It raises the cost of one specific attack — casual readout via the debug port. It does nothing about a malicious firmware update, an attacker with the ability to glitch, an exposed serial bootloader, or a supply-chain substitution.

## 19.2 The dependency chain

```
   HARDWARE ROOT OF TRUST
   (immutable boot ROM + keys/hashes in OTP)
              │  "this cannot be changed after manufacture"
              ▼
   SECURE BOOT STAGE 1
   (verifies stage 2's signature before executing it)
              │  "only code we signed runs"
              ▼
   SECURE BOOT STAGE 2 / BOOTLOADER
   (verifies the application; enforces anti-rollback)
              │
              ▼
   APPLICATION
   (enforces its own policies; protects its own secrets)
```

**[PRINCIPLE]** The chain is only as strong as its weakest link, and the anchor must be *immutable*. If the public key used to verify your firmware is stored in ordinary rewritable flash, an attacker who can write flash can substitute their own key and then sign anything. **The key or its hash must live in OTP, fuses, or mask ROM.**

**[INFERENCE]** This is the question to ask a vendor first when evaluating a part for secure boot: *"Where exactly is the root public key or its hash stored, who can write it, and when does it become immutable?"* If the answer is vague, the feature is decorative.

## 19.3 Lifecycle states

**[PRINCIPLE]** Secure devices implement a **lifecycle**: a one-way progression of security postures.

```
   BLANK ──► DEVELOPMENT ──► PROVISIONED ──► DEPLOYED ──► [RMA?] ──► DECOMMISSIONED
     │            │                │              │                        │
   full        debug open,      keys burned,   debug locked,          keys erased,
   access      no secure boot   secure boot    no readout             part unusable
                                enforced
```

Key properties **[VARIES]** by device:

- Transitions are typically **one-way**, enforced by fuses.
- Some devices offer an **RMA/failure-analysis** state that unlocks debug **only after erasing all secrets** — so returned units can be analysed without leaking customer keys.
- Some offer an authenticated debug unlock, where a challenge-response with a customer key re-enables debug on a specific device.

**[INFERENCE]** Plan the lifecycle before production, not after. Two failure modes are common and expensive:

1. **Locking too early** — you ship a locked device, find a field bug, and cannot attach a debugger to any returned unit. Design an authenticated-debug or erase-then-unlock path *before* you lock.
2. **Locking too late (or never)** — the "we'll enable security in a later revision" plan that never happens, because by then production tooling, test fixtures and the CI pipeline all depend on open debug access.

## 19.4 The configuration region: the most dangerous flash on the device

**[PRINCIPLE]** Every MCU has a small region holding boot and security policy. Names vary — option bytes, configuration words, boot configuration, fuse rows. Contents typically include:

- Boot source selection
- Read/write/erase protection settings
- Debug access policy
- Brown-out and watchdog defaults
- Secure boot enable and key hashes
- Application start address, on some parts

Why it is dangerous:

| Property | Consequence |
|---|---|
| It is read **before your code runs** | A bad value takes effect before you can correct it |
| It is often **CRC-checked by the boot ROM** | An inconsistent CRC can permanently lock the device by design |
| Some fields are **one-way** | No mistake is recoverable |
| It is usually a **single erase unit** | An interrupted update can destroy all policy at once |
| Recovery may require a **specific tool sequence** — or may not exist | An ordinary programmer cannot fix it |

**[INFERENCE]** Rules that will save you units and schedule:

1. **Never write configuration regions in the field.** Factory only. If a feature seems to require it, redesign the feature.
2. **Program configuration last** in the production sequence, after the application is programmed and verified, and after functional test passes.
3. **Verify configuration by reading it back and comparing** against a golden value in the production script — before the device leaves the fixture.
4. **Keep unlocked golden units** outside the locked lifecycle for engineering, and control them like the valuable assets they are.
5. **Read the "how to recover" section of the reference manual before you write the region the first time**, not after.

## 19.5 TrustZone-style isolation

**[PRINCIPLE]** Some architectures provide hardware partitioning of memory and peripherals into **secure** and **non-secure** worlds, with hardware-enforced transition points.

```
   ┌───────────────────────────────────────────────────────────┐
   │                     SECURE WORLD                          │
   │  secure flash  │  secure RAM  │  secure peripherals       │
   │  ┌──────────────────────────────────────────────────┐     │
   │  │  Non-Secure Callable (NSC) entry points          │     │
   │  │  — the ONLY legal way in from the other side —   │     │
   │  └──────────────────────────────────────────────────┘     │
   └───────────────────────┬───────────────────────────────────┘
                           │  hardware-checked transitions
   ┌───────────────────────┴───────────────────────────────────┐
   │                   NON-SECURE WORLD                        │
   │  application flash │ application RAM │ most peripherals   │
   └───────────────────────────────────────────────────────────┘
```

Concepts:

- **Attribution units** define which addresses are secure, non-secure, or non-secure-callable. There is typically an implementation-defined default attribution and a software-configurable one that can further constrain it.
- **Entry points** must be explicitly marked. A non-secure caller cannot jump into the middle of secure code — the hardware requires a special instruction at the target address.
- **Separate stacks and separate vector tables** exist for each world.
- **Peripherals and DMA** are attributed too. A DMA controller assigned to the non-secure world cannot read secure memory — but a *misconfigured* one can, which is a classic hole.

**[PRINCIPLE]** Relevance to flash: TrustZone-style isolation is how you keep keys and the secure bootloader out of reach of the application **at runtime**, not just at boot. Without it, an application-level bug (a buffer overflow, a debug command) can read the key material that secure boot depends on.

**[INFERENCE]** Two cautions:

1. **This is an architectural facility that vendors implement.** The processor architecture defines the isolation model; the silicon vendor decides which flash regions, which peripherals, and which DMA channels are attributable, and provides the configuration mechanism. The details differ between parts using the same core.
2. **Isolation is only as good as its configuration**, and the configuration is code you write, running early, with no isolation protecting it. Review it as carefully as the cryptography.

## 19.6 A worked threat model

**[INFERENCE]** Suppose a connected industrial device. Threats and mitigations:

| Threat | Mitigation | Which mechanism |
|---|---|---|
| Attacker reads firmware via debug port | Debug lock in production lifecycle | 2 |
| Attacker replaces firmware via the update channel | Signature verification in the bootloader | 3 + 4 |
| Attacker installs an old, vulnerable signed image | Anti-rollback floor in OTP | 4 |
| Application bug erases the bootloader | Non-volatile write protection on boot sectors | 1 |
| Attacker extracts keys from a running device | Secure world isolation | 5 |
| Attacker substitutes the verification key | Key hash in OTP; immutable root | 5 |
| Attacker glitches during signature check | Redundant checks, randomised delays, hardware countermeasures | beyond software alone |
| Attacker reads flash by decapsulation | Encryption at rest; accept residual risk | — |

**[INFERENCE]** Note the last two rows. **A threat model must include threats you decide not to defend against**, with that decision recorded. Security work without an explicit accepted-risk list tends to sprawl indefinitely and still miss the important cases.

## 19.7 Chapter exercises

1. Explain why a read-protection bit does not make a device secure. Give three attacks it does not address.
2. Where must the root verification key be stored, and why does storing it in ordinary flash defeat secure boot entirely?
3. Design the production sequence for a device with secure boot, stating exactly when configuration is written and when debug is locked.
4. Your device is locked and a field unit fails. Describe two ways you could have planned for analysis, and their trade-offs.

---

---

# PART VI — BEYOND THE MICROCONTROLLER

---

# Chapter 20 — External Flash, Serial Interfaces, and Storage Patterns

## 20.1 The interface family

**[PRINCIPLE]** External flash is reached over a serial interface whose width and clocking determine throughput. The progression:

```
   SPI (1-bit)      CLK, CS, MOSI, MISO                     ~1 bit/clock
        │
   Dual SPI         IO0, IO1 bidirectional                  ~2 bits/clock
        │
   Quad SPI (QSPI)  IO0..IO3                                ~4 bits/clock
        │
   Octal SPI (OSPI) IO0..IO7                                ~8 bits/clock
        │
   + DDR / DTR      data on both clock edges                ×2
        │
   + DQS            source-synchronous strobe for timing    enables higher clocks
```

### 20.1.1 Bandwidth arithmetic

```
   throughput_bytes_per_sec = (clock_Hz × data_lines × edges_per_clock) / 8
```

**Worked example [INFERENCE — invented but representative]:**

| Configuration | Clock | Lines | Edges | Theoretical |
|---|---|---|---|---|
| SPI single | 50 MHz | 1 | 1 | 6.25 MB/s |
| Quad SDR | 100 MHz | 4 | 1 | 50 MB/s |
| Quad DDR | 100 MHz | 4 | 2 | 100 MB/s |
| Octal DDR | 200 MHz | 8 | 2 | 400 MB/s |

**[PRINCIPLE]** Real throughput is always lower, and for XIP it can be *dramatically* lower. The reasons:

| Overhead | Effect |
|---|---|
| **Command + address + dummy cycles** | A fixed cost per transaction. For a 32-byte cache line read, this can exceed the data transfer time itself. |
| **Random access pattern** | XIP with poor locality pays the transaction overhead on every miss |
| **Chip-select turnaround** | Time between transactions |
| **Bus contention** | Other masters (DMA, a second core) sharing the controller |
| **Cache behaviour** | A hit costs nothing; a miss costs everything. Hit rate dominates effective performance. |

**[INFERENCE]** This is why external-XIP systems live or die on their cache. A 400 MB/s octal interface with a 60% cache hit rate on a branchy workload can deliver worse *effective* instruction throughput than a modest internal flash with a prefetch buffer. When evaluating an external-XIP architecture, benchmark your actual code, not the datasheet's sequential read rate.

## 20.2 The SPI NOR command model

**[PRINCIPLE]** Serial NOR devices from different manufacturers share a broadly common command model. **[VARIES]** — opcodes, timing, and extended features differ, and you must consult the specific device datasheet. But the *shape* is consistent, and it is worth knowing cold.

### 20.2.1 Anatomy of a transaction

```
   CS ‾‾‾\_______________________________________________/‾‾‾

   Phase:  [ OPCODE ][ ADDRESS ][ MODE ][ DUMMY ][ DATA ... ]
             8 bits    24/32b    opt.    N clks    n bytes

   - OPCODE  : what to do
   - ADDRESS : where (3-byte for ≤128 Mbit; 4-byte for larger)
   - MODE    : optional bits allowing the next transaction to skip
               the opcode ("continuous read")
   - DUMMY   : idle clocks giving the device time to fetch the first
               data from the array. THE COUNT IS CONFIGURABLE AND
               MUST MATCH THE CONTROLLER'S SETTING.
   - DATA    : payload, on 1/2/4/8 lines depending on the command
```

**[INFERENCE]** The dummy-cycle count is the single most common cause of "my QSPI flash returns garbage." The device and the controller must agree. They are configured independently — the device via a status/configuration register, the controller via its own register or via a boot configuration structure. A mismatch of one cycle shifts every bit and produces plausible-looking rubbish. **When debugging a new external flash bring-up, check dummy cycles before anything else.**

### 20.2.2 The command families

| Family | Purpose | Notes |
|---|---|---|
| **Identification** | Read JEDEC manufacturer/device ID; read SFDP | Always the first thing your driver should do — it verifies the device is present and is what you expect |
| **Read** | Slow read (no dummy cycles), fast read, dual/quad/octal read | The slow single-line read is the universally-supported fallback used at boot before configuration |
| **Write enable** | Arm the device for a modifying operation | See §20.2.4 |
| **Program** | Page program | Bounded by a **page** — a program that crosses a page boundary *wraps* rather than continuing |
| **Erase** | Sector erase, block erase (often two sizes), chip erase | Multiple granularities with very different durations |
| **Status/config** | Read/write status registers; enable quad mode; set dummy cycles; set address width | The device's own configuration, stored in its own non-volatile bits |
| **Protection** | Block protection bits, individual sector lock, permanent lock | Independent of your MCU's protection scheme |
| **Suspend/resume** | Pause an erase or program to allow a read | **[VARIES]** — support and restrictions differ |
| **Reset** | Software reset sequence | Important for recovering a device left mid-command by an unexpected MCU reset |

### 20.2.3 SFDP — the self-describing feature

**[PRINCIPLE]** Serial Flash Discoverable Parameters is a standardised table stored *inside* the flash device describing its own geometry and capabilities: capacity, erase sizes and their opcodes, supported read modes and their dummy-cycle counts, address width, and more.

**[INFERENCE]** SFDP is what lets a bootloader or a Linux driver support thousands of parts without a hard-coded table. For your own driver, reading SFDP at init and validating it against your expectations is excellent defensive engineering: it catches a wrong part fitted at assembly, a counterfeit, or a second-source substitution with different timing — at boot, with a clear error, instead of as intermittent corruption in the field.

### 20.2.4 Why "Write Enable" exists

**[PRINCIPLE]** Every modifying command must be immediately preceded by a separate Write Enable command, which sets a latch (commonly called WEL) in the status register. The latch clears automatically when the operation completes.

The reasons this design is universal:

1. **Noise immunity.** A single spurious opcode on the bus cannot erase a sector — it would need to be preceded by a valid Write Enable.
2. **Explicit intent.** Two distinct transactions must be issued, making accidental modification much less likely.
3. **Auto-clear.** The device does not stay armed. There is no "we forgot to re-lock" failure mode.
4. **A readable arming state.** Software can verify the latch was actually set before issuing the destructive command — catching a wiring or timing fault before damage.

**[INFERENCE]** Your driver should read back the status register after Write Enable and confirm the latch is set. If it is not, the bus is not working, and issuing the erase anyway achieves nothing except uncertainty about whether it happened.

### 20.2.5 The canonical sequence

```
   ┌───────────────────────────────────────────────────────┐
   │ 1.  READ ID / SFDP        — device present & correct? │
   └────────────────────────┬──────────────────────────────┘
                            ▼
   ┌───────────────────────────────────────────────────────┐
   │ 2.  READ STATUS          — wait until not busy        │
   └────────────────────────┬──────────────────────────────┘
                            ▼
   ┌───────────────────────────────────────────────────────┐
   │ 3.  WRITE ENABLE                                      │
   │ 3a. READ STATUS          — confirm the latch is set   │
   └────────────────────────┬──────────────────────────────┘
                            ▼
   ┌───────────────────────────────────────────────────────┐
   │ 4.  SECTOR ERASE (address)                            │
   └────────────────────────┬──────────────────────────────┘
                            ▼
   ┌───────────────────────────────────────────────────────┐
   │ 5.  POLL STATUS until busy clears  ── WITH A TIMEOUT   │
   │     (erase can take hundreds of ms; chip erase, tens   │
   │      of seconds)                                       │
   └────────────────────────┬──────────────────────────────┘
                            ▼
   ┌───────────────────────────────────────────────────────┐
   │ 6.  WRITE ENABLE  (again — the latch auto-cleared)    │
   └────────────────────────┬──────────────────────────────┘
                            ▼
   ┌───────────────────────────────────────────────────────┐
   │ 7.  PAGE PROGRAM (address, data ≤ one page,           │
   │                   not crossing a page boundary)       │
   └────────────────────────┬──────────────────────────────┘
                            ▼
   ┌───────────────────────────────────────────────────────┐
   │ 8.  POLL STATUS until busy clears                     │
   └────────────────────────┬──────────────────────────────┘
                            ▼
   ┌───────────────────────────────────────────────────────┐
   │ 9.  READ BACK AND VERIFY                              │
   └───────────────────────────────────────────────────────┘
```

### 20.2.6 A portable driver skeleton

```c
/* GENERIC EXAMPLE — abstract hardware layer; opcodes are placeholders
   that MUST be taken from the specific device datasheet.            */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Hardware abstraction the port layer must supply. */
typedef struct {
    void (*select)(bool assert);
    void (*xfer)(const uint8_t *tx, uint8_t *rx, size_t len);
    uint32_t (*now_us)(void);
} spi_hal_t;

typedef struct {
    const spi_hal_t *hal;
    uint32_t page_size;         /* from SFDP or datasheet */
    uint32_t sector_size;
    uint32_t capacity;
    uint32_t erase_timeout_us;  /* datasheet MAXIMUM, not typical */
    uint8_t  op_read, op_pp, op_se, op_wren, op_rdsr, op_rdid;
    uint8_t  status_busy_mask, status_wel_mask;
} spinor_t;

static flash_status_t spinor_wait_ready(const spinor_t *d, uint32_t timeout_us)
{
    uint32_t start = d->hal->now_us();
    for (;;) {
        uint8_t tx[2] = { d->op_rdsr, 0x00 };
        uint8_t rx[2] = { 0 };
        d->hal->select(true);
        d->hal->xfer(tx, rx, 2);
        d->hal->select(false);

        if ((rx[1] & d->status_busy_mask) == 0u) return FLASH_OK;
        if ((d->hal->now_us() - start) > timeout_us) return FLASH_ERR_TIMEOUT;
    }
}

static flash_status_t spinor_write_enable(const spinor_t *d)
{
    uint8_t op = d->op_wren;
    d->hal->select(true);
    d->hal->xfer(&op, NULL, 1);
    d->hal->select(false);

    /* Confirm the latch actually set — catches bus faults early. */
    uint8_t tx[2] = { d->op_rdsr, 0x00 };
    uint8_t rx[2] = { 0 };
    d->hal->select(true);
    d->hal->xfer(tx, rx, 2);
    d->hal->select(false);

    return (rx[1] & d->status_wel_mask) ? FLASH_OK : FLASH_ERR_HARDWARE;
}

flash_status_t spinor_erase_sector(const spinor_t *d, uint32_t addr)
{
    if (addr % d->sector_size) return FLASH_ERR_PARAM;
    if (addr >= d->capacity)   return FLASH_ERR_PARAM;

    flash_status_t s = spinor_wait_ready(d, d->erase_timeout_us);
    if (s != FLASH_OK) return s;

    s = spinor_write_enable(d);
    if (s != FLASH_OK) return s;

    uint8_t cmd[4] = { d->op_se,
                       (uint8_t)(addr >> 16),
                       (uint8_t)(addr >> 8),
                       (uint8_t)(addr) };
    d->hal->select(true);
    d->hal->xfer(cmd, NULL, sizeof cmd);
    d->hal->select(false);

    return spinor_wait_ready(d, d->erase_timeout_us);
}

flash_status_t spinor_program(const spinor_t *d, uint32_t addr,
                              const uint8_t *data, size_t len)
{
    while (len > 0u) {
        /* A page program must not cross a page boundary — the device
           wraps to the start of the page instead of continuing.     */
        uint32_t page_off   = addr % d->page_size;
        uint32_t chunk      = d->page_size - page_off;
        if (chunk > len) chunk = (uint32_t)len;

        flash_status_t s = spinor_wait_ready(d, d->erase_timeout_us);
        if (s != FLASH_OK) return s;
        s = spinor_write_enable(d);
        if (s != FLASH_OK) return s;

        uint8_t hdr[4] = { d->op_pp,
                           (uint8_t)(addr >> 16),
                           (uint8_t)(addr >> 8),
                           (uint8_t)(addr) };
        d->hal->select(true);
        d->hal->xfer(hdr, NULL, sizeof hdr);
        d->hal->xfer(data, NULL, chunk);
        d->hal->select(false);

        s = spinor_wait_ready(d, d->erase_timeout_us);
        if (s != FLASH_OK) return s;

        addr += chunk;
        data += chunk;
        len  -= chunk;
    }
    return FLASH_OK;
}
```

**[INFERENCE]** The page-boundary loop in `spinor_program()` is not optional politeness. Devices genuinely wrap within the page rather than continuing to the next one, so a naive contiguous write silently overwrites the beginning of the page with the tail of your data. It is a classic, hard-to-spot corruption bug: the first page looks fine, so testing with small writes passes.

## 20.3 Memory-mapped mode versus command mode

**[PRINCIPLE]** A serial-flash controller usually operates in one of two modes:

| Mode | Description | Used for |
|---|---|---|
| **Command / indirect** | Software composes transactions explicitly | Erase, program, status, identification |
| **Memory-mapped / XIP** | The controller translates CPU bus reads into read transactions automatically; the flash appears as ordinary memory | Instruction fetch, constant reads |

**[PRINCIPLE]** The transition between them is a hazard:

- You generally cannot issue an erase/program while the controller is servicing memory-mapped reads from the same device
- Therefore: leave memory-mapped mode, do the operation, invalidate caches, re-enter
- If the CPU is *executing* from that mapping, the code doing this must not be

**[INFERENCE]** The standard structure for updating external XIP flash: the update routine is `__ramfunc`, interrupts are disabled or their handlers are RAM-resident, memory-mapped mode is disabled, the operation is performed in command mode, caches are invalidated, memory-mapped mode is restored, and only then does control return to flash-resident code. Every step is a place to get it wrong, which is why the "copy the whole application to RAM at boot" architecture is genuinely attractive on parts with enough RAM.

## 20.4 The external memory technology comparison

| | **Serial NOR** | **Parallel/HyperBus NOR** | **Raw NAND** | **Managed NAND (eMMC/UFS)** |
|---|---|---|---|---|
| Pin count | Very low (6–11) | Higher | Moderate–high | Moderate |
| Capacity | KB → tens of MB | MB → hundreds of MB | Hundreds of MB → GB | GB → hundreds of GB |
| Random read latency | Low–moderate | Low | High (page load) | Moderate (controller-dependent) |
| XIP | Yes | Yes | No | No |
| ECC | Sometimes internal | Sometimes | **Host must provide** | Internal |
| Bad blocks | No | No | **Yes — host must manage** | Internal |
| Wear management | Host | Host | **Host (FTL)** | Internal |
| Cost/bit | High | High | Lowest | Low–moderate |
| Software burden | Low | Low | **Very high** | Low (block device) |
| Boot source | Common | Common | Possible with ROM support | Common |

**[INFERENCE]** The "software burden" row is the one that should drive the decision, and it is the one most often ignored during part selection. Raw NAND's cost advantage is real, and it is routinely consumed several times over by the engineering cost of a correct, power-fail-safe FTL. Choose raw NAND only when volume economics genuinely justify it *and* you are using a proven, qualified FTL rather than writing one.

## 20.5 Storage design patterns — choosing the right layer

**[PRINCIPLE]** There is a ladder of storage abstractions. Climbing higher costs code, RAM, and unpredictability; it buys flexibility. **Climb no higher than you need.**

```
   ┌──────────────────────────────────────────────┐
   │ 5. FILESYSTEM (LittleFS, FAT, ...)           │  files, directories
   ├──────────────────────────────────────────────┤
   │ 4. KEY-VALUE STORE                           │  named blobs
   ├──────────────────────────────────────────────┤
   │ 3. LOG-STRUCTURED / CIRCULAR STORAGE         │  append-only records
   ├──────────────────────────────────────────────┤
   │ 2. REDUNDANT FIXED STRUCTURES                │  A/B copies + CRC
   ├──────────────────────────────────────────────┤
   │ 1. RAW PARTITION                             │  firmware images
   └──────────────────────────────────────────────┘
```

| Requirement | Right answer | Wrong answer, and why |
|---|---|---|
| A firmware image | **Raw partition** | A filesystem — you gain nothing and add a dependency to the bootloader |
| A dozen settings | **EEPROM emulation / redundant structures** | A filesystem — hundreds of times the code for no benefit |
| Frequent small updates | **Append-only + wear levelling** | Fixed-address rewrite — burns endurance |
| Event log with bounded history | **Circular log** | A file that grows — you will run out and handle it badly |
| A few dozen named items of varying size | **Key-value store** | Fixed offsets — a schema change becomes a migration project |
| Arbitrary files, sizes unknown at design time | **Flash filesystem** | Rolling your own — you will reimplement it badly |
| Files interchanged with a PC | **FAT on a managed device** | A flash-native filesystem the PC cannot read |
| Secrets and keys | **Protected/secure storage** | Any of the above without protection |

**[INFERENCE]** Two blunt observations from practice:

1. **Do not put a filesystem on an MCU to store 200 bytes of configuration.** It is the most common over-engineering in embedded storage. You add several thousand lines of third-party code, a RAM buffer, a mount step that can fail, and a new class of corruption bugs — in exchange for an API you did not need.
2. **When you do need a filesystem, pick one designed for raw flash and for power loss.** A FAT implementation on raw NOR has no wear levelling and no power-fail atomicity; it will corrupt. Flash-native filesystems are built around the append-only, copy-on-write ideas of Chapter 10, scaled up — which is a good sign that you understood Chapter 10 correctly.

## 20.6 Data structures for on-flash storage

**[PRINCIPLE]** Whatever layer you choose, on-flash structures need the same discipline:

| Concern | Rule |
|---|---|
| **Alignment** | Size everything to a multiple of the program unit |
| **Padding** | Explicit. Never rely on compiler struct layout for anything that crosses a firmware version |
| **Endianness** | Chosen and documented; matters the moment a PC tool reads the flash |
| **Magic number** | Every structure. Distinguishes data from erased space and from other structures |
| **Schema version** | Every structure. This is what lets version N+1 read version N's data |
| **CRC** | Every structure, with header and payload separable |
| **Validity marker** | An explicit state, not "it looks plausible" |
| **Redundancy** | For anything whose loss is unacceptable, two copies in separate erase units |

### What to store where

| Data | Update rate | Loss tolerance | Placement |
|---|---|---|---|
| Firmware image | Rare | Must survive | Raw partition, hash-verified |
| Device identity, serial number | Once, in factory | Must never be lost | OTP if available; else redundant, write-protected |
| Calibration | Once or rarely | Must survive | Redundant copies, CRC, separate erase units |
| User settings | Occasional | Should survive | EEPROM emulation |
| Operating counters (energy, hours) | High | Must survive | RAM/FRAM hot copy + periodic + power-fail snapshot |
| Event log | Moderate | Best-effort | Circular log, oldest overwritten |
| OTA metadata | Per update | Must survive | Redundant boot control block |
| Keys | Once | Must never leak | Secure/protected storage; OTP where possible |

**[INFERENCE]** The row that shapes the most architectures is "operating counters." A utility meter's energy register, an hour meter, a cycle counter — these update constantly and must never lose more than a bounded amount. The pattern is always the same: authoritative value in volatile memory, periodic checkpoint to flash, and a power-fail interrupt that flushes. Everything in Chapters 9 and 12 exists to make that pattern safe.

## 20.7 Chapter exercises

1. Compute theoretical throughput for quad SDR at 80 MHz and octal DDR at 133 MHz. Then list four reasons your measured XIP throughput will be lower.
2. Explain the page-boundary wrap and write a test that would have caught it.
3. Why does Write Enable auto-clear, and what failure mode does that prevent?
4. Choose a storage layer for each of: a 60-byte calibration block; a 5000-entry event log; firmware slot B; a WiFi credential store. Justify each.

---

# Chapter 21 — MCU Flash versus MPU Storage

## 21.1 Two completely different meanings of "flash"

**[PRINCIPLE]** The word means different things on either side of this divide, and conflating them causes real architectural mistakes.

```
   ── MICROCONTROLLER ────────────────────────────────────────────
   ┌─────────────────────────────────────────┐
   │  CPU  ──  internal flash (memory-mapped)│
   │       ──  internal SRAM                 │
   │       ──  peripherals                   │
   └─────────────────────────────────────────┘
   Flash = a memory the CPU executes from directly.
   Boot  = fetch reset vector, run.
   Storage layers: 0–2.  RAM: KB.  Boot time: microseconds–ms.

   ── APPLICATION PROCESSOR / MPU ────────────────────────────────
   ┌─────────────────────────────────────────┐
   │  Boot ROM (mask)                        │
   │  Small internal SRAM                    │
   │  DDR controller  ──► external DRAM      │
   │  Storage controller ──► eMMC/UFS/NAND/  │
   │                          SPI NOR        │
   │  MMU, caches, multiple cores            │
   └─────────────────────────────────────────┘
   Flash = a storage device holding files and images,
           which are LOADED into DRAM to execute.
   Boot  = a multi-stage chain of signed images.
   Storage layers: 4–6. RAM: hundreds of MB–GB. Boot: seconds.
```

## 21.2 The MPU boot chain

**[PRINCIPLE]** The generic shape, which is remarkably consistent across vendors:

```
   ┌────────────────────────────────────────────────────────────┐
   │ 1. BOOT ROM (mask ROM, immutable — the root of trust)      │
   │    - reads boot mode from pins/fuses                       │
   │    - initialises the minimum: one storage interface, clocks│
   │    - loads the next stage into INTERNAL SRAM (DRAM is not  │
   │      yet available — this is why stage 2 must be small)    │
   │    - verifies its signature if secure boot is enabled      │
   └────────────────────────┬───────────────────────────────────┘
                            ▼
   ┌────────────────────────────────────────────────────────────┐
   │ 2. FIRST-STAGE LOADER (runs from internal SRAM)            │
   │    - initialises DDR — the whole reason this stage exists  │
   │    - initialises more clocks and power rails               │
   │    - loads and verifies the next stage into DRAM           │
   └────────────────────────┬───────────────────────────────────┘
                            ▼
   ┌────────────────────────────────────────────────────────────┐
   │ 3. MAIN BOOTLOADER (runs from DRAM)                        │
   │    - full storage stack: partitions, filesystems           │
   │    - may start a secure/trusted environment                │
   │    - loads kernel + hardware description + initial rootfs  │
   │    - implements A/B selection, rollback, recovery          │
   └────────────────────────┬───────────────────────────────────┘
                            ▼
   ┌────────────────────────────────────────────────────────────┐
   │ 4. OPERATING SYSTEM KERNEL                                 │
   │    - MMU, virtual memory, drivers, filesystems             │
   │    - mounts the root filesystem                            │
   └────────────────────────┬───────────────────────────────────┘
                            ▼
   ┌────────────────────────────────────────────────────────────┐
   │ 5. USERSPACE                                               │
   └────────────────────────────────────────────────────────────┘
```

**[PRINCIPLE]** Notice the constraint that drives the whole structure: **DRAM is not usable until it has been trained and configured, and the code that does that configuration must run from somewhere.** That somewhere is the SoC's small internal SRAM, which is why stage 2 has a hard size limit — often tens or low hundreds of kilobytes. Every design decision in that stage flows from this.

**[VARIES]** The number of stages, their names, where each is stored, and what each verifies differ substantially between SoC vendors. Some collapse stages 2 and 3; some add a separate security processor that boots first and supervises the application cores.

**Note on public documentation [INFERENCE]:** for several major SoC families, the *boot-chain and secure-boot architecture* is publicly documented in vendor whitepapers and security guides, while the *low-level storage controller register maps* are not public and are available only under NDA. When you cannot find controller-level documentation for a mobile-class SoC, that is usually why. Design against the documented architecture; do not infer register behaviour.

## 21.3 The storage hierarchy on an MPU

| Layer | Typical role |
|---|---|
| **SPI NOR** | Boot storage: early loaders, sometimes the whole bootloader. Small, reliable, cheap to make redundant. |
| **eMMC / UFS** | Main storage: OS images, root filesystem, application data, A/B slots |
| **Raw NAND** | Cost-optimised main storage where an FTL is available |
| **SD card** | Removable; development, field data, recovery |
| **DRAM** | Execution memory — everything runs here |

**[INFERENCE]** A very common and sensible embedded-Linux pattern: **SPI NOR for the boot chain, eMMC for everything else.** The NOR part is small, boots reliably, is easy to keep redundant, and is rarely written — so it almost never wears out. The eMMC handles the bulk, with its own internal wear levelling. Failures in eMMC then degrade the system without preventing boot, so the device can come up in a recovery mode and be reflashed.

## 21.4 The FTL, and why it changes your reasoning

**[PRINCIPLE]** Managed NAND (eMMC, UFS, SSDs) contains a Flash Translation Layer that provides: logical-to-physical mapping, wear levelling, bad-block management, ECC, garbage collection, and often write caching.

Consequences for the system designer:

| Consequence | Detail |
|---|---|
| **You cannot reason about physical layout** | Two logically adjacent blocks may be physically anywhere |
| **Write latency is unpredictable** | A write may trigger background garbage collection and take orders of magnitude longer than usual |
| **Power-fail behaviour is the device's, not yours** | The FTL's own metadata must survive; whether it does is a device property you must ask about |
| **A cache flush command exists and matters** | Data acknowledged by the device may still be volatile until flushed |
| **Wear is managed but not infinite** | Health/lifetime reporting exists on most managed devices — **read it and log it** |

**[INFERENCE]** The most common mistake when moving from MCU flash to managed NAND is assuming that a completed write is durable. It often is not until an explicit flush/sync completes. On Linux this means `fsync()` and correct filesystem barrier semantics; at the device level it means the cache-flush command. Products that skip this pass every bench test and then corrupt their filesystems in the field on power loss — the same failure the whole of Chapter 12 exists to prevent, appearing one abstraction layer higher.

## 21.5 Comparison summary

| Axis | MCU | MPU / application processor |
|---|---|---|
| Code execution | XIP from flash | Loaded into DRAM |
| Boot latency | Microseconds to milliseconds | Seconds |
| Boot stages | 1–2 | 3–5 |
| Storage abstraction | Raw or thin | Filesystem on a block device |
| Wear management | Your responsibility | Usually the device's |
| Power-fail safety | Your responsibility, entirely | Shared: FS + FTL + your `fsync` discipline |
| Update mechanism | Bootloader + image slots | Bootloader + partitions/slots + OS update framework |
| Memory protection | Optional MPU region unit | MMU, virtual memory, process isolation |
| Determinism | High | Low |
| Where "flash" bugs appear | Your driver | The interaction of five layers |

**[INFERENCE]** The most valuable thing this comparison teaches: on an MCU you can, with effort, reason about the entire path from the C statement to the silicon. On an MPU you cannot, and pretending otherwise is how storage bugs become unfixable. On the MPU side, the engineering skill is **choosing components with documented power-fail behaviour and validating them empirically** — the same fault-injection discipline of §12.6, applied to a system you do not fully control.

## 21.6 Chapter exercises

1. Explain why the first-stage loader on an MPU has a hard size limit, and what it must accomplish.
2. Why does an MCU boot in microseconds and an MPU in seconds? Give four contributing factors.
3. What does an FTL take away from you, and what does it give?
4. Design the storage architecture for a Linux-based device requiring A/B OS updates, a data partition, and unbrickable recovery. Justify each memory technology choice.

---

---

# PART VII — PRACTICE

---

# Chapter 22 — Reading a Datasheet, and the Variation Axes

## 22.1 Why a "vendor comparison table" is the wrong artefact

A table of "vendor A does X, vendor B does Y" goes stale, mixes device families, and — worst — teaches you to look up answers instead of forming questions.

**[INFERENCE]** The durable artefact is a **variation-axis table**: for each property, the *range* of behaviours that exist in the wild, and the *design consequence* of each. You then fill in your own device's column from its manual. This is the table to keep on your desk.

## 22.2 The variation axes

| Axis | Range observed across real devices | Why it varies | Consequence for you |
|---|---|---|---|
| **Erase unit** | Tens of bytes → tens of KB | Array architecture; density target | Determines write amplification and NVM layout |
| **Program unit** | 1 byte → 32+ bytes | ECC width; array width | Determines record sizing and alignment |
| **Re-program same word?** | Free / limited / forbidden | Presence and granularity of ECC | Determines whether in-place flags are legal |
| **Erased read value** | All-ones / all-zeros / **unspecified** | ECC over erased patterns | Determines whether you may compare against a constant |
| **ECC on flash** | None / parity / SEC-DED / stronger | Cost and safety target | Determines fault handling and diagnostics |
| **ECC data alias** | Absent / corrected+raw aliases | Diagnostic support | Determines how you blank-check and debug |
| **Data flash** | Absent / same macro / **separate macro** | Product positioning | Determines whether you can log while running |
| **Banks** | 1 / 2 / more | Die size; feature tier | Determines read-while-write and swap options |
| **Read-while-write** | None / cross-bank / cross-macro | Bank architecture | Determines the execution architecture (§6.6) |
| **Execute-while-program** | CPU stalled / other bank / RAM only | Same | Determines whether you need `.ramfunc` |
| **Erase suspend** | Absent / present with restrictions | Controller complexity | Determines real-time behaviour during updates |
| **Endurance** | ~10³ → >10⁶ cycles | Technology; region; grade | Determines the lifetime budget |
| **Retention** | Years, strongly temperature-dependent | Technology; temperature grade | Determines refresh policy |
| **Program/erase voltage window** | Often narrower than the operating window | Charge pump headroom | Determines brown-out threshold |
| **Flash clock constraint** | None / must be in a band | Sequencer timing derivation | Determines clock configuration ordering |
| **Command interface** | Registers / mode-bit + dummy write / vendor library | Design philosophy; IP protection | Determines how much you can control |
| **Write protection** | None / volatile / non-volatile / per-sector | Security tier | Determines bootloader protection strategy |
| **Config region** | Absent / small / CRC-enforced / one-way | Boot flexibility; security | Determines your brick risk |
| **Boot/bank swap** | Absent / boot-region swap / bank swap | Update support tier | Determines the update topology |
| **Secure boot** | Absent / vendor ROM-based / customer-key | Security tier | Determines the root-of-trust design |
| **Debug lock** | None / lockable / lock+erase-unlock / authenticated | Security tier | Determines the RMA plan |
| **External flash support** | None / SPI / QSPI / OSPI / XIP-capable | Product class | Determines the memory architecture |
| **Boot ROM** | Absent / serial loader / full boot chain | Product class | Determines recovery options |

**[INFERENCE]** Print that table. Fill a column for every part you evaluate. Two hours spent doing this before schematic capture saves months later — and it converts a vague "will this part work?" into twenty specific, answerable questions.

## 22.3 The datasheet reading protocol

**[INFERENCE]** A 1,200-page reference manual is navigated, not read. The efficient order:

### Pass 1 — Orientation (20 minutes)

1. **Datasheet, memory map section.** Sizes and base addresses of every region.
2. **Datasheet, "Flash memory characteristics" table.** Endurance, retention, program/erase times (**note the max column**), program/erase voltage range.
3. **Reference manual, flash chapter, first two pages.** The feature list tells you about ECC, banks, protection, swap.
4. **Errata document.** Search it for "flash" before believing anything above.

### Pass 2 — Geometry and rules (1 hour)

5. Erase unit and program unit, and whether they differ between regions.
6. Alignment rules and any ordering constraints.
7. Whether a word can be reprogrammed.
8. What an erased location reads as, and whether a blank-check command exists.
9. Read-while-write and execute-while-program tables. **These are usually a small table, easy to miss, and decisive.**

### Pass 3 — The control interface (2 hours)

10. Register map: command, status, address, data, protection.
11. The documented command sequences — vendors usually give flowcharts. Follow them exactly.
12. Error flags: what sets each, how each is cleared, whether a pending flag blocks new commands.
13. Clock and voltage preconditions for program/erase.
14. Interrupt behaviour.

### Pass 4 — System integration (2 hours)

15. Boot behaviour: what runs first, what selects the boot source.
16. Configuration/option region: layout, how to write it, **how to recover from a bad write**.
17. Protection mechanisms and their persistence.
18. Programming interfaces: debug port, serial loader, vendor tool.
19. Security features and lifecycle.
20. Cache/prefetch and the invalidation requirements after modification.

### Pass 5 — Confirmation (ongoing)

21. Vendor application notes on flash programming, EEPROM emulation, and bootloaders for **this family**. These often contain the practical constraints the manual states only implicitly.
22. The SDK's own flash driver source, if provided. It is the vendor's own answer to "what is the correct sequence," and it encodes workarounds the documentation omits.

**[INFERENCE]** Point 22 is undervalued. When documentation and the vendor's driver disagree, the driver is usually right — it has been run on silicon. When the driver does something the documentation does not explain, that is almost always an undocumented erratum workaround, and removing it will cost you weeks.

## 22.4 The fifteen-item checklist

Extract these, in writing, into your project's design document:

```
   ┌────┬──────────────────────────────────────────────┬─────────┐
   │  1 │ Memory map: every region, base and size      │         │
   │  2 │ Flash capacity per region                    │         │
   │  3 │ Erase unit per region                        │         │
   │  4 │ Program unit and alignment per region        │         │
   │  5 │ Endurance (with temperature + retention)     │         │
   │  6 │ Retention (with temperature + cycle count)   │         │
   │  7 │ Program/erase voltage range                  │         │
   │  8 │ Program time typ/MAX; erase time typ/MAX     │         │
   │  9 │ Protection mechanisms; volatile or not       │         │
   │ 10 │ ECC: present? granularity? fault behaviour?  │         │
   │ 11 │ Boot region and what runs first              │         │
   │ 12 │ Data flash: present? separate macro?         │         │
   │ 13 │ Read-while-write capability                  │         │
   │ 14 │ Programming restrictions (clock, IRQ, RAM)   │         │
   │ 15 │ Errata affecting any of the above            │         │
   └────┴──────────────────────────────────────────────┴─────────┘
```

## 22.5 Handling conflicting terminology

**[PRINCIPLE]** When two documents disagree in vocabulary:

1. Write down vendor A's term and its definition **from A's document**.
2. Write down vendor B's term and its definition **from B's document**.
3. Identify the underlying concept both are approximating.
4. State the actual architectural difference, if any.
5. **Never** silently substitute one term for the other.

**Worked example:**

| Vendor A says | Vendor B says | Underlying concept | Real difference |
|---|---|---|---|
| "page" = smallest erase unit | "page" = smallest program unit | Granularity | **Opposite meanings.** Using A's mental model on B's device produces code that erases 100× more than intended, or programs 1/100 of what it should. |
| "block" = erase unit | "block" = a group of sectors | Grouping | A factor-of-N difference in erase scope |
| "sector" | "segment" | Erase unit | Usually none — synonyms |
| "boot swap" | "bank swap" | Atomic image switch | Scope: a small boot region vs a whole bank. Different flash budget, different update design. |
| "data flash" | "FlexNVM" / "info memory" | Region optimised for data | May be a separate macro (concurrent access) or the same macro (no concurrency). **Decisive difference.** |

## 22.6 Chapter exercises

1. Fill the variation-axis table for a device you have worked with. Mark anything you cannot answer from documentation — those are your risks.
2. Time yourself doing Pass 1 on an unfamiliar reference manual.
3. Find one place in a vendor's SDK flash driver that does something the manual does not explain. What do you conclude?

---

# Chapter 23 — Debugging Flash Failures

## 23.1 The diagnostic method

**[INFERENCE]** Flash bugs are disproportionately difficult because the symptom is usually far from the cause, and because the system often cannot report anything. A disciplined order:

```
   1. Is it the read path or the modify path?
      (Can you read correctly? Then reads are fine.)
   2. Is the data wrong, or is the READ of the data wrong?
      (Check with a debugger, which may bypass the cache.)
   3. Did the operation report success?
      (If you don't know, your driver isn't checking status — fix that first.)
   4. Reproduce with interrupts disabled and cache disabled.
      (If it goes away, you have a concurrency or coherency bug.)
   5. Reproduce at low clock, room temperature, nominal supply.
      (If it goes away, you have a timing/voltage/temperature bug.)
   6. Only then start reading registers.
```

**[INFERENCE]** Step 3 is where most investigations should start and rarely do. A very large proportion of "flash is corrupting" reports resolve to "the driver never checked the status register, and the controller has been reporting a protection violation for six months."

## 23.2 The failure catalogue

### 23.2.1 Verify fails immediately after a successful write

| | |
|---|---|
| **Symptoms** | Program returns OK; readback differs; debugger shows the *correct* data |
| **Likely causes** | Stale cache or prefetch buffer; reading through the wrong address alias |
| **Measure** | Read the same address with the debugger (often bypasses cache) and compare |
| **Register checks** | Cache control; prefetch enable |
| **Fix** | Invalidate cache and prefetch after every modification, before readback |
| **Hardware** | None |

### 23.2.2 Data corrupts after days or weeks in the field

| | |
|---|---|
| **Symptoms** | Data was verified good at write time; wrong later; often temperature-correlated |
| **Likely causes** | Programming performed below the flash voltage minimum → marginal cells; end-of-life wear; retention at elevated temperature |
| **Measure** | Log supply voltage at every write; count erase cycles per unit; capture ECC correctable-error counts |
| **Register checks** | ECC status; any voltage-error flag |
| **Fix** | Gate writes on a supply threshold set to the *flash* minimum; add margin-read/refresh; reduce erase rate |
| **Hardware** | Brown-out threshold; bulk capacitance; supply integrity during program bursts |

**[INFERENCE]** This is the nastiest class of flash bug because the correlation between cause and effect is separated by weeks. If you take only one hardware lesson from this document: **the brown-out threshold must be set for flash programming, not for CPU operation.**

### 23.2.3 Device resets during a firmware update

| | |
|---|---|
| **Symptoms** | Update fails partway; device reboots; sometimes only in the field, only when cold, or only on old units |
| **Likely causes** | Watchdog expiring during a long erase (worst case ≫ typical); brown-out from the current surge of program/erase; interrupt storm during a critical section |
| **Measure** | Read the reset-cause register on boot; scope the supply during erase; instrument erase duration |
| **Register checks** | Reset cause; watchdog configuration; brown-out status |
| **Fix** | Erase sector-by-sector with watchdog service between; size the timeout from datasheet **max**; disable non-essential interrupts |
| **Hardware** | Decoupling adequate for the program/erase current burst |

### 23.2.4 Bootloader works, application does not start

| | |
|---|---|
| **Symptoms** | Bootloader runs; jump appears to happen; then nothing, or an immediate fault |
| **Likely causes** | Vector table not relocated; stack pointer not loaded; peripherals left running; the image was linked for a different base address |
| **Measure** | Breakpoint the application's reset handler; read PC, SP, and the vector base register; check the first two words of the image |
| **Register checks** | Vector table base; stack pointer; any pending interrupt |
| **Fix** | Follow the handover checklist of §14.4; consider the write-decision-and-reset style |
| **Hardware** | None |

### 23.2.5 Interrupts stop working after the jump

| | |
|---|---|
| **Symptoms** | Application runs but never receives interrupts, or vectors into bootloader code |
| **Likely causes** | Vector relocation not performed, or performed to the wrong base; core has no relocation register and needs a trampoline |
| **Measure** | Read the vector base register; dump the first words at the assumed base |
| **Fix** | Relocate to the application base after the jump; if unsupported, implement a trampoline |

### 23.2.6 The device is bricked after a configuration write

| | |
|---|---|
| **Symptoms** | No response on the debug port; the programmer cannot connect |
| **Likely causes** | Invalid configuration; protection or debug policy set incorrectly; an interrupted config write |
| **Measure** | Try the vendor's recovery sequence; check boot-mode pins; try the serial bootloader if the ROM provides one |
| **Fix** | Often none. This is why §19.4 says: factory only, verify by readback, read the recovery section first |
| **Hardware** | Boot-mode pins accessible on the PCB — **design them in on every board, including production units** |

**[INFERENCE]** Bring out the boot-mode/recovery pins to a test point on every board you ever design. The cost is zero. The number of units it will save over a product's life is not.

### 23.2.7 Everything works with the debugger attached and fails without it

| | |
|---|---|
| **Symptoms** | The classic |
| **Likely causes** | Timing — the debugger changes execution speed and masks a race; the debugger disables the watchdog; the debugger's flash algorithm configures the controller differently from your code; cache behaviour differs |
| **Measure** | Add a UART or GPIO trace; reproduce without breakpoints; log to NVM |
| **Fix** | Depends on the root cause — but **never ship a "works with debugger" state**; it means the failing mechanism is still present |

### 23.2.8 XIP from external flash fails or is intermittently corrupt

| | |
|---|---|
| **Symptoms** | Boot fails; random faults; works at low clock; works on one board and not another |
| **Likely causes** | Dummy-cycle mismatch; the device is not in the expected mode (quad not enabled); signal integrity at high clock; the boot configuration structure does not match the fitted part |
| **Measure** | Reduce the interface clock until it works — if that fixes it, it is timing/SI; read the device's JEDEC ID and SFDP over a slow single-line read |
| **Register checks** | Controller dummy-cycle, mode, and clock settings; the device's own status/configuration register |
| **Fix** | Reconcile dummy cycles; enable the required mode explicitly; correct the boot configuration structure |
| **Hardware** | Trace length matching; termination; DQS handling; decoupling |

### 23.2.9 Erase leaves the sector partially erased

| | |
|---|---|
| **Symptoms** | Blank check fails after a "successful" erase; some words correct, some not |
| **Likely causes** | Erase interrupted by reset/power; supply sagged; the sector is at end of life; the operation timed out and the driver did not notice |
| **Measure** | Blank-check the sector; count its erase cycles; check for an ECC error alias; scope the supply |
| **Fix** | Retry the erase; if it fails repeatedly, quarantine the sector and mark it bad in your storage metadata |

**[INFERENCE]** "Quarantine the sector" implies your storage layer can *tolerate* a bad sector. Most hand-rolled MCU storage layers cannot — they assume a fixed set of good sectors forever. Adding bad-sector tolerance is cheap at design time and impossible to retrofit after deployment.

### 23.2.10 The NVM store becomes unmountable

| | |
|---|---|
| **Symptoms** | After a power cut, all settings are lost or the device hangs at boot |
| **Likely causes** | Mount logic trusts an unvalidated length; two ACTIVE sectors from an interrupted compaction; no handling of a torn record |
| **Measure** | Dump the raw sectors and walk the record chain by hand |
| **Fix** | Chapter 10's mount algorithm; then prove it with the fault-injection rig of §12.6 |

## 23.3 The instrumentation you should build in from day one

**[INFERENCE]** These cost little and repay enormously:

| Instrument | What it catches |
|---|---|
| **Reset-cause logging to NVM** | Watchdog vs brown-out vs software reset — usually settles a whole investigation immediately |
| **Erase-count per sector** | Wear-out prediction; wear-levelling bugs |
| **Supply voltage sampled at every flash write** | The marginal-programming class of bug |
| **ECC correctable-error counters** | Early warning of degradation |
| **Flash operation timing histogram** | Detects the slow drift that precedes failure |
| **A diagnostic command dumping all of the above** | Turns a field return into a five-minute diagnosis |

## 23.4 Chapter exercises

1. For each of the ten catalogue entries, write the single most diagnostic measurement.
2. Design the NVM-resident diagnostic record: what fields, what size, where stored, and how it survives the fault it is recording.
3. Why is "works with the debugger" never an acceptable resolution?

---

# Chapter 24 — Production, Environment, and Safety

## 24.1 The production programming chain

```
   SOURCE ──► BUILD ──► IMAGE GENERATION ──► SIGNING ──► RELEASE ARTEFACT
                              │                              │
                              │  header, CRC/hash, version   │
                              ▼                              ▼
                        ┌──────────────────────────────────────────┐
                        │            PRODUCTION LINE               │
                        ├──────────────────────────────────────────┤
                        │ 1. Connect fixture (debug port / serial) │
                        │ 2. Verify blank / erase                  │
                        │ 3. Program bootloader                    │
                        │ 4. Program application                   │
                        │ 5. VERIFY by readback + hash             │
                        │ 6. Program unique data (serial, keys,    │
                        │    calibration)                          │
                        │ 7. Functional test                       │
                        │ 8. Calibrate; store calibration          │
                        │ 9. Set write protection                  │
                        │10. Program configuration region          │
                        │11. VERIFY configuration by readback      │
                        │12. Lock debug / advance lifecycle        │
                        │13. Final functional test (post-lock)     │
                        │14. Record everything to the traveller DB │
                        └──────────────────────────────────────────┘
```

**[PRINCIPLE]** The ordering is not arbitrary:

- **Verify before locking.** Once debug is locked you may have no way to diagnose.
- **Configuration and lock come last**, because they are irreversible.
- **A functional test after locking** is essential, because locking changes behaviour — this is where you discover that your secure-boot configuration rejects your own image.
- **Unique data is programmed per unit**, so the fixture must be able to inject it — which means your image format must accommodate it without invalidating the signature (usually by keeping unique data in a separate, unsigned region).

**[INFERENCE]** That last point catches teams out. If the serial number lives inside the signed application image, every unit needs its own signature — which means the signing key must be present on the production line. That is usually unacceptable. Keep per-unit data in a separate region covered by its own CRC, not by the firmware signature.

## 24.2 Programming interfaces

| Interface | Typical use | Notes |
|---|---|---|
| **Debug port (SWD/JTAG or vendor equivalent)** | Primary production programming | Fast; supports verification; the first thing you lock |
| **Serial bootloader in ROM** | Field recovery; production where debug is locked | **[VARIES]** Presence, protocol, and whether it can be disabled |
| **USB device firmware update** | Consumer products | Requires USB, and a host driver story |
| **Application-level protocol** (CAN, Ethernet, radio) | Field updates | Your bootloader implements it |
| **Gang programming / pre-programmed parts** | High volume | Programmed before assembly; complicates unique data and late firmware changes |

**[INFERENCE]** Decide early whether the ROM serial loader stays enabled in production. Leaving it enabled is a recovery path and a security hole; disabling it removes both. There is no universally right answer — but there is a wrong one, which is not making the decision consciously.

## 24.3 Voltage and power

**[PRINCIPLE]** Recap of the constraints that must appear in your hardware requirements:

| Requirement | Value |
|---|---|
| Minimum supply during program/erase | The **flash** programming minimum, which may exceed the CPU minimum |
| Brown-out threshold | At or above that flash minimum |
| Early power-fail interrupt | Above the brown-out threshold, with enough hold-up for a flush |
| Bulk capacitance | Sized for the flush budget (§12.5) |
| Decoupling | Adequate for the charge-pump current burst |
| Supply monitoring in software | Checked before every erase/program |

**[INFERENCE]** Write these into the hardware requirements document as numbered requirements with the datasheet reference. "The firmware assumes the supply is above X during flash writes" is a firmware assumption that must become a hardware guarantee, or it is question 20 of the twenty coming back to bite you.

## 24.4 Temperature

**[PRINCIPLE]** Temperature affects flash on three separate timescales:

| Timescale | Effect |
|---|---|
| **Instantaneous** | Program/erase timing and charge-pump efficiency change; some devices restrict programming outside a narrower range than the operating range |
| **Cumulative** | Retention degrades faster at higher temperature (Arrhenius); years of retention at 55 °C may be a fraction of that at 105 °C |
| **Cycling** | Thermal cycling stresses the package and can interact with wear-out |

**[VARIES]** Grade differences are real and consequential:

| Grade | Typical range | Flash implications |
|---|---|---|
| Commercial/consumer | ~0 to 70 °C | Headline retention figures usually assume something in this range |
| Industrial | ~-40 to 85 °C | Wider; retention may be de-rated |
| Extended/automotive | ~-40 to 105/125/150 °C | Retention significantly de-rated; endurance may be too; programming may be restricted at extremes |

**[INFERENCE]** If your product operates hot and stores data for years, the headline endurance/retention numbers are not your numbers. Ask the vendor for the qualification data. For automotive and industrial parts this data usually exists; you have to request it.

## 24.5 Flash in safety-related systems

**[PRINCIPLE]** These are general engineering principles. **Standards-specific requirements are a separate matter** and must be taken from the applicable standard and your safety assessment — not from a book.

General principles that appear in safety-oriented designs:

| Principle | Implementation |
|---|---|
| **Detect, don't hope** | ECC enabled and monitored; CRC over safety-relevant data; periodic background verification of code memory |
| **Redundancy for critical data** | Two copies in separate erase units; possibly different encodings |
| **Fail-safe boot** | Never execute an unvalidated image; a defined safe state when no valid image exists |
| **Diagnostic coverage** | Test the detection mechanism itself — e.g. a self-test that injects an error and confirms ECC reports it |
| **Bounded worst-case timing** | All flash operations characterised against datasheet maxima, not measurements |
| **Update integrity** | Authenticated, versioned, power-fail-safe update with a defined behaviour on failure |
| **Independence** | The mechanism that detects a fault should not share a failure mode with the thing it monitors |

**[INFERENCE]** The independence principle is the one most often violated in flash design: storing the CRC of a configuration record in the same erase unit as the record. An erase destroys both, so a corrupt-and-consistent state is indistinguishable from a valid one. **The check and the checked should not share a failure mode.** Where the CRC must live with the data (usually it must, for atomicity), get independence from a *second copy in a different erase unit* instead.

## 24.6 Chapter exercises

1. Order the production steps and justify why configuration and lock come last.
2. Why must per-unit data be outside the signed image? What is the alternative and why is it usually unacceptable?
3. Write three numbered hardware requirements covering the flash programming supply.
4. Give two examples of a check that shares a failure mode with the thing it checks, and fix each.

---

# Chapter 25 — Capstone: A Complete Storage Architecture

## 25.1 The product

**[INFERENCE]** A connected industrial device with:

- An MCU with internal flash, ECC, at least two banks, and a data-flash region
- Remote firmware update over an unreliable link
- Persistent configuration, factory calibration, an event log, and high-rate operating counters
- A ten-year service life in an industrial temperature range
- Uncontrolled power removal as a normal, frequent event
- A requirement that no single power loss can strand the device

## 25.2 Flash partition map

```
   ┌──────────────────────────────────────────────────────────────┐
   │ STAGE-1 BOOTLOADER            small, write-protected forever │
   │   - validate and select stage 2 (A or B)                     │
   │   - forced-recovery entry on GPIO                            │
   │   - never updated in the field                               │
   ├──────────────────────────────────────────────────────────────┤
   │ STAGE-2 BOOTLOADER A          updatable, validated by stage 1│
   │ STAGE-2 BOOTLOADER B          (A/B pair)                     │
   │   - update protocol, image validation, slot selection        │
   ├──────────────────────────────────────────────────────────────┤
   │ BOOT CONTROL BLOCK  copy 1    separate erase units           │
   │ BOOT CONTROL BLOCK  copy 2    slot states, versions, counters│
   ├──────────────────────────────────────────────────────────────┤
   │ APPLICATION SLOT A            bank 0                         │
   ├──────────────────────────────────────────────────────────────┤
   │ APPLICATION SLOT B            bank 1                         │
   ├──────────────────────────────────────────────────────────────┤
   │ FACTORY DATA                  write-protected after prod.    │
   │   serial number, hardware revision, model, public key hash   │
   │   (public key hash in OTP if available — preferred)          │
   ├──────────────────────────────────────────────────────────────┤
   │ CALIBRATION  copy 1           separate erase units,          │
   │ CALIBRATION  copy 2           CRC + sequence                 │
   ├──────────────────────────────────────────────────────────────┤
   │ CONFIGURATION POOL            4 × erase units,               │
   │   append-only records, wear-levelled, CRC per record         │
   ├──────────────────────────────────────────────────────────────┤
   │ OPERATING COUNTERS            data flash, small records,     │
   │   high endurance, checkpoint + power-fail flush              │
   ├──────────────────────────────────────────────────────────────┤
   │ EVENT LOG                     data flash, circular,          │
   │   oldest overwritten, best-effort                            │
   ├──────────────────────────────────────────────────────────────┤
   │ DEVICE CONFIGURATION REGION   ⚠ factory only, never in field │
   └──────────────────────────────────────────────────────────────┘
```

**[INFERENCE]** Every boundary in that map is a decision:

| Decision | Rationale |
|---|---|
| Two-stage bootloader | Stage 1 is tiny, reviewable, protected forever; stage 2 carries the risky logic and is itself A/B-protected |
| Boot control block separate from images | Slot state changes without touching an image; commit is one small write |
| BCB duplicated in separate erase units | It is the single point of failure for boot; §12.4 |
| Application A/B in separate banks | Enables update while running, and hardware swap if available |
| Calibration duplicated | Irreplaceable data; loss means the unit returns to the factory |
| Configuration in 4 erase units | Append-only + rotation for endurance headroom (Ch 11) |
| Counters in data flash | Highest write rate → highest endurance region, and (if it's a separate macro) writable while executing |
| Event log circular, best-effort | Diagnostics are valuable but must never block or fail the product |
| Config region factory-only | The unrecoverable brick class (§19.4) |

## 25.3 Boot sequence

```
   RESET
     │
     ▼
   [vendor boot ROM, if any]
     │
     ▼
   STAGE 1
     ├─ recovery GPIO asserted? ──────────► STAGE-1 RECOVERY LOADER
     │                                       (fixed serial protocol,
     │                                        no dependence on flash
     │                                        contents being valid)
     ├─ read BCB (copy 1, else copy 2)
     ├─ validate preferred stage-2 slot: magic, size, CRC, signature
     ├─ if invalid → validate the other slot
     ├─ if both invalid → STAGE-1 RECOVERY LOADER
     └─ jump ──► STAGE 2
                  │
                  ├─ apply volatile write protection to stage 1 + BCB
                  ├─ read BCB → preferred application slot
                  ├─ if state == PENDING:
                  │      increment boot-attempt counter in BCB
                  │      if counter > limit → mark slot BAD, use other
                  ├─ validate application: magic, device_id, size,
                  │      load_address, hash, signature, version ≥ floor
                  ├─ if invalid → try the other slot
                  ├─ if both invalid → STAGE-2 UPDATE MODE
                  └─ hand over ──► APPLICATION
                                     │
                                     ├─ self-test: peripherals, NVM
                                     │   mount, sensors
                                     ├─ establish the network link
                                     └─ on success: BCB state
                                        PENDING → CONFIRMED,
                                        clear boot-attempt counter
```

**[INFERENCE]** Confirmation only after the network link is established is the detail that makes this self-healing. An image that boots but cannot communicate is otherwise permanently un-updatable — the worst possible outcome for a remote device, and one that CRC and signature checking cannot prevent.

## 25.4 Update state machine

```
   IDLE
     │ server offers version V
     ▼
   DOWNLOADING ─────── error / timeout ──────► IDLE (retain old image)
     │ complete
     ▼
   VERIFYING   ─────── hash/sig/version fail ►  IDLE (erase candidate)
     │ pass
     ▼
   PENDING            ← BCB write: preferred = new slot, state = PENDING
     │                  ★ THIS SINGLE WRITE IS THE ATOMIC COMMIT
     │ reset
     ▼
   TESTING            ← boot-attempt counter incremented each boot
     │                  │
     │ app confirms     │ counter exceeded
     ▼                  ▼
   CONFIRMED          ROLLING BACK
     │                  │ BCB: preferred = old slot, mark new BAD
     │                  ▼
     │                IDLE
     ▼
   (optionally, later) advance the anti-rollback floor
```

## 25.5 Key data structures

```c
/* GENERIC EXAMPLE — capstone data structures */

#define BCB_MAGIC   0x42434231u   /* "BCB1" */

typedef enum {
    SLOT_EMPTY = 0, SLOT_CANDIDATE, SLOT_PENDING,
    SLOT_TESTING, SLOT_CONFIRMED, SLOT_BAD
} slot_state_t;

typedef struct {
    uint32_t     image_version;
    uint32_t     image_size;
    uint8_t      image_hash[32];
    uint32_t     state;              /* slot_state_t                   */
    uint32_t     boot_attempts;
} slot_info_t;

typedef struct {
    uint32_t     magic;
    uint16_t     version;            /* BCB format version             */
    uint16_t     size;
    uint32_t     sequence;           /* higher wins between copies     */
    uint32_t     preferred_slot;
    uint32_t     rollback_floor;     /* mirror of the OTP value        */
    slot_info_t  slot[2];
    uint32_t     crc;                /* over everything above          */
} boot_control_block_t;

_Static_assert(sizeof(boot_control_block_t) % FLASH_PROGRAM_UNIT == 0,
               "BCB must be a whole number of flash words");
```

## 25.6 Recovery strategy

**[PRINCIPLE]** Enumerate every failure and state the response. If a row has no response, the design is incomplete.

| Failure | Detection | Response |
|---|---|---|
| Application image corrupt | Hash mismatch at boot | Boot the other slot; report |
| Both application images corrupt | Both fail | Stage-2 update mode |
| Stage-2 corrupt | Stage 1's validation | Boot the other stage-2; else stage-1 recovery |
| Stage 1 corrupt | Not detectable from software | Requires the debug port. Mitigated by never writing it. |
| BCB corrupt (one copy) | CRC | Use the other; repair the bad one |
| BCB corrupt (both) | CRC | Assume a safe default (boot slot A, state CONFIRMED); log |
| Calibration corrupt (one copy) | CRC | Use the other; repair |
| Calibration corrupt (both) | CRC | Enter a degraded mode; refuse to report measurements as calibrated; signal for service |
| Configuration record corrupt | CRC | Use the previous valid record for that key; else the compiled default |
| NVM pool unmountable | Mount fails | Reformat the pool; restore from defaults; log prominently |
| Data-flash sector fails to erase | Blank check after retry | Quarantine the sector; continue with the remaining pool |
| ECC uncorrectable in application | Fault handler | Mark slot bad; reset; the other slot boots |
| Power lost mid-update | Boot-time state check | The commit protocol handles it; nothing special needed |
| New image boots but cannot communicate | Confirmation never arrives | Boot-attempt counter → automatic rollback |

**[INFERENCE]** Note the pattern across the table: **almost every row degrades to a working device.** Only "stage 1 corrupt" and "device configuration region corrupt" require intervention — and both are mitigated by *never writing them in the field*. That is what a fault-tolerant storage architecture looks like: the unrecoverable states are the ones you have designed yourself out of reaching.

## 25.7 Test and fault-injection plan

**Functional tests**

1. Program each slot; verify each boots
2. Update A→B and B→A; verify rollback in both directions
3. Corrupt each image deliberately; verify detection and fallback
4. Corrupt each BCB copy; verify repair
5. Corrupt both BCB copies; verify the default path
6. Exhaust the boot-attempt counter; verify automatic rollback
7. Install an image below the rollback floor; verify refusal
8. Install an image with the wrong `device_id`; verify refusal
9. Fill the configuration pool; verify compaction and continued operation
10. Fill the event log; verify wraparound
11. Force a sector erase failure; verify quarantine

**Fault injection**

12. Power cut at randomised delays throughout an update — thousands of iterations
13. Power cut during configuration writes — thousands of iterations
14. Power cut during compaction — thousands of iterations, biased toward the commit window
15. Power cut during the calibration two-copy update
16. Brown-out to intermediate voltages (not a clean cut) during writes
17. Reset assertion (not power removal) at randomised points
18. Watchdog reset injected during erase
19. Temperature extremes during update, at both limits
20. Endurance soak: cycle the configuration pool to its rated endurance and verify data integrity throughout

**Assertions after every injection**

- The device boots
- Every persistent item is either its old value or its new value — never a third thing
- Monotonic counters never decrease
- The NVM pool mounts
- Erase counts advance plausibly (no compaction loop)
- The event log contains a reset record with the correct cause

**[INFERENCE]** Item 20 is the one teams skip because it takes days of wall-clock time. Automate it and run it once per release on a rig in the corner. It is the only test that finds wear-related bugs before customers do.

## 25.8 Capstone exercises

1. Redesign this architecture for a device with a *single* flash bank and no data flash. What do you lose, and how do you compensate?
2. Redesign it for a device with only 128 KB of flash total. What do you drop first, and why?
3. Add encryption at rest. Where do the keys live, when are they used, and what new failure modes appear?
4. Write the complete recovery table for your own current product. Any empty response cell is a design defect — find it now.

---

---

# APPENDIX A — THE LAB PROGRAMME

**[INFERENCE]** Sixteen labs, in dependency order. Each is deliberately vendor-neutral: the *procedure* is what transfers between parts. Do them on whatever hardware you have. The engineering lesson at the end of each is the actual deliverable — the code is scaffolding.

**Standing requirements for every lab:** a debug probe, the reference manual and datasheet open, a UART or equivalent for output, and a notebook recording what you measured versus what the datasheet claimed.

---

### Lab 1 — Read from internal flash

**Objective.** Establish that flash is memory and prove it to yourself.
**Theory.** Ch 6. Memory-mapped read; the read path.
**Procedure.** Read and print the first 64 bytes at the flash base. Compare with the linker map and the `.hex` file. Read your own function's address and disassemble it in place.
**Expected.** The first two words are the initial SP and the reset vector. The bytes match the image you flashed.
**Failure cases.** Reading a reserved region → fault. Reading a locked region → zeros or a fault.
**Debugging.** Compare the debugger's memory view against your printed output.
**Lesson.** Your code is data at a known address. Everything else in this document follows from that.

---

### Lab 2 — Map the device

**Objective.** Produce the fifteen-item checklist (§22.4) for your part.
**Theory.** Ch 4, Ch 22.
**Procedure.** Work the five-pass protocol. Write a single `flash_map.h` with named constants for every boundary, each with a document reference in a comment. Add the errata findings.
**Expected.** A one-page table you can defend.
**Failure cases.** Family documentation that does not match your part number — always confirm against the *device* datasheet, not the family manual.
**Lesson.** Two hours here removes weeks of confusion later.

---

### Lab 3 — Erase and program one sector

**Objective.** A minimal, correct driver.
**Theory.** Ch 5, 7, 8.
**Procedure.** Pick a sector well away from your code. Erase it. Blank-check it. Program one flash word. Verify. Print the status register at each step.
**Expected.** Blank check passes; verify passes; status shows no errors.
**Failure cases.** Unaligned address; forgot to unlock; forgot to clear stale errors; region protected; cache returned stale data on verify.
**Debugging.** Print status *before and after* each command. Compare the debugger's view with your readback — a difference means a cache problem.
**Lesson.** Nine steps (§5.3), all of them. The three people skip are precondition checks, error-flag checking, and cache invalidation.

---

### Lab 4 — Measure everything

**Objective.** Replace assumptions with measurements — and then keep the datasheet maximum anyway.
**Theory.** Ch 8, 9.
**Procedure.** Time sector erase and word program with a hardware timer. Do 100 iterations; record min/mean/max. Repeat at your lowest and highest clock. Scope the supply current during erase.
**Expected.** Your measurements sit comfortably below the datasheet maximum.
**Lesson.** You now know the gap between typical and worst case. Design against the datasheet maximum regardless — you have measured one part, at one temperature, at the start of its life.

---

### Lab 5 — Store a configuration structure

**Objective.** The naive approach, done properly, so you feel its limits.
**Theory.** Ch 10, 12.
**Procedure.** Define a struct with magic, version, payload, CRC. Program it into a dedicated sector. Read and validate at boot. Deliberately corrupt one byte with the debugger; confirm detection.
**Expected.** Valid on a good record; detected on a corrupted one.
**Failure cases.** Struct size not a multiple of the program unit; compiler padding differing between builds; CRC computed over a different range than validated.
**Lesson.** `_Static_assert` your structure sizes. Serialise explicitly.

---

### Lab 6 — CRC-protected redundant storage

**Objective.** Survive power loss during a configuration update.
**Theory.** §12.4.
**Procedure.** Two copies in **separate erase units**, each with magic, sequence, CRC. Implement the update procedure of §12.4. Then cut power repeatedly during updates — by hand at first, a hundred times.
**Expected.** Every boot yields either the old value or the new one. Never a third thing, never a failure to read.
**Failure cases.** Both copies in one sector (the classic); sequence comparison that breaks on wraparound; erasing the wrong copy.
**Lesson.** This is the smallest complete power-fail-safe design. Everything larger is an elaboration of it.

---

### Lab 7 — EEPROM emulation

**Objective.** Implement Chapter 10.
**Procedure.** Append-only records with header CRC and payload CRC; sector states; mount that finds the frontier and the newest valid record per key. Test: 1,000 writes to one key; verify only the newest is returned; verify record count and space consumption.
**Failure cases.** Trusting an unvalidated `length`; a newer corrupt record shadowing an older valid one; not handling a torn record at the frontier.
**Lesson.** Validate metadata before you use it. That single rule is most of the difference between a robust store and a fragile one.

---

### Lab 8 — Wear levelling and compaction

**Objective.** Make the store survive its endurance budget.
**Theory.** Ch 11.
**Procedure.** Extend Lab 7 to four sectors with erase counts in the sector headers. Implement compaction with the ordering of §10.5.4. Instrument erase counts. Run a million simulated writes and plot the distribution across sectors.
**Expected.** Erase counts within a few percent of each other.
**Failure cases.** A target-selection policy that favours one sector; static data pinning a sector at erase count 1; a compaction loop when the live set is too large.
**Lesson.** Measure the distribution. "It should be even" is not evidence.

---

### Lab 9 — A minimal bootloader

**Objective.** Two images, one jump.
**Theory.** Ch 13, 14.
**Procedure.** Build a bootloader at the flash base and an application at an offset. Linker scripts for both. Implement the handover of §14.4. Verify that interrupts work in the application.
**Failure cases.** All six artefacts of §13.4. Interrupts vectoring into the bootloader is the most common.
**Debugging.** Breakpoint the application reset handler; check PC, SP, and the vector base.
**Lesson.** Make the application offset a single build parameter consumed by every artefact.

---

### Lab 10 — Image validation

**Objective.** Refuse to boot a bad image.
**Theory.** Ch 15.
**Procedure.** Define the image header. Write a post-build tool that computes the payload hash and fills the header. Bootloader validates plausibility → integrity → (optionally) signature. Test with: a valid image, a truncated image, a single flipped byte, an image for a different `device_id`, and blank flash.
**Expected.** Exactly one of those boots.
**Lesson.** Validate the header before using any field in it.

---

### Lab 11 — A/B update

**Objective.** Chapter 17, implemented.
**Procedure.** Two slots, a redundant boot control block, a boot-attempt counter, an application confirmation call. Update A→B, confirm; update B→A, confirm; then install an image that deliberately faults on boot and verify automatic rollback.
**Failure cases.** No boot-attempt counter → reboot loop. Non-redundant BCB → a single power loss loses everything. Wrong slot's image linked for the wrong address.
**Lesson.** The boot-attempt counter is what protects you from bugs, as opposed to corruption. They are different threats.

---

### Lab 12 — Power-loss fault injection

**Objective.** Turn a hypothesis into evidence.
**Theory.** §12.6.
**Procedure.** Build the rig: MOSFET or relay in the supply, controlled by a test computer; DUT continuously writing NVM and periodically updating firmware; randomised cut delays sweeping the full operation duration; automatic power restore, boot, and integrity query. Run 10,000 cycles overnight.
**Expected.** Zero integrity failures. Any failure is a real bug you would otherwise have shipped.
**Lesson.** Every power-fail-safe claim in your design document should cite a run of this rig.

---

### Lab 13 — RAM-resident flash routines

**Objective.** Chapter 16, in practice.
**Procedure.** Move the erase routine to `.ramfunc`. Verify placement in the map file. Set a breakpoint inside it and confirm the PC is in RAM. Then deliberately make it call `memcpy()` and observe the failure. Write the build-time check that catches this.
**Expected.** Works from RAM; fails when it calls into flash.
**Lesson.** The closure includes compiler-generated helpers you never wrote. Automate the check.

---

### Lab 14 — External SPI NOR

**Objective.** Bring up an external device from nothing.
**Theory.** Ch 20.
**Procedure.** Wire a serial NOR part. Read its JEDEC ID at a low clock, single-line. Read and parse SFDP. Erase a sector, program a page, read back. Then deliberately program across a page boundary and observe the wrap.
**Failure cases.** Wrong opcodes; wrong address width; device not out of a protected state; the page-boundary wrap.
**Lesson.** Read the ID first, always. It converts "nothing works" into "the device is not responding" or "the device is not what I expected."

---

### Lab 15 — QSPI/XIP configuration

**Objective.** Execute from external flash.
**Procedure.** Configure the controller for quad memory-mapped mode. Reconcile dummy cycles between device and controller. Link a function into the external flash region and call it. Measure execution time with cache enabled and disabled.
**Failure cases.** Dummy-cycle mismatch (check this first, always); quad mode not enabled in the device; signal integrity at high clock; attempting to erase the device you are executing from.
**Expected.** The cache-disabled measurement is dramatically slower — this is the number that tells you why cache is mandatory for external XIP.
**Lesson.** External XIP is a system built from three configurations that must agree: the device's, the controller's, and the boot configuration structure's.

---

### Lab 16 — Deliberate failure injection and diagnosis

**Objective.** Practise the diagnostic method with known answers.
**Procedure.** Have a colleague introduce one defect from the Chapter 23 catalogue into a working system without telling you which. Diagnose it using the method of §23.1. Record how long it took and which measurement settled it. Repeat for five different defects.
**Expected.** Time-to-diagnosis falls sharply after the first two.
**Lesson.** You cannot debug flash by reading code. You debug it by measuring the read path, the status register, the supply, and the cache — in that order.

---

---

# APPENDIX B — THE FLASH ENGINEER CHEAT SHEET

## B.1 The one-paragraph summary

Flash stores bits as trapped charge that shifts a transistor's threshold voltage. Charge can be added selectively in small units and removed only in large ones, so **erase granularity is coarse and program granularity is fine, and programming moves bits one way only**. The array is shared between a fast read path and a slow modify path, so **modifying flash takes it away from the CPU that executes from it**. Every stored bit degrades with time, temperature, and use, so **endurance and retention are one trade curve, not two numbers**, and ECC exists to manage the degradation. Every consequence in embedded firmware — erase-before-write, RAM-resident routines, wear levelling, EEPROM emulation, bootloaders that survive power loss — follows from those three sentences.

## B.2 Formulas

```
   wait_states        = ceil(t_access × f_CPU) − 1

   lifetime           = (endurance × sectors × records_per_sector)
                        ÷ writes_per_unit_time

   net_records/erase  = records_per_sector − live_items
                        (write amplification explodes as this → 0)

   write_amplification= bytes_erased ÷ bytes_logically_written

   holdup_time        ≈ C × (V_start − V_flash_min) ÷ I_load

   serial_throughput  = (clock × lines × edges_per_clock) ÷ 8

   ECC overhead       = check_bits ÷ data_bits
                        (SEC-DED: 8 check bits per 64 data bits = 12.5%)
```

## B.3 Design rules

| # | Rule |
|---|---|
| 1 | Erase is the expensive operation. Count erases, not writes. |
| 2 | Append, never overwrite. Newest valid record wins. |
| 3 | Validate metadata before you use it. Two CRCs: header and payload. |
| 4 | A newer corrupt record must never shadow an older valid one. |
| 5 | Redundant copies go in **separate erase units**. |
| 6 | One atomic commit point per transaction, and it must be integrity-protected. |
| 7 | Size structures to a multiple of the program unit. `_Static_assert` it. |
| 8 | Never mix update rates in one sector. |
| 9 | Set the brown-out threshold for **flash programming**, not CPU operation. |
| 10 | Increase wait states before raising the clock; lower the clock before reducing them. |
| 11 | Invalidate cache and prefetch after every modification, before readback. |
| 12 | Every flash function returns a status. Check every error flag. Time out every poll. |
| 13 | Design against datasheet **maximum**, not your measurement. |
| 14 | Never write the configuration/option region in the field. |
| 15 | Never update the bootloader in the field unless hardware makes it atomic. |
| 16 | A/B update plus a boot-attempt counter. Integrity checking alone cannot catch a bug. |
| 17 | Confirm a new image only after it has proved it can be updated again. |
| 18 | Application tasks do not write flash. One NVM owner, one queue, coalesced writes. |
| 19 | Erase sector-by-sector with a watchdog service between, not a bank erase. |
| 20 | Bring the boot-mode/recovery pins to a test point on every board. |

## B.4 The twenty questions (condensed)

Erase unit · program unit · re-program allowed? · erased value · endurance · retention · read-while-write · execute-while-program · interrupt behaviour · worst-case timings · power-fail behaviour · ECC and its faults · programming voltage and clock · reset vector source · vector relocation · bootloader protection · config region and its recovery · programming interfaces · errata · **what am I assuming that is not guaranteed?**

## B.5 Debugging triage

```
   Symptom                              Look here FIRST
   ──────────────────────────────────────────────────────────────
   Verify fails right after write   →   cache / prefetch invalidation
   Data wrong weeks later           →   supply during programming
   Reset during update              →   watchdog max vs erase max
   App won't start after bootloader →   vector base, SP, six artefacts
   No interrupts after jump         →   vector relocation
   Bricked after config write       →   §19.4 — you were warned
   Works only with debugger         →   timing, watchdog, cache
   QSPI garbage                     →   dummy cycles
   Partial erase                    →   supply, wear, timeout not checked
   NVM won't mount                  →   unvalidated length field
```

## B.6 Terminology decoder

| If a document says | It might mean | Confirm by |
|---|---|---|
| page | erase unit **or** program unit | Finding the erase-command description |
| block | erase unit **or** a group of them | Checking the erase granularity table |
| sector / segment / row | usually the erase unit | Same |
| flash word | program unit, often = ECC word | The program-command description |
| data flash / info / FlexNVM | a region optimised for data | Whether it is a **separate macro** |
| boot swap / bank swap | atomic image switch | The scope: boot region vs whole bank |
| self-programming | the device modifies its own flash | Which regions, and the constraints |
| read-while-write | concurrency across banks or macros | The specific table listing which pairs |

## B.7 The review checklist for someone else's flash code

- [ ] Does every flash function return a status, and is it checked at every call site?
- [ ] Is there a timeout on every busy-poll?
- [ ] Are stale error flags cleared *before* each command?
- [ ] Is cache/prefetch invalidated after modification, before readback?
- [ ] Are structures asserted to be a multiple of the program unit?
- [ ] Is the header validated before its length field is used?
- [ ] Are redundant copies in separate erase units?
- [ ] Is there exactly one atomic commit point per transaction, and is it CRC/ECC-protected?
- [ ] Is the supply checked before program/erase?
- [ ] Is the watchdog sized against the datasheet **maximum** erase time?
- [ ] Are `.ramfunc` symbols verified not to call flash-resident code?
- [ ] Does the mount path handle a torn record, and two ACTIVE sectors?
- [ ] Is there a boot-attempt counter, and does something clear it?
- [ ] Can the device recover if both images are invalid?
- [ ] Has any of this been proved on a power-cut rig?

---

---

# APPENDIX C — THE EXAMINATION

Answers are in Appendix D. Attempt everything before looking.

## C.1 Conceptual (30)

1. Define non-volatile precisely. What does the term *not* promise?
2. State the five-axis trade among non-volatile technologies and explain which axis flash optimises.
3. Explain the difference between flash and EEPROM in one sentence about silicon area.
4. Why is an eFuse a better place for "secure boot enabled" than a flash byte?
5. Distinguish floating-gate from charge-trap storage. Give one system-level consequence.
6. What physically happens during Fowler–Nordheim tunnelling, and why does it wear the device?
7. Why does a flash macro need charge pumps, and name three firmware-visible consequences.
8. Draw the threshold-voltage distribution picture. Mark erased, programmed, and the read reference.
9. Explain retention loss, endurance wear, and read disturb using that picture.
10. Why are endurance and retention two coordinates on one curve rather than independent numbers?
11. Why is erase granularity coarser than program granularity? Answer from the physics.
12. Explain why hardware ECC can make the erased read value non-deterministic.
13. Why does hardware ECC often forbid programming a word twice between erases?
14. From the wiring diagrams, explain why NOR supports XIP and NAND does not.
15. List four responsibilities a host must take on with raw NAND that managed NAND handles internally.
16. Explain how each additional bit per cell degrades endurance, retention, and program time.
17. Why is MCU embedded flash architecturally different from SSD NAND?
18. Name the two paths that share the flash array and explain why that sharing causes most flash hazards.
19. Give the generic region taxonomy of MCU flash and the role of each region.
20. Why is the flash controller located in peripheral address space rather than flash address space?
21. Distinguish bus width, flash word, and ECC word.
22. What are the two reasons flash banks exist?
23. Explain why raising the CPU clock can require more wait states.
24. State the correct ordering when raising and when lowering the clock, and the consequence of getting it wrong.
25. Why does prefetch make flash execution fast on average but non-deterministic in the worst case?
26. Describe the cache coherency hazard after a flash modification and three symptoms of ignoring it.
27. Explain the bootstrap paradox of external XIP and how boot ROM resolves it.
28. Name the four execution architectures for "can I execute while programming?" and the design implication of each.
29. Why should a `flash_program()` function never silently perform an erase?
30. Explain SEC-DED: what each part detects, what it corrects, and why plain SEC is dangerous.

## C.2 Calculation (15)

31. A macro has t_access = 35 ns. Compute required wait states at 20, 50, 100, and 180 MHz.
32. A 4 KB sector holds 32-byte records with a 24-byte header, aligned to 8 bytes. How many records fit?
33. With 10,000 cycles, one sector, and one erase per save: lifetime at one save per second, minute, hour, day.
34. Same part, append-only with 64 records per sector: recompute all four.
35. Add four-sector rotation to Q34 and recompute.
36. A store has 100 records per sector and 70 live items. Compute net new records absorbed per erase, and the write amplification versus a store with 10 live items.
37. Application 384 KB, bootloader 32 KB, metadata 8 KB, sector 8 KB. Compute total flash for single-slot, A/B, and A/B plus golden image, rounded to sectors.
38. C = 220 µF, V_start = 3.1 V, V_flash_min = 2.7 V, I_load = 55 mA. Compute hold-up time. If one 8-byte program takes 80 µs, how many bytes can you flush?
39. Quad SDR at 80 MHz: theoretical throughput. Octal DDR at 133 MHz: theoretical throughput.
40. A 32-byte cache line read costs 8 clocks of command+address and 20 dummy clocks before data. At quad SDR, what fraction of the transaction is overhead?
41. SEC-DED overhead for 16, 32, 64, and 128 data bits. Which is cheapest per bit?
42. Endurance 100,000. Log one 16-byte record per minute for 15 years. With a 2 KB erase unit and a 24-byte header, how many sectors do you need?
43. Erase takes 20 ms typical, 150 ms maximum. A bank erase covers 64 sectors. Compare worst-case blocking time for bank erase versus sector-by-sector.
44. A device writes a 64-byte config 40 times per second from the application. With coalescing at 1 Hz and a 4 KB sector holding 45 records, compute erases per year.
45. An image is 300 KB. Downloading over a link with 12 kbit/s effective throughput: how long, and what does that imply for the update state machine's timeouts?

## C.3 Datasheet reading (15)

46. Where in a datasheet do you find the minimum erase unit, and what words might it be called?
47. Where do you find whether a word can be reprogrammed without erasing?
48. What two conditions must accompany an endurance figure for it to be meaningful?
49. How do you determine whether data flash is a separate macro or part of the same array?
50. Which table tells you whether you can execute while programming, and what is it usually called?
51. How do you find the programming voltage minimum, and why might it differ from the operating minimum?
52. What must you check about the flash timing clock before enabling a PLL?
53. Where do you find whether the erased state is deterministic?
54. How do you determine whether a blank-check command exists and why you would need one?
55. Where is the configuration/option region layout documented, and what section must you read before writing it?
56. How do you find whether write protection is volatile or non-volatile?
57. Where do you find ECC granularity and the behaviour on an uncorrectable error?
58. What in the errata would change your flash driver design? Give three examples.
59. Two vendors both use the word "page." How do you determine which meaning each intends?
60. Your family manual and your device datasheet disagree. Which wins, and why?

## C.4 Debugging (15)

61. Program returns success; readback differs; the debugger shows correct data. Diagnose.
62. Data verified good at write time is wrong three weeks later, more often on hot units. Diagnose.
63. Firmware update resets the device, but only on units returned from cold climates. Diagnose.
64. The bootloader jumps; nothing happens. List the six things to check, in order.
65. The application runs but receives no interrupts. Diagnose.
66. Erase reports success; blank check fails; the sector's erase count is very high. Diagnose and state the response.
67. Everything works with the debugger attached. List four mechanisms that could explain it.
68. QSPI XIP returns plausible-looking garbage. What do you check first, and why that?
69. The NVM store is unmountable after a power cut, and the boot hangs. Diagnose.
70. The device reboots in a loop after a firmware update that passed signature verification. Diagnose.
71. Erase counts across sectors are 40,000 / 3 / 2 / 1. Diagnose.
72. A windowed watchdog resets the device intermittently during updates. Explain the mechanism.
73. Flash writes work at 8 MHz and fail after enabling the PLL. Give three candidate causes.
74. Two sectors are both marked ACTIVE at mount. What happened, and what do you do?
75. A DMA transfer returns corrupt data, but only during firmware updates. Diagnose.

## C.5 Architecture and design (15)

76. Design NVM storage for a device saving a 32-byte record once per second, with a ten-year life and 10,000-cycle flash. Show the arithmetic.
77. Design the boot flow for a device that must never be strandable by a single power loss.
78. Choose between single-slot, A/B, and external staging for a remote cellular sensor with tight flash. Justify.
79. Design a two-stage bootloader for a part with no boot-swap hardware. What is in stage 1?
80. Design the anti-rollback policy for a 15-year product with a 32-bit OTP counter.
81. Where do you store: firmware, serial number, calibration, user settings, energy counter, event log, keys? Justify each.
82. Design the RTOS architecture for flash access. What does the write API return when the queue is full?
83. Your part is Architecture A (CPU stalls). Design the update path for a system with a 50 ms hard deadline.
84. Design the recovery table for a device with a bootloader, two slots, and an NVM pool. Every row must have a response.
85. Design the production sequence for a device with secure boot and debug lock.
86. Design bad-sector tolerance into an EEPROM emulation layer.
87. A colleague proposes LittleFS for 200 bytes of configuration. Write the counter-argument and the alternative.
88. Design the "healthy" confirmation criterion for a remote device. Justify each condition.
89. Redesign the capstone architecture for a single-bank part with no data flash.
90. Design the fault-injection campaign that would justify shipping your NVM layer.

## C.6 Code review (10)

For each, state the defect and the fix.

91. `void flash_write(uint32_t a, uint32_t d) { FLASH->ADDR = a; FLASH->DATA = d; FLASH->CMD = GO; }`
92. `while (FLASH->STATUS & BUSY);`
93. `if (*(uint32_t*)addr == 0xFFFFFFFF) { /* sector is blank */ }`
94. `memcpy(&cfg, (void*)CFG_ADDR, sizeof cfg); if (cfg.crc == crc32(&cfg, cfg.length)) { use(cfg); }`
95. `flash_erase(SECTOR); flash_program(SECTOR, buf, len); if (memcmp((void*)SECTOR, buf, len)) return FAIL;`
96. Two copies of a config struct, both at `CONFIG_BASE` and `CONFIG_BASE + 256`, in a 4 KB sector.
97. `__attribute__((section(".ramfunc"))) void erase(uint32_t a) { memset(scratch, 0, 64); do_erase(a); }`
98. A record header with `uint16_t len; uint8_t data[]; uint32_t crc;` and no explicit padding.
99. Bootloader: `if (crc32((void*)APP_BASE, *(uint32_t*)(APP_BASE+4)) == stored_crc) jump();`
100. `settings.counter++; nvm_write(KEY_COUNTER, &settings.counter, 4);` called from an ISR at 100 Hz.

## C.7 Interview-level (20)

101. Walk me through everything that happens between `x = *(uint32_t*)addr;` and the value arriving in the register.
102. Why can't you just write a byte to flash?
103. What is the difference between endurance and retention, and why are they quoted together?
104. How would you store a value that changes 100 times per second and must survive power loss?
105. Explain your firmware update design and why it cannot brick the device.
106. Why might code need to run from RAM, and what exactly must be moved there?
107. What happens if power fails during an erase? During a program? During a compaction?
108. How do you make a multi-step flash update atomic?
109. What is the difference between a CRC and a signature, and when is each sufficient?
110. Where must the secure-boot root key be stored, and why?
111. Explain read-while-write and why it might not be available on your part.
112. Why does a hardware ECC change how you design a record format?
113. How do you choose between internal and external flash?
114. What does a Flash Translation Layer do, and what does it take away from you?
115. How would you diagnose data that verifies correctly and corrupts weeks later?
116. Why is the config/option region the most dangerous flash on the device?
117. What is wear levelling, and why does dynamic levelling fail with mixed-rate data?
118. Explain the difference between MCU flash and MPU storage to a new graduate.
119. What would you check first on an unfamiliar MCU before writing any flash code?
120. Tell me about a flash bug you have seen, what caused it, and what you changed as a result.

---

---

# APPENDIX D — ANSWER KEY

## D.1 Conceptual

1. The stored state survives loss of supply power. It does **not** promise unlimited retention, rewritability, cheap writes, unlimited endurance, or byte-granular writes.
2. Retention, endurance, write granularity, write energy/time, cost per bit. Flash optimises **cost per bit**, and every awkward property follows from that.
3. EEPROM replicates erase circuitry per byte; flash amortises it over a large block, trading erase granularity for density.
4. An eFuse change is physically irreversible, so the flag cannot be un-set by an attacker who gains write access. A flash byte can be erased.
5. FG stores charge in a conductive layer (mobile, shared pool); CT in an insulating layer (immobilised traps). Consequence: a single oxide defect can drain an entire FG cell but only local charge in a CT cell — different failure shapes, different ECC/refresh strategy.
6. A strong field thins the oxide's energy barrier enough for electrons to tunnel through. Each traversal damages the oxide and traps charge in it, narrowing the read margin — this is wear.
7. The supply is far below the tunnelling voltage. Consequences: large current bursts during program/erase; a minimum programming voltage stricter than the operating minimum; fixed ramp-up overhead per operation; temperature sensitivity.
8. Two populations separated by a margin window, with the read reference in the gap.
9. Retention loss drifts the programmed population toward erased; wear shifts and widens both; read disturb creeps unselected cells. All three narrow the window until a population crosses the reference.
10. Both describe the same margin window. Cycling narrows it, so more cycles means less residual margin, which means shorter retention. Quoting one without the other is meaningless.
11. Erase operates through structures shared across many cells (common wells, source lines). Making it finer would require replicating those structures — area cost that defeats flash's reason for existing.
12. The check bits for the erased data pattern may not be the erased check-bit pattern, so an erased word is not a valid code word and its decoded value is unspecified.
13. Reprogramming any byte requires new check bits, but the old check bits are already programmed and cannot be moved in the required direction.
14. NOR cells connect directly to bit lines, so any cell can be sensed independently and quickly — random access. NAND cells are in series strings; reading one requires driving the whole string and loading a page, which is far too slow and coarse for instruction fetch.
15. Bad-block management, ECC, wear levelling, logical-to-physical mapping, and power-fail-safe metadata.
16. The same V<sub>t</sub> range is split into more, narrower bands, so less drift is needed to cross a reference (retention ↓), less oxide degradation is tolerable (endurance ↓), more iterative program-verify steps are needed (time ↑), and more raw errors occur (ECC ↑).
17. MCU flash is SLC-class NOR-style, optimised for low-latency random read, long retention over wide temperature, and modest capacity. SSD NAND is optimised for cost per bit at huge capacity and accepts high error rates plus heavy ECC and an FTL.
18. A fast read path and a slow modify path. Because they share one array, modifying flash takes it away from the CPU executing from it — the root of erase stalls, RAM-resident code, cache coherency, and read-while-write.
19. Main/code flash (executable), data flash (optimised for data), information/factory (trim, calibration, ID), configuration/option (boot and security policy), boot region (protected first-stage code), OTP (irreversible identity and keys).
20. Because it is a peripheral that *acts on* the array. That separation is exactly why you can command an erase of the memory you are currently executing from.
21. Bus width = bytes per CPU transaction. Flash word = smallest programmable unit, often wider. ECC word = data span covered by one set of check bits, usually equal to the flash word. You must respect the flash word, not the bus width.
22. Concurrency (read/execute from one bank while modifying another) and swap (remap which bank appears at the boot address for atomic image switching).
23. Access time is fixed in nanoseconds; a faster clock means shorter cycles, so more cycles are needed to cover the same nanoseconds.
24. Raising: wait states first, then clock. Lowering: clock first, then wait states. Getting it wrong opens a window where instruction fetch outruns the flash, producing corrupted fetches and random faults.
25. Wide reads plus prefetch and cache make the average sequential/looping cost near one cycle, but branches and cold misses pay full latency — so worst case is far above average.
26. Cache and prefetch hold stale copies after a modification. Symptoms: verify-after-write fails on a good write; verify-after-erase fails on a good erase; an updated image runs the old code.
27. To execute from external flash you must configure the interface, but the configuration is stored in that flash. Boot ROM reads a fixed-format structure at a fixed offset using a slow universally-supported read, then configures the fast interface.
28. A: CPU stalls (erase time = interrupt latency). B: execute from another bank (linker becomes safety-critical). C: RAM-resident routine (most flexible, most subtle). D: separate macro, true concurrency (best for logging).
29. Because an erase destroys a whole sector — a far larger and irreversible side effect than the caller authorised. Read-modify-write belongs in a named higher layer with its own power-fail protocol.
30. SEC corrects one bit using the syndrome as a bit index; DED adds an overall parity bit so double errors are distinguishable. Plain SEC is dangerous because a double error yields a valid-looking syndrome and the decoder "corrects" the wrong bit, making three errors from two.

## D.2 Calculation

31. 20 MHz: 0.7 → 1 cycle → **0**. 50 MHz: 1.75 → 2 → **1**. 100 MHz: 3.5 → 4 → **3**. 180 MHz: 6.3 → 7 → **6**.
32. Record = 24 + 32 = 56 bytes, already 8-aligned. 4096/56 = **73 records**; with a 32-byte sector header, (4096−32)/56 = **72**.
33. 2.8 hours / 6.9 days / 1.14 years / 27.4 years.
34. 640,000 saves: 7.4 days / 1.22 years / 73 years / 1,753 years.
35. 2,560,000 saves: 29.6 days / 4.9 years / 292 years / 7,014 years.
36. 70 live → 30 net records per erase; 10 live → 90 net. The 70-live store performs **3× more erases** per logical write. Write amplification per net record: sector/30 vs sector/90.
37. Single-slot 424 KB (53 sectors). A/B: 32 + 768 + 16 = **816 KB** (102 sectors) → a 1 MB part. A/B + golden: 32 + 1152 + 16 = **1200 KB** (150 sectors) → a 2 MB part.
38. t = 220 µF × 0.4 V / 55 mA = **1.6 ms**. At 80 µs per 8 bytes → 20 programs → **160 bytes** theoretical. Design for ~80 bytes with margin, and measure it.
39. Quad SDR 80 MHz = 80e6 × 4 / 8 = **40 MB/s**. Octal DDR 133 MHz = 133e6 × 8 × 2 / 8 = **266 MB/s**.
40. Data = 32 × 8 / 4 = 64 clocks. Overhead = 28 clocks. Total 92. Overhead = **30.4%** — and that is with a favourable line size.
41. 16→6 bits (37.5%), 32→7 (21.9%), 64→8 (12.5%), 128→9 (7.0%). **128 is cheapest per bit** — which is why ECC flash has wide words.
42. Records = 15 × 365.25 × 24 × 60 = 7,889,400. Record = 40 bytes; per 2 KB sector with a 32-byte header: 50 records. Erases = 157,788. At 100,000 cycles per sector: **2 sectors minimum**, practically **4** to allow a compaction spare and 2× margin.
43. Same total time (~9.6 s at maximum). But the bank erase is one uninterruptible block of ~9.6 s, while sector-by-sector has a maximum blocking interval of **150 ms** — 64× better for watchdog and real-time purposes.
44. Coalescing 40/s down to 1/s still gives 31,536,000 writes/year. At 45 records per sector: **700,800 erases/year**, exhausting a 10,000-cycle sector in **5.2 days**. Conclusion: 1 Hz coalescing is nowhere near enough. Coalesce to a checkpoint interval of minutes, or move the hot value to RAM/FRAM with a power-fail flush.
45. 300 KB × 8 = 2,457,600 bits ÷ 12,000 bit/s = **205 s ≈ 3.4 minutes**. Implications: timeouts in minutes, resumable transfer, and the DOWNLOADING state must persist across reset — a device that restarts a 3.4-minute download from zero after every glitch may never complete one.

## D.3 Datasheet reading

46. In the flash characteristics table and the flash chapter's organisation section. Called sector, page, block, segment, row, or cluster.
47. In the flash chapter's programming section — look for "reprogramming," "multiple programming of the same address," or an ECC restriction note. If ECC is present, assume forbidden until proven otherwise.
48. Temperature and the retention period it guarantees (and often the cycle count at which retention is measured).
49. Look for a separate block in the flash chapter's block diagram, a separate controller/sequencer, and — decisively — a statement about concurrent access with code flash.
50. A small table usually titled read-while-write, concurrent operation, or flash access during program/erase. It is easy to miss and is decisive.
51. In the electrical characteristics, often as a separate row from the operating supply. It differs because charge pumps need input headroom to reach the tunnelling rail.
52. Whether the flash timing generator has its own clock source and divider, and whether its frequency must lie in a specified band. Configure it *before* raising the system clock.
53. In the flash chapter, near ECC or the erase description. Look for wording such as "the value read from erased flash is not specified."
54. Search the command list for "blank check." You need it when the erased read value is unspecified, because comparison against a constant is then invalid.
55. In its own chapter (option bytes / configuration / boot configuration). Read the recovery, failure-handling, or "invalid configuration" section **before** the first write.
56. The protection register description will state whether settings are reset on power-on (volatile) or stored in the configuration region (non-volatile).
57. In the flash chapter's ECC section and in the fault/interrupt chapter — you need both, because the reporting mechanism is often documented with the exception system.
58. Examples: a required dummy read after a command; a minimum delay between operations; a specific sequence that must not be interrupted; a protection bit that does not take effect until reset; a cache that must be disabled during programming.
59. Find each document's erase-command and program-command descriptions and see which granularity each attaches "page" to. Never carry the term across.
60. The **device datasheet** for parameters specific to that part; the family reference manual for architecture. Where they conflict on a parameter, the device datasheet wins — and raise it with the vendor, because one of them is an erratum.

## D.4 Debugging

61. Stale cache or prefetch, or the wrong address alias. Invalidate after modification, before readback.
62. Programming below the flash voltage minimum, leaving marginal cells that decay. Hot units decay faster. Fix the brown-out threshold and gate writes on supply.
63. Watchdog expiring during an erase whose duration at low temperature and end-of-life exceeds the typical you designed against. Split the erase; size against datasheet maximum.
64. Linker `ORIGIN`; vector table location; vector relocation call; bootloader jump target; debugger load offset; image generation/signing offsets.
65. Vector table not relocated, or relocated to the wrong base. Read the vector base register and dump the words at the assumed base.
66. The sector is likely worn out or the erase was interrupted. Retry once; on repeated failure, quarantine the sector and continue with the remaining pool — which requires your storage layer to tolerate bad sectors.
67. Execution timing masks a race; the debugger halts the watchdog; the debugger's flash algorithm configures the controller differently; cache/prefetch behaves differently under halt-mode debugging.
68. **Dummy cycles**, because device and controller are configured independently and a one-cycle mismatch shifts every bit into plausible-looking rubbish.
69. The mount path trusted an unvalidated length field and walked into garbage. Validate the header CRC before using any header field, and treat a torn record as the end of the log.
70. The image is intact but buggy — signature checking cannot detect that. Missing boot-attempt counter and rollback.
71. Dynamic wear levelling with static data pinning three sectors. The hot sector carries everything. Add static levelling that periodically relocates cold data.
72. A windowed watchdog also has a *minimum* service interval. Servicing immediately before and immediately after a short operation violates it.
73. Flash timing clock/divider not reconfigured for the new frequency; wait states raised after rather than before the clock; supply sagging under the higher load during program bursts.
74. A compaction was interrupted between marking the destination ACTIVE and retiring the source. Prefer the one with the higher maximum sequence number, retire the other, and finish the cleanup.
75. DMA was reading a lookup table from the flash region being erased. Enumerate active DMA channels and suspend those touching the affected region.

## D.5 Architecture and design

76. One write per second cannot be absorbed by any flash arrangement at 10,000 cycles (§9.2.2 shows even 4 sectors × 128 records gives ~59 days). The answer is architectural: keep the authoritative value in RAM, checkpoint to flash at a low rate, and flush on a power-fail interrupt. Then compute: at a 60 s checkpoint with 4 sectors × 128 records, ~9.7 years; extend the checkpoint interval or add sectors for margin.
77. Two-stage bootloader; A/B application slots; a redundant boot control block; a commit that is one integrity-protected write; a boot-attempt counter; a recovery mode reachable without any flash contents being valid.
78. External staging with a golden image, if an external part is already present; otherwise A/B if flash allows. Never single-slot on a remote device, because recovery requires the channel that just failed.
79. Stage 1 holds: reset/clock minimum, a validator (magic, size, CRC or hash), slot selection from a redundant control block, a forced-recovery entry on a pin, and a jump. A few hundred bytes to a couple of kilobytes. It is written once, protected permanently, and never updated.
80. Advance the floor only after the new image has been confirmed healthy for a defined period, and only on releases that fix a security issue. With 32 one-way bits and a 15-year life, that is roughly two advances per year — budget accordingly and record the policy.
81. Firmware → raw partition, hash-verified. Serial number → OTP if available, else redundant write-protected region. Calibration → two copies in separate erase units with CRC. Settings → EEPROM emulation. Energy counter → RAM/FRAM hot copy with periodic checkpoint and power-fail flush. Event log → circular, best-effort. Keys → secure/protected storage, ideally with the root hash in OTP.
82. A single NVM task owning the driver, fed by a bounded queue; duplicate keys coalesced; compaction scheduled when free space is low **and** the system is idle **and** the supply is healthy. The write API is non-blocking and returns a distinct "queue full" status; a separate `nvm_flush()` blocks until durable.
83. Do the update in a dedicated maintenance mode where the 50 ms deadline is suspended, entered deliberately and announced. If the deadline cannot be suspended, erase sector-by-sector only if the maximum single-sector erase fits inside the deadline; otherwise the part is unsuitable for in-service update.
84. See §25.6. The test is that every row has a response and that only "stage 1 corrupt" and "config region corrupt" require intervention.
85. Erase → program bootloader → program application → verify by readback and hash → program unique data → functional test → calibrate → set write protection → program configuration → verify configuration by readback → lock debug → **functional test again** → record to the traveller database.
86. Add a per-sector health field to the sector header; on repeated erase or blank-check failure, mark the sector BAD and exclude it from the rotation; ensure the pool still functions with N−1 sectors; report the event.
87. LittleFS adds thousands of lines, a RAM buffer, a mount step that can fail, and new corruption modes — for an API you do not need. Alternative: the Chapter 10 EEPROM emulation, a few hundred lines, purpose-built and testable.
88. Peripherals initialised, NVM mounted, sensors responding, and — critically — the update channel established and the server contacted. The last condition makes a broken-networking image self-healing via rollback.
89. Single bank: you cannot execute while programming, so the update path becomes RAM-resident or stalls; A/B still works but the swap is a software pointer, not hardware, so both images must be linked for their own addresses. No data flash: counters and logs move into main flash, so you must partition by update rate aggressively and accept lower endurance — which usually pushes you further toward RAM/FRAM hot copies.
90. Randomised power cuts across the full duration of every operation type (program, erase, compaction, config update, firmware update); brown-out to intermediate voltages; reset assertion; watchdog injection; temperature extremes; an endurance soak to rated cycles. Assertions after each: boots, every item is old-or-new, counters never decrease, pool mounts, erase counts plausible.

## D.6 Code review

91. No status return, no unlock, no precondition checks, no busy wait, no error check, no cache invalidation, no timeout. Rewrite against the nine steps of §5.3 with a `flash_status_t` return.
92. Unbounded poll — hangs forever if the controller wedges. Add a timeout derived from the datasheet maximum and return `FLASH_ERR_TIMEOUT`.
93. Invalid on ECC devices where the erased value is unspecified. Use the controller's blank-check command.
94. Uses `cfg.length` before validating the header. If `length` is corrupt the CRC is computed over the wrong range, or reads out of bounds. Validate a header CRC first, then use `length`.
95. Two defects: no status checks on erase or program, and `memcmp` against a possibly-stale cache. Check both statuses and invalidate the cache before comparing.
96. Both copies are in the same 4 KB erase unit — one erase destroys both. Place them in separate erase units.
97. `memset` will be linked into flash, so the routine calls into flash mid-erase. Write the clear as an explicit loop, and add the build-time check that verifies the `.ramfunc` closure.
98. `len` at offset 0 followed by a flexible array and a trailing CRC gives implementation-defined padding and alignment, and the CRC position depends on `len`. Use explicit padding, a fixed header size asserted to a multiple of the program unit, and separate header/payload CRCs.
99. Reads the length from the image (`APP_BASE+4` is the reset vector, not a length — and even a correct length field would be unvalidated) and uses it to bound the CRC. Read the length from a validated header, and bound it to the slot size.
100. Writing NVM from an ISR at 100 Hz: blocking flash access inside an interrupt, no coalescing, and an endurance rate that destroys the sector in days. Increment in RAM in the ISR; let the NVM task checkpoint at a low rate and flush on power-fail.

## D.7 Interview-level

101. Address decode in the bus fabric → flash read interface → cache/prefetch lookup → on a miss, word-line drive, bit-line sensing (analogue, takes ns) → ECC decode → line fill → data returned, with wait states inserted if the access exceeds one CPU cycle.
102. Because programming moves bits one direction only, in a fixed unit set by silicon (often wider than a byte), and reversing a bit requires erasing a whole sector. With ECC, a byte write would also require rewriting already-programmed check bits.
103. Endurance is erase/program cycles while still meeting retention; retention is how long data stays readable at a stated temperature. They are quoted together because they are two coordinates on one margin-window curve.
104. Keep it in RAM (or FRAM/BBSRAM), checkpoint to flash at a rate flash can sustain, and flush on a power-fail interrupt backed by adequate hold-up capacitance. Never write flash at 100 Hz.
105. Two-stage bootloader, A/B slots, redundant boot control block, single integrity-protected commit write, boot-attempt counter, application confirmation after the update channel is proven, automatic rollback, and a recovery mode independent of flash contents.
106. Because on many parts the CPU cannot fetch from an array being modified. The closure that must move: the routine, everything it calls including compiler helpers, any const tables it reads, and — if interrupts stay enabled — the vector table, every live ISR, and everything those call.
107. Erase: the sector is in an indeterminate state and must be erased again; anything uniquely stored there is lost. Program: a torn record that fails CRC and is discarded, with possible marginal cells — so erase rather than program over it. Compaction: the state machine detects the interruption at mount and redoes it idempotently.
108. Prepare in a non-authoritative location, validate, then perform **one** integrity-protected operation that transfers authority, then clean up. A partially-completed commit must be detectably invalid so it reads as "not committed."
109. A CRC detects accidental corruption and is trivially forgeable. A signature proves origin and requires an immutable key. CRC alone is sufficient only when the threat model excludes anyone who can write your flash or control your update channel.
110. In OTP, fuses, or mask ROM. If it is in rewritable flash, an attacker who can write flash substitutes their own key and the whole chain collapses.
111. The ability to read (and execute from) one region while modifying another. It may be unavailable because the device has one bank, or because the two regions share a macro or a sequencer — so it must be confirmed from the specific concurrency table.
112. Because the ECC word becomes the program unit and typically cannot be reprogrammed between erases. Records must therefore be sized and aligned to the flash word, and every state transition must write a fresh whole word rather than updating flags in place.
113. Firmware size, data volume, XIP requirement, boot requirements, BOM and pin cost, security, endurance, update strategy, temperature, and certification. Internal is simpler and more secure; external is larger and cheaper per bit but adds an interface, a driver, and a bring-up risk.
114. Logical-to-physical mapping, wear levelling, bad-block management, ECC, garbage collection, caching. It takes away your ability to reason about physical layout, makes write latency unpredictable, and moves power-fail safety into the device — where you must ask about it rather than control it.
115. Log supply voltage at every write; check ECC correctable-error counts; check erase counts per unit; correlate with temperature; verify the brown-out threshold against the flash programming minimum rather than the CPU minimum.
116. It is read before your code runs, may be CRC-enforced by boot ROM, may contain one-way fields, usually occupies a single erase unit, and an inconsistent state can permanently prevent both boot and reprogramming.
117. Distributing erases evenly so lifetime is set by the average rather than the worst sector. Dynamic levelling only moves actively-rewritten data, so cold data pins its sectors at low erase counts while hot sectors wear out — static levelling periodically relocates cold data to fix this.
118. On an MCU, flash is memory the CPU executes from directly, and you own all of wear, power-fail safety, and update. On an MPU, flash is a storage device holding images that are loaded into DRAM, behind a multi-stage boot chain and several layers of abstraction you do not control.
119. The fifteen-item checklist of §22.4 — with special attention to erase and program units, whether a word can be reprogrammed, the erased value, read-while-write, the worst-case timings, and the errata.
120. *(Open.)* A strong answer names a specific mechanism, the measurement that identified it, and the **process** change that followed — a build-time check, a hardware requirement, a test in the fault-injection rig — not just the code fix.

---

# APPENDIX E — GLOSSARY

**Append-only** — a storage discipline in which data is only added, never modified in place; the newest valid record wins.
**Bank** — an independently controllable sub-array, enabling concurrency or address swap.
**Blank check** — a controller command that determines whether a region is erased, needed where the erased read value is unspecified.
**Boot control block** — persistent metadata recording which image slot to boot and its state.
**Charge pump** — on-chip circuit generating the high voltage required for program/erase.
**Charge trap** — a storage layer of insulating material holding electrons in discrete traps.
**Commit point** — the single atomic operation that transfers authority from old data to new.
**Compaction / garbage collection** — copying live records to a fresh erase unit and reclaiming the old one.
**Disturb** — unintended V<sub>t</sub> change in cells adjacent to an operated cell.
**ECC word** — the data span protected by one set of check bits.
**Endurance** — erase/program cycles a unit tolerates while still meeting retention.
**Flash word** — the smallest programmable unit; often equal to the ECC word.
**Floating gate** — a conductive, electrically isolated storage layer.
**FTL** — Flash Translation Layer; the mapping and management layer inside managed NAND.
**LMA / VMA** — load address (where stored) versus virtual/execution address (where it runs).
**OTP** — one-time programmable; irreversible storage.
**Prefetch** — speculatively fetching the next flash line to hide latency.
**Retention** — how long data remains readable, at a stated temperature and cycle count.
**Read-while-write** — the ability to read one region while modifying another.
**SEC-DED** — single error correction, double error detection.
**SFDP** — a standardised self-description table inside serial flash devices.
**Torn write** — a program operation interrupted partway, leaving an incomplete record.
**Wait state** — an inserted CPU stall cycle covering flash access latency.
**Wear levelling** — distributing erases evenly across erase units.
**Write amplification** — bytes physically erased/written per byte logically written.
**XIP** — execute in place; fetching instructions directly from non-volatile memory.

---

# APPENDIX F — ON SOURCES

**[INFERENCE]** This document is deliberately architecture-general. Vendor documentation was consulted while writing it, not to build a per-device reference, but to check that every **[PRINCIPLE]** claim is true across real implementations and that every **[VARIES]** claim genuinely varies. Families examined for that purpose spanned 8/16-bit and 32-bit MCU embedded flash, ECC and non-ECC macros, single- and dual-bank parts, hardware-assisted EEPROM emulation, external serial NOR, and application-processor boot chains.

**What you should do with this document.** Use it to generate questions, not answers. For any real design:

1. **Datasheet** for your exact part number — parameters, geometry, electrical limits.
2. **Reference/technical manual** for the family — architecture, registers, sequences.
3. **Programming manual** where one exists — the authoritative command sequences.
4. **Application notes** for flash programming, EEPROM emulation, and bootloaders *on that family* — these carry the practical constraints the manual only implies.
5. **Errata** — check before believing anything above.
6. **SDK flash driver source** — the vendor's own answer to "what is the correct sequence," including undocumented workarounds.

Always record the **document revision and date** alongside any parameter you design against, in the header where you define it. Manuals are revised; your assumption should be traceable to the revision it came from.

**On the RL78/I1C document.** You referenced an attached Renesas flash chapter; the upload area was empty in this conversation, so nothing here relies on it. If you attach it, the natural next step is a device-specific companion that fills the variation-axis table of §22.2 from that manual — code flash and data flash geometry, the self-programming constraints, the boot/bank swap behaviour, the security settings, and the power-interruption recovery guarantees — mapped onto the concepts established here.
