// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file c6_defaults.h
 * @brief ESP32-C6 die profile - RISC-V HP + LP core, 512 KB SRAM, Wi-Fi 6 + BLE 5 + 802.15.4.
 *
 * Wi-Fi 6 with Thread/Zigbee (802.15.4) and a low-power core - a Matter-class part. 512 KB HP SRAM
 * with a roomy usable-DRAM map, so a small bump over the floor. Crypto HW: AES, SHA, RSA/MPI, ECC,
 * HMAC, DS - but NO ECDSA accelerator (unlike C5/H2/P4). No PSRAM. classic_defaults.h is the sizing
 * floor; every macro is `#ifndef`-guarded.
 */

#ifndef DWS_C6_DEFAULTS_H
#define DWS_C6_DEFAULTS_H

// --- HW crypto accelerators (no ECDSA peripheral) ---
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
#define DWS_HW_ECDSA 0
#endif
#ifndef DWS_HW_HMAC
#define DWS_HW_HMAC 1
#endif
#ifndef DWS_HW_DS
#define DWS_HW_DS 1
#endif

// --- Sizing (small bump over the floor) ---
#ifndef DWS_EDGE_CACHE_SLOTS
#define DWS_EDGE_CACHE_SLOTS 6
#endif
#ifndef DWS_EDGE_BODY_MAX
#define DWS_EDGE_BODY_MAX 4096
#endif

#include "classic_defaults.h"
#endif // DWS_C6_DEFAULTS_H
