// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file http_delivery.h
 * @brief HTTP delivery optimizations: stale-while-revalidate, Range/206 delta fetch, SW precache
 *        (DWS_ENABLE_HTTP_DELIVERY).
 *
 * Three pure cores that make HTTP serving cheaper on a constrained device, each mapping to a real web
 * standard:
 *
 *  - **Stale-while-revalidate** (RFC 5861): given a cached response's age and its `max-age` +
 *    `stale-while-revalidate` windows, decide FRESH / serve-stale-and-revalidate / EXPIRED, and build the
 *    matching `Cache-Control` header so a browser keeps the UI responsive while the device refreshes in
 *    the background.
 *  - **Delta / offset log fetch** (RFC 7233 byte ranges): parse a `Range: bytes=...` request against a
 *    resource of known length (all three forms - `X-Y`, `X-`, `-N`), and build the `Content-Range` header
 *    for the `206 Partial Content` reply, so a client streams only the new tail of a growing log.
 *  - **Service-worker precache manifest**: emit the versioned `{"version":..,"precache":[..]}` JSON a
 *    generated service worker consumes to cache-inject the app shell for offline / instant loads.
 *
 * Pure, zero heap, no stdlib (hand-rolled decimal parse/format), host-testable.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_HTTP_DELIVERY_H
#define DETERMINISTICESPASYNCWEBSERVER_HTTP_DELIVERY_H

#include "ServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DWS_ENABLE_HTTP_DELIVERY

/** @brief Freshness verdict for a cached response. */
/** @brief Cache-freshness verdict (the sole return of dws_delivery_swr). */
enum class DeliveryVerdict : uint8_t
{
    DELIVERY_FRESH = 0,            ///< age <= max-age: serve from cache, no revalidation.
    DELIVERY_STALE_REVALIDATE = 1, ///< within the stale-while-revalidate window: serve stale, refresh in bg.
    DELIVERY_EXPIRED = 2           ///< past both windows: must revalidate before serving.
};

/**
 * @brief RFC 5861 freshness decision.
 * @param age_s     seconds since the response was generated.
 * @param max_age_s the `max-age` window.
 * @param swr_s     the `stale-while-revalidate` window past max-age.
 * @return DELIVERY_FRESH / DELIVERY_STALE_REVALIDATE / DELIVERY_EXPIRED.
 */
DeliveryVerdict dws_delivery_swr(uint32_t age_s, uint32_t max_age_s, uint32_t swr_s);

/**
 * @brief Build a `Cache-Control` value: `public, max-age=N[, stale-while-revalidate=M]`.
 *        The swr directive is omitted when @p swr_s is 0.
 * @return length written (excl NUL), or 0 on overflow / bad args.
 */
size_t dws_delivery_cache_control(uint32_t max_age_s, uint32_t swr_s, char *out, size_t cap);

/**
 * @brief Parse a single-range `Range: bytes=...` header against a resource of @p total bytes.
 *
 * Handles `bytes=X-Y` (clamped), `bytes=X-` (X to end), and `bytes=-N` (last N). Multi-range requests
 * (a comma) are treated as unsupported. On success @p start / @p end are the inclusive byte offsets.
 * @return 1 if a satisfiable range was parsed; 0 otherwise (caller should send the full 200 or a 416).
 */
int dws_delivery_range(const char *range_header, uint32_t total, uint32_t *start, uint32_t *end);

/**
 * @brief Build the `Content-Range` value for a 206 reply: `bytes START-END/TOTAL`.
 * @return length written (excl NUL), or 0 on overflow / bad args.
 */
size_t dws_delivery_content_range(uint32_t start, uint32_t end, uint32_t total, char *out, size_t cap);

/**
 * @brief Emit the service-worker precache manifest: `{"version":"..","precache":["/a","/b",...]}`.
 * @param paths   asset paths to precache (borrowed).
 * @param n       number of paths.
 * @param version cache version tag (busts the SW cache on change).
 * @return length written (excl NUL), or 0 on overflow / bad args. Strings are JSON-escaped.
 */
size_t dws_delivery_sw_manifest(const char *const *paths, size_t n, const char *version, char *out, size_t cap);

#endif // DWS_ENABLE_HTTP_DELIVERY
#endif // DETERMINISTICESPASYNCWEBSERVER_HTTP_DELIVERY_H
