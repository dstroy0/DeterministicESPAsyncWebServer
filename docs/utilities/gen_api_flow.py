#!/usr/bin/env python3
# Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Generate the core-API request-lifecycle flowchart from the source.

The variable parts are read straight from the code so the picture cannot drift:

  - the public `DetWebServer` API methods, from src/DeterministicESPAsyncWebServer.h
    (access-specifier aware), bucketed into Register / Configure / Run / Respond;
  - the built-in application protocols, from the session registry
    src/network_drivers/session/proto_builtins.cpp (each `register_if(PROTO_x, ...)`);
  - the Layer-6 modules present on disk, from src/network_drivers/presentation/.

Those are placed into the fixed OSI request-lifecycle skeleton (transport L4 -> session L5
dispatch seam -> presentation L6 -> application L7 route dispatch -> response back out the
transport), which is the library's architecture (docs/ARCHITECTURE.md). The diagram is injected
into the README "Overview" between generated markers.

Run from the repo root:
    python docs/utilities/gen_api_flow.py            # rewrite the README block
    python docs/utilities/gen_api_flow.py --check    # CI: exit 1 if stale
"""

import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
API_H = os.path.join(ROOT, "src", "DeterministicESPAsyncWebServer.h")
PROTO_CPP = os.path.join(ROOT, "src", "network_drivers", "session", "proto_builtins.cpp")
PRESENTATION = os.path.join(ROOT, "src", "network_drivers", "presentation")
README = os.path.join(ROOT, "README.md")

BEGIN = "<!-- BEGIN GENERATED API FLOW (docs/utilities/gen_api_flow.py) -->"
END = "<!-- END GENERATED API FLOW -->"

# name -> API group. First matching rule wins; a startswith() prefix or an exact-name set.
GROUPS = ["Register", "Configure", "Run", "Respond"]
RULES = [
    ("Register", ("on", "serve_static", "dav", "use")),
    ("Configure", ("set_", "tls_", "enable_", "require_")),
    ("Run", ("begin", "handle", "service_once", "stop", "defer")),
    ("Respond", ("send", "redirect", "serve_file", "stats", "metrics", "sse_send", "diag")),
]
# A method declaration: `identifier( ...args... ) [const];` - args may span lines but hold no ;{}.
DECL = re.compile(r"\b([a-z]\w*)\s*\([^;{}]*\)\s*(?:const\s*)?;", re.S)
NOT_METHODS = {"if", "for", "while", "switch", "return", "sizeof", "operator"}


def strip_comments(text):
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)  # block comments (incl. @code braces)
    return re.sub(r"//[^\n]*", "", text)  # line comments


def class_body(text, name):
    """The brace-matched body of `class <name> { ... }` (its definition, not a forward decl)."""
    m = re.search(r"\bclass\s+" + re.escape(name) + r"\b[^;{]*\{", text, re.S)
    if not m:
        raise SystemExit(f"class {name} definition not found")
    depth, start = 0, m.end() - 1  # points at the opening brace
    for idx in range(start, len(text)):
        depth += (text[idx] == "{") - (text[idx] == "}")
        if depth == 0:
            return text[start + 1 : idx]
    raise SystemExit(f"class {name} body not closed")


def public_methods():
    """Parse the public method names of `class DetWebServer`, in bucket order."""
    body = class_body(strip_comments(open(API_H, encoding="utf-8").read()), "DetWebServer")
    # Split the body into (access, chunk) regions; a class body defaults to private.
    parts = re.split(r"\b(public|private|protected)\s*:", body)
    regions = [("private", parts[0])] + [(parts[k], parts[k + 1]) for k in range(1, len(parts) - 1, 2)]
    names = []
    for access, chunk in regions:
        if access != "public":
            continue
        for mm in DECL.finditer(chunk):
            if mm.group(1) not in NOT_METHODS and mm.group(1) not in names:
                names.append(mm.group(1))
    buckets = {g: [] for g in GROUPS}
    for n in names:
        for group, keys in RULES:
            if any(n == k or (k.endswith("_") and n.startswith(k)) or n.startswith(k) for k in keys):
                buckets[group].append(n)
                break
    return buckets


def protocols():
    """Parse the built-in protocols from the session registry (name, always_on)."""
    text = open(PROTO_CPP, encoding="utf-8").read()
    out = []
    for pm in re.finditer(r"register_if\(PROTO_(\w+),", text):
        name = pm.group(1)
        # "always present" ones carry that note on their register line; the rest sit behind a flag.
        line = text[text.rfind("\n", 0, pm.start()) + 1 : text.find("\n", pm.start())]
        out.append((name, "always present" in line))
    return out


def presentation_modules():
    """Layer-6 module directories that actually exist on disk."""
    if not os.path.isdir(PRESENTATION):
        return []
    return sorted(d for d in os.listdir(PRESENTATION) if os.path.isdir(os.path.join(PRESENTATION, d)))


def label(names, cap=5):
    shown = names[:cap]
    text = " / ".join(f"{n}()" for n in shown)  # ASCII separator (a middle-dot can trip GitHub's parser)
    if len(names) > cap:
        text += f" +{len(names) - cap}"
    return text or "-"


def mermaid():
    api = public_methods()
    protos = protocols()
    pres = presentation_modules()
    proto_names = " / ".join(p for p, _ in protos)
    pres_names = " / ".join(pres)

    out = ["flowchart TB"]
    out.append("  %% Auto-generated from the public API, proto_builtins.cpp, and presentation/ on disk.")
    out.append('  client(("client"))')
    out.append("")
    out.append('  subgraph APP["Application L7 - DetWebServer"]')
    out.append(f'    reg["Register: {label(api["Register"])}"]')
    out.append(f'    cfg["Configure: {label(api["Configure"])}"]')
    out.append(f'    run["Run: {label(api["Run"])}"]')
    out.append('    mae[["match_and_execute"]]')
    out.append('    mw["middleware chain"]')
    out.append('    routes[("route table")]')
    out.append('    handler>"your Handler"]')
    out.append(f'    resp["Respond: {label(api["Respond"])}"]')
    out.append("  end")
    out.append("")
    out.append(f'  subgraph L6["Presentation L6 - {pres_names}"]')
    out.append('    tls["det_tls decrypt + ALPN"]')
    out.append('    parser["http_parser fills http_pool slot"]')
    out.append('    h2["h2_conn"]')
    out.append('    h3["quic_conn + h3_conn"]')
    out.append("  end")
    out.append("")
    out.append('  subgraph L5["Session L5 - worker task"]')
    out.append('    tick["server_tick / dispatch_event"]')
    out.append(f'    seam{{{{"ProtoHandler seam: {proto_names}"}}}}')
    out.append("  end")
    out.append("")
    out.append('  subgraph L4["Transport L4"]')
    out.append('    listen["listener_accept_cb"]')
    out.append('    ring[("conn_pool slot + rx ring")]')
    out.append('    udp["det_udp listeners"]')
    out.append('    consend["det_conn_send"]')
    out.append("  end")
    out.append("")
    # registration + startup
    out.append("  reg --> routes")
    out.append("  run --> listen")
    # RX path
    out.append("  client -- TCP --> listen --> ring --> tick")
    out.append("  client -- UDP / QUIC --> udp --> tick")
    out.append("  tick --> seam")
    out.append("  seam -- TLS/TCP --> tls --> parser")
    out.append("  seam -- h2 --> h2")
    out.append("  seam -- HTTP/3 --> h3")
    out.append("  parser -- PARSE_COMPLETE --> mae")
    out.append("  h2 --> mae")
    out.append("  h3 --> mae")
    # dispatch
    out.append("  mae --> mw --> routes --> handler --> resp")
    # TX path
    out.append("  resp -- HTTP/1.1 --> consend --> client")
    out.append("  resp -- h2 --> h2")
    out.append("  resp -- HTTP/3 --> h3 --> udp --> client")
    out.append("")
    out.append(
        "  class client ext;"
    )  # class statement, not inline :::ext (GitHub rejects inline class on a shaped node)
    out.append("  classDef ext fill:#e85d04,stroke:#9d0208,color:#fff;")
    return "\n".join(out)


def build_block():
    return "\n".join(
        [
            BEGIN,
            "",
            "> Generated from the public API, `proto_builtins.cpp`, and `presentation/` by"
            " `docs/utilities/gen_api_flow.py` - do not edit by hand.",
            "",
            "How a request flows through the OSI layers: the app registers routes and calls `begin()`,"
            " the transport (L4) rings inbound bytes to a worker, the session (L5) dispatches through the"
            " protocol-agnostic `ProtoHandler` seam, the presentation (L6) turns bytes into a request, and"
            " every version converges on the one `match_and_execute` / `Handler` / `Respond` path (L7).",
            "",
            "```mermaid",
            mermaid(),
            "```",
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
            sys.stderr.write("README api-flow block is stale - run: python docs/utilities/gen_api_flow.py\n")
            return 1
        return 0
    if updated != text:
        with open(path, "w", encoding="utf-8", newline="") as f:
            f.write(updated)
        sys.stderr.write(f"updated {os.path.relpath(path, ROOT)}\n")
    else:
        sys.stderr.write("README api-flow block already up to date\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(apply_to(README, "--check" in sys.argv[1:]))
