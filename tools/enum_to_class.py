#!/usr/bin/env python3
"""Migrate a plain C enum to a scoped `enum class` (library-wide uniform conversion).

Usage:
    tools/enum_to_class.py <EnumName> <def_file> [--width uint8_t|uint16_t|uint32_t|int32_t]

Run from the repo root. The tool:
  1. Extracts the enum's members + the minimal safe underlying width (max defined value; OR-combinations
     of values <= that max still fit, so it is safe even for flag enums). Comment-stripped parse.
  2. Rewrites the definition to `enum class <Name> : <width>` (members stay prefixed).
  3. Scopes every member USE across src/ + test/ + examples/ (bare MEMBER -> Name::MEMBER), then
     un-scopes the declarations inside the enum body. Comments and string/char literals are stepped
     over: specs name their constants in prose ("the IKE_SA_INIT -> IKE_AUTH state machine"), and
     rewriting those into C++ would be both wrong and unreadable.
  4. Upgrades C-style `(Name)(expr)` casts to `static_cast<Name>(expr)`.
  5. Reports byte-OR / arithmetic sites (`| Name::` etc.) that need a manual `(uint8_t)` cast - `enum
     class` blocks the implicit int conversion, so the compiler will flag these; fix + recompile the env.

If any member value is a non-literal expression (bit-shift mask, `A|B`), the width may be underestimated;
pass --width explicitly for those (verify against the wire field). Anonymous enums must be named first.
"""

import sys, re, io, glob

# Comments and string/char literals, matched so a rewrite can step over them. Protocol specs name
# their constants in prose ("the IKE_SA_INIT -> IKE_AUTH state machine"), and a blind identifier
# substitution turns that documentation into C++ (`IkeExchange::IKE_SA_INIT`), which is wrong and
# unreadable. Leftmost-match ordering also makes a `//` inside a string literal stay part of the
# string rather than starting a comment.
SKIP_RE = re.compile(
    r"""
      //[^\n]*             # line comment
    | /\*.*?\*/            # block comment
    | "(?:\\.|[^"\\])*"    # string literal
    | '(?:\\.|[^'\\])*'    # char literal
    """,
    re.S | re.X,
)


def sub_code_only(subs, text):
    """Apply (compiled_pattern, replacement) pairs only outside comments and literals."""

    def apply(chunk):
        for pat, repl in subs:
            chunk = pat.sub(repl, chunk)
        return chunk

    out, pos = [], 0
    for m in SKIP_RE.finditer(text):
        out.append(apply(text[pos : m.start()]))
        out.append(m.group(0))  # preserved verbatim
        pos = m.end()
    out.append(apply(text[pos:]))
    return "".join(out)


def enum_body(text, name):
    m = re.search(r"\benum\s+(?:class\s+)?" + re.escape(name) + r"\b[^{]*\{", text)
    if not m:
        return None, None, None
    i = m.end()
    depth = 1
    start = i
    while i < len(text) and depth:
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
        i += 1
    return start, i - 1, text[start : i - 1]


def split_top_commas(s):
    """Split on commas at bracket-depth 0 only, so a macro value like COAP_CODE(2, 1) stays intact."""
    out, cur, depth = [], "", 0
    for ch in s:
        if ch in "([{":
            depth += 1
        elif ch in ")]}":
            depth -= 1
        if ch == "," and depth == 0:
            out.append(cur)
            cur = ""
        else:
            cur += ch
    out.append(cur)
    return out


def parse_members(body):
    body = re.sub(r"//[^\n]*", "", body)
    body = re.sub(r"/\*.*?\*/", "", body, flags=re.S)
    body = re.sub(r"(?m)^[ \t]*#.*$", "", body)  # drop preprocessor lines (feature-gated members)
    members = []
    cur = -1
    mn = 0
    mx = 0
    had_expr = False
    for chunk in split_top_commas(body):
        c = chunk.strip()
        # Grab the final identifier, skipping any leading scope chain: re-running after a partial write
        # (step 3 failed) leaves the body as `Name::MEMBER`, and a bare `\w+` match would parse `Name`.
        m = re.match(r"(?:[A-Za-z_]\w*::)*([A-Za-z_]\w*)", c)
        if not m:  # blank chunk or a stray non-identifier fragment
            continue
        members.append(m.group(1))
        if "=" in c:
            v = c.split("=", 1)[1].strip()
            if re.fullmatch(r"0[xX][0-9a-fA-F]+", v):
                cur = int(v, 16)
            elif re.fullmatch(r"-?\d+", v):
                cur = int(v)
            elif re.fullmatch(r"'.'", v):
                cur = ord(v[1])
            elif re.fullmatch(r"1\s*<<\s*\d+", v):
                cur = 1 << int(v.split("<<")[1])
            else:
                cur += 1
                had_expr = True
        else:
            cur += 1
        mn = min(mn, cur)
        mx = max(mx, cur)
    return members, mn, mx, had_expr


