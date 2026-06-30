#!/usr/bin/env python3
"""Merge per-environment compile_commands.json fragments into one, deduped by file.

PlatformIO writes compile_commands.json to the project root one env at a time, and
no single env enables all of the ~111 DETWS_ENABLE_* features, so a feature-gated
source file is only compiled in the env that turns its flag on. To give the
SonarQube C/C++ analyzer a command for *every* file, gen_compiledb.sh runs
`pio run -t compiledb` for each native env, stashes each result, and this script
merges them: the first env that compiled a given file wins (one command per file).

Usage: merge_compiledb.py <out_path> <fragments_glob>
"""
import glob
import json
import sys


def main():
    out_path = sys.argv[1] if len(sys.argv) > 1 else "compile_commands.json"
    frag_glob = sys.argv[2] if len(sys.argv) > 2 else "compiledb_frags/*.json"

    frags = sorted(glob.glob(frag_glob))
    seen = {}
    order = []
    for frag in frags:
        with open(frag, encoding="utf-8") as f:
            try:
                entries = json.load(f)
            except json.JSONDecodeError:
                continue
        for e in entries:
            key = e.get("file")
            if not key or key in seen:
                continue
            seen[key] = e
            order.append(key)

    merged = [seen[k] for k in order]
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(merged, f, indent=2)
    print(f"merged {len(merged)} unique files from {len(frags)} env fragments -> {out_path}")


if __name__ == "__main__":
    main()
