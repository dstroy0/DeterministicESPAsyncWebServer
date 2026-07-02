#!/usr/bin/env python3
# Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Generate the README feature/codec tables from docs/FEATURES.md.

docs/FEATURES.md is the single source of truth: every feature is a `## Name`
heading, an optional `` `DETWS_ENABLE_*` `` flag line, and a description
paragraph. This script parses those entries, splits them into a FEATURES table
and a CODECS table, and writes both (as HTML tables with a merged header) into
the marked region of README.md and docs/README.md. The hover tooltip is the
feature's description; the link target is the FEATURES.md anchor.

Run from the repo root:
    python docs/utilities/gen_feature_tables.py            # rewrite the tables
    python docs/utilities/gen_feature_tables.py --check    # CI: fail if stale

Keeping the tables generated means they can never drift from FEATURES.md (a
hand-maintained grid silently lost 28 of 106 features before this existed).
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
FEATURES_MD = os.path.join(ROOT, "docs", "FEATURES.md")

BEGIN = "<!-- BEGIN GENERATED FEATURE TABLES (docs/utilities/gen_feature_tables.py) -->"
END = "<!-- END GENERATED FEATURE TABLES -->"

COLUMNS = 5

# The CODECS table holds the standalone wire-format / protocol message codecs
# (encode/decode building blocks). Everything else - servers, clients, auth,
# transport, and core HTTP machinery - stays in the FEATURES table. Membership
# is by FEATURES.md heading; edit this set to move an entry between the tables.
CODEC_HEADINGS = {
    "AMQP",
    "BACnet",
    "C37.118",
    "CANopen",
    "CBOR",
    "CIP",
    "CloudEvents",
    "COTP",
    "DeviceNet",
    "DF1",
    "DMX512",
    "DNP3",
    "EnOcean",
    "EtherNet/IP",
    "FINS",
    "Flow Export",
    "gRPC-Web",
    "Host Link",
    "IEC 60870",
    "IO-Link",
    "J1939",
    "JSON",
    "LwM2M",
    "M-Bus",
    "MELSEC",
    "MessagePack",
    "Modbus RTU",
    "MQTT SN",
    "Multipart",
    "NATS",
    "NMEA 0183",
    "NMEA 2000",
    "Protobuf",
    "Proxy Protocol",
    "Redis",
    "S7comm",
    "SDI-12",
    "SenML",
    "Sparkplug",
    "Stomp",
    "SunSpec",
    "WAMP",
}

# The DRIVERS table holds chip-specific hardware drivers - a radio / peripheral register or
# command protocol talking to a real IC - as opposed to the byte-format codecs above (a
# codec builds/parses bytes; a driver drives a chip). Membership is by FEATURES.md heading.
DRIVER_HEADINGS = {
    "LoRa",
    "nRF24",
    "PN532",
    "Sigfox",
    "Z-Wave",
    "Zigbee",
}

# Where each target file's links to FEATURES.md point.
TARGETS = {
    os.path.join(ROOT, "README.md"): "docs/FEATURES.md",
    os.path.join(ROOT, "docs", "README.md"): "FEATURES.md",
}


def github_anchor(heading):
    """Reproduce GitHub's heading-to-anchor rule (lowercase, drop punctuation
    other than spaces and hyphens, spaces to hyphens)."""
    a = heading.strip().lower()
    a = re.sub(r"[^a-z0-9 \-]", "", a)
    return a.replace(" ", "-")


def html_escape(text):
    return (
        text.replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;")
        .replace('"', "&quot;")
    )


def parse_features(path):
    """Return [(name, description)] in document order. The description is the
    first prose paragraph under the heading (skipping the optional flag line)."""
    with open(path, "r", encoding="utf-8") as f:
        lines = f.read().splitlines()

    entries = []
    i = 0
    while i < len(lines):
        m = re.match(r"^## (.+?)\s*$", lines[i])
        if not m:
            i += 1
            continue
        name = m.group(1).strip()
        i += 1
        # Skip blank lines, then an optional `` `FLAG` `` line, then blanks.
        while i < len(lines) and not lines[i].strip():
            i += 1
        if i < len(lines) and re.match(r"^`DETWS_[A-Z0-9_]+`\s*$", lines[i].strip()):
            i += 1
            while i < len(lines) and not lines[i].strip():
                i += 1
        # Collect the description paragraph (until a blank line or next heading).
        desc = []
        while i < len(lines) and lines[i].strip() and not lines[i].startswith("#"):
            desc.append(lines[i].strip())
            i += 1
        entries.append((name, " ".join(desc).strip()))
    return entries


def render_table(title, rows, link_prefix):
    """Render one HTML table with a merged header spanning all columns."""
    out = ["<table>", f'<thead><tr><th colspan="{COLUMNS}">{title}</th></tr></thead>', "<tbody>"]
    for r in range(0, len(rows), COLUMNS):
        chunk = rows[r : r + COLUMNS]
        out.append("<tr>")
        for name, desc in chunk:
            href = f"{link_prefix}#{github_anchor(name)}"
            out.append(
                f'  <td><a href="{html_escape(href)}" title="{html_escape(desc)}">{html_escape(name)}</a></td>'
            )
        for _ in range(COLUMNS - len(chunk)):  # pad the last row
            out.append("  <td></td>")
        out.append("</tr>")
    out += ["</tbody>", "</table>"]
    return "\n".join(out)


def build_block(link_prefix):
    entries = parse_features(FEATURES_MD)
    features = [e for e in entries if e[0] not in CODEC_HEADINGS and e[0] not in DRIVER_HEADINGS]
    codecs = [e for e in entries if e[0] in CODEC_HEADINGS]
    drivers = [e for e in entries if e[0] in DRIVER_HEADINGS]
    missing = (CODEC_HEADINGS | DRIVER_HEADINGS) - {e[0] for e in entries}
    if missing:
        raise SystemExit(f"CODEC_HEADINGS / DRIVER_HEADINGS not found in FEATURES.md: {sorted(missing)}")
    parts = [
        BEGIN,
        "",
        render_table("FEATURES", features, link_prefix),
        "",
        render_table("CODECS", codecs, link_prefix),
        "",
        render_table("DRIVERS", drivers, link_prefix),
        "",
        END,
    ]
    return "\n".join(parts)


def apply_to(path, link_prefix, check):
    with open(path, "r", encoding="utf-8", newline="") as f:
        text = f.read()
    nl = "\r\n" if "\r\n" in text else "\n"
    flat = text.replace("\r\n", "\n")
    if BEGIN not in flat or END not in flat:
        raise SystemExit(f"{path}: missing the {BEGIN!r} / {END!r} markers")
    block = build_block(link_prefix)
    i = flat.index(BEGIN)
    j = flat.index(END) + len(END)
    new = (flat[:i] + block + flat[j:]).replace("\n", nl)
    if check:
        return new == text
    if new != text:
        with open(path, "w", encoding="utf-8", newline="") as f:
            f.write(new)
        print(f"updated {os.path.relpath(path, ROOT)}")
    else:
        print(f"unchanged {os.path.relpath(path, ROOT)}")
    return True


def main():
    check = "--check" in sys.argv[1:]
    ok = True
    for path, prefix in TARGETS.items():
        result = apply_to(path, prefix, check)
        if check and not result:
            print(f"STALE: {os.path.relpath(path, ROOT)} (run gen_feature_tables.py)")
            ok = False
    if check and not ok:
        sys.exit(1)


if __name__ == "__main__":
    main()
