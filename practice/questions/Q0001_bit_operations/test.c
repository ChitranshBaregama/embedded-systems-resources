#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

bool bit_set(uint32_t, unsigned, uint32_t *);
bool bit_clear(uint32_t, unsigned, uint32_t *);
bool bit_toggle(uint32_t, unsigned, uint32_t *);
bool bit_test(uint32_t, unsigned, bool *);
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL at %d\n", __LINE__); return 1; } } while (0)
int main(void) {
    uint32_t out = UINT32_C(0x12345678);
    bool value = false;
    CHECK(bit_set(0, 0, &out) && out == UINT32_C(1));
    CHECK(bit_set(0, 31, &out) && out == UINT32_C(0x80000000));
    CHECK(bit_clear(UINT32_MAX, 31, &out) && out == UINT32_C(0x7fffffff));
    CHECK(bit_toggle(UINT32_C(0x80000000), 31, &out) && out == 0);
    CHECK(bit_test(UINT32_C(0x80000000), 31, &value) && value);
    CHECK(bit_test(0, 31, &value) && !value);
    out = UINT32_C(17);
    CHECK(!bit_set(0, 32, &out) && out == UINT32_C(17));
    CHECK(!bit_clear(0, 100, &out) && out == UINT32_C(17));
    CHECK(!bit_toggle(0, 32, &out) && out == UINT32_C(17));
    value = true;
    CHECK(!bit_test(0, 32, &value) && value);
    CHECK(!bit_set(0, 0, NULL));
    CHECK(!bit_clear(0, 0, NULL));
    CHECK(!bit_toggle(0, 0, NULL));
    CHECK(!bit_test(0, 0, NULL));
    puts("Public contract checks passed; review-time tests and explanation still required.");
    return 0;
}
