# RTC: From First Principles to Expert Engineering Practice

*From a vibrating crystal to time you can trust after a power failure*

This handbook develops the real-time clock as a complete system: physics, electrical design, clock arithmetic, calendar representation, register access, firmware architecture, backup energy, measurement, synchronization, and failure recovery. It covers MCU RTC peripherals and external RTC ICs; a breakout module is a board containing an IC and additional components, not a separate kind of time standard.

Read Chapters 1–12 for foundations, 13–24 for hardware and drivers, 25–36 for precision and system behavior, and 37–45 for verification and expert practice. Each chapter connects mechanism, design decision, and observable failure. The worked numbers are original teaching calculations with stated assumptions, not measured performance of a board.

**Scope and verification.** Device-specific behavior must be checked against the exact part, silicon revision, datasheet, reference manual, and errata. Register transactions below are teaching contracts unless explicitly named otherwise. The accompanying portable C calendar/BCD helpers have host tests; no physical RTC, battery, oscillator, or modem has been tested for this handbook. UTC-like calendar examples use ordinary 00–59 seconds and do not implement leap seconds. A correct calendar conversion is not proof that the physical clock is accurate.

## Contents

- [1. What problem an RTC solves](#1-what-problem-an-rtc-solves)
- [2. RTC, timer, watchdog, and real-time software](#2-rtc-timer-watchdog-and-real-time-software)
- [3. Anatomy of a complete RTC system](#3-anatomy-of-a-complete-rtc-system)
- [4. Frequency, period, phase, and elapsed time](#4-frequency-period-phase-and-elapsed-time)
- [5. Why 32.768 kHz is convenient](#5-why-32768-khz-is-convenient)
- [6. The crystal is a mechanical resonator](#6-the-crystal-is-a-mechanical-resonator)
- [7. Load capacitance and board parasitics](#7-load-capacitance-and-board-parasitics)
- [8. Accuracy, resolution, stability, and jitter](#8-accuracy-resolution-stability-and-jitter)
- [9. Ppm and a practical error budget](#9-ppm-and-a-practical-error-budget)
- [10. Temperature, aging, and mechanical effects](#10-temperature-aging-and-mechanical-effects)
- [11. Calendar arithmetic from first principles](#11-calendar-arithmetic-from-first-principles)
- [12. UTC, local time, epochs, and leap seconds](#12-utc-local-time-epochs-and-leap-seconds)
- [13. Internal RTC or external module](#13-internal-rtc-or-external-module)
- [14. Read the module schematic, not only the IC name](#14-read-the-module-schematic-not-only-the-ic-name)
- [15. I2C wiring and power-state analysis](#15-i2c-wiring-and-power-state-analysis)
- [16. Backup batteries, capacitors, and energy budgets](#16-backup-batteries-capacitors-and-energy-budgets)
- [17. Reset and backup-domain behavior](#17-reset-and-backup-domain-behavior)
- [18. Binary-coded decimal and field validation](#18-binary-coded-decimal-and-field-validation)
- [19. Coherent reads across rollover](#19-coherent-reads-across-rollover)
- [20. Writing time without creating a false success](#20-writing-time-without-creating-a-false-success)
- [21. DS3231 as a concrete external example](#21-ds3231-as-a-concrete-external-example)
- [22. Why DS1307 and PCF8563 are not drop-in firmware substitutes](#22-why-ds1307-and-pcf8563-are-not-drop-in-firmware-substitutes)
- [23. STM32-style RTC initialization and shadow registers](#23-stm32-style-rtc-initialization-and-shadow-registers)
- [24. Status flags, races, and driver contracts](#24-status-flags-races-and-driver-contracts)
- [25. Alarms are comparators, not necessarily event queues](#25-alarms-are-comparators-not-necessarily-event-queues)
- [26. Wakeup timers and low-power sequencing](#26-wakeup-timers-and-low-power-sequencing)
- [27. Subseconds and hardware timestamps](#27-subseconds-and-hardware-timestamps)
- [28. Measuring rate correctly](#28-measuring-rate-correctly)
- [29. Calibration: offset correction versus rate correction](#29-calibration-offset-correction-versus-rate-correction)
- [30. Digital correction and output timing](#30-digital-correction-and-output-timing)
- [31. Synchronization with a network source](#31-synchronization-with-a-network-source)
- [32. GNSS pulse-per-second and the meaning of an edge](#32-gnss-pulse-per-second-and-the-meaning-of-an-edge)
- [33. A time service with explicit quality states](#33-a-time-service-with-explicit-quality-states)
- [34. Clock steps, slews, and monotonic event ordering](#34-clock-steps-slews-and-monotonic-event-ordering)
- [35. Security, tamper, and trustworthy time](#35-security-tamper-and-trustworthy-time)
- [36. Persistence, wear, and power-fail transactions](#36-persistence-wear-and-power-fail-transactions)
- [37. A worked battery-powered logger design](#37-a-worked-battery-powered-logger-design)
- [38. A worked calendar scheduler design](#38-a-worked-calendar-scheduler-design)
- [39. Symptom-first debugging matrix](#39-symptom-first-debugging-matrix)
- [40. Bench experiments from beginner to advanced](#40-bench-experiments-from-beginner-to-advanced)
- [41. Firmware verification strategy](#41-firmware-verification-strategy)
- [42. Interview questions with explained answers](#42-interview-questions-with-explained-answers)
- [43. Worked exercises](#43-worked-exercises)
- [44. Capstone: build a trustworthy time subsystem](#44-capstone-build-a-trustworthy-time-subsystem)
- [45. Final design review checklist](#45-final-design-review-checklist)
- [Appendix A. Portable C helpers and reproducible tests](#appendix-a-portable-c-helpers-and-reproducible-tests)
- [Appendix B. Vocabulary](#appendix-b-vocabulary)
- [Appendix C. Primary references and reading order](#appendix-c-primary-references-and-reading-order)
- [Appendix D. A driver walkthrough using a fictional RTC](#appendix-d-a-driver-walkthrough-using-a-fictional-rtc)
- [Appendix E. Expert measurement and uncertainty analysis](#appendix-e-expert-measurement-and-uncertainty-analysis)
- [Appendix F. Product qualification and fault-injection matrix](#appendix-f-product-qualification-and-fault-injection-matrix)
- [Appendix G. Common designs that look correct but are not](#appendix-g-common-designs-that-look-correct-but-are-not)
- [Appendix H. A zero-to-expert learning path](#appendix-h-a-zero-to-expert-learning-path)

## 1. What problem an RTC solves

A processor can count instructions, and a timer can count clock edges. Neither automatically knows that today is a particular date. To measure time, a system needs a repeating physical process, a counter, an interpretation of the counter, and an initial relationship to an external time reference.

An RTC combines some of these jobs in a power-efficient block. Its oscillator produces cycles; dividers turn cycles into slower ticks; a counter or calendar records progress. A retained power domain lets it continue when the main processor is off. An interface lets firmware read or set it. Alarms can bring the rest of the system back to life.

The RTC does not discover the date by itself. Someone must establish the calendar using a trusted source, a provisioning operation, or a user. Once set, the RTC predicts the passage of time using its local oscillator. Prediction error accumulates until corrected.

Consider a temperature logger that loses its main supply for six hours. A normal RAM variable vanishes. A flash-stored timestamp survives but does not advance. A working battery-backed RTC advances through the outage. On restart, the logger can distinguish a sample taken before the outage from one taken afterward. Whether those dates are accurate still depends on oscillator error and the validity of the initial setting.

## 2. RTC, timer, watchdog, and real-time software

The word *real-time* causes confusion. In scheduling, it concerns meeting deadlines. In an RTC, it usually concerns maintaining calendar time. A device can have an excellent calendar and miss every control deadline.

| Mechanism | Main job | Appropriate example |
| :--- | :--- | :--- |
| Hardware timer | Count ticks, capture edges, generate compares | Motor PWM, a 100 µs measurement |
| Monotonic software clock | Report elapsed time without calendar corrections | Retries, protocol timeouts |
| RTC | Retain approximate civil time and schedule low-power events | Date-stamped logs, overnight wakeup |
| Watchdog | Detect failure to make progress | Reset after a stuck task |
| Network/GNSS time source | Supply an external relationship to time | Set or discipline a local clock |

Never implement a five-second communication timeout by subtracting two adjustable wall-clock dates. A user correction backward can make the timeout much longer; a forward correction can make it expire immediately. Use elapsed ticks from an appropriate monotonic domain. Confirm that domain continues in the intended sleep state.

Conversely, “run at 09:00 tomorrow” is a calendar requirement. Converting it to a timer once and ignoring later timezone or clock changes may violate user intent. Requirements decide which clock to use.

## 3. Anatomy of a complete RTC system

```text
crystal / RC / external reference
                |
       oscillator and divider
                |
      seconds or calendar counters ---- alarm comparators
                |                              |
      snapshot / synchronization           interrupt pin
                |                              |
          register interface ------------ MCU driver
                                               |
backup source --> retained power domain --> time service
                                               |
                                      logs / scheduler / UI

external trusted time ------------------> time service
```

The oscillator, calendar, register interface, output pins, and RAM need not share one power domain. “RTC runs on battery” does not imply “I can communicate with it on battery,” “the square wave stays active,” or “the MCU can wake while its main rail is absent.” Trace each function separately.

A fault can occur at any arrow. A correct oscillator with a wrong divider produces a wrong calendar rate. A correct calendar read incoherently produces a false timestamp. Correct bytes interpreted as binary rather than BCD produce a wrong display. A correct display interpreted in the wrong timezone produces a wrong schedule.

Debug from the physical clock outward instead of changing all layers at once.

## 4. Frequency, period, phase, and elapsed time

Frequency `f` is cycles per second. Period `T = 1/f` is seconds per cycle. At 32,768 Hz, one cycle lasts approximately 30.517578125 µs. Counting 32,768 nominal cycles defines one nominal second.

Two clocks may have the same average frequency but different phase: their second boundaries occur at different instants. A clock can also start at the correct phase and then drift because its frequency differs from the reference. Setting the date fixes an offset at one instant; calibration addresses rate error.

Define displayed clock error as:

```text
e(t) = local_time(t) - reference_time(t)
y    = (f_actual - f_nominal) / f_nominal
```

For approximately constant fractional frequency error `y`, `e(t) = e(0) + y*t`. Positive `y` means a fast clock and increasing positive time error. If temperature changes, integrate the varying rate error: `e(t) = e(0) + integral(y(t) dt)`.

Keep units on the page. A value in ppm is dimensionless after multiplication by `10^-6`; multiplying it by elapsed seconds produces seconds of error. Frequency in hertz and clock error in seconds are related, but they are not interchangeable measurements.

## 5. Why 32.768 kHz is convenient

The useful identity is `32768 = 2^15`. Fifteen divide-by-two stages turn a nominal 32,768 Hz input into 1 Hz. Binary division is simple digital hardware, and this frequency supports low-power crystal designs.

An ideal divider preserves fractional frequency error. If the input is 20 ppm fast, the output is 20 ppm fast. Division reduces absolute frequency error in hertz, but does not turn an inaccurate source into an accurate second.

Some RTCs use two programmable divider stages. A conceptual arrangement has:

```text
f_tick = f_source / ((A + 1) * (S + 1))
```

For a nominal 32,768 Hz source, choosing `A = 127`, `S = 255` gives division by `128*256 = 32768`. These are arithmetic examples, not universal register values. Other architectures use direct divisors, fixed stages, fractional correction, or different source frequencies.

The first divider may trade internal switching power against subsecond resolution. Check what counter a subsecond register exposes; it may count down, not up. Configuring “255” without understanding the extra one is a classic 256/255 rate error.

## 6. The crystal is a mechanical resonator

A quartz crystal converts electrical excitation into mechanical motion and motion back into electrical charge through piezoelectric behavior. Near resonance, its impedance is strongly frequency dependent. An amplifier replaces losses so oscillation can continue.

A useful small-signal model contains a motional series resistance, inductance, and capacitance, in parallel with a shunt capacitance. Motional resistance represents loss. The model explains why a crystal is not simply a capacitor and why an arbitrary “32 kHz crystal” may not work with an arbitrary oscillator circuit.

The oscillator must start from noise and build amplitude. Its available gain must exceed losses with adequate margin across voltage, temperature, component spread, and board leakage. Eventually nonlinear behavior limits amplitude. Too little drive fails to start; too much can violate the crystal’s drive rating or worsen behavior.

Do not choose a crystal using nominal frequency alone. Record load capacitance, maximum ESR, drive limit, tolerance, temperature characteristic, aging, and package requirements. Then verify compatibility with the oscillator manufacturer’s procedure. ST’s [AN2867 oscillator guide](https://www.st.com/resource/en/application_note/an2867-oscillator-design-guide-for-stm8afals-stm32-mcus-and-mpus-stmicroelectronics.pdf) provides a device-oriented design workflow; its component and layout instructions must be applied to the appropriate MCU family.

## 7. Load capacitance and board parasitics

For a simplified two-capacitor load network:

```text
CL ≈ (C1*C2)/(C1+C2) + Cstray
```

Here `Cstray` is an effective equivalent term that includes the relevant pin and PCB contributions in this approximation. Do not add every physical capacitance twice. For equal external capacitors `C`, the equation becomes `CL ≈ C/2 + Cstray`.

**Worked example.** Suppose the selected crystal is specified at `CL = 12.5 pF`, and the engineering estimate of effective stray loading is `2 pF`. The initial equal-capacitor estimate is `C = 2*(12.5-2) = 21 pF`. This is a starting calculation, followed by startup-margin and frequency measurements. It is not permission to install 22 pF on every RTC.

An IC may include fixed or programmable load capacitors. Adding the usual external capacitors can then overload the crystal. A probe adds capacitance too: touching a sensitive oscillator node can change frequency or stop oscillation, making the act of measurement create the fault.

Increasing load commonly lowers the frequency around the intended load-resonance region, but derive adjustment direction from the actual circuit and crystal data. Use a buffered clock output for measurement when possible. Do not calibrate by trial-and-error component changes before identifying a wrong divider or a defective crystal.

## 8. Accuracy, resolution, stability, and jitter

These terms answer different questions:

| Term | Question |
| :--- | :--- |
| Resolution | What is the smallest representable or observable increment? |
| Time error | How far is the timestamp from the reference now? |
| Frequency error | How quickly does time error accumulate? |
| Stability | How does frequency vary over a specified interval? |
| Jitter | How much do individual edges vary around their expected timing? |
| Uncertainty | How well do we know the stated time or measurement? |

A display with six fractional digits can be minutes wrong. A clock with coarse one-second output can have an excellent long-term rate. A stable oscillator can remain consistently wrong until calibrated. NIST’s [stability terminology](https://www.nist.gov/pml/time-and-frequency-division/popular-links/time-frequency-z/time-and-frequency-z-st-sy) distinguishes repeatability of frequency from closeness to its nominal value.

State the measurement interval whenever quoting stability. A clock may look quiet over one second and wander over a day. Average frequency removes some short-term variation but does not reveal every individual output interval.

When choosing hardware, translate the application requirement into allowable error. “Accurate RTC” is not a specification; “within five seconds after seven days without synchronization over the stated temperature range” is.

## 9. Ppm and a practical error budget

One ppm is one part in a million. With constant error:

```text
error_seconds = ppm * 1e-6 * elapsed_seconds
ppm = error_seconds / elapsed_seconds * 1e6
```

| Constant rate error | Error per day | Error per 30-day interval |
| ---: | ---: | ---: |
| 1 ppm | 0.0864 s | 2.592 s |
| 2 ppm | 0.1728 s | 5.184 s |
| 10 ppm | 0.864 s | 25.92 s |
| 20 ppm | 1.728 s | 51.84 s |
| 100 ppm | 8.64 s | 259.2 s |

A bounded engineering budget can be written as `U(t) <= U0 + r*t + Uother`, where `U0` is setting uncertainty, `r` is the bounded residual fractional rate error, and `Uother` contains effects not already included. Do not double-count a temperature contribution already included in a manufacturer's total specification.

**Requirement example.** Allow two seconds after one day. If setting uncertainty consumes 0.25 s and other bounded effects consume 0.10 s, the rate allocation is `1.65/86400 * 1e6 = 19.10 ppm`. A nominal ±20 ppm crystal alone cannot guarantee that whole-system requirement.

Add worst-case bounds when their signs can align. Root-sum-square is appropriate only with justified statistical assumptions. A component's typical current or typical frequency curve is not a guaranteed production bound.

## 10. Temperature, aging, and mechanical effects

A useful illustrative tuning-fork model is `y_ppm(T) = y0 + a*(T-T0)^2`. For a hypothetical `a = -0.035 ppm/°C²`, turnover `T0 = 25 °C`, and `y0 = 0`, operation at `-15 °C` gives `-56 ppm`. That would lose approximately `4.8384 s/day` if held at that temperature. These coefficients are a teaching model, not a part specification.

Calibration at room temperature removes an initial offset; it does not remove a temperature curve. If a cabinet cycles between warm daylight and cold nights, its average error depends on time spent at each temperature. Using the average temperature in a nonlinear formula is generally not equivalent to averaging frequency over the full temperature history.

Aging changes frequency over time. Assembly stress, contamination, shock, and supply sensitivity may add effects. Keep manufacturing calibration and field calibration conceptually separate. A coefficient fitted on one board is not necessarily valid for all boards.

A temperature-compensated oscillator estimates temperature and adjusts its frequency behavior. Residual error, trim quantization, thermal lag, and aging remain. If the temperature sensor measures die temperature, it is not automatically an accurate ambient thermometer for your enclosure.

## 11. Calendar arithmetic from first principles

Seconds roll to minutes at 60; minutes roll to hours at 60; hours roll to a date at 24. Days do not roll at one fixed count: months differ, and February depends on the year. Calendar logic is arithmetic plus rules.

For the Gregorian calendar:

```text
leap = divisible_by_4 AND (NOT divisible_by_100 OR divisible_by_400)
```

Thus 2000 is a leap year, 2024 is a leap year, 2100 is not, and 2400 is. A peripheral that implements only “year divisible by four” needs a limited operating range or firmware correction. A two-digit year does not encode a century by itself.

Weekday is redundant information if year, month, and day are known. Some devices simply increment a user-programmed weekday counter rather than computing it from the date. A wrong initial weekday can remain wrong forever while all dates advance correctly. Establish the convention explicitly: Sunday zero, Monday one, or another device-specific encoding.

Validate the entire combination, not only each byte. April 31 uses individually plausible numbers but is not a valid date. Reject invalid inputs before touching hardware; do not silently reinterpret them as next month's date unless your API explicitly promises normalization.

## 12. UTC, local time, epochs, and leap seconds

Separate storage, ordering, and presentation. Store a documented timescale and epoch; convert to local civil time at the presentation or scheduling boundary. Local time can repeat or skip when civil rules change. An offset such as `+05:30` describes a relationship at an instant; a timezone identifier represents a rule set over time. The [IANA timezone material](https://www.iana.org/time-zones/theory) describes this distinction and the database model.

For an ordinary date conversion, define an epoch such as `2000-01-01 00:00:00` and count days and seconds since it. The C example later deliberately uses this epoch and supports 2000–2399. It is not Unix time. Unix/POSIX-style seconds conventionally use a 1970 epoch and do not count leap seconds as an additional number in the ordinary day arithmetic.

A leap second is not a leap day. An RTC with seconds 00–59 does not by itself implement a UTC leap-second policy. Decide whether the system steps, slews, follows a source's smear, or represents the event explicitly. Mixing smeared and unsmeared sources can produce apparent disagreement even when both behave as designed. Do not hard-code a current TAI–UTC offset into permanent firmware.

Document integer width, signedness, epoch, scale, and supported range in every file format and protocol. “Timestamp: uint32” is incomplete. Signed 32-bit Unix seconds encounter the 2038 boundary; unsigned 32-bit has a different range and boundary. Widening arithmetic after overflow has already happened cannot recover the lost value.

## 13. Internal RTC or external module

An internal RTC saves board space and may connect directly to sleep/wakeup hardware. Its retained supply, oscillator options, register access rules, and reset behavior belong to the MCU architecture. An external RTC has an independent component boundary and often an independent backup source; it adds a bus, pin-level power interactions, and board cost.

| Requirement | Questions to resolve |
| :--- | :--- |
| Main power absent | Which oscillator, calendar, output, and interface remain powered? |
| Long unattended interval | What bounds hold over temperature and aging? |
| Very low energy | What is total backup-domain and board leakage? |
| Alarm wakeup | Does the alarm reach a powered wake source or power controller? |
| Strong integrity | What evidence detects oscillator stop, reset, or tampering? |
| Fast timestamping | Is there hardware capture, subsecond access, or a reference output? |

Avoid selecting by a library name. Start with required holdover, environment, wakeup topology, supply range, service life, and acceptable failure behavior. Then select a compatible implementation.

A separate RTC is not automatically more accurate. An MCU with a carefully designed crystal and calibration may outperform a poor module. A compensated external device may simplify a difficult temperature budget. Both require evidence.

## 14. Read the module schematic, not only the IC name

A module may add an EEPROM, a power LED, I2C pull-ups, a battery holder, charging components, level shifting, and connectors. These additions determine important behavior. The IC datasheet cannot tell you whether the board pulls SDA to 5 V or tries to charge the fitted coin cell.

Before connecting it, identify each rail and component path. Measure resistance only on an unpowered board with the battery removed where appropriate. Trace the pull-up rail, battery path, and connector pin order. Photograph the board revision and record the actual chip marking. Marketplace descriptions are not a substitute for a schematic.

A primary coin cell must not be treated as rechargeable. If a board includes a charging path, use only the intended compatible storage chemistry and specified charging conditions, or use a board designed for your chosen source. Do not infer compatibility from the diameter of the holder.

An extra EEPROM sharing the bus is not calendar storage. Its address, endurance, and retention are separate. An I2C scan finding two addresses does not prove the RTC is counting or the battery is healthy. A power LED may dominate main-supply current while telling you nothing about backup consumption.

## 15. I2C wiring and power-state analysis

I2C SDA and SCL are normally open-drain signals with pull-ups. Their HIGH voltage follows the pull-up supply. Check the RTC and MCU absolute limits, input thresholds, and behavior when either device is unpowered. A 3.3 V MCU attached to a module with 5 V pull-ups is not safe merely because the interface is called I2C.

For an approximate RC rise from 30% to 70%, `tr ≈ 0.8473*Rp*Cb`. With `Rp = 4.7 kΩ` and `Cb = 100 pF`, `tr ≈ 398 ns`. Whether this meets the selected bus mode must be checked against the applicable bus and device limits. Pull-ups in parallel reduce resistance: two 4.7 kΩ pairs act like 2.35 kΩ, increasing LOW-state sink current.

Build a power-state table:

| MCU supply | RTC main supply | Backup | Investigation |
| :--- | :--- | :--- | :--- |
| On | On | Present | Normal transaction and logic levels |
| Off | On | Present | Back-power through SDA, SCL, or interrupt |
| On | Off | Present | Is bus access specified in backup mode? |
| Off | Off | Present | Retention and output leakage |
| On | On | Absent | Initial validity and backup-failure reporting |

Check interrupt and clock-output pull-ups too. A retained output tied to a dead main rail may not produce a useful HIGH. A pull-up on a retained rail may inject current into an unpowered MCU. Solve this electrically rather than relying on initialization code that cannot run while the MCU is off.

## 16. Backup batteries, capacitors, and energy budgets

For a first battery estimate, `life_hours ≈ usable_capacity_mAh / average_current_mA`. Suppose a source has an engineering allocation of 180 mAh usable capacity and measured/bounded average load of 2 µA. Arithmetic gives `90,000 h`, or about 10.3 years using 365-day years. This is not a service-life guarantee: self-discharge, temperature, cutoff voltage, holder reliability, pulse behavior, and chemistry limits remain.

Adding just 8 µA of board leakage raises total current to 10 µA and reduces the same arithmetic estimate to about 2.05 years. A clean chip specification cannot rescue a contaminated or incorrectly wired board.

For an ideal capacitor with approximately constant current, `t = C*(Vstart-Vmin)/I`. A 0.1 F capacitor dropping from 3.3 V to 1.5 V at 1 µA gives 180,000 s, or 50 hours. With another 5 µA of leakage, it falls to roughly 8.33 hours. Real capacitance tolerance, voltage dependence, leakage, charging time, and switching thresholds matter.

Distinguish backup retention current from active bus current, clock-output load, conversion current, and average current under your configuration. Measure over enough time to capture periodic internal activity. A meter's burden voltage can lower the supply and change the result. Preserve voltage at the device while measuring current.

## 17. Reset and backup-domain behavior

A main-core reset, watchdog reset, brownout, backup-domain reset, and complete power removal are different events. They may preserve different combinations of oscillator state, calendar, calibration, alarms, RAM, and status flags.

Never reset the backup domain unconditionally in normal startup code. That can erase valid time on every firmware reboot. Likewise, always writing a build timestamp at boot makes the calendar depend on compilation time and destroys retention.

Use a startup decision tree:

1. Identify the reset cause before software clears it.
2. Read power/oscillator validity evidence without destroying it.
3. Read a coherent calendar and validate its range.
4. Inspect versioned retained metadata if available.
5. Continue in holdover when evidence supports retention.
6. Otherwise mark time invalid and obtain a trustworthy setting.

A magic word in backup RAM proves only that those bits currently match the magic value. It does not prove oscillator operation, valid date, correct rate, or authenticity. Pair it with version, integrity protection appropriate to the threat model, and hardware evidence.

Maintain a reset matrix in the project. For each reset stimulus, record expected surviving state and measured result. Test firmware upgrades and bootloader paths as well as normal application boots.

## 18. Binary-coded decimal and field validation

BCD encodes decimal digits separately. Decimal 59 becomes `0x59`, whose binary integer value is 89. Decode an ordinary two-digit BCD field using `10*(x >> 4) + (x & 0x0F)`, but first validate that each digit is at most nine.

Registers often pack flags next to BCD digits. Extract flags, apply the documented field mask, validate the BCD digits, and then validate the semantic range. For example, a perfectly valid BCD `0x79` is not a valid minute. Do not mask arbitrary bits merely to make bad data look plausible.

The hour register may encode 12-hour or 24-hour mode. In 12-hour mode, 12 AM maps to hour zero and 12 PM maps to hour twelve. A mode bit is not part of the decimal hour. Set one deliberate mode and decode it explicitly; mixing modes causes faults that appear only around noon or midnight.

Validate before exposing data to applications. Preserve raw bytes and status separately for diagnostics, but return an error instead of manufacturing a plausible date. BCD conversion, date validation, and hardware validity are three separate gates.

The accompanying C helpers exercise all 256 possible BCD input bytes, valid decimal round trips, calendar boundaries, and invalid inputs. They operate on already-extracted fields and do not know a chip's register masks.

## 19. Coherent reads across rollover

Imagine reading seconds at 23:59:59, then reading the date after midnight. You can construct tomorrow's date with yesterday's time. Every individual register read succeeded, yet the timestamp never existed.

Common hardware solutions include snapshot-on-read, shadow registers, a latch command, or a freeze/read/release mechanism. Use the device's documented mechanism and transaction shape. A multi-byte bus transfer is not automatically a hardware snapshot.

Where no snapshot exists, a bounded retry can sometimes work: read a boundary field, read the rest, then reread the boundary field and retry if it changed. This is valid only under stated assumptions: the whole operation is shorter than the boundary period, no concurrent writer changes time, and the device's update behavior supports the technique. Seconds-before/seconds-after equality cannot detect a stall of exactly one minute. Place a transaction-time bound around the operation.

Use one driver owner to serialize reads, writes, calibration, and alarm updates. A coherent hardware read can still race against a firmware time-setting operation. Return the sample with monotonic acquisition bounds so callers know when it was observed.

For edge-sensitive work, do not infer submillisecond timing from when a task finishes printing a date. Capture an electrical edge with timer hardware or use an explicit device timestamp feature.

## 20. Writing time without creating a false success

A calendar update involves more than copying bytes. The device may require unlocking, entering initialization mode, stopping a divider, waiting for synchronization, or writing fields in a particular sequence. Writes can partially succeed when power fails or the bus errors.

A generic transaction contract is:

```text
validate proposed date, range, scale, source, and authority
lock the RTC service against competing operations
mark the update as pending in software
enter the documented write state with a bounded timeout
write the complete intended calendar using the required sequence
leave initialization state and wait for required synchronization
read back coherently and compare allowing documented tick progress
clear validity faults only when justified and with correct flag semantics
publish a new time generation and correction event
unlock
```

This is pseudocode, not a device driver. Implement cleanup for every failure path. An error after the first register write should leave the time service invalid or suspect, even if the bus recovers. Do not return success because the final STOP condition succeeded.

Setting phase precisely needs a defined effective instant. “Write these seconds” may restart a divider on one register write or apply at a later boundary. If the application tolerates ±1 s, document that. If it needs ±1 ms, characterize and control the setting path instead of hoping I2C latency is negligible.

## 21. DS3231 as a concrete external example

The DS3231 integrates a crystal and temperature compensation, uses I2C, has battery backup, two alarms, and an aging adjustment. Its published accuracy is ±2 ppm from 0 to 40 °C and ±3.5 ppm over the stated industrial range; apply the full datasheet conditions. It uses seven-bit address `0x68`. Calendar registers occupy `0x00–0x06`; control and status are `0x0E` and `0x0F`. The status OSF bit is evidence that oscillation stopped, not a general “date correct” certificate. Calendar reads use secondary buffers; follow the documented START/buffer behavior. Alarm output is open-drain. Source: [DS3231 datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/DS3231.pdf), address map, timekeeping, control/status, and electrical sections.

An engineering adapter around this part should expose `read_snapshot`, `set_time`, `read_health`, and `configure_alarm` operations. Keep register details inside that adapter. Tests should inject NACKs, invalid BCD, rollover, and interrupted writes. The module's pull-ups and battery circuit remain independent design responsibilities.

For the ordinary seven-bit address convention, `0x68` becomes wire address bytes `0xD0` for write and `0xD1` for read. Some MCU libraries accept seven-bit addresses and some expect shifted values. Check the API before shifting; double shifting often produces a perfectly formed transaction to the wrong device.

## 22. Why DS1307 and PCF8563 are not drop-in firmware substitutes

The DS1307 provides calendar time, battery-backed RAM, and a square-wave output; it depends on an external crystal. The [manufacturer's product documentation](https://www.analog.com/en/products/ds1307.html) identifies its own power and interface requirements. A familiar connector or shared address does not establish DS3231-compatible electrical or register behavior.

The PCF8563 uses an external 32.768 kHz crystal, provides voltage-low detection, and has different registers and alarm behavior. Its seven-bit I2C address is `0x51`; the datasheet also lists the corresponding write/read bytes `0xA2`/`0xA3`. Its supply and bus operating conditions are distinct. See the [PCF8563 datasheet](https://www.nxp.com/docs/en/data-sheet/PCF8563.pdf).

When porting, make a capabilities table rather than changing one address constant. Include calendar format, supported year range, validity flags, snapshot semantics, alarm precision, backup behavior, output electrical type, and write restrictions. An alarm library that assumes seconds matching may silently schedule the wrong event on a device whose alarm lacks a seconds field.

A portable application can depend on a capability-aware time service. A portable raw-register driver across unrelated RTCs is usually the wrong abstraction.

## 23. STM32-style RTC initialization and shadow registers

STM32 families differ; use the matching manual and errata. The family overview [AN4759](https://www.st.com/resource/en/application_note/an4759-introduction-to-using-the-hardware-realtime-clock-rtc-and-the-tamper-management-unit-tamp-with-stm32-mcus-stmicroelectronics.pdf) describes calendar, alarms, wakeup, calibration, timestamp, and tamper features across implementations. It is a navigation aid, not one universal register recipe.

For the STM32G0 shadow-register example, reading SSR or TR locks higher-order calendar shadow values until DR is read. A read sequence must finish by reading the date register even if the application only wants the time. Shadow bypass uses different consistency handling. See [STM32G0 RTC training](https://www.st.com/resource/en/product_training/STM32G0-WDG_TIMERS-Real-Time-Clock-RTC.pdf). Other families and revisions require their own timing and errata checks.

A portable initialization design should distinguish first provisioning from normal boot. Board support enables permitted access, selects the intended retained source, waits for readiness with a timeout, and configures only what requires change. It must avoid destructive backup reset on a normal restart.

The application's response to oscillator-start failure matters: record a diagnostic, degrade time quality, and continue or stop according to requirements. An infinite wait on an oscillator-ready flag can make a dead crystal look like a dead processor.

## 24. Status flags, races, and driver contracts

Flags may be read-only, write-one-to-clear, write-zero-to-clear, clear-on-read, or mixed with ordinary control bits. A read-modify-write that is safe for RAM may lose an interrupt event in a hardware register.

Suppose software reads a status byte, hardware sets a second event, then software writes the old byte back. Depending on semantics, the new event may be erased. Use the manufacturer's prescribed operation and represent flag handling explicitly in the driver. Do not create a generic `register &= ~flag` helper and apply it everywhere.

Every bus operation needs a result, deadline, and recovery policy. Distinguish no response, arbitration loss, timeout, invalid payload, and invalid time. Returning a zeroed calendar on every error makes a transport failure appear to be a real date.

Suggested application-facing results are `VALID`, `UNSET`, `OSCILLATOR_FAULT`, `OUT_OF_RANGE`, `BUS_ERROR`, and `UPDATE_INCOMPLETE`, with raw diagnostic detail retained separately. Avoid making applications decode silicon flags.

Keep interrupt handlers small: capture cause or enqueue work, then service the device in the permitted context. Blocking I2C inside an ISR can deadlock if that bus driver relies on interrupts or a mutex owned by an interrupted task.

## 25. Alarms are comparators, not necessarily event queues

A hardware alarm usually compares selected calendar fields against programmed values. Masked fields act as “don't care.” Matching seconds and minutes alone can recur every hour; a date-of-month alarm may have no month or year field. Writing the current matching time may create an immediate event depending on device behavior.

To implement a one-shot deadline, retain the full intended deadline in software, program the nearest supported hardware match, and validate the full date when awakened. Disable or rearm deliberately after handling. Check missed-deadline behavior if the MCU was unavailable at the intended instant.

Calendar recurrence and elapsed recurrence differ. “Every 24 hours” follows elapsed time; “every day at 08:00 local time” follows civil rules. Around a timezone transition they may not describe the same interval. Specify what to do when a local time is repeated or nonexistent.

Alarms often latch a flag and hold a pin asserted until cleared. With a level-sensitive wake source, failing to clear the source can cause an immediate wake loop. Clearing only the MCU pending bit while the external RTC still drives LOW does not remove the cause.

Test at the exact matching boundary, just before it, after a missed match, and after changing the calendar backward. A single noon demonstration cannot establish correct scheduling.

## 26. Wakeup timers and low-power sequencing

A wakeup timer usually counts a selected clock for a relative interval. The effective divisor, reload width, and `+1` convention determine range and resolution. Do the arithmetic from the reference manual; similar-looking peripherals can use different formulas.

Low-power entry is a race: an event can arrive after software checks a flag but before the sleep instruction. Use the architecture/RTOS-supported atomic wait pattern. “Clear everything then sleep” can erase the event you intended to handle.

Before entering sleep, configure the source, clear only stale state with the documented ordering, enable the wake route, verify the future deadline, and enter sleep using the supported synchronization sequence. On return, determine the actual wake cause; a debug connection, GPIO, watchdog, or brownout may have ended sleep instead of the RTC.

An RTC interrupt cannot directly power an unpowered MCU unless a powered circuit receives it and enables the rail. For full power gating, design a power-controller/latch path and verify pulse or level duration. The RTC pin's current rating is not a power-switch rating.

Measure both wake timing and total cycle energy. Saving 2 µA during sleep may not help if a faulty alarm wakes the processor continuously.

## 27. Subseconds and hardware timestamps

A calendar may advance in seconds while an internal prescaler exposes finer phase. If a downcounter starts at `P` and decrements to zero, one common interpretation is `(P - current)/(P+1)` seconds. This expression is architecture-dependent: confirm which stage, update instant, and calibration effects the register represents.

Read subsecond, time, and date coherently. Rollover between fields can create nearly one second of error. A high-resolution subsecond field attached to yesterday's date is still wrong.

For external events, hardware capture records a timer value at the input edge with bounded hardware behavior. An ISR records a later instant affected by masking, priority, and software latency. Pair a fast monotonic timer with a calendar anchor when microsecond event ordering is needed.

Store `{capture_ticks, clock_generation, anchor_id}` so later conversion can use the correct calibration and offset. When wall time is corrected, preserve the original event order. A timestamping system should expose what instant was captured: connector edge, synchronized input, ISR entry, or completed transaction.

If two devices timestamp the same event, include their independent clock and capture uncertainties before interpreting a small difference as a physical delay.

## 28. Measuring rate correctly

Measure against a reference whose uncertainty is small enough for the question. A low-cost frequency counter with an uncalibrated timebase may report many digits while being less accurate than the RTC. NIST's [frequency measurement overview](https://www.nist.gov/publications/frequency-measurement) is a useful guide to the distinction between instrumentation resolution and measurement quality.

A direct counter counts output edges during a reference gate. At 32,768 Hz, a one-second gate's one-count granularity corresponds to about 30.52 ppm. A 100-second gate reduces that granularity to about 0.305 ppm, before timebase and trigger errors. Reciprocal counting measures elapsed reference ticks across many RTC cycles and can provide finer estimates.

Alternatively, measure clock phase against a reference at two instants. If the local clock gains 0.432 seconds over 86,400 seconds, its measured average error is +5 ppm. Record temperature, supply, output configuration, calibration setting, and elapsed duration with the result.

Reading whole calendar seconds at arbitrary instants produces large quantization uncertainty. To resolve ppm quickly, compare edges rather than human displays. Avoid calibrating from one photograph of two clocks taken on different refresh cycles.

## 29. Calibration: offset correction versus rate correction

Offset correction changes what time the clock reports now. Rate correction changes how quickly it advances. A clock that is ten minutes wrong but runs at the correct rate needs setting, not frequency trim.

Estimate rate from multiple phase observations. A simple linear model is `e(t) = b + m*t`; `b` is phase offset and `m*1e6` is ppm. Inspect residuals. Curvature suggests temperature or aging; sudden steps suggest synchronization, reset, missed capture, or a data error.

If the uncorrected clock is +8 ppm fast and your correction mechanism changes effective rate, the intended rate correction is approximately -8 ppm. Hardware trim sign may be opposite to register sign; verify before applying. After adjustment, measure again over an adequate interval.

Store calibration provenance: board serial, date, method, reference, temperature range, raw observations, fitted coefficient, uncertainty, and firmware version. A bare number in EEPROM cannot explain whether the adjustment remains valid.

Do not trim away one cold-night observation and expect room-temperature improvement. Either characterize a model over the operating range or use an oscillator whose specified residual behavior satisfies the requirement. Calibration cannot repair intermittent oscillator stoppage.

## 30. Digital correction and output timing

Some calibration hardware changes oscillator loading; other designs periodically add or suppress clock pulses. A pulse-based correction changes average rate in discrete increments.

If one pulse is suppressed in a window of `N` source pulses, the approximate rate adjustment magnitude is `1/N`, or `1e6/N ppm`. With `N = 2^20`, one pulse represents approximately 0.953674 ppm. This is a generic derivation, not a register encoding for a particular device.

Average accuracy does not guarantee identical individual second intervals. Periodic correction can modulate interval length. An output derived before the correction point may retain the original rate while the calendar is corrected; another output may show the correction pattern. Draw the clock tree and identify the measurement point.

If a downstream circuit uses the RTC output as a sampling clock, analyze jitter and modulation as well as average ppm. A suitable calendar calibration scheme may be unsuitable for a uniform acquisition clock.

Changing calibration may require waiting for a boundary or completion flag. Serialize adjustments and account for settling. Repeatedly writing a trim in a fast control loop can prevent the intended correction cycle from completing.

## 31. Synchronization with a network source

Receiving a timestamp does not tell you exactly when it applied. Transmission, queueing, reception, and processing add delay. Network synchronization estimates both offset and delay rather than treating packet arrival as the server's stated instant.

For four NTP-style timestamps—client send `T1`, server receive `T2`, server send `T3`, client receive `T4`—the familiar estimates are:

```text
offset = ((T2-T1) + (T3-T4))/2
delay  = (T4-T1) - (T3-T2)
```

These equations and their protocol context are specified in [RFC 5905](https://www.rfc-editor.org/rfc/rfc5905.html). Path asymmetry biases the estimate; multiple good samples and source validation matter. These equations alone are not an NTP client implementation.

**Arithmetic example.** If forward travel is 20 ms, reverse travel is 60 ms, and the true server-minus-client offset is 100 ms, the two directional observations are 120 ms and 40 ms. Their half-sum estimates 80 ms, twenty milliseconds below the true offset. Symmetry is an assumption with a cost.

Keep packet validation, source selection, clock discipline, and RTC persistence separate. Synchronize the software time service; update the hardware RTC according to a controlled policy rather than rewriting it after every noisy packet.

## 32. GNSS pulse-per-second and the meaning of an edge

A pulse-per-second signal provides recurring timing edges. It does not by itself identify the date or which integer second an edge represents. The associated navigation message supplies that label, often with a receiver-specific relation between message and pulse.

Read the receiver timing documentation: validity flags, startup behavior, time scale, antenna delays, cable delay, output polarity, and whether the message labels the preceding or following edge. Capture PPS in hardware when precision matters, then associate it with validated time messages.

Use multiple intervals to estimate local rate. A missing or duplicate pulse should not silently shift the calendar by a second. Monitor interval bounds and source quality; enter holdover when the source is unavailable or suspect.

GNSS availability is not authenticity. A safety- or security-relevant system needs a threat model for interference, spoofing, antenna faults, and source disagreement. Agreement between two receivers with a common vulnerability is weaker evidence than genuinely independent references.

Do not advertise nanosecond timestamp accuracy merely because a receiver specification mentions a precise PPS output. Board capture, cable, association logic, and the application's timestamp point add their own uncertainties.

## 33. A time service with explicit quality states

Applications need more than a number. A useful reading includes a calendar/epoch value, acquisition instant, estimated or bounded uncertainty, source, last synchronization, and quality state.

```text
UNSET --> SYNCHRONIZED --> HOLDOVER --> EXPIRED
  ^            |              |          |
  |            +-------- SUSPECT <---------+
  +------------- invalid hardware --------+
```

These are proposed software states, not RTC register values. Define transitions precisely. `SYNCHRONIZED` means accepted source evidence within policy; `HOLDOVER` means advancing locally since that evidence; `EXPIRED` means the uncertainty budget is no longer sufficient. A hardware oscillator fault invalidates assumptions even if the displayed year looks reasonable.

For a 0.1 s starting bound and 5 ppm residual rate bound, holdover uncertainty reaches 1 s after `(1-0.1)/(5e-6) = 180,000 s`, or 50 hours, ignoring separately budgeted effects. If the requirement is one second, this calculation determines a synchronization deadline.

Expose failure explicitly. A logger may continue collecting with monotonic timestamps and `time_valid=false`, then associate them with a later anchor if the continuity assumptions hold. That is more honest than labeling every boot with an invented date.

## 34. Clock steps, slews, and monotonic event ordering

A step changes wall time immediately. A slew temporarily changes its rate until the desired offset is absorbed. A large step can make event dates go backward; a slow slew can take a long time.

If a correction is 100 ms and the allowed additional rate is 100 ppm, absorption takes approximately `0.1/100e-6 = 1000 s`. A one-hour correction would take far longer. Set policy by magnitude and application phase: boot before logging may permit a step, while an active ordered event stream may require explicit correction records.

Maintain a monotonic event sequence independent of wall time. Store a boot/session identifier, monotonically increasing sample index, monotonic tick count, and wall-clock generation. On correction, write a record linking the old and new mappings.

Never silently rewrite historical observations to hide a correction. If post-processing produces corrected times, preserve originals and record the transformation. This allows investigators to distinguish a physical gap from a clock adjustment.

Scheduling after a step needs a policy for deadlines crossed forward or backward. Execute overdue jobs once, skip them, or request operator review according to the product requirement; do not let accidental comparator behavior decide.

## 35. Security, tamper, and trustworthy time

An ordinary battery-backed RTC is not a cryptographic authority. Physical access may permit changing the calendar, removing the battery, injecting clock edges, or altering the bus. A valid CRC detects accidental corruption, not a deliberate attacker who can recompute it.

Separate authenticity of a time source from accuracy and freshness. A signed old timestamp can be authentic but stale. A network packet can be timely but forged. [RFC 8915](https://www.rfc-editor.org/rfc/rfc8915.html) specifies Network Time Security for NTP using authenticated mechanisms; deploying a correct implementation still requires source and delay policies.

For security decisions, define allowed backward movement, accepted sources, authorized setting operations, replay protection, and behavior when no trusted time is available. Record changes with source identity and reason. Protect persistent monotonic state if the threat model requires rollback resistance.

Tamper inputs can provide evidence of an event; they do not prove every kind of manipulation was detected. Erasing retained data may be the intended response on some devices and the wrong response for forensic logging on others. Configure it from requirements rather than enabling every security-looking bit.

Certificate validation creates a bootstrap challenge when time is unknown. Solve it using the system's explicit provisioning and trust design, not by permanently disabling certificate checks.

## 36. Persistence, wear, and power-fail transactions

RTC registers, backup SRAM, EEPROM, and flash have different retention and endurance properties. Saving time to flash every second consumes writes and does not make the saved value advance during an outage.

Persist information that must survive: last accepted synchronization, quality, calibration, timezone configuration, and schema version. Use a deliberate write frequency and transactional format. A two-slot record can contain a sequence, payload, integrity check, and final commit marker; write the inactive slot and make validity visible last.

On boot, validate both slots and choose the newest valid sequence with a documented wrap rule. An interrupted update should leave the previous record usable. Test power loss at every write boundary in a model, then test real hardware interruption; flash/EEPROM programming behavior cannot be established by RAM tests alone.

Distinguish “last known wall time” from “current wall time.” A saved timestamp supplies at most a lower-bound clue if monotonic progression and anti-rollback assumptions hold. Without an advancing retained counter, the outage duration is unknown.

When a firmware update changes timestamp format, perform versioned migration. A new epoch interpreted using old bytes can produce plausible but catastrophically wrong dates.

## 37. A worked battery-powered logger design

Assume a logger samples every ten elapsed minutes, labels samples with UTC-like dates, and must stay within ±5 s over seven days without synchronization. It has a 0.5 s initial setting bound and allocates another 0.5 s to capture/conversion effects.

The rate allocation is `(5-0.5-0.5)/604800 * 1e6 ≈ 6.614 ppm`. Hardware selection must meet that residual bound across the required environment, including effects not already included in the relevant specification. A component chosen only for ±20 ppm room-temperature tolerance is insufficient.

Use a monotonic scheduler for the sampling interval and a retained RTC for reboot recovery. Keep a software anchor linking monotonic ticks to the current wall-clock generation. Each sample stores sequence, monotonic tick, estimated wall time, and quality. If the RTC fails validation, continue sampling with invalid wall time rather than inventing dates.

Wakeup design checks include alarm precision, MCU wake routing, time to stabilize the sensor, sampling latency, and the energy of synchronization attempts. A retry storm after network loss can dominate the energy budget; back off while uncertainty grows according to policy.

Acceptance criteria include seven-day holdover characterization, temperature excursions, removal of main power, backup-low behavior, and alarm recovery after a deliberately missed wakeup. The arithmetic selects a design direction; those measurements establish the actual product evidence.

## 38. A worked calendar scheduler design

Assume a lighting controller must turn on at 18:30 local time each day, survive outages, and avoid duplicate activation after a clock correction. This is civil scheduling, not “every 86,400 elapsed seconds.”

Store the rule as local civil time plus timezone identifier and recurrence policy. Resolve the next occurrence to the storage timescale using the selected rule version. Keep a durable occurrence identifier so reboot or a backward correction cannot execute the same event twice accidentally.

Define outage behavior: if power returns at 18:35, should the light turn on, wait until tomorrow, or restore a computed desired state? For a stateful actuator, recomputing the desired state at the current valid time can be more robust than replaying every missed transition.

If time becomes invalid, apply a product-specific safe state and communicate uncertainty. The time library must not independently decide whether a lighting, access, heating, or industrial load should activate.

Test repeated local hours, nonexistent local times, timezone configuration changes, a forward step across the deadline, and a backward step after execution. Date arithmetic is only one part of the behavior specification.

## 39. Symptom-first debugging matrix

| Symptom | Leading hypotheses | Next discriminating observation |
| :--- | :--- | :--- |
| Clock loses time only unplugged | Backup source, oscillator control, leakage | Measure backup voltage/current and fault flags during transition |
| Time resets on every reboot | Unconditional initialization or backup reset | Trace startup register writes and reset causes |
| Minutes display as 89 | BCD treated as binary | Capture raw minute byte and decoder input |
| Wrong date only near midnight | Incoherent snapshot or concurrent writer | Force rollover under repeated reads |
| Error grows linearly | Constant rate error or divider mismatch | Plot phase versus elapsed reference time |
| Error changes with temperature | Oscillator characteristic or compensation issue | Record phase and temperature together |
| Alarm wakes continuously | Latched source not cleared or repeated match | Observe interrupt level and both RTC/MCU flags |
| Bus works only with main power | Access not supported in backup mode | Compare power state against datasheet conditions |
| Battery drains rapidly | Pull-up path, charging circuit, contamination | Isolate current paths and measure over full cycles |
| Clock freezes after reads | Snapshot/shadow latch not released | Audit required final register read |
| Exactly one-hour display error | Timezone/DST policy | Compare stored scale to presentation conversion |
| Exactly one-second boundary error | Setting phase or PPS association | Capture edge and associated message timing |
| Valid date but questionable time | Lost trust evidence or stale source | Inspect quality state, last sync, and uncertainty |

Change one variable at a time. Save raw data before clearing flags. A diagnostic log should include firmware version, reset cause, register bytes, supply state, source selection, and acquisition timing, while avoiding unrelated private data.

## 40. Bench experiments from beginner to advanced

**Experiment A: retention.** Set a known date, record fault flags, remove main power for a measured interval, restore it, and compare progress. Repeat with backup absent. Acceptance is not just a plausible display: verify elapsed progress and expected validity state.

**Experiment B: rollover.** Set a disposable test calendar just before midnight, month end, February end, and year end. Read repeatedly and reject impossible combinations. Use the exact supported hardware year range; test wider Gregorian logic separately on the host.

**Experiment C: alarm lifecycle.** Observe the interrupt pin on a logic analyzer. Program a near-future match, record assertion, service it, and record release. Repeat with the MCU asleep and with delayed service. Verify recurrence and missed-event policy.

**Experiment D: rate.** Capture a buffered RTC output and a suitable reference over an interval long enough to resolve the required ppm. Record configuration and temperature. Repeat after a controlled calibration change to establish direction and magnitude.

**Experiment E: power.** Measure main and backup currents with the bus active, idle, and main supply absent. Include the module, pull-ups, and attached MCU. Watch periodic current activity and verify measurement burden.

**Experiment F: fault injection.** Interrupt I2C at successive bytes, hold the bus in a fault state using appropriate test hardware, and interrupt power during a calendar update. Confirm bounded recovery and honest invalid-state reporting.

**Experiment G: holdover.** Synchronize, disconnect the source, and log phase drift across the environmental range. Compare the measured envelope with the promised uncertainty growth. Do not use the clock under test as its own reference.

## 41. Firmware verification strategy

Separate pure computation from hardware behavior. Host tests can establish BCD rejection, calendar boundaries, epoch round trips, scheduler policies, and state-machine transitions. They cannot establish oscillator startup or backup retention current.

Use a fake register transport that can fail on any operation. Test cleanup when unlock succeeds but write fails, when one calendar byte is changed, when readback is inconsistent, and when a second client attempts to modify time concurrently. Assert the quality state and externally visible result, not only the number of bus calls.

Use model/property checks across a broad date range. Round-trip conversion alone is insufficient because two inverse functions can share the same bug. Compare against an independent calendar implementation and known epoch anchors. Test 2000, 2100, 2200, 2300, and the upper boundary of your chosen range.

Test wraparound in timers and sequence counters. Unsigned subtraction works for bounded elapsed intervals under explicit assumptions; a 32-bit millisecond counter wraps after about 49.71 days. If more than a full wrap can pass without observation, subtraction alone cannot recover the number of wraps.

On the target, test compiler optimization, actual register timing, low-power transitions, interrupt priorities, and power failures. Keep the host evidence and bench evidence labeled separately in the release report.

## 42. Interview questions with explained answers

**Why can a battery-backed clock still show the wrong time?** Retention keeps state advancing; it does not guarantee correct initialization, oscillator rate, calendar interpretation, or uninterrupted oscillation. Inspect evidence for each.

**Does a 1 Hz output mean one accurate second?** It means the device divides its source to a nominal rate. Fractional source error survives division. The output may also be before or after calibration.

**Why read the date when I only need hours and minutes?** Some shadow mechanisms release their latch only after the date register is read. The answer depends on the device; obey the documented read contract.

**Is a stopped-oscillator flag equivalent to invalid BCD?** No. A stopped clock can contain perfectly valid BCD representing a stale date. Syntax, calendar validity, and time trust are separate checks.

**Can I fix a slow clock by adding one second every day?** That can reduce average displayed error under stable conditions, but introduces steps, may miss the true error rate, and can disturb scheduling. Measure and use an appropriate discipline/calibration policy.

**Why does time jump backward after a network update?** The local clock was ahead or the new source is wrong. Preserve monotonic ordering and record the correction; validate the source before changing wall time.

**Can an I2C scan prove an RTC module is good?** It proves an address acknowledged under that test condition. It does not test calendar progress, oscillator validity, backup, alarms, or accuracy.

**What is the difference between precision and correctness in timestamping?** Fine resolution or repeatable captures can coexist with a large offset or wrong epoch. Establish both measurement repeatability and reference relationship.

**Why not use local time inside the RTC?** It can work in a constrained product with a defined policy, but repeated/skipped civil times and rule updates complicate ordering and scheduling. A documented universal storage scale with separate presentation is usually easier to reason about.

**Does temperature compensation eliminate calibration?** It addresses a major variation, but residual offset, aging, board effects, and system setting errors remain. Requirements determine whether additional calibration is needed.

## 43. Worked exercises

### Exercise 1: month-long holdover

A clock is +12 ppm fast for 30 days, initially correct. Error is `12e-6*30*86400 = 31.104 s` ahead. If initial error was -2 s, final error is +29.104 s under the constant-rate assumption.

### Exercise 2: derive ppm from observation

The local clock loses 3.6 s over 72 hours. Rate estimate is `-3.6/(72*3600)*1e6 = -13.8889 ppm`. The negative sign matters: a compensating effective-rate adjustment should speed it up.

### Exercise 3: choose a synchronization interval

Allowed uncertainty is 0.5 s, initial bound is 0.08 s, residual rate bound is 7 ppm. Ignoring other effects, maximum interval is `0.42/7e-6 = 60,000 s`, or 16 h 40 min. Add operational margin for unavailable synchronization attempts.

### Exercise 4: backup leakage

Usable capacity is 120 mAh. RTC load is 0.8 µA and other leakage is 1.2 µA. Total is 2 µA, giving 60,000 h by capacity arithmetic. Ignoring the extra leakage would predict 150,000 h—2.5 times too optimistic.

### Exercise 5: BCD and flags

A hypothetical register uses bit 7 as a status flag and bits 6–0 as BCD seconds. Raw `0xA5` has flag one and masked seconds `0x25`, or 25. Raw `0x6A` has an invalid digit; raw `0x79` has valid digits but an invalid seconds range. The real device determines whether bit 7 has this meaning.

### Exercise 6: leap boundaries

The day after 2099-12-31 is 2100-01-01. The day after 2100-02-28 is 2100-03-01. The day after 2000-02-28 is 2000-02-29. A `%4` rule passes the last example and fails the middle one.

### Exercise 7: capture quantization

A timer runs nominally at 1 MHz. One tick is 1 µs. If two independent edge captures each have up to one tick of quantization uncertainty, their difference can contain up to two ticks of worst-case error, before oscillator and input-path effects. Do not claim 1 µs absolute accuracy solely from the timer frequency.

### Exercise 8: calibration resolution

A hypothetical correction removes one pulse per 524,288 pulses. Step size is about 1.90735 ppm. Nearest-step quantization alone can leave roughly ±0.953674 ppm residual error, assuming the correction range covers the needed value and the model is otherwise exact.

### Exercise 9: acquisition uncertainty

A calendar read begins at monotonic 100.000 s and completes at 100.004 s. Without a more precise latch contract, associating it with the midpoint incurs up to 2 ms acquisition-window uncertainty, plus calendar quantization and clock uncertainty. The successful bus status does not remove these terms.

### Exercise 10: a misleading battery test

A board retains time overnight but fails after a week. One night proves some retention under those conditions. It does not establish source capacity, low-voltage operation, temperature behavior, or long-term leakage. Measure current and voltage, then repeat over a justified interval and environment.

## 44. Capstone: build a trustworthy time subsystem

Implement a time service for a low-power logger with an external or internal RTC. Support valid/invalid time, coherent reads, bounded bus operations, explicit setting authority, calibration metadata, one-shot wakeup, monotonic sample order, and holdover uncertainty.

Deliver the schematic power-state analysis, driver contracts, supported year range, source policy, test suite, and measurement plan. Include a trace showing normal boot preserving time, failed initialization marking it invalid, a backward correction preserving event order, and an alarm that wakes once without looping.

Score the work across five categories:

| Category | Evidence required |
| :--- | :--- |
| Electrical design | Rail/pull-up limits and backup-current budget |
| Driver correctness | Snapshot/write semantics, timeouts, fault handling |
| Calendar correctness | Independent oracle, invalid dates, century tests |
| System behavior | Quality states, correction records, missed-event policy |
| Measurement | Reference uncertainty, rate/retention results or clearly pending bench plan |

An expert submission explains what remains unknown. A demonstration that prints a date is the starting point. Completion means each promised behavior has a requirement, implementation, test, and stated limit.

## 45. Final design review checklist

- The time scale, epoch, supported years, and leap-second policy are written down.
- The selected oscillator meets startup, load, drive, and environmental requirements.
- Total backup current includes the board and all pin paths.
- Primary and rechargeable storage sources are not confused.
- Normal boot preserves valid retained time and avoids destructive reset.
- Hardware validity evidence is read before it is cleared.
- Reads are coherent and writes have bounded failure cleanup.
- BCD, 12-hour mode, weekday convention, and century handling are explicit.
- Alarm recurrence and missed-deadline behavior match the application.
- The wake route remains powered in the selected sleep/power state.
- Monotonic time handles durations; wall time handles calendar interpretation.
- Calibration has units, sign, provenance, and uncertainty.
- Network/PPS sources have validity, freshness, and association checks.
- Logs preserve ordering across reboot and time correction.
- Host tests and bench evidence are separately reported.
- The product communicates invalid or expired time rather than inventing certainty.

## Appendix A. Portable C helpers and reproducible tests

The companion files are in [rtc-examples](rtc-examples/README.md). `rtc_math.h` implements checked BCD conversion, Gregorian date validation, and seconds-since-2000 conversion for 2000–2399. The example deliberately excludes timezone conversion and leap seconds. It uses `uint64_t` epoch seconds; embedded toolchains must provide the fixed-width types used.

The host executable enumerates every valid day in the supported range. A Python standard-library `datetime` oracle independently checks the day-to-epoch mapping. Tests also reject invalid digits/dates, verify known anchors and the upper range, and exercise failed conversions without modifying the caller's result.

Peripheral register operations in this handbook remain pseudocode/contracts. These helpers do not constitute a DS3231, PCF8563, or STM32 driver. Their passing tests establish only the pure computational behavior exercised.

## Appendix B. Vocabulary

| Term | Meaning in this handbook |
| :--- | :--- |
| RTC | A retained clock/counter/calendar facility |
| TCXO | Temperature-compensated crystal oscillator |
| LSE/LSI | Common MCU terminology for external/internal low-speed clock sources; verify family definitions |
| BCD | Decimal digits encoded in separate binary nibbles |
| Epoch | Origin from which a numeric time count is measured |
| Holdover | Local timekeeping after losing an accepted reference |
| Phase error | Time displacement relative to a reference |
| Rate error | Fractional frequency difference driving accumulated drift |
| Slew | Gradual offset correction by temporarily changing effective rate |
| Snapshot | A coherent view of fields corresponding to one calendar state |
| Backup domain | Circuitry supplied/retained independently of the main domain |
| PPS | Pulse per second; an edge reference requiring a time label |
| OSF/VL | Device-specific oscillator-stop/voltage-low evidence, not interchangeable generic flags |
| Calibration | Estimation/application of corrections with a stated model and uncertainty |

## Appendix C. Primary references and reading order

Use the current document for the exact part and retain its revision in the project record. Links were researched for this edition; manufacturer URLs may later serve newer revisions.

1. [Analog Devices DS3231 datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/DS3231.pdf): the concrete integrated-crystal device example; read electrical conditions and register semantics together.
2. [Analog Devices DS1307 documentation](https://www.analog.com/en/products/ds1307.html): an external-crystal RTC architecture and its own interface/power conditions.
3. [NXP PCF8563 datasheet](https://www.nxp.com/docs/en/data-sheet/PCF8563.pdf): a different register map and validity/alarm model.
4. [ST AN2867 oscillator guide](https://www.st.com/resource/en/application_note/an2867-oscillator-design-guide-for-stm8afals-stm32-mcus-and-mpus-stmicroelectronics.pdf): oscillator component selection and implementation guidance.
5. [ST AN4759 RTC and tamper overview](https://www.st.com/resource/en/application_note/an4759-introduction-to-using-the-hardware-realtime-clock-rtc-and-the-tamper-management-unit-tamp-with-stm32-mcus-stmicroelectronics.pdf): find family-specific features, then consult the exact reference manual and errata.
6. [STM32G0 RTC training](https://www.st.com/resource/en/product_training/STM32G0-WDG_TIMERS-Real-Time-Clock-RTC.pdf): the shadow-register example used here.
7. [NIST frequency measurement](https://www.nist.gov/publications/frequency-measurement): measurement terminology and instrumentation context.
8. [NIST stability terminology](https://www.nist.gov/pml/time-and-frequency-division/popular-links/time-frequency-z/time-frequency-z-st-sy): distinguish stability from offset/accuracy.
9. [RFC 5905](https://www.rfc-editor.org/rfc/rfc5905.html): NTP protocol and offset/delay context; consult the RFC page for updates and errata before implementation.
10. [RFC 8915](https://www.rfc-editor.org/rfc/rfc8915.html): authenticated network time mechanisms.
11. [IANA timezone theory](https://www.iana.org/time-zones/theory): civil-time representation and database assumptions.

Related chapters: [GPIO](gpio.md), [I2C](i2c.md), and [ADC](adc.md). This handbook's system examples and calculations are instructional designs, not vendor-certified implementations.

## Appendix D. A driver walkthrough using a fictional RTC

This exercise makes the register-level reasoning concrete without pretending one register sequence works on every part. **The following RTC is fictional. Do not send these addresses or masks to a real chip.** Its interface contract is intentionally small:

| Address | Fictional function | Access rule |
| :--- | :--- | :--- |
| 0x00 | BCD seconds | 00–59; no embedded flags |
| 0x01 | BCD minutes | 00–59 |
| 0x02 | BCD hours | 00–23, only 24-hour mode |
| 0x03 | BCD date | 01–31 subject to month |
| 0x04 | BCD month | 01–12 |
| 0x05 | BCD year | 00–99 interpreted as 2000–2099 |
| 0x10 | Control | Bit 0 FREEZE; bit 1 SNAPSHOT |
| 0x11 | Status | Bit 0 STOPPED, write-one-to-clear |

Assume SNAPSHOT copies all six calendar bytes to read buffers and clears itself when complete; FREEZE pauses counting and permits calendar writes. The bus write API reports success only after acknowledgment. The interface remains powered during normal transactions. These are explicit model assumptions, not inferred conventions.

### D.1 Decode a coherent sample

The transport sets SNAPSHOT and waits up to a monotonic deadline for it to clear. It reads the six buffered bytes in one burst, then reads status. Suppose the data are `58 59 23 28 02 24` in hexadecimal. After checked BCD conversion, the sample is 2024-02-28 23:59:58. The next day is a leap day under the Gregorian rule.

The driver must still validate each field and the combination. `58 59 23 31 04 24` decodes cleanly but must be rejected because April has 30 days. A set STOPPED flag makes the sample untrusted even if its date is valid.

The transaction's status observation is not infinitely atomic with its snapshot. A stop after the status read is outside that observation. A production time service periodically refreshes health and records the acquisition interval rather than claiming permanent certainty from one read.

### D.2 Set time with cleanup

Validate the proposed year first: this fictional device represents only 2000–2099, even though the portable calendar helpers support more. Encode all six bytes in memory before changing hardware. Acquire exclusive ownership, set FREEZE, write the complete calendar, release FREEZE, and verify by another snapshot.

If the burst fails after writing minutes, the hardware may contain mixed old and new fields. Keep software quality invalid. Attempt documented cleanup so counting is not left frozen; if cleanup fails, preserve the diagnostic. Do not clear STOPPED or announce success. A later trusted set can recover the device.

To clear the write-one-to-clear STOPPED flag, write exactly its clear mask under this fictional contract. Reading status and writing the same byte back could also clear unrelated flags if additional ones existed. This is why the adapter owns flag semantics.

### D.3 Define the successful result

Suppose the requested time is 12:00:00 and readback is 12:00:01. This may be correct if one second elapsed after release. Compare against an allowed time window based on measured monotonic elapsed time and the documented counter phase. Do not require exact equality blindly, and do not accept an arbitrary difference under “the RTC probably ticked.”

Publish `{new_generation, effective_time_window, source, uncertainty}` only after success. The old and new generations let event consumers detect a calendar discontinuity. Readback confirms programmed state; it does not establish long-term frequency accuracy.

### D.4 Derive tests from the contract

Test failure before FREEZE, after FREEZE, after each byte, on release, and during readback. Verify both return value and resulting quality. Inject STOPPED with an otherwise valid date. Simulate SNAPSHOT never completing and verify the timeout uses an independent monotonic source. Simulate a reset in the middle and inspect the next boot's behavior.

When implementing a real driver, replace every fictional assumption with a cited requirement from the real manual. If a chip cannot provide a corresponding guarantee, change the software contract rather than hiding the difference.

## Appendix E. Expert measurement and uncertainty analysis

### E.1 Separate the reference from the instrument

A frequency measurement has at least two clocks: the device under test and the reference defining the gate or capture counter. If the reference is +4 ppm fast, measuring an ideal RTC against it can make the RTC appear slow. Calibrating to that result transfers reference error into the RTC.

List the reference accuracy, capture resolution, input threshold variation, gate construction, sample duration, and environmental conditions. Choose a measurement duration that makes quantization small relative to the target correction. A long observation reduces random endpoint error but can average over environmental changes that you actually need to characterize.

For two phase observations with individual worst-case bounds `u0` and `u1`, the endpoint contribution to the rate bound is approximately `(u0+u1)/elapsed`. With ±1 ms per endpoint over 1000 s, this is 2 ppm. Over 100,000 s, it is 0.02 ppm—provided reference drift and other effects do not dominate.

### E.2 Fit rate without erasing faults

Collect pairs `(elapsed_reference_seconds, local_minus_reference_seconds)`. Plot them before fitting. A straight line suggests approximately constant frequency offset; repeated slope changes aligned with temperature suggest a thermal contribution. A discontinuity requires investigation rather than averaging it into a misleading rate.

For a simple least-squares slope:

```text
m = sum((t - mean(t)) * (e - mean(e))) / sum((t - mean(t))^2)
b = mean(e) - m*mean(t)
```

Centering time reduces numerical problems compared with multiplying enormous absolute epoch values. Use adequate precision and keep the raw observations. Least squares is sensitive to outliers; rejecting a sample requires an explicit reason such as an invalid capture flag, not simply that it spoils the preferred answer.

Residual scatter alone does not establish the full uncertainty. Reference error may shift every point together. A perfectly straight line can still be wrong because the reference was wrong.

### E.3 Allan deviation as an advanced tool

For equally spaced average fractional-frequency samples `y_k`, a simple non-overlapping two-sample Allan-variance estimate is:

```text
sigma_y^2(tau) = sum((y_(k+1) - y_k)^2) / (2*(M-1))
```

There are `M` samples, each averaged over `tau`, under the estimator's sampling assumptions. Taking the square root gives Allan deviation. Unlike a single standard deviation of all frequency values, this compares adjacent averages at a stated averaging interval. It can help distinguish behavior at different timescales. NIST's [frequency/time characterization chapter](https://www.nist.gov/system/files/documents/calibrations/pfc-2.pdf) is a primary reference for formal treatment.

This formula is an entry point, not a full metrology package. Missing samples, dead time, overlapping estimators, deterministic drift, and confidence intervals require care. A graph without the estimator, sample spacing, reference, and preprocessing is not reproducible evidence.

### E.4 Temperature-model validation

Fit a temperature model only after verifying the sensor measures a useful proxy for the oscillator's actual temperature. Apply slow temperature transitions first to distinguish static response from thermal lag. Then test the faster transitions expected in the enclosure.

Hold back part of the data for validation. A high-order polynomial that fits every calibration point can behave badly between them or outside the fitted range. Bound extrapolation and degrade quality when the operating temperature leaves the validated region.

Do not assume one temperature compensation curve removes aging or supply effects. Record the calibration's validity conditions. If compensation runs in the main MCU, determine what happens while it is off: the retained RTC may then free-run without the software correction.

## Appendix F. Product qualification and fault-injection matrix

A qualification plan connects requirement, stimulus, observation, and pass criterion. Avoid “test RTC: pass” as the only evidence.

| Requirement | Stimulus | Observation | Example pass criterion |
| :--- | :--- | :--- | :--- |
| Preserve time over reboot | Software and watchdog resets | Calendar progression and register-write trace | No unintended calendar reinitialization |
| Detect lost retention | Remove all power | First boot quality and flags | Invalid until an accepted set operation |
| Survive partial set | Fail each transaction stage | Cleanup state and returned status | No false success; no indefinite blocking |
| Coherent read | Repeated rollover reads | Raw snapshots and decoded values | No impossible mixed date/time |
| Alarm once | Delay ISR/task service | Pin level and event count | One application event per intended occurrence |
| Recover after missed alarm | MCU unavailable across deadline | Scheduler state | Documented catch-up/skip policy |
| Stay in budget | Main power off, worst environment | Integrated backup current | Within allocated energy and voltage limits |
| Bound holdover | Disconnect reference | Phase versus independent reference | Error stays within declared envelope |
| Handle clock corrections | Forward/backward setting | Log generations and job execution | Order retained; no accidental duplicates |
| Reject invalid input | Bad BCD/date/year | API output and hardware writes | Rejected before modifying hardware |

Choose environmental corners from the actual product requirement and component ratings. Do not exceed electrical limits merely to “stress test” an assembled unit without an appropriate test design. The aim is to establish promised operation and controlled failure, not to create unexplained damage.

Separate characterization from production screening. Characterization may involve long holds, multiple temperatures, and many samples. Production screening may use shorter startup, communication, backup switching, and output-frequency checks designed from that characterization. A short factory test cannot directly prove ten-year retention; it can catch defects and verify parameters linked to a justified lifetime model.

For a power-fail test, record where power is removed, rail fall times, backup voltage, bus state, and firmware phase. A bench supply's output-off behavior may differ from a loose battery contact or brownout. Repeat at several update boundaries and include recovery from repeated rapid cycling.

Preserve test artifacts: firmware hash, board revision, fixture revision, raw captures, calibration certificates where applicable, analysis script, and acceptance limits. This lets a later hardware revision be compared against the same evidence rather than someone else's memory.

## Appendix G. Common designs that look correct but are not

**The build-time initializer.** Firmware calls `set_time(__DATE__, __TIME__)` at every boot. It prints a plausible date in a demonstration, but every reboot moves time to compilation. Restrict any such convenience to an explicit development provisioning path and preserve valid retained time in ordinary startup.

**The apparently robust retry loop.** A driver retries forever until the oscillator starts or I2C succeeds. It converts a recoverable time failure into a system hang. Use bounded operations and an explicit degraded state. The rest of the product may still need to collect data or report the fault.

**The magic-marker certificate.** A backup word matches a constant, so firmware marks the time valid. The marker survives even though the oscillator stopped. Require independent hardware and policy evidence; a marker is only one piece of state.

**The two good reads.** Two reads agree, so the driver declares the time current. A frozen register, stopped oscillator, or shadow latch can return the same value forever. Coherence and progress are different properties. Test both on suitable timescales.

**The statistically impressive calibration.** A million readings are averaged against an uncalibrated reference. The random noise shrinks while the reference bias stays. Report what the reference actually supports.

**The one-number specification.** A device advertises low current, and the design ignores pull-ups, pin injection, and the module's circuitry. Measure the complete system in the exact power state. Main-mode current and backup-mode current belong to different budgets.

**The calendar timeout.** A communication deadline compares wall-clock seconds and fails after an administrator sets the date backward. Use a monotonic source with documented sleep behavior and wrap constraints.

**The full-featured portability layer.** Every RTC is exposed as if it supports subseconds, arbitrary one-shot alarms, and trustworthy fault detection. Unsupported features are silently approximated. Instead expose capabilities and reject requirements that the hardware cannot meet.

**The invisible correction.** Software adjusts time without recording it, so logs suggest an impossible event order. Treat a time correction as a first-class event and preserve monotonic sequence information.

**The copied battery circuit.** A holder and diode are copied from an unrelated module without checking chemistry, voltage, or charging path. Select the energy source and circuit together, then validate switching, leakage, and service conditions.

## Appendix H. A zero-to-expert learning path

### Stage 1: read and explain

Build a paper model of oscillator, divider, calendar, and interface. Convert frequency to period, ppm to daily error, and BCD to decimal without a library. Explain why a successful read does not prove correct time. Finish when you can distinguish resolution, rate error, and calendar validity in your own examples.

### Stage 2: implement pure logic

Run the companion tests, then independently implement a date parser with a written input grammar. Reject invalid dates, test leap boundaries, and state the epoch. Introduce a deliberate century bug and confirm the independent oracle catches it. Do not measure competence by whether one current date prints correctly.

### Stage 3: build a device adapter

Choose one actual RTC and read its electrical and register documentation. Write a capability table and a read/write transaction contract before coding. Add a fake bus that injects failures. Demonstrate that every waiting operation has a deadline and every partial update has a defined result.

### Stage 4: validate a board

Trace the module's power and pull-up paths. Measure main/backup current, retention, alarm assertion/release, and clock output. Verify normal boot preserves state. Produce annotated captures showing both successful behavior and a deliberately induced fault.

### Stage 5: characterize time quality

Measure rate against an adequate reference, record temperature, fit a simple model, and quantify residual error. Establish a holdover bound. Explain the uncertainty terms and why more display digits would not improve the physical clock.

### Stage 6: integrate a trustworthy service

Add synchronization, quality states, correction records, monotonic ordering, recurrence policy, and versioned persistence. Test outages and backward/forward corrections. Finish the capstone with a report listing verified behaviors, operating assumptions, and remaining bench work.

Expertise is the ability to connect a wrong timestamp to its physical, transactional, mathematical, or policy cause—and to design evidence that distinguishes those possibilities.
