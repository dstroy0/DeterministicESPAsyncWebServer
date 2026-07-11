// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dwserver_internal.h
 * @brief Library-private declarations shared between dwserver.cpp and the src/server/*.cpp
 *        request-handler translation units it is split into (WebDAV, file serving, ...).
 *
 * These are NOT part of the public API - they are the handful of dwserver.cpp file-scope helpers
 * that a split-out handler still needs, promoted from `static` to external linkage so the pieces
 * link together. Everything else a handler needs is either a public DetWebServer method (declared
 * in dwserver.h) or already an extern in the transport headers (e.g. conn_pool in tcp.h).
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SERVER_INTERNAL_H
#define DETERMINISTICESPASYNCWEBSERVER_SERVER_INTERNAL_H

#include "dwserver.h"
#include <time.h>

/** @brief Reason phrase for an HTTP status code (e.g. 404 -> "Not Found"). */
const char *status_text(int code);

/** @brief Initialize the common fields (path, flags) of a route-table entry from its pattern. */
void fill_route_base(Route *r, const char *path);

/** @brief Format @p t as an RFC 1123 GMT date into @p out (cap bytes); @p out is emptied for t <= 0. */
void http_rfc1123(time_t t, char *out, size_t cap);

#endif // DETERMINISTICESPASYNCWEBSERVER_SERVER_INTERNAL_H