def files():
    out = []
    for p in (
        "src/**/*.h",
        "src/**/*.cpp",
        "test/**/*.h",
        "test/**/*.cpp",
        "examples/**/*.ino",
        "examples/**/*.h",
        "examples/**/*.cpp",
    ):
        out += glob.glob(p, recursive=True)
    return out


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    name, deffile = args[0], args[1]
    width = None
    for a in sys.argv[1:]:
        if a.startswith("--width"):
            width = a.split("=", 1)[1] if "=" in a else sys.argv[sys.argv.index(a) + 1]

    d = io.open(deffile, encoding="utf-8").read()
    _, _, body = enum_body(d, name)
    assert body is not None, f"enum {name} not found in {deffile}"
    members, mn, mx, had_expr = parse_members(body)

    # An existing explicit underlying type (`enum X : uint16_t`) is authoritative: keep it so we never
    # silently shrink a wire enum's width. Only derive from the member range when neither --width nor an
    # existing type is present.
    m_ut = re.search(r"\benum\s+" + re.escape(name) + r"\b\s*:\s*([A-Za-z_]\w*(?:\s+\w+)*)", d)
    existing_ut = m_ut.group(1).strip() if m_ut else None
    if width is None:
        width = existing_ut
    if width is None:
        if had_expr:
            sys.exit(
                f"{name}: member value is an expression (mask/shift); pass --width explicitly "
                f"(parsed range {mn}..{mx})."
            )
        width = (
            "int32_t"
            if mn < 0
            else (
                "uint8_t"
                if mx <= 0xFF
                else "uint16_t" if mx <= 0xFFFF else "uint32_t" if mx <= 0xFFFFFFFF else "uint64_t"
            )
        )

    # 1) definition -> enum class (idempotent). The optional group absorbs any existing `: <type>` so a
    # pre-typed enum does not end up with a doubled `: uint8_t : uint8_t` specifier.
    if not re.search(r"\benum\s+class\s+" + re.escape(name) + r"\b", d):
        d2 = re.sub(
            r"\benum\s+" + re.escape(name) + r"\b(?!\s*::)(\s*:\s*[A-Za-z_]\w*(?:\s+\w+)*)?",
            f"enum class {name} : {width}",
            d,
            count=1,
        )
        assert d2 != d, f"def line not found for {name}"
        io.open(deffile, "w", encoding="utf-8", newline="\n").write(d2)

    # 2) scope member USES (idempotent via the : lookbehind), in code only - never in the comments
    # and literals that legitimately spell the bare protocol constant.
    subs = [
        (re.compile(r"(?<![\w:])" + re.escape(mbr) + r"\b"), f"{name}::{mbr}") for mbr in members
    ]
    subs.append((re.compile(r"\(\s*" + re.escape(name) + r"\s*\)\s*\("), f"static_cast<{name}>("))
    for f in files():
        s = io.open(f, encoding="utf-8", errors="replace").read()
        o = sub_code_only(subs, s)
        if o != s:
            io.open(f, "w", encoding="utf-8", newline="\n").write(o)

    # 3) un-scope the declarations inside the enum body
    d = io.open(deffile, encoding="utf-8").read()
    start, end, body = enum_body(d, name)
    b2 = body
    for mbr in members:
        b2 = b2.replace(f"{name}::{mbr}", mbr)
    if b2 != body:
        io.open(deffile, "w", encoding="utf-8", newline="\n").write(d[:start] + b2 + d[end:])
    print(f"{name} -> enum class {name} : {width}  ({len(members)} members)")


if __name__ == "__main__":
    main()
