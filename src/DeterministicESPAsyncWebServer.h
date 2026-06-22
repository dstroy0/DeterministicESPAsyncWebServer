// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file DeterministicESPAsyncWebServer.h
 * @brief Layer 7 (Application) - public HTTP routing API.
 *
 * This is the only header most application code needs to include.
 * The full OSI include chain is pulled in automatically:
 * @code
 *   DeterministicESPAsyncWebServer.h                  (L7 Application)
 *     ├── network_drivers/presentation/presentation.h (L6 Presentation)
 *     │       ├── network_drivers/presentation/http_parser.h (parser types)
 *     │       └── network_drivers/transport/transport.h      (L4 Transport)
 *     │               └── DetWebServerConfig.h   (compile-time config)
 *     └── network_drivers/session/session.h      (L5 Session - event drain)
 * @endcode
 *
 * **Feature flags** - define any of these to 0 before including to strip
 * the feature from the build entirely:
 * @code
 *   #define DETWS_ENABLE_WEBSOCKET    0
 *   #define DETWS_ENABLE_SSE          0
 *   #define DETWS_ENABLE_MULTIPART    0
 *   #define DETWS_ENABLE_FILE_SERVING 0
 *   #define DETWS_ENABLE_AUTH         0
 *   #include <DeterministicESPAsyncWebServer.h>
 * @endcode
 *
 * **Determinism guarantees**
 * - All buffers are statically allocated; no heap usage after begin().
 * - Every operation has O(1) or O(MAX_ROUTES) worst-case time.
 * - `handle()` is safe to call every Arduino `loop()` iteration.
 *
 * @author   Douglas Quigg (dstroy0)
 * @date     2026
 * @copyright Copyright (C) 2026 Douglas Quigg (dstroy0). AGPL-3.0-or-later.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_H
#define DETERMINISTICESPASYNCWEBSERVER_H

#include "network_drivers/presentation/presentation.h"
#include "network_drivers/session/session.h"
#if DETWS_ENABLE_WEBSOCKET
#include "network_drivers/presentation/websocket.h"
#endif
#if DETWS_ENABLE_SSE
#include "network_drivers/presentation/sse.h"
#endif
#if DETWS_ENABLE_MULTIPART
#include "network_drivers/presentation/multipart.h"
#endif
#include <Arduino.h>
#if DETWS_ENABLE_FILE_SERVING
#ifdef ARDUINO
#include <FS.h>
#else
#include "FS.h"
#endif
#endif

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
    HTTP_GET,           ///< Safe, idempotent read
    HTTP_POST,          ///< Non-idempotent create / action
    HTTP_PUT,           ///< Idempotent replace
    HTTP_DELETE,        ///< Idempotent delete
    HTTP_PATCH,         ///< Partial update
    HTTP_HEAD,          ///< Same as GET but no response body
    HTTP_OPTIONS,       ///< Capability query / CORS preflight
    HTTP_METHOD_UNKNOWN ///< Unrecognized method token → 501 Not Implemented
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

#if DETWS_ENABLE_WEBSOCKET
/**
 * @brief Callback fired when a WebSocket connection is established.
 *
 * @param ws_id  Index into ws_pool[] for this connection.
 */
typedef void (*WsConnectHandler)(uint8_t ws_id);

/**
 * @brief Callback fired when a WebSocket text or binary frame arrives.
 *
 * The payload is in ws_pool[ws_id].buf, null-terminated.  Length is in
 * ws_pool[ws_id].payload_len.  Opcode is in ws_pool[ws_id].opcode.
 *
 * @param ws_id  Index into ws_pool[].
 */
typedef void (*WsMessageHandler)(uint8_t ws_id);

/**
 * @brief Callback fired when a WebSocket connection closes.
 *
 * @param ws_id  Index into ws_pool[] (slot is still valid during callback).
 */
typedef void (*WsCloseHandler)(uint8_t ws_id);
#endif // DETWS_ENABLE_WEBSOCKET

#if DETWS_ENABLE_SSE
/**
 * @brief Callback fired when a new SSE client connects.
 *
 * Use sse_send() inside this callback to push an initial event if needed.
 *
 * @param sse_id  Index into sse_pool[] for this connection.
 */
typedef void (*SseConnectHandler)(uint8_t sse_id);
#endif // DETWS_ENABLE_SSE

// ---------------------------------------------------------------------------
// Route type discriminator
// ---------------------------------------------------------------------------

/** @brief Discriminates between HTTP, WebSocket, and SSE route entries. */
enum RouteType
{
    ROUTE_HTTP, ///< Standard HTTP request/response.
#if DETWS_ENABLE_WEBSOCKET
    ROUTE_WS, ///< WebSocket upgrade route.
#endif
#if DETWS_ENABLE_SSE
    ROUTE_SSE ///< Server-Sent Events route.
#endif
};

