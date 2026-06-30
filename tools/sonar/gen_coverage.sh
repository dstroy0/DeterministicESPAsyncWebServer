#!/usr/bin/env bash
# Run the native Unity test envs with gcov instrumentation and emit a merged
# SonarQube coverage report (src/ only) for the SonarCloud scan. Coverage needs
# the tests to actually RUN (not just compiledb), so this builds + runs them.
#
# A dedicated build dir (.pio_cov) and the coverage build flags keep this separate
# from the no-coverage compile-DB build (gen_compiledb.sh). Pass an explicit env
# list as args to cover a subset (handy locally); default is every native env
# except codeql and the ThreadSanitizer env (tsan + gcov do not mix).
#
# Requires: pio + gcovr on PATH.
set -uo pipefail
cd "$(dirname "$0")/../.."

export PLATFORMIO_BUILD_DIR="${PLATFORMIO_BUILD_DIR:-.pio_cov}"
export PLATFORMIO_BUILD_FLAGS="-fprofile-arcs -ftest-coverage -lgcov"

if [ "$#" -gt 0 ]; then
    envs="$*"
else
    envs=$(grep -oE '^\[env:native[A-Za-z0-9_]*\]' platformio.ini | sed -E 's/\[env:(.*)\]/\1/' | grep -vE 'codeql|native_tsan')
fi

for e in $envs; do
    echo "::group::coverage $e"
    pio test -e "$e" || echo "WARN: test env failed (coverage still collected from what ran): $e"
    echo "::endgroup::"
done

# Merge every env's .gcda into one SonarQube generic-coverage report, src/ only.
gcovr --root . --filter 'src/' --gcov-ignore-parse-errors --sonarqube coverage.xml --print-summary "$PLATFORMIO_BUILD_DIR"
echo "wrote coverage.xml"
