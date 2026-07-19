// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file s2_defaults.h
 * @brief ESP32-S2 die profile - single Xtensa LX7, 320 KB SRAM, Wi-Fi 4 only (no Bluetooth).
 *
 * Single core and 320 KB SRAM (tighter than the classic ESP32's usable DRAM), so it stays at the
 * conservative floor - no sizing bump. Native USB-OTG; the only radio is Wi-Fi 4 (no Bluetooth at
 * all). Crypto HW: AES, SHA, RSA/MPI, HMAC, DS (no ECC/ECDSA). Supports external PSRAM (2 MB in
 * the in-package R2 parts). classic_defaults.h is the sizing floor; every macro is `#ifndef`-guarded.
 */

#ifndef DWS_S2_DEFAULTS_H
#define DWS_S2_DEFAULTS_H

// --- HW crypto accelerators ---
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
#define DWS_HW_ECC 0
#endif
#ifndef DWS_HW_ECDSA
#define DWS_HW_ECDSA 0
#endif
#ifndef DWS_HW_HMAC
#define DWS_HW_HMAC 1
#endif
#ifndef DWS_HW_DS
#define DWS_HW_DS 1
#endif

// Sizing: stays at the classic floor (single core, 320 KB SRAM).
#include "classic_defaults.h"
#endif // DWS_S2_DEFAULTS_H
