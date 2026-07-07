#!/usr/bin/env python3
# Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Generate the build-flag dependency flowchart from the config header's own guards.

The single source of truth is src/ServerConfig.h. A child feature that cannot compile
without a parent is enforced there by a preprocessor guard whose #error says "... requires ...":

    #if DETWS_ENABLE_CHILD && !DETWS_ENABLE_PARENT
    #error "... DETWS_ENABLE_CHILD requires DETWS_ENABLE_PARENT"
    #endif

and a PSRAM-class feature by a guard whose #error mentions PSRAM:

    #if DETWS_ENABLE_HTTP2 && defined(ARDUINO) && !DETWS_H2_POOL_IN_PSRAM
    #error "... DETWS_ENABLE_HTTP2 needs PSRAM ..."
    #endif

This script walks the header with a preprocessor-condition stack. For each #error it reads the
active conditions - the positive DETWS_ENABLE_* term is the feature being built (the child), each
negated DETWS_* term a prerequisite - AND the error message: a "requires" message yields a hard
feature edge, a "PSRAM" message a resource gate, anything else (a worker-stack floor, a size range)
is ignored. Gating on the message is what keeps a scoping clause like `!DETWS_ENABLE_SSH` in a
stack-size guard from being mistaken for "OIDC requires SSH". Auto-derived flags (a `#define
DETWS_ENABLE_X 1` guarded by an OR of other ENABLE flags) are read too. The graph is injected into
the "Build-flag dependencies" section of README.md between generated markers, so it can never drift
from the guards the compiler actually enforces.

Run from the repo root:
    python docs/utilities/gen_flag_deps.py            # rewrite the README block
    python docs/utilities/gen_flag_deps.py --check    # CI: exit 1 if stale
"""

import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
CONFIG_H = os.path.join(ROOT, "src", "ServerConfig.h")
README = os.path.join(ROOT, "README.md")
DIAGRAMS = os.path.join(ROOT, "docs", "diagrams")

BEGIN = "<!-- BEGIN GENERATED FLAG DEPS (docs/utilities/gen_flag_deps.py) -->"
END = "<!-- END GENERATED FLAG DEPS -->"

ENABLE = re.compile(r"(!?)\s*DETWS_ENABLE_(\w+)")  # positive (child) / negated (parent) enable flags
NEG = re.compile(r"!\s*(DETWS_\w+)")  # any negated DETWS_* prerequisite (enable flag or resource knob)
NUM = re.compile(r"([A-Z_][A-Z0-9_]*\s*[<>]=?\s*\d+)")  # a numeric side-condition, used as an edge label


def error_message(lines, i):
    """Join the #error statement at line i across backslash continuations into one string."""
    parts = [lines[i].strip()]
    k = i
    while k < len(lines) and lines[k].rstrip().endswith("\\") and k + 1 < len(lines):
        k += 1
        parts.append(lines[k].strip())
    return " ".join(parts)


def parse_guards(text):
    """Return (hard, resource, derived) edges parsed from the header's guards.

    hard:     (parent_flag, child_flag)     - child requires parent (#error "... requires ...")
    resource: (child_flag, label)           - child is PSRAM-class  (#error "... PSRAM ...")
    derived:  (source_flag, derived_flag)   - derived_flag = ... || source_flag || ...
    """
    hard, resource, derived = set(), set(), set()
    stack = []  # active #if / #elif condition strings (innermost last)
    lines = text.splitlines()
    for i, raw in enumerate(lines):
        s = raw.strip()
        if s.startswith("#if ") or s.startswith("#ifdef ") or s.startswith("#ifndef "):
            stack.append(s[4:] if s.startswith("#if ") else "")  # ifdef/ifndef hold no ENABLE terms we track
        elif s.startswith("#elif "):
            if stack:
                stack[-1] = s[6:]
        elif s.startswith("#else"):
            if stack:
                stack[-1] = ""  # an #else branch never holds the deps we extract
        elif s.startswith("#endif"):
            if stack:
                stack.pop()
        elif s.startswith("#error"):
            active = " && ".join(c for c in stack if c)
            children = {m.group(2) for m in ENABLE.finditer(active) if m.group(1) != "!"}
            if not children:
                continue
            msg = error_message(lines, i)
            prereqs = NEG.findall(active)
            if "requires" in msg:
                for prereq in prereqs:
                    if prereq.startswith("DETWS_ENABLE_"):
                        parent = prereq[len("DETWS_ENABLE_") :]
                        for child in children:
                            if parent != child:
                                hard.add((parent, child))
            elif "PSRAM" in msg:
                m = NUM.search(stack[-1] if stack else "")
                # A raw < / > breaks a Mermaid edge label, so spell the operator out.
                label = m.group(1).replace(" ", "").replace(">", " gt ").replace("<", " lt ") if m else ""
                for prereq in prereqs:
                    if "PSRAM" in prereq and "ACK" not in prereq:
                        for child in children:
                            resource.add((child, label))
        elif s.startswith("#define DETWS_ENABLE_"):
            # Auto-derived flag: `#define DETWS_ENABLE_X 1` guarded by an OR of other ENABLE flags.
            active = " || ".join(c for c in stack if c)
            m = re.match(r"#define\s+DETWS_ENABLE_(\w+)\s+1\b", s)
            if not m or "DETWS_ENABLE_" not in active:
                continue
            dflag = m.group(1)
            for src in {e for e in re.findall(r"DETWS_ENABLE_(\w+)", active) if e != dflag}:
                derived.add((src, dflag))
    return hard, resource, derived


