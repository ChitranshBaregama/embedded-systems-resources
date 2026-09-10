#!/usr/bin/env bash
# Build and run every QEMU example, and report which ones exited cleanly.
# This is what CI runs; it is also the fastest way to check your toolchain.
set -uo pipefail

QEMU=${QEMU:-qemu-system-arm}
TIMEOUT=${TIMEOUT:-60}
here=$(cd "$(dirname "$0")" && pwd)
fail=0

for dir in "$here"/[0-9]*/; do
    name=$(basename "$dir")
    printf '%-34s ' "$name"

    if ! make -C "$dir" clean >/dev/null 2>&1 || ! make -C "$dir" >/dev/null 2>&1; then
        echo "BUILD FAILED"; fail=1; continue
    fi

    elf=$(ls "$dir"*.elf 2>/dev/null | head -1)
    out=$(timeout "$TIMEOUT" "$QEMU" -M lm3s6965evb -cpu cortex-m3 -nographic \
             -semihosting-config enable=on,target=native -kernel "$elf" 2>&1)
    rc=$?

    if [ $rc -eq 124 ]; then
        echo "TIMEOUT (did it call semihost_exit?)"; fail=1
    elif [ $rc -ne 0 ]; then
        echo "EXIT $rc"; fail=1
    elif grep -qiE '\*\*\*|FAILED|BROKEN' <<<"$out" \
         && ! grep -q 'this fault was deliberate' <<<"$out"; then
        echo "RAN, but reported a failure"; fail=1
    else
        echo "ok  ($(wc -l <<<"$out") lines of output)"
    fi
done

echo
[ $fail -eq 0 ] && echo "all examples passed" || echo "some examples failed"
exit $fail
