# Questions and scored attempts

The existing [1,500 exercise bank](../embedded-c-1500-exercises.md) is preserved. Its IDs are E0001â€“E1500 in [catalog.csv](../progress/catalog.csv). The migrated seed problems retain Q0001â€“Q0010, following the original tracker rather than accidental uploaded filename numbers. The [oral interview set](../interview-150/README.md) uses I001â€“I150.

Important coding attempts use `QXXXX_topic/` (or an E-prefixed bank ID) with question.md, solution.c, test.c and notes.md. Create a folder only when needed: `python3 scripts/learning.py new E0001 checked_add`. It never overwrites a folder or fills in the learner's solution. Before scoring an existing short drill, agree its complete contractâ€”including overflow, invalid input, target width, allocation and complexityâ€”rather than assuming the old one-line prompt proves a safe interface.

Compile a prepared task with `python3 scripts/learning.py run Q0001`. The initial Q0001 harness is real but the solution is deliberately absent; the build fails until the learner implements it. Other migrated test.c files contain an explicit compile-time blocker until their harness is defined. Design/oral questions are reviewed using notes and evidence; do not force fake executable tests onto them.

Public tests are visible learning material. Additional interviewer checks are created at review time and withheld until the attempt is submitted; this public repository does not claim to hide committed files. Do not read the solved reference core during an independent attempt. If you do, record assistance.

| Seed | Topic |
| --- | --- |
| [Q0001](Q0001_bit_operations/question.md) | Bit operations |
| [Q0002](Q0002_integer_promotions/question.md) | Integer promotions |
| [Q0003](Q0003_memmove/question.md) | Overlap-safe copying |
| [Q0004](Q0004_register_access/question.md) | Register access contracts |
| [Q0005](Q0005_isr_handoff/question.md) | ISR event handoff |
| [Q0006](Q0006_uart_ring/question.md) | UART ring buffer |
| [Q0007](Q0007_rtos_ipc/question.md) | RTOS IPC selection |
| [Q0008](Q0008_uart_debug/question.md) | UART diagnosis |
| [Q0009](Q0009_nonce_reuse/question.md) | Nonce reuse and persistence |
| [Q0010](Q0010_dlms_association/question.md) | DLMS association |
