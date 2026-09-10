# Contributing

This is a personal reference, but corrections are genuinely welcome — a
factual error here propagates into work, so finding one is a favour.

## The bar for a claim

Every statement in these documents should be traceable to one of:

1. **A primary source.** A reference manual, an IEC/IEEE standard, an ARM
   architecture document. Cite the document, the revision, and the section.
   Blog posts and Stack Overflow answers are leads, not sources.
2. **Something that was executed.** A program in `code/` that ran and produced
   the stated output.
3. **Something that was measured.** A logic analyzer or scope capture, with
   the board, clock and conditions recorded.

If a claim is none of the above, it is an inference and must be labelled as
one. The three verification labels used throughout are defined in the root
[README](README.md#what-verified-means-here); please keep them accurate rather
than optimistic.

## Reporting an error

Open an issue with:

- the file and section,
- what it says,
- what it should say,
- and the source that settles it.

Nine errors in a published DLMS specification were found this way, so no
correction is too small to be worth raising.

## Adding a peripheral document

Follow the eight-section template used by
[`peripherals/uart.md`](peripherals/uart.md). The order matters: the cheat
sheet is first because that is what you need at a bench, and the debugging
checklist is symptom-first because that is how you arrive at it — at 1am,
with a symptom and no theory.

## Adding code

- It must build with `-Wall -Wextra -Werror -Wconversion -Wshadow`. No exceptions.
- If it is hardware-independent logic, it goes in `code/portable/` and gets
  host tests in `code/host-tests/`.
- If it touches registers, keep that layer as thin as you can, so the logic
  above it stays testable.
- If it can run under QEMU, it goes in `code/qemu-cortex-m/` and must exit via
  `semihost_exit()` so `run-all.sh` can assert on it.
- State its verification level in the file header. "Compiles" and "runs" and
  "was measured on a board" are three different claims.

## Style

- Prose, not bullet fragments, where an idea needs a sentence.
- Say what fails and how it presents, not just what is correct. "The bus wedges
  and the analyzer shows SDA stuck low" is more useful than "always send STOP".
- No filler. If a section has nothing to say yet, mark it as not written rather
  than padding it.
