# Migration record

Source: [Firmware-Interview-Prep at b82839f](https://github.com/ChitranshBaregama/Firmware-Interview-Prep/tree/b82839f16f7fbed84911f818ec88a995c798a748). Canonical base: 831a96c2d14141a88e96f8167a2f61dc385ac6ff.

All 52 source blobs are preserved with original paths, contents and Git SHA-1 values in migration/Firmware-Interview-Prep.snapshot.json. Duplicate bytes are retained there for provenance but not duplicated in the active curriculum. No source repository file is deleted by this migration.

| Source question filename | Tracker identity | Canonical directory |
| --- | --- | --- |
| question (10).md | Q0001 | practice/questions/Q0001_bit_operations |
| question (4).md | Q0002 | practice/questions/Q0002_integer_promotions |
| question (1).md | Q0003 | practice/questions/Q0003_memmove |
| question (2).md | Q0004 | practice/questions/Q0004_register_access |
| question (6).md | Q0005 | practice/questions/Q0005_isr_handoff |
| question (7).md | Q0006 | practice/questions/Q0006_uart_ring |
| question (9).md | Q0007 | practice/questions/Q0007_rtos_ipc |
| question (5).md | Q0008 | practice/questions/Q0008_uart_debug |
| question (8).md | Q0009 | practice/questions/Q0009_nonce_reuse |
| question (3).md | Q0010 | practice/questions/Q0010_dlms_association |

## Other source disposition

README.md supplies the workflow and topic allocation; README (1).md supplies the separate 60/35/30/25 interview split. Readme.md is only a duplicate title. TRACKER.md records all ten seeds as Unseen with zero attempts, so no completed performance is imported. CURRICULUM.csv is retained as a proposal in the snapshot, not falsely substituted for an audit of the existing 1,500 exercises.

question.md is an unfilled template. solution*.c contain no learner implementation. notes*.md contain headings but no attempt evidence. test*.c print a reminder and return zero; active scaffolds instead fail explicitly until a meaningful harness is supplied. The original Makefile targets absent questions directories and the original generator assumes an absent templates directory; scripts/learning.py replaces that broken workflow with validated paths and no-overwrite behavior.

## Acceptance before archiving

- Verify all source blobs against their stored hashes, and all intended destination files against the remote tree.
- Verify the ten mappings by question content, not filename suffix.
- Run link checks, tooling tests, catalog count/uniqueness checks, and CI.
- Confirm original references and the 1,500 bank are unchanged.
- Perform a real learner session using the new workflow; review it with the user.
- Only then archive the source repository. Archival is deliberately deferred until this operational acceptance; preparation and CI alone do not satisfy it.

Current migration state: content prepared for canonical repository; operational learner acceptance pending. No source archive action performed.
