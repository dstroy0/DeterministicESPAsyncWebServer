# Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
# SPDX-License-Identifier: AGPL-3.0-or-later
"""SSH interop: complete a full SSH-2 session against the device's server with the reference
implementation, OpenSSH, and prove the modern suite negotiates and data flows encrypted.

Role: the *device is the server* (ConnProto::PROTO_SSH on :22). This peer drives the genuine OpenSSH
client (the authoritative SSH-2 stack) forced onto the device's preferred modern suite -
curve25519-sha256 KEX, ssh-ed25519 host key, chacha20-poly1305@openssh.com record layer - then RFC
4252 password auth and an RFC 4254 shell channel. It proves the device's binary-packet emit path
works end to end, not just the crypto primitives: the KEX-stall bug fixed in v5.95.13 passed every
host unit test because they wired the emit callback themselves; only a real client catches "the server
completes KEX but never actually replies".

OpenSSH is used rather than a pip client because it is *the* reference implementation and interops
with every cipher/MAC the device offers (a common Python SSH stack ships without chacha and then trips
on the aes256-ctr fallback - a client quirk, not a device fault). Needs `ssh` and `sshpass` on PATH
(apt install openssh-client sshpass / brew install openssh sshpass). The rig host key is a throwaway
demo key, so it is not checked against known_hosts - the check is the SSH-2 protocol + auth + channel,
not host trust.
"""

from __future__ import annotations

import shutil
import subprocess
import sys

from ._common import Probe

# Force the device's preferred modern suite so the check is deterministic (not "whatever OpenSSH and
# the device happen to agree on this release"). The device advertises exactly these first.
_KEX = "curve25519-sha256"
_HOSTKEY = "ssh-ed25519"
_CIPHER = "chacha20-poly1305@openssh.com"
_MARKER = "detws-interop-echo-42"


def add_args(p) -> None:
    p.add_argument("--host", required=True, help="device IP / hostname (the SSH server)")
    p.add_argument("--port", type=int, default=22, help="device SSH port (default 22)")
    p.add_argument("--user", default="admin", help="username (default admin)")
    p.add_argument("--password", default="s3cret", help="password (default s3cret)")
    p.add_argument("--timeout", type=float, default=15.0, help="timeout seconds")


NAME = "ssh"
HELP = "complete a full SSH-2 session (curve25519 KEX, ed25519 host key, chacha20 cipher, password auth, channel echo) via OpenSSH (device-as-server)"


def run(args) -> bool:
    pr = Probe(f"ssh device={args.host}:{args.port}")
    ssh = shutil.which("ssh")
    sshpass = shutil.which("sshpass")
    if not ssh or not sshpass:
        missing = ", ".join(n for n, v in (("ssh", ssh), ("sshpass", sshpass)) if not v)
        sys.stderr.write(
            f"error: missing native dependency for the ssh peer ({missing}).\n"
            f"       install: apt install openssh-client sshpass  (or brew install openssh sshpass)\n"
        )
        sys.exit(2)

    cmd = [
        sshpass, "-p", args.password,
        ssh, "-v", "-tt",
        "-p", str(args.port),
        "-o", "StrictHostKeyChecking=no",
        "-o", "UserKnownHostsFile=/dev/null",
        "-o", f"KexAlgorithms={_KEX}",
        "-o", f"HostKeyAlgorithms={_HOSTKEY}",
        "-o", f"Ciphers={_CIPHER}",
        "-o", "PubkeyAuthentication=no",
        "-o", "PreferredAuthentications=password",
        "-o", f"ConnectTimeout={int(args.timeout)}",
        f"{args.user}@{args.host}",
    ]  # fmt: skip

    try:
        # The rig echoes channel bytes, so feed a unique marker on stdin and expect it back on stdout.
        proc = subprocess.run(
            cmd,
            input=(_MARKER + "\n").encode(),
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=args.timeout,
        )
    except FileNotFoundError as exc:  # noqa: BLE001
        pr.check("launch the OpenSSH client", False, str(exc))
        return pr.summary()
    except subprocess.TimeoutExpired:
        pr.check("SSH session completes within the timeout", False, f"timed out after {args.timeout}s")
        return pr.summary()

    trace = proc.stderr.decode("latin1")
    out = proc.stdout.decode("latin1")

    def traced(needle: str) -> bool:
        return needle in trace

    pr.check(f"KEX negotiated {_KEX}", traced(f"kex: algorithm: {_KEX}"), _kex_line(trace))
    pr.check(f"host key is {_HOSTKEY}", traced(f"kex: host key algorithm: {_HOSTKEY}"), _hostkey_line(trace))
    pr.check(
        f"server->client cipher is {_CIPHER}",
        traced(f"server->client cipher: {_CIPHER}"),
        _cipher_line(trace),
    )
    pr.check("NEWKEYS: encryption activated both directions", traced("SSH2_MSG_NEWKEYS received"), "")
    pr.check(
        "password auth accepted (RFC 4252)",
        "Authenticated to " in trace and 'using "password"' in trace,
        _auth_line(trace),
    )
    pr.check(
        "channel data echoes back over the encrypted session (RFC 4254 CHANNEL_DATA)",
        _MARKER in out,
        out.replace("\n", " ")[:48].strip(),
    )
    return pr.summary()


def _grep1(trace: str, needle: str) -> str:
    for line in trace.splitlines():
        if needle in line:
            return line.split("debug1: ")[-1].strip()
    return ""


def _kex_line(trace: str) -> str:
    return _grep1(trace, "kex: algorithm:")


def _hostkey_line(trace: str) -> str:
    return _grep1(trace, "kex: host key algorithm:")


def _cipher_line(trace: str) -> str:
    return _grep1(trace, "server->client cipher:")


def _auth_line(trace: str) -> str:
    return _grep1(trace, "Authenticated to")
