// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 2mbpsram.h
 * @brief 2 MB PSRAM profile - the smallest external-RAM step (e.g. classic WROVER, some C-series).
 *
 * Included before the chip profile (wins for memory-bound sizes). A modest lift over the no-PSRAM
 * floor; the larger PSRAM profiles scale further. Same PSRAM-resident caveat as 8mbpsram.h: the
 * internal DRAM budget still bounds the L1 slot count until an edge-cache PSRAM placement lands.
 * `#ifndef`-guarded, so your -D overrides still win.
 */

#ifndef DWS_2MBPSRAM_H
#define DWS_2MBPSRAM_H

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

#endif // DWS_2MBPSRAM_H
