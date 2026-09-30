# 150 interview questions

Separate from both the 1,500 drills and the existing 150 solved C examples. Prompts only: answer aloud before writing notes. IDs I001–I150 are stable. The source plan’s 25 project/behavior prompts are split into 15 project and 10 behavioral prompts.

These are original practice prompts, not leaked or official company interview questions. Use the [full mock procedure](../../mock-interviews/LOOP.md) and [progress ledger](../progress/README.md). For each answer: state assumptions, give the mechanism, present evidence or a test, discuss failure modes, and explain a tradeoff. No solution is supplied here.

## Technical deep dive (60)

### I001

Explain integer promotions for uint8_t on two ABIs with different int widths.

### I002

Distinguish signed overflow, unsigned wrap and implementation-defined conversion.

### I003

Explain why shifting by the operand width is invalid and how to specify a safe bit API.

### I004

Trace an object's lifetime through returning a pointer to a local variable.

### I005

Compare an array expression, pointer parameter and pointer-to-array declaration.

### I006

Explain effective type and aliasing constraints in a binary parser.

### I007

Explain alignment requirements and safe decoding of unaligned wire data.

### I008

Distinguish const pointer, pointer-to-const and volatile object semantics.

### I009

Explain why volatile does not establish atomicity or inter-thread ordering.

### I010

Describe sequence/order constraints in expressions with multiple side effects.

### I011

Design an overflow contract for a size calculation before allocation.

### I012

Explain padding, packing, endianness and why raw structs are fragile wire formats.

### I013

Compare stack, static and dynamic allocation on a bounded-memory target.

### I014

Explain ownership and failure handling in a dynamically allocated C API.

### I015

Compare memcpy and memmove contracts including overlapping storage.

### I016

Analyze realloc failure handling without losing the original allocation.

### I017

Explain reentrancy versus thread safety using a concrete API.

### I018

Explain an interrupt vector table and reset-handler sequence.

### I019

Trace .data and .bss initialization from the linker map to runtime.

### I020

Explain how linker garbage collection interacts with vectors and KEEP.

### I021

Distinguish load and execution addresses in a firmware image.

### I022

Explain clock sources, PLLs, prescalers and peripheral timing dependencies.

### I023

Describe interrupt pending, enable and priority behavior under nesting.

### I024

Explain a critical section's scope and effects on worst-case latency.

### I025

Describe GPIO electrical modes and safe reset-state behavior.

### I026

Calculate a timer period and identify off-by-one register conventions.

### I027

Explain PWM frequency/resolution tradeoffs and update timing.

### I028

Explain ADC source impedance, acquisition time and reference error.

### I029

Describe DMA buffer ownership and cache maintenance requirements.

### I030

Explain RTC snapshot coherence and backup-domain validity.

### I031

Analyze UART baud mismatch, framing and receive overrun.

### I032

Explain SPI CPOL/CPHA and chip-select timing requirements.

### I033

Explain I2C open-drain signaling, pull-ups and clock stretching.

### I034

Explain CAN arbitration versus error handling and bus-off recovery.

### I035

Compare polling, interrupts and DMA for a bounded data rate.

### I036

Design a streaming parser contract for partial and malformed input.

### I037

Explain CRC coverage and why it is not message authentication.

### I038

Compare an RTOS queue, semaphore, notification, event group and mutex.

### I039

Explain priority inversion and the limits of priority inheritance.

### I040

Distinguish deadlock, livelock, starvation and a race condition.

### I041

Explain single-producer/single-consumer ownership and publication ordering.

### I042

Analyze acquire/release semantics in an actual producer-consumer example.

### I043

Explain why a lock-free operation need not be wait-free.

### I044

Describe deadline, period, execution time and blocking in a scheduling budget.

### I045

Explain tickless idle and the constraints of its wakeup timebase.

### I046

Distinguish a process, thread, syscall and interrupt in Linux.

### I047

Explain partial read/write, EINTR and nonblocking I/O handling.

### I048

Describe a Linux driver's user ABI, ownership and lifetime boundaries.

