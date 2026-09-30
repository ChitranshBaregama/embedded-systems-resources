# Embedded C — Register Access

## Problem
Design a small C interface for a hypothetical 32-bit peripheral register.

Support read, write, set-mask, clear-mask and field update. Explain where `volatile` is required and where it is insufficient for synchronization.


## Scored contract clarification

Specify the fictional register semantics first: ordinary RW versus W1C, read side effects, access width and concurrent writers. Do not treat all registers as safe read-modify-write targets. Use a mock memory model for host tests; never dereference a real MMIO address on a host.

Timebox: agree before starting (default 30 minutes coding or 20 minutes oral/design). Status: Unseen. Original source mapping is recorded in career/MIGRATION.md.
