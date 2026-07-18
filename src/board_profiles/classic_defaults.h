// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file classic_defaults.h
 * @brief Classic ESP32 (and host/native) default sizing - the universal conservative floor.
 *
 * Dual Xtensa LX6, 320 KB internal SRAM, ~122 KB `dram0_0_seg` after the stack/BSS, no PSRAM.
 * This is the smallest supported target, so these are the conservative defaults every other
 * variant file layers on top of (each chip profile includes this last as the floor). Values
 * match the library's historical flat defaults, so classic-ESP32 behavior is unchanged.
 *
 * Every macro is `#ifndef`-guarded, so a -D override or a richer variant profile wins.
 */

#ifndef DWS_CLASSIC_DEFAULTS_H
#define DWS_CLASSIC_DEFAULTS_H

// --- Edge cache (RAM-backed L1: each slot holds one cached object, ~2.6 KB) ---
#ifndef DWS_EDGE_CACHE_SLOTS
#define DWS_EDGE_CACHE_SLOTS 4 // L1 RAM entries
#endif
#ifndef DWS_EDGE_BODY_MAX
#define DWS_EDGE_BODY_MAX 2048 // largest cacheable body in bytes (per L1 entry)
#endif
#ifndef DWS_EDGE_FETCH_SLOTS
#define DWS_EDGE_FETCH_SLOTS 2 // concurrent in-flight origin fetches (<= DWS_CLIENT_CONNS)
#endif

// --- Edge mesh (sibling-cache distribution) ---
#ifndef DWS_MESH_MAX_PEERS
#define DWS_MESH_MAX_PEERS 4 // sibling peers queried on a local miss (in series, first hit wins)
#endif
#ifndef DWS_MESH_MAX_CONNS
#define DWS_MESH_MAX_CONNS 1 // concurrent inbound peer-serve connections
#endif

#endif // DWS_CLASSIC_DEFAULTS_H
