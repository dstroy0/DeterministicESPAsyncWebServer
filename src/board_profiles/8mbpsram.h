// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 8mbpsram.h
 * @brief 8 MB PSRAM profile - scale the RAM-backed pools up past the chip's internal SRAM.
 *
 * Included before the chip profile, so these win for the memory-bound sizes. Note: growing
 * the L1 slot count only pays off once the cache actually lives in PSRAM; until an edge-cache
 * PSRAM-resident placement lands, the internal DRAM budget still bounds these, so the bumps
 * here stay conservative. `#ifndef`-guarded, so your -D overrides still win.
 */

#ifndef DWS_8MBPSRAM_H
#define DWS_8MBPSRAM_H

#ifndef DWS_EDGE_CACHE_SLOTS
#define DWS_EDGE_CACHE_SLOTS 16
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

#endif // DWS_8MBPSRAM_H