### I049

Explain virtual memory, page faults and mapping device memory.

### I050

Explain C++ RAII and deterministic resource release on failure paths.

### I051

Compare unique ownership, shared ownership and borrowed references in C++.

### I052

Explain virtual dispatch and dynamic allocation tradeoffs in firmware.

### I053

Trace ARP/IP/UDP or TCP through a local network transaction.

### I054

Explain TCP stream framing and why one recv is not one message.

### I055

Describe boot image validation, trust anchors and rollback policy.

### I056

Explain power-fail-safe update state transitions and recovery.

### I057

Distinguish encryption, authentication, signature and secure boot.

### I058

Explain nonce uniqueness across reset and interrupted persistence.

### I059

Describe watchdog coverage and why feeding from a timer can hide a failure.

### I060

Explain compiler optimization effects on debugging and why debug success is insufficient.

## Debugging scenarios (35)

### I061

An MCU never reaches main after a linker change. What evidence do you collect first?

### I062

A board resets only during radio transmission. Separate supply and software hypotheses.

### I063

A UART line is LOW at idle. Design experiments before changing baud rate.

### I064

A receive ring drops bytes only under logging load. Find the limiting stage.

### I065

I2C SDA stays LOW after reset. Diagnose ownership and recovery limits.

### I066

SPI data are shifted one bit only on one sensor revision. Build a timing investigation.

### I067

CAN works with two nodes but fails after a long cable is added. Separate electrical and timing causes.

### I068

An ADC reading changes when a second channel is enabled. Investigate acquisition and sequencing.

### I069

DMA data look stale only with cache enabled. Prove which ownership boundary is wrong.

### I070

A GPIO pulse appears during boot and activates a load. Trace reset and mux transitions.

### I071

An RTC loses time only with main power removed. What power-state evidence matters?

### I072

A calendar read is wrong once a day near midnight. Test snapshot behavior.

### I073

An interrupt fires continuously after wakeup. Distinguish source and controller pending state.

### I074

A task misses deadlines after adding a low-priority logger. Test inversion and blocking.

### I075

A queue fills even though average consumer throughput is sufficient. Analyze bursts and scheduling.

### I076

A binary semaphore loses event multiplicity. Demonstrate the mismatch to requirements.

### I077

A crash disappears when a printf is added. Preserve evidence without assuming the print fixed it.

### I078

A hard fault occurs after return from a function. Inspect stack and exception evidence.

### I079

A memory corruption appears hundreds of calls after a write. Choose instrumentation and a reproducer.

### I080

A signed/unsigned comparison passes local tests but fails on another compiler. Isolate the language rule.

### I081

A bit operation fails at bit 31. Test type width and shift semantics.

### I082

A packed packet parser faults on one CPU but works on x86. Diagnose alignment without hiding bounds bugs.

### I083

A parser accepts truncated frames after a timeout. Analyze state reset and resynchronization.

### I084

A field firmware update bricks only when power is interrupted. Build a phase-by-phase failure matrix.

### I085

Boot verification rejects an image that verifies on a host. Compare exact bytes, formats and trust inputs.

### I086

Flash settings revert after a rapid reboot. Test commit ordering and power-fail behavior.

### I087

A watchdog reset occurs while all tasks appear alive. Define progress evidence for each subsystem.

### I088

A Linux application blocks forever on a device read. Trace user/kernel state and wakeups.

### I089

A userspace test passes but the driver fails under concurrent close. Investigate lifetime and reference counts.

### I090

A TCP parser fails with small network writes. Test stream fragmentation and coalescing.

### I091

A device occasionally reuses a cryptographic nonce after brownout. Design a safe reproduction with test keys.

### I092

Firmware passes at -O0 and fails at -O2. Separate UB, races and compiler defects with evidence.

### I093

Only one board out of fifty fails crystal startup. Form a component/environment measurement plan.

### I094

A low-power product drains its battery too quickly. Quantify duty cycle and leakage paths.

### I095

CI passes while the target fails. Identify exactly what CI proves and design the missing test.

