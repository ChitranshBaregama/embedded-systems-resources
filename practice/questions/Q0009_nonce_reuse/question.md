# Security — AES-GCM Nonce Reuse

## Problem
Explain why reusing the same AES-GCM nonce/IV with the same key is dangerous.

Describe confidentiality and authentication consequences conceptually. Then design an embedded-device nonce strategy that survives resets and power loss.


## Scored contract clarification

Use public toy data only. Explain consequences and build a power-cut/reboot nonce-allocation model, including interrupted persistence and rollback. Do not implement production cryptography for this exercise. Distinguish uniqueness requirements from secrecy.

Timebox: agree before starting (default 30 minutes coding or 20 minutes oral/design). Status: Unseen. Original source mapping is recorded in career/MIGRATION.md.
