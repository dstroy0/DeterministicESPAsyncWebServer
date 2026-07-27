// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dws_platform.h
 * @brief The one vendor/die selector for the whole library.
 *
 * Multi-vendor portability rests on a single rule: every silicon-specific layer (board profiles,
 * the crypto accelerator HAL, the physical MAC + PHY) is partitioned into a per-vendor subdir and a
 * common API header pulls in exactly ONE backend per build. This header owns the "which vendor" decision
 * so nothing downstream has to re-test toolchain-specific macros - a backend keys off `DWS_VENDOR_*`, not
 * off `CONFIG_IDF_TARGET_*` / `STM32*` / `PICO_*` scattered across the tree.
 *
 * Exactly one `DWS_VENDOR_*` is 1; every other is defined 0 (so `#if DWS_VENDOR_ESP` is always valid, never
 * relies on an undefined-macro-is-0 fallback). The vendor is derived from the toolchain's own target macro:
 *
 *   - `DWS_VENDOR_ESP`  - any Espressif target (ESP-IDF `ESP_PLATFORM` / Arduino-ESP32 `ARDUINO_ARCH_ESP32`).
 *   - `DWS_VENDOR_STM`  - STM32 (Arduino_Core_STM32 `ARDUINO_ARCH_STM32` / STM32Cube `USE_HAL_DRIVER`).
 *   - `DWS_VENDOR_RP`   - Raspberry Pi silicon (RP2040 / RP2350: `ARDUINO_ARCH_RP2040` / `PICO_*`).
 *   - `DWS_VENDOR_TI`   - Texas Instruments (`__TI_COMPILER_VERSION__` or an explicit force).
 *   - `DWS_VENDOR_HOST` - native / host build (unit tests): no accelerator, portable software everywhere.
 *
 * ESP is detected first and stays byte-for-byte compatible with the pre-selector behavior: on every ESP
 * build `DWS_VENDOR_ESP` is 1, and on host builds it is 0, exactly matching the old
 * `#if defined(CONFIG_IDF_TARGET_*)` test in board_profile.h.
 */

#ifndef DWS_PLATFORM_H
#define DWS_PLATFORM_H

// sdkconfig.h carries CONFIG_IDF_TARGET_* on ESP-IDF / Arduino-ESP32 builds and is absent on host and on
// other vendors' toolchains. Pull it in here (guarded by __has_include) so vendor + die detection is
// include-order-independent, the same reason board_profile.h does it. Preceded only by the include guard.
#if defined(__has_include)
#if __has_include("sdkconfig.h")
#include "sdkconfig.h"
#endif
#endif

// ---------------------------------------------------------------------------
// Vendor axis - exactly one is 1.
// ---------------------------------------------------------------------------
#if defined(ESP_PLATFORM) || defined(ARDUINO_ARCH_ESP32)
#define DWS_VENDOR_ESP 1
#elif defined(ARDUINO_ARCH_STM32) || defined(USE_HAL_DRIVER) || defined(STM32_CORE_VERSION)
#define DWS_VENDOR_STM 1
#elif defined(ARDUINO_ARCH_RP2040) || defined(PICO_RP2040) || defined(PICO_RP2350) || defined(PICO_SDK_VERSION_MAJOR)
#define DWS_VENDOR_RP 1
#elif defined(__TI_COMPILER_VERSION__) || defined(DWS_VENDOR_TI_FORCE)
#define DWS_VENDOR_TI 1
#else
#define DWS_VENDOR_HOST 1
#endif

// Every vendor macro is defined (0 when not selected) so downstream code can `#if DWS_VENDOR_x` freely.
#ifndef DWS_VENDOR_ESP
#define DWS_VENDOR_ESP 0
#endif
#ifndef DWS_VENDOR_STM
#define DWS_VENDOR_STM 0
#endif
#ifndef DWS_VENDOR_RP
#define DWS_VENDOR_RP 0
#endif
#ifndef DWS_VENDOR_TI
#define DWS_VENDOR_TI 0
#endif
#ifndef DWS_VENDOR_HOST
#define DWS_VENDOR_HOST 0
#endif

// A single "targets real silicon" convenience (any vendor backend, i.e. not the host software floor).
#define DWS_VENDOR_SILICON (DWS_VENDOR_ESP || DWS_VENDOR_STM || DWS_VENDOR_RP || DWS_VENDOR_TI)

#endif // DWS_PLATFORM_H
