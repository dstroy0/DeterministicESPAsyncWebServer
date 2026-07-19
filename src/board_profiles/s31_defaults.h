// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file s31_defaults.h
 * @brief ESP32-S31 die profile (PREVIEW) - dual RISC-V up to 320 MHz, 512 KB SRAM, multiprotocol.
 *
 * PREVIEW: the target exists in ESP-IDF `master` but not a stable release yet - re-verify specs at
 * release. Brings Bluetooth Classic back (BT 5.4 LE+Classic) alongside Wi-Fi 6, 802.15.4, and a
 * Gigabit-Ethernet MAC; an HMI/multiprotocol part. 512 KB SRAM, dual core, so a modest bump like
 * the S3. Full crypto HW: AES, SHA, RSA/MPI (4096-bit), ECC, ECDSA, HMAC, DS. Supports PSRAM.
 * classic_defaults.h is the sizing floor; every macro is `#ifndef`-guarded.
 */

#ifndef DWS_S31_DEFAULTS_H
#define DWS_S31_DEFAULTS_H

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

// --- Sizing (modest bump; dual core, 512 KB) ---
#ifndef DWS_EDGE_CACHE_SLOTS
#define DWS_EDGE_CACHE_SLOTS 8
#endif
#ifndef DWS_EDGE_BODY_MAX
#define DWS_EDGE_BODY_MAX 4096
#endif
#ifndef DWS_EDGE_FETCH_SLOTS
#define DWS_EDGE_FETCH_SLOTS 3
#endif
#ifndef DWS_MESH_MAX_PEERS
#define DWS_MESH_MAX_PEERS 6
#endif
#ifndef DWS_MESH_MAX_CONNS
#define DWS_MESH_MAX_CONNS 2
#endif

#include "classic_defaults.h"
#endif // DWS_S31_DEFAULTS_H
