# Debugging — UART RX Stuck Low

## Problem
A UART receiver repeatedly reads framing errors / unexpected `0x7F`, and the RX line appears low while idle.

Create a systematic hardware + firmware diagnosis plan. Include idle polarity, pin mux, pull configuration, voltage levels, ground reference, baud mismatch, inversion, contention and oscilloscope/logic-analyzer checks.


## Scored contract clarification

Order experiments by safety and information gained. State expected observations for each hypothesis and which measurement would reject it. Separate electrical idle level from decoded bytes; do not claim a particular byte uniquely diagnoses the failure.

Timebox: agree before starting (default 30 minutes coding or 20 minutes oral/design). Status: Unseen. Original source mapping is recorded in career/MIGRATION.md.
