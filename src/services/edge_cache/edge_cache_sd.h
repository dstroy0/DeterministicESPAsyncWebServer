// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file edge_cache_sd.h
 * @brief CDN edge-cache tier - L2 SD persistence (DETWS_ENABLE_EDGE_CACHE && DETWS_ENABLE_DBM).
 *
 * The persistent second tier behind the bounded L1 RAM store (edge_cache): an evicted L1 entry is
 * written back to a dbm key-value store on the WAL (services/dbm, SD-card backed on device, a RAM WalDev
 * for host tests), and an L1 miss is served by promoting the entry back from L2. Because the store is
 * log-structured on the WAL, the cached set survives a reboot (dbm rebuilds its index by replaying the
 * log on open).
 *
 * The L2 key is the entry's 32-byte SHA-256 digest (== DETWS_DBM_KEY_MAX), so no key is re-derived. The
 * value is a compact, versioned, little-endian serialization of the entry's response metadata + body.
 *
 * These are pure functions over a caller-owned dbm handle and a caller-owned scratch buffer (no
 * file-scope state); the proxy glue (edge_cache_proxy) owns the dbm and the buffer and installs the
 * write-back / promote wiring.
 *
 * Reboot note: the monotonic insert time is meaningless across a reboot (no wall clock), so a promoted
 * entry is always treated as stale by the caller and revalidated - which is why only entries carrying a
 * validator (ETag / Last-Modified) are spilled: they are exactly the ones a cheap 304 can refresh.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_EDGE_CACHE_SD_H
#define DETERMINISTICESPASYNCWEBSERVER_EDGE_CACHE_SD_H

#include "ServerConfig.h"

#if DETWS_ENABLE_EDGE_CACHE && DETWS_ENABLE_DBM

#include "services/dbm/dbm.h"
#include "services/edge_cache/edge_cache.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Worst-case serialized value size of one entry (every metadata string full + a max-size body).
 *
 * Size the scratch buffer with this. A serialized entry only fits in dbm when this (or the actual, often
 * smaller, size) is <= DETWS_DBM_VAL_MAX - otherwise the entry stays L1-only (::edge_sd_put returns false).
 */
static constexpr size_t EDGE_SD_VALUE_MAX = 1 /*version*/ + 2 /*status*/ + 2 /*body_len*/ + 7u * 2u /*str lengths*/
                                            + sizeof(EdgeEntry::key) + sizeof(EdgeEntry::content_type) +
                                            sizeof(EdgeEntry::etag) + sizeof(EdgeEntry::last_modified) +
                                            sizeof(EdgeEntry::content_encoding) + sizeof(EdgeEntry::vary_names) +
                                            sizeof(EdgeEntry::vary_vals) + DETWS_EDGE_BODY_MAX;

/**
 * @brief Serialize @p e's response metadata + body into @p out (little-endian, versioned).
 * @return the byte length written, or 0 if it would not fit @p cap. Freshness/age fields are intentionally
 *         not persisted (a promoted entry is force-revalidated, so they are always recomputed).
 */
size_t edge_sd_serialize(const EdgeEntry *e, uint8_t *out, size_t cap);

/**
 * @brief Rehydrate an entry from @p buf (as produced by ::edge_sd_serialize) into @p e.
 *
 * Fills only the content fields (key, digest, status, content-type, validators, encoding, Vary, body); the
 * caller owns @p e's LRU linkage, used flag, and freshness (which it sets to force a revalidation).
 * @return false on a short/corrupt/oversized buffer or a version mismatch (fails closed, no partial write
 *         of the body).
 */
bool edge_sd_deserialize(const uint8_t *buf, size_t len, EdgeEntry *e);

/**
 * @brief Write @p e back to the L2 store (keyed by its digest), using @p scratch to serialize.
 * @return true if it was spilled; false if @p e carries no validator, or its serialization does not fit
 *         @p scratch / DETWS_DBM_VAL_MAX, or the dbm write fails (the entry simply stays L1-only).
 */
bool edge_sd_put(DetwsDbm *db, const EdgeEntry *e, uint8_t *scratch, size_t scratch_cap);

/**
 * @brief Promote the entry stored under @p digest from L2 into @p e (via @p scratch).
 * @return true on a hit that deserialized cleanly; false on an L2 miss or a corrupt value.
 */
bool edge_sd_get(DetwsDbm *db, const uint8_t digest[32], EdgeEntry *e, uint8_t *scratch, size_t scratch_cap);

/** @brief Drop the L2 entry stored under @p digest. @return true if one existed. */
bool edge_sd_del(DetwsDbm *db, const uint8_t digest[32]);

/**
 * @brief Drop every L2 entry whose stored request path begins with @p prefix (via @p scratch to read each
 *        value's canonical key). @return the number purged.
 */
uint32_t edge_sd_purge_prefix(DetwsDbm *db, const char *path_prefix, uint8_t *scratch, size_t scratch_cap);

/** @brief Drop every L2 entry. @return the number purged. */
uint32_t edge_sd_purge_all(DetwsDbm *db);

#endif // DETWS_ENABLE_EDGE_CACHE && DETWS_ENABLE_DBM

#endif // DETERMINISTICESPASYNCWEBSERVER_EDGE_CACHE_SD_H
