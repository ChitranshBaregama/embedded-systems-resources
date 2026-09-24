#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Independent Python datetime oracle for the C program's daily output."""
import datetime as dt
import pathlib
import subprocess
import sys

if len(sys.argv) != 2:
    raise SystemExit("usage: python verify_calendar.py PATH_TO_TEST_EXECUTABLE")
exe = str(pathlib.Path(sys.argv[1]).resolve())
result = subprocess.run([exe, "--oracle"], check=True, capture_output=True, text=True)
epoch = dt.datetime(2000, 1, 1)
rows = result.stdout.splitlines()
end = dt.datetime(2400, 1, 1)
expected_days = (end - epoch).days
if len(rows) != expected_days:
    raise SystemExit(f"FAIL: expected {expected_days} rows, got {len(rows)}")
for index, row in enumerate(rows):
    year, month, day, seconds = map(int, row.split(","))
    actual = dt.datetime(year, month, day)
    expected = epoch + dt.timedelta(days=index)
    if actual != expected or seconds != int((actual - epoch).total_seconds()):
        raise SystemExit(f"FAIL: oracle mismatch at row {index}: {row}")
print(result.stderr.strip())
print(f"PASS: Python datetime independently verified all {expected_days} days")
