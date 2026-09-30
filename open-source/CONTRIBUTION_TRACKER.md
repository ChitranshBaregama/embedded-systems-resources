# Open-source contribution progression

Start with one ecosystem aligned with the active project rather than five simultaneous checkouts. Candidate primary resources: [Zephyr](https://docs.zephyrproject.org/latest/contribute/index.html), [FreeRTOS Kernel repository](https://github.com/FreeRTOS/FreeRTOS-Kernel), [MCUboot](https://docs.mcuboot.com/), [Trusted Firmware-M](https://trustedfirmware-m.readthedocs.io/en/latest/contributing/contributing_process.html), and [Linux kernel](https://docs.kernel.org/process/submitting-patches.html). Read the current upstream rules before preparing a submission; no issue is represented here as currently available or assigned.

| Stage | Evidence to retain |
| --- | --- |
| 1. Build locally | Upstream commit, toolchain, exact command and build log |
| 2. Run tests | Test selection, result and environment limitations |
| 3. Understand one subsystem | Ownership/data-flow notes and source map |
| 4. Reproduce an issue | Minimal reproducer, expected/actual behavior, confirmed affected revision |
| 5. Small test/docs/tooling change | Focused diff and relevant checks |
| 6. Real bug fix | Failing-before/passing-after regression evidence |
| 7. Subsystem/driver contribution | Design discussion, compatibility analysis, tests and review responses |

Upstream submission or maintainer messages require explicit user instructions. Do not invent authorship/sign-offs or claim a patch merged without a link. Observe the upstream licence and contribution process; do not submit employer code.

| Ecosystem / subsystem | Stage | Upstream SHA | Issue | Local evidence | Patch/PR | Review/merge state | Next action |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Not selected | 0 | — | — | None | — | Not submitted | Choose based on project and build feasibility |
