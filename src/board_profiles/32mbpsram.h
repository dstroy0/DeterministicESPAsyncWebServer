// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 32mbpsram.h
 * @brief 32 MB PSRAM profile - the largest RAM-backed pools.
 *
 * Included before the chip profile (wins for memory-bound sizes). Same PSRAM-resident caveat
 * as 8mbpsram.h. `#ifndef`-guarded, so your -D overrides still win.
 */

#ifndef DWS_32MBPSRAM_H
#define DWS_32MBPSRAM_H

// PSRAM fitted: move the big TLS/HTTP-2/zlib pools off internal DRAM and raise concurrency.
#ifndef DWS_TLS_ARENA_IN_PSRAM
#define DWS_TLS_ARENA_IN_PSRAM 1
#endif
#ifndef DWS_H2_POOL_IN_PSRAM
#define DWS_H2_POOL_IN_PSRAM 1
#endif
#ifndef DWS_SSH_ZLIB_IN_PSRAM
#define DWS_SSH_ZLIB_IN_PSRAM 1
#endif
#ifndef MAX_TLS_CONNS
#define MAX_TLS_CONNS 8
#endif
#ifndef DWS_H2_MAX_STREAMS
#define DWS_H2_MAX_STREAMS 32
#endif
#ifndef DWS_H3_MAX_STREAMS
#define DWS_H3_MAX_STREAMS 32
#endif
#ifndef DWS_EDGE_CACHE_SLOTS
#define DWS_EDGE_CACHE_SLOTS 32
#endif
#ifndef DWS_EDGE_BODY_MAX
#define DWS_EDGE_BODY_MAX 16384
#endif

#endif // DWS_32MBPSRAM_H
