// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 16mbpsram.h
 * @brief 16 MB PSRAM profile - larger RAM-backed pools than the 8 MB profile.
 *
 * Included before the chip profile (wins for memory-bound sizes). Same PSRAM-resident caveat
 * as 8mbpsram.h: the internal DRAM budget bounds the L1 slot count until an edge-cache
 * PSRAM placement lands. `#ifndef`-guarded, so your -D overrides still win.
 */

#ifndef DWS_16MBPSRAM_H
#define DWS_16MBPSRAM_H

#ifndef DWS_EDGE_CACHE_SLOTS
#define DWS_EDGE_CACHE_SLOTS 24
#endif
#ifndef DWS_EDGE_BODY_MAX
#define DWS_EDGE_BODY_MAX 16384
#endif
#ifndef DWS_EDGE_FETCH_SLOTS
#define DWS_EDGE_FETCH_SLOTS 6
#endif
#ifndef DWS_MESH_MAX_PEERS
#define DWS_MESH_MAX_PEERS 12
#endif
#ifndef DWS_MESH_MAX_CONNS
#define DWS_MESH_MAX_CONNS 3
#endif

#endif // DWS_16MBPSRAM_H
