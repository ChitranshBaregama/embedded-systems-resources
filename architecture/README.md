# Architecture

What the silicon is actually doing underneath the C.

| | |
| :--- | :--- |
| [startup-and-linker.md](startup-and-linker.md) | Reset to `main`, the vector table and its two easy-to-miss attributes, sections and their real cost, LMA versus VMA, map files, stack sizing, bootloader handoff. Backed directly by `code/qemu-cortex-m/common/` |
| [interrupts-and-nvic.md](interrupts-and-nvic.md) | Exception entry and exit, priority and preemption, `volatile`, atomicity, critical sections, latency and jitter, fault exceptions. The document most directly backed by runnable code — see `code/qemu-cortex-m/` examples 02, 03 and 05 |
| [memory-systems.md](memory-systems.md) | DRAM physics through to allocator engineering, via cache coherency, the architectural memory model, ECC, ARMv8-A translation, IOMMU/SMMU, TrustZone and side channels |
| [flash-memory.md](flash-memory.md) | A full curriculum on non-volatile memory: the floating gate, NOR vs NAND, threshold distributions, the MCU flash subsystem, the flash controller as a state machine, wear, and firmware update |

## Reading order

Start with `startup-and-linker.md`. It is the shortest, it has a runnable
program that proves every claim in it, and nothing else here makes sense
until you know where your bytes actually live.

Then `interrupts-and-nvic.md`, because everything else assumes it. Then `flash-memory.md` chapters 1–6
for the part you actually program. `memory-systems.md` is a continuation
document (parts XXXVIII–LX) and assumes the earlier material — it is the right
place to go for application-processor-class questions, not MCU ones.

## Still to write

The clock tree, DMA, and boot/OTA architecture (which builds on both
`startup-and-linker.md` and `flash-memory.md`). See
[`../ROADMAP.md`](../ROADMAP.md).
