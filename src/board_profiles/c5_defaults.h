// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file c5_defaults.h
 * @brief ESP32-C5 die profile - single RISC-V (+LP), 384 KB SRAM, dual-band Wi-Fi 6, PSRAM.
 *
 * First RISC-V part with 2.4 + 5 GHz dual-band Wi-Fi 6 (plus BLE 5.0 and 802.15.4). 384 KB HP SRAM
 * with a roomy usable-DRAM map, so a small bump over the floor. Full crypto HW: AES, SHA, RSA/MPI,
 * ECC, ECDSA, HMAC, DS. Supports external PSRAM. classic_defaults.h is the sizing floor; every
 * macro is `#ifndef`-guarded.
 */

#ifndef DWS_C5_DEFAULTS_H
#define DWS_C5_DEFAULTS_H

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

// --- Sizing (small bump over the floor) ---
#ifndef DWS_EDGE_CACHE_SLOTS
#define DWS_EDGE_CACHE_SLOTS 6
#endif
#ifndef DWS_EDGE_BODY_MAX
#define DWS_EDGE_BODY_MAX 4096
#endif
#ifndef DWS_MESH_MAX_PEERS
#define DWS_MESH_MAX_PEERS 6
#endif

#include "classic_defaults.h"
#endif // DWS_C5_DEFAULTS_H
