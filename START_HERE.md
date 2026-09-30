# Start here: the complete firmware learning workspace

This repository combines the reference library and Firmware-Interview-Prep into one active workspace. Use this repository for all new learning, code, notes, progress, projects and interview preparation. The former repository is retained as a historical archive; its original files are also preserved in the [migration snapshot](career/migration/Firmware-Interview-Prep.snapshot.json).

## 1. Set up and verify

Clone this repository and enter its directory:

```sh
git clone https://github.com/ChitranshBaregama/embedded-systems-resources.git
cd embedded-systems-resources
python3 scripts/learning.py validate
python3 -m unittest discover -s scripts -p test_learning.py
python3 scripts/check-links.py
```

On Windows, use `python` if `python3` is unavailable. Coding exercises require GCC or another compatible C compiler; see the [development environment](.devcontainer/README.md). Infrastructure checks do not mean you have passed any exercise.

## 2. Plan your progression

Read the [roadmap](career/TOP_TIER_FIRMWARE_ROADMAP.md) and [eight readiness gates](career/READINESS_GATES.md). Gates require evidence, not just question counts. The [coverage map](practice/google-firmware/COVERAGE.md) distinguishes existing material from gaps.

## 3. Learn and attempt

Use [architecture](architecture/README.md), [peripherals](peripherals/README.md), [patterns](patterns/README.md), [networking](networking/README.md), [security](security/README.md) and [debugging](debugging/README.md) as your reference library.

Start independent coding with [Q0001: bit operations](practice/questions/Q0001_bit_operations/question.md). Read its contract, write your own `solution.c`, then run:

```sh
python3 scripts/learning.py run Q0001
```

The initial implementation is intentionally blank, so the build initially fails. The task provides a real test harness. Other seed exercises explicitly identify unfinished harnesses or oral/design evidence. See the [question workflow](practice/questions/README.md) before creating additional tasks.

Use the [1,500 exercise bank](practice/embedded-c-1500-exercises.md) for drills, [DSA practice](practice/dsa/README.md), [debugging practice](practice/debugging/README.md), and [system design](practice/system-design/README.md) for deeper work. Existing solved examples are references; using them makes an attempt assisted.

## 4. Review and retain

Follow the [progress instructions](practice/progress/README.md) to record real attempts in one ledger, including time, assistance, correctness, evidence and review scores. Use the [weekly review](career/WEEKLY_REVIEW.md) to inspect independent success, weak topics and retention.

```sh
python3 scripts/learning.py summary
python3 scripts/learning.py log --help
```

No attempts or readiness scores are pre-filled. The [coaching specification](career/SYSTEM_SPEC.md) defines two-hour daily sessions, a shorter coding session, progressive hints and the rule that you write scored implementations.

## 5. Build and contribute

Work through the [three-project portfolio plan](projects/PORTFOLIO_PLAN.md). Record actual tests and measurements; plans alone are not completed projects. Advance through the [open-source contribution stages](open-source/CONTRIBUTION_TRACKER.md) using public, sanitized work.

## 6. Interview and prove readiness

Use the [150 interview prompts](practice/interview-150/README.md), then the [full mock loop](mock-interviews/LOOP.md). Review the readiness gates against actual evidence. Keep all future progress in this repository.

## Migration and preservation

The [migration record](career/MIGRATION.md) maps every seed question and explains the disposition of all source files. The source archive retains its original Git history; the master contains a content snapshot and the integrated active workflow. Git histories were not rewritten or combined.
