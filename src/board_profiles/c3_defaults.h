// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file c3_defaults.h
 * @brief ESP32-C3 die profile - single RISC-V, 400 KB SRAM, Wi-Fi 4 + BLE 5.0, no PSRAM.
 *
 * The mainstream single-core RISC-V drop-in for the classic ESP32. 400 KB SRAM with a roomier
 * usable-DRAM map than the classic die, so a small bump over the floor; single core keeps the
 * concurrency-bound pools modest. Crypto HW: AES, SHA, RSA/MPI, HMAC, DS (no ECC/ECDSA). No PSRAM.
 * classic_defaults.h is the sizing floor; every macro is `#ifndef`-guarded.
 */

#ifndef DWS_C3_DEFAULTS_H
#define DWS_C3_DEFAULTS_H

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

// --- Sizing (small bump over the floor; single core) ---
#ifndef DWS_EDGE_CACHE_SLOTS
#define DWS_EDGE_CACHE_SLOTS 6
#endif
#ifndef DWS_EDGE_BODY_MAX
#define DWS_EDGE_BODY_MAX 4096
#endif

#include "classic_defaults.h"
#endif // DWS_C3_DEFAULTS_H
