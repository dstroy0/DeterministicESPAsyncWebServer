// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file edge_cache_proxy.h
 * @brief CDN edge-cache tier - server glue (DETWS_ENABLE_EDGE_CACHE).
 *
 * Wires the pure engine (edge_cache) + async fetch (edge_fetch) into a DetWebServer: registers the
 * cache as a middleware and installs the async-fetch poll hook, maps request path prefixes to upstream
 * origins (fetched over det_client), and serves hits with the constant-memory send-pump. A miss or a
 * stale-entry revalidation suspends the client request and drives the origin fetch from the slot's poll,
 * so the worker never stalls; every failure path fails open. Purge + stats round it out.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_EDGE_CACHE_PROXY_H
#define DETERMINISTICESPASYNCWEBSERVER_EDGE_CACHE_PROXY_H

#include "ServerConfig.h"

#if DETWS_ENABLE_EDGE_CACHE

#include "services/edge_cache/edge_cache.h" // EdgeCacheStats
#include <stddef.h>
#include <stdint.h>

class DetWebServer;

/**
 * @brief Enable the edge cache on @p server: register the cache middleware + the async-fetch poll hook,
 *        bind the origin transport to det_client, and clear the L1 store. Call once.
 */
void det_edge_cache_enable(DetWebServer &server);

/**
 * @brief Map a request path prefix to an upstream origin (e.g. "/cdn/" -> "http://origin.local").
 *
 * A cacheable GET/HEAD under @p path_prefix is fetched from the origin host + the full request path.
 * v1 is plaintext-origin only (an `https://` base is rejected). @return false if the map table is full,
 * an argument overflows, or the origin URL is https / malformed.
 */
bool det_edge_cache_map(const char *path_prefix, const char *origin_base_url);

/** @brief Clear the L1 store and all route maps. */
void det_edge_cache_reset(void);

/** @brief Invalidate a single canonical key. @return true if an entry was purged. */
bool det_edge_cache_purge(const char *canonical_key);

/** @brief Invalidate every entry whose request path begins with @p prefix. @return the count purged. */
uint32_t det_edge_cache_purge_prefix(const char *path_prefix);

/** @brief Snapshot the cache counters. */
void det_edge_cache_stats(EdgeCacheStats *out);

#endif // DETWS_ENABLE_EDGE_CACHE

#endif // DETERMINISTICESPASYNCWEBSERVER_EDGE_CACHE_PROXY_H
