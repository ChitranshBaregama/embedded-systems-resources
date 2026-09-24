# Roadmap

Gaps, stated openly. A reference that hides its holes is harder to trust than
one that lists them, and listing them is also how they get closed.

Ordered by leverage, not by ease.

---

## 1. Hardware verification — the largest gap

Several documents are written and reasoned through but have never met a
board. Nothing else on this list changes the credibility of the repository as
much as closing this one.

| Blocked on | Needed for |
| :--- | :--- |
| Logic analyzer (8-channel with protocol decode) | Every `## 5. Captures` section — UART, I²C, SPI |
| STM32 Nucleo board | Running `code/stm32f4/` at all |
| USB-TTL serial adapter | UART captures, bootloader work |
| A USB-CAN adapter + a second node | Everything in `can.md` §5, and any real ISO-TP exchange |
| Oscilloscope access | Interrupt latency measurement, I²C rise time, SPI signal integrity |

Hardware on hand: ESP32, Raspberry Pi 4 (usable as an SWD debugger over GPIO
with OpenOCD).

**Definition of done:** every section 5 checklist item ticked, with a captioned
capture and a note on what the failure case looks like. Every "STATUS: not run
on hardware" warning either removed or replaced with measured numbers.

---

## 2. Missing peripheral and architecture documents

Now available: [GPIO](peripherals/gpio.md), [ADC](peripherals/adc.md), and
[RTC](peripherals/rtc.md). RTC includes host-tested calendar helpers; its oscillator,
backup-power, alarm, and retention bench experiments remain unperformed.
Their teaching examples and planned bench experiments do not establish hardware validation.

Written to the same eight-section template as the existing peripheral docs.

| Document | Priority | Notes |
| :--- | :--- | :--- |
| `architecture/dma.md` | High | Transfer types, circular mode, half/full callbacks, cache coherency on M7, and why DMA plus a non-cache-aligned buffer is the hardest class of embedded bug |
| `architecture/timers-and-clock-tree.md` | Medium | PLL configuration, prescaler arithmetic, input capture, output compare, PWM, encoder mode |
| `architecture/boot-and-ota.md` | Medium | Bootloader architecture, A/B slots, rollback protection, power-fail-safe update. Overlaps `flash-memory.md` — should reference it, not repeat it |
| `rtos/rtos-internals.md` | Medium | Context switch mechanics on Cortex-M, PendSV, priority inversion, queue and semaphore internals, stack sizing, tickless idle |
| `debugging/playbook.md` | Medium | GDB/OpenOCD workflow, SWO/ITM, map files, stack-depth measurement, heisenbug bisection |
| `practice/dsa-patterns.md` | Medium | ~15 recurring patterns worked through, aimed at the algorithmic interview round |

---

## 3. Code

| Item | Status |
| :--- | :--- |
| QEMU examples 01–06 | Done — build and run, verified by `run-all.sh` |
| Host tests for portable logic | Done — 45 tests, 1.8M assertions, ASan + UBSan |
| CAN bit-timing solver | Done — 8 tests |
| ISO-TP transport | Done — 18 tests including a fuzz pass |
| A register-level CAN driver | Not started. Needs two nodes and two transceivers; no usable QEMU target |
| STM32F4 I²C / SPI drivers | Compile-verified only; needs hardware |
| RTOS example — two tasks and a context switch | Not started. PendSV-based, would run under QEMU |
| DMA example | Not started. QEMU's lm3s6965 has no usable DMA; needs a different machine or real hardware |
| A CI job that runs everything | See below |

---

## 4. Infrastructure

- [x] Root README with a real index
- [x] Per-directory README files
- [x] Licence
- [x] `.gitignore`
- [x] CI: build all examples, run host tests, check every relative link
- [ ] GitHub Pages, so the long documents are readable without scrolling a
      raw markdown blob
- [ ] Split `flash-memory.md` (46k words) and `physical-layer.md` (66k words)
      into per-chapter files. They are past the size where a single file is
      the right container — GitHub truncates the rendered view and in-page
      anchors become the only navigation

---

## Explicit non-goals

Worth stating so the scope stays honest:

- **Not a tutorial series.** These documents assume you can already write C
  and read a datasheet. They are written for the second and third time you
  meet a topic, not the first.
- **Not vendor documentation.** Where a reference manual is the right answer,
  the document cites it rather than paraphrasing it.
- **Not a code library.** The code exists to demonstrate and verify the
  prose. Do not vendor it into a product.

