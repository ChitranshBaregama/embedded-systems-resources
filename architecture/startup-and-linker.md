# Startup Code and Linker Scripts

> Everything that happens between the reset pin going high and the first line
> of `main()` — and the file that decides where every byte of your image
> lives.

**Contents**
[Cheat sheet](#1-cheat-sheet) ·
[Reset to main](#2-reset-to-main-the-whole-sequence) ·
[The vector table](#3-the-vector-table) ·
[Sections](#4-sections-and-why-there-are-so-many) ·
[The linker script](#5-the-linker-script) ·
[Load vs virtual address](#6-load-address-vs-virtual-address) ·
[Code](#7-code) ·
[The map file](#8-reading-a-map-file) ·
[Stack sizing](#9-stack-sizing) ·
[Common failures](#10-common-failures) ·
[Beyond the basics](#11-beyond-the-basics) ·
[Questions](#12-questions-i-should-be-able-to-answer) ·
[Sources](#13-sources)

---

## 1. Cheat sheet

**On Cortex-M, reset does exactly three things in hardware:**

1. Load the word at address `0x00000000` into the **main stack pointer**.
2. Load the word at `0x00000004` into the **program counter**.
3. Start executing.

That is the entire hardware boot. Everything else is code you wrote.

| Section | Contents | Costs flash | Costs RAM | Initialised by |
| :--- | :--- | :---: | :---: | :--- |
| `.isr_vector` | Vector table | yes | no | linker placement |
| `.text` | Code | yes | no | — |
| `.rodata` | `const` data, string literals | yes | no | — |
| `.data` | Initialised globals ≠ 0 | **yes** | **yes** | copy loop in startup |
| `.bss` | Globals = 0 or uninitialised | no | yes | zero loop in startup |
| `.noinit` | Data that must survive reset | no | yes | **nothing** — deliberately |
| stack | Locals, saved registers | no | yes | SP set from vector[0] |
| heap | `malloc` | no | yes | `sbrk` / allocator |

**Six things worth knowing cold:**

1. **Vector entry 0 is not code.** It is the initial stack pointer value. Put a
   function pointer there and you fault before the first instruction.
2. **`.data` costs flash *and* RAM.** The initial values live in flash and are
   copied to RAM at startup. `static int x = 0;` is free (it lands in `.bss`);
   `static int x = 1;` costs four bytes of each.
3. **`.bss` is zeroed by your startup code**, not by the loader, not by the
   hardware. The C standard's promise is only kept if you keep it.
4. **Marking a large lookup table `const`** moves it from `.data` to `.rodata`
   and stops it eating RAM. This is the single cheapest RAM saving available.
5. **`.data` is uninitialised until the copy loop finishes**, so anything that
   runs before it — including some toolchains' `SystemInit()` — must not touch
   initialised globals.
6. **Stack and heap grow toward each other** and nothing checks. The collision
   is silent unless you put a guard there.

---

## 2. Reset to main: the whole sequence

```
   power / reset
        │
        ├─ hardware: MSP ← [0x00000000]
        ├─ hardware: PC  ← [0x00000004]
        │
   Reset_Handler:
        │
        ├─ (optional) set the clock tree - PLL, flash wait states
        ├─ copy .data from its flash image into SRAM      ← _sidata → _sdata.._edata
        ├─ zero .bss                                      ← _sbss.._ebss
        ├─ (C++) run static constructors                  ← __libc_init_array()
        ├─ (optional) enable FPU, configure MPU, set VTOR
        │
        └─ main()
```

**Why the clock tree comes first, if it comes at all.** Running the copy loop
at 16 MHz when the part can do 168 MHz just makes boot slower. But raising the
clock requires raising flash wait states *first*, and the code that does that
must not depend on initialised globals — because `.data` has not been copied
yet. That ordering constraint is the reason vendor `SystemInit()` functions are
written the way they are, and the reason they are so easy to break.

**Why `main()` never returns.** On a hosted system `main` returning means exit
to an OS. There is no OS. A `for(;;)` after the call is the honest answer;
falling off the end executes whatever follows in flash.

---

## 3. The vector table

```c
__attribute__((section(".isr_vector"), used))
void (* const g_vectors[])(void) = {
    (void (*)(void))(&_estack),  /*  0: initial stack pointer  */
    Reset_Handler,               /*  1: reset                  */
    NMI_Handler,                 /*  2                         */
    HardFault_Handler,           /*  3                         */
    /* ... 4-15: core exceptions ... */
    /* 16+: external interrupts, vendor-specific */
};
```

Four details that matter:

**`const`** puts it in `.rodata` and therefore in flash, where the core expects
to find it at reset. Drop the `const` and it lands in `.data` — which is not
copied to RAM until *after* reset has already read address zero. The table the
hardware reads would be whatever the linker left in flash at that address.

**`used`** stops the compiler discarding it. Nothing in your program references
`g_vectors`; only the hardware does. Without `used` — and without `KEEP()` in
the linker script — `--gc-sections` removes it and the image will not boot.

**`KEEP()`** in the linker script is the other half of the same problem. Both
are needed.

**Weak aliases** let a translation unit override a handler by simply defining a
function with the right name:

```c
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));
```

No registration, no table writes, no runtime cost. The linker resolves it. This
also means a **typo in a handler name silently does nothing** — the weak
default stays bound and your interrupt appears not to fire. It is a genuinely
common bug and there is no warning for it.

**`VTOR`** relocates the table on Cortex-M3 and above. A bootloader that jumps
to an application sets `SCB->VTOR` to the application's vector table, then loads
that table's MSP, then jumps. Miss the `VTOR` write and the application's
interrupts dispatch into the *bootloader's* handlers — which usually appears as
"the application works until the first interrupt".

---

## 4. Sections, and why there are so many

The distinctions exist because flash and RAM have different costs and different
persistence.

```c
const char msg[] = "hello";      /* .rodata - flash only            */
int   counter;                   /* .bss    - RAM, zeroed at start  */
int   limit = 100;               /* .data   - flash AND RAM         */
static int cache[256];           /* .bss    - 1 KB of RAM, no flash */
static const int lut[256] = {…}; /* .rodata - 1 KB of flash, no RAM */
```

Those last two lines are the same table. One costs a kilobyte of RAM, the other
a kilobyte of flash. On a part with 20 KB of RAM and 256 KB of flash, that
single `const` is the difference between fitting and not.

**`.noinit`** is the section people forget exists. Data placed there is neither
copied nor zeroed, so it survives a warm reset:

```c
__attribute__((section(".noinit"))) uint32_t reset_reason;
__attribute__((section(".noinit"))) uint32_t crash_pc;
```

That is how a fault handler leaves a breadcrumb for the next boot to report. It
needs a matching `NOLOAD` output section in the linker script, and it is
uninitialised after a *cold* start — so pair it with a magic word you check.

---

## 5. The linker script

Three parts, and it is worth writing one by hand once.

**`MEMORY`** — what physically exists:

```ld
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 256K
  SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}
```

**Symbols** — values the startup code needs:

```ld
_estack = ORIGIN(SRAM) + LENGTH(SRAM);
```

Linker symbols have no storage. `_estack` is an *address*, so C must take its
address, never its value:

```c
extern uint32_t _sdata;      /* declared as an object...  */
uint32_t *p = &_sdata;       /* ...but only &_sdata is meaningful */
```

Reading `_sdata` reads whatever bytes happen to sit at that address. This is
the most common linker-script bug in C, and it compiles without complaint.

**`SECTIONS`** — where each input section goes:

```ld
.isr_vector : ALIGN(4) { KEEP(*(.isr_vector)) } > FLASH

.text : ALIGN(4) {
    *(.text) *(.text*) *(.rodata) *(.rodata*)
} > FLASH

_sidata = LOADADDR(.data);

.data : ALIGN(4) {
    _sdata = .;
    *(.data) *(.data*)
    _edata = .;
} > SRAM AT > FLASH        /* ← the important line */

.bss (NOLOAD) : ALIGN(4) {
    _sbss = .;
    *(.bss) *(.bss*) *(COMMON)
    _ebss = .;
} > SRAM
```

`.` is the location counter. `_sdata = .;` records the current address into a
symbol; `. = ALIGN(4);` advances it.

---

## 6. Load address vs virtual address

This is the concept the whole file turns on, and it is worth stating plainly.

```
> SRAM AT > FLASH
```

means: **this section will RUN at an address in SRAM, but is STORED at an
address in FLASH.**

- **VMA** (virtual memory address) — where the code expects it at runtime.
  `_sdata`.
- **LMA** (load memory address) — where the programmer actually writes the
  bytes. `_sidata = LOADADDR(.data)`.

Nothing moves them for you. The copy loop in `Reset_Handler` is what bridges
LMA to VMA:

```c
uint32_t *src = &_sidata;    /* flash: the stored image  */
uint32_t *dst = &_sdata;     /* SRAM:  where it must be  */
while (dst < &_edata) { *dst++ = *src++; }
```

Delete those three lines and every initialised global holds garbage. On QEMU
that garbage is usually zero, which is *worse* — the bug hides until you flash
real silicon.

`.bss` needs no LMA because it has no contents; `(NOLOAD)` tells the linker not
to reserve image space for it. That is why a 4 KB `.bss` buffer adds nothing to
your `.bin` file size while a 4 KB `.data` buffer adds 4 KB.

The same mechanism is what "run this function from RAM" uses — a common trick
for flash-write routines, which cannot execute from the flash they are erasing:

```ld
.ramfunc : ALIGN(4) {
    *(.ramfunc) *(.ramfunc*)
} > SRAM AT > FLASH
```

with a matching copy loop.

---

## 7. Code

Everything above is implemented and running in this repository:

| File | What it demonstrates | Verified |
| :--- | :--- | :--- |
| [`code/qemu-cortex-m/common/lm3s6965.ld`](../code/qemu-cortex-m/common/lm3s6965.ld) | A complete, commented linker script — MEMORY, symbols, the `AT >` split, `NOLOAD`, a heap region | Links every example |
| [`code/qemu-cortex-m/common/startup.c`](../code/qemu-cortex-m/common/startup.c) | Vector table with weak aliases, the `.data` copy, the `.bss` zero, the trap on `main` returning | Runs under QEMU |
| [`code/qemu-cortex-m/01-startup-and-memory-map/`](../code/qemu-cortex-m/01-startup-and-memory-map/) | Prints the runtime address of a `.data`, `.bss`, `.rodata` and stack object, and **verifies the copy and zero loops actually ran** | Runs under QEMU |

```bash
cd code/qemu-cortex-m/01-startup-and-memory-map && make run
```

Its output, which is the whole of this document made concrete:

```
.data  g_initialised = 0xDEADBEEF   (copied OK)
.bss   g_zeroed      = 0x00000000   (zeroed OK)

-- linker symbols --
.data (RAM)  0x20000000 .. 0x20000004  size=4 bytes
.data image in flash at 0x0000073C          ← LMA, not VMA
.bss  (RAM)  0x20000004 .. 0x2000100C  size=4104 bytes

g_banner   (.rodata, flash) 0x00000710
g_big_buffer  (.bss,  SRAM) 0x20000008
stack_local   (stack, SRAM) 0x2000FFDC
_estack       (top of SRAM) 0x20010000

Stack has grown down 36 bytes from the top.
```

Read that alongside `make size` on the same binary and the relationship between
sections, flash and RAM stops being abstract.

---

## 8. Reading a map file

`-Wl,-Map=firmware.map` costs nothing and answers questions nothing else will.

**"Why is my image so big?"** The map lists every object file's contribution
per section. The culprit is usually `printf` — pulling in newlib's full
formatting machinery is routinely 8–20 KB, and `%f` drags in floating-point
conversion on top.

**"What pulled in this symbol?"** The map's cross-reference table names the
object that referenced it. This is how you discover that one `sprintf` in an
error path is costing you 12 KB.

**"Where did my RAM go?"** Sort the `.bss` entries by size. The answer is
almost always one buffer that was sized generously years ago.

**"Is my section actually where I think?"** The map shows the VMA and, where
they differ, the LMA. If a `.ramfunc` did not get its `AT >`, this is where you
see it.

Also add `-Wl,--print-memory-usage`, which prints a flash/RAM summary on every
link:

```
Memory region         Used Size  Region Size  %age Used
           FLASH:        1856 B       256 KB      0.71%
            SRAM:        5136 B        64 KB      7.84%
```

Watching those two numbers move as you edit builds intuition about cost faster
than any amount of reading.

---

## 9. Stack sizing

Stack overflow is the archetypal embedded bug: no fault, no message, just a
global that changes value on its own.

**Estimate statically.** GCC's `-fstack-usage` emits a `.su` file per
translation unit giving each function's frame size. Combined with a call graph,
that bounds the worst case — and it is the only method that covers a path your
tests never took. Watch for the things it cannot see: recursion, function
pointers, variable-length arrays, and `alloca`.

**Measure dynamically.** Paint the stack with a pattern at startup, run the
worst-case workload, then count how much is still unpainted:

```c
/* in Reset_Handler, before main, after .bss */
for (uint32_t *p = &_sbss_end; p < (uint32_t *)__get_MSP() - 16; p++) {
    *p = 0xC0FFEEu;
}
```

The high-water mark is the number you actually needed. Most projects find they
allocated three times too much or half enough, and rarely something sensible in
between.

**Catch it when it happens.** An MPU region of a few hundred bytes at the
stack's low limit, marked no-access, turns a silent corruption into a MemManage
fault with a recoverable stack frame — see
[`code/qemu-cortex-m/05-hardfault-decoder/`](../code/qemu-cortex-m/05-hardfault-decoder/).
On a part with no MPU, a magic word at the limit checked periodically is a
cheap approximation.

**Under an RTOS every task has its own stack**, and the sizing problem
multiplies. FreeRTOS's `uxTaskGetStackHighWaterMark()` does the painting trick
for you. Note that interrupts on Cortex-M run on the *main* stack (MSP) while
tasks run on PSP — so an ISR's worst-case depth is a separate calculation, and
one people routinely forget.

---

## 10. Common failures

| Symptom | Cause |
| :--- | :--- |
| Hard fault before `main`, PC nonsense | Vector entry 0 is not a valid stack pointer, or the table is not at the address the core reads |
| Every initialised global is 0 | The `.data` copy loop is missing, or `_sidata` is wrong |
| Globals hold garbage after warm reset only | `.bss` zero loop missing; cold boot happened to give zeros |
| Works in debug, fails at `-O2` | Missing `volatile`, or code that relies on `.data` before the copy loop |
| Image will not boot after adding `--gc-sections` | Missing `KEEP()` on `.isr_vector`, or missing `used` on the table |
| One interrupt never fires, others do | Handler name typo — the weak default is still bound. **No warning is issued** |
| Interrupts break after a bootloader jump | `SCB->VTOR` not set to the application's table |
| A global changes value with nothing writing it | Stack overflow into `.bss` |
| C++ objects have unconstructed members | `__libc_init_array()` not called |
| Linker: "region FLASH overflowed" | Genuinely too big — read the map before deleting features |
| Linker: "will not fit in region SRAM" | Usually one oversized `.bss` buffer |
| `float` printing prints nothing | Newlib-nano needs `-u _printf_float` explicitly |

---

## 11. Beyond the basics

**Bootloader handoff.** The full sequence, in order — get any of it wrong and
the symptom is "works until the first interrupt":

```c
uint32_t app_msp = *(volatile uint32_t *)APP_BASE;
uint32_t app_pc  = *(volatile uint32_t *)(APP_BASE + 4);

__disable_irq();
/* deinit every peripheral the bootloader started */
SCB->VTOR = APP_BASE;
__set_MSP(app_msp);
__enable_irq();
((void (*)(void))app_pc)();
```

**Multiple images.** A/B slot layouts need two application regions plus shared
metadata, expressed in `MEMORY` and selected at link time.

**XIP versus copy-to-RAM.** Executing in place from external QSPI flash is
slower and jitters with cache misses; copying to RAM at boot costs RAM and boot
time. Which is right depends on whether your worst case is throughput or
determinism.

**Flash-write routines must not execute from the flash being erased.** Hence
`.ramfunc` and its copy loop.

**Position-independent code** matters when an image can be relocated. It is a
whole subject; the entry point is `-fPIC` and the GOT.

---

## 12. Questions I should be able to answer

1. What exactly does Cortex-M hardware do on reset, before any of your code
   runs?
2. Why is the first vector table entry not a function pointer?
3. Why must the vector table be `const`, and what specifically breaks if it is
   not?
4. What is the difference between LMA and VMA, and which linker syntax creates
   the split?
5. `static int x = 0;` versus `static int x = 1;` — what does each cost in
   flash and in RAM, and why?
6. Why does making a large lookup table `const` save RAM?
7. Your `.bin` is 4 KB smaller than the sum of your globals. Why?
8. Why do `KEEP()` and `__attribute__((used))` both exist, and why do you often
   need both?
9. A handler name is misspelled. What happens at link time, and at run time?
10. Why must the clock-tree setup run before the `.data` copy on some parts,
    and what constraint does that place on the code that does it?
11. How would you place a variable so it survives a warm reset, and what must
    you add to make it trustworthy after a cold one?
12. Describe two ways to measure worst-case stack usage, and what each misses.
13. A bootloader jumps to an application. The application runs until the first
    interrupt, then hard-faults. What did the bootloader forget?
14. Why can a flash-write routine not execute from the flash it is erasing, and
    how is that solved in the linker script?

---

## 13. Sources

- **ARM, *ARMv7-M Architecture Reference Manual* (DDI 0403)** — reset
  behaviour, the vector table, `VTOR`, exception entry. Section B1.5 is the
  normative statement of what reset does.
- **ARM, *Cortex-M4 Devices Generic User Guide* (DUI 0553)** — the same
  material, more approachable.
- **GNU `ld` manual, "Linker Scripts"** — the reference for `MEMORY`,
  `SECTIONS`, `AT >`, `LOADADDR`, `ALIGN` and the location counter.
- **GCC manual, `-fstack-usage`, `-ffunction-sections`, `--gc-sections`.**
- **Newlib documentation** — `__libc_init_array`, `_sbrk`, and what
  newlib-nano leaves out.
- **ST AN2606** — bootloader and memory layout on STM32, as a concrete example
  of a real boot architecture.

## Related in this repository

- [`code/qemu-cortex-m/common/`](../code/qemu-cortex-m/common/) — the linker
  script and startup code this document describes, both running
- [`code/qemu-cortex-m/01-startup-and-memory-map/`](../code/qemu-cortex-m/01-startup-and-memory-map/)
  — the runnable proof
- [`architecture/interrupts-and-nvic.md`](interrupts-and-nvic.md) — what the
  vector table's other 100 entries do
- [`architecture/flash-memory.md`](flash-memory.md) — the flash subsystem the
  image is written into, and firmware update architecture
- [`patterns/driver-bring-up.md`](../patterns/driver-bring-up.md) — phase 1 is
  where a broken startup shows up first
