// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file p4_defaults.h
 * @brief ESP32-P4 die profile - dual RISC-V + FPU up to 400 MHz, 768 KB L2MEM, no radio.
 *
 * The high-performance host MCU: dual RISC-V with FPU/AI, MIPI-CSI/DSI, USB-HS, and the largest
 * internal SRAM (768 KB L2MEM). It has NO built-in radio (pairs with a companion Wi-Fi/BT chip),
 * and is commonly fitted with high-bandwidth PSRAM up to 32 MB (a PSRAM profile, included first,
 * scales further). Full crypto HW: AES, SHA, RSA/MPI (4096-bit), ECC, ECDSA, HMAC, DS.
 * classic_defaults.h is the sizing floor; every macro is `#ifndef`-guarded.
 */

#ifndef DWS_P4_DEFAULTS_H
#define DWS_P4_DEFAULTS_H

// --- HW crypto accelerators (full suite, RSA up to 4096-bit) ---
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

#include "classic_defaults.h"
#endif // DWS_P4_DEFAULTS_H
