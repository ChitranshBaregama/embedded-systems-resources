# UART — RX Ring Buffer

## Problem
Implement a fixed-size UART RX ring buffer suitable for one producer (ISR) and one consumer (main/task).

Define full/empty behavior and explain which shared fields need special treatment.


## Scored contract clarification

Use fixed storage; define capacity and whether one slot is reserved. State one producer/one consumer ownership, overflow policy and target atomicity assumptions. Test empty/full, wraparound, byte order, overrun accounting and adversarial interleavings. Host tests alone cannot prove target ISR timing.

Timebox: agree before starting (default 30 minutes coding or 20 minutes oral/design). Status: Unseen. Original source mapping is recorded in career/MIGRATION.md.
