#!/usr/bin/env python3
# Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Generate the theme library: one CSS per palette in src/web/themes/generated/.

Hand-writing a couple hundred theme stylesheets is not maintainable, so a theme is a **palette**
(a handful of colors + a light/dark mode) rendered through one shared **template** of CSS rules that
style the common page elements (body, headings, links, cards, buttons, inputs, tables, code). Every
theme therefore has the same structure and only its colors differ, which keeps them consistent and
lets the set grow to hundreds. The seven original hand-authored themes in src/web/themes/*.css are
left untouched (they carry bespoke personality - CRT scanlines, nyan trails); this only fills
src/web/themes/generated/.

Categories: `enterprise` (calm, professional), `editor` (recognizable code-editor schemes),
`retro` (terminals / 8-bit), `meme` (loud + fun), `pastel`, `nature`. Give the kids options and the
professionals a boardroom-safe default.

    python src/web/wizard/gen_themes.py           # (re)generate src/web/themes/generated/*.css
    python src/web/wizard/gen_themes.py --check    # CI: fail if any generated theme is stale
    python src/web/wizard/gen_themes.py --list      # print "name category mode" for every theme
"""

import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
OUT_DIR = os.path.normpath(os.path.join(SCRIPT_DIR, "..", "themes", "generated"))

# A palette: name -> (category, mode, colors). colors = (bg, bg2, card, ink, muted, accent, accent2).
#   bg/bg2 = page background gradient stops   card = panel surface   ink = body text
#   muted  = secondary text / borders          accent = primary      accent2 = secondary / links-hover
# mode "dark" flips a couple of derived tones (input surface, code block). Hex only, 6-digit.
P = {}


def add(name, category, mode, bg, bg2, card, ink, muted, accent, accent2):
    P[name] = (category, mode, bg, bg2, card, ink, muted, accent, accent2)


# --- enterprise: calm, professional, boardroom-safe -------------------------------------------------
add(
    "corporate-slate",
    "enterprise",
    "light",
    "#f8fafc",
    "#eef2f7",
    "#ffffff",
    "#1e293b",
    "#64748b",
    "#2563eb",
    "#0ea5e9",
)
add("graphite", "enterprise", "light", "#f5f5f4", "#e7e5e4", "#ffffff", "#292524", "#78716c", "#334155", "#0891b2")
add("azure-pro", "enterprise", "light", "#f0f6ff", "#e2edfb", "#ffffff", "#0f2540", "#5b7699", "#0067c5", "#00a3e0")
add("ibm-carbon", "enterprise", "dark", "#161616", "#262626", "#262626", "#f4f4f4", "#8d8d8d", "#0f62fe", "#33b1ff")
add("mono-minimal", "enterprise", "light", "#ffffff", "#fafafa", "#ffffff", "#111111", "#8a8a8a", "#111111", "#555555")
add("forest-exec", "enterprise", "light", "#f4f7f4", "#e6efe6", "#ffffff", "#1b3a2b", "#5f7d6c", "#1a7a4c", "#0f766e")
add("navy-oxford", "enterprise", "dark", "#0b1f3a", "#0f2a4d", "#132f56", "#e8eef7", "#8ba3c1", "#3b82f6", "#22d3ee")
add("bordeaux", "enterprise", "light", "#faf6f6", "#f0e6e6", "#ffffff", "#3a1420", "#8a6a72", "#7a1f3d", "#a8324f")
add("sandstone", "enterprise", "light", "#faf7f0", "#f0e9da", "#ffffff", "#3a3226", "#8a7a5f", "#b45309", "#0e7490")
add("steel", "enterprise", "dark", "#1c2530", "#232f3c", "#2a3745", "#e6edf3", "#93a4b5", "#4f8cc9", "#5eb0d8")

# --- editor: recognizable code-editor color schemes -------------------------------------------------
add("dracula", "editor", "dark", "#282a36", "#21222c", "#343746", "#f8f8f2", "#6272a4", "#bd93f9", "#ff79c6")
add("nord", "editor", "dark", "#2e3440", "#3b4252", "#3b4252", "#eceff4", "#81a1c1", "#88c0d0", "#a3be8c")
add("solarized-dark", "editor", "dark", "#002b36", "#073642", "#073642", "#eee8d5", "#93a1a1", "#268bd2", "#2aa198")
add("solarized-light", "editor", "light", "#fdf6e3", "#eee8d5", "#ffffff", "#586e75", "#93a1a1", "#268bd2", "#cb4b16")
add("gruvbox-dark", "editor", "dark", "#282828", "#32302f", "#3c3836", "#ebdbb2", "#a89984", "#fabd2f", "#fe8019")
add("gruvbox-light", "editor", "light", "#fbf1c7", "#f2e5bc", "#ffffff", "#3c3836", "#7c6f64", "#b57614", "#af3a03")
add("monokai", "editor", "dark", "#272822", "#2d2e27", "#3e3d32", "#f8f8f2", "#75715e", "#a6e22e", "#f92672")
add("one-dark", "editor", "dark", "#282c34", "#21252b", "#2c313a", "#abb2bf", "#5c6370", "#61afef", "#c678dd")
add("one-light", "editor", "light", "#fafafa", "#f0f0f0", "#ffffff", "#383a42", "#a0a1a7", "#4078f2", "#a626a4")
add("tokyo-night", "editor", "dark", "#1a1b26", "#16161e", "#24283b", "#c0caf5", "#565f89", "#7aa2f7", "#bb9af7")
add("catppuccin-mocha", "editor", "dark", "#1e1e2e", "#181825", "#313244", "#cdd6f4", "#6c7086", "#89b4fa", "#f5c2e7")
add("catppuccin-latte", "editor", "light", "#eff1f5", "#e6e9ef", "#ffffff", "#4c4f69", "#8c8fa1", "#1e66f5", "#ea76cb")
add("catppuccin-frappe", "editor", "dark", "#303446", "#292c3c", "#414559", "#c6d0f5", "#737994", "#8caaee", "#f4b8e4")
add(
    "catppuccin-macchiato",
    "editor",
    "dark",
    "#24273a",
    "#1e2030",
    "#363a4f",
    "#cad3f5",
    "#6e738d",
    "#8aadf4",
    "#f5bde6",
)
add("github-dark", "editor", "dark", "#0d1117", "#161b22", "#161b22", "#c9d1d9", "#8b949e", "#58a6ff", "#a371f7")
add("github-light", "editor", "light", "#ffffff", "#f6f8fa", "#ffffff", "#24292f", "#6e7781", "#0969da", "#8250df")
add("ayu-dark", "editor", "dark", "#0a0e14", "#0d1017", "#141a22", "#b3b1ad", "#565b66", "#ffb454", "#59c2ff")
add("ayu-mirage", "editor", "dark", "#1f2430", "#232834", "#2a303c", "#cbccc6", "#707a8c", "#ffcc66", "#5ccfe6")
add("ayu-light", "editor", "light", "#fafafa", "#f3f4f5", "#ffffff", "#5c6773", "#abb0b6", "#ff9940", "#399ee6")
add("rose-pine", "editor", "dark", "#191724", "#1f1d2e", "#26233a", "#e0def4", "#6e6a86", "#c4a7e7", "#ebbcba")
add("rose-pine-dawn", "editor", "light", "#faf4ed", "#fffaf3", "#ffffff", "#575279", "#9893a5", "#907aa9", "#d7827e")
add("everforest", "editor", "dark", "#2b3339", "#323c41", "#3a464c", "#d3c6aa", "#859289", "#a7c080", "#e69875")
add("kanagawa", "editor", "dark", "#1f1f28", "#16161d", "#2a2a37", "#dcd7ba", "#727169", "#7e9cd8", "#e6c384")
add("night-owl", "editor", "dark", "#011627", "#0b2942", "#0e293f", "#d6deeb", "#637777", "#82aaff", "#c792ea")
add("palenight", "editor", "dark", "#292d3e", "#242837", "#323649", "#a6accd", "#676e95", "#c792ea", "#89ddff")
add("cobalt2", "editor", "dark", "#193549", "#1f4662", "#15232d", "#ffffff", "#8aa6c1", "#ffc600", "#ff9d00")
add("synthwave", "editor", "dark", "#262335", "#241b2f", "#34294f", "#ffffff", "#848bbd", "#ff7edb", "#36f9f6")
add("oceanic-next", "editor", "dark", "#1b2b34", "#22333b", "#2b3a42", "#d8dee9", "#65737e", "#6699cc", "#5fb3b3")
add("shades-of-purple", "editor", "dark", "#2d2b55", "#1e1e3f", "#38356e", "#ffffff", "#a599e9", "#fad000", "#ff628c")

# --- retro / terminal -------------------------------------------------------------------------------
add("amber-crt", "retro", "dark", "#0a0600", "#140d00", "#140d00", "#ffb000", "#8a6000", "#ffcc33", "#ff8800")
add("green-phosphor", "retro", "dark", "#001100", "#002200", "#002200", "#33ff66", "#1a8a33", "#66ff99", "#00cc44")
add("ibm-3270", "retro", "dark", "#0d0208", "#1a0410", "#1a0410", "#33ff33", "#1a8a1a", "#33ff33", "#00cc00")
add("c64", "retro", "dark", "#40318d", "#4a3a9e", "#4a3a9e", "#a5a0e0", "#7a72c0", "#a5a0e0", "#ffffff")
add("gameboy", "retro", "light", "#9bbc0f", "#8bac0f", "#9bbc0f", "#0f380f", "#306230", "#0f380f", "#306230")
add("hotdog-stand", "meme", "light", "#ff0000", "#ffff00", "#ffff00", "#000000", "#aa0000", "#000000", "#ff0000")

# --- meme / fun -------------------------------------------------------------------------------------
add("vaporwave", "meme", "dark", "#2d1b4e", "#3d2063", "#ff71ce", "#ffffff", "#b967ff", "#01cdfe", "#05ffa1")
add("matrix", "meme", "dark", "#000000", "#001100", "#001a00", "#00ff41", "#008f11", "#00ff41", "#22ff66")
add("barbie", "meme", "light", "#ffe0f0", "#ffd0ec", "#ffffff", "#a10c58", "#d16ba5", "#e91e8c", "#ff4fa3")
add("minecraft", "meme", "light", "#7fb238", "#8fc44a", "#8b6f47", "#3a2a16", "#6b5333", "#5a9e2f", "#8b6f47")
add("among-us", "meme", "dark", "#132a3e", "#1a3a52", "#c51111", "#ffffff", "#7a8b99", "#c51111", "#ef7d0d")
add("doge", "meme", "light", "#f5e6c8", "#f0dcae", "#ffffff", "#5a4a2a", "#a08a5a", "#f2a900", "#3fa7d6")
add("cyberpunk", "meme", "dark", "#0d0221", "#190b33", "#241540", "#f0f0ff", "#7b6ba8", "#fcee0a", "#ff003c")
add("bubblegum", "meme", "light", "#fff0f6", "#ffe0ee", "#ffffff", "#7a2a55", "#c86a95", "#ff5fa2", "#7ac7ff")

# --- pastel / nature --------------------------------------------------------------------------------
add("mint", "pastel", "light", "#eafaf1", "#d6f5e3", "#ffffff", "#1e4635", "#5f8a74", "#2ecc71", "#16a085")
add("lavender", "pastel", "light", "#f3f0fb", "#e8e2f7", "#ffffff", "#3a2f5a", "#8579a5", "#8b5cf6", "#ec4899")
add("peach", "pastel", "light", "#fff3ec", "#ffe6d6", "#ffffff", "#5a3a2a", "#a5836a", "#ff8a5c", "#ffb703")
add("sky", "pastel", "light", "#eef7ff", "#d9edff", "#ffffff", "#123a5a", "#5f88a5", "#3b9dff", "#00b4d8")
add("sakura", "nature", "light", "#fff5f7", "#ffe4ea", "#ffffff", "#6a2a3a", "#b57a88", "#ff6f91", "#845ec2")
add("autumn", "nature", "light", "#fbf3e8", "#f3e2cc", "#ffffff", "#4a2f1a", "#8a6a4a", "#c05621", "#8a6d3b")
add("deep-sea", "nature", "dark", "#031b2e", "#052a45", "#073a5c", "#cfe8f7", "#5a8aa8", "#00b4d8", "#48cae4")
add("volcano", "nature", "dark", "#1a0d0a", "#2a1410", "#3a1c16", "#ffddcc", "#a56a5a", "#ff5722", "#ffab00")


# One template of CSS rules; every palette flows through it. {v} placeholders are the palette vars.
TEMPLATE = """/* {name} ({category}, {mode}) - generated by src/web/wizard/gen_themes.py from a palette.
   Opt a page in with <!--#theme generated/{name}-->. Hand-edit the palette in gen_themes.py, not here. */
