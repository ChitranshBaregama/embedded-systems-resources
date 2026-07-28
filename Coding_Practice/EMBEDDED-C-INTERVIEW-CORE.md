# Embedded C — Interview Core

> ~150 problems that actually get asked, every one solved. Curated down from two banks totalling 773.

**What was cut and why.** The source banks contain a lot of renames — `mask_n` and `mask_low` are one function asked twice, `ctz32`/`bsf`/`lsb_pos` are one operation under three names, and "swap nibbles" is not a question. Whole sections (14 code-review prompts, 25 design patterns) are discussion topics, not coding problems. What survives meets one of three tests:

1. It teaches a **reusable identity** you will apply in twenty other problems.
2. It exposes **undefined behaviour** that a compiler will silently exploit.
3. It is something an interviewer can **watch you get wrong** at a whiteboard.

**Reference platform:** STM32 / Cortex-M, bare metal, C11, `<stdint.h>` types. No dynamic allocation unless the problem is about allocation.

---

## How to use this

Solutions sit behind `Solution` toggles. Read the prototype, write it yourself, then open. Reading a solution feels like learning and is not.

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -Wconversion \
    -fsanitize=address,undefined problems.c -o problems
```

`-Wconversion` earns its place here: most bugs in this subject matter are silent narrowing and integer-promotion bugs, and it is the only flag that catches them.

`BEG` fundamentals · `INT` day-to-day firmware · `SEN` 3–7 year interview grade · `STA` concurrency and systems

---

## Index

| # | Part | Kept | From | Status |
|---|---|---|---|---|
| 01 | [Bit Manipulation](#part-01--bit-manipulation) | 20 | 70 | ✅ |
| 02 | [Register I/O](#part-02--register-io) | 12 | 30 | ✅ |
| 03 | [Memory & String Functions](#part-03--memory-and-string-functions) | 14 | 45 | ✅ |
| 04 | [Allocators](#part-04--allocators) | 8 | 35 | ✅ |
| 05 | [Data Structures](#part-05--data-structures) | 20 | 75 | ✅ |
| 06 | [CRC & Checksums](#part-06--crc-and-checksums) | 7 | 35 | ✅ |
| 07 | [Interrupts & Concurrency](#part-07--interrupts-and-concurrency) | 12 | 30 | ✅ |
| 08 | [RTOS Primitives](#part-08--rtos-primitives) | 10 | 50 | ✅ |
| 09 | [Drivers](#part-09--drivers) | 14 | 83 | ✅ |
| 10 | [Protocol Parsers](#part-10--protocol-parsers) | 9 | 50 | ✅ |
| 11 | [State Machines](#part-11--state-machines) | 6 | 40 | ✅ |
| 12 | [Embedded Algorithms](#part-12--embedded-algorithms) | 10 | 45 | ✅ |
| 13 | [Debugging — fix the bug](#part-13--debugging-fix-the-bug) | 10 | 26 | ✅ |
| 14 | [Review, Patterns, Optimization](#part-14--review-patterns-optimization) | 8 | 59 | ✅ |

**150 kept from 773.**

---

## Shared helpers

Assumed in scope throughout.

```c
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Mask of the n lowest bits. n == 32 must not reach (1u << 32) — that is UB. */
static inline uint32_t mask_n(uint8_t n)
{
    return (n >= 32u) ? 0xFFFFFFFFu : ((1UL << n) - 1UL);
}
```

---

# Part 01 — Bit Manipulation

20 entries, merged where the bank asked one thing four times. This part is the price of entry: if any of it needs thought, nothing later will land.

> [!IMPORTANT]
> **The four traps that govern every solution below.**
> 1. **Shift on unsigned.** `1 << 31` is UB on 32-bit `int`. Write `1UL << 31`.
> 2. **A shift count ≥ width is UB**, not zero. Arm's barrel shifter returns the operand unshifted for a count of 32; x86 masks the count to 5 bits. Same source, different answers.
> 3. **Mask the value, not just the field.** An oversized argument must not reach the neighbouring field.
> 4. **Integer promotion undoes narrow types.** `uint8_t << 4` becomes `int`; cast back or `-Wconversion` will tell you.

---

### 1. Bit primitives — set, clear, toggle, test `BEG`

```c
uint32_t set_bit(uint32_t x, uint8_t n);
uint32_t clear_bit(uint32_t x, uint8_t n);
uint32_t toggle_bit(uint32_t x, uint8_t n);
bool     bit_is_set(uint32_t x, uint8_t n);
```

<details><summary>Solution</summary>

```c
uint32_t set_bit(uint32_t x, uint8_t n)    { return x |  (1UL << n); }
uint32_t clear_bit(uint32_t x, uint8_t n)  { return x & ~(1UL << n); }
uint32_t toggle_bit(uint32_t x, uint8_t n) { return x ^  (1UL << n); }

bool bit_is_set(uint32_t x, uint8_t n)
{
    return ((x >> n) & 1UL) != 0UL;
}
```

Four one-liners, two things worth marks.

**`1UL`, not `1`.** On a target with 32-bit `int`, `1 << 31` shifts into the sign bit — undefined behaviour. This is the single most common defect in real register code.

**Shift down then mask for the test.** `return x & (1UL << n);` happens to work because `_Bool` normalises to 0/1, but change the return type to `int` and it returns `1 << n`, so a caller writing `if (f(x,5) == 1)` fails. For `clear_bit`, always build the mask then invert — never try to construct the inverted mask directly.
</details>

---

### 2. Force a bit to a value, branchless `BEG`

```c
uint32_t write_bit(uint32_t x, uint8_t n, bool v);
```
No `if`/`else`, constant time.

<details><summary>Solution</summary>

```c
uint32_t write_bit(uint32_t x, uint8_t n, bool v)
{
    return (x & ~(1UL << n)) | ((uint32_t)v << n);
}
```

Clear then OR. A `bool` is guaranteed 0 or 1, which is what makes the shift produce either nothing or exactly the bit. The cast is required — `bool` promotes to `int`, and a shift of 31 on `int` is trap 1 again.

If the flag arrives as a non-normalised `int` (say `x & 0x40`), normalise with `!!v` first or the shift injects bits in the wrong place.

The mask-based form generalises to multi-bit fields:

```c
uint32_t m = 1UL << n;
return (x & ~m) | ((uint32_t)(-(int32_t)v) & m);   /* -1 → all ones, 0 → all zeros */
```
</details>

---

### 3. Mask of N bits — and the shift-by-32 trap `BEG`

```c
uint32_t mask_n(uint8_t n);
```
`n` lowest bits set. Must handle `n == 32`.

<details><summary>Solution</summary>

```c
uint32_t mask_n(uint8_t n)
{
    return (n >= 32u) ? 0xFFFFFFFFUL : ((1UL << n) - 1UL);
}
```

`(1UL << 32)` is **undefined behaviour, not zero**. The C standard makes a shift count greater than or equal to the operand width undefined, and the hardware disagrees across targets: Arm's barrel shifter returns the operand unshifted, x86 masks the count to five bits. Identical source, different results, no warning.

This function is the dependency of entries 4, 5, 6 and half of Part 02. Build it once and stop rewriting the shift.

Branchless alternative if the ternary is unwelcome: `~0UL >> (31u - (n & 31u))` for `n >= 1`, still needing a zero case. The ternary compiles to a conditional move and is clearer.
</details>

---

### 4. Insert a bit field `INT`

```c
uint32_t insert_field(uint32_t reg, uint32_t val, uint8_t pos, uint8_t width);
```
Clear the field first. Handle `width == 32`. Mask `val` to width.

<details><summary>Solution</summary>

```c
uint32_t insert_field(uint32_t reg, uint32_t val, uint8_t pos, uint8_t width)
{
    if (width == 0u || (uint16_t)pos + (uint16_t)width > 32u) {
        return reg;
    }
    uint32_t m = mask_n(width) << pos;
    return (reg & ~m) | ((val << pos) & m);
}
```

**The most important routine in this part** — it is the read-modify-write behind every register field write in every driver you will ever write.

Three things earn the marks:

- `mask_n` covers `width == 32`.
- **`& m` after shifting masks the value too.** Omit it and an oversized `val` silently corrupts the neighbouring field. That is *the* peripheral-configuration bug: the UART works, and the timer three fields over does not.
- The bounds check promotes to `uint16_t` so `pos + width` cannot itself wrap in `uint8_t`.
</details>

---

### 5. Extract a bit field `INT`

```c
uint32_t extract_field(uint32_t reg, uint8_t pos, uint8_t width);
```

<details><summary>Solution</summary>

```c
uint32_t extract_field(uint32_t reg, uint8_t pos, uint8_t width)
{
    if (width == 0u || (uint16_t)pos + (uint16_t)width > 32u) {
        return 0UL;
    }
    return (reg >> pos) & mask_n(width);
}
```

Shift down, then mask. Mask-then-shift also works but needs the mask pre-shifted, so it is one more thing to get wrong under review.

Pair with entry 4 and every register access in the codebase goes through two reviewed functions instead of two hundred hand-written shift expressions. That argument, made unprompted, is what distinguishes a candidate who has maintained firmware from one who has only written it.
</details>

---

### 6. Extract a **signed** field `SEN`

```c
int32_t extract_signed(uint32_t r, uint8_t pos, uint8_t w);
```
Sign-extend the result.

<details><summary>Solution</summary>

```c
int32_t extract_signed(uint32_t r, uint8_t pos, uint8_t w)
{
    if (w == 0u || (uint16_t)pos + (uint16_t)w > 32u) return 0;

    uint32_t f = (r >> pos) & mask_n(w);

    if (w < 32u && (f & (1UL << (w - 1u))) != 0UL) {
        f |= ~mask_n(w);                  /* replicate the sign bit upward */
    }
    return (int32_t)f;
}
```

Extract unsigned, then if the field's top bit is set, fill everything above it with ones. That is sign extension by definition.

**Why this is a real bug and not an exercise:** a 12-bit accelerometer reading of `0xFFF` is −1, not 4095. Read it unsigned and gravity points the wrong way. Every signed sensor field — accelerometer, gyro, thermocouple ADC, any CAN signal with a signed encoding — needs this, and the failure is a plausible-looking number rather than a crash.

Two subtleties to name: converting an out-of-range `uint32_t` to `int32_t` is implementation-defined by the standard, though every two's-complement target does the obvious thing; and the `w < 32` guard prevents a shift by 32 when the field is the whole word.

The alternative some codebases prefer, letting the compiler do it:

```c
struct { int32_t v : 12; } s;   /* a signed bit-field sign-extends on read */
```
</details>

---

### 7. Population count `BEG`

```c
uint8_t popcount32(uint32_t x);
```
No intrinsics. Should work on an 8-bit MCU.

<details><summary>Solution</summary>

```c
/* Kernighan: one iteration per SET bit, not per bit. */
uint8_t popcount32(uint32_t x)
{
    uint8_t c = 0u;
    while (x != 0UL) {
        x &= (x - 1UL);          /* clears the lowest set bit */
        c++;
    }
    return c;
}
```

**`x & (x - 1)` clearing the lowest set bit is the most reusable identity in this part.** Borrowing flips the lowest 1 to 0 and sets all the zeros below it; the AND keeps only the untouched high bits. It reappears in entries 10 and 15.

The SWAR version is O(1) but multiplies, so it is the wrong answer on an 8-bit target and the right one on Cortex-M:

```c
uint8_t popcount32_swar(uint32_t x)
{
    x = x - ((x >> 1) & 0x55555555UL);
    x = (x & 0x33333333UL) + ((x >> 2) & 0x33333333UL);
    x = (x + (x >> 4)) & 0x0F0F0F0FUL;
    return (uint8_t)((x * 0x01010101UL) >> 24);
}
```

Give both and say which you would ship for which core. That comparison is the actual question.
</details>

---

### 8. Count trailing zeros `INT`

```c
uint8_t ctz32(uint32_t x);
```
Return 32 if `x == 0`.

<details><summary>Solution</summary>

```c
uint8_t ctz32(uint32_t x)
{
    if (x == 0UL) return 32u;

    uint8_t n = 0u;
    if ((x & 0x0000FFFFUL) == 0UL) { n += 16u; x >>= 16; }
    if ((x & 0x000000FFUL) == 0UL) { n += 8u;  x >>= 8;  }
    if ((x & 0x0000000FUL) == 0UL) { n += 4u;  x >>= 4;  }
    if ((x & 0x00000003UL) == 0UL) { n += 2u;  x >>= 2;  }
    if ((x & 0x00000001UL) == 0UL) { n += 1u;         }
    return n;
}
```

Binary search: five fixed steps instead of up to 32 iterations, and no data-dependent loop count — which matters when this sits in an ISR with a latency budget.

The `x == 0` guard is the whole test. Without it the loop version returns 32 by luck and the search version returns 31.

`ctz`, `bsf` and "position of lowest set bit" are three names for this one operation; the bank asks it three times. x86 has `BSF`; Arm has no direct equivalent and uses `RBIT` then `CLZ`.
</details>

---

### 9. Count leading zeros `INT`

```c
uint8_t clz32(uint32_t x);
```

<details><summary>Solution</summary>

```c
uint8_t clz32(uint32_t x)
{
    if (x == 0UL) return 32u;

    uint8_t n = 0u;
    if ((x & 0xFFFF0000UL) == 0UL) { n += 16u; x <<= 16; }
    if ((x & 0xFF000000UL) == 0UL) { n += 8u;  x <<= 8;  }
    if ((x & 0xF0000000UL) == 0UL) { n += 4u;  x <<= 4;  }
    if ((x & 0xC0000000UL) == 0UL) { n += 2u;  x <<= 2;  }
    if ((x & 0x80000000UL) == 0UL) { n += 1u;         }
    return n;
}
```

Mirror of entry 8 — shift the other way, test from the top.

Two derived functions worth having ready, because both come up:

```c
int8_t  msb_pos(uint32_t x) { return (x == 0UL) ? -1 : (int8_t)(31u - clz32(x)); }
uint8_t clo32(uint32_t x)   { return clz32(~x); }    /* leading ONES */
```

`msb_pos` is `floor(log2(x))` — used by allocators sizing a buddy block and by `next_pow2`. Never reach for `log2()`: it drags in libm, costs microseconds, and rounds wrong at exact powers of two. `clo32` gives you CIDR prefix length from a netmask.

Cortex-M3 and above have a one-cycle `CLZ`. Write the manual version, then name the instruction.
</details>

---

### 10. Lowest-set-bit idioms and the iteration loop `BEG`

```c
uint32_t isolate_lsb(uint32_t x);   /* keep only the lowest set bit */
uint32_t clear_lsb(uint32_t x);    /* clear only the lowest set bit */
```

<details><summary>Solution</summary>

```c
uint32_t isolate_lsb(uint32_t x) { return x & (~x + 1UL); }   /* == x & -x */
uint32_t clear_lsb(uint32_t x)   { return x & (x - 1UL);  }
```

Two's-complement negation flips every bit above the lowest set bit and leaves it and the zeros below unchanged — so the AND keeps exactly one bit. `0b10110` → `0b00010`.

`x & -x` is identical and more idiomatic, but MISRA C prohibits unary minus on unsigned operands, so firmware codebases write `~x + 1`. Knowing *why* the codebase looks like that is worth a mark.

**What they combine into** is the thing to actually memorise — the standard walk over set bits, which is how you service a pending-interrupt mask or a free-block bitmap:

```c
while (mask != 0UL) {
    uint32_t bit = mask & (~mask + 1UL);
    handle(ctz32(bit));
    mask &= (mask - 1UL);          /* drop it and continue */
}
```

Cost is proportional to the number of set bits, not the word width.
</details>

---

### 11. Reverse the bits in a word `INT`

```c
uint32_t reverse_bits32(uint32_t x);
uint8_t  reverse_byte(uint8_t x);
```

<details><summary>Solution</summary>

```c
uint32_t reverse_bits32(uint32_t x)
{
    x = ((x & 0x55555555UL) << 1) | ((x >> 1) & 0x55555555UL);   /* bits    */
    x = ((x & 0x33333333UL) << 2) | ((x >> 2) & 0x33333333UL);   /* pairs   */
    x = ((x & 0x0F0F0F0FUL) << 4) | ((x >> 4) & 0x0F0F0F0FUL);   /* nibbles */
    x = ((x & 0x00FF00FFUL) << 8) | ((x >> 8) & 0x00FF00FFUL);   /* bytes   */
    return (x << 16) | (x >> 16);                                /* halves  */
}

uint8_t reverse_byte(uint8_t x)
{
    x = (uint8_t)(((x & 0xF0u) >> 4) | ((x & 0x0Fu) << 4));
    x = (uint8_t)(((x & 0xCCu) >> 2) | ((x & 0x33u) << 2));
    x = (uint8_t)(((x & 0xAAu) >> 1) | ((x & 0x55u) << 1));
    return x;
}
```

Divide and conquer: swap halves, then quarters, then neighbours. log2(32) = five steps, all branch-free. The last line of the 32-bit version needs no mask because the two halves cannot collide.

**Why you need it in practice, not just in interviews:** SD cards and many SPI displays clock LSB-first while the MCU's SPI peripheral is MSB-first. Reversing each byte of a buffer via a 256-byte lookup table (`buf[i] = rev8_lut[buf[i]]`) is one load per byte and the standard fix.

Careful with the distinction: reversing each byte in place leaves byte order alone; reversing a whole *buffer* bit-wise means reversing byte order **and** each byte's bits. Doing only one of the two is the common wrong answer.

Cortex-M3+ has `RBIT` — one instruction. Mention after writing the portable version.
</details>

---

### 12. Byte swap / endian conversion `BEG`

```c
uint32_t bswap32(uint32_t x);
uint16_t bswap16(uint16_t x);
```

<details><summary>Solution</summary>

```c
uint32_t bswap32(uint32_t x)
{
    return ((x & 0x000000FFUL) << 24) |
           ((x & 0x0000FF00UL) <<  8) |
           ((x & 0x00FF0000UL) >>  8) |
           ((x & 0xFF000000UL) >> 24);
}

uint16_t bswap16(uint16_t x)
{
    return (uint16_t)((x >> 8) | (uint16_t)(x << 8));
}
```

Mask first, then shift. The 16-bit cast is not optional: `x << 8` on a `uint16_t` promotes to `int`, and on a 16-bit-`int` target a set bit 15 shifts into the sign bit.

**The point that matters more than the function:** never `memcpy` a struct onto the wire and byte-swap it. Serialise field by field with shifts (entry 13) and the code is endian-independent *by construction* — it never asks how the CPU stores anything. Also note `bswap32` on an array assumes 4-byte alignment; a `uint8_t *` packet buffer cast to `uint32_t *` faults on Cortex-M0/M0+.
</details>

---

### 13. Pack and unpack big-endian — safe serialisation `BEG`

```c
uint32_t pack_be(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
void     unpack_be(uint32_t v, uint8_t out[4]);
```

<details><summary>Solution</summary>

```c
uint32_t pack_be(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    return ((uint32_t)a << 24) |
           ((uint32_t)b << 16) |
           ((uint32_t)c <<  8) |
            (uint32_t)d;
}

void unpack_be(uint32_t v, uint8_t out[4])
{
    if (out == NULL) return;
    out[0] = (uint8_t)(v >> 24);
    out[1] = (uint8_t)(v >> 16);
    out[2] = (uint8_t)(v >>  8);
    out[3] = (uint8_t)v;
}
```

**Every cast in `pack_be` is load-bearing.** `a << 24` on a `uint8_t` promotes to `int`, and if bit 7 of `a` is set the result lands on the sign bit — UB. Cast to `uint32_t` *before* shifting, not after.

This is how you correctly deserialise a wire protocol: same value on a big-endian and a little-endian CPU, no alignment requirement, no compiler-dependent struct layout. `*(uint32_t *)buf` gives a different answer per target *and* faults on Cortex-M0.

`uint8_t out[4]` in a parameter list is just `uint8_t *out` — the 4 is documentation the compiler discards, so the null check is the only real protection and the caller still owns the size guarantee. No `& 0xFF` is needed on unpack; the cast truncates, which is defined for unsigned types.
</details>

---

### 14. Parity `BEG`

```c
bool parity32(uint32_t x);
```
Branchless.

<details><summary>Solution</summary>

```c
bool parity32(uint32_t x)
{
    x ^= x >> 16;
    x ^= x >> 8;
    x ^= x >> 4;
    x ^= x >> 2;
    x ^= x >> 1;
    return (x & 1UL) != 0UL;
}
```

XOR-folding halves the width each step until bit 0 holds the XOR of all 32 bits, which *is* the parity. Upper garbage is harmless because only bit 0 is read. Five operations, no loop, no table.

Where it lives: UART parity generation, and cheap corruption detection on a link too slow to afford a CRC. Its weakness is why CRCs exist — **parity misses every even-numbered bit error**, so two flipped bits look clean. Being able to state that limitation is the follow-up question.

Related and free: `hamming_distance(a,b) == popcount32(a ^ b)`. Useful for accepting a radio preamble within 2 bits of the expected pattern instead of demanding an exact match — often the difference between a link that syncs and one that does not.
</details>

---

### 15. Powers of two `BEG`→`INT`

```c
bool     is_pow2(uint32_t x);
uint32_t next_pow2(uint32_t x);     /* smallest power of two >= x, 1 for x==0 */
```

<details><summary>Solution</summary>

```c
bool is_pow2(uint32_t x)
{
    return (x != 0UL) && ((x & (x - 1UL)) == 0UL);
}

uint32_t next_pow2(uint32_t x)
{
    if (x <= 1UL) return 1UL;
    x--;                        /* so an exact power of two maps to itself */
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1UL;
}
```

A power of two has exactly one set bit, so `x - 1` flips it and sets everything below — entry 7's identity again.

**The `x != 0` guard is the entire question.** Without it `is_pow2(0)` returns true, and an allocator that trusts it computes an alignment mask of `0 - 1 == 0xFFFFFFFF`.

For `next_pow2`, the OR-shift cascade smears the highest set bit down over every lower position to give `2^k - 1`; adding one lands on `2^k`. The leading `x--` is what makes `next_pow2(8) == 8` rather than 16. Inputs above `0x80000000` overflow to 0 — document it or return 0 deliberately.
</details>

---

### 16. Alignment `INT`

```c
uintptr_t align_up(uintptr_t p, size_t a);
uintptr_t align_down(uintptr_t p, size_t a);
bool      is_aligned(uintptr_t p, size_t a);
```
`a` is a power of two. No division.

<details><summary>Solution</summary>

```c
uintptr_t align_up(uintptr_t p, size_t a)
{
    return (p + (uintptr_t)a - 1u) & ~((uintptr_t)a - 1u);
}

uintptr_t align_down(uintptr_t p, size_t a)
{
    return p & ~((uintptr_t)a - 1u);
}

bool is_aligned(uintptr_t p, size_t a)
{
    return (p & ((uintptr_t)a - 1u)) == 0u;
}
```

Add `a-1` to push past the boundary, then clear the low bits to land on it. For `a == 8` the mask is `~7`. `align_down` cannot overflow; **`align_up` can** — a `p` within `a-1` of `UINTPTR_MAX` wraps to a small value, which in an allocator is an exploitable bug. Real code checks `p <= UINTPTR_MAX - (a-1)` first and asserts `is_pow2(a)` in debug builds.

Not academic: an unaligned 32-bit access **faults** on Cortex-M0/M0+ and on M3/M4 with the trap enabled. `align_down` is how you find the start of the cache line, flash page, or MPU region containing an address, and `is_aligned` is what a fast `memcpy` tests to decide whether its word loop is legal.
</details>

---

### 17. Rotate `BEG`

```c
uint32_t rotl32(uint32_t x, uint8_t n);
uint32_t rotr32(uint32_t x, uint8_t n);
```
Handle `n == 0` and `n >= 32` without UB.

<details><summary>Solution</summary>

```c
uint32_t rotl32(uint32_t x, uint8_t n)
{
    n &= 31u;
    return (x << n) | (x >> ((32u - n) & 31u));
}

uint32_t rotr32(uint32_t x, uint8_t n)
{
    n &= 31u;
    return (x >> n) | (x << ((32u - n) & 31u));
}
```

The naive `(x << n) | (x >> (32 - n))` is **UB at `n == 0`**, because it becomes `x >> 32`. Masking the second shift with `& 31` turns that into `x >> 0`, and `x | x == x` — the correct answer for a zero rotate. The leading `n &= 31` handles `n >= 32` by taking the rotation modulo the width, which is what a rotate means.

GCC recognises this exact shape and emits a single `ROR`. Deviate and you get five instructions. Cortex-M has `ROR` but no `ROL` — a left rotate is assembled as `ROR #(32-n)`, which is literally this expression.
</details>

---

### 18. Gray code, both directions `INT`

```c
uint32_t bin_to_gray(uint32_t x);
uint32_t gray_to_bin(uint32_t g);
```

<details><summary>Solution</summary>

```c
uint32_t bin_to_gray(uint32_t x)
{
    return x ^ (x >> 1);
}

uint32_t gray_to_bin(uint32_t g)
{
    g ^= g >> 16;
    g ^= g >> 8;
    g ^= g >> 4;
    g ^= g >> 2;
    g ^= g >> 1;
    return g;
}
```

Encoding is one line. Decoding is a prefix-XOR — each binary bit is the XOR of all Gray bits at or above it — which is the same fold as `parity32` but keeping every intermediate bit instead of only bit 0. Five steps, not a 32-iteration loop.

**Why it exists:** successive Gray values differ in exactly one bit, so sampling a rotary encoder or absolute position sensor mid-transition can only ever return one of the two adjacent codes. Binary 3→4 changes three bits at once, and a mid-transition sample can read anything from 0 to 7 — a position glitch that looks like a mechanical fault.
</details>

---

### 19. Branchless min/max — and why the classic answer is wrong `INT`

```c
int32_t min_bl(int32_t a, int32_t b);
int32_t max_bl(int32_t a, int32_t b);
```

<details><summary>Solution</summary>

```c
int32_t min_bl(int32_t a, int32_t b) { return (a < b) ? a : b; }
int32_t max_bl(int32_t a, int32_t b) { return (a > b) ? a : b; }
```

The expected trick is `b + ((a - b) & ((a - b) >> 31))`. **It is broken for signed inputs.** `a - b` overflows whenever the operands straddle zero widely — `a = INT32_MAX, b = -1` — and signed overflow is undefined behaviour, not wraparound.

The ternary is the correct answer. On Cortex-M it compiles to `CMP` plus a conditional move: genuinely branchless, with no UB. **Recognising that the clever version is broken is the entire reason this is tagged above beginner.**

If branchless masking is genuinely required, do it in unsigned space where wraparound is defined:

```c
uint32_t ua = (uint32_t)a ^ 0x80000000UL;   /* order-preserving bias */
uint32_t ub = (uint32_t)b ^ 0x80000000UL;
uint32_t d  = ua - ub;
uint32_t m  = (uint32_t)0 - (d >> 31);
return (int32_t)(((ua & ~m) | (ub & m)) ^ 0x80000000UL);
```

Free companion, and the correct shape for a `qsort` comparator:

```c
int32_t sign(int32_t x) { return (x > 0) - (x < 0); }   /* never overflows */
```

`a - b` as a comparator overflows for large-magnitude operands of opposite sign. `(a > b) - (a < b)` does not.
</details>

---

### 20. Overflow detection, signed and unsigned `SEN`

```c
bool add_overflows_s32(int32_t a, int32_t b);
bool sub_overflows_s32(int32_t a, int32_t b);
bool add_overflows_u32(uint32_t a, uint32_t b);
```

<details><summary>Solution</summary>

```c
bool add_overflows_s32(int32_t a, int32_t b)
{
    if (b > 0 && a > INT32_MAX - b) return true;
    if (b < 0 && a < INT32_MIN - b) return true;
    return false;
}

bool sub_overflows_s32(int32_t a, int32_t b)
{
    if (b < 0 && a > INT32_MAX + b) return true;
    if (b > 0 && a < INT32_MIN + b) return true;
    return false;
}

bool add_overflows_u32(uint32_t a, uint32_t b)
{
    return (uint32_t)(a + b) < a;
}
```

**The signed and unsigned cases are fundamentally different problems, and that asymmetry is the question.**

Unsigned arithmetic is *defined* to wrap modulo 2³², so you may perform the addition and inspect the result — one line. Signed overflow is undefined behaviour, so you must rearrange the inequality to keep the arithmetic in range: test `a > MAX - b` instead of `a + b > MAX`. `MAX - b` cannot overflow when `b > 0`, and `MIN - b` cannot when `b < 0`.

**GCC at `-O2` will delete `if (a + b < a)` on signed operands entirely**, on the reasoning that signed overflow cannot happen, so the check is dead code. This is not theoretical — it has produced shipped security bugs.

Note you cannot implement subtraction as `add_overflows_s32(a, -b)`: negating `INT32_MIN` is itself UB.

Production answer: `__builtin_add_overflow(a, b, &r)`, which compiles to an add plus a carry-flag branch. Write the portable version first, then name the builtin.

The same "unsigned wrap is defined" property is what makes ring-buffer index arithmetic and `(now - then) < timeout` timer comparisons work correctly across a counter rollover — see Part 02 entry 10.
</details>

---

## Part 01 — retention table

Cover the right column and reproduce each from memory. Anything that takes more than a second is the one to redo.

| Identity | Effect |
|---|---|
| `x & (x - 1)` | clear lowest set bit → popcount, is_pow2 |
| `x & (~x + 1)` | isolate lowest set bit → bit iteration |
| `x ^ (x >> 1)` | binary → Gray |
| `x ^= x >> k` fold | parity, Gray → binary |
| <code>x &#124;= x >> 1,2,4,8,16</code> | smear high bit down → next_pow2 |
| `(p + a - 1) & ~(a - 1)` | align up |
| <code>(v & ~m) &#124; (val & m)</code> | field insert — **mask the value too** |
| `(a > b) - (a < b)` | sign, safe comparator |
| `f \|= ~mask_n(w)` | sign-extend a narrow field |
| `(a + b) < a` | unsigned carry — **unsigned only** |

---

# Part 02 — Register I/O

12 entries from 30. This is where bit manipulation meets hardware, and where `volatile` stops being a trivia answer.

> [!IMPORTANT]
> **What `volatile` does and does not do.** It tells the compiler this memory can change without it seeing the write, so every access in the source must appear in the object code, in order, un-cached and un-elided.
>
> It does **not** provide atomicity, and it is **not** a memory barrier. It constrains the compiler, not the processor's store buffer or an interrupt landing mid-sequence. Treating it as a concurrency primitive is the most common misunderstanding in embedded C.

---

### 1. Read and write a hardware register `BEG`

```c
uint32_t reg_read(uintptr_t addr);
void     reg_write(uintptr_t addr, uint32_t val);
```

<details><summary>Solution</summary>

```c
uint32_t reg_read(uintptr_t addr)
{
    return *(volatile uint32_t *)addr;
}

void reg_write(uintptr_t addr, uint32_t val)
{
    *(volatile uint32_t *)addr = val;
}
```

`volatile` inside the cast is the whole answer. Drop it and the compiler is free to hoist the read out of a polling loop, cache it in a register, merge two writes into one, or delete a write whose value it thinks is already there. All four are legal optimisations on non-volatile memory and all four break hardware.

**Why a read can matter as much as a write:** many status registers are read-to-clear. A `volatile` read that the compiler removes because the value is unused fails to clear the flag, and the peripheral re-asserts its interrupt forever.

The usual production form defines the register once rather than casting at each call site:

```c
#define UART1_SR   (*(volatile uint32_t *)0x40011000UL)

typedef struct {              /* better: a peripheral block */
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
} uart_t;
#define UART1  ((uart_t *)0x40011000UL)
```

Note `volatile` goes on each member, not on the struct pointer.
</details>

---

### 2. Modify a register field `INT`

```c
void reg_field_write(volatile uint32_t *r, uint8_t pos, uint8_t width, uint32_t val);
```

<details><summary>Solution</summary>

```c
void reg_field_write(volatile uint32_t *r, uint8_t pos, uint8_t width, uint32_t val)
{
    if (r == NULL || width == 0u || (uint16_t)pos + (uint16_t)width > 32u) return;

    uint32_t m = mask_n(width) << pos;
    uint32_t v = *r;                       /* ONE read  */
    v = (v & ~m) | ((val << pos) & m);
    *r = v;                                /* ONE write */
}
```

Part 01 entry 4 with a `volatile` pointer, and the read/write discipline is now the point. Compute into a local, then commit once.

**Why one write, not several.** Configuring five fields with five separate RMW cycles means the register passes through four intermediate states that the hardware *acts on*: a clock divider briefly wrong, a UART momentarily at the wrong baud, a PWM output glitching. Batch them:

```c
struct field { uint32_t mask; uint32_t value; };

void set_fields(volatile uint32_t *r, const struct field *f, size_t n)
{
    if (r == NULL || f == NULL) return;
    uint32_t v = *r;
    for (size_t i = 0u; i < n; i++) {
        v = (v & ~f[i].mask) | (f[i].value & f[i].mask);
    }
    *r = v;
}
```

**This is still not atomic with respect to interrupts.** If an ISR touches the same register the sequence needs a critical section or the CAS loop in entry 6. Saying that unprompted is what makes this a senior answer.
</details>

---

### 3. Poll a register bit with timeout `INT`

```c
int reg_wait_bit(volatile uint32_t *r, uint32_t mask, bool set, uint32_t timeout_us);
```
Return 0 on success, −1 on timeout.

<details><summary>Solution</summary>

```c
int reg_wait_bit(volatile uint32_t *r, uint32_t mask, bool set, uint32_t timeout_us)
{
    if (r == NULL) return -1;

    uint32_t start = micros();                  /* free-running monotonic counter */

    for (;;) {
        uint32_t v = *r & mask;                 /* single read per iteration */
        bool done = set ? (v == mask) : (v == 0UL);
        if (done) return 0;

        if ((uint32_t)(micros() - start) >= timeout_us) {
            return -1;
        }
    }
}
```

**The timeout is the entire question**, and it is the single most valuable habit in driver code. `while (!(I2C1->SR & FLAG));` with no exit is why a product ships and then hangs in the field once a year when a sensor browns out — and a watchdog reset is the *symptom*, not the diagnosis.

Two details that get checked:

- **`(uint32_t)(now - start) >= timeout` is wrap-safe.** Unsigned subtraction is defined modulo 2³², so it stays correct across a counter rollover. `now >= start + timeout` is **not** — it breaks exactly once per wrap, which on a 32-bit microsecond counter is every 71 minutes.
- **`v == mask`, not `v != 0`.** For a multi-bit mask, waiting until *all* the bits are set is usually what is meant; `!= 0` returns on the first one. Ambiguous in the prototype, so state your reading.

For waits longer than a few hundred microseconds, yield or sleep instead of spinning — a busy loop at full clock burns power and, in an RTOS, starves lower-priority tasks.
</details>

---

### 4. Clear a write-1-to-clear flag `BEG`

```c
void w1c_clear(volatile uint32_t *sr, uint32_t mask);
```

<details><summary>Solution</summary>

```c
void w1c_clear(volatile uint32_t *sr, uint32_t mask)
{
    if (sr == NULL) return;
    *sr = mask;                 /* plain write. NOT |= */
}
```

**`*sr |= mask` is the bug**, and it is the most common register mistake after missing `volatile`.

Read-modify-write on a W1C register reads *every currently pending flag*, ORs in your bit, and writes the whole word back — which clears every other pending flag too. The interrupt you were not handling yet silently disappears. Symptom: lost bytes under load, or an event that "sometimes doesn't fire", appearing only when two interrupts are close together.

Write only the bits you intend to clear. No read at all, so there is nothing to race with — which makes this incidentally the one register operation that is inherently atomic.

The mirror-image trap: on a **read-to-clear** register, `if (SR & FLAG)` inside an `if` that also reads `SR` again clears it twice and loses the second flag. Read once into a local, then test the local.
</details>

---

### 5. Read a 64-bit counter split across two registers `SEN`

```c
uint64_t read_hw_counter64(volatile uint32_t *lo, volatile uint32_t *hi);
```

<details><summary>Solution</summary>

```c
uint64_t read_hw_counter64(volatile uint32_t *lo, volatile uint32_t *hi)
{
    if (lo == NULL || hi == NULL) return 0u;

    uint32_t h1, l, h2;
    do {
        h1 = *hi;
        l  = *lo;
        h2 = *hi;
    } while (h1 != h2);          /* retry if lo rolled over mid-read */

    return ((uint64_t)h1 << 32) | (uint64_t)l;
}
```

The counter increments in hardware while you read it. Read `hi` then `lo` naively and if `lo` wraps between the two accesses you get the old high word with the new low word — a timestamp that **jumps backwards by 2³²**. That corrupts every elapsed-time calculation downstream, and it happens once every 4.29 billion ticks, which is often enough to matter and rare enough to be undebuggable from a log.

Read `hi`, `lo`, `hi` again; if the high word is unchanged, no wrap occurred and the pair is consistent. The loop retries at most once in practice, since a second wrap would need 2³² ticks.

This pattern generalises to **any** multi-word value shared with something that updates it asynchronously — a 64-bit software tick counter maintained by an ISR needs exactly the same treatment from task context, or a critical section. Same class of bug as `volatile uint64_t micros;` being read as two non-atomic loads.
</details>

---

### 6. Atomic bit set without disabling interrupts `SEN`

```c
void atomic_set_bits(volatile uint32_t *r, uint32_t mask);
```
LDREX/STREX, ARMv7-M.

<details><summary>Solution</summary>

```c
#include <stdatomic.h>   /* or CMSIS __LDREXW / __STREXW */

void atomic_set_bits(volatile uint32_t *r, uint32_t mask)
{
    if (r == NULL) return;

    uint32_t v;
    do {
        v = __LDREXW((volatile uint32_t *)r);   /* load-exclusive: tags the address */
        v |= mask;
    } while (__STREXW(v, (volatile uint32_t *)r) != 0u);   /* 0 = success */
}
```

`LDREX` marks the address in the exclusive monitor. `STREX` succeeds only if nothing — another context, an ISR, another core — touched that address since. If it failed, retry with a fresh read.

**Why this beats a critical section:** disabling interrupts to protect a two-instruction RMW adds latency to *every* interrupt in the system, including a hard-real-time one that has nothing to do with this register. The CAS loop blocks nobody and is typically faster in the uncontended case.

Three things that get asked as follow-ups:

- **Cortex-M0/M0+ has no `LDREX`/`STREX`** (ARMv6-M). There, disabling interrupts is the only option — which is why portable code hides this behind an OSAL.
- **Any exception return clears the monitor**, so `STREX` fails and retries. That is correct behaviour, not a bug, but it means the loop is not wait-free in the presence of frequent interrupts.
- **Do not put unbounded work between the LDREX and the STREX.** Every added instruction widens the window for a spurious failure.

Modern alternative: `atomic_fetch_or(&x, mask)` from `<stdatomic.h>`, which the compiler lowers to exactly this loop on ARMv7-M. Some peripherals also expose atomic set/clear registers (`BSRR` on STM32 GPIO) — using those is better than either approach, and knowing they exist is the best answer available.
</details>

---

### 7. Barriers — DSB and ISB `SEN`

```c
uint32_t reg_read_dsb(volatile uint32_t *r);
void     reg_write_dsb(volatile uint32_t *r, uint32_t v);
```

<details><summary>Solution</summary>

```c
uint32_t reg_read_dsb(volatile uint32_t *r)
{
    __DSB();                     /* prior memory accesses complete first */
    return *r;
}

void reg_write_dsb(volatile uint32_t *r, uint32_t v)
{
    *r = v;
    __DSB();                     /* the write has actually reached the device */
    __ISB();                     /* flush the pipeline so following code sees the effect */
}
```

`volatile` orders accesses in the *compiled code*. It says nothing about the write buffer between the core and the peripheral bus, so a store can still be in flight when the next instruction executes.

- **`DSB` (Data Synchronisation Barrier)** — do not proceed until all outstanding memory accesses have completed.
- **`ISB` (Instruction Synchronisation Barrier)** — flush the pipeline and refetch, so already-fetched instructions cannot execute against stale state.

**Where you genuinely need them**, and where their absence produces the worst class of bug:

| Situation | Why |
|---|---|
| Disabling an interrupt at the peripheral, then returning from the ISR | Without `DSB`, the write is still in flight and the interrupt re-fires spuriously |
| Writing `SCB->VTOR`, `CONTROL`, or `MPU->CTRL` | `ISB` required before the new configuration governs execution |
| Enabling a peripheral clock, then immediately writing its registers | The clock write must land first or the writes go nowhere |
| After a self-reset request (`SCB->AIRCR`) | `DSB` then spin — without it, execution continues past the reset |

The failure mode is timing-dependent and disappears under a debugger, which is exactly why the barriers belong in the driver rather than being added later during a two-week bug hunt.
</details>

---

### 8. Bit-band alias address `SEN`

```c
uintptr_t bitband_alias(uintptr_t addr, uint8_t bit);
```

<details><summary>Solution</summary>

```c
/* Cortex-M3/M4 bit-band regions. */
#define SRAM_BASE        0x20000000UL
#define SRAM_BB_BASE     0x22000000UL
#define PERIPH_BASE      0x40000000UL
#define PERIPH_BB_BASE   0x42000000UL

uintptr_t bitband_alias(uintptr_t addr, uint8_t bit)
{
    uintptr_t base, bb;

    if (addr >= PERIPH_BASE && addr < PERIPH_BASE + 0x00100000UL) {
        base = PERIPH_BASE; bb = PERIPH_BB_BASE;
    } else if (addr >= SRAM_BASE && addr < SRAM_BASE + 0x00100000UL) {
        base = SRAM_BASE;   bb = SRAM_BB_BASE;
    } else {
        return 0u;                      /* not bit-bandable */
    }
    if (bit > 31u) return 0u;

    return bb + ((addr - base) * 32u) + ((uintptr_t)bit * 4u);
}

/* Use: writing 0 or 1 to the alias word sets or clears exactly that bit. */
static inline void bb_write(uintptr_t addr, uint8_t bit, bool v)
{
    uintptr_t a = bitband_alias(addr, bit);
    if (a != 0u) *(volatile uint32_t *)a = (uint32_t)v;
}
```

Each bit in the 1 MB bit-band region is mapped to its own 32-bit word in a 32 MB alias region — hence `×32` per byte and `×4` per bit. Writing 1 or 0 to the alias word performs the bit operation **in hardware**, so the read-modify-write disappears.

**Why it matters beyond saving two instructions: it is atomic.** No read means no race with an ISR that modifies a neighbouring bit in the same register, so it replaces a critical section for single-bit updates.

The limits are the follow-up: only the first 1 MB of SRAM and the first 1 MB of peripheral space are bit-banded, only on Cortex-M3/M4 — **M0, M0+ and M7 do not have it**. Code that relies on bit-banding does not port, which is why STM32's GPIO `BSRR` (atomic set/reset by writing a mask) is the more portable answer for the common case.
</details>

---

### 9. Configure SysTick for a 1 kHz tick `INT`

```c
void systick_init(uint32_t reload);
```

<details><summary>Solution</summary>

```c
#define SYST_CSR    (*(volatile uint32_t *)0xE000E010UL)
#define SYST_RVR    (*(volatile uint32_t *)0xE000E014UL)
#define SYST_CVR    (*(volatile uint32_t *)0xE000E018UL)

#define CSR_ENABLE      (1UL << 0)
#define CSR_TICKINT     (1UL << 1)
#define CSR_CLKSOURCE   (1UL << 2)   /* 1 = processor clock */

void systick_init(uint32_t reload)
{
    if (reload == 0u || reload > 0x00FFFFFFUL) return;   /* 24-bit counter */

    SYST_CSR = 0UL;                  /* stop before reconfiguring */
    SYST_RVR = reload - 1UL;         /* N-1: period is RVR+1 cycles */
    SYST_CVR = 0UL;                  /* clears COUNTFLAG too */
    SYST_CSR = CSR_CLKSOURCE | CSR_TICKINT | CSR_ENABLE;
}

/* 1 kHz from a 72 MHz core clock: systick_init(72000u); */
```

Four things the reviewer is looking for:

- **`RVR = reload - 1`.** The counter runs from RVR down to 0 inclusive, so the period is `RVR + 1` cycles. Off by one here and every timeout in the system is off by one cycle — harmless at 1 kHz, not harmless if you are generating a bit-banged protocol clock.
- **The reload is 24-bit.** At 72 MHz the maximum period is about 233 ms; anyone asking for a 1-second tick directly is asking for something the hardware cannot do.
- **Disable before reconfiguring**, and write CVR to force the new reload to take effect immediately rather than after the current count expires.
- **`COUNTFLAG` in CSR is read-to-clear.** Reading CSR for any reason clears it, so a polling loop that also reads CSR to check the enable bit will silently consume the tick. Do not mix polled and interrupt use of the same flag.

SysTick's interrupt priority is set in `SCB->SHP`, not the NVIC IPR array — a detail that catches people configuring an RTOS tick.
</details>

---

### 10. Busy delay with wrap-safe tick arithmetic `BEG`

```c
void delay_ms(uint32_t ms);
```
Handle tick wrap.

<details><summary>Solution</summary>

```c
static volatile uint32_t g_ticks;        /* incremented by SysTick_Handler */

void SysTick_Handler(void)
{
    g_ticks++;                           /* 32-bit on a 32-bit core: atomic */
}

void delay_ms(uint32_t ms)
{
    uint32_t start = g_ticks;
    while ((uint32_t)(g_ticks - start) < ms) {
        /* __WFI(); if an interrupt is guaranteed to arrive */
    }
}
```

**Two bugs, both classic.**

`volatile` on `g_ticks` is mandatory. Without it the compiler sees no write to `g_ticks` inside `delay_ms`, hoists the read out of the loop, and generates an infinite loop. This is the canonical `volatile` demonstration and it is asked constantly.

`(g_ticks - start) < ms` is **wrap-safe**; `g_ticks < start + ms` is not. Unsigned subtraction is defined modulo 2³², so the difference is correct even when the counter has rolled over between the two reads. The additive form breaks once per wrap — at 1 kHz that is every 49.7 days, which is precisely the bug that surfaces after a month of field deployment and cannot be reproduced on a bench.

Follow-ups worth having ready: 32-bit reads and writes are atomic on Cortex-M so no critical section is needed here, but a 64-bit tick counter would need entry 5's retry loop. And a busy delay is the wrong tool in an RTOS — it burns power and starves lower-priority tasks; use a blocking sleep. The non-blocking form is the shape to prefer in bare-metal main loops:

```c
if ((uint32_t)(g_ticks - last) >= interval) { last += interval; do_work(); }
```

Note `last += interval` rather than `last = g_ticks` — that preserves the average period instead of drifting by the handler's latency each time.
</details>

---

### 11. Access width matters `INT`

```c
void reg_write_byte(volatile uint32_t *r, uint8_t byte_idx, uint8_t val);
```

<details><summary>Solution</summary>

```c
/* Only valid if the peripheral documents byte access to this register. */
void reg_write_byte(volatile uint32_t *r, uint8_t byte_idx, uint8_t val)
{
    if (r == NULL || byte_idx > 3u) return;
    volatile uint8_t *b = (volatile uint8_t *)r;
    b[byte_idx] = val;               /* little-endian byte ordering assumed */
}
```

The code is trivial; **the correct answer is mostly a warning.**

Many peripherals require a specific access width. Writing a 32-bit register as four bytes can:

- write the low byte only and drop the rest, on registers that latch on a word write,
- trigger the peripheral's side effect three times instead of once (a FIFO push, a W1C clear),
- fault, on a peripheral bus that does not support sub-word transfers.

FIFO data registers are the common legitimate case — an STM32 SPI `DR` accessed as `uint8_t` sends 8 bits and as `uint16_t` sends 16, and that is documented behaviour you are supposed to use. Everything else: use the width the reference manual specifies, and never let `memcpy` near a peripheral register, because it will choose whatever width it likes.

The byte-index-to-endianness dependency is the other half — `b[0]` is the least significant byte only on a little-endian core.
</details>

---

### 12. Relocate the vector table `SEN`

```c
void vtor_set(const void *table_base);
```

<details><summary>Solution</summary>

```c
#define SCB_VTOR   (*(volatile uint32_t *)0xE000ED08UL)

void vtor_set(const void *table_base)
{
    uintptr_t a = (uintptr_t)table_base;

    /* VTOR requires alignment to the table size, rounded up to a power of two,
       and at least 128 bytes (32 entries × 4). 512 is the safe common choice. */
    if ((a & 0x1FFUL) != 0u) return;

    SCB_VTOR = (uint32_t)a;
    __DSB();
    __ISB();                    /* new table must govern before the next exception */
}

/* Bootloader handing off to an application: */
static void jump_to_app(uintptr_t app_base)
{
    uint32_t app_sp    = *(volatile uint32_t *)app_base;
    uint32_t app_reset = *(volatile uint32_t *)(app_base + 4u);

    __disable_irq();
    /* de-init peripherals, disable and clear every pending IRQ here */
    vtor_set((const void *)app_base);
    __set_MSP(app_sp);                        /* app's initial stack pointer */
    __enable_irq();
    ((void (*)(void))app_reset)();             /* never returns */
}
```

The vector table is an array of function pointers at the start of flash; entry 0 is the **initial stack pointer**, not a handler, which is why the core can push registers before executing a single instruction of your code. `VTOR` tells the core where that array lives.

**This is the bootloader problem**, and it is asked at any company shipping OTA updates. The failure if you skip `VTOR`: the application runs, then the first interrupt vectors into the *bootloader's* handler table, which jumps into de-initialised code. Symptom is a hard fault seconds after a successful-looking boot.

Four things that separate a working handoff:

- **Alignment.** VTOR's low bits are reserved; the table must be aligned to its own size rounded up to a power of two.
- **Set MSP from the application's own vector 0**, or the app runs on the bootloader's stack and overflows into it.
- **Disable and clear pending interrupts before the jump.** A pending IRQ that fires during the transition vectors through whichever table is current at that instant.
- **`DSB` + `ISB` after the write** — see entry 7. Without `ISB`, an already-fetched instruction can execute against the old table.

For a RAM-resident table, copy the flash table to a `__attribute__((aligned(512)))` RAM array first, then point VTOR at it. That is how you patch a handler at runtime.
</details>

---

# Part 05 — Data Structures

20 entries from 75. Skip lists, segment trees and tries are gone — they do not appear in firmware and they do not appear in firmware interviews. What replaced them is the ring buffer family, which is the single most-asked embedded data structure, and the static-pool patterns that exist because `malloc` is banned.

> [!IMPORTANT]
> **The constraint that shapes this entire part: no dynamic allocation.**
> Every structure here is backed by a fixed array declared at compile time. That is not a stylistic preference — a heap in long-running firmware fragments until an allocation fails at hour 900, and there is no operator to restart it. So "where do the nodes come from" is a real design question with a real answer (entry 13), and any solution that opens with `malloc` has failed the interview before the logic is examined.

---

## The shared ring buffer

Entries 3–6 all operate on this. Read it once.

```c
#define RB_SIZE   256u                       /* MUST be a power of two */
#define RB_MASK   (RB_SIZE - 1u)

struct rb {
    uint8_t  buf[RB_SIZE];
    volatile uint16_t head;                  /* producer writes, consumer reads */
    volatile uint16_t tail;                  /* consumer writes, producer reads */
};
```

**Free-running indices, masked only when indexing.** `head` and `tail` count monotonically and wrap naturally at 65536; the mask converts an index to a slot. Two consequences that matter:

- `(uint16_t)(head - tail)` is the occupancy, and it is **wrap-safe** because unsigned subtraction is defined modulo 2¹⁶.
- **No slot is wasted.** The alternative design keeps indices inside `[0, RB_SIZE)` and must sacrifice one slot so that "head == tail" can mean empty rather than full. Free-running indices distinguish the two cases by the difference, so all 256 bytes are usable.

The requirement is that the index type's range be an exact multiple of `RB_SIZE` — `65536 % 256 == 0` holds. Change `RB_SIZE` to 100 and this breaks silently, which is why the power-of-two constraint is not negotiable.

---

### 1. Static stack `BEG`

```c
bool stack_push(struct stk *s, int v);
bool stack_pop(struct stk *s, int *out);
bool stack_peek(const struct stk *s, int *out);
```

<details><summary>Solution</summary>

```c
#define STK_CAP 32u

struct stk {
    int    data[STK_CAP];
    size_t top;                 /* number of items == index of next free slot */
};

bool stack_push(struct stk *s, int v)
{
    if (s == NULL || s->top >= STK_CAP) return false;
    s->data[s->top++] = v;
    return true;
}

bool stack_pop(struct stk *s, int *out)
{
    if (s == NULL || out == NULL || s->top == 0u) return false;
    s->top--;
    *out = s->data[s->top];
    return true;
}

bool stack_peek(const struct stk *s, int *out)
{
    if (s == NULL || out == NULL || s->top == 0u) return false;
    *out = s->data[s->top - 1u];
    return true;
}
```

`top` as a *count* rather than an index to the last element is the choice that removes the special case: empty is `top == 0`, full is `top == CAP`, and there is no `-1` sentinel to get wrong.

**Return `bool`, do not return the value.** `int stack_pop(void)` has no way to report an empty stack — every possible `int` is a legitimate payload. Out-parameter plus status is the C idiom, and choosing it unprompted signals you have maintained an API rather than only written one.

Note `stack_peek` takes `const struct stk *`. Marking read-only parameters `const` documents intent and lets the compiler keep values in registers across the call.
</details>

---

### 2. Bounded FIFO — the full-versus-empty problem `BEG`

```c
bool queue_enqueue(struct q *q, int x);
bool queue_dequeue(struct q *q, int *x);
```

<details><summary>Solution</summary>

```c
#define Q_CAP 16u

struct q {
    int    data[Q_CAP];
    size_t head;                /* next write slot */
    size_t tail;                /* next read slot  */
    size_t count;               /* items held      */
};

bool queue_enqueue(struct q *q, int x)
{
    if (q == NULL || q->count == Q_CAP) return false;
    q->data[q->head] = x;
    q->head = (q->head + 1u) % Q_CAP;
    q->count++;
    return true;
}

bool queue_dequeue(struct q *q, int *x)
{
    if (q == NULL || x == NULL || q->count == 0u) return false;
    *x = q->data[q->tail];
    q->tail = (q->tail + 1u) % Q_CAP;
    q->count--;
    return true;
}
```

**`head == tail` is ambiguous** — it means empty when the queue has just been drained and full when it has just been filled. Three ways to resolve it, and the interviewer wants you to name them:

| Approach | Cost | Use when |
|---|---|---|
| Keep an explicit `count` | one extra field, **and a third variable two contexts must both update** | single-threaded |
| Waste one slot: full is `(head+1) % CAP == tail` | one lost element | simple, thread-safe |
| Free-running indices, occupancy is `head - tail` | requires power-of-two capacity | ISR ↔ task (entry 3) |

The `count` field is the clearest single-threaded answer and the **wrong** answer for an ISR-to-task queue, because both sides must modify it — which reintroduces exactly the race the ring buffer design exists to avoid. Say that, because it is the bridge to the next entry.

`% Q_CAP` on a non-power-of-two capacity emits a division instruction, which Cortex-M0 does not have and which costs 20+ cycles where it does. Power-of-two plus a mask is the firmware answer.
</details>

---

### 3. SPSC lock-free ring buffer `SEN`

```c
bool rb_write(struct rb *r, uint8_t b);
bool rb_read(struct rb *r, uint8_t *b);
```
Safe between an ISR and a task with no critical section.

<details><summary>Solution</summary>

```c
/* Producer side only. Called from the ISR (or the task — one of the two, never both). */
bool rb_write(struct rb *r, uint8_t b)
{
    if (r == NULL) return false;

    uint16_t head = r->head;                          /* we own this  */
    uint16_t tail = r->tail;                          /* single read of theirs */

    if ((uint16_t)(head - tail) >= RB_SIZE) {
        return false;                                 /* full */
    }

    r->buf[head & RB_MASK] = b;
    __DMB();                                          /* data lands before index moves */
    r->head = head + 1u;
    return true;
}

/* Consumer side only. */
bool rb_read(struct rb *r, uint8_t *b)
{
    if (r == NULL || b == NULL) return false;

    uint16_t tail = r->tail;                          /* we own this */
    uint16_t head = r->head;                          /* single read of theirs */

    if (head == tail) {
        return false;                                 /* empty */
    }

    *b = r->buf[tail & RB_MASK];
    __DMB();                                          /* read data before releasing slot */
    r->tail = tail + 1u;
    return true;
}
```

**This is the most important function in embedded C**, and it is the one to be able to write from memory. Every UART driver, every logging subsystem, every ISR-to-task handoff is this.

Four properties make it correct without a lock:

1. **Each index has exactly one writer.** The producer writes `head` and only reads `tail`; the consumer writes `tail` and only reads `head`. No shared variable is modified by both sides, so there is no read-modify-write to race on. **This is why the `count` field from entry 2 must go** — it would be written by both.
2. **`volatile` on both indices.** Without it the compiler caches `tail` in a register and the producer never observes the consumer draining the buffer. This is the same defect as the `delay_ms` loop in Part 02 entry 10.
3. **A barrier between the payload and the index update.** The store to `buf[]` must be visible before the store to `head`, or a consumer that sees the new index reads a slot that has not been filled. `volatile` orders the *compiler*; `__DMB()` orders the *memory system*. On single-core Cortex-M the write buffer makes this mostly theoretical, but it is free and it is the correct answer — and on a multi-core part it is mandatory.
4. **One read of the other side's index, into a local.** Reading `r->tail` twice invites two different values and a decision made on the stale one.

**Follow-ups to have ready.** 16-bit index loads and stores are single instructions on Cortex-M, so they are atomic — on an 8-bit AVR they are not, and the whole scheme collapses without a critical section. Also this is SPSC only: two producers race on `head` and need a CAS loop or a lock. And the fullness test is `>= RB_SIZE`, not `== RB_SIZE`, so a corrupted index cannot slip past the guard and overwrite live data.
</details>

---

### 4. Ring buffer bulk transfer `SEN`

```c
size_t rb_write_bulk(struct rb *r, const uint8_t *src, size_t n);
size_t rb_read_bulk(struct rb *r, uint8_t *dst, size_t n);
```
Return bytes transferred.

<details><summary>Solution</summary>

```c
#include <string.h>

size_t rb_write_bulk(struct rb *r, const uint8_t *src, size_t n)
{
    if (r == NULL || src == NULL) return 0u;

    uint16_t head = r->head;
    uint16_t tail = r->tail;
    size_t   free_space = RB_SIZE - (size_t)(uint16_t)(head - tail);

    if (n > free_space) n = free_space;          /* partial write, not failure */
    if (n == 0u) return 0u;

    size_t off   = head & RB_MASK;
    size_t first = RB_SIZE - off;                /* bytes until the wrap point */
    if (first > n) first = n;

    memcpy(&r->buf[off], src, first);
    if (n > first) {
        memcpy(&r->buf[0], src + first, n - first);   /* second copy after wrap */
    }

    __DMB();
    r->head = (uint16_t)(head + n);
    return n;
}

size_t rb_read_bulk(struct rb *r, uint8_t *dst, size_t n)
{
    if (r == NULL || dst == NULL) return 0u;

    uint16_t tail = r->tail;
    uint16_t head = r->head;
    size_t   avail = (size_t)(uint16_t)(head - tail);

    if (n > avail) n = avail;
    if (n == 0u) return 0u;

    size_t off   = tail & RB_MASK;
    size_t first = RB_SIZE - off;
    if (first > n) first = n;

    memcpy(dst, &r->buf[off], first);
    if (n > first) {
        memcpy(dst + first, &r->buf[0], n - first);
    }

    __DMB();
    r->tail = (uint16_t)(tail + n);
    return n;
}
```

**Two `memcpy` calls, never a byte loop.** That is the whole point of the problem: a wrapping region is at most two contiguous runs, and `memcpy` on a Cortex-M4 moves 4–8 bytes per iteration where a hand-written byte loop moves one. On a 115200-baud UART draining 64 bytes per interrupt the difference is measurable in ISR latency.

**Return the count, do not fail.** A partial write is the useful contract — the caller retries the remainder next tick. Returning `false` on a full buffer forces the caller to reimplement the split.

The index arithmetic is the part to get right: `first` is bytes-until-wrap clamped to `n`, and the second copy only happens when `n` exceeds it. Test with a buffer deliberately positioned so `off + n` straddles the boundary; a test that never wraps passes on broken code.

Note the single index update at the end rather than one per byte — that is what keeps the operation atomic from the other side's point of view.
</details>

---

### 5. Ring buffer queries — peek, used, free `BEG`→`INT`

```c
bool   rb_peek(const struct rb *r, uint8_t *b);
size_t rb_used(const struct rb *r);
size_t rb_free(const struct rb *r);
bool   rb_empty(const struct rb *r);
bool   rb_full(const struct rb *r);
```

<details><summary>Solution</summary>

```c
size_t rb_used(const struct rb *r)
{
    if (r == NULL) return 0u;
    return (size_t)(uint16_t)(r->head - r->tail);       /* wrap-safe */
}

size_t rb_free(const struct rb *r)
{
    return RB_SIZE - rb_used(r);
}

bool rb_empty(const struct rb *r) { return rb_used(r) == 0u; }
bool rb_full(const struct rb *r)  { return rb_used(r) >= RB_SIZE; }

bool rb_peek(const struct rb *r, uint8_t *b)
{
    if (r == NULL || b == NULL) return false;
    uint16_t tail = r->tail;
    if (r->head == tail) return false;
    *b = r->buf[tail & RB_MASK];
    return true;                    /* tail NOT advanced */
}
```

`head - tail` in unsigned arithmetic handles the wrap for free — that identity is why the free-running index design was chosen, and it is the same wrap-safe subtraction as the timer comparison in Part 02.

**The trap in these functions is that the answer is stale the instant it is returned.** If an ISR is producing, `rb_used()` is a lower bound on what is available and `rb_free()` is a lower bound on space. So this is safe:

```c
if (rb_used(&r) >= 4u) { read four bytes }        /* can only have grown */
```

and this is a bug:

```c
size_t n = rb_free(&r);
rb_write_bulk(&r, src, n);       /* fine — but only because write clamps internally */
```

Any decision that assumes the value is still exact when acted upon is wrong. This is why `rb_write_bulk` re-reads the indices itself and clamps rather than trusting a count the caller passed in.

`rb_peek` is genuinely useful for protocol parsers: inspect the next byte to decide whether a complete frame has arrived before committing to consuming it.
</details>

---

### 6. Full-buffer policy — drop versus overwrite `INT`

```c
size_t rb_write_drop(struct rb *r, const uint8_t *src, size_t n);      /* keep old data */
void   rb_write_overwrite(struct rb *r, uint8_t b);                    /* keep new data */
```

<details><summary>Solution</summary>

```c
/* Drop-on-full: refuse the newest bytes. Unread data is never destroyed. */
size_t rb_write_drop(struct rb *r, const uint8_t *src, size_t n)
{
    return rb_write_bulk(r, src, n);          /* already clamps to free space */
}

/* Overwrite-on-full: discard the OLDEST byte to make room for the newest. */
void rb_write_overwrite(struct rb *r, uint8_t b)
{
    if (r == NULL) return;

    uint16_t head = r->head;

    if ((uint16_t)(head - r->tail) >= RB_SIZE) {
        r->tail = r->tail + 1u;               /* drop oldest — SEE WARNING */
    }
    r->buf[head & RB_MASK] = b;
    __DMB();
    r->head = head + 1u;
}
```

Not a coding problem — a **design** problem, and the interviewer is checking whether you ask which behaviour is wanted instead of picking one.

| Policy | Loses | Correct for |
|---|---|---|
| Drop-on-full | newest data | commands, protocol frames, anything where a gap corrupts the stream |
| Overwrite-on-full | oldest data | sensor samples, debug logs, a crash trace where you want the *last* N events |

Getting it backwards is a real fault: overwriting on a command queue means executing a command that was half-consumed; dropping on a diagnostic log means keeping the boot messages and losing the ones describing the crash.

> [!WARNING]
> **`rb_write_overwrite` breaks the SPSC guarantee.** The producer now writes `tail`, which the consumer also writes — the single-writer-per-index property from entry 3 is gone, and the two can advance `tail` simultaneously. An overwrite-on-full ring shared between an ISR and a task **needs a critical section around the tail update**, or it needs the consumer to detect the overrun instead (compare a sequence number, or expose a dropped-bytes counter). Spotting that this "small change" invalidates the lock-free proof is the senior-level observation in this part.
</details>

---

### 7. Bit array `BEG`

```c
void bitarr_set(uint8_t *a, size_t bit);
void bitarr_clear(uint8_t *a, size_t bit);
bool bitarr_test(const uint8_t *a, size_t bit);
size_t bitarr_find_first_clear(const uint8_t *a, size_t nbits);
```

<details><summary>Solution</summary>

```c
#define BITARR_BYTES(nbits)  (((nbits) + 7u) / 8u)

void bitarr_set(uint8_t *a, size_t bit)
{
    if (a == NULL) return;
    a[bit >> 3] |= (uint8_t)(1u << (bit & 7u));
}

void bitarr_clear(uint8_t *a, size_t bit)
{
    if (a == NULL) return;
    a[bit >> 3] &= (uint8_t)~(1u << (bit & 7u));
}

bool bitarr_test(const uint8_t *a, size_t bit)
{
    if (a == NULL) return false;
    return (a[bit >> 3] & (1u << (bit & 7u))) != 0u;
}

/* Returns nbits if every bit is set. */
size_t bitarr_find_first_clear(const uint8_t *a, size_t nbits)
{
    if (a == NULL) return nbits;

    size_t nbytes = BITARR_BYTES(nbits);
    for (size_t i = 0u; i < nbytes; i++) {
        if (a[i] != 0xFFu) {
            uint8_t inv = (uint8_t)~a[i];
            size_t  bit = (i * 8u) + ctz32(inv);      /* Part 01 entry 8 */
            return (bit < nbits) ? bit : nbits;
        }
    }
    return nbits;
}
```

`bit >> 3` is the byte, `bit & 7` is the position within it — division and modulo by 8 as a shift and a mask, valid because the operand is unsigned. This two-line addressing rule is the core of every bitmap in firmware: allocator free maps, dirty-page tracking, "which of these 200 sensors has reported" flags.

`find_first_clear` is the one with content. **Scan a byte at a time and only descend to bits when a byte is not full** — skipping 8 bits per comparison, then `ctz` on the inverted byte to find the exact position. That is the free-block search in a bitmap allocator, and doing it bit by bit is 8× slower for no reason.

Note `BITARR_BYTES` rounds up, and that the trailing bits of the last byte are outside `nbits` — hence the `bit < nbits` check before returning. Set those padding bits to 1 at initialisation and the check becomes unnecessary, which is the trick worth mentioning.

**Not ISR-safe.** `|=` on a byte is a read-modify-write, so two contexts setting different bits in the same byte can lose one. Either guard it or use the atomic bit-band alias from Part 02 entry 8.
</details>

---

### 8. Singly linked list — insert, find, remove `BEG`→`INT`

```c
void  sll_push_front(struct node **head, struct node *n);
struct node *sll_find(struct node *head, int key);
bool  sll_remove(struct node **head, int key);
```

<details><summary>Solution</summary>

```c
struct node {
    int          key;
    struct node *next;
};

void sll_push_front(struct node **head, struct node *n)
{
    if (head == NULL || n == NULL) return;
    n->next = *head;
    *head   = n;
}

struct node *sll_find(struct node *head, int key)
{
    for (struct node *p = head; p != NULL; p = p->next) {
        if (p->key == key) return p;
    }
    return NULL;
}

/* Pointer-to-pointer walk: no special case for the head node. */
bool sll_remove(struct node **head, int key)
{
    if (head == NULL) return false;

    for (struct node **pp = head; *pp != NULL; pp = &(*pp)->next) {
        if ((*pp)->key == key) {
            *pp = (*pp)->next;          /* unlink — works at the head too */
            return true;
        }
    }
    return false;
}
```

**`sll_remove` is the entry worth studying.** The naive version keeps a `prev` pointer and needs a separate branch for removing the head, which is where the bug goes. Walking a **pointer to the link** instead means `*pp` is either `*head` or some node's `next` field, and one assignment covers both — the special case ceases to exist.

Being able to write and *explain* the `struct node **` walk is a reliable signal that someone is comfortable with indirection rather than reciting it.

Note the nodes are passed in, not allocated. In firmware they come from a static pool (entry 13). `sll_push_front` taking an already-existing node is deliberate, not an oversight.
</details>

---

### 9. Reverse a linked list `INT`

```c
struct node *sll_reverse(struct node *head);
```

<details><summary>Solution</summary>

```c
struct node *sll_reverse(struct node *head)
{
    struct node *prev = NULL;

    while (head != NULL) {
        struct node *next = head->next;    /* save before overwriting */
        head->next = prev;                 /* flip the link           */
        prev = head;                       /* advance both            */
        head = next;
    }
    return prev;                           /* old tail is the new head */
}
```

Three pointers, one pass, O(1) extra space. **The `next` save must happen first** — overwrite `head->next` before saving it and the rest of the list is unreachable. That single line is what the question tests.

The recursive form is asked as a follow-up and is worth answering with a caveat:

```c
struct node *sll_reverse_rec(struct node *head)
{
    if (head == NULL || head->next == NULL) return head;
    struct node *rest = sll_reverse_rec(head->next);
    head->next->next = head;
    head->next = NULL;
    return rest;
}
```

It is elegant and **wrong for firmware**: recursion depth equals list length, so a 500-node list is 500 stack frames. On a part with 8 KB of RAM and a 1 KB stack that is an overflow, and stack overflow on Cortex-M without an MPU is silent corruption of whatever sits below. Give the iterative version, mention the recursive one, and say why you would not ship it.
</details>

---

### 10. Cycle detection — Floyd `INT`

```c
bool sll_has_cycle(const struct node *head);
struct node *sll_cycle_start(struct node *head);
```

<details><summary>Solution</summary>

```c
bool sll_has_cycle(const struct node *head)
{
    const struct node *slow = head;
    const struct node *fast = head;

    while (fast != NULL && fast->next != NULL) {
        slow = slow->next;                 /* one step  */
        fast = fast->next->next;           /* two steps */
        if (slow == fast) return true;
    }
    return false;
}

/* Where the loop begins. */
struct node *sll_cycle_start(struct node *head)
{
    struct node *slow = head, *fast = head;

    while (fast != NULL && fast->next != NULL) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) {
            slow = head;                   /* reset one pointer to the head */
            while (slow != fast) {         /* now advance both at one step  */
                slow = slow->next;
                fast = fast->next;
            }
            return slow;                   /* they meet at the entry point  */
        }
    }
    return NULL;
}
```

Two pointers at different speeds. If a cycle exists the fast pointer laps the slow one and they collide; if not, fast reaches `NULL`. O(n) time, O(1) space — which is what beats the obvious "keep a set of visited nodes" answer on a part with no heap.

**`fast != NULL && fast->next != NULL` is the guard that matters.** Check only `fast` and `fast->next->next` dereferences a null pointer on an even-length list. Order matters too — short-circuit evaluation is what makes the second test safe.

The cycle-start step looks like magic and has a one-line justification: at the meeting point, the distance from the head to the cycle entry equals the distance from the meeting point to the cycle entry. Be ready to say that; interviewers ask.

**Why an embedded engineer should care**, beyond it being a stock question: a corrupted `next` pointer creates a loop, and a traversal that assumed termination hangs the system. Adding a Floyd check to a linked-list walk in a debug build catches heap or pool corruption at the point of use rather than three subsystems later.
</details>

---

### 11. Doubly linked list `BEG`

```c
void dll_init(struct dnode *sentinel);
void dll_insert_after(struct dnode *pos, struct dnode *n);
void dll_remove(struct dnode *n);
bool dll_empty(const struct dnode *sentinel);
```

<details><summary>Solution</summary>

```c
struct dnode {
    struct dnode *prev;
    struct dnode *next;
};

/* Circular list with a sentinel: empty means it points at itself. */
void dll_init(struct dnode *s)
{
    if (s == NULL) return;
    s->next = s;
    s->prev = s;
}

void dll_insert_after(struct dnode *pos, struct dnode *n)
{
    if (pos == NULL || n == NULL) return;
    n->next         = pos->next;
    n->prev         = pos;
    pos->next->prev = n;
    pos->next       = n;
}

void dll_remove(struct dnode *n)
{
    if (n == NULL) return;
    n->prev->next = n->next;
    n->next->prev = n->prev;
    n->next = NULL;                  /* poison, so a stale use is caught */
    n->prev = NULL;
}

bool dll_empty(const struct dnode *s)
{
    return (s == NULL) || (s->next == s);
}
```

**The sentinel is the design decision.** A circular list with a dummy head node means no node is ever `NULL`-adjacent, so `dll_remove` needs **no null checks and no branches** — it is four unconditional pointer writes, valid for the first node, the last node and the only node alike. The non-sentinel version needs three special cases and that is where the bug lives.

`dll_remove` taking only the node, with no list argument, is the payoff: **O(1) removal given just a pointer to the element.** That is why every RTOS ready-queue and every LRU list is a doubly linked list — unblocking a task means removing it from wherever it is without searching.

Poisoning the pointers after removal is cheap and turns use-after-remove into an immediate null dereference at a known line instead of silent corruption. In a release build, `#ifdef` it out.
</details>

---

### 12. Intrusive lists and `container_of` `INT`

```c
#define container_of(ptr, type, member)  /* ... */
```

<details><summary>Solution</summary>

```c
#include <stddef.h>       /* offsetof */

#define container_of(ptr, type, member) \
    ((type *)(void *)((char *)(ptr) - offsetof(type, member)))

/* Usage: the list node is EMBEDDED in the payload, not pointing at it. */
struct msg {
    uint32_t     id;
    uint8_t      payload[32];
    struct dnode link;              /* intrusive hook */
};

void process_all(struct dnode *sentinel)
{
    for (struct dnode *p = sentinel->next; p != sentinel; p = p->next) {
        struct msg *m = container_of(p, struct msg, link);
        handle(m->id, m->payload);
    }
}
```

Given a pointer to a member, recover a pointer to the enclosing struct by subtracting the member's byte offset. `offsetof` supplies the offset; the cast through `char *` is what makes the arithmetic byte-granular.

**Why intrusive lists are the firmware default.** A conventional list node holds a `void *data` pointer, which means (a) a separate allocation for every node, and (b) an extra indirection per access. Embedding the link *inside* the payload gives you zero allocations, one cache line, and the ability to put the same object on two lists at once by adding a second hook. This is how the Linux kernel, FreeRTOS and Zephyr all do it.

Details that get probed:

- **The cast through `void *`** silences the alignment warning from casting `char *` to `struct msg *`. Some codebases omit it; `-Wcast-align` will complain.
- **`offsetof` on a non-POD or bit-field member is undefined.** Keep the hook a plain struct member.
- **The `type` argument is unchecked.** Pass the wrong type and you get a silently wrong pointer — no diagnostic. The kernel's version adds a `__same_type` compile-time assertion for exactly this reason:

```c
#define container_of(ptr, type, member) __extension__ ({          \
    const __typeof__(((type *)0)->member) *__mptr = (ptr);        \
    (type *)(void *)((char *)__mptr - offsetof(type, member)); })
```

That form is a GCC statement expression — not portable C, but worth knowing since you will read it.
</details>

---

### 13. Static node pool — where nodes come from `INT`

```c
void  pool_init(void);
int   pool_alloc(void);            /* returns index, or -1 */
void  pool_free(int idx);
```

<details><summary>Solution</summary>

```c
#define POOL_N   32
#define IDX_NONE (-1)

struct pnode {
    int  key;
    int  next;                     /* INDEX, not a pointer */
};

static struct pnode g_pool[POOL_N];
static int          g_free_head;

void pool_init(void)
{
    for (int i = 0; i < POOL_N - 1; i++) {
        g_pool[i].next = i + 1;               /* thread the free list */
    }
    g_pool[POOL_N - 1].next = IDX_NONE;
    g_free_head = 0;
}

int pool_alloc(void)
{
    int i = g_free_head;
    if (i == IDX_NONE) return IDX_NONE;       /* exhausted — a real, testable state */
    g_free_head = g_pool[i].next;
    g_pool[i].next = IDX_NONE;
    return i;
}

void pool_free(int idx)
{
    if (idx < 0 || idx >= POOL_N) return;     /* reject out-of-range */
    g_pool[idx].next = g_free_head;
    g_free_head = idx;
}
```

**This is the answer to "but where do the nodes come from without `malloc`".** A fixed array plus a free list threaded through the unused entries: O(1) allocate, O(1) free, zero fragmentation, and a bounded worst case you can prove at compile time.

**Indices rather than pointers**, which is the detail worth defending:

- An index fits in `uint8_t` or `uint16_t` where a pointer costs 4 bytes — on a 32-node pool that is 75% saved per link.
- Indices are **position-independent**: the structure can be copied to another address, written to flash and restored, or shared across a memory boundary. Pointers cannot.
- **Bounds-checkable.** `idx < POOL_N` is a valid test; a wild pointer has no equivalent.

Follow-ups to be ready for: **double-free** corrupts the list into a cycle, so a debug build should track an in-use bitmap (entry 7) and assert. And `pool_alloc` from an ISR races with `pool_alloc` from a task on `g_free_head` — either guard it with a critical section or make the free-list head a CAS loop (Part 02 entry 6).
</details>

---

### 14. Object pool with reference counting `SEN`

```c
struct buf *buf_acquire(void);
void buf_ref(struct buf *b);
void buf_release(struct buf *b);
```

<details><summary>Solution</summary>

```c
#define NBUF 8

struct buf {
    uint8_t  data[256];
    uint16_t len;
    uint8_t  refs;                 /* 0 == in the free pool */
};

static struct buf g_bufs[NBUF];

struct buf *buf_acquire(void)
{
    for (int i = 0; i < NBUF; i++) {
        bool taken = false;
        ENTER_CRITICAL();
        if (g_bufs[i].refs == 0u) { g_bufs[i].refs = 1u; taken = true; }
        EXIT_CRITICAL();
        if (taken) {
            g_bufs[i].len = 0u;
            return &g_bufs[i];
        }
    }
    return NULL;                   /* pool exhausted */
}

void buf_ref(struct buf *b)
{
    if (b == NULL) return;
    ENTER_CRITICAL();
    if (b->refs > 0u && b->refs < 0xFFu) b->refs++;
    EXIT_CRITICAL();
}

void buf_release(struct buf *b)
{
    if (b == NULL) return;
    ENTER_CRITICAL();
    if (b->refs > 0u) b->refs--;
    /* refs == 0 returns it to the pool implicitly — no free list to update */
    EXIT_CRITICAL();
}
```

Solves the ownership problem in a layered stack: a received packet is handed from the driver to the MAC layer to the application, and **nobody knows who should release it**. Reference counting answers that — whoever takes a reference drops it, and the last drop frees.

**The count must be modified atomically.** `refs++` is a read-modify-write; two contexts incrementing concurrently can lose one, and the buffer is then freed while still in use. Either a critical section as shown, `atomic_fetch_add`, or the LDREX/STREX loop from Part 02.

Three failure modes to name, because they are the interview:

| Bug | Result |
|---|---|
| Forgotten `release` | pool leaks until exhausted, then `NULL` returns and packets drop — hours later |
| Extra `release` | buffer reused while a reader still holds it; corruption far from the cause |
| `refs` overflow | wraps to 0 and the buffer is freed while referenced — hence the `< 0xFF` guard |

Returning `NULL` on exhaustion, and having every caller handle it, is not optional. Pool exhaustion is the normal steady state of a system under load, not an exceptional condition.
</details>

---

### 15. Binary heap — insert and pop-min `SEN`

```c
bool heap_push(struct heap *h, int v);
bool heap_pop_min(struct heap *h, int *out);
```

<details><summary>Solution</summary>

```c
#define HEAP_CAP 64

struct heap {
    int    a[HEAP_CAP];
    size_t n;
};

/* Array-backed: children of i are 2i+1 and 2i+2, parent is (i-1)/2. */
bool heap_push(struct heap *h, int v)
{
    if (h == NULL || h->n >= HEAP_CAP) return false;

    size_t i = h->n++;
    h->a[i] = v;

    while (i > 0u) {                          /* sift up */
        size_t p = (i - 1u) / 2u;
        if (h->a[p] <= h->a[i]) break;
        int t = h->a[p]; h->a[p] = h->a[i]; h->a[i] = t;
        i = p;
    }
    return true;
}

bool heap_pop_min(struct heap *h, int *out)
{
    if (h == NULL || out == NULL || h->n == 0u) return false;

    *out = h->a[0];
    h->n--;
    if (h->n == 0u) return true;

    h->a[0] = h->a[h->n];                     /* move last to root */

    size_t i = 0u;
    for (;;) {                                /* sift down */
        size_t l = (2u * i) + 1u;
        size_t r = l + 1u;
        size_t m = i;

        if (l < h->n && h->a[l] < h->a[m]) m = l;
        if (r < h->n && h->a[r] < h->a[m]) m = r;
        if (m == i) break;

        int t = h->a[m]; h->a[m] = h->a[i]; h->a[i] = t;
        i = m;
    }
    return true;
}
```

A complete binary tree stored in a flat array — **no pointers, no allocation, no fragmentation**, which is why it is the priority queue firmware actually uses. O(log n) both ways, and the array is contiguous so it is cache-friendly.

The two things that break implementations:

- **Sift *down* from the root after a pop, not up.** Popping moves the last element to the root, which is almost certainly too large, so it must descend. Writing sift-up there produces a structure that looks fine on small inputs and violates the heap property as it grows.
- **Bounds-check both children before comparing.** `l < h->n` and `r < h->n` — a node can have one child or none, and reading `a[r]` past `n` compares against stale data, silently producing a wrong ordering rather than a crash.

`(i - 1) / 2` for the parent requires `i > 0`, hence the loop guard. With `size_t`, `i == 0` would wrap to `SIZE_MAX` and index far out of bounds.

Note this is a *min*-heap; flipping every comparison gives a max-heap, and getting asked to convert on the spot is common.
</details>

---

### 16. Priority queue over a static pool `SEN`

```c
bool pq_push(struct pq *q, uint8_t prio, uint32_t payload);
bool pq_pop(struct pq *q, uint32_t *payload);
```

<details><summary>Solution</summary>

```c
#define PQ_CAP 32

struct pq_item {
    uint8_t  prio;              /* 0 == highest */
    uint32_t seq;               /* insertion order, for stability */
    uint32_t payload;
};

struct pq {
    struct pq_item a[PQ_CAP];
    size_t         n;
    uint32_t       next_seq;
};

/* Order by priority, then by sequence — so equal priorities stay FIFO. */
static bool pq_less(const struct pq_item *x, const struct pq_item *y)
{
    if (x->prio != y->prio) return x->prio < y->prio;
    return (int32_t)(x->seq - y->seq) < 0;         /* wrap-safe comparison */
}

bool pq_push(struct pq *q, uint8_t prio, uint32_t payload)
{
    if (q == NULL || q->n >= PQ_CAP) return false;

    size_t i = q->n++;
    q->a[i].prio    = prio;
    q->a[i].seq     = q->next_seq++;
    q->a[i].payload = payload;

    while (i > 0u) {
        size_t p = (i - 1u) / 2u;
        if (!pq_less(&q->a[i], &q->a[p])) break;
        struct pq_item t = q->a[p]; q->a[p] = q->a[i]; q->a[i] = t;
        i = p;
    }
    return true;
}

bool pq_pop(struct pq *q, uint32_t *payload)
{
    if (q == NULL || payload == NULL || q->n == 0u) return false;

    *payload = q->a[0].payload;
    q->n--;
    if (q->n == 0u) return true;

    q->a[0] = q->a[q->n];

    size_t i = 0u;
    for (;;) {
        size_t l = (2u * i) + 1u, r = l + 1u, m = i;
        if (l < q->n && pq_less(&q->a[l], &q->a[m])) m = l;
        if (r < q->n && pq_less(&q->a[r], &q->a[m])) m = r;
        if (m == i) break;
        struct pq_item t = q->a[m]; q->a[m] = q->a[i]; q->a[i] = t;
        i = m;
    }
    return true;
}
```

Entry 15's heap with a comparator — but the **sequence number is the point of the problem**.

A binary heap is **not stable**: two items of equal priority come out in arbitrary order, because sifting moves elements past each other. For a task or message queue that is a bug — two commands of the same priority must execute in the order they were submitted. Adding a monotonic sequence number as the tie-break restores FIFO ordering within each priority level.

`(int32_t)(x->seq - y->seq) < 0` rather than `x->seq < y->seq`: the signed difference is **wrap-safe**, so ordering stays correct after `next_seq` rolls over 2³². Same identity as the timer comparison in Part 02 entry 3, applied to sequencing rather than time.

This structure *is* an RTOS ready queue in miniature. The follow-up is usually priority inversion — see Part 08.
</details>

---

### 17. Timer list sorted by expiry `SEN`

```c
void tl_add(struct tl *l, struct tmr *t);
void tl_tick(struct tl *l);
```

<details><summary>Solution</summary>

```c
struct tmr {
    uint32_t     delta;           /* ticks AFTER the previous timer in the list */
    void       (*cb)(void *);
    void        *arg;
    struct tmr  *next;
};

struct tl { struct tmr *head; };

/* Insert by absolute delay, converting to a delta chain. */
void tl_add(struct tl *l, struct tmr *t, uint32_t delay)
{
    if (l == NULL || t == NULL) return;

    struct tmr **pp = &l->head;
    uint32_t     acc = 0u;

    while (*pp != NULL && (acc + (*pp)->delta) <= delay) {
        acc += (*pp)->delta;
        pp = &(*pp)->next;
    }

    t->delta = delay - acc;
    t->next  = *pp;
    if (*pp != NULL) {
        (*pp)->delta -= t->delta;      /* the follower's delta shrinks */
    }
    *pp = t;
}

/* Called once per tick from the timer ISR — O(1) in the common case. */
void tl_tick(struct tl *l)
{
    if (l == NULL || l->head == NULL) return;

    if (l->head->delta > 0u) {
        l->head->delta--;              /* ONE decrement, not one per timer */
        return;
    }

    while (l->head != NULL && l->head->delta == 0u) {
        struct tmr *t = l->head;
        l->head = t->next;
        t->next = NULL;
        if (t->cb != NULL) t->cb(t->arg);      /* see warning */
    }
}
```

**The delta encoding is the entire idea.** Each timer stores its delay *relative to its predecessor*, so a tick decrements exactly one counter — the head — no matter how many timers are pending. Storing absolute expiry times instead means walking the whole list every tick, which at 1 kHz with 50 timers is 50,000 comparisons per second inside an ISR.

Cost profile: O(1) per tick, O(n) to insert. That is the right trade, because ticks vastly outnumber insertions.

The `struct tmr **pp` walk is entry 8's technique again — insertion at the head needs no special case. And the follower's delta must be **reduced** by the new timer's delta, or every timer after the insertion point fires late. That line is the one people forget.

> [!WARNING]
> **Calling `cb()` from the tick ISR is a design decision, not a detail.** The callback runs at interrupt priority, so it must not block, must not allocate, and must not call an RTOS API that is task-only. It also must not re-enter `tl_add` for the same list without care, since the list is mid-modification. The safer pattern is for `tl_tick` to move expired timers onto a "fired" queue and let a task drain it — the two-half handler split from Part 07.

A **timing wheel** is the alternative when timer counts get large: bucket timers by `expiry % N` into an array of lists, so insertion is O(1) too. Worth naming as the scaling answer; the delta list is correct for the tens-of-timers case that firmware actually has.
</details>

---

### 18. Hash table with open addressing `SEN`

```c
void ht_init(struct ht *h);
bool ht_put(struct ht *h, uint32_t key, uint32_t val);
bool ht_get(const struct ht *h, uint32_t key, uint32_t *val);
bool ht_del(struct ht *h, uint32_t key);
```

<details><summary>Solution</summary>

```c
#define HT_CAP  64u                  /* power of two */
#define HT_MASK (HT_CAP - 1u)

enum slot_state { SLOT_EMPTY = 0, SLOT_USED = 1, SLOT_DEAD = 2 };

struct ht {
    uint32_t key[HT_CAP];
    uint32_t val[HT_CAP];
    uint8_t  state[HT_CAP];
    size_t   used;                   /* USED + DEAD, for load factor */
};

static uint32_t hash32(uint32_t x)   /* Knuth multiplicative */
{
    return x * 2654435761UL;
}

void ht_init(struct ht *h)
{
    if (h == NULL) return;
    for (size_t i = 0u; i < HT_CAP; i++) h->state[i] = SLOT_EMPTY;
    h->used = 0u;
}

bool ht_put(struct ht *h, uint32_t key, uint32_t val)
{
    if (h == NULL || h->used >= (HT_CAP - (HT_CAP / 4u))) return false;  /* 75% cap */

    size_t i = hash32(key) & HT_MASK;

    for (size_t probe = 0u; probe < HT_CAP; probe++) {
        if (h->state[i] == SLOT_USED && h->key[i] == key) {
            h->val[i] = val;                     /* update in place */
            return true;
        }
        if (h->state[i] != SLOT_USED) {          /* EMPTY or DEAD: reuse */
            if (h->state[i] == SLOT_EMPTY) h->used++;
            h->state[i] = SLOT_USED;
            h->key[i]   = key;
            h->val[i]   = val;
            return true;
        }
        i = (i + 1u) & HT_MASK;                  /* linear probe */
    }
    return false;
}

bool ht_get(const struct ht *h, uint32_t key, uint32_t *val)
{
    if (h == NULL || val == NULL) return false;

    size_t i = hash32(key) & HT_MASK;

    for (size_t probe = 0u; probe < HT_CAP; probe++) {
        if (h->state[i] == SLOT_EMPTY) return false;        /* stop: never occupied */
        if (h->state[i] == SLOT_USED && h->key[i] == key) {
            *val = h->val[i];
            return true;
        }
        i = (i + 1u) & HT_MASK;                             /* DEAD: keep going */
    }
    return false;
}

bool ht_del(struct ht *h, uint32_t key)
{
    if (h == NULL) return false;

    size_t i = hash32(key) & HT_MASK;

    for (size_t probe = 0u; probe < HT_CAP; probe++) {
        if (h->state[i] == SLOT_EMPTY) return false;
        if (h->state[i] == SLOT_USED && h->key[i] == key) {
            h->state[i] = SLOT_DEAD;         /* tombstone, NOT empty */
            return true;
        }
        i = (i + 1u) & HT_MASK;
    }
    return false;
}
```

**Open addressing, not chaining** — chaining needs a node allocation per collision, which is the thing firmware cannot do. Flat arrays, fixed capacity, no pointers.

**The tombstone is the whole question, and `ht_del` is where candidates fail.** Marking a deleted slot `EMPTY` breaks every lookup whose probe sequence passed through it: `ht_get` stops at the first `EMPTY`, concludes the key is absent, and returns false for a key that is sitting three slots further along. `SLOT_DEAD` means "not here, but keep probing" — and `ht_put` may reuse it while `ht_get` must probe past it. That asymmetry is the answer.

Three supporting details:

- **Power-of-two capacity plus a mask**, so the modulo is a bit operation. A prime capacity spreads better with a weak hash but costs a division.
- **A load-factor cap.** Linear probing degrades to a linear scan as the table fills; refusing inserts above ~75% keeps probe counts short. Tombstones count toward the load, which is why a delete-heavy workload eventually needs a rehash.
- **Separate parallel arrays** rather than an array of structs — keeps the `state` scan dense in cache, and avoids padding a 4+4+1 struct out to 12 bytes.
</details>

---

### 19. LRU cache, fixed size `STA`

```c
bool lru_get(struct lru *c, uint32_t key, uint32_t *val);
void lru_put(struct lru *c, uint32_t key, uint32_t val);
```

<details><summary>Solution</summary>

```c
#define LRU_CAP 8
#define LRU_NIL 0xFFu

struct lru_entry {
    uint32_t key, val;
    uint8_t  prev, next;         /* indices into e[] */
    bool     valid;
};

struct lru {
    struct lru_entry e[LRU_CAP];
    uint8_t          mru, lru;   /* head and tail of the recency list */
    uint8_t          count;
};

/* Unlink idx from the recency list. */
static void lru_unlink(struct lru *c, uint8_t i)
{
    if (c->e[i].prev != LRU_NIL) c->e[c->e[i].prev].next = c->e[i].next;
    else                         c->mru = c->e[i].next;

    if (c->e[i].next != LRU_NIL) c->e[c->e[i].next].prev = c->e[i].prev;
    else                         c->lru = c->e[i].prev;
}

/* Push idx to the front (most recently used). */
static void lru_push_front(struct lru *c, uint8_t i)
{
    c->e[i].prev = LRU_NIL;
    c->e[i].next = c->mru;
    if (c->mru != LRU_NIL) c->e[c->mru].prev = i;
    c->mru = i;
    if (c->lru == LRU_NIL) c->lru = i;
}

bool lru_get(struct lru *c, uint32_t key, uint32_t *val)
{
    if (c == NULL || val == NULL) return false;

    for (uint8_t i = 0u; i < LRU_CAP; i++) {
        if (c->e[i].valid && c->e[i].key == key) {
            *val = c->e[i].val;
            lru_unlink(c, i);                /* touching it makes it newest */
            lru_push_front(c, i);
            return true;
        }
    }
    return false;
}

void lru_put(struct lru *c, uint32_t key, uint32_t val)
{
    if (c == NULL) return;

    uint32_t dummy;
    if (lru_get(c, key, &dummy)) {           /* existing key: update, already MRU */
        c->e[c->mru].val = val;
        return;
    }

    uint8_t i;
    if (c->count < LRU_CAP) {
        i = c->count++;                      /* still filling */
    } else {
        i = c->lru;                          /* evict the least recently used */
        lru_unlink(c, i);
    }

    c->e[i].key   = key;
    c->e[i].val   = val;
    c->e[i].valid = true;
    lru_push_front(c, i);
}
```

Two structures over one array: a lookup, and a **recency-ordered doubly linked list built from indices**. Every access unlinks the entry and pushes it to the front, so the tail is by definition the eviction candidate — O(1) eviction with no scanning for a minimum timestamp.

**Indices, not pointers** — 1 byte per link instead of 4, and the whole cache is memcpy-able. Same argument as entry 13.

The honest caveat, which is what an interviewer is listening for: **lookup here is O(n)**, a linear scan over 8 entries. For a small cache that is faster than hashing, because 8 comparisons in cache beat a hash plus a probe. Above roughly 32 entries, add the hash table from entry 18 to map key → index and the whole thing becomes O(1) — that combination *is* the textbook LRU. State the crossover rather than reaching for the complex version immediately.

Where it earns its place in firmware: an address-translation or sector cache in a flash file system, or a small ARP/neighbour table.
</details>

---

### 20. Search and sort for small n `BEG`→`SEN`

```c
int  bsearch_lower(const int *a, size_t n, int key);
void insertion_sort(int *a, size_t n);
```

<details><summary>Solution</summary>

```c
/* Half-open [lo, hi). Returns index, or -1. */
int bsearch_lower(const int *a, size_t n, int key)
{
    if (a == NULL) return -1;

    size_t lo = 0u, hi = n;
    while (lo < hi) {
        size_t mid = lo + ((hi - lo) / 2u);        /* NOT (lo + hi) / 2 */
        if (a[mid] == key)      return (int)mid;
        else if (a[mid] < key)  lo = mid + 1u;
        else                    hi = mid;
    }
    return -1;
}

void insertion_sort(int *a, size_t n)
{
    if (a == NULL) return;

    for (size_t i = 1u; i < n; i++) {
        int    key = a[i];
        size_t j   = i;
        while (j > 0u && a[j - 1u] > key) {
            a[j] = a[j - 1u];
            j--;
        }
        a[j] = key;
    }
}
```

**`lo + (hi - lo) / 2`, never `(lo + hi) / 2`.** The latter overflows once the indices are large — the bug that sat in the Java standard library for nine years and in `bsearch` implementations for longer. On a 32-bit MCU with small arrays it will not bite, but naming it is free and it is the detail interviewers check.

The half-open interval `[lo, hi)` is what makes the loop condition `lo < hi` and the update `hi = mid` (not `mid - 1`), which removes the off-by-one that closed intervals invite.

**Insertion sort is the right sort here**, and defending that is the actual question. It is O(n²), and for the array sizes firmware sorts — 8 calibration points, 20 timer entries, 32 CAN filter IDs — it beats quicksort outright: no recursion, no stack depth, ~10 lines, stable, adaptive (O(n) on nearly-sorted input, which sensor data usually is), and in-place. Real library sorts fall back to it below about 16 elements for exactly these reasons.

Reach for quicksort or heapsort only when *n* is genuinely large, and then say what you are giving up: quicksort's O(log n) recursion is stack you may not have, and it is not stable. Heapsort (entry 15) is in-place, iterative and O(n log n) worst case — usually the better firmware answer than quicksort.
</details>

---

## Part 05 — retention table

| Pattern | Where it applies |
|---|---|
| Free-running indices, mask to slot | ring buffer; `head - tail` is wrap-safe occupancy |
| One writer per shared variable | the entire basis of lock-free SPSC |
| Barrier between payload and index | producer must publish data before the index |
| `struct node **pp` walk | removal with no head special case |
| Sentinel node | branch-free doubly linked list removal |
| `container_of` | intrusive lists — zero allocation, two lists per object |
| Free list threaded through unused slots | static pool; O(1) alloc and free, no fragmentation |
| Indices instead of pointers | smaller, relocatable, bounds-checkable |
| Delta encoding | timer list — O(1) per tick regardless of timer count |
| Tombstone, not empty | open-addressed delete; empty breaks the probe chain |
| Sequence number as tie-break | stability in a heap-based priority queue |

**The three questions to ask before writing any of these:** where do the nodes come from, which contexts touch it, and what happens when it is full.

---

# Part 07 — Interrupts and Concurrency

12 entries from 30. Highest value per problem in the entire bank: this is where the hardest bugs in firmware live, and where an interviewer can tell in two questions whether someone has debugged concurrent code or only read about it.

> [!IMPORTANT]
> **The three facts everything here rests on.**
> 1. **An ISR is a second thread of execution that you did not schedule.** It can land between any two machine instructions, including in the middle of a `x++`.
> 2. **`volatile` prevents compiler caching. It does not provide atomicity and it is not a barrier.** Two separate guarantees, and conflating them is the most common misunderstanding in embedded C.
> 3. **On Cortex-M, aligned 32-bit loads and stores are atomic. Nothing wider is, and no read-modify-write is** — including `++`, `+=`, `|=` and every bit-field write.

---

### 1. Critical section, with nesting `BEG`

```c
uint32_t crit_enter(void);
void     crit_exit(uint32_t saved);
```

<details><summary>Solution</summary>

```c
static inline uint32_t crit_enter(void)
{
    uint32_t saved = __get_PRIMASK();     /* remember whether IRQs were already off */
    __disable_irq();
    __DSB();
    __ISB();
    return saved;
}

static inline void crit_exit(uint32_t saved)
{
    __DSB();
    if ((saved & 1u) == 0u) {             /* only re-enable if WE disabled them */
        __enable_irq();
    }
}

/* Usage — the saved value must be a local, never a global. */
void update_shared(void)
{
    uint32_t s = crit_enter();
    g_shared_a++;
    g_shared_b--;
    crit_exit(s);
}
```

**Save and restore; never unconditionally `__enable_irq()` on exit.** That is the whole question. An unconditional enable inside a nested critical section re-enables interrupts while the *outer* section still believes it holds exclusivity — and the bug appears only when the two functions happen to be called in that order, so it survives testing.

The saved state must live in a local (on the stack, per call) rather than a global, or nesting from two contexts corrupts it.

**`PRIMASK` versus `BASEPRI`** is the follow-up:

| | Effect | Cost |
|---|---|---|
| `PRIMASK` | masks **all** configurable-priority interrupts | simple; adds latency to every IRQ including hard-real-time ones |
| `BASEPRI` | masks interrupts at or below a given priority | high-priority IRQs keep running — but they must then not touch the protected data |

`BASEPRI` is what an RTOS uses for its own critical sections, which is why FreeRTOS defines `configMAX_SYSCALL_INTERRUPT_PRIORITY`: interrupts above it may not call any kernel API, and in exchange their latency is unaffected. Neither masks `NMI` or `HardFault`.

**Keep the section short.** Every cycle inside it is added directly to worst-case interrupt latency for the whole system. No loops of unknown length, no function calls you have not audited, never a blocking wait.
</details>

---

### 2. Atomic read-modify-write `INT`→`SEN`

```c
uint32_t atomic_inc(volatile uint32_t *p);
uint32_t atomic_add_fetch(volatile uint32_t *p, uint32_t v);
bool     atomic_cas(volatile uint32_t *p, uint32_t expect, uint32_t desired);
```

<details><summary>Solution</summary>

```c
uint32_t atomic_inc(volatile uint32_t *p)
{
    uint32_t v;
    do {
        v = __LDREXW(p) + 1u;
    } while (__STREXW(v, p) != 0u);       /* 0 == store succeeded */
    return v;
}

uint32_t atomic_add_fetch(volatile uint32_t *p, uint32_t add)
{
    uint32_t v;
    do {
        v = __LDREXW(p) + add;
    } while (__STREXW(v, p) != 0u);
    return v;
}

bool atomic_cas(volatile uint32_t *p, uint32_t expect, uint32_t desired)
{
    if (__LDREXW(p) != expect) {
        __CLREX();                        /* MUST release the monitor on the early exit */
        return false;
    }
    return __STREXW(desired, p) == 0u;
}
```

`counter++` compiles to load, add, store. An ISR landing between the load and the store updates the counter, the interrupted code then writes back its stale value, and the ISR's increment is **lost**. This is the canonical embedded race and it produces a counter that drifts slowly low under load — almost impossible to attribute after the fact.

`LDREX` tags the address in the exclusive monitor; `STREX` writes only if nothing touched it since, returning non-zero on failure so the loop retries with a fresh value.

Four things that get probed:

- **`__CLREX()` on the early return in `atomic_cas`.** Leaving the monitor armed can cause an unrelated later `STREX` to behave unexpectedly. Easy to forget, and the reason CAS is tagged senior while increment is not.
- **Any exception return clears the monitor**, so `STREX` fails and retries. Correct, but it means the loop is not wait-free under heavy interrupt load.
- **Cortex-M0/M0+ (ARMv6-M) has no `LDREX`/`STREX`.** There, a critical section is the only option — which is why portable code hides this behind an OSAL layer.
- **Keep the body minimal.** Every instruction between `LDREX` and `STREX` widens the window for a spurious failure.

Modern equivalent: `atomic_fetch_add(&x, 1)` from `<stdatomic.h>`, which lowers to exactly this loop. Write the loop, then name the builtin.
</details>

---

### 3. 64-bit counter shared with an ISR `SEN`

```c
uint64_t micros_read(void);          /* task context */
void     TIM2_IRQHandler(void);      /* ISR increments */
```

<details><summary>Solution</summary>

```c
static volatile uint64_t g_micros;              /* written only by the ISR */

void TIM2_IRQHandler(void)
{
    TIM2->SR = ~TIM_SR_UIF;                     /* W1C — see Part 02 entry 4 */
    g_micros += 1000u;                          /* ISR is the only writer */
}

/* WRONG on a 32-bit core: two loads, and the ISR can land between them. */
uint64_t micros_read_broken(void) { return g_micros; }

/* Option A — critical section. Simple, adds a few cycles of IRQ latency. */
uint64_t micros_read(void)
{
    uint32_t s = crit_enter();
    uint64_t v = g_micros;
    crit_exit(s);
    return v;
}

/* Option B — retry loop. No interrupts disabled at all. */
uint64_t micros_read_lockfree(void)
{
    const volatile uint32_t *w = (const volatile uint32_t *)&g_micros;
    uint32_t hi1, lo, hi2;
    do {
        hi1 = w[1];                              /* little-endian layout */
        lo  = w[0];
        hi2 = w[1];
    } while (hi1 != hi2);
    return ((uint64_t)hi1 << 32) | lo;
}
```

**`volatile` does not make a 64-bit access atomic**, and that is the entire point. On a 32-bit core `g_micros` is two words. Read low-then-high with a carry landing in between and you get the old high word with the new low word — a timestamp that jumps backwards by 2³². Every elapsed-time calculation downstream then produces a garbage interval, and it happens once every 2³² microseconds: roughly every 71 minutes, which is frequent enough to matter and rare enough to be unreproducible.

Identical in structure to reading a hardware 64-bit counter (Part 02 entry 5) — same bug, same two fixes.

Which to choose: the critical section is clearer and correct everywhere; the retry loop adds no interrupt latency, which matters if this is called from a hot path. Note the retry loop depends on the ISR being the **only writer** and on 32-bit word accesses being atomic, both of which hold on Cortex-M.

The general rule to state: **anything wider than the machine word, shared across contexts, needs either a lock or a consistency check.** That includes a `struct { uint32_t x, y; }` where both fields must agree.
</details>

---

### 4. Flag handoff from ISR to task `INT`

```c
volatile bool g_ready;
uint8_t g_data[64];
```

<details><summary>Solution</summary>

```c
static volatile bool g_ready;              /* volatile is MANDATORY */
static uint8_t       g_data[64];           /* payload need not be volatile */

void DMA1_Channel1_IRQHandler(void)
{
    DMA1->IFCR = DMA_IFCR_CTCIF1;          /* clear first */
    /* g_data was filled by the DMA engine */
    __DMB();                               /* payload visible BEFORE the flag */
    g_ready = true;
}

void main_loop(void)
{
    for (;;) {
        if (g_ready) {
            __DMB();                       /* do not read payload before the flag */
            process(g_data, sizeof g_data);
            g_ready = false;               /* consume */
        }
    }
}
```

**Two independent requirements, and candidates usually name only the first.**

`volatile` on `g_ready` stops the compiler hoisting the read out of the loop. Without it, `if (g_ready)` is evaluated once, found false, and the loop spins forever — the same defect as the `delay_ms` loop in Part 02 entry 10.

**The barrier ordering the payload against the flag is the part that gets missed.** The producer must make `g_data` visible *before* `g_ready` becomes true, and the consumer must not speculatively read `g_data` before observing the flag. `volatile` orders the accesses the compiler emits; it says nothing about the write buffer or a store that is still in flight. `__DMB()` is free on Cortex-M and mandatory on anything with a store buffer or multiple cores.

Note `g_data` itself is **not** `volatile`. Marking a whole payload buffer volatile defeats every `memcpy` optimisation for no benefit — the flag is the synchronisation point, and one barrier covers the buffer.

The weakness of this pattern: **one flag holds exactly one event.** If a second DMA completion arrives before the task consumes the first, it is silently lost. That is why a ring buffer (Part 05 entry 3) or a counting semaphore is the correct structure whenever events can queue.
</details>

---

### 5. Two-half handler — deferring work out of the ISR `INT`

```c
void UART1_IRQHandler(void);      /* top half  */
void uart_task(void);             /* bottom half */
```

<details><summary>Solution</summary>

```c
/* TOP HALF: minimal. Move bytes, clear flags, leave. */
void UART1_IRQHandler(void)
{
    uint32_t sr = USART1->ISR;                 /* read ONCE into a local */

    if (sr & USART_ISR_RXNE) {
        (void)rb_write(&g_rx, (uint8_t)USART1->RDR);   /* read RDR clears RXNE */
    }
    if (sr & USART_ISR_ORE) {
        USART1->ICR = USART_ICR_ORECF;         /* must clear or the link is dead */
        g_stats.overruns++;
    }
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;        /* schedule the bottom half */
}

/* BOTTOM HALF: runs at lowest priority, may take as long as it needs. */
void PendSV_Handler(void)
{
    uint8_t b;
    while (rb_read(&g_rx, &b)) {
        protocol_feed(b);                      /* parsing, CRC, dispatch, logging */
    }
}
```

**Everything expensive belongs outside the ISR.** While a handler runs, every equal or lower priority interrupt is blocked — so a 200 µs parse inside the UART ISR adds 200 µs to the worst-case latency of every other interrupt in the system, including the motor-control timer that needed 50 µs.

The split: the top half captures, the bottom half interprets. Linux calls this top-half/bottom-half; Windows calls it a deferred procedure call. On Cortex-M, `PendSV` at the **lowest** priority is the hardware-supported mechanism — it is exactly how RTOS context switches are implemented, and setting `PENDSVSET` is a single register write.

**What must never appear in an ISR**, and each has a specific reason:

| Forbidden | Why |
|---|---|
| `printf` | non-reentrant, takes a lock, blocks on the UART for milliseconds |
| `malloc`/`free` | non-reentrant; heap lock deadlocks against a task holding it |
| A busy delay | blocks every lower-priority interrupt for the duration |
| A blocking RTOS call | there is no task to block; most kernels fault or corrupt |
| Any unbounded loop | latency becomes unanalysable |

Note the single read of `ISR` into a local. Reading a status register twice can return two different values, and on a read-to-clear register the second read consumes a flag.
</details>

---

### 6. Debounce a button in an ISR `INT`

```c
void EXTI0_IRQHandler(void);
```

<details><summary>Solution</summary>

```c
#define DEBOUNCE_MS 20u

static volatile uint32_t g_last_edge;
static volatile bool     g_button_event;

void EXTI0_IRQHandler(void)
{
    EXTI->PR = EXTI_PR_PR0;                    /* W1C: clear the pending edge */

    uint32_t now = g_ticks;                    /* 1 kHz tick, 32-bit: atomic read */

    if ((uint32_t)(now - g_last_edge) < DEBOUNCE_MS) {
        return;                                /* inside the dead time: ignore */
    }
    g_last_edge    = now;
    g_button_event = true;                     /* task picks this up */
}
```

**Never delay inside the ISR.** The naive fix — `EXTI0_IRQHandler` calling `delay_ms(20)` — blocks every lower-priority interrupt for 20 ms, which on a system also running a UART at 115200 baud loses about 230 bytes. It is the wrong answer even though it "works" on the bench.

Timestamp comparison instead: record when the last accepted edge occurred and reject anything inside the dead window. Cost is a subtraction.

`(uint32_t)(now - g_last_edge) < DEBOUNCE_MS` is **wrap-safe**; `now < g_last_edge + DEBOUNCE_MS` is not, and breaks once per counter rollover — the same identity as Part 02 entry 3, and it keeps recurring because it keeps being written wrongly.

Two follow-ups worth pre-empting:

- **A mechanical switch bounces for 5–50 ms**, and the number belongs in a `#define` with a comment naming the part, not as a bare literal.
- **The polled alternative is often better.** Sampling the pin every 10 ms in a task and requiring N consecutive agreeing samples needs no interrupt at all, cannot be swamped by a bouncing contact generating hundreds of edges, and gives a clean state machine. An interrupt storm from a dirty switch is a real failure mode — mention that the choice depends on whether you need the wake-from-sleep that only the interrupt provides.
</details>

---

### 7. Sleep until an event — the lost-wakeup race `INT`

```c
void wait_for_event(void);
```

<details><summary>Solution</summary>

```c
static volatile bool g_event;

/* BROKEN: a race window between the test and the sleep. */
void wait_for_event_broken(void)
{
    while (!g_event) {
        __WFI();          /* if the IRQ fires HERE, on some architectures the wake is lost */
    }
    g_event = false;
}

/* CORRECT on Cortex-M: close the window with PRIMASK. */
void wait_for_event(void)
{
    __disable_irq();
    while (!g_event) {
        __WFI();          /* wakes on a PENDING interrupt even with PRIMASK set */
        __enable_irq();   /* let the handler actually run */
        __ISB();
        __disable_irq();
    }
    g_event = false;
    __enable_irq();
}
```

The classic race: the flag is tested, found false, and the interrupt fires *before* `WFI` executes. On an architecture where the wake condition is edge-based, the wakeup is lost and the core sleeps forever waiting for an event that already happened. In the field that is a device that stops responding once every few days.

**Cortex-M has a specific property that makes the fix clean:** `WFI` wakes on a *pending* interrupt even when `PRIMASK` is set — the handler does not run, but the sleep ends. So disabling interrupts before the test closes the window: if the interrupt arrives during the test, its pending bit is set, `WFI` returns immediately, and enabling interrupts then lets the handler run and set the flag.

This is the pattern behind every correct tickless-idle implementation, and getting it wrong is why some low-power designs hang overnight.

Two things to add unprompted:

- **`WFE` and the event register** behave differently — the event latch makes `WFE` naturally race-tolerant, but it can also be set by unrelated events, so it needs the loop anyway.
- **In an RTOS, use the kernel's blocking primitive instead.** `xSemaphoreTake(sem, portMAX_DELAY)` handles this race internally, and hand-rolling `WFI` inside an RTOS task fights the scheduler.
</details>

---

### 8. ISR-safe logging `SEN`

```c
void isr_log(uint8_t event_id, uint32_t arg);
void log_drain(void);
```

<details><summary>Solution</summary>

```c
struct log_rec {
    uint32_t ts;              /* cycle counter or tick — cheap to capture */
    uint32_t arg;
    uint8_t  id;
};

#define LOG_N 64u                        /* power of two */
static struct log_rec   g_log[LOG_N];
static volatile uint16_t g_log_head;     /* producer: ISR */
static volatile uint16_t g_log_tail;     /* consumer: task */
static volatile uint32_t g_log_dropped;

/* Called from ISR context. No formatting, no locks, no blocking. */
void isr_log(uint8_t id, uint32_t arg)
{
    uint16_t h = g_log_head;

    if ((uint16_t)(h - g_log_tail) >= LOG_N) {
        g_log_dropped++;                 /* record the loss, never block */
        return;
    }
    g_log[h & (LOG_N - 1u)].ts  = DWT->CYCCNT;
    g_log[h & (LOG_N - 1u)].id  = id;
    g_log[h & (LOG_N - 1u)].arg = arg;
    __DMB();
    g_log_head = h + 1u;
}

/* Task context: formatting and I/O happen HERE. */
void log_drain(void)
{
    while (g_log_tail != g_log_head) {
        const struct log_rec *r = &g_log[g_log_tail & (LOG_N - 1u)];
        printf("[%lu] evt=%u arg=%lu\n",
               (unsigned long)r->ts, r->id, (unsigned long)r->arg);
        __DMB();
        g_log_tail++;
    }
    uint32_t d = g_log_dropped;
    if (d != 0u) { printf("dropped %lu\n", (unsigned long)d); g_log_dropped -= d; }
}
```

**`printf` in an ISR is the bug this problem exists to prevent.** It is non-reentrant, it takes an internal lock, it can allocate, and it blocks on the UART for milliseconds. Called from an ISR while a task is mid-`printf`, it either deadlocks on that lock or interleaves output into garbage. It also destroys the timing you were trying to measure.

The fix is the SPSC ring buffer from Part 05 entry 3 carrying **binary records, not strings**. Formatting is the expensive part, so it moves to the consumer.

Three details that make this a senior answer:

- **Store an ID and arguments, not a formatted string.** A record is 12 bytes and a fixed cost; `snprintf` into the ISR is hundreds of cycles and a variable one.
- **Count drops rather than blocking.** A logger that blocks when full changes the timing of the system it is observing — the observer effect, and the reason a "debug build only" hang is so often the logger itself.
- **Timestamp with `DWT->CYCCNT`**, a single register read at cycle resolution. Capturing it inside the ISR is what makes the log useful for latency work.
</details>

---

### 9. Signalling a task from an ISR `INT`

```c
void ADC_IRQHandler(void);
```

<details><summary>Solution</summary>

```c
/* FreeRTOS shape; the concept is identical in every kernel. */
void ADC_IRQHandler(void)
{
    ADC1->SR = ~ADC_SR_EOC;                       /* clear the flag first */

    BaseType_t woken = pdFALSE;

    xSemaphoreGiveFromISR(g_adc_sem, &woken);     /* note: ...FromISR */

    portYIELD_FROM_ISR(woken);                    /* switch NOW if a task woke */
}
```

Two things, and each is a separate failure if omitted.

**The `FromISR` variant is mandatory.** The ordinary `xSemaphoreGive` may block and manipulates kernel structures assuming task context. Calling it from an ISR corrupts the scheduler — sometimes immediately, sometimes an hour later. Every kernel has this split (`...FromISR` in FreeRTOS, `k_sem_give` being ISR-safe in Zephyr, `osSemaphoreRelease` in CMSIS-RTOS), and knowing that the distinction exists is the point.

**`portYIELD_FROM_ISR(woken)` is what makes it real-time.** Without it the woken task does not run until the next scheduler tick — up to 1 ms of latency added for no reason on a system that just went to the trouble of an interrupt. The yield sets `PendSV` so the context switch happens on the way out of the ISR.

Two supporting rules to state:

- **The ISR's priority must be numerically at or below `configMAX_SYSCALL_INTERRUPT_PRIORITY`** (i.e. logically lower or equal). A higher-priority interrupt may not call any kernel API at all — that is the deal `BASEPRI`-based critical sections make, from entry 1. Violating it is the single most common FreeRTOS crash, and `configASSERT` catches it if enabled.
- **A binary semaphore holds one signal.** If conversions can arrive faster than the task drains them, use a counting semaphore or a queue, or samples are silently lost — same limitation as the single flag in entry 4.
</details>

---

### 10. EXTI edge configuration `BEG`

```c
void exti_config_falling(uint8_t pin);
```

<details><summary>Solution</summary>

```c
void exti_config_falling(uint8_t pin)
{
    if (pin > 15u) return;

    uint32_t m = 1UL << pin;

    EXTI->RTSR &= ~m;                     /* rising  disabled */
    EXTI->FTSR |=  m;                     /* falling enabled  */

    EXTI->PR    =  m;                     /* clear any stale pending edge (W1C) */
    EXTI->IMR  |=  m;                     /* unmask the line LAST */

    NVIC_ClearPendingIRQ(EXTI0_IRQn + pin);   /* also clear at the NVIC */
    NVIC_SetPriority(EXTI0_IRQn + pin, 5u);
    NVIC_EnableIRQ(EXTI0_IRQn + pin);
}
```

The code is short; the **ordering** is the question.

**Clear pending before unmasking.** Configuring the trigger can itself latch an edge in `EXTI->PR`, and the NVIC may already hold a pending bit from before. Unmask first and you take a spurious interrupt immediately on a pin nothing has touched — which then gets misdiagnosed as electrical noise and "fixed" with a capacitor.

**Unmask last, enable in the NVIC last.** Everything must be configured before the line can fire.

The other half is the two flags that trip people, and it is worth stating because it is Part 02 entry 4 in a different costume: `EXTI->PR` is **write-1-to-clear**, so `EXTI->PR |= m` clears every other pending line as well — losing an unrelated interrupt. Write the mask directly.

Follow-ups: on STM32, `SYSCFG->EXTICR` selects *which port* pin N is routed from, so PA0 and PB0 share `EXTI0` and cannot both be used. And on the older families several lines share one IRQ (`EXTI9_5_IRQn`), so the handler must test `PR` to discover which pin fired and clear only that bit.
</details>

---

### 11. Ping-pong DMA with half-transfer `SEN`

```c
void DMA1_Stream0_IRQHandler(void);
```

<details><summary>Solution</summary>

```c
#define BUF_N 512u                              /* total; each half is BUF_N/2 */
static uint16_t g_adc_buf[BUF_N];               /* circular DMA target */

void DMA1_Stream0_IRQHandler(void)
{
    if (DMA1->LISR & DMA_LISR_HTIF0) {          /* half transfer complete */
        DMA1->LIFCR = DMA_LIFCR_CHTIF0;
        /* First half is full and stable; DMA is now writing the second half. */
        process_half(&g_adc_buf[0], BUF_N / 2u);
    }

    if (DMA1->LISR & DMA_LISR_TCIF0) {          /* transfer complete */
        DMA1->LIFCR = DMA_LIFCR_CTCIF0;
        /* Second half is full; DMA has wrapped to the first. */
        process_half(&g_adc_buf[BUF_N / 2u], BUF_N / 2u);
    }

    if (DMA1->LISR & DMA_LISR_TEIF0) {          /* transfer error */
        DMA1->LIFCR = DMA_LIFCR_CTEIF0;
        g_stats.dma_errors++;
        dma_restart();
    }
}
```

Circular DMA plus the half-transfer interrupt gives **continuous acquisition with no gaps**: the CPU always works on the half the DMA engine is not touching, so there is no need to stop and restart, and no sample is missed. This is how audio, high-rate ADC and UART bulk receive are actually done.

**The real-time constraint is the answer to "what can go wrong".** `process_half` must complete before the DMA fills the other half. Take longer and the engine overwrites data you are still reading, silently — no flag, no fault, just corrupted output. The budget is `(BUF_N/2) / sample_rate`, and it should be written in a comment next to the buffer.

Two things that separate people who have shipped this:

- **Handle the error flag.** A DMA transfer error leaves the stream disabled and the interrupt silent; without the `TEIF` branch the system simply stops acquiring, and the symptom is "it worked for six hours".
- **Cache maintenance on Cortex-M7.** With the D-cache enabled, DMA writes to SRAM are invisible to the CPU until the line is invalidated, and CPU writes may sit in the cache where the DMA engine cannot see them. `SCB_InvalidateDCache_by_Addr()` before reading a DMA-filled buffer, `SCB_CleanDCache_by_Addr()` after filling a buffer for transmit — and the buffer must be cache-line aligned and a multiple of 32 bytes, or invalidating one buffer corrupts its neighbour. This is the single most common M7 bring-up bug and it does not exist on M4, which is why code ports badly upward.

`g_adc_buf` need not be `volatile` — the interrupt is the synchronisation point. It **must** be in a memory region the DMA controller can reach, which on some parts excludes CCM/DTCM RAM.
</details>

---

### 12. Measure interrupt latency `SEN`

```c
uint32_t isr_latency_cycles(void);
```

<details><summary>Solution</summary>

```c
static volatile uint32_t g_t_trigger;
static volatile uint32_t g_latency_max;

static void dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0u;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
}

/* Trigger side: stamp immediately before causing the interrupt. */
void trigger(void)
{
    g_t_trigger = DWT->CYCCNT;
    NVIC_SetPendingIRQ(EXTI0_IRQn);
}

void EXTI0_IRQHandler(void)
{
    uint32_t entry = DWT->CYCCNT;                 /* FIRST line of the handler */
    EXTI->PR = EXTI_PR_PR0;

    uint32_t d = entry - g_t_trigger;             /* wrap-safe subtraction */
    if (d > g_latency_max) g_latency_max = d;

    /* ... real work ... */
}
```

`DWT->CYCCNT` is a free-running 32-bit cycle counter — one register read, no overhead worth naming, and the correct instrument for anything under a microsecond. `TRCENA` must be set first or the counter never runs, which is the step people miss and then conclude the hardware is broken.

**Timestamp on the first line of the handler**, before clearing flags. Anything above it is measured as latency that is really your own code.

What the number is made of, and why quoting the datasheet figure alone is the wrong answer:

| Contribution | Typical Cortex-M4 |
|---|---|
| Core exception entry (stack 8 registers, fetch vector) | 12 cycles |
| Flash wait states on the vector and handler fetch | 0–10 |
| **Blocking by an equal or higher priority handler already running** | unbounded |
| **A critical section holding `PRIMASK`** | length of that section |
| Longest single instruction that cannot be abandoned | up to ~10 |

The 12-cycle figure is the *best* case. **Worst-case latency is dominated by the longest critical section and the longest higher-priority handler in the system** — which is why entry 1 insists on keeping sections short and entry 5 insists on moving work out of ISRs. Measuring the maximum over hours of realistic load, not the average, is what makes the number meaningful.

Two extras: `-O0` versus `-O2` changes this materially, so measure the build you ship; and toggling a GPIO on entry and exit lets a logic analyser show the same thing externally, which is how you catch the case where the ISR runs but the *system* is still late.
</details>

---

## Part 07 — retention table

| Rule | Consequence if broken |
|---|---|
| Save/restore IRQ state, don't blindly enable | nested critical section silently opens |
| `x++` on shared data is not atomic | lost increments, counter drifts low under load |
| 64-bit or multi-field shared data needs a lock or retry | timestamp jumps backwards once per wrap |
| `volatile` ≠ atomic ≠ barrier | three different bugs, one misunderstanding |
| Barrier between payload and flag | consumer reads a buffer that isn't filled yet |
| ISR does capture only; task does interpretation | one slow ISR adds latency to every other IRQ |
| Never `printf`, `malloc`, delay or block in an ISR | deadlock, corruption, or unanalysable timing |
| Debounce by timestamp, never by delay | 20 ms delay in an ISR loses 230 UART bytes |
| Disable IRQs before test-then-`WFI` | lost wakeup, device hangs after days |
| `FromISR` API + yield-from-ISR | scheduler corruption, or 1 ms added latency |
| Clear pending before unmasking | spurious interrupt misdiagnosed as noise |
| Invalidate/clean D-cache around DMA on M7 | works on M4, fails on M7 — the classic port bug |

**The question to ask about any variable:** which contexts touch it, how wide is it, and is every access to it atomic? If you cannot answer all three, it is a bug waiting for load.

---

# Part 13 — Debugging: Fix the Bug

10 entries from 26. Different format, because these are asked differently: you are shown broken code and asked what is wrong with it.

**Why this part matters more than its size suggests.** "Tell me about a bug you fixed" is the question that separates candidates, and it cannot be answered from written notes — the answer has to come from having chased something. These ten are the root causes that account for most real firmware defects, and each one is worth being able to describe as a *story*: symptom first, then how you localised it, then the fix. Rehearse them in that order.

Each entry gives the symptom before the cause, because that is the direction you meet them in.

---

### 1. The loop that never exits `INT`

```c
bool g_flag;

void EXTI0_IRQHandler(void) { g_flag = true; }

void wait(void)
{
    while (!g_flag) { }
    g_flag = false;
}
```

**Symptom:** hangs in `wait()` forever. Works perfectly at `-O0`. Works if you add a `printf` inside the loop.

<details><summary>Root cause and fix</summary>

**`g_flag` is not `volatile`.** Inside `wait()` the compiler sees no write to `g_flag`, so caching it in a register and testing that register forever is a legal and desirable optimisation. It is not a compiler bug — the compiler's model of the program is correct; the program lied about what can modify that memory.

```c
static volatile bool g_flag;
```

**Why the two clues point straight at it.** `-O0` disables the optimisation, so the bug vanishes — any bug that appears only with optimisation on is a `volatile`, aliasing, or undefined-behaviour bug. Adding `printf` forces a reload because the compiler cannot prove the call does not touch `g_flag`, which is the same mechanism wearing a disguise.

**Where `volatile` is required:** memory-mapped registers, anything an ISR modifies, anything shared between tasks, and the counter in a busy-wait loop.

**What it does not do**, and the follow-up that always comes: it is not atomic and not a barrier. It would not save you in entries 2 or 3 below.
</details>

---

### 2. The counter that drifts low `SEN`

```c
volatile uint32_t g_events;

void TIM2_IRQHandler(void) { g_events++; }

void task(void)
{
    if (g_events > 0u) { g_events--; handle(); }
}
```

**Symptom:** under heavy load the count slowly loses events. Totals do not reconcile. Fine on the bench, wrong in the field after an hour.

<details><summary>Root cause and fix</summary>

**`g_events++` is a read-modify-write, and `volatile` does not make it atomic.** It compiles to three instructions:

```
LDR  r0, [g_events]
ADDS r0, #1
STR  r0, [g_events]
```

The ISR can land between the task's `LDR` and `STR`. The ISR increments, the task then writes back its stale value plus one, and the ISR's increment is **lost**. Both sides modify the variable, so both sides can lose.

Two correct fixes:

```c
/* A: critical section around the task's RMW */
uint32_t s = crit_enter();
if (g_events > 0u) { g_events--; }
crit_exit(s);

/* B: atomic, no interrupts disabled */
uint32_t v;
do { v = __LDREXW(&g_events); } while (v > 0u && __STREXW(v - 1u, &g_events) != 0u);
```

**The better fix is usually structural.** Give each side its own variable so neither modifies the other's:

```c
static volatile uint32_t g_produced;    /* ISR increments only  */
static uint32_t          g_consumed;    /* task increments only */
/* outstanding == (uint32_t)(g_produced - g_consumed) */
```

Now no variable has two writers, no lock is needed, and the difference is wrap-safe. That is the ring buffer's insight from Part 05 entry 3 applied to a counter — and offering it unprompted is what makes this a senior answer rather than a correct one.
</details>

---

### 3. The comparison that is always false `INT`

```c
int len = get_length();                 /* may return -1 on error */

if (len < sizeof(buf)) {
    memcpy(buf, src, len);              /* boom */
}
```

**Symptom:** buffer overflow, or a `memcpy` of about 4 GB, when `get_length()` returns −1. The bounds check appeared to be there.

<details><summary>Root cause and fix</summary>

**`sizeof` yields `size_t`, which is unsigned, so the usual arithmetic conversions convert `len` to unsigned too.** `-1` becomes `0xFFFFFFFF`, which is *not* less than `sizeof(buf)`, so the guard passes for the one input it existed to reject. Then `memcpy` receives `(size_t)-1` as its length.

```c
int len = get_length();

if (len < 0) return;                          /* handle the error FIRST */
if ((size_t)len > sizeof buf) return;         /* now the comparison is honest */
memcpy(buf, src, (size_t)len);
```

Check the error case separately, then compare in one type deliberately.

**This is the trap the `100 C Exercises` book lists as number 7 of its ten, and it catches almost everyone:** `if (-1 < sizeof(int))` is **false**. Any mixed signed/unsigned comparison converts the signed operand, and `-Wsign-compare` (inside `-Wextra`) is the only thing that reliably catches it — which is a concrete argument for building with the flags rather than the defaults.

Related instances of the same conversion rule, worth naming because they are all the same bug:

```c
for (size_t i = n - 1u; i >= 0u; i--) { }     /* never terminates: i is unsigned */
for (size_t i = 0; i < n - 1u; i++) { }       /* n == 0 → n-1 is SIZE_MAX */
if (strlen(s) - 1 > 0) { }                    /* empty string → huge value */
```
</details>

---

### 4. The struct that reads garbage over the wire `INT`

```c
struct packet {
    uint8_t  type;
    uint32_t timestamp;
    uint16_t crc;
};

void send(const struct packet *p)
{
    uart_write((const uint8_t *)p, sizeof *p);      /* 12 bytes, not 7 */
}
```

**Symptom:** the receiver decodes `timestamp` as garbage. `sizeof(struct packet)` is 12, not 7. The same code works between two identical MCUs and fails against a PC or a different compiler.

<details><summary>Root cause and fix</summary>

**The compiler inserts padding to satisfy alignment.** `uint32_t` must sit on a 4-byte boundary, so three bytes appear after `type`, and the struct is padded to a multiple of its largest member's alignment:

```
offset 0  type       (1 byte)
offset 1  ---padding--- (3 bytes)
offset 4  timestamp  (4 bytes)
offset 8  crc        (2 bytes)
offset 10 ---padding--- (2 bytes)
total 12
```

Padding bytes are **uninitialised** — they leak stack contents onto the wire, which is an information-disclosure bug as well as a protocol bug.

**`#pragma pack(1)` is the tempting fix and the wrong one.** It removes the padding, but then `p->timestamp` is an unaligned 32-bit access: a **HardFault on Cortex-M0/M0+**, and slower or fault-trapped on M3/M4. You have traded a wire-format bug for a runtime fault.

**The correct fix is explicit serialisation** — the shift-based approach from Part 01 entry 13:

```c
size_t pack_packet(const struct packet *p, uint8_t *out)
{
    out[0] = p->type;
    out[1] = (uint8_t)(p->timestamp >> 24);       /* big-endian, explicit */
    out[2] = (uint8_t)(p->timestamp >> 16);
    out[3] = (uint8_t)(p->timestamp >>  8);
    out[4] = (uint8_t)(p->timestamp);
    out[5] = (uint8_t)(p->crc >> 8);
    out[6] = (uint8_t)(p->crc);
    return 7u;
}
```

This is byte-exact, endian-independent, alignment-safe and portable across compilers. It is more code, and it is the answer that ships.

Bonus point, and a genuinely useful habit: **reordering fields largest-first eliminates most padding for free** — `uint32_t, uint16_t, uint8_t` is 8 bytes instead of 12. That is the RAM-reduction technique the Optimization section asks about separately.
</details>

---

### 5. UART that stops receiving under load `INT`

```c
void USART1_IRQHandler(void)
{
    if (USART1->ISR & USART_ISR_RXNE) {
        rb_write(&g_rx, (uint8_t)USART1->RDR);
    }
}
```

**Symptom:** works at low data rates. Under sustained traffic, reception stops completely and never recovers until reset. Not "loses a byte" — dies.

<details><summary>Root cause and fix</summary>

**The overrun error `ORE` is never cleared.** When a byte arrives before the previous one has been read, the peripheral sets `ORE` and **stops receiving**. `RXNE` no longer asserts, so the handler's only branch never runs, so nothing ever clears the error. The link is dead permanently from one missed byte.

This is the crucial distinction: an overrun does not cost you one byte, it costs you **the connection**. It explains most "worked then stopped" UART reports.

```c
void USART1_IRQHandler(void)
{
    uint32_t sr = USART1->ISR;                 /* read ONCE */

    if (sr & USART_ISR_ORE) {
        USART1->ICR = USART_ICR_ORECF;         /* clear it or the link stays dead */
        g_stats.overruns++;                    /* count it — silent recovery hides load problems */
    }
    if (sr & (USART_ISR_NE | USART_ISR_FE | USART_ISR_PE)) {
        USART1->ICR = USART_ICR_NCF | USART_ICR_FECF | USART_ICR_PECF;
        g_stats.frame_errors++;
    }
    if (sr & USART_ISR_RXNE) {
        (void)rb_write(&g_rx, (uint8_t)USART1->RDR);
    }
}
```

**Handle every error flag the peripheral can raise, not just the one you want.** That is the general lesson, and it applies to SPI `OVR`, I²C `BERR`/`ARLO`, and CAN error states equally.

Then fix the *cause*: the ISR was too slow, or the buffer too small, or interrupts were disabled too long somewhere else. Enabling DMA with idle-line detection removes the per-byte interrupt entirely and is the real answer at high baud rates. Note also that on the older STM32 families reading `SR` then `DR` is the clear sequence — read the reference manual for the part, because it differs.
</details>

---

### 6. I²C hangs on a single-byte read `SEN`

```c
/* Old-style STM32 F1/F4 I2C, reading 1 byte */
I2C1->CR1 |= I2C_CR1_ACK;                  /* ACK enabled */
i2c_send_addr(dev, I2C_READ);
while (!(I2C1->SR1 & I2C_SR1_RXNE)) { }    /* never becomes true */
data = I2C1->DR;
I2C1->CR1 |= I2C_CR1_STOP;
```

**Symptom:** multi-byte reads work. A one-byte read hangs forever, or returns a byte and leaves SDA held low so the bus is dead afterwards.

<details><summary>Root cause and fix</summary>

**A single-byte read needs `ACK` cleared and `STOP` requested *before* the byte arrives.** The controller must signal NACK on the byte it is currently receiving, and the decision has to be made while it is still on the wire — after the address phase, before the data phase. The sequence above enables ACK, so the peripheral acknowledges and waits for a second byte that the driver never intends to read.

```c
I2C1->CR1 &= ~I2C_CR1_ACK;                 /* NACK the byte we are about to get */
i2c_send_addr(dev, I2C_READ);
(void)I2C1->SR1; (void)I2C1->SR2;          /* clear ADDR by reading SR1 then SR2 */
I2C1->CR1 |= I2C_CR1_STOP;                 /* request STOP before RXNE */

uint32_t t0 = micros();
while (!(I2C1->SR1 & I2C_SR1_RXNE)) {
    if ((uint32_t)(micros() - t0) > 1000u) return -1;   /* TIMEOUT — always */
}
data = I2C1->DR;
```

**Two lessons, and the second is the one that generalises.**

The specific one: I²C 1-byte, 2-byte and N-byte reads are **three different code paths** in the old STM32 peripheral, documented in an errata-adjacent section of the reference manual that everyone skips. The 2-byte case needs `POS` set. This is the canonical example of why "read the reference manual, including the notes" is a real skill.

The general one: **`while (!(REG & FLAG));` with no timeout is the bug**, and it appears in thousands of drivers. Any hardware wait needs a bound, or one browning-out sensor hangs the product until the watchdog fires — and then you get a reset loop with no diagnosis. Every wait in a driver you write should look like Part 02 entry 3.

Recovery matters too: a hung I²C bus with SDA held low by a confused slave needs clocking out up to nine manual SCL pulses via GPIO to free it. A driver without that recovery path requires a power cycle.
</details>

---

### 7. DMA data that is stale on Cortex-M7 `STA`

```c
uint8_t rx_buf[256];

void start_receive(void)
{
    HAL_UART_Receive_DMA(&huart1, rx_buf, sizeof rx_buf);
}

void on_complete(void)
{
    parse(rx_buf);          /* reads old data, or a mix of old and new */
}
```

**Symptom:** the identical code works on an STM32F4 (Cortex-M4) and fails on an H7 (Cortex-M7). Data is partly stale, or the first bytes are correct and the rest garbage. Disabling the D-cache makes it work.

<details><summary>Root cause and fix</summary>

**The DMA engine and the CPU have different views of memory once a data cache exists.**

- DMA writes to SRAM go straight to RAM. The CPU reads a cached copy of that line and does not see them.
- CPU writes may sit in the cache (write-back), so a DMA engine reading that buffer for transmit sends stale bytes.

Cortex-M4 has no D-cache, so the bug does not exist there. This is the most common M7 bring-up failure and the reason working code breaks when ported *upward*.

```c
/* Receive: invalidate BEFORE reading, so the CPU refetches from RAM. */
void on_complete(void)
{
    SCB_InvalidateDCache_by_Addr((uint32_t *)rx_buf, sizeof rx_buf);
    parse(rx_buf);
}

/* Transmit: clean AFTER filling, so the data actually reaches RAM. */
void send(void)
{
    fill(tx_buf, n);
    SCB_CleanDCache_by_Addr((uint32_t *)tx_buf, (int32_t)n);
    HAL_UART_Transmit_DMA(&huart1, tx_buf, n);
}
```

**The alignment requirement is the trap inside the trap.** Cache maintenance operates on 32-byte lines, so the buffer must be **32-byte aligned and a multiple of 32 bytes in size** — otherwise invalidating your buffer also invalidates part of whatever shares its first or last line, discarding another variable's cached value:

```c
static uint8_t rx_buf[256] __attribute__((aligned(32)));
```

Two alternatives worth naming, both often better than sprinkling maintenance calls:

- **Put DMA buffers in a non-cacheable MPU region.** Configure once at startup, then no maintenance is ever needed. This is what most production H7 projects do.
- **Use a memory the cache does not cover**, though note the reverse constraint: DTCM is fast and uncached but **not reachable by all DMA controllers** on the H7, which is its own well-known bring-up failure.
</details>

---

### 8. Debug build deadlocks, release build does not `INT`

```c
void ADC_IRQHandler(void)
{
    ADC1->SR = ~ADC_SR_EOC;
    printf("adc=%u\n", ADC1->DR);       /* debug logging */
}
```

**Symptom:** the system freezes intermittently, only in the debug build. Sometimes output is interleaved garbage. Removing the `printf` fixes it — and so does looking at it in the debugger.

<details><summary>Root cause and fix</summary>

**`printf` is not reentrant and it blocks.** Three distinct failures, any of which is sufficient:

- It takes an internal lock on the stdout stream. If a task is mid-`printf` when the ISR fires, the ISR blocks on a lock held by code that cannot run until the ISR returns — **deadlock**.
- Many implementations call `malloc` for buffering, which has the same lock problem and a worse one: a corrupted heap.
- It blocks on the UART for **milliseconds**, inside an ISR, blocking every lower-priority interrupt for the duration.

It also destroys the timing you were probably trying to observe — a Heisenbug by construction.

The fix is Part 07 entry 8: push a binary record to a ring buffer, format in a task.

```c
void ADC_IRQHandler(void)
{
    ADC1->SR = ~ADC_SR_EOC;
    isr_log(EVT_ADC, ADC1->DR);         /* ~10 cycles, non-blocking */
}
```

**The general principle:** instrumentation must cost less than the thing it measures, or it changes the behaviour under study. Alternatives that satisfy that, in rough order of preference: SWO/ITM `printf` (a few cycles per character, hardware-buffered), SEGGER RTT (a ring buffer read out over the debug probe, no target UART at all), a GPIO toggle plus a logic analyser (zero software cost, and it shows timing directly), or a binary event log.

Also worth stating: **an `#ifdef DEBUG` around a `printf` in an ISR is not a fix.** It means the debug build has different timing from the shipped build, so the build you tested is not the build you ship.
</details>

---

### 9. Stack overflow that looks like memory corruption `SEN`

```c
void parse_node(struct node *n)
{
    char scratch[512];                  /* on the stack, per frame */
    if (n == NULL) return;
    format(scratch, sizeof scratch, n);
    parse_node(n->child);               /* recursion, unbounded depth */
}
```

**Symptom:** a global variable changes value with nothing writing to it. Or a HardFault with a nonsense PC. Or the system runs for minutes then behaves randomly. The corrupted variable moves when unrelated code is added.

<details><summary>Root cause and fix</summary>

**Recursion with a 512-byte frame overflows the stack, and on a Cortex-M without an MPU the overflow silently overwrites whatever sits below it** — usually `.bss`, i.e. your globals. There is no fault at the moment of overflow; the fault or the wrong value appears later, somewhere else. That is why "a variable changed by itself" is the classic signature, and why the symptom moves when the link order changes.

Fix the algorithm — convert to an explicit stack, which also gives a bound you can prove:

```c
void parse_all(struct node *root)
{
    static char        scratch[512];         /* ONE buffer, not one per frame */
    struct node       *stack[MAX_DEPTH];
    size_t             top = 0u;

    if (root != NULL) stack[top++] = root;

    while (top > 0u) {
        struct node *n = stack[--top];
        format(scratch, sizeof scratch, n);
        if (n->child != NULL && top < MAX_DEPTH) stack[top++] = n->child;
        if (n->next  != NULL && top < MAX_DEPTH) stack[top++] = n->next;
    }
}
```

**Depth is now bounded by `MAX_DEPTH` and enforced**, and the 512-byte buffer exists once instead of once per level.

Three defences to name, because "how would you have caught this earlier" is the follow-up:

| Technique | Catches |
|---|---|
| **MPU region with no access at the stack limit** | overflow at the instant it happens, as a fault with a usable PC |
| **Stack painting** — fill with `0xDEADBEEF` at boot, inspect the high-water mark | how much headroom you actually have |
| **`-fstack-usage`**, plus a static analyser for worst-case depth | frames too large, at compile time |

FreeRTOS's `configCHECK_FOR_STACK_OVERFLOW` does the painting check at every context switch, which is why it is worth enabling even in production.

**The rule for firmware:** no recursion on a bounded stack unless the depth is provably small and documented. Not a style preference — an 8 KB part with a 1 KB stack has no margin for a data-dependent depth.
</details>

---

### 10. The register write that does not take effect `INT`

```c
RCC->APB2ENR |= RCC_APB2ENR_GPIOAEN;    /* enable GPIOA clock */
GPIOA->MODER |= (1UL << 10);            /* configure PA5 as output */
GPIOA->ODR   |= (1UL << 5);             /* set it high */
/* nothing happens on the pin */
```

**Symptom:** the pin never changes. Adding any code between the lines — even a `printf` — makes it work. Single-stepping in the debugger works. It fails only at `-O2`.

<details><summary>Root cause and fix</summary>

**The peripheral clock is not yet running when the following register writes execute.** On STM32 the clock-enable write takes a couple of cycles to propagate through the bus matrix; writes to a peripheral whose clock is still off are silently discarded. The debugger works because stepping inserts delay. Any inserted code works for the same reason, which is what makes this look like a compiler bug.

```c
RCC->APB2ENR |= RCC_APB2ENR_GPIOAEN;
(void)RCC->APB2ENR;                     /* read back forces the write to complete */
__DSB();

GPIOA->MODER |= (1UL << 10);
GPIOA->ODR   |= (1UL << 5);
```

The read-back is the idiomatic fix and is exactly what ST's own HAL does after every clock enable — `__HAL_RCC_GPIOA_CLK_ENABLE()` expands to a set followed by a dummy read, which people often mistake for redundant code and delete.

**The general family of bugs**, all with the same shape and all worth being able to list:

| Situation | Required |
|---|---|
| Enable a peripheral clock, then configure it | read back, or `__DSB()` |
| Disable an interrupt at the peripheral, then return from the ISR | `__DSB()`, or the IRQ re-fires spuriously |
| Write `SCB->VTOR`, `CONTROL`, or `MPU->CTRL` | `__DSB()` then `__ISB()` |
| Request a self-reset via `SCB->AIRCR` | `__DSB()` then spin, or execution continues |
| Exit a critical section before a dependent access | `__DSB()` |

**`volatile` does not help here** — the write *was* emitted, in order. The problem is downstream of the CPU, in the write buffer and the bus. `volatile` constrains the compiler; barriers constrain the memory system. Being able to state that distinction cleanly is the difference between having memorised "use volatile" and understanding the machine.
</details>

---

## Part 13 — the diagnostic table

Symptom-first, because that is the direction you meet them.

| Symptom | Look first at |
|---|---|
| Works at `-O0`, fails at `-O2` | missing `volatile`, UB, strict aliasing |
| Works under the debugger, fails standalone | a timing or barrier problem — clock enable, DMA, race |
| Hangs forever in a driver | a wait with no timeout |
| Peripheral stops and never recovers | an unhandled error flag (`ORE`, `OVR`, `BERR`) |
| A global changes with nothing writing it | stack overflow, or a wild pointer |
| Counter drifts low under load | non-atomic read-modify-write on shared data |
| Bounds check passes for a negative value | signed/unsigned conversion |
| Fails only on the newer, faster part | D-cache and DMA coherency |
| Fine between two identical MCUs, fails cross-platform | struct padding, endianness |
| Reappears only in the release build | `#ifdef DEBUG` changed the timing |

**How to tell this as an interview answer:** symptom, what you measured, what it ruled out, then the fix — not "it was a missing `volatile`". The reasoning is what is being assessed; the root cause is just the punchline.

---

# Part 09 — Drivers

14 entries from 83. The bank asks for GPIO write, GPIO read, read whole port, write whole port and set drive strength as five problems; they are one problem. What is kept is **one real driver per peripheral**, chosen so that between them they cover every mechanism a driver interview probes: baud arithmetic, full-duplex shift registers, repeated start, bus recovery, DMA, capture timing, and flash programming.

> [!IMPORTANT]
> **What a driver interview is actually testing.** Not whether you memorised register names — you will have the reference manual. It is testing whether you know: what happens on error, what happens on timeout, what the initialisation order must be, and which flag means "the shift register is empty" versus "the last bit has left the pin". Those four questions decide every entry below.

Register names follow STM32F4/F7. The mechanisms are identical across vendors; only the spelling changes.

---

### 1. GPIO — configure, read, write atomically `BEG`

```c
void gpio_init_output(GPIO_TypeDef *port, uint8_t pin, bool open_drain);
void gpio_init_input(GPIO_TypeDef *port, uint8_t pin, uint8_t pull);
void gpio_write(GPIO_TypeDef *port, uint8_t pin, bool level);
bool gpio_read(const GPIO_TypeDef *port, uint8_t pin);
```

<details><summary>Solution</summary>

```c
void gpio_init_output(GPIO_TypeDef *port, uint8_t pin, bool open_drain)
{
    if (port == NULL || pin > 15u) return;

    /* Caller must have enabled the port clock already — and read it back. */
    port->MODER   = (port->MODER   & ~(3UL << (pin * 2u))) | (1UL << (pin * 2u)); /* 01 = output */
    port->OSPEEDR = (port->OSPEEDR & ~(3UL << (pin * 2u))) | (0UL << (pin * 2u)); /* low speed  */
    port->PUPDR   =  port->PUPDR   & ~(3UL << (pin * 2u));                        /* no pull    */

    if (open_drain) port->OTYPER |=  (1UL << pin);
    else            port->OTYPER &= ~(1UL << pin);
}

void gpio_init_input(GPIO_TypeDef *port, uint8_t pin, uint8_t pull)   /* 0 none,1 up,2 down */
{
    if (port == NULL || pin > 15u || pull > 2u) return;
    port->MODER = port->MODER & ~(3UL << (pin * 2u));                  /* 00 = input */
    port->PUPDR = (port->PUPDR & ~(3UL << (pin * 2u))) | ((uint32_t)pull << (pin * 2u));
}

/* ATOMIC: no read-modify-write, so it cannot race with an ISR touching another pin. */
void gpio_write(GPIO_TypeDef *port, uint8_t pin, bool level)
{
    if (port == NULL || pin > 15u) return;
    port->BSRR = level ? (1UL << pin)          /* low half sets   */
                       : (1UL << (pin + 16u)); /* high half resets */
}

bool gpio_read(const GPIO_TypeDef *port, uint8_t pin)
{
    if (port == NULL || pin > 15u) return false;
    return (port->IDR & (1UL << pin)) != 0UL;      /* IDR, not ODR */
}
```

**`BSRR` instead of `ODR |=` is the answer this problem exists for.** `ODR |= (1UL << pin)` is a read-modify-write on a register shared by sixteen pins: an ISR toggling a different pin on the same port between the read and the write has its change discarded. `BSRR` is write-only with separate set and reset halves, so it is atomic by construction — no critical section, no lost updates. Every vendor has an equivalent (`BSRR`, `BSET`/`BCLR`, `OUTSET`/`OUTCLR`); using `ODR` for anything but a whole-port write is a defect.

**Read `IDR`, not `ODR`.** `IDR` is what is physically on the pin; `ODR` is what you asked for. On an open-drain output with a pull-up, or a pin being driven by something else, they differ — and that difference is how you detect a short, an SMBus stretch, or a stuck bus (entry 7).

Two ordering points that get asked: the port clock must be enabled **and read back** before any of these writes land (Part 13 entry 10), and for a peripheral pin `MODER` must be `10` (alternate function) with the peripheral number in `AFR[pin/8]` — configuring the pin as a plain output and wondering why the UART is silent is a standard first-day mistake.
</details>

---

### 2. UART init — the baud rate calculation `INT`

```c
int uart_init(USART_TypeDef *u, uint32_t pclk_hz, uint32_t baud);
```

<details><summary>Solution</summary>

```c
int uart_init(USART_TypeDef *u, uint32_t pclk_hz, uint32_t baud)
{
    if (u == NULL || baud == 0u || pclk_hz == 0u) return -1;

    /* Rounded divisor, OVER8 = 0 (16x oversampling). */
    uint32_t div = (pclk_hz + (baud / 2u)) / baud;
    if (div < 16u || div > 0xFFFFu) return -1;        /* unachievable */

    /* Reject if the resulting error exceeds 2% — see below. */
    uint32_t actual = pclk_hz / div;
    uint32_t err_ppm = (actual > baud)
                     ? ((actual - baud) * 1000000u) / baud
                     : ((baud - actual) * 1000000u) / baud;
    if (err_ppm > 20000u) return -1;                 /* > 2% */

    u->CR1 = 0UL;                                    /* disable while configuring */
    u->BRR = div;
    u->CR2 = 0UL;                                    /* 1 stop bit */
    u->CR3 = 0UL;                                    /* no flow control */
    u->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;   /* enable LAST */

    return 0;
}
```

**The arithmetic is the question, and rounding is where it goes wrong.** `pclk / baud` truncates; adding `baud/2` before dividing rounds to nearest, which halves the worst-case error. At 8 MHz and 115200 the exact divisor is 69.44 — truncating gives 69 (117391 baud, +1.9%), rounding gives 69 as well here, but at other combinations the difference crosses the failure threshold.

**Why 2% matters.** UART has no shared clock: the receiver samples in the middle of each bit based on its own timebase. Errors accumulate across the frame, and by the stop bit of a 10-bit frame a 5% mismatch has drifted half a bit — the receiver samples the wrong bit and reports a framing error. Both ends contribute, so the practical budget is about **2% per end**. Being able to explain *why* the tolerance is what it is, rather than quoting it, is what separates answers here.

This is also the answer to "the UART outputs garbage": in order of likelihood, wrong `pclk` assumed (the clock tree was reconfigured and the driver was not told), truncation instead of rounding, or a crystal-less internal RC oscillator whose accuracy is ±1% before temperature.

`CR1` is written twice deliberately — disable, configure, enable. `UE` last, because some bits are only latched while the peripheral is disabled.
</details>

---

### 3. UART interrupt RX and TX with ring buffers `SEN`

```c
void   uart_isr(void);
size_t uart_send(const uint8_t *buf, size_t n);
```

<details><summary>Solution</summary>

```c
static struct rb g_rx, g_tx;          /* Part 05 entry 3 */

void uart_isr(void)
{
    uint32_t sr = USART1->ISR;                       /* read ONCE */

    /* --- errors first: an uncleared ORE kills reception permanently --- */
    if (sr & (USART_ISR_ORE | USART_ISR_NE | USART_ISR_FE | USART_ISR_PE)) {
        USART1->ICR = USART_ICR_ORECF | USART_ICR_NCF |
                      USART_ICR_FECF  | USART_ICR_PECF;
        g_stats.rx_errors++;
    }

    /* --- receive --- */
    if (sr & USART_ISR_RXNE) {
        uint8_t b = (uint8_t)USART1->RDR;            /* reading RDR clears RXNE */
        if (!rb_write(&g_rx, b)) g_stats.rx_dropped++;
    }

    /* --- transmit --- */
    if ((sr & USART_ISR_TXE) && (USART1->CR1 & USART_CR1_TXEIE)) {
        uint8_t b;
        if (rb_read(&g_tx, &b)) {
            USART1->TDR = b;                         /* writing TDR clears TXE */
        } else {
            USART1->CR1 &= ~USART_CR1_TXEIE;         /* nothing left: MUST disable */
        }
    }
}

size_t uart_send(const uint8_t *buf, size_t n)
{
    size_t w = rb_write_bulk(&g_tx, buf, n);
    if (w > 0u) {
        USART1->CR1 |= USART_CR1_TXEIE;              /* kick the ISR */
    }
    return w;
}
```

**Disabling `TXEIE` when the buffer empties is the bug this problem is built around.** `TXE` means "the transmit data register is free" and it is *permanently true* once you stop writing to it. Leave the interrupt enabled with nothing to send and the ISR re-enters immediately, forever — the system locks up with no fault, which presents as a total freeze rather than a UART problem. Interrupt-driven TX is level-triggered on a condition that is true at idle, and that is the trap.

**`TXE` versus `TC`** is the other question, and it is asked constantly:

| Flag | Means | Use for |
|---|---|---|
| `TXE` | data register free; **a byte may still be shifting out** | feeding the next byte |
| `TC` | the last stop bit has left the pin | RS-485 direction turnaround, entering sleep, cutting power |

Switching an RS-485 transceiver from transmit to receive on `TXE` truncates the final character on the wire. It must be `TC`. That single distinction is the most repeated UART bug in the field.

Errors are handled **before** `RXNE` because an overrun suppresses `RXNE` — see Part 13 entry 5 for the failure mode. And `rb_write` returning false must increment a counter rather than being ignored: silent drops are indistinguishable from a hardware fault when you are debugging six months later.
</details>

---

### 4. UART DMA circular RX with idle-line detection `STA`

```c
void uart_dma_rx_start(void);
void uart_idle_isr(void);
```

<details><summary>Solution</summary>

```c
#define DMA_BUF 256u
static uint8_t  g_dma_buf[DMA_BUF];
static size_t   g_read_pos;                  /* where the software has consumed to */

void uart_dma_rx_start(void)
{
    DMA1_Stream5->CR   &= ~DMA_SxCR_EN;
    while (DMA1_Stream5->CR & DMA_SxCR_EN) { }        /* wait for it to actually stop */

    DMA1_Stream5->PAR   = (uint32_t)&USART1->RDR;
    DMA1_Stream5->M0AR  = (uint32_t)g_dma_buf;
    DMA1_Stream5->NDTR  = DMA_BUF;
    DMA1_Stream5->CR   |= DMA_SxCR_CIRC;              /* CIRCULAR: never stops */
    DMA1_Stream5->CR   |= DMA_SxCR_EN;

    USART1->CR3 |= USART_CR3_DMAR;
    USART1->CR1 |= USART_CR1_IDLEIE;                  /* interrupt on line idle */
    g_read_pos = 0u;
}

/* Fires when the line has been idle for one frame time — i.e. a burst ended. */
void uart_idle_isr(void)
{
    if (!(USART1->ISR & USART_ISR_IDLE)) return;
    USART1->ICR = USART_ICR_IDLECF;                   /* W1C */

    /* NDTR counts DOWN, so the write position is derived from it. */
    size_t write_pos = DMA_BUF - DMA1_Stream5->NDTR;

    if (write_pos != g_read_pos) {
        if (write_pos > g_read_pos) {
            protocol_feed(&g_dma_buf[g_read_pos], write_pos - g_read_pos);
        } else {                                      /* wrapped: two segments */
            protocol_feed(&g_dma_buf[g_read_pos], DMA_BUF - g_read_pos);
            protocol_feed(&g_dma_buf[0], write_pos);
        }
        g_read_pos = write_pos;
    }
}
```

**This is the pattern that separates people who have shipped STM32 UART from people who have read about it**, and it is worth being able to justify in one sentence: circular DMA plus idle-line detection gives you zero per-byte interrupts and no fixed expectation of message length.

Why each piece is necessary:

- **Circular mode** means the DMA never stops and never needs restarting, so there is no gap during which bytes are lost. `NDTR` reloads automatically.
- **`NDTR` counts down**, so `DMA_BUF - NDTR` is where the hardware has written to. There is no "bytes received" register; you compute it.
- **The idle interrupt is what makes variable-length messages work.** Transfer-complete only fires after 256 bytes; a 12-byte command would sit in the buffer indefinitely. Idle fires one frame time after the last byte of *any* burst, which is exactly the end-of-message signal a text or framed protocol needs.
- **The wrap produces two segments**, handled as two calls — the same two-copy split as `rb_read_bulk`.

Add the half-transfer and transfer-complete interrupts too if bursts can exceed the buffer, and the real-time constraint from Part 07 entry 11 applies: consume before the DMA laps you. On Cortex-M7, invalidate the cache lines before reading `g_dma_buf`.

The comparison to have ready: per-byte interrupts at 115200 baud cost about 11,500 interrupts per second and roughly 2–5% CPU; this costs one interrupt per message.
</details>

---

### 5. SPI transfer — the shift register `BEG`→`INT`

```c
uint8_t spi_xfer_byte(SPI_TypeDef *s, uint8_t out);
void    spi_xfer(SPI_TypeDef *s, const uint8_t *tx, uint8_t *rx, size_t n);
```

<details><summary>Solution</summary>

```c
uint8_t spi_xfer_byte(SPI_TypeDef *s, uint8_t out)
{
    while (!(s->SR & SPI_SR_TXE)) { }        /* room in the transmit register */
    *(volatile uint8_t *)&s->DR = out;       /* 8-bit access for an 8-bit frame */

    while (!(s->SR & SPI_SR_RXNE)) { }       /* a byte has been shifted IN */
    return *(volatile uint8_t *)&s->DR;      /* reading DR clears RXNE */
}

void spi_xfer(SPI_TypeDef *s, const uint8_t *tx, uint8_t *rx, size_t n)
{
    for (size_t i = 0u; i < n; i++) {
        uint8_t b = spi_xfer_byte(s, (tx != NULL) ? tx[i] : 0xFFu);
        if (rx != NULL) rx[i] = b;
    }
}

/* Chip select, with the two waits that matter. */
void spi_transaction(SPI_TypeDef *s, GPIO_TypeDef *cs_port, uint8_t cs_pin,
                     const uint8_t *tx, uint8_t *rx, size_t n)
{
    gpio_write(cs_port, cs_pin, false);              /* assert (active low) */
    spi_xfer(s, tx, rx, n);

    while (s->SR & SPI_SR_BSY) { }                   /* last bit still on the wire */
    gpio_write(cs_port, cs_pin, true);               /* only now deassert */
}
```

**SPI is two shift registers wired into a ring: every byte out produces a byte in.** There is no transmit-only or receive-only operation — to read, you must clock out a dummy byte (`0xFF` by convention, because it leaves a bus with a pull-up idle-high). Candidates who write a `spi_read` that does not write anything have missed the mechanism, and that is the thing being checked.

**You must read `DR` even when you do not want the data.** Leave it unread and `RXNE` stays set; the next byte overruns, `OVR` sets, and the peripheral stops. Same shape as the UART `ORE` bug.

**Deassert CS on `BSY`, not on `TXE`.** `TXE` means the data register is free — the last byte may still be shifting out. Dropping CS early truncates the final byte, and the classic symptom is a write that works at 1 MHz and fails at 8 MHz. Same `TXE`-versus-done distinction as `TXE`/`TC` on the UART.

Two more that come up: **the access width of `DR` selects the frame size** on some families, so an accidental 16-bit write sends two bytes; and if it works slow and fails fast, halve the clock — if the problem disappears it is signal integrity or the device's output-valid time (`t_V`), not logic. Series termination of 22–33 Ω on SCLK at the driver is the standard fix above ~10 MHz.
</details>

---

### 6. I²C write-then-read with repeated start `SEN`

```c
int i2c_write_read(I2C_TypeDef *i, uint8_t addr,
                   const uint8_t *wr, size_t wn,
                   uint8_t *rd, size_t rn);
```

<details><summary>Solution</summary>

```c
int i2c_write_read(I2C_TypeDef *i, uint8_t addr,
                   const uint8_t *wr, size_t wn, uint8_t *rd, size_t rn)
{
    if (i == NULL) return -1;

    /* ---- phase 1: write the register pointer, NO stop ---- */
    i->CR2 = ((uint32_t)addr << 1)
           | ((uint32_t)wn << 16)
           | I2C_CR2_START;                       /* AUTOEND = 0: keep the bus */

    for (size_t k = 0u; k < wn; k++) {
        if (i2c_wait(i, I2C_ISR_TXIS) != 0) return -1;
        i->TXDR = wr[k];
    }
    if (i2c_wait(i, I2C_ISR_TC) != 0) return -1;  /* transfer complete, bus HELD */

    /* ---- phase 2: repeated START, now reading ---- */
    i->CR2 = ((uint32_t)addr << 1)
           | ((uint32_t)rn << 16)
           | I2C_CR2_RD_WRN
           | I2C_CR2_START
           | I2C_CR2_AUTOEND;                     /* STOP automatically at the end */

    for (size_t k = 0u; k < rn; k++) {
        if (i2c_wait(i, I2C_ISR_RXNE) != 0) return -1;
        rd[k] = (uint8_t)i->RXDR;
    }
    return i2c_wait(i, I2C_ISR_STOPF);
}

/* Every wait is bounded. This is not optional. */
static int i2c_wait(I2C_TypeDef *i, uint32_t flag)
{
    uint32_t t0 = micros();
    while (!(i->ISR & flag)) {
        if (i->ISR & (I2C_ISR_NACKF | I2C_ISR_BERR | I2C_ISR_ARLO)) return -1;
        if ((uint32_t)(micros() - t0) > 2000u) return -1;      /* 2 ms */
    }
    return 0;
}
```

**The repeated start is the point.** Reading a register from an I²C sensor is: write the register address, then read the data — and a STOP between the two **releases the bus**. Another controller can take it, or the device can reset its internal address pointer, and you read the wrong register. Holding the bus with a repeated START makes the pair indivisible.

That is why this is a single function rather than `i2c_write()` followed by `i2c_read()`. Offering the composed version is the wrong answer, and noticing why is what is being tested.

Three supporting details:

- **`AUTOEND = 0` on the write phase, `1` on the read phase.** The first must not emit STOP; the second must.
- **Check `NACKF` in the wait loop.** A device that is absent or busy NACKs, and without that check you wait out the full timeout on every access — turning a missing sensor into a system that runs 100× too slowly rather than one that reports an error.
- **Every wait is bounded**, for the reason in Part 13 entry 6. `while (!(i->ISR & flag));` is how products hang in the field.

The old F1/F4 peripheral is materially different — `NBYTES` does not exist, and 1-byte and 2-byte reads need special ACK/POS handling (Part 13 entry 6). Knowing that the two generations differ, and that the reference manual is the authority, is a better answer than reciting either.
</details>

---

### 7. Recover a stuck I²C bus `SEN`

```c
int i2c_bus_recover(GPIO_TypeDef *port, uint8_t scl_pin, uint8_t sda_pin);
```

<details><summary>Solution</summary>

```c
int i2c_bus_recover(GPIO_TypeDef *port, uint8_t scl_pin, uint8_t sda_pin)
{
    /* Take both pins away from the I2C peripheral: GPIO, open-drain, high. */
    gpio_init_output(port, scl_pin, true);
    gpio_init_output(port, sda_pin, true);
    gpio_write(port, scl_pin, true);
    gpio_write(port, sda_pin, true);
    delay_us(10);

    if (gpio_read(port, sda_pin)) return 0;          /* SDA already free */

    /* Clock up to 9 pulses: enough for a slave to finish any byte it is sending. */
    for (int k = 0; k < 9; k++) {
        gpio_write(port, scl_pin, false);
        delay_us(5);
        gpio_write(port, scl_pin, true);
        delay_us(5);
        if (gpio_read(port, sda_pin)) break;         /* released */
    }

    /* Issue a manual STOP: SDA low→high while SCL is high. */
    gpio_write(port, sda_pin, false);
    delay_us(5);
    gpio_write(port, scl_pin, true);
    delay_us(5);
    gpio_write(port, sda_pin, true);                 /* STOP condition */
    delay_us(5);

    if (!gpio_read(port, sda_pin)) return -1;        /* still stuck: hardware fault */

    i2c_reinit();                                    /* SWRST, then reconfigure */
    return 0;
}
```

**Why this function has to exist.** I²C is open-drain with pull-ups: any device can hold SDA low. If the controller is reset or the power browns out mid-byte, the slave is left waiting for the rest of a transfer and keeps SDA low forever. The bus is dead, and **resetting the MCU does not fix it** — the slave's state machine is what is stuck, and it has not been reset. Only a power cycle of the slave, or this recovery sequence, clears it.

That last point is the one that earns the marks: it is the answer to "why does the product need a power cycle rather than a reset", which is a real field-failure report.

The mechanism: a slave holding SDA low is mid-byte, waiting for clocks. **Nine pulses** is worst case — eight data bits plus the ACK — after which it must release. Then a manual STOP resynchronises its state machine.

Two things to add: **the peripheral must be released to GPIO first**, because the I²C block will not drive pins while it thinks the bus is busy; and a driver should call this automatically on `BERR`, `ARLO` or a timeout rather than requiring a reboot. If SDA is *still* low afterwards, it is a shorted pin or a dead device — a hardware fault, and the driver should report it rather than retry forever.

Prevention belongs in the same answer: a watchdog on the I²C task, and pull-ups sized for the bus capacitance (typically 4.7 kΩ at 100 kHz, 2.2 kΩ at 400 kHz — too weak and the rise time violates the spec, giving intermittent failures that look like software bugs).
</details>

---

### 8. ADC oversampling and calibration `INT`

```c
uint16_t adc_read_oversampled(ADC_TypeDef *a, uint8_t ch, uint8_t log2n);
uint32_t adc_to_millivolts(uint16_t raw);
```

<details><summary>Solution</summary>

```c
/* Average 2^log2n samples: gains log2n/2 bits of effective resolution. */
uint16_t adc_read_oversampled(ADC_TypeDef *a, uint8_t ch, uint8_t log2n)
{
    if (a == NULL || log2n > 6u) return 0u;

    uint32_t n   = 1UL << log2n;
    uint32_t acc = 0u;

    for (uint32_t k = 0u; k < n; k++) {
        acc += adc_read_once(a, ch);           /* 12-bit result, max 4095 */
    }
    return (uint16_t)(acc >> log2n);           /* divide by n — a shift */
}

/* Use the factory-calibrated internal reference, not the nominal 3.3 V. */
uint32_t adc_to_millivolts(uint16_t raw)
{
    /* VREFINT_CAL is measured at 3.0 V during production and stored in flash. */
    const uint16_t cal    = *(const uint16_t *)0x1FFF7A2AUL;
    uint16_t       vrefint = adc_read_once(ADC1, ADC_CH_VREFINT);

    if (vrefint == 0u) return 0u;

    uint32_t vdda_mv = (3000u * (uint32_t)cal) / vrefint;      /* actual supply */
    return ((uint32_t)raw * vdda_mv) / 4095u;
}
```

**Oversampling is the cheapest resolution you will ever buy**, and the ratio is the thing to know: averaging 2ⁿ samples gains **n/2 bits** of effective resolution, because uncorrelated noise falls as √n. 16 samples buys 2 bits, turning a 12-bit ADC into an effective 14-bit one. 64 samples buys 3.

**The condition attached to that** is what a good answer includes: it only works for *random* noise. Correlated noise — 50/60 Hz mains hum, a switching regulator's ripple, digital crosstalk — is not reduced by averaging. For mains pickup the fix is to average over an exact multiple of the line period, which cancels it structurally rather than statistically.

`acc >> log2n` rather than `/ n` is why the sample count is a power of two, and `uint32_t` for the accumulator is required: 64 samples × 4095 exceeds 16 bits.

**The calibration half is the part most candidates skip entirely.** An ADC reading is a ratio against `VDDA`, not an absolute voltage. Assuming 3.3 V when the regulator is actually delivering 3.27 V is a permanent 1% error on every measurement. Reading the internal reference — whose true value was measured and stored in flash at manufacture — lets you compute the real supply and cancel it. That is the difference between a reading and a measurement, and it is why the factory calibration values exist.

Add: sampling time must suit the source impedance (a high-impedance sensor needs a long sample time or the sample-and-hold capacitor never charges — reads low), and the temperature sensor needs its own calibration pair.
</details>

---

### 9. ADC continuous DMA `SEN`

```c
void adc_dma_start(uint16_t *buf, size_t n);
```

<details><summary>Solution</summary>

```c
void adc_dma_start(uint16_t *buf, size_t n)
{
    if (buf == NULL || n == 0u) return;

    /* --- DMA: circular, 16-bit, peripheral to memory, half + full interrupts --- */
    DMA2_Stream0->CR  &= ~DMA_SxCR_EN;
    while (DMA2_Stream0->CR & DMA_SxCR_EN) { }

    DMA2_Stream0->PAR  = (uint32_t)&ADC1->DR;
    DMA2_Stream0->M0AR = (uint32_t)buf;
    DMA2_Stream0->NDTR = n;
    DMA2_Stream0->CR   = DMA_SxCR_CIRC
                       | DMA_SxCR_MINC                       /* advance memory ptr */
                       | (1UL << DMA_SxCR_PSIZE_Pos)         /* 16-bit peripheral */
                       | (1UL << DMA_SxCR_MSIZE_Pos)         /* 16-bit memory     */
                       | DMA_SxCR_TCIE | DMA_SxCR_HTIE | DMA_SxCR_TEIE;
    DMA2_Stream0->CR  |= DMA_SxCR_EN;

    /* --- ADC: continuous conversion, DMA requests kept coming --- */
    ADC1->CR2 |= ADC_CR2_DMA | ADC_CR2_DDS | ADC_CR2_CONT;
    ADC1->CR2 |= ADC_CR2_ADON;
    ADC1->CR2 |= ADC_CR2_SWSTART;
}
```

Free-running acquisition with the CPU touching nothing: the ADC converts continuously, the DMA files results, and the half-transfer and transfer-complete interrupts hand you alternating halves to process — the ping-pong pattern from Part 07 entry 11.

**`DDS` is the bit that catches people.** Without "DMA disable selection" set, the DMA request generation stops after the first full transfer, so you get exactly one buffer and then silence. It looks like the DMA died; it did what it was told.

The rest of the answer is the constraints:

- **The real-time budget** is `(n/2) / sample_rate`. Exceed it and the DMA overwrites data you are still reading — silently, with no flag. Write the number in a comment.
- **Sample rate is set by the trigger**, not by `CONT` alone. For signal processing you want a **timer** trigger (`EXTSEL`) so the interval is exact; `CONT` free-runs at whatever the conversion time happens to be, which makes any frequency-domain result meaningless.
- **`TEIE`, and a handler for it.** A transfer error disables the stream and the interrupts go quiet — the "it acquired for six hours then stopped" failure.
- **On Cortex-M7**, invalidate before reading and align the buffer to 32 bytes (Part 13 entry 7).

For multiple channels, configure the scan sequence in `SQRx` with `SCAN` enabled: results land in the buffer in sequence order, so a 4-channel scan into a 256-entry buffer gives 64 samples per channel interleaved. Getting the de-interleaving right is the follow-up.
</details>

---

### 10. PWM — init and set duty `INT`

```c
int  pwm_init(TIM_TypeDef *t, uint32_t tclk_hz, uint32_t freq_hz);
void pwm_set_duty_permille(TIM_TypeDef *t, uint8_t ch, uint16_t permille);
```

<details><summary>Solution</summary>

```c
int pwm_init(TIM_TypeDef *t, uint32_t tclk_hz, uint32_t freq_hz)
{
    if (t == NULL || freq_hz == 0u || tclk_hz == 0u) return -1;

    /* Choose the SMALLEST prescaler that fits ARR in 16 bits: maximum resolution. */
    uint32_t total = tclk_hz / freq_hz;                  /* ticks per period */
    uint32_t psc   = (total + 65535u) / 65536u;          /* round up */
    if (psc == 0u) psc = 1u;
    uint32_t arr   = (total / psc);
    if (arr == 0u || arr > 65536u) return -1;

    t->PSC  = (uint16_t)(psc - 1u);                      /* divider is PSC+1 */
    t->ARR  = (uint16_t)(arr - 1u);                      /* period is ARR+1  */
    t->CCR1 = 0u;

    t->CCMR1 = (6UL << TIM_CCMR1_OC1M_Pos)               /* PWM mode 1 */
             | TIM_CCMR1_OC1PE;                          /* preload enable */
    t->CCER |= TIM_CCER_CC1E;
    t->CR1  |= TIM_CR1_ARPE;                             /* buffer ARR too */
    t->EGR   = TIM_EGR_UG;                               /* force a load of PSC/ARR */
    t->CR1  |= TIM_CR1_CEN;
    return 0;
}

void pwm_set_duty_permille(TIM_TypeDef *t, uint8_t ch, uint16_t permille)
{
    if (t == NULL || permille > 1000u) return;
    uint32_t arr = t->ARR;
    uint32_t ccr = ((arr + 1u) * permille) / 1000u;
    if (ch == 1u) t->CCR1 = (uint16_t)ccr;               /* preloaded: safe any time */
}
```

**Two off-by-ones, both mandatory.** The prescaler divides by `PSC + 1` and the period is `ARR + 1` ticks, because both counters include zero. Writing `ARR = total` gives a period one tick long — negligible at 1 kHz, and a real error when you are generating a precise frequency.

**Choosing the prescaler for maximum resolution is the design content.** Duty resolution *is* `ARR`: with `ARR = 999` you have 0.1% steps, with `ARR = 99` you have 1% steps and visible stepping in an LED fade. So take the smallest prescaler that still fits `ARR` in 16 bits, rather than picking a round prescaler and accepting whatever `ARR` falls out. That trade-off — frequency versus duty resolution, fixed by the 16-bit counter — is the thing being probed.

**`OC1PE` (preload) is what makes `pwm_set_duty` safe to call at any moment.** Without it, writing `CCR1` mid-period can produce a compare value the counter has already passed, so that cycle never turns off — a full-width output pulse. On a motor bridge or a buck converter, one unintended full-duty cycle is a real fault, not a glitch. With preload, the new value is latched at the next update event and every cycle is well-formed.

`EGR = UG` forces the prescaler and ARR to load immediately instead of at the next natural update, which matters because the first period would otherwise use whatever was there before.

For motor control the follow-up is complementary outputs with **dead time** (`BDTR`), preventing both halves of a bridge conducting simultaneously — a shoot-through that destroys the FETs. Centre-aligned mode reduces harmonic content. Both are worth naming as the next layer.
</details>

---

### 11. Timer input capture — measure a frequency `SEN`

```c
uint32_t measure_frequency_hz(void);
```

<details><summary>Solution</summary>

```c
static volatile uint32_t g_period_ticks;
static volatile uint32_t g_overflows;
static volatile bool     g_valid;

void TIM3_IRQHandler(void)
{
    if (TIM3->SR & TIM_SR_UIF) {              /* counter wrapped */
        TIM3->SR = ~TIM_SR_UIF;
        if (g_overflows < 0xFFFFu) g_overflows++;
    }

    if (TIM3->SR & TIM_SR_CC1IF) {
        static uint32_t prev;
        uint32_t now = TIM3->CCR1;             /* reading CCR1 clears CC1IF */

        /* Full elapsed count, including every wrap since the last edge. */
        uint32_t elapsed = (g_overflows * (TIM3->ARR + 1u)) + now - prev;

        if (TIM3->SR & TIM_SR_CC1OF) {         /* capture overrun: we missed one */
            TIM3->SR = ~TIM_SR_CC1OF;
            g_valid = false;                   /* discard, do not report a wrong value */
        } else {
            g_period_ticks = elapsed;
            g_valid = true;
        }
        prev        = now;
        g_overflows = 0u;
    }
}

uint32_t measure_frequency_hz(void)
{
    if (!g_valid || g_period_ticks == 0u) return 0u;
    return TIMER_CLK_HZ / g_period_ticks;
}
```

**Input capture latches the counter in hardware at the edge**, so the measurement carries no interrupt-latency error. Reading the counter inside the ISR instead would add the entry latency to every sample — 12+ cycles of jitter, which at high frequencies swamps the signal. That distinction is the reason the peripheral exists and the first thing to say.

**Handling counter overflow is what makes it correct across the range.** A 16-bit timer wraps every 65536 ticks; a low-frequency input can span several wraps between edges. Counting update events and folding them into the elapsed calculation extends the range to 32 bits. Omit it and low frequencies read as absurdly high ones — the classic symptom.

**`CC1OF` — capture overflow — is the flag candidates never mention.** It means a second edge arrived before the first was read, so `CCR1` holds the newer value and one period is lost. Reporting a measurement anyway gives a plausible but wrong number. Detecting it and marking the sample invalid is the difference between a driver and a demo.

Two extras worth having: **PWM input mode** captures period and pulse width simultaneously using two channels on one input, which is how you measure duty cycle; and for very high frequencies, counting edges over a fixed gate time is more accurate than timing one period, with the crossover roughly where the period approaches the interrupt latency.
</details>

---

### 12. Watchdog — init, kick, and where to kick `INT`

```c
void wdt_init(uint32_t timeout_ms);
void wdt_kick(void);
```

<details><summary>Solution</summary>

```c
void wdt_init(uint32_t timeout_ms)
{
    /* IWDG runs from the ~32 kHz LSI: independent of the main clock and PLL. */
    IWDG->KR  = 0x5555u;                       /* unlock the config registers */
    IWDG->PR  = 4u;                            /* prescaler /64 → ~500 Hz */
    IWDG->RLR = (timeout_ms * 500u) / 1000u;   /* ticks for the requested timeout */
    while (IWDG->SR != 0u) { }                 /* wait for RLR/PR to be accepted */
    IWDG->KR  = 0xAAAAu;                       /* reload */
    IWDG->KR  = 0xCCCCu;                       /* START — cannot be stopped in software */
}

void wdt_kick(void) { IWDG->KR = 0xAAAAu; }

/* The pattern that makes a watchdog worth having. */
static volatile uint32_t g_alive_flags;
#define TASK_SENSOR   (1UL << 0)
#define TASK_COMMS    (1UL << 1)
#define TASK_CONTROL  (1UL << 2)
#define ALL_TASKS     (TASK_SENSOR | TASK_COMMS | TASK_CONTROL)

void wdt_task(void)                            /* the ONLY caller of wdt_kick */
{
    if ((g_alive_flags & ALL_TASKS) == ALL_TASKS) {
        g_alive_flags = 0u;                     /* everyone must re-check in */
        wdt_kick();
    }
    /* else: do nothing, and let the watchdog fire */
}
```

**Where you kick matters more than how**, and this is the whole question.

A `wdt_kick()` inside the main loop, or worse inside a `while (!(REG & FLAG))` wait, tests **nothing**. The loop keeps spinning, the watchdog keeps getting fed, and the product hangs forever with a perfectly healthy watchdog. A watchdog kicked from a timer ISR is the same mistake automated: the ISR keeps running while every task is deadlocked.

The correct structure is the one above: **each task sets its own liveness flag, and a single supervisor kicks only when all of them have checked in.** Now a hung task actually causes a reset. That is the answer that distinguishes someone who has debugged a field failure.

Supporting points, each of which gets asked:

- **`IWDG` cannot be stopped once started**, by design, and it runs from the LSI — so it survives a PLL failure or a clock-tree misconfiguration that would stop a timer-based watchdog. That independence is the reason to prefer it.
- **A windowed watchdog (`WWDG`) also faults an early kick**, which catches code running *too fast* — a loop that skipped its work, or a task stuck in a tight retry. Strictly stronger, and worth naming.
- **Record the reset cause** (`RCC->CSR` `IWDGRSTF`) and log it on the next boot. A watchdog reset nobody notices is a bug that never gets fixed; the counter in non-volatile memory is what turns it into a bug report.
- **Set the timeout from the worst-case loop time with margin** — typically 2–3× — and remember to suspend or account for it around long flash erases, which can take hundreds of milliseconds.
</details>

---

### 13. Quadrature rotary encoder decode `SEN`

```c
int8_t encoder_update(uint8_t ab);      /* returns -1, 0, or +1 */
```

<details><summary>Solution</summary>

```c
/* Lookup indexed by (prev << 2) | curr, where each state is the 2-bit AB code.
   +1 = clockwise, -1 = counter-clockwise, 0 = no change or an illegal jump.   */
static const int8_t k_qdec[16] = {
     0, +1, -1,  0,      /* prev 00 → 00,01,10,11 */
    -1,  0,  0, +1,      /* prev 01 */
    +1,  0,  0, -1,      /* prev 10 */
     0, -1, +1,  0       /* prev 11 */
};

int8_t encoder_update(uint8_t ab)
{
    static uint8_t prev = 0u;

    ab &= 3u;
    int8_t step = k_qdec[(prev << 2) | ab];
    prev = ab;
    return step;
}
```

**A quadrature encoder emits Gray code**, and that is the link to Part 01 entry 18: exactly one of the two signals changes per step, so any transition where *both* changed is either a missed step or contact noise. The table returns 0 for those four illegal transitions — which is the built-in debounce, and the reason a lookup table beats a chain of `if` statements. It is also branch-free and one memory access.

The diagonal entries (`prev == curr`) are 0 because nothing moved.

Three things to add, because they are what the question is really probing:

- **Counting illegal transitions is a free diagnostic.** A steadily rising count means noise, a failing encoder, or a polling rate too slow for the shaft speed. Exposing that counter turns "the position drifts" into a measurement.
- **The polling rate must exceed 4× the maximum step rate**, or transitions are missed and position drifts permanently — an error that accumulates and never self-corrects, which is why absolute position needs a reference/index pulse to re-zero against.
- **Most STM32 timers have hardware encoder mode** (`SMS = 011` in `SMCR`), which counts in `TIM->CNT` with no CPU involvement at all and cannot miss a step. Naming that as the production answer, with the software version as the fallback for a part or pin set that lacks it, is the complete response.

Note `static uint8_t prev` makes this non-reentrant and single-instance. For multiple encoders, pass a state struct — an interviewer may well ask you to refactor it, and the fix is to move `prev` into a caller-owned context.
</details>

---

### 14. Internal flash — erase and program `SEN`

```c
int flash_erase_sector(uint8_t sector);
int flash_write_words(uint32_t addr, const uint32_t *data, size_t n);
```

<details><summary>Solution</summary>

```c
static int flash_unlock(void)
{
    if (FLASH->CR & FLASH_CR_LOCK) {
        FLASH->KEYR = 0x45670123UL;               /* the two magic keys, in order */
        FLASH->KEYR = 0xCDEF89ABUL;
    }
    return (FLASH->CR & FLASH_CR_LOCK) ? -1 : 0;  /* a wrong sequence locks until reset */
}

static int flash_wait(void)
{
    uint32_t t0 = micros();
    while (FLASH->SR & FLASH_SR_BSY) {
        if ((uint32_t)(micros() - t0) > 3000000u) return -1;   /* erase can take seconds */
    }
    uint32_t err = FLASH->SR & (FLASH_SR_PGSERR | FLASH_SR_PGPERR |
                                FLASH_SR_PGAERR | FLASH_SR_WRPERR);
    if (err != 0u) { FLASH->SR = err; return -1; }             /* W1C, then report */
    return 0;
}

int flash_erase_sector(uint8_t sector)
{
    if (flash_unlock() != 0) return -1;
    if (flash_wait()   != 0) return -1;

    FLASH->CR &= ~FLASH_CR_PSIZE;
    FLASH->CR |= (2UL << FLASH_CR_PSIZE_Pos);     /* 32-bit programming */
    FLASH->CR &= ~FLASH_CR_SNB;
    FLASH->CR |= ((uint32_t)sector << FLASH_CR_SNB_Pos) | FLASH_CR_SER;
    FLASH->CR |= FLASH_CR_STRT;

    int rc = flash_wait();

    FLASH->CR &= ~FLASH_CR_SER;
    FLASH->CR |=  FLASH_CR_LOCK;                  /* re-lock: not optional */

    __DSB();
    flash_cache_reset();                          /* see below */
    return rc;
}

int flash_write_words(uint32_t addr, const uint32_t *data, size_t n)
{
    if (data == NULL || (addr & 3u) != 0u) return -1;          /* must be aligned */
    if (flash_unlock() != 0) return -1;

    FLASH->CR &= ~FLASH_CR_PSIZE;
    FLASH->CR |= (2UL << FLASH_CR_PSIZE_Pos) | FLASH_CR_PG;

    int rc = 0;
    for (size_t i = 0u; i < n; i++) {
        *(volatile uint32_t *)(addr + (i * 4u)) = data[i];
        if (flash_wait() != 0) { rc = -1; break; }             /* wait after EACH word */
    }

    FLASH->CR &= ~FLASH_CR_PG;
    FLASH->CR |=  FLASH_CR_LOCK;
    __DSB();
    flash_cache_reset();
    return rc;
}
```

The bootloader and OTA foundation, and the entry point to firmware-update work.

**The five rules of NOR flash, each of which is a separate bug if broken:**

1. **You can only clear bits, never set them.** Programming ANDs with the existing content, so writing `0xFF` over `0x00` leaves `0x00`. **Erase is the only operation that produces ones**, and it works on whole sectors — 16 KB to 128 KB. Attempting to "update" a word in place without erasing yields the AND of old and new, which is the single most common flash bug.
2. **Wait for `BSY` after every word.** The flash controller is busy for tens of microseconds per word; issuing the next write early sets `PGSERR` and the data is lost. A loop without the per-word wait appears to work and drops most of the payload.
3. **Check and clear the error flags.** `WRPERR` means the sector is write-protected — a silently failed OTA that reports success is worse than one that fails loudly.
4. **Re-lock afterwards.** Leaving `CR.LOCK` clear means any wild pointer write can erase the application. On a device that must be field-recoverable, that is the difference between a bug and a brick.
5. **Invalidate the instruction cache and prefetch buffer after erasing or programming.** The core may hold stale copies of lines you just changed, so freshly written code or constants read as their old values — the "the update installed but the old version is running" failure.

**Why the CPU stalls during a flash operation, and why that matters:** on most parts you cannot fetch instructions from the same flash bank you are erasing, so the core halts for the duration — hundreds of milliseconds for a sector. Consequences to name: **interrupt latency is destroyed for that period**, so a real-time system must either suspend hard-real-time work, run the flash routine from RAM (`__attribute__((section(".RamFunc")))`), or use a dual-bank part. And the watchdog will fire mid-erase unless its timeout accounts for it (entry 12).

For OTA, the follow-up is always the same: **an A/B (dual-bank) layout with a validity marker written last**, so a power failure at any point leaves a bootable image. Write the image, verify its CRC, and only then write the "valid" flag — because the flag is the commit point, and it must be a single word so it cannot be half-written.
</details>

---

## Part 09 — retention table

| Mechanism | The detail that decides it |
|---|---|
| GPIO write | `BSRR` is atomic; `ODR \|=` races with an ISR on the same port |
| GPIO read | `IDR` is the pin, `ODR` is your intent — they differ, and that's diagnostic |
| UART baud | round, don't truncate; 2% error per end is the budget |
| UART TX interrupt | **disable `TXEIE` when the buffer empties**, or it re-enters forever |
| UART done | `TXE` = register free; `TC` = last bit gone. RS-485 needs `TC` |
| UART at speed | circular DMA + IDLE line: one interrupt per message, not per byte |
| SPI | two shift registers; must clock out to read, must read `DR` to avoid `OVR` |
| SPI CS release | wait `BSY`, not `TXE`, or the last byte is truncated |
| I²C register read | repeated START — a STOP between phases releases the bus |
| I²C stuck | 9 clocks + manual STOP; **an MCU reset does not fix the slave** |
| ADC resolution | 2ⁿ samples → n/2 bits, **random noise only** |
| ADC accuracy | ratio against `VDDA` — use `VREFINT_CAL`, not nominal 3.3 V |
| PWM | period is `ARR+1`; preload (`OCxPE`) prevents a full-width pulse |
| Input capture | hardware latches the edge — no ISR latency; handle `CCxOF` |
| Watchdog | kick from a supervisor gated on per-task liveness flags, never in a wait loop |
| Encoder | Gray code; both bits changing = missed step, count it |
| Flash | erase sets ones, program only clears; wait per word; re-lock; invalidate cache |

**The four questions to ask of any driver you are asked to write:** what happens on error, what bounds the wait, what order must initialisation follow, and which flag means *actually finished*.

---

# Part 10 — Protocol Parsers

9 entries from 50. The bank lists MavLink, Zigbee, LoRaWAN, CoAP, DLMS, BACnet, M-Bus, Z-Wave and ADS-B as separate problems — they are the same three mechanisms with different constants. What is kept covers each mechanism once: **byte-fed state machines, framing and escaping, delimiter-free timing, text field parsing, table dispatch, binary TLV, transcoding, and a transfer protocol with retries.**

> [!IMPORTANT]
> **The three rules that govern every parser below.**
> 1. **Feed one byte at a time and never block.** Bytes arrive from an ISR or a DMA buffer in arbitrary chunks; a parser that assumes a whole frame has arrived is a parser that works on the bench and fails on a real link.
> 2. **Validate the length field before you use it.** A length byte from the wire indexing your buffer is the single most exploited bug class in embedded networking. Check it against your capacity *and* against the bytes remaining, every time.
> 3. **Every frame boundary must be recoverable.** After corruption the parser has to resynchronise on its own, without a reset and without a timeout you invented.

---

### 1. Byte-fed frame parser — the foundation pattern `INT`

```c
void parser_feed(struct parser *p, uint8_t b);
```
Frame: `SOF(0xAA) | LEN | PAYLOAD[LEN] | CRC16_LO | CRC16_HI`

<details><summary>Solution</summary>

```c
#define SOF          0xAAu
#define MAX_PAYLOAD  64u
#define FRAME_GAP_MS 50u

enum pstate { P_SOF = 0, P_LEN, P_DATA, P_CRC_LO, P_CRC_HI };

struct parser {
    enum pstate st;
    uint8_t     buf[MAX_PAYLOAD];
    uint8_t     len;
    uint8_t     idx;
    uint16_t    crc_rx;
    uint32_t    last_ms;
    uint32_t    n_bad_len, n_bad_crc, n_timeout;
};

void parser_feed(struct parser *p, uint8_t b)
{
    if (p == NULL) return;

    /* Inter-byte timeout: a stalled partial frame must not wedge the parser. */
    uint32_t now = g_ticks;
    if (p->st != P_SOF && (uint32_t)(now - p->last_ms) > FRAME_GAP_MS) {
        p->st = P_SOF;
        p->n_timeout++;
    }
    p->last_ms = now;

    switch (p->st) {
    case P_SOF:
        if (b == SOF) p->st = P_LEN;
        break;                                  /* silently discard until SOF */

    case P_LEN:
        if (b == 0u || b > MAX_PAYLOAD) {       /* ---- THE critical check ---- */
            p->n_bad_len++;
            p->st = (b == SOF) ? P_LEN : P_SOF; /* b might itself be a new SOF */
            break;
        }
        p->len = b;
        p->idx = 0u;
        p->st  = P_DATA;
        break;

    case P_DATA:
        p->buf[p->idx++] = b;                   /* bounded by the check above */
        if (p->idx >= p->len) p->st = P_CRC_LO;
        break;

    case P_CRC_LO:
        p->crc_rx = b;
        p->st = P_CRC_HI;
        break;

    case P_CRC_HI:
        p->crc_rx |= (uint16_t)((uint16_t)b << 8);
        if (crc16_ccitt(p->buf, p->len) == p->crc_rx) {
            handle_frame(p->buf, p->len);
        } else {
            p->n_bad_crc++;
        }
        p->st = P_SOF;
        break;

    default:
        p->st = P_SOF;
        break;
    }
}
```

**This is the shape every other entry in this part reduces to**, and the reason it is first: one byte in, no blocking, no allocation, constant stack, and it can be called directly from an ISR or from a task draining a ring buffer.

**`if (b == 0 || b > MAX_PAYLOAD)` is the line that matters.** Without it, a corrupted or hostile length byte of 0xFF writes 255 bytes into a 64-byte buffer — a stack or `.bss` overflow driven entirely by wire data. This is the archetypal embedded network vulnerability, and it is the first thing a security-minded interviewer looks for.

Three details that separate a working parser from a demo:

- **The inter-byte timeout.** A frame truncated mid-payload leaves the state machine waiting forever, and every subsequent byte is consumed as payload — the link appears dead though bytes are flowing. The timeout is what makes it self-healing.
- **`p->st = (b == SOF) ? P_LEN : P_SOF` on a bad length.** The rejected byte may itself be the start of the next frame; discarding it unconditionally costs you the frame after an error, which turns one glitch into two lost frames.
- **Counters for each failure mode.** `n_bad_crc` rising means a noisy link; `n_bad_len` rising means a framing desync or a mismatched peer. Without them "it sometimes doesn't work" is unanswerable.

The weakness to name unprompted: **`0xAA` can occur inside the payload**, so resynchronisation after corruption may lock onto a false SOF. The CRC rejects it, but you lose frames until alignment recovers. Entry 2 is the structural fix.
</details>

---

### 2. COBS encode and decode `SEN`

```c
size_t cobs_encode(const uint8_t *src, size_t len, uint8_t *dst);
size_t cobs_decode(const uint8_t *src, size_t len, uint8_t *dst);
```

<details><summary>Solution</summary>

```c
/* dst needs len + len/254 + 1 bytes. Output contains NO zero bytes. */
size_t cobs_encode(const uint8_t *src, size_t len, uint8_t *dst)
{
    if (src == NULL || dst == NULL) return 0u;

    size_t  rd = 0u, wr = 1u, code_i = 0u;
    uint8_t code = 1u;

    while (rd < len) {
        if (src[rd] == 0u) {
            dst[code_i] = code;            /* close this group */
            code_i = wr++;                 /* reserve the next code slot */
            code = 1u;
            rd++;
        } else {
            dst[wr++] = src[rd++];
            code++;
            if (code == 0xFFu) {           /* group is full at 254 data bytes */
                dst[code_i] = code;
                code_i = wr++;
                code = 1u;
            }
        }
    }
    dst[code_i] = code;
    return wr;
}

/* Returns 0 on malformed input. Caller appends/strips the 0x00 delimiter. */
size_t cobs_decode(const uint8_t *src, size_t len, uint8_t *dst)
{
    if (src == NULL || dst == NULL) return 0u;

    size_t rd = 0u, wr = 0u;

    while (rd < len) {
        uint8_t code = src[rd++];
        if (code == 0u) return 0u;                 /* delimiter inside the frame */

        for (uint8_t i = 1u; i < code; i++) {
            if (rd >= len) return 0u;              /* truncated */
            dst[wr++] = src[rd++];
        }
        if (code < 0xFFu && rd < len) {
            dst[wr++] = 0u;                        /* the zero this group replaced */
        }
    }
    return wr;
}
```

**Consistent Overhead Byte Stuffing removes every zero byte from the payload, so `0x00` becomes an unambiguous frame delimiter.** Each group is a length code followed by up to 254 non-zero bytes; a code below 0xFF means the group ended at a zero, which the decoder reinserts.

**Why this beats the alternatives, and it is the whole reason to know it:**

| Framing | Overhead | Worst case | Resync after corruption |
|---|---|---|---|
| SOF + length (entry 1) | 4 bytes | 4 bytes | ambiguous — SOF can occur in payload |
| SLIP (escape `0xC0`/`0xDB`) | 1 byte per escaped byte | **2n + 2** — doubles | on `0xC0`, reliable |
| **COBS** | **⌈n/254⌉ + 1** | **n/254 + 1** | on `0x00`, **guaranteed** |

**SLIP's worst case is the argument.** A 256-byte payload of all `0xC0` encodes to 514 bytes, so every buffer in the system must be sized for 2n — and a device that only ever sends text never exercises that path, so the overflow ships. COBS overhead is 1 byte per 254 regardless of content: **deterministic buffer sizing**, which is what you actually need on a part with 20 KB of RAM.

**And resynchronisation is guaranteed**, not probabilistic. Because `0x00` cannot appear inside an encoded frame, the next zero byte is unambiguously the next boundary — no false locks, no waiting for a CRC to reject a misaligned frame. That property is worth more than the byte it saves.

The decoder's two rejection paths are both required: a zero inside the data means the framing was violated, and running past `len` mid-group means the frame was truncated. Returning 0 rather than partial data is the right contract — a half-decoded frame is worse than none.
</details>

---

### 3. HDLC framing, byte-fed with escaping `SEN`

```c
void hdlc_feed(struct hdlc *h, uint8_t b);
```
Flag `0x7E`, escape `0x7D`, XOR mask `0x20`.

<details><summary>Solution</summary>

```c
#define HDLC_FLAG 0x7Eu
#define HDLC_ESC  0x7Du
#define HDLC_MASK 0x20u

struct hdlc {
    uint8_t  buf[256];
    uint16_t idx;
    bool     esc;
    bool     overflow;
};

void hdlc_feed(struct hdlc *h, uint8_t b)
{
    if (h == NULL) return;

    if (b == HDLC_FLAG) {                       /* unconditional boundary */
        if (!h->overflow && h->idx > 2u) {
            /* FCS is computed over data+FCS; a good frame yields the magic value. */
            if (crc16_x25(h->buf, h->idx) == 0xF0B8u) {
                handle_frame(h->buf, (uint16_t)(h->idx - 2u));   /* strip the FCS */
            }
        }
        h->idx = 0u;                            /* reset regardless — resync */
        h->esc = false;
        h->overflow = false;
        return;
    }

    if (b == HDLC_ESC) {                        /* next byte is escaped */
        h->esc = true;
        return;
    }

    if (h->esc) {
        b ^= HDLC_MASK;
        h->esc = false;
    }

    if (h->idx < sizeof h->buf) {
        h->buf[h->idx++] = b;
    } else {
        h->overflow = true;                     /* remember, but keep consuming */
    }
}
```

**The flag byte resets the parser unconditionally, and that is the self-synchronising property.** Whatever state the parser was in — mid-frame, mid-escape, overflowed — a `0x7E` starts a clean frame. No timeout is needed to recover from corruption, which is why HDLC framing has survived since the 1970s and appears inside PPP, LAPB and dozens of industrial protocols.

**The FCS magic value is the trick to know.** Rather than computing the CRC over the payload and comparing against the received bytes, run the CRC over payload *and* FCS together: a valid frame produces the constant `0xF0B8` (for CRC-16/X-25, initial value `0xFFFF`, final XOR `0xFFFF`). One comparison, no need to extract and byte-order the received CRC. Being able to explain *why* — the CRC of a message with its own remainder appended is a fixed value — is what makes this a senior answer.

Three details:

- **`overflow` is a flag, not an early return.** Continue consuming bytes so the parser stays aligned to the flag; just refuse to deliver the frame. Bailing out mid-frame leaves the escape state inconsistent.
- **Escape handling must survive a flag arriving mid-escape.** The `h->esc = false` in the flag branch is what prevents the first byte of the next frame being silently XORed.
- **`idx > 2` guards against a bare `7E 7E`**, which is legal — back-to-back flags are how idle line fill works, and an empty frame must not be delivered as a 65534-byte one after the `idx - 2`.

The comparison to COBS: HDLC's overhead is content-dependent (each `0x7E` or `0x7D` in the payload costs a byte, so worst case is 2n), while COBS is bounded. HDLC wins on ubiquity and on bit-level implementations; COBS wins on deterministic sizing.
</details>

---

### 4. Modbus RTU parser `SEN`

```c
void modbus_rtu_feed(struct mb *m, uint8_t b, uint32_t now_us);
void modbus_rtu_tick(struct mb *m, uint32_t now_us);
```

<details><summary>Solution</summary>

```c
struct mb {
    uint8_t  buf[256];
    uint16_t idx;
    uint32_t last_us;
    uint32_t t35_us;            /* 3.5 character times */
    uint8_t  my_addr;
};

void modbus_rtu_init(struct mb *m, uint32_t baud, uint8_t addr)
{
    /* One character is 11 bits (start + 8 data + parity + stop). */
    m->t35_us  = (baud > 19200u) ? 1750u             /* spec: fixed above 19200 */
                                 : (11u * 3500000u) / baud;
    m->my_addr = addr;
    m->idx     = 0u;
}

void modbus_rtu_feed(struct mb *m, uint8_t b, uint32_t now_us)
{
    if (m == NULL) return;

    /* A gap of >= 3.5 chars before this byte means the previous frame ended. */
    if (m->idx > 0u && (uint32_t)(now_us - m->last_us) >= m->t35_us) {
        m->idx = 0u;                             /* start a new frame */
    }
    m->last_us = now_us;

    if (m->idx < sizeof m->buf) m->buf[m->idx++] = b;
}

/* Called from a timer: detects the trailing silence that ENDS a frame. */
void modbus_rtu_tick(struct mb *m, uint32_t now_us)
{
    if (m == NULL || m->idx == 0u) return;
    if ((uint32_t)(now_us - m->last_us) < m->t35_us) return;      /* still receiving */

    uint16_t n = m->idx;
    m->idx = 0u;                                 /* consume regardless of validity */

    if (n < 4u) return;                          /* addr + fn + 2 CRC minimum */
    if (crc16_modbus(m->buf, n) != 0u) return;   /* CRC over the whole frame == 0 */
    if (m->buf[0] != m->my_addr && m->buf[0] != 0u) return;       /* 0 = broadcast */

    modbus_dispatch(m->buf, (uint16_t)(n - 2u));
}
```

**Modbus RTU has no start byte, no end byte and no length field.** Frames are delimited **purely by silence** — 3.5 character times of idle line before and after. That is the defining property of the protocol and the entire point of the question: if a candidate reaches for a SOF byte, they have not read the spec.

The consequences, and each is a real implementation decision:

- **You need a timer, not just the receive path.** The end of a frame is the *absence* of data, which no receive interrupt will ever tell you. `modbus_rtu_tick` from a timer, or a UART idle-line interrupt (Part 09 entry 4) configured for the right duration, is what closes the frame.
- **The timing budget is tight and baud-dependent.** At 9600 baud 3.5 characters is 4 ms; at 115200 the spec fixes it at 1750 µs because the calculated value becomes shorter than realistic interrupt latency. **An ISR that takes longer than t3.5 to run splits one frame into two** — which is why Modbus at high baud rates on a busy MCU is genuinely hard, and why DMA is the right answer.
- **A gap of 1.5 characters mid-frame is technically an error** in the spec. Most implementations tolerate it; mentioning that you know the distinction is worth more than implementing it.

**`crc16_modbus(buf, n) == 0` over the whole frame including the CRC** is the same magic-value trick as entry 3 — no extraction, no byte-order handling. Note Modbus sends its CRC **little-endian** while every length and register value in the payload is **big-endian**, which is a genuine wart and a favourite gotcha.

The dispatch half is worth a sentence: an error response is the function code with bit 7 set (`fn | 0x80`) plus a one-byte exception code, and an unsupported function must return exception 0x01 rather than silence — a silent controller is indistinguishable from a dead one, and the master will retry forever.
</details>

---

### 5. NMEA sentence parser `INT`

```c
int nmea_parse(char *line, char *fields[], size_t max_fields);
```
Format: `$GPGGA,123519,4807.038,N,...*47`

<details><summary>Solution</summary>

```c
/* Splits IN PLACE. Returns field count, or -1 on a bad sentence. */
int nmea_parse(char *line, char *fields[], size_t max_fields)
{
    if (line == NULL || fields == NULL || max_fields == 0u) return -1;
    if (line[0] != '$') return -1;

    /* --- locate '*' and verify the XOR checksum of everything between $ and * --- */
    char *star = strchr(line, '*');
    if (star == NULL || star[1] == '\0' || star[2] == '\0') return -1;

    uint8_t sum = 0u;
    for (const char *p = line + 1; p < star; p++) {
        sum ^= (uint8_t)*p;
    }

    uint8_t want = (uint8_t)((hexval(star[1]) << 4) | hexval(star[2]));
    if (sum != want) return -1;

    *star = '\0';                        /* terminate the payload at '*' */

    /* --- split on commas, PRESERVING empty fields --- */
    size_t n = 0u;
    char  *p = line + 1;                 /* skip '$' */
    fields[n++] = p;

    while (*p != '\0' && n < max_fields) {
        if (*p == ',') {
            *p = '\0';
            fields[n++] = p + 1;         /* may point at '\0' — an EMPTY field */
        }
        p++;
    }
    return (int)n;
}
```

**Empty fields are the entire problem, and `strtok` is the wrong tool.** A GPS with no fix emits `$GPGGA,,,,,,0,00,,,,,,,*66` — the empty fields are *meaningful*, they say "no data for latitude". `strtok` treats runs of delimiters as one, so it silently collapses those and every subsequent field shifts left. Altitude is read as satellite count, and the bug is a plausible wrong number rather than a failure.

That single point is what the question tests. Splitting manually and letting a field point at `'\0'` preserves position, which is what the protocol requires.

Two more things `strtok` disqualifies itself on, worth naming: it holds **static state**, so it is not reentrant and cannot be used from two contexts or interleaved across two sentences; and it **modifies its input**, which this version also does but deliberately and visibly.

**The checksum is a plain XOR of everything between `$` and `*`**, exclusive — weak by design, since NMEA runs over a short local wire. It catches single-bit errors and nothing systematic; do not present it as integrity protection.

Details that come up: sentences are terminated `\r\n` and the parser should tolerate either or both; the talker ID varies (`$GPGGA`, `$GNGGA`, `$BDGGA`) so match on the last three characters, not all five; and coordinates are `DDMM.mmmm`, i.e. **degrees and decimal minutes, not decimal degrees** — dividing by 100 is a common and quietly wrong conversion. Correct form is `deg = trunc(v/100) + fmod(v,100)/60`.
</details>

---

### 6. AT command parser with a dispatch table `INT`

```c
void at_feed(char c);
```

<details><summary>Solution</summary>

```c
typedef int (*at_handler)(const char *args);

static const struct {
    const char *prefix;
    at_handler  fn;
} k_at_table[] = {
    { "AT+CSQ",     at_csq     },
    { "AT+CGATT",   at_cgatt   },
    { "AT+CIPSEND", at_cipsend },
    { "ATE",        at_echo    },
    { "AT",         at_ping    },        /* shortest prefix LAST */
};

#define AT_LINE_MAX 128u

void at_feed(char c)
{
    static char   line[AT_LINE_MAX];
    static size_t idx;

    if (c == '\r' || c == '\n') {
        if (idx == 0u) return;                     /* ignore blank lines */
        line[idx] = '\0';
        idx = 0u;
        at_dispatch(line);
        return;
    }

    if (idx < AT_LINE_MAX - 1u) {
        line[idx++] = c;
    } else {
        idx = 0u;                                  /* overlong: drop and resync */
        at_err("line too long");
    }
}

static void at_dispatch(const char *line)
{
    for (size_t i = 0u; i < (sizeof k_at_table / sizeof k_at_table[0]); i++) {
        size_t plen = strlen(k_at_table[i].prefix);

        if (strncmp(line, k_at_table[i].prefix, plen) == 0) {
            const char *args = line + plen;
            if (*args == '=' || *args == '?') args++;   /* AT+X=1 / AT+X? */
            (void)k_at_table[i].fn(args);
            return;
        }
    }
    at_err("unknown command");
}
```

**The dispatch table is the answer, not an `if`/`else if` chain.** Adding a command becomes one table row; the table is `const` so it lives in flash rather than RAM; and the lookup is uniform, so there is one place where argument extraction and error reporting happen. This is the same pattern as a driver registration table or a state machine transition table — and recognising it as the same pattern is the point.

**Prefix ordering matters and is easy to get wrong.** With `"AT"` first in the table, `AT+CSQ` matches it and the specific handler never runs. Either order longest-prefix-first as above, or require an exact match on the command portion. That ordering bug is subtle, passes a test that only exercises `AT`, and is exactly what a reviewer looks for.

Three practical points:

- **Bound the line and resync on overflow.** A modem emitting an unexpected multi-line dump must not overflow the buffer; resetting the index and reporting is the recoverable behaviour.
- **Unsolicited result codes interleave with responses.** A real modem sends `+CMTI:` or `RING` at any moment, including between your command and its `OK`. A parser that assumes the next line is the answer to the last command will misattribute them, so unsolicited codes need their own table and dispatch path.
- **Echo doubles everything.** With `ATE1` the modem echoes your command back before responding, so the first line received is your own text. Either disable echo at init (`ATE0`) or filter it — this is the usual cause of "the first response is always wrong".

Note `static char line[]` makes this single-instance and non-reentrant. Two modems means moving the state into a context struct, which is a fair follow-up request.
</details>

---

### 7. Binary TLV decoder `STA`

```c
int tlv_walk(const uint8_t *buf, size_t len,
             bool (*cb)(uint8_t type, const uint8_t *val, uint16_t vlen, void *ctx),
             void *ctx);
```

<details><summary>Solution</summary>

```c
/* Type (1 byte) | Length (1 byte, or 0xFF + 2 bytes) | Value.
   Returns the number of records walked, or -1 on malformed input.        */
int tlv_walk(const uint8_t *buf, size_t len,
             bool (*cb)(uint8_t, const uint8_t *, uint16_t, void *), void *ctx)
{
    if (buf == NULL || cb == NULL) return -1;

    size_t off = 0u;
    int    count = 0;

    while (off < len) {
        if ((len - off) < 2u) return -1;              /* no room for T and L */

        uint8_t  type = buf[off++];
        uint16_t vlen = buf[off++];

        if (vlen == 0xFFu) {                          /* extended length */
            if ((len - off) < 2u) return -1;
            vlen = (uint16_t)(((uint16_t)buf[off] << 8) | buf[off + 1u]);
            off += 2u;
        }

        /* ---- THE check: does the declared length fit in what remains? ---- */
        if (vlen > (len - off)) return -1;

        if (!cb(type, &buf[off], vlen, ctx)) break;   /* callback asked to stop */

        off += vlen;
        count++;

        if (count > 256) return -1;                   /* sanity bound */
    }
    return count;
}
```

**`if (vlen > (len - off)) return -1;` is the only line in this function that matters.** Every TLV vulnerability in history is that check being absent or wrong. A record declaring 60000 bytes inside a 40-byte packet, handed to a callback that trusts `vlen`, reads far past the buffer — and TLV is how BLE advertising data, CoAP options, EMV card data and dozens of industrial protocols are encoded, so the attack surface is real.

**Write it as `vlen > (len - off)`, never `off + vlen > len`.** The second form can overflow `size_t` for a large `vlen` and wrap to a small value, passing the check it was meant to fail. Subtracting the known-good quantity keeps the arithmetic in range — the same rearrangement discipline as the signed-overflow detection in Part 01 entry 20.

Three supporting decisions:

- **Check `(len - off) < 2` before reading T and L.** A truncated packet ending on a type byte would otherwise read one byte past the end.
- **The `count` bound catches a zero-length loop** — a malformed stream of `vlen == 0` records is not infinite here, but the bound is cheap insurance and makes the worst-case iteration count provable, which MISRA and any safety review will ask for.
- **The callback returning `bool` lets the caller stop early** without the walker needing to know what it is looking for. Passing `void *ctx` is what makes it usable without globals — the same argument as the callback entry in the general-C book.

Being able to say "this is the parser I would fuzz first" is a good closing remark: TLV walkers are the highest-value fuzzing target in any firmware image, which is also the practical route to the CVE work you would want on a security-focused CV.
</details>

---

### 8. Base64 encode and decode `INT`

```c
size_t b64_encode(const uint8_t *in, size_t n, char *out, size_t out_cap);
size_t b64_decode(const char *in, size_t n, uint8_t *out, size_t out_cap);
```

<details><summary>Solution</summary>

```c
static const char k_b64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

size_t b64_encode(const uint8_t *in, size_t n, char *out, size_t out_cap)
{
    if (in == NULL || out == NULL) return 0u;

    size_t need = (((n + 2u) / 3u) * 4u) + 1u;          /* +1 for NUL */
    if (out_cap < need) return 0u;

    size_t o = 0u, i = 0u;

    while ((n - i) >= 3u) {                              /* full 3-byte groups */
        uint32_t v = ((uint32_t)in[i] << 16) |
                     ((uint32_t)in[i + 1u] << 8) |
                      (uint32_t)in[i + 2u];
        out[o++] = k_b64[(v >> 18) & 0x3Fu];
        out[o++] = k_b64[(v >> 12) & 0x3Fu];
        out[o++] = k_b64[(v >>  6) & 0x3Fu];
        out[o++] = k_b64[ v        & 0x3Fu];
        i += 3u;
    }

    if ((n - i) == 1u) {                                 /* one byte left */
        uint32_t v = (uint32_t)in[i] << 16;
        out[o++] = k_b64[(v >> 18) & 0x3Fu];
        out[o++] = k_b64[(v >> 12) & 0x3Fu];
        out[o++] = '=';
        out[o++] = '=';
    } else if ((n - i) == 2u) {                          /* two bytes left */
        uint32_t v = ((uint32_t)in[i] << 16) | ((uint32_t)in[i + 1u] << 8);
        out[o++] = k_b64[(v >> 18) & 0x3Fu];
        out[o++] = k_b64[(v >> 12) & 0x3Fu];
        out[o++] = k_b64[(v >>  6) & 0x3Fu];
        out[o++] = '=';
    }

    out[o] = '\0';
    return o;
}

static int b64_val(char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return (c - 'a') + 26;
    if (c >= '0' && c <= '9') return (c - '0') + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;                                  /* invalid, or '=' */
}

size_t b64_decode(const char *in, size_t n, uint8_t *out, size_t out_cap)
{
    if (in == NULL || out == NULL || (n % 4u) != 0u) return 0u;

    size_t o = 0u;

    for (size_t i = 0u; i < n; i += 4u) {
        int c0 = b64_val(in[i]),      c1 = b64_val(in[i + 1u]);
        int c2 = b64_val(in[i + 2u]), c3 = b64_val(in[i + 3u]);

        if (c0 < 0 || c1 < 0) return 0u;                 /* first two are mandatory */

        uint32_t v = ((uint32_t)c0 << 18) | ((uint32_t)c1 << 12);
        size_t   produce = 1u;

        if (c2 >= 0) { v |= (uint32_t)c2 << 6; produce = 2u; }
        if (c3 >= 0) { v |= (uint32_t)c3;      produce = 3u; }

        if ((o + produce) > out_cap) return 0u;          /* bounds, every group */

        out[o++] = (uint8_t)(v >> 16);
        if (produce > 1u) out[o++] = (uint8_t)(v >> 8);
        if (produce > 2u) out[o++] = (uint8_t) v;
    }
    return o;
}
```

Three bytes become four characters, six bits at a time. **The 24-bit accumulator is what makes it readable** — pack the group, then extract four 6-bit fields. Doing it with running bit offsets works and is much easier to get wrong.

**The padding cases are the question.** `n % 3` decides everything: a remainder of 1 produces two characters plus `==`, a remainder of 2 produces three plus `=`. Handling only the full groups silently truncates, and the decoder on the far end gets a short buffer with no error — which is why base64 bugs show up as "the last byte of the key is missing".

**Size the output before writing anything.** `((n + 2) / 3) * 4` is the ceiling-division idiom, and returning 0 on insufficient capacity rather than writing what fits is the safe contract. Cost is 33% expansion, which matters when you are putting a certificate in a 20 KB device.

Two things worth adding: **base64url** substitutes `-` and `_` for `+` and `/` so the output is safe in URLs and JWTs — a one-line table change that comes up constantly in IoT provisioning. And the decoder should reject `n % 4 != 0` up front, since a stream missing its padding is a truncation you want reported, not silently decoded.
</details>

---

### 9. XMODEM receive — retries and sequencing `SEN`

```c
int xmodem_recv(uint8_t *dst, size_t cap);
```

<details><summary>Solution</summary>

```c
#define SOH  0x01u
#define EOT  0x04u
#define ACK  0x06u
#define NAK  0x15u
#define CAN  0x18u

#define BLK  128u
#define RETRY_MAX 10u

int xmodem_recv(uint8_t *dst, size_t cap)
{
    uint8_t  blk[BLK];
    uint8_t  expect = 1u;                  /* block numbers start at 1, wrap at 255 */
    size_t   total  = 0u;
    uint32_t retries = 0u;

    uart_putc(NAK);                        /* NAK starts a checksum-mode transfer */

    for (;;) {
        int c = uart_getc_timeout(1000u);          /* 1 s per the spec */

        if (c < 0) {                               /* timeout */
            if (++retries > RETRY_MAX) return -1;
            uart_putc(NAK);                        /* ask for a resend */
            continue;
        }

        if (c == EOT) { uart_putc(ACK); return (int)total; }
        if (c == CAN) { return -1; }                /* sender aborted */
        if (c != SOH) { continue; }                 /* junk: resync on SOH */

        int bn  = uart_getc_timeout(1000u);
        int bnc = uart_getc_timeout(1000u);
        if (bn < 0 || bnc < 0 || ((bn + bnc) & 0xFFu) != 0xFFu) {
            if (++retries > RETRY_MAX) return -1;
            uart_flush_rx();
            uart_putc(NAK);
            continue;
        }

        uint8_t sum = 0u;
        bool     ok  = true;
        for (size_t i = 0u; i < BLK; i++) {
            int d = uart_getc_timeout(1000u);
            if (d < 0) { ok = false; break; }
            blk[i] = (uint8_t)d;
            sum = (uint8_t)(sum + blk[i]);
        }
        int rxsum = uart_getc_timeout(1000u);

        if (!ok || rxsum < 0 || (uint8_t)rxsum != sum) {
            if (++retries > RETRY_MAX) return -1;
            uart_flush_rx();
            uart_putc(NAK);
            continue;
        }

        /* ---- DUPLICATE: our ACK was lost, sender resent. ACK, do not store. ---- */
        if ((uint8_t)bn == (uint8_t)(expect - 1u)) {
            uart_putc(ACK);
            retries = 0u;
            continue;
        }

        if ((uint8_t)bn != expect) {               /* out of sequence: unrecoverable */
            uart_putc(CAN);
            return -1;
        }

        if ((total + BLK) > cap) { uart_putc(CAN); return -1; }   /* would overflow */

        memcpy(&dst[total], blk, BLK);
        total  += BLK;
        expect  = (uint8_t)(expect + 1u);          /* wraps 255 → 0 → 1 naturally */
        retries = 0u;
        uart_putc(ACK);
    }
}
```

**The duplicate-block case is why this problem is worth keeping**, and it is the part almost everyone omits. If the receiver's ACK is lost, the sender times out and **resends the same block**. A receiver that stores it appends 128 duplicate bytes and every subsequent block lands at the wrong offset — the file is corrupt, and the transfer reports success. Recognising that the block number can legitimately be *one less than expected*, and that the correct response is ACK-without-storing, is the whole insight.

The rest is the anatomy of any reliable transfer over an unreliable link, and it generalises far beyond XMODEM:

| Element | Purpose |
|---|---|
| Block number **and its complement** | detects corruption of the header itself |
| Checksum per block | detects corruption of the payload |
| ACK / NAK | per-block acknowledgement, so one bad block costs 128 bytes not the file |
| Timeout with retry limit | the link can fail silently; unbounded retries hang forever |
| `uart_flush_rx()` before NAK | discard the tail of the bad block, or it is parsed as the next header |
| Capacity check before `memcpy` | a longer-than-expected transfer must not overflow the destination |

**Flushing the receive buffer before sending NAK** is the subtle one: without it, the remaining bytes of the rejected block are still queued and get interpreted as the next block's header, so the retry fails too and the transfer collapses after a single bit error.

Worth naming as the modern context: XMODEM-CRC (start with `C` instead of NAK, use CRC-16 instead of a sum) and YMODEM (1024-byte blocks, filename and size in block 0) fix the obvious weaknesses — an 8-bit sum misses about 1 in 256 corruptions, which over a firmware image is not acceptable. The reason to know XMODEM at all is that it is still the fallback bootloader protocol on a great many products, precisely because it fits in a few hundred bytes of ROM.
</details>

---

## Part 10 — retention table

| Mechanism | The detail that decides it |
|---|---|
| Byte-fed state machine | **validate LEN against capacity before indexing** |
| Partial frame | inter-byte timeout, or the parser wedges forever |
| Resync after error | reconsider the rejected byte — it may be the next SOF |
| COBS | removes all zeros → `0x00` delimits; overhead ⌈n/254⌉, **deterministic** |
| SLIP | worst case **2n** — every buffer must be sized for it |
| HDLC | flag byte resets unconditionally = self-synchronising |
| CRC magic value | run the CRC over data+FCS; a good frame yields a constant |
| Modbus RTU | **no delimiters at all** — 3.5 char silence; needs a timer, not just RX |
| Modbus byte order | CRC little-endian, everything else big-endian |
| NMEA | **never `strtok`** — it collapses the empty fields that carry meaning |
| AT dispatch | table not if-chain; longest prefix first; filter echo |
| TLV | `vlen > (len - off)`, never `off + vlen > len` (overflow) |
| Base64 | size the output first; `n % 3` drives the padding |
| XMODEM | block number one *less* than expected = duplicate → ACK, don't store |
| Any retry protocol | flush the RX buffer before NAK |

**The question to ask of any parser:** where does it get its length from, what happens if that length is a lie, and how does it find the next frame after it gives up on this one?

---

# Part 08 — RTOS Primitives

10 entries from 50. These are asked nearly as often as ring buffers, and they are asked *conceptually* as much as in code — "mutex or semaphore here, and why" is a design question with a right answer.

> [!IMPORTANT]
> **The distinction that underpins this entire part.** A semaphore is a **signal**: it counts events, and whoever waits on it need have nothing to do with whoever posts it. A mutex is **ownership**: exactly one task holds it, only that task may release it, and the kernel knows who that is — which is what makes priority inheritance possible. Using a semaphore where you meant a mutex compiles, runs, and silently loses you the only protection that matters.

---

### 1. Cooperative scheduler and the context switch `SEN`

```c
void sched_start(void);
void task_yield(void);
```

<details><summary>Solution</summary>

```c
#define MAX_TASKS 8

enum tstate { T_UNUSED = 0, T_READY, T_BLOCKED, T_SUSPENDED };

struct tcb {
    uint32_t   *sp;                 /* saved stack pointer — MUST be first member */
    enum tstate state;
    uint8_t     prio;
    uint32_t   *stack_base;
    size_t      stack_words;
};

static struct tcb  g_tcb[MAX_TASKS];
struct tcb        *g_current;       /* used by the assembly in PendSV */
struct tcb        *g_next;

/* Build a stack frame that looks exactly like one the hardware just pushed. */
int task_create(void (*entry)(void), uint32_t *stack, size_t words, uint8_t prio)
{
    for (int i = 0; i < MAX_TASKS; i++) {
        if (g_tcb[i].state != T_UNUSED) continue;

        uint32_t *sp = &stack[words];              /* stacks grow DOWN */
        sp = (uint32_t *)((uintptr_t)sp & ~0x7UL); /* 8-byte align per AAPCS */

        *(--sp) = 0x01000000UL;                    /* xPSR: Thumb bit set */
        *(--sp) = (uint32_t)entry;                 /* PC  */
        *(--sp) = 0xFFFFFFFDUL;                    /* LR  = EXC_RETURN, thread/PSP */
        *(--sp) = 0u;                              /* R12 */
        *(--sp) = 0u; *(--sp) = 0u;                /* R3, R2 */
        *(--sp) = 0u; *(--sp) = 0u;                /* R1, R0 */
        for (int k = 0; k < 8; k++) *(--sp) = 0u;  /* R11..R4 — WE save these */

        g_tcb[i].sp          = sp;
        g_tcb[i].state       = T_READY;
        g_tcb[i].prio        = prio;
        g_tcb[i].stack_base  = stack;
        g_tcb[i].stack_words = words;
        return i;
    }
    return -1;
}

void task_yield(void)
{
    g_next = sched_pick_highest();                 /* entry 7 */
    if (g_next != g_current) {
        SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;        /* switch on the way out */
        __DSB();
    }
}
```

```asm
/* The context switch itself must be assembly: C cannot guarantee which
   registers are live, and the stack pointer is being swapped underneath it. */
PendSV_Handler:
    MRS     R0, PSP                 /* current task's stack               */
    STMDB   R0!, {R4-R11}           /* push the callee-saved half          */
    LDR     R1, =g_current
    LDR     R2, [R1]
    STR     R0, [R2]                /* tcb->sp = R0  (sp is offset 0)      */

    LDR     R1, =g_next
    LDR     R2, [R1]
    LDR     R3, =g_current
    STR     R2, [R3]                /* g_current = g_next                  */

    LDR     R0, [R2]                /* new task's saved sp                 */
    LDMIA   R0!, {R4-R11}           /* pop the callee-saved half           */
    MSR     PSP, R0
    BX      LR                      /* hardware pops R0-R3,R12,LR,PC,xPSR  */
```

**The hardware does half the work and you do the other half — that split is the answer.** On exception entry Cortex-M automatically stacks R0–R3, R12, LR, PC and xPSR (the caller-saved set). R4–R11 are callee-saved, so the compiler assumes they survive a call and the hardware does *not* push them. The switcher's entire job is those eight registers plus the stack-pointer swap.

Three details that get probed:

- **`sp` must be the first member of the TCB**, so the assembly can use `STR R0, [R2]` with no offset. A comment saying so is what stops someone reordering the struct and breaking it silently.
- **`EXC_RETURN = 0xFFFFFFFD`** means return to Thread mode using the Process Stack Pointer. Kernels run tasks on PSP and handlers on MSP, so a stack overflow in a task cannot corrupt the exception path.
- **The `xPSR` Thumb bit must be set.** Clear it and the first `BX` to the task entry faults immediately — a classic bring-up failure with a completely unhelpful HardFault.

**Why `PendSV` rather than switching inside the ISR:** `PendSV` is set to the *lowest* priority, so it runs only after every other pending interrupt has completed. That means a switch triggered from a high-priority ISR does not delay other interrupts, and there is never a switch attempted while another handler is mid-flight. This is the same deferred-work mechanism as Part 07 entry 5, doing the most important deferred job in the system.

Cooperative versus preemptive is the follow-up: cooperative switches only at `task_yield`, so no critical sections are needed around shared data *between tasks* — a real simplification. It fails when one task runs long, since nothing can preempt it, and a single missing `yield` hangs everything.
</details>

---

### 2. Binary and counting semaphores `INT`→`SEN`

```c
bool sem_take(struct sem *s, uint32_t timeout_ticks);
void sem_give(struct sem *s);
```

<details><summary>Solution</summary>

```c
struct sem {
    volatile uint32_t count;
    uint32_t           max;         /* 1 == binary, >1 == counting */
    struct tcb        *waiters[MAX_TASKS];
    uint8_t            n_waiters;
};

bool sem_take(struct sem *s, uint32_t timeout)
{
    uint32_t st = crit_enter();

    if (s->count > 0u) {
        s->count--;                            /* got it immediately */
        crit_exit(st);
        return true;
    }
    if (timeout == 0u) {                       /* non-blocking poll */
        crit_exit(st);
        return false;
    }

    g_current->state = T_BLOCKED;
    sem_add_waiter(s, g_current);              /* insert by priority, entry 7 */
    task_set_timeout(g_current, timeout);
    crit_exit(st);

    task_yield();                              /* returns when given or timed out */
    return g_current->wait_result;
}

void sem_give(struct sem *s)
{
    uint32_t st = crit_enter();

    struct tcb *t = sem_pop_highest_waiter(s);
    if (t != NULL) {
        t->state       = T_READY;
        t->wait_result = true;                 /* hand the token DIRECTLY over */
    } else if (s->count < s->max) {
        s->count++;                            /* nobody waiting: bank it */
    }
    /* count saturates at max — an over-give is silently dropped */
    crit_exit(st);

    if (t != NULL && t->prio < g_current->prio) task_yield();
}
```

**Binary and counting differ only in `max`, and that one field changes what the primitive is for.**

| | Holds | Use for |
|---|---|---|
| **Binary** (`max == 1`) | one pending signal | "the DMA finished", "a key was pressed" |
| **Counting** (`max == N`) | N pending signals | resource pools, and **every case where events can arrive faster than they are consumed** |

**The binary semaphore's saturation is the trap.** Two DMA completions before the task runs leave the count at 1, so one event is silently lost. That is the same limitation as the single flag in Part 07 entry 4, and it is why a counting semaphore or a queue is correct whenever the producer can outpace the consumer. Naming that unprompted is the difference between knowing the API and knowing what it does.

**Handing the token directly to a waiter, rather than incrementing and letting it wake and decrement, prevents theft** — a third task running between the give and the wake would otherwise take the token, and the intended waiter blocks again. That is a real starvation bug in naive implementations.

Two more: waiters are woken **highest priority first**, not FIFO, or you have built priority inversion into the primitive itself. And the yield-if-woken-task-is-higher-priority at the end is what makes the release immediate rather than waiting for the next tick — the task-context twin of `portYIELD_FROM_ISR`.
</details>

---

### 3. Mutex, priority inversion, and inheritance `SEN`

```c
bool mutex_lock(struct mutex *m, uint32_t timeout);
void mutex_unlock(struct mutex *m);
```

<details><summary>Solution</summary>

```c
struct mutex {
    struct tcb *owner;              /* NULL == free. A semaphore has no equivalent. */
    uint8_t     owner_orig_prio;    /* saved so inheritance can be undone */
    struct tcb *waiters[MAX_TASKS];
    uint8_t     n_waiters;
};

bool mutex_lock(struct mutex *m, uint32_t timeout)
{
    uint32_t st = crit_enter();

    if (m->owner == NULL) {
        m->owner           = g_current;
        m->owner_orig_prio = g_current->prio;
        crit_exit(st);
        return true;
    }

    /* ---- PRIORITY INHERITANCE: lift the owner to our priority ---- */
    if (g_current->prio < m->owner->prio) {          /* lower number == higher prio */
        m->owner->prio = g_current->prio;
        sched_requeue(m->owner);                     /* move it in the ready queue */
    }

    g_current->state = T_BLOCKED;
    mutex_add_waiter(m, g_current);
    task_set_timeout(g_current, timeout);
    crit_exit(st);

    task_yield();
    return g_current->wait_result;
}

void mutex_unlock(struct mutex *m)
{
    uint32_t st = crit_enter();

    if (m->owner != g_current) {                     /* ---- ownership check ---- */
        crit_exit(st);
        return;                                      /* or assert: this is a bug */
    }

    g_current->prio = m->owner_orig_prio;            /* drop the inherited priority */

    struct tcb *t = mutex_pop_highest_waiter(m);
    if (t != NULL) {
        m->owner           = t;
        m->owner_orig_prio = t->prio;
        t->state           = T_READY;
        t->wait_result     = true;
    } else {
        m->owner = NULL;
    }
    crit_exit(st);

    if (t != NULL && t->prio < g_current->prio) task_yield();
}
```

**This is the most-asked RTOS question, and it has three parts.**

**Mutex versus semaphore.** A mutex has an `owner`; a binary semaphore does not. Everything follows from that field:

| | Mutex | Binary semaphore |
|---|---|---|
| Purpose | mutual exclusion | signalling |
| Release | **only by the owner** | by anyone, including an ISR |
| Priority inheritance | yes — the kernel knows who to boost | impossible, nobody to boost |
| Recursive | optionally (entry 4) | no |
| Usable from an ISR | **no** — an ISR is not a task and cannot own | yes |

**Priority inversion**, the failure it prevents. Low-priority task L takes the mutex. High-priority H preempts and blocks on it. Medium-priority M — which wants nothing to do with the mutex — then preempts L and runs indefinitely. **H is now blocked by M, a task of lower priority than itself**, for unbounded time. This is not academic: it is what nearly lost Mars Pathfinder in 1997, where a watchdog kept resetting the lander because a high-priority bus task was blocked behind a low-priority meteorological task.

**Priority inheritance**, the fix. While H is blocked on the mutex, L temporarily runs at H's priority, so M cannot preempt it. L finishes, releases, and drops back. The bound on H's delay becomes the length of L's critical section rather than unbounded.

Three details worth stating: **inheritance must be undone on unlock**, or L keeps a high priority forever and starves everything; the ownership check makes a foreign unlock a detectable bug rather than silent corruption; and the alternative scheme, **priority ceiling**, raises the priority on *acquisition* rather than on contention — deterministic and deadlock-preventing, but pessimistic, which is why it appears in safety-critical designs and inheritance appears everywhere else.

Closing point that lands well: **the real fix is to keep the critical section short.** Inheritance bounds the damage; it does not eliminate it.
</details>

---

### 4. Recursive mutex `SEN`

```c
bool rmutex_lock(struct rmutex *m, uint32_t timeout);
void rmutex_unlock(struct rmutex *m);
```

<details><summary>Solution</summary>

```c
struct rmutex {
    struct mutex base;
    uint32_t     depth;
};

bool rmutex_lock(struct rmutex *m, uint32_t timeout)
{
    uint32_t st = crit_enter();
    if (m->base.owner == g_current) {          /* already ours: just count */
        m->depth++;
        crit_exit(st);
        return true;
    }
    crit_exit(st);

    if (!mutex_lock(&m->base, timeout)) return false;
    m->depth = 1u;
    return true;
}

void rmutex_unlock(struct rmutex *m)
{
    uint32_t st = crit_enter();
    if (m->base.owner != g_current) { crit_exit(st); return; }

    if (--m->depth > 0u) {                     /* still nested: do NOT release */
        crit_exit(st);
        return;
    }
    crit_exit(st);
    mutex_unlock(&m->base);
}
```

**The problem it solves:** a function that takes a lock calls another function in the same module that takes the same lock. With a plain mutex, the task blocks waiting for a lock **it already holds** — an instant, permanent self-deadlock. With a recursive mutex the owner check succeeds and a depth counter increments.

**The problem it usually indicates**, which is the more valuable half of the answer: recursive locking is nearly always a symptom of unclear layering. The standard fix is to split each public function into a thin locking wrapper and an internal `_locked` version that assumes the lock is already held:

```c
static void queue_push_locked(...);            /* assumes lock held, takes nothing */

void queue_push(...)                           /* public: locks, delegates, unlocks */
{
    mutex_lock(&m, WAIT);
    queue_push_locked(...);
    mutex_unlock(&m);
}
```

That is explicit, cheaper, and reviewable — you can see from the name which functions expect the lock. Recursive mutexes hide the question.

Two hazards to name: **the depth counter must reach zero before the lock is released**, so an unbalanced unlock leaves it held forever with no error; and recursion **interacts badly with priority inheritance**, since the owner's priority must not be restored until the outermost release. That is why some kernels do not offer recursive mutexes at all, and why MISRA-adjacent coding standards discourage them.
</details>

---

### 5. Message queue with blocking `SEN`

```c
bool q_send(struct mq *q, const void *item, uint32_t timeout);
bool q_recv(struct mq *q, void *item, uint32_t timeout);
```

<details><summary>Solution</summary>

```c
struct mq {
    uint8_t    *buf;                /* item_size * capacity */
    size_t      item_size;
    size_t      capacity;
    size_t      head, tail, count;
    struct tcb *send_waiters[MAX_TASKS];   uint8_t n_send;
    struct tcb *recv_waiters[MAX_TASKS];   uint8_t n_recv;
};

bool q_send(struct mq *q, const void *item, uint32_t timeout)
{
    uint32_t st = crit_enter();

    while (q->count == q->capacity) {                  /* full */
        if (timeout == 0u) { crit_exit(st); return false; }

        g_current->state = T_BLOCKED;
        add_waiter(q->send_waiters, &q->n_send, g_current);
        task_set_timeout(g_current, timeout);
        crit_exit(st);
        task_yield();
        if (!g_current->wait_result) return false;      /* timed out */
        st = crit_enter();                              /* RE-CHECK, don't assume */
    }

    memcpy(&q->buf[q->head * q->item_size], item, q->item_size);   /* copy BY VALUE */
    q->head = (q->head + 1u) % q->capacity;
    q->count++;

    struct tcb *t = pop_highest(q->recv_waiters, &q->n_recv);
    if (t != NULL) { t->state = T_READY; t->wait_result = true; }
    crit_exit(st);

    if (t != NULL && t->prio < g_current->prio) task_yield();
    return true;
}
```

**The queue copies the item by value, and that is the design decision to defend.** Passing a pointer instead is faster and immediately raises three questions the copy makes disappear: who owns the buffer, when may it be reused, and is it still alive when the receiver reads it. A pointer to a sender's stack local is a use-after-scope bug waiting for a scheduling delay to expose it. Copy small items; for large payloads pass a pointer to a **pool block** (Part 05 entry 14) so ownership is explicit and refcounted.

**`while`, not `if`, around the blocking wait.** After waking, the condition must be re-checked: between the wake and the re-acquisition of the critical section another task may have filled the queue again. Writing `if` here is the classic spurious-wakeup bug — it works under light load and corrupts under contention.

Two more points that get asked:

- **Both directions can block**, so a queue is two waiter lists. A full queue blocking the sender is the mechanism that gives you **backpressure**: a fast producer is throttled to the consumer's rate instead of dropping data. That is often exactly what you want, and it is also how you deadlock two tasks that send to each other.
- **The full-queue policy must be a decision**, as in Part 05 entry 6: block, drop-newest, or overwrite-oldest. `timeout == 0` gives the non-blocking variant, which is the only form callable from an ISR — and from an ISR it must be the `FromISR` API (Part 07 entry 9).
</details>

---

### 6. Event groups `SEN`

```c
uint32_t ev_wait(struct evgroup *g, uint32_t mask, bool wait_all, uint32_t timeout);
void     ev_set(struct evgroup *g, uint32_t bits);
```

<details><summary>Solution</summary>

```c
struct evgroup {
    volatile uint32_t bits;
    struct tcb       *waiters[MAX_TASKS];
    uint32_t          wait_mask[MAX_TASKS];
    bool              wait_all[MAX_TASKS];
    uint8_t           n;
};

static bool ev_satisfied(uint32_t bits, uint32_t mask, bool all)
{
    return all ? ((bits & mask) == mask)      /* AND: every bit */
               : ((bits & mask) != 0u);       /* OR:  any bit   */
}

uint32_t ev_wait(struct evgroup *g, uint32_t mask, bool all, uint32_t timeout)
{
    uint32_t st = crit_enter();

    while (!ev_satisfied(g->bits, mask, all)) {
        if (timeout == 0u) { crit_exit(st); return g->bits; }

        g_current->state = T_BLOCKED;
        ev_add_waiter(g, g_current, mask, all);
        task_set_timeout(g_current, timeout);
        crit_exit(st);
        task_yield();
        if (!g_current->wait_result) return g->bits;      /* timeout */
        st = crit_enter();
    }

    uint32_t snapshot = g->bits;
    g->bits &= ~mask;                        /* consume — see the note below */
    crit_exit(st);
    return snapshot;
}

void ev_set(struct evgroup *g, uint32_t bits)
{
    uint32_t st = crit_enter();
    g->bits |= bits;

    for (uint8_t i = 0u; i < g->n; i++) {    /* possibly wake SEVERAL tasks */
        if (ev_satisfied(g->bits, g->wait_mask[i], g->wait_all[i])) {
            g->waiters[i]->state       = T_READY;
            g->waiters[i]->wait_result = true;
        }
    }
    crit_exit(st);
}
```

**What an event group does that a semaphore cannot: wait on a *combination* of conditions.** "Proceed when the network is up **and** the config has loaded **and** the sensor has calibrated" is one `ev_wait` with `wait_all = true`. Building that from three semaphores means taking them in some order and getting the priority-inversion and ordering problems for free.

`wait_all` is the AND/OR switch, and both forms are used: AND for "all prerequisites met", OR for "any of these three error conditions".

**The subtlety worth raising unprompted is who clears the bits.** With several tasks waiting on overlapping masks, one task consuming `mask` can clear a bit another was waiting for, and the second task blocks forever on an event that already happened. FreeRTOS makes this explicit with a `xClearOnExit` parameter for exactly this reason. Options: only the setter clears, each waiter uses a disjoint bit, or bits are sticky and treated as state rather than events. Picking one deliberately is the answer; not noticing the question is the failure.

Note `ev_set` may wake **multiple** tasks in one call, which is the other difference from a semaphore — and it is safe from an ISR in its `FromISR` form, which is how a driver signals "transfer complete" to several interested tasks at once.
</details>

---

### 7. O(1) priority-based ready queue `SEN`

```c
struct tcb *sched_pick_highest(void);
```

<details><summary>Solution</summary>

```c
#define N_PRIO 32                       /* 0 == highest */

static struct tcb  *g_ready[N_PRIO];    /* head of a list per priority level */
static uint32_t     g_ready_mask;       /* bit p set == level p is non-empty */

void sched_make_ready(struct tcb *t)
{
    t->next = g_ready[t->prio];
    g_ready[t->prio] = t;
    g_ready_mask |= (1UL << t->prio);           /* Part 01 entry 1 */
}

void sched_remove_ready(struct tcb *t)
{
    /* ... unlink from g_ready[t->prio] ... */
    if (g_ready[t->prio] == NULL) {
        g_ready_mask &= ~(1UL << t->prio);      /* level now empty */
    }
}

/* Constant time, no scanning, regardless of task count. */
struct tcb *sched_pick_highest(void)
{
    if (g_ready_mask == 0UL) return &g_idle_tcb;

    uint8_t p = ctz32(g_ready_mask);            /* lowest set bit == highest prio */
    return g_ready[p];
}
```

**A bitmap plus a count-trailing-zeros is how a real-time kernel picks the next task in constant time**, and it is a satisfying payoff for Part 01 entry 8: one bit per priority level, one `CLZ`/`CTZ` instruction to find the highest occupied level, one array index to reach the list. On Cortex-M3+ that is about three instructions total.

**Why O(1) is a requirement and not an optimisation.** The scheduler runs on every tick and every switch, so an O(n) scan over 30 tasks means scheduler cost grows with system size — and worst-case latency becomes a function of how many tasks happen to exist, which destroys the timing analysis a real-time system depends on. Determinism is the product; the speed is a side effect.

Details that come up:

- **Numerically lower = higher priority** here (matching Cortex-M NVIC and FreeRTOS's internal convention, though FreeRTOS's *API* inverts it). State your convention, because half the confusion in RTOS interviews is people using opposite ones.
- **32 levels fit one word.** More needs a two-level bitmap — a summary word of which groups are non-empty, then a word per group. That is how Linux's O(1) scheduler and TLSF-style allocators handle 256 levels, and naming it shows the pattern generalises.
- **Same-priority tasks round-robin** by rotating the list head on each tick, which is where a time slice comes from.
- **The idle task must always exist** at the lowest priority, so `pick_highest` never returns NULL. Its body is entry 10.
</details>

---

### 8. Deadlock — nested locks and how to prevent it `SEN`

```c
/* Two tasks, two mutexes. */
```

<details><summary>Solution</summary>

```c
/* ---- BROKEN: classic AB-BA deadlock ---- */
void task_a(void)
{
    mutex_lock(&m_sensor, WAIT);
    mutex_lock(&m_log, WAIT);            /* B waits here holding m_log */
    /* ... */
    mutex_unlock(&m_log);
    mutex_unlock(&m_sensor);
}

void task_b(void)
{
    mutex_lock(&m_log, WAIT);            /* opposite order */
    mutex_lock(&m_sensor, WAIT);         /* A waits here holding m_sensor */
    /* ... */
    mutex_unlock(&m_sensor);
    mutex_unlock(&m_log);
}

/* ---- FIX 1: a global lock ORDER, enforced by rank ---- */
#define RANK_SENSOR 1u
#define RANK_LOG    2u                   /* always acquire in ascending rank */

void task_b_fixed(void)
{
    mutex_lock(&m_sensor, WAIT);         /* rank 1 first, always */
    mutex_lock(&m_log, WAIT);            /* then rank 2 */
    mutex_unlock(&m_log);
    mutex_unlock(&m_sensor);
}

/* ---- FIX 2: never block on the second lock (backoff) ---- */
bool try_both(void)
{
    if (!mutex_lock(&m_log, 0u)) return false;
    if (!mutex_lock(&m_sensor, 0u)) {
        mutex_unlock(&m_log);            /* release what we hold and retry later */
        return false;
    }
    return true;
}

/* ---- FIX 3 (debug builds): assert the ordering ---- */
static uint32_t t_held_ranks;            /* per-task in a real implementation */

void ranked_lock(struct mutex *m, uint32_t rank)
{
    configASSERT(rank > (32u - clz32(t_held_ranks | 1u)));   /* strictly ascending */
    mutex_lock(m, WAIT);
    t_held_ranks |= (1UL << rank);
}
```

**Four conditions must all hold for deadlock**, and naming them is the structured answer: mutual exclusion, hold-and-wait, no preemption of a held lock, and circular wait. Break any one and deadlock is impossible.

**Consistent lock ordering breaks the circular wait, and it is the fix that scales.** Assign every lock a rank and require ascending acquisition. It costs nothing at runtime, it is checkable by review, and in a debug build the assertion above catches a violation at the moment it happens rather than during a field hang. Linux's lockdep is this idea industrialised.

**Timeout-and-backoff breaks hold-and-wait**, and is what you use when ordering is genuinely impossible — but it introduces livelock, where two tasks retry in lockstep forever. Randomised backoff fixes that, which is the same shape as entry 4 of the state machine part.

The practical points that make this an experienced answer:

- **Deadlock is load-dependent and near-unreproducible.** It needs a specific interleaving, so it passes every test and appears in the field. That is why prevention by design beats detection.
- **A watchdog is your last line** (Part 09 entry 12), and the per-task liveness pattern is what turns a deadlock into a logged reset rather than a silent hang.
- **The best fix is often to not need two locks.** One coarser lock, or restructuring so the two resources are owned by a single task and reached by message passing, removes the problem rather than managing it. Offering that is the senior answer.
</details>

---

### 9. Stack sizing and overflow detection `INT`

```c
void   stack_paint(uint32_t *base, size_t words);
size_t stack_high_water(const uint32_t *base, size_t words);
```

<details><summary>Solution</summary>

```c
#define PAINT 0xA5A5A5A5UL

void stack_paint(uint32_t *base, size_t words)
{
    for (size_t i = 0u; i < words; i++) base[i] = PAINT;
}

/* Words ever used. Stacks grow DOWN, so scan up from the low end. */
size_t stack_high_water(const uint32_t *base, size_t words)
{
    size_t untouched = 0u;
    while (untouched < words && base[untouched] == PAINT) {
        untouched++;
    }
    return words - untouched;
}

/* Called at every context switch: cheap, catches most overflows. */
bool stack_check(const struct tcb *t)
{
    return (t->stack_base[0] == PAINT) &&              /* bottom word intact */
           ((uintptr_t)t->sp >= (uintptr_t)t->stack_base);
}
```

**Why an overflow is so hard to diagnose without this.** On a Cortex-M with no MPU there is no fault at the moment of overflow — the write simply lands in whatever is below, usually another task's stack or `.bss`. The symptom appears later and elsewhere: a global that changes with nothing writing to it, or a HardFault with a nonsense PC. That is Part 13 entry 9, and painting is how you get ahead of it.

**Painting gives you the number you actually need**, which is not "did it overflow" but "how much headroom is there". Run the system through its worst-case path — deepest call chain, all interrupts nesting, `printf` with floats, the error handler — then read the high-water marks. Sizing from measurement is the only defensible method; picking 1024 because it looks generous is how you get a 4 KB stack for a task using 200 bytes and a 512-byte stack for one that needs 600.

Four defences, in increasing strength — worth listing, because "how would you have caught it" is the follow-up:

| Technique | Catches |
|---|---|
| Painting + high-water inspection | how much margin you have, before it matters |
| Bottom-word canary at each switch | most overflows, one comparison, cheap enough for production |
| `-fstack-usage` + call-graph analysis | oversized frames at **compile time**, no runtime cost |
| **MPU region with no access below the stack** | the overflow itself, as a fault with a usable PC |

The MPU is the only one that catches the write *at the instruction that does it*, and on parts that have one it is worth the setup.

Two things to add: **ISR stack usage is separate** — handlers run on MSP, so the main stack must be sized for the worst nesting depth of interrupts plus their locals, and that is easy to forget when every task stack has been carefully measured. And `printf` with floating point can consume over a kilobyte in one call, which is the usual reason a task that "obviously" needs 256 bytes overflows.
</details>

---

### 10. Idle task and tickless idle `INT`→`STA`

```c
void idle_task(void);
void tickless_idle(uint32_t idle_ticks);
```

<details><summary>Solution</summary>

```c
void idle_task(void)
{
    for (;;) {
        /* Race-free sleep: see Part 07 entry 7 for why the masking is required. */
        __disable_irq();
        if (g_ready_mask == 0UL) {           /* re-check with interrupts off */
            __WFI();                         /* wakes on a PENDING irq even so */
        }
        __enable_irq();
        __ISB();
    }
}

/* Stop the periodic tick and sleep for the whole idle period at once. */
void tickless_idle(uint32_t idle_ticks)
{
    if (idle_ticks < 2u) return;                        /* not worth it */

    uint32_t max = SYSTICK_MAX / TICK_RELOAD;
    if (idle_ticks > max) idle_ticks = max;             /* 24-bit counter limit */

    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->LOAD  = (idle_ticks * TICK_RELOAD) - 1u;
    SysTick->VAL   = 0u;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    __disable_irq();
    if (g_ready_mask == 0UL) __WFI();
    __enable_irq();

    /* ---- On waking, work out how long we ACTUALLY slept ---- */
    uint32_t remaining = SysTick->VAL;
    uint32_t elapsed_ticks = (SysTick->LOAD == 0u) ? idle_ticks
                           : idle_ticks - (remaining / TICK_RELOAD);

    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->LOAD  = TICK_RELOAD - 1u;                  /* restore the normal tick */
    SysTick->VAL   = 0u;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    g_ticks += elapsed_ticks;                            /* CATCH UP the clock */
    timers_advance(elapsed_ticks);
}
```

**The idle task exists so `pick_highest` never returns NULL**, and its body should be `WFI` rather than a spin. A busy idle loop draws full operating current for no work — often the difference between a year of battery life and a month.

**Tickless idle is the real content.** A 1 kHz tick wakes the core 1000 times a second even when nothing is scheduled, and each wake costs the exception entry, the tick handler, and the return to sleep. On a device that is idle 99.9% of the time, the tick is nearly the entire power budget. Tickless reprogrammes the timer for the *whole* idle interval and sleeps once.

Three things it must get right, and each is a distinct bug:

- **Catch the tick counter up on waking.** Sleeping 500 ms with the tick stopped means 500 ticks did not happen; failing to add them makes every timeout in the system late and the wall clock drift permanently.
- **Compute the actual elapsed time**, because the wake may be an unrelated interrupt long before the scheduled expiry. Assuming the full interval elapsed is a common and quietly wrong shortcut.
- **Respect the counter width.** SysTick is 24-bit, so at 72 MHz the maximum sleep is about 233 ms; a longer idle period must be split, or a low-power timer used instead. That is also why deep sleep uses an RTC or LPTIM, which keeps running when the core clock stops entirely.

Worth adding: the deeper sleep modes stop peripheral clocks and may lose SRAM or peripheral state, so the wake path has to reconfigure. And measuring idle percentage — a counter incremented in the idle loop, sampled per second — is the standard way to report CPU load, which is a question that follows this one naturally.
</details>

---

## Part 08 — retention table

| Concept | The point |
|---|---|
| Mutex vs semaphore | **ownership** — a mutex knows who holds it, so it can inherit priority |
| Priority inversion | H blocked behind L, preempted by unrelated M. Mars Pathfinder |
| Priority inheritance | boost L to H's priority while H waits; **undo it on unlock** |
| Priority ceiling | boost on acquisition — deterministic, pessimistic, safety-critical |
| Binary semaphore | saturates at 1 — **events are silently lost** if the producer is faster |
| Direct token handover | prevents a third task stealing the signal |
| Blocking wait | `while`, not `if` — re-check the condition after waking |
| Queue by value | removes the ownership question a pointer creates |
| Full queue blocks sender | that is backpressure; it is also how you deadlock |
| Event group | wait on AND/OR of several conditions; decide **who clears the bits** |
| Ready queue | bitmap + `ctz` = O(1); determinism is the product |
| Deadlock | mutual exclusion + hold-and-wait + no preemption + circular wait |
| Deadlock fix | rank the locks, acquire ascending, assert it in debug |
| Stack sizing | paint, run the worst path, read the high-water mark |
| ISR stack is separate | MSP must cover worst-case interrupt nesting |
| Idle task | `WFI`, not a spin; tickless must **catch the tick counter up** |

**The two questions to ask of any RTOS design:** what is the longest time a high-priority task can be blocked, and by what? If you cannot answer, it is not real-time.

---

# Part 11 — State Machines

6 entries from 40. The bank asks for a traffic light, an elevator, a washing machine, a coffee machine, a vending machine, a camera shutter and a bank vault timer — those are seven statements of **one** problem. What differs between them is the state table, which is domain knowledge, not skill.

What is kept is the six genuinely distinct mechanisms: **how to implement an FSM at all, hysteresis, sample-based debouncing, timed retry with backoff, hierarchy, and a sequence where being interrupted mid-transition must not brick the device.**

---

### 1. The four implementation patterns `INT`

```c
void fsm_dispatch(struct fsm *f, enum event e);
```

<details><summary>Solution</summary>

```c
enum state { S_IDLE, S_ARMED, S_RUNNING, S_FAULT, S_COUNT };
enum event { E_START, E_STOP, E_TICK, E_ERROR, E_COUNT };

/* ---- Pattern 1: nested switch. Fine up to ~5 states. ---- */
void fsm_switch(struct fsm *f, enum event e)
{
    switch (f->st) {
    case S_IDLE:    if (e == E_START) f->st = S_ARMED;   break;
    case S_ARMED:   if (e == E_TICK)  f->st = S_RUNNING; break;
    case S_RUNNING: if (e == E_STOP)  f->st = S_IDLE;
                    else if (e == E_ERROR) f->st = S_FAULT; break;
    default: break;
    }
}

/* ---- Pattern 2: transition TABLE. The firmware default. ---- */
struct trans {
    enum state next;
    void     (*action)(struct fsm *);
};

static const struct trans k_table[S_COUNT][E_COUNT] = {
/*              E_START                E_STOP              E_TICK                 E_ERROR        */
/* S_IDLE    */{{S_ARMED,  act_arm}, {S_IDLE,   NULL}, {S_IDLE,    NULL},    {S_FAULT, act_trip}},
/* S_ARMED   */{{S_ARMED,  NULL},    {S_IDLE,   NULL}, {S_RUNNING, act_go},  {S_FAULT, act_trip}},
/* S_RUNNING */{{S_RUNNING,NULL},    {S_IDLE, act_halt},{S_RUNNING, act_run}, {S_FAULT, act_trip}},
/* S_FAULT   */{{S_FAULT,  NULL},    {S_IDLE, act_clear},{S_FAULT,  NULL},    {S_FAULT, NULL}},
};

void fsm_table(struct fsm *f, enum event e)
{
    if (f == NULL || f->st >= S_COUNT || e >= E_COUNT) return;   /* bounds! */

    const struct trans *t = &k_table[f->st][e];
    if (t->action != NULL) t->action(f);
    f->st = t->next;
}

/* ---- Pattern 3: state as a function pointer. Scales to many states. ---- */
typedef void (*state_fn)(struct fsm *, enum event);

static void st_idle(struct fsm *f, enum event e)
{
    if (e == E_START) { act_arm(f); f->fn = st_armed; }
}

void fsm_fnptr(struct fsm *f, enum event e) { f->fn(f, e); }
```

**Pattern 2 is the firmware default, and the reason is reviewability.** The table *is* the specification: every state/event pair has an explicit cell, so an unhandled combination is visible as a gap rather than hidden in a missing `else`. A reviewer can compare the table against the requirements document line by line, which is precisely what a safety audit asks for. It is also `const`, so it sits in flash rather than RAM.

Choosing between them:

| Pattern | Best when | Cost |
|---|---|---|
| Nested `switch` | ≤ 5 states, few events | becomes unreadable fast; gaps invisible |
| **Transition table** | fixed, well-specified machines | `states × events` memory even where sparse |
| Function pointer per state | many states, sparse transitions | no single place to see the whole machine |
| Table + hierarchy (entry 5) | complex, shared behaviour | most code |

**`if (f->st >= S_COUNT || e >= E_COUNT) return;` is not decoration.** A state or event value from outside the enum — corrupted memory, a wire-derived event ID — indexes past the table and calls whatever function pointer it finds. That is arbitrary code execution driven by a bad byte, and in a table-driven design it is the *one* place it can happen.

Two additions worth making: **entry and exit actions** belong on the state, not on every transition into it, or the same setup code gets duplicated across five cells and drifts. And **events should be queued**, not dispatched from an ISR, so a transition cannot be re-entered while its action is still running — dispatch from a task draining a ring buffer.
</details>

---

### 2. Hysteresis — the thermostat `INT`

```c
bool thermostat_update(struct thermo *t, int16_t temp_c10);
```

<details><summary>Solution</summary>

```c
struct thermo {
    int16_t setpoint_c10;         /* tenths of a degree */
    int16_t band_c10;             /* deadband width */
    bool    heating;
};

bool thermostat_update(struct thermo *t, int16_t temp_c10)
{
    if (t == NULL) return false;

    if (t->heating) {
        /* Only turn OFF once we are clearly above the setpoint. */
        if (temp_c10 >= t->setpoint_c10 + t->band_c10) t->heating = false;
    } else {
        /* Only turn ON once we are clearly below it. */
        if (temp_c10 <= t->setpoint_c10 - t->band_c10) t->heating = true;
    }
    return t->heating;
}
```

**The bug this exists to prevent is chatter.** A single threshold — `heating = (temp < setpoint)` — sits at the boundary and the output toggles on every ADC noise sample. On a relay that is thousands of operations an hour and mechanical failure in weeks; on a compressor it is destruction, because a compressor restarted against residual head pressure stalls and overheats.

**The current output is part of the input**, which is what makes this a state machine rather than a comparison. The threshold depends on which way you are travelling — that is exactly hysteresis, and the same structure appears in a Schmitt trigger, in carrier-detect logic, and in every level alarm.

Two things to add:

- **Deadband must exceed the noise amplitude**, or it does not help. Size it from the measured peak-to-peak noise, not from a round number — and if the noise is larger than the acceptable control band, filter the input (Part 12) rather than widening the deadband.
- **Deadband alone does not bound the cycle rate.** A slow-moving process with a narrow band can still cycle faster than the equipment allows, so real controllers add a **minimum off-time** — a timer that refuses to restart within N minutes regardless of temperature. Mentioning that is the difference between the textbook answer and one from someone who has shipped HVAC.

`int16_t` in tenths of a degree, not `float`: no FPU needed, exact arithmetic, and a range of ±3276 °C. Fixed-point choice like this is the Part 12 material showing up early.
</details>

---

### 3. N-sample debounce `INT`

```c
bool debounce_update(struct deb *d, bool raw);
```

<details><summary>Solution</summary>

```c
#define DEB_N 4u                       /* consecutive agreeing samples required */

struct deb {
    uint8_t history;                   /* shift register of recent samples */
    bool    stable;
    bool    changed;                   /* edge flag for the caller */
};

/* Call at a FIXED rate — every 5 ms is typical. */
bool debounce_update(struct deb *d, bool raw)
{
    if (d == NULL) return false;

    d->history = (uint8_t)((d->history << 1) | (raw ? 1u : 0u));

    uint8_t mask = (uint8_t)((1u << DEB_N) - 1u);
    uint8_t recent = d->history & mask;

    d->changed = false;
    if (recent == mask && !d->stable) {            /* N consecutive 1s */
        d->stable = true;  d->changed = true;
    } else if (recent == 0u && d->stable) {        /* N consecutive 0s */
        d->stable = false; d->changed = true;
    }
    return d->stable;
}
```

**A shift register plus two comparisons replaces a counter and a pile of `if`s.** Each sample shifts in; the state only changes when the last N samples all agree. Any bounce breaks the run and the timer effectively restarts, which is precisely the required behaviour.

**Sampling beats interrupts for buttons**, and this is the entry that makes the argument concretely. A bouncing contact generates hundreds of edges in a few milliseconds; an interrupt-driven approach takes hundreds of interrupts and can swamp a loaded system — a genuine denial-of-service from a dirty switch. Polling at a fixed 5 ms is bounded work, needs no debounce logic in the ISR, and gives clean edges. Use the interrupt only when you need wake-from-sleep, and then debounce in the task anyway.

The numbers, which get asked: **N × sample period must exceed the bounce time**, typically 5–50 ms for a mechanical switch. Four samples at 5 ms is 20 ms — a reasonable default, and the added latency is imperceptible to a human. Both figures belong in `#define`s with a comment naming the switch part.

The `changed` flag matters more than it looks: callers almost always want the *edge* ("a press happened"), not the level, and computing edges by comparing against a remembered previous value in every caller is how that logic gets duplicated and drifts. A `history` of `uint8_t` supports up to 8 samples; widen the type for more.
</details>

---

### 4. Retry with exponential backoff `INT`

```c
void retry_tick(struct retry *r);
```

<details><summary>Solution</summary>

```c
enum rstate { R_IDLE, R_ATTEMPT, R_WAIT, R_FAILED, R_DONE };

struct retry {
    enum rstate st;
    uint8_t     attempt;
    uint32_t    wait_until_ms;
    uint32_t    base_ms;              /* first backoff, e.g. 100 */
    uint8_t     max_attempts;         /* e.g. 6 */
};

void retry_tick(struct retry *r)
{
    if (r == NULL) return;
    uint32_t now = g_ticks;

    switch (r->st) {
    case R_IDLE:
        r->attempt = 0u;
        r->st = R_ATTEMPT;
        break;

    case R_ATTEMPT:
        if (try_operation() == 0) {                 /* success */
            r->st = R_DONE;
            break;
        }
        r->attempt++;
        if (r->attempt >= r->max_attempts) {
            r->st = R_FAILED;                        /* give up — bounded */
            break;
        }
        {
            /* Exponential: base * 2^(attempt-1), capped, plus jitter. */
            uint32_t shift = (r->attempt - 1u);
            if (shift > 8u) shift = 8u;              /* cap the growth */
            uint32_t delay = r->base_ms << shift;
            delay += (rand_u32() % (delay / 4u + 1u));   /* ±25% JITTER */
            r->wait_until_ms = now + delay;
        }
        r->st = R_WAIT;
        break;

    case R_WAIT:
        if ((uint32_t)(now - r->wait_until_ms) < 0x80000000UL) {   /* wrap-safe */
            r->st = R_ATTEMPT;
        }
        break;

    case R_FAILED:
    case R_DONE:
    default:
        break;                                       /* caller inspects and resets */
    }
}
```

**Three things must be right, and candidates usually get one.**

**Bounded attempts.** `max_attempts` means the machine reaches a terminal `R_FAILED` state that something else can act on — alert, fall back, enter a degraded mode. Retrying forever is not resilience; it is a hang with extra steps, and it hides the fault from whatever could have handled it.

**Exponential growth with a cap.** Doubling backs off quickly from a persistent failure instead of hammering a dead peer, and the cap stops the delay reaching hours. Linear retry at a fixed interval is what turns a brief server outage into a sustained load spike from every device at once.

**Jitter is the one people miss, and it is the most interesting.** Without it, a thousand devices that all lost connectivity at the same moment retry at *exactly* the same moments forever — a synchronised thundering herd that guarantees the recovering server fails again. Randomising by ±25% decorrelates them. This is a genuinely non-obvious systems insight and it lands well in an interview.

Two supporting notes: the wrap-safe comparison `(uint32_t)(now - deadline) < 0x80000000` handles the tick rollover for a signed-style "has the deadline passed" test — the same family as Part 02 entry 3. And a real implementation distinguishes **retryable** from **permanent** failures: retrying a malformed-request error six times with backoff is pure waste, so `try_operation` should return a classification, not just a boolean.
</details>

---

### 5. Hierarchical state machine `SEN`

```c
void hsm_dispatch(struct hsm *h, enum event e);
```

<details><summary>Solution</summary>

```c
/* Substates share their parent's behaviour. Unhandled events go UP. */
enum state {
    S_TOP,
      S_OPERATIONAL,          /* parent: handles E_ESTOP for all children */
        S_IDLE, S_HEATING, S_PUMPING,
      S_FAULT,
    S_COUNT
};

static const enum state k_parent[S_COUNT] = {
    [S_TOP]         = S_COUNT,          /* no parent */
    [S_OPERATIONAL] = S_TOP,
    [S_IDLE]        = S_OPERATIONAL,
    [S_HEATING]     = S_OPERATIONAL,
    [S_PUMPING]     = S_OPERATIONAL,
    [S_FAULT]       = S_TOP,
};

/* Each handler returns true if it consumed the event. */
typedef bool (*state_h)(struct hsm *, enum event);

static bool h_operational(struct hsm *h, enum event e)
{
    if (e == E_ESTOP) { act_all_off(h); hsm_transition(h, S_FAULT); return true; }
    return false;                        /* anything else: not ours */
}

static bool h_heating(struct hsm *h, enum event e)
{
    if (e == E_AT_TEMP) { hsm_transition(h, S_PUMPING); return true; }
    return false;                        /* E_ESTOP falls through to the parent */
}

static const state_h k_handler[S_COUNT] = {
    [S_OPERATIONAL] = h_operational,
    [S_IDLE]        = h_idle,
    [S_HEATING]     = h_heating,
    [S_PUMPING]     = h_pumping,
    [S_FAULT]       = h_fault,
};

void hsm_dispatch(struct hsm *h, enum event e)
{
    if (h == NULL || h->st >= S_COUNT) return;

    for (enum state s = h->st; s < S_COUNT; s = k_parent[s]) {
        if (k_handler[s] != NULL && k_handler[s](h, e)) return;   /* consumed */
    }
    /* Reached the top unhandled — log it; a silent drop hides real bugs. */
    h->unhandled++;
}
```

**The problem hierarchy solves: shared behaviour duplicated across every state.** An emergency stop must work from Idle, Heating and Pumping. In a flat table that is the same `E_ESTOP` cell copied into three rows — and when a fourth state is added, someone forgets it, and the E-stop does nothing from that state. That is a safety defect produced by a copy-paste omission.

Putting `E_ESTOP` on the **parent** state means every child inherits it. Adding a fifth substate inherits it automatically. **The behaviour cannot be forgotten because it was never written per-state.**

The mechanism is one loop: try the current state's handler, and on `false` walk up the parent chain. That "unhandled events propagate upward" rule is the whole of hierarchy.

Three things worth adding:

- **Entry and exit actions must fire in the right order** on a transition that crosses levels: exit actions from the source up to the common ancestor, then entry actions down to the target. Getting this right is most of the complexity in a real HSM framework, and it is why frameworks like QP/Miro Samek's exist rather than everyone rolling their own.
- **Count the unhandled events.** A silent drop at the top means a missing transition looks like nothing happening — the hardest FSM bug to see. The counter turns it into a number you can watch.
- **Do not reach for hierarchy too early.** It is the right answer when several states genuinely share behaviour; on a six-state machine with no shared handling it is cost without benefit. Saying when *not* to use it is a stronger answer than describing it.
</details>

---

### 6. OTA update FSM — power-fail safe `STA`

```c
void ota_tick(struct ota *o);
```

<details><summary>Solution</summary>

```c
enum ota_state {
    O_IDLE, O_ERASING, O_DOWNLOADING, O_VERIFYING, O_COMMITTING, O_DONE, O_ABORT
};

/* Persisted in flash. The ORDER of writes here is the whole design. */
struct ota_header {
    uint32_t magic;             /* written FIRST  — "a slot exists" */
    uint32_t length;
    uint32_t crc32;
    uint32_t version;
    uint32_t valid_marker;      /* written LAST   — the commit point */
};

void ota_tick(struct ota *o)
{
    switch (o->st) {
    case O_IDLE:
        if (!o->requested) break;
        o->st = O_ERASING;
        break;

    case O_ERASING:
        /* Erase the INACTIVE bank only. The running image is never touched. */
        if (flash_erase_sector(o->next_sector) != 0) { o->st = O_ABORT; break; }
        if (++o->next_sector > o->last_sector) {
            o->offset = 0u;
            o->st = O_DOWNLOADING;
        }
        break;

    case O_DOWNLOADING:
        if (o->offset >= o->length) { o->st = O_VERIFYING; break; }
        if (!chunk_available()) break;                       /* non-blocking */
        if (o->offset + CHUNK > o->capacity) { o->st = O_ABORT; break; }  /* bounds */
        if (flash_write_words(o->bank_base + o->offset, chunk(), CHUNK/4u) != 0) {
            o->st = O_ABORT; break;
        }
        o->offset += CHUNK;
        wdt_task_alive(TASK_OTA);                            /* erase/write is slow */
        break;

    case O_VERIFYING:
        /* Read back from FLASH, not from the buffer we just sent. */
        if (crc32_flash(o->bank_base, o->length) != o->expected_crc) {
            o->st = O_ABORT; break;
        }
        o->st = O_COMMITTING;
        break;

    case O_COMMITTING:
        /* ONE word. This single write is the atomic commit. */
        flash_write_words(o->hdr_addr + offsetof(struct ota_header, valid_marker),
                          &k_valid, 1u);
        o->st = O_DONE;
        break;

    case O_ABORT:
        /* Leave the marker unwritten: the bootloader will ignore this bank. */
        o->st = O_IDLE;
        break;

    default:
        o->st = O_IDLE;
        break;
    }
}
```

**The requirement that makes this a staff-level problem: power can fail between any two instructions, and the device must still boot.** Every state above is a point at which losing power is survivable, and that property is designed, not hoped for.

**The single-word validity marker written last is the mechanism.** Flash programs a 32-bit word atomically, so the marker is either fully written or not written at all — never half. Everything before it can be interrupted freely, because the bootloader treats an image without the marker as absent and boots the existing one. **The marker is the commit point**, which is why it must be one word and why it must be last.

Four supporting rules:

- **Write to the inactive bank only.** The running image is never modified, so there is no window in which no bootable image exists. That is the whole argument for A/B (dual-bank) layout, and it is the answer to "what if the update fails halfway".
- **Verify by reading back from flash**, not by checksumming the buffer you transmitted. That catches a failed write, a bad sector, and a bit that did not program — the failures that matter. Checking the source proves only that you computed a CRC.
- **Kick the watchdog through the slow states.** A sector erase can take hundreds of milliseconds with the CPU stalled (Part 09 entry 14), so a naive watchdog fires mid-update — and a watchdog reset during a flash write is exactly the event this design exists to survive, so do not create it yourself.
- **Bound the offset against capacity on every chunk.** A length field from the network indexing flash writes is the TLV bug (Part 10 entry 7) with permanent consequences.

The follow-up is always rollback: keep the previous image's marker intact, and have the bootloader fall back if the new image fails to set a "booted successfully" flag within N seconds. That turns a bad-but-valid image — one that passes CRC and then crashes on boot — from a brick into a reboot. CRC proves integrity, not correctness, and the distinction is the point.
</details>

---

## Part 11 — retention table

| Pattern | The point |
|---|---|
| Transition table | the table **is** the spec; gaps are visible, `const`, reviewable |
| Bounds-check state and event | an out-of-range index calls an arbitrary function pointer |
| Entry/exit actions on the state | not duplicated across every transition into it |
| Queue events, dispatch from a task | a transition must not be re-entered mid-action |
| Hysteresis | the current output is an input — deadband > noise amplitude |
| Minimum off-time | deadband bounds *chatter*, not *cycle rate* |
| N-sample debounce | shift register; poll at a fixed rate, don't interrupt on a bouncing contact |
| Backoff | bounded attempts, exponential with a cap, **and jitter** |
| Jitter | without it, a thousand devices retry in lockstep forever |
| Hierarchy | shared behaviour on the parent, so it cannot be forgotten in a new child |
| Count unhandled events | a silent drop makes a missing transition invisible |
| OTA commit | one atomic word, written **last**; verify by reading back from flash |

---

# Part 03 — Memory and String Functions

14 entries from 45. The bank asks for `strlen`, `strnlen`, `strcpy`, `strncpy`, `strlcpy`, `strcat`, `strncat`, `strcmp`, `strncmp`, `strchr` and `memchr` as eleven problems; they are four ideas. What is kept is the set where something can actually go wrong.

> [!IMPORTANT]
> **A C string is a convention, not a type.** It is a `char` array whose end is marked by a `'\0'` byte. Every function here stops at that byte, so an array of *n* characters needs *n+1* bytes — and forgetting the `+1` is the most common buffer overflow in the language.
>
> **The second rule: an array decays to a pointer the moment it is passed.** `sizeof` inside the callee gives the size of a pointer, not the array. So the length always travels as a second parameter.

---

### 1. `memcpy` from scratch `BEG`

```c
void *my_memcpy(void *dst, const void *src, size_t n);
```

<details><summary>Solution</summary>

```c
void *my_memcpy(void *dst, const void *src, size_t n)
{
    unsigned char       *d = dst;
    const unsigned char *s = src;

    while (n-- > 0u) {
        *d++ = *s++;
    }
    return dst;                    /* returns dst, for chaining */
}
```

`unsigned char` is the correct working type: it is the only type guaranteed to have no padding bits and no trap representations, so it can alias any object legally. Using `char` risks signedness surprises; using `int *` breaks alignment and strict aliasing.

**The contract to state, because it is what the next entry is about: `memcpy` requires the regions not to overlap.** Overlapping is undefined behaviour, not "probably fine" — the standard permits an implementation to copy in any order, in blocks, or backwards. `memmove` is the function for overlap.

`n-- > 0u` rather than `n--` alone: with `size_t`, testing `n--` as a truthy value works, but the explicit comparison is what stops someone "simplifying" it to `--n >= 0`, which never terminates on an unsigned type.

Returning `dst` matches the standard so the function is a drop-in. And note the real `memcpy` is usually a compiler builtin — for small fixed sizes GCC replaces the call with inline loads and stores, which is why a hand-rolled version can be *slower* than the one you were asked to replace.
</details>

---

### 2. Word-aligned fast `memcpy` `SEN`

```c
void *fast_memcpy(void *dst, const void *src, size_t n);
```

<details><summary>Solution</summary>

```c
void *fast_memcpy(void *dst, const void *src, size_t n)
{
    unsigned char       *d = dst;
    const unsigned char *s = src;

    /* Only the word loop is legal when BOTH pointers share the same phase. */
    if ((((uintptr_t)d | (uintptr_t)s) & 3u) == 0u) {

        uint32_t       *dw = (uint32_t *)d;
        const uint32_t *sw = (const uint32_t *)s;

        while (n >= 16u) {                    /* unrolled: 4 words per iteration */
            dw[0] = sw[0]; dw[1] = sw[1];
            dw[2] = sw[2]; dw[3] = sw[3];
            dw += 4; sw += 4; n -= 16u;
        }
        while (n >= 4u) {
            *dw++ = *sw++;
            n -= 4u;
        }
        d = (unsigned char *)dw;
        s = (const unsigned char *)sw;
    }

    while (n-- > 0u) {                        /* byte tail, and the unaligned case */
        *d++ = *s++;
    }
    return dst;
}
```

**`(((uintptr_t)d | (uintptr_t)s) & 3) == 0` is the check that makes this correct**, and ORing the two pointers before masking tests both in one operation — a neat idiom worth knowing. Both must be 4-byte aligned, because a 32-bit load or store on an unaligned address **faults on Cortex-M0/M0+** and is trap-configurable on M3/M4.

**Same-phase misalignment is the case people get wrong.** If `src` is at offset 1 and `dst` at offset 1, they share a phase and could be handled by copying 3 bytes then switching to words. If `src` is at offset 1 and `dst` at offset 0, no amount of skipping aligns both, and the word loop is impossible without shift-and-merge. Falling back to bytes for every unaligned case is the honest, correct answer; mentioning the shift-and-merge refinement shows you know why the fast path is conditional.

The gain is real: 4× fewer memory transactions, plus the unrolling amortises the loop overhead. On Cortex-M4 that is roughly a 3–4× speedup for large aligned blocks.

Two things to add: a real library `memcpy` also aligns the destination first and may use `LDM`/`STM` to move eight words per instruction; and on Cortex-M7 with DMA available, a large copy is better handed to the DMA engine entirely, freeing the core — provided you handle the cache maintenance from Part 13 entry 7.
</details>

---

### 3. `memmove` — handling overlap `INT`

```c
void *my_memmove(void *dst, const void *src, size_t n);
```

<details><summary>Solution</summary>

```c
void *my_memmove(void *dst, const void *src, size_t n)
{
    unsigned char       *d = dst;
    const unsigned char *s = src;

    if (d == s || n == 0u) return dst;

    if (d < s) {
        while (n-- > 0u) *d++ = *s++;            /* forward: safe */
    } else {
        d += n;                                  /* backward: start at the end */
        s += n;
        while (n-- > 0u) *--d = *--s;
    }
    return dst;
}
```

**The direction of the copy is the entire question.** Consider `memmove(buf+1, buf, 5)` — shifting right by one. Copying forward reads `buf[0]`, writes `buf[1]`, then reads `buf[1]` — which is the byte just written. Every subsequent byte is the same value: the first byte smeared across the region. Copying **backward** reads each source byte before it can be overwritten.

The rule, worth stating as a rule rather than a case: **if the destination is below the source, copy forward; if above, copy backward.** The direction of overlap determines which end is safe to start from.

**Why `memcpy` cannot just do this:** it could, and some implementations effectively do — but the standard leaves overlap undefined precisely so `memcpy` can be as fast as the hardware allows without a direction test or an aliasing check. Relying on your platform's `memcpy` tolerating overlap is code that breaks on the next compiler.

Two honest caveats worth raising: comparing `d < s` when the two pointers are into **different objects** is technically unspecified in C, though it works on every flat-address machine — the standard-blessed alternative is to compare `uintptr_t` values. And the `d == s` early return is not just an optimisation; it makes the function total for a self-move.

Where this bites in firmware: consuming the front of a buffer by shifting the remainder down (`memmove(buf, buf+used, remaining)`) is an overlapping move, and it is what a parser does on every frame. Using `memcpy` there works until the day the compiler vectorises it.
</details>

---

### 4. `memset` and word broadcast `BEG`→`SEN`

```c
void *my_memset(void *dst, int c, size_t n);
```

<details><summary>Solution</summary>

```c
void *my_memset(void *dst, int c, size_t n)
{
    unsigned char *d  = dst;
    unsigned char  b  = (unsigned char)c;         /* only the low byte is used */

    /* Byte-fill until aligned. */
    while ((n > 0u) && (((uintptr_t)d & 3u) != 0u)) {
        *d++ = b;
        n--;
    }

    /* Broadcast the byte into all four lanes, then fill words. */
    uint32_t w = 0x01010101UL * (uint32_t)b;
    uint32_t *dw = (uint32_t *)d;
    while (n >= 4u) {
        *dw++ = w;
        n -= 4u;
    }

    d = (unsigned char *)dw;
    while (n-- > 0u) *d++ = b;                    /* tail */
    return dst;
}
```

**`c` is an `int` in the signature but only its low byte is used** — a deliberate quirk of the standard, and the reason `memset(buf, 0xFFFFFF00, n)` fills with zeros rather than doing anything interesting. Converting to `unsigned char` first makes that explicit.

**`0x01010101 * b` is the broadcast trick.** Multiplying by that constant places a copy of the byte in each of the four lanes, so one 32-bit store does four bytes of work. It generalises: `0x0101010101010101` for 64-bit.

Note this version aligns the **destination** before the word loop rather than requiring alignment up front — `memset` has only one pointer, so alignment is always achievable, unlike `memcpy` in entry 2.

The follow-up worth pre-empting: **`memset` on a struct does not portably produce a "zeroed" struct** in the sense of null pointers and zero floats. It sets every byte to zero, which on every real platform gives null pointers and `0.0`, but the standard does not guarantee that a null pointer is all-zero bits. It is also the wrong tool for clearing padding-sensitive structs that go on the wire — see Part 13 entry 4.
</details>

---

### 5. `memcmp` and constant-time comparison `BEG`→`SEN`

```c
int  my_memcmp(const void *a, const void *b, size_t n);
bool ct_equal(const uint8_t *a, const uint8_t *b, size_t n);
```

<details><summary>Solution</summary>

```c
int my_memcmp(const void *a, const void *b, size_t n)
{
    const unsigned char *p = a;
    const unsigned char *q = b;

    while (n-- > 0u) {
        if (*p != *q) {
            return (int)*p - (int)*q;      /* unsigned char, so the sign is right */
        }
        p++; q++;
    }
    return 0;
}

/* For secrets: MACs, tokens, passwords. Always touches all n bytes. */
bool ct_equal(const uint8_t *a, const uint8_t *b, size_t n)
{
    uint8_t diff = 0u;

    for (size_t i = 0u; i < n; i++) {
        diff |= (uint8_t)(a[i] ^ b[i]);    /* accumulate; never branch or return */
    }
    return diff == 0u;
}
```

**`my_memcmp` must compare as `unsigned char`.** Plain `char` is signed on most targets, so byte `0x80` would compare as −128 and order below `0x01` — giving the wrong sign for any comparison involving a high byte. That is the same cast requirement as `strcmp` in entry 9.

**`ct_equal` is the entry that earns its place.** `memcmp` returns as soon as it finds a difference, so **how long it takes reveals how many leading bytes matched.** An attacker comparing a guessed MAC or token measures the response time, learns the first byte, then the second — reducing a 2¹²⁸ brute force to 16 × 256 guesses. This is a practical, exploited attack, not a theoretical one.

The fix has two required properties: **no early return** and **no branch on secret data**. OR-accumulating the XOR differences touches every byte in the same time regardless of content, and only the final comparison depends on the result.

Details that come up as follow-ups: the length itself still leaks, so compare fixed-size digests rather than variable-length strings; a sufficiently aggressive compiler could in principle short-circuit the loop, which is why production code uses `volatile` accumulators or a vetted library (`mbedtls_ct_memcmp`, `sodium_memcmp`) rather than trusting this to survive `-O3`. And the rule of thumb worth stating: **any comparison against a secret must be constant time** — for everything else, `memcmp` is correct and faster.
</details>

---

### 6. `strlen` and `strnlen` `BEG`

```c
size_t my_strlen(const char *s);
size_t my_strnlen(const char *s, size_t maxlen);
```

<details><summary>Solution</summary>

```c
size_t my_strlen(const char *s)
{
    const char *p = s;
    while (*p != '\0') p++;
    return (size_t)(p - s);              /* pointer difference == element count */
}

/* Bounded: never reads past maxlen, even with no terminator. */
size_t my_strnlen(const char *s, size_t maxlen)
{
    size_t i = 0u;
    while (i < maxlen && s[i] != '\0') i++;
    return i;
}
```

Pointer subtraction gives the count directly, which is cleaner than maintaining a counter.

**`strnlen` is the one that belongs in firmware.** Any string that arrived from outside the program — a wire buffer, a flash record, an EEPROM field — may have no terminator at all, and `strlen` on it walks until it happens to find a zero byte, potentially far outside the buffer. It may fault, or worse it may return a plausible large number that then drives a `memcpy`. `strnlen` bounds the damage.

**The `+1` rule to restate:** `strlen` does not count the terminator, but the buffer must hold it. A field of at most 31 characters needs `char name[32]`.

The word-at-a-time optimisation is worth mentioning if asked how a real `strlen` is fast: load a whole word and test whether any byte is zero using `(w - 0x01010101) & ~w & 0x80808080`, which is nonzero exactly when some byte is zero. Same family as the zero-nibble SWAR detector in Part 01. It requires alignment, and it may read a few bytes past the terminator — legal within a page, which is why library implementations align first.
</details>

---

### 7. `strcpy`, `strncpy`, and why `strlcpy` exists `INT`

```c
size_t my_strlcpy(char *dst, const char *src, size_t dsize);
```

<details><summary>Solution</summary>

```c
/* What the interviewer asks for. Unsafe by construction. */
char *my_strcpy(char *dst, const char *src)
{
    char *d = dst;
    while ((*d++ = *src++) != '\0') { }     /* copies the terminator too */
    return dst;
}

/* Returns the length it TRIED to create — so truncation is detectable. */
size_t my_strlcpy(char *dst, const char *src, size_t dsize)
{
    size_t slen = 0u;
    while (src[slen] != '\0') slen++;

    if (dsize != 0u) {
        size_t copy = (slen < (dsize - 1u)) ? slen : (dsize - 1u);
        for (size_t i = 0u; i < copy; i++) dst[i] = src[i];
        dst[copy] = '\0';                   /* ALWAYS terminated */
    }
    return slen;                            /* caller: if (ret >= dsize) truncated */
}
```

**`strncpy` is not the safe version of `strcpy`, and believing it is is the mistake this problem tests.** It has two behaviours nobody wants:

- **It does not terminate if the source is longer than `n`.** `strncpy(buf, src, sizeof buf)` leaves `buf` unterminated when `src` fills it, and the next `strlen` runs off the end. The overflow moves from the copy to the *next* operation, which is worse because it is further from the cause.
- **It pads the rest of the destination with zeros.** Copying a 3-character name into a 256-byte buffer writes 253 pointless zeros. It was designed for fixed-width record fields, not for strings.

**`strlcpy` fixes both and adds the thing that matters most: a detectable truncation.** It returns the length of the source, so `if (ret >= dsize)` tells the caller their data was cut. `strncpy` returns `dst`, which conveys nothing, so truncation is silent — and silent truncation of a URL, a filename or a certificate is a bug that surfaces far away.

**What to actually ship**, and the answer to give after writing the manual version:

```c
if (snprintf(dst, sizeof dst, "%s", src) >= (int)sizeof dst) { /* truncated */ }
```

`snprintf` is standard C (`strlcpy` is BSD and not in the C standard), always terminates, and reports the required length. On a part where `snprintf` is too heavy, `strlcpy` is the right fallback — and knowing why you would choose one over the other is the complete answer.
</details>

---

### 8. Bounded append `INT`

```c
size_t str_append(char *dst, size_t dsize, const char *src);
```

<details><summary>Solution</summary>

```c
size_t str_append(char *dst, size_t dsize, const char *src)
{
    size_t dlen = my_strnlen(dst, dsize);       /* bounded, in case dst is malformed */
    if (dlen >= dsize) return dlen;             /* already full or unterminated */

    size_t remaining = dsize - dlen - 1u;       /* -1 reserves the terminator */
    size_t i = 0u;
    while (i < remaining && src[i] != '\0') {
        dst[dlen + i] = src[i];
        i++;
    }
    dst[dlen + i] = '\0';

    return dlen + i + my_strlen(&src[i]);       /* length it WANTED to be */
}
```

**Concatenation is O(length of dst)** because it must walk to the terminator before it can start. That has a consequence worth volunteering: **building a string by appending in a loop is quietly O(n²)** — each append rescans everything written so far. Ten appends to a growing 1 KB buffer is 10 KB of scanning to write a few hundred bytes. The fix is to track the current length and pass it in, or to use `snprintf` with a running offset:

```c
size_t off = 0u;
off += (size_t)snprintf(&buf[off], sizeof buf - off, "a=%d ", a);
off += (size_t)snprintf(&buf[off], sizeof buf - off, "b=%d ", b);
```

That is the pattern to reach for, and note it needs the same `off >= sizeof buf` truncation check each time — `snprintf` returns the length it *wanted*, which can exceed the buffer.

Two details in the implementation: `my_strnlen(dst, dsize)` rather than `strlen`, so a destination that arrived unterminated cannot cause a runaway scan; and `dsize - dlen - 1` reserving the terminator, which is the off-by-one this function exists to get right.
</details>

---

### 9. `strcmp` and `strncmp` `BEG`

```c
int my_strcmp(const char *a, const char *b);
int my_strncmp(const char *a, const char *b, size_t n);
```

<details><summary>Solution</summary>

```c
int my_strcmp(const char *a, const char *b)
{
    while (*a != '\0' && *a == *b) { a++; b++; }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

int my_strncmp(const char *a, const char *b, size_t n)
{
    while (n > 0u && *a != '\0' && *a == *b) { a++; b++; n--; }
    if (n == 0u) return 0;                        /* first n chars matched */
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}
```

**The `unsigned char` cast is the point.** Plain `char` is signed on ARM and x86 by default, so a byte of `0x80` is −128 and would order *below* `0x01`. Any comparison involving non-ASCII bytes then gives the wrong sign — a sorted table of UTF-8 strings comes out in the wrong order, and a binary search over it silently fails to find entries. The standard requires the comparison be done as `unsigned char`.

**The second point is the return convention**, which the general-C book lists among its ten classic traps: `strcmp` returns **0 for equal**, so `if (strcmp(a, b))` means *different*. Reading it as "if a equals b" is a real and common inversion. Write `if (strcmp(a, b) == 0)` and the intent is unmistakable.

The magnitude of the return value is unspecified beyond its sign — do not depend on it being the byte difference, even though that is what these implementations return.

`strncmp` is what you want for prefix matching, and it is what the AT command dispatcher in Part 10 entry 6 uses. Note the `n == 0` check must come *before* dereferencing, or a fully-matching bounded comparison reads one byte past its limit.
</details>

---

### 10. `strstr` — substring search `INT`

```c
char *my_strstr(const char *hay, const char *needle);
```

<details><summary>Solution</summary>

```c
char *my_strstr(const char *hay, const char *needle)
{
    if (*needle == '\0') return (char *)hay;      /* empty needle matches at 0 */

    for (; *hay != '\0'; hay++) {
        const char *h = hay;
        const char *n = needle;

        while (*n != '\0' && *h == *n) { h++; n++; }

        if (*n == '\0') return (char *)hay;       /* consumed the whole needle */
    }
    return NULL;
}
```

Naive O(n·m): try to match at every position. For the string lengths firmware deals with — finding `"OK\r\n"` in a 256-byte modem response — that is the right algorithm, and saying so is better than reaching for something clever.

**The empty-needle case must return `hay`, not `NULL`**, per the standard. It looks like a triviality and it is the sort of edge an interviewer probes because it shows whether you thought about the boundaries or only the main path.

**When to escalate, and why usually not:** Boyer-Moore or KMP reduce the worst case, but both need a preprocessing table proportional to the pattern or alphabet — on a part with 20 KB of RAM, a 256-entry skip table to search a 200-byte buffer is a poor trade. The worst case for the naive version needs highly repetitive input (`"aaaaaaab"` in `"aaaaaaaaaaa"`), which real protocol data does not resemble.

The firmware caveat worth adding: `strstr` requires **both** arguments to be NUL-terminated, so it cannot search a binary buffer containing zero bytes. For that you need `memmem` (a GNU extension) or a hand-rolled bounded search — and reaching for `strstr` on a wire buffer is how you get a scan that runs past the data.
</details>

---

### 11. `strtok` and why it is dangerous `SEN`

```c
char *my_strtok_r(char *str, const char *delim, char **saveptr);
```

<details><summary>Solution</summary>

```c
static bool is_delim(char c, const char *delim)
{
    for (; *delim != '\0'; delim++) {
        if (c == *delim) return true;
    }
    return false;
}

char *my_strtok_r(char *str, const char *delim, char **saveptr)
{
    if (saveptr == NULL) return NULL;

    char *p = (str != NULL) ? str : *saveptr;
    if (p == NULL) return NULL;

    while (*p != '\0' && is_delim(*p, delim)) p++;    /* skip leading delimiters */
    if (*p == '\0') { *saveptr = NULL; return NULL; }

    char *tok = p;
    while (*p != '\0' && !is_delim(*p, delim)) p++;

    if (*p != '\0') { *p = '\0'; *saveptr = p + 1; }  /* terminate, save position */
    else            { *saveptr = NULL; }

    return tok;
}
```

**Three separate defects in the standard `strtok`, and all three matter in firmware:**

1. **It holds static state**, so it is **not reentrant.** Two tasks tokenising two strings interleave and corrupt each other's iteration; a call from an ISR destroys a task's in-progress parse. `strtok_r` takes the state as a `char **` parameter, which is why it exists.
2. **It modifies its input**, writing `'\0'` over each delimiter. Pass it a string literal and it is undefined behaviour (the literal is read-only); pass it a buffer you still need intact and it is gone.
3. **It collapses runs of delimiters**, so consecutive delimiters are treated as one — which means **empty fields disappear.** That is the NMEA bug from Part 10 entry 5: `$GPGGA,,,,,,0` loses its empty fields and every subsequent field shifts left, producing plausible wrong numbers rather than an error.

Defect 3 is the one that decides whether you can use it at all. For CSV, NMEA, or any format where position carries meaning, **`strtok` is the wrong tool regardless of reentrancy** — you need a manual comma split that preserves empty fields.

So the honest answer has three layers: write `strtok_r`, name all three defects, and say that for positional formats you would hand-roll the split anyway. That progression is what a senior answer looks like.
</details>

---

### 12. `atoi` versus `strtol` `BEG`→`INT`

```c
int  my_atoi(const char *s);
bool parse_i32(const char *s, int32_t *out);
```

<details><summary>Solution</summary>

```c
/* What is asked for. Note what it CANNOT do. */
int my_atoi(const char *s)
{
    int sign = 1, v = 0;

    while (*s == ' ' || *s == '\t') s++;
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') { s++; }

    while (*s >= '0' && *s <= '9') {
        v = (v * 10) + (*s - '0');            /* OVERFLOW IS UB, undetectable */
        s++;
    }
    return sign * v;
}

/* What you ship: every failure mode reported. */
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

bool parse_i32(const char *s, int32_t *out)
{
    if (s == NULL || out == NULL || *s == '\0') return false;

    char *end;
    errno = 0;
    long v = strtol(s, &end, 10);

    if (end == s)             return false;        /* no digits at all */
    if (*end != '\0')         return false;        /* trailing junk: "12abc" */
    if (errno == ERANGE)      return false;        /* out of long range */
    if (v > INT32_MAX || v < INT32_MIN) return false;

    *out = (int32_t)v;
    return true;
}
```

**`atoi` cannot report failure, and that is the whole question.** It returns 0 for `"abc"`, 0 for `""`, and 0 for `"0"` — three cases you cannot distinguish. On overflow it is **undefined behaviour**, so a config field of `"99999999999"` can produce anything the optimiser likes.

**`strtol` reports every failure**, through three separate channels that all have to be checked:

| Check | Catches |
|---|---|
| `end == s` | no digits were converted |
| `*end != '\0'` | trailing garbage — `"12abc"`, `"1.5"` |
| `errno == ERANGE` | value outside `long` |
| explicit range test | value outside your narrower target type |

**`errno = 0` before the call is mandatory**, because `strtol` only *sets* `errno` on error and never clears it — a stale `ERANGE` from an earlier unrelated call otherwise rejects a valid number.

The firmware relevance is direct: these functions parse a config file, a command line, a Modbus register value, an AT response. Accepting `"12abc"` as 12 is how a typo in a config becomes a device that runs at the wrong baud rate and nobody knows why. `atoi` is acceptable in an exam; in shipped code it is a defect.
</details>

---

### 13. `itoa` — integer to string `BEG`

```c
size_t u32_to_str(uint32_t v, char *buf, size_t cap);
size_t i32_to_str(int32_t v, char *buf, size_t cap);
size_t u32_to_hex(uint32_t v, char *buf, size_t cap);
```

<details><summary>Solution</summary>

```c
size_t u32_to_str(uint32_t v, char *buf, size_t cap)
{
    char tmp[10];                            /* 4294967295 == 10 digits */
    size_t n = 0u;

    do {
        tmp[n++] = (char)('0' + (v % 10u));  /* digits come out BACKWARDS */
        v /= 10u;
    } while (v != 0u);

    if (cap < n + 1u) return 0u;

    for (size_t i = 0u; i < n; i++) {
        buf[i] = tmp[n - 1u - i];            /* reverse into place */
    }
    buf[n] = '\0';
    return n;
}

size_t i32_to_str(int32_t v, char *buf, size_t cap)
{
    /* Negate in UNSIGNED space: -INT32_MIN is not representable as int32_t. */
    uint32_t mag = (v < 0) ? (uint32_t)0 - (uint32_t)v : (uint32_t)v;

    if (v < 0) {
        if (cap < 2u) return 0u;
        buf[0] = '-';
        size_t n = u32_to_str(mag, &buf[1], cap - 1u);
        return (n == 0u) ? 0u : n + 1u;
    }
    return u32_to_str(mag, buf, cap);
}

size_t u32_to_hex(uint32_t v, char *buf, size_t cap)
{
    static const char k_hex[] = "0123456789ABCDEF";
    if (cap < 9u) return 0u;

    for (size_t i = 0u; i < 8u; i++) {
        buf[i] = k_hex[(v >> ((7u - i) * 4u)) & 0x0Fu];    /* MSB first, fixed width */
    }
    buf[8] = '\0';
    return 8u;
}
```

**Digits are generated least-significant first, so a reversal is unavoidable** — buffer into a temporary and copy back, or generate into the end of the destination and return a pointer into it. That reversal *is* the exercise.

**`-INT32_MIN` is the trap, and it is the reason `i32_to_str` is separate.** `INT32_MIN` is −2147483648 and the maximum positive `int32_t` is 2147483647, so negating it overflows — undefined behaviour. Converting to `uint32_t` **before** negating keeps the arithmetic in a type where wraparound is defined and the magnitude is representable. Any conversion routine that does `if (v < 0) v = -v;` is broken for exactly one input, and that input appears in real data as a sensor error sentinel.

The hex version uses a **fixed width with a lookup table**, which is what you want for a register dump or a log — variable-width hex makes columns unreadable. And a hex nibble is a shift and a mask, no division, which matters on a Cortex-M0.

Worth stating: this exists because `sprintf("%d")` pulls in the whole formatting engine — often 2–8 KB of flash and a large stack frame. On a part with 32 KB of flash, replacing `printf` with three functions like these is a standard and significant win, which is exactly what the Optimization section asks about.
</details>

---

### 14. Secure zeroing `SEN`

```c
void secure_zero(void *p, size_t n);
```

<details><summary>Solution</summary>

```c
/* BROKEN: the compiler is allowed to delete this memset entirely. */
void clear_key_broken(void)
{
    uint8_t key[32];
    load_key(key);
    use_key(key);
    memset(key, 0, sizeof key);        /* dead store: key is never read again */
}

/* Correct: a volatile pointer makes every write observable. */
void secure_zero(void *p, size_t n)
{
    volatile unsigned char *q = p;
    while (n-- > 0u) {
        *q++ = 0u;
    }
}
```

**Dead-store elimination is the bug, and it is not a compiler defect.** The compiler can prove `key` is never read after the `memset` and that its lifetime ends at the closing brace, so the writes have no observable effect and removing them is a valid optimisation. The key stays in stack memory — and that stack is reused by the next function, written to a core dump, or read by an attacker who can trigger a fault dump.

This is a real, documented CVE class, and it only manifests with optimisation enabled, which means it never appears in a debug build.

**`volatile` is what fixes it**, because `volatile` accesses are by definition observable and may not be optimised away. This is one of the few places where `volatile` is the *right* tool for something other than hardware or concurrency — and it is a good answer to "give me a use of `volatile` that isn't a register or an ISR flag".

Platform alternatives, worth naming: `explicit_bzero` (BSD, glibc), `SecureZeroMemory` (Windows), `memset_s` (C11 Annex K, optional and rarely implemented), or a compiler barrier such as `__asm__ __volatile__("" ::: "memory")` after the `memset`.

**The rest of the answer is that zeroing is necessary but not sufficient**, and this is where it becomes a firmware-security question rather than a C question. The secret may also exist in: a register (the compiler chose one), a spilled copy elsewhere on the stack, a `printf` buffer, a DMA buffer, a cache line, or flash if it was ever persisted. Minimising the window and the number of copies matters as much as clearing the one you know about — which is why secure code keeps keys in a dedicated region, or in hardware that never exposes them, rather than relying on cleanup.
</details>

---

## Part 03 — retention table

| Function | The detail that decides it |
|---|---|
| `memcpy` | regions must not overlap — UB, not "probably fine" |
| Fast `memcpy` | `((uintptr_t)d \| (uintptr_t)s) & 3` — **both** must be aligned |
| `memmove` | `d < s` → forward, else **backward**; direction is the whole answer |
| `memset` | `0x01010101 * b` broadcasts a byte into four lanes |
| `memcmp` | compare as `unsigned char` or high bytes order wrongly |
| Secret compare | **no early return** — OR-accumulate; `memcmp` leaks match length |
| `strlen` | needs `n+1` bytes; use `strnlen` on anything from outside |
| `strncpy` | **does not terminate** on truncation, and pads to `n`. Not the safe version |
| `strlcpy` / `snprintf` | always terminate, and **report** truncation |
| Append in a loop | O(n²) — track the offset instead |
| `strcmp` | returns **0 for equal**; `if (strcmp(a,b))` means *different* |
| `strtok` | static state, mutates input, **collapses empty fields** |
| `atoi` | cannot report failure; overflow is UB. Use `strtol` + all four checks |
| `itoa` | digits emerge reversed; **negate `INT32_MIN` in unsigned space** |
| Zeroing secrets | plain `memset` is deleted as a dead store — use `volatile` |

---

# Part 04 — Allocators

8 entries from 35. TLSF, slab and log-structured allocators are gone: they are interesting and they are not what firmware interviews ask. What remains is the progression from "why not `malloc`" through the three allocators that actually appear in embedded systems.

> [!IMPORTANT]
> **Why `malloc` is banned in most firmware, stated properly.** Not because it is slow — because it has **unbounded worst-case latency** and it **fragments**. A long-running system that allocates and frees blocks of varying sizes eventually reaches a state where 8 KB is free but no contiguous 2 KB block exists, and the allocation fails at hour 900 with no operator to restart it. Neither property can be tested away: the failure depends on an allocation history you cannot reproduce.
>
> Every allocator below removes one or both of those properties by giving something up.

---

### 1. Fixed-size block pool `INT`

```c
void  pool_init(struct pool *p, void *mem, size_t block_size, size_t count);
void *pool_alloc(struct pool *p);
void  pool_free(struct pool *p, void *blk);
```

<details><summary>Solution</summary>

```c
struct pool {
    void  **free_head;              /* singly linked list threaded through free blocks */
    size_t  block_size;
    size_t  count;
    uint8_t *base;
    size_t  in_use, high_water;     /* diagnostics — see below */
};

void pool_init(struct pool *p, void *mem, size_t block_size, size_t count)
{
    if (p == NULL || mem == NULL || block_size < sizeof(void *) || count == 0u) return;

    p->base       = mem;
    p->block_size = block_size;
    p->count      = count;
    p->in_use     = 0u;
    p->high_water = 0u;

    /* Thread the free list through the blocks themselves: zero overhead. */
    p->free_head = NULL;
    for (size_t i = count; i > 0u; i--) {
        void **blk = (void **)&p->base[(i - 1u) * block_size];
        *blk = p->free_head;
        p->free_head = (void **)blk;
    }
}

void *pool_alloc(struct pool *p)
{
    if (p == NULL || p->free_head == NULL) return NULL;    /* exhausted */

    void **blk   = p->free_head;
    p->free_head = *blk;                                   /* pop */

    if (++p->in_use > p->high_water) p->high_water = p->in_use;
    return blk;
}

void pool_free(struct pool *p, void *blk)
{
    if (p == NULL || blk == NULL) return;

    /* Reject a pointer that did not come from this pool. */
    uint8_t *b = blk;
    if (b < p->base || b >= p->base + (p->count * p->block_size)) return;
    if (((size_t)(b - p->base) % p->block_size) != 0u) return;   /* not a block start */

    *(void **)blk = p->free_head;
    p->free_head  = blk;
    p->in_use--;
}
```

**This is the allocator firmware actually uses**, and it beats `malloc` on every axis that matters: **O(1) alloc, O(1) free, and zero fragmentation** — because every block is interchangeable, so a free block can always satisfy a request. The worst case is provable at compile time.

**The free list is threaded through the free blocks themselves**, costing no metadata: a block that is free has no user data, so its first word holds the next pointer. That requires `block_size >= sizeof(void *)`, which is what the init check enforces.

What you give up is flexibility — one size only. The standard response is several pools of different sizes, which reintroduces a smaller version of the fragmentation question (pool A is exhausted while pool B is empty) but keeps every operation bounded.

Three things that make this a complete answer:

- **The two validation checks in `pool_free`** — in range, and on a block boundary. A foreign or mid-block pointer would otherwise corrupt the free list into something that hands out overlapping blocks later, and the crash appears nowhere near the cause.
- **`high_water`** is what lets you size the pool from evidence rather than guesswork. Run the worst-case load, read the mark, add margin.
- **Not ISR-safe as written.** `pool_alloc` from an ISR races with `pool_alloc` from a task on `free_head`. See entry 8.
</details>

---

### 2. Arena / bump allocator `BEG`

```c
void *arena_alloc(struct arena *a, size_t n, size_t align);
void  arena_reset(struct arena *a);
```

<details><summary>Solution</summary>

```c
struct arena {
    uint8_t *base;
    size_t   size;
    size_t   offset;
    size_t   high_water;
};

void *arena_alloc(struct arena *a, size_t n, size_t align)
{
    if (a == NULL || n == 0u || align == 0u || (align & (align - 1u)) != 0u) {
        return NULL;                                  /* align must be a power of 2 */
    }

    uintptr_t cur     = (uintptr_t)a->base + a->offset;
    uintptr_t aligned = (cur + align - 1u) & ~(uintptr_t)(align - 1u);   /* Part 01 */
    size_t    pad     = (size_t)(aligned - cur);

    /* Overflow-safe capacity test: never compute offset + pad + n. */
    if (pad > (a->size - a->offset)) return NULL;
    if (n > (a->size - a->offset - pad)) return NULL;

    a->offset += pad + n;
    if (a->offset > a->high_water) a->high_water = a->offset;

    return (void *)aligned;
}

/* Frees EVERYTHING at once. There is no individual free. */
void arena_reset(struct arena *a)
{
    if (a != NULL) a->offset = 0u;
}
```

**The simplest allocator that exists: a pointer that only moves forward.** Allocation is an add and a bounds check; there is no free list, no metadata, no fragmentation, and no way to release one block.

**That restriction is the feature, not a limitation to apologise for.** It fits any workload with a natural lifetime boundary: parse a message using arena memory then reset, build a frame then reset, run one control cycle then reset. All the bookkeeping that makes `malloc` unpredictable disappears because nothing is ever individually released.

Two implementation points that get checked:

- **The capacity test must be written as subtraction.** `a->offset + pad + n > a->size` can overflow `size_t` for a large `n` and wrap to a small value, passing the check it exists to fail. Subtracting from the known-good total keeps the arithmetic in range — the same discipline as the TLV length check in Part 10 entry 7 and the signed overflow tests in Part 01 entry 20.
- **Alignment must be requested, not assumed.** Returning an arbitrary offset for a `uint32_t` gives an unaligned pointer that faults on Cortex-M0. The `align_up` expression here is Part 01 entry 16.

The variants worth naming: a **stack allocator** adds a marker so you can unwind to a saved point (a scoped free, which is strictly more useful and barely more code), and a **double-ended arena** allocates permanent data from one end and temporary from the other, so one region serves both lifetimes.
</details>

---

### 3. Free-list allocator — first fit `SEN`

```c
void *fl_alloc(struct fl *h, size_t n);
```

<details><summary>Solution</summary>

```c
struct blk {
    size_t      size;              /* payload bytes, excluding this header */
    bool        free;
    struct blk *next;              /* address-ordered list — required for entry 4 */
};

#define HDR sizeof(struct blk)
#define MIN_SPLIT (HDR + 16u)      /* don't split off a useless fragment */

void *fl_alloc(struct fl *h, size_t n)
{
    if (h == NULL || n == 0u) return NULL;

    n = (n + 3u) & ~(size_t)3u;                     /* round up to 4-byte alignment */

    for (struct blk *b = h->head; b != NULL; b = b->next) {
        if (!b->free || b->size < n) continue;

        /* Split only if the remainder is big enough to be useful. */
        if (b->size >= n + MIN_SPLIT) {
            struct blk *rest = (struct blk *)((uint8_t *)(b + 1) + n);
            rest->size = b->size - n - HDR;
            rest->free = true;
            rest->next = b->next;

            b->size = n;
            b->next = rest;
        }
        b->free = false;
        return (void *)(b + 1);                     /* payload follows the header */
    }
    return NULL;                                    /* no block large enough */
}
```

**This is `malloc`, and writing it is how you understand why it is avoided.** Two properties fall straight out of the code:

- **Allocation is O(n) in the number of blocks**, and *n* depends on allocation history. So worst-case latency is unbounded in any way you can analyse or test. That alone disqualifies it from a hard-real-time path.
- **Splitting creates fragmentation.** Every split leaves a smaller remainder, and over time the free space is distributed across many blocks too small to be useful. Total free memory stays high while the largest allocatable block shrinks — the failure mode described at the top of this part.

**`MIN_SPLIT` is the detail worth having.** Splitting off 4 bytes produces a fragment that can never satisfy a request but must still be walked on every future allocation — pure cost. Refusing to split below a threshold trades a little internal waste for a shorter list.

The fit strategies, and the honest summary:

| Strategy | Behaviour |
|---|---|
| **First fit** | fastest scan; fragments the front of the heap |
| **Best fit** | scans everything; leaves the smallest remainder, and so the most unusable fragments |
| **Next fit** | resumes where it stopped; spreads wear, worse locality |

Best fit sounds better and measurably is not — that counter-intuitive result is a good thing to know.

**The address-ordered list is not incidental**: coalescing in entry 4 requires knowing which blocks are physically adjacent, so insertion must maintain address order. A list ordered by size makes allocation faster and coalescing impossible.
</details>

---

### 4. Free with coalescing `SEN`

```c
void fl_free(struct fl *h, void *p);
```

<details><summary>Solution</summary>

```c
void fl_free(struct fl *h, void *p)
{
    if (h == NULL || p == NULL) return;

    struct blk *b = ((struct blk *)p) - 1;          /* header sits before the payload */

    if (b->free) return;                            /* DOUBLE FREE — see entry 6 */
    b->free = true;

    /* --- forward merge: is the next block free and physically adjacent? --- */
    if (b->next != NULL && b->next->free &&
        ((uint8_t *)(b + 1) + b->size == (uint8_t *)b->next)) {
        b->size += HDR + b->next->size;
        b->next  = b->next->next;
    }

    /* --- backward merge: find the predecessor (why a doubly linked list helps) --- */
    struct blk *prev = NULL;
    for (struct blk *c = h->head; c != NULL && c != b; c = c->next) prev = c;

    if (prev != NULL && prev->free &&
        ((uint8_t *)(prev + 1) + prev->size == (uint8_t *)b)) {
        prev->size += HDR + b->size;
        prev->next  = b->next;
    }
}
```

**Coalescing is what stops fragmentation from being unbounded**, and it is the reason `free` is harder than `alloc`. Without it, a repeated alloc/free cycle of alternating sizes ratchets the heap into ever-smaller pieces and never recovers. With it, adjacent free blocks merge back into one large block.

**Both directions are required.** Merging only forward leaves a hole whenever the block below is freed first, which happens roughly half the time — so half the fragmentation remains and the improvement looks like it works in testing.

**The adjacency test is the part to get right:** `(uint8_t *)(b + 1) + b->size == (uint8_t *)b->next`. Being next in the *list* is not the same as being next in *memory* — that is only true because the list is address-ordered, and if someone reorders it for speed, this check silently starts merging non-adjacent blocks and corrupting the heap. A comment saying so belongs in the code.

**The backward scan is O(n)**, which is why real allocators use a **doubly linked list**, or a **boundary tag**: a copy of the size stored at the *end* of each block, so the previous block's header can be found by arithmetic rather than by searching. Boundary tags are how dlmalloc does it, and naming the technique is the follow-up answer.

`if (b->free) return;` catches the simplest double free. Entry 6 does it properly.
</details>

---

### 5. Aligned allocation via a header `INT`

```c
void *aligned_alloc_hdr(size_t n, size_t align);
void  aligned_free_hdr(void *p);
```

<details><summary>Solution</summary>

```c
/* Over-allocate, align inside, and stash the original pointer just below. */
void *aligned_alloc_hdr(size_t n, size_t align)
{
    if (align == 0u || (align & (align - 1u)) != 0u) return NULL;   /* power of 2 */
    if (align < sizeof(void *)) align = sizeof(void *);

    /* Worst case: (align - 1) of padding, plus room for the saved pointer. */
    size_t total = n + align - 1u + sizeof(void *);
    if (total < n) return NULL;                        /* overflow */

    void *raw = malloc(total);
    if (raw == NULL) return NULL;

    uintptr_t after = (uintptr_t)raw + sizeof(void *);
    uintptr_t user  = (after + align - 1u) & ~(uintptr_t)(align - 1u);

    ((void **)user)[-1] = raw;                         /* save the real base */
    return (void *)user;
}

void aligned_free_hdr(void *p)
{
    if (p == NULL) return;
    free(((void **)p)[-1]);                            /* recover it and free that */
}
```

**The pointer you return is not the pointer you must free**, and that is the entire problem. `free` requires exactly what the allocator returned, so the aligned address inside the block has to carry a way back to the original. Storing it in the word immediately below the returned pointer is the standard trick — `((void **)user)[-1]`.

**Sizing the request is the arithmetic to get right:** `n + align - 1 + sizeof(void *)`. The `align - 1` covers the worst-case padding, and the pointer slot must be accounted for *before* aligning, which is why `after` starts past it. Getting this wrong by one gives a heap overflow that only triggers for particular alignments of the underlying `malloc`.

Two supporting points: `align < sizeof(void *)` is raised to the pointer size so the stashed pointer is itself aligned; and the overflow check matters because `n` may come from outside.

**Why this exists in firmware, concretely:** DMA buffers on Cortex-M7 must be 32-byte aligned for cache maintenance (Part 13 entry 7), and some DMA controllers require alignment to the transfer width. The alternatives are usually better when available — C11's `aligned_alloc`, or `__attribute__((aligned(32)))` on a static buffer, which costs nothing at runtime. This technique is for the case where the memory must come from a heap you do not control.
</details>

---

### 6. Detecting double free and corruption `SEN`

```c
void *guarded_alloc(struct pool *p);
int   guarded_free(struct pool *p, void *blk);
```

<details><summary>Solution</summary>

```c
#define MAGIC_USED 0xA110C8EDUL
#define MAGIC_FREE 0xDEADBEEFUL
#define CANARY     0x5A5A5A5AUL

struct ghdr {
    uint32_t magic;
    uint32_t size;
};

void *guarded_alloc(struct pool *p)
{
    struct ghdr *h = pool_alloc(p);
    if (h == NULL) return NULL;

    size_t payload = p->block_size - sizeof(struct ghdr) - sizeof(uint32_t);

    h->magic = MAGIC_USED;
    h->size  = (uint32_t)payload;

    uint8_t *user = (uint8_t *)(h + 1);
    uint32_t can  = CANARY;
    memcpy(&user[payload], &can, sizeof can);       /* trailing canary */

    return user;
}

int guarded_free(struct pool *p, void *blk)
{
    if (blk == NULL) return 0;

    struct ghdr *h = ((struct ghdr *)blk) - 1;

    if (h->magic == MAGIC_FREE) return -1;          /* ---- DOUBLE FREE ---- */
    if (h->magic != MAGIC_USED) return -2;          /* corrupt or foreign pointer */

    uint32_t can;
    memcpy(&can, (uint8_t *)blk + h->size, sizeof can);
    if (can != CANARY) return -3;                   /* ---- BUFFER OVERRUN ---- */

    h->magic = MAGIC_FREE;                          /* poison, so a repeat is caught */
    memset(blk, 0xDD, h->size);                     /* poison, so use-after-free shows */

    pool_free(p, h);
    return 0;
}
```

**Three distinct bugs, each caught by a different mechanism**, and being able to separate them is the answer:

| Bug | Detected by | Why it is otherwise invisible |
|---|---|---|
| **Double free** | `magic == MAGIC_FREE` | corrupts the free list; the crash is a later allocation |
| **Buffer overrun** | trailing canary changed | overwrites the *next* block's header |
| **Use after free** | payload poisoned with `0xDD` | reads plausible stale data and appears to work |

**Poisoning on free is the highest-value part.** Setting the payload to `0xDD` turns a use-after-free from "reads the old data, works by accident" into an immediately visible wrong value — and `0xDDDDDDDD` as a pointer faults on dereference, so it fails loudly at the point of misuse. Likewise flipping the magic to `MAGIC_FREE` is what makes the *second* free detectable rather than merely wrong.

**Choose magic values that are implausible as data and recognisable in a memory dump.** `0xDEADBEEF` and `0xA5A5A5A5` exist because you can spot them by eye at 3 a.m. A magic of `0` or `1` catches nothing, because those appear everywhere.

The costs, which should be stated rather than hidden: 12 bytes of overhead per block, and time on every alloc and free. So this is a **debug-build feature** — `#ifdef` the guards and keep the plain pool in production, or keep only the magic check, which is one comparison and catches the worst of the three.

Note the `memcpy` for the canary rather than a direct `uint32_t *` write: the payload end may not be 4-byte aligned, and an unaligned 32-bit store faults on Cortex-M0.
</details>

---

### 7. Buddy allocator `STA`

```c
void *buddy_alloc(struct buddy *b, size_t n);
void  buddy_free(struct buddy *b, void *p, size_t n);
```

<details><summary>Solution</summary>

```c
#define MIN_ORDER 4u                          /* 16-byte smallest block */
#define MAX_ORDER 16u                         /* 64 KB total */
#define N_ORDERS  (MAX_ORDER - MIN_ORDER + 1u)

struct buddy {
    uint8_t *base;
    void    *free_list[N_ORDERS];             /* per-order singly linked lists */
};

static uint8_t order_for(size_t n)
{
    uint8_t o = MIN_ORDER;
    while ((1UL << o) < n && o < MAX_ORDER) o++;
    return o;                                 /* == ceil(log2(n)), Part 01 entry 9 */
}

void *buddy_alloc(struct buddy *b, size_t n)
{
    if (b == NULL || n == 0u) return NULL;

    uint8_t want = order_for(n);
    uint8_t o    = want;

    while (o <= MAX_ORDER && b->free_list[o - MIN_ORDER] == NULL) o++;
    if (o > MAX_ORDER) return NULL;                       /* nothing large enough */

    /* Pop the block we found. */
    void *blk = b->free_list[o - MIN_ORDER];
    b->free_list[o - MIN_ORDER] = *(void **)blk;

    /* Split down, putting each unused half on its order's list. */
    while (o > want) {
        o--;
        void *half = (uint8_t *)blk + (1UL << o);
        *(void **)half = b->free_list[o - MIN_ORDER];
        b->free_list[o - MIN_ORDER] = half;
    }
    return blk;
}

void buddy_free(struct buddy *b, void *p, size_t n)
{
    if (b == NULL || p == NULL) return;

    uint8_t  o   = order_for(n);
    uintptr_t off = (uintptr_t)((uint8_t *)p - b->base);

    /* Merge with the buddy while it is free. The buddy address is one XOR. */
    while (o < MAX_ORDER) {
        uintptr_t buddy_off = off ^ (1UL << o);           /* <-- the whole trick */
        if (!list_remove(&b->free_list[o - MIN_ORDER], b->base + buddy_off)) break;

        off = (off < buddy_off) ? off : buddy_off;        /* the merged block's start */
        o++;
    }

    void *blk = b->base + off;
    *(void **)blk = b->free_list[o - MIN_ORDER];
    b->free_list[o - MIN_ORDER] = blk;
}
```

**`off ^ (1 << order)` is the reason buddy allocation exists.** A block's buddy — the sibling it would merge with — is found by flipping a single bit of its offset. No search, no adjacency test, no address-ordered list: **O(1) buddy location**, which makes coalescing cheap enough to do eagerly on every free. Compare the O(n) predecessor scan in entry 4.

The trade, stated honestly: **internal fragmentation up to 2×.** A 33-byte request takes a 64-byte block, wasting 31 bytes. In exchange you get bounded O(log n) alloc and free, and coalescing that actually keeps up — which is why Linux uses it for physical pages and why it appears in RTOS heaps.

Three details worth having ready:

- **The order is `ceil(log2(n))`**, which is `msb_pos` plus a rounding adjustment — the allocator use of Part 01 entry 9 that was flagged there.
- **Splitting is recursive downward**, and each unused half goes on its own order's list, so no memory is lost during the descent.
- **`buddy_free` needs the size**, because the order cannot be recovered from the pointer alone. Either the caller passes it (as here) or you store an order byte per block in a side table — a real design decision, and the reason `kfree` in Linux does not need a size while this does.

Power-of-two sizing throughout means every block of a given order is naturally aligned to its own size, which is a useful free property when the memory is destined for DMA or an MPU region.
</details>

---

### 8. Interrupt-safe allocation `SEN`

```c
void *pool_alloc_isr(struct pool *p);
```

<details><summary>Solution</summary>

```c
/* Option A: critical section. Correct everywhere, adds a little IRQ latency. */
void *pool_alloc_isr(struct pool *p)
{
    if (p == NULL) return NULL;

    uint32_t st = crit_enter();
    void **blk = p->free_head;
    if (blk != NULL) {
        p->free_head = *blk;
        p->in_use++;
    }
    crit_exit(st);
    return blk;
}

/* Option B: lock-free pop via LDREX/STREX. No interrupts disabled at all. */
void *pool_alloc_lockfree(struct pool *p)
{
    if (p == NULL) return NULL;

    void **head;
    do {
        head = (void **)__LDREXW((volatile uint32_t *)&p->free_head);
        if (head == NULL) {
            __CLREX();                                   /* release the monitor! */
            return NULL;
        }
    } while (__STREXW((uint32_t)*head, (volatile uint32_t *)&p->free_head) != 0u);

    return head;
}
```

**`malloc` from an ISR is never acceptable**, and the reason is worth stating precisely: it is non-reentrant and takes a lock, so an ISR firing while a task holds the heap lock **deadlocks** — the ISR waits for a lock held by code that cannot run until the ISR returns. That is Part 13 entry 8's `printf` problem with the same root cause.

A pool is the right answer because the operation is short and bounded, but the naive version still has a race: `free_head` is read, then written, and an ISR landing between the two hands the same block to two owners.

**Option A is the answer to give.** It is correct on every core including Cortex-M0, and the critical section is three instructions long, so the added latency is negligible — the thing entry 1 of Part 07 warns about is *long* sections, not short ones.

**Option B is worth showing for the follow-up**, with two caveats that are the real content: `__CLREX()` on the early return is mandatory (leaving the monitor armed can make an unrelated later `STREX` misbehave), and this is unavailable on ARMv6-M. It also has the **ABA problem** in the general case — if the head is popped and pushed back between the `LDREX` and `STREX`, the store succeeds against a stale view. Here the exclusive monitor is cleared by any exception return, which closes it on Cortex-M; on a multi-core system you need a tagged pointer or a different structure. Raising ABA unprompted is a strong signal.

The best answer overall: **pre-allocate at init and never allocate at runtime.** An ISR that needs a buffer should take one from a pool filled at startup, and if the pool is empty it must drop the event and count it (Part 07 entry 8) rather than wait. Allocation failure in an ISR has no recovery path, so the design should make it impossible rather than handling it.
</details>

---

## Part 04 — retention table

| Allocator | Gives you | Costs you |
|---|---|---|
| **Fixed block pool** | O(1) both ways, **zero fragmentation**, provable worst case | one size only |
| **Arena / bump** | trivially fast, no metadata | no individual free — reset only |
| **Stack allocator** | scoped free via a saved marker | LIFO order only |
| **Free list + coalesce** | arbitrary sizes | O(n) alloc, fragmentation, unbounded latency |
| **Buddy** | O(log n), **O(1) buddy via XOR**, eager coalescing | up to 2× internal waste |

| Detail | Why |
|---|---|
| Thread the free list through free blocks | zero metadata overhead |
| Validate range **and** block alignment in `free` | a foreign pointer corrupts the list silently |
| Track `high_water` | size the pool from evidence, not guesswork |
| Capacity test by subtraction | `offset + n > size` overflows and passes |
| Coalesce **both** directions | forward-only leaves half the fragmentation |
| Address-ordered list | adjacency in the list ≠ adjacency in memory |
| Aligned alloc | stash the real base at `((void **)user)[-1]` |
| Poison on free | turns use-after-free from "works" into a visible fault |
| Never `malloc` in an ISR | non-reentrant, and the heap lock deadlocks |
| Best fix | pre-allocate at init; make runtime failure impossible |

---

# Part 06 — CRC and Checksums

7 entries from 35. The bank lists FNV-1a, DJB2, SDBM, PJW, Murmur3, MMH and "XOR mixing hash" as seven problems — they are one problem (a non-cryptographic hash for a hash table) with seven constants. What is kept is the progression from the weakest check to the strongest, because **the interview question is almost never "write a CRC", it is "which one, and why".**

> [!IMPORTANT]
> **A checksum answers one question: was this corrupted in transit?** It is not authentication — anyone can recompute it — and it is not encryption. If the threat is a malicious actor rather than a noisy wire, you need a MAC (HMAC, CMAC), and saying so unprompted is what separates a considered answer from a recited one.

---

### 1. XOR and sum — and what they miss `BEG`

```c
uint8_t xor_checksum(const uint8_t *d, size_t n);
uint8_t sum_checksum(const uint8_t *d, size_t n);
```

<details><summary>Solution</summary>

```c
uint8_t xor_checksum(const uint8_t *d, size_t n)
{
    uint8_t c = 0u;
    while (n-- > 0u) c ^= *d++;
    return c;
}

uint8_t sum_checksum(const uint8_t *d, size_t n)
{
    uint8_t s = 0u;
    while (n-- > 0u) s = (uint8_t)(s + *d++);      /* wraps mod 256 — defined */
    return s;
}
```

Two lines each. **The value of this entry is entirely in the failure analysis**, which is what gets asked.

| Error | XOR | Sum | Why |
|---|---|---|---|
| Single bit flip | caught | caught | changes the result |
| **Two bit flips in the same column** | **missed** | caught | XOR is its own inverse |
| **Any reordering of bytes** | **missed** | **missed** | both are commutative |
| **Inserted or deleted zero byte** | **missed** | **missed** | zero is the identity for both |
| Whole-message corruption | 1 in 256 slips through | 1 in 256 | only 8 bits of check |

**Position blindness is the fatal weakness.** Neither notices that `[0x12, 0x34]` and `[0x34, 0x12]` are different messages — and byte reordering is exactly what a desynchronised UART or a partially-overwritten ring buffer produces. That is the specific failure mode these checks cannot see, and it is common.

So when are they acceptable? When the check is a **sanity test on a short local wire** and something else provides real integrity — NMEA over a 20 cm GPS connection (Part 10 entry 5), or a header check backed by a CRC over the payload. On anything longer, noisier, or safety-relevant, a 1-in-256 miss rate over millions of frames means regular undetected corruption.

Worth naming: 8 bits of check gives 1/256 residual error probability *at best*, before the structural blind spots. A CRC-16 gives 1/65536 and catches all the structural cases below.
</details>

---

### 2. Internet checksum (RFC 1071) `INT`

```c
uint16_t inet_checksum(const uint8_t *d, size_t n);
```

<details><summary>Solution</summary>

```c
uint16_t inet_checksum(const uint8_t *d, size_t n)
{
    uint32_t sum = 0u;

    while (n > 1u) {                             /* 16-bit words, big-endian */
        sum += ((uint32_t)d[0] << 8) | d[1];
        d += 2; n -= 2u;
    }
    if (n == 1u) {
        sum += (uint32_t)d[0] << 8;              /* odd tail: pad with zero */
    }

    while ((sum >> 16) != 0u) {                  /* fold carries back in */
        sum = (sum & 0xFFFFu) + (sum >> 16);
    }
    return (uint16_t)~sum;                       /* one's complement */
}
```

The check used by IP, TCP, UDP and ICMP. Sum 16-bit words, **fold the carries back into the low half**, then complement.

**Two properties make it worth knowing.**

**It is endian-independent** in a way that surprises people: computing the sum with byte-swapped words produces a byte-swapped result, so a big-endian and a little-endian machine agree on the checksum without either doing a conversion. RFC 1071 documents this deliberately — it is why the algorithm survived into every IP stack.

**It is incrementally updatable.** A router that decrements the TTL does not recompute the whole checksum; it adjusts it by the difference. That is what makes it cheap enough for per-hop use, and it is the reason it beat a CRC for this role despite being weaker.

**The carry fold is the part people get wrong.** Dropping the carries makes it a plain 16-bit sum and loses most of the detection strength. The `while` loop rather than a single fold handles the case where folding itself generates a carry.

Verification trick worth mentioning, and the same idea as the CRC magic value in Part 10 entry 3: computing the checksum over a message that *includes* its own checksum field yields `0xFFFF` for a valid message — one comparison, no extraction.

Its weakness relative to a CRC: still a sum, so it is blind to word reordering, and it detects far fewer burst errors. Acceptable in IP because a link-layer CRC sits underneath it.
</details>

---

### 3. Fletcher — position sensitivity for free `INT`

```c
uint16_t fletcher16(const uint8_t *d, size_t n);
uint32_t fletcher32(const uint16_t *d, size_t n);
```

<details><summary>Solution</summary>

```c
uint16_t fletcher16(const uint8_t *d, size_t n)
{
    uint16_t s1 = 0u, s2 = 0u;

    while (n-- > 0u) {
        s1 = (uint16_t)((s1 + *d++) % 255u);
        s2 = (uint16_t)((s2 + s1)   % 255u);      /* s2 accumulates s1: POSITION */
    }
    return (uint16_t)((s2 << 8) | s1);
}

uint32_t fletcher32(const uint16_t *d, size_t n)
{
    uint32_t s1 = 0xFFFFu, s2 = 0xFFFFu;

    while (n > 0u) {
        size_t chunk = (n > 359u) ? 359u : n;      /* defer the modulo safely */
        n -= chunk;
        do {
            s1 += *d++;
            s2 += s1;
        } while (--chunk != 0u);

        s1 = (s1 & 0xFFFFu) + (s1 >> 16);
        s2 = (s2 & 0xFFFFu) + (s2 >> 16);
    }
    s1 = (s1 & 0xFFFFu) + (s1 >> 16);
    s2 = (s2 & 0xFFFFu) + (s2 >> 16);
    return (s2 << 16) | s1;
}
```

**`s2 += s1` is the whole idea, and it is elegant.** `s1` is a plain running sum — position-blind. `s2` accumulates `s1` at every step, so each byte's contribution to `s2` is weighted by **how many bytes follow it**. Reorder two bytes and `s1` is unchanged but `s2` differs. That is position sensitivity bought for one addition per byte, which is what the XOR and sum checks in entry 1 fundamentally lack.

**Why modulo 255 rather than 256**, which is the question that follows: with mod 256, a byte of `0x00` and a byte of `0xFF`... more importantly, mod 255 is a prime-adjacent modulus that avoids the systematic blind spots a power-of-two modulus creates, and it makes the all-zeros and all-ones cases distinguishable. Adler-32 uses 65521 — the largest prime below 2¹⁶ — for the same reason, at the cost of a real modulo operation.

**The deferred-modulo trick in `fletcher32` is worth knowing.** `% 255` per byte is expensive on a core without a divider. Accumulating in 32 bits for up to 359 iterations is provably safe against overflow, then folding once amortises the cost. 359 is the largest count for which `s2` cannot overflow — a bound derived, not guessed.

Position: Fletcher sits between a sum and a CRC. Roughly CRC-grade detection for random errors, notably weaker on burst errors, and **much** cheaper — no table, no bit loop. That is why it appears in ZFS, TCP alternatives and firmware where flash is tight.
</details>

---

### 4. CRC-8 bitwise — the mechanism `INT`

```c
uint8_t crc8(const uint8_t *d, size_t n);
```

<details><summary>Solution</summary>

```c
/* Polynomial 0x07 (CRC-8/SMBUS), init 0x00, MSB-first, no reflection. */
uint8_t crc8(const uint8_t *d, size_t n)
{
    uint8_t crc = 0x00u;

    while (n-- > 0u) {
        crc ^= *d++;                              /* bring in the next byte */

        for (uint8_t bit = 0u; bit < 8u; bit++) {
            if ((crc & 0x80u) != 0u) {
                crc = (uint8_t)((crc << 1) ^ 0x07u);   /* shift out a 1: subtract poly */
            } else {
                crc = (uint8_t)(crc << 1);
            }
        }
    }
    return crc;
}
```

**A CRC is polynomial long division, and the loop above is the division.** Treat the message as a binary polynomial, divide by the generator polynomial, and the CRC is the remainder. "Subtracting" in this arithmetic is XOR, which is why there are no carries and why the whole thing is a shift and an XOR.

**Why this beats every sum-based check:** division is not commutative. A byte's contribution depends on its position because subsequent shifts move it, so reordering, insertion and deletion all change the remainder. That is the structural property entry 1 lacks and entry 3 approximates.

The guarantees a well-chosen polynomial gives, which are the reason CRCs are specified in standards:

- **all single-bit errors**
- **all double-bit errors** (for messages under a bound set by the polynomial)
- **all odd numbers of bit errors**, if the polynomial has `(x + 1)` as a factor
- **all burst errors shorter than the CRC width**

Those are proofs, not statistics. A checksum has none of them.

**The five parameters that define a CRC** — and the reason "compute a CRC-16" is an under-specified request: width, polynomial, initial value, input reflection, and final XOR. Two implementations agreeing on the polynomial and differing on the init value produce completely different results, which is the single most common source of "my CRC does not match the device's". Always name the full variant (CRC-16/CCITT-FALSE, CRC-16/MODBUS) and check it against a known vector: the ASCII string `"123456789"` has a published check value for every standard variant.

Cost here is 8 iterations per byte. Entry 5 fixes that.
</details>

---

### 5. CRC-16 table-driven, and the reflected variant `SEN`

```c
uint16_t crc16_ccitt(const uint8_t *d, size_t n);       /* MSB-first */
uint16_t crc16_modbus(const uint8_t *d, size_t n);      /* reflected */
uint16_t crc16_table(const uint8_t *d, size_t n);
```

<details><summary>Solution</summary>

```c
/* CRC-16/CCITT-FALSE: poly 0x1021, init 0xFFFF, no reflection, no final XOR. */
uint16_t crc16_ccitt(const uint8_t *d, size_t n)
{
    uint16_t crc = 0xFFFFu;

    while (n-- > 0u) {
        crc ^= (uint16_t)((uint16_t)*d++ << 8);        /* byte into the HIGH half */
        for (uint8_t b = 0u; b < 8u; b++) {
            crc = (crc & 0x8000u) ? (uint16_t)((crc << 1) ^ 0x1021u)
                                  : (uint16_t)(crc << 1);
        }
    }
    return crc;
}

/* CRC-16/MODBUS: reflected. poly 0xA001 (bit-reversed 0x8005), init 0xFFFF. */
uint16_t crc16_modbus(const uint8_t *d, size_t n)
{
    uint16_t crc = 0xFFFFu;

    while (n-- > 0u) {
        crc ^= (uint16_t)*d++;                         /* byte into the LOW half */
        for (uint8_t b = 0u; b < 8u; b++) {
            crc = (crc & 1u) ? (uint16_t)((crc >> 1) ^ 0xA001u)
                             : (uint16_t)(crc >> 1);
        }
    }
    return crc;
}

/* Table-driven: 8x faster, 512 bytes of flash. */
static uint16_t g_crc_tab[256];

void crc16_table_init(void)
{
    for (uint16_t i = 0u; i < 256u; i++) {
        uint16_t c = (uint16_t)(i << 8);
        for (uint8_t b = 0u; b < 8u; b++) {
            c = (c & 0x8000u) ? (uint16_t)((c << 1) ^ 0x1021u) : (uint16_t)(c << 1);
        }
        g_crc_tab[i] = c;
    }
}

uint16_t crc16_table(const uint8_t *d, size_t n)
{
    uint16_t crc = 0xFFFFu;
    while (n-- > 0u) {
        crc = (uint16_t)((crc << 8) ^ g_crc_tab[(crc >> 8) ^ *d++]);
    }
    return crc;
}
```

**Reflected versus non-reflected is the thing to understand, not memorise.** A reflected CRC processes each byte least-significant bit first, so the implementation shifts **right** and uses the bit-reversed polynomial. `0xA001` is `0x8005` reversed. Everything else is identical.

Why both exist: **serial hardware shifts LSB-first** (UART, and hence Modbus), so a reflected CRC matches the wire order and a hardware implementation needs no bit reversal. MSB-first variants come from parallel and telecom contexts. This is not arbitrary history — it is the wire format determining the algorithm, and being able to explain it is what makes the two variants memorable rather than confusing.

**The table method's derivation is worth stating**, because it explains why the table is possible at all: the CRC of a byte depends only on the top byte of the current CRC XORed with the input byte, so all 256 outcomes can be precomputed. One table lookup and one XOR replaces eight iterations — **about 8× faster for 512 bytes of flash.**

The variants of that trade, which is the real engineering content:

| Method | Speed | Flash |
|---|---|---|
| Bitwise | 1× | ~40 bytes |
| **Nibble table (16 entries)** | ~4× | **32 bytes** |
| Byte table (256 entries) | ~8× | 512 bytes |
| Hardware CRC unit | ~32× | 0 |

**The nibble table is the one people forget and it is often the right answer** on a small part: half the speedup for 1/16th the memory. And most STM32s have a hardware CRC peripheral — check before writing any of this.

Note `crc16_table_init` computes the table at boot; a `const` table generated offline puts it in flash instead and costs no RAM, which is what production code does.
</details>

---

### 6. CRC-32 streaming and the residue trick `SEN`

```c
uint32_t crc32_update(uint32_t crc, const uint8_t *d, size_t n);
uint32_t crc32(const uint8_t *d, size_t n);
```

<details><summary>Solution</summary>

```c
/* CRC-32/ISO-HDLC (Ethernet, zip, PNG):
   poly 0xEDB88320 reflected, init 0xFFFFFFFF, final XOR 0xFFFFFFFF. */

uint32_t crc32_update(uint32_t crc, const uint8_t *d, size_t n)
{
    while (n-- > 0u) {
        crc ^= *d++;
        for (uint8_t b = 0u; b < 8u; b++) {
            crc = (crc & 1u) ? ((crc >> 1) ^ 0xEDB88320UL) : (crc >> 1);
        }
    }
    return crc;                                  /* NOT finalised — see below */
}

uint32_t crc32(const uint8_t *d, size_t n)
{
    return crc32_update(0xFFFFFFFFUL, d, n) ^ 0xFFFFFFFFUL;
}

/* Streaming over a whole firmware image, chunk by chunk. */
uint32_t crc32_flash(uint32_t addr, size_t len)
{
    uint32_t crc = 0xFFFFFFFFUL;
    uint8_t  buf[256];

    while (len > 0u) {
        size_t chunk = (len > sizeof buf) ? sizeof buf : len;
        flash_read(addr, buf, chunk);
        crc = crc32_update(crc, buf, chunk);     /* state carries between calls */
        addr += chunk;
        len  -= chunk;
        wdt_task_alive(TASK_VERIFY);             /* this can take a while */
    }
    return crc ^ 0xFFFFFFFFUL;                  /* finalise ONCE, at the end */
}
```

**Splitting update from finalise is the design point**, and it is what the OTA verification in Part 11 entry 6 needs. The running state is the raw CRC register; the final XOR is applied exactly once. Applying it per chunk gives a wrong answer — and it is the standard bug when someone wraps a one-shot `crc32()` in a loop.

That structure is also why the intermediate value must not be exposed as "the CRC so far" without qualification: it is the state, not the result.

**The residue property**, which is the same trick as Part 10 entry 3 and worth knowing for all widths: appending the correct CRC to a message and then computing the CRC over the combined data yields a **fixed constant** — `0x2144DF1C` for this variant, or zero for variants without the final XOR. So verification is one comparison with no extraction and no byte-order handling. That is why Modbus can check `crc16_modbus(frame, len) == 0` over the whole frame including its CRC.

**Why CRC-32 for firmware images specifically:** 32 bits gives a residual error probability around 2⁻³², and it detects every burst error up to 32 bits. Over a 256 KB image that is the right strength. Two caveats to state, because they are where CRC gets misused:

- **A CRC is not a signature.** It detects accidental corruption. An attacker who modifies the image simply recomputes it. Secure boot needs a cryptographic hash and a signature — which is exactly why the OTA entry validates with CRC *and* why a security-conscious design adds ECDSA on top.
- **CRC proves integrity, not correctness.** An image that passes CRC and then crashes on boot is a valid image of broken code, which is why the rollback flag in Part 11 entry 6 exists.
</details>

---

### 7. Non-cryptographic hashes `INT`

```c
uint32_t fnv1a(const void *data, size_t n);
uint32_t djb2(const char *s);
```

<details><summary>Solution</summary>

```c
/* FNV-1a: XOR then multiply. Good avalanche, one multiply per byte. */
uint32_t fnv1a(const void *data, size_t n)
{
    const uint8_t *p = data;
    uint32_t h = 2166136261UL;                   /* offset basis */

    while (n-- > 0u) {
        h ^= *p++;                               /* XOR first — that is the "1a" */
        h *= 16777619UL;                         /* FNV prime */
    }
    return h;
}

/* DJB2: shift-add. No multiply — cheaper on a core without one. */
uint32_t djb2(const char *s)
{
    uint32_t h = 5381UL;
    while (*s != '\0') {
        h = ((h << 5) + h) + (uint8_t)*s++;      /* h * 33 + c */
    }
    return h;
}
```

**The bank asks for seven of these. They are one function with different constants, and the only interesting question is which properties you need.**

| Need | Use |
|---|---|
| Hash table index (Part 05 entry 18) | FNV-1a or DJB2 — fast, decent distribution |
| Better distribution, more speed on long keys | Murmur3, xxHash |
| **Detect transmission corruption** | **a CRC, not a hash** |
| **Detect tampering** | **SHA-256 + a signature** |

**The distinction that matters: a hash optimises for uniform distribution; a CRC optimises for error detection.** They are different objectives and they are not interchangeable. FNV-1a has no guarantee about two-bit errors; CRC-16 has a proof. Using a hash as a frame check is a real and common mistake.

**Order of operations in FNV-1a is not cosmetic.** FNV-1 multiplies then XORs; FNV-1a XORs then multiplies, and 1a has measurably better avalanche — a one-bit input change propagates further. Same constants, different quality, which is why the variant letter is part of the name.

**`(h << 5) + h` is `h * 33`**, and DJB2's reason for existing is that it needs no multiplier — on a Cortex-M0 without a fast multiply, or an 8-bit part, that is a real saving. The choice of 33 is empirical; the author never fully justified it, which is worth saying because it illustrates that these constants are measured rather than derived.

The practical warnings: **truncate by masking to a power of two only if the hash has good low bits** (FNV-1a does; a plain sum does not), and for a hash table exposed to external keys, a non-cryptographic hash is vulnerable to deliberate collision flooding — which matters less on a device with a 64-entry table and no adversary, but is worth naming.
</details>

---

## Part 06 — retention table

| Check | Catches | Misses | Cost |
|---|---|---|---|
| XOR | single bits | **even bits per column, reordering, zero insert** | 1 op/byte |
| Sum | single bits | **reordering, zero insert** | 1 op/byte |
| Internet checksum | more than a plain sum; **endian-independent, incrementally updatable** | word reordering | 1 add/word |
| Fletcher | **position** (via `s2 += s1`) | weak on bursts | 2 adds/byte |
| CRC-8/16/32 | all 1-bit, 2-bit, odd-count, bursts < width — **proven** | nothing accidental, at rate 2⁻ⁿ | 8 ops or 1 lookup |
| Hash (FNV/DJB2) | nothing — it is for **distribution** | everything | 1–2 ops/byte |
| SHA + signature | tampering | — | expensive |

| Detail | Why |
|---|---|
| Five CRC parameters | width, poly, init, reflection, final XOR — "CRC-16" alone is ambiguous |
| Reflected variants | serial hardware shifts LSB-first; that is where `0xA001` comes from |
| Verify with `"123456789"` | every standard variant has a published check value |
| Nibble table | 4× faster for **32 bytes** — the forgotten middle option |
| Residue trick | CRC over data+CRC is a constant — verify in one comparison |
| Split update/finalise | apply the final XOR **once**, or streaming is wrong |
| CRC ≠ signature | detects accidents, not attackers |

---

# Part 12 — Embedded Algorithms

10 entries from 45. The bank's thermistor, voltage-divider, power-factor and battery-SoH problems are applications of two skills: **fixed-point arithmetic** and **filtering**. Those two, plus the control and lookup patterns, are what is kept.

> [!IMPORTANT]
> **Why integers, not floats.** Cortex-M0/M0+/M3 have **no FPU**, so a `float` operation is a library call costing tens to hundreds of cycles. Even on an M4F, `double` is software. Fixed-point is exact, deterministic, and often faster than the FPU path once conversions are counted — and determinism is what a control loop needs.

---

### 1. Fixed-point arithmetic — Q format `INT`

```c
/* Q16.16: 16 integer bits, 16 fractional bits. */
typedef int32_t q16_t;
```

<details><summary>Solution</summary>

```c
#define Q       16
#define Q_ONE   (1L << Q)                          /* 1.0 */

#define TO_Q(x)      ((q16_t)((x) * (double)Q_ONE))     /* compile-time constants only */
#define FROM_Q(q)    ((int32_t)((q) >> Q))              /* truncates toward -inf */

/* Rounded conversion back to an integer. */
static inline int32_t q_round(q16_t q)
{
    return (int32_t)((q + (Q_ONE / 2)) >> Q);
}

/* Multiply: the product has 2Q fractional bits, so shift back by Q. */
static inline q16_t q_mul(q16_t a, q16_t b)
{
    return (q16_t)(((int64_t)a * (int64_t)b) >> Q);     /* 64-bit intermediate! */
}

/* Divide: pre-shift the numerator so the quotient lands with Q fraction bits. */
static inline q16_t q_div(q16_t a, q16_t b)
{
    if (b == 0) return (a >= 0) ? INT32_MAX : INT32_MIN;
    return (q16_t)((((int64_t)a) << Q) / b);
}
```

**The 64-bit intermediate in `q_mul` is the single most important line.** Two Q16.16 values each up to 32767 multiply to a product needing 64 bits before the shift. Doing it in `int32_t` overflows for any operands above about 128, silently, and the result is garbage that looks like a plausible number. On Cortex-M, `SMULL` gives the 64-bit product in one instruction, so the correct version is not even slower.

**Choosing the split is the design decision.** Q16.16 gives ±32767 with a resolution of 1/65536. If your signal is a temperature in the range ±150 °C, Q8.24 gives far more precision. **Pick the split from the actual range and required resolution, then document it** — a comment naming the range is what stops the next person overflowing it.

Two details worth having: **shifting a negative value right rounds toward negative infinity**, not toward zero, so `FROM_Q(-1.5)` is −2 while `(int)(-1.5)` is −1 — a real source of asymmetric behaviour in control loops, and `q_round` is the fix. And `TO_Q` with a floating-point literal is fine because the compiler evaluates it at compile time; using it on a runtime `double` reintroduces the software float you were avoiding.

The alternative worth naming: for many problems, plain integers in scaled units (millivolts, tenths of a degree, microseconds) are simpler and sufficient, and that is what the thermostat in Part 11 entry 2 uses. Reach for Q format when you need fractional *arithmetic*, not merely fractional *units*.
</details>

---

### 2. Integer scaling with rounding `BEG`

```c
int32_t scale(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max);
```

<details><summary>Solution</summary>

```c
int32_t scale(int32_t x, int32_t in_min, int32_t in_max,
              int32_t out_min, int32_t out_max)
{
    int32_t in_span  = in_max - in_min;
    if (in_span == 0) return out_min;

    int32_t out_span = out_max - out_min;

    /* Multiply BEFORE dividing, in 64 bits, with rounding. */
    int64_t num = (int64_t)(x - in_min) * (int64_t)out_span;
    int64_t half = (num >= 0) ? (in_span / 2) : -(in_span / 2);

    return out_min + (int32_t)((num + half) / in_span);
}

/* ADC counts to millivolts: scale(raw, 0, 4095, 0, 3300) */
```

Three mistakes in one small function, and all three are common.

**Multiply before dividing.** `((x - in_min) / in_span) * out_span` in integer arithmetic truncates the ratio to 0 or 1 first, destroying all precision. The order matters and it is the whole reason this is a problem rather than one line.

**Use a 64-bit intermediate.** `(x - in_min) * out_span` with an ADC count of 4000 and an output span of 1000000 overflows `int32_t`. This is the same failure as `q_mul` above, and the same fix.

**Round, do not truncate.** Adding half the divisor before dividing rounds to nearest, halving the worst-case error. The sign handling matters: adding a positive half to a negative numerator rounds the wrong way, which biases every negative reading.

That last point generalises — it is the same rounding discipline as the UART baud divisor in Part 09 entry 2, where truncation instead of rounding pushed the error over the 2% budget.

For a hot path where `in_span` is a compile-time constant, the compiler turns the division into a multiply-and-shift. If it is not constant and the core has no divider, precompute a reciprocal in Q format and use `q_mul` — that is the "convert divide to reciprocal" optimisation the bank asks about separately.
</details>

---

### 3. Moving average `BEG`

```c
uint16_t ma_update(struct ma *m, uint16_t sample);
```

<details><summary>Solution</summary>

```c
#define MA_LOG2 4u                          /* window of 16 — a POWER OF TWO */
#define MA_N    (1u << MA_LOG2)

struct ma {
    uint16_t buf[MA_N];
    uint32_t sum;                           /* running sum: O(1) per update */
    uint8_t  idx;
    bool     primed;
};

uint16_t ma_update(struct ma *m, uint16_t sample)
{
    if (m == NULL) return 0u;

    m->sum -= m->buf[m->idx];               /* remove the oldest */
    m->sum += sample;                       /* add the newest    */
    m->buf[m->idx] = sample;
    m->idx = (uint8_t)((m->idx + 1u) & (MA_N - 1u));

    return (uint16_t)(m->sum >> MA_LOG2);   /* divide by N — a shift */
}
```

**The running sum is the point: O(1) per sample, not O(N).** Subtract the value leaving the window, add the one entering. Re-summing the whole buffer every update is the naive version and it is what the question is testing against.

**Power-of-two window** so the division is a shift. A window of 10 needs a real division; a window of 16 needs one instruction, and the difference in filter behaviour is negligible.

**The accumulator must not overflow:** `uint32_t` holds 16 × 65535 comfortably. Sizing it as `uint16_t` is a silent wraparound that produces wild output — check `N × max_sample` against the type every time.

What it is good and bad at, which is the follow-up:

- **Good:** exactly cancels periodic noise whose period divides the window. Averaging 16 samples taken at 1 kHz cancels 62.5 Hz and its harmonics — this is how you structurally remove mains hum rather than statistically attenuating it (Part 09 entry 8).
- **Bad:** a single wild outlier is spread across N outputs rather than rejected. For spike rejection you want the median filter in entry 5.
- **Latency:** the output lags the input by roughly N/2 samples, which in a control loop is phase delay that can destabilise the loop.

`primed` handles the startup transient — for the first N samples the window contains zeros, so the output ramps up from zero. Either report "not ready" or pre-fill the buffer with the first sample.
</details>

---

### 4. Exponential moving average, done right `INT`

```c
int32_t ema_update(struct ema *e, int32_t sample);
```

<details><summary>Solution</summary>

```c
struct ema {
    int32_t acc;                            /* holds the value SCALED UP by 2^shift */
    uint8_t shift;                          /* larger == smoother, slower */
};

/* BROKEN: for small differences the shift truncates to zero and the filter STICKS. */
int32_t ema_broken(struct ema *e, int32_t x)
{
    e->acc += (x - e->acc) >> e->shift;     /* (x - acc) < 2^shift  ->  adds 0 */
    return e->acc;
}

/* CORRECT: keep the accumulator scaled, so the fractional part survives. */
int32_t ema_update(struct ema *e, int32_t sample)
{
    if (e == NULL) return 0;

    /* acc is the true value << shift. The residual is never discarded. */
    e->acc += sample - (e->acc >> e->shift);
    return e->acc >> e->shift;
}

void ema_init(struct ema *e, uint8_t shift, int32_t initial)
{
    e->shift = shift;
    e->acc   = initial << shift;            /* seed, or the output ramps from 0 */
}
```

**The truncation stall is the bug this entry exists for, and it is genuinely subtle.** In the broken form, once `|x - acc|` is smaller than `2^shift`, the shift yields zero and the accumulator stops moving. With `shift = 6`, the filter can never track a change smaller than 64 counts — so a slowly drifting temperature reads as constant, and a slow leak in a pressure signal is invisible. It looks like the filter working well.

**Keeping the accumulator pre-scaled fixes it** because the residual stays in the low bits and accumulates across samples. This is the same "keep the remainder" principle as entry 10.

**Why an EMA rather than a moving average:** no buffer at all — one `int32_t` of state instead of N samples. On a system filtering 30 sensor channels that is 30 words rather than 480. The response is exponential rather than a hard window, and the smoothing factor is `1/2^shift`, so `shift` trades responsiveness against noise rejection with a single knob.

Two more: **seed the accumulator** with the first sample or the output visibly ramps from zero over dozens of samples, which in a control loop is a startup transient that can trip a limit. And this is a **single-pole IIR low-pass filter** — naming it that, with a cutoff of roughly `f_sample / (2π · 2^shift)`, connects it to the signal-processing vocabulary an interviewer may be using.
</details>

---

### 5. Median filter — spike rejection `INT`

```c
uint16_t median3(uint16_t a, uint16_t b, uint16_t c);
uint16_t median_n(struct medfilt *m, uint16_t sample);
```

<details><summary>Solution</summary>

```c
/* Branch-light median of three — the common case in firmware. */
uint16_t median3(uint16_t a, uint16_t b, uint16_t c)
{
    uint16_t lo = (a < b) ? a : b;
    uint16_t hi = (a < b) ? b : a;
    if (c < lo) return lo;
    if (c > hi) return hi;
    return c;
}

#define MED_N 5u

struct medfilt {
    uint16_t buf[MED_N];
    uint8_t  idx;
};

uint16_t median_n(struct medfilt *m, uint16_t sample)
{
    if (m == NULL) return 0u;

    m->buf[m->idx] = sample;
    m->idx = (uint8_t)((m->idx + 1u) % MED_N);

    uint16_t tmp[MED_N];
    memcpy(tmp, m->buf, sizeof tmp);

    /* Insertion sort: MED_N is tiny, so this is the right algorithm (Part 05/20). */
    for (uint8_t i = 1u; i < MED_N; i++) {
        uint16_t k = tmp[i];
        uint8_t  j = i;
        while (j > 0u && tmp[j - 1u] > k) { tmp[j] = tmp[j - 1u]; j--; }
        tmp[j] = k;
    }
    return tmp[MED_N / 2u];
}
```

**A median rejects an outlier completely; a mean does not.** One corrupted ADC sample of 4095 in an otherwise steady 2000 shifts a 5-sample mean by 419 counts and appears in five consecutive outputs. The median ignores it entirely — a single spike among five samples cannot be the middle value.

That is the property to state, and it is why median filtering is the correct first stage for **any signal subject to impulse noise**: an ADC near a switching regulator, an ultrasonic rangefinder getting a spurious echo, a capacitive touch reading during an RF burst.

**Median for spikes, mean for random noise — and they compose.** The standard chain is median first to remove impulses, then a moving average or EMA to smooth what remains. Doing it the other way round is wrong, because averaging spreads the spike into neighbouring samples where the median can no longer isolate it. That ordering point is a good thing to volunteer.

**Odd window sizes only**, so there is a unique middle element and no averaging of the two central values.

Insertion sort is deliberate here, for exactly the reason given in Part 05 entry 20: at N = 5 it beats anything asymptotically better, and it needs no recursion. For median-of-3 the branch version above is faster still and is what you use in an ISR. The costs to acknowledge: O(N log N) or O(N²) per sample versus O(1) for an EMA, and it distorts genuine fast edges — a real step change is delayed by N/2 samples.
</details>

---

### 6. Integer square root `INT`

```c
uint32_t isqrt(uint32_t n);
```

<details><summary>Solution</summary>

```c
/* Digit-by-digit (restoring) method: shifts, adds and compares only. */
uint32_t isqrt(uint32_t n)
{
    uint32_t res = 0u;
    uint32_t bit = 1UL << 30;                    /* highest even power of two */

    while (bit > n) bit >>= 2;                   /* skip leading zero pairs */

    while (bit != 0u) {
        if (n >= res + bit) {
            n   -= res + bit;
            res  = (res >> 1) + bit;
        } else {
            res >>= 1;
        }
        bit >>= 2;
    }
    return res;                                  /* floor(sqrt(n)) */
}
```

**No floating point, no `sqrt()`, no libm** — which on a Cortex-M0 saves both the several-hundred-cycle software float and roughly a kilobyte of flash. Fixed 16 iterations, so the timing is deterministic: usable in an ISR, unlike a library call with a data-dependent path.

The method is long division in base 4: two bits of the operand per digit of the result, hence `bit >>= 2`. `bit` starting at 2³⁰ is the largest even power of two below 2³², and the skip loop avoids iterating over leading zero pairs.

**It returns `floor(sqrt(n))`**, which must be documented because callers assume otherwise. `isqrt(15) == 3`, and if you need rounding, test whether `(res+1)² <= n`.

Where it earns its place: **RMS calculation** for a current or voltage measurement — accumulate the squares in a `uint64_t`, divide by the count, take the integer square root. That is the whole of a power meter's front end, and it is the reason this appears in the bank alongside power factor and energy accumulation.

The alternatives worth naming: for repeated calls over a small domain, a lookup table with interpolation (entry 7) is faster; for a normalisation where you only need `1/sqrt(x)`, the reciprocal-square-root trick avoids the division as well. And on a Cortex-M4F, hardware `VSQRT` is one instruction — check what the target actually has before hand-rolling.
</details>

---

### 7. Lookup table with linear interpolation `INT`

```c
int32_t lut_interp(const int16_t *y, size_t n, int32_t x_min, int32_t x_step, int32_t x);
```

<details><summary>Solution</summary>

```c
/* Table of n outputs at evenly spaced inputs x_min, x_min+step, ... */
int32_t lut_interp(const int16_t *y, size_t n, int32_t x_min, int32_t x_step, int32_t x)
{
    if (y == NULL || n == 0u || x_step <= 0) return 0;

    if (x <= x_min)                                   return y[0];          /* clamp */
    if (x >= x_min + (int32_t)(n - 1u) * x_step)      return y[n - 1u];     /* clamp */

    int32_t off = x - x_min;
    size_t  i   = (size_t)(off / x_step);             /* segment index */
    int32_t rem = off % x_step;                       /* position within it */

    int32_t y0 = y[i];
    int32_t y1 = y[i + 1u];

    /* y0 + (y1-y0) * rem / step, rounded, 64-bit intermediate. */
    int64_t num = (int64_t)(y1 - y0) * rem;
    int32_t half = (num >= 0) ? (x_step / 2) : -(x_step / 2);

    return y0 + (int32_t)((num + half) / x_step);
}
```

**This is how firmware evaluates any nonlinear function**, and the argument for it is concrete: a thermistor's resistance-to-temperature relation involves a logarithm, and a 33-point table with interpolation is accurate to a fraction of a degree, costs about 66 bytes of flash, and runs in tens of cycles with deterministic timing. `log()` costs a library call, hundreds of cycles, and several hundred bytes.

The same pattern covers thermocouple linearisation, battery state-of-charge from open-circuit voltage, sensor calibration curves, gamma correction and sine generation.

Three things that matter:

- **Clamp both ends.** Extrapolating past the table is how you get a temperature reading of −200 °C from a disconnected sensor. Clamping turns an out-of-range input into a saturated value the caller can sanity-check.
- **Even spacing makes the index a division**, and if `x_step` is a power of two it is a shift. That is worth designing the table around — choose the sample points for arithmetic convenience, not round decimal values.
- **64-bit intermediate and rounding**, for the reasons in entry 2.

Two extensions worth naming: **non-uniform spacing** needs a binary search for the segment (Part 05 entry 20), which costs O(log n) but lets you put points densely where the curve bends and sparsely where it is flat — usually a better accuracy-per-byte trade. And for a **sine table**, exploit the symmetry: one quarter-wave table plus sign and index manipulation covers the full cycle at a quarter of the memory.
</details>

---

### 8. Integer PID controller `SEN`

```c
int32_t pid_update(struct pid *p, int32_t setpoint, int32_t measured);
```

<details><summary>Solution</summary>

```c
struct pid {
    int32_t kp, ki, kd;              /* Q16.16 gains */
    int32_t integral;                /* Q16.16 accumulator */
    int32_t prev_measured;           /* for derivative-on-measurement */
    int32_t out_min, out_max;
    int32_t i_min, i_max;            /* separate integral clamp */
    bool    primed;
};

int32_t pid_update(struct pid *p, int32_t setpoint, int32_t measured)
{
    if (p == NULL) return 0;

    int32_t err = setpoint - measured;

    /* --- proportional --- */
    int32_t out = q_mul(p->kp, err << Q);

    /* --- integral, with ANTI-WINDUP clamping --- */
    p->integral += q_mul(p->ki, err << Q);
    if (p->integral > p->i_max) p->integral = p->i_max;
    if (p->integral < p->i_min) p->integral = p->i_min;
    out += p->integral;

    /* --- derivative ON MEASUREMENT, negated: avoids setpoint kick --- */
    if (p->primed) {
        int32_t d = measured - p->prev_measured;
        out -= q_mul(p->kd, d << Q);
    }
    p->prev_measured = measured;
    p->primed = true;

    /* --- output saturation --- */
    int32_t result = FROM_Q(out);
    if (result > p->out_max) result = p->out_max;
    if (result < p->out_min) result = p->out_min;
    return result;
}
```

**Three implementation details separate a working PID from a textbook one, and they are the interview content.**

**Integral windup.** If the output is saturated — the heater is already at 100% — the error stays positive and the integral keeps accumulating, sometimes to an enormous value. When the process finally responds, that stored integral takes minutes to unwind, so the output stays pinned and overshoots badly. **Clamping the integral separately from the output** is the fix. The alternative, conditional integration (stop accumulating while saturated), is equally valid and worth naming.

**Derivative kick.** Computing the derivative of the *error* means a step change in setpoint produces an instantaneous infinite derivative, and the output slams to a limit for one cycle. Taking the derivative of the **measurement** instead (and negating, since `d(error) = -d(measurement)` for a constant setpoint) gives identical disturbance rejection with no kick on a setpoint change. This is the single most useful practical PID detail.

**Fixed sample rate.** The `ki` and `kd` terms have `dt` folded into them, so calling `pid_update` at an irregular interval silently changes the tuning. Call it from a timer, and if the rate must change, rescale the gains. Stating that the loop period is part of the tuning is what shows you have run one.

Two more: `primed` prevents a spurious derivative on the first call, when `prev_measured` is meaningless. And the derivative term amplifies noise by definition, so it usually needs the measurement low-pass filtered first (entry 4) — which is why many industrial controllers ship as PI with `kd = 0`.
</details>

---

### 9. Slew rate limiter `INT`

```c
int32_t slew_update(struct slew *s, int32_t target);
```

<details><summary>Solution</summary>

```c
struct slew {
    int32_t current;
    int32_t max_up;                  /* max increase per call */
    int32_t max_down;                /* max decrease per call — often different */
};

int32_t slew_update(struct slew *s, int32_t target)
{
    if (s == NULL) return 0;

    int32_t delta = target - s->current;

    if (delta > s->max_up)        s->current += s->max_up;
    else if (delta < -s->max_down) s->current -= s->max_down;
    else                          s->current  = target;    /* close enough: snap */

    return s->current;
}
```

Ten lines, and it prevents a class of physical damage.

**Why asymmetric limits matter**, which is the detail that makes this a real problem rather than a clamp: a motor may accelerate slowly but must be able to decelerate quickly for safety; a heater may ramp up gently but should be able to shut off immediately; an LED dimmer wants a smooth rise and an instant off. **One rate is almost never correct for both directions**, and a single `max_rate` parameter is the usual oversight.

Where it is required, not merely nice:

| System | Consequence of no limit |
|---|---|
| Motor drive | current spike, torque shock, mechanical damage |
| Switching regulator setpoint | inrush current, possible overcurrent trip |
| Valve or actuator | water hammer, mechanical wear |
| LED brightness | visible stepping |
| Load-shedding relay | contact arcing |

**The `else` snap-to-target is not cosmetic.** Without it, `current` approaches `target` asymptotically and never arrives, so a "reached setpoint" comparison never becomes true and the state machine above it hangs. That is a real bug and it is easy to write.

Two extensions: rate is per *call*, so the limiter is coupled to the call frequency exactly as the PID is — document the intended period. And for something needing bounded jerk as well as bounded acceleration, the next step is an S-curve profile, which limits the rate of change *of the rate*; naming that shows where this pattern leads in motion control.
</details>

---

### 10. Accumulation without drift `SEN`

```c
uint32_t energy_accumulate(struct energy *e, uint32_t power_mw, uint32_t dt_ms);
```

<details><summary>Solution</summary>

```c
struct energy {
    uint64_t mwms;                   /* accumulate in the SMALLEST unit */
    uint32_t mwh;                    /* the reported value */
};

/* 1 mWh == 3,600,000 mW·ms */
#define MWMS_PER_MWH 3600000ULL

uint32_t energy_accumulate(struct energy *e, uint32_t power_mw, uint32_t dt_ms)
{
    if (e == NULL) return 0u;

    e->mwms += (uint64_t)power_mw * dt_ms;       /* exact, no division yet */

    while (e->mwms >= MWMS_PER_MWH) {            /* carry whole units out */
        e->mwms -= MWMS_PER_MWH;                 /* REMAINDER IS KEPT */
        e->mwh++;
    }
    return e->mwh;
}
```

**The bug this prevents is cumulative truncation drift, and it is the reason the entry exists.** The naive version divides on every sample:

```c
e->mwh += (power_mw * dt_ms) / MWMS_PER_MWH;     /* almost always adds ZERO */
```

At 1 W sampled every 100 ms, each increment is 100000 mW·ms against a divisor of 3600000 — the integer division yields 0, **every single time**. The meter reads zero forever. Even where it does not truncate entirely, discarding the remainder loses a fraction on every sample, and over a million samples that is a systematic under-read.

**Accumulating in the smallest unit and carrying out whole units keeps the remainder**, so the total is exact regardless of sample rate. This is the same principle as the EMA accumulator in entry 4: **never discard the fractional part of a running total.**

Three supporting points:

- **`uint64_t` for the accumulator**, sized against the worst case. 100 W for a year in mW·ms is about 3×10¹⁵ — beyond `uint32_t` by five orders of magnitude, and this is precisely where a 32-bit accumulator overflows quietly six months into deployment.
- **`while`, not `if`**, in case a long `dt_ms` (after a sleep, or a missed sample) contributes more than one whole unit.
- **Persist both fields.** Writing only `mwh` to flash loses the remainder on every power cycle, which reintroduces the drift you just eliminated — a device power-cycled daily accumulates a real error.

The same pattern is the correct shape for anything integrating over time: coulomb counting for battery state-of-charge, run-hour meters, flow totalisers, and the tickless-idle catch-up in Part 08 entry 10.
</details>

---

## Part 12 — retention table

| Technique | The point |
|---|---|
| Fixed point | **64-bit intermediate** in multiply, or it overflows silently |
| Q format | pick the split from the range; right-shift rounds toward −∞ |
| Scaling | multiply before dividing, 64-bit intermediate, **round** |
| Moving average | running sum = O(1); power-of-two window = shift |
| Window cancels periodic noise | average over an exact multiple of the noise period |
| EMA | **keep the accumulator pre-scaled** or it stalls on small deltas |
| Median vs mean | median rejects **spikes**, mean rejects **random noise** |
| Filter order | median **first**, then average — never the reverse |
| `isqrt` | floor, deterministic, no libm; the front end of any RMS |
| LUT + interpolation | how nonlinear functions are evaluated; **clamp both ends** |
| PID | anti-windup clamp, derivative **on measurement**, fixed sample rate |
| Slew limiter | asymmetric up/down rates; **snap to target** or it never arrives |
| Accumulation | keep the remainder; size the accumulator; persist both parts |

---

# Part 14 — Review, Patterns, Optimization

8 entries from 59. The bank's 14 code-review prompts, 25 design patterns and 20 optimization items are discussion questions, not coding problems — so these are written as **talking points you can deliver**, not functions to reproduce.

---

### 1. What to look for in a firmware code review `INT`

<details><summary>The checklist</summary>

Ordered by how often the finding is real. Delivering this as a *structured* list rather than ad-hoc observations is itself the signal.

**Correctness under concurrency** — the highest-yield category:
- Which contexts touch each shared variable? Is every access to it atomic or guarded?
- Is `volatile` present where an ISR writes, and absent where it is merely cargo-culted?
- Any read-modify-write on data an ISR touches (`|=`, `++`, bit-fields)?
- Any lock taken while another is held — and if so, is the order consistent?

**Bounds and lifetimes:**
- Every length that came from outside: validated against capacity *before* use?
- Every array index: provably in range, or checked?
- Any pointer stored beyond the lifetime of what it points to?
- Any `memcpy` where the regions might overlap?

**Hardware discipline:**
- Every wait on hardware: does it have a timeout?
- Every error flag the peripheral can raise: is it handled and cleared?
- Read-modify-write on a write-1-to-clear register?
- Barriers after a clock enable, a `VTOR` write, or an interrupt disable?

**Failure paths:**
- What does this function do when the pool is empty, the queue is full, the device NACKs?
- Are error returns checked at the call site, or discarded?
- Is there a path where a resource is acquired and not released?

**Maintainability**, last because it is least likely to be a live defect:
- Magic numbers, duplicated constants, functions over ~50 lines, globals with no owner.

**How to deliver a review well**, which is also being assessed: separate *defects* from *preferences*, explain the consequence rather than the rule ("this will lose an interrupt under load" beats "avoid `|=` on status registers"), and lead with the one thing that will actually break.
</details>

---

### 2. The three most common review findings `INT`

<details><summary>Magic numbers, error codes, unclear ownership</summary>

**Magic numbers.** `if (status == 3)` tells a reader nothing and cannot be searched. The fix is not merely a `#define` — it is naming the *domain*:

```c
/* Bad */   if (mode == 2) { delay_ms(500); }
/* Good */  if (mode == MODE_CALIBRATING) { delay_ms(CAL_SETTLE_MS); }
```

The subtler version is a constant repeated in two places that must agree — a buffer size in one file and the loop bound in another. Derive one from the other (`sizeof buf`) rather than writing the number twice.

**Unclear error codes.** `int` returns where −1 means one thing in this function and something else in the next, and where the caller cannot tell a recoverable failure from a fatal one:

```c
/* Bad */   int sensor_read(int *out);        /* -1? -2? 0 on success or failure? */

/* Good */
typedef enum {
    SENSOR_OK = 0,
    SENSOR_ERR_TIMEOUT,       /* retryable */
    SENSOR_ERR_NACK,          /* device absent */
    SENSOR_ERR_RANGE,         /* data invalid, device fine */
} sensor_err_t;
```

The distinction that matters is **retryable versus permanent** — a caller that cannot tell them apart either retries forever on a permanent fault or gives up on a transient one. That is the same classification the backoff FSM in Part 11 entry 4 needs.

**Unclear buffer ownership.** The most consequential of the three:

```c
uint8_t *get_frame(void);       /* Who frees this? When may I reuse it? */
```

Three answers, and the API must pick one and document it: the caller supplies the buffer (`get_frame(uint8_t *dst, size_t cap)`); the callee owns it and the caller must copy before the next call; or it is reference-counted (Part 05 entry 14). Leaving it implicit is how you get a use-after-free that only appears under load.
</details>

---

### 3. HAL layering — the abstraction that pays for itself `SEN`

<details><summary>The pattern</summary>

```c
/* --- Interface: what the application depends on --- */
struct uart_ops {
    int  (*init)(void *ctx, uint32_t baud);
    int  (*write)(void *ctx, const uint8_t *d, size_t n);
    int  (*read)(void *ctx, uint8_t *d, size_t n);
};

struct uart_dev {
    const struct uart_ops *ops;
    void                  *ctx;        /* driver-private state */
};

static inline int uart_write(struct uart_dev *d, const uint8_t *b, size_t n)
{
    return d->ops->write(d->ctx, b, n);
}

/* --- One implementation per target --- */
static const struct uart_ops stm32_uart_ops = { stm32_init, stm32_write, stm32_read };
static const struct uart_ops mock_uart_ops  = { mock_init,  mock_write,  mock_read  };
```

**The `void *ctx` is what makes this usable**, and it is the same argument as the callback context in the general-C material: without it, the driver needs globals and cannot have two instances.

**What the layering actually buys**, in priority order — and the third is the one that justifies it:

1. **Portability.** Changing MCU means writing one new `uart_ops`, not editing the application.
2. **Multiple instances.** Two UARTs are two `uart_dev` values, not duplicated code.
3. **Testability.** `mock_uart_ops` lets the protocol parser and application logic run on a **host machine** under a real test framework, with no hardware. That is the difference between a codebase you can regression-test in CI and one where every change requires a board.

**What it costs**, stated honestly because an interviewer will push: one indirect call per operation (a few cycles, and it defeats inlining), plus the ops table in flash. For a bit-banged protocol in a tight loop that overhead is unacceptable and you call the register directly. For anything above the driver layer it is free in practice.

**Where the ST-style HAL goes wrong**, if asked: it abstracts the *registers* but not the *policy*, so it still leaks blocking calls and fixed buffer strategies into the application. A good HAL abstracts the operation ("send this frame"), not the peripheral.
</details>

---

### 4. Driver registration table `SEN`

<details><summary>The pattern</summary>

```c
struct sensor_drv {
    const char *name;
    uint8_t     i2c_addr;
    uint8_t     whoami_reg;
    uint8_t     whoami_val;
    int       (*init)(struct sensor *s);
    int       (*read)(struct sensor *s, int32_t *out);
};

/* Adding a sensor is ONE table row. No existing code changes. */
static const struct sensor_drv k_drivers[] = {
    { "BME280",  0x76u, 0xD0u, 0x60u, bme280_init,  bme280_read  },
    { "SHT31",   0x44u, 0x89u, 0x07u, sht31_init,   sht31_read   },
    { "MPU6050", 0x68u, 0x75u, 0x68u, mpu6050_init, mpu6050_read },
};

/* Probe: identify what is actually on the bus. */
const struct sensor_drv *sensor_probe(void)
{
    for (size_t i = 0u; i < (sizeof k_drivers / sizeof k_drivers[0]); i++) {
        uint8_t v;
        if (i2c_read_reg(k_drivers[i].i2c_addr, k_drivers[i].whoami_reg, &v) == 0 &&
            v == k_drivers[i].whoami_val) {
            return &k_drivers[i];
        }
    }
    return NULL;
}
```

**This is the same pattern as the FSM transition table, the AT command dispatch table and the CRC table** — data replacing control flow. Recognising it as one pattern across those four contexts is worth more than any individual instance.

The properties that make it the right structure:

- **`const`, so it lives in flash**, not RAM.
- **Adding a variant is a data change**, reviewable in isolation, with no risk of breaking existing entries.
- **Probing becomes possible.** A single firmware image supports whichever sensor the factory fitted, which is a real manufacturing requirement — a second-source part arriving mid-production is common, and a table-driven probe handles it without a new build.

The cost is one indirect call per operation and the table's flash. The alternative — a chain of `#ifdef`s per board variant — is what this replaces, and the argument against `#ifdef` is that only one configuration is ever compiled, so the others rot silently until someone builds them.
</details>

---

### 5. Observer / event bus `SEN`

<details><summary>The pattern, and when not to use it</summary>

```c
#define MAX_SUBS 8

typedef void (*ev_handler)(uint16_t event, const void *data, void *ctx);

struct sub { ev_handler fn; void *ctx; uint32_t mask; };

static struct sub g_subs[MAX_SUBS];
static uint8_t    g_n_subs;

bool bus_subscribe(ev_handler fn, void *ctx, uint32_t event_mask)
{
    if (g_n_subs >= MAX_SUBS || fn == NULL) return false;   /* bounded */
    g_subs[g_n_subs].fn   = fn;
    g_subs[g_n_subs].ctx  = ctx;
    g_subs[g_n_subs].mask = event_mask;
    g_n_subs++;
    return true;
}

/* Call from a TASK, never an ISR — handlers are arbitrary code. */
void bus_publish(uint16_t event, const void *data)
{
    for (uint8_t i = 0u; i < g_n_subs; i++) {
        if ((g_subs[i].mask & (1UL << (event & 31u))) != 0u) {
            g_subs[i].fn(event, data, g_subs[i].ctx);
        }
    }
}
```

Decouples producers from consumers: a button driver publishes `EV_BUTTON_PRESS` without knowing that the display, the logger and the power manager all care.

**The three caveats are the substance of the answer**, and offering them unprompted is what distinguishes someone who has used this from someone who has read about it:

- **Publish from a task, not an ISR.** Handlers are arbitrary code of unbounded duration, so publishing from an ISR runs all of them at interrupt priority. The correct shape is: ISR pushes the event to a ring buffer, a task drains it and publishes.
- **Control flow becomes invisible.** You cannot tell from the publish site what will run, which makes debugging and timing analysis materially harder. This is the real cost, and it is why a bus is wrong for a two-party relationship — a direct call is clearer.
- **Re-entrancy.** A handler that publishes another event recurses through `bus_publish`. Either forbid it, or queue events and drain in a loop.

**Bounded subscriber array, no allocation** — the same static-pool discipline as everywhere else in this document. And a subscriber that blocks stalls every other subscriber, so handlers must be short or the bus must dispatch asynchronously.
</details>

---

### 6. Making firmware testable off-target `SEN`

<details><summary>The seams that matter</summary>

The single highest-leverage practice, and a strong answer because most candidates have no story here.

**What to separate.** Split every module into logic that has no hardware dependency and a thin layer that does:

```c
/* Testable on a host: pure function of its inputs. */
size_t cobs_encode(const uint8_t *src, size_t len, uint8_t *dst);
void   parser_feed(struct parser *p, uint8_t b);
int32_t pid_update(struct pid *p, int32_t sp, int32_t pv);

/* Not testable on a host: touches registers. Keep it thin. */
void uart_write_byte(uint8_t b);
```

Everything in Parts 01, 03, 05, 06, 10, 11 and 12 of this document is host-testable as written. That is not a coincidence — it is what a testable design looks like.

**The four seams, in increasing invasiveness:**

| Seam | Mechanism |
|---|---|
| Pure functions | no seam needed — call them from a host test |
| Injected ops table | `struct uart_ops` with a mock implementation (entry 3) |
| Injected time | `uint32_t (*now_ms)(void)` instead of calling `g_ticks` directly |
| Linker substitution | provide an alternative `uart_write_byte` in the test build |

**Injecting time is the one people miss**, and it unlocks the most. A timeout, a debounce, a backoff and a timer list are all untestable if they read a global tick — you would have to wait in real time. Passing time in lets a test advance the clock by an hour instantly and assert the behaviour at every boundary.

**What this gets you:** the protocol parser's malformed-length case, the ring buffer's wrap, the CRC vectors, the PID's anti-windup and the backoff's jitter can all be tested in CI on every commit, in milliseconds, with a coverage report. The hardware-dependent remainder is small enough to test on a board deliberately.

Also worth naming: **fuzzing the parsers on a host** (Part 10 entry 7) is the practical route to finding the bugs that matter, and it is only possible because they are pure functions.
</details>

---

### 7. Reducing RAM `INT`

<details><summary>Techniques, in order of return</summary>

**1. Reorder struct members, largest first.** Free, and often the biggest single win:

```c
struct bad  { uint8_t a; uint32_t b; uint8_t c; uint32_t d; };   /* 16 bytes */
struct good { uint32_t b; uint32_t d; uint8_t a; uint8_t c; };   /* 12 bytes */
```

Alignment padding is invisible until you look. On an array of 1000 records that is 4 KB. Print `sizeof` for your hot structs — the number is often a surprise. (And per Part 13 entry 4: reorder for RAM, but **never** rely on layout for a wire format.)

**2. Bit-fields or a bitmask for flags.** Eight `bool` members cost 8 bytes; eight bits cost 1. Worth it for arrays; note that bit-field access is a read-modify-write and therefore **not atomic**, so flags shared with an ISR need care.

**3. `const` everything that never changes.** A `const` array lives in flash; the same array without `const` is copied into RAM at startup and occupies both. This is frequently the largest easy win in a codebase that grew organically — lookup tables, string tables, configuration defaults.

**4. Union mutually exclusive state.** If a device is either in calibration mode or streaming mode, their working buffers can share memory. Requires discipline about which member is live — a tag field, and the discipline that reading the wrong member is a bug (Part 13 entry 11).

**5. Size stacks from measurement.** Painting and high-water marks (Part 08 entry 9). Eight tasks over-provisioned by 512 bytes each is 4 KB recovered.

**6. Eliminate duplicate buffers.** A frame copied from a DMA buffer to a parse buffer to an application buffer occupies three times what it needs. Parse in place where possible.

**How to know where to start:** read the `.map` file and the `size` output. `arm-none-eabi-size -A` gives `.data` and `.bss` per object; the map file gives per-symbol. **Measure before optimising** — the intuition about which structure is largest is usually wrong.
</details>

---

### 8. Reducing flash and cycles `INT`→`SEN`

<details><summary>Measure first, then these</summary>

**The rule that comes before any technique: measure.** `DWT->CYCCNT` for cycles (Part 07 entry 12), a GPIO toggle plus a logic analyser for timing in context, the map file for size. Optimising the wrong function is the default outcome of guessing.

**Flash**, in order of return:

- **Replace `printf`.** The formatting engine is typically 2–8 KB and has a large stack frame. Substituting the integer and hex converters from Part 03 entry 13 is often the single biggest saving available.
- **Avoid software floating point.** One `float` division pulls in the whole soft-float library. Fixed point (Part 12 entry 1) removes it entirely.
- **`-ffunction-sections -fdata-sections` plus `--gc-sections`**, so unreferenced functions are discarded rather than linked.
- **`-Os`**, and compare — occasionally `-O2` is both smaller and faster.
- **Deduplicate string literals** and move them to `const` in flash.
- **Compile out debug code**, but be aware that `#ifdef DEBUG` changes timing, so the build you tested is not the build you ship (Part 13 entry 8).

**Cycles:**

- **Divide by a power of two → shift.** Only valid for **unsigned** operands: for a signed negative value, `>> 3` and `/ 8` round in opposite directions.
- **Multiply by a reciprocal** in fixed point where the divisor is a runtime value and the core has no divider.
- **Lookup tables** for transcendental functions (Part 12 entry 7) — flash for cycles, and deterministic.
- **Word-at-a-time** instead of byte-at-a-time in `memcpy`, CRC and string loops (Part 03 entry 2).
- **Move work out of ISRs** (Part 07 entry 5). This improves *latency*, which is usually the real requirement rather than throughput.
- **DMA instead of CPU copies** for anything bulk.

**Power**, which is often the actual constraint and gets forgotten:

- `WFI` in idle rather than a spin loop; **tickless idle** removes the 1 kHz wake (Part 08 entry 10).
- Batch transmissions — a radio waking once for ten messages costs far less than ten wakes.
- Reduce sample rates; gate peripheral clocks when idle. An enabled-but-unused peripheral draws current.

**What to say about all of it:** the largest wins are architectural, not micro-optimisations. Circular DMA with idle-line detection instead of per-byte interrupts (Part 09 entry 4) beats any amount of tuning the ISR — one interrupt per message instead of 11,500 per second.
</details>

---

## Part 14 — retention table

| Area | The point |
|---|---|
| Review priority | concurrency → bounds → hardware discipline → failure paths → style |
| Error codes | distinguish **retryable from permanent**, or callers cannot behave correctly |
| Buffer ownership | caller-supplied, callee-owned, or refcounted — pick one and document it |
| HAL | abstract the *operation*, not the peripheral; `void *ctx` enables instances |
| Table-driven design | FSM, dispatch, driver registry, CRC — **one pattern**, `const`, in flash |
| Event bus | publish from a task; control flow becomes invisible — that is the cost |
| Testability | pure logic + thin hardware layer; **inject time** to test timeouts |
| RAM | reorder structs, `const` to flash, measure stacks, read the map file |
| Flash | `printf` and soft-float are usually the two biggest items |
| Cycles | shift only for **unsigned**; move work out of ISRs; DMA for bulk |
| Above all | **measure first** — and the biggest wins are architectural |

---

# Finishing

**150 problems, curated from 773, every one solved.**

| Part | Kept | From |
|---|---|---|
| 01 Bit Manipulation | 20 | 70 |
| 02 Register I/O | 12 | 30 |
| 03 Memory & String | 14 | 45 |
| 04 Allocators | 8 | 35 |
| 05 Data Structures | 20 | 75 |
| 06 CRC & Checksums | 7 | 35 |
| 07 Interrupts & Concurrency | 12 | 30 |
| 08 RTOS Primitives | 10 | 50 |
| 09 Drivers | 14 | 83 |
| 10 Protocol Parsers | 9 | 50 |
| 11 State Machines | 6 | 40 |
| 12 Embedded Algorithms | 10 | 45 |
| 13 Debugging | 10 | 26 |
| 14 Review, Patterns, Optimization | 8 | 59 |

## The ten things this document keeps saying

If nothing else survives a revision pass, these do — each appears in four or more parts.

1. **`volatile` ≠ atomic ≠ barrier.** Three separate guarantees, three separate bugs.
2. **Validate a length before you index with it.** Wire data, config data, any length you did not compute.
3. **Write capacity tests as subtraction.** `off + n > len` overflows and passes; `n > len - off` does not.
4. **Every hardware wait needs a timeout.** `while (!(REG & FLAG));` is how products hang in the field.
5. **Handle every error flag the peripheral can raise**, not just the one you want. An uncleared `ORE` kills the link permanently.
6. **One writer per shared variable.** The whole basis of lock-free design; the reason a `count` field breaks a ring buffer.
7. **Unsigned subtraction is wrap-safe.** `(now - then) < timeout`, never `now < then + timeout`.
8. **Mask the value, not just the field.** An oversized argument must not corrupt its neighbour.
9. **Bound the worst case and prove it.** Static pools over `malloc`, O(1) schedulers, no recursion on a small stack.
10. **Measure before optimising, and before believing.** Cycle counters, high-water marks, the map file.

## How to use it

**Revision:** read only the retention tables — one per part, fourteen tables. Anything you cannot reconstruct is the entry to reopen.

**Practice:** work from the prototype with the solution collapsed. Compile with `-Wall -Wextra -Wpedantic -Wconversion -fsanitize=address,undefined`. Redo the ones you got wrong three days later.

**Interview morning:** the fourteen retention tables and the ten items above. That is the whole document at a scannable length.

## The gap this document cannot close

Everything here is written knowledge, and it will get you through the technical screen. What it will not do is answer *"tell me about a bug you fixed"* — and that is the question that decides senior offers.

Part 13 gives you the vocabulary for ten root causes. It cannot give you the story of having chased one: the symptom you saw, the measurement that ruled things out, the moment the cause became obvious. That comes from a logic analyser trace of a bus you actually broke, a hardfault you actually decoded, an `ORE` flag you actually found set.

So the highest-value next step is not more of this. It is: flash entry 09-3 on real hardware, capture a UART frame, deliberately overrun it, and watch reception die. That one afternoon converts fifty entries of this document from things you have read into things you have seen.

---

*Sources: two problem banks totalling 773 exercises, curated to 150. Reference platform STM32 / Cortex-M, C11.*