/**
 * @brief Internal route entry stored in the routing table.
 *
 * Populated by DetWebServer::on(), on_ws(), or on_sse().
 * Application code does not interact with this struct directly.
 */
struct Route
{
    char path[MAX_PATH_LEN]; ///< Null-terminated path pattern.
    RouteType type;          ///< HTTP, WS, or SSE.
    HttpMethod method;       ///< HTTP method (ROUTE_HTTP only).
    Handler callback;        ///< HTTP handler (ROUTE_HTTP only).

#if DETWS_ENABLE_WEBSOCKET
    WsConnectHandler ws_connect; ///< Fired on upgrade success.
    WsMessageHandler ws_message; ///< Fired on each data frame.
    WsCloseHandler ws_close;     ///< Fired on close.
#endif

#if DETWS_ENABLE_SSE
    SseConnectHandler sse_connect; ///< Fired when client subscribes.
#endif

#if DETWS_ENABLE_AUTH
    bool auth_required;            ///< True when this route requires Basic Auth.
    char auth_realm[MAX_AUTH_LEN]; ///< WWW-Authenticate realm string.
    char auth_user[MAX_AUTH_LEN];  ///< Required username.
    char auth_pass[MAX_AUTH_LEN];  ///< Required password.
#endif

    bool is_active;   ///< `false` for unused table slots.
    bool is_wildcard; ///< `true` when path ends with `*`.
};

// ---------------------------------------------------------------------------
// DetWebServer - the main application class
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
 *     int32_t result = server.begin(80);
 *     if (result < 0) { } // abs(result) == heap bytes needed
 * }
 *
 * void loop() {
 *     server.handle();   // call every iteration - O(MAX_CONNS) per call
 * }
 * @endcode
 *
 * ## Design constraints
 * - Maximum simultaneous connections: `MAX_CONNS` (default 4).
 * - Maximum registered routes: `MAX_ROUTES` (default 16).
 * - Responses are sent synchronously and the TCP connection is closed
 *   immediately after every response (HTTP/1.0 close semantics).
 */
class DetWebServer
{
  private:
    Route _routes[MAX_ROUTES]; ///< Flat routing table; searched linearly.
    uint8_t _route_count;      ///< Number of active entries in _routes.

    Handler _not_found_handler; ///< Called when no route matches; may be null.
    bool _cors_enabled;         ///< True after a non-empty set_cors() call.

    uint16_t _listen_ports[MAX_LISTENERS];   ///< Ports registered via listen() or begin(port).
    ConnProto _listen_protos[MAX_LISTENERS]; ///< Protocol for each registered listener.
    uint8_t _listener_count;                 ///< Number of registered listeners.

