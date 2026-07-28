# Embedded C — 1500 Exercises, Easiest to Hardest

> Questions only. Five tiers of 300. Each tier covers every topic at one difficulty level.

**Why tiers rather than topics.** Grouped by subject, you would spend your first month on bit manipulation and still not know what a ring buffer is. Each tier here touches every area, so after Tier 1 you have met the whole landscape — then Tier 2 revisits all of it one level deeper.

| Tier | Range | Level | What it feels like |
|---|---|---|---|
| [1](#tier-1--foundations) | 1–300 | Foundations | One function, one loop, no data structures |
| [2](#tier-2--core-competence) | 301–600 | Core competence | Two-pointer, recursion, first structures, first registers |
| [3](#tier-3--applied) | 601–900 | Applied | Real structures, filters, drivers, parsers |
| [4](#tier-4--systems) | 901–1200 | Systems | Concurrency, allocators, protocol stacks |
| [5](#tier-5--senior) | 1201–1500 | Senior | Lock-free, full subsystems, design judgement |

---

## How to use this

**Read before you write.** With limited exposure, reading fifty exercise statements teaches you what problems *exist* — worth doing before solving any of them. Skim a whole section, then come back and implement.

**Every exercise states an exact contract**, so you can check yourself with no solution:

```c
/* Ex 72 — uint8_t popcount_kernighan(uint32_t x) — number of 1-bits. */
assert(popcount_kernighan(0u) == 0u);
assert(popcount_kernighan(0xFFFFFFFFu) == 32u);
assert(popcount_kernighan(0x80000001u) == 2u);
```

If you cannot write the asserts, you have not understood the contract. That is a signal to go read, not to guess.

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -Wconversion \
    -fsanitize=address,undefined drill.c -o drill && ./drill
```

**Assume in scope:** `<stdint.h>`, `<stdbool.h>`, `<stddef.h>`, `<string.h>`, `<assert.h>`. No `malloc` unless stated. Little-endian 32-bit target.

**Marking.** `✓` first try → retire 30 days · `~` slow or hinted → redo in 3 days · `✗` failed → redo tomorrow.

**Pace.** Do not aim to finish. Aim for three passes over the ones you keep failing. Eight to ten a day is a real pace; Tier 1 is about five weeks at that rate.

---

# Tier 1 — Foundations

**Exercises 1–300.** One function, one or two loops, no data structures. Nothing here needs a datasheet. If you have never practised coding problems, everything in this tier is learnable in one sitting each.

## 1.1 First functions and integer basics

1. `int add(int a, int b)` — return the sum.
2. `int square(int x)` — return x².
3. `bool is_even(int32_t x)` — true iff x is even; correct for negatives.
4. `bool is_odd(int32_t x)` — true iff x is odd.
5. `int32_t abs_val(int32_t x)` — absolute value; note what happens at `INT32_MIN`.
6. `int32_t max2(int32_t a, int32_t b)` — larger of two.
7. `int32_t min2(int32_t a, int32_t b)` — smaller of two.
8. `int32_t max3(int32_t a, int32_t b, int32_t c)` — largest of three.
9. `int32_t sign(int32_t x)` — −1, 0, or +1.
10. `bool in_range(int32_t v, int32_t lo, int32_t hi)` — inclusive range test.
11. `int32_t clamp(int32_t v, int32_t lo, int32_t hi)` — clamp to a range.
12. `void swap_temp(int32_t *a, int32_t *b)` — swap using a temporary.
13. `bool is_leap(uint16_t y)` — leap year; verify 1900 and 2000.
14. `uint8_t days_in_month(uint8_t m, uint16_t y)` — 1..12; 0 if m is invalid.
15. `int32_t celsius_to_f_x10(int32_t c_x10)` — °C×10 to °F×10, integer only.
16. `int32_t f_to_celsius_x10(int32_t f_x10)` — the inverse.
17. `uint32_t seconds_to_hms(uint32_t s, uint8_t *h, uint8_t *m, uint8_t *sec)` — split; return whole days.
18. `uint32_t hms_to_seconds(uint8_t h, uint8_t m, uint8_t s)` — combine.
19. `int32_t percent_of(int32_t value, int32_t pct)` — pct% of value, integer, rounded.
20. `uint32_t div_round(uint32_t a, uint32_t b)` — a/b rounded to nearest; 0 if b == 0.

## 1.2 Loops and accumulation

21. `uint32_t sum_to_n(uint32_t n)` — 1+2+…+n with a loop.
22. `uint32_t sum_to_n_formula(uint32_t n)` — the same, O(1); cast before multiplying.
23. `uint32_t sum_squares(uint32_t n)` — 1²+2²+…+n².
24. `uint32_t sum_evens(uint32_t n)` — even numbers up to n.
25. `uint64_t factorial(uint8_t n)` — n!; state the largest n that fits.
26. `uint32_t power_int(uint32_t base, uint8_t exp)` — repeated multiplication.
27. `uint32_t multiply_by_adding(uint32_t a, uint32_t b)` — product using addition only.
28. `uint32_t divide_by_subtracting(uint32_t a, uint32_t b, uint32_t *rem)` — quotient and remainder, no `/`.
29. `uint32_t gcd_subtract(uint32_t a, uint32_t b)` — GCD by repeated subtraction.
30. `uint32_t gcd_mod(uint32_t a, uint32_t b)` — GCD by Euclid's remainder method.
31. `uint32_t lcm(uint32_t a, uint32_t b)` — divide before multiplying to avoid overflow.
32. `void print_fizzbuzz(uint32_t n)` — 1..n with Fizz / Buzz / FizzBuzz.
33. `uint32_t count_multiples(uint32_t n, uint32_t k)` — multiples of k up to n.
34. `bool is_perfect_square(uint32_t n)` — no floating point.
35. `uint32_t triangular(uint32_t n)` — the n-th triangular number.
36. `uint32_t collatz_steps(uint32_t n)` — steps to reach 1.
37. `uint32_t collatz_max(uint32_t n)` — largest value reached on the way.
38. `void fib_print(uint8_t n)` — print the first n Fibonacci numbers.
39. `uint64_t fib_nth(uint8_t n)` — n-th Fibonacci, iterative, two variables.
40. `uint32_t sum_range(uint32_t lo, uint32_t hi)` — inclusive sum; handle lo > hi.

## 1.3 Digits and number properties

41. `uint8_t count_digits(uint32_t n)` — decimal digit count; 1 for 0.
42. `uint32_t sum_digits(uint32_t n)` — sum of decimal digits.
43. `uint32_t reverse_digits(uint32_t n)` — 12345 → 54321.
44. `bool is_palindrome_num(uint32_t n)` — same read either way; reverse a copy.
45. `uint8_t nth_digit(uint32_t n, uint8_t i)` — i-th digit from the right.
46. `uint8_t first_digit(uint32_t n)` — leading decimal digit.
47. `uint8_t digital_root(uint32_t n)` — repeated digit-sum to one digit.
48. `bool is_prime(uint32_t n)` — trial division to √n; handle 0, 1, 2 correctly.
49. `uint32_t next_prime(uint32_t n)` — smallest prime greater than n.
50. `uint32_t nth_prime(uint16_t k)` — the k-th prime, 1-indexed.
51. `void print_factors(uint32_t n)` — all divisors ascending.
52. `uint8_t count_factors(uint32_t n)` — number of divisors.
53. `void prime_factorise(uint32_t n)` — prime factors with multiplicity; do not lose the last one.
54. `bool is_perfect(uint32_t n)` — proper divisors sum to n (6, 28, 496).
55. `bool is_abundant(uint32_t n)` — proper divisors sum to more than n.
56. `bool is_armstrong(uint32_t n)` — digits raised to the digit count sum to n; integer power only.
57. `bool is_strong(uint32_t n)` — digit factorials sum to n (145); use a 10-entry table.
58. `bool is_automorphic(uint32_t n)` — n² ends with n (5, 25, 76).
59. `uint32_t sum_primes_below(uint32_t n)` — sum of all primes < n.
60. `bool are_coprime(uint32_t a, uint32_t b)` — GCD is 1.

## 1.4 Bases and bit basics

61. `void print_binary(uint32_t v, uint8_t bits)` — print `bits` digits, MSB first.
62. `void print_hex(uint32_t v)` — eight uppercase hex digits.
63. `uint32_t binstr_to_u32(const char *s)` — parse a binary string; stop at the first invalid char.
64. `uint32_t hexstr_to_u32(const char *s)` — parse hex, either case.
65. `void u32_to_base(uint32_t v, uint8_t base, char *out)` — base 2..16; digits emerge reversed.
66. `uint32_t set_bit(uint32_t x, uint8_t n)` — set bit n.
67. `uint32_t clr_bit(uint32_t x, uint8_t n)` — clear bit n.
68. `uint32_t tgl_bit(uint32_t x, uint8_t n)` — flip bit n.
69. `bool tst_bit(uint32_t x, uint8_t n)` — is bit n set; return a proper `bool`.
70. `uint32_t put_bit(uint32_t x, uint8_t n, bool v)` — force bit n to v.
71. `uint8_t popcount_loop(uint32_t x)` — one shift per bit.
72. `uint8_t popcount_kernighan(uint32_t x)` — one iteration per set bit.
73. `uint8_t count_zero_bits(uint32_t x)` — count 0-bits.
74. `bool is_pow2(uint32_t x)` — exactly one bit set, and x != 0.
75. `uint8_t swap_nibbles(uint8_t x)` — 0xAB → 0xBA.
76. `uint16_t bswap16(uint16_t x)` — swap the two bytes; mind the promotion.
77. `uint32_t bswap32(uint32_t x)` — reverse all four bytes.
78. `uint8_t rev8_loop(uint8_t x)` — reverse bits in a byte, one at a time.
79. `bool parity8_loop(uint8_t x)` — odd parity, counted with a loop.
80. `uint32_t mask_low(uint8_t n)` — n lowest bits set; correct for n == 32.
81. `uint32_t clear_lsb(uint32_t x)` — clear the lowest set bit.
82. `uint32_t isolate_lsb(uint32_t x)` — keep only the lowest set bit.
83. `int8_t lsb_pos(uint32_t x)` — index of the lowest set bit; −1 if none.
84. `int8_t msb_pos(uint32_t x)` — index of the highest set bit; −1 if none.
85. `uint8_t nibble_at(uint32_t x, uint8_t i)` — nibble i (0..7), right-aligned.
86. `uint8_t byte_at(uint32_t x, uint8_t i)` — byte i (0..3), 0 = least significant.
87. `uint32_t pack_be32(uint8_t a, uint8_t b, uint8_t c, uint8_t d)` — a is the most significant.
88. `void unpack_be32(uint32_t v, uint8_t out[4])` — split into four bytes, big-endian.
89. `uint8_t bcd_to_bin(uint8_t bcd)` — packed BCD to binary; 0xFF if a nibble exceeds 9.
90. `uint8_t bin_to_bcd(uint8_t v)` — 0..99 to packed BCD; 0xFF if out of range.

## 1.5 Array basics

91. `int32_t arr_sum(const int32_t *a, size_t n)` — sum of elements.
92. `int32_t arr_max(const int32_t *a, size_t n)` — largest; seed from a[0], not from 0.
93. `int32_t arr_min(const int32_t *a, size_t n)` — smallest.
94. `int32_t arr_avg(const int32_t *a, size_t n)` — mean, integer, rounded.
95. `size_t arr_count_eq(const int32_t *a, size_t n, int32_t v)` — occurrences of v.
96. `int arr_find_first(const int32_t *a, size_t n, int32_t v)` — index or −1.
97. `int arr_find_last(const int32_t *a, size_t n, int32_t v)` — last index or −1.
98. `bool arr_contains(const int32_t *a, size_t n, int32_t v)` — presence test.
99. `void arr_fill(int32_t *a, size_t n, int32_t v)` — set every element.
100. `void arr_iota(int32_t *a, size_t n, int32_t start)` — consecutive values.
101. `void arr_copy(int32_t *dst, const int32_t *src, size_t n)` — element-wise copy.
102. `void arr_reverse(int32_t *a, size_t n)` — in place, two indices; handle n == 0.
103. `void arr_scale(int32_t *a, size_t n, int32_t k)` — multiply each element by k.
104. `void arr_add(int32_t *dst, const int32_t *src, size_t n)` — element-wise add.
105. `size_t arr_count_positive(const int32_t *a, size_t n)` — elements greater than 0.
106. `size_t arr_count_even(const int32_t *a, size_t n)` — even elements.
107. `int32_t arr_sum_even_indices(const int32_t *a, size_t n)` — a[0] + a[2] + …
108. `bool arr_is_sorted(const int32_t *a, size_t n)` — non-decreasing.
109. `bool arr_is_sorted_desc(const int32_t *a, size_t n)` — non-increasing.
110. `bool arr_all_equal(const int32_t *a, size_t n)` — every element identical.
111. `int32_t arr_second_max(const int32_t *a, size_t n)` — second largest distinct value.
112. `size_t arr_index_of_max(const int32_t *a, size_t n)` — index of the largest.
113. `void arr_swap_ends(int32_t *a, size_t n)` — exchange first and last.
114. `void arr_rotate_one_left(int32_t *a, size_t n)` — shift left by one, wrapping.
115. `void arr_rotate_one_right(int32_t *a, size_t n)` — shift right by one, wrapping.
116. `int32_t arr_range(const int32_t *a, size_t n)` — max minus min.
117. `size_t arr_remove_value(int32_t *a, size_t n, int32_t v)` — remove all v; return the new length.
118. `size_t arr_dedup_sorted(int32_t *a, size_t n)` — drop adjacent duplicates; return the new length.
119. `void arr_abs_all(int32_t *a, size_t n)` — replace each with its absolute value.
120. `int32_t arr_dot(const int32_t *a, const int32_t *b, size_t n)` — dot product; accumulate wider.

## 1.6 String basics

121. `size_t str_len(const char *s)` — length, excluding the terminator.
122. `void str_copy(char *dst, const char *src)` — copy including the terminator.
123. `void str_concat(char *dst, const char *src)` — append to dst.
124. `int str_compare(const char *a, const char *b)` — negative, zero, or positive.
125. `bool str_equal(const char *a, const char *b)` — exact match.
126. `void str_reverse(char *s)` — in place; note why passing a literal crashes.
127. `bool str_is_palindrome(const char *s)` — exact, case-sensitive.
128. `size_t str_count_char(const char *s, char c)` — occurrences of c.
129. `int str_find_char(const char *s, char c)` — index or −1.
130. `int str_find_last_char(const char *s, char c)` — last index or −1.
131. `size_t str_count_vowels(const char *s)` — a, e, i, o, u, either case.
132. `size_t str_count_words(const char *s)` — count transitions into a word, not spaces.
133. `void str_to_upper(char *s)` — uppercase in place.
134. `void str_to_lower(char *s)` — lowercase in place.
135. `void str_toggle_case(char *s)` — swap the case of each letter.
136. `bool char_is_digit(char c)` — without `<ctype.h>`.
137. `bool char_is_alpha(char c)` — without `<ctype.h>`.
138. `bool char_is_space(char c)` — space, tab, newline, carriage return.
139. `uint8_t char_to_digit(char c)` — '0'..'9' → 0..9; 0xFF otherwise.
140. `char digit_to_char(uint8_t d)` — 0..15 → '0'..'F'; '?' if out of range.
141. `void str_remove_spaces(char *s)` — compact in place and re-terminate.
142. `void str_trim_trailing(char *s)` — remove trailing whitespace.
143. `void str_trim_leading(char *s)` — remove leading whitespace, shifting down.
144. `bool str_starts_with(const char *s, const char *prefix)` — prefix test.
145. `bool str_ends_with(const char *s, const char *suffix)` — suffix test.
146. `int32_t str_to_int(const char *s)` — decimal with an optional sign.
147. `void int_to_str(int32_t v, char *out)` — decimal with a minus sign when needed.
148. `void u32_to_str(uint32_t v, char *out)` — unsigned decimal; digits emerge reversed.
149. `size_t str_replace_char(char *s, char from, char to)` — replace all; return the count.
150. `bool str_all_digits(const char *s)` — true iff non-empty and every character is a digit.

## 1.7 Pointer basics

151. `void ptr_swap(int32_t *a, int32_t *b)` — swap through pointers.
152. `void ptr_set(int32_t *p, int32_t v)` — write through a pointer; no-op if NULL.
153. `int32_t ptr_get(const int32_t *p, int32_t def)` — read, or return def if NULL.
154. `void increment_both(int32_t *a, int32_t *b)` — increment both pointees.
155. `void divmod(int32_t a, int32_t b, int32_t *q, int32_t *r)` — two outputs via pointers.
156. `void min_max_out(const int32_t *a, size_t n, int32_t *mn, int32_t *mx)` — one pass, both outputs.
157. `int32_t sum_ptr_walk(const int32_t *a, size_t n)` — sum using `*p++`, no indexing.
158. `int32_t sum_begin_end(const int32_t *begin, const int32_t *end)` — half-open range.
159. `void copy_ptr_walk(int32_t *d, const int32_t *s, size_t n)` — pointers only.
160. `void reverse_ptr(int32_t *a, size_t n)` — two pointers moving inward.
161. `const int32_t *find_ptr(const int32_t *a, size_t n, int32_t v)` — pointer to match, else NULL.
162. `ptrdiff_t ptr_gap(const int32_t *a, const int32_t *b)` — element distance, not bytes.
163. `void print_addresses(const int32_t *a, size_t n)` — print each address; cast for `%p`.
164. `void sizeof_demo(void)` — print `sizeof` for char, short, int, long, pointer; use `%zu`.
165. `void array_decay_demo(int32_t a[10])` — print `sizeof a` inside the function; explain it.
166. `void str_upper_ptr(char *s)` — uppercase using a walking pointer.
167. `size_t strlen_ptr(const char *s)` — length by pointer subtraction.
168. `void fill_ptr(uint8_t *p, size_t n, uint8_t v)` — byte fill with pointers.
169. `bool bytes_equal(const uint8_t *a, const uint8_t *b, size_t n)` — compare two regions.
170. `void two_dim_flat(int32_t *flat, size_t rows, size_t cols)` — fill a flattened 2D array with r×c.

## 1.8 Simple 2D arrays and matrices

171. `void mat_fill(int32_t m[4][4], int32_t v)` — set every cell.
172. `void mat_identity(int32_t m[4][4])` — ones on the diagonal.
173. `int32_t mat_sum(const int32_t m[4][4])` — sum of all cells.
174. `int32_t mat_diag_sum(const int32_t m[4][4])` — main diagonal.
175. `int32_t mat_anti_diag_sum(const int32_t m[4][4])` — the other diagonal.
176. `void mat_add(const int32_t a[4][4], const int32_t b[4][4], int32_t out[4][4])` — element-wise.
177. `void mat_transpose(const int32_t a[4][4], int32_t out[4][4])` — swap indices.
178. `void mat_transpose_inplace(int32_t a[4][4])` — in place; square only.
179. `void mat_row_sums(const int32_t m[4][4], int32_t out[4])` — one sum per row.
180. `void mat_col_sums(const int32_t m[4][4], int32_t out[4])` — one sum per column.
181. `bool mat_is_symmetric(const int32_t m[4][4])` — equal to its transpose.
182. `int32_t mat_max(const int32_t m[4][4], size_t *r, size_t *c)` — value and position.
183. `void mat_multiply(const int32_t a[4][4], const int32_t b[4][4], int32_t out[4][4])` — accumulate wider.
184. `void mat_rotate90(const int32_t a[4][4], int32_t out[4][4])` — clockwise.
185. `void mat_print_spiral(const int32_t m[4][4])` — cells in spiral order.

## 1.9 Pattern printing

186. `void tri_left(uint8_t n)` — row i has i stars.
187. `void tri_inverted(uint8_t n)` — the same, counted down.
188. `void tri_right(uint8_t n)` — spaces then stars.
189. `void pyramid(uint8_t n)` — row i: n−i spaces, 2i−1 stars.
190. `void diamond(uint8_t n)` — pyramid plus reflection; the middle row appears once.
191. `void hollow_square(uint8_t n)` — border only.
192. `void hollow_pyramid(uint8_t n)` — outline only.
193. `void floyd_triangle(uint8_t n)` — consecutive numbers; one counter outside both loops.
194. `void pascal_triangle(uint8_t n)` — use the binomial recurrence, no 2D array.
195. `void number_pyramid(uint8_t n)` — 1, 121, 12321, …
196. `void alphabet_triangle(uint8_t n)` — A, AB, ABC, …
197. `void butterfly(uint8_t n)` — stars, gap, stars, mirrored.
198. `void checkerboard(uint8_t n)` — alternating characters.
199. `void multiplication_table(uint8_t n)` — aligned grid using a fixed field width.
200. `void bar_chart(const uint8_t *v, size_t n)` — one row per value.

## 1.10 Simple search and sort

201. `int linear_search(const int32_t *a, size_t n, int32_t key)` — index or −1.
202. `int binary_search(const int32_t *a, size_t n, int32_t key)` — sorted input, half-open interval.
203. `size_t lower_bound(const int32_t *a, size_t n, int32_t key)` — first index not less than key.
204. `size_t upper_bound(const int32_t *a, size_t n, int32_t key)` — first index greater than key.
205. `void bubble_sort(int32_t *a, size_t n)` — with an early-exit flag.
206. `void selection_sort(int32_t *a, size_t n)` — minimum number of swaps.
207. `void insertion_sort(int32_t *a, size_t n)` — shift, do not swap.
208. `void sort_3(int32_t *a)` — sort exactly three elements.
209. `void counting_sort_u8(uint8_t *a, size_t n)` — 256 buckets, then rewrite.
210. `void sort_evens_first(int32_t *a, size_t n)` — evens before odds.
211. `void sort_binary(uint8_t *a, size_t n)` — array of 0s and 1s, one pass.
212. `void move_zeros_end(int32_t *a, size_t n)` — zeros last, others keep their order.
213. `size_t merge_sorted(const int32_t *a, size_t na, const int32_t *b, size_t nb, int32_t *out)` — merge two.
214. `bool is_permutation(const int32_t *a, const int32_t *b, size_t n)` — same multiset.
215. `bool arrays_equal(const int32_t *a, const int32_t *b, size_t n)` — element-wise equality.

## 1.11 Simple structs

216. Define `struct point { int32_t x, y; }` and write `point_add`.
217. `int32_t point_dist_sq(struct point a, struct point b)` — squared distance, no `sqrt`.
218. `bool point_equal(struct point a, struct point b)` — field comparison.
219. Define `struct rect { struct point tl, br; }` and write `rect_area`.
220. `bool rect_contains(struct rect r, struct point p)` — inclusive containment.
221. `bool rect_overlap(struct rect a, struct rect b)` — do two rectangles intersect.
222. Define `struct student { char name[16]; uint8_t roll; uint8_t marks; }` and write `student_print`.
223. `uint8_t class_average(const struct student *s, size_t n)` — mean of marks.
224. `const struct student *top_student(const struct student *s, size_t n)` — highest marks.
225. `void sort_students_by_marks(struct student *s, size_t n)` — insertion sort, descending.
226. Write `f(struct student s)` and `g(const struct student *s)`; measure and compare the cost.
227. `void sizeof_struct_demo(void)` — print `sizeof` and every `offsetof`; explain the padding.
228. Reorder that struct for minimum size; verify the saving.
229. Define a union of `uint32_t` and `uint8_t[4]`; write `uint8_t low_byte(uint32_t v)`.
230. `bool detect_endian(void)` — true if little-endian, using a union.

## 1.12 C semantics and output prediction

231. Predict `int i = 5; printf("%d", i++ + ++i);` — state the answer and why there is not one.
232. Predict `printf("%d", sizeof(int));` — what is wrong with the specifier.
233. Predict `char c = 200; printf("%d", c);` — with signed `char`.
234. Predict `printf("%d %d", -5 % 2, 5 % -2);`
235. Predict `printf("%d", -7 / 2);` — which way does it round.
236. Predict `if (-1 < sizeof(int)) puts("yes");` — does it print.
237. Predict `printf("%d", 1 << 31);` on 32-bit `int` — name the problem.
238. Predict `uint8_t a = 200, b = 100; printf("%d", a + b);` — explain promotion.
239. Predict `float f = 0.1f + 0.2f; printf("%d", f == 0.3f);`
240. Predict `printf("%zu", strlen("hi") - 3);` — explain the value.
241. Predict `for (size_t i = 5; i >= 0; i--)` — how many iterations.
242. Predict `int a[3] = {1}; printf("%d %d", a[1], a[2]);`
243. Predict `printf("%c", 2["abcdef"]);` — explain why it compiles.
244. Predict `x = x++;` — name the rule it violates.
245. Predict a function with `static int n = 0; return ++n;` called three times.
246. Predict `char *s = "hi"; s[0] = 'H';` — what happens and why.
247. Predict `printf("%s", NULL);` — and say what you would write instead.
248. Predict `int *p; printf("%d", *p);` — name both problems.
249. Predict `#define SQ(x) x*x` with `SQ(2+3)` — then fix the macro.
250. Predict `#define MAX(a,b) ((a)>(b)?(a):(b))` with `MAX(i++, j)` — name the defect.
251. Predict `sizeof(char)` and `sizeof('a')` in C — and why they differ.
252. Predict `if (0.1 + 0.2 == 0.3)` — then write the correct comparison.
253. Predict `int a = 010; printf("%d", a);` — explain the literal.
254. Predict `while (n--)` versus `while (--n)` for n = 3 — iteration counts.
255. Predict `if (strcmp(a, b))` when a and b are equal — what does the branch mean.

## 1.13 First hardware exercises

256. `uint32_t reg_read(uintptr_t addr)` — read a 32-bit register with the correct `volatile` cast.
257. `void reg_write(uintptr_t addr, uint32_t v)` — write one.
258. `void reg_set_bits(volatile uint32_t *r, uint32_t mask)` — read-modify-write, set.
259. `void reg_clear_bits(volatile uint32_t *r, uint32_t mask)` — read-modify-write, clear.
260. `bool reg_test_bits(volatile uint32_t *r, uint32_t mask)` — any mask bit set.
261. Define a peripheral struct of three `volatile uint32_t` members plus a base-address macro.
262. Write `BIT(n)` and the four macros `SET_BIT`, `CLR_BIT`, `TGL_BIT`, `TST_BIT`.
263. `void gpio_set_output(volatile uint32_t *moder, uint8_t pin)` — write mode bits `01`.
264. `void gpio_write_pin(volatile uint32_t *bsrr, uint8_t pin, bool level)` — set/reset halves.
265. `bool gpio_read_pin(volatile uint32_t *idr, uint8_t pin)` — read the input register, not the output.
266. `void led_blink_count(uint8_t n)` — toggle a pin n times with a delay between.
267. `void busy_delay(uint32_t loops)` — a delay loop the optimiser will not remove.
268. `uint32_t ticks_elapsed(uint32_t now, uint32_t then)` — wrap-safe difference.
269. `bool deadline_passed(uint32_t now, uint32_t deadline)` — wrap-safe comparison.
270. `void w1c_clear(volatile uint32_t *sr, uint32_t mask)` — clear write-1-to-clear flags correctly.
271. Explain in two sentences why `*sr |= mask` is wrong on a W1C register.
272. `uint32_t baud_divisor(uint32_t pclk, uint32_t baud)` — rounded, not truncated.
273. `uint32_t baud_error_ppm(uint32_t pclk, uint32_t div, uint32_t baud)` — the resulting error.
274. `uint16_t pwm_ccr_from_percent(uint16_t arr, uint8_t pct)` — duty register value, rounded.
275. `uint32_t adc_to_mv(uint16_t raw, uint32_t vref_mv)` — 12-bit count to millivolts, rounded.

## 1.14 Mixed foundations review

276. `uint32_t count_bits_in_buffer(const uint8_t *a, size_t n)` — total 1-bits.
277. `uint8_t xor_checksum(const uint8_t *a, size_t n)` — XOR of every byte.
278. `uint8_t sum_checksum(const uint8_t *a, size_t n)` — 8-bit wrapping sum.
279. `bool verify_checksum(const uint8_t *frame, size_t n)` — last byte is the XOR of the rest.
280. `void hex_dump_line(const uint8_t *a, size_t n)` — up to 16 bytes as spaced hex.
281. `size_t hex_to_bytes(const char *hex, uint8_t *out, size_t cap)` — parse a hex string.
282. `void bytes_to_hex(const uint8_t *in, size_t n, char *out)` — fixed width, uppercase.
283. `bool array_has_duplicate(const int32_t *a, size_t n)` — O(n²) is acceptable here.
284. `int32_t find_single(const int32_t *a, size_t n)` — every value twice except one; XOR.
285. `int32_t find_missing(const int32_t *a, size_t n)` — 1..n with one absent; sum and XOR methods.
286. `size_t count_pairs_sum(const int32_t *a, size_t n, int32_t target)` — pairs summing to target.
287. `int32_t max_recursive(const int32_t *a, size_t n)` — recursion instead of a loop.
288. `uint32_t sum_recursive(const int32_t *a, size_t n)` — recursive sum.
289. `void print_reverse_recursive(const char *s)` — print backwards using recursion.
290. `uint32_t fib_recursive(uint8_t n)` — naive; then state its cost.
291. `bool is_palindrome_recursive(const char *s, size_t lo, size_t hi)` — recursive check.
292. `void towers_of_hanoi(uint8_t n, char from, char to, char via)` — print each move.
293. `uint32_t gcd_recursive(uint32_t a, uint32_t b)` — one-line recursion.
294. `uint32_t power_recursive(uint32_t b, uint8_t e)` — halve the exponent each step.
295. `void permute_string(char *s, size_t k)` — print all permutations.
296. `void subsets_print(const int32_t *a, size_t n)` — all subsets via a bit counter.
297. `uint32_t count_set_bits_range(uint32_t lo, uint32_t hi)` — total 1-bits across a range.
298. `uint32_t count_grid_paths(uint8_t r, uint8_t c)` — monotone paths in a grid.
299. `bool safe_atoi(const char *s, int32_t *out)` — reject empty, junk, and overflow.
300. `void self_test_tier1(void)` — assert-test twenty of your own Tier 1 functions; make it pass.

---

# Tier 2 — Core Competence

**Exercises 301–600.** Two-pointer technique, recursion with purpose, the first real data structures, and hardware that needs a reference manual. Everything here appears in real firmware.

## 2.1 Bit manipulation, one level up

301. `uint8_t popcount_swar(uint32_t x)` — constant time, no loop.
302. `uint8_t popcount_nibble_lut(uint32_t x)` — using a 16-entry table.
303. `uint8_t ctz32(uint32_t x)` — trailing zeros by binary search; 32 for x == 0.
304. `uint8_t clz32(uint32_t x)` — leading zeros by binary search; 32 for x == 0.
305. `uint8_t clo32(uint32_t x)` — leading ones, reusing `clz32`.
306. `uint32_t rev32(uint32_t x)` — reverse 32 bits in five divide-and-conquer steps.
307. `uint8_t rev8_lut(uint8_t x)` — generate a 256-byte table at init, then index it.
308. `bool parity32(uint32_t x)` — XOR-fold, branchless.
309. `uint32_t next_pow2(uint32_t x)` — smear and increment; 1 for x == 0.
310. `uint8_t log2_floor(uint32_t x)` — via `clz32`.
311. `uint32_t rotl32(uint32_t x, uint8_t n)` — correct for n == 0 and n ≥ 32.
312. `uint32_t rotr32(uint32_t x, uint8_t n)` — the same guards.
313. `uint32_t get_field(uint32_t r, uint8_t pos, uint8_t w)` — right-aligned extract.
314. `uint32_t put_field(uint32_t r, uint32_t v, uint8_t pos, uint8_t w)` — insert; mask v to w bits.
315. `int32_t get_field_signed(uint32_t r, uint8_t pos, uint8_t w)` — extract and sign-extend.
316. `uint32_t mask_range(uint8_t lo, uint8_t hi)` — bits lo..hi inclusive.
317. `uint32_t toggle_range(uint32_t x, uint8_t lo, uint8_t hi)` — flip a range; handle hi == 31.
318. `uint32_t swap_bits(uint32_t x, uint8_t i, uint8_t j)` — branchless when the bits are equal.
319. `uint32_t swap_bit_pairs(uint32_t x)` — exchange the bits within each pair.
320. `uint8_t count_runs_of_ones(uint32_t x)` — using `x & ~(x << 1)`.
321. `uint8_t longest_run_ones(uint32_t x)` — using repeated `x &= x << 1`.
322. `uint32_t gray_encode(uint32_t x)` — binary to Gray.
323. `uint32_t gray_decode(uint32_t g)` — Gray to binary, log steps.
324. `int32_t abs_branchless(int32_t x)` — no branch; document `INT32_MIN`.
325. `bool add_ovf_i32(int32_t a, int32_t b)` — detect signed overflow without causing it.
326. `bool add_ovf_u32(uint32_t a, uint32_t b)` — one line.
327. `uint32_t sat_add_u32(uint32_t a, uint32_t b)` — saturating add.
328. `int16_t sat_add_i16(int16_t a, int16_t b)` — saturating both directions.
329. `bool has_zero_byte(uint32_t x)` — branchless test.
330. `uint8_t count_bytes_eq(uint32_t x, uint8_t b)` — SWAR comparison.
331. `uintptr_t align_up(uintptr_t p, size_t a)` — power-of-two alignment, no division.
332. `bool is_aligned(uintptr_t p, size_t a)` — one line.
333. `size_t div_ceil(size_t a, size_t b)` — ceiling division.
334. `void bit_addr(uint32_t idx, uint32_t *byte, uint8_t *bit)` — no `/` or `%`.
335. `bitarr_set` / `bitarr_clear` / `bitarr_test` — the three bitmap primitives.

## 2.2 Two-pointer and sliding window

336. `bool two_sum_sorted(const int32_t *a, size_t n, int32_t t, size_t *i, size_t *j)` — inward pointers.
337. `size_t remove_duplicates(int32_t *a, size_t n)` — sorted input, read/write pointers.
338. `size_t remove_value(int32_t *a, size_t n, int32_t v)` — compact in place.
339. `void partition_even_odd(int32_t *a, size_t n)` — evens first, two pointers.
340. `void sort_colors(uint8_t *a, size_t n)` — values 0/1/2, one pass, three pointers.
341. `size_t rle_encode(const uint8_t *in, size_t n, uint8_t *out, size_t cap)` — run-length encode.
342. `size_t rle_decode(const uint8_t *in, size_t n, uint8_t *out, size_t cap)` — the inverse.
343. `int32_t max_sum_window(const int32_t *a, size_t n, size_t k)` — fixed window, running sum.
344. `size_t min_window_sum_atleast(const int32_t *a, size_t n, int32_t target)` — shortest window ≥ target.
345. `size_t longest_run_below(const int32_t *a, size_t n, int32_t limit)` — longest run of values < limit.
346. `int32_t max_subarray_sum(const int32_t *a, size_t n)` — Kadane's algorithm.
347. `void prefix_sums(const int32_t *a, size_t n, int32_t *out)` — running totals.
348. `int32_t range_sum(const int32_t *prefix, size_t i, size_t j)` — O(1) via prefix sums.
349. `size_t count_subarrays_sum(const int32_t *a, size_t n, int32_t t)` — subarrays summing to t.
350. `bool has_pair_diff(const int32_t *a, size_t n, int32_t d)` — sorted input, two pointers.
351. `size_t merge_intervals(struct iv *a, size_t n)` — sort, then merge overlapping ranges.
352. `bool is_subsequence(const char *s, const char *t)` — is s a subsequence of t.
353. `size_t longest_unique_window(const char *s)` — longest substring with no repeat.
354. `void reverse_words_inplace(char *s)` — reverse all, then reverse each word.
355. `int32_t max_area_two_ptr(const int32_t *h, size_t n)` — largest rectangle between two heights.

## 2.3 Memory functions

356. `void *my_memcpy(void *d, const void *s, size_t n)` — correct working type.
357. `void *my_memmove(void *d, const void *s, size_t n)` — both overlap directions.
358. `void *my_memset(void *d, int c, size_t n)` — low byte of c only.
359. `int my_memcmp(const void *a, const void *b, size_t n)` — sign correct for high bytes.
360. `void *my_memchr(const void *s, int c, size_t n)` — first match or NULL.
361. `void *fast_memcpy(void *d, const void *s, size_t n)` — word loop when both are aligned.
362. `void *fast_memset(void *d, uint8_t b, size_t n)` — align first, then word-broadcast.
363. `void memxor(uint8_t *d, const uint8_t *s, size_t n)` — d ^= s.
364. `bool mem_all_zero(const uint8_t *s, size_t n)` — word-at-a-time.
365. `bool ct_equal(const uint8_t *a, const uint8_t *b, size_t n)` — constant time, no early exit.
366. `void secure_zero(void *p, size_t n)` — a clear the optimiser cannot delete.
367. `void mem_swap(void *a, void *b, size_t n)` — exchange two non-overlapping regions.
368. `size_t mem_common_prefix(const uint8_t *a, const uint8_t *b, size_t n)` — matching prefix length.
369. `const uint8_t *find_pattern(const uint8_t *h, size_t hn, const uint8_t *nd, size_t nn)` — binary-safe search.
370. `void copy_stride(uint8_t *d, const uint8_t *s, size_t n, size_t stride)` — every stride-th byte.

## 2.4 Strings, one level up

371. `size_t my_strnlen(const char *s, size_t max)` — never read past max.
372. `size_t my_strlcpy(char *d, const char *s, size_t dsize)` — always terminate; return source length.
373. `size_t str_append_bounded(char *d, size_t dsize, const char *s)` — safe concat, report truncation.
374. `int my_strcmp(const char *a, const char *b)` — cast to `unsigned char` before subtracting.
375. `int my_strncmp(const char *a, const char *b, size_t n)` — bounded compare.
376. `int str_casecmp(const char *a, const char *b)` — case-insensitive.
377. `char *my_strstr(const char *h, const char *n)` — substring; an empty needle returns h.
378. `char *my_strchr(const char *s, int c)` — including a match on the terminator.
379. `char *my_strrchr(const char *s, int c)` — last occurrence.
380. `size_t str_span(const char *s, const char *set)` — length of the leading run from set.
381. `char *my_strtok_r(char *s, const char *delim, char **save)` — reentrant tokeniser.
382. `size_t split_preserve_empty(char *s, char delim, char **out, size_t max)` — keep empty fields.
383. `bool str_to_u32(const char *s, uint32_t *out)` — reject empty, junk, overflow.
384. `bool str_to_i32(const char *s, int32_t *out)` — the same, with a sign.
385. `bool hex_to_u32(const char *s, uint32_t *out)` — validated hex parse.
386. `size_t u32_to_str_bounded(uint32_t v, char *out, size_t cap)` — return length, or 0 if it will not fit.
387. `size_t i32_to_str_bounded(int32_t v, char *out, size_t cap)` — handle `INT32_MIN`.
388. `size_t u32_to_hex(uint32_t v, char *out, size_t cap)` — fixed eight digits.
389. `size_t fixed_to_str(int32_t q16, char *out, size_t cap, uint8_t decimals)` — Q16.16 to text.
390. `bool is_anagram(const char *a, const char *b)` — count up, count down, check all zero.
391. `void char_frequency(const char *s, uint16_t freq[256])` — index with `unsigned char`.
392. `char first_unique_char(const char *s)` — first non-repeating character; 0 if none.
393. `size_t longest_word(const char *s, const char **start)` — length and position, no copying.
394. `void capitalise_words(char *s)` — uppercase the first letter of each word.
395. `size_t str_squeeze(char *s, char c)` — collapse runs of c to one; return the new length.

## 2.5 Recursion with purpose

396. `uint64_t fib_memo(uint8_t n, uint64_t *cache)` — linear time with a cache.
397. `uint32_t power_fast(uint32_t b, uint8_t e)` — binary exponentiation, recursive.
398. `int bsearch_rec(const int32_t *a, int lo, int hi, int32_t key)` — recursive binary search.
399. `void reverse_rec(int32_t *a, size_t lo, size_t hi)` — recursive in-place reverse.
400. `uint32_t sum_digits_rec(uint32_t n)` — recursive digit sum.
401. `void print_binary_rec(uint32_t v)` — print bits recursively, no buffer.
402. `size_t flood_count(uint8_t g[8][8], size_t r, size_t c, uint8_t target)` — recursive fill, count cells.
403. `bool maze_solve(const uint8_t g[8][8], size_t r, size_t c)` — recursive path search.
404. `void permute_cb(char *s, size_t k, void (*cb)(const char *))` — permutations via callback.
405. `void combinations(const int32_t *a, size_t n, size_t k, int32_t *buf, size_t d)` — all k-subsets.
406. `uint32_t count_paths_rec(uint8_t r, uint8_t c)` — grid paths, recursive.
407. `uint32_t ackermann(uint8_t m, uint8_t n)` — implement, then explain why it is unusable on 1 KB of stack.
408. `size_t stack_depth_probe(size_t d)` — recurse and report the deepest safe depth on your target.
409. `void hanoi_count(uint8_t n, uint32_t *moves)` — count moves without printing.
410. Convert exercise 402 to an iterative version with an explicit stack; compare stack usage.

## 2.6 Linked lists

411. Define `struct node { int32_t v; struct node *next; }` and write `list_push_front`.
412. `void list_push_back(struct node **head, struct node *n)` — append, handling an empty list.
413. `struct node *list_find(struct node *h, int32_t v)` — first match or NULL.
414. `size_t list_length(const struct node *h)` — count nodes.
415. `bool list_remove(struct node **h, int32_t v)` — remove by value using a `**` walk, no head special case.
416. `struct node *list_pop_front(struct node **h)` — detach and return the head.
417. `void list_print(const struct node *h)` — print all values.
418. `struct node *list_reverse(struct node *h)` — iterative, three pointers.
419. `struct node *list_reverse_rec(struct node *h)` — recursive; state the stack cost.
420. `struct node *list_middle(struct node *h)` — one pass, slow and fast pointers.
421. `bool list_has_cycle(const struct node *h)` — Floyd's detection.
422. `struct node *list_cycle_start(struct node *h)` — where the loop begins.
423. `struct node *list_nth_from_end(struct node *h, size_t k)` — one pass, two pointers.
424. `struct node *list_merge_sorted(struct node *a, struct node *b)` — merge two sorted lists.
425. `struct node *list_sort(struct node *h)` — merge sort on a list.
426. `void list_dedup_sorted(struct node *h)` — sorted input, remove duplicates in place.
427. `bool list_is_palindrome(struct node *h)` — reverse the second half, compare, restore.
428. `struct node *list_intersection(struct node *a, struct node *b)` — first shared node or NULL.
429. `void list_split_alternate(struct node *h, struct node **a, struct node **b)` — deal into two.
430. `void list_rotate(struct node **h, size_t k)` — rotate left by k.
431. Define a doubly linked list with a sentinel; write `dll_init` and `dll_insert_after`.
432. `void dll_remove(struct dnode *n)` — O(1) with no null checks; explain why the sentinel allows it.
433. `bool dll_empty(const struct dnode *s)` — one comparison.
434. Write `container_of(ptr, type, member)` and use it to walk an intrusive list.
435. Build a static node pool with a free list of **indices**; write `pool_init`, `pool_alloc`, `pool_free`.

## 2.7 Stacks and queues

436. Define a fixed-capacity `int32_t` stack; write `push`, `pop`, `peek`, all returning `bool`.
437. `bool stack_empty(const struct stk *s)` and `stack_full` — the two state tests.
438. `bool paren_balanced(const char *s)` — stack-based, handling `()[]{}`.
439. `int32_t eval_postfix(const char *expr)` — RPN evaluator.
440. `bool infix_to_postfix(const char *in, char *out, size_t cap)` — shunting-yard.
441. `void stack_reverse_array(int32_t *a, size_t n)` — reverse using a stack.
442. `int32_t stack_min(const struct stk *s)` — O(1) minimum using an auxiliary stack.
443. Two stacks in one array, growing toward each other; write both push functions.
444. Define a circular queue with an explicit `count`; write `enqueue` and `dequeue`.
445. Define the same queue **without** `count`, wasting one slot; write both, and state the trade-off.
446. `bool queue_full(const struct q *q)` and `queue_empty` — for the slot-wasting version.
447. `size_t queue_count(const struct q *q)` — for the free-running-index version.
448. Implement a queue using two stacks; write `enqueue` and `dequeue`.
449. Implement a stack using two queues; write `push` and `pop`.
450. Define a byte ring buffer with power-of-two capacity; write `rb_write` and `rb_read`.
451. `size_t rb_used(const struct rb *r)` and `rb_free` — wrap-safe occupancy.
452. `bool rb_peek(const struct rb *r, uint8_t *b)` — inspect without consuming.
453. `size_t rb_write_bulk(struct rb *r, const uint8_t *s, size_t n)` — two `memcpy` calls, not a byte loop.
454. `size_t rb_read_bulk(struct rb *r, uint8_t *d, size_t n)` — the inverse.
455. `void rb_write_overwrite(struct rb *r, uint8_t b)` — drop the oldest when full; state what this breaks.

## 2.8 Sorting and searching, one level up

456. `void insertion_sort_generic(void *b, size_t n, size_t sz, int (*cmp)(const void *, const void *))`.
457. `void merge_sort(int32_t *a, size_t n, int32_t *scratch)` — top-down with an explicit buffer.
458. `void merge_sort_bottom_up(int32_t *a, size_t n, int32_t *scratch)` — no recursion.
459. `void quicksort(int32_t *a, size_t n)` — Lomuto partition.
460. `void quicksort_median3(int32_t *a, size_t n)` — median-of-three pivot.
461. `void quicksort_iterative(int32_t *a, size_t n)` — explicit stack, bounded depth.
462. `void heap_sort(int32_t *a, size_t n)` — sift down, in place.
463. `void shell_sort(int32_t *a, size_t n)` — gap sequence of your choice.
464. `void radix_sort_u32(uint32_t *a, size_t n, uint32_t *scratch)` — LSD, one byte per pass.
465. `void counting_sort_range(int32_t *a, size_t n, int32_t lo, int32_t hi)` — bounded value range.
466. Construct a test case proving your quicksort is not stable.
467. `int32_t kth_smallest(int32_t *a, size_t n, size_t k)` — quickselect.
468. `int32_t median_of(int32_t *a, size_t n)` — via selection, not a full sort.
469. `size_t count_less_than(const int32_t *a, size_t n, int32_t v)` — sorted input, binary search.
470. `bool binary_search_rotated(const int32_t *a, size_t n, int32_t key, size_t *idx)` — rotated sorted array.
471. `size_t find_rotation_point(const int32_t *a, size_t n)` — index of the smallest element.
472. `bool search_2d_sorted(const int32_t *m, size_t rows, size_t cols, int32_t key)` — row- and column-sorted.
473. `size_t first_true(bool (*pred)(size_t), size_t n)` — binary search over a predicate.
474. `uint32_t isqrt_binary(uint32_t n)` — integer square root by binary search.
475. `uint32_t nth_root(uint32_t v, uint8_t k)` — integer k-th root by binary search.

## 2.9 Hashing and sets

476. `uint32_t hash_djb2(const char *s)` — shift-add hash.
477. `uint32_t hash_fnv1a(const void *d, size_t n)` — XOR then multiply.
478. `uint32_t hash_u32(uint32_t x)` — Knuth multiplicative, for integer keys.
479. Define an open-addressed table with linear probing; write `init` and `put`.
480. `bool ht_get(const struct ht *h, uint32_t k, uint32_t *v)` — probe, stopping at an empty slot.
481. `bool ht_del(struct ht *h, uint32_t k)` — tombstone, not empty; explain why.
482. `size_t ht_count(const struct ht *h)` — live entries, excluding tombstones.
483. `void ht_rehash(struct ht *h)` — rebuild to purge tombstones.
484. Define a chained hash table over a static node pool; write `put` and `get`.
485. `bool set_add(struct bitset *s, uint16_t v)` — bitmap-backed set of small integers.
486. `bool set_contains(const struct bitset *s, uint16_t v)` — one test.
487. `void set_union(struct bitset *d, const struct bitset *s)` — word-wise OR.
488. `void set_intersect(struct bitset *d, const struct bitset *s)` — word-wise AND.
489. `size_t set_count(const struct bitset *s)` — popcount over the words.
490. `void bloom_add(struct bloom *b, const void *k, size_t n)` — k hashes, k bits set.
491. `bool bloom_maybe(const struct bloom *b, const void *k, size_t n)` — state exactly what `true` means.
492. `bool first_repeated_char(const char *s, char *out)` — using a 256-bit bitmap.
493. `size_t count_distinct(const int32_t *a, size_t n)` — bounded range, bitmap.
494. `bool arrays_disjoint(const int32_t *a, size_t na, const int32_t *b, size_t nb)` — set-based.
495. `size_t intersection_size(const int32_t *a, size_t na, const int32_t *b, size_t nb)` — set-based.

## 2.10 Trees

496. Define a BST node; write `bst_insert` iteratively.
497. `struct bnode *bst_find(struct bnode *r, int32_t k)` — iterative search.
498. `int32_t bst_min(struct bnode *r)` and `bst_max` — leftmost and rightmost.
499. `size_t bst_size(const struct bnode *r)` — node count.
500. `size_t bst_height(const struct bnode *r)` — height; define the empty-tree result.
501. `void bst_inorder(const struct bnode *r, void (*cb)(int32_t))` — sorted traversal.
502. `void bst_preorder(const struct bnode *r, void (*cb)(int32_t))` — root first.
503. `void bst_postorder(const struct bnode *r, void (*cb)(int32_t))` — children first.
504. `void bst_inorder_iterative(struct bnode *r, void (*cb)(int32_t))` — explicit stack.
505. `void bst_level_order(struct bnode *r, void (*cb)(int32_t))` — breadth-first with a queue.
506. `bool bst_is_valid(const struct bnode *r)` — validate ordering using bounds, not just parent checks.
507. `struct bnode *bst_delete(struct bnode *r, int32_t k)` — all three cases.
508. `int32_t bst_kth_smallest(struct bnode *r, size_t k)` — in-order with a counter.
509. `struct bnode *bst_lca(struct bnode *r, int32_t a, int32_t b)` — lowest common ancestor.
510. `size_t bst_count_leaves(const struct bnode *r)` — nodes with no children.
511. `bool tree_is_balanced(const struct bnode *r)` — height difference ≤ 1 everywhere.
512. `void tree_mirror(struct bnode *r)` — swap left and right throughout.
513. `bool trees_identical(const struct bnode *a, const struct bnode *b)` — same shape and values.
514. `int32_t tree_max_path_sum(const struct bnode *r)` — largest root-to-leaf sum.
515. `struct bnode *bst_from_sorted(const int32_t *a, size_t n, struct bnode *pool)` — build balanced.

## 2.11 Heaps

516. `void heap_push(struct heap *h, int32_t v)` — array-backed min-heap, sift up.
517. `bool heap_pop_min(struct heap *h, int32_t *out)` — move last to root, sift **down**.
518. `bool heap_peek(const struct heap *h, int32_t *out)` — inspect the root.
519. `void heapify(int32_t *a, size_t n)` — build in O(n).
520. `bool is_heap(const int32_t *a, size_t n)` — validate the heap property.
521. `void heap_decrease_key(struct heap *h, size_t i, int32_t v)` — lower a value, restore order.
522. `void heap_delete_at(struct heap *h, size_t i)` — remove an arbitrary element.
523. `void top_k_stream(struct heap *h, int32_t v, size_t k)` — maintain the top k with a min-heap.
524. `void merge_k_sorted(const int32_t **arrs, const size_t *lens, size_t k, int32_t *out)` — heap merge.
525. Define a priority queue with a sequence number for stability; write `push` and `pop`.

## 2.12 Registers, GPIO, timers

526. `void gpio_init_output(volatile uint32_t *moder, volatile uint32_t *otyper, uint8_t pin, bool od)`.
527. `void gpio_init_input(volatile uint32_t *moder, volatile uint32_t *pupdr, uint8_t pin, uint8_t pull)`.
528. `void gpio_init_af(volatile uint32_t *moder, volatile uint32_t *afr, uint8_t pin, uint8_t af)`.
529. `void gpio_toggle(volatile uint32_t *odr, uint8_t pin)` — then say why `BSRR` is better.
530. `void gpio_write_port(volatile uint32_t *odr, uint16_t value, uint16_t mask)` — masked whole-port write.
531. `void reg_field_write(volatile uint32_t *r, uint8_t pos, uint8_t w, uint32_t v)` — one read, one write.
532. `void set_fields(volatile uint32_t *r, const struct field *f, size_t n)` — batch mask/value pairs.
533. `int reg_wait_bit(volatile uint32_t *r, uint32_t mask, bool set, uint32_t timeout_us)` — bounded poll.
534. `uint64_t read_counter64(volatile uint32_t *lo, volatile uint32_t *hi)` — retry until `hi` is stable.
535. `void systick_init(uint32_t reload)` — 1 kHz tick; remember `RVR = reload − 1`.
536. `void delay_ms(uint32_t ms)` — wrap-safe tick comparison, `volatile` counter.
537. `bool interval_elapsed(uint32_t *last, uint32_t interval)` — periodic trigger with no drift.
538. `void dwt_init(void)` and `uint32_t dwt_cycles(void)` — enable and read the cycle counter.
539. `uint32_t measure_cycles(void (*fn)(void))` — time a function in cycles.
540. `int pwm_init(volatile TIM_T *t, uint32_t tclk, uint32_t freq)` — smallest PSC that fits ARR.
541. `void pwm_set_duty(volatile TIM_T *t, uint8_t ch, uint16_t permille)` — with preload enabled.
542. `uint32_t capture_period(void)` — input capture, handling counter overflow.
543. `void wdt_init(uint32_t timeout_ms)` — independent watchdog with the unlock sequence.
544. `void wdt_supervisor(void)` — kick only when every task has set its liveness flag.
545. `uintptr_t bitband_alias(uintptr_t addr, uint8_t bit)` — Cortex-M bit-band arithmetic.

## 2.13 First drivers

546. `int uart_init(volatile UART_T *u, uint32_t pclk, uint32_t baud)` — reject > 2% baud error.
547. `void uart_putc_blocking(volatile UART_T *u, uint8_t b)` — wait for `TXE`, then write.
548. `bool uart_getc_timeout(volatile UART_T *u, uint8_t *b, uint32_t us)` — bounded receive.
549. `void uart_puts(volatile UART_T *u, const char *s)` — blocking string send.
550. `void uart_flush(volatile UART_T *u)` — wait for `TC`, not `TXE`; explain the difference.
551. `uint8_t spi_xfer_byte(volatile SPI_T *s, uint8_t out)` — wait `TXE`, write, wait `RXNE`, read.
552. `void spi_xfer(volatile SPI_T *s, const uint8_t *tx, uint8_t *rx, size_t n)` — full duplex.
553. `void spi_transaction(...)` — assert CS, transfer, wait `BSY`, deassert.
554. `int i2c_write(volatile I2C_T *i, uint8_t addr, const uint8_t *d, size_t n)` — detect NACK.
555. `int i2c_read(volatile I2C_T *i, uint8_t addr, uint8_t *d, size_t n)` — bounded waits.
556. `int i2c_write_read(...)` — repeated START, no STOP between phases.
557. `int i2c_read_reg(uint8_t addr, uint8_t reg, uint8_t *v)` — the one-byte sensor read.
558. `void i2c_scan(uint8_t *found, size_t cap, size_t *n)` — probe every address, record responders.
559. `int i2c_bus_recover(...)` — nine clock pulses plus a manual STOP.
560. `uint16_t adc_read_once(volatile ADC_T *a, uint8_t ch)` — configure, start, wait, read.
561. `uint16_t adc_read_averaged(volatile ADC_T *a, uint8_t ch, uint8_t log2n)` — oversample and shift.
562. `uint32_t adc_to_mv_calibrated(uint16_t raw)` — using `VREFINT`, not the nominal supply.
563. `void dac_write(volatile DAC_T *d, uint16_t v)` — 12-bit output with alignment handled.
564. `int flash_unlock(void)` and `flash_lock(void)` — key sequence, and verify it took effect.
565. `int flash_write_word(uint32_t addr, uint32_t v)` — wait `BSY`, check every error flag.

## 2.14 C semantics and undefined behaviour

566. Write a `volatile`-less busy-wait that hangs at `-O2`; fix it and explain why.
567. Show that `x++` on a `uint32_t` is not atomic by naming the three instructions.
568. Write a `memset` on a secret buffer that the compiler removes; prove it from the disassembly.
569. Demonstrate strict-aliasing breakage: write through an `int *`, read through a `float *`.
570. Show `sizeof(arr)` differing inside and outside a function; explain decay.
571. Write a macro that misbehaves when an argument has a side effect; then fix it.
572. Show why `#define SQ(x) (x*x)` still fails for `SQ(1+2)`; fix with full parenthesisation.
573. Write a function-like macro with a `do { } while (0)` body and explain what it fixes.
574. Demonstrate that plain `char` signedness is implementation-defined.
575. Show a signed left-shift into the sign bit; name the standard clause it violates.
576. Show a shift by 32 on a `uint32_t`; explain why Arm and x86 disagree.
577. Demonstrate integer promotion changing the result of `uint16_t * uint16_t`.
578. Show `-1 < sizeof(int)` evaluating false; fix it two ways.
579. Write a `size_t` loop counting down to 0 that never terminates; fix it.
580. Show `n - 1` wrapping when `n == 0` in a loop bound; fix it.
581. Return the address of a local and show the consequence; fix it three ways.
582. Show a use-after-free reading plausible stale data; add poisoning so it becomes visible.
583. Show a double free corrupting a free list; add a magic value that detects it.
584. Show struct padding leaking stack bytes into a serialised frame; fix by field-wise packing.
585. Show `#pragma pack(1)` producing an unaligned access; explain the Cortex-M0 fault.
586. Write two translation units with mismatched `extern` declarations; explain what the linker misses.
587. Contrast a function-scope `static` with a file-scope one; describe lifetime versus linkage.
588. Write `const char *` and `char *const`; try to modify through each and explain the errors.
589. Write `volatile const uint32_t *` and say what it correctly describes.
590. Show `restrict` enabling an optimisation; describe what breaks if the promise is false.

## 2.15 Tier 2 review

591. `uint8_t crc8(const uint8_t *d, size_t n)` — bitwise, polynomial 0x07.
592. `uint16_t crc16_ccitt(const uint8_t *d, size_t n)` — init 0xFFFF, poly 0x1021, MSB-first.
593. `uint16_t crc16_modbus(const uint8_t *d, size_t n)` — reflected, poly 0xA001.
594. `uint16_t fletcher16(const uint8_t *d, size_t n)` — two running sums mod 255.
595. `uint16_t inet_checksum(const uint8_t *d, size_t n)` — one's complement with carry folding.
596. `void parser_feed(struct parser *p, uint8_t b)` — SOF/LEN/DATA/CRC machine; validate LEN.
597. `bool thermostat_update(struct thermo *t, int16_t temp_x10)` — hysteresis; output feeds back as input.
598. `bool debounce_update(struct deb *d, bool raw)` — shift register, N agreeing samples.
599. `uint16_t moving_average(struct ma *m, uint16_t s)` — running sum, power-of-two window.
600. `void self_test_tier2(void)` — assert-test thirty Tier 2 functions; make the whole suite pass.

---

# Tier 3 — Applied

**Exercises 601–900.** Everything here is something real firmware contains. Tier 2 gave you a ring buffer; this tier makes it ISR-safe. Tier 2 gave you a bitwise CRC; this tier makes it table-driven and streaming. Expect to need a reference manual and expect each exercise to take 15–45 minutes.

## 3.1 Bit manipulation, applied

601. `size_t pack_12bit(const uint16_t *s, size_t n, uint8_t *out)` — 2 samples per 3 bytes, MSB-first, handle odd n.
602. `size_t unpack_12bit(const uint8_t *in, size_t bytes, uint16_t *out)` — the exact inverse; round-trip test it.
603. `size_t pack_10bit(const uint16_t *s, size_t n, uint8_t *out)` — 4 samples per 5 bytes.
604. `size_t pack_6bit(const uint8_t *s, size_t n, uint8_t *out)` — 4 samples per 3 bytes.
605. `size_t pack_bits(const uint32_t *s, size_t n, uint8_t width, uint8_t *out)` — arbitrary width 1..32.
606. `size_t unpack_bits(const uint8_t *in, size_t n, uint8_t width, uint32_t *out)` — inverse of 605.
607. `uint32_t morton_encode2(uint16_t x, uint16_t y)` — interleave two values, log-time spread.
608. `void morton_decode2(uint32_t c, uint16_t *x, uint16_t *y)` — log-time compact.
609. `uint32_t morton_encode3(uint16_t x, uint16_t y, uint16_t z)` — three 10-bit values interleaved.
610. `uint32_t bit_deposit(uint32_t src, uint32_t mask)` — scatter low bits into mask positions.
611. `uint32_t bit_extract(uint32_t src, uint32_t mask)` — gather mask-selected bits to the low end.
612. `void double_dabble16(uint16_t v, uint8_t bcd[5])` — binary to 5-digit BCD, no division.
613. `void double_dabble32(uint32_t v, uint8_t bcd[10])` — the same for 32 bits.
614. `uint32_t bcd8_to_bin(uint32_t bcd)` — 8-digit packed BCD to binary.
615. `uint32_t bin_to_bcd32(uint32_t v)` — binary to 8-digit packed BCD.
616. `uint32_t next_same_popcount(uint32_t x)` — Gosper's hack; 0 if none exists.
617. `uint32_t prev_same_popcount(uint32_t x)` — the downward equivalent.
618. `uint8_t nth_set_bit(uint32_t x, uint8_t k)` — position of the k-th set bit; 32 if absent.
619. `uint8_t count_set_below(uint32_t x, uint8_t n)` — rank: set bits at positions below n.
620. `uint32_t select_bit(uint32_t x, uint8_t rank)` — inverse of rank; isolate that bit.
621. `void reverse_bit_buffer(uint8_t *b, size_t n)` — reverse byte order **and** bits within each byte.
622. `void reverse_bits_each_byte(uint8_t *b, size_t n)` — bits only; contrast with 621.
623. `uint32_t rotl_field(uint32_t x, uint8_t pos, uint8_t w, uint8_t n)` — rotate within a bit field.
624. `bool mask_is_contiguous(uint32_t m)` — true iff the set bits form one unbroken run.
625. `uint8_t netmask_to_cidr(uint32_t m)` — prefix length; 0xFF if the mask is malformed.

## 3.2 Fixed-point arithmetic

626. Define `q16_t` for Q16.16 with `TO_Q`, `FROM_Q`, and a rounded `q_round`.
627. `q16_t q_mul(q16_t a, q16_t b)` — 64-bit intermediate; prove the 32-bit version overflows.
628. `q16_t q_div(q16_t a, q16_t b)` — pre-shift the numerator; saturate on divide-by-zero.
629. `q16_t q_from_ratio(int32_t num, int32_t den)` — exact conversion with rounding.
630. `int32_t q_to_int_round(q16_t v)` — round to nearest, correct for negatives.
631. `q16_t q_sqrt(q16_t v)` — fixed-point square root.
632. `q16_t q_recip(q16_t v)` — reciprocal; state the precision loss for small v.
633. Define Q8.24 and Q24.8 types; write conversions between them and Q16.16.
634. `int32_t scale_round(int32_t x, int32_t in_lo, int32_t in_hi, int32_t out_lo, int32_t out_hi)` — map with rounding, 64-bit intermediate.
635. `uint32_t mul_div(uint32_t a, uint32_t b, uint32_t c)` — (a×b)/c without overflowing.
636. `uint32_t mul_shift(uint32_t a, uint32_t mult, uint8_t shift)` — reciprocal multiply, no division.
637. `void reciprocal_prepare(uint32_t divisor, uint32_t *mult, uint8_t *shift)` — precompute for 636.
638. `int32_t lerp_q16(int32_t a, int32_t b, q16_t t)` — linear interpolation in fixed point.
639. `q16_t q_sin(uint16_t angle)` — quarter-wave table plus symmetry; angle spans a full turn.
640. `q16_t q_cos(uint16_t angle)` — via `q_sin` with a phase offset.
641. `uint16_t q_atan2(int32_t y, int32_t x)` — CORDIC or a rational approximation.
642. `uint32_t isqrt_digit(uint32_t n)` — digit-by-digit integer square root, fixed iterations.
643. `uint32_t ilog10(uint32_t v)` — integer base-10 logarithm, no floating point.
644. `uint32_t ipow10(uint8_t e)` — powers of ten via a table.
645. `int32_t thermistor_temp_x10(uint16_t adc, const int16_t *lut, size_t n)` — LUT plus interpolation.
646. `int32_t lut_interp(const int16_t *y, size_t n, int32_t x_min, int32_t x_step, int32_t x)` — clamp both ends.
647. `int32_t lut_interp_nonuniform(const int32_t *x, const int32_t *y, size_t n, int32_t v)` — binary search the segment.
648. `uint32_t rms_u16(const uint16_t *s, size_t n)` — accumulate squares in 64 bits, then `isqrt`.
649. `int32_t vector_magnitude(int16_t x, int16_t y, int16_t z)` — 3-axis magnitude, integer only.
650. `uint32_t alpha_beta_magnitude(int32_t i, int32_t q)` — the cheap |I|+|Q| approximation; state the error.

## 3.3 Filters and signal processing

651. `uint16_t ma_update(struct ma *m, uint16_t s)` — moving average with a running sum, power-of-two window.
652. `int32_t ema_update(struct ema *e, int32_t s)` — keep the accumulator pre-scaled so it cannot stall.
653. Demonstrate the stalling bug: write the naive `acc += (x - acc) >> k` and find an input where it sticks.
654. `int32_t iir_biquad(struct biquad *b, int32_t x)` — direct-form-I biquad in Q16, saturating.
655. `void biquad_lowpass_coeffs(struct biquad *b, uint32_t fs, uint32_t fc, q16_t q)` — compute coefficients.
656. `int32_t fir_apply(const int16_t *taps, size_t n, struct fir *s, int32_t x)` — circular delay line.
657. `uint16_t median3(uint16_t a, uint16_t b, uint16_t c)` — branch-light median of three.
658. `uint16_t median_n(struct medfilt *m, uint16_t s)` — sliding median over an odd window.
659. `uint16_t median5_network(const uint16_t v[5])` — sorting network, fixed comparisons, no loop.
660. `int32_t hampel_filter(struct hampel *h, int32_t x)` — reject outliers beyond k × MAD.
661. `int32_t deadband(int32_t x, int32_t last, int32_t band)` — suppress changes smaller than the band.
662. `int32_t slew_update(struct slew *s, int32_t target)` — asymmetric up/down limits; snap when close.
663. `int32_t rate_limit_leaky(struct bucket *b, int32_t v, uint32_t dt_ms)` — leaky-bucket rate limiter.
664. `int32_t pid_update(struct pid *p, int32_t sp, int32_t pv)` — anti-windup clamp, derivative on measurement.
665. Demonstrate derivative kick: run 664 with derivative-on-error and show the output spike on a setpoint step.
666. Demonstrate integral windup: saturate the output and show the overshoot; then fix it two ways.
667. `int32_t pi_controller(struct pi *p, int32_t err)` — PI only, with conditional integration.
668. `uint32_t energy_accumulate(struct energy *e, uint32_t mw, uint32_t dt_ms)` — keep the remainder; no drift.
669. `int32_t coulomb_count(struct coul *c, int32_t ma, uint32_t dt_ms)` — signed charge integration.
670. `uint32_t freq_from_period(uint32_t period_ticks, uint32_t tick_hz)` — with rounding.
671. `uint32_t freq_from_gate(uint32_t edges, uint32_t gate_ms)` — counting method; state the crossover with 670.
672. `size_t zero_crossings(const int16_t *s, size_t n)` — count sign changes with hysteresis.
673. `int32_t goertzel(const int16_t *s, size_t n, q16_t coeff)` — single-bin tone detection.
674. `void correlate(const int16_t *a, const int16_t *b, size_t n, int32_t *out)` — cross-correlation.
675. `size_t peak_detect(const uint16_t *s, size_t n, size_t *idx, size_t cap)` — local maxima above a threshold.
676. `int32_t dc_remove(struct dcblock *d, int32_t x)` — high-pass to strip the DC offset.
677. `uint16_t agc_update(struct agc *a, uint16_t level)` — automatic gain control toward a target level.
678. `int32_t kalman_1d(struct kal1 *k, int32_t z)` — scalar Kalman filter in fixed point.
679. `int32_t complementary_filter(struct comp *c, int32_t acc, int32_t gyro, uint32_t dt_ms)` — fuse two sensors.
680. `void decimate(const int16_t *in, size_t n, uint8_t factor, int16_t *out)` — filter then downsample; explain why filtering first matters.

## 3.4 Applied data structures

681. `bool rb_write_spsc(struct rb *r, uint8_t b)` — one writer per index, `volatile`, barrier before publishing.
682. `bool rb_read_spsc(struct rb *r, uint8_t *b)` — the consumer half; barrier before releasing the slot.
683. `size_t rb_write_bulk_wrap(struct rb *r, const uint8_t *s, size_t n)` — two `memcpy` calls, single index update.
684. `size_t rb_contiguous_free(const struct rb *r)` — free bytes before the wrap point, for zero-copy DMA.
685. `uint8_t *rb_reserve(struct rb *r, size_t n)` — hand out a writable region; then `rb_commit`.
686. `void rb_commit(struct rb *r, size_t n)` — publish a reserved region.
687. `bool rb_write_seq(struct rb *r, uint8_t b, uint32_t *seq)` — attach a sequence number to detect drops.
688. Define a ring buffer of fixed-size **records** rather than bytes; write push and pop.
689. `bool deque_push_front(struct dq *d, int32_t v)` and the other three operations — fixed capacity.
690. `void timer_list_add(struct tl *l, struct tmr *t, uint32_t delay)` — delta-encoded, address-ordered insert.
691. `void timer_list_tick(struct tl *l)` — decrement only the head; fire and unlink expired timers.
692. `void timer_list_cancel(struct tl *l, struct tmr *t)` — remove and repair the following delta.
693. Define a timing wheel with N buckets; write `wheel_add` and `wheel_tick`.
694. `bool lru_get(struct lru *c, uint32_t k, uint32_t *v)` — index-linked recency list, move to front.
695. `void lru_put(struct lru *c, uint32_t k, uint32_t v)` — evict the tail when full.
696. Combine 694 with a hash table so lookup is O(1); state the crossover where it pays.
697. `bool obj_acquire(struct pool *p, struct obj **out)` — reference-counted object pool.
698. `void obj_ref(struct obj *o)` and `obj_release(struct obj *o)` — atomic count, free at zero.
699. Demonstrate a refcount leak and a double release; add assertions that catch both.
700. `bool trie_insert(struct trie *t, const char *key)` — fixed-pool nodes, no `malloc`.
701. `bool trie_lookup(const struct trie *t, const char *key)` — exact match.
702. `size_t trie_prefix_count(const struct trie *t, const char *prefix)` — keys under a prefix.
703. `bool dsu_union(struct dsu *d, uint16_t a, uint16_t b)` — union-find with path compression.
704. `uint16_t dsu_find(struct dsu *d, uint16_t x)` — with union by rank.
705. `void segtree_build(int32_t *tree, const int32_t *a, size_t n)` — range-sum segment tree.
706. `void segtree_update(int32_t *tree, size_t n, size_t i, int32_t v)` — point update.
707. `int32_t segtree_query(const int32_t *tree, size_t n, size_t l, size_t r)` — range query.
708. `void bit_update(int32_t *bit, size_t n, size_t i, int32_t delta)` — Fenwick tree update.
709. `int32_t bit_prefix(const int32_t *bit, size_t i)` — Fenwick prefix sum.
710. `bool skiplist_insert(struct sl *s, int32_t v)` — fixed level count, static node pool.
711. `bool sorted_array_insert(int32_t *a, size_t *n, size_t cap, int32_t v)` — binary search then shift.
712. `bool small_map_put(struct smap *m, uint8_t k, uint32_t v)` — linear-scan map; state when to switch to hashing.
713. `const char *intern(struct pool *p, const char *s)` — string interning into a fixed arena.
714. `void free_list_indices_init(struct fl *f, size_t n)` — free list threaded through an index array.
715. `bool graph_add_edge(struct graph *g, uint8_t a, uint8_t b)` — adjacency list over a static pool.

## 3.5 Allocators

716. `void pool_init(struct pool *p, void *mem, size_t bsize, size_t count)` — thread the free list through free blocks.
717. `void *pool_alloc(struct pool *p)` — O(1) pop; return NULL when exhausted.
718. `void pool_free(struct pool *p, void *b)` — validate range **and** block alignment before accepting.
719. `size_t pool_high_water(const struct pool *p)` — peak usage, for evidence-based sizing.
720. `bool pool_owns(const struct pool *p, const void *b)` — does this pointer belong to this pool.
721. `void *arena_alloc(struct arena *a, size_t n, size_t align)` — bump pointer; capacity test by subtraction.
722. Show the overflow bug: write the `offset + n > size` version and find an `n` that passes it.
723. `void arena_reset(struct arena *a)` — release everything.
724. `size_t arena_mark(const struct arena *a)` and `arena_release(struct arena *a, size_t mark)` — scoped free.
725. `void *arena_alloc_from_end(struct arena *a, size_t n)` — double-ended arena; permanent from one side.
726. `void *fl_alloc(struct fl *h, size_t n)` — first-fit free list with a minimum split size.
727. `void fl_free(struct fl *h, void *p)` — coalesce **both** directions; test the adjacency check.
728. Show fragmentation: alternate allocations and frees until a large request fails with ample free space.
729. `void *fl_alloc_best_fit(struct fl *h, size_t n)` — then measure whether it fragments less than first-fit.
730. `void *buddy_alloc(struct buddy *b, size_t n)` — split down from the smallest sufficient order.
731. `void buddy_free(struct buddy *b, void *p, size_t n)` — merge via `off ^ (1 << order)`.
732. `size_t buddy_internal_waste(size_t request)` — how many bytes a request wastes.
733. `void *aligned_alloc_hdr(size_t n, size_t align)` — over-allocate, align, stash the base pointer below.
734. `void aligned_free_hdr(void *p)` — recover the base and release it.
735. `void *guarded_alloc(struct pool *p)` — header magic plus a trailing canary.
736. `int guarded_free(struct pool *p, void *b)` — distinguish double free, overrun, and foreign pointer.
737. `void poison_block(void *p, size_t n)` — fill so a use-after-free is immediately visible.
738. `void *pool_alloc_critical(struct pool *p)` — ISR-safe via a critical section.
739. `void *pool_alloc_cas(struct pool *p)` — lock-free pop; remember to clear the exclusive monitor.
740. Explain the ABA hazard in 739 and why the Cortex-M monitor closes it.

## 3.6 CRC, checksums, hashing

741. `void crc16_table_build(uint16_t *tab, uint16_t poly)` — generate a 256-entry table at init.
742. `uint16_t crc16_table(const uint8_t *d, size_t n, const uint16_t *tab)` — one lookup per byte.
743. `uint16_t crc16_nibble(const uint8_t *d, size_t n)` — 16-entry table; compare size and speed with 742.
744. `uint32_t crc32_update(uint32_t crc, const uint8_t *d, size_t n)` — running state, no final XOR.
745. `uint32_t crc32_finalise(uint32_t crc)` — apply the final XOR exactly once.
746. `uint32_t crc32_stream(uint32_t addr, size_t len)` — chunked over flash; kick the watchdog.
747. Show the streaming bug: apply the final XOR per chunk and demonstrate the wrong result.
748. `bool crc_residue_check(const uint8_t *frame, size_t n)` — verify by computing over data **and** CRC.
749. Find the residue constant for CRC-16/CCITT-FALSE and for CRC-32; verify both empirically.
750. `uint8_t crc8_sae_j1850(const uint8_t *d, size_t n)` — init 0xFF, poly 0x1D, final XOR 0xFF.
751. `uint16_t crc16_x25(const uint8_t *d, size_t n)` — reflected, final XOR; used by HDLC.
752. `uint32_t crc32c(const uint8_t *d, size_t n)` — Castagnoli polynomial.
753. Verify every CRC you have written against the `"123456789"` check value; tabulate the results.
754. `uint32_t adler32(const uint8_t *d, size_t n)` — with the deferred-modulo optimisation.
755. `uint32_t adler32_update(uint32_t s, const uint8_t *d, size_t n)` — streaming form.
756. `uint32_t fletcher32(const uint16_t *d, size_t n)` — 359-iteration deferred fold.
757. `uint8_t smbus_pec(const uint8_t *d, size_t n)` — SMBus packet error code.
758. `uint32_t murmur3_32(const void *d, size_t n, uint32_t seed)` — full mix and finalise.
759. Compare distribution: hash 10 000 sequential integers with DJB2, FNV-1a, and Murmur3; count bucket collisions for a 256-slot table.
760. Demonstrate that a hash is not a CRC: find two inputs one bit apart whose FNV-1a values collide modulo 256.

## 3.7 Protocol framing and parsers

761. `void parser_feed(struct parser *p, uint8_t b)` — SOF/LEN/DATA/CRC machine with an inter-byte timeout.
762. Show the vulnerability: remove the LEN bound from 761 and demonstrate the buffer overrun with a crafted frame.
763. `void parser_resync(struct parser *p, uint8_t b)` — after a bad length, reconsider the byte as a possible SOF.
764. `size_t cobs_encode(const uint8_t *s, size_t n, uint8_t *d)` — no zero bytes in the output.
765. `size_t cobs_decode(const uint8_t *s, size_t n, uint8_t *d)` — reject an embedded zero and a truncated group.
766. Round-trip 764 and 765 over 1000 random buffers including all-zero and all-0xFF; assert equality.
767. `size_t cobs_max_encoded(size_t n)` — the exact worst-case output size; prove it.
768. `size_t slip_encode(const uint8_t *s, size_t n, uint8_t *d, size_t cap)` — escape `0xC0` and `0xDB`.
769. `size_t slip_decode(const uint8_t *s, size_t n, uint8_t *d, size_t cap)` — the inverse.
770. Construct the SLIP worst case: an input whose encoding is exactly 2n+2 bytes.
771. `void hdlc_feed(struct hdlc *h, uint8_t b)` — flag resync, escape handling, FCS residue check.
772. Show that a flag arriving mid-escape must clear the escape state; write the failing case first.
773. `size_t hdlc_encode(const uint8_t *s, size_t n, uint8_t *d, size_t cap)` — add FCS and escape.
774. `void modbus_rtu_feed(struct mb *m, uint8_t b, uint32_t now_us)` — start a new frame after t3.5 silence.
775. `void modbus_rtu_tick(struct mb *m, uint32_t now_us)` — detect the trailing silence that ends a frame.
776. `uint32_t modbus_t35_us(uint32_t baud)` — 3.5 character times; fixed at 1750 above 19200.
777. `int modbus_dispatch(const uint8_t *f, size_t n)` — read/write holding registers, exception responses.
778. `size_t modbus_build_exception(uint8_t addr, uint8_t fn, uint8_t code, uint8_t *out)` — `fn | 0x80`.
779. `int nmea_parse(char *line, char **fields, size_t max)` — XOR checksum, empty fields preserved.
780. Show why `strtok` breaks 779: parse `$GPGGA,,,,,,0` both ways and compare the field positions.
781. `bool nmea_coord_to_udeg(const char *field, char hemi, int32_t *udeg)` — DDMM.mmmm to microdegrees.
782. `void at_feed(char c)` — line assembly with bounded length and overflow resync.
783. `void at_dispatch(const char *line)` — prefix table, longest match first.
784. Show the ordering bug in 783: put `"AT"` first in the table and demonstrate that `AT+CSQ` misroutes.
785. `bool at_is_unsolicited(const char *line)` — separate URCs from command responses.
786. `int tlv_walk(const uint8_t *b, size_t n, tlv_cb cb, void *ctx)` — bounds-check with subtraction, not addition.
787. Write the overflowing version of 786's length check and find a `vlen` that defeats it.
788. `bool tlv_find(const uint8_t *b, size_t n, uint8_t type, const uint8_t **val, uint16_t *vlen)`.
789. `size_t tlv_build(uint8_t *out, size_t cap, uint8_t type, const uint8_t *val, uint16_t n)` — with extended length.
790. `size_t b64_encode(const uint8_t *in, size_t n, char *out, size_t cap)` — correct padding for n%3 of 1 and 2.
791. `size_t b64_decode(const char *in, size_t n, uint8_t *out, size_t cap)` — reject invalid characters and bad length.
792. `size_t b64url_encode(const uint8_t *in, size_t n, char *out, size_t cap)` — `-` and `_` substitution.
793. `int xmodem_recv(uint8_t *dst, size_t cap)` — block numbers, complements, retries, and the duplicate-block case.
794. Show the duplicate-block bug: store a resent block and demonstrate the corrupted output offset.
795. `int xmodem_send(const uint8_t *src, size_t n)` — the transmit side with ACK/NAK handling.

## 3.8 State machines

796. Define a transition **table** for a 5-state, 4-event machine; write `fsm_dispatch` with bounds checks.
797. Show the danger: pass an out-of-range event to a table without bounds checks and observe the result.
798. Convert 796 to a function-pointer-per-state implementation; compare readability and size.
799. Add entry and exit actions to 796 so they fire exactly once per state change.
800. `void fsm_queue_event(struct fsm *f, enum event e)` — queue events so a transition cannot re-enter.
801. `void hsm_dispatch(struct hsm *h, enum event e)` — walk the parent chain until an event is consumed.
802. Add an `E_ESTOP` handler on a parent state and verify all three children inherit it.
803. `size_t hsm_unhandled_count(const struct hsm *h)` — count events that reached the top; explain why.
804. Implement entry/exit ordering for a transition that crosses two levels of hierarchy.
805. `bool thermostat_update(struct thermo *t, int16_t temp_x10)` — hysteresis with asymmetric thresholds.
806. Add a minimum off-time to 805; explain why the deadband alone does not bound the cycle rate.
807. `bool debounce_n(struct deb *d, bool raw)` — N agreeing samples with an edge flag.
808. `bool debounce_multi(struct debm *d, uint16_t port)` — debounce 16 pins in parallel with word operations.
809. `uint8_t multi_tap(struct tap *t, bool pressed, uint32_t now_ms)` — detect single, double, and long press.
810. `void retry_tick(struct retry *r)` — bounded attempts, exponential backoff with a cap, **plus jitter**.
811. Show the thundering herd: run 810 without jitter across 100 simulated devices and plot the retry collisions.
812. `void blink_pattern(struct blink *b, uint32_t now_ms)` — table-driven on/off pattern engine.
813. `void ota_tick(struct ota *o)` — erase, download, verify, commit; power-fail safe at every step.
814. Explain why the validity marker in 813 must be a single word and must be written last.
815. `void bootloader_fsm(void)` — check marker, validate CRC, set VTOR, jump; handle every failure.
816. `void charger_fsm(struct chg *c)` — trickle, constant current, constant voltage, done, fault.
817. `void bldc_commutate(struct bldc *m, uint8_t hall)` — six-step commutation table from Hall inputs.
818. `void modbus_master_fsm(struct mm *m)` — request, wait, timeout, retry, next.
819. `void ble_pairing_fsm(struct ble *b, enum ev e)` — idle, advertising, connected, bonding, error.
820. `void menu_fsm(struct menu *m, enum key k)` — hierarchical menu with back navigation.

## 3.9 Complete drivers

821. `void uart_isr(void)` — RX to a ring buffer, TX from a ring buffer, and **every** error flag cleared.
822. Show the lockup: leave `TXEIE` enabled with an empty buffer and explain the symptom.
823. Show the permanent failure: omit the `ORE` clear and demonstrate that reception never recovers.
824. `size_t uart_send(const uint8_t *b, size_t n)` — enqueue and kick the transmit interrupt.
825. `void uart_dma_rx_start(void)` — circular DMA plus idle-line interrupt.
826. `void uart_idle_isr(void)` — compute the write position from `NDTR`; handle the wrap as two segments.
827. `void uart_dma_tx(const uint8_t *b, size_t n)` — one-shot DMA transmit with a completion callback.
828. `void rs485_send(const uint8_t *b, size_t n)` — assert DE, send, wait for `TC`, release DE.
829. Show the truncation: release DE on `TXE` instead of `TC` and capture the missing final character.
830. `int spi_dma_xfer(const uint8_t *tx, uint8_t *rx, size_t n)` — full-duplex DMA with two streams.
831. `int spi_flash_read(uint32_t addr, uint8_t *b, size_t n)` — command, address, dummy, data.
832. `int spi_flash_page_write(uint32_t addr, const uint8_t *b, size_t n)` — write enable, program, poll busy.
833. `int spi_flash_sector_erase(uint32_t addr)` — erase and poll to completion with a timeout.
834. `int sdcard_init_spi(void)` — the CMD0/CMD8/ACMD41 initialisation sequence.
835. `int i2c_eeprom_write_page(uint16_t addr, const uint8_t *b, size_t n)` — respect the page boundary.
836. `int i2c_eeprom_read(uint16_t addr, uint8_t *b, size_t n)` — write address, repeated start, read.
837. `int i2c_eeprom_wait_ready(uint32_t timeout_ms)` — ACK polling during the internal write cycle.
838. `int mpu6050_init(void)` — WHO_AM_I check, wake from sleep, set ranges.
839. `int mpu6050_read_accel(int16_t *x, int16_t *y, int16_t *z)` — burst read, big-endian, sign-extended.
840. `int bme280_read_compensated(int32_t *t, uint32_t *p, uint32_t *h)` — apply the datasheet compensation in integers.
841. `void adc_dma_start(uint16_t *buf, size_t n)` — circular, `DDS` set, half and full interrupts.
842. `void adc_dma_isr(void)` — process the half the DMA is not writing; handle the transfer-error flag.
843. `void adc_scan_config(const uint8_t *channels, size_t n)` — sequence registers, then de-interleave the results.
844. `uint32_t adc_read_vdda_mv(void)` — from `VREFINT` and its factory calibration value.
845. `int32_t adc_read_temp_c_x10(void)` — internal temperature sensor with both calibration points.
846. `void pwm_complementary_init(volatile TIM_T *t, uint16_t dead_ns)` — with dead-time insertion.
847. Explain what happens without dead time in 846, in terms of the half-bridge.
848. `void pwm_center_aligned(volatile TIM_T *t)` — configure and state the harmonic benefit.
849. `uint32_t encoder_read_hw(volatile TIM_T *t)` — hardware encoder mode; no missed steps.
850. `int8_t encoder_decode_sw(uint8_t ab)` — software quadrature table returning −1/0/+1.
851. `size_t encoder_error_count(void)` — count illegal transitions; explain what a rising count means.
852. `void can_init(volatile CAN_T *c, uint32_t bitrate)` — compute BRP, BS1, BS2 for the target bitrate.
853. `int can_transmit(volatile CAN_T *c, uint32_t id, const uint8_t *d, uint8_t dlc)` — with mailbox selection.
854. `int can_filter_config(volatile CAN_T *c, uint32_t id, uint32_t mask)` — mask mode acceptance filter.
855. `void can_error_isr(void)` — handle error-passive and bus-off; implement recovery.

## 3.10 Interrupts and concurrency patterns

856. `uint32_t crit_enter(void)` and `crit_exit(uint32_t)` — save and restore, correct under nesting.
857. Show the nesting bug: use an unconditional `__enable_irq()` on exit and demonstrate the exposure.
858. `void crit_enter_basepri(uint8_t prio)` — mask by priority instead of globally; state the constraint.
859. `uint32_t atomic_inc(volatile uint32_t *p)` — LDREX/STREX retry loop.
860. `bool atomic_cas(volatile uint32_t *p, uint32_t expect, uint32_t desired)` — with `__CLREX` on the early exit.
861. Show the lost update: increment a shared counter from an ISR and a task without protection; measure the drift.
862. `uint64_t micros_read(void)` — read a 64-bit software counter safely from task context.
863. Show the backwards jump: read it naively and construct the interleaving that produces it.
864. `void isr_flag_handoff(void)` — payload barrier before the flag; consumer barrier after reading it.
865. `void isr_deferred(void)` — top half captures, `PendSV` bottom half interprets.
866. `void pendsv_handler(void)` — drain the queue filled by 865.
867. `void isr_log(uint8_t id, uint32_t arg)` — binary record to a ring buffer; count drops, never block.
868. `void log_drain(void)` — format and emit from task context; report the dropped count.
869. `void debounce_isr(void)` — timestamp-based rejection inside an EXTI handler; no delay.
870. `void wait_for_event(void)` — disable interrupts, test, `WFI`, enable; explain the race it closes.
871. Show the lost wakeup: write the naive test-then-`WFI` and describe the failure window.
872. `void sem_give_from_isr(void)` — the `FromISR` API plus yield-from-ISR.
873. Explain what happens if the yield is omitted, in milliseconds.
874. `void exti_config_falling(uint8_t pin)` — clear pending before unmasking; unmask last.
875. `void dma_pingpong_isr(void)` — half-transfer and transfer-complete, plus the error branch.
876. `bool dma_buffer_ok(const void *p, size_t n)` — 32-byte aligned and a multiple of 32; explain why on M7.
877. `void dcache_before_dma_read(void *p, size_t n)` — invalidate; say what happens if you skip it.
878. `void dcache_after_dma_write(void *p, size_t n)` — clean; say what happens if you skip it.
879. `uint32_t isr_latency_max(void)` — measure worst-case latency with `DWT->CYCCNT` over a long run.
880. Enumerate every contributor to the number in 879 and identify which one you control.

## 3.11 Applied algorithms

881. `size_t lis_length(const int32_t *a, size_t n)` — longest increasing subsequence, O(n log n).
882. `size_t edit_distance(const char *a, const char *b)` — two-row DP, bounded memory.
883. `bool subset_sum(const uint16_t *a, size_t n, uint16_t target)` — bitset DP, no allocation.
884. `size_t knapsack(const uint16_t *w, const uint16_t *v, size_t n, uint16_t cap)` — one-dimensional DP.
885. `size_t coin_change_min(const uint16_t *coins, size_t n, uint16_t amount)` — fewest coins; 0 if impossible.
886. `size_t lcs_length(const char *a, const char *b)` — longest common subsequence, rolling rows.
887. `void bfs_grid(const uint8_t g[16][16], uint8_t sr, uint8_t sc, uint16_t *dist)` — queue-based flood.
888. `bool dfs_path_exists(const struct graph *g, uint8_t a, uint8_t b)` — explicit stack, no recursion.
889. `bool topo_sort(const struct graph *g, uint8_t *order)` — Kahn's algorithm; detect a cycle.
890. `uint32_t dijkstra(const struct wgraph *g, uint8_t src, uint8_t dst)` — binary heap, static pool.
891. `uint32_t mst_weight(const struct wgraph *g)` — Kruskal with union-find.
892. `size_t connected_components(const struct graph *g)` — count using DSU.
893. `bool graph_is_bipartite(const struct graph *g)` — two-colouring by BFS.
894. `size_t schedule_greedy(struct task *t, size_t n)` — maximum non-overlapping intervals.
895. `bool rate_monotonic_feasible(const struct rtask *t, size_t n)` — utilisation bound test.
896. `uint32_t response_time(const struct rtask *t, size_t n, size_t i)` — iterative response-time analysis.
897. `size_t huffman_build(const uint32_t *freq, size_t n, struct hnode *pool)` — build a code tree with a heap.
898. `size_t huffman_encode(const uint8_t *in, size_t n, const struct hcode *tab, uint8_t *out)` — bit-level output.
899. `size_t lz77_compress(const uint8_t *in, size_t n, uint8_t *out, size_t cap)` — small sliding window.
900. `void self_test_tier3(void)` — assert-test forty Tier 3 functions, including three round-trip pairs.

---

# Tier 4 — Systems

**Exercises 901–1200.** Components that interact. Everything here has more than one correct design, so most exercises ask you to build one *and* state what it costs. Expect 30–90 minutes each, and expect several to need the ARM architecture reference manual rather than a device datasheet.

## 4.1 Scheduler and task management

901. Define a TCB with `sp` as the **first** member; explain why the assembly depends on that.
902. `int task_create(void (*entry)(void), uint32_t *stack, size_t words, uint8_t prio)` — forge an initial exception frame.
903. Explain each of the eight words the hardware stacks and the eight you must stack yourself.
904. Write `PendSV_Handler` in assembly: save R4–R11, swap PSP, restore, `BX LR`.
905. Explain `EXC_RETURN = 0xFFFFFFFD` and what changes if you use `0xFFFFFFF9`.
906. Show the bring-up failure: clear the Thumb bit in the forged `xPSR` and describe the fault.
907. `void sched_start(void)` — set PSP, switch to unprivileged thread mode, launch the first task.
908. `void task_yield(void)` — pick the next task and set `PENDSVSET` only if it differs.
909. Explain why `PendSV` is set to the lowest priority rather than the highest.
910. `void sched_make_ready(struct tcb *t)` — insert into a per-priority list and set the bitmap bit.
911. `struct tcb *sched_pick_highest(void)` — O(1) selection via `ctz` on the ready bitmap.
912. Extend 911 to 256 priority levels with a two-level bitmap; state the extra cost.
913. `void sched_tick(void)` — time-slice rotation among equal-priority tasks.
914. `void task_sleep_ms(uint32_t ms)` — block on a delay list ordered by wake time.
915. `void sched_wake_expired(uint32_t now)` — move due tasks from the delay list to ready.
916. `void task_suspend(struct tcb *t)` and `task_resume(struct tcb *t)` — with correct ready-queue bookkeeping.
917. `void task_set_priority(struct tcb *t, uint8_t p)` — requeue safely, including for the running task.
918. Implement a **cooperative** scheduler with no preemption; state exactly which races it removes.
919. Convert 918 to preemptive; enumerate every shared structure that now needs protection.
920. `void idle_task(void)` — `WFI` with the race-free masking pattern.
921. `uint8_t cpu_load_percent(void)` — measure idle time over one second and report utilisation.
922. `void runtime_stats(struct tcb *t, uint32_t *ticks)` — per-task CPU accounting at each switch.
923. `void task_list_dump(void)` — print name, state, priority, and stack high-water for every task.
924. `void stack_paint(uint32_t *base, size_t words)` and `stack_high_water(...)` — measured stack sizing.
925. `bool stack_check(const struct tcb *t)` — bottom-word canary, cheap enough for production.
926. Configure an MPU region below each task stack so an overflow faults immediately.
927. Show the silent corruption: overflow a task stack without the MPU and identify what it damaged.
928. `void tickless_idle(uint32_t idle_ticks)` — reprogram the tick for the whole idle period.
929. Complete 928 by catching the tick counter up on wake; show the drift if you omit it.
930. Handle the case where 928 wakes early on an unrelated interrupt; compute the true elapsed time.
931. `void sched_lock(void)` and `sched_unlock(void)` — suspend switching without disabling interrupts.
932. Explain the difference between 931 and a critical section, and when each is correct.
933. `bool feasible_rate_monotonic(const struct rtask *t, size_t n)` — utilisation bound.
934. `uint32_t worst_case_response(const struct rtask *t, size_t n, size_t i)` — iterative analysis.
935. Given three tasks and their periods and WCETs, prove or disprove schedulability by hand, then verify in code.

## 4.2 Synchronisation primitives

936. `bool sem_take(struct sem *s, uint32_t timeout)` — block, timeout, and re-check on wake.
937. `void sem_give(struct sem *s)` — hand the token directly to the highest-priority waiter.
938. Show token theft: increment and wake instead of handing over, then construct the starvation case.
939. Extend 936 to a counting semaphore; show the event loss a binary one suffers.
940. `bool mutex_lock(struct mutex *m, uint32_t timeout)` — with an owner field.
941. `void mutex_unlock(struct mutex *m)` — reject a release by a non-owner.
942. Add priority inheritance to 940; verify the owner's priority is restored on unlock.
943. Construct the priority inversion scenario with three tasks and measure the blocking time.
944. Repeat 943 with inheritance enabled and show the bound is now the critical-section length.
945. Implement priority **ceiling** instead; compare determinism and pessimism against inheritance.
946. `bool rmutex_lock(struct rmutex *m, uint32_t timeout)` — recursive, with a depth counter.
947. Show the self-deadlock a plain mutex causes in the same call pattern.
948. Refactor a module to use `_locked` internal functions instead of recursion; compare.
949. `bool q_send(struct mq *q, const void *item, uint32_t timeout)` — copy by value, block when full.
950. `bool q_recv(struct mq *q, void *item, uint32_t timeout)` — block when empty, re-check on wake.
951. Show the spurious-wakeup bug: use `if` instead of `while` around the wait and construct the failure.
952. `bool q_send_front(struct mq *q, const void *item)` — high-priority message jumps the queue.
953. `bool q_peek(const struct mq *q, void *item)` — inspect without consuming.
954. Implement a **stream** buffer (byte-oriented, variable length) rather than a message queue.
955. `uint32_t ev_wait(struct evgroup *g, uint32_t mask, bool all, uint32_t timeout)` — AND and OR modes.
956. `void ev_set(struct evgroup *g, uint32_t bits)` — may wake several waiters in one call.
957. Resolve the clear-on-exit question in 955 three ways; pick one and justify it.
958. `bool rwlock_read_lock(struct rw *l)` and `rwlock_write_lock(struct rw *l)` — reader/writer lock.
959. Show writer starvation in 958; add a policy that prevents it.
960. `bool barrier_wait(struct barrier *b, size_t n)` — release all participants when the n-th arrives.
961. `void mailbox_post(struct mbox *m, uint32_t v)` — single-slot overwrite mailbox with a valid flag.
962. Implement a task notification (single 32-bit value per task) and compare its cost with a semaphore.
963. Solve dining philosophers with five mutexes and no deadlock; state which condition you broke.
964. Add lock ranking with a debug-build assertion that catches out-of-order acquisition.
965. `bool try_lock_both(struct mutex *a, struct mutex *b)` — backoff instead of blocking; note the livelock risk.

## 4.3 Lock-free and atomics

966. `bool rb_write_spsc(struct rb *r, uint8_t b)` — one writer per index, `volatile`, `DMB` before publishing.
967. State the four properties that make 966 correct without a lock.
968. Show what breaks if you add a shared `count` field to 966.
969. Show what breaks if you remove the barrier; describe the window on a core with a store buffer.
970. `bool rb_write_overwrite(struct rb *r, uint8_t b)` — and explain why it invalidates the lock-free proof.
971. Fix 970 so the producer never writes `tail`: expose a dropped-bytes counter instead.
972. `uint32_t atomic_fetch_add(volatile uint32_t *p, uint32_t v)` — LDREX/STREX loop.
973. `bool atomic_cas(volatile uint32_t *p, uint32_t exp, uint32_t des)` — with `__CLREX` on the early exit.
974. Show what goes wrong if `__CLREX` is omitted in 973.
975. `void *lf_stack_pop(struct lfstack *s)` — lock-free LIFO via CAS on the head.
976. Describe the ABA hazard in 975 and explain why the Cortex-M exclusive monitor closes it.
977. Construct an ABA failure on a hypothetical machine without the monitor guarantee.
978. `bool lf_queue_push(struct lfq *q, uint32_t v)` — bounded MPSC queue with CAS on the tail.
979. `bool lf_queue_pop(struct lfq *q, uint32_t *v)` — single consumer, no CAS needed on this side.
980. Explain why MPMC is materially harder than MPSC and name what it requires.
981. `void seqlock_write(struct seq *s, const struct data *d)` — increment, write, increment.
982. `bool seqlock_read(const struct seq *s, struct data *d)` — retry while the counter is odd or changed.
983. Compare 981/982 with a critical section for a frequently-read, rarely-written structure.
984. `uint64_t counter64_read(void)` — lock-free read of a 64-bit counter written by an ISR.
985. `void atomic_bitset(volatile uint32_t *p, uint8_t bit)` — via bit-band, then via LDREX; compare.
986. Show that a bit-field write is a read-modify-write and therefore not ISR-safe.
987. `bool ticket_lock(struct ticket *t)` — FIFO-fair spinlock using two counters.
988. Explain why a spinlock is almost always wrong on a single-core MCU.
989. Implement a memory barrier macro set (`DMB`, `DSB`, `ISB`) and document when each is required.
990. Write a test that fails without a barrier and passes with one; explain why it is hard to make deterministic.

## 4.4 Memory protection and the MPU

991. `void mpu_configure_region(uint8_t n, uintptr_t base, uint8_t size_log2, uint32_t attrs)` — one region.
992. Explain the base-address alignment requirement and what happens if it is violated.
993. `void mpu_enable(void)` — with the `DSB`/`ISB` sequence and the background-region decision.
994. Configure a no-access region below each task stack; verify it faults on overflow.
995. Configure flash as read-only and execute; show that a write attempt faults.
996. Configure SRAM as non-executable; demonstrate that a jump into a buffer faults.
997. Configure a non-cacheable region for DMA buffers; verify no maintenance is then needed.
998. Explain MPU region overlap precedence and construct a case where the result surprises you.
999. `void mpu_task_switch(const struct tcb *t)` — reprogram per-task regions on each context switch.
1000. Measure the added context-switch cost of 999 in cycles.
1001. `void mpu_dump(void)` — print every configured region in human-readable form.
1002. Implement a privileged/unprivileged split: tasks run unprivileged, system calls via `SVC`.
1003. `void SVC_Handler(void)` — decode the immediate from the instruction and dispatch.
1004. Explain why the SVC number must be read from the stacked PC, not from a register.
1005. `void heap_guard_init(void)` — place guard regions either side of the heap; detect overrun.
1006. `bool ptr_is_in_task_memory(const struct tcb *t, const void *p, size_t n)` — validate a syscall argument.
1007. Show the confused-deputy problem: pass a kernel address from unprivileged code and block it.
1008. `void stack_limit_set(uintptr_t limit)` — use `PSPLIM` on ARMv8-M; compare with the MPU approach.
1009. Configure an MPU region to catch a null-pointer dereference; explain why address 0 is often valid flash.
1010. Enumerate the trade-offs of MPU use in a small system: RAM cost, cycles, complexity, benefit.

## 4.5 Startup, linker, and the C runtime

1011. Write a minimal vector table as a `const` array of function pointers in its own section.
1012. Explain why entry 0 is the initial stack pointer and not a handler.
1013. Write `Reset_Handler` in C: copy `.data`, zero `.bss`, call `SystemInit`, call `main`.
1014. Write a linker script defining `.text`, `.rodata`, `.data`, `.bss`, and the stack.
1015. Define `_sidata`, `_sdata`, `_edata`, `_sbss`, `_ebss` and use them in 1013.
1016. Explain what happens if `.data` initialisation is skipped, with a concrete example variable.
1017. Explain what happens if `.bss` is not zeroed, and which standard guarantee is violated.
1018. Place a function in RAM with a section attribute; verify from the map file that it is there.
1019. Explain when a RAM function is necessary, using self-programming flash as the example.
1020. Add a `.noinit` section that survives a reset; use it for a crash counter.
1021. Place a variable at an absolute address with the linker; explain a use for it.
1022. Compute total RAM usage from the map file: `.data` + `.bss` + stack + heap.
1023. Find the ten largest symbols in your image from the map file; write the command you used.
1024. Add `-ffunction-sections -fdata-sections` and `--gc-sections`; measure the size reduction.
1025. Explain why `--gc-sections` cannot remove a function referenced only from a table.
1026. Implement `__libc_init_array` handling so constructor-like initialisers run.
1027. Provide the minimal newlib stubs: `_write`, `_sbrk`, `_exit`; explain what each is for.
1028. Implement `_sbrk` with a hard limit that fails rather than colliding with the stack.
1029. Show heap/stack collision without that limit; describe the symptom.
1030. Write a second linker script for a bootloader at 0x08000000 and an app at 0x08008000.
1031. Explain every change needed in the app's script and startup code for 1030.
1032. Compute and verify the vector-table alignment requirement for the relocated app.
1033. Add a build-time version string and CRC placeholder to a known offset in the image.
1034. Write a post-build step that computes the image CRC and patches it into 1033.
1035. Verify the patched CRC at runtime from the running image; explain why the placeholder must be excluded.

## 4.6 Bootloader and OTA

1036. `bool image_header_valid(const struct img_hdr *h)` — magic, length, version, CRC field sanity.
1037. `bool image_crc_ok(uint32_t base, const struct img_hdr *h)` — read back from flash, chunked.
1038. `void jump_to_app(uintptr_t base)` — disable interrupts, set VTOR, set MSP, jump.
1039. Explain each of the four steps in 1038 and the failure mode if any is skipped.
1040. Show the pending-interrupt hazard in 1038 and add the code that clears it.
1041. Implement A/B slot selection: pick the newest valid image; fall back if it fails.
1042. `void ota_begin(uint32_t len, uint32_t crc)` — erase the inactive slot, record the expected values.
1043. `int ota_chunk(uint32_t off, const uint8_t *d, size_t n)` — bounds-check the offset against slot capacity.
1044. `int ota_finalise(void)` — verify from flash, then write the validity marker as one word.
1045. Explain precisely why the marker must be a single word and must be written last.
1046. Simulate power loss at ten different points in the OTA sequence; verify the device still boots each time.
1047. Implement a boot-confirm flag: the new image must set it within N seconds or the bootloader reverts.
1048. Show the failure 1047 catches that CRC verification cannot.
1049. `uint8_t boot_attempt_count(void)` — persist and increment; enter recovery after three failures.
1050. Implement a recovery mode entered by a GPIO strap; document the pin and its pull.
1051. `int xmodem_bootloader(void)` — receive an image over UART with retries and a size limit.
1052. Add a signature check to 1044: verify an ECDSA or RSA signature over the header and payload.
1053. Explain why the public key must be immutable and where it should live.
1054. Explain the difference between what a CRC proves and what a signature proves.
1055. Implement anti-rollback: refuse an image whose version is below a monotonic counter.
1056. Store that counter so it cannot be reverted by an attacker with flash write access; discuss options.
1057. Implement a chain of trust: ROM verifies the bootloader, which verifies the app.
1058. Explain what breaks the chain if the bootloader is field-updatable, and how to handle it.
1059. `void secure_boot_fail(void)` — decide and justify the behaviour: halt, recover, or wipe.
1060. Measure the boot-time cost of signature verification and state whether it is acceptable for your product.

## 4.7 Security primitives

1061. `bool ct_equal(const uint8_t *a, const uint8_t *b, size_t n)` — constant time, no early return.
1062. Measure the timing difference between `memcmp` and 1061 on mismatched-at-byte-0 versus byte-31 inputs.
1063. Explain how the measurement in 1062 reduces a 128-bit brute force, with numbers.
1064. `void secure_zero(void *p, size_t n)` — a clear the compiler cannot eliminate.
1065. Prove the naive `memset` is removed at `-O2` by inspecting the disassembly.
1066. Enumerate every place a key may still exist after 1064 and how to reduce the count.
1067. `void aes128_key_expand(const uint8_t key[16], uint8_t rk[176])` — key schedule only.
1068. `void aes128_encrypt_block(const uint8_t rk[176], const uint8_t in[16], uint8_t out[16])`.
1069. Explain why a table-based AES S-box is vulnerable to cache timing, and when that matters on an MCU.
1070. `void aes_ctr_crypt(const uint8_t rk[176], uint8_t nonce[16], uint8_t *buf, size_t n)` — stream mode.
1071. Explain what happens if a CTR nonce is reused with the same key.
1072. `void aes_cbc_mac(const uint8_t rk[176], const uint8_t *d, size_t n, uint8_t mac[16])`.
1073. Explain why CBC-MAC is insecure for variable-length messages and what CMAC changes.
1074. `void sha256_init/update/final(...)` — streaming SHA-256 over a firmware image.
1075. `void hmac_sha256(const uint8_t *key, size_t kn, const uint8_t *d, size_t n, uint8_t out[32])`.
1076. Explain why HMAC needs two hash passes and what a naive `hash(key || msg)` is vulnerable to.
1077. `bool verify_tag(const uint8_t *msg, size_t n, const uint8_t *tag)` — hash then constant-time compare.
1078. `uint32_t rng_from_adc_noise(void)` — collect entropy; then explain why this is usually inadequate.
1079. `void csprng_init(const uint8_t seed[32])` and `csprng_bytes(...)` — a hash-based DRBG.
1080. Explain the consequence of seeding a CSPRNG from a device serial number.
1081. Implement key storage in a dedicated flash sector; describe how you would protect it.
1082. Explain readout protection (RDP) levels and what each actually prevents.
1083. Explain the debug-port lockout trade-off: field diagnostics versus key extraction.
1084. Implement a fault-injection countermeasure: verify a security-critical branch twice.
1085. Explain why a single-bit glitch on a comparison result is a realistic attack on a boot check.

## 4.8 Protocol stacks

1086. Design a three-layer stack (PHY, MAC, APP) with a `struct` interface per layer; write the headers only.
1087. `bool phy_send(struct phy *p, const uint8_t *d, size_t n)` — with an ops table and a context pointer.
1088. Implement zero-copy buffer passing between layers using a reference-counted pool.
1089. Explain the ownership rule at each layer boundary and where the reference is dropped.
1090. `void mac_rx(struct mac *m, struct pbuf *b)` — address filtering, then hand upward or free.
1091. Implement a retransmission timer with exponential backoff and a maximum attempt count.
1092. `bool seq_window_accept(struct win *w, uint16_t seq)` — sliding receive window with duplicate rejection.
1093. Handle sequence-number wraparound in 1092 correctly; construct the failing naive comparison.
1094. Implement selective acknowledgement: a bitmap of received sequence numbers.
1095. `size_t reassemble(struct reasm *r, uint16_t off, const uint8_t *d, size_t n)` — fragment reassembly.
1096. Add a reassembly timeout and a maximum fragment count to 1095; explain the DoS you just prevented.
1097. Detect and reject overlapping fragments in 1095; explain the attack that motivates it.
1098. Implement flow control: a receive-window advertisement the sender must respect.
1099. Implement congestion backoff distinct from flow control; explain the difference.
1100. `void keepalive_tick(struct link *l)` — detect a dead peer without generating excess traffic.
1101. Implement a request/response transaction layer with IDs, timeouts, and out-of-order matching.
1102. Handle a late response arriving after its transaction timed out; explain why it must be discarded.
1103. Implement a publish/subscribe layer over 1101 with a bounded subscriber table.
1104. Implement message prioritisation so a control frame preempts queued bulk data.
1105. Implement a duplicate-suppression cache keyed on source and sequence.
1106. `size_t stack_overhead(size_t payload)` — total bytes on the wire for your stack; tabulate per layer.
1107. Instrument every layer with counters: sent, received, dropped, retried, malformed.
1108. Implement a loopback PHY so the whole stack can be tested on a host with no hardware.
1109. Write a host-side test that drives 1108 through packet loss, duplication, and reordering.
1110. Fuzz your MAC parser through 1108; report what you found or that you found nothing and why.
1111. Implement Modbus TCP framing (MBAP header) on top of your transaction layer.
1112. Implement a CoAP-style option parser using the TLV walker from Tier 3.
1113. Implement MQTT-SN CONNECT and PUBLISH encoding with the variable-length field.
1114. Implement a LoRaWAN-style frame counter with replay rejection.
1115. Compare your stack's RAM cost against lwIP's minimum footprint; state where the difference comes from.

## 4.9 Power management

1116. `void enter_sleep(void)` — `WFI` with the race-free masking sequence.
1117. `void enter_stop(void)` — configure wake sources, enter stop mode, restore clocks on wake.
1118. Enumerate what state is lost in each sleep mode on your target; write it as a table.
1119. `void wake_source_config(uint8_t src)` — configure an EXTI, RTC alarm, and watchdog wake.
1120. `uint8_t wake_reason(void)` — determine what woke the device; clear the flags.
1121. Measure current in run, sleep, stop, and standby; tabulate against the datasheet figures.
1122. Explain any discrepancy in 1121; the usual cause is a floating or driven-high pin.
1123. `void gpio_park_all(void)` — put every unused pin in its lowest-power state before sleeping.
1124. `void peripheral_clock_gate(uint32_t mask, bool on)` — disable clocks for idle peripherals.
1125. Measure the saving from 1124 for one peripheral; state whether it justified the complexity.
1126. `void lptim_wake_after(uint32_t ms)` — use a low-power timer to wake from deep sleep.
1127. Implement tickless idle using 1126 rather than SysTick; handle the tick catch-up.
1128. `uint32_t energy_budget_uah(const struct duty *d, size_t n)` — compute daily consumption from a duty cycle table.
1129. Use 1128 to compute battery life for a given cell capacity; state your assumptions.
1130. Implement batched transmission: buffer N messages and send once; measure the energy saving.
1131. Implement adaptive sampling: reduce the rate when readings are stable.
1132. Explain the risk in 1131 and add a maximum interval that bounds it.
1133. Implement a brown-out detector response: save critical state before the supply collapses.
1134. Compute how much time you have in 1133 from the bulk capacitance and the current draw.
1135. Implement clock scaling: reduce the core clock under light load and restore it; handle the UART baud.

## 4.10 Testing and instrumentation

1136. Write a minimal assert-based test harness: `TEST(name)`, `RUN`, pass/fail counting, a summary.
1137. Add per-test setup and teardown to 1136.
1138. Structure one module as pure logic plus a thin hardware layer; compile the logic for the host.
1139. Write a mock `struct uart_ops` and test your protocol parser on a host with no hardware.
1140. Inject time: replace direct `g_ticks` reads with a function pointer; test a timeout instantly.
1141. Write a test that advances mock time across a tick-counter wraparound; verify no misbehaviour.
1142. Write a table-driven test for your CRC functions using published check values.
1143. Write a round-trip property test: encode then decode 10 000 random buffers; assert equality.
1144. Write a fuzz harness for your frame parser: random bytes, assert no crash and no out-of-bounds write.
1145. Run 1144 under `-fsanitize=address,undefined`; report the first failure or explain why there was none.
1146. Add coverage instrumentation; report line coverage for your parser and identify the untested branch.
1147. Write a fault-injection mock: make `i2c_read` fail on the third call and verify the driver recovers.
1148. Write a mock that returns a NACK, then a timeout, then success; assert the retry count.
1149. Build a hardware-in-the-loop harness: a host script drives the target over UART and checks responses.
1150. Add a target-side command interface so 1149 can trigger internal states directly.
1151. Instrument a function with `DWT->CYCCNT`; report min, max, and mean over 10 000 calls.
1152. Explain why the maximum matters more than the mean for anything in an ISR.
1153. Toggle a GPIO at ISR entry and exit; capture with a logic analyser and measure jitter.
1154. Compare the GPIO measurement with the cycle-counter measurement; explain any difference.
1155. Implement a binary event log with timestamps; write a host-side decoder for it.
1156. Add an assertion handler that records file, line, and the register context to `.noinit`, then resets.
1157. Recover and print that record on the next boot; verify it survives a watchdog reset.
1158. Implement a runtime histogram of ISR execution times using power-of-two buckets.
1159. Implement a stack high-water report for every task, printed on demand.
1160. Write a test that deliberately overflows one task's stack and asserts your detector catches it.

## 4.11 Performance

1161. Profile a function three ways: cycle counter, GPIO toggle, and sampling; compare the results.
1162. Measure `memcpy` throughput for aligned and unaligned buffers; tabulate bytes per cycle.
1163. Implement the word-loop `memcpy` and measure the speedup; state the alignment condition.
1164. Replace a byte-at-a-time CRC with a 256-entry table; measure both size and speed changes.
1165. Replace it with a 16-entry table instead; decide which you would ship and why.
1166. Convert a runtime division by a constant into a multiply-and-shift; verify exactness across the range.
1167. Replace `sin`/`cos` with a quarter-wave table plus interpolation; measure the error and the speedup.
1168. Convert per-byte UART interrupts to circular DMA plus idle line; measure the CPU saving.
1169. Compute the interrupt rate before and after 1168 at 115200 baud; state both numbers.
1170. Move a hot function to RAM; measure the difference and explain it in terms of flash wait states.
1171. Enable the instruction cache or prefetch buffer; measure the effect on a loop-heavy workload.
1172. Reorder a struct to eliminate padding; report the size before and after and the array-wide saving.
1173. Convert eight `bool` flags to a bit-field; report the saving and note the atomicity you lost.
1174. Move a large lookup table to `const`; verify from the map file that RAM usage dropped.
1175. Replace `printf` with fixed-purpose formatters; report the flash saving.
1176. Compare `-O0`, `-Os`, `-O2`, `-O3` on your image: size and one benchmark each; pick one and justify it.
1177. Find a case where `-O3` is larger and slower than `-O2`; explain the likely cause.
1178. Unroll a hot loop by four; measure whether it actually helped.
1179. Add `restrict` to a copy loop; check the disassembly for a change and state what you promised.
1180. Identify your system's worst-case interrupt latency by measurement, and name what dominates it.

## 4.12 Fault handling and diagnostics

1181. `void HardFault_Handler(void)` — capture the stacked frame and store it before doing anything else.
1182. Write the assembly stub that determines whether MSP or PSP was in use and passes the correct pointer.
1183. Decode `CFSR`, `HFSR`, and `MMFAR`/`BFAR`; print a human-readable cause.
1184. Deliberately cause an unaligned access fault and confirm your decoder identifies it.
1185. Deliberately cause a bus fault by reading an unmapped address; confirm the decode.
1186. Deliberately cause a usage fault via a divide by zero with the trap enabled; confirm the decode.
1187. Reconstruct the faulting PC and LR from the stacked frame and map them to symbols using the ELF.
1188. Walk the stack from the fault handler to produce a best-effort backtrace; state its limitations.
1189. Store a crash record in `.noinit` and emit it on the next boot.
1190. Add a crash counter that enters a safe mode after repeated faults.
1191. Distinguish a watchdog reset from a power-on reset from a software reset; log which occurred.
1192. Implement a software reset via `SCB->AIRCR` with the required `DSB` and the spin afterwards.
1193. Show what happens without the spin in 1192.
1194. Implement a graceful degradation policy: on a sensor fault, run on the last known value with a flag.
1195. Implement a fault-tolerant retry wrapper with a classification of retryable versus permanent errors.
1196. Add an error-code enum that distinguishes those two classes; refactor one driver to use it.
1197. Implement a health-monitor task that aggregates subsystem status into one system state.
1198. Implement a diagnostic dump command that prints every counter, high-water mark, and error tally.
1199. Design and document your system's failure policy: what faults are recoverable, what halts, what resets.
1200. `void self_test_tier4(void)` — a power-on self test exercising RAM, flash CRC, and each peripheral.

---

# Tier 5 — Senior

**Exercises 1201–1500.** Three kinds of exercise here, and they are not all code.

- **Builds** — complete subsystems, hours to days each.
- **Proofs** — show that something is correct or bounded, not just that it works once.
- **Arguments** — the deliverable is a written position with the trade-offs stated. Write these out properly; "it depends" is not an answer, and an interviewer asking one of these is assessing exactly this.

Nothing in this tier is a two-minute rep. If you are here, the reps are done.

## 5.1 Lock-free structures and the memory model

1201. Write out the four properties that make an SPSC ring buffer correct, and prove each one holds in your implementation.
1202. Explain acquire and release semantics, and identify precisely which access in your ring buffer needs each.
1203. Rewrite your SPSC buffer using `<stdatomic.h>` with explicit memory orders instead of `volatile` plus `DMB`.
1204. Compare the generated code from 1203 against your hand-written version; explain any difference.
1205. Construct a scenario where `memory_order_relaxed` is sufficient and one where it is not.
1206. Implement a bounded MPSC queue: CAS on the tail, single consumer, no CAS on the read side.
1207. Prove that 1206 cannot lose an element, and identify what it does under contention instead.
1208. Explain why MPMC is qualitatively harder than MPSC and enumerate what it requires.
1209. Implement a bounded MPMC queue with per-slot sequence numbers (Vyukov's algorithm).
1210. Explain the role of the per-slot sequence number in 1209 and why a head/tail pair alone is insufficient.
1211. Implement a lock-free LIFO free list; then construct the ABA failure it would suffer without the exclusive monitor.
1212. Explain how the Cortex-M exclusive monitor closes ABA, and what changes on a multi-core part.
1213. Implement a tagged pointer scheme that closes ABA without the monitor; state the cost.
1214. Implement a seqlock; prove a reader can never observe a torn structure.
1215. Explain why a seqlock is unsuitable if readers must never retry, and give the alternative.
1216. Implement epoch-based reclamation for a structure where nodes are freed; explain the grace-period rule.
1217. Explain hazard pointers and when they beat epoch reclamation.
1218. Implement an RCU-style read-mostly configuration swap: readers never block, the writer publishes a new pointer.
1219. State the exact condition under which the old configuration in 1218 can be freed.
1220. Explain false sharing, and construct a two-task benchmark on a cached core that demonstrates it.
1221. Pad two hot counters to separate cache lines; measure the improvement.
1222. Explain why false sharing does not exist on a Cortex-M0 and does on an M7.
1223. Implement a wait-free single-writer statistics counter read safely by multiple readers.
1224. Prove your implementation of 1223 is wait-free rather than merely lock-free; define both terms.
1225. Write a stress test that runs two contexts against your lock-free structure for an hour and detects any lost or duplicated element.

## 5.2 Allocator design

1226. Implement a segregated-fit allocator with size classes; state the class spacing and justify it.
1227. Implement a two-level bitmap index (TLSF-style) over those classes for O(1) fit selection.
1228. Prove the allocation path in 1227 is O(1) — bounded instruction count, independent of heap state.
1229. Implement boundary tags so the previous block's header is reachable by arithmetic, not by search.
1230. Show the O(n) predecessor scan your earlier coalescing free required, and measure the improvement.
1231. Implement a slab allocator: per-size caches of pre-carved objects with a partial/full/empty list structure.
1232. Explain what a slab buys over a plain fixed-block pool, and when that difference matters.
1233. Implement a region allocator with per-subsystem quotas; a subsystem that exceeds its quota fails, not the system.
1234. Argue for or against quotas versus a shared pool, with a concrete failure scenario for each.
1235. `float fragmentation_ratio(const struct heap *h)` — define a metric, implement it, and justify the definition.
1236. Write a workload that drives your free-list allocator to failure with 60% of memory free.
1237. Run the same workload against the buddy allocator and the pool; tabulate the results.
1238. Implement a compacting allocator using handles instead of pointers; state what it costs callers.
1239. Explain why compaction is generally impossible with raw pointers, and what handles change.
1240. Implement a log-structured allocator for flash; explain how it avoids in-place update.
1241. Implement an emergency reserve pool released only in a defined failure path; justify the size.
1242. Instrument every allocation site with a tag; produce a report of bytes outstanding per site.
1243. Use 1242 to find a leak in a program you deliberately break.
1244. Implement allocation-failure injection: fail the n-th allocation; run your test suite for every n.
1245. Report what 1244 found. Most systems handle the first failure and not the tenth.
1246. Prove a bound on the worst-case allocation latency of your pool allocator, in cycles.
1247. Argue whether a heap belongs in your product at all; give the criteria under which you would allow one.
1248. Design an allocator for a system with 8 KB of RAM and a 30-day uptime requirement; document the choice.
1249. Design one for a system with 512 KB and hard 100 µs deadlines; document how the requirement changed it.
1250. Write the coding-standard rule you would enforce about dynamic allocation, and its exceptions.

## 5.3 Complete subsystems

1251. Build a complete UART subsystem: init, ring buffers, ISR, DMA path, error recovery, statistics, and a host-side test.
1252. Add flow control to 1251 — hardware RTS/CTS and software XON/XOFF — and state when each is appropriate.
1253. Add automatic baud detection to 1251 by measuring the first character's bit timing.
1254. Build a complete I²C subsystem: bounded waits, NACK handling, bus recovery, per-device retry policy, statistics.
1255. Add a transaction queue to 1254 so callers submit asynchronous requests with completion callbacks.
1256. Add clock-stretch tolerance and a slave-mode register map to 1254.
1257. Build a complete SPI subsystem with DMA, chip-select management for four devices, and per-device clock settings.
1258. Build a complete CAN subsystem: filters, mailbox management, error states, bus-off recovery, and a transmit queue.
1259. Add a CAN transport layer (ISO-TP style) for messages longer than eight bytes, with flow control.
1260. Build a logging subsystem: severity levels, binary records, ring buffer, drop counting, host decoder.
1261. Add compile-time level filtering to 1260 so disabled levels cost nothing, not even the argument evaluation.
1262. Add rate limiting to 1260 so a repeating fault cannot flood the log.
1263. Build a configuration subsystem: defaults in flash, overrides in a persistent store, validation, atomic commit.
1264. Add versioned migration to 1263 so an old stored format is upgraded on first boot.
1265. Build a command shell over UART: line editing, history, argument parsing, a command table, and help text.
1266. Add a permission level to 1265 so diagnostic commands are unavailable in production builds.
1267. Build a sensor subsystem: probe, init, periodic sample, filter chain, fault detection, last-known-good fallback.
1268. Add hot-plug detection to 1267 so a disconnected sensor is reported and re-probed.
1269. Build a motor control subsystem: PWM generation, current limiting, ramping, stall detection, thermal derating.
1270. Add closed-loop speed control to 1269 using the encoder and PID from earlier tiers.
1271. Build a display subsystem: framebuffer, dirty-rectangle tracking, DMA flush, and a text renderer.
1272. Add double buffering to 1271 and explain the tearing it eliminates.
1273. Build a time subsystem: monotonic ticks, wall clock from an RTC, drift correction, and timezone-free storage.
1274. Add NTP or SNTP synchronisation to 1273 with a bounded step and a slew mode.
1275. Build a persistent counter subsystem on flash that survives power loss and does not wear out one sector.
1276. Build a state persistence subsystem: save on brown-out detection, restore on boot, validate with a CRC.
1277. Build a bootloader with UART, A/B slots, signature verification, rollback, and a recovery mode.
1278. Build an OTA client: chunked download, resume after interruption, verification, commit, and confirm.
1279. Build a watchdog subsystem: per-task liveness, a supervisor, reset-cause logging, and a safe mode.
1280. Build a diagnostics subsystem that reports every counter, high-water mark, and error tally from all of the above.
1281. Integrate all of the above into one application; report total flash and RAM, and identify the largest consumer.
1282. Reduce that image by 20% without removing functionality; document every change and its saving.
1283. Write the integration test that exercises every subsystem from the host in one run.
1284. Measure worst-case interrupt latency in the integrated system and identify what dominates it.
1285. Write the README a new engineer would need to understand your architecture in twenty minutes.
1286. Build a HAL layer beneath all of the above so the same application compiles for a second MCU family.
1287. Actually port it to a second family; report what broke and what the HAL failed to abstract.
1288. Refactor the HAL based on 1287; state the rule you now use for what belongs behind it.
1289. Build a simulator backend for the HAL so the whole application runs on a host.
1290. Run your full test suite against the simulator in CI; report coverage of the hardware-independent code.

## 5.4 An RTOS from scratch

1291. Design your kernel: state the scheduling policy, priority count, primitives, and memory model before writing code.
1292. Implement the TCB, the ready bitmap, and O(1) task selection.
1293. Implement the context switch in assembly for ARMv7-M; verify it with two tasks toggling pins.
1294. Add task creation with static stack allocation; reject a stack smaller than a documented minimum.
1295. Implement `task_yield`, `task_sleep`, and the delay list.
1296. Implement the tick handler: advance time, wake expired tasks, and time-slice equal priorities.
1297. Implement critical sections with `BASEPRI`, and define your maximum syscall priority.
1298. Document the rule that follows from 1297 for ISR priority assignment; enforce it with an assertion.
1299. Implement counting semaphores with priority-ordered wake and timeout.
1300. Implement mutexes with ownership, priority inheritance, and inheritance restoration.
1301. Prove that your inheritance implementation bounds blocking time by the critical-section length.
1302. Implement message queues with blocking send and receive, and copy-by-value semantics.
1303. Implement event groups with AND/OR waits, and resolve the clear-on-exit question explicitly.
1304. Implement software timers on top of the delta timer list, running in a dedicated task.
1305. Implement the idle task with `WFI` and an idle hook.
1306. Implement tickless idle; prove the tick counter cannot drift.
1307. Implement stack overflow detection at every context switch.
1308. Implement per-task MPU regions; measure the added switch cost.
1309. Implement the unprivileged-task model with `SVC`-based system calls.
1310. Validate every syscall pointer argument against the calling task's memory; block a hostile one.
1311. Implement task deletion, and enumerate everything that must be released.
1312. Explain why task deletion is genuinely hard, and argue for or against supporting it at all.
1313. Implement runtime statistics: per-task CPU time, switch count, and maximum ready-to-run latency.
1314. Measure your kernel's context-switch time in cycles; compare it with FreeRTOS on the same part.
1315. Measure your kernel's worst-case interrupt latency; account for every contributor.
1316. Report your kernel's RAM and flash footprint; compare with FreeRTOS configured equivalently.
1317. Write a stress test: twelve tasks, random priorities, random blocking, running for 24 hours with assertions on.
1318. Run 1317 with `configASSERT`-equivalent checks on every invariant you can define; report what fired.
1319. Write the documentation a user of your kernel needs: API, constraints, priority rules, and footprint.
1320. Write an honest comparison of your kernel against FreeRTOS and Zephyr: what you would use each for.

## 5.5 Storage and filesystems

1321. Implement a wear-levelled key-value store on flash: append-only records, compaction when full.
1322. Prove your store in 1321 survives power loss at any point without losing a previously committed value.
1323. Implement a two-sector ping-pong scheme for a single configuration blob with a sequence number.
1324. Explain why a sequence number beats a "valid" flag for choosing between two copies.
1325. Implement bad-block detection and remapping for NAND; explain why NOR usually does not need it.
1326. Implement erase-count tracking per sector and use it to distribute wear.
1327. Compute the write endurance of your scheme: writes per day against a 10 000-cycle rating.
1328. Implement a journal: write intent, apply, clear intent; recover by replaying an incomplete entry.
1329. Prove that your journal makes a multi-sector update atomic.
1330. Implement a FAT16 reader: boot sector, FAT chain, root directory, file read.
1331. Add long-filename support to 1330, or argue why you would not.
1332. Implement a minimal append-only log filesystem with a bounded number of files.
1333. Implement a circular flight-recorder log that always retains the last N kilobytes.
1334. Implement CRC per record in 1333 so a partial final write is detected and discarded.
1335. Implement flash read-back verification after every write; measure the cost and decide whether to keep it.
1336. Implement a defragmentation pass for 1321; bound its worst-case duration.
1337. Explain why 1336 must be interruptible, and make it so.
1338. Implement an external SPI-flash driver with sector erase, page program, and a busy-wait timeout.
1339. Add a block cache above 1338; measure the hit rate for a realistic access pattern.
1340. Implement encryption at rest for 1321; state the key management assumption you are making.
1341. Compute the storage overhead of your scheme: headers, CRCs, padding, spare sectors — as a percentage.
1342. Design a storage layout for a device that must retain 90 days of hourly samples in 512 KB; document the arithmetic.
1343. Add a schema version to every record type; write the migration path for one format change.
1344. Write a corruption-injection test: flip random bits in the store and assert graceful recovery every time.
1345. Report what 1344 found, and fix the worst case.

## 5.6 Advanced algorithms and signal processing

1346. Implement a radix-2 FFT in fixed point; state the scaling strategy that prevents overflow.
1347. Explain block floating point and apply it to 1346 to improve dynamic range.
1348. Compute the FFT of a known two-tone signal and verify both bin magnitudes analytically.
1349. Implement a windowing function (Hann) and explain what spectral leakage it reduces.
1350. Implement an FFT-based frequency estimator with parabolic interpolation for sub-bin accuracy.
1351. Compare 1350 against the Goertzel algorithm for detecting three known tones; state which you would ship.
1352. Implement a multi-dimensional Kalman filter in fixed point for a 2-state system.
1353. Explain each matrix in 1352 in physical terms, and how you would choose the noise covariances.
1354. Implement a complementary filter for accelerometer and gyroscope fusion; compare with 1352 on cost and quality.
1355. Implement quaternion-based orientation integration; explain why Euler angles fail.
1356. Implement magnetometer calibration: hard-iron offset and soft-iron scaling from collected samples.
1357. Implement PID autotuning by the relay (Åström–Hägglund) method.
1358. Implement gain scheduling: different PID gains per operating region, with smooth transitions.
1359. Implement feedforward alongside your PID; demonstrate the improvement on a step response.
1360. Implement anti-windup by back-calculation rather than clamping; compare the recovery behaviour.
1361. Implement a Kalman-based battery state-of-charge estimator combining coulomb counting and voltage.
1362. Implement MPPT by perturb-and-observe; state the step size trade-off.
1363. Implement a CORDIC routine for sine, cosine, and arctangent; measure cycles against a table.
1364. Implement fixed-point matrix inversion for a 3×3 matrix; state the conditioning problem.
1365. Implement a least-squares line fit in integers for sensor calibration.
1366. Implement Huffman coding end to end: build, encode, decode; verify round-trip on real sensor data.
1367. Implement a delta-plus-varint encoder for time-series samples; measure the compression on real data.
1368. Compare 1367 against Huffman and against raw on the same data; tabulate size and cycles.
1369. Implement Reed–Solomon or Hamming error correction for a small block; state the correction capability.
1370. Explain when forward error correction beats retransmission, with a link-budget argument.

## 5.7 Timing analysis and real-time proofs

1371. Determine the WCET of one function by measurement; state why measurement alone is not a proof.
1372. Determine the same WCET by static analysis of the disassembly; compare with 1371.
1373. Identify every source of timing variability in that function: branches, cache, flash waits, DMA contention.
1374. Compute your system's worst-case interrupt latency analytically; then measure it and explain the gap.
1375. Enumerate every critical section in your codebase and its maximum duration; find the longest.
1376. Prove that your longest critical section does not violate your most demanding deadline.
1377. Assign priorities by rate-monotonic ordering; verify the utilisation bound.
1378. Perform response-time analysis for a five-task set; identify which task is closest to its deadline.
1379. Add a sixth task and re-run 1378; state whether the set is still schedulable and why.
1380. Model priority inversion in your analysis by adding the blocking term; recompute.
1381. Compute the jitter budget for a 1 kHz control loop and measure your actual jitter against it.
1382. Explain why average latency is the wrong metric for a control loop, using your own numbers.
1383. Measure DMA bus contention: run a DMA transfer during a timed CPU loop and report the slowdown.
1384. Explain flash wait states and measure their effect by running the same function from RAM.
1385. Measure the cost of a cache miss on a Cortex-M7 and state its implication for WCET analysis.
1386. Explain why enabling the cache can make a system *less* predictable while making it faster.
1387. Design a system that meets a 50 µs deadline with the cache enabled; state what you had to do.
1388. Implement deadline monitoring: detect and log when a task misses its deadline.
1389. Implement a mode change (normal to degraded) triggered by repeated deadline misses.
1390. Document your system's timing contract: every deadline, its source, and the evidence it is met.
1391. Argue for or against using an RTOS at all for a hard-real-time loop; give the criteria.
1392. Argue for or against running the control loop entirely in an ISR; give the criteria.
1393. Design the timing architecture for a system with one 20 kHz loop, one 1 kHz loop, and best-effort comms.
1394. State every assumption in 1393 that would need re-verification if the MCU clock were halved.
1395. Write the timing section of a design review document for 1393.

## 5.8 Security engineering

1396. Write a threat model for a battery-powered sensor on a public network: assets, adversaries, capabilities, mitigations.
1397. Write one for a device an attacker can hold in their hand; state what changes.
1398. Rank your mitigations by cost against risk reduction; identify the one you would drop first.
1399. Implement a full secure-boot chain: immutable root, signed bootloader, signed application.
1400. Explain what each link proves and what the chain does not protect against.
1401. Implement key provisioning at manufacture; state where the private key lives and who can access it.
1402. Argue for or against a per-device key versus a shared key; give the failure scenario for each.
1403. Implement secure firmware update with signature verification and anti-rollback.
1404. Implement a monotonic anti-rollback counter that survives an attacker with flash write access, or argue that it cannot.
1405. Configure readout protection; then document exactly what an attacker with physical access can still do.
1406. Explain a voltage-glitching attack on a signature comparison and implement two countermeasures.
1407. Implement redundant verification of a security decision; explain why a single comparison is insufficient.
1408. Measure the timing side channel in a naive tag comparison; then eliminate it and re-measure.
1409. Explain a power-analysis attack on an AES implementation, and name a mitigation.
1410. Implement constant-time conditional selection with no branches on secret data.
1411. Audit one of your own modules for secret-dependent control flow; report what you found.
1412. Implement secure key erasure across every location a key may exist; enumerate them first.
1413. Implement a debug-port lockout with a documented recovery path for authorised service.
1414. Argue for or against leaving JTAG accessible in production; state the deciding factor.
1415. Implement TrustZone-M partitioning for a key store: secure and non-secure worlds, with a gateway API.
1416. Explain what the secure world protects against that an MPU alone does not.
1417. Design the non-secure callable interface for 1415 so it cannot be used as an oracle.
1418. Implement rate limiting on an authentication attempt to make brute forcing impractical; compute the numbers.
1419. Implement secure logging so an attacker cannot erase evidence of tampering.
1420. Write the security section of a design review, including what you explicitly chose not to defend against.

## 5.9 Architecture and design judgement

*Written answers. State a position, give the reasoning, and name what you traded away.*

1421. Interrupt-driven versus DMA versus polling for a 1 Mbit/s data stream. Choose and justify.
1422. Superloop versus RTOS for a system with three periodic tasks and one comms link.
1423. Static allocation versus a pool versus a heap. Give the decision criteria, not a preference.
1424. Table-driven versus function-pointer state machines. When does each win?
1425. Blocking versus asynchronous driver APIs. What does each force on the caller?
1426. Error codes versus a global `errno` versus an error callback. Choose one for a driver layer.
1427. One coarse lock versus several fine ones. Give the case for each with a concrete example.
1428. Copy-by-value versus reference-counted buffers between protocol layers.
1429. A HAL that abstracts registers versus one that abstracts operations. Which and why?
1430. Compile-time configuration versus runtime configuration. Where is the boundary?
1431. Vendor HAL versus registers versus your own layer. Give the criteria for each.
1432. C versus C++ for firmware in 2026. Argue both, then state your position.
1433. MISRA compliance: what it buys, what it costs, and when you would adopt it.
1434. Design a fault-response policy: which faults reset, which degrade, which halt.
1435. Design a versioning scheme covering firmware, protocol, and stored data formats.
1436. Design a logging strategy for a device you cannot physically reach.
1437. Design a manufacturing test that catches a cold solder joint on an I²C line.
1438. Design a field diagnostic that distinguishes a sensor failure from a wiring failure.
1439. Design for a five-year field life with no updates. What changes about your choices?
1440. Design for weekly OTA updates. What changes?
1441. You inherit 80 000 lines of undocumented firmware with no tests. Write your first-90-days plan.
1442. You must add a feature to code you believe is unsafe. Describe how you proceed.
1443. A driver works on the bench and fails in the field 1% of the time. Write your investigation plan.
1444. Sales has committed to a feature that needs 20% more RAM than you have. Write your options memo.
1445. Choose between two MCUs for a new product; write the selection criteria and the decision.
1446. Estimate the flash and RAM budget for a specified product before writing any code; state your method.
1447. Write the coding standard for a five-person firmware team: ten rules, each with a reason.
1448. Write your code review checklist, ordered by how often the finding is a real defect.
1449. Define the boundary between what belongs in an ISR and what does not, as a rule others can apply.
1450. Write the firmware architecture document for a product of your choosing: layers, interfaces, and constraints.

## 5.10 Hard debugging

1451. Reproduce a race deliberately: write code that fails once in ten thousand runs, then make it fail every time.
1452. Explain three techniques for making an intermittent race deterministic.
1453. Debug a fault that disappears under the debugger; explain the three usual causes.
1454. Debug a fault that appears only at `-O2`; enumerate the four categories it is likely to be.
1455. Find a memory corruption whose symptom is a global changing value; describe your method.
1456. Use a watchpoint on that address to catch the writer; explain what you did when there were no watchpoints left.
1457. Find a stack overflow with no MPU available; describe the technique.
1458. Decode a HardFault with a nonsense PC; explain what a corrupted stack frame implies.
1459. Debug a system that hangs with no fault and no output; describe your first three actions.
1460. Debug a peripheral that works once and never again after reset; name the likely class of cause.
1461. Debug data corruption that appears only when a second peripheral is enabled.
1462. Debug a timing failure that appears only when the log is disabled.
1463. Explain how instrumentation changed the behaviour in 1462, and how to instrument without doing so.
1464. Debug a field failure from a single log line and a reset counter; describe what you would ask for next.
1465. Design the minimum instrumentation that would have diagnosed 1464 remotely.
1466. Debug an I²C bus that is dead after a power glitch; explain why the MCU reset did not fix it.
1467. Debug a UART that loses bytes only under load; enumerate five candidate causes and how to distinguish them.
1468. Debug an ADC reading that drifts with ambient temperature; separate sensor, reference, and code causes.
1469. Debug a DMA transfer that delivers stale data on one MCU family and not another.
1470. Debug a system that fails after 49.7 days of uptime; state the cause without further information.
1471. Debug one that fails after 71 minutes; same.
1472. Debug one that fails on the 32 768th message; same.
1473. Debug one that fails only on the second unit built; describe your approach.
1474. Write a post-mortem for a bug you introduced: symptom, cause, why it escaped review, and the process change.
1475. Write the bisection procedure you would use to find a regression across 400 commits with a 20-minute test.

## 5.11 Capstone projects

*Multi-day builds. Each produces something demonstrable and each is a portfolio artefact.*

1476. Build a bare-metal blinky from an empty file: linker script, startup, vector table, no vendor code. Document every line.
1477. Bring up a UART from the reference manual alone, with no example code. Log what you got wrong.
1478. Write a driver for an unfamiliar I²C sensor from its datasheet only; capture the transaction on an analyser.
1479. Build a logic-analyser capture set for I²C, SPI, and UART, each with one deliberate failure mode, annotated.
1480. Write your own preemptive RTOS and run a five-task application on it for 24 hours.
1481. Write a bootloader with A/B slots, signature verification, and rollback; demonstrate a survived power cut.
1482. Build a complete OTA pipeline: build server, signing, transport, device update, and confirmation.
1483. Build a data logger: sensor, filter, compress, store to flash, retrieve over UART, decode on a host.
1484. Build a closed-loop motor controller with encoder feedback, tuned PID, and stall protection.
1485. Build a protocol stack for a two-node link with framing, CRC, retransmission, and flow control; test with injected loss.
1486. Build a host-side simulator for your whole application and run the test suite in CI.
1487. Fuzz a real open-source embedded parser for 24 hours; report findings or a coverage analysis explaining the absence.
1488. Take one finding from 1487 through responsible disclosure; document the process and outcome.
1489. Port a non-trivial application from one MCU family to another; report every leaky abstraction.
1490. Reduce an existing firmware image by 30%; document each change and its measured saving.
1491. Take an existing driver to 90% branch coverage on a host; report the bugs the tests found.
1492. Instrument a system for worst-case latency and produce a timing report with evidence for every deadline.
1493. Implement TrustZone-M separation for a key store and demonstrate that the non-secure world cannot extract the key.
1494. Build a manufacturing test fixture and firmware that validates every peripheral and reports pass or fail.
1495. Write and publish a technical article on one thing from 1476–1494, with your own captures and measurements.
1496. Contribute a patch to an upstream embedded project (Zephyr, U-Boot, Linux, FreeRTOS); document the review process.
1497. Write the design document for a product of your own choosing, complete enough for someone else to build it.
1498. Build that product.
1499. Write the post-mortem: what the design document got wrong, and what you would change.
1500. Teach one topic from this document to someone else, from scratch, without notes. That is the actual test.

---

# Finishing

**1500 exercises, five tiers, easiest to hardest.**

| Tier | Range | Level | Typical time each |
|---|---|---|---|
| 1 | 1–300 | Foundations | 2–10 min |
| 2 | 301–600 | Core competence | 10–30 min |
| 3 | 601–900 | Applied | 15–45 min |
| 4 | 901–1200 | Systems | 30–90 min |
| 5 | 1201–1500 | Senior | hours to days |

## The schedule, restated

The count is the least important thing in this document.

- **Daily, 20–30 minutes.** Interleave two sections rather than working one front to back.
- **Mark every attempt.** `✓` retire 30 days · `~` redo in 3 days · `✗` redo tomorrow.
- **Chase the `✗` list.** An exercise you got right teaches you nothing. You are finished with a section when its `✗` list is empty, not when you have touched every number.
- **Weekly, one timed 45-minute exercise** with no reference. That is the only part that resembles an interview.

**Three passes over the 400 you keep failing beats one pass over all 1500.** If you finish Tier 3 having genuinely internalised it, you are more competent than most candidates who have read twice as much.

## How this fits with the solved document

Two documents, two jobs:

- **`EMBEDDED-C-INTERVIEW-CORE.md`** — 150 problems, every one solved with the reasoning. Read this when an exercise here defeats you, or when you want to know *why* something is the way it is.
- **This document** — 1500 prompts, no solutions. Use it to build fluency and to discover what you do not know.

When an exercise here stops you, the corresponding topic in the core document is the answer. Read it, then come back and write the code without looking.

## What neither document can give you

Both are written knowledge. The gap they cannot close is the one that made you go blank in front of a new driver: **the experience of not knowing, and starting anyway.**

Exercises 1476 to 1479 are the ones that close it. Bring up a UART from the reference manual with no example code. Write a driver for a sensor you have never used. Capture the transaction and see the failure on a screen. Those four are worth more than the other 1496 combined, and they are the only ones that produce something you can show someone.

Everything before them is preparation for being able to do them.

---

*Companion to `EMBEDDED-C-INTERVIEW-CORE.md`. Reference platform STM32 / Cortex-M, C11.*