def node_class(hard, resource, derived):
    """Assign each node exactly one class, precedence derived > parent > child."""
    parents = {p for p, _ in hard}
    derived_flags = {d for _, d in derived}
    nodes = set()
    for p, c in hard:
        nodes |= {p, c}
    nodes |= {c for c, _ in resource}
    nodes |= {s for s, _ in derived} | derived_flags
    cls = {}
    for n in nodes:
        cls[n] = "derived" if n in derived_flags else "parent" if n in parents else "child"
    return cls


def mermaid(hard, resource, derived):
    # Layout only (curved edges, roomy spacing). No 'theme' override so GitHub swaps its light / dark
    # Mermaid theme automatically; the classDefs use translucent fills + no fixed text color to match.
    init = (
        "%%{init: {'themeVariables':{"
        "'fontFamily':'ui-sans-serif,system-ui,Segoe UI,Roboto,sans-serif','fontSize':'13px',"
        "'lineColor':'#94a3b8'},"  # soft slate-gray edges, gentle on both light and dark
        "'flowchart':{'curve':'basis','nodeSpacing':40,'rankSpacing':55,'padding':8,'useMaxWidth':true}}}%%"
    )
    out = [init, "flowchart TD"]
    out.append("  %% Reading: A --> B means B requires A (enable the parent to build the child).")
    out.append("  %% Auto-generated from the #error / #if guards in src/ServerConfig.h.")
    out.append("")
    if resource:
        out.append('  PSRAM(["PSRAM<br/>or *_ACK_DRAM"])')
    for p, c in sorted(hard):
        out.append(f"  {p} --> {c}")
    for c, label in sorted(resource):
        out.append(f"  PSRAM -. {label} .-> {c}" if label else f"  PSRAM -.-> {c}")
    for src, d in sorted(derived):
        out.append(f"  {src} -. derived .-> {d}")
    out.append("")
    cls = node_class(hard, resource, derived)
    for klass in ("parent", "child", "derived"):
        members = sorted(n for n, k in cls.items() if k == klass)
        if members:
            out.append(f"  class {','.join(members)} {klass};")
    if resource:
        out.append(
            "  class PSRAM res;"
        )  # a class statement, not inline :::res (GitHub rejects inline class on a shaped node)
    # Translucent fills (~15% alpha) + accent stroke + no fixed text color, so nodes read on either the
    # light or dark GitHub background (the page tints through) and the theme's text color stays legible.
    out.append("  classDef parent fill:#10b98126,stroke:#059669,stroke-width:1.5px;")
    out.append("  classDef child fill:#6366f126,stroke:#6366f1,stroke-width:1.5px;")
    out.append("  classDef derived fill:#a855f726,stroke:#a855f7,stroke-width:1.5px;")
    out.append("  classDef res fill:#f9731626,stroke:#f97316,stroke-width:1.5px;")
    # Thicker links - the default hairline is hard to follow (>= 2x).
    out.append("  linkStyle default stroke-width:2px;")
    return "\n".join(out)


def build_block():
    hard, resource, derived = parse_guards(open(CONFIG_H, encoding="utf-8").read())
    # Pre-render to a PNG (via tools/render_diagrams.sh) so it shows in the GitHub app + Doxygen too.
    os.makedirs(DIAGRAMS, exist_ok=True)
    with open(os.path.join(DIAGRAMS, "flag_deps.mmd"), "w", encoding="utf-8", newline="\n") as f:
        f.write(mermaid(hard, resource, derived) + "\n")
    return "\n".join(
        [
            BEGIN,
            "",
            "> Generated from the `#error` / `#if` guards in"
            " [src/ServerConfig.h](src/ServerConfig.h) by"
            " `docs/utilities/gen_flag_deps.py` - do not edit by hand. Pre-rendered PNG (shows in the"
            " GitHub app + Doxygen); mermaid source: [`docs/diagrams/flag_deps.mmd`](docs/diagrams/flag_deps.mmd).",
            "",
            "**Green** = a parent feature; **blue** = a child that requires it (hard `#error`);"
            " **orange PSRAM** = a PSRAM-class feature (pool cannot fit internal DRAM;"
            " needs `*_IN_PSRAM` or an `*_ACK_DRAM` opt-out); **purple** = an auto-derived flag"
            " (do not set it yourself).",
            "",
            "<picture>",
            '  <source media="(prefers-color-scheme: dark)" srcset="docs/diagrams/flag_deps.dark.png">',
            '  <img alt="Build-flag dependencies" src="docs/diagrams/flag_deps.light.png">',
            "</picture>",
            "",
            f"_{len(hard)} hard dependencies, {len(resource)} PSRAM gates, {len(derived)} derived flags._",
            "",
            END,
        ]
    )


def apply_to(path, check):
    with open(path, "r", encoding="utf-8", newline="") as f:
        text = f.read()
    nl = "\r\n" if "\r\n" in text else "\n"
    flat = text.replace("\r\n", "\n")
    if BEGIN not in flat or END not in flat:
        raise SystemExit(f"{path}: missing the {BEGIN!r} / {END!r} markers")
    block = build_block()
    i = flat.index(BEGIN)
    j = flat.index(END) + len(END)
    updated = (flat[:i] + block + flat[j:]).replace("\n", nl)
    if check:
        if updated != text:
            sys.stderr.write("README flag-deps block is stale - run: python docs/utilities/gen_flag_deps.py\n")
            return 1
        return 0
    if updated != text:
        with open(path, "w", encoding="utf-8", newline="") as f:
            f.write(updated)
        sys.stderr.write(f"updated {os.path.relpath(path, ROOT)}\n")
    else:
        sys.stderr.write("README flag-deps block already up to date\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(apply_to(README, "--check" in sys.argv[1:]))