:root {{
  --bg: {bg};
  --bg2: {bg2};
  --card: {card};
  --ink: {ink};
  --muted: {muted};
  --accent: {accent};
  --accent2: {accent2};
  --radius: {radius};
}}
* {{ box-sizing: border-box; }}
html, body {{
  margin: 0;
  min-height: 100%;
  color: var(--ink);
  font: 16px/1.6 {font};
  background: linear-gradient(135deg, var(--bg), var(--bg2)) fixed;
}}
main, .wrap {{ max-width: 880px; margin: 0 auto; padding: 24px; }}
h1, h2, h3 {{ color: var(--accent); font-weight: 700; line-height: 1.25; }}
a {{ color: var(--accent2); text-decoration: none; }}
a:hover {{ text-decoration: underline; }}
.muted, small {{ color: var(--muted); }}
.card, .panel {{
  background: var(--card);
  border: 1px solid color-mix(in srgb, var(--muted) 35%, transparent);
  border-radius: var(--radius);
  padding: 18px 20px;
  margin: 16px 0;
  box-shadow: {shadow};
}}
button, .btn {{
  background: var(--accent);
  color: {on_accent};
  border: 0;
  border-radius: var(--radius);
  padding: 10px 18px;
  font: inherit;
  font-weight: 600;
  cursor: pointer;
}}
button:hover, .btn:hover {{ filter: brightness(1.08); }}
button.secondary {{ background: var(--accent2); }}
input, select, textarea {{
  width: 100%;
  background: {input_bg};
  color: var(--ink);
  border: 1px solid color-mix(in srgb, var(--muted) 45%, transparent);
  border-radius: var(--radius);
  padding: 10px 12px;
  font: inherit;
}}
input:focus, select:focus, textarea:focus {{ outline: 2px solid var(--accent); border-color: var(--accent); }}
table {{ width: 100%; border-collapse: collapse; }}
th, td {{ text-align: left; padding: 8px 10px; border-bottom: 1px solid color-mix(in srgb, var(--muted) 30%, transparent); }}
th {{ color: var(--accent); }}
code, pre {{
  font-family: ui-monospace, "SF Mono", "Cascadia Code", Consolas, monospace;
  background: {code_bg};
  border-radius: calc(var(--radius) * 0.6);
}}
code {{ padding: 2px 6px; }}
pre {{ padding: 14px 16px; overflow-x: auto; }}
"""

# Per-category shaping (radius / shadow / font). Enterprise = restrained; meme = loud.
STYLE = {
    "enterprise": {
        "radius": "8px",
        "font": '15px/1.6 "Segoe UI", system-ui, sans-serif',
        "shadow": "0 1px 3px rgba(0,0,0,.08)",
    },
    "editor": {
        "radius": "6px",
        "font": "15px/1.6 ui-sans-serif, system-ui, sans-serif",
        "shadow": "0 2px 8px rgba(0,0,0,.18)",
    },
    "retro": {
        "radius": "0",
        "font": '15px/1.55 ui-monospace, "Cascadia Code", monospace',
        "shadow": "0 0 0 1px currentColor inset",
    },
    "meme": {
        "radius": "18px",
        "font": '16px/1.6 "Comic Sans MS", "Segoe UI", sans-serif',
        "shadow": "0 6px 20px rgba(0,0,0,.25)",
    },
    "pastel": {
        "radius": "16px",
        "font": '16px/1.7 "Nunito", system-ui, sans-serif',
        "shadow": "0 4px 14px rgba(0,0,0,.08)",
    },
    "nature": {
        "radius": "12px",
        "font": '16px/1.65 "Georgia", "Segoe UI", serif',
        "shadow": "0 3px 12px rgba(0,0,0,.12)",
    },
}


def luminance(hexc):
    r, g, b = (int(hexc[i : i + 2], 16) / 255 for i in (1, 3, 5))
    return 0.2126 * r + 0.7152 * g + 0.0722 * b


# --- palette derivation: the picky-user tool (give it your one exact color, get a whole theme) -------
import colorsys


def _rgb(h):
    h = h.lstrip("#")
    return tuple(int(h[i : i + 2], 16) / 255 for i in (0, 2, 4))


def _hex(r, g, b):
    return "#%02x%02x%02x" % tuple(max(0, min(255, round(c * 255))) for c in (r, g, b))


def _shift(hexc, dl=0.0, ds=0.0, dh=0.0):
    """Nudge a color in HLS space (lightness / saturation / hue), clamped."""
    r, g, b = _rgb(hexc)
    h, l, s = colorsys.rgb_to_hls(r, g, b)
    return _hex(*colorsys.hls_to_rgb((h + dh) % 1.0, max(0.0, min(1.0, l + dl)), max(0.0, min(1.0, s + ds))))


def derive_palette(base, mode, accent=None):
    """Build a full 7-color palette from one anchor color + a light/dark mode (and an optional accent).

    `base` is treated as the panel/surface color - the color the picky user actually cares about
    (their charcoal gray). Everything else is derived so contrast and the accent read well."""
    if accent:
        acc = accent
    else:
        h, l, s = colorsys.rgb_to_hls(*_rgb(base))
        # A near-gray base has no meaningful hue to complement, so give it a friendly blue; otherwise
        # take a vivid complementary at a fixed, readable lightness/saturation (not a muddy shift).
        acc = "#3b82f6" if s < 0.12 else _hex(*colorsys.hls_to_rgb((h + 0.5) % 1.0, 0.58, 0.72))
    acc2 = _shift(acc, dh=0.09, ds=0.05)
    if mode == "dark":
        return (
            _shift(base, dl=-0.06),
            base,
            _shift(base, dl=0.05),
            "#e8eaed",
            _shift(base, dl=0.32, ds=-0.15),
            acc,
            acc2,
        )
    return (
        _shift(base, dl=0.05),
        _shift(base, dl=-0.02),
        "#ffffff",
        "#171a1f",
        _shift(base, dl=-0.28, ds=-0.1),
        acc,
        acc2,
    )


def render_from(name, category, mode, bg, bg2, card, ink, muted, accent, accent2):
    st = STYLE[category]
    on_accent = "#0b0b0b" if luminance(accent) > 0.6 else "#ffffff"
    input_bg = "rgba(255,255,255,.06)" if mode == "dark" else "#ffffff"
    code_bg = "rgba(255,255,255,.08)" if mode == "dark" else "color-mix(in srgb, var(--muted) 14%, transparent)"
    return TEMPLATE.format(
        name=name,
        category=category,
        mode=mode,
        bg=bg,
        bg2=bg2,
        card=card,
        ink=ink,
        muted=muted,
        accent=accent,
        accent2=accent2,
        radius=st["radius"],
        font=st["font"],
        shadow=st["shadow"],
        on_accent=on_accent,
        input_bg=input_bg,
        code_bg=code_bg,
    )


def render(name):
    return render_from(name, *P[name])


# The seven original bespoke themes (hand-authored, not palette-generated) - shown in the gallery too.
HAND_THEMES = ["bubbly", "crt-green", "cute", "keroppi", "nyancat", "rainbow", "ssh-terminal"]
GALLERY = os.path.normpath(os.path.join(SCRIPT_DIR, "..", "..", "..", "docs", "THEMES.md"))
CAT_ORDER = ["enterprise", "editor", "retro", "pastel", "nature", "meme"]
CAT_BLURB = {
    "enterprise": "Calm, boardroom-safe. Reach for these on a product.",
    "editor": "Recognizable code-editor schemes, matched to their canonical palettes.",
    "retro": "Terminals and 8-bit machines.",
    "pastel": "Soft and friendly.",
    "nature": "Earth, sea, and seasons.",
    "meme": "Loud and fun - for the kids (and the young at heart).",
}


def _grid(names):
    """A 3-column HTML table of theme previews with their names (renders on GitHub + Doxygen)."""
    out = ["<table>"]
    for i in range(0, len(names), 3):
        out.append("<tr>")
        for n in names[i : i + 3]:
            out.append(
                f'<td align="center"><img src="theme_preview/{n}.png" width="240" alt="{n}"><br><code>{n}</code></td>'
            )
        out.append("</tr>")
    out.append("</table>")
    return "\n".join(out)


def cmd_gallery():
    by_cat = {}
    for n, v in P.items():
        by_cat.setdefault(v[0], []).append(n)
    lines = [
        "# Themes",
        "",
        "> Gallery generated by `src/web/wizard/gen_themes.py gallery`; previews by"
        " `tools/render_theme_previews.cjs`. Do not edit by hand.",
        "",
        f"**{len(P) + len(HAND_THEMES)} themes.** Opt a served page into one with a build directive -"
        " `<!--#theme dracula-->` or `<!--#theme generated/dracula-->` before `</head>` (see"
        " [src/web/README.md](../src/web/README.md)). Need an exact color that is not here? Generate your"
        " own from one anchor color:",
        "",
        "```sh",
        "python src/web/wizard/gen_themes.py custom --base '#36454f' --mode dark --name charcoal --out my.css",
        "```",
        "",
        "## Bespoke",
        "",
        "Hand-authored, with personality the palette engine cannot capture (CRT scanlines, nyan trails).",
        "",
        _grid(HAND_THEMES),
        "",
    ]
    for cat in CAT_ORDER:
        if by_cat.get(cat):
            lines += [f"## {cat.title()}", "", CAT_BLURB.get(cat, ""), "", _grid(sorted(by_cat[cat])), ""]
    with open(GALLERY, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines).rstrip("\n") + "\n")
    print(f"wrote {os.path.relpath(GALLERY)} ({len(P) + len(HAND_THEMES)} themes)")
    return 0


def _arg(flag, default=None):
    a = sys.argv[1:]
    return a[a.index(flag) + 1] if flag in a and a.index(flag) + 1 < len(a) else default


def main():
    argv = sys.argv[1:]
    # The picky-user tool: build a full theme around one exact color.
    #   gen_themes.py custom --base '#36454f' --mode dark --name charcoal [--accent '#00b4d8'] [--out file.css]
    if argv and argv[0] == "custom":
        base = _arg("--base")
        if not base:
            sys.exit("custom: --base '#rrggbb' is required (the one color you want the theme built around)")
        name = _arg("--name", "custom")
        mode = _arg("--mode", "dark")
        category = _arg("--category", "enterprise")
        css = render_from(name, category, mode, *derive_palette(base, mode, _arg("--accent")))
        out = _arg("--out")
        if out:
            with open(out, "w", encoding="utf-8", newline="\n") as f:
                f.write(css)
            print(f"wrote {out}")
        else:
            sys.stdout.write(css)
        return 0
    if argv and argv[0] == "gallery":
        return cmd_gallery()
    check = "--check" in argv
    if "--list" in argv:
        for n in sorted(P):
            print(f"{n} {P[n][0]} {P[n][1]}")
        return 0
    os.makedirs(OUT_DIR, exist_ok=True)
    stale = []
    for name in sorted(P):
        path = os.path.join(OUT_DIR, name + ".css")
        content = render(name)
        old = open(path, encoding="utf-8").read() if os.path.isfile(path) else None
        if old != content:
            stale.append(name)
            if not check:
                with open(path, "w", encoding="utf-8", newline="\n") as f:
                    f.write(content)
    if check:
        if stale:
            sys.stderr.write("STALE themes (run gen_themes.py): " + ", ".join(stale) + "\n")
            return 1
        print(f"{len(P)} themes in sync")
        return 0
    print(f"generated {len(P)} themes -> {os.path.relpath(OUT_DIR)}" + (f" ({len(stale)} changed)" if stale else ""))
    return 0


if __name__ == "__main__":
    sys.exit(main())
