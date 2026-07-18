// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file p4_defaults.h
 * @brief ESP32-P4 chip defaults - dual RISC-V (high performance), 768 KB SRAM, no radio.
 *
 * The largest internal SRAM of the supported chips and a fast dual core, so the pools get
 * the biggest no-PSRAM bump; the P4 has no built-in Wi-Fi (it pairs with a companion radio),
 * and is commonly fitted with PSRAM (a PSRAM profile, included first, scales further). HW-
 * specific switches live here. classic_defaults.h is pulled in last as the floor.
 */

#ifndef DWS_P4_DEFAULTS_H
#define DWS_P4_DEFAULTS_H

// --- HW-specific switches (P4 has a large crypto/DMA accelerator set) ---
#ifndef DWS_HAS_CRYPTO_HWACCEL
#define DWS_HAS_CRYPTO_HWACCEL 1
#endif

// --- Sizing (largest no-PSRAM bump: 768 KB SRAM + fast dual core) ---
#ifndef DWS_EDGE_CACHE_SLOTS
#define DWS_EDGE_CACHE_SLOTS 12
#endif
#ifndef DWS_EDGE_BODY_MAX
#define DWS_EDGE_BODY_MAX 8192
#endif
#ifndef DWS_EDGE_FETCH_SLOTS
#define DWS_EDGE_FETCH_SLOTS 4
#endif
#ifndef DWS_MESH_MAX_PEERS
#define DWS_MESH_MAX_PEERS 8
#endif
#ifndef DWS_MESH_MAX_CONNS
#define DWS_MESH_MAX_CONNS 2
#endif

#include "classic_defaults.h" // universal floor for anything not set above
#endif                        // DWS_P4_DEFAULTS_H
