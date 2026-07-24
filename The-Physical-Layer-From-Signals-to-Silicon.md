# The Physical Layer: From Signals to Silicon

### Sub-GHz, Bluetooth Low Energy, and Real Wireless PHY Design

---

## About This Book

Most networking books treat the Physical Layer as a single introductory chapter — a
paragraph about voltages, a diagram of a sine wave, and then a rapid escape upward into
frames and addresses, where the material is comfortable and discrete. This book does the
opposite. It stays at the bottom of the stack for its entire length, because that is where
wireless products actually fail.

The intended reader is an engineer who has to make a radio work: an embedded firmware
developer handed a Sub-GHz transceiver and a datasheet, a systems engineer asked why the
range is half what the link budget promised, a networking engineer who has traced a problem
downward through every layer and finally arrived at the PHY with no vocabulary for what to
do next.

The book has three commitments.

**First, every claim is quantified.** "Lower frequencies travel further" is not an
engineering statement. "At a fixed distance, free-space path loss at 868 MHz is 9 dB lower
than at 2.44 GHz, which is worth roughly 2.8× in range under a square-law model" is. Where a
number exists, this book gives it, gives the formula it came from, and works the arithmetic
in front of you.

**Second, theory is always closed back to hardware.** A chapter on noise is not finished
when it defines thermal noise power; it is finished when you can compute a receiver's noise
floor, compare it to the sensitivity figure on page 3 of a datasheet, and explain the
difference. A chapter on state machines is not finished until you can read the register
writes that drive them.

**Third, nothing is said twice.** Concepts are introduced once, in the place where they can
be developed most completely, and afterwards they are used rather than re-explained. Noise
is defined in Chapter 3 and never redefined. Path loss is derived in Chapter 12 and
thereafter simply applied.

### How the book is organised

**Part I — Foundations** builds the physical vocabulary: signals, bandwidth, noise, and the
information-theoretic ceilings that no amount of clever engineering can lift.

**Part II — Modulation and Coding** covers how bits become waveforms and how errors are
detected and corrected once the channel has damaged them.

**Part III — Media** examines copper, fiber, and free space as three different physical
environments with three different failure modes.

**Part IV — Wireless System Engineering** is where the discipline becomes practical: link
budgets, interference, regulation, antennas, and modem architecture.

**Part V — Silicon, Firmware, and Practice** descends into the chip: packet formats, state
machines, register-level firmware, driver code, laboratory measurement, and debugging.

### Prerequisites

Comfort with algebra and logarithms. Familiarity with C and with microcontroller concepts —
interrupts, DMA, memory-mapped registers — for Part V. No prior RF background is assumed;
decibels, impedance, and modulation are developed from first principles.

### A note on conventions

Frequencies are in hertz, powers in dBm unless stated otherwise, and all logarithms written
`log` are base 10 while `log₂` is explicit. Sub-GHz examples use the 868 MHz European and
915 MHz North American ISM bands. BLE examples target Bluetooth 5.x PHY modes. Datasheet
figures are drawn from commercially available transceivers and are representative rather
than specific to any one vendor.

---

## Table of Contents

**Part I — Foundations of Physical Communication**

1. What the Physical Layer Is, and Why It Must Exist
2. Signals: Time, Frequency, and the Meaning of Bandwidth
3. The Four Impairments: Noise, Attenuation, Distortion, Interference
4. The Ceilings: Nyquist, Shannon, and Eb/N0

**Part II — Modulation and Coding**

5. Line Coding and Baseband Signaling
6. Digital Modulation I: ASK, FSK, PSK, and the Constellation
7. Digital Modulation II: GFSK, MSK, OQPSK, DSSS, and Why Sub-GHz Chooses What It Does
8. Bit Errors: From Eb/N0 to a Sensitivity Number
9. Error Control: Whitening, CRC, FEC, Interleaving, and the BLE Coded PHY

**Part III — Physical Media**

10. Wired Physical Layers: Copper, Differential Signaling, and Ethernet
11. Optical Physical Layers: Fiber, Transceivers, and Dispersion
12. The Wireless Channel: Path Loss, Multipath, Fading, and Doppler

**Part IV — Wireless System Engineering**

13. Link Budgets: A Complete Worked Analysis
14. Interference, Coexistence, and Shared Spectrum
15. Regulatory Constraints: FCC, ETSI, Duty Cycle, and Listen-Before-Talk
16. Antennas and RF Front-Ends
17. Digital Baseband and Modem Architecture

**Part V — Silicon, Firmware, and Practice**

18. PHY Packet Structure: Preamble, Sync Word, Header, Payload, CRC
19. PHY State Machines and Timing
20. PHY Firmware Architecture: Registers, Interrupts, and DMA
21. End-to-End Signal Flow: From Buffer to Air and Back
22. Reading and Writing a Real Sub-GHz PHY Driver
23. Bring-Up and Measurement: Spectrum, Sensitivity, and PER
24. Debugging the Physical Layer: A Field Guide

**Appendices**

- A. Decibel and RF Mathematics
- B. Formula Reference
- C. Sub-GHz and BLE Parameter Tables
- D. Standards Map
- E. Glossary

---

# Part I — Foundations of Physical Communication

---

# Chapter 1

## What the Physical Layer Is, and Why It Must Exist

### 1.1 The Discontinuity at the Bottom of the Stack

Every layer of a protocol stack above the Physical Layer shares a comfortable assumption:
that information exists. A router examines an IP header because the header is *there*. A TCP
implementation retransmits a segment because it knows the segment was lost — a statement
that presupposes segments are the kind of thing that can be present or absent.

The Physical Layer enjoys no such assumption. Beneath it there are no bits. There is a
copper pair carrying a voltage difference of 340 millivolts, or a laser diode emitting 1.2
milliwatts at 1310 nanometres, or an electromagnetic field at 868.3 MHz whose amplitude at
the receiving antenna is on the order of a few microvolts. None of these things is a one or
a zero. They are continuous physical quantities, and the mapping between them and binary
information is a convention that the Physical Layer imposes and must defend against a
universe that does not respect it.

This is the discontinuity. Above the PHY, information is discrete, reliable, and
addressable. Below it, there is only energy propagating through matter, degraded by
distance, corrupted by thermal agitation, and contended for by every other transmitter in
range. The Physical Layer is the machinery that manufactures the illusion of discreteness
out of a continuous and hostile substrate.

Understanding this is not philosophical. It is the reason PHY failures are qualitatively
different from failures anywhere else in the stack. When TCP fails, it reports failure. When
the PHY fails, it produces *plausible nonsense*: a packet whose CRC happens to pass, a sync
word detected in pure noise, a receiver locked to an adjacent-channel interferer and
delivering that transmitter's data upward with every appearance of legitimacy. There is no
higher layer that can detect these conditions, because from above they are indistinguishable
from correct operation.

### 1.2 What the Layer Is Required to Do

Stripped to essentials, a Physical Layer implementation is obliged to perform six functions.
Every one of them is a place where a real system can break, and the rest of this book is
substantially an expansion of this list.

**Bit representation.** Establish a convention by which a physical quantity encodes a
logical value. The convention must be unambiguous to a receiver that did not observe the
transmitter directly and has only a corrupted copy of its output.

**Signal generation.** Synthesise the physical waveform corresponding to a bit sequence,
within the amplitude, bandwidth, and spectral-mask constraints imposed by hardware and by
regulation.

**Transmission.** Couple that waveform into the medium efficiently. An antenna that reflects
half its input power back toward the amplifier has thrown away 3 dB of link budget before
propagation has begun.

**Reception.** Detect the waveform after the medium has attenuated it — commonly by a factor
of 10⁹ to 10¹³ in wireless systems — and buried it in noise and interference.

**Synchronisation.** Establish *when* symbols begin and end. This is the least appreciated
and most frequently fatal PHY function. A receiver with a perfect view of the waveform but
an incorrect symbol clock recovers nothing.

**Impairment management.** Compensate, within the limits of physics, for attenuation, noise,
distortion, multipath, frequency offset, and interference.

And, equally definitively, the Physical Layer does **not**:

- interpret addresses or route,
- retransmit,
- guarantee delivery,
- or, in general, *correct* errors — though it may detect them, and modern PHYs increasingly
  do correct a subset of them via forward error correction (Chapter 9).

The boundary is worth being precise about, because it determines what the layer above must
be prepared to handle. A PHY delivers *frames that it believes to be intact*, accompanied by
metadata about the conditions under which they were received. Belief is not knowledge, and
the MAC layer above must be architected on that understanding.

### 1.3 Bits, Symbols, and Signals

Three terms are used loosely in casual speech and must be used precisely from here onward.
Nearly every confusion in PHY engineering can be traced to conflating them.

A **bit** is a unit of information. It has no physical existence. It exists in registers, in
memory, in the abstraction maintained by software — and nowhere else. You cannot measure a
bit with an oscilloscope.

A **signal** is a physical quantity that varies over time and propagates through a medium: a
voltage on a conductor, an optical power level in a fiber, an electromagnetic field in free
space. Signals are continuous, real-valued, and measurable. They are what actually exists.

A **symbol** is the bridge. It is a signal state, held for a defined interval, that carries
an agreed number of bits. Symbols are the unit in which the Physical Layer actually
operates.

The relationship between bits and symbols is the single most consequential design parameter
in a PHY:

$$
R_b = R_s \cdot \log_2(M)
$$

where `R_b` is the bit rate in bits per second, `R_s` is the symbol rate in symbols per
second (often written in baud), and `M` is the number of distinguishable states a symbol can
take.

The implications run through the entire book:

- When `M = 2`, one symbol carries one bit and bit rate equals symbol rate. This is the
  regime of BLE, of most Sub-GHz FSK links, and of every system that prioritises robustness.
- When `M = 4`, each symbol carries two bits: the same bit rate in half the symbol rate, and
  therefore roughly half the bandwidth — but the four states must be packed into the same
  signal space, so they sit closer together and are more easily confused by noise.
- When `M = 64` or `M = 256`, as in Wi-Fi or DOCSIS, spectral efficiency becomes very high
  and the required signal-to-noise ratio becomes correspondingly brutal.

There is no free lunch anywhere in this relationship. Chapter 4 will show that the exchange
rate between spectral efficiency and required SNR is not merely empirical but a theorem.

**A worked instance.** A BLE 1M PHY link runs GFSK with `M = 2` at 1 Msym/s, giving exactly
1 Mbit/s. A hypothetical 4-FSK system at the same 1 Msym/s would deliver 2 Mbit/s in
comparable bandwidth — but would require approximately 4 dB more SNR for the same error
rate, which at fixed transmit power translates to roughly 37% less range in free space. The
BLE designers chose robustness. A Wi-Fi designer, operating over tens of metres with
mains power available, chooses differently. Neither is wrong; they are answering different
questions.

### 1.4 The Two Regimes: Deterministic and Probabilistic PHYs

Physical Layers divide into two families whose engineering cultures barely overlap, and the
division is worth establishing at the outset because this book will move between them.

**Guided-media PHYs** — copper and fiber — operate in an environment that is bounded,
shielded, and characterised. A 100-metre run of Category 6A cable has an insertion loss that
is known within a decibel, does not vary with the weather, and does not change when someone
walks past. The channel is effectively static. Consequently, guided PHYs are engineered to
be *deterministic*: 10GBASE-T targets a bit error rate below 10⁻¹², a figure so low that a
link running continuously at full rate encounters an uncorrected error roughly once a day,
and even that is normally eliminated by the forward error correction built into the standard.

**Unguided PHYs** — wireless — operate in an environment that is shared, unbounded, and
time-varying on scales from microseconds to months. The path loss between two nodes changes
when a door closes. The interference environment changes when a neighbour's Wi-Fi access
point selects a new channel. Multipath nulls move when anything in the room moves.
Consequently, wireless PHYs are engineered to be *probabilistic*: a packet error rate of 1%
is normal, entirely acceptable, and explicitly designed for.

This distinction propagates upward with force. Ethernet's MAC layer has essentially no error
recovery, because the PHY beneath it does not require any. BLE's link layer is built around
acknowledgement, retransmission, and adaptive frequency hopping, because its PHY cannot
promise anything. When an engineer accustomed to wired networking begins working on wireless
systems, the recurring surprise is not that packets are lost — it is that packet loss is not
a fault condition.

### 1.5 Why Sub-GHz Is a Distinct Discipline

Within wireless, this book gives particular attention to Sub-GHz systems — the unlicensed
bands near 433 MHz, 868 MHz, and 915 MHz — alongside BLE at 2.4 GHz. These are not simply
the same problem at different frequencies. They have structurally different design centres,
and a summary of the differences serves as a map of Part IV.

**Propagation strongly favours Sub-GHz.** Free-space path loss rises as the square of
frequency. The 9.0 dB advantage of 868 MHz over 2.44 GHz at any given distance is
substantial and, unlike most engineering advantages, entirely free. Diffraction around
obstacles and penetration through building materials both improve at lower frequency as
well, so the real-world advantage typically exceeds the free-space figure.

