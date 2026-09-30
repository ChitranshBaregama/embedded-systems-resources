# C Fundamentals — Bit Operations

## Problem
Implement functions/macros to set, clear, toggle and test bit `n` of a 32-bit unsigned value.

Do not use binary-string conversion. Handle bit positions 0..31.

### Interview follow-ups
- Why use `1u` rather than `1`?
- What is undefined/problematic about shifting by the type width?
- How would this change for a `volatile` memory-mapped register?


## Scored contract clarification

Use functions bool bit_set(uint32_t x, unsigned n, uint32_t *out), bit_clear, bit_toggle, and bit_test. bit_test takes bool *out; others take uint32_t *out. Return false for n >= 32 or NULL output and leave any non-NULL output unchanged. Use uint32_t arithmetic; assume the type exists, not that unsigned int is necessarily 32 bits. Do not access MMIO in this task.

Timebox: agree before starting (default 30 minutes coding or 20 minutes oral/design). Status: Unseen. Original source mapping is recorded in career/MIGRATION.md.
