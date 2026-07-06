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


def label(title, names, cap=2):
    """A compact multi-line node label: a bold-ish title, up to `cap` method names, then a +N tail.

    Short labels are the single biggest lever against the wide, overlapping nodes long lists produce;
    <br/> wraps so a node grows in height (cheap) instead of width (expensive). ASCII separators only
    (a middle-dot can trip GitHub's Mermaid parser).
    """
    shown = [f"{n}()" for n in names[:cap]]
    body = " / ".join(shown) if shown else "-"
    tail = f"<br/>+{len(names) - cap} more" if len(names) > cap else ""
    return f"{title}<br/>{body}{tail}"


def short(names, cap=3):
    """A ' / '-joined, capped list with a +N tail (ASCII only - a middle-dot trips GitHub's parser)."""
    body = " / ".join(names[:cap])
    return body + (f" / +{len(names) - cap}" if len(names) > cap else "")


def mermaid():
    api = public_methods()
    protos = protocols()
    pres = presentation_modules()
    proto_short = short([p for p, _ in protos])

    # A theme + layout directive: soft palette, curved edges, and roomy spacing so nodes never overlap.
    init = (
        "%%{init: {'theme':'base','themeVariables':{"
        "'fontFamily':'ui-sans-serif,system-ui,Segoe UI,Roboto,sans-serif','fontSize':'13px',"
        "'lineColor':'#94a3b8','clusterBkg':'transparent','clusterBorder':'#cbd5e1'},"
        "'flowchart':{'curve':'basis','nodeSpacing':42,'rankSpacing':50,'padding':10,'useMaxWidth':true}}}%%"
    )
    out = [init, "flowchart TB"]
    out.append("  %% Auto-generated from the public API, proto_builtins.cpp, and presentation/ on disk.")
    # A single top-to-bottom spine: request flows down to your handler, response flows back to the client.
    # No per-layer boxes (they stretch to enclose both the incoming and outgoing node of a layer, which
    # is what made this sprawl) - the layer is shown by colour instead, per the key above the diagram.
    # Setup runs once at boot; keep it as a small reference panel, wired in with one faint dashed edge.
    out.append('  subgraph SETUP["First, set up your server (once, at boot)"]')
    out.append("    direction LR")
    out.append(f'    reg["{label("1 Register routes", api["Register"])}"]')
    out.append(f'    cfg["{label("2 Set options", api["Configure"])}"]')
    out.append(f'    run["{label("3 Start it", api["Run"])}"]')
    out.append("  end")
    out.append("")
    # One straight waterfall: a client sends at the top, the request flows down to your handler, the
    # response flows on down, and a client receives at the bottom - so there are no long back-edges. The
    # bidirectional UDP socket is drawn as its receive + send directions for the same reason. Node colour
    # is the OSI layer (L4 amber, L5 green seams, L6 blue, L7 indigo), per the key above the diagram.
    out.append('  cin(["A client sends a request<br/>browser / app / curl"])')
    out.append('  listen["Accept a connection<br/>listener_accept_cb"]')
    out.append('  ring[("Hold the bytes<br/>conn_pool + rx ring")]')
    out.append('  udprx["Receive a datagram<br/>det_udp"]')
    out.append(f'  seam{{{{"Which protocol?<br/>ProtoHandler seam<br/>{proto_short}"}}}}')
    out.append('  tls["Decrypt + choose version<br/>det_tls + ALPN"]')
    out.append('  parser["Read HTTP/1.1<br/>http_parser"]')
    out.append('  h2["Decode HTTP/2<br/>h2_conn"]')
    out.append('  h3["Decode HTTP/3<br/>quic_conn + h3_conn"]')
    out.append('  mae{{"Find the matching route<br/>match_and_execute"}}')
    out.append('  mw["Run your middleware"]')
    out.append('  routes[("Route table")]')
    out.append('  handler>"YOUR HANDLER runs"]')
    out.append(f'  resp["{label("Build the response", api["Respond"])}"]')
    out.append('  sink{{"Frame the reply per protocol<br/>resp_sink seam<br/>HTTP/1.1 / h2 / h3"}}')
    out.append('  consend["Write bytes back<br/>det_conn_send"]')
    out.append('  udptx["Send a datagram<br/>det_udp"]')
    out.append('  cout(["The client gets the response"])')
    out.append("")

    # Edges. Track response-path edge indices so linkStyle can tint them a distinct colour.
    edges = []
    res = []

    def edge(line, is_res=False):
        if is_res:
            res.append(len(edges))
        edges.append("  " + line)

    edge("run -.->|starts| listen")  # the one setup->flow link, faint + dashed
    # Request in: a client's bytes travel down to your handler.
    edge("cin ==>|TCP| listen")
    edge("listen --> ring")
    edge("ring --> seam")
    edge("cin ==>|UDP / QUIC| udprx")
    edge("udprx --> seam")
    edge("seam --> tls")
    edge("tls -->|HTTP/1.1| parser")
    edge("tls -->|ALPN h2| h2")
    edge("seam -->|HTTP/3| h3")
    edge("parser --> mae")
    edge("h2 --> mae")
    edge("h3 --> mae")
    edge("mae --> mw")
    edge("mw --> routes")
    edge("routes --> handler")
    edge("handler --> resp")
    # Response out (green): one seam frames the reply for whatever protocol, then it reaches the client.
    edge("resp --> sink", True)
    edge("sink -->|TCP: 1.1, h2| consend", True)
    edge("sink -->|QUIC: h3| udptx", True)
    edge("consend ==> cout", True)
    edge("udptx ==> cout", True)
    out += edges
    out.append("")

    # Palette: one soft colour per OSI layer, an accent for the two seams and the two you-touch parts.
    out.append("  class cin,cout ext;")  # class statement, not inline :::ext (GitHub rejects inline)
    out.append("  class reg,cfg,run,routes setup;")
    out.append("  class listen,ring,udprx,udptx,consend l4;")
    out.append("  class seam,sink seam;")
    out.append("  class tls,parser,h2,h3 l6;")
    out.append("  class mae,mw l7;")
    out.append("  class handler,resp you;")
    out.append("  classDef ext fill:#0f172a,stroke:#0f172a,color:#fff;")
    out.append("  classDef setup fill:#f1f5f9,stroke:#94a3b8,color:#334155;")
    out.append("  classDef l4 fill:#fff7ed,stroke:#fb923c,color:#7c2d12;")
    out.append("  classDef seam fill:#10b981,stroke:#047857,color:#fff;")
    out.append("  classDef l6 fill:#eff6ff,stroke:#60a5fa,color:#1e3a8a;")
    out.append("  classDef l7 fill:#eef2ff,stroke:#818cf8,color:#3730a3;")
    out.append("  classDef you fill:#f59e0b,stroke:#b45309,color:#3b2508;")
    out.append("  style SETUP fill:#f8fafc,stroke:#cbd5e1,stroke-width:1px;")
    # Tint the response path green so "reply going out" is visually distinct from "request coming in".
    out.append(f"  linkStyle {','.join(str(i) for i in res)} stroke:#10b981,stroke-width:2.5px;")
    return "\n".join(out)


