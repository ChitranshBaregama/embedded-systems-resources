# Practice

Problem sets for building and keeping embedded C fluency.

| | |
| :--- | :--- |
| [embedded-c-interview-core.md](embedded-c-interview-core.md) | 150 solved problems across 14 parts, each with a worked solution and the reasoning behind it |
| [embedded-c-1500-exercises.md](embedded-c-1500-exercises.md) | A five-tier drill bank, structured so each tier covers every topic area at one depth before the next tier revisits it deeper |

## How they fit together

They are not two versions of the same thing.

**The 1500 exercises build fluency.** Tier 1 is deliberately easy — the point
is speed and volume, not difficulty. Working a tier straight through, then
starting the next, gives spaced repetition across topics for free.

**The 150 core problems build depth.** These are the ones with a subtlety in
them: the ring buffer that has to be ISR-safe, the allocator with a real
fragmentation story, the parser that must not overrun. Each has a retention
table at the end of its part — the intended use is three passes, spaced, not
one heroic read.

## Related runnable code

Several core problems have working implementations in this repository:

- Ring buffer → [`code/portable/ringbuf/`](../code/portable/ringbuf/), with
  [tests](../code/host-tests/test_ringbuf.c) covering the index-overflow case
  at 2³² that almost nobody writes
- CRC-16 and protocol parser → [`code/portable/frame/`](../code/portable/frame/),
  with a 200k-byte fuzz pass under AddressSanitizer
- Interrupts and concurrency → [`code/qemu-cortex-m/`](../code/qemu-cortex-m/)
  examples 03 and 04

## Still to write

A DSA pattern set — the ~15 recurring patterns, worked, aimed at the
algorithmic round rather than the firmware round. See
[`../ROADMAP.md`](../ROADMAP.md).

## Scored preparation workflow

Use the [preparation hub](google-firmware/README.md), [question contracts](questions/README.md),
[150 separate interview prompts](interview-150/README.md), and [progress ledger](progress/README.md).
The solved core above is reference material: viewing it makes an attempt assisted.
Track 7- and 30-day retention explicitly; reading tiers in sequence is not proof of retention.
