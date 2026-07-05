#!/usr/bin/env python3
# Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Generate the interactive build configurator (docs/configurator.html).

Single source of truth: src/DetWebServerConfig.h. This parses that header for
every user-toggleable feature flag (`#ifndef DETWS_ENABLE_X / #define ... 0|1`),
every override-able tuning knob (`#ifndef KNOB / #define KNOB value`), the section
titles the file already groups them under, and the hard dependencies encoded as
`#if DETWS_ENABLE_child && !DETWS_ENABLE_parent` guards. It emits one self-contained
HTML page (inline CSS/JS, data embedded as JSON) that lets you tick features, tune
their knobs, and copy out either a platformio.ini `build_flags` block or a set of
`#define`s - emitting only the values that differ from the library defaults.

Because it is generated, the configurator never drifts from the real config:
re-run it whenever DetWebServerConfig.h changes.

    python docs/utilities/gen_configurator.py          # write docs/configurator.html
    python docs/utilities/gen_configurator.py check    # CI gate: fail if stale
"""

import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
CONFIG = os.path.join(ROOT, "src", "DetWebServerConfig.h")
OUT = os.path.join(ROOT, "docs", "configurator.html")

DASH = re.compile(r"^//\s*-{20,}\s*$")
IFNDEF = re.compile(r"^#ifndef\s+([A-Za-z_][A-Za-z0-9_]*)\s*$")
DEFINE = re.compile(r"^#define\s+([A-Za-z_][A-Za-z0-9_]*)\s+(.+?)\s*$")
IF_GATE = re.compile(r"^#if\s+DETWS_ENABLE_([A-Za-z0-9_]+)\s*$")
ANY_IF = re.compile(r"^#\s*(if|ifdef|ifndef)\b")
ENDIF = re.compile(r"^#\s*endif\b")
DEP = re.compile(r"#if\s+DETWS_ENABLE_([A-Za-z0-9_]+)\s*&&\s*!\s*DETWS_ENABLE_([A-Za-z0-9_]+)")
REQ = re.compile(r"requires\s+DETWS_ENABLE_([A-Za-z0-9_]+)")
TRAILING = re.compile(r"///<\s*(.*)$")


def clean_brief(text):
    """First sentence of a doc blob, whitespace-collapsed."""
    text = re.sub(r"\s+", " ", text).strip()
    text = re.sub(r"^@brief\s*", "", text)
    # Cut at the first sentence end that is followed by a space + capital / paren.
    m = re.search(r"\.\s", text)
    if m:
        text = text[: m.start() + 1]
    return text.strip()


def title_of(line):
    t = line.lstrip("/").strip()
    t = re.sub(r"\s*\(DETWS_ENABLE_[^)]*\)", "", t)
    t = t.split(" - ")[0].strip() if " - " in t else t
    return t


def value_kind(raw):
    v = raw.strip().rstrip("uUlL")
    if re.match(r"^0[xX][0-9A-Fa-f]+$", v):
        return "hex"
    if re.match(r"^-?[0-9]+$", v):
        return "int"
    return "other"


def feature_suffix(name):
    return name[len("DETWS_ENABLE_") :]


def parse(path):
    with open(path, "r", encoding="utf-8") as f:
        lines = f.read().splitlines()

    features = {}  # NAME -> {name, label, default, desc, group}
    knobs = {}  # NAME -> {name, default, kind, desc, group, owner}
    order_feat = []
    order_knob = []
    group = "General"
    gate_stack = []  # feature suffix for each open `#if DETWS_ENABLE_X`, else None
    doc = []  # pending block-comment lines

    i = 0
    n = len(lines)
    while i < n:
        line = lines[i]
        stripped = line.strip()

        # Section title: dashline / title / dashline.
        if DASH.match(stripped) and i + 2 < n and DASH.match(lines[i + 2].strip()):
            t = title_of(lines[i + 1].strip())
            if t and not DASH.match(("// " + t)):
                group = t
            doc = []
            i += 3
            continue

        # Block comment capture (/** ... */ or /* ... */).
        if stripped.startswith("/*"):
            buf = []
            while i < n:
                s = lines[i].strip()
                buf.append(re.sub(r"^/\*+|^\*+/?|\*+/$", "", s).strip("* ").strip())
                if "*/" in lines[i]:
                    break
                i += 1
            doc = [b for b in buf if b]
            i += 1
            continue

        # Standalone // comment: remember as a light doc hint.
        if stripped.startswith("//"):
            doc = [stripped.lstrip("/").strip()]
            i += 1
            continue

        # Conditional nesting: track `#if DETWS_ENABLE_X` gates for knob ownership.
        mg = IF_GATE.match(stripped)
        if mg:
            gate_stack.append(mg.group(1))
            i += 1
            continue
        if ANY_IF.match(stripped) and not IFNDEF.match(stripped):
            gate_stack.append(None)
            i += 1
            continue

        # `#ifndef NAME` + `#define NAME value` = an override-able default.
        mi = IFNDEF.match(stripped)
        if mi and i + 1 < n:
            md = DEFINE.match(lines[i + 1].strip())
            if md and md.group(1) == mi.group(1):
                name = md.group(1)
                rhs = md.group(2)
                tc = TRAILING.search(rhs)
                trailing = tc.group(1).strip() if tc else ""
                value = re.sub(r"\s*///<.*$", "", rhs).strip()
                desc = clean_brief(" ".join(doc)) if doc else trailing
                if not desc:
                    desc = trailing
                if name.startswith("DETWS_ENABLE_"):
                    if name not in features:
                        full = " ".join(doc)
                        me = feature_suffix(name)
                        prose = [p for p in REQ.findall(full) if p != me]
                        features[name] = {
                            "name": name,
                            "label": me,
                            "default": 1 if value.strip() in ("1", "1u") else 0,
                            "desc": desc,
                            "group": group,
                            "prose_req": prose,
                        }
                        order_feat.append(name)
                else:
                    if name not in knobs:
                        owner = next((g for g in reversed(gate_stack) if g), None)
                        knobs[name] = {
                            "name": name,
                            "default": value,
                            "kind": value_kind(value),
                            "desc": desc or trailing,
                            "group": group,
                            "owner": owner,  # feature suffix or None (resolved later)
                        }
                        order_knob.append(name)
                doc = []
                gate_stack.append(None)  # the ifndef guard itself
                i += 1
                continue

        if ENDIF.match(stripped):
            if gate_stack:
                gate_stack.pop()
            doc = []
            i += 1
            continue

        if stripped and not stripped.startswith("#"):
            doc = []
        i += 1

    # Dependencies: child requires parent(s).
    deps = {}
    text = "\n".join(lines)
    for child, parent in DEP.findall(text):
        deps.setdefault(child, [])
        if parent not in deps[child]:
            deps[child].append(parent)

    # Resolve knob ownership: explicit gate, else longest feature-suffix prefix match.
    suffixes = sorted((f["label"] for f in features.values()), key=len, reverse=True)
    for k in knobs.values():
        if k["owner"] and ("DETWS_ENABLE_" + k["owner"]) in features:
            continue
        stem = k["name"][len("DETWS_") :] if k["name"].startswith("DETWS_") else k["name"]
        owner = None
        for suf in suffixes:
            if stem == suf or stem.startswith(suf + "_"):
                owner = suf
                break
        k["owner"] = owner  # None => a core / always-on knob

    return {
        "features": features,
        "order_feat": order_feat,
        "knobs": knobs,
        "order_knob": order_knob,
        "deps": deps,
    }


def build_model(parsed):
    features = parsed["features"]
    knobs = parsed["knobs"]

    # Attach each knob to its owning feature; the rest are core knobs.
    by_owner = {}
    core = []
    for name in parsed["order_knob"]:
        k = knobs[name]
        entry = {"name": name, "default": k["default"], "kind": k["kind"], "desc": k["desc"]}
        if k["owner"] and ("DETWS_ENABLE_" + k["owner"]) in features:
            by_owner.setdefault(k["owner"], []).append(entry)
        else:
            core.append(entry)

    have = set(f["label"] for f in features.values())
    feat_list = []
    for name in parsed["order_feat"]:
        f = features[name]
        suf = f["label"]
        # Union of machine-checked (#if child && !parent) and prose ("requires ...") deps,
        # keeping only parents that are real toggleable features.
        req = []
        for p in parsed["deps"].get(suf, []) + f.get("prose_req", []):
            if p in have and p != suf and p not in req:
                req.append(p)
        feat_list.append(
            {
                "name": name,
                "suffix": suf,
                "default": f["default"],
                "desc": f["desc"],
                "group": f["group"],
                "knobs": by_owner.get(suf, []),
                "requires": req,
            }
        )

    # Preserve first-seen group order.
    groups = []
    for f in feat_list:
        if f["group"] not in groups:
            groups.append(f["group"])

    return {"groups": groups, "features": feat_list, "core": core}


PAGE = r"""<!-- GENERATED by docs/utilities/gen_configurator.py - do not edit by hand. -->
<!-- Edit src/DetWebServerConfig.h and re-run the generator. -->
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8" />
<meta name="viewport" content="width=device-width, initial-scale=1" />
<title>DeterministicESPAsyncWebServer - build configurator</title>
<style>
  :root {
    --bg: #0f1419; --panel: #171d26; --panel2: #1e2632; --line: #2b3543;
    --ink: #e7edf3; --dim: #96a3b3; --accent: #4aa3ff; --accent2: #2b7fd4;
    --ok: #5bd6a0; --warn: #ffcc66; --chip: #223047;
  }
  * { box-sizing: border-box; }
  body {
    margin: 0; background: var(--bg); color: var(--ink);
    font: 15px/1.5 -apple-system, Segoe UI, Roboto, Helvetica, Arial, sans-serif;
  }
  header { padding: 22px 20px 14px; border-bottom: 1px solid var(--line); }
  h1 { margin: 0 0 4px; font-size: 20px; }
  header p { margin: 0; color: var(--dim); max-width: 70ch; }
  .wrap { display: grid; grid-template-columns: 1fr 400px; gap: 0; align-items: start; }
  @media (max-width: 900px) { .wrap { grid-template-columns: 1fr; } .out { position: static !important; } }
  .main { padding: 16px 20px 80px; }
  .toolbar { display: flex; gap: 10px; flex-wrap: wrap; align-items: center; margin: 4px 0 16px; }
  input[type=search] {
    flex: 1 1 220px; min-width: 160px; background: var(--panel); color: var(--ink);
    border: 1px solid var(--line); border-radius: 8px; padding: 9px 12px; font-size: 14px;
  }
  button {
    background: var(--panel2); color: var(--ink); border: 1px solid var(--line);
    border-radius: 8px; padding: 8px 12px; cursor: pointer; font-size: 13px;
  }
  button:hover { border-color: var(--accent2); }
  button.primary { background: var(--accent2); border-color: var(--accent); color: #fff; }
  .count { color: var(--dim); font-size: 13px; }
  .group { margin: 18px 0 6px; color: var(--accent); font-size: 12px; letter-spacing: .06em;
           text-transform: uppercase; border-bottom: 1px solid var(--line); padding-bottom: 4px; }
  .feat { background: var(--panel); border: 1px solid var(--line); border-radius: 10px;
          padding: 10px 12px; margin: 8px 0; }
  .feat.on { border-color: var(--accent2); background: var(--panel2); }
  .feat.hidden { display: none; }
  .frow { display: flex; align-items: flex-start; gap: 10px; }
  .frow input[type=checkbox] { margin-top: 3px; width: 17px; height: 17px; accent-color: var(--accent); }
  .fname { font-weight: 600; font-family: ui-monospace, Menlo, Consolas, monospace; font-size: 13px; }
  .fdesc { color: var(--dim); font-size: 13px; margin-top: 2px; }
  .badge { font-size: 11px; color: var(--warn); margin-left: 8px; }
  .badge.dflt { color: var(--ok); }
  .knobs { margin: 10px 0 2px 27px; display: none; }
  .feat.on .knobs { display: block; }
  .knob { display: flex; align-items: center; gap: 8px; margin: 6px 0; flex-wrap: wrap; }
  .knob label { font-family: ui-monospace, Menlo, Consolas, monospace; font-size: 12px; min-width: 220px; }
  .knob input { background: var(--bg); color: var(--ink); border: 1px solid var(--line);
                border-radius: 6px; padding: 5px 8px; width: 130px; font-family: ui-monospace, monospace; }
  .knob input.changed { border-color: var(--warn); }
  .knob .kd { color: var(--dim); font-size: 12px; flex: 1 1 100%; margin-left: 228px; }
  @media (max-width: 620px) { .knob label { min-width: 0; } .knob .kd { margin-left: 0; } }
  .out { position: sticky; top: 0; height: 100vh; display: flex; flex-direction: column;
         border-left: 1px solid var(--line); background: var(--panel); }
  .out .head { padding: 14px 16px 8px; }
  .seg { display: inline-flex; border: 1px solid var(--line); border-radius: 8px; overflow: hidden; }
  .seg button { border: 0; border-radius: 0; }
  .seg button.sel { background: var(--accent2); color: #fff; }
  textarea { flex: 1; margin: 8px 16px 12px; background: var(--bg); color: var(--ink);
             border: 1px solid var(--line); border-radius: 8px; padding: 12px; resize: none;
             font-family: ui-monospace, Menlo, Consolas, monospace; font-size: 12.5px; white-space: pre; }
  .out .foot { padding: 0 16px 16px; display: flex; gap: 10px; align-items: center; }
  .msg { color: var(--warn); font-size: 12px; padding: 0 16px 10px; min-height: 16px; }
</style>
</head>
<body>
<header>
  <h1>DeterministicESPAsyncWebServer &middot; build configurator</h1>
  <p>Tick the features you need and tune their knobs. Dependencies resolve automatically; only
     values that differ from the library defaults are emitted. <b>PlatformIO:</b> copy the result
     into your <code>platformio.ini</code> <code>build_flags</code>. <b>Arduino IDE:</b> switch to the
     <code>build_opt.h</code> tab and <b>Download</b> it into the same folder as your <code>.ino</code>
     &mdash; that is the only file the IDE feeds to the separately-compiled library, so a plain
     <code>#define</code> in the sketch will not turn a feature on. Everything here is generated from
     <code>src/DetWebServerConfig.h</code>.</p>
</header>
<div class="wrap">
  <div class="main">
    <div class="toolbar">
      <input id="q" type="search" placeholder="Filter features (name or description)..." />
      <button id="reset">Reset to defaults</button>
      <span class="count" id="count"></span>
    </div>
    <div id="list"></div>
  </div>
  <aside class="out">
    <div class="head">
      <div class="seg">
        <button data-fmt="pio" class="sel">platformio.ini</button>
        <button data-fmt="buildopt">Arduino build_opt.h</button>
        <button data-fmt="defines">#define</button>
      </div>
    </div>
    <div class="msg" id="msg"></div>
    <textarea id="output" readonly spellcheck="false"></textarea>
    <div class="foot">
      <button class="primary" id="copy">Copy</button>
      <button id="download">Download</button>
      <span class="count" id="ocount"></span>
    </div>
  </aside>
</div>
<script id="model" type="application/json">__DATA__</script>
<script>
const MODEL = JSON.parse(document.getElementById("model").textContent);
const state = { on: {}, knob: {}, fmt: "pio" };
const bySuffix = {};
MODEL.features.forEach(f => { bySuffix[f.suffix] = f; state.on[f.suffix] = !!f.default; });

// requires: child suffix -> [parent suffixes]. dependents: parent -> [children].
const dependents = {};
MODEL.features.forEach(f => (f.requires || []).forEach(p => {
  (dependents[p] = dependents[p] || []).push(f.suffix);
}));

function enableWithParents(suf, seen) {
  seen = seen || {};
  if (seen[suf]) return; seen[suf] = 1;
  state.on[suf] = true;
  (bySuffix[suf].requires || []).forEach(p => bySuffix[p] && enableWithParents(p, seen));
}
function disableWithDependents(suf, seen) {
  seen = seen || {};
  if (seen[suf]) return; seen[suf] = 1;
  state.on[suf] = false;
  (dependents[suf] || []).forEach(c => disableWithDependents(c, seen));
}

function esc(s) { return s.replace(/[&<>"]/g, c => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;" }[c])); }

function render() {
  const list = document.getElementById("list");
  list.innerHTML = "";
  let shown = 0;
  MODEL.groups.forEach(g => {
    const feats = MODEL.features.filter(f => f.group === g);
    const gh = document.createElement("div");
    gh.className = "group"; gh.textContent = g; gh.dataset.group = g;
    list.appendChild(gh);
    feats.forEach(f => {
      const on = state.on[f.suffix];
      const el = document.createElement("div");
      el.className = "feat" + (on ? " on" : "");
      el.dataset.suffix = f.suffix;
      el.dataset.hay = (f.name + " " + f.desc).toLowerCase();
      const req = (f.requires || []).length ? ` <span class="badge">needs ${f.requires.join(", ")}</span>` : "";
      let knobsHtml = "";
      if (f.knobs.length) {
        knobsHtml = '<div class="knobs">' + f.knobs.map(k => knobRow(k)).join("") + "</div>";
      }
      el.innerHTML =
        '<div class="frow"><input type="checkbox"' + (on ? " checked" : "") +
        ' data-suf="' + f.suffix + '"><div><div class="fname">' + esc(f.name) +
        (f.default ? ' <span class="badge dflt">default on</span>' : "") + req +
        '</div><div class="fdesc">' + esc(f.desc || "") + "</div>" + knobsHtml + "</div></div>";
      list.appendChild(el);
      shown++;
    });
  });
  // core knobs group
  if (MODEL.core.length) {
    const gh = document.createElement("div");
    gh.className = "group"; gh.textContent = "Core / always-on knobs"; gh.dataset.group = "__core__";
    list.appendChild(gh);
    const el = document.createElement("div");
    el.className = "feat on"; el.dataset.suffix = "__core__";
    el.dataset.hay = "core " + MODEL.core.map(k => k.name).join(" ").toLowerCase();
    el.innerHTML = '<div class="knobs" style="margin-left:0">' + MODEL.core.map(k => knobRow(k)).join("") + "</div>";
    list.appendChild(el);
  }
  wire();
  filter(document.getElementById("q").value);
  update();
}

function knobRow(k) {
  const val = (state.knob[k.name] !== undefined) ? state.knob[k.name] : k.default;
  const changed = String(val) !== String(k.default) ? " changed" : "";
  return '<div class="knob"><label title="default ' + esc(k.default) + '">' + esc(k.name) + '</label>' +
    '<input class="kin' + changed + '" data-knob="' + k.name + '" value="' + esc(String(val)) + '">' +
    (k.desc ? '<span class="kd">' + esc(k.desc) + " &middot; default <code>" + esc(k.default) + "</code></span>" : "") +
    "</div>";
}

function wire() {
  document.querySelectorAll('input[type=checkbox][data-suf]').forEach(cb => {
    cb.onchange = () => {
      const suf = cb.dataset.suf;
      if (cb.checked) enableWithParents(suf); else disableWithDependents(suf);
      render();
    };
  });
  document.querySelectorAll("input[data-knob]").forEach(inp => {
    inp.oninput = () => {
      const name = inp.dataset.knob;
      state.knob[name] = inp.value;
      inp.classList.toggle("changed", String(inp.value) !== String(knobDefault(name)));
      update();
    };
  });
}

const KNOB_DEF = {};
MODEL.features.forEach(f => f.knobs.forEach(k => (KNOB_DEF[k.name] = k.default)));
MODEL.core.forEach(k => (KNOB_DEF[k.name] = k.default));
function knobDefault(n) { return KNOB_DEF[n]; }

function knobVisible(name) {
  // A knob counts only if its owning feature is enabled (core knobs always count).
  const owner = KNOB_OWNER[name];
  return owner === null || state.on[owner];
}
const KNOB_OWNER = {};
MODEL.features.forEach(f => f.knobs.forEach(k => (KNOB_OWNER[k.name] = f.suffix)));
MODEL.core.forEach(k => (KNOB_OWNER[k.name] = null));

function computeLines() {
  const feats = [];
  MODEL.features.forEach(f => {
    if (state.on[f.suffix] !== !!f.default) feats.push([f.name, state.on[f.suffix] ? 1 : 0]);
  });
  const kn = [];
  Object.keys(state.knob).forEach(name => {
    if (!knobVisible(name)) return;
    const v = state.knob[name];
    if (String(v) !== String(knobDefault(name)) && v !== "") kn.push([name, v]);
  });
  return { feats, kn };
}

function update() {
  const { feats, kn } = computeLines();
  let text;
  if (state.fmt === "pio") {
    const rows = feats.map(([n, v]) => "    -D" + n + "=" + v)
      .concat(kn.map(([n, v]) => "    -D" + n + "=" + v));
    text = rows.length ? "build_flags =\n" + rows.join("\n") : "; all defaults - no build_flags needed";
  } else if (state.fmt === "buildopt") {
    // A build_opt.h next to your .ino: Arduino IDE feeds it to every compilation unit,
    // library sources included (a plain -D in the sketch does not reach them). One flag/line.
    const rows = feats.map(([n, v]) => "-D" + n + "=" + v)
      .concat(kn.map(([n, v]) => "-D" + n + "=" + v));
    text = rows.length ? rows.join("\n") : "-DDETWS_ENABLE_WEBSOCKET=1  // (example) all defaults - build_opt.h optional";
  } else {
    const rows = feats.map(([n, v]) => "#define " + n + " " + v)
      .concat(kn.map(([n, v]) => "#define " + n + " " + v));
    text = rows.length ? rows.join("\n") : "// all defaults - nothing to override";
  }
  document.getElementById("output").value = text;
  const nOn = MODEL.features.filter(f => state.on[f.suffix]).length;
  document.getElementById("count").textContent = nOn + " / " + MODEL.features.length + " features on";
  document.getElementById("ocount").textContent = (feats.length + kn.length) + " overrides";
  const missing = [];
  MODEL.features.forEach(f => {
    if (state.on[f.suffix]) (f.requires || []).forEach(p => { if (bySuffix[p] && !state.on[p]) missing.push(f.suffix + " needs " + p); });
  });
  document.getElementById("msg").textContent = missing.length ? ("Unmet: " + missing.join("; ")) : "";
}

function filter(qraw) {
  const q = (qraw || "").trim().toLowerCase();
  document.querySelectorAll(".feat").forEach(el => {
    if (el.dataset.suffix === "__core__") return;
    const hit = !q || el.dataset.hay.indexOf(q) >= 0;
    el.classList.toggle("hidden", !hit);
  });
  // Show a group heading only if a feature under it (up to the next heading) is visible.
  document.querySelectorAll(".group").forEach(gh => {
    if (gh.dataset.group === "__core__") return;
    let sib = gh.nextElementSibling, vis = false;
    while (sib && !sib.classList.contains("group")) {
      if (sib.classList.contains("feat") && !sib.classList.contains("hidden")) vis = true;
      sib = sib.nextElementSibling;
    }
    gh.style.display = (q && !vis) ? "none" : "";
  });
}

document.getElementById("q").oninput = e => filter(e.target.value);
document.getElementById("reset").onclick = () => {
  state.knob = {};
  MODEL.features.forEach(f => (state.on[f.suffix] = !!f.default));
  render();
};
document.querySelectorAll(".seg button").forEach(b => {
  b.onclick = () => {
    state.fmt = b.dataset.fmt;
    document.querySelectorAll(".seg button").forEach(x => x.classList.toggle("sel", x === b));
    update();
  };
});
document.getElementById("copy").onclick = async () => {
  const ta = document.getElementById("output");
  try { await navigator.clipboard.writeText(ta.value); }
  catch (e) { ta.select(); document.execCommand("copy"); }
  const btn = document.getElementById("copy"); const t = btn.textContent;
  btn.textContent = "Copied"; setTimeout(() => (btn.textContent = t), 1200);
};
document.getElementById("download").onclick = () => {
  // Filename by mode: build_opt.h drops straight next to an Arduino sketch.
  const names = { buildopt: "build_opt.h", pio: "platformio-build_flags.ini", defines: "DetWebServerOverrides.h" };
  const blob = new Blob([document.getElementById("output").value + "\n"], { type: "text/plain" });
  const a = document.createElement("a");
  a.href = URL.createObjectURL(blob);
  a.download = names[state.fmt] || "config.txt";
  document.body.appendChild(a); a.click(); a.remove();
  setTimeout(() => URL.revokeObjectURL(a.href), 1000);
};
render();
</script>
</body>
</html>
"""


def generate():
    parsed = parse(CONFIG)
    model = build_model(parsed)
    data = json.dumps(model, separators=(",", ":"))
    return PAGE.replace("__DATA__", data), model


def main():
    check = len(sys.argv) > 1 and sys.argv[1] == "check"
    html, model = generate()
    if check:
        cur = open(OUT, "r", encoding="utf-8").read() if os.path.exists(OUT) else ""
        if cur.replace("\r\n", "\n") != html.replace("\r\n", "\n"):
            print("configurator.html is stale; run: python docs/utilities/gen_configurator.py")
            sys.exit(1)
        print("configurator.html up to date")
        return
    with open(OUT, "w", encoding="utf-8", newline="\n") as f:
        f.write(html)
    nk = sum(len(f["knobs"]) for f in model["features"]) + len(model["core"])
    print(
        "wrote %s: %d features, %d knobs (%d core), %d groups"
        % (os.path.relpath(OUT, ROOT), len(model["features"]), nk, len(model["core"]), len(model["groups"]))
    )


if __name__ == "__main__":
    main()
