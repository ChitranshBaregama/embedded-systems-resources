# Patterns

How to structure firmware so that it stays debuggable after it grows.

| | |
| :--- | :--- |
| [state-machines.md](state-machines.md) | Five implementations compared honestly, hierarchical state machines, run-to-completion semantics, timers, anti-patterns, and how to test one |
| [driver-bring-up.md](driver-bring-up.md) | A repeatable seven-phase procedure for an unfamiliar peripheral, a bisection method for a dead bus, a symptom table, and a worked example |

The state machine document has a working companion:
[`code/portable/frame/`](../code/portable/frame/) is a framed-protocol receiver
written as a pure function of `(state, byte)`. It is compiled unmodified by
both the target build and the host test suite, which is the practical payoff
of writing it that way — see
[`code/host-tests/test_frame.c`](../code/host-tests/test_frame.c) for the tests
that matter, which are the malformed inputs rather than the well-formed ones.

## Still to write

Layered driver architecture, error propagation strategies, and the argument
for and against HALs. See [`../ROADMAP.md`](../ROADMAP.md).
