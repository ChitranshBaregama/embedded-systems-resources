#!/usr/bin/env python3
"""Verify that every link in every markdown file resolves.

Two classes of link are checked:

  relative paths   ./foo/bar.md        -> the file must exist
  in-page anchors  #some-heading       -> a heading must produce that slug

The anchor check is the one that earns its keep. A table of contents whose
entries look right but scroll nowhere is worse than no table of contents,
and nothing about it is visible when you skim the rendered page.

External http(s) links are not checked - that needs network access and
turns a deterministic check into a flaky one. Anything inside a fenced
code block or inline backticks is skipped, or C like `swap(h, e)` reads as
a markdown link and the checker cries wolf.
"""
import re
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LINK = re.compile(r'\]\(([^)\s]+)(?:\s+"[^"]*")?\)')
FENCE = re.compile(r'^\s*(```|~~~)')
INLINE_CODE = re.compile(r'`[^`\n]*`')
HEADING = re.compile(r'^(#{1,6})\s+(.*?)\s*#*$')
SKIP = re.compile(r'^(https?://|mailto:|tel:)')


def slug(text):
    """Reproduce GitHub's heading-anchor algorithm.

    Lowercase, drop everything that is not alphanumeric / space / hyphen /
    underscore, then spaces to hyphens. Note that an em dash is *removed*
    rather than replaced, so 'Chapter 1 - Foo' with an em dash yields a
    double hyphen. Getting that detail wrong is why hand-written anchors
    into long documents so often miss.
    """
    text = re.sub(r'\[([^\]]*)\]\([^)]*\)', r'\1', text)   # strip link syntax
    text = text.replace('`', '')
    text = text.lower()
    text = re.sub(r'[^\w\s-]', '', text, flags=re.UNICODE)
    # One hyphen PER space, not per run. GitHub does not collapse them, so
    # 'Part 01 - Bit Manipulation' with an em dash - which is deleted above,
    # leaving two spaces - anchors as 'part-01--bit-manipulation'. Collapsing
    # the run here makes every anchor into a long document look broken when
    # it is fine, which is exactly what the first version of this file did.
    return re.sub(r'\s', '-', text.strip())


def parse(text):
    """Return (anchors, links) for one file, ignoring code."""
    anchors, links, seen = set(), [], Counter()
    in_fence = False
    for n, line in enumerate(text.splitlines(), 1):
        if FENCE.match(line):
            in_fence = not in_fence
            continue
        if in_fence:
            continue

        m = HEADING.match(line)
        if m:
            base = slug(m.group(2))
            seen[base] += 1
            anchors.add(base if seen[base] == 1 else f'{base}-{seen[base] - 1}')

        for match in LINK.finditer(INLINE_CODE.sub('', line)):
            links.append((n, match.group(1)))
    return anchors, links


def main():
    docs = {p: parse(p.read_text(encoding='utf-8', errors='replace'))
            for p in sorted(ROOT.rglob('*.md')) if '.git' not in p.parts}

    broken, checked_paths, checked_anchors = [], 0, 0

    for md, (_, links) in docs.items():
        for line_no, target in links:
            if SKIP.match(target):
                continue
            path_part, _, anchor = target.partition('#')

            if path_part:
                checked_paths += 1
                dest = (md.parent / path_part).resolve()
                if not dest.exists():
                    broken.append((md, line_no, target, 'no such file'))
                    continue
            else:
                dest = md

            if anchor:
                checked_anchors += 1
                target_doc = docs.get(dest)
                if target_doc is None:
                    continue          # anchor into a non-markdown file
                if anchor.lower() not in target_doc[0]:
                    broken.append((md, line_no, target, 'no such heading'))

    for f, n, t, why in broken:
        print(f'BROKEN  {f.relative_to(ROOT)}:{n}  ->  {t}   ({why})')

    print(f'\n{len(docs)} files, {checked_paths} paths and '
          f'{checked_anchors} anchors checked, {len(broken)} broken')
    return 1 if broken else 0


if __name__ == '__main__':
    sys.exit(main())
