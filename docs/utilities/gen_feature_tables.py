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
CONFIG_H = os.path.join(ROOT, "src", "ServerConfig.h")

# Internal derived flags: auto-set from other flags, not user-facing opt-ins, so they
# get no FEATURES.md entry of their own. Every other DETWS_ENABLE_* must be documented
# (the coverage guard below fails CI otherwise - this is how the whole industrial-protocol
# wave once drifted out of the feature grid unnoticed).
INTERNAL_FLAGS = {
    "DETWS_ENABLE_STREAM_BODY",  # = OTA || UPLOAD || WEBDAV (shared parser machinery)
    "DETWS_ENABLE_CLIENT_TLS",  # = HTTP_CLIENT_TLS || MQTT_TLS || WS_CLIENT_TLS
}

BEGIN = "<!-- BEGIN GENERATED FEATURE TABLES (docs/utilities/gen_feature_tables.py) -->"
END = "<!-- END GENERATED FEATURE TABLES -->"

COLUMNS = 5

# Features are grouped into tables by the OSI layer they live at (the same view the
# README's Overview describes), and alphabetized within each layer. LAYER_ORDER is the
# render order; LAYER_MEMBERS lists the FEATURES.md headings that belong to each
# non-application layer - everything not listed falls through to "Application (L7)"
# (most features are application-layer services). Edit a set to move an entry.
LAYER_ORDER = [
    "Foundation",
    "Physical & Data Link (L1-L2)",
    "Network (L3)",
    "Transport (L4)",
    "Session (L5)",
    "Presentation (L6)",
    "Application (L7)",
]
APPLICATION_LAYER = "Application (L7)"
LAYER_MEMBERS = {
    "Foundation": {
        "Config IO",
        "Config Store",
        "Device ID",
        "DMA Peripheral Ingest",
        "Exception Decoder",
        "Failsafe Watchdog",
        "GPIO Map",
        "Guardrails",
        "Hardware Health",
        "Preempting Work Queue",
        "PSRAM Pool",
        "RTC",
        "Sleep Scheduler",
        "Southbound",
        "Time Source",
        "VFS",
        "Wear Leveling",
    },
    "Physical & Data Link (L1-L2)": {
        "ADS1115",
        "BLE GATT",
        "Bus Capture",
        "CC1101",
        "DShot",
        "EnOcean",
        "ESP-NOW",
        "Ethernet",
        "FDC2214",
        "INA219",
        "Interface Forwarding",
        "LD2410",
        "LDC1614",
        "LoRa",
        "MPR121",
        "nRF24",
        "PCA9685",
        "PN532",
        "Radio Gateway",
        "Radio Power",
        "Radio Sniffer",
        "Raw L2",
        "SHT3x",
        "Sigfox",
        "Thread",
        "VL53L0X",
        "Wi-Fi Capture",
        "Wi-Fi Sniffer",
        "Wi-SUN",
        "Z-Wave",
        "Zigbee",
    },
    "Network (L3)": {
        "Dns Resolver",
        "Happy Eyeballs",
        "IPv6",
        "Link Manager",
        "Network Adaptation",
        "Proxy Protocol",
    },
    "Transport (L4)": {
        "Accept Throttle",
        "IP Allowlist",
        "Keep-Alive",
        "MTLS",
        "Per IP Throttle",
        "Socket Pool",
        "TLS",
        "TLS Policy",
        "TLS Resumption",
    },
    "Session (L5)": {
        "SSH",
        "SSH Compression",
        "Telnet",
    },
    "Presentation (L6)": {
        "Auth",
        "Auth Lockout",
        "CBOR",
        "CloudEvents",
        "HTTP Delivery",
        "HTTP/1.1 Parser",
        "HTTP/2",
        "HTTP/3",
        "JSON",
        "JWT",
        "MessagePack",
        "Multipart",
        "Protobuf",
        "SenML",
        "SSE",
        "Web Terminal",
        "WebSocket",
        "WS Deflate",
    },
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
    return text.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace('"', "&quot;")


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
    """Render one HTML table with a merged, centered header spanning all columns.

    All tables are full width (`width="100%"`) and their cells centered
    (`align="center"`) so they render consistently on GitHub, which honors only those
    presentational attributes. The short last row carries no empty padding cells (the
    absent columns simply collapse). The `feature-table` class lets the docs site
    (docs/custom.css) apply the themed borders / equal columns / hover on top.
    """
    out = [
        '<table class="feature-table" width="100%">',
        f'<thead><tr><th colspan="{COLUMNS}" align="center">{html_escape(title)}</th></tr></thead>',
        "<tbody>",
    ]
    for r in range(0, len(rows), COLUMNS):
        chunk = rows[r : r + COLUMNS]
        out.append("<tr>")
        for name, desc in chunk:
            href = f"{link_prefix}#{github_anchor(name)}"
            out.append(
                f'  <td align="center"><a href="{html_escape(href)}" title="{html_escape(desc)}">{html_escape(name)}</a></td>'
            )
        out.append("</tr>")  # no padding: absent trailing cells collapse
    out += ["</tbody>", "</table>"]
    return "\n".join(out)


def layer_of(name):
    for layer, members in LAYER_MEMBERS.items():
        if name in members:
            return layer
    return APPLICATION_LAYER


def build_block(link_prefix):
    entries = parse_features(FEATURES_MD)
    # Validate the layer map so a renamed/removed heading is caught, not silently dropped.
    known = {e[0] for e in entries}
    mapped = set().union(*LAYER_MEMBERS.values())
    missing = mapped - known
    if missing:
        raise SystemExit(f"LAYER_MEMBERS headings not found in FEATURES.md: {sorted(missing)}")

    by_layer = {layer: [] for layer in LAYER_ORDER}
    for name, desc in entries:
        by_layer[layer_of(name)].append((name, desc))

    parts = [BEGIN, ""]
    for layer in LAYER_ORDER:
        rows = sorted(by_layer[layer], key=lambda e: e[0].lower())
        if not rows:
            continue
        parts += [render_table(layer, rows, link_prefix), ""]
    parts.append(END)
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


def check_flag_coverage():
    """Fail if a DETWS_ENABLE_* flag in the config header has no FEATURES.md entry
    (excluding the internal derived flags). Guards against a shipped feature silently
    never reaching the feature grid."""
    cfg = open(CONFIG_H, "r", encoding="utf-8").read()
    feat = open(FEATURES_MD, "r", encoding="utf-8").read()
    defined = set(re.findall(r"^#define (DETWS_ENABLE_[A-Z0-9_]+) 0", cfg, re.M))
    documented = set(re.findall(r"^`(DETWS_ENABLE_[A-Z0-9_]+)`", feat, re.M))
    missing = sorted(defined - documented - INTERNAL_FLAGS)
    if missing:
        raise SystemExit(
            "FEATURES.md is missing entries for these DETWS_ENABLE_* flags "
            "(add a `## Name` section, or list the flag in INTERNAL_FLAGS if it is "
            f"internal): {missing}"
        )


def main():
    check = "--check" in sys.argv[1:]
    check_flag_coverage()
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
