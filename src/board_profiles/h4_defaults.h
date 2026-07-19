// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file h4_defaults.h
 * @brief ESP32-H4 die profile (PREVIEW) - dual RISC-V, 384 KB SRAM, BLE 5.4 + 802.15.4, no Wi-Fi.
 *
 * PREVIEW (target present since ESP-IDF v6.0, not a stable release - re-verify at release). The
 * dual-core H2 successor with more RAM and PSRAM support, for wireless-audio / Matter. No Wi-Fi;
 * BLE 5.4 + 802.15.4. Crypto HW is the odd one among the H-parts: AES, SHA, ECC, ECDSA, HMAC -
 * but NO RSA/MPI and NO DS. Stays at the floor (no Wi-Fi, 384 KB). classic_defaults.h is the sizing
 * floor; every macro is `#ifndef`-guarded.
 */

#ifndef DWS_H4_DEFAULTS_H
#define DWS_H4_DEFAULTS_H

// --- HW crypto accelerators (no RSA/MPI, no DS) ---
#ifndef DWS_HW_AES
#define DWS_HW_AES 1
#endif
#ifndef DWS_HW_SHA
#define DWS_HW_SHA 1
#endif
#ifndef DWS_HW_RSA
#define DWS_HW_RSA 0
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
#define DWS_HW_DS 0
#endif

// Sizing: stays at the classic floor (no Wi-Fi, 384 KB).
#include "classic_defaults.h"
#endif // DWS_H4_DEFAULTS_H
