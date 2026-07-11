#!/usr/bin/env python3
# Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
# SPDX-License-Identifier: AGPL-3.0-or-later
"""select_envs.py - map a set of changed files to the native test envs they affect.

CI runs the whole ~200-env native matrix on every push, which is slow. Most commits
touch one feature's `.cpp` + its `test_x/` dir, so only one env is actually affected.
This reads the changed-file list (git diff --name-only, on stdin or via --changed-file)
and prints exactly one of:

    FULL                    run the entire matrix (a shared/core/header/config/tooling
                            file changed, or --full was forced)
    NONE                    no test env is affected (e.g. a docs-only change)
    native_x native_y ...   the space-separated set of affected envs

The mapping is exact. For BOTH sources and headers under src/ it consults the compiler
dependency graph (test/dep_graph.json, built by tools/gen_dep_graph.py from `g++ -MM`):
a file maps to exactly the envs whose translation units include it, so a feature header
like coap.h hits only native_coap / native_coap_observe, not the whole matrix. A changed
`test/test_x/` maps via the envs whose `test_filter` names it. Only a file absent from the
graph (a brand-new source/header, or when the graph is missing) falls back to the
build_src_filter mapping / FULL - we would rather over-run than silently skip a suite. A
change hitting >=90% of envs (a shared header) is reported FULL so coverage regenerates
cleanly.

Usage:
    git diff --name-only origin/main...HEAD | python3 test/select_envs.py
    python3 test/select_envs.py --changed-file src/services/opcua/opcua.cpp
    python3 test/select_envs.py --full            # -> FULL (forced)
"""

import argparse
import fnmatch
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
INI = os.path.join(ROOT, "platformio.ini")
DEP_GRAPH = os.path.join(HERE, "dep_graph.json")

# A changed header/source mapping (via the compiler dep graph) to at least this fraction of the
# testable envs is a shared header - run FULL (regenerate coverage cleanly) rather than an
# "affected" run that would merge nearly every row and overlay coverage for no changed source.
FULL_FRACTION = 0.9

# Envs that exist in the ini but are never part of the report suite (special tooling
# targets); never select them even when "affected".
NEVER_SELECT = {"native_pentest", "native_codeql"}

# A changed file matching any of these forces FULL: it can influence how every env
# builds or what the whole matrix looks like, so no narrow subset is trustworthy.
FORCE_FULL_EXACT = {
    "platformio.ini",
    "test/test_matrix.json",
    "test/gen_test_envs.py",
    "test/select_envs.py",
    "test/run_tests.sh",
    "test/run_tests.ps1",
    "test/merge_report.py",
    "test/gen_test_readme.py",
    "sonar-project.properties",
    "library.json",
    "library.properties",
    "tools/sonar/merge_coverage.py",
}
FORCE_FULL_PREFIX = (
    "tools/",
    ".github/workflows/",
    "test/mocks/",
    "test/support/",
    "test/fixtures/",
    "test/servers/",
)

# A changed file under any of these has no bearing on the native suite - ignore it.
IGNORE_PREFIX = (
    "docs/",
    "examples/",
    ".vscode/",
    ".github/ISSUE_TEMPLATE/",
)
IGNORE_EXACT = {
    "README.md",
    "CHANGELOG.md",
    "LICENSE",
    ".gitignore",
    ".clang-format",
    ".prettierignore",
    ".editorconfig",
    "CONTRIBUTING.md",
    "SECURITY.md",
    # CI-committed outputs / data - changing these must not trigger a suite.
    "test/TEST_REPORT.md",
    "test/coverage.xml",
    "test/dep_graph.json",
}


def parse_envs(ini_path):
    """Return {env_name: {"src": [globs...], "tests": [dirs...]}} for native_* envs."""
    envs = {}
    cur = None
    section = None  # "src" | "tests" | None
    with open(ini_path, encoding="utf-8") as fh:
        for raw in fh:
            line = raw.rstrip("\n")
            m = re.match(r"^\[env:(native[A-Za-z0-9_]*)\]\s*$", line)
            if m:
                cur = m.group(1)
                envs[cur] = {"src": [], "tests": []}
                section = None
                continue
            if line.startswith("["):  # any other section ends the current env
                cur = None
                section = None
                continue
            if cur is None:
                continue
            if re.match(r"^\s*build_src_filter\s*=", line):
                section = "src"
                rest = line.split("=", 1)[1].strip()
                if rest:
                    _add_src(envs[cur], rest)
                continue
            if re.match(r"^\s*test_filter\s*=", line):
                section = "tests"
                rest = line.split("=", 1)[1].strip()
                if rest:
                    envs[cur]["tests"].append(rest)
                continue
            if re.match(r"^\s*[A-Za-z_]+\s*=", line):  # a different key ends the list
                section = None
                continue
            if section == "src" and line.strip():
                _add_src(envs[cur], line.strip())
            elif section == "tests" and line.strip():
                envs[cur]["tests"].append(line.strip())
    return envs


