// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file board_profile.h
 * @brief Per-variant default sizing: pick sane DWS_* defaults for the target board.
 *
 * The library's sizing defaults used to be a single flat set tuned to fit the smallest
 * classic-ESP32 DRAM ceiling, so a board with far more RAM/flash silently inherited the
 * same cramped numbers. This selector instead layers per-variant default files:
 *
 *   - chip variant  (classic / s3 / c6 / p4) - HW-specific switches + chip-appropriate
 *     defaults (internal SRAM, core count, crypto HW accel).
 *   - PSRAM size    (8 / 16 / 32 MB)          - RAM-backed buffer sizes; a given chip
 *     ships with or without PSRAM, so it is its own axis.
 *   - flash size    (8 / 16 / 32 MB)          - flash-backed sizing; likewise independent.
 *
 * Every default is set behind an `#ifndef`, so precedence is "first definition wins":
 *   your -D / build_opt.h override  >  PSRAM profile  >  flash profile  >  chip profile
 * (chip files pull in classic_defaults.h last as the universal floor). Nothing here forces
 * a value you set yourself.
 *
 * Chip is auto-detected from the SoC target macro. PSRAM/flash size can't be read reliably
 * from the Arduino core, so set them for your board (they default to "none / smallest"):
 * @code
 *   build_flags = -DDWS_PSRAM_MB=8 -DDWS_FLASH_MB=16
 * @endcode
 * ESP-IDF builds auto-fill both from the sdkconfig below.
 */

#ifndef DWS_BOARD_PROFILE_H
#define DWS_BOARD_PROFILE_H

// --- flash size (MB): honor an explicit -DDWS_FLASH_MB, else read the ESP-IDF sdkconfig ---
#if !defined(DWS_FLASH_MB)
#if defined(CONFIG_ESPTOOLPY_FLASHSIZE_32MB)
#define DWS_FLASH_MB 32
#elif defined(CONFIG_ESPTOOLPY_FLASHSIZE_16MB)
#define DWS_FLASH_MB 16
#elif defined(CONFIG_ESPTOOLPY_FLASHSIZE_8MB)
#define DWS_FLASH_MB 8
#elif defined(CONFIG_ESPTOOLPY_FLASHSIZE_4MB)
#define DWS_FLASH_MB 4
#elif defined(CONFIG_ESPTOOLPY_FLASHSIZE_2MB)
#define DWS_FLASH_MB 2
#endif
#endif

// --- PSRAM size (MB): honor an explicit -DDWS_PSRAM_MB, else read the ESP-IDF sdkconfig ---
#if !defined(DWS_PSRAM_MB) && defined(CONFIG_SPIRAM_SIZE)
#if CONFIG_SPIRAM_SIZE >= (32 * 1024 * 1024)
#define DWS_PSRAM_MB 32
#elif CONFIG_SPIRAM_SIZE >= (16 * 1024 * 1024)
#define DWS_PSRAM_MB 16
#elif CONFIG_SPIRAM_SIZE >= (8 * 1024 * 1024)
#define DWS_PSRAM_MB 8
#elif CONFIG_SPIRAM_SIZE >= (4 * 1024 * 1024)
#define DWS_PSRAM_MB 4
#elif CONFIG_SPIRAM_SIZE >= (2 * 1024 * 1024)
#define DWS_PSRAM_MB 2
#endif
#endif

// --- PSRAM-size profile (most specific: RAM-backed buffers scale with available PSRAM) ---
#if defined(DWS_PSRAM_MB)
#if DWS_PSRAM_MB >= 32
#include "32mbpsram.h"
#elif DWS_PSRAM_MB >= 16
#include "16mbpsram.h"
#elif DWS_PSRAM_MB >= 8
#include "8mbpsram.h"
#elif DWS_PSRAM_MB >= 4
#include "4mbpsram.h"
#elif DWS_PSRAM_MB >= 2
#include "2mbpsram.h"
#endif
#endif

// --- flash-size profile (flash-backed sizing scales with available flash) ---
#if defined(DWS_FLASH_MB)
#if DWS_FLASH_MB >= 32
#include "32mbflash.h"
#elif DWS_FLASH_MB >= 16
#include "16mbflash.h"
#elif DWS_FLASH_MB >= 8
#include "8mbflash.h"
#elif DWS_FLASH_MB >= 4
#include "4mbflash.h"
#elif DWS_FLASH_MB >= 2
#include "2mbflash.h"
#endif
#endif

// --- chip profile (auto-selected from the SoC target macro; each pulls classic_defaults.h in
//     last as the universal sizing floor). Every macro name is uppercase with no hyphen/underscore
//     in the suffix (e.g. ...ESP32C61), verified against ESP-IDF's components/soc/<target>/.
//     S31/H4/H21 are preview targets (in ESP-IDF master, not a stable release yet). ---
#if defined(CONFIG_IDF_TARGET_ESP32P4)
#include "p4_defaults.h"
#elif defined(CONFIG_IDF_TARGET_ESP32S31)
#include "s31_defaults.h"
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
#include "s3_defaults.h"
#elif defined(CONFIG_IDF_TARGET_ESP32S2)
#include "s2_defaults.h"
#elif defined(CONFIG_IDF_TARGET_ESP32C2)
#include "c2_defaults.h"
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
#include "c3_defaults.h"
#elif defined(CONFIG_IDF_TARGET_ESP32C5)
#include "c5_defaults.h"
#elif defined(CONFIG_IDF_TARGET_ESP32C61)
#include "c61_defaults.h"
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
#include "c6_defaults.h"
#elif defined(CONFIG_IDF_TARGET_ESP32H21)
#include "h21_defaults.h"
#elif defined(CONFIG_IDF_TARGET_ESP32H2)
#include "h2_defaults.h"
#elif defined(CONFIG_IDF_TARGET_ESP32H4)
#include "h4_defaults.h"
#else
// Classic ESP32 and host/native builds (no SoC target macro) land here.
#include "classic_defaults.h"
#endif

#endif // DWS_BOARD_PROFILE_H
