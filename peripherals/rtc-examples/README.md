# RTC calendar examples

Companion to the [RTC handbook](../rtc.md). These are portable computational
examples, not peripheral drivers. Code is MIT licensed; see each C file's SPDX
identifier and the repository [code licence](../../LICENSE-CODE).

## Contract

- Gregorian years 2000 through 2399 inclusive.
- Epoch: 2000-01-01 00:00:00, **not Unix time**.
- Every represented day has 86,400 seconds; no leap-second or timezone handling.
- BCD inputs must already have device-specific flag bits extracted.
- Conversion failures leave output arguments unchanged.
- Iterative conversion prioritizes readability; benchmark before using in tight ISRs.

## Run

From this directory on a host with GCC and Python 3:

```sh
gcc -std=c11 -Wall -Wextra -Werror -pedantic -O2 test_rtc.c -o test_rtc
./test_rtc
python3 verify_calendar.py ./test_rtc
```

On a compatible Linux compiler, additionally use `-fsanitize=address,undefined
-fno-omit-frame-pointer -g` in a sanitizer build. This option is separate from
the portable Windows-host checks.

The suite checks every BCD byte and every day in the 400-year supported range,
including three times of day, invalid inputs, century exceptions, and output
preservation on failure. Python's standard-library datetime independently
checks all daily epoch values. This does not verify hardware retention,
oscillator accuracy, bus transactions, alarm registers, or an entire time service.
