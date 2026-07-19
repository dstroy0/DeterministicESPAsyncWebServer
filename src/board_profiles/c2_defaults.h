// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file c2_defaults.h
 * @brief ESP32-C2 (ESP8684) die profile - single RISC-V, 272 KB SRAM, Wi-Fi 4 + BLE 5.0, no PSRAM.
 *
 * The cheapest/smallest die: 272 KB SRAM is tighter than the classic ESP32's usable DRAM, so it
 * stays at the conservative floor (no sizing bump). Reduced crypto: SHA + ECC only - there is NO
 * general-purpose AES peripheral (AES exists only as the XTS flash-encryption engine) and NO
 * RSA/MPI, HMAC or DS, so secure boot is ECC-based. Do not assume "any ESP32 has AES/RSA". No PSRAM.
 * classic_defaults.h is pulled in last as the sizing floor; every macro is `#ifndef`-guarded.
 */

#ifndef DWS_C2_DEFAULTS_H
#define DWS_C2_DEFAULTS_H

// --- HW crypto accelerators (reduced: SHA + ECC only; no general AES, no RSA/HMAC/DS) ---
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
#define DWS_HW_ECDSA 0
#endif
#ifndef DWS_HW_HMAC
#define DWS_HW_HMAC 0
#endif
#ifndef DWS_HW_DS
#define DWS_HW_DS 0
#endif

// Sizing: stays at the classic floor (272 KB SRAM is tighter than the classic usable DRAM).
#include "classic_defaults.h"
#endif // DWS_C2_DEFAULTS_H
