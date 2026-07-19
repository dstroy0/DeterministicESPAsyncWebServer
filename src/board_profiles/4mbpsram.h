// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 4mbpsram.h
 * @brief 4 MB PSRAM profile - between the 2 MB and 8 MB steps.
 *
 * Included before the chip profile (wins for memory-bound sizes). Same PSRAM-resident caveat as
 * 8mbpsram.h. `#ifndef`-guarded, so your -D overrides still win.
 */

#ifndef DWS_4MBPSRAM_H
#define DWS_4MBPSRAM_H

#ifndef DWS_EDGE_CACHE_SLOTS
#define DWS_EDGE_CACHE_SLOTS 12
#endif
#ifndef DWS_EDGE_BODY_MAX
#define DWS_EDGE_BODY_MAX 8192
#endif
#ifndef DWS_EDGE_FETCH_SLOTS
#define DWS_EDGE_FETCH_SLOTS 3
#endif
#ifndef DWS_MESH_MAX_PEERS
#define DWS_MESH_MAX_PEERS 8
#endif
#ifndef DWS_MESH_MAX_CONNS
#define DWS_MESH_MAX_CONNS 2
#endif

#endif // DWS_4MBPSRAM_H