**Regulation is far more restrictive in Sub-GHz.** European operation under EN 300 220
imposes duty-cycle limits — commonly 1% in the 868.0–868.6 MHz band, meaning a device may
transmit for 36 seconds in any hour. There is no equivalent constraint in the 2.4 GHz band.
An engineer who designs a Sub-GHz protocol without a duty-cycle budget has designed a
product that cannot be sold in Europe. Chapter 15 treats this in full.

**Data rates are low, and deliberately so.** Sub-GHz links commonly run between 1.2 kbps and
250 kbps. This is not a limitation of the technology but a choice: narrow channel bandwidth
means a low noise floor, and a low noise floor means extraordinary sensitivity. A receiver
operating in a 5 kHz channel can achieve a sensitivity around −123 dBm — roughly 27 dB better
than a BLE 1M receiver — and 27 dB of link budget buys a great deal of distance.

**Channel structure differs fundamentally.** BLE uses 40 fixed 2 MHz channels with adaptive
hopping across them, a strategy that works because the band is wide and the interference is
dense and uncorrelated. Sub-GHz systems more often use narrow static channels, sometimes
with listen-before-talk, because the band is narrow and the interference is sparse but
persistent.

**Sub-GHz is not a standard; it is a band.** BLE gives you a complete specification: PHY,
link layer, host stack, interoperability testing. Sub-GHz gives you spectrum and a
regulatory rulebook. Everything above the antenna is a decision someone has to make — which
is precisely why Sub-GHz engineers need to understand the Physical Layer in a depth that BLE
application developers can often avoid.

### 1.6 The Path Through This Book

The remainder of Part I builds the physical and mathematical vocabulary. Chapter 2 develops
signals and bandwidth, Chapter 3 the four impairments, and Chapter 4 the theoretical limits
that bound every subsequent design choice. By the end of Part I you will be able to compute a
receiver noise floor from first principles and state, with numbers, why a particular data
rate cannot be achieved over a particular link at a particular power.

Part II converts bits into waveforms and back. Part III examines the three media. Part IV
assembles these into engineered systems. Part V opens the chip.

The organising claim of the book is that these are not five subjects but one, and that an
engineer who can trace a single bit from a firmware buffer, through a modulator, out of an
antenna, across a fading channel, into a low-noise amplifier, through a demodulator, and
back into a buffer — with the numbers attached at every stage — can debug almost anything.

---

# Chapter 2

## Signals: Time, Frequency, and the Meaning of Bandwidth

### 2.1 Why the Frequency Domain Is Not Optional

An oscilloscope shows a signal as amplitude against time. This is the intuitive
representation, and for many purposes it is the useful one: it shows rise times, symbol
boundaries, overshoot, and timing relationships directly.

It is also almost useless for radio engineering, and understanding why is the first
conceptual hurdle in the subject.

Consider a practical question. A Sub-GHz transmitter operates on a 200 kHz-wide channel
centred at 868.3 MHz. Regulation requires that its emissions outside that channel be
suppressed by a specified amount. Nothing in a time-domain view of the transmitted waveform
answers the question "how much energy is this transmitter putting into my neighbour's
channel?" — yet that question determines whether the product is legal.

The frequency domain answers it directly. And the transformation between the two views is
not an approximation or a modelling convenience: it is an exact, invertible, information-
preserving change of coordinates. The same signal, described in a different basis.

### 2.2 The Sine Wave and Its Three Parameters

Every signal this book considers can be decomposed into sinusoids, so the sinusoid is where
we begin.

$$
s(t) = A \cdot \sin(2\pi f t + \phi)
$$

Three parameters completely specify it:

| Parameter | Symbol | Units | Physical meaning |
|---|---|---|---|
| Amplitude | A | volts, or √watts | Signal strength; determines received power |
| Frequency | f | hertz | Oscillations per second; determines the band occupied |
| Phase | φ | radians | Position within the cycle at t = 0; the timing reference |

The reason the sinusoid is privileged above all other waveforms is a property of linear
systems: **a sinusoid passed through any linear time-invariant system emerges as a sinusoid
of the same frequency.** Amplitude and phase may change; frequency never does. No other
waveform family has this property.

This is the entire justification for frequency-domain analysis. Cables, filters, amplifiers,
and free space are all approximately linear and time-invariant. So if you know what a system
does to a sinusoid at each frequency — its *transfer function* — you know what it does to any
signal, by decomposing that signal into sinusoids, applying the transfer function to each,
and reassembling. A messy differential-equation problem becomes multiplication.

Digital modulation is, in this light, precisely the act of varying one of these three
parameters in step with the data. Vary A and you have amplitude shift keying. Vary f and you
have frequency shift keying. Vary φ and you have phase shift keying. Vary A and φ together
and you have QAM. There are no other options, because there are no other parameters.
Chapters 6 and 7 develop each in turn.

### 2.3 Time and Frequency as Two Views of One Object

Fourier's result — that any well-behaved signal can be expressed as a sum (or integral) of
sinusoids — gives us the bridge.

A **periodic** signal decomposes into a *discrete* set of sinusoids: a fundamental at
frequency `f₀` and harmonics at integer multiples. Its spectrum is a series of spikes.

A **non-periodic** signal decomposes into a *continuous* range of frequencies. Its spectrum
is a smooth curve.

Real data is non-periodic — that is what makes it data — so real transmissions occupy
continuous bands rather than discrete lines. This has an immediate and important
consequence, which is worth stating as a principle:

> **Information requires bandwidth. A signal that occupies zero bandwidth carries zero
> information.**

The reasoning is short. A perfectly pure sinusoid, unchanging for all time, has a spectrum
consisting of exactly one line at exactly one frequency and zero width. It is also
completely predictable: knowing it for one microsecond tells you it for all time. It carries
no information. To carry information a signal must change unpredictably, and changing
unpredictably requires energy at multiple frequencies. The faster the change, the wider the
spread.

**The canonical example: the square wave.** A square pulse train at frequency `f₀`
decomposes into odd harmonics with amplitudes falling as 1/n:

$$
s(t) = \frac{4A}{\pi}\left[\sin(2\pi f_0 t) + \frac{1}{3}\sin(2\pi \cdot 3f_0 t) + \frac{1}{5}\sin(2\pi \cdot 5f_0 t) + \cdots\right]
$$

A perfect square wave requires infinite bandwidth. This is not a curiosity — it is why
digital signals cannot be transmitted directly over band-limited channels without
modification, and why the sharp edges you see on an oscilloscope inside a chip are gone by
the time the signal reaches an antenna. Sharp edges *are* high-frequency content, and every
real channel is a low-pass filter.

### 2.4 Bandwidth: Four Definitions and Why They Differ

"Bandwidth" is used to mean at least four distinct things, and datasheet comparisons go
wrong when the definitions are mixed. All are measured in hertz.

**Occupied bandwidth (OBW)** — the width containing a specified fraction of total transmitted
power, conventionally 99%. This is the definition used by most regulators, and the one that
matters for compliance.

**−3 dB bandwidth** — the width between the points where power falls to half its peak. This
is the standard definition for filters and receiver front ends.

**Null-to-null bandwidth** — for signals with a well-defined spectral shape, the width
between the first spectral nulls either side of the carrier. Convenient for analysis; rarely
what regulators mean.

**Channel spacing** — the frequency separation between adjacent channels in a channel plan.
Always wider than occupied bandwidth, with the difference forming a guard band.

A concrete illustration of why the distinction matters: a BLE 1M transmitter has an occupied
bandwidth of roughly 1.0–1.2 MHz, but BLE channel spacing is 2 MHz. The extra 0.8 MHz is not
waste — it is the margin that permits a receiver to reject an adjacent-channel transmitter
that may be 40 dB stronger than the wanted signal. Chapter 14 quantifies this.

### 2.5 The Fundamental Bandwidth–Rate Relationship

For any modulation, the transmitted bandwidth is proportional to the *symbol* rate, not the
bit rate:

$$
B \approx (1 + \alpha) \cdot R_s
$$

where `α` is the roll-off factor of the pulse-shaping filter, typically between 0.2 and 0.5
in practical systems. The bandwidth cannot be less than `R_s` — a result that Chapter 4
establishes as the Nyquist limit.

Combining with the bit-rate relation from Section 1.3:

$$
B \approx \frac{(1 + \alpha) \cdot R_b}{\log_2(M)}
$$

This one expression captures the central trade of digital communications. To transmit more
bits per second you may either widen the bandwidth or increase `M`. Bandwidth is limited by
regulation and by the noise it admits. Increasing `M` is limited by SNR. Every PHY in
existence is a particular resolution of this tension.

**Worked example — BLE 1M.** `R_b` = 1 Mbps, `M` = 2, so `R_s` = 1 Msym/s. With GFSK the
relevant estimate is Carson's rule for frequency modulation:

$$
B \approx 2 f_d + R_b
$$

BLE specifies a nominal frequency deviation `f_d` of 250 kHz, giving:

$$
B \approx 2(250\text{ kHz}) + 1\text{ MHz} = 1.5\text{ MHz}
$$

The Gaussian filter (BT = 0.5) suppresses the spectral tails considerably, bringing measured
occupied bandwidth to roughly 1.1 MHz — comfortably inside the 2 MHz channel.

**Worked example — Sub-GHz 50 kbps.** A typical 802.15.4g-style configuration uses `R_b` =
50 kbps, 2-FSK, `f_d` = 25 kHz:

$$
B \approx 2(25\text{ kHz}) + 50\text{ kHz} = 100\text{ kHz}
$$

Ten times narrower than BLE — which, as Chapter 3 will show, is worth 10 dB of noise floor
and is the principal reason Sub-GHz achieves the sensitivity it does.

### 2.6 Modulation Index: The Parameter That Ties It Together

For frequency-based modulations, a single dimensionless parameter governs the trade between
bandwidth and robustness. The **modulation index**:

$$
h = \frac{2 f_d}{R_s}
$$

Both of the examples above have `h = 0.5`, which is not a coincidence — it is the value at
which the two frequency states become orthogonal at minimum spacing, and it is the design
point of an enormous number of practical radios.

| `h` | Bandwidth | Sensitivity | Where used |
|---|---|---|---|
| 0.3 | Narrowest | Poorest | Bandwidth-constrained narrowband links |
| 0.5 | Minimum for orthogonality (MSK) | Good | BLE, most Sub-GHz, GSM (h = 0.5, GMSK) |
| 1.0 | ~2× wider | Best; tolerates frequency error | Long-range Sub-GHz, low-cost crystal designs |

The choice of `h` is one of the few PHY parameters an embedded engineer routinely configures
directly in a register, and Chapter 7 returns to it with the demodulator architecture in
view. The practical point to carry forward: **higher `h` costs bandwidth and buys tolerance
to frequency error** — which matters enormously when your bill of materials specifies a
±20 ppm crystal rather than a ±2 ppm TCXO. At 868 MHz, 20 ppm is 17.4 kHz of offset, which
is a significant fraction of a 25 kHz deviation.

### 2.7 What Carries Forward

Four results from this chapter are used continuously in what follows:

1. Any signal is equivalently described in time or frequency; the frequency view is the one
   in which channels, filters, and regulations are expressed.
2. Modulation is the variation of amplitude, frequency, or phase — there is nothing else to
   vary.
3. Bandwidth scales with symbol rate, and bit rate exceeds symbol rate only by increasing
   `M`, which costs SNR.
4. Narrow bandwidth is the foundation of Sub-GHz sensitivity, and modulation index is the
   dial that sets it.

Chapter 3 introduces the adversary: the four mechanisms by which a channel destroys the
signal you have just learned to describe.

---

# Chapter 3

## The Four Impairments: Noise, Attenuation, Distortion, Interference

### 3.1 A Single Consolidated Treatment

Communication channels damage signals through exactly four mechanisms. They are frequently
confused with one another, and the confusion is expensive, because each has a different
cause, a different mathematical description, and — most importantly — a different remedy.
Applying the wrong remedy is one of the most common failure modes in wireless product
development.

| Impairment | What it does | Mitigated by |
|---|---|---|
| **Attenuation** | Reduces signal power | More TX power, better antennas, lower frequency, shorter distance |
| **Noise** | Adds random energy | Narrower bandwidth, lower noise figure, coding — **never** by more TX power alone |
| **Distortion** | Alters signal shape | Equalisation, filtering, linear amplifiers |
| **Interference** | Adds *another signal's* energy | Frequency planning, hopping, filtering, spatial separation |

The single most important row in that table is the second, and it deserves stating as a
principle before the mathematics:

> **Increasing transmit power does not improve signal-to-noise ratio at the receiver in any
> way that matters, if the noise is generated in the receiver itself.** It improves the
> received signal, and the receiver's own thermal noise stays where it is — so SNR does
> improve. But when noise is dominated by external interference that scales with your own
> transmissions in a shared band, or when regulation caps your power, the lever is not
> available. Understanding which noise you have determines which lever works.

This chapter treats all four mechanisms once and completely. They are not revisited in later
chapters; they are applied.

### 3.2 Attenuation

Attenuation is the loss of signal power with distance through a medium. It is deterministic,
predictable, and — critically — it affects signal and does not affect the receiver's internal
noise. This asymmetry is what makes range a finite quantity.

Because losses multiply as a signal passes through successive elements, they are universally
expressed in decibels, where multiplication becomes addition:

