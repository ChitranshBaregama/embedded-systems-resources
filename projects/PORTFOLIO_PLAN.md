# Portfolio plan

All three projects start **Not started / no learner implementation evidence recorded**. Generated plans and existing AI-assisted examples are study resources, not proof of independent delivery. Select supported public hardware after checking available equipment; a hardware purchase is not authorized by this plan.

| Project | Deliverable | Acceptance evidence |
| --- | --- | --- |
| 1: Production-style RTOS firmware | Acquisition + bounded telemetry pipeline; separate drivers, tasks, ownership and diagnostics | Timing/memory budget, queue overload handling, fault injection, watchdog policy, tests and target measurements or clearly limited emulator evidence |
| 2: Bootloader / secure update | Signed-image validation and recoverable update using a reviewed upstream foundation | Threat model, slot/state diagram, bad-signature rejection, interrupted updates at each phase, rollback policy and recovery tests; only public test keys |
| 3: Linux / systems / driver | Observable device interface or driver-backed service with a userspace client | ABI/lifetime design, partial I/O, errors/concurrency, reproducible build, tests, traces and performance limitations |

## Required project record

Each selected project gets architecture.md, learner implementation, tests, CI, DEBUGGING.md, LIMITATIONS.md and VERIFICATION.md. Verification records compiler/version, command, commit, expected/actual result, board/emulator and measurement conditions. Do not create a “complete” badge from a scaffold.

Milestones: requirements → architecture review → smallest executable slice → negative/fault tests → integrated CI → measured characterization → public write-up → mock deep dive. Every milestone has an artifact link, date, owner and unresolved risks. Never substitute a simulated timing number for a bench measurement.

## Tracker

| Project | Stage | Artifact | Next action | Verified level |
| --- | --- | --- | --- | --- |
| RTOS pipeline | Planned | None | Select public target and write constraints | None |
| Secure update | Planned | None | Compare upstream foundation and failure model | None |
| Linux system | Planned | None | Define device boundary and reproduction environment | None |
