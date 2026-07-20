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

// --- Sizing (bump over the floor; HP+LP core, 512 KB SRAM) ---
// Internal-SRAM-budget values (no PSRAM assumed); a PSRAM-size profile, included first, scales the
// RAM-backed buffers further and moves the big TLS / HTTP-2 pools off-chip.

// Connection pools + per-connection buffers.
#ifndef MAX_CONNS
#define MAX_CONNS 12
#endif
#ifndef RX_BUF_SIZE
#define RX_BUF_SIZE 1536
#endif
#ifndef DWS_SCRATCH_ARENA_SIZE
#define DWS_SCRATCH_ARENA_SIZE 10240
#endif
#ifndef DWS_CLIENT_RX_BUF
#define DWS_CLIENT_RX_BUF 8192
#endif

// HTTP surface.
#ifndef MAX_ROUTES
#define MAX_ROUTES 24
#endif
#ifndef MAX_HEADERS
#define MAX_HEADERS 12
#endif
#ifndef BODY_BUF_SIZE
#define BODY_BUF_SIZE 512
#endif

// WebSocket / SSE fan-out.
#ifndef MAX_WS_CONNS
#define MAX_WS_CONNS 4
#endif
#ifndef MAX_SSE_CONNS
#define MAX_SSE_CONNS 4
#endif

// TLS: one handshake on the internal-DRAM arena; a PSRAM profile raises this with the arena in PSRAM.
#ifndef MAX_TLS_CONNS
#define MAX_TLS_CONNS 1
#endif

// SSH server + reverse-SSH client.
#ifndef MAX_SSH_CONNS
#define MAX_SSH_CONNS 2
#endif
#ifndef DWS_SSH_MAX_CHANNELS
#define DWS_SSH_MAX_CHANNELS 4
#endif
#ifndef DWS_SSH_CLIENT_MAX_CHANNELS
#define DWS_SSH_CLIENT_MAX_CHANNELS 4
#endif

// Edge cache + mesh (RAM-backed L1).
#ifndef DWS_EDGE_CACHE_SLOTS
#define DWS_EDGE_CACHE_SLOTS 6
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
#define DWS_MESH_MAX_CONNS 1
#endif

#include "classic_defaults.h"
#endif // DWS_C6_DEFAULTS_H