$$
L_{dB} = 10 \log_{10}\left(\frac{P_{in}}{P_{out}}\right)
$$

**In copper**, attenuation is caused by resistive loss and by the skin effect, in which
high-frequency current crowds into the outer surface of the conductor and raises effective
resistance. Loss rises approximately with the square root of frequency, which is why cable
categories are specified by the bandwidth they support rather than by length alone. Cat 6A
loses roughly 20 dB over 100 m at 100 MHz, and roughly 45 dB at 500 MHz.

**In fiber**, attenuation is caused by Rayleigh scattering and material absorption. It is
remarkably low — approximately 0.35 dB/km at 1310 nm and 0.20 dB/km at 1550 nm — which is
why fiber spans reach 80 km without amplification while copper cannot reach 100 metres at
comparable rates.

**In free space**, the mechanism is different: no energy is absorbed, but the power spreads
over an expanding spherical surface. The result is the free-space path loss equation, in the
form used for engineering work:

$$
\text{FSPL}_{dB} = 20\log_{10}(d_{km}) + 20\log_{10}(f_{MHz}) + 32.44
$$

Two properties follow immediately and are used constantly in Part IV.

*Doubling distance costs 6 dB.* Because loss goes as `d²`, `20 log(2) = 6.02 dB`. Every 6 dB
of additional link margin buys a factor of two in free-space range.

*Doubling frequency also costs 6 dB.* This is the entire quantitative basis of the Sub-GHz
advantage:

| Distance | 868 MHz | 2440 MHz | Advantage |
|---|---|---|---|
| 100 m | 71.2 dB | 80.2 dB | 9.0 dB |
| 1 km | 91.2 dB | 100.2 dB | 9.0 dB |
| 10 km | 111.2 dB | 120.2 dB | 9.0 dB |

The 9.0 dB advantage is constant with distance and, in terms of range at fixed margin, is
worth a factor of `10^(9/20) ≈ 2.8×` under free-space conditions. In real environments with
diffraction and material penetration, the advantage is typically larger still.

*Verification of the 1 km, 868 MHz figure:* `20 log(1) = 0`; `20 log(868) = 58.77`;
`0 + 58.77 + 32.44 = 91.2 dB`. A transmitter radiating +14 dBm delivers `14 − 91.2 = −77.2 dBm`
to an isotropic receiving antenna at that range, before any real-world losses.

### 3.3 Noise

Noise is unwanted random energy added to the signal. Unlike attenuation it cannot be undone,
because it carries no structure to exploit. It sets the absolute floor on receiver
performance, and computing that floor is the single most useful calculation in wireless
engineering.

#### 3.3.1 Thermal noise and the −174 dBm/Hz constant

Every resistive element at a temperature above absolute zero generates noise from the random
thermal motion of charge carriers. The available noise power is:

$$
N = kTB
$$

