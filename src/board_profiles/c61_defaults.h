// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file c61_defaults.h
 * @brief ESP32-C61 die profile - single RISC-V, 320 KB SRAM, cost-optimized Wi-Fi 6 + BLE 5.0.
 *
 * A cost-optimized Wi-Fi 6 part (no 802.15.4). 320 KB SRAM is tighter than the classic ESP32's
 * usable DRAM, so it stays at the conservative floor - no sizing bump. Reduced crypto like the C2:
 * SHA + ECC + ECDSA only - NO general-purpose AES peripheral (AES is XTS-flash-only) and NO
 * RSA/MPI, HMAC or DS. Supports in-package Quad PSRAM. classic_defaults.h is the sizing floor;
 * every macro is `#ifndef`-guarded.
 */

#ifndef DWS_C61_DEFAULTS_H
#define DWS_C61_DEFAULTS_H

// --- HW crypto accelerators (reduced: SHA + ECC + ECDSA; no general AES, no RSA/HMAC/DS) ---
#ifndef DWS_HW_AES
#define DWS_HW_AES 0
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
#define DWS_HW_HMAC 0
#endif
#ifndef DWS_HW_DS
#define DWS_HW_DS 0
#endif

// Sizing: stays at the classic floor (320 KB SRAM).
#include "classic_defaults.h"
#endif // DWS_C61_DEFAULTS_H
