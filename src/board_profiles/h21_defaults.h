// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file h21_defaults.h
 * @brief ESP32-H21 die profile (PREVIEW) - single RISC-V, 320 KB SRAM, BLE 5 + 802.15.4, no Wi-Fi.
 *
 * PREVIEW: the target exists in ESP-IDF `master` but not a stable release yet - re-verify at
 * release. An ultra-low-power BLE / 802.15.4 part with an integrated DC-DC; no Wi-Fi. 320 KB SRAM,
 * single core, so it stays at the conservative floor. Full crypto HW: AES, SHA, RSA/MPI, ECC,
 * ECDSA, HMAC, DS. No PSRAM. classic_defaults.h is the sizing floor; every macro is `#ifndef`-guarded.
 */

#ifndef DWS_H21_DEFAULTS_H
#define DWS_H21_DEFAULTS_H

// --- HW crypto accelerators (full suite) ---
#ifndef DWS_HW_AES
#define DWS_HW_AES 1
#endif
#ifndef DWS_HW_SHA
#define DWS_HW_SHA 1
#endif
#ifndef DWS_HW_RSA
#define DWS_HW_RSA 1
#endif
#ifndef DWS_HW_ECC
#define DWS_HW_ECC 1
#endif
#ifndef DWS_HW_ECDSA
#define DWS_HW_ECDSA 1
#endif
#ifndef DWS_HW_HMAC
#define DWS_HW_HMAC 1
#endif
#ifndef DWS_HW_DS
#define DWS_HW_DS 1
#endif

// Sizing: stays at the classic floor (single core, 320 KB, no Wi-Fi).
#include "classic_defaults.h"
#endif // DWS_H21_DEFAULTS_H