where `k` = 1.38 × 10⁻²³ J/K (Boltzmann's constant), `T` is absolute temperature in kelvin,
and `B` is bandwidth in hertz.

At the standard reference temperature of 290 K, the noise power spectral density is:

$$
kT = 1.38 \times 10^{-23} \times 290 = 4.00 \times 10^{-21}\ \text{W/Hz}
$$

Converting to dBm per hertz:

$$
10\log_{10}\left(\frac{4.00 \times 10^{-21}}{10^{-3}}\right) = -174\ \text{dBm/Hz}
$$

**−174 dBm/Hz is the most important number in RF engineering.** It is a constant of nature at
room temperature. It is the floor below which no receiver, at any price, can operate. Commit
it to memory.

#### 3.3.2 Noise figure

Real receivers add noise of their own. The **noise figure (NF)** quantifies how much, in dB,
a receiver degrades the SNR presented to it:

$$
\text{NF}_{dB} = \text{SNR}_{in,dB} - \text{SNR}_{out,dB}
$$

A perfect receiver has NF = 0 dB. Practical integrated transceivers achieve 4–10 dB.
Discrete designs with a dedicated external low-noise amplifier reach 2–3 dB.

Noise figure is dominated by the *first* amplifying stage, a result formalised by the Friis
cascade equation:

$$
F_{total} = F_1 + \frac{F_2 - 1}{G_1} + \frac{F_3 - 1}{G_1 G_2} + \cdots
$$

(with `F` and `G` as linear ratios, not decibels). Because `G₁` divides every subsequent
term, a high-gain low-noise first stage makes everything after it nearly irrelevant. This is
why the LNA sits immediately behind the antenna, and why inserting a lossy switch, filter,
or long coaxial run *before* the LNA is so damaging: every dB of pre-LNA loss is a dB
directly onto the system noise figure. Chapter 16 develops the consequences for front-end
layout.

#### 3.3.3 Computing the noise floor

Assembling the pieces gives the working formula:

$$
N_{floor,dBm} = -174 + 10\log_{10}(B_{Hz}) + \text{NF}_{dB}
$$

**Worked example — BLE 1M.** Receiver bandwidth ≈ 1 MHz, NF = 8 dB:

$$
N = -174 + 10\log_{10}(10^6) + 8 = -174 + 60 + 8 = -106\ \text{dBm}
$$

**Worked example — Sub-GHz 50 kbps.** Receiver bandwidth ≈ 100 kHz, NF = 6 dB:

$$
N = -174 + 50 + 6 = -118\ \text{dBm}
$$

**Worked example — Sub-GHz narrowband 1.2 kbps.** Receiver bandwidth ≈ 5 kHz, NF = 6 dB:

$$
N = -174 + 37 + 6 = -131\ \text{dBm}
$$

The progression tells the whole Sub-GHz story. Reducing bandwidth from 1 MHz to 5 kHz — a
factor of 200 — lowers the noise floor by `10 log(200) = 23 dB`, and the improved noise
figure adds 2 dB more. Twenty-five decibels of free sensitivity, purchased entirely by
accepting a lower data rate.

#### 3.3.4 From noise floor to sensitivity

Sensitivity is the noise floor plus the SNR the demodulator requires:

$$
S_{dBm} = N_{floor} + \text{SNR}_{req}
$$

Required SNR depends on modulation and target error rate; Chapter 8 derives it properly. For
now, representative values: uncoded GFSK with `h = 0.5` needs roughly 9–12 dB for a bit error
rate of 10⁻³.

| System | Bandwidth | NF | Noise floor | SNR req | Sensitivity | Typical datasheet |
|---|---|---|---|---|---|---|
| BLE 1M | 1 MHz | 8 dB | −106 dBm | 10 dB | −96 dBm | −95 to −97 dBm |
| BLE Coded S=8 | 1 MHz | 8 dB | −106 dBm | −2 dB | −108 dBm | −103 to −106 dBm |
| Sub-GHz 50 kbps | 100 kHz | 6 dB | −118 dBm | 9 dB | −109 dBm | −108 to −110 dBm |
| Sub-GHz 1.2 kbps | 5 kHz | 6 dB | −131 dBm | 8 dB | −123 dBm | −121 to −124 dBm |

The agreement between the calculated and published figures is close enough to be useful and
loose enough to be honest — implementation losses of 1–3 dB are normal. **If your measured
sensitivity is more than about 3 dB worse than this calculation predicts, you have a
hardware problem, and Chapter 23 tells you how to find it.**

Note the Coded PHY row: a *negative* required SNR is not an error. Forward error correction
allows correct decoding when signal power is below noise power, because the redundancy is
spread over enough symbols that the noise averages out. Chapter 9 explains the mechanism.

#### 3.3.5 Other noise sources

**Phase noise** is short-term random fluctuation in an oscillator's frequency. It matters
because it smears the carrier, degrading the ability to reject adjacent channels — a
mechanism called reciprocal mixing, treated in Chapter 16.

**Flicker (1/f) noise** dominates at low frequencies and affects baseband and DC-coupled
stages rather than the RF path.

**Quantisation noise** is introduced by the analogue-to-digital converter. Each bit of ADC
resolution contributes approximately 6 dB of dynamic range; Chapter 17 covers the sizing
decision.

### 3.4 Distortion

Distortion differs fundamentally from noise: it is deterministic. The signal is altered
rather than buried, and because the alteration follows rules, it can in principle be
reversed.

**Linear distortion** occurs when different frequency components are attenuated or delayed by
different amounts. Amplitude distortion changes the spectral shape; phase distortion —
better described as group delay variation — smears symbols in time. The result of both is
**inter-symbol interference (ISI)**, in which energy from one symbol spills into the decision
window of the next. Linear distortion is correctable by equalisation, and Chapter 17
describes how modem blocks do it.

**Nonlinear distortion** arises when a component's output is not proportional to its input,
most commonly a power amplifier driven near saturation. It generates harmonics and
intermodulation products at frequencies that were not present in the input — meaning energy
appears in bands you are not licensed to occupy. This is the most common cause of a product
failing spectral-mask certification.

The relevant figure of merit is the **third-order intercept point (IP3)**, and the reason
third order dominates is geometric: for two input tones at `f₁` and `f₂`, third-order
products appear at `2f₁ − f₂` and `2f₂ − f₁`, which fall *inside or adjacent to* the wanted
band and therefore cannot be filtered out. Second-order products land far away and are
easily removed.

This creates a direct and unavoidable conflict with modulation choice, and it is the reason
Chapter 7 concludes as it does. Amplifiers are most efficient near saturation — where they
are most nonlinear. Constant-envelope modulations such as FSK and GFSK carry no amplitude
information, so nonlinear distortion of the envelope costs nothing, and the PA may be run
hard. Amplitude-varying modulations such as QAM require the PA to be backed off by 6–10 dB
into its linear region, sacrificing both efficiency and output power. For a battery-powered
Sub-GHz node, that trade is not close.

### 3.5 Interference

Interference is energy from another transmitter. It is the impairment that distinguishes
shared unlicensed spectrum from every other engineering environment, and it behaves
differently from noise in three ways that matter operationally.

**It is structured.** Interference has a modulation, a bandwidth, and a duty cycle. Noise
does not. Structure can sometimes be exploited — a receiver can be built to reject a known
interferer — but structure also means interference does not average out the way noise does.

**It is bursty.** A Wi-Fi transmission occupies 20 MHz for a few hundred microseconds and
then vanishes. Average interference power is a nearly useless statistic; what matters is the
distribution of instantaneous power and its correlation with your packet timing. A link that
is fine on average can fail completely because the interferer's period aligns with your
transmission schedule.

**It is not reduced by narrowing bandwidth.** This is the crucial asymmetry. Narrowing your
receiver bandwidth by 10× reduces thermal noise by 10 dB — but if a narrowband interferer sits
inside your remaining passband, you have gained nothing at all against it. Sensitivity
improvements help against noise and are useless against co-channel interference.

Interference types are classified by frequency relationship:

- **Co-channel** — the interferer occupies your channel. Only spatial separation, time
  separation, or frequency change helps.
- **Adjacent-channel** — the interferer sits in a neighbouring channel and leaks in through
  imperfect filtering. Mitigated by better receiver selectivity and wider guard bands.
- **Blocking / desensitisation** — a strong out-of-band signal drives the receiver front end
  toward compression, raising its effective noise figure and degrading sensitivity even
  though the interferer never enters the passband. A receiver can be desensitised by 20 dB by
  a signal it is nominally filtering out entirely.

Chapter 14 treats coexistence strategy in full; Chapter 15 covers the regulatory mechanisms
— duty cycle limits and listen-before-talk — that exist specifically to make shared spectrum
survivable.

### 3.6 Which Impairment Do You Have?

A diagnostic summary, expanded into a full procedure in Chapter 24:

| Symptom | Likely impairment |
|---|---|
| Range shorter than predicted, but consistent | Attenuation — check antenna match, front-end loss |
| Errors increase smoothly as distance grows | Noise-limited — check noise figure, bandwidth |
| Link works, then fails abruptly at a fixed spot | Multipath fading (Chapter 12) |
| Errors are bursty, link fine between bursts | Interference |
| Sensitivity fine, but fails with a strong nearby transmitter | Blocking / desensitisation |
| Fails spectral mask, works functionally | Nonlinear distortion — PA backoff |
| High error rate only at high data rates | ISI / linear distortion — needs equalisation |

---

# Chapter 4

## The Ceilings: Nyquist, Shannon, and Eb/N0

### 4.1 Limits That Cannot Be Engineered Around

The impairments of Chapter 3 are, in principle, adversaries to be fought. The limits of this
chapter are not. They are theorems. No modulation scheme, no coding scheme, no amount of
processing power, and no future technology will exceed them, any more than a heat engine will
exceed Carnot efficiency.

This is liberating rather than depressing. Knowing the ceiling tells you how much room you
have left, and it lets you recognise immediately when a specification is impossible — a
recognition that has saved many engineering programmes from spending a year discovering it
empirically.

### 4.2 The Nyquist Limit: Bandwidth Constrains Symbol Rate

Nyquist established in 1924 that a channel of bandwidth `B` hertz can carry at most `2B`
independent symbols per second without inter-symbol interference:

$$
R_{s,max} = 2B
$$

Equivalently, transmitting `R_s` symbols per second requires at least `R_s / 2` hertz —
though this is the theoretical minimum, achieved only with an ideal brick-wall filter that
cannot be built. Practical systems using raised-cosine pulse shaping with roll-off `α`
require:

$$
B = \frac{(1 + \alpha) R_s}{2}
$$

With `M` levels per symbol, the resulting bit-rate ceiling is:

$$
C = 2B \log_2(M)
$$

Note carefully what this expression contains and what it does not: it contains bandwidth and
constellation size, and it contains **no noise term**. Nyquist's limit says that in a
perfectly noiseless channel, capacity is unbounded — just keep increasing `M`. That is
obviously not how the world works, and closing the gap is Shannon's contribution.

### 4.3 The Shannon Limit: Noise Constrains Everything

Shannon's 1948 theorem gives the true ceiling:

$$
C = B \log_2\left(1 + \text{SNR}\right)
$$

with `C` in bits per second, `B` in hertz, and SNR as a **linear power ratio, not decibels** —
the most frequent error in applying this formula.

The theorem's content is stronger than the formula suggests. Shannon proved that at any rate
below `C`, codes exist that achieve arbitrarily low error probability; and that above `C`, no
code achieves reliable communication at all. The transition is sharp. There is no gentle
degradation past capacity — there is a wall.

**Worked example — BLE 1M.** `B` = 1 MHz. At the sensitivity point, SNR ≈ 10 dB = 10 linear:

$$
C = 10^6 \times \log_2(11) = 10^6 \times 3.46 = 3.46\ \text{Mbps}
$$

BLE transmits 1 Mbps in that bandwidth, so it operates at roughly 29% of Shannon capacity.
The remaining 71% is the price of a simple, cheap, low-power, uncoded demodulator — and for
a coin-cell device, that is an excellent trade.

**Worked example — Sub-GHz 50 kbps.** `B` = 100 kHz, SNR at sensitivity ≈ 9 dB ≈ 7.9 linear:

$$
C = 10^5 \times \log_2(8.9) = 10^5 \times 3.15 = 315\ \text{kbps}
$$

Again, a system transmitting well below its theoretical ceiling, for the same reasons.

**Where the gap is actually closed.** LTE and 5G operate within 1–2 dB of Shannon capacity,
using turbo and LDPC codes with iterative soft-decision decoding. The cost is a receiver that
consumes orders of magnitude more energy and silicon area than a BLE radio. Capacity is
available for purchase; the currency is power and complexity.

### 4.4 Reading Shannon Properly

Three consequences of the formula are worth extracting explicitly, because each contradicts a
common intuition.

**Bandwidth beats power.** Capacity is *linear* in `B` and only *logarithmic* in SNR.
Doubling bandwidth doubles capacity. Doubling transmit power (+3 dB) at an SNR of 10 dB
raises capacity by only about 26%. This is why every high-rate system — Wi-Fi, 5G, UWB — is
fundamentally a bandwidth story rather than a power story.

**Capacity does not vanish as SNR falls below 1.** At SNR = 0.1 (−10 dB), `log₂(1.1) = 0.137`,
so a 1 MHz channel still supports 137 kbps. Communication below the noise floor is not a
paradox; it is ordinary. This is the regime in which spread spectrum, GPS, and the BLE Coded
PHY operate.

**There is an absolute energy floor.** As bandwidth grows without limit, the required energy
per bit approaches a finite minimum:

$$
\frac{E_b}{N_0}\bigg|_{min} = \ln(2) = 0.693 = -1.59\ \text{dB}
$$

This is the Shannon limit proper. No system, ever, communicates reliably below −1.59 dB of
Eb/N0. Modern LDPC-coded systems operate about 1 dB above it, which is one of the more
remarkable achievements in engineering.

### 4.5 Eb/N0: The Only Fair Basis for Comparison

SNR is an unsatisfactory metric for comparing modulation schemes, because it depends on
receiver bandwidth — and different schemes use different bandwidths for the same data rate.
Comparing a 1 Mbps system in 1 MHz against a 1 Mbps system in 2 MHz by their SNR figures
measures the filter, not the modulation.

**Eb/N0** — energy per bit divided by noise power spectral density — removes the dependency.
It is dimensionless, normalised per bit, and directly comparable across any two systems.

The conversion:

$$
\frac{E_b}{N_0} = \text{SNR} \cdot \frac{B}{R_b}
\qquad\text{or, in dB:}\qquad
\left(\frac{E_b}{N_0}\right)_{dB} = \text{SNR}_{dB} + 10\log_{10}\left(\frac{B}{R_b}\right)
$$

**Worked example.** A Sub-GHz link runs 50 kbps in 100 kHz at 9 dB SNR:

$$
\frac{E_b}{N_0} = 9 + 10\log_{10}\left(\frac{100{,}000}{50{,}000}\right) = 9 + 3.01 = 12.0\ \text{dB}
$$

**Worked example.** BLE 1M runs 1 Mbps in 1 MHz at 10 dB SNR:

$$
\frac{E_b}{N_0} = 10 + 10\log_{10}(1) = 10.0\ \text{dB}
$$

Now the comparison is meaningful: the BLE demodulator is about 2 dB more efficient per bit
than the Sub-GHz example — a real difference attributable to implementation, not to a
difference in filter width.

### 4.6 Required Eb/N0 by Modulation

The following are theoretical requirements for a bit error rate of 10⁻⁵ over an additive
white Gaussian noise channel. Real implementations add 1–3 dB.

| Modulation | Eb/N0 for BER 10⁻⁵ | Bits/symbol | Notes |
|---|---|---|---|
| Coherent BPSK | 9.6 dB | 1 | The reference point |
| Coherent QPSK / OQPSK | 9.6 dB | 2 | Same energy efficiency, twice the spectral efficiency |
| Coherent 2-FSK | 12.6 dB | 1 | 3 dB worse than BPSK |
| Non-coherent 2-FSK | 13.4 dB | 1 | The practical Sub-GHz case |
| GFSK, h = 0.5, limiter-discriminator | ~13–14 dB | 1 | BLE-class receivers |
| 16-QAM | 13.4 dB | 4 | High spectral efficiency, needs linear PA |
| 64-QAM | 17.8 dB | 6 | Wi-Fi territory |
| Shannon bound (infinite BW) | −1.59 dB | — | Unreachable ceiling |

Two observations from this table govern much of Part II.

QPSK achieves twice the spectral efficiency of BPSK at *identical* Eb/N0. This is a genuinely
free improvement in the AWGN channel and is why QPSK-family modulations are so widespread.
The catch, as Chapter 7 explains, is envelope variation — which brings back the PA
linearity problem of Section 3.4.

Non-coherent FSK is 3.8 dB worse than coherent BPSK, and Sub-GHz systems use it anyway. The
reason is entirely practical: non-coherent detection requires no carrier phase recovery,
which means it tolerates large frequency offsets, starts up in microseconds rather than
milliseconds, and works with an inexpensive crystal. For a device that wakes for 5 ms every
minute, acquisition speed and BOM cost dominate a 3.8 dB penalty. Chapter 7 works this trade
in detail.

### 4.7 Putting the Ceilings to Work

A specification arrives: *2 Mbps over 500 metres, 868 MHz, battery powered, EU compliant.*
The limits of this chapter let you evaluate it in about two minutes, before any hardware
exists.

**Step 1 — bandwidth.** At 2 Mbps with binary modulation, `R_s` = 2 Msym/s, requiring roughly
2–3 MHz of bandwidth. The entire EU 868 MHz allocation under EN 300 220 is 868.0–868.6 MHz:
600 kHz. The requirement exceeds the available spectrum by a factor of four.

**Step 2 — could higher-order modulation save it?** To fit 2 Mbps into 600 kHz requires a
spectral efficiency of 3.33 bits/s/Hz, implying `M ≥ 16`. From the table, 16-QAM needs 13.4 dB
Eb/N0 and a linear PA.

**Step 3 — the link budget.** Noise floor in 600 kHz at NF = 6 dB is
`−174 + 57.8 + 6 = −110.2 dBm`. Required SNR for 16-QAM at 2 Mbps in 600 kHz is
`13.4 + 10 log(2×10⁶/600×10³) = 13.4 + 5.2 = 18.6 dB`, so sensitivity is `−91.6 dBm`. EU power
is capped at +14 dBm ERP. FSPL at 500 m and 868 MHz is
`20 log(0.5) + 58.77 + 32.44 = 85.2 dB`. Received power: `14 − 85.2 = −71.2 dBm`. Margin:
`−71.2 − (−91.6) = 20.4 dB`.

**Step 4 — the verdict.** The link budget closes with margin in free space. The specification
fails on *spectrum*, not on power: 2 Mbps does not fit in a 600 kHz allocation with any
practical modulation, and a linear PA for 16-QAM would also wreck the battery budget. The
honest answers are to reduce the rate to roughly 250 kbps, or to move to the 2.4 GHz band
where bandwidth is available, or to use the 863–870 MHz wideband allocations under different
regulatory sub-bands with their own duty-cycle constraints.

That analysis required no hardware, no simulation, and no vendor. This is what Part I is
for.

### 4.8 What Carries Forward

- **−174 dBm/Hz** is the thermal noise floor at 290 K.
- **Noise floor = −174 + 10 log(B) + NF**; **sensitivity = noise floor + required SNR**.
- **Nyquist** bounds symbol rate by bandwidth; **Shannon** bounds bit rate by bandwidth *and*
  SNR, linearly in the former and logarithmically in the latter.
- **Eb/N0** is the only fair comparison between modulations; **−1.59 dB** is the absolute
  floor.
- Every PHY is a chosen point in the space bounded by these results. Part II examines the
  choices.

---

# Part II — Modulation and Coding

---

# Chapter 5

## Line Coding and Baseband Signaling

### 5.1 The Problem With Sending Bits Directly

The most obvious way to transmit a bit stream is to hold a high voltage for a 1 and a low
voltage for a 0. This is called **non-return-to-zero (NRZ)**, and it is the natural output of
any digital logic. It is also, on its own, unusable over almost every real channel — for
three reasons that between them motivate the entire subject of line coding.

**It has a DC component.** A long run of 1s produces a sustained non-zero average voltage.
Most channels cannot pass DC: Ethernet couples through transformers, many serial links couple
through capacitors, and every AC-coupled path exhibits **baseline wander**, in which the
average level drifts and the receiver's decision threshold ends up on the wrong side of the
signal. A long enough run of identical bits will cause a receiver to lose the distinction
between high and low entirely.

**It provides no timing information.** A receiver must know where each bit begins. It
recovers this from *transitions* in the signal. NRZ transmitting one thousand consecutive
zeros contains no transitions at all, so the receiver's clock free-runs for a thousand bit
periods. With a 50 ppm oscillator mismatch, a thousand bits accumulates 0.05 bit periods of
error — tolerable. With a 500 ppm mismatch and ten thousand bits, five bit periods — the
receiver is now sampling entirely the wrong bits.

**Its spectrum is badly shaped.** NRZ has most of its energy near DC and a `sinc²` spectrum
with substantial high-frequency tails, which is precisely the opposite of what a band-limited
channel wants.

Line coding is the transformation applied to a bit stream *before* modulation to fix these
problems. It operates at baseband — in the world of ones and zeros and voltage levels — and
is entirely separate from the carrier-frequency modulation of Chapters 6 and 7. Both are
always present in a real system.

### 5.2 The Four Objectives

Every line code is a compromise among four goals, and knowing which goal a code prioritises
tells you where it is used.

**DC balance.** The long-run average of the encoded signal should be zero, so that AC-coupled
paths do not wander. Measured as **running disparity** — the cumulative excess of 1s over 0s.
A code that bounds running disparity to a small number is DC-balanced.

**Transition density.** There should be a guaranteed maximum interval without a transition,
called the **run length limit (RLL)**. This bounds how long the receiver's clock recovery must
coast.

**Bandwidth efficiency.** Encoding overhead costs signalling rate. A code that transmits two
symbols per bit doubles the required bandwidth — a serious cost.

**Error detection and special symbols.** Some codes reserve encoded patterns that never occur
in valid data, allowing them to serve as frame delimiters or to flag errors.

### 5.3 The Principal Codes

**NRZ.** One level per bit. Maximum efficiency (1 symbol/bit), no DC balance, unbounded run
length. Usable only when combined with scrambling (Section 5.5), which is exactly what modern
systems do.

**NRZI (non-return-to-zero inverted).** A 1 causes a *transition*; a 0 causes no change.
Encodes information in changes rather than levels, which makes it immune to signal inversion —
useful when a differential pair may be wired either way round. Still has unbounded run length
on zeros. Used in USB (combined with bit stuffing) and in fiber links.

**Manchester.** Each bit becomes two half-bit levels: a 1 is high-then-low, a 0 is
low-then-high. Guarantees a transition in the centre of every bit, giving perfect clock
recovery and perfect DC balance regardless of data.

The cost is severe: the signalling rate doubles, so a 10 Mbps link occupies the bandwidth of a
20 Mbaud signal. This is why Manchester survives in 10BASE-T Ethernet and in low-rate RFID and
Sub-GHz OOK links, where bandwidth is plentiful relative to rate, and has been abandoned
everywhere that bandwidth is scarce.

**Differential Manchester.** Like Manchester but encodes data in the *presence or absence* of a
transition at the bit boundary, with a mid-bit transition always present for clocking.
Polarity-independent. Used in Token Ring and in some industrial buses.

**4B/5B.** Maps every 4-bit nibble to a 5-bit code word, choosing the 16 code words from the
32 available so that none has more than three consecutive zeros. Overhead is 25% rather than
Manchester's 100%. The 16 unused code words provide control symbols — `/J/K/` start-of-stream
delimiters, `/T/R/` end-of-stream, `/I/` idle. Used in 100BASE-TX and FDDI.

**8B/10B.** Maps 8 bits to 10, with 25% overhead, bounded run length of 5, and — the key
feature — bounded running disparity. The encoder maintains state and selects between two
alternative code words to steer the DC average back toward zero. This makes it genuinely
DC-balanced rather than merely transition-rich. Used in Gigabit Ethernet (1000BASE-X), PCI
Express Gen 1 and 2, SATA, DisplayPort, and Fibre Channel.

**64B/66B and 128B/130B.** At 10 Gbps and above, 25% overhead is intolerable. These codes
prepend a 2-bit synchronisation header to a 64- or 128-bit block and *scramble* the payload
rather than substituting code words. Overhead falls to 3.1% and 1.6% respectively. DC balance
becomes statistical rather than guaranteed — acceptable because at these rates the coupling
components can be designed for it. Used in 10GBASE-R and PCIe Gen 3 onward.

| Code | Overhead | Max run | DC balanced | Where used |
|---|---|---|---|---|
| NRZ | 0% | Unbounded | No | Everywhere, with scrambling |
| NRZI | 0% | Unbounded on 0s | No | USB, fiber |
| Manchester | 100% | 1 bit | Yes, exactly | 10BASE-T, RFID, Sub-GHz OOK |
| Diff. Manchester | 100% | 1 bit | Yes | Token Ring, industrial buses |
| 4B/5B | 25% | 3 bits | Approximately | 100BASE-TX, FDDI |
| 8B/10B | 25% | 5 bits | Yes, bounded disparity | GbE, PCIe 1–2, SATA |
| 64B/66B | 3.1% | Statistical | Statistically | 10GbE and above |

### 5.4 Multilevel Signaling

Everything above is binary — two levels. Increasing the number of levels trades noise margin
for bandwidth, exactly as Section 1.3 predicted.

**MLT-3** uses three levels (−1, 0, +1) and cycles through them on each 1 bit, staying put on
each 0. Because a full cycle takes four bit periods, the fundamental frequency is one quarter
of the bit rate. 100BASE-TX combines 4B/5B with MLT-3 to fit 100 Mbps into 31.25 MHz of
bandwidth on Category 5 cable — which is the reason 100 Mbps Ethernet works on cable rated to
100 MHz.

**PAM-5** uses five levels. 1000BASE-T transmits on all four pairs simultaneously, in both
directions, at 125 Mbaud with PAM-5, giving 4 × 125 × 2 = 1000 Mbps. The fifth level provides
redundancy for a four-dimensional trellis code that recovers roughly 6 dB of coding gain.

**PAM-16 and beyond.** 10GBASE-T uses PAM-16 with DSQ128 coding and LDPC forward error
correction. Sixteen levels in the same voltage range means the spacing between adjacent
levels is one fifteenth of what a binary scheme would have — a 23.5 dB loss in noise margin,
recovered by heavy coding and sophisticated equalisation. This is why 10GBASE-T PHYs consume
several watts while 1000BASE-T PHYs consume a fraction of one.

The pattern is now familiar: **every level added buys bandwidth and costs SNR.** Chapter 4
made this a theorem; here it is a product line.

### 5.5 Scrambling: Statistical Rather Than Structural

Substitution codes such as 8B/10B guarantee their properties but pay a fixed overhead.
**Scrambling** achieves similar properties with essentially zero overhead, at the cost of
guaranteeing nothing — only making failure improbable.

A scrambler XORs the data stream with a pseudo-random sequence generated by a **linear
feedback shift register (LFSR)**. The receiver runs an identical LFSR and XORs again,
recovering the original data because `(D ⊕ P) ⊕ P = D`.

The effect on a pathological input — say, ten thousand consecutive zeros — is to convert it
into a pseudo-random sequence with the transition density and DC balance of random data. The
effect on a genuinely random input is negligible. Scrambling does not compress, does not add
redundancy, and does not detect errors. It only removes structure.

Two forms exist, and the distinction matters:

**Self-synchronising (multiplicative) scramblers** feed the transmitted data back into the
shift register. The receiver synchronises automatically without needing to know the initial
state — but a single channel error propagates into several output errors, one for each
feedback tap. Used in SONET/SDH and in 64B/66B.

**Additive (frame-synchronous) scramblers** run the LFSR independently of the data and require
the receiver to know the seed. No error multiplication. This is the form used in packet
radio, where the seed can be derived from something both ends know — such as the channel
index.

In wireless PHYs this operation is called **whitening**, and Chapter 9 develops it with the
BLE implementation as the worked example. It is the same mechanism under a different name.

### 5.6 Line Coding in Sub-GHz and BLE Systems

Low-power radio systems make a characteristic set of choices, and it is worth stating them
plainly because they differ from the wired world.

**NRZ plus whitening is the dominant combination.** Nearly every Sub-GHz FSK transceiver and
BLE radio transmits NRZ data through a whitener. There is no substitution code, because the
25% overhead of an 8B/10B-class code would directly cost 25% of the link's energy budget, and
because the packets are short enough that statistical DC balance is entirely adequate.

**Manchester is offered but rarely used.** Most Sub-GHz transceivers include a Manchester
encoder in hardware. It appears in OOK links — where DC balance genuinely matters because the
receiver uses an automatic-gain-controlled envelope detector whose threshold depends on the
average level — and in legacy protocols. For FSK it is normally left disabled, because halving
the data rate for the same bandwidth is a poor trade when whitening solves the same problem
free.

**Clock recovery relies on the preamble, not on the code.** A Sub-GHz packet begins with a
preamble of alternating 1s and 0s — typically 4 to 8 bytes of `0x55` or `0xAA`. Under 2-FSK
this produces a clean alternating tone pair, which is the ideal input for a timing recovery
loop. The receiver locks its symbol clock during the preamble and then coasts, aided by
whitening, through the payload. Preamble length is therefore a direct trade between
acquisition reliability and airtime, and Chapter 18 treats it as a design parameter.

**BLE is a special case worth noting.** The BLE 1M and 2M PHYs use NRZ with whitening and a
fixed 8-bit (1M) or 16-bit (2M) preamble. The Coded PHY adds forward error correction and a
pattern mapper on top, which is covered in Chapter 9.

### 5.7 What Carries Forward

- NRZ alone fails on DC balance and clock recovery; every real system fixes this with
  scrambling, a substitution code, or both.
- Substitution codes guarantee properties at fixed overhead; scrambling achieves them
  statistically at zero overhead.
- Multilevel signalling trades noise margin for bandwidth, and 10GBASE-T is the extreme case.
- Low-power radio uses NRZ plus whitening and recovers timing from an explicit preamble.

Line coding has now shaped the bit stream. Chapter 6 puts it on a carrier.

---

# Chapter 6

## Digital Modulation I: ASK, FSK, PSK, and the Constellation

### 6.1 Why a Carrier Is Necessary

Baseband signalling — putting the coded bit stream directly onto the medium — works for
copper over short distances and for fiber. It does not work for radio, for three independent
reasons, any one of which would be sufficient.

**Antennas.** Efficient radiation requires an antenna comparable to a quarter wavelength. A
1 kHz baseband signal has a wavelength of 300 km, requiring a 75 km antenna. Shifting the same
information onto an 868 MHz carrier reduces the wavelength to 34.6 cm and the quarter-wave
antenna to 8.6 cm — a component that fits on a circuit board. At 2.44 GHz it is 3.07 cm, small
enough to print as a trace.

**Spectrum sharing.** If every transmitter emitted at baseband, every transmitter would occupy
the same frequencies and none would be separable. Carriers allow simultaneous independent
links by assigning each to a different part of the spectrum. The entire concept of a channel
depends on it.

**Propagation.** Different frequencies propagate differently — through walls, around
obstacles, over the horizon. Choosing a carrier is choosing a propagation regime, which
Chapter 12 examines in detail.

The carrier itself is a pure sinusoid and carries no information. Information is imposed by
varying one of its three parameters, and as established in Section 2.2 there are exactly
three:

$$
s(t) = A(t)\cdot\cos\big(2\pi f_c t + \phi(t)\big)
$$

Vary `A` → amplitude shift keying. Vary `f_c` → frequency shift keying. Vary `φ` → phase shift
keying. Vary `A` and `φ` jointly → quadrature amplitude modulation. That exhausts the
possibilities. Everything in this chapter and the next is a refinement of one of these four.

### 6.2 The I/Q Representation

Before examining the schemes individually, one mathematical tool makes all of them a single
subject.

Any modulated signal can be written as the sum of two carriers ninety degrees apart:

$$
s(t) = I(t)\cos(2\pi f_c t) - Q(t)\sin(2\pi f_c t)
$$

`I` is the **in-phase** component, `Q` the **quadrature** component. Because sine and cosine at
the same frequency are orthogonal, a receiver can separate them perfectly, and two independent
information streams can share one carrier.

The relationship to amplitude and phase is straightforward polar-to-Cartesian conversion:

$$
A(t) = \sqrt{I^2 + Q^2} \qquad\qquad \phi(t) = \arctan\!\left(\frac{Q}{I}\right)
$$

This is not merely notation. It is how radios are actually built. A modern transceiver
contains an I/Q modulator on the transmit side and an I/Q demodulator on the receive side, and
firmware manipulates I and Q samples rather than amplitudes and phases. Every modulation in
this book is, at the silicon level, a rule for generating a sequence of `(I, Q)` pairs.

Plotting those pairs on a plane gives the **constellation diagram**, which is the single most
useful visualisation in digital communications. Each valid symbol is a point. The receiver
observes a noisy version of a point and decides which constellation point was most likely
transmitted — normally by choosing the nearest one.

That immediately gives the governing principle of modulation design:

> **The probability of a symbol error is determined by the minimum Euclidean distance between
> constellation points, relative to the noise standard deviation.**

Packing more points into the plane raises the bit rate and shrinks the minimum distance.
Chapter 8 turns this observation into exact error probabilities.

### 6.3 Amplitude Shift Keying

ASK varies carrier amplitude. Its binary form, **on-off keying (OOK)**, transmits the carrier
for a 1 and nothing for a 0.

$$
s(t) = \begin{cases} A\cos(2\pi f_c t) & \text{for bit} = 1 \\ 0 & \text{for bit} = 0\end{cases}
$$

Its constellation is two points on the I axis, at 0 and at A.

**The case for OOK** is entirely economic. The transmitter is a switch. The receiver is a
diode, a capacitor, and a comparator — an envelope detector requiring no carrier recovery, no
phase reference, and almost no current. An OOK receiver can be built for pennies and can run
in a duty-cycled "sniff" mode at microamps. This is why OOK persists in 433 MHz garage door
openers, tyre pressure monitors, and passive RFID, and why it will continue to.

**The case against OOK** is that amplitude is the one signal parameter that the channel
attacks hardest. Fading, shadowing, distance variation, and interference all change amplitude.
An envelope detector must maintain a decision threshold between "carrier present" and "carrier
absent", and that threshold depends on received power, which is exactly what is varying. Long
runs of zeros mean no signal at all, so automatic gain control has nothing to track and the
threshold drifts. This is why OOK systems mandate Manchester coding (Section 5.6) — it
guarantees the 50% duty cycle that keeps AGC stable.

Energy efficiency is also poor. For a given *average* power, OOK's constellation points are
closer together than a scheme that uses the full amplitude range in both states, costing
roughly 3 dB relative to antipodal signalling.

### 6.4 Frequency Shift Keying

FSK varies the carrier frequency. Binary FSK transmits one of two tones:

$$
s(t) = A\cos\big(2\pi (f_c \pm f_d) t\big)
$$

where `f_d` is the **frequency deviation** and the total separation between tones is `2f_d`.

FSK is the workhorse of low-power radio, and the reasons are worth being explicit about
because they explain almost every Sub-GHz design decision in this book.

**Constant envelope.** Amplitude never changes. The peak-to-average power ratio is exactly
1 (0 dB). This means the power amplifier can be driven into saturation, where it is most
efficient — 50–60% rather than the 15–25% typical of a backed-off linear amplifier. For a
battery-powered node this is not a refinement; it is the difference between a two-year and a
six-month battery life.

**Immunity to amplitude impairment.** Because information lives in frequency, anything that
attacks amplitude — fading, AGC error, nonlinear compression — leaves the data untouched. A
limiter can be placed in the receive path to strip amplitude variation entirely before
demodulation.

**Non-coherent detection is possible.** The receiver does not need to recover the carrier's
phase. It can simply measure instantaneous frequency, using a limiter-discriminator or a
digital arctangent-differentiator. This eliminates the phase-locked loop, its acquisition
time, and its sensitivity to frequency offset. A non-coherent FSK receiver can acquire a
packet in a few symbol periods, which is what makes microsecond-scale duty cycling possible.

The cost, from the table in Section 4.6, is 3.8 dB of Eb/N0 relative to coherent BPSK. For
mains-powered systems that is a poor trade. For a sensor waking for 5 ms per minute from a
coin cell, it is an obvious one.

**Deviation and modulation index.** Section 2.6 defined `h = 2f_d/R_s`. The two tones become
orthogonal — maximally distinguishable for a given separation — at `h = 0.5`, and this is the
most common design point. Raising `h` widens the spectrum but increases tolerance to carrier
frequency error, which matters when both ends run cheap crystals. At 868 MHz, two nodes each
with ±20 ppm crystals can differ by 34.7 kHz; if the deviation is only 25 kHz, that offset
exceeds the signal's own frequency structure and the link will not close without correction.
This is a real and frequent field failure, and Chapter 24 covers diagnosing it.

**4-FSK** extends the idea to four tones carrying two bits each, halving the symbol rate for a
given bit rate. It appears in 802.15.4g and in some proprietary long-range Sub-GHz systems.
The penalty is roughly 3–4 dB of required SNR and tighter frequency accuracy requirements.

### 6.5 Phase Shift Keying

PSK varies carrier phase.

**BPSK** uses two phases, 0° and 180°:

$$
s(t) = A\cos(2\pi f_c t + \pi b), \quad b \in \{0, 1\}
$$

Its constellation is two points at `(+A, 0)` and `(−A, 0)` — **antipodal**, meaning separated by
the maximum possible distance for a given power. This is why BPSK is the reference against
which all other modulations are measured: no binary scheme can do better in additive white
Gaussian noise.

**QPSK** uses four phases at 45°, 135°, 225°, 315°, carrying two bits per symbol. The
remarkable property established in Section 4.6 is that QPSK requires *exactly the same* Eb/N0
as BPSK for the same bit error rate. It achieves double the spectral efficiency for free.

The reason is geometric. QPSK is two independent BPSK signals on the orthogonal I and Q
carriers. Each carries half the total power and half the total bits, so the energy per bit is
unchanged, and the two streams do not interfere because I and Q are orthogonal. The apparent
free lunch is real — in AWGN, with a perfectly linear transmitter.

The catch is envelope variation, and it is a serious one. Moving between diagonally opposite
QPSK points passes through the origin, meaning the carrier amplitude drops to zero and back.
This produces a peak-to-average power ratio around 3–4 dB, requiring the amplifier to back off
into its linear region and forfeiting the efficiency that FSK enjoys. Section 7.4 shows how
OQPSK largely repairs this.

**Differential PSK (DPSK)** encodes data in the *change* of phase rather than in absolute
phase. This removes the need for an absolute phase reference — a genuine simplification —
at a cost of roughly 1 dB, because each decision depends on two noisy symbols instead of one.
Errors also tend to occur in pairs.

**Gray coding.** In any constellation with more than two points, the bit-to-symbol mapping
should be chosen so that adjacent points differ in exactly one bit. Because noise most often
causes confusion between *adjacent* points, Gray coding ensures that the most probable symbol
error produces only a single bit error. For QPSK this reduces the bit error rate by
approximately a factor of two relative to a naive mapping, at zero cost. There is never a
reason not to Gray code.

### 6.6 Comparing the Three

| Property | ASK / OOK | FSK | PSK |
|---|---|---|---|
| Parameter varied | Amplitude | Frequency | Phase |
| Envelope | Varies fully | Constant | Constant (BPSK/QPSK ideal) |
| PAPR | High | 0 dB | 0 dB ideal, 3–4 dB with filtering |
| PA efficiency | Poor | Excellent | Moderate |
| Non-coherent detection | Yes, trivially | Yes | No (DPSK approximates) |
| Eb/N0 for BER 10⁻⁵ | ~13.4 dB (non-coh.) | 13.4 dB (non-coh. binary) | 9.6 dB (coherent BPSK) |
| Tolerance to fading | Poor | Good | Good |
| Tolerance to freq. error | Excellent | Moderate, set by `h` | Poor |
| Receiver complexity | Minimal | Low | Moderate to high |
| Typical use | RFID, 433 MHz remotes | Sub-GHz IoT, BLE | Satellite, cellular, 802.15.4 |

The table explains the market. OOK wins on cost alone. PSK wins on energy efficiency alone.
FSK wins on the combination that a battery-powered radio actually faces — decent efficiency,
constant envelope, non-coherent detection, and tolerance to a cheap crystal — which is why it
dominates the Sub-GHz and BLE world despite being theoretically inferior to PSK.

### 6.7 What Carries Forward

- All modulation is amplitude, frequency, phase, or a combination; the I/Q representation
  unifies them.
- Error probability is governed by minimum Euclidean distance in the constellation.
- Constant envelope enables saturated power amplifiers, which is the decisive advantage for
  battery-powered radio.
- Non-coherent detection trades ~3.8 dB of Eb/N0 for the elimination of carrier recovery, fast
  acquisition, and tolerance to frequency error.

Chapter 7 takes these four basic schemes and shows what must be added to make them legal,
efficient, and manufacturable.

---

# Chapter 7

## Digital Modulation II: GFSK, MSK, OQPSK, DSSS, and the Sub-GHz Choice

### 7.1 The Problem With Ideal Modulation

Chapter 6 described the modulation schemes as textbooks present them: instantaneous
transitions between constellation points. Real transmitters cannot do this, and should not
want to.

An instantaneous transition is a discontinuity, and Section 2.3 established that
discontinuities require infinite bandwidth. A transmitter switching abruptly between two FSK
tones produces **spectral splatter** — energy spread far outside the intended channel, into
frequencies allocated to other users. Every regulator in the world specifies a **spectral
mask** limiting how much power may appear at a given offset from the carrier, and unfiltered
modulation fails it comprehensively.

The fix is **pulse shaping**: filtering the baseband data before it reaches the modulator, so
that transitions become smooth rather than abrupt. This chapter is largely about the
consequences of that filtering, and the schemes that were designed around it.

The trade is immediate and unavoidable. Smoothing the transitions narrows the spectrum but
spreads each symbol in time, so adjacent symbols begin to overlap — **inter-symbol
interference**. Every pulse-shaping filter is a chosen point between splatter and ISI.

### 7.2 Gaussian Filtering and GFSK

The Gaussian filter is the standard choice for frequency-based modulation, because it has no
overshoot and the narrowest possible time-bandwidth product. Applying it to the data before an
FSK modulator gives **Gaussian frequency shift keying (GFSK)**, which is the modulation used by
Bluetooth, BLE, and the overwhelming majority of Sub-GHz transceivers.

The filter is specified by the dimensionless **BT product** — the filter's 3 dB bandwidth
multiplied by the symbol period:

$$
BT = B_{filter} \cdot T_{symbol}
$$

| BT | Filtering | Spectrum | ISI | Used by |
|---|---|---|---|---|
| ∞ | None (raw FSK) | Widest | None | Legacy, low-rate |
| 1.0 | Light | Wide | Negligible | Some Sub-GHz |
| 0.5 | Moderate | Narrow | Mild, ~1 symbol spread | **BLE**, most Sub-GHz |
| 0.3 | Heavy | Narrowest | Significant, ~3 symbol spread | **GSM (GMSK)** |

A lower BT confines more energy inside the channel but smears each symbol across more of its
neighbours. GSM chose BT = 0.3 because spectral efficiency was paramount in a cellular system
with reused channels, and accepted an equaliser in every handset to undo the resulting ISI.
BLE chose BT = 0.5 because it wanted a receiver simple enough to run without an equaliser.
That single parameter choice cascades into the entire receiver architecture.

**BLE's parameters concretely:** GFSK, BT = 0.5, modulation index nominally 0.5 (the
specification permits 0.45 to 0.55), symbol rate 1 Msym/s for the 1M PHY, giving a nominal
deviation of 250 kHz and an occupied bandwidth near 1.1 MHz inside a 2 MHz channel.