def _add_src(env, token):
    # Only additive '+<glob>' entries define what an env compiles; '-<...>' excludes and
    # bare tokens are ignored for affected-detection (erring toward running more).
    m = re.match(r"^\+<(.+)>$", token)
    if m:
        env["src"].append(m.group(1))


def _match_glob(rel, glob):
    # PlatformIO src globs use '*'; treat it as matching across path separators too so a
    # broad '+<dir/*>' still catches nested files (over-approximate -> safe).
    return fnmatch.fnmatch(rel, glob) or fnmatch.fnmatch(rel, glob.replace("*", "**"))


def load_graph():
    """{repo-relative path -> [envs]} from test/dep_graph.json (compiler -MM output), or {}.

    Empty (missing file) preserves the pre-graph behaviour: src/ changes fall back to the
    build_src_filter mapping and any header returns FULL.
    """
    try:
        with open(DEP_GRAPH, encoding="utf-8") as fh:
            return json.load(fh)
    except (OSError, ValueError):
        return {}


def classify(changed, envs, graph, total_testable):
    """Return "FULL", "NONE", or a sorted list of affected env names."""
    affected = set()
    src_globs = [(name, g) for name, e in envs.items() for g in e["src"]]
    test_map = {}
    for name, e in envs.items():
        for t in e["tests"]:
            test_map.setdefault(t, set()).add(name)

    saw_relevant = False
    for f in changed:
        f = f.strip().replace("\\", "/")
        if not f:
            continue
        if f in FORCE_FULL_EXACT or f.startswith(FORCE_FULL_PREFIX):
            return "FULL"
        if f in IGNORE_EXACT or f.startswith(IGNORE_PREFIX):
            continue
        if f.startswith("src/"):
            # The compiler dep graph maps a source OR header to exactly the envs whose include
            # closure contains it - so a feature header hits only its feature's envs, not FULL.
            if f in graph:
                affected |= set(graph[f]) - NEVER_SELECT
                saw_relevant = True
                continue
            if f.endswith((".cpp", ".c")):
                rel = f[len("src/") :]
                hits = {name for name, g in src_globs if _match_glob(rel, g)}
                if not hits:
                    return "FULL"  # a compiled source we cannot attribute to a feature env
                affected |= hits
                saw_relevant = True
                continue
            return "FULL"  # a header/source absent from the graph (new file, or graph stale)
        if f.startswith("test/"):
            m = re.match(r"^test/(test_[A-Za-z0-9_]+)(/|$)", f)
            if m:
                dirs = test_map.get(m.group(1))
                if not dirs:
                    return "FULL"  # a test dir with no env owning it
                affected |= dirs
                saw_relevant = True
                continue
            return "FULL"  # shared test infra under test/ not caught above
        # Anything else (unknown top-level path) -> be safe.
        return "FULL"

    affected -= NEVER_SELECT
    if not affected:
        return "NONE"
    # A shared header hits almost every env; run FULL so coverage regenerates from scratch instead
    # of an "affected" run that merges nearly every row with nothing changed to overlay.
    if total_testable and len(affected) >= FULL_FRACTION * total_testable:
        return "FULL"
    return sorted(affected)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--full", action="store_true", help="force FULL regardless of the diff")
    ap.add_argument("--changed-file", action="append", default=[], help="a changed path (repeatable)")
    ap.add_argument("--ini", default=INI, help="platformio.ini path")
    args = ap.parse_args()

    if args.full:
        print("FULL")
        return

    changed = list(args.changed_file)
    if not sys.stdin.isatty():
        changed += [ln for ln in sys.stdin.read().splitlines()]
    changed = [c for c in changed if c.strip()]
    if not changed:
        print("NONE")
        return

    envs = parse_envs(args.ini)
    graph = load_graph()
    total_testable = len(set(envs) - NEVER_SELECT)
    result = classify(changed, envs, graph, total_testable)
    if isinstance(result, list):
        print(" ".join(result))
    else:
        print(result)


if __name__ == "__main__":
    main()
