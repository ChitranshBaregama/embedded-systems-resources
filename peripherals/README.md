# Peripherals

Serial buses, treated as engineering problems rather than API tours.

Each document follows the same eight-section shape so they can be used
interchangeably at a bench:

| Section | Purpose |
| :--- | :--- |
| 1. Cheat sheet | Everything you need in the first thirty seconds |
| 2. How it actually works | The mechanism, from the wire up |
| 3. Register-level walkthrough | What a driver writes, and in what order |
| 4. Code | Links to code in this repo, with its verification status stated |
| 5. Captures | Logic-analyzer traces — what to look at, and what failure looks like |
| 6. Debugging checklist | Symptom-first, so it is usable at 1am |
| 7. Questions I should be able to answer | Self-assessment, interview-shaped |
| 8. Sources | Primary documents only |

Then depth sections: electricals, timing, error recovery, driver architecture,
peripheral/target mode, board-level failure modes, comparison against the other
buses, and how to test a driver.

| | |
| :--- | :--- |
| [uart.md](uart.md) | Asynchronous serial — and the only bus here with no clock line, which is why baud accuracy matters so much |
| [i2c.md](i2c.md) | Two wires, many devices, open-drain. Simple to describe, the richest failure modes of the three |
| [spi.md](spi.md) | Fast, full-duplex, four modes, one chip select per device |

## Still to write

CAN and LIN (LIN is currently covered inside `uart.md` §17), USB device basics,
and 1-Wire. See [`../ROADMAP.md`](../ROADMAP.md).