    /**
     * @brief Pre-built CORS header block injected into every response.
     *
     * Built once by set_cors() to avoid repeated snprintf at dispatch time.
     */
    char _cors_header_buf[CORS_HDR_BUF_SIZE];

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
    static bool path_matches(const char *route, bool is_wildcard, const char *req_path);

#if DETWS_ENABLE_AUTH
    /// @brief Validate the request's HTTP Basic credentials against route @p r. @return true if authorized.
    static bool check_basic_auth(uint8_t slot_id, HttpReq *req, const Route *r);
    /// @brief Send 401 Unauthorized with a `WWW-Authenticate: Basic realm="<realm>"` header.
    void send_unauth(uint8_t slot_id, const char *realm);
#endif

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
     * @brief Bytes of contiguous heap that begin() will allocate.
     *
     * The event queue is the library's only dynamic allocation.  Compare
     * this value against heap_caps_get_largest_free_block(MALLOC_CAP_8BIT)
     * to verify a suitable block exists before calling begin().
     *
     * @code
     *   if (!DetWebServer::heap_available()) {
     *       Serial.printf("need %u contiguous bytes, largest block is %u\n",
     *                     DetWebServer::heap_needed(),
     *                     heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
     *       return;
     *   }
     *   server.begin(80);
     * @endcode
     */
    static size_t heap_needed();

    /**
     * @brief True if the largest contiguous free heap block >= heap_needed().
     *
     * A false return means begin() will fail; check heap fragmentation or
     * reduce EVT_QUEUE_DEPTH.
     */
    static bool heap_available();

    /**
     * @brief Construct a DetWebServer with an empty routing table.
     *
     * All route slots are marked inactive.  CORS is disabled.  The
     * not-found handler is null (falls back to built-in 404 response).
     */
    DetWebServer();

    /**
     * @brief Register a port to listen on when begin() is called.
     *
     * Call this before begin() for each port you want the server to accept
     * connections on.  The @p proto argument tells the session layer which
     * protocol handler to invoke for events on this port.
     *
     * For the common single-HTTP-port case, prefer `begin(80)` which calls
     * this internally.  Use the explicit listen() + begin() form when you
     * need multiple ports (e.g., HTTP on 80 and Telnet on 23).
     *
     * @code
     * server.listen(80, PROTO_HTTP);
     * server.listen(23, PROTO_TELNET);
     * server.begin();
     * @endcode
     *
     * @param port  TCP port to open.
     * @param proto Application protocol; defaults to PROTO_HTTP.
     * @return Positive value on success; -1 if the listener pool is full.
     */
    int32_t listen(uint16_t port, ConnProto proto = PROTO_HTTP);

    /**
     * @brief Initialize all connection slots and open all registered listeners.
     *
     * Resets the HTTP parser pool, calls DeterministicAsyncTCP::pool_init(),
     * then calls listener_add() for each port registered via listen().
     * Requires at least one prior listen() call; returns -1 if no ports are
     * registered.  For the common single-port case use begin(port, cfg) instead.
     *
     * @param cfg  Optional runtime configuration.  Pass nullptr for defaults.
     * @return Positive value on success; -1 on failure.
     */
    int32_t begin(const WebServerConfig *cfg = nullptr);

    /**
     * @brief Convenience overload: register @p port as HTTP and start listening.
     *
     * Equivalent to `listen(port); begin(cfg);`.  Preserved for backward
     * compatibility with single-port sketches.
     *
     * @param port TCP port to listen on (typically 80).
     * @param cfg  Optional runtime configuration.  Pass nullptr for defaults.
     * @return Positive value on success; -1 on failure.
     */
    int32_t begin(uint16_t port, const WebServerConfig *cfg = nullptr);

    /**
     * @brief Gracefully stop the server.
     *
     * Aborts all active connections, closes the listener, frees the event
     * queue, and resets all HTTP parser slots.  The WiFi and TCP/IP stack
     * remain active.  Call begin() or restart() to bring the server back up.
     */
    void stop();

    /**
     * @brief Hard-reset all connections and re-open all registered listeners.
     *
     * Equivalent to stop() followed by begin(cfg) using the ports and protocols
     * registered via listen() (or the port passed to begin(port)).  The WiFi
     * and TCP/IP stack are not touched.
     *
     * Calling restart() before any listen() / begin(port) has no effect and
     * returns -1.
     *
     * @param cfg Optional new runtime configuration.  Pass nullptr to reuse
     *            the compile-time default (CONN_TIMEOUT_MS).
     */
    int32_t restart(const WebServerConfig *cfg = nullptr);

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

#if DETWS_ENABLE_AUTH
    /**
     * @brief Register a route handler protected by HTTP Basic Authentication.
     *
     * If the request does not include valid credentials, the library sends
     * `401 Unauthorized` with a `WWW-Authenticate: Basic realm="<realm>"`
     * header automatically; the callback is not invoked.
     *
     * @param path     URL path pattern.
     * @param method   HTTP method.
     * @param callback Handler invoked only on successful authentication.
     * @param realm    WWW-Authenticate realm displayed by the browser.
     * @param user     Required username.
     * @param pass     Required password.
     */
    void on(const char *path, HttpMethod method, Handler callback, const char *realm, const char *user,
            const char *pass);
#endif // DETWS_ENABLE_AUTH

#if DETWS_ENABLE_FILE_SERVING
    /**
     * @brief Serve a file from any Arduino-compatible filesystem.
     *
     * Opens @p fs_path on @p file_sys, sends HTTP 200 with the appropriate
     * headers (Content-Type, Content-Length), and streams the file body in
     * FILE_CHUNK_SIZE chunks via tcp_write().  Sends 404 if the file cannot
     * be opened.
     *
     * @param slot_id      Connection slot index.
     * @param file_sys     Filesystem reference (e.g. SPIFFS, LittleFS).
     * @param fs_path      Path to the file on the filesystem.
     * @param content_type MIME type string, e.g. "text/html".
     */
    void serve_file(uint8_t slot_id, fs::FS &file_sys, const char *fs_path, const char *content_type);
#endif // DETWS_ENABLE_FILE_SERVING

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
     * @brief Drive the server - call every Arduino `loop()` iteration.
     *
     * Internally this:
     * 1. Calls `DeterministicAsyncTCP::check_timeouts()` to kill stale
     *    connections.
     * 2. Drains the event queue (connections, data, disconnects, errors).
     * 3. Scans all connection slots for `PARSE_COMPLETE` requests and
     *    dispatches them to the matching route handler.
     * 4. Auto-sends 400 for any slot stuck in `PARSE_ERROR`.
     * 5. Auto-sends 413 for any slot stuck in `PARSE_ENTITY_TOO_LARGE`.
     * 6. Auto-sends 414 for any slot stuck in `PARSE_URI_TOO_LONG`.
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
    void send(uint8_t slot_id, int code, const char *content_type, const char *payload);

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

#if DETWS_ENABLE_DIAG
    /**
     * @brief Send the diagnostic JSON and close the connection.
     *
     * Responds with 200 application/json containing the compile-time feature
     * flags and all capacity constants.  Only available when
     * DETWS_ENABLE_DIAG is set to 1 - disable before deploying to production.
     *
     * @param slot_id Connection slot index.
     */
    void diag(uint8_t slot_id);
#endif

#if DETWS_ENABLE_WEBSOCKET
    // -----------------------------------------------------------------------
    // WebSocket API
    // -----------------------------------------------------------------------

    /**
     * @brief Register a WebSocket upgrade route.
     *
     * When a GET request arrives for @p path with `Upgrade: websocket`, the
     * library performs the RFC 6455 handshake automatically and fires
     * @p on_connect.  Subsequent frames fire @p on_message.  Closing the
     * connection fires @p on_close.
     *
     * Ping frames are answered with Pong automatically; no handler needed.
     *
     * @param path        URL path the client connects to, e.g. `"/ws"`.
     * @param on_connect  Fired once when the handshake completes.  May be nullptr.
     * @param on_message  Fired for each text or binary frame.  Must not be nullptr.
     * @param on_close    Fired when the connection closes.  May be nullptr.
     */
    void on_ws(const char *path, WsConnectHandler on_connect, WsMessageHandler on_message, WsCloseHandler on_close);

    /**
     * @brief Send a text frame to a WebSocket client.
     *
     * @param ws_id    Index into ws_pool[] (from the WsConnectHandler or WsMessageHandler).
     * @param text     Null-terminated UTF-8 string to send.
     */
    void ws_send_text(uint8_t ws_id, const char *text);

    /**
     * @brief Send a binary frame to a WebSocket client.
     *
     * @param ws_id    Index into ws_pool[].
     * @param data     Payload bytes.
     * @param len      Payload length in bytes; must be <= WS_FRAME_SIZE.
     */
    void ws_send_binary(uint8_t ws_id, const uint8_t *data, uint16_t len);

    /**
     * @brief Initiate a graceful WebSocket close.
     *
     * Sends a Close frame with WS_CLOSE_NORMAL and marks the slot WS_CLOSED.
     * The on_close handler fires on the next handle() call.
     *
     * @param ws_id  Index into ws_pool[].
     */
    void ws_disconnect(uint8_t ws_id);
#endif // DETWS_ENABLE_WEBSOCKET

#if DETWS_ENABLE_SSE
    // -----------------------------------------------------------------------
    // Server-Sent Events API
    // -----------------------------------------------------------------------

    /**
     * @brief Register a Server-Sent Events endpoint.
     *
     * When a GET request arrives for @p path, the library sends the SSE
     * headers and keeps the connection open.  @p on_connect fires so the
     * handler can push an initial event with sse_send().
     *
     * @param path        URL path, e.g. `"/events"`.
     * @param on_connect  Fired when a client subscribes.  May be nullptr.
     */
    void on_sse(const char *path, SseConnectHandler on_connect);

    /**
     * @brief Push an event to one SSE client.
     *
     * Formats and sends `event: ...\ndata: ...\nid: ...\n\n` to the client
     * on @p sse_id.  Any field may be nullptr to omit it from the output.
     * The data field is required; passing nullptr sends nothing.
     *
     * @param sse_id  Index into sse_pool[].
     * @param data    Event data string (required).
     * @param event   Optional event name (sets the `event:` field).
     * @param id      Optional event ID (sets the `id:` field).
     */
    void sse_send(uint8_t sse_id, const char *data, const char *event = nullptr, const char *id = nullptr);

    /**
     * @brief Push an event to all connected SSE clients on a given path.
     *
     * Iterates sse_pool[] and calls sse_send() for every active client
     * whose path matches @p path.
     *
     * @param path   SSE endpoint path, e.g. `"/events"`.
     * @param data   Event data string.
     * @param event  Optional event name.
     * @param id     Optional event ID.
     */
    void sse_broadcast(const char *path, const char *data, const char *event = nullptr, const char *id = nullptr);
#endif // DETWS_ENABLE_SSE
};

#endif
