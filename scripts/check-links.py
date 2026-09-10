#!/usr/bin/env python3
"""Verify that every relative link in every markdown file resolves.

External http(s) links and bare #anchors are skipped, as is anything inside a
fenced code block or inline backticks - otherwise C like `swap(h, e)` reads as
a markdown link and the checker cries wolf.

This exists because the repository shipped for a while with three documents
linking to a code/ directory that had never been created. Broken links on a
public repo are the cheapest possible own goal, and the cheapest possible fix.
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LINK = re.compile(r'\]\(([^)\s]+)(?:\s+"[^"]*")?\)')
FENCE = re.compile(r'^\s*(```|~~~)')
INLINE_CODE = re.compile(r'`[^`\n]*`')
SKIP = re.compile(r'^(https?://|#|mailto:|tel:)')


def links_in(text):
    """Yield (line_number, target) for every markdown link outside code."""
    in_fence = False
    for n, line in enumerate(text.splitlines(), 1):
        if FENCE.match(line):
            in_fence = not in_fence
            continue
        if in_fence:
            continue
        for match in LINK.finditer(INLINE_CODE.sub('', line)):
            yield n, match.group(1)


def main():
    broken, checked, files = [], 0, 0
    for md in sorted(ROOT.rglob('*.md')):
        if '.git' in md.parts:
            continue
        files += 1
        for line_no, target in links_in(md.read_text(encoding='utf-8', errors='replace')):
            if SKIP.match(target):
                continue
            path = target.split('#', 1)[0]
            if not path:
                continue
            checked += 1
            if not (md.parent / path).exists():
                broken.append((md.relative_to(ROOT), line_no, target))

    for f, n, t in broken:
        print(f'BROKEN  {f}:{n}  ->  {t}')

    print(f'\n{files} files, {checked} relative links checked, {len(broken)} broken')
    return 1 if broken else 0


if __name__ == '__main__':
    sys.exit(main())