def build_block():
    return "\n".join(
        [
            BEGIN,
            "",
            "> Generated from the public API, `proto_builtins.cpp`, and `presentation/` by"
            " `docs/utilities/gen_api_flow.py` - do not edit by hand.",
            "",
            "**How to read it:** follow the arrows. A **request comes in** at the top from a client, travels"
            " **down** through the four OSI layers - L4 wire bytes, L5 protocol pick, L6 decode into a request,"
            " L7 your routes - your handler runs, and the **response goes back out** along the **green** arrows."
            " Each box shows a plain-English step with the exact function underneath. You only write the two"
            " **amber** parts: register your routes (top) and your handler (middle).",
            "",
            "The one idea worth taking away: every HTTP version (1.1, 2, 3) is decoded into the *same* request"
            " and answered through *one* response seam, so your routes and handlers never care which protocol a"
            " client used.",
            "",
            "| Colour | Layer |",
            "| --- | --- |",
            "| Amber outline | **L4 Transport** - raw bytes on/off the wire |",
            "| Green | **L5 Session** - the two seams that pick the protocol in and frame the reply out |",
            "| Blue | **L6 Presentation** - decrypt + turn bytes into a request |",
            "| Indigo | **L7 Application** - route matching + your handlers |",
            "| Solid amber fill | the parts **you** write |",
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
