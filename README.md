# Embedded Systems Reference

Working notes on bare-metal firmware, written to be used at a bench rather
than read once. Roughly 250,000 words of reference material, plus code that
builds and runs on a laptop with no development board attached.

Everything here is written from primary sources — reference manuals, IEC and
IEEE standards, ARM architecture documents — and checked against a compiler
or an emulator wherever a claim can be checked at all. Where something has
**not** been verified on hardware, it says so in the text. That distinction is
kept deliberately, because a note that quietly blurs "I read this" into "I
measured this" is worse than no note.

---

## Start here

| If you want to… | Go to |
| :--- | :--- |
| See working code in the next 60 seconds | [`code/`](code/) — `./run-all.sh` |
| Understand a specific bus | [Peripherals](#peripherals) |
| Understand what the silicon is doing | [Architecture](#architecture) |
| Structure firmware that stays debuggable | [Patterns](#patterns) |
| Prepare for embedded C interviews | [Practice](#practice) |
| Understand the wire beneath the protocol | [Networking](#networking) |

---

## Peripherals

Serial buses, each written to the same eight-section shape: cheat sheet →
how it works → registers → code → captures → debugging → questions → sources,
followed by depth sections on electricals, timing, error recovery, driver
architecture and testing.

| Topic | Covers | Size |
| :--- | :--- | ---: |
| [UART](peripherals/uart.md) | Framing, baud arithmetic, ring buffers, DMA and idle-line detection, RS-485, RS-232 handshaking, LIN, bootloading over serial | 7.9k words |
| [I²C](peripherals/i2c.md) | Open-drain electricals, clock stretching, error-recovery ladder, target mode, bit-banging, 10-bit addressing, SMBus/PMBus, I3C | 9.0k words |
| [SPI](peripherals/spi.md) | The four modes, signal integrity, round-trip timing, DMA, QSPI and memory-mapped flash, SD over SPI, peripheral mode | 7.3k words |
| [CAN](peripherals/can.md) | Bitwise arbitration, frame format, bit stuffing, bit timing and the sample point, error counters and fault confinement, filters and mailboxes, CAN FD, J1939/CANopen/UDS, worst-case response time analysis | 6.2k words |

## Architecture

| Topic | Covers | Size |
| :--- | :--- | ---: |
| [Interrupts and the NVIC](architecture/interrupts-and-nvic.md) | What belongs in an ISR, `volatile`, race conditions, atomicity, latency and jitter, nesting, RTOS interaction, fault exceptions | 6.4k words |
| [Memory systems](architecture/memory-systems.md) | DRAM cell to channel, controller engineering, cache coherency, the architectural memory model, ECC, ARMv8-A translation, IOMMU/SMMU, TrustZone, side channels, allocator engineering | 7.4k words |
| [Flash memory engineering](architecture/flash-memory.md) | A full vendor-independent curriculum: the physics of a floating gate, NOR vs NAND, the MCU flash subsystem, the controller as a state machine, wear and endurance, bootloaders and firmware update | 46k words |

## Patterns

| Topic | Covers | Size |
| :--- | :--- | ---: |
| [State machines and statecharts](patterns/state-machines.md) | Five implementations compared, hierarchical state machines, run-to-completion semantics, timers, anti-patterns, testing, code generation | 5.8k words |
| [Driver bring-up](patterns/driver-bring-up.md) | A seven-phase procedure for an unfamiliar peripheral: what to prove in what order, how to read a datasheet for bring-up rather than reference, how to bisect a dead bus, and a symptom table | 3.6k words |

## Networking

| Topic | Covers | Size |
| :--- | :--- | ---: |
| [The physical layer: from signals to silicon](networking/physical-layer.md) | Book-length treatment of the PHY — bandwidth and impairments, Nyquist and Shannon, line coding, digital modulation, FEC, copper and fibre, the wireless channel, link budgets, regulatory constraints, antennas and RF front-ends | 66k words |

## Practice

| Topic | Covers | Size |
| :--- | :--- | ---: |
| [Embedded C interview core](practice/embedded-c-interview-core.md) | 150 solved problems across 14 parts — bit manipulation, register I/O, memory and string functions, allocators, data structures, CRC, interrupts and concurrency, RTOS primitives, drivers, protocol parsers, state machines, algorithms, debugging — with retention tables | 63k words |
| [1500 exercises](practice/embedded-c-1500-exercises.md) | A five-tier drill bank. Each tier covers every topic area at one depth before the next tier revisits it deeper | 21k words |

## Code

Runnable. See [`code/README.md`](code/README.md) for the full list.

```bash
cd code/host-tests    && make          # 45 unit tests under ASan + UBSan, ~4s
cd code/qemu-cortex-m && ./run-all.sh  # six bare-metal examples under QEMU
```

| | |
| :--- | :--- |
| [`code/qemu-cortex-m/`](code/qemu-cortex-m/) | Six bare-metal Cortex-M3 programs: startup and memory map, SysTick, NVIC preemption, a lock-free ring buffer driven by a real ISR, a HardFault decoder with six selectable faults, and a framed-protocol parser |
| [`code/portable/`](code/portable/) | Hardware-independent logic — protocol parser, CRC-16, SPSC ring buffer, CAN bit-timing solver, ISO-TP transport — compiled unmodified by both the target build and the host tests |
| [`code/host-tests/`](code/host-tests/) | 45 tests, 1.8M assertions, including two fuzz passes under AddressSanitizer |
| [`code/stm32f4/`](code/stm32f4/) | Register-accurate I²C and SPI masters. Compile-verified for Cortex-M4; **not** hardware-verified, and labelled as such |

---

## What "verified" means here

Three different claims, kept separate on purpose:

| Label | Means |
| :--- | :--- |
| **Runs under QEMU** | The program was executed and its output checked. Logic, control flow and register sequencing are real. Timing and analogue behaviour are not. |
| **Compile-verified** | Builds clean at `-Wall -Wextra -Werror -Wconversion` for the target core. The compiler agrees; no hardware has seen it. |
| **Not yet verified** | Written from the reference manual and reasoned through. Trust it the way you would trust a careful colleague's whiteboard sketch. |

[`code/README.md`](code/README.md) documents the specific places QEMU
diverges from real silicon — the ones found while writing these examples,
not a generic disclaimer.

---

## Related

- [**DLMS/COSEM security**](https://github.com/ChitranshBaregama/EncryptionAlgorithm-) — a seven-volume reference on the DLMS/COSEM security stack: AES-GCM/GMAC, HLS/LLS, key architecture, ECDSA/ECDH, PKI, wire-format analysis, attack analysis and labs, built from the Green Book 8th edition with independently verified test vectors.
- [**SMS P10 notice board**](https://github.com/ChitranshBaregama/sms-p10-notice-board) — GSM-controlled scrolling LED notice board on ATmega2560 + SIM800.

## Roadmap

Gaps are tracked openly in [`ROADMAP.md`](ROADMAP.md) rather than left as
silent holes. The largest ones right now: DMA, timers and the clock tree, RTOS
internals, startup and linker scripts, and the hardware captures that several
documents are waiting on.

## Licence

Prose is [CC BY 4.0](LICENSE); code is [MIT](LICENSE-CODE). Use it, adapt it,
credit it.
