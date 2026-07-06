#!/usr/bin/env bash
# Wrap a cross-toolchain's gcc/g++ with ccache so ABSOLUTE-path compiler invocations route through it.
#
# PlatformIO (pio ci) and arduino-cli invoke the ESP32 xtensa/riscv compiler by absolute path, so a
# PATH masquerade (as used for the native host gcc) is bypassed. The reliable fix is to replace the
# toolchain binary with a tiny shim that execs `ccache <real> "$@"`. Every example then reuses the
# cached Arduino-core object compiles (the bulk of each build) across examples and, with ~/.ccache
# restored, across CI runs. Idempotent (re-running skips already-wrapped binaries) and a no-op when
# ccache is absent or the toolchain is not installed yet (cold cache; the next run wraps it).
#
#   tools/ci/ccache_wrap.sh <search-root> <compiler-basename>...
#   tools/ci/ccache_wrap.sh ~/.platformio/packages xtensa-esp32-elf-gcc xtensa-esp32-elf-g++
set -uo pipefail

command -v ccache >/dev/null 2>&1 || {
    echo "ccache_wrap: ccache not installed - skipping"
    exit 0
}
ROOT="${1:?usage: ccache_wrap.sh <search-root> <compiler-basename>...}"
shift
[ -d "$ROOT" ] || {
    echo "ccache_wrap: $ROOT not present yet (cold cache) - skipping"
    exit 0
}

wrapped=0
for name in "$@"; do
    while IFS= read -r real; do
        [ -n "$real" ] || continue
        case "$real" in *.real) continue ;; esac # already the saved original
        [ -e "$real.real" ] && continue           # already wrapped
        mv "$real" "$real.real"
        printf '#!/bin/sh\nexec ccache "%s" "$@"\n' "$real.real" >"$real"
        chmod +x "$real"
        echo "ccache_wrap: wrapped $real"
        wrapped=$((wrapped + 1))
    done < <(find "$ROOT" -type f -name "$name" 2>/dev/null)
done
echo "ccache_wrap: $wrapped binaries newly wrapped"