**Sub-GHz practice:** BT = 0.5 with `h` between 0.5 and 1.0. Higher `h` is chosen where crystal
tolerance is loose or where the receiver uses a simple discriminator; lower `h` where channel
spacing is tight.

### 7.3 MSK: The Special Case at h = 0.5

When continuous-phase FSK is operated at exactly `h = 0.5`, something structurally
interesting happens. The phase advances or retards by exactly 90° per symbol, and the
resulting signal is simultaneously describable as frequency modulation *and* as offset
quadrature phase modulation. This is **minimum shift keying (MSK)**.

MSK is significant for three reasons:

- It is the minimum-bandwidth continuous-phase FSK that keeps the two tones orthogonal —
  hence the name.
- Its phase is continuous, so there are no discontinuities and the spectral tails fall much
  faster than ordinary FSK.
- Its envelope is exactly constant, so it retains the full PA-efficiency advantage.

Applying a Gaussian filter to MSK gives **GMSK**, the modulation of GSM. Applying a half-sine
pulse shape to OQPSK gives a signal mathematically identical to MSK — which is why the
802.15.4 2.4 GHz PHY, nominally described as OQPSK, is in practice often implemented and
demodulated as MSK. These are not three modulations; they are three descriptions of nearly the
same waveform, and recognising that saves a great deal of confusion when reading standards
documents.

