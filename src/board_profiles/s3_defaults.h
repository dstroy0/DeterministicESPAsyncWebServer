// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file s3_defaults.h
 * @brief ESP32-S3 die profile - dual Xtensa LX7, 512 KB SRAM, Wi-Fi 4 + BLE 5.0, Octal PSRAM.
 *
 * Roomier usable DRAM than the classic ESP32 plus AI/vector DSP instructions, so the RAM-backed
 * pools get a modest bump even with no PSRAM fitted (a PSRAM profile, included first, scales them
 * further). Crypto HW: AES, SHA, RSA/MPI, HMAC, DS (no ECC/ECDSA). classic_defaults.h is pulled
 * in last as the sizing floor; every macro is `#ifndef`-guarded so a -D override wins.
 */

#ifndef DWS_S3_DEFAULTS_H
#define DWS_S3_DEFAULTS_H

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

// --- Sizing (bumped over the classic floor to use the roomier DRAM) ---
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

#include "classic_defaults.h" // sizing floor for anything not set above
#endif                        // DWS_S3_DEFAULTS_H
