// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file h2_defaults.h
 * @brief ESP32-H2 die profile - single RISC-V, 320 KB SRAM, BLE 5.0 + 802.15.4, NO Wi-Fi.
 *
 * A Thread/Zigbee/Matter radio part: 802.15.4 + BLE 5.0, no Wi-Fi. 320 KB SRAM, single core, so it
 * stays at the conservative floor. Full crypto HW: AES, SHA, RSA/MPI, ECC, ECDSA, HMAC, DS. No
 * PSRAM. classic_defaults.h is the sizing floor; every macro is `#ifndef`-guarded.
 */

#ifndef DWS_H2_DEFAULTS_H
#define DWS_H2_DEFAULTS_H

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

// --- Sizing (conservative: single core, 320 KB SRAM, no Wi-Fi) ---
// Internal-SRAM-budget values (no PSRAM assumed); a PSRAM-size profile, included first, scales the
// RAM-backed buffers further and moves the big TLS / HTTP-2 pools off-chip.

// Connection pools + per-connection buffers.
#ifndef MAX_CONNS
#define MAX_CONNS 8
#endif
#ifndef RX_BUF_SIZE
#define RX_BUF_SIZE 1024
#endif
#ifndef DWS_SCRATCH_ARENA_SIZE
#define DWS_SCRATCH_ARENA_SIZE 8192
#endif
#ifndef DWS_CLIENT_RX_BUF
#define DWS_CLIENT_RX_BUF 4096
#endif

// HTTP surface.
#ifndef MAX_ROUTES
#define MAX_ROUTES 16
#endif
#ifndef MAX_HEADERS
#define MAX_HEADERS 8
#endif
#ifndef BODY_BUF_SIZE
#define BODY_BUF_SIZE 256
#endif

// WebSocket / SSE fan-out.
#ifndef MAX_WS_CONNS
#define MAX_WS_CONNS 2
#endif
#ifndef MAX_SSE_CONNS
#define MAX_SSE_CONNS 2
#endif

// TLS: a single handshake fits the tight internal SRAM; a PSRAM profile raises this and moves the arena.
#ifndef MAX_TLS_CONNS
#define MAX_TLS_CONNS 1
#endif

// SSH server + reverse-SSH client.
#ifndef MAX_SSH_CONNS
#define MAX_SSH_CONNS 1
#endif
#ifndef DWS_SSH_MAX_CHANNELS
#define DWS_SSH_MAX_CHANNELS 2
#endif
#ifndef DWS_SSH_CLIENT_MAX_CHANNELS
#define DWS_SSH_CLIENT_MAX_CHANNELS 2
#endif

// Edge cache + mesh (RAM-backed L1).
#ifndef DWS_EDGE_CACHE_SLOTS
#define DWS_EDGE_CACHE_SLOTS 4
#endif
#ifndef DWS_EDGE_BODY_MAX
#define DWS_EDGE_BODY_MAX 2048
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

#include "classic_defaults.h"
#endif // DWS_H2_DEFAULTS_H