### 7.4 OQPSK: Repairing QPSK's Envelope

Section 6.5 identified QPSK's defect: transitions between diagonally opposite constellation
points pass through the origin, driving the envelope to zero and forcing PA backoff.

**Offset QPSK** fixes this with a change of remarkable simplicity. The Q-channel bit stream is
delayed by half a symbol period relative to the I-channel. Because I and Q now never change at
the same instant, the signal can never make a 180° transition — only ±90° transitions are
possible, and those pass around the origin rather than through it.

| Scheme | Max phase jump | Envelope minimum | PAPR | PA backoff |
|---|---|---|---|---|
| QPSK | 180° | Zero | 3–4 dB | 6–8 dB |
| OQPSK | 90° | Non-zero | ~1–2 dB | 2–3 dB |
| OQPSK, half-sine shaped (= MSK) | 90° continuous | Constant | 0 dB | ~0 dB |

Critically, OQPSK achieves this at **no cost in Eb/N0**. The I and Q streams are still
independent BPSK signals; delaying one does not change either one's error probability. It is
one of the genuinely free improvements in communications engineering.

Its principal deployment is IEEE 802.15.4 in the 2.4 GHz band — the PHY beneath Zigbee,
Thread, and Matter — where it is combined with direct sequence spreading.

### 7.5 Direct Sequence Spread Spectrum

DSSS multiplies each data bit by a much faster pseudo-random **chip** sequence. The
transmitted bandwidth expands by the ratio of chip rate to bit rate, and the receiver
correlates against the known sequence to recover the data.

The benefit is **processing gain**:

$$
G_p = 10\log_{10}\left(\frac{R_{chip}}{R_{bit}}\right)\ \text{dB}
$$

Correlation coherently accumulates the wanted signal while noise and uncorrelated interference
add incoherently, so the effective SNR after despreading exceeds the SNR on the air by `G_p`.

**Worked example — 802.15.4 at 2.4 GHz.** Four data bits select one of sixteen 32-chip
pseudo-noise sequences. The chip rate is 2 Mchip/s and the resulting bit rate is 250 kbps:

$$
G_p = 10\log_{10}\left(\frac{2 \times 10^6}{250 \times 10^3}\right) = 10\log_{10}(8) = 9.0\ \text{dB}
$$

Nine decibels of interference rejection, which is what allows Zigbee and Thread to survive in
a 2.4 GHz band saturated by Wi-Fi.

DSSS also provides resistance to narrowband jamming (a narrow interferer is spread out by the
receiver's despreading operation) and to multipath (delayed copies decorrelate from the
sequence). Its costs are bandwidth — considerably more than the data requires — and receiver
complexity.

**LoRa and chirp spread spectrum** deserve mention as the other major spreading approach used
in Sub-GHz. Instead of a pseudo-random chip sequence, LoRa modulates data onto a linear
frequency chirp sweeping across the channel. The **spreading factor** SF, from 7 to 12, sets
the number of chips per symbol as `2^SF`:

$$
R_s = \frac{BW}{2^{SF}} \qquad\qquad R_b = SF \cdot \frac{BW}{2^{SF}} \cdot CR
$$

At SF12 with 125 kHz bandwidth, the symbol rate is `125000/4096 ≈ 30.5 sym/s` and the raw bit
rate is `12 × 30.5 ≈ 366 bps` before coding. In exchange, sensitivity reaches roughly −137 dBm
— far below the thermal noise floor in that bandwidth — enabling kilometre-scale links at
+14 dBm. The trade is stark and explicit: **airtime for range.** A single SF12 packet can
occupy over a second, which under a 1% duty-cycle regulation means roughly 36 such packets per
hour. Chapter 15 returns to this constraint.

### 7.6 Why Sub-GHz Avoids QAM

Quadrature amplitude modulation varies amplitude and phase together, achieving the highest
spectral efficiency available. It is used in Wi-Fi, cable, cellular downlinks, and every
system where bandwidth is the binding constraint. It is almost never used in Sub-GHz IoT, and
the reasons compound.

