// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file DeterministicESPAsyncWebServer.h
 * @brief Layer 7 (Application) — public HTTP routing API.
 *
 * This is the only header most application code needs to include.
 * The full OSI include chain is pulled in automatically:
 * @code
 *   DeterministicESPAsyncWebServer.h
 *     └── network_drivers/presentation.h  (Layer 6 — HTTP parser types)
 *           └── network_drivers/transport.h (Layer 4 — TCP types)
 *     └── network_drivers/session.h       (Layer 5 — event queue)
 * @endcode
 *
 * **Determinism guarantees**
 * - All buffers are statically allocated; no heap usage.
 * - Every operation has O(1) or O(MAX_ROUTES) worst-case time.
 * - `handle()` is safe to call every Arduino `loop()` iteration.
 *
 * @author   Douglas Quigg (dstroy0)
 * @date     2026
 * @copyright Copyright (C) 2026 Douglas Quigg (dstroy0). AGPL-3.0-or-later.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_H
#define DETERMINISTICESPASYNCWEBSERVER_H

#include <Arduino.h>
#include "network_drivers/presentation.h"
#include "network_drivers/session.h"

/** @brief Maximum number of simultaneously registered routes. */
#define MAX_ROUTES 16

// ---------------------------------------------------------------------------
// HTTP method enumeration
// ---------------------------------------------------------------------------

/**
 * @brief HTTP request methods supported by the router.
 *
 * Pass one of these values to DetWebServer::on() to bind a route to a
 * specific method.  PATCH, HEAD, and OPTIONS were added in v1.0 alongside
 * CORS preflight support.
 */
enum HttpMethod
{
    HTTP_GET,      ///< Safe, idempotent read
    HTTP_POST,     ///< Non-idempotent create / action
    HTTP_PUT,      ///< Idempotent replace
    HTTP_DELETE,   ///< Idempotent delete
    HTTP_PATCH,    ///< Partial update
    HTTP_HEAD,     ///< Same as GET but no response body
    HTTP_OPTIONS   ///< Capability query / CORS preflight
};

// ---------------------------------------------------------------------------
// Handler and route types
// ---------------------------------------------------------------------------

/**
 * @brief Callback signature for HTTP request handlers.
 *
 * The callback receives the connection slot index and a pointer to the
 * fully-parsed request.  Call DetWebServer::send() or DetWebServer::send_empty()
 * from inside the callback to write a response.
 *
 * @param slot_id  Index into the connection pool (0 … MAX_CONNS-1).
 * @param request  Pointer to the parsed HTTP request.  Valid only during the
 *                 callback; do not cache this pointer.
 *
 * @note If the callback returns without calling send(), the framework will
 *       reset the slot automatically (no response is sent to the client).
 */
typedef void (*Handler)(uint8_t slot_id, HttpReq *request);

/**
 * @brief Internal route entry stored in the routing table.
 *
 * Populated by DetWebServer::on().  Application code does not interact
 * with this struct directly.
 */
struct Route
{
    char       path[MAX_PATH_LEN]; ///< Null-terminated path pattern.
    HttpMethod method;             ///< HTTP method this route responds to.
    Handler    callback;           ///< User-supplied handler function.
    bool       is_active;          ///< `false` for unused table slots.
    bool       is_wildcard;        ///< `true` when path ends with `*`.
};

// ---------------------------------------------------------------------------
// DetWebServer — the main application class
// ---------------------------------------------------------------------------

/**
 * @class DetWebServer
 * @brief Single-port HTTP server with deterministic, zero-allocation execution.
 *
 * ## Typical usage
 * @code
 * DetWebServer server;
 *
 * void handle_api(uint8_t slot_id, HttpReq *req) {
 *     server.send(slot_id, 200, "application/json", "{\"ok\":true}");
 * }
 *
 * void setup() {
 *     WiFi.begin("SSID", "PASSWORD");
 *     server.on("/api/*", HTTP_GET, handle_api);
 *     server.set_cors("*");
 *     server.begin(80);
 * }
 *
 * void loop() {
 *     server.handle();   // call every iteration — O(MAX_CONNS) per call
 * }
 * @endcode
 *
 * ## Design constraints
 * - Maximum simultaneous connections: `MAX_CONNS` (4).
 * - Maximum registered routes: `MAX_ROUTES` (16).
 * - Responses are sent synchronously and the TCP connection is closed
 *   immediately after every response (HTTP/1.0 close semantics).
 */
class DetWebServer
{
private:
    Route   _routes[MAX_ROUTES]; ///< Flat routing table; searched linearly.
    uint8_t _route_count;        ///< Number of active entries in _routes.

    Handler _not_found_handler;  ///< Called when no route matches; may be null.
    bool    _cors_enabled;       ///< True after a non-empty set_cors() call.

