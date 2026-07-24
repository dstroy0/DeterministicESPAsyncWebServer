#!/usr/bin/env python3
"""Fail if a ``src/`` C/C++ file uses a construct banned by ``docs/SRCBANNED.md``.

This is the mechanical guardrail behind that checklist: the pre-commit hook runs it on the staged
``src/`` sources and refuses the commit if any banned construct appears, so a violation can never
land. It scans for the machine-detectable hard bans - unbounded ``strlen``, ``<stdlib.h>`` and its
heap / parse functions, the ``auto`` keyword, blocking ``delay()``, the non-reentrant ``gmtime`` /
``localtime`` / ``ctime`` / ``asctime`` family, and em-dashes.

Ban #5 (bare ``millis()``) is deliberately *not* enforced here: it is a "for new timing" rule, so a
whole-file scan cannot distinguish a new call from a grandfathered timing site, and the clock source
(``services/clock.h``) must call the platform ``millis()`` to provide ``dws_millis()``. It stays a
review item (``rg -n '\bmillis\s*\(' src/``) rather than a mechanical gate.

Comments and string / char literals are blanked out first (line numbers preserved), so a construct
merely *named* in a comment ("without <string.h> / strlen") is not a violation - only real code is.

Usage::

    python tools/check_src_banned.py <file>...   # scan the given files (pre-commit: the staged set)
    python tools/check_src_banned.py --all        # scan every C/C++ file under src/

Exit status is 1 if any violation is found (with a file:line report on stderr), else 0.
Only paths under ``src/`` are scanned; ``examples/`` and ``test/`` are exempt per docs/SRCBANNED.md.
"""

import pathlib
import re
import sys

SRC = pathlib.Path("src")
EXTS = {".c", ".cc", ".cpp", ".h", ".hpp", ".ino"}

# (compiled pattern, ban number, message). Patterns run on comment/string-stripped code. Every
# use-instead pattern (dwsdelay, dws_millis, strnlen, gmtime_r) survives because the banned token is
# not on a word boundary there ("dws_millis" has no boundary before "millis", etc.).
BANS = [
    (re.compile(r"\bstrlen\s*\("), 1, "strlen (unbounded read); use strnlen(p, cap)"),
    (re.compile(r"#\s*include\s*<c?stdlib\.h>"), 2, "<stdlib.h>/<cstdlib>; no heap / hidden parse"),
    (re.compile(r"\b(?:malloc|calloc|realloc|free|aligned_alloc)\s*\("), 2, "heap allocation; use fixed BSS buffers"),
    (
        re.compile(r"\b(?:atoi|atol|atoll|strtol|strtoll|strtoul|strtoull|strtod|strtof|qsort|srand|rand)\s*\("),
        2,
        "stdlib parse/util function; hand-roll it",
    ),
    (re.compile(r"\bauto\b"), 3, "auto keyword; spell the explicit type"),
    (re.compile(r"\bdelay\s*\("), 4, "delay(); use dwsdelay(ms) from services/clock.h"),
    (re.compile(r"\b(?:gmtime|localtime|ctime|asctime)\s*\("), 8, "non-reentrant time; use the _r form"),
    (re.compile("—"), 7, "em-dash; use a comma / parentheses / a linking word"),
]

# Blank out // line comments, /* */ block comments, and string / char literals, keeping newlines so
# reported line numbers stay accurate. The leftmost-match rule protects "http://" inside a string.
_STRIP = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'', re.DOTALL)


def _blank(match):
    return re.sub(r"[^\n]", " ", match.group(0))


def scan_file(path):
    """Return a list of (path, line_no, ban_no, message) violations in one file."""
    try:
        code = pathlib.Path(path).read_text(encoding="utf-8", errors="replace")
    except OSError:
        return []
    clean = _STRIP.sub(_blank, code)
    hits = []
    for line_no, line in enumerate(clean.splitlines(), 1):
        for pattern, ban_no, message in BANS:
            if pattern.search(line):
                hits.append((str(path), line_no, ban_no, message))
    return hits


def _norm(path):
    return str(path).replace("\\", "/")


def collect(argv):
    if "--all" in argv:
        return [p for p in SRC.rglob("*") if p.suffix in EXTS]
    return [f for f in argv if pathlib.Path(f).suffix in EXTS and _norm(f).startswith("src/")]


def main(argv):
    violations = []
    for path in collect(argv):
        violations.extend(scan_file(path))
    if violations:
        print("check_src_banned: banned constructs in src/ (see docs/SRCBANNED.md):", file=sys.stderr)
        for path, line_no, ban_no, message in violations:
            print(f"  {path}:{line_no}: [ban #{ban_no}] {message}", file=sys.stderr)
        print(
            f"check_src_banned: {len(violations)} violation(s) - fix them; src/ is fully constrained.", file=sys.stderr
        )
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
