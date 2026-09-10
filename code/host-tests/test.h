/* A 40-line test harness, on purpose.
 *
 * Embedded teams skip unit tests because "you can't test firmware
 * without hardware". Most of that is false: anything that is a pure
 * function of its inputs - parsers, CRCs, state machines, ring buffers,
 * scaling maths, packing and unpacking - compiles and runs on a build
 * machine, at a million iterations per second, under a sanitiser.
 *
 * The part that genuinely needs hardware is the register poking. Keep
 * that layer thin and this layer thick.
 */
#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <string.h>

static int tests_run, tests_failed, checks_run;

#define CHECK(cond) do {                                              \
        checks_run++;                                                 \
        if (!(cond)) {                                                \
            printf("    FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond);\
            tests_failed++;                                           \
            return;                                                   \
        }                                                             \
    } while (0)

#define CHECK_EQ(a, b) do {                                           \
        long _a = (long)(a), _b = (long)(b);                          \
        checks_run++;                                                 \
        if (_a != _b) {                                               \
            printf("    FAIL %s:%d  %s == %s  (%ld vs %ld)\n",        \
                   __FILE__, __LINE__, #a, #b, _a, _b);               \
            tests_failed++;                                           \
            return;                                                   \
        }                                                             \
    } while (0)

#define RUN(fn) do {                                                  \
        int before = tests_failed;                                    \
        tests_run++;                                                  \
        printf("  %-52s", #fn);                                       \
        fflush(stdout);                                               \
        fn();                                                         \
        printf("%s\n", (tests_failed == before) ? "ok" : "");         \
    } while (0)

#define TEST_SUMMARY() do {                                           \
        printf("\n%d tests, %d checks, %d failed\n",                  \
               tests_run, checks_run, tests_failed);                  \
        return tests_failed ? 1 : 0;                                  \
    } while (0)

#endif
