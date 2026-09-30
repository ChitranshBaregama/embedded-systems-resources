# Progress ledger

`attempts.csv` is the canonical append-only learning record. `catalog.csv` indexes the preserved E0001–E1500 exercise bank. Q0001–Q0010 identify migrated seed prompts; I001–I150 identify the separate interview set. Never renumber one namespace into another or count duplicate variants as unique coverage.

All items are Unseen until an actual attempt is recorded. The script reports the latest recorded state, not a competence certification. Allowed states: Unseen, Attempted, Solved, Solved without help, Interview Ready, Revisit. Use Attempted for unfinished work, Solved for assisted completion, Solved without help for a correct zero-hint unassisted attempt, and Revisit after a failed retention attempt. Interview Ready requires reviewer evidence and retained performance.

Run `python3 scripts/learning.py log --help` after a real session. It requires explicit metrics and an evidence note/path; it never infers a solve from compilation. `--kind first`, `retention7`, or `retention30` separates original attempts from retests. Retest rows must refer to a previous correct attempt and meet the minimum elapsed interval. Due reminders are based on that original success, not reset by every review.

Run `python3 scripts/learning.py summary --today YYYY-MM-DD` for a reproducible snapshot. The default uses today's date. Scoring files are public: store sanitized evidence only.
