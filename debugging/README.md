# Debugging

Not written yet as a standalone document — but the material is not missing,
it is distributed. Every peripheral and architecture document carries a
**symptom-first debugging checklist** in section 6, written to be usable at
1am without reading the rest of the file:

- [UART §6](../peripherals/uart.md#6-debugging-checklist)
- [I²C §6](../peripherals/i2c.md#6-debugging-checklist)
- [SPI §6](../peripherals/spi.md#6-debugging-checklist)
- [Interrupts §6](../architecture/interrupts-and-nvic.md#6-debugging-checklist)

And one debugging tool here is runnable rather than described:

### [`code/qemu-cortex-m/05-hardfault-decoder/`](../code/qemu-cortex-m/05-hardfault-decoder/)

A fault handler that recovers the stacked exception frame, prints the faulting
PC, and decodes CFSR/HFSR into English. Six selectable fault causes:

```bash
cd code/qemu-cortex-m/05-hardfault-decoder
make FAULT=1 run   # undefined instruction
make FAULT=2 run   # function pointer with LSB clear -> INVSTATE
make FAULT=3 run   # divide by zero
make FAULT=4 run   # unaligned access
```

The default `while(1)` fault handler is where most firmware debugging dies.
Replacing it is the highest-value hour you can spend on a new project.

## Planned

A consolidated playbook: GDB and OpenOCD workflow, SWO/ITM tracing, printf
alternatives that do not break timing, bisecting a heisenbug, reading a map
file, stack-depth measurement, and postmortem analysis from a crash dump.

See [`../ROADMAP.md`](../ROADMAP.md).
