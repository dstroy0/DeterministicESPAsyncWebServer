#!/usr/bin/env bash
# Render each docs/diagrams/*.mmd to a light + dark PNG. The READMEs / docs embed these PNGs through a
# <picture> element (instead of a live ```mermaid``` fence) so the diagrams show in the GitHub web UI,
# the GitHub mobile app, AND Doxygen - the app and Doxygen do not render mermaid, and GitHub strips the
# foreignObject an inline SVG would need for the multi-line labels. A PNG bakes the labels into pixels.
#
# The .mmd sources are written by docs/utilities/gen_api_flow.py and gen_flag_deps.py; run those first.
# Needs mermaid-cli (mmdc) + a Chromium. In CI, install both; locally, run on a host that has them.
#
#   tools/render_diagrams.sh
set -euo pipefail
DIR="$(cd "$(dirname "$0")/../docs/diagrams" && pwd)"
PPTR="$(mktemp)"
printf '{"args":["--no-sandbox","--disable-gpu"]}' >"$PPTR"
trap 'rm -f "$PPTR"' EXIT
shopt -s nullglob
for mmd in "$DIR"/*.mmd; do
    name="$(basename "$mmd" .mmd)"
    width=1100
    [ "$name" = "flag_deps" ] && width=1600 # the dependency graph is wide and shallow
    mmdc -p "$PPTR" -i "$mmd" -o "$DIR/$name.light.png" -t default -b white -w "$width" --scale 2
    mmdc -p "$PPTR" -i "$mmd" -o "$DIR/$name.dark.png" -t dark -b "#0d1117" -w "$width" --scale 2
    echo "rendered $name (light + dark)"
done
