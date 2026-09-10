# The Physical Layer: From Signals to Silicon

> Sub-GHz, Bluetooth Low Energy, and real wireless PHY design. Written for the
> embedded engineer who keeps hitting the physical layer from above — the one
> debugging why a link that works on the bench fails at 200 metres.

**Contents**
[About this book](#about-this-book) ·
[Full chapter list](#table-of-contents)

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

1. [What the Physical Layer Is, and Why It Must Exist](#what-the-physical-layer-is-and-why-it-must-exist)
2. [Signals: Time, Frequency, and the Meaning of Bandwidth](#signals-time-frequency-and-the-meaning-of-bandwidth)
3. [The Four Impairments: Noise, Attenuation, Distortion, Interference](#the-four-impairments-noise-attenuation-distortion-interference)
4. [The Ceilings: Nyquist, Shannon, and Eb/N0](#the-ceilings-nyquist-shannon-and-ebn0)

**Part II — Modulation and Coding**

5. [Line Coding and Baseband Signaling](#line-coding-and-baseband-signaling)
6. [Digital Modulation I: ASK, FSK, PSK, and the Constellation](#digital-modulation-i-ask-fsk-psk-and-the-constellation)
7. [Digital Modulation II: GFSK, MSK, OQPSK, DSSS, and the Sub-GHz Choice](#digital-modulation-ii-gfsk-msk-oqpsk-dsss-and-the-sub-ghz-choice)
8. [Bit Errors: From Eb/N0 to a Sensitivity Number](#bit-errors-from-ebn0-to-a-sensitivity-number)
9. [Error Control: Whitening, CRC, FEC, Interleaving, and the Coded PHY](#error-control-whitening-crc-fec-interleaving-and-the-coded-phy)

**Part III — Physical Media**

10. [Wired Physical Layers: Copper, Differential Signaling, and Ethernet](#wired-physical-layers-copper-differential-signaling-and-ethernet)
11. [Optical Physical Layers: Fiber, Transceivers, and Dispersion](#optical-physical-layers-fiber-transceivers-and-dispersion)
12. [The Wireless Channel: Path Loss, Multipath, Fading, and Doppler](#the-wireless-channel-path-loss-multipath-fading-and-doppler)

**Part IV — Wireless System Engineering**

13. [Link Budgets: A Complete Worked Analysis](#link-budgets-a-complete-worked-analysis)
14. [Interference, Coexistence, and Shared Spectrum](#interference-coexistence-and-shared-spectrum)
15. [Regulatory Constraints: FCC, ETSI, Duty Cycle, and Listen-Before-Talk](#regulatory-constraints-fcc-etsi-duty-cycle-and-listen-before-talk)
16. [Antennas and RF Front-Ends](#antennas-and-rf-front-ends)
17. [Digital Baseband and Modem Architecture](#digital-baseband-and-modem-architecture)

**Part V — Silicon, Firmware, and Practice**

18. [PHY Packet Structure: Preamble, Sync Word, Header, Payload, CRC](#phy-packet-structure-preamble-sync-word-header-payload-crc)
19. [PHY State Machines and Timing](#phy-state-machines-and-timing)
20. [PHY Firmware Architecture: Registers, Interrupts, and DMA](#phy-firmware-architecture-registers-interrupts-and-dma)
21. [End-to-End Signal Flow: From Buffer to Air and Back](#end-to-end-signal-flow-from-buffer-to-air-and-back)
22. [Reading and Writing a Real Sub-GHz PHY Driver](#reading-and-writing-a-real-sub-ghz-phy-driver)
23. [Bring-Up and Measurement: Spectrum, Sensitivity, and PER](#bring-up-and-measurement-spectrum-sensitivity-and-per)
24. [Debugging the Physical Layer: A Field Guide](#debugging-the-physical-layer-a-field-guide)

**Appendices**

- A. Decibel and RF Mathematics
- B. Formula Reference
- C. Sub-GHz and BLE Parameter Tables
- D. Standards Map
- E. Three Complete Design Walkthroughs
- F. PHY Troubleshooting Quick Reference
- G. Glossary
- H. How to Keep Learning

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

### 1.6 The PHY in the Protocol Stack

It is worth locating the Physical Layer precisely within the layered model, because the
boundaries above and below it define what the layer is responsible for and — equally
important — what it is entitled to assume.

The seven-layer OSI reference model places the Physical Layer at the bottom, beneath the Data
Link Layer. In the practical five-layer model used for the Internet, the same position holds:
the PHY sits beneath the link layer (which contains the MAC), and everything above is
concerned with logical rather than physical entities. The layering is not merely
organisational. It is a contract. Each layer offers a defined service to the layer above and
consumes a defined service from the layer below, and the value of the arrangement is that a
layer may be replaced entirely — copper for fiber, one modulation for another — without
disturbing the layers around it, provided the service contract is honoured.

The PHY's contract with the layer above is narrow and specific. It offers: *give me a frame of
bytes and a set of transmission parameters, and I will attempt to deliver those bytes to a
peer PHY; when I receive bytes, I will hand them up with metadata describing the conditions of
reception, and a flag indicating whether I believe them intact.* That is the whole of it.
The PHY does not promise delivery, does not order frames, does not retransmit, and does not
interpret content. Everything the PHY does is in service of that one narrow promise, and every
capability the layer above needs beyond it — reliability, ordering, addressing, flow control —
is the responsibility of higher layers precisely because the PHY does not provide it.

The metadata accompanying a received frame is more important than it first appears, because it
is the only window the higher layers have into the physical world. It typically includes the
received signal strength (RSSI), a link-quality estimate, the measured frequency offset, and
sometimes a per-bit or per-symbol confidence. Adaptive protocols live on this metadata: a link
layer decides whether to increase transmit power, switch to a more robust modulation, or change
channel entirely, and it makes those decisions from PHY metadata alone. A PHY that reports its
metadata inaccurately — an RSSI that does not track received power, a link-quality figure that
does not correlate with error rate — silently sabotages every adaptive mechanism above it, and
because the sabotage is silent, it is among the hardest classes of system failure to diagnose.
Chapter 23 accordingly treats metadata accuracy as a first-class bring-up measurement.

### 1.7 Inside the PHY: Sublayers and the Anatomy of the Layer

The Physical Layer is not monolithic. In every modern standard it is decomposed into
sublayers, and understanding the decomposition clarifies where a given piece of functionality
belongs and, in practice, which block of silicon or firmware owns it.

Wired standards make the decomposition explicit. IEEE 802.3 Ethernet divides the PHY into
three sublayers:

- The **PCS (Physical Coding Sublayer)** handles the line coding of Chapter 5 — the 8B/10B or
  64B/66B encoding, scrambling, and the block synchronisation that frames the coded stream. It
  is the boundary between logical bits and the coded symbol stream.
- The **PMA (Physical Medium Attachment)** handles serialisation, clock recovery, and the
  bit-level timing that turns a parallel word into a serial stream and back. It is where the
  phase-locked loop of Chapter 10 lives.
- The **PMD (Physical Medium Dependent)** handles the actual electrical or optical interface —
  the drivers, the receivers, the connectors. It is the only sublayer that changes when the
  medium changes, which is precisely why the decomposition exists: swapping copper for fiber
  replaces the PMD and leaves PCS and PMA untouched.

Wireless standards use different names but the same idea. A radio PHY separates the **baseband
processing** — modulation, coding, framing, synchronisation, everything in the digital domain —
from the **RF front-end** — the analogue chain of amplifiers, mixers, and filters that Chapter
16 examines. The boundary between them is the data converter: the DAC on transmit and the ADC
on receive. Everything on the digital side of that converter is, in principle, reconfigurable
in firmware; everything on the analogue side is fixed in silicon and printed-circuit-board
copper. This single boundary explains a great deal about what can and cannot be changed in a
deployed radio: a modulation parameter is a register write, but a front-end filter is a
respin.

The decomposition also maps onto the structure of this book. Parts I and II are about the
information-theoretic and baseband concerns that live on the digital side — the PCS and
baseband. Part III is about the media, the PMD's domain. Part IV's antenna and modem chapters
straddle the converter boundary. Part V is about the firmware that configures and coordinates
the whole assembly. Keeping the sublayer picture in mind is a way of always knowing which part
of the machine a given chapter is describing.

### 1.8 A Short History, and Why It Illuminates the Present

The Physical Layer's design choices are more intelligible when read as responses to the
constraints of the eras that produced them, and a brief history explains why several
apparently arbitrary conventions persist.

The earliest electrical communication — the telegraph — was, in the vocabulary of this book, an
on-off-keyed baseband system with a human demodulator. Its symbol rate was set by how fast an
operator could key and read Morse, and its line code (the dot-dash alphabet) was a
variable-length source code chosen for average efficiency in English text. The concerns are
already recognisable: symbol representation, timing, and the trade between speed and
reliability.

The telephone introduced the analogue voice channel and, with it, the 300–3400 Hz bandwidth
that still echoes through the field. When data had to be carried over telephone lines, the
modem was born — a device that modulated digital data onto an audio carrier that the telephone
network would pass. The progression of modem standards from 300 bps to 56 kbps is a compressed
history of everything in Part II: it began with simple FSK, moved through PSK, added
quadrature amplitude modulation, and finally approached the Shannon capacity of the voiceband
channel so closely that no further improvement was possible without changing the channel
itself. The V.34 modem operating at 33.6 kbps over a 3.4 kHz channel achieves a spectral
efficiency near 10 bits/s/Hz — a figure that requires an SNR around 36 dB and is a direct
demonstration of the Shannon bound of Chapter 4.

Cellular telephony drove the next era and forced the field to confront the wireless channel of
Chapter 12 at scale. The move from analogue (1G) to digital (2G GSM) established GMSK and the
constant-envelope philosophy that Part II will argue for. The subsequent generations —
CDMA in 3G, OFDM in 4G, and massive MIMO in 5G — are each a response to the demand for more
capacity in a fixed, fading, interference-limited spectrum, and each is an application of a
principle this book develops: spreading, orthogonal multiplexing, and spatial diversity.

The low-power radio world that is this book's particular focus emerged later and from a
different pressure: not capacity but energy. When the constraint became a coin cell rather than
a mains supply, the entire optimisation flipped. Spectral efficiency, the obsession of the
cellular and modem eras, became almost irrelevant; energy per bit and acquisition time became
everything. This is why Sub-GHz and BLE make choices — non-coherent FSK, minimal coding, short
packets — that would be considered primitive in a cellular downlink and are exactly correct for
a sensor that must live a decade on a battery. The history matters because it shows that there
is no universally best PHY, only a PHY best suited to a particular constraint, and that the
constraint has shifted from speed to capacity to energy as the applications changed.

### 1.9 The Path Through This Book

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

### 2.7 Complex Exponentials and the Meaning of Negative Frequency

The I/Q representation that Chapter 6 relies on, and that every modern radio implements
directly, rests on a mathematical object that deserves its own treatment: the complex
exponential. Engineers who are uncomfortable with it will find the entire receive chain of
Chapter 17 opaque, and those who are comfortable with it will find that chain nearly obvious.

Euler's identity states:

$$
e^{j\theta} = \cos\theta + j\sin\theta
$$

A complex exponential `e^{j2\pi f t}` is therefore a point rotating around the unit circle at
`f` revolutions per second, its cosine projected onto the real axis and its sine onto the
imaginary axis. A real sinusoid is the sum of two such rotations in opposite directions:

$$
\cos(2\pi f t) = \frac{1}{2}\left(e^{j2\pi f t} + e^{-j2\pi f t}\right)
$$

The second term rotates at `−f`, and this is the origin of **negative frequency**, a concept
that troubles newcomers and is entirely concrete. A negative frequency is a rotation in the
opposite direction. A real signal always contains equal and opposite rotations, which is why
its spectrum is symmetric about zero. A complex signal — an I/Q signal, where I and Q are
genuinely independent — need not be symmetric, and can carry information in the distinction
between positive and negative frequency. This is precisely what a quadrature receiver exploits:
by keeping I and Q separate rather than collapsing them into a single real waveform, it can
distinguish a signal just above the carrier from one just below it, doubling the usable
spectrum and enabling the image rejection that Chapter 16 requires.

The practical consequence appears everywhere in Part V. When firmware configures a
"complex" or "IQ" data path, it is preserving this distinction. When a receiver suffers from
**I/Q imbalance** (Section 16.8), the imperfection breaks the symmetry cancellation and lets a
signal at `+f` leak into `−f` — the image — corrupting reception. The abstract mathematics of
negative frequency becomes, in the laboratory, a spurious tone in the wrong place, and knowing
the mathematics is what lets an engineer recognise the cause.

### 2.8 A Fourier Series, Computed

Section 2.3 asserted that a square wave decomposes into odd harmonics. It is worth computing
the decomposition once, in full, because the mechanics illuminate why bandwidth and signal
sharpness are inseparable.

Consider a symmetric square wave of amplitude `A` and period `T`, equal to `+A` for the first
half-period and `−A` for the second. Its Fourier series coefficients are found by integrating
the signal against each harmonic. The even harmonics vanish by symmetry (the square wave is an
odd function about its midpoint), and the odd-harmonic amplitudes work out to `4A/(nπ)` for the
`n`th harmonic. The series is:

$$
s(t) = \frac{4A}{\pi}\sum_{n=1,3,5,\dots}^{\infty} \frac{1}{n}\sin\!\left(\frac{2\pi n t}{T}\right)
$$

The first few terms carry the following fractions of total power (power goes as amplitude
squared, and the total is the sum of the squared coefficients):

| Harmonic | Amplitude (× 4A/π) | Relative power | Cumulative power |
|---|---|---|---|
| 1st (fundamental) | 1.000 | 81.1% | 81.1% |
| 3rd | 0.333 | 9.0% | 90.1% |
| 5th | 0.200 | 3.2% | 93.4% |
| 7th | 0.143 | 1.7% | 95.0% |
| 9th | 0.111 | 1.0% | 96.0% |

Two lessons fall out of this table directly. First, **81% of a square wave's power is in its
fundamental** — which means a band-limited channel that passes only the fundamental still
delivers most of the energy and a recognisable, if rounded, version of the signal. This is why
digital signalling survives channels far narrower than the "infinite bandwidth" a perfect
square wave nominally demands. Second, the **sharp edges live in the high harmonics**: reaching
95% of the power requires harmonics out to the 7th, and the corners of the waveform sharpen
only as still-higher harmonics are added. Removing the high harmonics — which every real
channel does, being a low-pass filter — does not destroy the signal; it rounds its edges. The
famous **Gibbs phenomenon**, the overshoot ringing that appears near the edges of a
band-limited square wave, is the visible signature of the truncated series, and it is the same
ringing that Chapter 10 warns about in the form of reflections and that Chapter 7 tames with
pulse shaping.

### 2.9 The Discrete Fourier Transform and Spectral Leakage

Everything above concerns continuous signals. Real receivers work with samples, and the tool
that gives the spectrum of a sampled signal is the **Discrete Fourier Transform (DFT)**,
computed efficiently by the **Fast Fourier Transform (FFT)**. It is the workhorse of the
digital baseband and of every spectrum analyser, and its two characteristic artefacts —
leakage and the picket-fence effect — must be understood by anyone who reads a measured
spectrum, which is to say anyone doing the bring-up of Chapter 23.

The DFT operates on a finite block of `N` samples and returns `N` frequency bins spaced by the
**bin width**:

$$
\Delta f = \frac{f_s}{N}
$$

where `f_s` is the sampling rate. Finer frequency resolution requires a longer block, which is
the same time-frequency trade the whole chapter has been developing: to resolve two closely
spaced tones, you must observe for longer.

The subtlety is that the DFT implicitly assumes the `N`-sample block repeats periodically. If a
signal's frequency does not fall exactly on a bin — if an integer number of its cycles does not
fit exactly in the block — then the assumed periodic extension has a discontinuity at the block
boundary, and that discontinuity spreads the signal's energy across many bins. This is
**spectral leakage**, and it is why a pure tone, which should appear as a single spectral line,
often appears in a real measurement as a peak with skirts spreading either side.

Leakage is controlled by **windowing**: multiplying the sample block by a function that tapers
smoothly to zero at both ends, removing the boundary discontinuity. The choice of window trades
two properties against each other — the width of the main lobe (frequency resolution) against
the height of the side lobes (leakage suppression):

| Window | Main-lobe width | Side-lobe level | Use |
|---|---|---|---|
| Rectangular (none) | Narrowest | −13 dB | Best resolution, worst leakage |
| Hann | Moderate | −31 dB | General purpose |
| Hamming | Moderate | −42 dB | Good side-lobe suppression |
| Blackman | Wider | −58 dB | Strong leakage suppression |
| Flat-top | Widest | −44 dB | Accurate amplitude measurement |

The practical relevance is direct. When a spectrum analyser shows a transmitted signal's
occupied bandwidth (Chapter 15's compliance measurement), the window choice affects the
apparent spectrum, and a measurement made with the wrong window can appear to pass or fail a
mask that the signal actually fails or passes. An engineer reading a spectrum must know which
window produced it. This is one of the many places where an abstract signal-processing concept
becomes a concrete measurement decision with regulatory consequences.

### 2.10 What Carries Forward

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

### 3.6 Noise Temperature and the Cascade, Worked

Noise figure is the engineer's everyday currency, but it has a companion, **noise
temperature**, that is more natural for cascade analysis and essential for the very low-noise
systems — satellite ground stations, radio astronomy — where noise figures in fractions of a
decibel are awkward to add. The relationship is:

$$
T_e = T_0 (F - 1)
$$

where `T_e` is the equivalent noise temperature, `F` is the noise factor (the linear noise
figure), and `T_0 = 290\ \text{K}` is the reference. A noise figure of 3 dB (`F = 2`)
corresponds to `T_e = 290\ \text{K}`; a noise figure of 0.5 dB corresponds to `T_e = 35\ \text{K}`,
a distinction that decibels obscure and temperature makes plain.

The cascade equation of Section 3.3.2 is worth working through with numbers, because the result
is counterintuitive to anyone who has not done it and because it governs the front-end layout
of Chapter 16. Consider a receive chain of three stages:

- **Stage 1 — LNA:** gain `G_1 = 15\ \text{dB}` (linear 31.6), noise figure `F_1 = 1.5\ \text{dB}`
  (linear 1.41)
- **Stage 2 — mixer:** gain `G_2 = -6\ \text{dB}` (linear 0.25, a loss), noise figure
  `F_2 = 8\ \text{dB}` (linear 6.31)
- **Stage 3 — IF amplifier:** noise figure `F_3 = 10\ \text{dB}` (linear 10)

Applying Friis:

$$
F_{total} = 1.41 + \frac{6.31 - 1}{31.6} + \frac{10 - 1}{31.6 \times 0.25} = 1.41 + 0.168 + 1.139 = 2.72
$$

which is `10\log_{10}(2.72) = 4.3\ \text{dB}`.

Now observe what happened. The mixer, with its 8 dB noise figure, contributed only 0.17 to the
total — because the LNA's 15 dB of gain divided its contribution down. But the IF amplifier, with
its 10 dB noise figure, contributed 1.14 — more than six times as much — because the mixer's
6 dB *loss* undid part of the LNA's gain before the IF amplifier's noise was referred back to
the input. The lesson is stark and practical: **a lossy stage anywhere in the chain amplifies
the noise contribution of everything after it.** This is why passive filters and switches are
kept out of the early chain where possible, and why, when a filter must precede the LNA for
blocking protection (Chapter 14), its insertion loss is paid for twice — once in the budget and
once by unmasking the noise of later stages.

Repeat the calculation with the LNA gain raised to 25 dB and the total noise figure falls to
about 2.0 dB; drop it to 6 dB and the total climbs above 8 dB. The single most effective lever
on a receiver's noise figure is the gain of its first stage, which is the whole reason the "low
noise *amplifier*" is an amplifier and not merely a low-noise passive.

### 3.7 Intermodulation and the Third-Order Intercept, Derived

Section 3.4 asserted that third-order intermodulation products are the dangerous ones because
they fall in band. The derivation is short and worth doing, because it explains both why the
third order dominates and why the IP3 specification is quoted the way it is.

Model a mildly nonlinear amplifier by a power series relating output to input:

$$
v_{out} = a_1 v_{in} + a_2 v_{in}^2 + a_3 v_{in}^3 + \cdots
$$

The `a_1` term is the wanted linear gain. Now drive it with two tones at frequencies `f_1` and
`f_2`, as happens whenever two signals share the receiver's band. Expanding the squared and
cubed terms with trigonometric identities generates a forest of new frequencies. The
second-order term (`a_2 v_{in}^2`) produces components at `2f_1`, `2f_2`, `f_1 + f_2`, and
`f_1 - f_2` — all far from the original tones, and easily removed by filtering.

The third-order term (`a_3 v_{in}^3`) is the problem. Among its products are components at:

$$
2f_1 - f_2 \qquad\text{and}\qquad 2f_2 - f_1
$$

If `f_1` and `f_2` are two signals close together in the receiver's band — a wanted signal and a
nearby interferer, say — then `2f_1 - f_2` lands *right next to them*, inside the band, where no
filter can remove it without also removing the wanted signal. This is why third-order
intermodulation is the receiver's characteristic nonlinear failure and why it, not the
second-order product, defines the linearity specification.

The IP3 figure captures the severity in a single number. As input power rises, the wanted
(first-order) output rises 1 dB per dB, while the third-order products rise **3 dB per dB** —
three times as fast, because they come from a cubed term. Plotted on log axes, the two lines
have slopes 1 and 3 and must eventually cross; the crossing point, extrapolated (the amplifier
saturates before actually reaching it), is the **third-order intercept point**. A higher IP3
means the third-order line starts lower and the amplifier tolerates stronger signals before
intermodulation becomes troublesome. The relationship between the intercept, the input level,
and the resulting intermodulation is:

$$
\text{IM3 ratio (dBc)} = 2(\text{IIP3} - P_{in})
$$

so every dB of headroom below the intercept buys 2 dB of intermodulation suppression. This
factor of two is why receiver linearity is specified so carefully and why the near-far problem
of Chapter 14 is so damaging: a strong nearby transmitter drives `P_in` up, and the
intermodulation it generates rises twice as fast as the interferer itself.

### 3.8 Which Impairment Do You Have?

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

### 4.8 Where Shannon's Formula Comes From

The capacity formula is usually presented as a result to be memorised, but a sketch of its
origin makes it far more usable, because it reveals *why* capacity depends on SNR the way it
does and what "reliable communication" actually means.

Shannon's argument is geometric and rests on counting distinguishable signals. Over an interval
of `T` seconds in a channel of bandwidth `B`, the Nyquist result says there are `2BT`
independent signal dimensions — degrees of freedom in which the transmitter may place energy.
Think of each transmitted waveform as a single point in this `2BT`-dimensional space, its
coordinates the amplitudes in each dimension.

Noise displaces the received point from the transmitted one. With average noise power `N` per
dimension, the received point lands within a "noise ball" of radius proportional to `\sqrt{N}`
around the true point. Two transmitted points can be reliably distinguished only if their noise
balls do not overlap — otherwise the receiver cannot tell which was sent.

The transmitter has average power `S` per dimension, so all its points lie within a large ball
of radius proportional to `\sqrt{S + N}` (signal plus the noise that will be added). The
question "how many distinguishable messages can I send?" becomes "how many small noise balls of
radius `\sqrt{N}` fit inside the large ball of radius `\sqrt{S+N}`?" — a sphere-packing problem.
In `2BT` dimensions, the number of balls that fit is the ratio of volumes, which scales as:

$$
\left(\frac{\sqrt{S+N}}{\sqrt{N}}\right)^{2BT} = \left(1 + \frac{S}{N}\right)^{BT}
$$

The number of bits is the base-2 logarithm of the number of distinguishable messages, and
dividing by `T` for the rate gives:

$$
C = \frac{1}{T}\log_2\left(1 + \frac{S}{N}\right)^{BT} = B\log_2\left(1 + \text{SNR}\right)
$$

which is exactly the capacity formula. The derivation makes three things intuitive that the
bare formula does not. The logarithm comes from *counting messages* and taking bits. The linear
dependence on `B` comes from the *number of dimensions*. And "reliable communication at rates
below `C`" means, concretely, that the noise balls can be packed without overlap — while above
`C` they must overlap, and reliable distinction becomes impossible. The sharp wall at capacity
is the point where the packing fails.

### 4.9 The Spectral-Efficiency Plane and the Two Regimes

The relationship between capacity, bandwidth, and power is most illuminating when plotted as
**spectral efficiency** (bits per second per hertz, `C/B`) against `Eb/N0`. This plot, one of
the most important in communications, divides all systems into two regimes and locates every
modulation scheme relative to the Shannon bound.

Rewriting the capacity formula in terms of spectral efficiency `η = C/B` and Eb/N0:

$$
\eta = \log_2\left(1 + \frac{E_b}{N_0}\eta\right)
\qquad\Longrightarrow\qquad
\frac{E_b}{N_0} = \frac{2^\eta - 1}{\eta}
$$

This relation is the Shannon bound expressed as the minimum Eb/N0 required to achieve a given
spectral efficiency. Evaluating it:

| Spectral efficiency η | Minimum Eb/N0 | Regime |
|---|---|---|
| 0.1 bits/s/Hz | −1.55 dB | Deep power-limited |
| 0.5 | −0.82 dB | Power-limited |
| 1.0 | 0.00 dB | The crossover |
| 2.0 | 1.76 dB | Bandwidth-limited |
| 4.0 | 3.83 dB | Bandwidth-limited |
| 8.0 | 8.28 dB | Deep bandwidth-limited |

As `η → 0`, Eb/N0 approaches the `−1.59\ \text{dB}` limit of Section 4.4 — the **power-limited**
regime, where bandwidth is abundant and the constraint is energy per bit. This is where Sub-GHz
long-range links, spread spectrum, GPS, and the BLE Coded PHY operate: they spend bandwidth
freely to minimise the energy each bit requires. Doubling bandwidth in this regime nearly
doubles the achievable rate at fixed power.

As `η` grows large, Eb/N0 rises steeply — the **bandwidth-limited** regime, where spectrum is
the binding constraint and each additional bit per hertz costs disproportionately more power.
This is where Wi-Fi, cable, and cellular downlinks operate, pushing to 64-QAM and beyond and
paying the SNR price because spectrum is what they lack.

The regime a system occupies determines its entire design philosophy, and the two are almost
opposite. A power-limited designer widens bandwidth and simplifies modulation; a
bandwidth-limited designer narrows bandwidth and complicates modulation. Recognising which
regime a requirement falls into is the first strategic decision in any PHY design, and it is
why the feasibility analysis of Section 4.7 began by asking whether the problem was
spectrum-bound or power-bound. Sub-GHz IoT is almost always power-limited; the mistake of
importing bandwidth-limited intuitions (chase spectral efficiency, use high-order QAM) into a
power-limited problem is the single most common conceptual error in the field, and Chapter 7's
argument against QAM in Sub-GHz is, at bottom, a statement about which regime applies.

### 4.10 Capacity in the Fading Channel

The Shannon formula assumes a fixed SNR. The wireless channel of Chapter 12 has an SNR that is
itself random, and the capacity of a fading channel is a distribution rather than a number —
which has consequences that AWGN intuition does not prepare one for.

Two notions of capacity emerge, and the distinction is practical rather than academic.
**Ergodic capacity** is the average capacity over the fading distribution, achievable by a code
long enough to average across many independent fades — appropriate for a delay-tolerant system
that can interleave over the channel's variations. **Outage capacity** is the rate that can be
sustained with a specified probability, the relevant figure for a delay-constrained system that
cannot wait for the channel to improve.

The gap between them is large in deep fading and is the quantitative reason Chapter 8's Rayleigh
penalty is so severe. A channel whose ergodic capacity is a comfortable 2 bits/s/Hz may have an
outage capacity at 99% reliability of a small fraction of that, because 1% of the time the
channel is in a deep fade where almost nothing gets through. A system that must work 99% of the
time must be designed to the outage capacity, not the average — which means either accepting a
much lower rate or, far better, providing the diversity of Chapter 8 to compress the fading
distribution and pull the outage capacity up toward the ergodic value. This is the
information-theoretic statement of why diversity is not an optimisation but a necessity: it
converts a channel with a punishing outage capacity into one whose outage capacity approaches
its average, recovering most of the rate that fading would otherwise deny.

### 4.11 What Carries Forward

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

### 5.7 The Mathematics of Running Disparity and DC Balance

Section 5.2 named DC balance as an objective and Section 5.3 described 8B/10B as achieving it
through bounded running disparity. It is worth making the notion quantitative, because it is the
property that separates codes that merely have many transitions from codes that are genuinely
DC-free, and the distinction matters for any AC-coupled path.

Define the **digital sum variation (DSV)** as the running total of the signal, counting +1 for a
transmitted 1 and −1 for a transmitted 0. A perfectly DC-balanced code keeps this sum bounded:
it may wander up and down, but never drifts without limit. NRZ has *unbounded* DSV — a long run
of ones sends the sum to infinity — which is exactly the baseline-wander problem of Section 5.1.

The power spectral density of a line code has a component at DC proportional to the variance of
its DSV. A code with bounded DSV has *zero* spectral content at exactly DC, which is what allows
a transformer- or capacitor-coupled channel to pass it without loss. This is why 8B/10B's
disparity control is not cosmetic: by choosing between two code words (one with more ones, one
with more zeros) to steer the DSV back toward zero, the encoder forces a spectral null at DC.
The code guarantees DSV stays within a bound of roughly ±3, which places a hard floor under how
much the baseline can wander and lets the receiver's slicer keep its threshold correctly
centred indefinitely.

Scrambled codes such as 64B/66B achieve a *statistical* null instead. The scrambler randomises
the data, so the DSV performs a random walk whose expected excursion grows only as the square
root of the run length — slowly enough that the coupling components, sized for the worst
statistically likely excursion, cope. The trade is explicit: 8B/10B pays 25% overhead for a
*guaranteed* bound; 64B/66B pays 3% overhead for a bound that holds with overwhelming
probability but not certainty. At 10 Gbps the statistical approach wins because the overhead
saving is enormous and the residual risk is engineered away; at lower rates where overhead is
cheap, the guarantee is often preferred.

### 5.8 Bit Stuffing and Framing Codes

Two mechanisms adjacent to line coding solve related problems and appear throughout the field,
so they belong here.

**Bit stuffing** handles the run-length problem without a full substitution code. The
transmitter, after some number of consecutive identical bits, inserts an extra opposite bit;
the receiver, seeing that pattern, removes it. HDLC and USB use this: HDLC inserts a 0 after
five consecutive 1s, both guaranteeing a transition for clock recovery and preventing the data
from accidentally imitating the frame delimiter (a specific flag byte). The cost is a small,
data-dependent overhead — worst case one stuffed bit per five, but typically far less on random
data — and a variable transmitted length, which complicates timing budgets. Bit stuffing is a
cheaper, coarser alternative to a table-based code, appropriate where the run-length guarantee
matters but full DC balance does not.

**Comma and control symbols** solve framing. A receiver locked to a bit stream still must find
*byte* and *frame* boundaries — where one code word ends and the next begins. Codes like 8B/10B
reserve special "comma" symbols that never appear inside valid data and whose bit pattern is
uniquely recognisable at any alignment, so the receiver can slide its byte boundary until it
finds a comma and thereby achieve **word alignment**. This is the wired analogue of the sync
word of Chapter 18: a known pattern, distinguishable from data, that establishes framing. The
recurrence of this idea — a reserved, unmistakable pattern that bootstraps alignment — across
wired and wireless systems is one of the unifying threads of the physical layer.

### 5.9 What Carries Forward

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

### 6.7 Constellation Geometry and the Union Bound

Section 6.2 stated the governing principle — error probability is set by minimum Euclidean
distance — and Chapter 8 will make it exact. Between the two, it is worth developing the geometry
a little further, because it is the language in which all modulation performance is actually
reasoned about, and because it explains the numbers in the comparison tables rather than merely
asserting them.

Place a constellation's points in the I/Q plane, each carrying `\log_2 M` bits, and normalise so
that the *average* energy per symbol is `E_s`. The receiver, given a noisy point, chooses the
nearest constellation point — the **maximum-likelihood** decision, optimal for Gaussian noise.
An error occurs when noise pushes the received point across a decision boundary into a
neighbouring point's region.

For a single pair of points separated by Euclidean distance `d`, the probability that noise
carries one into the other's region is `Q(d/2\sigma)`, where `\sigma` is the noise standard
deviation per dimension. When a point has several neighbours, the total error probability is
bounded by summing these pairwise terms — the **union bound**:

$$
P_{symbol} \le \sum_{k} Q\!\left(\frac{d_k}{2\sigma}\right)
$$

At any useful SNR the `Q` function falls so steeply (Section 8.2) that this sum is dominated
utterly by its smallest-distance term. The consequence is the principle in operational form:
**a constellation's error rate is set almost entirely by its minimum distance and by how many
neighbour pairs share that minimum distance** (the "nearest-neighbour count"). Everything else is
negligible.

This immediately explains the tables. BPSK's two points are separated by `2\sqrt{E_b}` — the
maximum for a given energy — so it has the best error performance of any binary scheme. QPSK
places four points on a circle; its minimum distance, worked in terms of energy per bit, is
identical to BPSK's, which is the geometric reason for the "QPSK is free" result. 16-QAM packs
sixteen points into a 4×4 grid, and its minimum distance for a given average energy is much
smaller — the points are crowded — so it needs far more SNR. The whole hierarchy of required
Eb/N0 values is a hierarchy of minimum distances at fixed energy, and once the geometry is seen
this way the tables cease to be facts to memorise and become consequences to derive.

### 6.8 Coherent and Non-Coherent Detection, Compared Concretely

Chapter 6 has repeatedly invoked the coherent/non-coherent distinction; here it is worth pinning
down what each receiver actually does, because the difference is the hinge on which the entire
Sub-GHz design philosophy turns.

A **coherent** receiver knows, or estimates, the exact phase of the incoming carrier and uses it.
It multiplies the received signal by a locally generated carrier of matched phase, integrates
over a symbol, and reads off the result — a process called correlation or matched filtering. This
extracts the maximum possible SNR from each symbol, which is why coherent schemes sit at the top
of the efficiency table. The price is that the local carrier's phase must be locked to the
incoming signal's, which requires a carrier-recovery loop: circuitry (or firmware) that estimates
and tracks the phase, takes time to acquire it, and fails if the frequency offset is too large
for it to pull in.

A **non-coherent** receiver deliberately discards phase. For FSK it measures which frequency is
present without caring about the carrier's absolute phase — an energy or frequency measurement,
not a correlation against a phase reference. This throws away the ~3–4 dB that phase information
would have provided, but it eliminates the carrier-recovery loop entirely, and with it the
acquisition time and the frequency-pull-in limit.

The trade, laid side by side:

| Property | Coherent | Non-coherent |
|---|---|---|
| Uses carrier phase | Yes | No |
| Eb/N0 penalty | 0 (reference) | +3 to +4 dB |
| Carrier-recovery loop | Required | None |
| Acquisition time | 100s of symbols | A few symbols |
| Frequency-offset tolerance | Limited by loop pull-in | Wide (set by `h`) |
| Startup energy | High | Low |
| Suits | Mains-powered, long packets | Battery, short packets, cheap crystals |

For a cellular handset streaming a long packet with a precise oscillator, the 3–4 dB is decisive
and coherence is worth its cost. For a Sub-GHz sensor waking for 5 ms, transmitting 20 bytes, and
running a ±20 ppm crystal, the acquisition time and frequency tolerance dominate, and
non-coherence is the obvious choice despite the penalty. This single table, more than any other
in Part II, explains why the low-power world looks the way it does — and why importing coherent,
spectrally-efficient designs from the cellular world into a coin-cell product fails.

### 6.9 What Carries Forward

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

### 7.8 Continuous-Phase Modulation and the Phase Trellis

The GFSK, MSK, and GMSK schemes of this chapter all belong to a family — **continuous-phase
modulation (CPM)** — whose defining property is that the carrier phase never jumps. This property
is worth developing, because it is the source of both the spectral compactness and the constant
envelope that make these modulations attractive, and because it introduces the trellis structure
that Chapter 9's decoders exploit.

In ordinary PSK the phase can jump instantaneously between symbols, and Section 7.1 showed that
those jumps are what create spectral splatter. CPM forbids the jumps: the phase evolves
*continuously*, ramping smoothly from one value to the next. For MSK, each bit ramps the phase by
exactly ±90° over one symbol period; the phase trajectory is a connected path, never a
discontinuity. Because the signal is `\cos(\text{carrier} + \phi(t))` with `\phi(t)` continuous,
the envelope is constant and the spectrum's tails fall as the fourth power of frequency offset
rather than the second — dramatically cleaner.

The continuity has a second, deeper consequence: the phase at any symbol boundary depends on the
*entire history* of bits, because each has contributed its ramp. The phase therefore carries
memory, and the set of reachable phase states forms a **trellis** — a diagram of states and the
transitions between them, identical in structure to the trellis of a convolutional code
(Section 9.4). This means a CPM signal can be demodulated not merely symbol by symbol but as a
sequence, using the same maximum-likelihood sequence estimation (the Viterbi algorithm) that
decodes convolutional codes. A receiver that does so recovers 1–2 dB over a simple symbol-by-
symbol discriminator, because it exploits the memory the modulation deliberately built in. Most
low-power radios do not bother — the simple discriminator is cheaper and the penalty acceptable —
but the option exists, and GSM handsets take it, which is part of why GMSK at BT = 0.3 performs
as well as it does despite heavy filtering.

### 7.9 OFDM: The Wideband Alternative

Every modulation in this chapter is **single-carrier** — one carrier, modulated fast. The
dominant technique in high-rate wireless (Wi-Fi, LTE, 5G, DVB) is the opposite:
**orthogonal frequency-division multiplexing (OFDM)**, which divides the data across many
slow, closely-spaced subcarriers. It is outside this book's low-power focus, but it deserves a
section because it is the natural answer to a problem Chapter 12 raises and because the contrast
sharpens the understanding of why Sub-GHz does *not* use it.

The problem is frequency-selective fading (Section 12.6). A single high-rate carrier occupies a
bandwidth wider than the channel's coherence bandwidth, so different parts of its spectrum fade
independently, smearing symbols into each other as severe inter-symbol interference that demands
a complex equaliser. OFDM sidesteps this elegantly. By splitting the data across hundreds or
thousands of subcarriers, each subcarrier is narrow enough to fade *flat* — no ISI on any
individual subcarrier — and the wideband ISI problem dissolves into a set of independent
narrowband channels, each trivially equalised by a single complex multiplication.

The subcarriers are spaced so that each one's spectral peak falls exactly on its neighbours'
nulls — the "orthogonal" in the name — so they overlap without interfering, and the whole
ensemble is generated and recovered by a single FFT (Section 2.9), which is what makes OFDM
computationally feasible. A **cyclic prefix**, a guard interval copied from the end of each
symbol to its start, absorbs the residual multipath delay and preserves the orthogonality.

OFDM is superb for its regime and wrong for Sub-GHz, for reasons that recapitulate the whole
argument of Section 7.6. It has a high peak-to-average power ratio — the sum of many subcarriers
occasionally aligns into a large peak — forcing the linear, backed-off, inefficient power
amplifier that a battery device cannot afford. It requires the FFT engine and the tight
frequency synchronisation that low-power silicon omits. And it solves frequency-selective
fading, a problem that narrowband Sub-GHz simply does not have, since a 100 kHz Sub-GHz channel
sits comfortably inside the coherence bandwidth and fades flat. OFDM is the right tool for a
wideband, mains-powered, spectrum-limited system and precisely the wrong one for a narrowband,
battery, range-limited system — a clean illustration of the book's recurring theme that the best
modulation is the one whose costs match the resources available.

### 7.10 Spreading in Depth: Correlation, Jamming, and the Processing-Gain Ledger

Section 7.5 introduced DSSS and LoRa; the mechanism repays a closer look, because processing
gain is often misunderstood as free sensitivity when it is in fact a specific and bounded
resource.

The despreading operation is a **correlation**: the receiver multiplies the incoming signal by
the same pseudo-random sequence the transmitter used, then integrates. For the wanted signal,
the multiplication aligns perfectly and the sequence squares to a constant, so integration
accumulates the signal coherently — its contribution grows linearly with the number of chips.
For noise and any signal *not* matching the sequence, the multiplication produces a random
result that integrates toward zero — its contribution grows only as the square root. The ratio
of the two growth rates is the processing gain, and this is why it improves the effective SNR:
the wanted signal is built up coherently while everything else averages down.

The crucial limitation, easy to forget, is that processing gain helps against noise and against
*uncorrelated* interference, but does **nothing** against a signal that happens to match the
sequence, and only partially against a strong in-band interferer that the front end cannot
handle linearly. It also does not create energy — LoRa's remarkable −137 dBm sensitivity at SF12
is bought by spending over a second of airtime per packet, coherently integrating the tiny
received energy over that whole interval. The processing-gain ledger always balances: every dB
of gain is paid for in bandwidth (DSSS) or time (LoRa's chirp), and Chapter 15 will show that in
the EU Sub-GHz bands the time is a legally metered resource. Spreading moves the link into the
deep power-limited regime of Section 4.9, trading the one resource it has (bandwidth or airtime)
for the one it needs (energy per bit) — which is exactly what the spectral-efficiency plane
predicts a range-limited system should do.

### 7.11 What Carries Forward

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

### 8.8 The Matched Filter: Why It Is Optimal

Every sensitivity figure in this book assumes the receiver extracts the maximum SNR the channel
allows, and the device that does so is the **matched filter**. It is worth understanding why it
is optimal, because a receiver that fails to implement it — through an incorrectly configured
channel filter (Chapter 17) or a demodulator that samples at the wrong instant — silently forfeits
sensitivity, and the loss is invisible without the calculation to expect better.

The receiver's job at the symbol level is to decide, from a noisy waveform, which symbol was
sent. Intuitively, it should weight the parts of the received waveform where the signal is
strong and de-emphasise where it is weak, so as to collect signal energy while averaging out
noise. The matched filter formalises this: its impulse response is a time-reversed copy of the
transmitted pulse, so that at the correct sampling instant it has integrated the received signal
against the known pulse shape, coherently accumulating all the symbol's energy while the noise,
being uncorrelated with the pulse, accumulates only incoherently.

The result is that the matched filter maximises the SNR at the sampling instant, and the peak
SNR it achieves is:

$$
\text{SNR}_{max} = \frac{2E_s}{N_0}
$$

— exactly the `2E_b/N_0` inside the BPSK error formula of Section 8.3. The matched filter is not
one design choice among several; it is *the* optimum, and every good receiver approximates it.
The pulse-shaping filter of Chapter 7 and the matched filter are two halves of a designed pair:
the transmitter shapes the pulse, the receiver matches to it, and when the two are correctly
paired the combination both controls bandwidth and maximises SNR. A common and costly error is
to shape only at the transmitter and use a mismatched receive filter — often a channel filter set
wider than the signal for convenience — which admits excess noise and gives up 1–3 dB. This is a
frequent, entirely avoidable contributor to the implementation loss of Section 8.4, and it is
one of the first things Chapter 23 checks when measured sensitivity disappoints.

### 8.9 Diversity Combining, Quantified

Section 8.7 established that diversity is the answer to fading and that `L` independent branches
make the error rate fall as `\gamma^{-L}`. The remaining question is how the branches are
combined, because the combining method determines how much of the potential diversity gain is
actually realised.

Three schemes span the range from cheapest to best:

**Selection combining** picks the strongest branch and ignores the rest. Simple — it needs only
a comparator — and it captures most of the diversity order, but it wastes the energy in the
branches it discards. Its average SNR gain over a single branch with `L` branches is the
harmonic-series sum `\sum_{k=1}^{L} 1/k`, so two branches give 1.5×, three give 1.83×, with
diminishing returns.

**Equal-gain combining** co-phases the branches and adds them with equal weight, using the
energy in all of them. Better than selection, and it needs only phase alignment, not amplitude
weighting.

**Maximal-ratio combining (MRC)** is optimal: it co-phases the branches and weights each by its
own SNR before summing, so strong branches contribute more and weak ones less. MRC achieves an
output SNR equal to the *sum* of the branch SNRs — the full theoretical diversity — and its gain
with `L` branches is exactly `L`× in the average. It is more complex, requiring per-branch SNR
estimation, but at high fading it recovers dramatically more than the simpler schemes.

The practical impact is large. Recall from Section 8.7 that Rayleigh fading costs up to 34 dB at
a BER of 10⁻⁵ relative to AWGN. Two-branch MRC recovers roughly 20 dB of that; three-branch,
still more. This is why antenna diversity — two antennas and an MRC combiner — is standard in
Wi-Fi and increasingly in BLE, and why its absence in a compact Sub-GHz device (where a
half-wavelength antenna separation is 17 cm and rarely available) is a real limitation that must
be compensated by other diversity forms: frequency hopping (Chapter 14) and coding across time
(Chapter 9). The diversity may come from space, frequency, time, or code, but *some* diversity
is not optional in a fading channel, and the combining method sets how much of it the system
keeps.

### 8.10 What Carries Forward

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

### 9.8 A Convolutional Encoder and Viterbi Decoder, Worked

Section 9.4 described convolutional codes and the Viterbi algorithm in outline. Because these
are the codes actually used in the BLE Coded PHY and in countless Sub-GHz systems, and because
they are the clearest illustration of how forward error correction turns redundancy into coding
gain, it is worth working a complete example small enough to trace by hand.

Consider a rate-1/2 convolutional encoder with constraint length `K = 3`. It has a two-bit shift
register holding the previous two input bits, and it produces two output bits for each input
bit, computed as modulo-2 sums (XORs) of the current and stored bits according to two generator
polynomials, conventionally written in octal as `(7, 5)`:

$$
g_1 = 111_2 \;(\text{output} = b_0 \oplus b_1 \oplus b_2), \qquad
g_2 = 101_2 \;(\text{output} = b_0 \oplus b_2)
$$

where `b_0` is the current bit and `b_1, b_2` are the two previous bits. The encoder has four
states, named by the two stored bits: `00`, `01`, `10`, `11`.

**Encoding the sequence `1 0 1 1`** (starting from state `00`):

| Input | State before | `g_1` output | `g_2` output | Output pair | State after |
|---|---|---|---|---|---|
| 1 | 00 | 1⊕0⊕0 = 1 | 1⊕0 = 1 | **11** | 10 |
| 0 | 10 | 0⊕1⊕0 = 1 | 0⊕0 = 0 | **10** | 01 |
| 1 | 01 | 1⊕0⊕1 = 0 | 1⊕1 = 0 | **00** | 10 |
| 1 | 10 | 1⊕1⊕0 = 0 | 1⊕0 = 1 | **01** | 01 |

The four input bits produce eight output bits: `11 10 00 01`. The redundancy is visible — each
input bit influences two output pairs, spreading its information across time, which is what lets
the decoder recover it even when some output bits are corrupted.

**Decoding with errors.** Suppose the channel corrupts the received stream to `11 10 01 01` — one
bit flipped in the third pair (`00` became `01`). A hard-decision Viterbi decoder finds the
transmitted sequence closest, in Hamming distance, to the received one, by walking the trellis
and at each step keeping, for every state, only the single most-likely path leading to it (the
"survivor") along with its accumulated distance (the "path metric").

At each trellis stage the decoder computes, for each of the four states, the distance between the
received pair and the output the encoder would have produced on each incoming transition, adds it
to the predecessor's metric, and keeps the smaller. Tracing this through, the path corresponding
to input `1 0 1 1` accumulates a total distance of 1 (the single flipped bit), while every
competing path accumulates more — because to explain the received sequence any other input would
require *two or more* bit errors, which is less likely. The decoder therefore selects `1 0 1 1`,
**correcting the channel error**. The single flipped bit was recovered because the code's memory
tied it to bits before and after, and no single-error explanation other than the true one was
consistent with the whole received sequence.

This is the mechanism behind every coding-gain figure in Section 9.4. The decoder does not
examine bits in isolation; it finds the most likely *sequence*, and the code is designed so that
the true sequence remains the most likely even after the channel has flipped several bits. The
constraint length sets how far the memory reaches and therefore how many errors can be
disentangled — and, exponentially, how many trellis states the decoder must track, which is why
`K = 7` (64 states) is about the practical ceiling for low-power hardware.

**The soft-decision improvement.** If instead of hard bits the demodulator provides confidences
(Section 9.4), the decoder replaces Hamming distance with Euclidean distance, weighting each
received bit by how sure the demodulator was. A bit the demodulator flagged as uncertain
contributes little to the path metrics, so a strong, confident bit is not overruled by a weak,
doubtful one. This is where the ~2 dB soft-decision gain comes from concretely: the decoder stops
treating a barely-decided bit and a firmly-decided bit as equally reliable, and that
extra information is worth 2 dB of transmit power — free, if the demodulator exports its
confidence rather than thresholding it away.

### 9.9 Reed–Solomon Codes and Burst Correction

Convolutional codes excel against scattered random errors. **Reed–Solomon (RS)** codes excel
against *bursts*, and because bursts are exactly what fading and interference produce, RS
appears wherever the channel damages contiguous runs of bits — optical transport, storage, deep
space, and several Sub-GHz standards. Its mechanism is worth understanding at the level of why it
works, because it is structurally different from the bit-level codes above.

RS operates on **symbols**, not bits — typically 8-bit symbols, so each symbol is a byte. An
RS(`n`, `k`) code takes `k` data symbols and produces `n` coded symbols by treating the data as
the coefficients of a polynomial over a finite field (Galois field GF(2⁸)) and evaluating or
extending it. The `n − k` added symbols are parity, and the code can correct up to `t = (n−k)/2`
symbol errors anywhere in the block.

The property that makes RS special is that it corrects **symbol** errors regardless of how many
bits within each symbol are wrong. RS(255, 223), the classic choice, adds 32 parity bytes and
corrects any 16 corrupted bytes. Whether a corrupted byte has one bit wrong or all eight wrong,
it counts as a single symbol error. This is precisely the right property for burst errors: a
burst that destroys, say, 100 consecutive bits corrupts at most 100/8 + 1 ≈ 14 bytes, well within
the 16-byte correction capacity of RS(255, 223) — whereas a bit-level code would see 100 bit
errors and be overwhelmed. RS turns the burst's concentration, which defeats bit-level codes,
into an advantage, because concentrating the damage into few symbols is exactly what keeps the
symbol-error count low.

This is why the strongest real systems **concatenate** codes: an inner convolutional code cleans
up the scattered random errors, and when it fails it fails in bursts (a Viterbi decoder that
loses the trellis produces a run of errors), which an outer Reed–Solomon code then mops up. The
combination — Viterbi inner, RS outer, with an interleaver between them to spread the inner
decoder's bursts — was the workhorse of deep-space and digital-television links for decades and
achieves within a couple of dB of the Shannon limit at a fraction of the complexity of the
iterative codes that later surpassed it. Each code covers the other's weakness, which is the same
layering principle that Chapters 8 and 14 apply to diversity and coexistence.

### 9.10 What Carries Forward

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

---

# Part III — Physical Media

---

# Chapter 10

## Wired Physical Layers: Copper, Differential Signaling, and Ethernet

### 10.1 Why Copper Still Wins Where It Competes

Fiber has more bandwidth. Wireless has no cables. Yet copper carries the overwhelming majority
of the world's data over its first and last few metres, and understanding why frames the whole
chapter. Copper is cheap, it carries power alongside data, its connectors are robust and
field-terminable, and — the decisive point — over short distances its bandwidth is far more
than enough. A category cable delivering 10 Gbps over 30 metres is competing against a
requirement, not against fiber's theoretical ceiling.

The wired channel is the deterministic regime introduced in Section 1.4. It is bounded,
shielded, and characterised to within a decibel. This is what allows wired PHYs to target bit
error rates of 10⁻¹² — figures that would be meaningless in the wireless world — and it is why
this chapter can quantify things that Chapters 12 onward can only bound statistically.

### 10.2 Voltage, Current, and Impedance

A copper conductor carries a signal as a voltage relative to a reference, or as a current
through the loop. Both are used, and the distinction matters.

**Voltage signalling** places information on the voltage level. It is simple and universal but
vulnerable: any noise voltage coupled onto the conductor adds directly to the signal, and the
receiver cannot distinguish them. Most short-range digital interfaces — a chip talking to
another chip on the same board — use single-ended voltage signalling because the noise
environment is benign.

**Current signalling** places information on the loop current. Because current is set by the
source and is the same at every point in a series loop, it is inherently more robust against
series voltage noise, and it dominates in industrial environments — the 4–20 mA current loop
is a century-old standard still ubiquitous in process control precisely because it survives
long runs through electrically hostile factories.

**Characteristic impedance** is the property that governs high-speed behaviour, and ignoring it
is the most common cause of signal-integrity failure. A transmission line has an impedance —
100 Ω for twisted-pair Ethernet, 50 Ω for most coaxial RF, 90 Ω for USB — determined by its
geometry and dielectric. When a signal reaches a point where the impedance changes — a
connector, a stub, an unterminated end — part of its energy **reflects** back toward the source.
The reflection coefficient is:

$$
\Gamma = \frac{Z_L - Z_0}{Z_L + Z_0}
$$

A line terminated in its own characteristic impedance (`Z_L = Z_0`) has `Γ = 0` — no reflection,
all energy absorbed. An open circuit (`Z_L = ∞`) reflects everything with `Γ = +1`; a short
(`Z_L = 0`) reflects everything inverted with `Γ = −1`. Reflections cause ringing, overshoot,
and ISI, and at high data rates they close the eye entirely. This is why every high-speed link
is impedance-controlled end to end and terminated at its ends.

### 10.3 Single-Ended Versus Differential Signaling

The single most important technique in wired PHY design is **differential signalling**, and its
value is worth developing carefully because the same idea reappears in Chapter 16's discussion
of receiver architecture.

A single-ended signal is one conductor referenced to ground. A differential signal uses two
conductors carrying equal and opposite voltages; the receiver responds to their *difference*.

The power of this arrangement is **common-mode rejection**. External noise — from a nearby
motor, a switching power supply, a radio transmitter — couples almost equally onto two
conductors that run side by side. On a single-ended line that noise adds to the signal. On a
differential pair it adds equally to both conductors, so it appears in the *sum* and cancels in
the *difference*. A well-balanced differential receiver rejects common-mode noise by 40–60 dB.

Differential signalling also produces almost no net radiated emission, because the equal and
opposite currents generate opposing magnetic fields that cancel in the far field. A pair that
does not radiate also does not easily receive — reciprocity works in both directions. This is
why every high-speed interface — Ethernet, USB, HDMI, PCIe, SATA, LVDS — is differential, and
why the twisting of the pair (Section 10.4) matters so much.

The cost is two conductors instead of one and a receiver that must be well-balanced. In every
high-speed application that cost is trivially justified.

### 10.4 Twisted Pair and the Cable Categories

Twisting the two conductors of a differential pair around each other accomplishes two things at
once. It keeps the conductors physically close so that external noise couples equally onto both
(preserving common-mode rejection), and it ensures that over any half-twist the geometry
relative to an external field reverses, so that induced currents cancel along the length. The
tighter the twist, the higher the frequency at which cancellation remains effective — which is
the physical basis of the cable category system.

| Category | Bandwidth | Max rate (100 m) | Twist / construction |
|---|---|---|---|
| Cat 5e | 100 MHz | 1 Gbps | Moderate twist, unshielded |
| Cat 6 | 250 MHz | 1 Gbps (10 Gbps ≤ 55 m) | Tighter twist, often a spline |
| Cat 6A | 500 MHz | 10 Gbps | Tight twist, improved shielding |
| Cat 7 | 600 MHz | 10 Gbps | Individually shielded pairs |
| Cat 8 | 2000 MHz | 25–40 Gbps (≤ 30 m) | Fully shielded, short reach |

Every pair within a cable has a slightly different twist rate. This is deliberate: if all four
pairs shared a twist rate, signals would couple resonantly between them. Differing the rates
decorrelates the coupling — a direct countermeasure against the crosstalk of the next section.

### 10.5 Attenuation and Crosstalk

Two mechanisms limit copper, and both worsen with frequency, which is why cable reach shrinks
as data rate rises.

**Attenuation** in copper is dominated by the **skin effect**: as frequency rises, current
crowds into the outer surface of the conductor, reducing the effective cross-section and
raising resistance. Loss therefore rises approximately with the square root of frequency. Cat
6A loses roughly 20 dB per 100 m at 100 MHz and roughly 45 dB at 500 MHz. This is a loss the
receiver's equaliser must undo, and there is a frequency beyond which no equaliser can recover
the signal from the noise — which sets the category's reach.

**Crosstalk** is the coupling of signal from one pair into another within the same cable, and in
short high-speed links it is usually the *dominant* impairment, exceeding attenuation. Two forms
matter:

- **NEXT (near-end crosstalk)** — coupling measured at the same end as the disturbing
  transmitter. It is the most damaging because the disturbing signal is at full strength while
  the victim's incoming signal has been attenuated by the full cable length. NEXT is why full
  duplex links need careful cancellation.
- **FEXT (far-end crosstalk)** — coupling measured at the far end, where the disturbing signal
  has also been attenuated. Less severe, but at 10 Gbps still significant enough to require
  equalisation.

10GBASE-T is essentially a war against crosstalk. It transmits and receives simultaneously on
all four pairs in both directions, so every pair is a victim of NEXT and FEXT from the other
three plus echo of its own transmission. The PHY contains digital cancellers that estimate and
subtract all of it — which is why a 10GBASE-T PHY consumes several watts and contains more
signal-processing silicon than an entire Sub-GHz transceiver.

**Alien crosstalk** — coupling between *adjacent cables* in a bundle — cannot be cancelled,
because the PHY has no access to the disturbing signal. It is the reason Cat 6A specifies
improved shielding and the reason dense cable bundles are derated.

### 10.6 Clock Recovery and Synchronization

A wired receiver faces the same timing problem as any other (Section 5.1): it must recover the
symbol clock from the data, because sending a separate clock line does not work at high speed —
the clock and data would skew relative to each other over the cable length.

The solution is **embedded clocking**. The line code guarantees enough transitions (Chapter 5),
and a **phase-locked loop** in the receiver locks onto those transitions and regenerates a
clock aligned to the incoming symbols. This is why the run-length guarantees of 8B/10B and the
scrambling of 64B/66B are not optional niceties — they are what keep the PLL fed.

The residual timing error after recovery is **jitter**, and the eye diagram (Chapter 23) is the
standard tool for measuring how much timing and amplitude margin remains. A link fails when
jitter plus noise closes the eye at the sampling instant.

### 10.7 Why Wired Achieves Rates Wireless Cannot

It is worth stating explicitly why copper reaches 10 Gbps over 100 m while a Sub-GHz radio
struggles to reach 250 kbps over a kilometre, because the contrast illuminates both.

- **Bandwidth is available.** Cat 6A offers 500 MHz of controlled bandwidth with no regulator
  to answer to. A Sub-GHz radio has tens of kilohertz and a rulebook.
- **SNR is enormous.** A shielded, terminated, characterised cable delivers SNRs of 30–40 dB,
  permitting high-order modulation (PAM-16). A wireless link fights for every dB above a noise
  floor it shares with the world.
- **The channel is static.** No fading, no Doppler, no moving interferers. The equaliser solves
  a fixed problem once, not a time-varying one continuously.
- **Power is not the constraint.** A wired PHY may spend watts on signal processing. A
  coin-cell radio may spend milliwatts.

Every one of these advantages is precisely what Chapter 12 will show the wireless channel
lacks. Copper's determinism is the luxury; wireless's variability is the norm.

### 10.8 The Transmission Line as a Distributed Circuit

Chapter 10 has treated impedance and reflection descriptively; the underlying model is the
**distributed-element transmission line**, and understanding it explains where characteristic
impedance comes from, why it is real (resistive) even for a lossless line, and how signals
actually propagate down a cable. This is the physical foundation beneath every high-speed wired
link, and an engineer who grasps it stops treating impedance matching as a rule to follow and
starts treating it as a consequence to reason about.

A short section of cable has four properties per unit length: series inductance `L` (the
magnetic field around the conductors), series resistance `R` (the conductor's ohmic loss), shunt
capacitance `C` (the electric field between the conductors), and shunt conductance `G` (leakage
through the dielectric). A real line is an infinite ladder of these elements, and analysing that
ladder yields the **characteristic impedance**:

$$
Z_0 = \sqrt{\frac{R + j\omega L}{G + j\omega C}}
$$

For a low-loss line at high frequency, where `\omega L \gg R` and `\omega C \gg G`, this
simplifies to the striking result:

$$
Z_0 \approx \sqrt{\frac{L}{C}}
$$

— a *real* number, independent of frequency and independent of length, set purely by the cable's
geometry. This is why a 100 Ω cable is 100 Ω whether it is one metre or one hundred, and why the
impedance is resistive even though it is built from purely reactive inductance and capacitance:
the ratio of the two, in the ladder, produces a resistance. The signal propagates as a wave down
this ladder at a velocity `v = 1/\sqrt{LC}`, typically 60–80% of the speed of light in common
cables, and the **propagation delay** this implies — roughly 5 ns per metre — is what causes the
skew that forbids sending a separate clock line at high speed (Section 10.6).

The reflection coefficient of Section 10.2 now has a physical meaning: when the wave reaches a
point where `L/C` changes — a connector, a stub, an unterminated end — the ladder's impedance
changes, and the wave cannot fully transfer into the new section, so part of it reflects. A
termination resistor equal to `Z_0` makes the line's end look like more line of the same
impedance, so the wave sees no discontinuity and is fully absorbed. This is why termination
values are not arbitrary: they must equal the characteristic impedance the geometry produced, and
a 50 Ω line terminated in 75 Ω reflects `(75-50)/(75+50) = 0.2`, sending 4% of the power back to
corrupt subsequent symbols.

### 10.9 The Eye Diagram, Quantified

Section 10.6 introduced the eye diagram as a health check; it is worth developing as a
quantitative tool, because it is the single most information-dense measurement in wired PHY work
and the vocabulary it establishes — eye height, eye width, jitter — recurs in every high-speed
debugging session.

An eye diagram is built by overlaying many symbol periods of the received signal on the same time
axis, triggered on the recovered clock. The overlaid traces form an "eye" — an open region in the
centre where, if the receiver samples there, it will correctly distinguish the symbol levels. The
eye's dimensions measure the two margins the link has left:

- **Eye height** (vertical opening) is the voltage margin between the levels at the sampling
  instant, after noise and interference have narrowed it. It measures margin against amplitude
  noise: a tall eye tolerates more noise before the receiver's slicer makes an error. Noise, ISI
  (Section 3.4), and crosstalk (Section 10.5) all close the eye vertically.
- **Eye width** (horizontal opening) is the time margin — the span over which the sampling
  instant may fall and still land in the open region. It measures margin against **jitter**, the
  timing uncertainty in the recovered clock. A wide eye tolerates more jitter before the receiver
  samples at the wrong instant.

A receiver makes an error when noise and jitter together close the eye at the sampling point, so
the two margins trade against each other and the total error rate depends on both. The
relationship to the bit error rate is direct: the eye must stay open with enough margin that the
combined noise-and-jitter distribution reaches the decision boundary only with the target
probability — for a 10⁻¹² wired link, the eye must remain open across roughly ±7 standard
deviations of the combined impairment, which is why wired links that target such low error rates
demand eyes that look, to the untrained observer, absurdly wide open.

Jitter itself decomposes into components that point to different causes, and separating them is a
standard diagnostic step:

| Jitter type | Nature | Typical cause |
|---|---|---|
| Random jitter | Unbounded, Gaussian | Thermal noise, oscillator phase noise |
| Deterministic — periodic | Bounded, repeating | Crosstalk from a periodic aggressor, power-supply ripple |
| Deterministic — data-dependent | Bounded, correlated with data | ISI, reflections, insufficient equalisation |
| Duty-cycle distortion | Bounded | Asymmetric rise/fall times, threshold offset |

The value of the decomposition is that each component names a different fix: random jitter is
attacked by improving the oscillator and reducing noise; data-dependent jitter by better
equalisation or termination; periodic jitter by finding and isolating the aggressor. An eye that
is closing because of data-dependent jitter and one closing because of random jitter look
different on the instrument and demand opposite responses, and reading the eye correctly is what
tells them apart — the wired analogue of the impairment-diagnosis table of Section 3.8.

### 10.10 What Carries Forward

- Copper is the deterministic regime: bounded, static, high-SNR, targeting BER 10⁻¹².
- Impedance control and termination eliminate reflections; differential signalling delivers
  40–60 dB of common-mode rejection and near-zero radiated emission.
- Attenuation (skin effect, ∝ √f) and crosstalk (NEXT/FEXT) both worsen with frequency and set
  cable reach; short high-speed links are crosstalk-limited.
- Wired rates exceed wireless because bandwidth, SNR, channel stability, and power budget are
  all vastly more favourable — the exact opposite of Chapter 12's conditions.

---

# Chapter 11

## Optical Physical Layers: Fiber, Transceivers, and Dispersion

### 11.1 Why Light

Copper's limits are set by the frequencies it can carry — hundreds of megahertz to a few
gigahertz. An optical carrier oscillates at roughly 193 **tera**hertz (the 1550 nm band), five
orders of magnitude higher, and even a modulation occupying a tiny fractional bandwidth of that
carrier represents tens of gigahertz of usable signal bandwidth. This is the fundamental reason
fiber dominates long-distance and high-capacity links: the carrier frequency is so high that
bandwidth ceases to be the binding constraint.

Fiber's second decisive advantage is loss. Section 3.2 quoted the numbers: roughly 0.35 dB/km
at 1310 nm and 0.20 dB/km at 1550 nm, against copper's tens of dB per 100 m. A signal can
travel 80 km of fiber and lose less power than it would in 100 m of Cat 6A. Add immunity to
electromagnetic interference (light in a dielectric does not couple to external fields), no
crosstalk between fibers, no radiated emission, and galvanic isolation, and the case for fiber
over any distance beyond a building is overwhelming.

### 11.2 How Light Is Guided

An optical fiber is a cylinder of glass with a **core** of slightly higher refractive index
surrounded by a **cladding** of lower index. Light entering the core within a certain angle
undergoes **total internal reflection** at the core–cladding boundary and is trapped, bouncing
along the fiber for kilometres. The maximum acceptance angle is captured by the **numerical
aperture**:

$$
\text{NA} = \sqrt{n_{core}^2 - n_{cladding}^2}
$$

The index difference is tiny — typically under 1% — but sufficient to guide light with
remarkable efficiency.

The single most important structural distinction in fiber is core diameter, because it
determines how many distinct paths (modes) light can take.

**Multimode fiber** has a large core, typically 50 or 62.5 µm, wide enough that light propagates
along many distinct paths simultaneously. Because different paths have different lengths, a
pulse launched as a sharp edge arrives smeared — this is **modal dispersion**, and it is the
dominant limit on multimode reach. Multimode is cheap to source (it works with inexpensive LEDs
and VCSELs and tolerates loose alignment) and is used for short reaches inside data centres and
buildings — hundreds of metres, not kilometres.

**Single-mode fiber** has a core of roughly 9 µm, small enough that only one path exists. Modal
dispersion vanishes entirely, and reach extends to tens or hundreds of kilometres. The cost is
tighter tolerances and more expensive laser sources. Every long-haul and metro link is
single-mode.

| Property | Multimode | Single-mode |
|---|---|---|
| Core diameter | 50 / 62.5 µm | ~9 µm |
| Modes | Many | One |
| Dominant dispersion | Modal | Chromatic |
| Source | LED / VCSEL | Laser (DFB, tunable) |
| Reach | ≤ ~550 m | Tens to hundreds of km |
| Cost | Lower | Higher |
| Typical use | Data-centre, in-building | Metro, long-haul, submarine |

### 11.3 Transmitters and Receivers

**Transmitters** convert current to light. A **light-emitting diode** is cheap and broad-spectrum,
suitable only for low rates over multimode. A **laser diode** — Fabry–Pérot, or the narrower
distributed-feedback (DFB) type — produces coherent, near-monochromatic light suitable for high
rates over single-mode. A **VCSEL** (vertical-cavity surface-emitting laser) sits between the
two in cost and performance and dominates short-reach multimode links.

Modulation is applied in one of two ways. **Direct modulation** switches the laser drive current
on and off, which is simple but induces **chirp** — the laser's wavelength shifts slightly during
switching, worsening chromatic dispersion and limiting reach. **External modulation** keeps the
laser running continuously and modulates the light afterward with a separate device (a
Mach–Zehnder or electro-absorption modulator), eliminating chirp at higher cost. Long-haul
systems use external modulation; short reaches use direct.

**Receivers** convert light back to current with a **photodiode**. A **PIN** photodiode is simple
and used at short reach. An **avalanche photodiode (APD)** provides internal gain — each photon
releases multiple electrons — giving 5–10 dB better sensitivity at the cost of higher voltage
and more noise, used where reach must be maximised.

The receiver's own noise sets its sensitivity, exactly as in the RF case. The two dominant
noise mechanisms are **shot noise** (the discrete, Poisson-distributed arrival of photons — a
fundamental quantum limit that improves with more received power) and **thermal noise** in the
transimpedance amplifier that follows the photodiode (independent of signal, dominant at low
light levels).

### 11.4 Optical Modulation

Most deployed optical links use the simplest possible modulation: **on-off keying**, called
**NRZ-OOK** in this context — light on for a 1, off for a 0. It works because fiber's SNR is
high and its bandwidth is abundant; there is no need for spectral efficiency when spectrum is
effectively free. Direct detection with a photodiode recovers it trivially.

As per-wavelength rates climbed past 10 Gbps, simple OOK ran into dispersion limits, and the
industry adopted the same techniques the RF world had used for decades:

- **PAM-4** — four intensity levels carrying two bits per symbol, halving the symbol rate for a
  given bit rate. Standard for 100G and 400G short-reach links (100GBASE and 400GBASE).
- **Coherent detection with QPSK and 16-QAM** — for long-haul, the receiver mixes the incoming
  light with a local laser (exactly analogous to RF coherent detection), recovering both
  amplitude and phase, and enabling polarisation multiplexing that doubles capacity again. A
  modern long-haul channel carries 400 Gbps or more using dual-polarisation 16-QAM.

The convergence is worth noting: optical and RF communications, which developed largely
independently, have arrived at the same modulation and coding toolkit, because they are both
governed by Shannon.

### 11.5 The Two Impairments That Matter: Attenuation and Dispersion

Fiber has essentially two enemies, and unlike the wireless channel neither is random.

**Attenuation** was covered above. It is low, predictable, and overcome by amplification.
Erbium-doped fiber amplifiers (EDFAs) boost the 1550 nm band optically without converting back
to electrical form, which is why 1550 nm is the long-haul band of choice — it aligns with both
the loss minimum and the amplifier's gain window.

**Dispersion** is pulse spreading, and it is the true reach limit at high rates. Three types:

- **Modal dispersion** (multimode only) — different paths, different lengths. Eliminated by
  single-mode fiber.
- **Chromatic dispersion** — different wavelengths travel at slightly different speeds, so a
  pulse containing a spread of wavelengths (every real pulse does) spreads in time. It
  accumulates linearly with distance and is compensated with dispersion-compensating fiber or
  in the digital domain by coherent receivers.
- **Polarisation-mode dispersion** — the two polarisation states travel at slightly different
  speeds due to fiber imperfections. Small, but a limit at 40 Gbps and above, and awkward
  because it varies randomly with temperature and stress.

Dispersion sets the **bandwidth-distance product** — a fiber rated at 500 MHz·km supports 500
MHz over 1 km, or 250 MHz over 2 km, or 1 GHz over 500 m. It is the optical analogue of the
Nyquist limit and the single most useful figure for reach planning.

### 11.6 Optical Link Budgets

An optical link budget is conceptually identical to the RF link budget of Chapter 13, and
simpler because the channel is deterministic:

$$
P_{received} = P_{launched} - \alpha L - L_{connectors} - L_{splices} - M
$$

**Worked example.** A single-mode link at 1310 nm: launch power 0 dBm; fiber loss 0.35 dB/km
over 40 km = 14 dB; four connectors at 0.5 dB = 2 dB; ten splices at 0.1 dB = 1 dB; system
margin 3 dB.

$$
P_{received} = 0 - 14 - 2 - 1 - 3 = -20\ \text{dBm}
$$

If the receiver's sensitivity is −28 dBm, the link closes with 8 dB to spare. Because the
channel does not fade, that 8 dB is real margin, not a hedge against variability — which is
exactly the luxury the wireless engineer of Chapter 13 does not have.

### 11.7 Wavelength-Division Multiplexing: Many Colours in One Fiber

The single most important reason fiber's capacity dwarfs every other medium is not the raw
bandwidth of one wavelength but the ability to run *many independent wavelengths down the same
fiber simultaneously* — **wavelength-division multiplexing (WDM)**. It is the optical analogue of
the frequency-division multiplexing that lets many radio stations share the air, and it turns a
single fiber into dozens or hundreds of parallel channels.

A WDM system assigns each data stream a slightly different wavelength — a different colour of
light — and combines them into one fiber with a passive optical multiplexer that is,
essentially, a wavelength-selective prism. At the far end a demultiplexer separates them again,
each to its own receiver. Because the wavelengths do not interfere (they are orthogonal in
exactly the sense Section 2.2 established for different frequencies), each carries its full data
rate independently, and the fiber's capacity multiplies by the number of channels.

The systems are classified by channel spacing:

| System | Channel spacing | Channels | Typical use |
|---|---|---|---|
| CWDM (coarse) | 20 nm | ~8–18 | Metro, lower cost, uncooled lasers |
| DWDM (dense) | 0.8 nm (100 GHz) | 40–96 | Long-haul, high capacity |
| DWDM (ultra-dense) | 0.4 nm (50 GHz) or less | 96+ | Submarine, maximum capacity |

A modern DWDM system carrying 96 channels, each at 400 Gbps, delivers 38.4 Tbps down a single
fiber pair — a figure no other medium approaches, and one achieved by exploiting the enormous
optical bandwidth that Section 11.1 identified as fiber's fundamental advantage. The tight
channel spacing of DWDM demands wavelength-stable, temperature-controlled lasers, which is the
principal cost driver and the reason CWDM, with its relaxed spacing and uncooled sources,
remains attractive for shorter, less capacity-hungry links.

### 11.8 Optical Amplification and the Reach of Long-Haul Systems

Section 11.5 noted that attenuation is overcome by amplification; the way this is done is central
to why fiber spans continents, and it is a genuinely elegant piece of engineering that deserves
more than the passing mention it received.

The naive approach to a long link — convert light to electrical signal, amplify, convert back —
is called optical-electrical-optical (O-E-O) regeneration and has a fatal cost at scale: it
requires a full receiver and transmitter at every amplification site, one per wavelength, so a
96-channel WDM system would need 96 receiver-transmitter pairs at every hut along the route. This
is prohibitive, and WDM would be impractical if it were the only option.

The **erbium-doped fiber amplifier (EDFA)** solves it. A length of fiber doped with erbium ions,
pumped with light at 980 or 1480 nm, amplifies *all wavelengths in the 1550 nm band at once,
optically, without conversion to electrical form*. A single EDFA amplifies the entire WDM comb —
all 96 channels — in one passive device, and it does so with a gain of 20–40 dB and a low enough
noise figure to be cascaded through dozens of spans. This is why 1550 nm is the long-haul band:
it is not merely fiber's loss minimum (Section 11.1) but the exact window where erbium provides
gain, and the two coinciding is the enabling coincidence of modern optical networking.

The limit on amplified reach is not gain but accumulated noise. Each EDFA adds a little
amplified spontaneous emission — the optical equivalent of thermal noise — and after enough spans
the accumulated noise degrades the SNR (the optical signal-to-noise ratio) below what the
receiver needs. Ultra-long-haul and submarine systems manage this with careful per-span power
budgeting, occasional electrical regeneration, and increasingly with the coherent detection and
forward error correction of the next section, which together push transatlantic distances on a
single amplified chain with no electrical regeneration in between.

### 11.9 Coherent Optical Detection and the Convergence With Radio

Section 11.4 mentioned that long-haul systems adopted coherent detection; the parallel with the
radio receivers of Chapters 16 and 17 is exact and worth drawing explicitly, because it shows
that optical and RF engineering, having developed in separate communities, have converged on the
same solution to the same problem.

Direct detection — a photodiode measuring intensity (Section 11.3) — recovers only optical power,
discarding phase and polarisation, exactly as a non-coherent radio receiver discards carrier
phase (Section 6.8). It is simple and adequate for on-off keying, and for the same reason
non-coherent FSK is adequate for low-power radio: when the modulation carries information only in
intensity, phase recovery buys nothing.

**Coherent optical detection** mixes the incoming light with a local laser — a "local
oscillator", in precisely the radio sense — and recovers amplitude, phase, and both polarisations.
This is the optical implementation of the I/Q receiver of Chapter 17, and it enables the same
capabilities: higher-order modulation (optical QPSK and 16-QAM), polarisation multiplexing (two
independent data streams on the two polarisations, doubling capacity), and digital compensation
of channel impairments. A modern coherent optical receiver runs the incoming light through the
same conceptual chain a radio modem runs — down-conversion against a local oscillator, ADC, and
digital signal processing that corrects chromatic dispersion, polarisation-mode dispersion, and
frequency offset in the digital domain — recovering electronically what earlier systems needed
dispersion-compensating fiber to fix physically.

The convergence is complete and instructive: a 400 Gbps coherent optical channel and a 5G radio
channel are, at the signal-processing level, the same machine operating at different carrier
frequencies. Dual-polarisation 16-QAM with coherent detection and LDPC coding is the optical
expression of exactly the techniques Part IV develops for radio, and both are governed by the
same Shannon bound of Chapter 4. An engineer who understands the radio modem understands the
optical one, and vice versa — which is why this book treats them as one subject viewed through
two media rather than two subjects that happen to resemble each other.

### 11.10 What Carries Forward

- Fiber's ~193 THz carrier makes bandwidth a non-constraint; its ~0.2 dB/km loss makes distance
  one.
- Single-mode eliminates modal dispersion and reaches hundreds of km; multimode is cheap and
  short.
- Optical links converged on the same modulation/coding toolkit as RF (PAM-4, coherent QPSK/QAM)
  as rates rose — Shannon governs both.
- The two impairments are attenuation (predictable, amplified away) and dispersion (the real
  reach limit, quantified by the bandwidth-distance product).
- The optical link budget is the RF link budget without the fading margin.

---

# Chapter 12

## The Wireless Channel: Path Loss, Multipath, Fading, and Doppler

### 12.1 The Hardest Channel

Copper and fiber are engineered environments. The wireless channel is found, not built. It is
shared with every other transmitter in range, unbounded, and — the property that dominates
everything else — **time-varying on every scale from microseconds to seasons**. A link that works
perfectly can fail when a door closes, when a person walks between the nodes, when a truck
parks outside, or when the humidity changes.

This chapter consolidates the propagation material that Chapters 3 and 8 introduced and
develops the statistics properly. It is the physical foundation for the entire link-budget and
interference analysis of Part IV, and it is where the probabilistic character of wireless PHY
design (Section 1.4) comes from.

### 12.2 Free-Space Path Loss, Derived

Chapter 3 stated the free-space path loss formula; here is where it comes from, because the
derivation makes the frequency dependence intuitive rather than arbitrary.

An isotropic transmitter radiating power `P_t` spreads it over the surface of an expanding
sphere. At distance `d`, the power density is `P_t / (4πd²)`. A receiving antenna captures power
proportional to its **effective aperture**, which for an isotropic antenna is `λ²/(4π)`.
Multiplying:

$$
\frac{P_r}{P_t} = \left(\frac{\lambda}{4\pi d}\right)^2 = \left(\frac{c}{4\pi d f}\right)^2
$$

The frequency dependence enters through the *aperture*, not the propagation: free space does
not attenuate more at higher frequency; rather, a fixed-gain antenna captures a smaller
physical area at shorter wavelength. This distinction matters — it means the Sub-GHz advantage
of Section 3.2 can be partly clawed back at 2.4 GHz by using a *higher-gain* antenna, since gain
compensates for the smaller aperture. It is a real design lever.

In the engineering form (distance in km, frequency in MHz):

$$
\text{FSPL}_{dB} = 20\log_{10}(d) + 20\log_{10}(f) + 32.44
$$

### 12.3 Real Environments: The Path-Loss Exponent

Free space is an idealisation almost never encountered. Real environments have ground
reflections, obstacles, and absorption, and empirically the received power falls faster than
the square of distance. The general model:

$$
PL(d) = PL(d_0) + 10\,n\log_{10}\!\left(\frac{d}{d_0}\right)
$$

where `n` is the **path-loss exponent**, equal to 2 in free space and larger in cluttered
environments.

| Environment | Path-loss exponent n |
|---|---|
| Free space | 2.0 |
| Urban line-of-sight | 2.0–2.5 |
| Suburban | 2.7–3.5 |
| Indoor, line-of-sight | 1.6–1.8 |
| Indoor, obstructed | 4.0–6.0 |
| Dense urban, non-line-of-sight | 3.0–5.0 |

The indoor line-of-sight figure below 2 is not an error: a corridor can act as a waveguide,
channelling energy and producing *less* loss than free space. Conversely, an obstructed indoor
path with `n = 5` loses power catastrophically fast. This spread is why a link budget for a
real deployment must use a measured or conservatively assumed exponent — using `n = 2` for an
indoor obstructed link overestimates range by a factor that can exceed ten.

**Worked comparison.** At 868 MHz, extending from 100 m to 200 m:
- Free space (`n = 2`): additional loss `= 10 × 2 × log(2) = 6.0 dB`.
- Obstructed indoor (`n = 5`): additional loss `= 10 × 5 × log(2) = 15.0 dB`.

The same doubling of distance costs 6 dB in one environment and 15 dB in the other. A 15 dB
change is the difference between a solid link and no link at all.

### 12.4 Shadowing: The Slow Random Variation

On top of the average path loss, real received power varies randomly from location to location
because of large obstacles — walls, buildings, terrain. This is **shadowing** or **slow fading**,
and it is well modelled as **log-normal**: the received power in dB is Gaussian-distributed about
the path-loss prediction, with a standard deviation `σ` of typically 4–12 dB.

The consequence for design is a **shadowing margin**. If a link must work at a given location
with 95% probability, and `σ = 8 dB`, the budget must include `1.64 × 8 ≈ 13 dB` of margin above
the median prediction — because 95% of a Gaussian lies within 1.64 standard deviations. This is
real margin that must be paid for in transmit power, antenna gain, or sensitivity, and omitting
it is why field coverage so often falls short of a budget computed from median path loss alone.

### 12.5 Multipath: The Fast Random Variation

A transmitted signal reaches the receiver by many paths simultaneously — direct, plus
reflections off walls, floor, ceiling, ground, and every object in the environment. These
copies arrive with different delays and phases and **add as vectors**. Depending on the phase
alignment, they can reinforce (constructive) or cancel (destructive).

This is **multipath fading** or **fast fading**, and its defining property is that it varies over
a *half wavelength* of movement — 17 cm at 868 MHz, 6 cm at 2.44 GHz. Move a node by that
distance and the received power can change by 20–30 dB. This is the physical reason a wireless
link can be perfect in one spot and dead a few centimetres away, and why moving your phone
slightly changes the signal bars.

The statistics depend on whether a dominant path exists:

- **Rayleigh fading** — no line-of-sight component; all paths are comparable reflections. The
  received amplitude follows a Rayleigh distribution. This is the pessimistic case and the one
  Chapter 8's 34 dB penalty was computed for.
- **Rician fading** — a dominant line-of-sight path plus weaker reflections. Characterised by the
  **K-factor** — the ratio of dominant-path power to scattered power. As K → ∞ the channel
  approaches free space; as K → 0 it approaches Rayleigh.

### 12.6 Frequency Selectivity and Coherence Bandwidth

Because multipath copies arrive at different delays, the channel affects different frequencies
differently — a null that cancels one frequency may leave a nearby frequency untouched. The
**delay spread** `τ` (the spread of arrival times) determines the **coherence bandwidth**:

$$
B_c \approx \frac{1}{5\tau}
$$

the band over which the channel is approximately flat.

This is the parameter that divides narrowband from wideband systems, and the division is not
arbitrary — it is a comparison between signal bandwidth and coherence bandwidth:

- **If signal bandwidth < coherence bandwidth** — the whole signal fades together (**flat
  fading**). This is the Sub-GHz narrowband regime. A fade takes out the entire signal, which is
  why frequency diversity (retry on another channel) is the remedy.
- **If signal bandwidth > coherence bandwidth** — different parts of the signal fade
  independently (**frequency-selective fading**). This causes inter-symbol interference and
  requires equalisation or OFDM. This is the wideband Wi-Fi regime.

**Typical delay spreads:** indoor 10–50 ns (coherence bandwidth 4–20 MHz); outdoor urban
0.5–5 µs (coherence bandwidth 40–400 kHz). A BLE 2 MHz channel indoors is comfortably within
coherence bandwidth and fades flat; a 20 MHz Wi-Fi channel indoors spans several coherence
bandwidths and is frequency-selective — which is exactly why Wi-Fi uses OFDM and BLE does not.

### 12.7 Doppler and Time Selectivity

When transmitter, receiver, or the scattering environment moves, the multipath structure
changes over time, and the received frequency shifts. The **Doppler shift** for relative velocity
`v`:

$$
f_d = \frac{v}{c}\,f_c
$$

**Worked example.** A person walking at 1.5 m/s carrying an 868 MHz Sub-GHz tag:

$$
f_d = \frac{1.5}{3 \times 10^8} \times 868 \times 10^6 = 4.3\ \text{Hz}
$$

Small in absolute terms, but its inverse sets the **coherence time** — the interval over which
the channel stays roughly constant:

$$
T_c \approx \frac{1}{f_d} \approx 230\ \text{ms}
$$

The coherence time is the parameter that governs whether time diversity works. A retransmission
sent *within* the coherence time sees the same fade and probably fails again; a retransmission
sent *after* it sees an independent channel and probably succeeds. For the walking tag above,
retries should be spaced by more than ~200 ms to gain time diversity — a design rule that falls
directly out of the Doppler calculation and is otherwise entirely non-obvious.

For a fast-moving vehicle at 30 m/s the coherence time drops to ~11 ms, and for a
100 km/h vehicle at 2.4 GHz to under 2 ms — fast enough that the channel changes within a
single long packet, which is why vehicular links use short frames and aggressive interleaving.

### 12.8 Combating the Wireless Channel

The remedies were introduced in Chapter 8; here they are tied to the mechanism each addresses,
which is the correct way to choose among them:

| Impairment | Time scale | Remedy |
|---|---|---|
| Path loss | Static | Link budget, TX power, antenna gain, lower frequency |
| Shadowing | Metres / slow | Shadowing margin (log-normal), site selection |
| Flat fading | Half-wavelength | Frequency diversity (hopping), spatial diversity |
| Frequency-selective fading | Half-wavelength, across band | Equalisation, OFDM, spread spectrum |
| Doppler / time selectivity | Coherence time | Time diversity, interleaving, pilot tracking |

The unifying idea is **diversity**: obtain multiple independent observations of the channel so
that the probability of *all* of them being faded is small. With `L` independent branches, the
outage probability falls as the `L`th power. This is why BLE hops, why good receivers use two
antennas, and why coding is spread across time and frequency — all four are diversity in
different dimensions, and a robust wireless system uses several at once.

### 12.9 Why This Chapter Governs Part IV

Every number in the wireless link budget of Chapter 13 is really a distribution, not a value.
Path loss has a shadowing spread. Received power has a fading distribution. Interference is
bursty and time-varying. The engineering response is not to compute a single link margin but
to compute margins for each source of variability and add them:

$$
M_{total} = M_{fading} + M_{shadowing} + M_{interference} + M_{implementation}
$$

A wireless link budget that reports a single deterministic margin, as a copper or fiber budget
legitimately can, has misunderstood the channel. Chapter 13 does it correctly.

### 12.10 The Statistics of Fading, in Detail

Section 12.5 named the Rayleigh and Rician distributions; because every wireless margin in
Chapter 13 is ultimately a statement about these distributions, it is worth developing them
enough to compute with, and to understand *why* fading is as punishing as Chapter 8's numbers
showed.

In a multipath channel with no dominant path, the received signal is the sum of many
independent reflections with random phases. By the central limit theorem, the in-phase and
quadrature components of this sum are each Gaussian, and the *magnitude* of a
complex Gaussian is **Rayleigh-distributed**:

$$
p(r) = \frac{r}{\sigma^2}\exp\!\left(-\frac{r^2}{2\sigma^2}\right), \qquad r \ge 0
$$

The received *power*, being magnitude squared, is exponentially distributed. The crucial feature
is the behaviour near zero: the probability that the received power falls more than `x` dB below
its average is, for deep fades, approximately `10^{-x/10}` — so a fade 20 dB deep occurs about 1%
of the time, and a fade 30 dB deep about 0.1%. These deep fades are rare but not negligible, and
they are the entire reason a link that is fine "on average" fails intermittently: the average is
dominated by the good moments, while the outages come from the exponential tail near zero, which
the average conceals. This is the statistical statement of the 34 dB Rayleigh penalty of
Section 8.7 — to keep the error rate low even during the deep fades, a non-diverse system must
run its average SNR enormously high.

When a dominant line-of-sight path is present, the distribution becomes **Rician**, parameterised
by the K-factor — the ratio of dominant-path power to scattered power:

$$
K = \frac{\text{dominant power}}{\text{scattered power}}
$$

As `K \to 0` the Rician distribution becomes Rayleigh (no dominant path); as `K \to \infty` it
approaches a constant (pure line-of-sight, no fading). The K-factor is the single number that
captures how benign or hostile a fading environment is, and it varies enormously: an open
outdoor line-of-sight link might have `K = 10` (10 dB) and fade gently, while a non-line-of-sight
indoor link might have `K = 0` and fade fully Rayleigh. The fade margin of Chapter 13 depends
directly on K — a high-K link needs only a few dB, a Rayleigh link needs the full 20–30 dB —
which is why the same nominal link budget can succeed in one environment and fail in another, and
why "it worked in an open field" predicts little about indoor performance.

### 12.11 The Two-Ray Ground-Reflection Model

Free-space path loss and the statistical models bracket the extremes; a specific, deterministic
model — the **two-ray ground-reflection model** — fills the important middle case of an
outdoor link over flat ground, and it produces a result that surprises anyone expecting simple
inverse-square behaviour.

Consider a transmitter and receiver above a reflecting ground plane. The receiver gets two rays:
the direct path and a ground reflection. The two travel slightly different distances, so they
arrive with a phase difference that depends on antenna heights and separation, and they add as
vectors — sometimes reinforcing, sometimes cancelling. At short range the phase difference sweeps
rapidly, producing an oscillating received power (a series of peaks and nulls) as distance
changes; this is why an outdoor link can show sharp signal variations over short distance changes
even with no obstacles.

Beyond a **breakpoint distance**, the geometry settles and the model yields a clean asymptotic
result:

$$
P_r \propto \frac{h_t^2 h_r^2}{d^4}
$$

The received power falls as the *fourth* power of distance, not the second — a path-loss exponent
of 4, far steeper than free space. This is not an anomaly; it is the normal behaviour of a
ground-based outdoor link beyond its breakpoint, and it explains why real outdoor ranges fall so
much shorter than a free-space calculation predicts. It also reveals a design lever hidden in the
`h_t^2 h_r^2` term: raising either antenna increases received power by the square of the height,
so mounting a gateway antenna higher is often the cheapest available range improvement — doubling
the height quadruples the received power, worth 6 dB, before any change to power or sensitivity.
The two-ray model is the quantitative reason antenna height matters so much in outdoor
deployments, and it belongs in every outdoor link budget as a check against the optimism of the
free-space figure.

### 12.12 Diversity Order and the Slope of the Error Curve

Section 8.9 quantified diversity combining; the channel's-eye view completes the picture and
explains *why* diversity works at the level of the error-rate curve's shape, which is the most
useful way to think about it in system design.

Without diversity, a Rayleigh channel's error rate falls only *inversely* with average SNR
(Section 8.7) — a slope of 1 on a log-log plot. This shallow slope is the visible signature of the
deep-fade tail: because outages come from the channel occasionally being near zero, throwing more
average power at the problem helps only linearly, since it does not change how often the channel
visits the deep-fade region.

Diversity changes the slope. With `L` independent branches, the probability that *all* of them are
simultaneously in a deep fade is the product of the individual probabilities, so the error rate
falls as SNR to the power `L` — a slope of `L` on the log-log plot. This is the **diversity
order**, and it is the single most important number describing a fading-mitigation scheme. A
diversity order of 1 (no diversity) gives the punishing shallow slope; order 2 (two independent
branches) squares the tail probability and steepens the curve dramatically; order 3 and beyond
approach the steep, near-vertical waterfall of the AWGN channel.

The practical reading is that diversity does not merely shift the error curve left (as more power
would); it *rotates* it steeper, so that its benefit grows without bound as the target error rate
falls. At a modest error rate the diversity gain is a few dB; at a stringent one it is tens of
dB, because the shallow non-diverse curve and the steep diverse curve diverge ever further as
they descend. This is why diversity is indispensable precisely where reliability matters most,
and why every design that must work 99.9% of the time in a fading channel — not 90% — must obtain
diversity order greater than one from *some* dimension: space, frequency, time, or code. The
dimension is a design choice; the necessity is not.

### 12.13 What Carries Forward

- FSPL's frequency dependence comes from antenna aperture, not propagation — so antenna gain is
  a real lever against the Sub-GHz/2.4 GHz gap.
- Real path loss follows `n` between 2 and 6; using the wrong exponent can overestimate range
  tenfold.
- Shadowing is log-normal and demands a margin of `z·σ` (≈13 dB for 95% at σ = 8 dB).
- Multipath fades over a half wavelength, costs 20–30 dB, and is Rayleigh (no LOS) or Rician
  (LOS present).
- Coherence bandwidth divides flat from selective fading; coherence time divides slow from fast,
  and sets the minimum retry spacing for time diversity.
- Diversity in frequency, space, time, or coding is the universal remedy. Robust systems use
  several simultaneously.

Part IV now assembles all of this into an engineered wireless system, beginning with the link
budget.

---

# Part IV — Wireless System Engineering

---

# Chapter 13

## Link Budgets: A Complete Worked Analysis

### 13.1 The Central Calculation of Wireless Engineering

Every question a wireless system raises — how far will it reach, what antenna is needed, will
it pass certification, how long will the battery last — reduces to a link budget. It is an
accounting of every decibel gained and lost between the transmitter's data and the receiver's
decision, and its output is a single number: the **link margin**, the amount by which received
power exceeds the minimum the receiver needs.

The link budget is the point where every result in Parts I–III becomes a line item. The noise
floor from Chapter 3, the required SNR from Chapter 8, the path loss and fading margins from
Chapter 12 — all of them appear here, added up. An engineer who can build a link budget from
first principles can evaluate a wireless product before any hardware exists, which is the most
valuable single skill in the field.

### 13.2 The Equation

In its complete form:

$$
P_{rx} = P_{tx} + G_{tx} - L_{tx} - L_{path} - L_{misc} + G_{rx} - L_{rx}
$$

and the link closes with margin `M`:

$$
M = P_{rx} - S_{rx}
$$

where `S_rx` is receiver sensitivity. Every term is in dB or dBm, so the entire calculation is
addition and subtraction. The terms:

| Term | Meaning | Typical value / source |
|---|---|---|
| `P_tx` | Transmit power at the PA output | +14 dBm (EU), +20 dBm (US), 0 dBm (BLE) |
| `G_tx` | Transmit antenna gain | 0–3 dBi (chip/PCB), up to 10+ dBi (external) |
| `L_tx` | TX-side losses: matching, trace, connector | 1–3 dB |
| `L_path` | Propagation loss | Chapter 12: FSPL + exponent + margins |
| `L_misc` | Fading, shadowing, polarisation, body loss | 15–30 dB total (Section 13.5) |
| `G_rx` | Receive antenna gain | 0–3 dBi typical |
| `L_rx` | RX-side losses ahead of the LNA | 1–3 dB — costs directly against NF (Ch 3) |
| `S_rx` | Receiver sensitivity | Chapter 8: noise floor + required SNR |

The distinction between `G_tx/G_rx` (antenna gains) and `L_tx/L_rx` (circuit losses) is worth
keeping sharp, because the losses on the receive side are more damaging than their decibel
count suggests — every dB of `L_rx` ahead of the low-noise amplifier adds directly to the
system noise figure (Section 3.3.2), so it hurts twice: once in the budget and once in the
sensitivity.

### 13.3 A Full Sub-GHz Worked Example

*Requirement: an 868 MHz sensor reporting to a gateway, EU compliant, 50 kbps, outdoors with
partial obstruction. How far will it reach at 95% location reliability?*

**Transmit side.**
- `P_tx` = +14 dBm (the EU ERP limit under EN 300 220)
- `G_tx` = +2 dBi (a decent PCB antenna)
- `L_tx` = 1.5 dB (matching network and trace)
- Effective radiated power = 14 + 2 − 1.5 = **+14.5 dBm**

**Receive side (from Chapter 3).**
- Bandwidth 100 kHz, NF 6 dB → noise floor = −174 + 50 + 6 = **−118 dBm**
- Required SNR for GFSK at BER 10⁻³ ≈ 9 dB → sensitivity = **−109 dBm**
- `G_rx` = +2 dBi, `L_rx` = 1.5 dB → effective sensitivity = −109 − 2 + 1.5 = **−109.5 dBm**

**Margins required (from Chapter 12).**
- Fading margin (Rician, moderate): 10 dB
- Shadowing margin (σ = 6 dB, 95% → 1.64σ): 10 dB
- Implementation loss already folded into sensitivity

**Solving for range.** The allowable path loss is:

$$
L_{path,max} = P_{ERP} - S_{eff} - M_{fade} - M_{shadow} = 14.5 - (-109.5) - 10 - 10 = 104\ \text{dB}
$$

Using the outdoor obstructed exponent `n = 3` with a 1 m reference (`FSPL(1 m, 868 MHz) =
20log(0.001) + 58.77 + 32.44 = 31.2 dB`):

$$
104 = 31.2 + 10 \times 3 \times \log_{10}(d) \Rightarrow \log_{10}(d) = 2.43 \Rightarrow d \approx 267\ \text{m}
$$

**The answer: about 270 metres**, with 95% reliability, under the stated obstruction. Note how
far this is from a naive free-space calculation, which with `n = 2` and no margins would give
several kilometres. The 20 dB of margins and the higher path-loss exponent — the very things
Chapter 12 insisted upon — account for almost the entire difference, and omitting them is the
single most common reason field range disappoints.

### 13.4 The Same Link at 2.4 GHz (BLE), for Contrast

*Same physical scenario, BLE 1M instead.*

- `P_tx` = 0 dBm (typical BLE), `G_tx` = 0 dBi, `L_tx` = 1 dB → ERP = −1 dBm
- Sensitivity (Chapter 3): 1 MHz bandwidth, NF 8 dB → noise floor −106 dBm; +10 dB SNR →
  sensitivity −96 dBm
- Same 20 dB of margins

$$
L_{path,max} = -1 - (-96) - 20 = 75\ \text{dB}
$$

At 2.4 GHz with `n = 3` (`FSPL(1 m) = 20log(0.001) + 20log(2440) + 32.44 = 40.2 dB`):

$$
75 = 40.2 + 30\log_{10}(d) \Rightarrow \log_{10}(d) = 1.16 \Rightarrow d \approx 14.5\ \text{m}
$$

**BLE reaches ~15 m to the Sub-GHz link's ~270 m** in the same scenario — an 18× range
difference. The gap decomposes cleanly: 15 dB of it is the lower transmit power, 9 dB is the
sensitivity difference (narrower bandwidth, lower NF), and 9 dB is the frequency advantage in
path loss. This single comparison is the entire quantitative case for choosing Sub-GHz over
BLE when range matters and the two are otherwise viable.

### 13.5 Margins Are Not Optional

The margins are the part of the budget that novices omit and experts obsess over, because they
are the difference between a link that works in the lab and one that works in the field. Each
addresses a specific mechanism from Chapter 12:

| Margin | Guards against | Typical value |
|---|---|---|
| Fading | Multipath, half-wavelength variation | 10–20 dB (Rayleigh worse than Rician) |
| Shadowing | Large-obstacle blocking | `z·σ` — 8–13 dB for 90–95% at σ = 6–8 dB |
| Interference | Co-channel and adjacent activity | 3–10 dB, environment-dependent |
| Polarisation mismatch | Antenna orientation | 3 dB typical, up to 20 dB worst case |
| Body loss | Human proximity (wearables) | 3–10 dB at 2.4 GHz, less at Sub-GHz |
| Temperature / ageing | Component drift over life | 1–3 dB |

A wearable BLE device operated near a body in an interference-rich indoor environment can
easily need 35 dB of aggregate margin. A fixed Sub-GHz link between mounted antennas with
line-of-sight might need only 12 dB. The correct margin is not a fixed number; it is the sum of
the mechanisms actually present, which is why Chapter 12 spent its length identifying them.

### 13.6 The Fade Margin–Reliability Trade

Margin buys reliability, but with diminishing returns, and the curve is worth internalising
because it tells you when to stop paying. For log-normal shadowing with σ = 8 dB:

| Location reliability | Margin required |
|---|---|
| 50% | 0 dB |
| 90% | 10.2 dB |
| 95% | 13.2 dB |
| 99% | 18.6 dB |
| 99.9% | 24.7 dB |

Going from 90% to 99% coverage costs 8.4 dB — which at fixed power means roughly halving the
range, or equivalently quadrupling the number of gateways needed to cover an area. This is a
system-cost decision disguised as a PHY parameter, and it belongs in front of whoever owns the
deployment budget, not buried in a firmware constant.

### 13.7 Link Budgets Are Approximate — Use Them Anyway

A link budget is a model, and every term carries uncertainty: transmit power varies ±2 dB with
temperature and supply, antenna gain depends on mounting and nearby objects, path loss is a
statistical average, and sensitivity is a typical figure that individual units miss by a dB or
two. The honest output is a range, not a point — "250 to 300 metres" rather than "267 metres".

This imprecision is not a reason to skip the calculation; it is a reason to build in margin and
then **verify by measurement** (Chapter 23). The budget tells you whether a design is plausible,
which antenna class to specify, and whether a requirement is impossible — all before committing
to hardware. It does not replace a field trial; it tells you whether a field trial is worth
running.

### 13.8 From Link Budget to Battery Budget

A link budget answers "will it reach?"; a battery budget answers "for how long?", and for the
overwhelming majority of low-power radio products the second question decides the product. The
two are joined at the hip, because every decibel of link margin is bought with energy, and the
purpose of this section is to make the conversion explicit so that a range requirement can be
priced in battery life before it is committed to.

A duty-cycled sensor's energy per reporting cycle decomposes into four terms:

$$
E_{cycle} = E_{sleep} + E_{wakeup} + E_{tx} + E_{rx}
$$

Consider a concrete node: it wakes once per minute, transmits a 20-byte packet at 50 kbps and
+14 dBm, and listens briefly for an acknowledgement. Working the terms with representative
numbers (3 V supply):

- **Transmit.** Packet airtime including preamble ≈ 5 ms. A +14 dBm saturated PA draws ≈ 45 mW
  (Section 16.7), plus ≈ 15 mW for the rest of the radio, so ≈ 60 mW for 5 ms = **0.30 mJ**.
- **Receive (ack window).** The radio draws ≈ 30 mW in RX; a 3 ms window = **0.09 mJ**.
- **Wake-up.** Crystal startup and calibration (Section 19.3) ≈ 2 ms at ≈ 20 mW = **0.04 mJ**.
- **Sleep.** 60 s at ≈ 3 µW (1 µA at 3 V) = **0.18 mJ**.

Total ≈ **0.61 mJ per cycle**, of which sleep — despite drawing a thousandth of the transmit
power — contributes nearly a third, because it lasts twelve thousand times as long. This is the
first lesson of battery budgeting: **average current, not peak current, sets battery life, and
the longest-lasting state usually dominates it.** A CR2032 coin cell holds ≈ 220 mAh ≈ 2400 J.
At 0.61 mJ per minute the radio consumes ≈ 0.88 J/day, giving a radio-limited life of years —
but only if the sleep current is genuinely in the microamp range. A sleep current ten times
higher, from a radio that fails to enter its deepest sleep state (a common firmware bug,
Chapter 20), would make sleep the overwhelming term and cut battery life by most of its value.

The link-to-battery connection now becomes visible as a design lever. Extending range by
enabling the Coded PHY or a spreading mode (Chapters 7, 9) multiplies airtime — the S=8 BLE
Coded PHY octuples it — and therefore multiplies `E_tx` and `E_rx`. A range improvement that
looks free in the link budget is paid for in the battery budget, and the two must be evaluated
together. This is why the correct design question is never "how much range can I get?" but "how
much range can I get *within the battery budget?*" — and why the feasibility analysis of
Section 4.7, the link budget of this chapter, and the battery budget here are one continuous
calculation.

### 13.9 Transmit Power Control and the Multi-Node Budget

The single-link budget of Section 13.3 assumes one transmitter and one receiver. Real networks
have many nodes at many distances, and the near-far problem of Section 14.6 means the budget must
be computed not for a link but for a *population* of links, with power control as the tool that
keeps them compatible.

Consider a star network: one gateway, many sensors, distances from 5 m to 500 m. If every sensor
transmits at the same +14 dBm, the 5 m sensor arrives at the gateway roughly 40 dB stronger than
the 500 m sensor (from the path-loss difference). When they collide, the capture effect
(Section 14.6) lets the strong one through and starves the weak one — but worse, the strong
sensor's signal may desensitise the gateway's front end (Section 14.2) even when they do *not*
collide, degrading reception of every distant node.

**Transmit power control (TPC)** equalises the received levels: each sensor measures its link
(from acknowledgement RSSI or a beacon) and reduces power so that all nodes arrive at the gateway
at a similar, adequate level. The 5 m node might drop to −20 dBm while the 500 m node stays at
+14 dBm, and both arrive near the same received power. The benefits compound:

- The near-far disparity collapses, so weak nodes are no longer starved.
- The close nodes save energy — the 5 m node transmitting at −20 dBm instead of +14 dBm cuts its
  PA power by a factor of several thousand, extending its battery life enormously.
- The aggregate interference the network generates falls, improving coexistence with neighbours
  (Chapter 14) and, in the EU, easing the duty-cycle pressure of Chapter 15.

TPC turns a single fixed budget into a set of per-node budgets, each closed with just enough
margin. The cost is the protocol machinery to measure links and command power changes, and a
control loop that must be stable — a node that oscillates between too much and too little power
is worse than one at fixed power. Cellular systems run TPC loops thousands of times per second
for exactly these reasons; a Sub-GHz star network can run a far coarser version, adjusting power
once per few packets, and still capture most of the benefit. The principle to carry forward is
that in any multi-node network the link budget is a distribution across nodes, and power control
is what keeps that distribution compatible with a single shared receiver.

### 13.10 What Carries Forward

- The link budget is pure dB addition: `M = P_rx − S_rx`, with every Part I–III result as a line
  item.
- RX-side losses ahead of the LNA hurt twice — in the budget and in the noise figure.
- Margins for fading, shadowing, interference, polarisation, and body loss are the difference
  between lab and field; they are summed from the mechanisms actually present.
- Reliability above ~95% costs steeply in margin and therefore in range or infrastructure.
- The budget qualifies a design; measurement confirms it.

---

# Chapter 14

## Interference, Coexistence, and Shared Spectrum

### 14.1 The Defining Problem of Unlicensed Spectrum

Every system in this book's scope — Sub-GHz ISM, BLE, Zigbee, Wi-Fi — operates in **unlicensed**
spectrum. No one owns the channel; no one coordinates access; anyone with a compliant device
may transmit. The consequence is that interference is not an edge case but the normal operating
condition, and coexistence — surviving amid other transmitters — is a first-class design
requirement rather than an afterthought.

Section 3.5 established how interference differs from noise: it is structured, bursty, and not
reduced by narrowing bandwidth. This chapter turns those properties into design strategy.

### 14.2 The Interference Taxonomy, Applied

The frequency-relationship classification from Chapter 3 maps directly onto mitigation:

**Co-channel interference** — another transmitter on your channel. Nothing in the frequency
domain helps because there is no frequency separation to exploit. The only remedies are in
*time* (transmit when the interferer is silent), *space* (separate the nodes or use directional
antennas), or *code* (spreading, so the interferer decorrelates). This is the interference that
frequency hopping is designed to escape.

**Adjacent-channel interference** — a transmitter in a neighbouring channel whose energy leaks
into yours through imperfect filtering. Governed by two figures: the interferer's
**adjacent-channel power** (how much it spills, set by its spectral mask) and your receiver's
**adjacent-channel selectivity** (how well it rejects out-of-channel energy). Both are
improvable, and the guard band between channels (Section 2.4) exists precisely to give
selectivity room to work.

**Blocking and desensitisation** — the most insidious. A strong out-of-band signal, one the
receiver nominally filters out entirely, drives the front-end amplifier toward compression,
raising its effective noise figure and degrading sensitivity across the *whole* band. A receiver
rated at −109 dBm sensitivity can be desensitised to −85 dBm by a strong nearby transmitter it
is not even trying to receive. This is why a Sub-GHz node next to a cellular modem can fail
mysteriously, and why front-end linearity (the IP3 of Section 3.4) matters even for signals you
reject.

### 14.3 The Coexistence Environment Quantified

The 2.4 GHz band is the extreme case, and understanding it explains most real-world BLE and
Zigbee behaviour. It is shared by:

- **Wi-Fi** — up to three non-overlapping 20 MHz channels (or fewer, wider ones), each capable of
  +20 dBm, transmitting in bursts of hundreds of microseconds. Wi-Fi is the elephant: high
  power, wide bandwidth, aggressive duty cycle.
- **Classic Bluetooth and BLE** — frequency-hopping across the band.
- **Zigbee / Thread / 802.15.4** — 16 fixed channels, several of which fall in Wi-Fi passbands.
- **Microwave ovens** — a broadband noise source sweeping across the band's centre at the mains
  frequency, radiating tens of watts of leakage.

A Zigbee network on a channel overlapping an active Wi-Fi access point can see its packet error
rate rise from under 1% to over 50%. This is the single most common cause of "unreliable Zigbee"
complaints, and its fix is channel selection, not power.

Sub-GHz bands are quieter but not empty. The 868 MHz band carries alarm systems, smart meters,
and a growing density of LPWAN traffic; the 915 MHz US band shares space with the same plus
some cordless devices. The interference is sparser than at 2.4 GHz but often more persistent —
a smart meter reporting on a fixed schedule is a very different adversary from bursty Wi-Fi.

### 14.4 Frequency Hopping

The dominant coexistence strategy in BLE and classic Bluetooth is **frequency hopping**: the link
changes channel on a schedule known to both ends, so that no single interferer can block more
than the fraction of hops that land on its channel.

BLE uses 37 data channels of 2 MHz each and hops between them every connection interval. Two
mechanisms operate:

- **Basic hopping** spreads transmissions pseudo-randomly across all 37 channels. An interferer
  occupying, say, 4 channels blocks roughly 4/37 ≈ 11% of transmissions, which retransmission
  easily recovers.
- **Adaptive frequency hopping (AFH)** goes further: the link *measures* which channels are bad
  and removes them from the hopping set. A BLE link in a Wi-Fi-heavy environment may drop to 20
  usable channels, concentrating traffic on the clear ones. AFH is why BLE and Wi-Fi coexist as
  well as they do, and it is not optional in any serious deployment.

Hopping also delivers the **frequency diversity** that Chapter 12 identified as the remedy for
flat fading — a fade on one channel is independent of the fade on the next hop. Hopping thus
solves interference and fading with one mechanism, which is why it is so widely adopted.

### 14.5 Listen-Before-Talk

The complementary strategy, mandatory in parts of the EU Sub-GHz bands and central to Wi-Fi, is
**listen-before-talk (LBT)**, also called clear-channel assessment: before transmitting, the
device measures the channel's energy, and transmits only if it is below a threshold.

LBT is polite where hopping is evasive. It avoids collisions rather than surviving them, and it
is the basis of every carrier-sense multiple-access scheme. Its costs are latency (the device
may wait, and the wait is unbounded in a busy channel) and the **hidden-node problem** — two
transmitters may each find the channel clear because they cannot hear each other, yet both
collide at a receiver that hears both. LBT reduces collisions; it does not eliminate them.

Chapter 15 covers the regulatory dimension: where LBT is legally required and how its
parameters are specified.

### 14.6 The Capture Effect and the Near–Far Problem

Two receiver phenomena shape how collisions actually resolve, and both surprise engineers who
assume a collision always destroys both packets.

**The capture effect** works in your favour. When two packets collide, a receiver can often
decode the *stronger* one cleanly if it exceeds the weaker by the **capture ratio** — typically
6–10 dB for FSK. A collision is therefore not automatically a mutual loss; the stronger signal
frequently survives. This is why real network throughput under contention is better than a naive
collision model predicts, and it is a genuine advantage of constant-envelope FSK, whose capture
behaviour is good.

**The near–far problem** works against you. A distant transmitter can be completely swamped by a
nearby one, even on an adjacent channel, because the near transmitter's leakage exceeds the far
transmitter's wanted signal. In a star network with nodes at varying distances, the close nodes
can starve the far ones entirely. The remedy is **transmit power control** — nearby nodes reduce
power so that all signals arrive at the gateway at comparable strength — which is why cellular
systems obsess over power control and why a well-designed Sub-GHz star network implements at
least a coarse version.

### 14.7 Designing for Coexistence: A Checklist

The strategies combine into a design discipline:

- **Choose the quietest band and channel available**, and for fixed channels, survey the site
  first.
- **Hop if you can** — it delivers interference and fading diversity together.
- **Listen before talking** where latency permits and regulation requires.
- **Keep packets short** — a short packet presents a smaller collision target and spends less
  time exposed (Section 8.5 quantified the error-rate benefit; the collision benefit compounds
  it).
- **Use spreading** where the band is hostile and airtime permits (Chapter 7).
- **Control transmit power** to equalise received levels and mitigate near–far.
- **Provide retransmission** (ARQ, Chapter 9) so that the residual collisions that survive all of
  the above are recovered.
- **Budget an interference margin** (Chapter 13) rather than assuming a clean channel.

No single mechanism suffices; robust coexistence is the layering of several, each covering the
others' failure modes.

### 14.8 Contention, CSMA, and the Throughput Ceiling

Listen-before-talk (Section 14.5) is the atom of a larger structure: **carrier-sense multiple
access (CSMA)**, the family of protocols by which uncoordinated nodes share a channel politely.
Because CSMA sits directly on the PHY and its performance is governed by PHY timing, it belongs
here, and its central result — that a shared channel has a throughput ceiling well below its raw
capacity — shapes every dense-network design.

In CSMA a node wishing to transmit first senses the channel; if busy, it waits; if idle, it
transmits, often after a random backoff to avoid colliding with other nodes that were also
waiting. The scheme works well at low load but degrades as offered traffic rises, because two
effects compound. First, the **vulnerable period**: a node commits to transmit based on a channel
that was idle when it sensed, but another node's transmission may begin during the propagation and
detection delay, causing a collision that sensing could not prevent. Second, the **hidden-node
problem** (Section 14.5): two nodes out of range of each other both sense idle and both transmit,
colliding at a receiver that hears both.

The consequence is a throughput curve that rises with offered load, peaks, and then *falls* as
load increases further — because at high load most airtime is consumed by collisions and backoff
rather than successful transmission. Un-slotted CSMA peaks at a throughput of roughly 50–60% of
raw capacity; even idealised slotted variants rarely exceed 80%. A network designed on the
assumption that it can use its full PHY data rate will collapse under load, and the collapse is
not graceful — past the peak, adding traffic *reduces* delivered throughput, the classic
congestion catastrophe.

The PHY parameters feed directly into this ceiling. Shorter packets (Section 8.5) reduce the
collision target and raise the peak throughput. Faster carrier sensing — a lower detection
threshold and shorter sensing time — shrinks the vulnerable period. The capture effect
(Section 14.6) raises effective throughput by letting some collisions resolve in favour of the
stronger packet. And the acknowledgement timing (the T_IFS of Chapter 19) sets the per-packet
overhead. An engineer tuning a dense network is, whether or not they frame it this way, tuning
these PHY parameters against the CSMA throughput curve, and understanding the curve is what turns
that tuning from guesswork into engineering.

### 14.9 Channel Planning and the Reuse Distance

For fixed-channel systems — those that do not hop — coexistence is managed by **channel planning**:
assigning channels to nodes so that co-channel transmitters are far enough apart that their mutual
interference is tolerable. The governing concept, borrowed from cellular engineering, is the
**reuse distance**, and it quantifies how far apart two transmitters on the same channel must be.

The requirement is a signal-to-interference ratio (SIR) adequate for the modulation
(Section 6.7). If a receiver needs, say, 9 dB of SIR, then a co-channel interferer must arrive at
least 9 dB weaker than the wanted signal. With a path-loss exponent `n`, this translates into a
ratio of distances:

$$
\frac{d_{interferer}}{d_{signal}} \ge 10^{\text{SIR}/(10n)}
$$

For SIR = 9 dB and `n = 3`, the interferer must be at least `10^{9/30} = 2` times farther than
the wanted transmitter — so a receiver at 100 m from its transmitter needs the nearest co-channel
interferer beyond 200 m. In a denser environment (higher `n`), the required ratio shrinks, because
interference attenuates faster with distance; in free space (`n = 2`), it grows, because
interference carries farther. This is the quantitative basis of frequency planning: it tells the
designer how many distinct channels are needed to cover an area given the node density and the
propagation environment, and it explains why frequency-hopping systems (Section 14.4), which
spread each link's exposure across all channels, are so much easier to deploy densely — they
replace static planning with statistical averaging and sidestep the reuse-distance calculation
entirely. The choice between fixed channels with planning and hopping without it is one of the
first architectural decisions in a dense Sub-GHz deployment, and the reuse-distance formula is
what makes the trade concrete.

### 14.10 What Carries Forward

- Interference is the normal condition of unlicensed spectrum, not an edge case.
- Co-channel interference yields only to time, space, or code; adjacent-channel to selectivity
  and guard bands; blocking to front-end linearity.
- Frequency hopping (especially adaptive) escapes interference and provides fading diversity in
  one mechanism; LBT avoids collisions at a latency cost.
- The capture effect lets the stronger colliding packet survive; the near–far problem starves
  distant nodes and demands power control.
- Coexistence is engineered by layering several strategies, not by any one.

---

# Chapter 15

## Regulatory Constraints: FCC, ETSI, Duty Cycle, and Listen-Before-Talk

### 15.1 Why a Chapter on Regulation Belongs in a PHY Book

A Sub-GHz radio that is technically excellent and legally non-compliant is a product that
cannot be sold. Regulation is not a bureaucratic layer bolted on after the engineering; it is a
**hard constraint on the PHY design itself**, dictating transmit power, occupied bandwidth,
duty cycle, and channel access. An engineer who designs the protocol first and reads the
regulations later routinely discovers that the protocol is illegal in its target market — after
the silicon is committed. This chapter exists so that does not happen, and it is the chapter
your intuition from the 2.4 GHz world will most mislead you on, because 2.4 GHz is unusually
permissive.

The two regulatory regimes that matter for most of the world are the **FCC** (United States,
Title 47 CFR Part 15) and **ETSI** (Europe, EN 300 220 for Sub-GHz and EN 300 328 for 2.4 GHz).
They differ profoundly in philosophy, and a product sold in both markets must satisfy both —
often with region-specific firmware.

### 15.2 The Two Philosophies

The single most important thing to understand is that FCC and ETSI regulate *different
quantities*, and a design optimised for one is frequently illegal under the other.

**The FCC regulates power and spectral occupancy.** In the US 902–928 MHz band, a frequency-
hopping system may transmit up to +30 dBm (1 W) conducted, provided it hops across at least 50
channels. The FCC is comparatively generous on power and imposes no duty-cycle limit — you may
transmit continuously — but requires hopping or digital modulation with adequate bandwidth.

**ETSI regulates duty cycle and mandates polite access.** In the EU 868 MHz band, power is
capped much lower (typically +14 dBm ERP), and — the constraint with no US equivalent — each
sub-band carries a **duty-cycle limit**, commonly 0.1%, 1%, or 10%, restricting how long a device
may transmit per hour. ETSI is stingy on both power and airtime but permits narrowband fixed-
channel operation that the FCC's hopping requirement discourages.

| Aspect | FCC (US, 902–928 MHz) | ETSI (EU, 863–870 MHz) |
|---|---|---|
| Max power | +30 dBm (1 W) with hopping | +14 dBm ERP (typical) |
| Duty cycle | Unlimited | 0.1% / 1% / 10% by sub-band |
| Channel access | ≥50 hop channels, or wideband | Duty cycle, or LBT + AFA |
| Bandwidth | Governed by hopping/occupancy rules | Sub-band specific |
| Governing rule | 47 CFR §15.247, §15.249 | EN 300 220, EN 300 328 |

A protocol that transmits a 1-second beacon every 10 seconds (10% duty cycle) is legal in the
US and illegal in the 1% EU sub-bands. A protocol relying on +27 dBm output is legal in the US
and illegal in the EU. These are not corner cases; they are routine, and they are why "it works
in our US trial" tells you nothing about EU compliance.

### 15.3 The Duty-Cycle Constraint, Quantified

The EU duty-cycle limit is the constraint most likely to break a naive Sub-GHz design, so it is
worth working concretely. A 1% duty cycle means a device may transmit for at most **36 seconds
per hour** on that sub-band. That is a hard airtime budget the protocol must live within.

**Worked example.** A sensor sends a 50-byte packet at 50 kbps. Packet airtime, including a
typical preamble and header overhead of ~10 bytes:

$$
t_{packet} = \frac{60 \times 8\ \text{bits}}{50000\ \text{bps}} = 9.6\ \text{ms}
$$

Under a 1% limit (36 s/hour), the maximum transmission rate is:

$$
N_{max} = \frac{36\ \text{s}}{9.6\ \text{ms}} = 3750\ \text{packets/hour} \approx 1\ \text{per second}
$$

Comfortable for a sensor. But now consider a **LoRa SF12** packet from Chapter 7, with an
airtime over 1 second:

$$
N_{max} = \frac{36\ \text{s}}{1.1\ \text{s}} \approx 32\ \text{packets/hour}
$$

Roughly one every two minutes. This is why LPWAN protocols like LoRaWAN build duty-cycle
accounting directly into the stack and why their application designers must treat airtime as
the scarce resource it legally is. The range that spreading buys (Chapter 7) is paid for twice:
once in data rate, and once in regulatory airtime.

### 15.4 Listen-Before-Talk and Adaptive Frequency Agility

ETSI offers an alternative to the duty-cycle limit: a device implementing **LBT + AFA** (listen-
before-talk with adaptive frequency agility) may be exempted from the duty-cycle cap, because
it is demonstrably polite. This is the regulatory home of the LBT mechanism introduced in
Section 14.5.

The regulation specifies the parameters precisely — the sensing threshold (a function of
bandwidth), the minimum listening time, and the behaviour on a busy channel. A design that
implements LBT to escape the duty-cycle limit must implement it to the standard's letter, and
certification will test it. This is a case where a Chapter 14 coexistence strategy and a
Chapter 15 legal requirement are the same mechanism viewed from two angles.

### 15.5 The Spectral Mask

Both regimes limit **out-of-band and spurious emissions** through a spectral mask — a template the
transmitted spectrum must fit under. This is where the pulse shaping of Chapter 7 meets the
law: an unfiltered FSK transmitter produces splatter that violates the mask, and the Gaussian
filter exists as much to pass certification as to be a good neighbour.

Spurious emissions — harmonics of the carrier, mixer products, and reference spurs — are limited
separately and are a frequent certification failure. The second and third harmonics of an
868 MHz carrier fall at 1736 MHz and 2604 MHz, and if the antenna matching network does not
adequately suppress them, the device fails on emissions in bands it never intended to use. This
is why the front-end filter (Chapter 16) is a compliance component, not merely a performance
one.

### 15.6 ERP, EIRP, and How Power Is Actually Measured

Regulations specify power at the antenna, not at the PA, and the distinction trips up many
first submissions. Two reference terms:

- **ERP (effective radiated power)** — power relative to a half-wave dipole. Used by ETSI.
- **EIRP (effective isotropic radiated power)** — power relative to an isotropic radiator. Used
  by the FCC. `EIRP = ERP + 2.15 dB`.

The measured radiated power is `P_tx + G_antenna − L_losses`. A design with +14 dBm at the PA
and a +3 dBi antenna radiates +17 dBm ERP — which *exceeds* the EU +14 dBm limit and fails
certification, even though the PA setting looked compliant. The antenna gain counts against the
budget, which means the antenna and the power setting must be certified *together*, and changing
the antenna can invalidate the certification. Firmware that sets transmit power must therefore
know the antenna it is feeding.

### 15.7 The Global Fragmentation Problem

There is no single Sub-GHz band. The allocations differ by region, and a global product must
support several, often with region-locked firmware:

| Region | Common Sub-GHz band | Regime |
|---|---|---|
| Europe | 863–870 MHz | ETSI EN 300 220 |
| North America | 902–928 MHz | FCC Part 15 |
| Japan | 920–928 MHz | ARIB STD-T108 |
| China | 470–510 MHz, 779–787 MHz | SRRC |
| Global (2.4 GHz) | 2400–2483.5 MHz | FCC / ETSI / worldwide |

This fragmentation is a large part of why 2.4 GHz remains attractive despite its inferior
propagation and crowded spectrum: it is available almost everywhere under similar rules, so a
single design and a single certification path serves the world. Sub-GHz buys range and quiet at
the cost of a per-region engineering and certification burden. That trade — global simplicity
versus regional performance — is one of the first architectural decisions a wireless product
must make, and it is a PHY decision with business consequences.

### 15.8 Dwell Time, Hopping Rules, and the FCC Arithmetic

The FCC's frequency-hopping rules (Section 15.2) are specified precisely enough to compute
against, and a design that hops must satisfy them exactly or fail certification. Working the
arithmetic makes the constraints concrete and shows how they shape a hopping protocol.

Under 47 CFR §15.247 for the 902–928 MHz band, a frequency-hopping system must use its hopping
channels roughly equally and must limit its **dwell time** — the time spent on any one channel — to
no more than 0.4 seconds within any period equal to the number of channels times 0.4 seconds. For
a system with 50 hopping channels, the averaging period is `50 × 0.4 = 20` seconds, and the
system may spend at most 0.4 s on any single channel within each 20 s window.

The channel-count requirement interacts with the allowed power. Systems hopping over at least 50
channels may transmit up to +30 dBm (1 W); systems using between 25 and 49 channels are limited to
+21 dBm; and the occupied bandwidth per channel must stay within specified limits. A designer
therefore faces a direct trade: more hopping channels permit higher power (and better
interference averaging), but require wider total spectrum and more complex synchronisation. A
protocol hopping over exactly 50 channels to claim the 1 W allowance must genuinely visit all 50
with roughly equal probability — a pseudo-random hop sequence that inadvertently favours some
channels can fail certification even though it "hops."

The contrast with the EU duty-cycle regime (Section 15.3) is instructive. The FCC constrains
*where and how often* you may dwell but not *how much total airtime* you may use — a compliant FHSS
system may transmit continuously, provided it keeps moving across channels. The EU constrains
total airtime but is more permissive about staying on one channel. A protocol optimised for one
regime — continuous hopping for the FCC, or bursty narrowband transmission within a duty-cycle
budget for the EU — is frequently non-compliant in the other, which is why genuinely global
Sub-GHz products carry region-specific PHY configurations and often region-specific certification
per market.

### 15.9 The Certification Process and Pre-Compliance

Regulation is enforced through a **certification** process, and understanding its shape prevents
the expensive surprise of a design that is technically excellent and unsellable. This section is
practical rather than theoretical, because the process itself is a constraint on the engineering
schedule.

A product transmitting intentionally must be certified before sale: FCC certification in the US
(with an FCC ID), and the CE marking process referencing the relevant ETSI standards in Europe,
with analogous regimes elsewhere (Section 15.7). Certification is performed by an accredited test
laboratory measuring the emissions covered throughout this chapter — output power, occupied
bandwidth, spectral mask, spurious emissions, and, where applicable, hopping behaviour and
listen-before-talk parameters.

The measurements that most often fail a first submission are the ones Chapter 23 emphasises:
spurious emissions (harmonics inadequately suppressed by the antenna match, Section 15.5) and the
spectral mask (inadequate pulse shaping, Section 7.1). Because a full certification test is
expensive and slow, disciplined teams perform **pre-compliance** testing on their own bench —
using a spectrum analyser to check the mask and spurs against the limits before committing to the
lab. A pre-compliance measurement that reveals a 3 dB spectral-mask violation costs an afternoon
to find and a filter-value change to fix; the same violation discovered at the certification lab
costs a failed test, a redesign, and weeks of schedule. This is the concrete reason the bring-up
checklist of Section 23.7 places spectral-mask and spurious-emission checks early, before design
lock: they are the measurements where a late failure is most costly, so they are made first.

A crucial subtlety already noted (Section 15.6) is that certification is granted for a specific
*configuration* — a specific power setting, antenna, and often firmware. Changing the antenna to
one with more gain, or raising the power in firmware, can invalidate the certification and require
re-testing. Firmware that sets transmit power must therefore be constrained to configurations the
product is certified for, and a "field-configurable power" feature is a certification liability
unless every reachable setting is covered. This is a case where a software convenience collides
with a regulatory reality, and the regulation wins.

### 15.10 What Carries Forward

- Regulation is a hard PHY constraint, not an afterthought; the 2.4 GHz world's permissiveness
  does not generalise.
- FCC regulates power and occupancy (generous power, no duty cycle, hopping required); ETSI
  regulates airtime and politeness (low power, strict duty cycle, LBT as an escape).
- The EU duty-cycle limit is a hard airtime budget — ~36 s/hour at 1% — that spreading spends
  quickly.
- Power is measured radiated (ERP/EIRP), so antenna gain counts against the limit and antenna
  plus power certify together.
- Sub-GHz's regional fragmentation is the hidden cost that keeps 2.4 GHz attractive.

---

# Chapter 16

## Antennas and RF Front-Ends

### 16.1 The Boundary Between Silicon and Space

The antenna and RF front-end are where the digital design meets physics that firmware cannot
fix. A modulation scheme can be changed in a register; an antenna that radiates half its power
back into the amplifier cannot. This chapter covers the analogue boundary, and its recurring
theme is that decibels lost here are lost permanently — no coding gain, no retransmission, and
no clever firmware recovers a badly matched antenna or a lossy front-end filter placed ahead of
the LNA.

### 16.2 The Antenna as a Transducer

An antenna converts guided electrical energy into radiated electromagnetic waves and back. Its
key parameters:

**Gain** measures how much an antenna concentrates energy in a direction, relative to an
isotropic radiator (dBi) or a dipole (dBd), with `dBi = dBd + 2.15`. Gain is not amplification —
an antenna is passive — but redistribution: a high-gain antenna radiates more in its favoured
direction by radiating less elsewhere. A +10 dBi antenna delivers 10 dB more in its main beam
at the cost of a narrow beam that must be pointed.

**Radiation pattern** describes that directional distribution. An **omnidirectional** antenna
(the whip, the PCB trace) radiates roughly equally in a plane and suits nodes of unknown
orientation. A **directional** antenna (Yagi, patch) concentrates energy and suits fixed
point-to-point links where both ends can be aimed.

**Efficiency** is the fraction of delivered power actually radiated rather than dissipated as
heat. Chip antennas and small PCB antennas can be badly inefficient — 30–50% (−3 to −5 dB) is
common — and this loss is often the largest single term degrading a compact device's link
budget. It rarely appears on a datasheet's front page and must be extracted from the detail or
measured.

### 16.3 Size, Frequency, and the Small-Antenna Penalty

Efficient radiation requires an antenna comparable to the wavelength, and this is where
Sub-GHz's propagation advantage collides with a physical inconvenience. The quarter-wave
lengths from Chapter 6:

| Band | Wavelength | Quarter-wave |
|---|---|---|
| 433 MHz | 69 cm | 17.3 cm |
| 868 MHz | 34.6 cm | 8.6 cm |
| 915 MHz | 32.8 cm | 8.2 cm |
| 2.44 GHz | 12.3 cm | 3.07 cm |

An 8.6 cm antenna does not fit in a wearable. Designers shrink antennas below their natural
size — with meandered traces, chip antennas, and loading coils — but the **Chu limit** imposes an
unavoidable penalty: an electrically small antenna trades efficiency and bandwidth for size. A
Sub-GHz antenna squeezed into a 2 cm space may lose 6–10 dB of efficiency, partly erasing the
9 dB propagation advantage that motivated choosing Sub-GHz in the first place. This is a genuine
and often overlooked tension: the band that propagates best is the band whose antennas are
hardest to fit, and the two effects can cancel.

### 16.4 Impedance Matching

Maximum power transfers from source to antenna only when their impedances are conjugate-matched,
conventionally to 50 Ω. A mismatch reflects power back toward the PA, quantified by **return
loss** or **VSWR** (voltage standing wave ratio):

| VSWR | Return loss | Power reflected | Power lost |
|---|---|---|---|
| 1.0:1 | ∞ | 0% | 0 dB |
| 1.5:1 | 14 dB | 4% | 0.18 dB |
| 2.0:1 | 9.5 dB | 11% | 0.5 dB |
| 3.0:1 | 6 dB | 25% | 1.25 dB |
| 5.0:1 | 3.5 dB | 44% | 2.6 dB |

A matching network — a few capacitors and inductors — transforms the antenna's impedance to
50 Ω. It is tuned for a specific antenna in a specific enclosure, because nearby objects,
including the enclosure and the user's hand, shift the antenna's impedance. This is why a design
that matches perfectly on the bench detunes in a plastic case against a body, and why matching
is verified in the final mechanical assembly, not on a bare board.

### 16.5 Polarisation

An antenna radiates with a polarisation — the orientation of its electric field, vertical,
horizontal, or circular. A receiving antenna best captures energy of matching polarisation, and
a mismatch costs power: 3 dB for a 45° misalignment, and theoretically infinite (in practice
20+ dB) for orthogonal linear polarisations.

For devices of fixed orientation this is manageable. For devices of unknown orientation — a
sensor that may be mounted any way, a handset that rotates — polarisation mismatch is a random
loss that must be budgeted (Chapter 13's polarisation margin). Multipath partially rescues the
situation by scrambling polarisation through reflection, which is one of the rare cases where
multipath helps rather than harms.

### 16.6 The Receive Front-End and the Primacy of the LNA

The receive chain runs antenna → filter → **low-noise amplifier** → mixer → filters → ADC, and
Chapter 3's Friis analysis established the governing rule: the first amplifying stage dominates
the system noise figure, so the LNA must come first and must be quiet.

The consequence for layout is strict and frequently violated: **every dB of loss before the LNA
adds directly to the noise figure and subtracts directly from sensitivity.** A front-end filter
with 2 dB of insertion loss, a switch with 1 dB, and a connector with 0.5 dB placed ahead of
the LNA cost 3.5 dB of sensitivity — which, from Chapter 13, is roughly 30% of range, thrown
away in passive components. Sometimes the filter must go there anyway, to protect the LNA from a
strong out-of-band blocker (Section 14.2), and then the loss is the price of not being
desensitised. That trade — filter loss versus blocking immunity — is one of the central
front-end design decisions.

### 16.7 The Transmit Front-End and the PA

The transmit chain runs baseband → mixer/up-conversion → **power amplifier** → filter → antenna,
and its dominant component is the PA, whose linearity requirement was set in Chapter 3 and whose
consequences ran through Chapter 7.

The recurring theme completes here. A constant-envelope modulation (FSK, GFSK) lets the PA run
saturated at 50–60% efficiency; an envelope-varying modulation (QAM) forces 6–10 dB of backoff
into the linear region at 15–25% efficiency. For a battery device this is the difference between
transmitting at +14 dBm on 45 mW of supply power and on 165 mW. The PA is also the largest
consumer in the whole radio during transmit, so its efficiency directly sets battery life —
which is the ultimate reason the entire Sub-GHz/BLE world settled on constant-envelope
modulation.

### 16.8 Front-End Non-Idealities

Real front-ends depart from the ideal in ways that limit performance and that firmware must
sometimes calibrate around:

- **Phase noise** in the local oscillator smears the carrier and, through **reciprocal mixing**,
  lets a strong adjacent signal leak into the wanted channel — degrading selectivity in a way no
  filter can fix.
- **I/Q imbalance** — gain or phase mismatch between the I and Q paths — creates an image of the
  wanted signal in the mirror channel and degrades every I/Q modulation. Modern transceivers
  calibrate it at startup.
- **DC offset** in a direct-conversion (zero-IF) receiver sits right in the middle of the
  wanted signal and must be removed, which is one reason whitening (Chapter 9) matters — it
  keeps the signal's own DC content from being confused with offset.
- **Nonlinearity** (the IP3 of Chapter 3) generates intermodulation that both fails the spectral
  mask and desensitises the receiver.

These are the reasons a real radio contains calibration routines and why Chapter 20's firmware
spends startup cycles on them.

### 16.9 Synthesising a Matching Network

Section 16.4 established that an impedance match is necessary and tuned to the final assembly;
this section shows how a match is actually synthesised, because the L-network is the workhorse of
Sub-GHz front-end design and an engineer who can reason about it can diagnose and adjust a match
rather than treating it as a black box of vendor-supplied component values.

The simplest matching network is the **L-network**: two reactive components — one series, one
shunt — that transform a load impedance to the desired source impedance (usually 50 Ω). The two
components give two degrees of freedom, exactly enough to match any single complex impedance at a
single frequency. The design proceeds by moving on the Smith chart, or equivalently by algebra:
the shunt element moves the impedance along a circle of constant conductance, and the series
element along a circle of constant resistance, and the two are chosen so that their combined path
reaches the 50 Ω centre.

Concretely, to match a load `R_L + jX_L` to a source `R_S`, when `R_L < R_S`, one adds a series
reactance to cancel `X_L` and reach the resistance circle, then a shunt reactance to move to the
centre; the required component values follow from the **quality factor** of the match:

$$
Q = \sqrt{\frac{R_S}{R_L} - 1}
$$

This `Q` is more than an intermediate quantity — it sets the **bandwidth** of the match:

$$
\text{fractional bandwidth} \approx \frac{1}{Q}
$$

A high-Q match (transforming a very low load resistance up to 50 Ω) is narrowband — it works over
a small frequency range and detunes readily when the antenna's impedance shifts (as it does in an
enclosure, against a body). A low-Q match is broadband and tolerant. This is why antennas whose
natural impedance is close to 50 Ω are prized: they need only a gentle, low-Q, broadband match,
while an electrically small antenna (Section 16.3) with a low radiation resistance requires a
high-Q match that is both lossy and fragile. The small-antenna penalty of Section 16.3 therefore
has a second face here: not only is the small antenna inefficient, its match is narrowband and
detune-prone, compounding the difficulty. The matching network is where the antenna's physical
compromises meet the circuit, and reading the network's Q tells an engineer immediately how
forgiving or how touchy the resulting front end will be.

### 16.10 Antenna Types in Practice

The abstract parameters of Section 16.2 become design choices through the specific antenna types
a low-power product might use, each with a characteristic trade that determines where it belongs.

**The PCB trace antenna** — a meandered or inverted-F trace etched directly on the circuit board —
costs nothing in materials and is the default for volume products. Its efficiency is moderate
(often −2 to −4 dB) and highly dependent on the surrounding copper, ground plane, and enclosure,
which is why it must be tuned in the final assembly (Section 16.4). It is the right choice when
cost dominates and the efficiency penalty is affordable.

**The chip antenna** — a small ceramic component — trades a few dB of efficiency for a compact,
somewhat more predictable footprint. It suits products too small for an adequate trace antenna,
accepting the Chu-limit penalty (Section 16.3) in exchange for fitting the space.

**The external whip or dipole** — a quarter- or half-wave element — offers the best efficiency
(near 0 dB) and a clean omnidirectional pattern, at the cost of size and a connector. It suits
gateways and base stations where size is not constrained and every dB of link budget is worth
capturing, and it is often the difference between a marginal and a solid deployment.

**The directional antenna** — patch, Yagi, or panel — concentrates energy (Section 16.2) for
fixed point-to-point links, buying gain in one direction at the cost of coverage in others. A
+10 dBi patch on a fixed backhaul link adds 10 dB to the budget at both ends if both are
directional — 20 dB total — which can extend a link by an order of magnitude, but only when both
ends can be aimed and stay aimed.

The choice among these is a system decision, not merely an RF one: it trades cost, size, link
budget, and deployment constraints, and it is made early because it drives the enclosure, the
board layout, and the certification (the antenna is certified with the radio, Section 15.9). An
engineer who understands the efficiency and pattern consequences of each type can price a range
requirement in antenna cost and product form factor before committing to a design.

### 16.11 What Carries Forward

- Antenna gain is redistribution, not amplification; efficiency loss in small antennas can erase
  the Sub-GHz propagation advantage (Chu limit).
- Matching is verified in the final assembly, because enclosure and body detune the antenna.
- Loss before the LNA subtracts directly from sensitivity — the single most damaging place to
  lose a decibel.
- Constant-envelope modulation lets the PA run saturated, which is the root reason for the
  modulation choices of Part II and the dominant factor in battery life.
- Front-end non-idealities (phase noise, I/Q imbalance, DC offset) are why real radios calibrate
  at startup.

---

# Chapter 17

## Digital Baseband and Modem Architecture

### 17.1 Where Analogue Becomes Bits

The RF front-end of Chapter 16 delivers a stream of I/Q samples from the ADC. These are not yet
data — they are a noisy, frequency-offset, imperfectly-timed representation of the transmitted
symbols. The **digital baseband**, or modem, is the signal-processing chain that turns those
samples into bits, and it is where the receiver actually earns its sensitivity. This chapter
traces that chain, and it is the bridge from the analogue world of Part IV into the firmware
world of Part V.

### 17.2 The Receive Chain

The digital receive path is a sequence of operations, each undoing a specific channel or
hardware impairment:

```
ADC samples → DC removal → digital down-conversion → channel filter →
  AGC → timing recovery → frequency/phase correction →
  demodulation → soft decision → de-whiten → FEC decode → CRC → payload
```

**DC removal** strips the offset that a zero-IF front-end introduces (Section 16.8).

**Digital down-conversion** mixes the signal to true baseband (zero centre frequency) by
multiplying with a numerically-controlled oscillator, moving the last frequency-shift step from
analogue into the more precise digital domain.

**Channel filtering** — a digital low-pass or band-pass filter — rejects everything outside the
wanted channel. This filter *sets the receiver bandwidth* and therefore, through Chapter 3's
noise-floor equation, sets the sensitivity. A filter made wider than the signal needs admits
extra noise and degrades sensitivity directly; this is one of the most common causes of a
receiver underperforming its datasheet, and one entirely under firmware control.

**Automatic gain control** scales the signal to use the ADC's range without clipping. Too little
gain wastes ADC bits (quantisation noise); too much clips on peaks. AGC must settle fast enough
to be ready before the payload arrives, which is one of the jobs the preamble buys time for.

### 17.3 Synchronisation: The Hard Part

Section 1.2 named synchronisation the most frequently fatal PHY function, and the modem is where
it is won or lost. Three distinct synchronisations must succeed, in order:

**Timing recovery** finds the symbol boundaries — *when* to sample. The receiver's sample clock
is not aligned to the transmitter's symbol clock, and the offset must be estimated and tracked.
Timing recovery locks during the preamble's alternating pattern (Section 5.6), which is why
preamble length trades against acquisition reliability.

**Frequency correction** removes the carrier frequency offset between transmitter and receiver.
Chapter 6 quantified the problem: two ±20 ppm crystals at 868 MHz can differ by 34.7 kHz, which
for a 25 kHz-deviation FSK signal exceeds the signal's own frequency structure. The modem
estimates this offset — usually from the preamble — and corrects it before demodulation. A modem
that cannot handle the crystal tolerance in the bill of materials will fail in the field even
though it passes on the bench with matched references, and this is one of the most common and
most confusing field failures.

**Phase recovery** (coherent receivers only) tracks the carrier phase. Non-coherent FSK
receivers skip this step entirely, which is precisely the simplification that makes FSK
attractive for fast-acquisition, low-power operation (Section 6.4).

### 17.4 Demodulation and the Value of Soft Decisions

Demodulation extracts the symbol estimate. For FSK, a **discriminator** measures instantaneous
frequency — classically a limiter-discriminator, in modern radios a digital
arctangent-differentiator on the I/Q samples. For PSK, an I/Q correlator compares against the
constellation.

The critical architectural choice is what the demodulator *outputs*. A **hard decision** outputs
a bit: this symbol was a 1. A **soft decision** outputs the bit plus a confidence — how far the
sample was from the decision boundary. Chapter 9 established that soft decisions are worth ~2 dB
of coding gain, so a modem feeding an FEC decoder should always pass soft metrics forward and
never threshold early. This is a design rule that costs nothing but a few bits of datapath width
and is frequently missed, leaving 2 dB of sensitivity unclaimed.

The soft metric for FSK is naturally the magnitude of the discriminator output; for I/Q
schemes, the Euclidean distance to the nearest constellation point. Preserving it through the
chain is the modem's contribution to the coding gain of Chapter 9.

### 17.5 The Preamble's Work

It is worth collecting what the preamble accomplishes, because its length is a design parameter
that trades directly against airtime and energy, and every microsecond of it must be justified.
During the preamble the receiver must, in sequence: settle the AGC, acquire symbol timing,
estimate and correct frequency offset, and prepare the demodulator — all before the sync word
arrives to mark the start of real data. A preamble too short leaves one of these incomplete and
the packet is missed; a preamble too long wastes airtime (which under Chapter 15's duty cycle is
legally scarce) and receive energy (which sets battery life). Typical Sub-GHz preambles run 4–8
bytes; BLE uses 1–2 bytes because its higher symbol rate packs the same acquisition time into
less airtime. Chapter 18 treats the full packet structure of which the preamble is the opening.

### 17.6 Hardware Acceleration Versus Software

Where does the modem run? The answer has migrated over time and shapes how firmware interacts
with it.

Early software-defined radios ran the whole chain on a general-purpose processor, which is
flexible but power-hungry — unacceptable for a battery device. Modern low-power transceivers put
the entire modem in **dedicated hardware**: fixed-function blocks for down-conversion, filtering,
timing recovery, and demodulation, configured by registers and running autonomously. Firmware
sets parameters and reads results; it does not process samples.

The trade is the usual one. Hardware is efficient and fixed; software is flexible and costly.
The low-power radio world sits firmly at the hardware end, which is why Part V's firmware is
about *configuring and coordinating* a modem rather than *implementing* one — the DSP is in
silicon, and the firmware's job is to drive it correctly.

### 17.7 Calibration

Section 16.8 listed the front-end non-idealities; the modem is where several are measured and
corrected. At startup and sometimes periodically, a modern transceiver runs calibration
routines: I/Q imbalance correction, DC offset cancellation, and sometimes filter and oscillator
trimming against temperature. These consume startup time and energy, which is why a
deeply duty-cycled device must weigh how often to recalibrate against how much the temperature
has drifted — a trade that appears again in the state-machine timing of Chapter 19.

### 17.8 Equalisation: Undoing the Channel

Section 3.4 named inter-symbol interference as a correctable impairment and Section 12.6 tied it
to frequency-selective fading; the modem block that corrects it is the **equaliser**, and while
narrowband Sub-GHz systems often avoid it, wideband and high-rate systems cannot, so it belongs
in any complete account of the modem.

An equaliser is a filter that approximates the *inverse* of the channel: if the channel smears a
sharp pulse into a spread one, the equaliser sharpens it back. Several architectures span the
cost-performance range:

- **Linear equaliser** — a tapped-delay-line filter whose coefficients are set to flatten the
  combined channel-plus-equaliser response. Simple, but it amplifies noise at frequencies where
  the channel is deeply attenuated (inverting a near-null means multiplying by a huge number,
  which magnifies the noise there), so it performs poorly on channels with deep spectral nulls.
- **Decision-feedback equaliser (DFE)** — uses the receiver's own past decisions to subtract the
  ISI they contribute to the current symbol. Because it feeds back decided (noise-free) symbols
  rather than the noisy received signal, it does not amplify noise the way a linear equaliser
  does, and it handles severe channels far better — at the risk of *error propagation*, where a
  wrong decision corrupts the subtraction and causes further errors.
- **Maximum-likelihood sequence estimation** — the Viterbi algorithm of Chapter 9 applied to the
  channel's ISI, finding the most likely transmitted sequence given the smeared received signal.
  Optimal, and the same machinery that decodes convolutional codes, but its complexity grows
  exponentially with the channel's memory, limiting it to short-memory channels.

The equaliser must be *adaptive*, because the wireless channel changes (Chapter 12). It typically
trains on a known sequence — a preamble or embedded pilot symbols — to set its coefficients, then
tracks the channel's variation during the packet. This is a further job the preamble does
(Section 17.5) in equalised systems, and a reason wideband preambles are longer. The decision of
whether to include an equaliser is, at bottom, the narrowband-versus-wideband decision of
Section 12.6: a signal narrower than the coherence bandwidth fades flat and needs no equaliser,
which is exactly why Sub-GHz omits the block and Wi-Fi cannot.

### 17.9 The AGC Loop and the ADC Dynamic-Range Budget

Section 17.2 introduced automatic gain control as a step in the chain; its dynamics deserve
development, because AGC settling time is one of the hidden costs the preamble pays for and
because the AGC and ADC together set the receiver's dynamic range — the span between the weakest
and strongest signals it can handle.

The AGC is a feedback loop: it measures the signal level after the variable-gain amplifier,
compares it to a target, and adjusts the gain to hit the target. Its **settling time** — how long
it takes to converge after a signal appears — trades against stability. A fast loop settles
quickly (good, because it must be ready before the payload arrives) but risks overshoot and
oscillation; a slow loop is stable but wastes preamble. For a duty-cycled receiver that wakes and
must acquire within a few symbols, AGC settling is a real constraint, and it is one reason
non-coherent FSK (Section 6.8), which tolerates rapid acquisition, suits low-power operation.

The AGC's purpose is to present the ADC with a signal that uses its range well, and here the
**dynamic-range budget** becomes explicit. An ADC of `N` bits provides a dynamic range of
approximately:

$$
\text{DR} \approx 6.02 N + 1.76\ \text{dB}
$$

so an 8-bit ADC gives ≈ 50 dB, a 12-bit ADC ≈ 74 dB. This range must accommodate, simultaneously,
the weakest wanted signal (near the noise floor) and the strongest interferer the front end
passes (Section 14.2). If a strong adjacent-channel signal is 40 dB above the wanted signal, and
the wanted signal needs 10 dB of SNR above quantisation noise, then the ADC must span at least
50 dB *at the sampling instant* — and if the AGC has set the gain for the strong interferer, the
wanted signal risks falling into the ADC's quantisation noise. This is why receivers with
demanding blocking requirements use higher-resolution ADCs and why the AGC strategy — how it
weights wanted signal against interferers when setting gain — is a genuine design decision, not a
mere convenience. The AGC, the ADC resolution, and the front-end filtering together determine
whether a receiver can hear a weak signal in the presence of a strong one, which is precisely the
coexistence challenge of Chapter 14 seen from inside the modem.

### 17.10 What Carries Forward

- The modem turns I/Q samples into bits by undoing impairments in a fixed order; the channel
  filter sets receiver bandwidth and therefore sensitivity.
- Three synchronisations must succeed in order — timing, frequency, phase — and frequency offset
  from cheap crystals is a leading field-failure cause.
- Soft decisions are worth ~2 dB and must be preserved to the FEC decoder; never threshold
  early.
- The preamble buys time for AGC, timing, and frequency acquisition; its length trades against
  airtime and energy.
- Low-power modems live in dedicated hardware, so firmware configures and coordinates rather
  than computes — which is exactly what Part V describes.

---

# Part V — Silicon, Firmware, and Practice

---

# Chapter 18

## PHY Packet Structure: Preamble, Sync Word, Header, Payload, CRC

### 18.1 Why the Frame Has the Shape It Does

Every wireless packet has the same skeleton, and the skeleton is not arbitrary — each field
solves a specific problem that Parts I–IV identified. Reading a packet structure is reading a
compressed history of the receiver's needs: the preamble is there because synchronisation is
hard (Chapter 17), the sync word because false detection is dangerous (Chapter 1), the CRC
because the channel corrupts (Chapter 9). This chapter assembles those pieces into the frame
that Part V's firmware will build and parse.

The generic Sub-GHz / BLE frame:

```
+----------+-----------+--------+---------+-------+
| Preamble | Sync Word | Header | Payload |  CRC  |
+----------+-----------+--------+---------+-------+
   4-8 B      1-4 B      1-4 B    0-255 B   2-4 B
```

Each field is examined in transmission order.

### 18.2 The Preamble

The preamble is a repeating pattern — almost always alternating ones and zeros, transmitted as
`0x55` or `0xAA` bytes — whose sole purpose is to give the receiver a known signal to lock onto.
Chapter 17 enumerated the work done during it: AGC settling, symbol-timing acquisition,
frequency-offset estimation. Under 2-FSK the alternating pattern produces a clean tone pair at
the two deviation frequencies, which is the ideal input for a timing-recovery loop.

The length is a genuine design parameter, not a constant to copy from an example. Too short and
acquisition is incomplete, raising the packet miss rate; too long and airtime is wasted — which
under Chapter 15's duty cycle is legally scarce and under a battery budget is energy spent
listening. Typical values:

| System | Preamble | Rationale |
|---|---|---|
| BLE 1M | 1 byte (8 bits) | High symbol rate packs acquisition into little airtime |
| BLE Coded | 80 µs pattern | Longer, because coded reception needs more settling |
| Sub-GHz 50 kbps | 4 bytes | Moderate rate, moderate crystal tolerance |
| Sub-GHz long-range | 8+ bytes | Loose crystals and low SNR need more acquisition time |
| Wake-on-radio | 100s of bytes | Receiver samples periodically; preamble must span a full sniff period |

The wake-on-radio case is worth noting: a deeply duty-cycled receiver that wakes for a few
microseconds every few milliseconds will miss a short preamble entirely, so the transmitter
must send a preamble long enough that at least one receiver wake-up lands inside it. Here the
preamble length is set by the *receiver's* sleep schedule, not by acquisition time — a
system-level coupling that catches designers who tune the two ends independently.

### 18.3 The Sync Word

After the preamble, the sync word (also access address, in BLE) marks the exact boundary
between "getting ready" and "real data starts now". The receiver correlates incoming bits
against the known sync word and declares a packet when the correlation exceeds a threshold.

The sync word solves two problems at once. It provides **frame alignment** — the precise bit
position where the header begins — and it provides **false-packet rejection**. Chapter 1 warned
that a PHY can detect a packet in pure noise; the sync word is the primary defence. A 32-bit
sync word is matched by random noise with probability `2⁻³²`, so a receiver processing millions
of noise-bit-positions per second still false-triggers only rarely.

Sync word choice is a small art. It should have good autocorrelation (a sharp peak only at zero
offset, so it is not confused with a shifted copy of itself), should not resemble the preamble
(or the correlator may trigger inside the preamble), and should be chosen to differ between
nearby networks so that one network's packets are not detected by another's receiver. BLE
reserves specific access addresses for advertising (`0x8E89BED6`) and derives random ones for
each connection, precisely to keep piconets from detecting each other.

The trade in length: a longer sync word rejects false packets better but costs airtime and,
crucially, must survive channel errors — a sync word with too many bit errors is missed, dropping
the packet, so the detection threshold usually allows a few mismatched bits, trading false-alarm
rate against miss rate.

### 18.4 The Header

The header carries the metadata the receiver needs to parse the rest of the packet. Its contents
vary, but the essential field is nearly always the **length** — how many payload bytes follow —
because a variable-length packet cannot be parsed without it.

This creates the bootstrapping problem noted in Chapter 9: the receiver must decode the header
*before* it knows the payload length, so the header must be self-contained and robustly protected.
Many systems give the header its own CRC or its own stronger coding, separate from the payload's,
so that a corrupted header fails cleanly rather than causing the receiver to read a wrong length
and misframe everything after it. BLE Coded PHY's decision to always transmit the coding
indicator at S=8 (Section 9.6) is exactly this principle: protect the field that everything else
depends on more strongly than the fields that depend on it.

Other common header fields: a packet type (data, acknowledgement, control), addressing (though
often this lives in the MAC payload above the PHY), and flags for whitening, encryption, or
sequence numbers.

### 18.5 The Payload

The payload is the actual data, handed down from the MAC layer above. From the PHY's point of
view it is opaque bytes — the PHY neither knows nor cares what they mean, consistent with the
layer boundary of Chapter 1. The PHY's only concerns are its length (bounded by the header
field's range, commonly 255 bytes) and that it is whitened and CRC-protected.

Payload length interacts with everything: Chapter 8 showed that long payloads need better SNR
(`PER ≈ N × BER`), Chapter 14 showed they present a larger collision target, and Chapter 15
showed they consume more of the duty-cycle budget. The PHY does not choose the length, but the
protocol designer who does is making a decision with consequences at every layer below.

### 18.6 The CRC

The CRC closes the frame, and Chapter 9 covered it fully: computed over header and payload before
whitening, checked after de-whitening, admitting undetected errors with probability ~`2⁻ⁿ`. Its
placement at the end is not incidental — the transmitter can compute it as the payload streams
out, and the receiver can check it as the payload streams in, so neither end needs to buffer the
whole packet before starting.

### 18.7 Two Real Frame Formats

**BLE (uncoded) link-layer packet:**

```
+----------+----------------+-----------+-------------+-------+
| Preamble | Access Address |  Header   |   Payload   |  CRC  |
|  1 byte  |    4 bytes     | 2 bytes   |  0-255 B    | 3 B   |
+----------+----------------+-----------+-------------+-------+
                            | whitened over this span |
```

The preamble and access address are transmitted unwhitened (the receiver needs them to
synchronise before it can de-whiten); whitening covers header, payload, and CRC. The CRC is 24
bits, seeded per-connection.

**A typical 802.15.4-style Sub-GHz frame:**

```
+----------+-----------+--------+---------------+-------+
| Preamble |    SFD    | PHR    |    PSDU       |  FCS  |
|  4 bytes |  1 byte   | 1 byte |  0-127 B      | 2 B   |
+----------+-----------+--------+---------------+-------+
```

SFD is the start-of-frame delimiter (the sync word), PHR the PHY header (carrying the length),
PSDU the payload, and FCS the frame check sequence (the CRC-16 of Chapter 9). The parallels to
BLE are exact — the same skeleton, different names and field widths, which is the point of this
chapter.

### 18.8 Addressing, Filtering, and the Hardware That Reads Headers

Section 18.4 treated the header's length field; most real headers carry more, and the fields that
matter most for power are the ones that let the radio's *hardware* decide whether a packet is
worth waking the processor for. This capability — **hardware address filtering** — is one of the
most important energy-saving features in a low-power radio, and understanding it explains a header
field that would otherwise seem to belong to the MAC.

A duty-cycled receiver in a busy network hears many packets, most of them addressed to other
nodes. If every received packet woke the processor to be parsed and then discarded, the receive
energy would be dominated by packets the node did not want. Hardware address filtering avoids
this: the radio's packet engine reads the address field in the header *itself*, compares it to a
configured local address (and often a broadcast address), and raises the "packet received"
interrupt only for packets that match. Packets for other nodes are dropped silently, in hardware,
without ever waking the processor. In a network where a node's own traffic is 1% of the airtime,
this cuts the packet-processing wakeups by a hundredfold, and the energy saving is
correspondingly large.

The filtering hardware typically supports several match conditions, configured through registers
the firmware sets at initialisation:

- **Exact address match** — accept only packets addressed to this node.
- **Broadcast match** — also accept packets to the broadcast address.
- **Multicast / group match** — accept packets to a configured group.
- **Promiscuous mode** — accept everything, used for sniffing and diagnostics (Chapter 24).

The header must therefore place the address early and in a fixed position, so the hardware can
find it before deciding whether to continue. This is why address fields sit near the front of the
header, right after the length: the packet engine reads length, then address, and makes its
keep-or-drop decision as early in the packet as possible to minimise the energy spent on
unwanted traffic. The field that looks like a MAC concern is positioned by a PHY energy
constraint — a small but telling example of how the layer boundary is negotiated in practice.

### 18.9 Multi-Frame Structures and Aggregation

The single-frame picture of Section 18.1 is complete for simple protocols, but high-throughput
systems complicate it, and the complications recur across standards in ways worth recognising.

**Aggregation** combines several payloads into one transmission to amortise the fixed per-packet
overhead — preamble, sync word, and the turnaround gaps between packets. At high data rates the
preamble and inter-frame spacing can consume more airtime than a short payload itself, so sending
several payloads under one preamble sharply improves efficiency. Wi-Fi's A-MPDU aggregation is the
prominent example; BLE's data-length extension serves a similar purpose by allowing longer
payloads under one packet header. The trade is the packet-length effect of Section 8.5: an
aggregated frame is longer, so it needs better SNR and presents a larger collision target, which
is why aggregation is used when the channel is good and abandoned when it degrades.

**Fragmentation** is the opposite move: splitting a large payload across several frames when the
channel is marginal, because Section 8.5 showed that shorter frames tolerate worse SNR for a given
frame-error rate. On a poor link, sending four 64-byte fragments may deliver more successful data
than one 256-byte frame, even though the fragments carry more aggregate overhead, because each
fragment's success probability is so much higher. Adaptive protocols choose the frame length
dynamically from the link quality — aggregating when the channel is good, fragmenting when it is
bad — which is a direct application of the PER-versus-length relationship and one of the clearest
places where a PHY-level result (Section 8.5) drives a MAC-level decision.

### 18.10 What Carries Forward

- The frame skeleton is universal: preamble, sync word, header, payload, CRC — each solving a
  specific problem from Parts I–IV.
- Preamble length is set by acquisition needs or by the receiver's sleep schedule (wake-on-radio),
  not copied blindly.
- The sync word provides alignment and false-packet rejection; its length trades false-alarm
  against miss rate.
- The header must be self-protected because everything after it depends on its length field.
- CRC placement at the end lets both ends stream rather than buffer.

---

# Chapter 19

## PHY State Machines and Timing

### 19.1 The Radio Is Not Always On

A wireless PHY is not a continuously running system. It is a state machine that spends most of
its life asleep, wakes to transmit or receive, and returns to sleep — and for a battery device
the *transitions* between these states, not the states themselves, dominate both the timing
budget and the energy budget. This chapter maps the states and the transitions, and it is the
conceptual model that Chapter 20's firmware implements.

The reason a state machine exists rather than a simple on/off is that a radio cannot transmit or
receive instantly. Oscillators must stabilise, synthesisers must lock, calibrations must run.
These take time, and managing that time correctly is the difference between a radio that meets
its timing requirements and one that misses packets intermittently for reasons that are
maddening to debug.

### 19.2 The Canonical States

Most transceivers implement a variation of these states:

| State | Power | Function | Typical entry time |
|---|---|---|---|
| **Sleep / Off** | Lowest (nA–µA) | Everything off except wake logic | — |
| **Standby / Idle** | Low (µA) | Registers retained, oscillator off | — |
| **Ready / FS-on** | Medium | Frequency synthesiser locked | 50–200 µs from standby |
| **RX** | High (mA) | Receiving | 100–300 µs from ready |
| **TX** | Highest (tens of mA) | Transmitting | 100–300 µs from ready |
| **Calibration** | Medium | Trimming I/Q, DC, oscillator | 100 µs–1 ms |

The power spread across these states is enormous — often five orders of magnitude between sleep
and transmit — which is why the *fraction of time* spent in each state determines average current
and therefore battery life. A sensor that transmits for 10 ms per minute spends 0.017% of its
life in TX; if it is not returned promptly to sleep afterward, the idle leakage between
transmissions can exceed the transmit energy entirely.

### 19.3 The Transitions Are the Expensive Part

The timing that matters is not how long a state lasts but how long it takes to *enter*. Consider
a receive operation from sleep:

```
Sleep → (crystal startup: 0.5-2 ms) → Standby
      → (synthesiser lock: 50-200 µs) → Ready
      → (RX warm-up: 100-300 µs) → RX
```

The crystal startup dominates and is often the largest single time and energy cost in a
duty-cycled cycle. A 32 MHz crystal may take 1–2 ms to stabilise — far longer than the actual
packet reception — which is why low-power protocols work so hard to avoid waking from full sleep,
preferring a standby state that keeps the crystal running at the cost of higher idle current. The
optimal sleep depth depends on the wake interval: for frequent wakes, staying in standby saves
the repeated startup cost; for rare wakes, deep sleep wins despite the startup penalty. This
crossover is a real calculation every low-power design must make.

### 19.4 Timing Sources

A radio needs two kinds of clock, and confusing their roles causes subtle timing bugs:

- A **high-frequency clock** (typically a 26–40 MHz crystal) times the RF operations — symbol
  timing, synthesiser reference. It is accurate but power-hungry, so it runs only when the radio
  is active.
- A **low-frequency clock** (typically a 32.768 kHz crystal, or a less accurate internal RC
  oscillator) times the *sleep* interval — how long to stay asleep before the next wake. It is
  cheap to run continuously but far less accurate.

The low-frequency clock's accuracy directly sets the required receive window. If two nodes agree
to rendezvous in 1 second, and each low-frequency clock has ±20 ppm error, they may disagree by
up to 40 µs — so the receiver must open its window 40 µs early and close it 40 µs late to be sure
of catching the transmitter. A worse clock means a wider window means more receive energy. This
is why premium low-power designs pay for an accurate 32 kHz crystal rather than using the free
internal RC oscillator: clock accuracy is directly convertible to battery life through the
receive-window width.

### 19.5 Absolute Versus Relative Timing

Two timing philosophies appear, and protocols choose deliberately:

**Relative timing** measures intervals from events — "receive 150 µs after the transmission ends".
Simple, but errors accumulate: each interval's error adds to the next, so a long sequence drifts.

**Absolute timing** references a shared timeline — "the next event is at timestamp T". Errors do
not accumulate because each event is referenced to the same origin. BLE uses this: connection
events are anchored to a common clock, so a missed event does not desynchronise the link.
Absolute timing is more robust but requires the shared clock to be maintained, which is one of
the link layer's central jobs.

### 19.6 Interrupts and Event Signalling

The state machine communicates with firmware through interrupts (developed fully in Chapter 20).
The radio raises an interrupt on events: sync word detected, packet received, transmission
complete, CRC error, buffer threshold reached. Firmware responds by reading status, servicing
the event, and often commanding the next state transition. The timing of this
interrupt-to-response path is critical: if firmware is too slow to respond to a "packet received"
interrupt, the next packet may overwrite the buffer before the first is read — an overrun, and one
of the classic PHY firmware bugs.

### 19.7 Timing Violations and Their Consequences

The state machine imposes hard timing constraints, and violating them fails in characteristic
ways that Chapter 24 catalogues:

- **Turnaround too slow** — a protocol requiring the receiver to send an acknowledgement within a
  fixed window (BLE's T_IFS is 150 µs) fails if the state machine cannot switch RX→TX in time. The
  ack is late, the transmitter assumes failure, and the link retransmits unnecessarily.
- **Receive window opened too late** — the preamble is missed and the packet is lost even though
  the signal was perfect.
- **Insufficient warm-up** — transmitting before the synthesiser has fully locked emits an
  off-frequency, spectrally dirty signal that may fail the mask and be missed by the receiver.

These are timing bugs, not signal bugs, and they are among the hardest to diagnose because the
RF looks fine — the failure is in *when*, not *what*.

### 19.8 Scheduled Access: TDMA and the Timing It Demands

The state machine of this chapter enables not just isolated transmissions but *scheduled* ones,
and the dominant scheduling discipline in low-power networks — **time-division multiple access
(TDMA)** — places timing demands on the PHY that are worth developing, because meeting them is
what separates a protocol that saves energy from one that merely claims to.

In TDMA, nodes transmit in assigned time slots rather than contending for the channel (as in the
CSMA of Chapter 14). The advantages for a battery device are decisive: a node knows exactly when
its slot occurs, so it can sleep with the radio fully off and wake just in time, and there are no
collisions to recover from because no two nodes share a slot. BLE connections are fundamentally
TDMA — connection events at scheduled intervals — and most long-lived Sub-GHz sensor networks
adopt some scheduled variant for the same reason.

The cost is that TDMA requires the nodes to agree on time, and maintaining that agreement is a PHY
timing problem. Two nodes' clocks drift apart at a rate set by their combined tolerance
(Section 19.4): two ±20 ppm clocks drift by up to 40 ppm, or 40 µs per second of elapsed time
between synchronisations. A node waking for a scheduled slot one second after the last
synchronisation must therefore open its receive window early and close it late by at least this
drift, plus a margin — a **guard time** — to be certain of catching the transmitter despite the
uncertainty. The guard time is pure overhead: the radio is on, listening, but no useful signal is
present for most of it.

This creates a direct trade between synchronisation frequency and energy. Synchronising often
keeps the accumulated drift small, so the guard time is short and the receive window narrow — but
each synchronisation itself costs a wakeup. Synchronising rarely saves those wakeups but widens
every guard window. The optimum depends on the clock accuracy: a node with a precise ±2 ppm TCXO
can tolerate long intervals between synchronisations with small guard times, while a node with a
±100 ppm RC oscillator must either resynchronise constantly or accept wide, wasteful windows. This
is the quantitative payoff of the clock-accuracy discussion of Section 19.4 — clock quality
converts directly into either energy or synchronisation overhead — and it is why premium low-power
designs invest in accurate low-frequency crystals that a casual reading of the bill of materials
might think extravagant.

### 19.9 A Complete Timing Budget for a Wake-Receive Cycle

To make the state-machine timing concrete, it is worth assembling a full timing budget for a
single scheduled wake-and-receive cycle, because the sum of the pieces is what a protocol
designer must fit into a slot and what a firmware engineer must not exceed.

A node wakes to receive a scheduled packet. The sequence, with representative durations:

| Phase | Duration | Source |
|---|---|---|
| Wake low-frequency timer fires, MCU starts | 10–50 µs | MCU wakeup latency |
| High-frequency crystal startup | 300 µs – 2 ms | Section 19.3 (dominant term) |
| Synthesiser lock to channel | 50–150 µs | Section 19.2 |
| RX warm-up (AGC, filter settling) | 50–100 µs | Section 17.9 |
| Guard time for clock drift | 40 µs per second since sync | Section 19.8 |
| Preamble acquisition | 100–200 µs | Chapter 18 |
| Packet reception | payload-dependent | — |
| Processing and return to sleep | 50–200 µs | Chapter 20 |

The striking feature is that for a short packet, the *overhead* — everything before and after the
actual reception — dominates the useful time. Crystal startup alone can exceed the packet
duration by an order of magnitude. This is why the deepest sleep state, which requires full
crystal restart, is not always the most efficient (Section 19.3): if wakes are frequent, keeping
the crystal running in a lighter sleep state avoids repaying the startup cost every cycle, and the
crossover between the two strategies is found by comparing the crystal's idle current against its
startup energy amortised over the wake interval. The timing budget is thus not merely a schedule
to fit into but the raw material of the energy optimisation, and a firmware engineer who can
account for every microsecond in this table is the one who can extend a product's battery life
from months to years.

### 19.10 What Carries Forward

- The PHY is a state machine dominated, in time and energy, by its transitions rather than its
  states.
- Crystal startup is often the largest single cost in a duty-cycled cycle; sleep depth is a real
  optimisation against wake frequency.
- Two clocks: an accurate high-frequency clock for RF, a continuous low-frequency clock for
  sleep, whose accuracy sets the receive-window width and thus battery life.
- Absolute timing resists error accumulation; BLE uses it.
- Timing violations fail in ways that look nothing like signal problems, which is what makes
  them hard.

---

# Chapter 20

## PHY Firmware Architecture: Registers, Interrupts, and DMA

### 20.1 What PHY Firmware Actually Does

Chapter 17 established that in a low-power radio the modem lives in hardware; firmware does not
process samples. So what does it do? It **configures and coordinates**: it writes the registers
that set frequency, modulation, and power; it drives the state machine of Chapter 19; it services
the interrupts the hardware raises; and it moves data between memory and the radio's buffers. PHY
firmware is a control plane, not a data path, and understanding that framing prevents a great
deal of misdirected effort.

This chapter presents the architecture in vendor-neutral C. The register names are generic; the
mapping to real parts is:

| Concept here | SX126x | CC13xx | EFR32 | nRF52 |
|---|---|---|---|---|
| Command interface | SPI opcodes | Radio commands (CPE) | RAIL API | Radio registers + PPI |
| Config registers | SetPacketParams etc. | Command structs | RAIL_Config | Memory-mapped |
| IRQ source | IRQ pin + status | Interrupt flags | RAIL events | EVENTS_* registers |
| Data transfer | Data buffer + SPI | Data queue + DMA | FIFO + DMA | EasyDMA |

The concepts are universal even though every vendor names them differently, and an engineer who
understands the generic model reads any vendor's driver quickly.

### 20.2 Register Categories

Radio registers fall into four functional groups, and knowing which group a register belongs to
tells you when and how to touch it:

**Configuration registers** set operating parameters — frequency, data rate, deviation,
modulation, power, packet format. Written once during initialisation, rarely changed. Writing
some of them while the radio is active has no effect or causes glitches, so they are set in a
defined state (usually standby).

**Control registers** command actions — enter TX, enter RX, sleep, calibrate. These are the
state-machine triggers of Chapter 19, and their timing matters.

**Status registers** report state — current mode, RSSI, whether a packet is ready, error flags.
Read frequently, often in the interrupt handler.

**Measurement registers** expose metrics — RSSI, link quality, frequency offset estimate, CRC
result. Read after reception to inform higher layers.

A representative configuration structure:

```c
typedef struct {
    uint32_t frequency_hz;      /* carrier centre, e.g. 868300000 */
    uint32_t bitrate_bps;       /* e.g. 50000 */
    uint32_t deviation_hz;      /* FSK deviation, e.g. 25000 */
    uint16_t rx_bandwidth_hz;   /* channel filter width; sets sensitivity */
    int8_t   tx_power_dbm;      /* e.g. 14 */
    uint8_t  preamble_len;      /* in bytes */
    uint32_t sync_word;         /* frame delimiter */
    uint8_t  sync_word_len;     /* in bytes */
    bool     whitening_enable;
    uint8_t  whitening_seed;
    bool     crc_enable;
    uint16_t crc_poly;
} phy_config_t;
```

Note that `rx_bandwidth_hz` is a configuration parameter, and Chapter 17 explained its
consequence: set it too wide and sensitivity degrades directly through the noise-floor equation.
This is the single register most often set wrong, and the resulting sensitivity loss is invisible
without measurement (Chapter 23).

### 20.3 Register Access Patterns

Radios are accessed over a bus, and the access pattern shapes the driver. Most Sub-GHz
transceivers hang off SPI; integrated SoC radios (nRF, EFR32) use memory-mapped registers. The
SPI case is instructive because its latency is explicit:

```c
/* Generic SPI register access. Real drivers add chip-select
 * timing, command framing, and often DMA for the data buffer. */

void phy_write_reg(uint8_t addr, uint8_t value)
{
    spi_select();
    spi_transfer(addr | WRITE_FLAG);
    spi_transfer(value);
    spi_deselect();
}

uint8_t phy_read_reg(uint8_t addr)
{
    uint8_t value;
    spi_select();
    spi_transfer(addr | READ_FLAG);
    value = spi_transfer(0x00);   /* clock out a dummy byte to read */
    spi_deselect();
    return value;
}

/* Burst access matters: the packet buffer is read/written as a
 * block, and per-byte chip-select toggling would waste time in the
 * critical post-reception window. */
void phy_write_burst(uint8_t addr, const uint8_t *data, size_t len)
{
    spi_select();
    spi_transfer(addr | WRITE_FLAG | BURST_FLAG);
    for (size_t i = 0; i < len; i++) {
        spi_transfer(data[i]);
    }
    spi_deselect();
}
```

The reason burst access matters is timing: after a "packet received" interrupt, firmware has a
bounded window to read the buffer before the next packet could overwrite it (Section 19.6).
Reading 255 bytes with 255 individual chip-select cycles may not fit the window; a single burst
does. This is why real drivers use DMA for the buffer transfer — the CPU issues one command and
the DMA engine moves the block while the CPU does other work.

### 20.4 The Initialisation Sequence

Bringing a radio up follows a fixed order, and skipping or reordering steps is a common cause of
a radio that "almost works":

```c
int phy_init(const phy_config_t *cfg)
{
    /* 1. Reset to a known state. */
    phy_hardware_reset();
    phy_wait_ready();                    /* poll until the chip responds */

    /* 2. Set the oscillator and wait for it to stabilise.
     *    Transmitting before this is stable emits off-frequency. */
    phy_set_oscillator();
    if (!phy_wait_osc_stable(2000)) {    /* timeout in µs */
        return PHY_ERR_OSC_TIMEOUT;
    }

    /* 3. Configure RF parameters. Order can matter: some parts
     *    require frequency before bandwidth, etc. Follow the datasheet. */
    phy_set_frequency(cfg->frequency_hz);
    phy_set_modulation(cfg->bitrate_bps, cfg->deviation_hz);
    phy_set_rx_bandwidth(cfg->rx_bandwidth_hz);
    phy_set_tx_power(cfg->tx_power_dbm);

    /* 4. Configure packet handling. */
    phy_set_preamble(cfg->preamble_len);
    phy_set_sync_word(cfg->sync_word, cfg->sync_word_len);
    if (cfg->whitening_enable) {
        phy_set_whitening(cfg->whitening_seed);
    }
    if (cfg->crc_enable) {
        phy_set_crc(cfg->crc_poly);
    }

    /* 5. Run calibration (I/Q, DC offset) now that config is set. */
    phy_calibrate();

    /* 6. Configure which events raise interrupts. */
    phy_set_irq_mask(IRQ_RX_DONE | IRQ_TX_DONE | IRQ_CRC_ERR | IRQ_TIMEOUT);

    /* 7. Enter the low-power idle state, ready for commands. */
    phy_set_state(PHY_STANDBY);
    return PHY_OK;
}
```

Two steps are most often mishandled. Skipping the oscillator-stable wait (step 2) produces
intermittent failures that correlate with temperature, because a cold crystal starts more slowly.
Running calibration before configuration (reordering step 5) calibrates for the wrong parameters.
Both produce symptoms that look like hardware faults but are firmware ordering bugs.

### 20.5 Interrupts: The Backbone of Control Flow

A radio is an asynchronous device: packets arrive when they arrive, transmissions complete when
they complete. Polling for these events wastes power (the CPU must stay awake to poll) and adds
latency. Interrupts are the answer, and the discipline around them defines good PHY firmware.

The governing principle is **minimal ISR, deferred processing**. The interrupt service routine
must be short — read the status, clear the flag, signal the main loop, return. Anything
substantial (parsing the packet, running the protocol) happens outside the ISR, because a long
ISR blocks other interrupts and, worse, may still be running when the next radio event arrives.

```c
/* The ISR: as short as possible. Reads what caused the interrupt,
 * clears it, and hands off. No packet parsing, no SPI bursts if
 * avoidable, no protocol logic. */
volatile phy_event_t g_pending_event;
volatile bool        g_event_ready;

void phy_isr(void)
{
    uint8_t irq = phy_read_irq_status();
    phy_clear_irq(irq);                  /* clear early to catch re-triggers */

    if (irq & IRQ_RX_DONE) {
        g_pending_event = EVT_RX_DONE;
    } else if (irq & IRQ_TX_DONE) {
        g_pending_event = EVT_TX_DONE;
    } else if (irq & IRQ_CRC_ERR) {
        g_pending_event = EVT_CRC_ERR;
    } else if (irq & IRQ_TIMEOUT) {
        g_pending_event = EVT_TIMEOUT;
    }
    g_event_ready = true;
    /* Wake the main loop; return immediately. */
}

/* The deferred handler, run in the main loop or an RTOS task.
 * This is where the real work happens, with interrupts enabled. */
void phy_process_events(void)
{
    if (!g_event_ready) {
        return;
    }
    g_event_ready = false;

    switch (g_pending_event) {
    case EVT_RX_DONE: {
        uint8_t buf[256];
        uint8_t len = phy_read_rx_length();
        phy_read_burst(FIFO_ADDR, buf, len);   /* the big transfer, deferred */
        int8_t rssi = phy_read_rssi();
        mac_deliver_packet(buf, len, rssi);    /* hand up to the MAC */
        phy_set_state(PHY_RX);                 /* re-arm for the next packet */
        break;
    }
    case EVT_TX_DONE:
        mac_tx_complete();
        phy_set_state(PHY_STANDBY);            /* return to low power promptly */
        break;
    case EVT_CRC_ERR:
        mac_rx_failed();
        phy_set_state(PHY_RX);
        break;
    case EVT_TIMEOUT:
        mac_rx_timeout();
        break;
    }
}
```

The reason the buffer read is deferred rather than done in the ISR is deliberate and worth
stating: a burst read of 255 bytes over SPI takes tens of microseconds, and holding the CPU in
the ISR for that long risks missing or delaying other interrupts. The ISR notes *what* happened;
the main loop handles *what to do about it*. Getting this division wrong — doing too much in the
ISR — is the most common structural flaw in PHY firmware, and it produces intermittent, timing-
dependent bugs that are extremely hard to reproduce.

### 20.6 Buffering and the Data Path

Data moves between application memory and the radio's buffer, and the ownership of that buffer is
a frequent source of bugs. The buffer belongs to the radio while a transmission or reception is
in progress; firmware must not touch it during that window. Violating this — modifying a TX
buffer after starting transmission, or reading an RX buffer before reception completes —
corrupts data in ways that appear random.

Three buffering approaches, in increasing sophistication:

- **Blocking** — firmware writes the buffer, starts the operation, and waits. Simple, wasteful of
  CPU and power. Acceptable only for the simplest applications.
- **Interrupt-driven** — firmware starts the operation and sleeps; the completion interrupt wakes
  it. The standard low-power approach, shown above.
- **DMA** — the DMA engine moves the buffer autonomously, and firmware is involved only at start
  and completion. Essential at high data rates where per-byte CPU involvement cannot keep up, and
  increasingly the default even at low rates because it lets the CPU sleep during the transfer.

For long packets at high rates, the buffer may be smaller than the packet, requiring **FIFO
threshold** interrupts: the radio interrupts when the buffer is half-empty (TX) or half-full
(RX), and firmware refills or drains it mid-packet. This streaming mode is more complex but
avoids requiring a full-packet buffer, and it is where DMA becomes not merely helpful but
necessary.

### 20.7 Error Handling and Recovery

Real radios encounter errors — CRC failures, timeouts, buffer overruns, and occasionally a radio
that stops responding entirely (a lockup, sometimes from an electrical transient). Robust firmware
plans for all of them:

```c
typedef enum {
    PHY_OK = 0,
    PHY_ERR_OSC_TIMEOUT,
    PHY_ERR_CRC,
    PHY_ERR_TIMEOUT,
    PHY_ERR_OVERRUN,
    PHY_ERR_NOT_RESPONDING,
} phy_status_t;

/* A watchdog for the radio itself: if it has not raised an expected
 * event within a timeout, assume a lockup and reinitialise. This
 * turns a permanent hang into a recoverable glitch. */
phy_status_t phy_recover(void)
{
    phy_hardware_reset();
    if (phy_init(&g_saved_config) != PHY_OK) {
        return PHY_ERR_NOT_RESPONDING;   /* escalate: hardware fault */
    }
    return PHY_OK;
}
```

The distinction that matters is between *recoverable* and *fatal* errors. A CRC failure is normal
(Chapter 8 designed for a 1% PER) and recovered by re-arming RX. A radio that stops responding is
a fault, recovered by reinitialisation. Confusing the two — treating a normal CRC failure as a
fault and reinitialising the radio on every bad packet — destroys throughput, and it is a
surprisingly common overreaction in immature firmware.

### 20.8 Integrating the PHY with an RTOS

Chapter 20's event model assumed a simple main loop. Most real products run a **real-time
operating system**, and the PHY's interaction with it introduces concurrency concerns that, done
wrong, produce exactly the intermittent, unreproducible bugs this book keeps warning about. The
integration pattern is worth setting down because it is both standard and easy to get subtly
wrong.

The clean structure separates three contexts by priority. The **ISR** (Section 20.5) runs at the
highest priority, does the minimum, and signals a task. A dedicated **PHY task** runs at high
priority, blocks waiting for the ISR's signal, and does the deferred processing — reading the
buffer, running the protocol. The **application** runs at lower priority and consumes the packets
the PHY task delivers. The signalling between ISR and task uses an RTOS primitive designed for
the purpose:

```c
/* RTOS integration: ISR signals a task via a semaphore.
 * The task blocks with zero CPU cost until the radio has an event. */

static SemaphoreHandle_t phy_sem;   /* given by ISR, taken by task */
static QueueHandle_t     rx_queue;  /* PHY task → application */

/* ISR context: give the semaphore, yield if a higher-priority
 * task was unblocked. Nothing else happens here. */
void phy_isr(void)
{
    BaseType_t woken = pdFALSE;
    uint8_t irq = phy_read_irq_status();
    phy_clear_irq(irq);
    g_last_irq = irq;
    xSemaphoreGiveFromISR(phy_sem, &woken);
    portYIELD_FROM_ISR(woken);      /* switch to PHY task immediately if needed */
}

/* PHY task: blocks with no CPU cost until signalled, then does
 * the real work at task level with interrupts enabled. */
void phy_task(void *arg)
{
    for (;;) {
        if (xSemaphoreTake(phy_sem, portMAX_DELAY) == pdTRUE) {
            if (g_last_irq & IRQ_RX_DONE) {
                phy_packet_t pkt;
                pkt.len  = phy_read_rx_length();
                phy_read_burst(FIFO_ADDR, pkt.data, pkt.len);
                pkt.rssi = phy_read_rssi();
                xQueueSend(rx_queue, &pkt, 0);   /* hand to application */
                phy_set_state(PHY_RX);           /* re-arm */
            }
            /* ... other events ... */
        }
    }
}
```

Two concurrency hazards must be guarded against, and both are classic sources of field failures.
First, the radio's registers are a shared resource: if both the PHY task and the application (or a
second task) can issue radio commands, their accesses can interleave and corrupt the radio's
state. The fix is a **mutex** guarding all register access, or — cleaner — a rule that *only* the
PHY task touches the radio, and everyone else communicates with it through queues. Second, the
`g_last_irq` variable is written by the ISR and read by the task; on a multi-issue or
out-of-order core it must be declared `volatile` and, strictly, accessed with appropriate
memory barriers, or the compiler may cache a stale value. These are the kinds of bugs that pass
every bench test and fail once a month in the field, and they are why concurrency discipline
around the PHY is not optional pedantry but the difference between a product that ships and one
that is recalled.

### 20.9 DMA and the Ring Buffer for Continuous Reception

Section 20.6 introduced DMA; a continuous or high-rate receiver needs more — a **ring buffer** fed
by DMA — because at high data rates or with back-to-back packets, the processor cannot be relied
upon to drain a single buffer before the next packet arrives. The ring-buffer pattern decouples
the radio's fill rate from the processor's drain rate and is standard in any serious PHY driver.

The idea is a circular region of memory with a write pointer (advanced by the DMA engine as it
delivers received data) and a read pointer (advanced by firmware as it consumes data). As long as
the read pointer keeps up on average, the buffer absorbs bursts without loss:

```c
/* DMA ring buffer for received data. The DMA engine advances
 * write_idx autonomously; firmware advances read_idx as it consumes.
 * Overrun occurs if write catches up to read — detected, not ignored. */

#define RING_SIZE 2048            /* power of two: mask instead of modulo */
#define RING_MASK (RING_SIZE - 1)

static uint8_t          ring[RING_SIZE];
static volatile uint32_t write_idx;   /* owned by DMA/ISR */
static uint32_t          read_idx;    /* owned by consumer */

/* How many bytes are available to read right now. */
static inline uint32_t ring_available(void)
{
    return (write_idx - read_idx) & RING_MASK;
}

/* Consume up to len bytes; returns the number actually copied. */
uint32_t ring_read(uint8_t *dst, uint32_t len)
{
    uint32_t avail = ring_available();
    if (len > avail) len = avail;
    for (uint32_t i = 0; i < len; i++) {
        dst[i] = ring[(read_idx + i) & RING_MASK];
    }
    read_idx = (read_idx + len) & RING_MASK;
    return len;
}

/* Called from the DMA half/full-transfer interrupt to check health.
 * If the writer has lapped the reader, data was lost — surface it,
 * because a silent overrun corrupts the stream invisibly. */
bool ring_check_overrun(void)
{
    return ring_available() > (RING_SIZE - DMA_BLOCK_SIZE);
}
```

The design choices here are the ones that matter in practice. The ring size is a power of two so
that the wraparound is a cheap bitwise AND rather than a modulo. The write and read indices are
owned by exactly one context each, so no lock is needed for the common case — a lock-free
single-producer, single-consumer queue, one of the few places concurrency can safely proceed
without a mutex. And overrun is *detected and surfaced*, not silently ignored: an overrun means
received data was lost, and a driver that hides it delivers a corrupted stream that looks like a
channel error and wastes days of debugging (Chapter 24). Making the overrun visible turns an
invisible corruption into an explicit, diagnosable event — an instance of the general principle
that a PHY should report the conditions of reception honestly (Section 1.6), because the layers
above can only handle what they can see.

### 20.10 A Register Map, Read and Configured

To ground the abstract register categories of Section 20.2, it is worth walking a small,
representative register map and the configuration it drives, because reading a datasheet's
register tables is a daily task in PHY firmware and the skill transfers across every vendor.

Consider a simplified transceiver with these registers:

| Address | Name | Type | Function |
|---|---|---|---|
| 0x01 | OPMODE | Control | 0=sleep, 1=standby, 2=RX, 3=TX |
| 0x02 | FREQ_MSB | Config | Carrier frequency, high byte |
| 0x03 | FREQ_MID | Config | Carrier frequency, middle byte |
| 0x04 | FREQ_LSB | Config | Carrier frequency, low byte |
| 0x05 | BITRATE | Config | Symbol rate divisor |
| 0x06 | DEVIATION | Config | FSK deviation |
| 0x07 | RXBW | Config | Receive channel-filter bandwidth |
| 0x08 | PACONFIG | Config | TX power and PA ramp |
| 0x09 | IRQFLAGS | Status | Event flags (RX done, TX done, CRC) |
| 0x0A | RSSI | Measurement | Received signal strength |

Frequency is set by loading a 24-bit value into the three FREQ registers. The value is not the
frequency in hertz but a divisor of the synthesiser's reference, and computing it is a routine
firmware operation with a routine pitfall:

```c
/* Convert a carrier frequency to the 24-bit register value.
 * The step size is F_XTAL / 2^19 for a typical fractional-N synth.
 * Integer overflow is the classic bug here: freq_hz * 2^19 overflows
 * 32 bits, so the multiply must be done in 64-bit arithmetic. */
uint32_t freq_to_reg(uint32_t freq_hz, uint32_t f_xtal_hz)
{
    uint64_t step_num = (uint64_t)freq_hz << 19;   /* 64-bit: no overflow */
    return (uint32_t)(step_num / f_xtal_hz);
}

/* Set the carrier by writing the three frequency registers.
 * Order can matter: some parts latch on the LSB write, so write
 * MSB→LSB and let the LSB write commit the whole value atomically. */
void phy_set_frequency(uint32_t freq_hz)
{
    uint32_t frf = freq_to_reg(freq_hz, F_XTAL);
    phy_write_reg(REG_FREQ_MSB, (frf >> 16) & 0xFF);
    phy_write_reg(REG_FREQ_MID, (frf >> 8)  & 0xFF);
    phy_write_reg(REG_FREQ_LSB,  frf        & 0xFF);   /* commits */
}
```

The RXBW register deserves the emphasis Section 20.2 gave it. Its value selects the receive
channel filter's bandwidth, and Section 3.3.3 showed that this directly sets the noise floor: a
filter twice as wide as necessary raises the noise floor by 3 dB and costs 3 dB of sensitivity,
silently. Configuring RXBW to match the actual signal bandwidth — neither wider (excess noise) nor
narrower (signal clipped, ISI) — is among the most consequential single register writes in the
whole configuration, and it is the one most often left at a default that does not match the
deployed data rate. When Chapter 23's sensitivity measurement comes in 3 dB low, RXBW is the
first register to check, and this small map shows exactly where to look.

### 20.11 What Carries Forward

- PHY firmware is a control plane: it configures registers, drives the state machine, services
  interrupts, and moves buffers — it does not process samples.
- The four register groups (config, control, status, measurement) each have their own timing
  discipline; `rx_bandwidth` is the config register most often set wrong.
- Initialisation order is fixed; the oscillator-stable wait and calibration-after-config are the
  steps most often mishandled.
- Minimal ISR, deferred processing: the ISR notes what happened, the main loop decides what to
  do. Doing too much in the ISR is the classic structural bug.
- Buffer ownership transfers to the radio during operations; DMA and FIFO thresholds handle long
  packets. Distinguish recoverable errors from faults.

---

# Chapter 21

## End-to-End Signal Flow: From Buffer to Air and Back

### 21.1 Assembling the Whole Chain

Every previous chapter examined one link in the chain. This chapter connects them, tracing a
single packet from a firmware buffer, through modulation, out of the antenna, across the channel,
into the receiver, and back into a buffer — with the relevant chapter attached at each step. The
purpose is integration: an engineer who can hold the whole path in mind, with the numbers, can
reason about failures that span layers, which is where the hardest bugs live.

### 21.2 The Transmit Path, Step by Step

```
[Application data]
   │  MAC hands down a payload (Chapter 1: PHY treats it as opaque bytes)
   ▼
[Firmware buffer]
   │  Firmware writes the buffer, ownership passes to radio (Chapter 20)
   ▼
[CRC append]  ──────── Chapter 9: computed over payload before whitening
   ▼
[Whitening]   ──────── Chapter 9: LFSR removes structure
   ▼
[FEC encode]  ──────── Chapter 9: redundancy added (if used)
   ▼
[Framing]     ──────── Chapter 18: preamble + sync word prepended
   ▼
[Baseband modulation] ─ Chapter 7: bits → I/Q symbols, Gaussian shaped
   ▼
[Up-conversion] ─────── Chapter 16: mixer shifts to carrier frequency
   ▼
[Power amplifier] ───── Chapter 16: saturated PA (constant envelope, Ch 7)
   ▼
[Antenna]     ──────── Chapter 16: matched, radiates (Chapter 15: within ERP limit)
   ▼
[Free space]
```

Each arrow is a place a real system fails. The CRC-before-whitening order (Chapter 9) is a
frequent bug. The PA operating point (Chapter 16) determines both efficiency and spectral
compliance (Chapter 15). The antenna match (Chapter 16) determines how much of the PA's power
actually radiates. A packet that leaves the buffer correctly can still fail to reach the air
intact if any of these is wrong.

### 21.3 The Channel

```
[Free space]
   │  Free-space path loss (Chapter 12): 20log(d) + 20log(f) + 32.44
   │  Path-loss exponent n = 2 to 6 in real environments (Chapter 12)
   │  Shadowing: log-normal, σ = 4-12 dB (Chapter 12)
   │  Multipath fading: 20-30 dB variation over half a wavelength (Chapter 12)
   │  Interference: co-channel, adjacent, blocking (Chapter 14)
   │  Noise added: thermal floor -174 dBm/Hz + bandwidth + NF (Chapter 3)
   ▼
[Receive antenna]
```

The channel is the one segment firmware cannot touch and the link budget (Chapter 13) exists to
account for. Everything the transmitter and receiver do is in service of surviving what happens
here.

### 21.4 The Receive Path, Step by Step

```
[Receive antenna] ──── Chapter 16: captures µV-level signal
   ▼
[Low-noise amplifier] ─ Chapter 16: FIRST stage sets noise figure (Ch 3 Friis)
   ▼
[Down-conversion] ───── Chapter 16: mixer shifts to baseband
   ▼
[ADC]         ──────── Chapter 17: samples I/Q; quantisation noise
   ▼
[DC removal, DDC, channel filter] ─ Chapter 17: filter sets RX bandwidth → sensitivity
   ▼
[AGC]         ──────── Chapter 17: scales to ADC range
   ▼
[Timing recovery] ───── Chapter 17: finds symbol boundaries (uses preamble, Ch 18)
   ▼
[Frequency correction] ─ Chapter 17: removes crystal offset (Ch 6: up to 34 kHz)
   ▼
[Preamble detect + sync] ─ Chapter 18: aligns frame, rejects false packets
   ▼
[Demodulation → soft decisions] ─ Chapter 17: I/Q → bits + confidence (Ch 9: worth 2 dB)
   ▼
[FEC decode]  ──────── Chapter 9: corrects errors
   ▼
[De-whiten]   ──────── Chapter 9: reverse the LFSR
   ▼
[CRC check]   ──────── Chapter 9: intact? deliver : discard
   ▼
[RX-done interrupt] ─── Chapter 20: minimal ISR signals main loop
   ▼
[Firmware reads buffer] ─ Chapter 20: deferred burst/DMA read
   ▼
[MAC receives packet + RSSI/metrics] ─ Chapter 8: PER, link quality
```

The receive path is longer than the transmit path because reception is harder: the transmitter
knows what it is sending, while the receiver must *discover* timing, frequency, and frame boundary
from a corrupted signal. This asymmetry is why most of the PHY's cleverness — and most of its
failures — live on the receive side.

### 21.5 A Worked Trace With Numbers

To make the chain concrete, here is a single Sub-GHz packet traced with values, tying together
the calculations from across the book:

*Sub-GHz link, 868 MHz, 50 kbps, +14 dBm, 200 m outdoor, n = 2.7:*

| Stage | Value | Source |
|---|---|---|
| TX power at PA | +14.0 dBm | Config |
| Antenna gain (TX) | +2.0 dBi | Chapter 16 |
| Match/trace loss | −1.5 dB | Chapter 16 |
| **ERP** | **+14.5 dBm** | Chapter 15 |
| Path loss (200 m, n=2.7) | −77.0 dB | Chapter 12 |
| Fade + shadow margin consumed | −18.0 dB | Chapter 13 |
| **Power at RX antenna** | **−80.5 dBm** | — |
| Antenna gain (RX) | +2.0 dBi | Chapter 16 |
| RX front-end loss | −1.5 dB | Chapter 16 |
| **Power into LNA** | **−80.0 dBm** | — |
| Noise floor (100 kHz, NF 6) | −118 dBm | Chapter 3 |
| **SNR at demodulator** | **38 dB** | — |
| Required SNR (GFSK, BER 1e-3) | 9 dB | Chapter 8 |
| **Link margin** | **29 dB** | Chapter 13 |

Twenty-nine decibels of margin at 200 m — the link is solid, and could reach considerably further
before the margin exhausts. This single table is the whole book in miniature: modulation choice
(Chapter 7) set the required SNR, bandwidth (Chapter 3) set the noise floor, the channel
(Chapter 12) set the path loss, and the budget (Chapter 13) combined them into the one number
that says whether it works.

### 21.6 Where Multi-Layer Bugs Hide

The value of the end-to-end view is diagnosing failures that no single-chapter view reveals.
Examples that recur in practice:

- A link with **good RSSI but constant CRC failures** — signal strength is fine, so it is not
  path loss; the fault is whitening seed mismatch (Chapter 9) or CRC coverage mismatch
  (Chapter 18). RSSI measures the transmit path and channel; CRC measures the whole chain
  including configuration.
- A link that **works close and fails far, worse than the budget predicts** — the path loss is
  right but a margin is missing; likely fading (Chapter 12) with no diversity (Chapter 8), or a
  path-loss exponent assumed too low.
- A link that **works on the bench and fails in the product** — antenna detuning in the enclosure
  (Chapter 16) or crystal offset the modem cannot track (Chapter 17), neither visible until the
  final assembly.
- A link that **works until another device transmits nearby** — desensitisation (Chapter 14),
  invisible to a link budget that assumes a clean channel.

Each of these is diagnosable only by someone holding the whole chain in view, which is what this
chapter and Chapter 24 exist to build.

### 21.7 The Latency Budget of a Round Trip

Chapter 21 traced the packet's path; overlaying *time* on that path produces the latency budget,
which matters because many protocols impose hard turnaround deadlines — BLE's 150 µs
inter-frame space being the canonical example — and missing them fails the link in the timing-bug
manner of Section 19.7. Assembling the budget shows where the microseconds go and which stages a
firmware engineer can influence.

For a request-response exchange — node transmits, gateway replies — the round-trip latency sums:

| Segment | Duration | Influenced by |
|---|---|---|
| TX processing (CRC, whiten, frame) | 10–50 µs | Often hardware; firmware for setup |
| TX packet airtime | payload / rate | Modulation, packet length |
| Propagation | ~3.3 µs/km | Physics; negligible at short range |
| RX chain latency (through modem) | 10–100 µs | Modem pipeline depth |
| RX processing + RX-done interrupt | 20–100 µs | ISR design (Section 20.5) |
| Turnaround: RX→TX state switch | 100–300 µs | State machine (Section 19.3) |
| Reply airtime + return chain | mirror of above | — |

Two segments dominate and repay attention. The **turnaround** — switching the radio from receive
to transmit to send the reply — is often the single largest fixed cost, because it involves a
state transition and synthesiser re-lock (Section 19.3). A protocol whose inter-frame space is
tighter than the radio's turnaround time simply cannot be met, and this is a hard constraint that
must be checked against the datasheet before a protocol is chosen, not discovered afterward. The
**RX processing** segment is where firmware quality shows: a bloated ISR (Section 20.5) that does
the buffer read inline can add tens of microseconds of latency and jitter, which for a
deadline-driven protocol is the difference between meeting T_IFS and missing it. This is the
practical, timing-domain reason the minimal-ISR discipline of Chapter 20 matters — not merely
throughput, but the ability to meet a hard turnaround deadline at all.

### 21.8 Cross-Layer Timing and the Deadline Chain

The latency budget of a single exchange composes into a chain when a protocol requires several
exchanges — an association, a key negotiation, a multi-fragment transfer — and the composition
reveals cross-layer timing dependencies that no single layer's view exposes.

Consider a scheduled network (Section 19.8) where a node must receive a beacon, compute a
response, and transmit in an assigned slot. The beacon reception has the latency of Section 21.7;
the response computation adds MAC and possibly security-layer processing; and the transmission
must begin at a precise scheduled time. If the sum of reception latency plus computation exceeds
the interval to the assigned slot, the node misses its slot — a failure that originates in the
*composition* of PHY receive latency, MAC processing time, and the schedule, and is invisible to
any one of them examined alone.

This is why the end-to-end view of Chapter 21 extends naturally into a *timing* view, and why the
hardest real-time bugs in wireless systems are cross-layer. A firmware engineer optimising the
ISR, a protocol designer setting the slot schedule, and a security engineer choosing a cipher are
all spending from the same latency budget, and none of them sees the whole of it unless someone
holds the composed chain in view. The discipline that Chapter 24 will call for in debugging —
holding the whole chain in mind, with the numbers — applies to timing as much as to signal: a
deadline miss is diagnosed by decomposing the deadline chain and finding which segment overran,
exactly as a sensitivity shortfall is diagnosed by decomposing the link budget. The method is the
same; only the budget being decomposed differs.

### 21.9 What Carries Forward

- The transmit path is short (the sender knows its data); the receive path is long (the receiver
  must discover timing, frequency, and framing) — which is why failures cluster on receive.
- Every arrow in the chain maps to a chapter and is a place real systems fail.
- A single worked trace with numbers integrates the whole book: modulation sets required SNR,
  bandwidth sets noise floor, channel sets path loss, budget combines them.
- Multi-layer bugs (RSSI-good-but-CRC-bad, works-near-fails-far, bench-vs-product) are
  diagnosable only from the end-to-end view.

---

# Chapter 22

## Reading and Writing a Real Sub-GHz PHY Driver

### 22.1 From Model to Code

The previous chapters built a mental model; this one grounds it in a driver a working engineer
would recognise. The goal is twofold: to make Chapter 20's fragments cohere into something
whole, and to build the skill of *reading* an unfamiliar vendor driver — because in practice you
inherit drivers far more often than you write them, and the ability to map ten thousand lines of
someone else's code onto the model in your head is what makes you productive on a new part in
days rather than weeks.

### 22.2 The Anatomy of a Driver

Nearly every PHY driver, regardless of vendor, decomposes into the same layers. Recognising this
structure is the key to reading any of them:

```
+------------------------------------------------------+
|  Public API: init, send, receive, sleep, set_config  |  ← what the MAC calls
+------------------------------------------------------+
|  State machine management: TX/RX/idle transitions    |  ← Chapter 19
+------------------------------------------------------+
|  Interrupt handling: ISR + deferred processing        |  ← Chapter 20
+------------------------------------------------------+
|  Register access layer: read/write/burst              |  ← Chapter 20
+------------------------------------------------------+
|  Hardware abstraction: SPI/GPIO/timer primitives      |  ← platform-specific
+------------------------------------------------------+
```

When you open an unfamiliar driver, find these five layers first. The public API tells you what
the driver does; the HAL tells you what platform it assumes; the three middle layers are where
the radio-specific logic lives. A driver that does not separate these cleanly — that mixes SPI
transfers into protocol logic, or does packet parsing in the ISR — is a driver that will be hard
to port and hard to debug, and recognising that on first read saves you from trusting it.

### 22.3 A Complete Minimal Driver

The following is a compact but complete driver skeleton that ties the whole of Part V together.
It is vendor-neutral; the `hw_*` calls are the HAL that a real port implements against a specific
chip.

```c
/* --- Public API --- */

typedef enum { ST_SLEEP, ST_STANDBY, ST_RX, ST_TX } phy_state_t;

typedef struct {
    phy_config_t   config;
    phy_state_t    state;
    uint8_t        tx_buf[256];
    uint8_t        rx_buf[256];
    volatile bool  operation_done;
    volatile int8_t last_rssi;
} phy_context_t;

static phy_context_t g_phy;

/* Initialise: follows the fixed order from Chapter 20. */
int phy_open(const phy_config_t *cfg)
{
    g_phy.config = *cfg;

    hw_reset_radio();
    if (!hw_wait_ready(2000)) return PHY_ERR_NOT_RESPONDING;

    hw_set_oscillator();
    if (!hw_wait_osc_stable(2000)) return PHY_ERR_OSC_TIMEOUT;

    phy_write_reg(REG_FREQ,   freq_to_reg(cfg->frequency_hz));
    phy_write_reg(REG_BITRATE, rate_to_reg(cfg->bitrate_bps));
    phy_write_reg(REG_DEV,    dev_to_reg(cfg->deviation_hz));
    phy_write_reg(REG_RXBW,   bw_to_reg(cfg->rx_bandwidth_hz));  /* sets sensitivity! */
    phy_write_reg(REG_POWER,  power_to_reg(cfg->tx_power_dbm));

    phy_configure_packet(cfg);       /* preamble, sync, whitening, CRC */
    phy_calibrate();                 /* after config, per Chapter 20 */

    phy_write_reg(REG_IRQ_MASK, IRQ_RX_DONE | IRQ_TX_DONE | IRQ_CRC_ERR);
    hw_enable_radio_interrupt();

    g_phy.state = ST_STANDBY;
    phy_write_reg(REG_STATE, CMD_STANDBY);
    return PHY_OK;
}

/* Transmit: blocking variant for clarity; production uses the event loop. */
int phy_send(const uint8_t *data, uint8_t len)
{
    if (len == 0 || len > 255) return PHY_ERR_LENGTH;

    /* Buffer ownership passes to the radio after the state command. */
    memcpy(g_phy.tx_buf, data, len);
    phy_write_burst(REG_FIFO, g_phy.tx_buf, len);
    phy_write_reg(REG_PAYLEN, len);

    g_phy.operation_done = false;
    g_phy.state = ST_TX;
    phy_write_reg(REG_STATE, CMD_TX);       /* trigger transmission */

    /* Wait for TX_DONE (a real driver sleeps here and wakes on IRQ). */
    if (!phy_wait_done(TX_TIMEOUT_US)) {
        phy_recover();
        return PHY_ERR_TIMEOUT;
    }

    g_phy.state = ST_STANDBY;
    phy_write_reg(REG_STATE, CMD_STANDBY);  /* back to low power promptly */
    return PHY_OK;
}

/* Receive: arm the radio and return; delivery happens via callback. */
int phy_receive_start(void)
{
    g_phy.state = ST_RX;
    phy_write_reg(REG_STATE, CMD_RX);
    return PHY_OK;
}

/* --- Interrupt path (Chapter 20: minimal ISR) --- */

void phy_isr(void)
{
    uint8_t irq = phy_read_reg(REG_IRQ_STATUS);
    phy_write_reg(REG_IRQ_STATUS, irq);      /* clear by write-back */

    if (irq & IRQ_RX_DONE) {
        g_phy.last_rssi = reg_to_rssi(phy_read_reg(REG_RSSI));
        g_phy.operation_done = true;
        /* Buffer read deferred to phy_poll(); NOT done here. */
    } else if (irq & IRQ_TX_DONE) {
        g_phy.operation_done = true;
    } else if (irq & IRQ_CRC_ERR) {
        g_phy.operation_done = true;         /* recoverable, not a fault */
    }
}

/* --- Deferred processing (Chapter 20: the real work) --- */

void phy_poll(void)
{
    if (!g_phy.operation_done) return;
    g_phy.operation_done = false;

    if (g_phy.state == ST_RX) {
        uint8_t len = phy_read_reg(REG_RXLEN);
        if (len > 0 && len <= 255) {
            phy_read_burst(REG_FIFO, g_phy.rx_buf, len);   /* deferred burst */
            mac_on_packet(g_phy.rx_buf, len, g_phy.last_rssi);
        }
        phy_write_reg(REG_STATE, CMD_RX);    /* re-arm for next packet */
    }
}
```

### 22.4 Reading Someone Else's Driver

When handed an unfamiliar driver, the following order gets you oriented fastest — it is the
practical distillation of everything above:

1. **Find the config struct and the init function.** They tell you every parameter the radio
   needs and the order it must be set. This is the fastest route to understanding what the driver
   assumes.
2. **Find the ISR.** Its length tells you the quality of the driver immediately — a short ISR that
   only sets flags is a good sign; an ISR that parses packets or does SPI bursts is a warning.
3. **Trace one packet through transmit.** Follow the data from the public send function to the
   register write that triggers transmission. Every processing step (Chapter 9's CRC, whitening,
   FEC) will be on that path, in order.
4. **Trace one packet through receive.** This is harder and more revealing, because it exposes how
   the driver handles the asynchronous arrival and the buffer-ownership window.
5. **Find the recovery path.** How does it handle a radio that stops responding? A driver with no
   recovery path will hang in the field.

### 22.5 The Common Pitfalls, Collected

Part V has flagged failure modes as they arose; here they are in one place, because these are the
bugs that actually consume engineering weeks:

- **RX bandwidth set too wide** — sensitivity silently degraded (Chapter 20). Invisible without
  measurement.
- **Whitening seed or CRC coverage mismatch** — sync detects, CRC always fails (Chapter 9).
- **Too much work in the ISR** — intermittent, timing-dependent, unreproducible bugs (Chapter 20).
- **Buffer touched during an operation** — random data corruption (Chapter 20).
- **Oscillator-stable wait skipped** — temperature-dependent intermittent failures (Chapter 20).
- **Calibration before configuration** — calibrated for the wrong parameters (Chapter 20).
- **Recoverable errors treated as faults** — throughput destroyed by needless reinitialisation
  (Chapter 20).
- **Frequency offset exceeds modem tracking** — works with matched references, fails with real
  crystals (Chapter 17).
- **Antenna certified with a different power setting** — radiated power exceeds the legal limit
  (Chapter 15).

Every one of these has a distinctive signature, and Chapter 24 turns the signatures into a
diagnostic procedure.

### 22.6 Portability: Isolating the Hardware-Dependent Layer

Section 22.2 identified the HAL as the bottom layer of a driver; the discipline of *keeping it
isolated* is what determines whether a driver ported to a new microcontroller takes a day or a
month, and since porting is a routine reality it deserves explicit treatment. The principle is
simple to state and constantly violated: **all hardware dependence lives in the HAL, and nothing
above the HAL knows what platform it runs on.**

A well-isolated HAL exposes a small, stable interface — the primitives the driver needs and
nothing more:

```c
/* The complete hardware-abstraction interface. A port to a new MCU
 * reimplements exactly these functions and touches nothing else.
 * If the driver above needs to know the MCU, the abstraction leaked. */

/* SPI transfer of one byte, full duplex. */
uint8_t  hal_spi_transfer(uint8_t out);

/* Chip-select control. */
void     hal_cs_assert(void);
void     hal_cs_deassert(void);

/* Radio reset line. */
void     hal_reset_assert(void);
void     hal_reset_deassert(void);

/* Microsecond-resolution delay and timestamp. */
void     hal_delay_us(uint32_t us);
uint32_t hal_timestamp_us(void);

/* Interrupt line from the radio: enable, disable, and register a
 * callback the port wires to the actual IRQ vector. */
void     hal_irq_enable(void);
void     hal_irq_disable(void);
void     hal_irq_set_callback(void (*cb)(void));
```

Every function above is a few lines on any given MCU, and the driver — all its state-machine logic,
its packet handling, its register sequences — is written *once* against this interface and never
changes across platforms. When the driver is ported from, say, an STM32 to a Nordic SoC, only
these functions are rewritten; the thousands of lines above them are untouched. A driver that
instead sprinkles `HAL_SPI_Transmit()` calls (a vendor-specific function) through its protocol
logic has fused itself to one platform, and porting it means finding and rewriting every such call
scattered through the codebase — the difference between the day and the month. The test of a
well-structured driver is simple: can you find every hardware-dependent line by looking only at
the HAL? If hardware dependence has leaked upward, the structure has failed, and recognising the
leak on first read (Section 22.4) tells you the port will be painful before you begin.

### 22.7 Testing a PHY Driver Without a Radio

A driver whose HAL is isolated gains a second benefit beyond portability: it can be **tested
without hardware**, by substituting a simulated radio for the HAL. This is valuable because
hardware testing is slow, requires instruments, and cannot easily reproduce rare conditions —
overruns, lockups, malformed packets — that a simulated radio can produce on demand.

The technique replaces the HAL with a mock that models the radio's register behaviour in software.
The mock maintains an array representing the register file, responds to reads and writes as the
real radio would, and can be scripted to inject events — raise an RX-done interrupt with a chosen
packet, report a CRC error, simulate a non-responding radio. The driver, unable to tell the mock
from real silicon, exercises its full logic against controlled scenarios:

- Feed a valid packet and assert the driver delivers it correctly with the right length and RSSI.
- Feed a CRC-error event and assert the driver re-arms RX rather than treating it as a fault
  (the Section 20.7 distinction).
- Simulate a radio that stops responding and assert the recovery path (Section 20.7) triggers.
- Inject a buffer overrun and assert it is detected and surfaced (Section 20.9), not hidden.
- Race two events and assert the concurrency handling (Section 20.8) holds.

These tests run in seconds on a development machine, cover error paths that are hard to provoke on
real hardware, and catch regressions the moment they are introduced. They cannot replace hardware
bring-up — they cannot measure sensitivity or verify the spectral mask (Chapter 23) — but they
verify that the driver's *logic* is correct, which separates driver bugs from hardware bugs before
the hardware is even involved. A driver developed this way arrives at bring-up with its firmware
already trustworthy, so that when something fails, the failure is far more likely to be in the RF
or the configuration than in the control logic — which is exactly the localisation that makes the
debugging of Chapter 24 tractable.

### 22.8 What Carries Forward

- Every driver decomposes into five layers: public API, state machine, interrupts, register
  access, HAL. Find them first.
- A short flag-only ISR is the mark of a good driver; a busy ISR is a warning.
- Read an unfamiliar driver by tracing config, ISR, one TX packet, one RX packet, and the
  recovery path.
- The recurring pitfalls have distinctive signatures — the RX-bandwidth and whitening bugs above
  all, because they are silent or misleading.

---

# Chapter 23

## Bring-Up and Measurement: Spectrum, Sensitivity, and PER

### 23.1 Why Measurement Is Not Optional

Every calculation in this book is a model, and a model is a hypothesis about hardware. Bring-up is
where the hypothesis meets reality, and the recurring experience of RF engineering is that the
first measurement disagrees with the calculation — usually because of an impairment the model
omitted. This chapter is the bridge from "the design should work" to "the design does work", and
it is the chapter that separates engineers who ship products from those who ship simulations.

The instruments involved — spectrum analyser, signal generator, vector signal analyser, power
meter — are expensive, but the measurements they enable are the only ground truth. A link budget
that predicts −109 dBm sensitivity is a prediction; a measured −107 dBm is a fact, and the 2 dB
gap is information (implementation loss, as Chapter 8 predicted). A gap of 10 dB is a defect to
be found.

### 23.2 Transmit Measurements

Transmit is measured first, because a broken transmitter makes every receive measurement
meaningless.

**Output power** — measured with a power meter or spectrum analyser into a calibrated load. It
must match the configured value and, critically, must not exceed the regulatory limit *including
antenna gain* (Chapter 15). A common surprise: the measured power differs from the register
setting by a fixed offset, because the datasheet's power table assumes a reference load the actual
match does not provide.

**Frequency accuracy** — the carrier must sit where it should, within the crystal's tolerance. An
error here (Chapter 17) will not show as a transmit fault but will cause receive failures at the
other end, which is why it is measured on transmit where it is visible directly.

**Occupied bandwidth and spectral mask** — the spectrum analyser shows the transmitted spectrum,
which must fit under the regulatory mask (Chapter 15). This is where the pulse shaping of
Chapter 7 is verified: an inadequately filtered signal shows splatter beyond the channel edges
that violates the mask. This measurement is the single most common cause of a first certification
failure.

**Modulation quality** — a vector signal analyser demodulates the signal and reports error vector
magnitude (for I/Q schemes) or frequency-deviation accuracy (for FSK). A deviation that is wrong
by 20% — a frequent configuration error — degrades the far end's sensitivity even though the
spectrum looks fine.

**Harmonics and spurs** — the second and third harmonics (Chapter 15) and any reference spurs must
be below the spurious-emission limits. This requires looking far from the carrier, at 2× and 3×
the frequency, where the antenna match provides the suppression.

### 23.3 Receive Measurements: The Sensitivity Test

Sensitivity is the headline receive measurement, and measuring it correctly is a discipline. The
setup: a signal generator produces a known modulated signal at a known, calibrated power level,
fed to the receiver through a calibrated cable (whose loss must be subtracted). The receiver
reports how many packets it receives.

The procedure:

1. Configure the signal generator to produce packets identical to what the real transmitter sends
   — same modulation, deviation, data rate, packet format, whitening, and CRC. A mismatch here
   invalidates the result.
2. Set a known transmit level, well above sensitivity, and confirm the receiver achieves near-100%
   packet reception.
3. Lower the level in 1 dB steps, sending many packets (1000 or more) at each level, and record
   the packet error rate.
4. The sensitivity is the level at which the PER crosses the defined threshold — 1% for
   802.15.4, or whatever the standard specifies (Chapter 8).

The measured sensitivity is then compared to the calculation of Chapter 3. Agreement within the
2–4 dB implementation-loss budget (Chapter 8) confirms the design. A larger gap points to a
specific fault, and the value of having done the calculation is that it tells you *how much* is
wrong, which narrows the search:

| Sensitivity gap vs. calculation | Likely cause |
|---|---|
| 0–3 dB worse | Normal implementation loss — design is healthy |
| 3–6 dB worse | RX bandwidth too wide, deviation mismatch, or elevated NF |
| 6–10 dB worse | Front-end loss ahead of LNA, poor match, or config error |
| >10 dB worse | Broken signal chain, wrong bandwidth, or gross misconfiguration |

### 23.4 The Packet Error Rate Test

Sensitivity is measured at a single point; the full PER-versus-power curve characterises the
receiver's behaviour and reveals the waterfall shape that Chapter 8 predicted. Plotting PER
against received power should show the sharp transition from near-100% loss to near-0% loss over a
few decibels. A curve that is too gradual indicates a problem — often a frequency offset or timing
issue that costs SNR unevenly. A curve shifted right of prediction is a sensitivity shortfall. The
*shape* is as diagnostic as the position.

### 23.5 The RSSI and Link-Quality Check

The receiver's reported RSSI should track the actual received power linearly across its range.
Bring-up includes verifying this: feed known levels, read reported RSSI, and confirm the mapping.
An RSSI that is nonlinear or offset misleads every higher-layer algorithm that depends on it —
adaptive power control (Chapter 14), link-quality estimation, and channel selection all consume
RSSI and all fail quietly if it is wrong.

### 23.6 The Eye Diagram (Wired and Wideband)

For wired links and wideband receivers, the **eye diagram** (Chapter 10) is the standard signal-
quality view: many symbol periods overlaid, forming an "eye" whose opening measures the margin
against noise (vertical) and jitter (horizontal). A wide-open eye has ample margin; a closing eye
warns of ISI, jitter, or noise. It is the fastest visual check of a high-speed link's health and
the first thing to capture when a wired PHY misbehaves.

### 23.7 A Bring-Up Checklist

The order matters — each step depends on the previous ones being sound:

1. **Power and current** in each state — confirm the state machine (Chapter 19) reaches sleep,
   standby, RX, TX, and that the currents match expectations. A radio that does not sleep will
   fail on battery regardless of RF quality.
2. **Transmit power and frequency** — the transmitter is correct before anything else is trusted.
3. **Spectral mask and spurs** — regulatory viability confirmed early, before design lock.
4. **Modulation quality** — deviation and EVM within spec.
5. **Receive sensitivity** — the headline number, compared to calculation.
6. **PER curve** — the full receiver characteristic.
7. **RSSI linearity** — the metric higher layers depend on.
8. **Range test** — the end-to-end confirmation in the real environment (Chapter 13's budget
   validated against reality).

Skipping ahead — measuring range before confirming transmit power, for instance — produces
results that cannot be interpreted, because a failure could be anywhere. The checklist order
isolates one variable at a time.

### 23.8 Blocking, Selectivity, and Coexistence Measurements

Sensitivity (Section 23.3) measures a receiver against noise; a receiver in the real world of
Chapter 14 must also be measured against *interference*, and these coexistence measurements are
where a receiver that looks excellent on a sensitivity bench reveals whether it survives a shared
band. They are frequently skipped in early bring-up and frequently the cause of a product that
tests well and fails in deployment.

**Adjacent-channel selectivity** measures how well the receiver rejects a signal in a neighbouring
channel. The setup adds a second signal generator, tuned to the adjacent channel, to the
sensitivity test: the wanted signal is held a few dB above sensitivity, the interferer is raised
until the wanted packet error rate degrades to the threshold, and the ratio of interferer to
wanted power at that point is the selectivity, in dB. It measures the guard band and receiver
filtering of Section 14.2, and a good result is tens of dB — meaning the receiver tolerates an
adjacent-channel signal far stronger than the wanted one.

**Blocking (desensitisation)** measures the same idea for a strong *out-of-band* signal
(Section 14.2). The interferer is placed well outside the channel, where the receiver should
filter it entirely, and raised until it desensitises the receiver by driving the front end toward
compression. The level at which a specified desensitisation occurs is the blocking figure, and it
depends on the front-end linearity (the IP3 of Section 3.7) rather than on filtering. A receiver
can have excellent sensitivity and poor blocking — quiet in isolation, but crippled by a strong
nearby transmitter — and only the blocking measurement reveals it. This is the measurement that
predicts whether a Sub-GHz node will survive next to a cellular modem or a Wi-Fi access point, and
skipping it is why "it worked in the lab" so often precedes "it fails in the customer's
installation."

**Intermodulation** measurement uses two interferers whose third-order product (Section 3.7) falls
in the wanted channel, and measures the level at which the resulting in-band product degrades
reception. It characterises the front end's IP3 in operation and predicts behaviour in a band
crowded with multiple transmitters. Together, selectivity, blocking, and intermodulation are the
receiver's coexistence report card, and a bring-up that measures only sensitivity has graded only
the easy exam.

### 23.9 Temperature, Production Test, and the Golden Unit

Laboratory bring-up (Section 23.7) characterises one unit under benign conditions; a product must
work across temperature and across manufacturing variation, and the measurements that ensure this
are what separate a demonstration from a product.

**Temperature.** Every RF parameter drifts with temperature: crystal frequency (Section 19.4),
PA output power, receiver noise figure, and matching-network tuning all move as the device heats
and cools. A design verified only at room temperature can fail at the extremes of its rated range —
a frequency offset that grows past the modem's tracking limit (Section 17.3) at −40 °C, or an
output power that droops below the link budget at +85 °C. Temperature characterisation measures the
key parameters across the rated range, and it is where a design's margins are truly tested,
because the room-temperature measurement is the best case and the extremes are where products
fail. A frequency offset that is a comfortable 10 kHz at 25 °C may be 30 kHz at −40 °C, and if the
modem tolerates only 20 kHz, the product works on the bench and fails in a cold warehouse.

**Production test.** Characterising one unit thoroughly is bring-up; testing every unit quickly is
production. The two have opposite priorities — bring-up is exhaustive and slow, production is
minimal and fast — and a production test measures only the few parameters that catch the likely
manufacturing faults: output power (catches PA and match defects), frequency (catches crystal
faults), and a basic sensitivity or packet-error check (catches gross assembly errors). It runs in
seconds per unit because it must, and it is calibrated against a **golden unit** — a reference
device, thoroughly characterised during bring-up, against which production units are compared. A
unit that deviates from the golden unit beyond a set tolerance is rejected. The golden-unit
concept ties production back to bring-up: the exhaustive characterisation done once, on the
reference, defines the pass criteria applied quickly to every unit thereafter, and maintaining a
trustworthy golden unit is one of the quiet disciplines that separates a manufacturable design
from a laboratory prototype.

### 23.10 What Carries Forward

- Every calculation is a hypothesis; measurement is the only ground truth, and the first
  measurement usually disagrees with the model.
- Measure transmit first (power, frequency, mask, modulation quality, spurs) — a broken TX
  invalidates all RX measurements.
- Sensitivity is measured by stepping power and finding the PER threshold; the gap to calculation
  localises the fault.
- The PER curve's shape, RSSI linearity, and (for wired) the eye diagram are all diagnostic.
- The bring-up checklist isolates one variable at a time; its order is not negotiable.

---

# Chapter 24

## Debugging the Physical Layer: A Field Guide

### 24.1 The Discipline of PHY Debugging

The Physical Layer fails differently from every other layer, for the reason Chapter 1 gave: it
produces plausible nonsense rather than clean errors. A dropped packet reports nothing. A
desensitised receiver looks identical to one that is simply out of range. A whitening mismatch
delivers a detected packet with a failed CRC, indistinguishable at a glance from a weak signal.
Debugging the PHY is therefore a discipline of *distinguishing between failures that look
identical*, and this final chapter collects the diagnostic knowledge scattered through the book
into a single procedure.

The governing principle: **the PHY spans firmware, analogue hardware, and physics, and a symptom
in one domain frequently originates in another.** A firmware engineer who assumes the bug is in
firmware, or an RF engineer who assumes it is in the RF, will both spend weeks in the wrong place.
The end-to-end view of Chapter 21 is the antidote.

### 24.2 The First Question: Which Direction?

Before anything else, localise the failure to transmit or receive, because they are debugged
differently. The fastest test is to substitute a known-good device:

- Replace the receiver with a spectrum analyser or known-good radio. If it sees the transmission
  correctly, the transmitter is fine and the fault is in the original receiver.
- Replace the transmitter with a signal generator or known-good radio. If the receiver decodes it,
  the receiver is fine and the fault is in the original transmitter.

This single substitution halves the search space and is worth the setup time on any non-trivial
bug. Skipping it — debugging both ends at once — is the most common way PHY debugging goes slow.

### 24.3 The Symptom-to-Cause Table

The book's diagnostic threads, collected. This table is the chapter's core, and it works because
each symptom pattern points to a small set of causes rather than the whole chain:

| Symptom | Likely cause | Chapter | First check |
|---|---|---|---|
| Sync detected, CRC always fails | Whitening seed mismatch | 9 | Compare seeds both ends |
| Sync detected, CRC always fails | CRC coverage/poly mismatch | 9, 18 | Compare CRC config |
| No packets received at all | Sync word or frequency mismatch | 17, 18 | Verify both on a spectrum analyser |
| Works near, fails far, worse than budget | Missing fade/shadow margin | 12, 13 | Recompute with real exponent |
| Works with matched refs, fails with crystals | Frequency offset exceeds modem tracking | 6, 17 | Measure TX frequency error |
| Sensitivity 3–6 dB low | RX bandwidth too wide | 3, 20 | Check RXBW register vs. signal BW |
| Sensitivity 6–10 dB low | Loss ahead of LNA, poor match | 16 | Measure front-end insertion loss |
| Intermittent, temperature-correlated | Oscillator-stable wait skipped | 19, 20 | Check init sequence timing |
| Intermittent, unreproducible, timing-linked | Too much work in ISR | 20 | Measure ISR duration |
| Random data corruption | Buffer touched during operation | 20 | Audit buffer ownership |
| Fails only when another device is near | Desensitisation / blocking | 14 | Measure with interferer present |
| Fails spectral mask, works functionally | PA nonlinearity, weak pulse shaping | 3, 7, 15 | Check PA backoff, BT product |
| Radiated power over legal limit | Antenna gain not accounted | 15, 16 | Recompute ERP with antenna gain |
| Throughput collapses on marginal link | Recoverable errors treated as faults | 20 | Check error-recovery logic |
| Bursty errors, fine between bursts | Interference | 14 | Spectrum analyser during failure |
| Errors rise smoothly with distance | Noise-limited (normal) | 3, 8 | Confirm against link budget |
| Abrupt failure at a fixed location | Multipath null | 12 | Move node a few cm |

### 24.4 The Whitening/CRC Signature in Detail

The most confusing PHY bug deserves its own treatment, because it is common and its signature is
counterintuitive. When the sync word is *detected* but the CRC *always* fails, the instinct is to
blame signal quality — but signal quality would cause *intermittent* CRC failures, not universal
ones, and would also cause sync detection to fail sometimes. A universal CRC failure with reliable
sync detection means the signal is fine and the *data interpretation* differs between the ends:

- The sync word is transmitted and matched unwhitened (Chapter 18), so it detects correctly
  regardless of the whitening configuration.
- The payload is whitened, and if the seeds differ (Chapter 9), the receiver de-whitens with the
  wrong sequence, producing garbage that fails the CRC every time.
- Alternatively, the CRC polynomial, initial value, or coverage span differs, so a correctly
  received payload computes a different CRC.

The fix is to verify, byte for byte, that both ends agree on: whitening enable, whitening seed,
CRC polynomial, CRC initial value, and exactly which bytes the CRC covers. This five-item check
resolves the overwhelming majority of "detects but never decodes" reports, and it takes minutes
against days of chasing a phantom signal problem.

### 24.5 The Tools and What Each Reveals

| Tool | Reveals | When to reach for it |
|---|---|---|
| Spectrum analyser | TX spectrum, spurs, interference, frequency | Any TX fault, any "fails near other devices" |
| Signal generator | Known reference for RX testing | Any RX fault — substitute for the transmitter |
| Vector signal analyser | Modulation quality, EVM, deviation | Sensitivity shortfalls, "works but marginal" |
| Logic analyser / SPI capture | Register writes, init sequence, timing | Firmware ordering bugs, ISR timing |
| Oscilloscope | GPIO timing, interrupt latency, state transitions | Timing violations (Chapter 19) |
| Power analyser | Current in each state, sleep behaviour | Battery-life failures, stuck-awake radios |
| Known-good radio | End-to-end sanity, direction isolation | Always, as the first substitution |

The logic analyser deserves emphasis, because firmware bugs (the whole of Chapter 20's pitfall
list) are invisible to RF instruments. Capturing the SPI bus during initialisation shows the exact
register writes and their order, which resolves the "calibration before config" and
"oscillator wait skipped" bugs directly — bugs that a spectrum analyser can only show as vague
intermittency.

### 24.6 A Worked Debugging Session

To show the method rather than just the table, a representative session:

*Symptom: a Sub-GHz link works reliably at 50 metres but fails completely at 150 metres — far
worse than a budget that predicted 400 metres.*

1. **Direction isolation.** Substitute a spectrum analyser for the receiver at 150 m. The signal
   is present at −95 dBm, well above the −109 dBm sensitivity. So the transmitter and channel are
   fine, and the fault is in reception — yet the receiver has margin on paper. The contradiction
   is the clue.
2. **Sensitivity measurement.** On the bench, measure the receiver's actual sensitivity with a
   signal generator. It comes in at −98 dBm, not −109. An 11 dB shortfall — too large for
   implementation loss (Chapter 8), pointing to a configuration or front-end fault.
3. **Localise the 11 dB.** From the table (Section 23.3), 6–10 dB suggests front-end loss and
   >10 dB suggests bandwidth. Check the RXBW register: it is set to 200 kHz for a signal that
   occupies 100 kHz. Widening the bandwidth by 2× raised the noise floor by 3 dB (Chapter 3) —
   accounting for 3 dB. The remaining 8 dB: measure the front-end, and find a filter with 2 dB
   loss plus a poor antenna match contributing more.
4. **Fix and confirm.** Correct the RXBW register and rematch the front end. Re-measure
   sensitivity: −108 dBm, within 1 dB of calculation. The 150 m link now works with margin, and
   the range extends toward the predicted 400 m.

The session succeeded because each step used a specific chapter's result to narrow the search: the
budget said the signal should be adequate, the sensitivity measurement said the receiver was the
fault, the gap size localised the cause category, and the register check found it. This is PHY
debugging done well — not guessing, but successive elimination guided by the numbers.

### 24.7 The Closing Principle

The Physical Layer is where the abstraction of digital communication meets the reality of physics,
and its bugs are hard precisely because that meeting is where the illusion of discreteness
(Chapter 1) is manufactured and can break. But the layer is not mysterious. Every symptom has a
cause, every cause has a signature, and every signature is findable by an engineer who holds the
whole chain in view — from the bit in the buffer to the wave in the air and back — and who has the
numbers to say not just that something is wrong, but by how much and therefore where.

That is what this book set out to build: not a catalogue of facts about the Physical Layer, but
the integrated, quantified understanding that turns "the radio doesn't work" into "the receive
bandwidth is 3 dB too wide and the front end is losing 8 dB, here." The difference between those
two sentences is the difference between an engineer who is stuck and one who is finished.

### 24.8 Two More Worked Sessions

The session in Section 24.6 illustrated the method on a sensitivity shortfall. Two more, on
different failure classes, show the same discipline applied where the symptom points elsewhere.

**Session: intermittent failures correlated with nothing obvious.** *A Sub-GHz link works for
hours, then drops several packets in a burst, then recovers, with no correlation to distance or
obvious interference.*

1. **Characterise the pattern.** Log the failures with timestamps and RSSI. The RSSI during
   failures is unchanged from success — so the signal is present and strong, ruling out fading and
   range. The failures cluster in short bursts, which points away from steady noise and toward
   something episodic.
2. **Look for a periodic aggressor.** A spectrum analyser set to record over time, or a
   logging receiver, catches the band during a failure burst. An interferer appears intermittently
   on a nearby channel — a smart meter reporting on a schedule (Section 14.3). The bursts correlate
   with its transmissions.
3. **Confirm the mechanism.** The interferer is adjacent-channel, and its bursts desensitise the
   receiver (Section 24.3). The blocking measurement (Section 23.8), not previously run, confirms
   the receiver's blocking performance is marginal.
4. **Fix.** Improve the front-end selectivity, or — cheaper and often better — enable frequency
   hopping (Section 14.4) so the link is not tied to the one channel the interferer disrupts. The
   hopping also delivers the fading diversity of Chapter 12 as a bonus.

The session succeeded by refusing to accept "intermittent" as a diagnosis and instead
characterising *when* it failed until the pattern named its cause — the discipline of turning a
vague symptom into a specific correlation.

**Session: works on the bench, fails in the enclosure.** *A BLE product passes every test on the
open board and loses 15 dB of range once assembled in its plastic case against a battery.*

1. **Isolate the change.** The only difference is the enclosure and battery, so the fault is
   physical, not firmware — which immediately excludes the whole of Chapter 20's pitfall list and
   points at the antenna (Section 16.4).
2. **Measure the match in situ.** With the board in its enclosure, measure the antenna's return
   loss (Section 16.4). It has shifted badly from its bench value — the enclosure and nearby
   battery have detuned the antenna, reflecting much of the power (Section 16.4).
3. **Confirm the loss.** The degraded match accounts for several dB directly, and the detuning has
   also shifted the antenna's resonance off the operating frequency, costing efficiency — together
   the 15 dB.
4. **Fix.** Re-tune the matching network *with the board in its final enclosure* (Section 16.4),
   accepting that the match is valid only for that mechanical configuration. Re-measure assembled;
   range returns.

This is the canonical bench-versus-product failure of Section 21.6, and it succeeded because the
first step correctly localised the fault to the physical domain by identifying what had changed —
sparing a fruitless search through firmware that was never at fault.

### 24.9 Designing to Prevent Bugs

The best debugging is the bug that never happens, and the failure catalogue of this book doubles
as a checklist of preventable faults. Collecting the preventive measures turns the debugging
knowledge into design discipline.

- **Export honest metadata.** A receiver that reports accurate RSSI, link quality, and frequency
  offset (Section 1.6) hands the debugger the evidence to localise faults. A receiver that reports
  misleading metadata sabotages every diagnosis. Accurate metadata is the single most valuable
  debugging aid, and it is designed in, not added later.
- **Surface errors, never hide them.** Overruns (Section 20.9), lockups (Section 20.7), and
  configuration mismatches should raise explicit, distinguishable signals. A driver that silently
  masks an overrun converts a diagnosable event into an invisible corruption.
- **Isolate the HAL.** A driver whose hardware dependence is confined (Section 22.6) can be tested
  in simulation (Section 22.7), arriving at bring-up with trustworthy logic so that failures
  localise to hardware.
- **Configure RXBW and whitening deliberately.** The two silent-failure champions — receive
  bandwidth (Section 20.10) and whitening seed / CRC coverage (Section 24.4) — should be set
  explicitly and verified, not left at defaults.
- **Budget margins honestly.** A link budget that includes the fading, shadowing, and interference
  margins of Chapter 13, and a battery budget (Section 13.8) that accounts for every state,
  prevents the "works near, fails far" and "dies early" classes of field failure.
- **Measure coexistence, not just sensitivity.** Blocking and selectivity (Section 23.8) predict
  the shared-band failures that a sensitivity-only bring-up misses.
- **Verify across temperature (Section 23.9)** and in the final enclosure (Section 24.8), because
  the bench best case predicts little about the deployed worst case.

Every item on this list is the inverse of a failure the book has catalogued. A design that follows
them does not eliminate debugging — the physical layer will always surprise — but it eliminates the
*avoidable* surprises, leaving only the genuinely hard ones, and it ensures that when a failure
does occur, the system is instrumented and structured to make the failure findable. That is the
final lesson: the physical layer is debuggable in proportion to how honestly it reports itself and
how cleanly it is structured, and both are decisions made at design time, long before the first
bug appears.

### 24.10 What Carries Forward

- PHY failures look alike; debugging is the discipline of distinguishing them.
- Isolate direction first with a known-good substitution — it halves the search space.
- The symptom-to-cause table maps patterns to a small set of causes; the whitening/CRC signature
  is the most common and most counterintuitive.
- Firmware bugs are invisible to RF instruments — reach for the logic analyser.
- Debug by successive elimination guided by the numbers, not by guessing. The measured gap tells
  you where to look.

---

# Appendices

---

## Appendix A — Decibel and RF Mathematics

### A.1 Why Decibels

Communication systems span an enormous dynamic range — a transmitter may radiate a watt while a
receiver detects a femtowatt, a ratio of 10¹⁵. Decibels compress this into manageable numbers and,
crucially, turn the multiplication of gains and losses into addition, which is why every quantity
in this book is expressed in dB.

### A.2 The Definitions

Power ratio:

$$
\text{dB} = 10\log_{10}\left(\frac{P_1}{P_2}\right)
$$

Absolute power referenced to 1 mW (the unit used for all RF power in this book):

$$
\text{dBm} = 10\log_{10}\left(\frac{P}{1\ \text{mW}}\right)
$$

Voltage ratio (note the factor of 20, because power goes as voltage squared):

$$
\text{dB} = 20\log_{10}\left(\frac{V_1}{V_2}\right)
$$

### A.3 The Values Worth Memorising

| Ratio | dB | Mnemonic |
|---|---|---|
| ×2 | +3.01 dB | "3 dB is double" |
| ×4 | +6.02 dB | two doublings |
| ×10 | +10 dB | exact by definition |
| ×100 | +20 dB | two decades |
| ×1000 | +30 dB | three decades |
| ÷2 | −3.01 dB | half power |
| ×1.26 | +1 dB | the smallest change worth noting |

From these, any ratio can be estimated mentally: +26 dB = +20 + 6 = ×100 × 4 = ×400.

### A.4 Absolute Power Reference Points

| dBm | Power | Context |
|---|---|---|
| +30 dBm | 1 W | US Sub-GHz maximum (FHSS) |
| +20 dBm | 100 mW | Wi-Fi, US Sub-GHz typical |
| +14 dBm | 25 mW | EU Sub-GHz ERP limit |
| 0 dBm | 1 mW | Typical BLE transmit |
| −96 dBm | 0.25 pW | BLE 1M sensitivity |
| −109 dBm | 12.6 fW | Sub-GHz 50 kbps sensitivity |
| −123 dBm | 0.5 fW | Sub-GHz narrowband sensitivity |
| −174 dBm/Hz | — | Thermal noise floor at 290 K |

### A.5 Common Conversions

$$
\text{EIRP} = \text{ERP} + 2.15\ \text{dB}
$$
$$
\text{VSWR} \to \text{Return loss:}\quad RL = -20\log_{10}\left(\frac{\text{VSWR}-1}{\text{VSWR}+1}\right)
$$
$$
\text{Watts} \to \text{dBm:}\quad P_{dBm} = 10\log_{10}(P_W) + 30
$$

### A.6 Worked Problems in RF Mathematics

The fluency these conversions require is best built by working problems. The following are the
kinds of calculation that arise daily in PHY engineering, worked in full.

**Problem 1 — Combining gains and losses.** A transmitter outputs +14 dBm. It passes through a
matching network with 1.5 dB loss, a PCB trace with 0.5 dB loss, and an antenna with +2.2 dBi
gain. What is the effective radiated power?

$$
\text{ERP} = 14 - 1.5 - 0.5 + 2.2 = +14.2\ \text{dBm}
$$

Because the antenna gain (+2.2 dBi) very nearly cancels the losses (−2.0 dB), the radiated power
is close to the PA output — but note it now *exceeds* the +14 dBm EU limit (Section 15.6), so this
design would fail certification and the power setting must be reduced.

**Problem 2 — Converting a voltage ratio.** A differential receiver rejects a common-mode noise
voltage, reducing it from 100 mV to 0.1 mV. What is the common-mode rejection in dB?

$$
\text{CMRR} = 20\log_{10}\!\left(\frac{100}{0.1}\right) = 20\log_{10}(1000) = 60\ \text{dB}
$$

The factor of 20 (not 10) applies because this is a voltage ratio (Section A.2).

**Problem 3 — Noise floor from scratch.** A receiver has a 250 kHz bandwidth and a 5 dB noise
figure. What is its noise floor, and if the demodulator needs 8 dB SNR, its sensitivity?

$$
N_{floor} = -174 + 10\log_{10}(250{,}000) + 5 = -174 + 54.0 + 5 = -115.0\ \text{dBm}
$$
$$
S = -115.0 + 8 = -107.0\ \text{dBm}
$$

**Problem 4 — Range from a link budget.** A link has +14 dBm ERP, −107 dBm sensitivity, and 20 dB
of combined margins, at 915 MHz with path-loss exponent 2.5. What range does it reach? First the
allowable path loss:

$$
L_{max} = 14 - (-107) - 20 = 101\ \text{dB}
$$

With the 1 m reference `FSPL(1 m, 915 MHz) = 20\log(0.001) + 20\log(915) + 32.44 = 31.7\ \text{dB}`:

$$
101 = 31.7 + 10(2.5)\log_{10}(d) \Rightarrow \log_{10}(d) = 2.77 \Rightarrow d \approx 590\ \text{m}
$$

**Problem 5 — Doubling distance.** By how much must transmit power increase to double the range of
a free-space link (exponent 2)?

$$
\Delta L = 10 \times 2 \times \log_{10}(2) = 6.0\ \text{dB}
$$

Power must quadruple (+6 dB) to double free-space range — and in a cluttered environment with
exponent 3, it would need to increase by `10 \times 3 \times \log(2) = 9\ \text{dB}`, a factor of
eight, which is why range is so hard to buy with power alone.

**Problem 6 — VSWR to power lost.** An antenna presents a 2.5:1 VSWR. What fraction of power is
reflected?

$$
\Gamma = \frac{2.5 - 1}{2.5 + 1} = 0.429, \qquad \text{reflected} = \Gamma^2 = 0.184 = 18.4\%
$$

so 18.4% of the power is reflected and 81.6% radiated — a loss of `10\log_{10}(0.816) = 0.88\ \text{dB}`.

**Problem 7 — Eb/N0 comparison across bandwidths.** System A runs 100 kbps in 150 kHz at 10 dB
SNR; system B runs 100 kbps in 300 kHz at 7 dB SNR. Which demodulator is more efficient per bit?

$$
\left(\frac{E_b}{N_0}\right)_A = 10 + 10\log_{10}\!\left(\frac{150}{100}\right) = 10 + 1.76 = 11.76\ \text{dB}
$$
$$
\left(\frac{E_b}{N_0}\right)_B = 7 + 10\log_{10}\!\left(\frac{300}{100}\right) = 7 + 4.77 = 11.77\ \text{dB}
$$

Nearly identical — the 3 dB SNR difference is entirely explained by the 2× bandwidth difference,
and neither demodulator is actually more efficient. This is exactly why SNR is a misleading basis
for comparison and Eb/N0 is the fair one (Section 4.5).

---

## Appendix B — Formula Reference

Every working equation in the book, collected for quick reference.

**Bit rate and symbol rate** (Ch 1)
$$R_b = R_s \log_2(M)$$

**Bandwidth estimate** (Ch 2)
$$B \approx (1+\alpha)R_s \qquad B_{FM} \approx 2f_d + R_b\ \text{(Carson)}$$

**Modulation index** (Ch 2)
$$h = \frac{2f_d}{R_s}$$

**Thermal noise power** (Ch 3)
$$N = kTB \qquad kT|_{290K} = -174\ \text{dBm/Hz}$$

**Noise floor and sensitivity** (Ch 3, 8)
$$N_{floor} = -174 + 10\log_{10}(B) + \text{NF}$$
$$S = N_{floor} + \text{SNR}_{req}$$

**Noise figure cascade (Friis)** (Ch 3)
$$F_{total} = F_1 + \frac{F_2-1}{G_1} + \frac{F_3-1}{G_1 G_2} + \cdots$$

**Free-space path loss** (Ch 3, 12)
$$\text{FSPL}_{dB} = 20\log_{10}(d_{km}) + 20\log_{10}(f_{MHz}) + 32.44$$

**Path loss with exponent** (Ch 12)
$$PL(d) = PL(d_0) + 10n\log_{10}(d/d_0)$$

**Nyquist limit** (Ch 4)
$$C = 2B\log_2(M)$$

**Shannon capacity** (Ch 4)
$$C = B\log_2(1 + \text{SNR})$$

**Shannon energy limit** (Ch 4)
$$\left.\frac{E_b}{N_0}\right|_{min} = \ln 2 = -1.59\ \text{dB}$$

**Eb/N0 from SNR** (Ch 4)
$$\frac{E_b}{N_0} = \text{SNR}\cdot\frac{B}{R_b}$$

**Bit error rates** (Ch 8)
$$P_b^{BPSK} = Q\!\left(\sqrt{2E_b/N_0}\right) \qquad P_b^{FSK,coh} = Q\!\left(\sqrt{E_b/N_0}\right)$$
$$P_b^{FSK,noncoh} = \tfrac{1}{2}e^{-E_b/2N_0}$$

**Packet error rate** (Ch 8)
$$\text{PER} = 1-(1-\text{BER})^N \approx N\cdot\text{BER}$$

**Processing gain** (Ch 7)
$$G_p = 10\log_{10}(R_{chip}/R_{bit})$$

**Link budget** (Ch 13)
$$P_{rx} = P_{tx} + G_{tx} - L_{tx} - L_{path} - L_{misc} + G_{rx} - L_{rx}$$
$$M = P_{rx} - S_{rx}$$

**Coherence bandwidth and time** (Ch 12)
$$B_c \approx \frac{1}{5\tau} \qquad T_c \approx \frac{1}{f_d} \qquad f_d = \frac{v}{c}f_c$$

**Reflection coefficient** (Ch 10)
$$\Gamma = \frac{Z_L - Z_0}{Z_L + Z_0}$$

**Numerical aperture** (Ch 11)
$$\text{NA} = \sqrt{n_{core}^2 - n_{clad}^2}$$

---

## Appendix C — Sub-GHz and BLE Parameter Tables

### C.1 BLE PHY Modes

| Mode | Rate | Modulation | Sensitivity (typ) | Range (rel) |
|---|---|---|---|---|
| LE 1M | 1 Mbps | GFSK, BT 0.5, h 0.5 | −96 dBm | 1× |
| LE 2M | 2 Mbps | GFSK, BT 0.5 | −93 dBm | 0.7× |
| LE Coded S=2 | 500 kbps | GFSK + conv. 1/2 | −99 dBm | 1.4× |
| LE Coded S=8 | 125 kbps | GFSK + conv. + 4× | −104 dBm | 2.5× |

### C.2 Representative Sub-GHz Configurations

| Rate | Modulation | Deviation | RX BW | Sensitivity (typ) |
|---|---|---|---|---|
| 1.2 kbps | 2-FSK | 5 kHz | 10 kHz | −123 dBm |
| 4.8 kbps | 2-GFSK | 5 kHz | 25 kHz | −118 dBm |
| 50 kbps | 2-GFSK | 25 kHz | 100 kHz | −109 dBm |
| 100 kbps | 2-GFSK | 50 kHz | 200 kHz | −106 dBm |
| 500 kbps | 4-GFSK | 175 kHz | 800 kHz | −98 dBm |
| LoRa SF7 | CSS | — | 125 kHz | −123 dBm |
| LoRa SF12 | CSS | — | 125 kHz | −137 dBm |

### C.3 Required Eb/N0 (BER 10⁻⁵, AWGN)

| Modulation | Eb/N0 | Bits/sym |
|---|---|---|
| BPSK / QPSK / OQPSK | 9.6 dB | 1 / 2 / 2 |
| Coherent 2-FSK | 12.6 dB | 1 |
| Non-coherent 2-FSK | 13.4 dB | 1 |
| 16-QAM | 13.4 dB | 4 |
| 64-QAM | 17.8 dB | 6 |
| Shannon limit | −1.59 dB | — |

### C.4 Quarter-Wave Antenna Lengths

| Band | λ | λ/4 |
|---|---|---|
| 433 MHz | 69.2 cm | 17.3 cm |
| 868 MHz | 34.6 cm | 8.6 cm |
| 915 MHz | 32.8 cm | 8.2 cm |
| 2.44 GHz | 12.3 cm | 3.07 cm |

---

## Appendix D — Standards Map

| Standard | Layer | Band | Modulation | Notes |
|---|---|---|---|---|
| Bluetooth LE (5.x) | PHY + stack | 2.4 GHz | GFSK | 40 ch, AFH; Coded PHY for range |
| IEEE 802.15.4 | PHY + MAC | 2.4 GHz | OQPSK + DSSS | Zigbee, Thread, Matter |
| IEEE 802.15.4g | PHY | Sub-GHz | 2/4-GFSK, OFDM | Smart utility networks |
| LoRaWAN | PHY + MAC | Sub-GHz | CSS | Long range, duty-cycle bound |
| Wi-Fi (802.11) | PHY + MAC | 2.4/5/6 GHz | OFDM, QAM | High rate, wideband |
| FCC Part 15.247 | Regulation | US ISM | — | FHSS/DTS, power rules |
| ETSI EN 300 220 | Regulation | EU Sub-GHz | — | Duty cycle, LBT+AFA |
| ETSI EN 300 328 | Regulation | EU 2.4 GHz | — | Power, medium access |
| 10GBASE-T | Wired PHY | Copper | PAM-16 + LDPC | Cat 6A, 100 m |
| 1000BASE-X | Wired PHY | Fiber/copper | 8B/10B | Gigabit |

---

---

---

## Appendix E — Three Complete Design Walkthroughs

The chapters develop principles; this appendix applies them end to end to three realistic
requirements, showing the whole calculation an engineer performs before committing to a design.
Each walkthrough integrates the link budget (Chapter 13), the regulatory constraints (Chapter
15), the modulation choice (Chapters 6–7), and the battery budget (Section 13.8) into a single
verdict.

### F.1 A Battery Soil-Moisture Sensor, 868 MHz, Europe

**Requirement.** A soil-moisture sensor reports a 16-byte reading every 15 minutes to a gateway up
to 800 m away across open farmland, on a single AA cell for five years, EU compliant.

**Band and regulation.** The EU 868 MHz band under EN 300 220 (Chapter 15): +14 dBm ERP, and a
duty-cycle limit — the 868.0–868.6 MHz sub-band permits 1%. A reading every 15 minutes is four per
hour; even at a generous 20 ms airtime each, that is 80 ms per hour against a 36 s/hour budget —
utterly comfortable, so duty cycle is not the constraint here.

**Modulation and rate.** Open farmland is close to line-of-sight, so a moderate path-loss exponent
(≈ 2.5) applies. Range, not throughput, dominates, so choose a low data rate for sensitivity: 2-GFSK
at 10 kbps, deviation 5 kHz (`h = 1.0` for crystal tolerance), receive bandwidth 30 kHz.

**Link budget.** Noise floor: `−174 + 10\log(30{,}000) + 6 = −119.2\ \text{dBm}`. Required SNR for
GFSK ≈ 9 dB, so sensitivity ≈ −110 dBm. With +14 dBm ERP, +2 dBi antennas both ends, 1.5 dB
front-end losses, and 15 dB combined margin (open terrain, mostly Rician):

$$
L_{max} = (14 + 2 - 1.5) - (-110 + 2 - 1.5) - 15 = 14.5 + 109.5 - 15 = 109\ \text{dB}
$$

Range at exponent 2.5 (`FSPL(1 m, 868 MHz) = 31.2\ \text{dB}`):

$$
109 = 31.2 + 25\log_{10}(d) \Rightarrow \log_{10}(d) = 3.11 \Rightarrow d \approx 1290\ \text{m}
$$

Comfortably beyond the 800 m requirement, with margin for the farmland not being perfectly open.

**Battery budget.** Airtime for 16 bytes plus overhead (≈ 26 bytes) at 10 kbps ≈ 21 ms. Per cycle:
TX ≈ 21 ms × 60 mW = 1.26 mJ; brief RX ack window ≈ 0.1 mJ; wakeup ≈ 0.04 mJ; sleep 900 s × 3 µW =
2.7 mJ. Total ≈ 4.1 mJ per 15-minute cycle, or ≈ 0.39 J/day. An AA cell holds ≈ 2700 mAh ≈ 29 kJ,
giving a radio-limited life far beyond five years — so the sensor electronics and self-discharge,
not the radio, will set the actual life.

**Verdict.** Feasible with wide margin. The low data rate buys both the range and, through short
airtime, the battery life. This is the Sub-GHz sweet spot.

### F.2 A Wearable Health Monitor, BLE, Worldwide

**Requirement.** A wrist-worn monitor streams 5 kbps of data to a phone up to 10 m away, worldwide,
on a small rechargeable cell, for a week between charges.

**Band and regulation.** Worldwide reach and phone compatibility mandate BLE at 2.4 GHz
(Chapter 15's fragmentation argument): one design, one certification path, universal phone support.

**Modulation and rate.** BLE 1M GFSK is fixed by the standard. The 5 kbps payload is trivial for a
1 Mbps PHY, so the link spends most of its time idle — good for battery.

**Link budget.** BLE 1M sensitivity ≈ −96 dBm. Body loss is the distinctive challenge: at 2.4 GHz a
wrist-worn antenna against tissue loses 5–10 dB (Chapter 13's body-loss margin). With 0 dBm TX,
−4 dBi effective antenna gain (small, body-detuned), 10 m indoor range at exponent 3:

$$
L_{max} = (0 - 4) - (-96 - 4) - (10 + 8) = -4 + 100 - 18 = 78\ \text{dB}
$$

FSPL at 10 m, 2.44 GHz, exponent 3: `40.2 + 30\log(10) = 70.2\ \text{dB}`. Margin `78 − 70.2 = 7.8\ \text{dB}` —
adequate but not generous, which is realistic for a body-worn 2.4 GHz device and why such products
sometimes struggle when the wrist is turned away from the phone (a further polarisation and body-
shadowing loss).

**Battery budget.** BLE's connection-oriented TDMA (Section 19.8) lets the radio sleep between
connection events. At a 5 kbps offered load with a modest connection interval, the radio is active
well under 1% of the time, and average current is dominated by sleep. A small 100 mAh cell
supports a week easily *provided* the sleep current is genuinely low — the recurring caveat.

**Verdict.** Feasible; the binding constraint is body loss and antenna efficiency, not the budget's
headline numbers. This is why wearable RF design obsesses over antenna placement.

### F.3 A Long-Range Asset Tracker, LoRa, Regional

**Requirement.** A shipping-container tracker reports position twice daily over up to 15 km in a
suburban environment, on a battery lasting the container's multi-year journey, EU compliant.

**Band and technology.** 15 km on a battery demands the deep power-limited regime (Section 4.9):
LoRa chirp spread spectrum (Section 7.5) in the EU 868 MHz band.

**Spreading factor and airtime.** 15 km suburban needs high spreading — SF11 or SF12. At SF12,
125 kHz bandwidth, sensitivity reaches ≈ −137 dBm. But airtime is punishing: a ~30-byte position
report at SF12 occupies well over 1 second. Under the 1% duty cycle (Section 15.3), that permits
only ~30 such packets per hour — far more than the twice-daily requirement needs, so duty cycle is
satisfied, but the airtime dominates the energy budget.

**Link budget.** With +14 dBm ERP, −137 dBm sensitivity, +2 dBi antennas, and 20 dB combined
margin (suburban, higher shadowing):

$$
L_{max} = 16 - (-135) - 20 = 131\ \text{dB}
$$

At 868 MHz suburban exponent ≈ 3: `131 = 31.2 + 30\log(d) \Rightarrow \log(d) = 3.33 \Rightarrow d \approx 2140\ \text{m}$`
per the simple model — short of 15 km, which reveals that reaching 15 km suburban requires either a
gateway with an elevated, higher-gain antenna (the two-ray height advantage of Section 12.11), a
lower path-loss environment than exponent 3, or acceptance that 15 km is achievable only in
favourable conditions. This is the honest outcome: LoRa's advertised "15 km" ranges assume
line-of-sight or elevated gateways, and a ground-level suburban link falls short — exactly the kind
of reality a link budget exposes before deployment rather than after.

**Battery budget.** Two SF12 packets per day at ~1.5 s and ~120 mW ≈ 0.36 J/day for TX, plus sleep.
Over years, sleep current and battery self-discharge dominate; the twice-daily transmissions are a
minor contributor. A high-capacity lithium primary cell supports the multi-year mission.

**Verdict.** Feasible for range *if* the gateway is elevated (Section 12.11); marginal for a
ground-level suburban link. The walkthrough's value is exposing that the headline range figure is
conditional, and identifying antenna height as the lever — before a field trial discovers it the
expensive way.

---

## Appendix F — PHY Troubleshooting Quick Reference

A condensed, scannable companion to Chapter 24, organised for use at the bench when a link is
misbehaving. Find the symptom, check the causes in order, use the named tool.

### G.1 The Five-Minute Triage

1. **Is there signal?** Spectrum analyser or RSSI at the receiver. No signal → transmitter,
   frequency, or antenna. Signal present → receiver or configuration.
2. **Does sync detect?** Sync detected but CRC fails → configuration (whitening/CRC), not signal.
   No sync → frequency offset, sync-word mismatch, or too little signal.
3. **Is it consistent or intermittent?** Consistent → configuration or budget. Intermittent →
   interference, fading, or a timing/concurrency bug.
4. **Bench or product?** Fails only assembled → antenna detuning (Section 16.4).
5. **Near or far?** Fails only far → margin/budget. Fails everywhere → configuration.

### G.2 Symptom Index

- **No packets at all** → frequency mismatch, sync-word mismatch, radio not in RX, antenna
  disconnected. Tools: spectrum analyser, SPI capture.
- **Sync detects, CRC always fails** → whitening seed, CRC polynomial/init/coverage mismatch.
  Tool: compare both ends' config byte for byte (Section 24.4).
- **Sensitivity 3–6 dB low** → RXBW too wide, deviation mismatch. Tool: signal generator + check
  RXBW register (Section 20.10).
- **Sensitivity 6–10 dB low** → front-end loss before LNA, poor antenna match. Tool: VNA on the
  front end (Section 16.4).
- **Works near, fails far** → missing fade/shadow margin, wrong path-loss exponent. Tool:
  recompute budget (Chapter 13).
- **Bench-good, product-bad** → enclosure/body antenna detuning. Tool: VNA in final assembly.
- **Intermittent bursts, RSSI normal** → interference/desensitisation. Tools: time-logged spectrum
  analyser, blocking measurement (Section 23.8).
- **Fails only near another transmitter** → blocking/desensitisation. Tool: blocking test.
- **Fails spectral mask** → PA nonlinearity, weak pulse shaping. Tool: spectrum analyser, check PA
  backoff and BT (Sections 7.1, 15.5).
- **Radiated power over limit** → antenna gain not counted in ERP. Tool: recompute (Section 15.6).
- **Random data corruption** → buffer touched during operation, overrun. Tool: audit ownership
  (Sections 20.6, 20.9).
- **Intermittent, temperature-linked** → oscillator-stable wait skipped, or drift at temperature
  extreme. Tools: SPI capture of init, temperature chamber (Sections 20.4, 23.9).
- **Intermittent, timing-linked, unreproducible** → too much work in ISR, concurrency race.
  Tool: oscilloscope on ISR duration (Sections 20.5, 20.8).
- **Throughput collapses under load** → CSMA congestion past the throughput peak. Tool: measure
  offered vs delivered load (Section 14.8).
- **Turnaround deadline missed** → state-machine switch too slow, or bloated ISR adding latency.
  Tool: oscilloscope on the RX→TX transition (Sections 19.7, 21.7).

### G.3 The Instruments and What Each Is For

- **Spectrum analyser** — TX spectrum, spurs, mask, interference, frequency. First for any TX or
  interference problem.
- **Signal generator** — a known reference to substitute for the transmitter, isolating RX faults.
- **Vector signal analyser** — modulation quality, EVM, deviation accuracy.
- **VNA** — antenna match and return loss, especially in the final enclosure.
- **Logic analyser / SPI capture** — register writes and init order; the only view of firmware
  configuration bugs, which are invisible to RF instruments.
- **Oscilloscope** — GPIO and interrupt timing, state transitions, ISR duration.
- **Power analyser** — per-state current and sleep behaviour, for battery-life faults.
- **Known-good radio** — end-to-end sanity and direction isolation; the first substitution in any
  session.

### G.4 The One-Line Principles

- Isolate direction first; it halves the search.
- Sync-detects-but-CRC-fails is configuration, not signal.
- The measured gap to the calculation tells you where to look.
- Firmware bugs are invisible to RF instruments — reach for the logic analyser.
- The bench is the best case; the enclosure at temperature is the real one.
- A receiver that reports honest metadata is a receiver you can debug.

---

## Appendix G — Glossary

A comprehensive reference to the terms used in this book. Each entry gives a working definition
and the chapter where the concept is developed.

**Adjacent-channel interference** — Interference from a transmitter in a neighbouring channel that
leaks into yours through imperfect filtering; governed by the interferer's spectral spill and your
receiver's selectivity (Ch 14).

**AFH (Adaptive Frequency Hopping)** — Frequency hopping that measures which channels are bad and
removes them from the hopping set, concentrating traffic on clear channels; the reason BLE
coexists well with Wi-Fi (Ch 14).

**AGC (Automatic Gain Control)** — A feedback loop that scales the received signal to use the ADC's
range without clipping; its settling time is part of what the preamble buys time for (Ch 17).

**Aggregation** — Combining several payloads under one preamble to amortise per-packet overhead;
used when the channel is good (Ch 18).

**ARQ (Automatic Repeat reQuest)** — Error recovery by retransmission; appropriate when a return
path exists, latency tolerance is adequate, and the channel is usually good (Ch 9).

**Attenuation** — Loss of signal power with distance through a medium; deterministic, affects
signal but not the receiver's internal noise, and thus the reason range is finite (Ch 3).

**Baseline wander** — Drift of the average signal level on an AC-coupled path when the line code
lacks DC balance, moving the receiver's decision threshold to the wrong side of the signal (Ch 5).

**BER (Bit Error Rate)** — The fraction of bits received in error; the fundamental measure of link
quality, from which packet error rate follows (Ch 8).

**Blocking (desensitisation)** — Degradation of receiver sensitivity by a strong out-of-band signal
that drives the front end toward compression, raising its effective noise figure even though the
signal never enters the passband (Ch 14).

**BT product** — The Gaussian filter's 3 dB bandwidth times the symbol period; the dial that trades
spectral containment against inter-symbol interference in GFSK (Ch 7).

**Capture effect** — A receiver's ability to decode the stronger of two colliding packets if it
exceeds the weaker by the capture ratio; makes real network throughput better than a naive
collision model predicts (Ch 14).

**Carrier** — The sinusoid whose amplitude, frequency, or phase is varied to carry information;
necessary for antenna size, spectrum sharing, and propagation control (Ch 6).

**Coherence bandwidth** — The band over which a multipath channel is approximately flat; divides
narrowband (flat-fading) from wideband (frequency-selective) systems (Ch 12).

**Coherence time** — The interval over which a fading channel stays roughly constant; sets the
minimum retry spacing for time diversity (Ch 12).

**Coherent detection** — Demodulation that uses the carrier's phase, extracting maximum SNR at the
cost of a carrier-recovery loop, acquisition time, and frequency-offset sensitivity (Ch 6).

**Constellation** — The plot of a modulation's valid symbols as points in the I/Q plane; error
probability is set by the minimum distance between points (Ch 6).

**CPM (Continuous-Phase Modulation)** — Modulation whose carrier phase never jumps, giving a
constant envelope and compact spectrum; GFSK, MSK, and GMSK are members (Ch 7).

**CRC (Cyclic Redundancy Check)** — Error detection by polynomial division; detects all bursts up
to its length and admits undetected errors with probability ~2⁻ⁿ — a filter, not a guarantee (Ch 9).

**Crosstalk (NEXT/FEXT)** — Coupling of signal from one cable pair into another; the dominant
impairment in short high-speed wired links (Ch 10).

**CSMA (Carrier-Sense Multiple Access)** — The family of protocols by which uncoordinated nodes
share a channel by sensing before transmitting; has a throughput ceiling well below raw capacity
(Ch 14).

**dBm** — Power in decibels relative to 1 milliwatt; the unit for all RF power in this book
(Appendix A).

**DFE (Decision-Feedback Equaliser)** — An equaliser that subtracts the ISI contributed by past
decisions, avoiding the noise amplification of a linear equaliser at the risk of error propagation
(Ch 17).

**Differential signalling** — Transmission on two conductors carrying opposite voltages, with the
receiver responding to their difference; rejects common-mode noise by 40–60 dB and radiates almost
nothing (Ch 10).

**Diversity** — Obtaining several independent observations of a fading channel so that the
probability of all being faded is small; available in frequency, space, time, or code, and
indispensable in fading (Ch 8, 12).

**Diversity order** — The number of independent branches, equal to the slope of the error-rate
curve on a log-log plot; the single most important number describing a fading-mitigation scheme
(Ch 12).

**Doppler shift** — The frequency shift caused by relative motion; its inverse sets the coherence
time (Ch 12).

**DSSS (Direct Sequence Spread Spectrum)** — Spreading by multiplying data with a fast chip
sequence, yielding processing gain against noise and uncorrelated interference (Ch 7).

**Dispersion** — Pulse spreading in fiber (modal, chromatic, or polarisation-mode); the true reach
limit at high optical rates (Ch 11).

**Duty cycle** — The fraction of time a device transmits; capped by EU regulation (e.g. 1% = 36
s/hour) as the mechanism for sharing Sub-GHz spectrum (Ch 15).

**Eb/N0** — Energy per bit divided by noise power spectral density; the only fair basis for
comparing modulations, because it removes the dependence on receiver bandwidth (Ch 4).

**EDFA (Erbium-Doped Fiber Amplifier)** — An optical amplifier that boosts the whole 1550 nm WDM
band at once without electrical conversion; the enabler of long-haul fiber (Ch 11).

**EIRP / ERP** — Effective (Isotropic) Radiated Power; the radiated-power quantities regulators
limit, so antenna gain counts against the limit (Ch 15, Appendix A).

**Equaliser** — A modem block that approximates the inverse of the channel to undo inter-symbol
interference; required in wideband systems, omitted in narrowband Sub-GHz (Ch 17).

**Eye diagram** — Many symbol periods overlaid, forming an "eye" whose vertical opening measures
noise margin and horizontal opening measures jitter margin (Ch 10, 23).

**Fading** — Random variation of received power; fast (multipath, over a half-wavelength) or slow
(shadowing, over metres); the defining hardship of the wireless channel (Ch 12).

**FEC (Forward Error Correction)** — Adding structured redundancy so the receiver repairs errors
without retransmission; quantified by coding gain (Ch 9).

**FHSS (Frequency-Hopping Spread Spectrum)** — Changing channel on a schedule so no single
interferer blocks more than a fraction of transmissions; provides interference and fading
diversity together (Ch 14).

**Friis cascade** — The equation showing that a receiver's noise figure is dominated by its first
stage; the reason the LNA comes first and loss before it is doubly damaging (Ch 3).

**FSPL (Free-Space Path Loss)** — The propagation loss in free space, rising as the square of both
distance and frequency; the basis of the Sub-GHz advantage (Ch 3, 12).

**Gray coding** — A bit-to-symbol mapping in which adjacent constellation points differ by one bit,
so the most probable symbol error causes only one bit error; free, and always used (Ch 6).

**GFSK (Gaussian Frequency Shift Keying)** — FSK with Gaussian pulse shaping; the dominant
low-power modulation, used by BLE and most Sub-GHz radios (Ch 7).

**GMSK (Gaussian Minimum Shift Keying)** — Gaussian-filtered MSK; the modulation of GSM, at BT =
0.3 (Ch 7).

**Hidden-node problem** — Two transmitters out of range of each other both sense the channel clear
and collide at a receiver that hears both; a limit on CSMA (Ch 14).

**I/Q** — The in-phase and quadrature components into which any modulated signal decomposes; how
radios are actually built and how firmware manipulates signals (Ch 2, 6).

**Implementation loss** — The gap between a receiver's theoretical and actual sensitivity, normally
2–4 dB; more than 6 dB indicates a fault (Ch 8).

**Interference** — Energy from another transmitter; structured, bursty, and — unlike noise — not
reduced by narrowing bandwidth; the defining condition of unlicensed spectrum (Ch 3, 14).

**Interleaving** — Permuting bit order before transmission so that a channel burst becomes scattered
errors the FEC decoder can handle, at the cost of latency (Ch 9).

**IP3 (Third-Order Intercept)** — The figure of merit for front-end linearity; third-order
intermodulation products fall in-band and rise 3 dB per dB of input (Ch 3).

**ISI (Inter-Symbol Interference)** — Energy from one symbol spilling into the decision window of
the next; caused by band-limiting and multipath, corrected by equalisation (Ch 3, 7).

**Jitter** — Timing uncertainty in a recovered clock; decomposes into random and deterministic
components that point to different causes (Ch 10).

**LBT (Listen Before Talk)** — Sensing the channel before transmitting; a coexistence strategy and,
in the EU, a regulatory alternative to the duty-cycle limit (Ch 14, 15).

**Line coding** — The transformation of a bit stream before modulation to provide DC balance, clock
transitions, and spectral shaping (Ch 5).

**Link budget** — The dB accounting of every gain and loss between transmitter data and receiver
decision, yielding the link margin (Ch 13).

**LNA (Low-Noise Amplifier)** — The first amplifying stage of a receiver, which by the Friis
cascade sets the system noise figure; must come first and be quiet (Ch 3, 16).

**LoRa / CSS (Chirp Spread Spectrum)** — Spreading by a frequency chirp, trading airtime for
range; reaches −137 dBm sensitivity at SF12 (Ch 7).

**Manchester coding** — A line code with a guaranteed mid-bit transition; perfect DC balance and
clock recovery at the cost of doubling the signalling rate (Ch 5).

**Matched filter** — The receiver filter matched to the transmitted pulse, which maximises SNR at
the sampling instant; the optimum every good receiver approximates (Ch 8).

**MRC (Maximal-Ratio Combining)** — The optimal diversity-combining method, weighting each branch
by its SNR; achieves an output SNR equal to the sum of branch SNRs (Ch 8).

**Modulation index (h)** — 2f_d/R_s, the dimensionless dial that trades FSK bandwidth against
tolerance to frequency error; h = 0.5 is the orthogonality point (Ch 2).

**MSK (Minimum Shift Keying)** — Continuous-phase FSK at h = 0.5, simultaneously describable as FSK
and offset QPSK; minimum-bandwidth orthogonal FSK (Ch 7).

**Multipath** — Signal arriving by multiple paths that add as vectors, causing fading that varies
over a half-wavelength (Ch 12).

**Near-far problem** — A distant transmitter swamped by a nearby one; remedied by transmit power
control (Ch 14).

**Noise figure (NF)** — The SNR degradation a receiver adds, in dB; 0 dB is perfect, 4–10 dB is
typical for integrated transceivers (Ch 3).

**Noise floor** — The absolute power floor of a receiver, −174 + 10log(B) + NF dBm; the level below
which no receiver can operate (Ch 3).

**Noise temperature** — An alternative to noise figure, natural for cascade analysis and very
low-noise systems; T_e = T₀(F−1) (Ch 3).

**NRZ (Non-Return-to-Zero)** — The basic line code, one level per bit; unusable alone (no DC
balance, no clock) but universal when combined with scrambling (Ch 5).

**Nyquist limit** — The bound that a bandwidth B carries at most 2B symbols per second without ISI
(Ch 4).

**OFDM (Orthogonal Frequency-Division Multiplexing)** — Dividing data across many slow orthogonal
subcarriers so each fades flat; the wideband answer to frequency-selective fading, and wrong for
low-power Sub-GHz (Ch 7).

**OOK (On-Off Keying)** — Binary ASK, carrier on for a 1 and off for a 0; the cheapest modulation,
used in RFID and remotes (Ch 6).

**OQPSK (Offset QPSK)** — QPSK with the Q-channel delayed half a symbol, eliminating zero-crossing
transitions and the envelope problem at no energy cost (Ch 7).

**PAPR (Peak-to-Average Power Ratio)** — The ratio that sets how far a power amplifier must back
off; 0 dB for constant-envelope modulations, several dB for QAM (Ch 6, 16).

**Path-loss exponent (n)** — The rate at which power falls with distance, 2 in free space and up to
6 indoors; using the wrong value can overestimate range tenfold (Ch 12).

**PER (Packet Error Rate)** — The fraction of packets received with errors; PER ≈ N × BER, so long
packets need better SNR (Ch 8).

**Phase noise** — Short-term random oscillator frequency fluctuation; degrades adjacent-channel
rejection through reciprocal mixing (Ch 3, 16).

**Preamble** — The repeating pattern that opens a packet, giving the receiver a known signal for
AGC settling, timing, and frequency acquisition (Ch 18).

**Processing gain** — The SNR improvement from spreading, 10log(chip rate / bit rate); helps
against noise and uncorrelated interference but not matched interference (Ch 7).

**Pulse shaping** — Filtering the data before modulation so transitions are smooth, controlling
spectral splatter at the cost of ISI (Ch 7).

**Q function** — The Gaussian tail integral that governs all AWGN error probabilities; its
steepness is why BER curves have a waterfall shape (Ch 8).

**QAM (Quadrature Amplitude Modulation)** — Modulation varying amplitude and phase together for
high spectral efficiency; avoided in Sub-GHz because its costs attack a battery node's scarce
resources (Ch 6, 7).

**Rayleigh / Rician** — Fading distributions without / with a dominant path; Rician's K-factor
captures how benign the environment is (Ch 12).

**Reed–Solomon** — A symbol-based block code exceptionally good against burst errors; corrects any
16 bad bytes in RS(255,223) regardless of bit distribution (Ch 9).

**Reuse distance** — The minimum separation between co-channel transmitters for adequate SIR; the
basis of frequency planning (Ch 14).

**RSSI (Received Signal Strength Indicator)** — The receiver's report of received power; must track
actual power linearly or every adaptive algorithm above it fails silently (Ch 20, 23).

**Running disparity** — The cumulative excess of ones over zeros in a coded stream; bounding it is
what makes a code genuinely DC-balanced (Ch 5).

**Scrambling** — XORing data with a pseudo-random sequence to remove structure; achieves DC balance
and transitions statistically at near-zero overhead (Ch 5); called whitening in radio (Ch 9).

**Sensitivity** — The minimum received power for an acceptable error rate, equal to the noise floor
plus the required SNR; the headline receiver specification (Ch 3, 8).

**Shadowing** — Slow, log-normal variation of received power from large obstacles; demands a margin
of z·σ for a target reliability (Ch 12).

**Shannon capacity** — The maximum reliable bit rate, B·log₂(1+SNR); linear in bandwidth,
logarithmic in SNR, with an absolute floor of −1.59 dB Eb/N0 (Ch 4).

**Skin effect** — The crowding of high-frequency current into a conductor's surface, raising
resistance so copper loss rises with the square root of frequency (Ch 3, 10).

**SNR (Signal-to-Noise Ratio)** — The ratio of signal power to noise power; the everyday measure,
though Eb/N0 is fairer for comparing modulations (Ch 3).

**Soft decision** — A demodulator output that includes confidence, not just a bit; worth ~2 dB of
coding gain and must be preserved to the FEC decoder (Ch 9).

**Spectral efficiency** — Bits per second per hertz; the axis on which power-limited and
bandwidth-limited regimes divide (Ch 4).

**Spectral mask** — The regulatory template the transmitted spectrum must fit under; the reason
pulse shaping is mandatory (Ch 15).

**Spreading factor (SF)** — In LoRa, the number of chips per symbol (2^SF); higher SF buys
sensitivity and range at the cost of airtime (Ch 7).

**Sync word** — The known pattern marking the start of real data in a packet; provides frame
alignment and false-packet rejection (Ch 18).

**TDMA (Time-Division Multiple Access)** — Scheduled channel access in assigned time slots;
enables deep sleep and collision-free operation at the cost of clock synchronisation (Ch 19).

**TPC (Transmit Power Control)** — Adjusting each node's power so all arrive at the gateway at
comparable strength; mitigates the near-far problem and saves energy (Ch 13, 14).

**Two-ray model** — The ground-reflection propagation model giving a d⁴ path loss beyond the
breakpoint and revealing antenna height as a 6 dB-per-doubling lever (Ch 12).

**VSWR (Voltage Standing Wave Ratio)** — The measure of antenna match quality; a mismatch reflects
power back toward the amplifier (Ch 16).

**WDM (Wavelength-Division Multiplexing)** — Running many wavelengths down one fiber simultaneously;
the source of fiber's enormous aggregate capacity (Ch 11).

**Whitening** — Radio's name for scrambling; removes payload structure so the modulated signal is
well-behaved. Seed mismatch produces the classic sync-detects-but-CRC-fails signature (Ch 9).

---

## Appendix H — How to Keep Learning

This book builds an integrated, quantified understanding of the physical layer, but the field
moves and no single volume is complete. This closing appendix points toward how to deepen and
maintain the knowledge, organised by the kind of understanding sought.

**To deepen the theory**, the standard graduate texts on digital communications develop the
probability, detection theory, and information theory that this book uses but does not derive in
full. The material on the matched filter (Ch 8), the Q-function error analysis (Ch 8), and the
Shannon bound (Ch 4) is treated there with the rigour a specialist needs. A reader who wants to
*design* modulations rather than *choose* them should study detection and estimation theory
directly.

**To master the hardware**, the vendor datasheets and application notes for the transceiver
families mapped in Chapter 20 — the SX126x, CC13xx, EFR32, and nRF series — are the authoritative
source, and reading them against the model this book builds is the fastest way to make the model
concrete. The register maps, timing diagrams, and reference designs in those documents are where
the abstractions of Part V become specific silicon, and an engineer who can map a datasheet onto
Chapters 18–22 will come up to speed on a new part in days.

**To understand the standards**, the specifications themselves — the Bluetooth Core Specification,
IEEE 802.15.4 and 802.15.4g, the LoRaWAN regional parameters, and the FCC Part 15 and ETSI EN 300
220/328 regulatory documents — are the ground truth that no secondary source fully captures. They
are dense and precise, and the vocabulary this book establishes is largely what makes them
readable. When a design question turns on exactly what a standard requires, only the standard
answers it, and the numbers in this book's tables should always be verified against the current
version before they are relied upon.

**To build practical skill**, there is no substitute for the bench. Building a link, measuring its
sensitivity against the calculation (Chapter 23), deliberately breaking it and diagnosing the
break (Chapter 24), and watching the spectrum as parameters change teaches what no reading can. The
software-defined radio has made this accessible: an inexpensive SDR and open-source tools let an
engineer see the I/Q samples, the constellation, and the spectrum directly, turning the
abstractions of Chapters 2, 6, and 7 into things one can watch move. An afternoon spent watching a
GFSK signal's spectrum widen as the modulation index is raised teaches Section 2.6 more durably
than any table.

**To stay current**, the field's active frontiers — the evolution of low-power wide-area networks,
the ongoing refinement of BLE, the spread of ultra-wideband for ranging, and the continual
tightening of the gap to the Shannon limit through better codes — are followed through the
standards bodies, the vendor roadmaps, and the research literature. The fundamentals in this book
do not change: thermal noise is still −174 dBm/Hz, Shannon still bounds capacity, and the matched
filter is still optimal. What changes is which point in the space these fundamentals define a given
application chooses to occupy, and understanding the fundamentals is precisely what lets an
engineer evaluate each new development on its merits rather than its marketing.

The physical layer rewards the engineer who holds the whole chain in view — from the bit in the
buffer to the wave in the air and back — with the numbers attached at every stage. That capacity,
more than any specific fact, is what this book has aimed to build, and it is what turns the
physical layer from a source of mysterious failures into a system that can be reasoned about,
measured, and made to work.

---

*End of book.*
