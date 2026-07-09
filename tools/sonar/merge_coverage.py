#!/usr/bin/env python3
"""Union per-env SonarQube generic-coverage reports into one, with optional baseline overlay.

gcov cannot merge the same source compiled with different flags across envs in a
single pass (it raises a worker exception), so gen_coverage.sh emits one report
per env and this unions them: a line is covered if it is covered in ANY env, and
the per-line branch counts from the env that covered the most branches are kept.
Output is a single SonarQube generic-coverage report (one <file> per path).

Incremental (affected-only) CI runs pass --baseline: only the envs affected by the
diff run, so the fresh reports cover only some files. The committed coverage.xml is
overlaid so the output is still whole-project:

  * a file listed in --changed (the .cpp that actually changed) is REPLACED by its
    fresh coverage - the baseline's line numbers are stale after the edit, and the
    changed feature's env(s) ran to completion so the fresh view is authoritative;
  * any other file present in a fresh report (a shared header, or a neighbour source
    recompiled because it shares an env) is UNIONED with the baseline - a partial run
    only exercised part of its callers, so replacing would wrongly lower it;
  * a file only in the baseline (not recompiled this run) is kept verbatim.

Usage:
    merge_coverage.py <out.xml> <report-glob>
    merge_coverage.py <out.xml> <report-glob> --baseline coverage.xml --changed changed.txt
"""

import argparse
import glob
import xml.etree.ElementTree as ET


def parse_report(path, into):
    """Union one SonarQube report at `path` into the {path: {line: [cov,btc,cb]}} dict `into`."""
    try:
        root = ET.parse(path).getroot()
    except (ET.ParseError, FileNotFoundError):
        return False
    for f in root.findall("file"):
        _union_file(into.setdefault(f.get("path"), {}), f)
    return True


def _union_line(lines, ln, cov, btc, cb):
    if ln not in lines:
        lines[ln] = [cov, btc, cb]
    else:
        cur = lines[ln]
        cur[0] = cur[0] or cov
        # keep the branch pair from whichever env covered the most branches
        if (cb if cb is not None else -1) > (cur[2] if cur[2] is not None else -1):
            cur[1], cur[2] = btc, cb


def _union_file(lines, file_el):
    for lc in file_el.findall("lineToCover"):
        try:
            ln = int(lc.get("lineNumber"))
        except (TypeError, ValueError):
            continue  # malformed entry (no / non-integer lineNumber): skip
        if ln < 1:
            continue  # Sonar requires lineNumber >= 1; a stray line-0 entry would break the whole report
        cov = lc.get("covered") == "true"
        btc = lc.get("branchesToCover")
        cb = lc.get("coveredBranches")
        _union_line(lines, ln, cov, int(btc) if btc is not None else None, int(cb) if cb is not None else None)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("out", nargs="?", default="coverage.xml")
    ap.add_argument("reports", nargs="?", default="coverage_reports/*.xml", help="glob of per-env reports")
    ap.add_argument("--baseline", help="committed coverage.xml to overlay under the fresh reports")
    ap.add_argument("--changed", help="file with one changed path per line; these are replaced, not unioned")
    args = ap.parse_args()

    # 1) Union the fresh per-env reports (this run's actual coverage).
    files = {}
    nrep = 0
    for rep in sorted(glob.glob(args.reports)):
        if parse_report(rep, files):
            nrep += 1

    # 2) Overlay the baseline so the report stays whole-project on a partial run.
    if args.baseline:
        changed = set()
        if args.changed:
            try:
                with open(args.changed, encoding="utf-8") as fh:
                    changed = {ln.strip().replace("\\", "/") for ln in fh if ln.strip()}
            except FileNotFoundError:
                pass
        base = {}
        parse_report(args.baseline, base)
        for path, lines in base.items():
            if path in changed:
                continue  # changed source: baseline is stale, keep only the fresh view
            if path not in files:
                files[path] = lines  # untouched file: keep the baseline verbatim
            else:
                for ln, val in lines.items():  # shared/recompiled: union (never lower)
                    _union_line(files[path], ln, val[0], val[1], val[2])

    # Emit only Sonar-valid <file> elements. The generic-coverage parser rejects a file with no
    # <lineToCover>, an empty path, a lineNumber < 1, or coveredBranches > branchesToCover; a single
    # such entry (e.g. a line-0 artefact from a differing gcov version) makes the WHOLE report
    # unparseable ("Error during parsing of the generic coverage report"). Drop the offenders here so a
    # tool quirk degrades one line instead of zeroing all coverage, and name them so CI stays diagnosable.
    cov_el = ET.Element("coverage", {"version": "1"})
    dropped = []
    for path in sorted(files):
        if not path or not files[path]:
            dropped.append(path or "<no-path>")
            continue
        fe = ET.SubElement(cov_el, "file", {"path": path})
        for ln in sorted(files[path]):
            cov, btc, cb = files[path][ln]
            attrs = {"lineNumber": str(ln), "covered": "true" if cov else "false"}
            if btc is not None:
                cov_b = cb if cb is not None else 0
                if cov_b > btc:  # never report more covered branches than exist
                    cov_b = btc
                attrs["branchesToCover"] = str(btc)
                attrs["coveredBranches"] = str(cov_b)
            ET.SubElement(fe, "lineToCover", attrs)
    ET.ElementTree(cov_el).write(args.out, encoding="utf-8", xml_declaration=True)
    tag = f" (+baseline {args.baseline})" if args.baseline else ""
    emitted = len(files) - len(dropped)
    print(f"merged {nrep} reports{tag} -> {args.out}: {emitted} files, {sum(len(v) for v in files.values())} lines")
    if dropped:
        print(f"  dropped {len(dropped)} empty/invalid <file> entries: {', '.join(dropped[:10])}")


if __name__ == "__main__":
    main()
