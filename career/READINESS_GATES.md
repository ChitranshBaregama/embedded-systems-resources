# Eight readiness gates

These thresholds are proposed personal coaching criteria, not company hiring rules. All gates start **Unassessed**. A coach reviews actual evidence; a script does not certify competence.

| Gate | Scope | Demonstration | Status |
| --- | --- | --- | --- |
| G1: C mastery | Pointers, lifetime, arrays, integer conversions, overflow, aliasing, alignment, volatile and undefined behavior | Implement and explain three unseen memory/bit tasks; diagnose two UB examples without guessing | Unassessed |
| G2: DSA | Arrays, lists, stacks, queues, hash tables, trees, heaps, searching, sorting and complexity | Solve three unseen mixed patterns under stated constraints; explain invariants and bounded memory use | Unassessed |
| G3: MCU/SoC fundamentals | Reset, startup/linker, memory map, clocks, NVIC, GPIO, timers, ADC, DMA and RTC | Trace reset to main and an interrupt; derive one clock/timing budget; debug a peripheral configuration | Unassessed |
| G4: Drivers and protocols | UART, SPI, I2C, CAN, framing, timeouts, retries, partial reads, bus recovery | Deliver a parser/driver exercise with malformed input and transport failure tests; separate portable logic from hardware | Unassessed |
| G5: RTOS and concurrency | Scheduling, priority inversion, mutexes, queues, notifications, ISR handoff, atomics and memory ordering | Explain three interleavings, reproduce a race, fix a synchronization defect and justify ISR/task API choices | Unassessed |
| G6: Systems depth | Linux processes/kernel interfaces, bootloaders, networking, C++ ownership, debugger and build tools | Complete a Linux diagnosis, an update failure analysis and a C++ lifetime review; explain hardware/software boundaries | Unassessed |
| G7: Production engineering | Architecture, tests, CI, fault injection, performance, documentation and open source | Present the three portfolio projects with honest evidence; complete an upstream-quality contribution with regression evidence | Unassessed |
| G8: Full interview readiness | Coding, deep C, architecture, debugging, RTOS, design, projects and behavioral communication | Pass three consecutive complete mock loops with no major fundamentals gap | Unassessed |

## Common pass criteria

For gates 1–6, collect at least five representative attempts including the gate demonstrations. On the most recent five, target at least 80% independent success, no unresolved critical correctness defect, warning-clean code where applicable, and test/explanation scores at least 3/4. Require at least one successful unassisted 7-day and one 30-day retest on representative material before marking a gate retained. A provisional pass while retention is pending is explicitly provisional.

Independent success means correct against the agreed contract, no implementation assistance or viewed solution, zero hints, and within the agreed task budget. An assisted solve remains useful but is not independent. Classify a major gap as an unresolved misunderstanding of memory safety, C undefined behavior, synchronization/ownership, interrupt safety, or a core design requirement—not a minor syntax slip.

## Scoring anchors

| Score | Test quality | Explanation/debugging quality |
| --- | --- | --- |
| 0 | No meaningful tests | Cannot explain or diagnose |
| 1 | Happy path only | Guesses without evidence |
| 2 | Some boundaries; important holes | Mostly correct with prompts |
| 3 | Boundaries, invalid input and justified oracle | Independent coherent reasoning and discriminating experiments |
| 4 | Fault injection/properties and regression evidence | Explains tradeoffs, limitations and transfers reasoning to a new case |

## Evidence register

| Gate | Attempt IDs / artifact links | 7-day | 30-day | Reviewer/date | Decision and next gap |
| --- | --- | --- | --- | --- | --- |
| G1–G8 | None recorded yet | Pending | Pending | — | Unassessed |

Gate 7 requires learner-authored implementation evidence; generated plans are not projects. Gate 8 additionally requires the [three-loop rule](../mock-interviews/LOOP.md). A new major gap resets the consecutive successful-loop streak.
