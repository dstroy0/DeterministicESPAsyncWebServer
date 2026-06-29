# Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
# SPDX-License-Identifier: AGPL-3.0-or-later
"""SNMP interop: query the device agent with net-snmp (snmpget / snmpwalk).

Role: the *device is the agent*; this peer is the manager. net-snmp is the
canonical reference manager, so a pass proves real-world interop. Requires the
net-snmp command-line tools on PATH (Debian/Ubuntu: `apt install snmp`;
Windows: the Net-SNMP installer or `choco install net-snmp`).
"""

from __future__ import annotations

import shutil
import subprocess

from ._common import Probe

NAME = "snmp"
HELP = "query the device SNMP agent with net-snmp (snmpget/snmpwalk)"

# sysDescr.0 - the first scalar every MIB-II agent answers.
SYSDESCR = "1.3.6.1.2.1.1.1.0"
SYSTEM = "1.3.6.1.2.1.1"


def add_args(p) -> None:
    p.add_argument("--host", required=True, help="device IP / hostname")
    p.add_argument("--port", type=int, default=161, help="agent UDP port (default 161)")
    p.add_argument("--community", default="public", help="v2c community (default public)")
    p.add_argument("--version", default="2c", choices=["1", "2c"], help="SNMP version")
    p.add_argument("--timeout", type=float, default=3.0, help="per-request timeout seconds")


def _invoke(args, tool: str, *oid_and_extra) -> tuple[int, str]:
    target = f"{args.host}:{args.port}"
    cmd = [tool, "-v", args.version, "-c", args.community, "-t", str(args.timeout), "-r", "1", target, *oid_and_extra]
    try:
        cp = subprocess.run(cmd, capture_output=True, text=True, timeout=args.timeout * 4 + 5)
        out = (cp.stdout + cp.stderr).strip()
        return cp.returncode, out
    except subprocess.TimeoutExpired:
        return 124, "timed out"


def run(args) -> bool:
    pr = Probe(f"snmp {args.host}:{args.port} v{args.version}")
    snmpget = shutil.which("snmpget")
    snmpwalk = shutil.which("snmpwalk")
    if not snmpget or not snmpwalk:
        pr.check("net-snmp tools on PATH", False, "install net-snmp (snmpget/snmpwalk)")
        return pr.summary()

    rc, out = _invoke(args, snmpget, SYSDESCR)
    pr.check("GET sysDescr.0", rc == 0 and "=" in out, out.splitlines()[0] if out else "no output")

    rc, out = _invoke(args, snmpwalk, SYSTEM)
    lines = [ln for ln in out.splitlines() if "=" in ln]
    pr.check("WALK system group returns rows", rc == 0 and len(lines) >= 1, f"{len(lines)} varbinds")

    # A GETNEXT past a known leaf must not error (endOfMibView is fine, not a crash).
    rc, out = _invoke(args, snmpget, "1.3.6.1.2.1.1.1")  # the object, no instance
    pr.check("GET non-instance is answered (no hang/abort)", rc in (0, 1, 2), out.splitlines()[0] if out else "")

    return pr.summary()