**PA linearity.** 16-QAM has a PAPR around 5–6 dB and requires 6–8 dB of amplifier backoff. A
saturated PA delivering +14 dBm at 55% efficiency draws roughly 45 mW from the supply; a
backed-off linear PA delivering the same +14 dBm at 15% efficiency draws roughly 165 mW.
Nearly four times the transmit current, for a device whose entire power budget is a coin cell.

**Phase noise.** QAM constellation points are distinguished by phase as well as amplitude, so
oscillator phase noise directly rotates them toward their neighbours. The low-cost fractional-N
synthesisers in Sub-GHz transceivers have phase noise adequate for FSK and marginal for
16-QAM.

**Frequency offset.** A 20 ppm crystal error at 868 MHz is 17.4 kHz. A coherent QAM receiver
must track and remove this before it can make decisions, requiring a carrier recovery loop
with acquisition time measured in hundreds of symbols. At 50 kbps that is milliseconds of
receive-on time per packet, which for a duty-cycled node is the dominant energy cost.

**Amplitude fading.** QAM encodes information in amplitude, and the wireless channel attacks
amplitude. Recovering QAM through a fading channel requires channel estimation and
equalisation — hardware that does not exist in a low-power transceiver.

**And, decisively, the bandwidth QAM buys is not available.** Sub-GHz allocations are tens to
hundreds of kilohertz wide. Spectral efficiency is not the binding constraint; regulation,
sensitivity, and energy are. QAM optimises the one resource that Sub-GHz systems have no
shortage of relative to their needs, while consuming the three they are short of.

This is the clearest illustration in the book of a central theme: **the best modulation is not
the most efficient one, but the one whose costs align with the resources you actually have.**

### 7.7 Comparative Summary

| Modulation | Bits/sym | PAPR | Detection | Rel. Eb/N0 | Primary application |
|---|---|---|---|---|---|
| OOK | 1 | High | Non-coherent envelope | +4 dB | RFID, 433 MHz remotes |
| 2-FSK | 1 | 0 dB | Non-coherent | +3.8 dB | Sub-GHz IoT |
| GFSK (BT 0.5, h 0.5) | 1 | 0 dB | Non-coherent | +3.5–4.5 dB | **BLE, Sub-GHz** |
| GMSK (BT 0.3) | 1 | 0 dB | Coherent, equalised | +0.5 dB | GSM |
| 4-FSK | 2 | 0 dB | Non-coherent | +7 dB | 802.15.4g long range |
| BPSK | 1 | 0 dB | Coherent | 0 dB (reference) | Satellite, deep space |
| QPSK | 2 | 3–4 dB | Coherent | 0 dB | Cellular, satellite |
| OQPSK + DSSS | 2 | 1–2 dB | Coherent | 0 dB, +9 dB proc. gain | **802.15.4 / Zigbee / Thread** |
| LoRa CSS SF12 | — | 0 dB | Correlation | −20 dB effective | LPWAN long range |
| 16-QAM | 4 | 5–6 dB | Coherent + equalised | +3.8 dB | Wi-Fi, cable |
| 64-QAM | 6 | 7–8 dB | Coherent + equalised | +8.2 dB | Wi-Fi, LTE downlink |

### 7.8 What Carries Forward

- Pulse shaping is mandatory for regulatory compliance and trades spectral containment
  against ISI; BT is the dial.
- GFSK at BT = 0.5, h = 0.5 is the dominant low-power modulation, and MSK/GMSK/OQPSK are
  closely related descriptions of nearly the same waveform.
- OQPSK repairs QPSK's envelope problem at zero energy cost.
- Spreading buys interference immunity and sensitivity at the price of bandwidth or airtime.
- Sub-GHz avoids QAM because QAM's costs — PA linearity, phase noise, acquisition time —
  attack precisely the resources a battery-powered node is short of.

Chapter 8 converts these choices into numbers: given a modulation and an Eb/N0, what is the
error rate, and what sensitivity figure appears on the datasheet?

---

# Chapter 8

## Bit Errors: From Eb/N0 to a Sensitivity Number

### 8.1 Closing the Loop

Chapter 4 gave the theoretical ceilings. Chapters 6 and 7 gave the modulation choices. This
chapter connects them to the number that actually appears on the front page of a transceiver
datasheet — the sensitivity figure — and to the number an engineer must meet in the field: the
packet error rate.

The chain runs in one direction, and being able to walk it in both is the practical skill this
chapter exists to build:

> modulation → BER equation → required Eb/N0 → required SNR → sensitivity → link budget → range

### 8.2 The Q Function

Every bit error probability in additive white Gaussian noise is expressed through one
function. `Q(x)` is the probability that a standard normal random variable exceeds `x`:

$$
Q(x) = \frac{1}{\sqrt{2\pi}}\int_x^{\infty} e^{-t^2/2}\,dt
$$

Its meaning in this context is direct. A constellation point sits at some distance from the
decision boundary. Noise displaces the received point by a Gaussian random amount. `Q` is the
probability that the displacement is large enough to cross the boundary.

It has no closed form, but the following values cover practical work:

| x | Q(x) |
|---|---|
| 3.09 | 10⁻³ |
| 3.72 | 10⁻⁴ |
| 4.27 | 10⁻⁵ |
| 4.75 | 10⁻⁶ |
| 5.20 | 10⁻⁷ |
| 6.00 | 10⁻⁹ |

Note how steeply it falls. Between `Q = 10⁻³` and `Q = 10⁻⁶`, the argument changes by only 54%
— roughly 3.7 dB in power. This is the origin of the **waterfall** shape of BER curves, and of
the practical experience that a wireless link goes from unusable to perfect over a very small
change in position.

### 8.3 Bit Error Probability by Modulation

**Coherent BPSK** — the reference. Antipodal signalling gives the maximum possible distance
between two points:

$$
P_b = Q\!\left(\sqrt{\frac{2E_b}{N_0}}\right)
$$

**Coherent QPSK / OQPSK** — identical, because QPSK is two orthogonal BPSK channels:

$$
P_b = Q\!\left(\sqrt{\frac{2E_b}{N_0}}\right)
$$

**Coherent orthogonal binary FSK** — the two tones are orthogonal rather than antipodal, which
costs a factor of two in the argument, i.e. exactly 3 dB:

$$
P_b = Q\!\left(\sqrt{\frac{E_b}{N_0}}\right)
$$

**Non-coherent binary FSK** — the practical Sub-GHz case, with a closed form:

$$
P_b = \frac{1}{2}\exp\!\left(-\frac{E_b}{2N_0}\right)
$$

**Differential BPSK:**

$$
P_b = \frac{1}{2}\exp\!\left(-\frac{E_b}{N_0}\right)
$$

**Square M-QAM**, approximately, with Gray coding:

$$
P_b \approx \frac{4}{\log_2 M}\left(1 - \frac{1}{\sqrt{M}}\right)Q\!\left(\sqrt{\frac{3\log_2 M}{M-1}\cdot\frac{E_b}{N_0}}\right)
$$

**Worked verification — non-coherent FSK at BER 10⁻³.** Rearranging:

$$
\frac{E_b}{N_0} = -2\ln(2 P_b) = -2\ln(2 \times 10^{-3}) = -2(-6.215) = 12.43 \Rightarrow 10.9\ \text{dB}
$$

**Worked verification — coherent BPSK at BER 10⁻⁵.** From the table, `Q(4.27) = 10⁻⁵`, so:

$$
\sqrt{2E_b/N_0} = 4.27 \Rightarrow E_b/N_0 = 9.12 \Rightarrow 9.6\ \text{dB}
$$

which confirms the reference value used throughout Chapter 4.

### 8.4 Implementation Loss

The equations above describe a perfect receiver. Real receivers fall short, and the gap is
called **implementation loss**. Its components:

| Source | Typical penalty |
|---|---|
| Non-ideal filtering, ISI from pulse shaping | 0.5–1.5 dB |
| Imperfect symbol timing recovery | 0.3–1.0 dB |
| Residual frequency offset | 0.5–2.0 dB |
| Phase noise | 0.2–1.0 dB |
| ADC quantisation | 0.2–0.5 dB |
| Automatic gain control error | 0.2–0.5 dB |
| **Typical total** | **2–4 dB** |

A well-engineered low-power transceiver achieves 2–3 dB of implementation loss. A poor design,
or a good design misconfigured, shows 6 dB or more — and misconfiguration is far more common
than bad silicon. If measured sensitivity is 6 dB from theory, the first suspects are receiver
bandwidth set wider than necessary, deviation mismatched between the two ends, and crystal
error, in that order.

This is the practical value of being able to compute the theoretical number: it turns
"sensitivity seems poor" into "sensitivity is 7 dB worse than it should be, which is too much
to be implementation loss, so something is wrong."

### 8.5 From Bit Errors to Packet Errors

Users and higher layers care about packets, not bits. If bit errors are independent, a packet
of `N` bits survives only if every bit survives:

$$
\text{PER} = 1 - (1 - \text{BER})^N
$$

and for small BER this is well approximated by `PER ≈ N × BER`.

The dependence on packet length is the important consequence, and it is stronger than
intuition suggests:

| Payload | Bits | BER for 1% PER |
|---|---|---|
| 8 bytes | 64 | 1.6 × 10⁻⁴ |
| 32 bytes | 256 | 3.9 × 10⁻⁵ |
| 127 bytes | 1016 | 9.9 × 10⁻⁶ |
| 255 bytes | 2040 | 4.9 × 10⁻⁶ |
| 1500 bytes | 12000 | 8.4 × 10⁻⁷ |

*Verification of the 255-byte row:* `BER = 1 − 0.99^(1/2040) = 1 − exp(ln(0.99)/2040) = 4.93 × 10⁻⁶`.

A 255-byte packet requires a bit error rate 32 times lower than an 8-byte packet for the same
packet success rate — worth about 1.5 dB of link margin. **Long packets need more SNR than
short ones.** This is why long-range protocols use short frames, why fragmentation sometimes
improves throughput on a marginal link even though it adds overhead, and why a link that
carries acknowledgements perfectly may fail on data.

### 8.6 How Standards Define Sensitivity

Sensitivity is not a physical constant; it is a measurement made under a defined condition,
and the conditions differ between standards. Comparing numbers across standards without
checking the definition is a common error.

| Standard | Criterion | Required minimum |
|---|---|---|
| Bluetooth LE | PER ≤ 30.8% with a 37-byte reference packet | −70 dBm |
| IEEE 802.15.4 (2.4 GHz) | PER ≤ 1% with a 20-byte PSDU | −85 dBm |
| Sub-GHz, vendor practice | BER = 10⁻³, or PER = 1% | −100 to −125 dBm |

The BLE figure is worth dwelling on. The specification demands only −70 dBm, while real
silicon achieves −95 to −97 dBm. The specification is a floor for interoperability, not a
description of the state of the art, and a product designed to the specification minimum
would have roughly one thirtieth the range of one designed to typical silicon capability.
Always design to the datasheet number, and always verify it (Chapter 23).

Note also that BLE's 30.8% PER criterion is far looser than 802.15.4's 1%. A BLE sensitivity
figure is therefore optimistic relative to an 802.15.4 figure by several dB before any
hardware difference is considered.

### 8.7 The Fading Channel Changes Everything

All of the above assumes additive white Gaussian noise — a channel that only adds noise. The
wireless channel also *fades*, and the effect on error rate is severe enough that it
invalidates AWGN intuition entirely.

In a Rayleigh fading channel, the received amplitude is itself a random variable. Averaging
the BER expression over that distribution changes its character completely. For coherent BPSK,
at high SNR:

$$
P_b \approx \frac{1}{4\,\overline{\gamma}}
$$

where `γ̄` is the average SNR. The error rate now falls only **inversely** with SNR, not
exponentially.

The consequence, in numbers:

| Target BER | AWGN Eb/N0 | Rayleigh Eb/N0 | Penalty |
|---|---|---|---|
| 10⁻² | 4.3 dB | 14 dB | 9.7 dB |
| 10⁻³ | 6.8 dB | 24 dB | 17.2 dB |
| 10⁻⁴ | 8.4 dB | 34 dB | 25.6 dB |
| 10⁻⁵ | 9.6 dB | 44 dB | 34.4 dB |

Thirty-four decibels. No practical system pays this. What systems do instead is refuse to
operate in a single fading realisation, using **diversity** to obtain several independent
looks at the channel:

- **Frequency diversity** — retry on a different channel. This is what BLE's adaptive frequency
  hopping provides, and why it is not optional.
- **Time diversity** — retry later, after the channel has changed. Requires the retry interval
  to exceed the channel coherence time.
- **Spatial diversity** — two antennas separated by more than half a wavelength experience
  substantially independent fades. At 868 MHz that is 17 cm; at 2.44 GHz it is 6 cm, which is
  why antenna diversity is cheap at 2.4 GHz and awkward in Sub-GHz.
- **Coding diversity** — spread the codeword over enough time or frequency that a fade damages
  only a correctable fraction. This is the subject of Chapter 9.

With `L`-fold diversity the error rate falls as `γ̄^(−L)`, which recovers most of the loss. The
practical rule to carry into Part IV: **a fading margin of 10–20 dB must be included in any
link budget for a non-diverse system**, and Chapter 13 does exactly that.

### 8.8 What Carries Forward

- `Q(x)` governs all AWGN error probabilities; BER curves are steep, so links transition from
  broken to perfect over a few dB.
