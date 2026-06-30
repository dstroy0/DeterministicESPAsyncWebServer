#!/usr/bin/env bash
# Generate a merged compile_commands.json covering every native env, for the
# SonarQube C/C++ analyzer. Run from anywhere with PlatformIO (`pio`) on PATH.
#
# No single env enables all DETWS_ENABLE_* features, so we run compiledb per env
# and merge (see merge_compiledb.py) to cover every feature-gated source file.
# Generate this in the same environment that runs the scan so the database's
# absolute `directory` matches the analyzer's checkout (the CI workflow does this).
set -euo pipefail
cd "$(dirname "$0")/../.."

FRAGS=compiledb_frags
rm -rf "$FRAGS" compile_commands.json
mkdir -p "$FRAGS"

envs=$(grep -oE '^\[env:native[A-Za-z0-9_]*\]' platformio.ini | sed -E 's/\[env:(.*)\]/\1/' | grep -vE 'codeql')
count=0
for e in $envs; do
    echo "::group::compiledb $e"
    pio run -t compiledb -e "$e"
    mv compile_commands.json "$FRAGS/$e.json"
    count=$((count + 1))
    echo "::endgroup::"
done
echo "ran compiledb for $count envs"

python3 tools/sonar/merge_compiledb.py compile_commands.json "$FRAGS/*.json"
rm -rf "$FRAGS"
