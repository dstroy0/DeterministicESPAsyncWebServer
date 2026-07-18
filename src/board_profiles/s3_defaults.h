// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file s3_defaults.h
 * @brief ESP32-S3 chip defaults - dual Xtensa LX7, 512 KB internal SRAM, crypto HW accel.
 *
 * More internal SRAM than the classic ESP32, so the RAM-backed pools get a modest bump even
 * with no PSRAM fitted; a PSRAM profile (included first) scales them further. HW-specific
 * switches for this SoC also live here. classic_defaults.h is pulled in last as the floor.
 */

#ifndef DWS_S3_DEFAULTS_H
#define DWS_S3_DEFAULTS_H

// --- HW-specific switches (S3 has AES / SHA / RSA-MPI acceleration) ---
#ifndef DWS_HAS_CRYPTO_HWACCEL
#define DWS_HAS_CRYPTO_HWACCEL 1
#endif

// --- Sizing (bumped over the classic floor to use the larger 512 KB SRAM) ---
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

#include "classic_defaults.h" // universal floor for anything not set above
#endif                        // DWS_S3_DEFAULTS_H
