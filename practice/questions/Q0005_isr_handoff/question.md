# Interrupts — ISR Event Handoff

## Problem
Design an ISR-to-main-loop event handoff for a bare-metal MCU.

The ISR may receive bursts of events. Avoid long ISR execution and avoid silently losing events. Discuss atomicity and overflow behavior.


## Scored contract clarification

State single-core assumptions, event multiplicity, capacity, overflow reporting and interrupt priorities. Provide interleaving traces including an interrupt between read and clear. Volatile alone is not a synchronization proof. Identify which operations require critical sections or atomic operations.

Timebox: agree before starting (default 30 minutes coding or 20 minutes oral/design). Status: Unseen. Original source mapping is recorded in career/MIGRATION.md.
