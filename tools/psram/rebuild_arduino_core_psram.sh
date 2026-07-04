#!/usr/bin/env bash
# Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# rebuild_arduino_core_psram.sh - rebuild the arduino-esp32 core for ESP32-S3 with
# CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY=y so a static EXT_RAM_BSS_ATTR array (the
# DeterministicESPAsyncWebServer TLS arena under DETWS_TLS_ARENA_IN_PSRAM=1) is placed in
# PSRAM with ZERO heap.
#
# WHY THIS EXISTS
#   The stock precompiled arduino-esp32 core (verified 2.0.x and 3.3.x) ships
#   CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY OFF. With it off, EXT_RAM_BSS_ATTR expands
#   to nothing and the "PSRAM" array silently stays in internal DRAM (empirically: the
#   symbol lands at 0x3fxxxxxx, and there is no .ext_ram.bss output section). The linker
#   script even routes .ext_ram.bss *into* .dram0.bss. Only a core rebuilt with the flag
#   regenerates sections.ld to place .ext_ram.bss in the external-RAM segment (0x3Cxxxxxx)
#   and compiles the startup code that maps/zeroes it. Just -D-defining the macro against
#   the stock core CRASHES (the app expects a mapping the bootloader/linker never set up).
#
#   This uses Espressif's official esp32-arduino-lib-builder to produce a self-consistent
#   core (proper octal-PSRAM config for N16R8 boards + the BSS-external flag) and installs
#   it over the arduino-cli / Arduino IDE core. Runs on Linux / WSL / macOS (needs the
#   ESP-IDF build prerequisites: git, python3, cmake, ninja, wget, jq). ~30-60 min the first
#   time; it clones ESP-IDF and builds the libraries from source.
#
# USAGE
#   ./rebuild_arduino_core_psram.sh [--branch idf-release_v5.5] [--core-dir <path>]
#     --branch     esp32-arduino-lib-builder branch (default idf-release_v5.5 = IDF 5.5,
#                  matching arduino-esp32 3.3.x). Use release/v5.3 for arduino 3.1.x.
#     --core-dir   arduino-cli/IDE core dir to install into (auto-detected if omitted),
#                  e.g. ~/.arduino15/packages/esp32/tools/esp32s3-libs/<ver>
#     --no-install build only; print where the libs are so you can install manually.
#     --jobs N     cap build parallelism to N cores (default: half of nproc, min 2). Keep
#                  this well under your core count - the IDF build is memory-hungry and
#                  saturating every core can wedge or crash a workstation.
#
# After running, rebuild your sketch (a FULL clean) for an S3 board with PSRAM=OPI, and
# EXT_RAM_BSS_ATTR arrays land in PSRAM. Verify with: nm firmware.elf | grep <symbol>
# (address 0x3Cxxxxxx = PSRAM) and check the ".ext_ram.bss" section exists.
set -euo pipefail

BRANCH="idf-release_v5.5" # tag for IDF 5.5 (arduino-esp32 3.3.x); no release/v5.5 branch exists
CORE_DIR=""
INSTALL=1
JOBS=""
WORK="${TMPDIR:-/tmp}/detws-arduino-psram"

while [ $# -gt 0 ]; do
  case "$1" in
    --branch) BRANCH="$2"; shift 2 ;;
    --core-dir) CORE_DIR="$2"; shift 2 ;;
    --no-install) INSTALL=0; shift ;;
    --jobs) JOBS="$2"; shift 2 ;;
    -h|--help) sed -n '2,48p' "$0"; exit 0 ;;
    *) echo "unknown arg: $1" >&2; exit 2 ;;
  esac
done

# Default to half the cores (min 2) so the build does not saturate the machine. Cap the
# parallelism for every downstream build tool (cmake/ninja via idf.py, and make).
if [ -z "$JOBS" ]; then
  NPROC="$(nproc 2>/dev/null || echo 4)"
  JOBS=$(( NPROC / 2 )); [ "$JOBS" -lt 2 ] && JOBS=2
fi
export CMAKE_BUILD_PARALLEL_LEVEL="$JOBS"
export MAKEFLAGS="-j${JOBS}"
export IDF_BUILD_JOBS="$JOBS"
echo "==> build parallelism capped at $JOBS core(s)"

echo "==> esp32-arduino-lib-builder ref: $BRANCH"
mkdir -p "$WORK"; cd "$WORK"
if [ ! -d esp32-arduino-lib-builder ]; then
  git clone --depth 1 -b "$BRANCH" https://github.com/espressif/esp32-arduino-lib-builder.git
fi
cd esp32-arduino-lib-builder
# build.sh runs `git symbolic-ref HEAD`; a tag checkout leaves a detached HEAD ("fatal: ref
# HEAD is not a symbolic ref") which breaks it. Pin to a local branch so HEAD is symbolic.
git switch -c detws-psram-build 2>/dev/null || git checkout -B detws-psram-build 2>/dev/null || true

# Enable the BSS-in-external-RAM Kconfig for every ESP32-S3 build variant. The octal-PSRAM
# mode (needed by N16R8 boards) already comes from the per-variant (qio_opi) config that
# lib-builder builds, so we only add the one capability flag here.
CFG="configs/defconfig.esp32s3"
touch "$CFG"
if ! grep -q "^CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY=y" "$CFG"; then
  echo "CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY=y" >> "$CFG"
  echo "==> added CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY=y to $CFG"
fi

echo "==> building arduino-esp32 libraries for esp32s3 (this is the long part)"
./build.sh -t esp32s3

LIBS_OUT="$(pwd)/out/tools/esp32-arduino-libs/esp32s3"
[ -d "$LIBS_OUT" ] || LIBS_OUT="$(pwd)/out/esp32s3"
echo "==> built libraries at: $LIBS_OUT"

if [ "$INSTALL" = "0" ]; then
  echo "==> --no-install: copy $LIBS_OUT/* over your core's esp32s3-libs dir yourself."
  exit 0
fi

# Auto-detect the arduino-cli / IDE S3 libs dir if not given.
if [ -z "$CORE_DIR" ]; then
  for base in "$HOME/.arduino15" "$HOME/Library/Arduino15" "$HOME/AppData/Local/Arduino15"; do
    cand="$(ls -d "$base"/packages/esp32/tools/esp32s3-libs/* 2>/dev/null | sort -V | tail -1 || true)"
    [ -n "$cand" ] && CORE_DIR="$cand" && break
  done
fi
[ -n "$CORE_DIR" ] || { echo "ERROR: could not find the arduino esp32s3-libs dir; pass --core-dir" >&2; exit 1; }

echo "==> installing into: $CORE_DIR"
BK="${CORE_DIR}.bak.$(date +%s)"
cp -a "$CORE_DIR" "$BK"; echo "==> backed up original to $BK"
cp -a "$LIBS_OUT/." "$CORE_DIR/"
echo "==> done. Do a FULL clean rebuild of your sketch (S3 + PSRAM=OPI)."
echo "    EXT_RAM_BSS_ATTR arrays now land in PSRAM (0x3Cxxxxxx), zero heap."