## Firmware/system design (30)

### I096

Design a battery-powered sensor logger with a bounded timestamp error during outages.

### I097

Design a UART-to-network gateway with bounded RAM under malicious or stalled input.

### I098

Design an RTOS acquisition pipeline with explicit DMA ownership and overload handling.

### I099

Design a recoverable signed firmware update for a two-slot flash layout.

### I100

Design a single-slot update strategy and explain its power-loss limitations.

### I101

Design an I2C bus manager for multiple tasks and an occasionally stuck target.

### I102

Design a reusable SPI driver boundary for devices with different transaction timing.

### I103

Design a CAN telemetry node with bus-off recovery and diagnostic counters.

### I104

Design a bootloader/application handoff contract including vectors and interrupts.

### I105

Design persistent settings with wear limits and interrupted-write recovery.

### I106

Design a crash-record system that preserves useful evidence without leaking secrets.

### I107

Design a watchdog supervisor based on subsystem progress rather than task existence.

### I108

Design a monotonic and wall-clock service across sleep, reset and synchronization.

### I109

Design a bounded command parser with authentication and replay considerations.

### I110

Design a sensor calibration record with versioning and traceability.

### I111

Design an event queue for ISR-to-task delivery with explicit overflow policy.

### I112

Design a real-time control loop alongside best-effort networking.

### I113

Design a Linux userspace service and driver ABI for a custom measurement device.

### I114

Design hot-unplug handling and lifetime ownership for a Linux device interface.

### I115

Design a C++ embedded component with RAII and no hidden allocation after startup.

### I116

Design a test strategy separating pure logic, emulator behavior and hardware validation.

### I117

Design deterministic startup when optional peripherals fail.

### I118

Design secure provisioning using public test credentials in a portfolio environment.

### I119

Design a device identity and key-rotation lifecycle with stated trust assumptions.

### I120

Design an energy budget and measurement plan for a year-long battery target.

### I121

Design a firmware observability plan with bounded logging overhead.

### I122

Design a rate-limited retry policy across an unreliable link and scarce power.

### I123

Design a protocol version migration that preserves backward compatibility.

### I124

Design a timing analysis for nested interrupts and a high-priority task.

### I125

Design a fault-containment boundary between safety-relevant control and diagnostic features.

## Project deep dive (15)

### I126

Draw your project's actual architecture and identify the highest-risk boundary.

### I127

Explain the hardest bug you personally reproduced and the evidence that isolated it.

### I128

Show a test that failed before your fix and explain why it is a useful regression.

### I129

Defend a memory-budget decision using a map file or measured artifact.

### I130

Defend a timing claim and identify every unmeasured assumption.

### I131

Explain an interface you changed after discovering a failure mode.

### I132

Describe your project's overload behavior and demonstrate it.

### I133

Walk through power loss at the worst possible update/persistence instant.

### I134

Explain what your CI does not verify.

### I135

Show the ownership model for a shared buffer or resource.

### I136

Explain a rejected design and the concrete tradeoff behind rejection.

### I137

Describe an upstream component you used and what you verified yourself.

### I138

Separate your own implementation from generated, borrowed or reference code.

### I139

Explain how another engineer can reproduce the build and a known failure.

### I140

State the project's biggest remaining limitation and the next experiment to reduce it.

## Behavioral communication (10)

### I141

Describe a time you were wrong technically and how evidence changed your decision.

### I142

Describe a disagreement where you clarified requirements rather than winning an argument.

### I143

Explain a missed commitment, its impact and what you changed afterward.

### I144

Describe how you prioritized reliability against feature pressure.

### I145

Explain a complex firmware issue to a non-specialist without hiding uncertainty.

### I146

Describe how you asked for help after doing a focused investigation.

### I147

Describe a review comment you initially resisted and what you learned.

### I148

Give an example of improving a team's reproducibility or debugging process.

### I149

Describe a decision involving incomplete data and how you bounded the risk.

### I150

Explain a project outcome honestly, including your individual contribution and limitations.