- Implementation loss of 2–4 dB is normal; more than 6 dB indicates a configuration fault.
- `PER ≈ N × BER` — long packets demand materially better SNR.
- Sensitivity definitions differ by standard; read the criterion before comparing numbers.
- Rayleigh fading costs 17–34 dB unless diversity is provided. Diversity is not an
  optimisation; it is a requirement.

---

# Chapter 9

## Error Control: Whitening, CRC, FEC, Interleaving, and the Coded PHY

### 9.1 Where Error Control Sits

The channel has damaged the signal and the demodulator has produced a bit stream containing
errors. Three distinct mechanisms respond, and they are frequently confused:

- **Whitening** removes structure from the data. It corrects nothing. Its purpose is to make
  the *modulated signal* well-behaved.
- **Error detection** — the CRC — determines whether a received frame is intact. It corrects
  nothing either; it tells the layer above whether to discard.
- **Forward error correction** adds redundancy that allows a limited number of errors to be
  *repaired* without retransmission.

The processing order in a transmitter is fixed and worth memorising, because reversing any two
stages breaks the link in ways that are difficult to diagnose:

```
payload → CRC append → whiten → FEC encode → interleave → modulate → RF
```

and the receiver reverses it exactly:

```
RF → demodulate → de-interleave → FEC decode → de-whiten → CRC check → payload
```

The CRC is computed *before* whitening so that it protects the original data, and checked
*after* de-whitening. FEC wraps outside whitening so that its structured redundancy is not
destroyed by the scrambler. Interleaving is outermost because its job is to redistribute
channel burst errors before the decoder sees them.

### 9.2 Whitening

Section 5.5 introduced scrambling generically. In radio it is called whitening, and it exists
to solve four concrete problems that arise when a payload happens to contain a long run of
identical bytes — which real payloads do constantly, since padding, zeroed fields, and
repeated sensor readings are ubiquitous.

**Clock recovery.** A run of identical bits under 2-FSK is a constant tone. The timing recovery
loop has nothing to lock to and drifts.

**Spectral concentration.** A repeating pattern produces discrete spectral lines rather than a
smooth spectrum. Regulators specify limits on peak power spectral density, and a strong
spectral line can fail the mask even when total power is well within limits.

**DC offset in the demodulator.** A long run of one FSK tone biases the frequency
discriminator's DC estimate, shifting the decision threshold.

**Automatic gain control.** AGC loops assume statistically stationary input.

The implementation is a linear feedback shift register. **BLE uses a 7-bit LFSR with the
polynomial `x⁷ + x⁴ + 1`, initialised with the channel index in its upper bits and ones in the
lower position.** Seeding with the channel index is a small but elegant detail: the same
payload transmitted on different channels produces different transmitted waveforms, which
decorrelates interference between simultaneous piconets.

A minimal implementation:

```c
/* BLE-style whitener: 7-bit LFSR, x^7 + x^4 + 1.
 * Self-inverse: calling it twice restores the original data.
 * seed: for BLE, 0x40 | (channel_index & 0x3F)
 */
void whiten(uint8_t *data, size_t len, uint8_t seed)
{
    uint8_t lfsr = seed & 0x7F;

    for (size_t i = 0; i < len; i++) {
        uint8_t out = 0;

        for (int bit = 0; bit < 8; bit++) {
            /* Output bit is the LFSR MSB (position 6). */
            uint8_t msb = (lfsr >> 6) & 0x01;
            uint8_t in  = (data[i] >> bit) & 0x01;

            out |= (uint8_t)((in ^ msb) << bit);

            /* Shift left; feed back tap at position 3 (x^4 term). */
            lfsr = ((lfsr << 1) | msb) & 0x7F;
            if (msb) {
                lfsr ^= 0x08;                /* XOR into position 3 */
            }
        }
        data[i] = out;
    }
}
```

Most transceivers implement whitening in hardware, exposed as a single enable bit and a seed
register. The commonest field failure is a **seed mismatch** between the two ends: the link
appears completely dead, the receiver detects the sync word correctly (because sync words are
transmitted unwhitened), and every CRC fails. That specific signature — sync detected, CRC
always bad — is nearly diagnostic of a whitening misconfiguration, and Chapter 24 lists it
accordingly.

### 9.3 Cyclic Redundancy Check

The CRC treats the message as the coefficients of a binary polynomial, divides it by a
generator polynomial, and appends the remainder. The receiver repeats the division over
message-plus-remainder and expects zero.

Its properties, for a generator of degree `n`, are strong and provable:

- **All single-bit errors** are detected.
- **All burst errors up to `n` bits** are detected.
- **All odd numbers of bit errors** are detected, if the generator has `(x + 1)` as a factor.
- **Random errors** escape detection with probability approximately `2⁻ⁿ`.

| Standard | CRC | Polynomial | Undetected error probability |
|---|---|---|---|
| IEEE 802.15.4 | CRC-16 | x¹⁶ + x¹² + x⁵ + 1 | ~1.5 × 10⁻⁵ |
| Bluetooth LE | CRC-24 | x²⁴ + x¹⁰ + x⁹ + x⁶ + x⁴ + x³ + x + 1 | ~6 × 10⁻⁸ |
| Ethernet | CRC-32 | 0x04C11DB7 | ~2.3 × 10⁻¹⁰ |

That `2⁻ⁿ` figure deserves emphasis rather than a footnote. A CRC-16 link passing 1000 corrupt
frames per second admits an undetected corruption roughly once a minute. **The CRC is a filter,
not a guarantee**, and any application whose data integrity requirements exceed `2⁻ⁿ` must
implement its own end-to-end check. This is the concrete reason the transport layer has a
checksum even though every link beneath it already has one.

A table-free bitwise implementation, adequate for the short frames typical of low-power radio:

```c
/* CRC-16/CCITT as used by IEEE 802.15.4 (reflected, init 0x0000). */
uint16_t crc16_802154(const uint8_t *data, size_t len)
{
    uint16_t crc = 0x0000;

    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; bit++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0x8408;   /* reflected 0x1021 */
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}
```

In practice the CRC is computed by hardware in every modern transceiver, and firmware's role
is to configure the polynomial, the initial value, and — a persistent source of bugs — whether
the CRC covers the header, the payload, or both. A mismatch in *coverage* between two
implementations produces the same signature as a seed mismatch and is equally confusing.

### 9.4 Forward Error Correction

FEC adds structured redundancy so the receiver can repair errors rather than discard the
frame. The benefit is quantified as **coding gain**: the reduction in required Eb/N0 for a
given error rate.

The **code rate** `R = k/n` is the ratio of information bits to transmitted bits. A rate-1/2
code doubles the transmitted length. This costs bandwidth or airtime, and the coding gain must
exceed that cost to be worthwhile — which, at the error rates that matter, it comfortably does.

**Block codes** operate on fixed-size blocks.

- *Hamming(7,4)* corrects one error in seven bits. Mainly of pedagogical interest.
- *BCH* codes correct multiple errors and are used in flash memory and some Sub-GHz standards.
- *Reed–Solomon* operates on symbols rather than bits, which makes it exceptionally good
  against *burst* errors — an RS(255,223) code corrects any 16 corrupted bytes regardless of
  how the bit errors are distributed within them. Used in optical transport, storage, and deep
  space.

**Convolutional codes** process the bit stream continuously through a shift register, with each
output bit depending on the current input and the previous `K−1` inputs, where `K` is the
**constraint length**. They are decoded with the **Viterbi algorithm**, which finds the most
likely transmitted sequence. Coding gain rises with `K`, and so does decoder complexity —
exponentially, as `2^(K−1)` states.

**Modern iterative codes** — turbo and LDPC — approach within 1–2 dB of the Shannon limit and
are used in LTE, 5G, Wi-Fi 6, and DVB. They are absent from low-power radio because their
decoders cost orders of magnitude more energy per bit than the link itself.

**Hard versus soft decision.** A hard-decision decoder receives bits. A soft-decision decoder
receives the demodulator's confidence in each bit — for FSK, the magnitude of the frequency
discriminator output. Using that confidence is worth approximately **2 dB** of additional
coding gain, which is why Chapter 17 emphasises that the demodulator should pass soft metrics
forward rather than thresholding early.

| Scheme | Rate | Coding gain (hard) | Coding gain (soft) | Cost |
|---|---|---|---|---|
| Hamming(7,4) | 0.57 | ~1.5 dB | ~2.5 dB | Trivial |
| Convolutional K=4 | 1/2 | ~3 dB | ~5 dB | Low |
| Convolutional K=7 | 1/2 | ~4 dB | ~6 dB | Moderate |
| RS(255,223) | 0.87 | ~5 dB (burst) | — | Moderate |
| LDPC / Turbo | varies | — | ~9–11 dB | High |

### 9.5 Interleaving

FEC codes are designed against *random* errors. Real channels produce *bursts* — a fade, an
interference pulse, or a collision destroys hundreds of consecutive bits. A rate-1/2
convolutional code that comfortably repairs scattered errors is overwhelmed by a burst.

**Interleaving** permutes bit order before transmission and restores it at the receiver. A burst
that destroys 100 consecutive transmitted bits, after de-interleaving, becomes 100 isolated
errors scattered across the codeword — exactly the condition the decoder was designed for.

The cost is **latency**, and it is unavoidable: the receiver cannot begin decoding until the
whole interleaver block has arrived. Interleaver depth is therefore a direct trade against
delay, and it is the reason low-latency protocols use shallow interleaving or none at all.

### 9.6 The BLE Coded PHY: A Complete Worked Example

Bluetooth 5 introduced LE Coded, which applies exactly the mechanisms above to extend range.
It is the clearest production example available and rewards close reading.

The scheme uses a rate-1/2 convolutional code, plus an optional **pattern mapper** that repeats
each coded bit four times. Two schemes result:

| Scheme | Coding | Data rate | Sensitivity gain | Range vs 1M |
|---|---|---|---|---|
| **S = 2** | Convolutional 1/2 | 500 kbps | ~+3 dB typical | ~1.4× |
| **S = 8** | Convolutional 1/2 + 4× repeat | 125 kbps | ~+8 dB typical | ~2.5× |

The mechanism for S = 8 is worth decomposing, because it shows both effects working together:

- The rate-1/2 convolutional code contributes roughly 4–5 dB of coding gain against random
  errors.
- The 4× repetition contributes roughly `10 log(4) = 6 dB` of coherent combining gain, offset
  by the rate loss.
- The net, after accounting for the eightfold rate reduction, is the ~8 dB improvement quoted.

Note that the packet structure is not uniformly coded. The preamble is longer (80 µs), and the
Access Address, Coding Indicator, and first terminator are **always transmitted at S = 8**, even
when the payload uses S = 2. The reason is that the receiver must reliably decode the coding
indicator before it knows how the rest of the packet is encoded — a bootstrapping problem
solved by encoding the bootstrap itself as robustly as possible. This pattern, where the
header is protected more strongly than the payload, recurs throughout communications
engineering and appears again in Chapter 18.

The cost is airtime. At S = 8 a packet occupies eight times the duration it would at 1M. For a
duty-cycled device this is eight times the receive energy per packet, and in a shared channel
it is eight times the collision exposure. **Coded PHY buys range with time and energy**, which
is the same currency LoRa spends, and it should be enabled per-connection rather than
globally.

### 9.7 FEC Versus Retransmission

Both repair errors. They are appropriate in different regimes, and choosing wrongly is a
common architectural mistake.

| Consideration | FEC | ARQ (retransmission) |
|---|---|---|
| Overhead | Always paid, even when the channel is clean | Paid only on failure |
| Latency | Fixed and predictable | Variable; unbounded in the worst case |
| Requires return path | No | Yes |
| Broadcast / multicast | Works | Does not work |
| Efficiency on a clean channel | Poor | Excellent |
| Efficiency on a bad channel | Excellent | Collapses |
| Deep fade | Repairs, if within correction capacity | Retry may also fail |

The practical guidance follows directly. Use **ARQ** when a return path exists, latency
tolerance is adequate, and the channel is usually good — which describes most BLE connections
and most Sub-GHz star networks. Use **FEC** for broadcast, for one-way telemetry, for
latency-critical links, and whenever the channel is consistently marginal — which describes
LPWAN and long-range Sub-GHz. Use **both**, as LTE and Wi-Fi do, when the channel varies widely
and neither alone is sufficient.

### 9.8 What Carries Forward

- Order is fixed: CRC, then whiten, then FEC, then interleave. Reversing any pair breaks the
  link subtly.
- Whitening corrects nothing; it makes the waveform well-behaved. Seed mismatch is a classic
  bug with a distinctive signature.
- A CRC-`n` admits undetected errors with probability ~`2⁻ⁿ`. It is a filter, not a guarantee.
- Soft-decision decoding is worth ~2 dB over hard-decision and requires the demodulator to
  export confidence metrics.
- BLE Coded PHY buys ~8 dB of sensitivity for 8× the airtime and energy.
- FEC and ARQ solve the same problem in different regimes; the decision hinges on return path,
  latency, and how often the channel is good.

Part II has taken bits to waveforms and protected them against error. Part III examines the
three physical environments those waveforms must cross.
