// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file c6_defaults.h
 * @brief ESP32-C6 chip defaults - single RISC-V, 512 KB SRAM, Wi-Fi 6 + BLE + 802.15.4.
 *
 * Similar internal SRAM to the S3 but a single core, so the concurrency-bound pools stay
 * nearer the classic floor while the memory-bound ones get a modest bump. HW-specific
 * switches for this SoC live here. classic_defaults.h is pulled in last as the floor.
 */

#ifndef DWS_C6_DEFAULTS_H
#define DWS_C6_DEFAULTS_H

// --- HW-specific switches (C6 has AES / SHA / ECC acceleration) ---
#ifndef DWS_HAS_CRYPTO_HWACCEL
#define DWS_HAS_CRYPTO_HWACCEL 1
#endif

// --- Sizing (memory-bound pools bumped; single core keeps concurrency near the floor) ---
#ifndef DWS_EDGE_CACHE_SLOTS
#define DWS_EDGE_CACHE_SLOTS 6
#endif
#ifndef DWS_EDGE_BODY_MAX
#define DWS_EDGE_BODY_MAX 4096
#endif
#ifndef DWS_EDGE_FETCH_SLOTS
#define DWS_EDGE_FETCH_SLOTS 2
#endif
#ifndef DWS_MESH_MAX_PEERS
#define DWS_MESH_MAX_PEERS 4
#endif
#ifndef DWS_MESH_MAX_CONNS
#define DWS_MESH_MAX_CONNS 1
#endif

#include "classic_defaults.h" // universal floor for anything not set above
#endif                        // DWS_C6_DEFAULTS_H
