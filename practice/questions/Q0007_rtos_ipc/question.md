# FreeRTOS — IPC Selection

## Problem
For each scenario, choose among direct-to-task notification, binary semaphore, counting semaphore, queue, event group, or mutex and justify the choice:

1. ISR wakes exactly one processing task.
2. Transfer eight-byte messages between tasks.
3. Protect a shared I2C bus.
4. Count repeated hardware events.
5. Wait for several independent subsystem-ready flags.


## Scored contract clarification

For each scenario specify payload ownership, whether events coalesce, ISR API constraints, timeout behavior and priority-inversion concerns. Cite the exact selected RTOS version/API documentation; this is a design review, not a simulated passing C test.

Timebox: agree before starting (default 30 minutes coding or 20 minutes oral/design). Status: Unseen. Original source mapping is recorded in career/MIGRATION.md.
