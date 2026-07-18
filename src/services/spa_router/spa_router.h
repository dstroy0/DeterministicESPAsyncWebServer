// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file spa_router.h
 * @brief Single-page-app micro-routing decision (DWS_ENABLE_SPA_ROUTER).
 *
 * A single-page web UI does its own client-side routing: the browser navigates to `/dashboard` or
 * `/devices/42`, but there is no such file on the device - the server must return the app shell
 * (`index.html`) and let the JavaScript router take over, while still serving real asset files
 * (`/app.js`, `/style.css`) and letting API calls (`/api/...`) fall through to their handlers. This is
 * that routing decision: given a request path, return whether to serve the file, serve the shell, or
 * pass through to the app.
 *
 * The rule: a path under a configured API prefix passes through; a path whose last segment has a file
 * extension (a dot) is a real asset request; anything else is a client route and gets the shell. Pure,
 * zero heap, no stdlib, host-testable; the caller wires the result into serve_static / the router.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SPA_ROUTER_H
#define DETERMINISTICESPASYNCWEBSERVER_SPA_ROUTER_H

#include "ServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DWS_ENABLE_SPA_ROUTER

/** @brief What to do with a request path. */
enum class DetwsSpaAction : uint8_t
{
    DWS_SPA_SERVE_FILE,  ///< a real asset (has a file extension): serve it statically.
    DWS_SPA_SERVE_SHELL, ///< a client route (extensionless): serve the SPA shell (index.html).
    DWS_SPA_PASSTHROUGH  ///< under the API prefix: let the app's handlers run.
};

/** @brief True if the last path segment has a file extension (a '.' after the last '/'). */
bool dws_spa_has_extension(const char *path);

/**
 * @brief Decide how to route @p path for a single-page app.
 * @param path       the request path (e.g. "/devices/42", "/app.js", "/api/state").
 * @param api_prefix a prefix whose paths pass through to handlers (e.g. "/api/"); null/empty = none.
 * @return the routing action.
 *
 * "/" (or empty) serves the shell. A path starting with @p api_prefix passes through. A path whose last
 * segment has an extension serves the file; otherwise the shell.
 */
DetwsSpaAction dws_spa_route(const char *path, const char *api_prefix);

#endif // DWS_ENABLE_SPA_ROUTER
#endif // DETERMINISTICESPASYNCWEBSERVER_SPA_ROUTER_H
