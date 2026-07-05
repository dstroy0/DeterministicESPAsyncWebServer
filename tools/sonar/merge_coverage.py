#!/usr/bin/env python3
"""Union per-env SonarQube generic-coverage reports into one.

gcov cannot merge the same source compiled with different flags across envs in a
single pass (it raises a worker exception), so gen_coverage.sh emits one report
per env and this unions them: a line is covered if it is covered in ANY env, and
the per-line branch counts from the env that covered the most branches are kept.
Output is a single SonarQube generic-coverage report (one <file> per path).

Usage: merge_coverage.py <out.xml> <report-glob>
"""

import glob
import sys
import xml.etree.ElementTree as ET


def main():
    out_path = sys.argv[1] if len(sys.argv) > 1 else "coverage.xml"
    rep_glob = sys.argv[2] if len(sys.argv) > 2 else "coverage_reports/*.xml"

    # files[path][line] = [covered(bool), branchesToCover(int|None), coveredBranches(int|None)]
    files = {}
    nrep = 0
    for rep in sorted(glob.glob(rep_glob)):
        try:
            root = ET.parse(rep).getroot()
        except ET.ParseError:
            continue
        nrep += 1
        for f in root.findall("file"):
            lines = files.setdefault(f.get("path"), {})
            for lc in f.findall("lineToCover"):
                ln = int(lc.get("lineNumber"))
                cov = lc.get("covered") == "true"
                btc = lc.get("branchesToCover")
                cb = lc.get("coveredBranches")
                btc = int(btc) if btc is not None else None
                cb = int(cb) if cb is not None else None
                if ln not in lines:
                    lines[ln] = [cov, btc, cb]
                else:
                    cur = lines[ln]
                    cur[0] = cur[0] or cov
                    # keep the branch pair from whichever env covered the most branches
                    if (cb if cb is not None else -1) > (cur[2] if cur[2] is not None else -1):
                        cur[1], cur[2] = btc, cb

    cov_el = ET.Element("coverage", {"version": "1"})
    for path in sorted(files):
        fe = ET.SubElement(cov_el, "file", {"path": path})
        for ln in sorted(files[path]):
            cov, btc, cb = files[path][ln]
            attrs = {"lineNumber": str(ln), "covered": "true" if cov else "false"}
            if btc is not None:
                attrs["branchesToCover"] = str(btc)
                attrs["coveredBranches"] = str(cb if cb is not None else 0)
            ET.SubElement(fe, "lineToCover", attrs)
    ET.ElementTree(cov_el).write(out_path, encoding="utf-8", xml_declaration=True)
    print(f"merged {nrep} reports -> {out_path}: {len(files)} files, {sum(len(v) for v in files.values())} lines")


if __name__ == "__main__":
    main()