    /**
     * @brief Pre-built CORS header block injected into every response.
     *
     * Built once by set_cors() to avoid repeated snprintf at dispatch time.
     */
    char _cors_header_buf[192];

    /**
     * @brief Evaluate whether a route pattern matches a request path.
     *
     * Wildcard routes end with `*`; the `*` is replaced by a prefix match.
     * Exact routes use strcmp.
     *
     * @param route       Null-terminated route pattern.
     * @param is_wildcard True if route ends with `*`.
     * @param req_path    Null-terminated path from the parsed request.
     * @return True if the route matches the request path.
     */
    static bool path_matches(const char *route, bool is_wildcard,
                             const char *req_path);

    /**
     * @brief Look up and invoke the first matching route for the given slot.
     *
     * If CORS is enabled and the method is OPTIONS, the preflight is
     * short-circuited here with a 204 response.  If no route matches, the
     * not-found handler is invoked (or a default 404 is sent).
     *
     * @param slot_id Connection slot to dispatch.
     */
    void match_and_execute(uint8_t slot_id);

public:
    /**
     * @brief Construct a DetWebServer with an empty routing table.
     *
     * All route slots are marked inactive.  CORS is disabled.  The
     * not-found handler is null (falls back to built-in 404 response).
     */
    DetWebServer();

    /**
     * @brief Initialise all connection slots and open the TCP listener.
     *
     * Calls http_reset() on every slot before accepting connections so
     * the parser pool is always in a known state.
     *
     * @param port TCP port to listen on (typically 80).
     * @return True on success; false if the lwIP listen setup failed.
     */
    bool begin(uint16_t port);

    /**
     * @brief Register a route handler.
     *
     * Routes are matched in registration order (first match wins).
     * A trailing `*` in @p path enables prefix matching:
     * `"/api/*"` matches `"/api/users"`, `"/api/devices"`, etc.
     *
     * @param path     URL path pattern, e.g. `"/api/status"` or `"/files/*"`.
     *                 Must be ≤ `MAX_PATH_LEN - 1` characters.
     * @param method   HTTP method this route accepts.
     * @param callback Function called when this route is matched.
     *
     * @note Registering more than MAX_ROUTES routes silently drops extras.
     */
    void on(const char *path, HttpMethod method, Handler callback);

    /**
     * @brief Register a fallback handler for unmatched requests.
     *
     * Called instead of sending a built-in 404 when no route matches.
     * The callback may call send() to return a custom error page.
     *
     * @param callback Handler to invoke on a 404 condition.
     */
    void on_not_found(Handler callback);

    /**
     * @brief Enable CORS by pre-building the Access-Control headers.
     *
     * Once called, every response produced by send() and send_empty()
     * includes the CORS headers.  OPTIONS requests are intercepted and
     * answered with 204 automatically (preflight short-circuit).
     *
     * @param origin `Access-Control-Allow-Origin` value, e.g. `"*"` or
     *               `"https://example.com"`.  Pass `""` to disable CORS.
     */
    void set_cors(const char *origin);

    /**
     * @brief Drive the server — call every Arduino `loop()` iteration.
     *
     * Internally this:
     * 1. Calls `DeterministicAsyncTCP::check_timeouts()` to kill stale
     *    connections.
     * 2. Drains the event queue (connections, data, disconnects, errors).
     * 3. Scans all connection slots for `PARSE_COMPLETE` requests and
     *    dispatches them to the matching route handler.
     * 4. Auto-sends 400 for any slot stuck in `PARSE_ERROR`.
     */
    void handle();

    /**
     * @brief Send an HTTP response with a body and close the connection.
     *
     * Writes status line, Content-Type, Content-Length, optional CORS
     * headers, and the payload; then calls tcp_close (tcp_abort on failure).
     * Always calls http_reset() at the end to free the parser slot.
     *
     * @param slot_id      Connection slot index returned by the router.
     * @param code         HTTP status code (200, 404, 500, …).
     * @param content_type MIME type string, e.g. `"application/json"`.
     * @param payload      Null-terminated response body.
     *
     * @note If the underlying PCB has already been freed (e.g. by a
     *       concurrent timeout), this function is a no-op that just
     *       resets the slot.
     */
    void send(uint8_t slot_id, int code, const char *content_type,
              const char *payload);

    /**
     * @brief Send a headers-only HTTP response and close the connection.
     *
     * Equivalent to send() with an empty body and Content-Length: 0.
     * Useful for 204 No Content, 304 Not Modified, HEAD responses, and
     * CORS preflight replies.
     *
     * @param slot_id Connection slot index.
     * @param code    HTTP status code.
     */
    void send_empty(uint8_t slot_id, int code);
};

#endif
