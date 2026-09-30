# Pointers & Memory — Implement memmove

## Problem
Implement your own `my_memmove(void *dst, const void *src, size_t n)`.

It must work correctly when source and destination regions overlap. Do not call the standard `memmove`.

Explain when forward copying is safe and when backward copying is required.


## Scored contract clarification

Use standard memmove semantics for valid object ranges: return dst, copy n bytes, support overlap. For n=0 this exercise additionally permits NULL and performs no access. For n>0 both ranges must be valid. Do not compare unrelated object pointers relationally without justifying the C portability model. State whether uintptr_t exists and whether address ordering is a platform assumption. Test both overlap directions, identical ranges, zero length and separate arrays.

Timebox: agree before starting (default 30 minutes coding or 20 minutes oral/design). Status: Unseen. Original source mapping is recorded in career/MIGRATION.md.
