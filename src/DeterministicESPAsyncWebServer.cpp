// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file DeterministicESPAsyncWebServer.cpp
 * @brief Layer 7 (Application) — HTTP routing and request handler implementation.
 *
 * **Dispatch pipeline (called from DetWebServer::handle())**
 * ```
 * handle()
 *   └─ server_tick()                 ← drain FreeRTOS event queue
 *   └─ for each slot:
 *        PARSE_COMPLETE          → match_and_execute()
 *        PARSE_ERROR             → send(400)
 *        PARSE_ENTITY_TOO_LARGE  → send(413)
 *        PARSE_URI_TOO_LONG      → send(414)
 * ```
 *
 * **Route table**
 * Routes are stored in a fixed-size array of `Route` structs.  Both exact
 * and wildcard (suffix `*`) routes are supported; exact routes always take
 * priority because the loop checks them in insertion order and returns on
 * the first match.
 *
 * **PCB lifecycle in send() / send_empty()**
 * Before writing to the PCB the slot is set to `CONN_FREE` and the pcb
 * pointer is nulled.  This mirrors the pattern in transport.cpp:
 *   1. Save a local copy of the pcb pointer.
 *   2. Detach our slot from it (`tcp_arg(pcb, nullptr)`).
 *   3. Null out `conn->pcb` and set `conn->state = CONN_FREE`.
 *   4. Do the write + close/abort on the saved local pointer.
 *
 * This means any lwIP error callback that fires mid-write sees the slot as
 * already free and takes no action — preventing a double-free scenario.
 */

#include "DeterministicESPAsyncWebServer.h"

/**
 * @brief Convert an HTTP status code to its standard reason phrase.
 *
 * Covers the 18 codes that arise in typical REST micro-server usage.
 * Unknown codes produce "Unknown" so callers never receive a null pointer.
 *
 * @param code HTTP status integer.
 * @return Pointer to a string-literal reason phrase; never null.
 */
static const char *status_text(int code)
{
    switch (code)
    {
    case 200:
        return "OK";
    case 201:
        return "Created";
    case 204:
        return "No Content";
    case 301:
        return "Moved Permanently";
    case 302:
        return "Found";
    case 304:
        return "Not Modified";
    case 400:
        return "Bad Request";
    case 401:
        return "Unauthorized";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 408:
        return "Request Timeout";
    case 409:
        return "Conflict";
    case 413:
        return "Payload Too Large";
    case 414:
        return "URI Too Long";
    case 429:
        return "Too Many Requests";
    case 500:
        return "Internal Server Error";
    case 501:
        return "Not Implemented";
    case 503:
        return "Service Unavailable";
    default:
        return "Unknown";
    }
}

/**
 * @brief Map a method string (from the parsed request line) to an HttpMethod enum.
 *
 * Falls through to HTTP_GET for any unrecognised method.  This keeps the
 * no-match path simple: the route table will fail to find an exact method
 * match and will return 404 (or invoke the not-found handler).
 *
 * @param m Null-terminated method string, e.g. "POST".
 * @return Matching HttpMethod enum value.
 */
static HttpMethod parse_method(const char *m)
{
    if (strcmp(m, "POST") == 0)
        return HTTP_POST;
    if (strcmp(m, "PUT") == 0)
        return HTTP_PUT;
    if (strcmp(m, "DELETE") == 0)
        return HTTP_DELETE;
    if (strcmp(m, "PATCH") == 0)
        return HTTP_PATCH;
    if (strcmp(m, "HEAD") == 0)
        return HTTP_HEAD;
    if (strcmp(m, "OPTIONS") == 0)
        return HTTP_OPTIONS;
    return HTTP_GET;
}

DetWebServer::DetWebServer() : _route_count(0), _not_found_handler(nullptr), _cors_enabled(false)
{
    for (int i = 0; i < MAX_ROUTES; i++)
        _routes[i].is_active = false;
    _cors_header_buf[0] = '\0';
}

int32_t DetWebServer::begin(uint16_t port, const WebServerConfig *cfg)
{
    for (uint8_t i = 0; i < MAX_CONNS; i++)
        http_reset(i);
    return DeterministicAsyncTCP::init(port, cfg);
}

void DetWebServer::stop()
{
    DeterministicAsyncTCP::stop();
    for (uint8_t i = 0; i < MAX_CONNS; i++)
        http_reset(i);
}

/**
 * @brief Register a route in the route table.
 *
 * Paths are stored null-terminated and truncated to MAX_PATH_LEN.  The
 * trailing character of the stored path is inspected to detect wildcard
 * routes: any path ending in `*` is treated as a prefix match.
 *
 * Registrations beyond MAX_ROUTES are silently ignored — callers should
 * verify return values if overflow is a concern.
 *
 * @param path     URL path to match, e.g. "/api/*".
 * @param method   HTTP method that triggers this route.
 * @param callback Handler invoked with (slot_id, request).
 */
void DetWebServer::on(const char *path, HttpMethod method, Handler callback)
{
    if (_route_count >= MAX_ROUTES)
        return;

    Route *r = &_routes[_route_count];
    strncpy(r->path, path, MAX_PATH_LEN - 1);
    r->path[MAX_PATH_LEN - 1] = '\0';
    r->method = method;
    r->callback = callback;
    r->is_active = true;
    // A trailing '*' means prefix match
    size_t len = strlen(r->path);
    r->is_wildcard = (len > 0 && r->path[len - 1] == '*');
    _route_count++;
}

void DetWebServer::on_not_found(Handler callback)
{
    _not_found_handler = callback;
}

/**
 * @brief Enable CORS and pre-build the Access-Control response header block.
 *
 * The header string is constructed once here rather than at response time to
 * avoid repeated snprintf calls on the hot path.  It is stored in
 * `_cors_header_buf[]` and injected verbatim into every response when
 * `_cors_enabled` is true.
 *
 * Passing an empty or null origin disables CORS without clearing the buffer —
 * only the `_cors_enabled` flag matters at dispatch time.
 *
 * @param origin Value for the Access-Control-Allow-Origin header, e.g. "*".
 */
void DetWebServer::set_cors(const char *origin)
{
    if (!origin || origin[0] == '\0')
    {
        _cors_enabled = false;
        _cors_header_buf[0] = '\0';
        return;
    }
    snprintf(_cors_header_buf, sizeof(_cors_header_buf),
             "Access-Control-Allow-Origin: %s\r\n"
             "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, PATCH, HEAD, OPTIONS\r\n"
             "Access-Control-Allow-Headers: Content-Type\r\n",
             origin);
    _cors_enabled = true;
}

/**
 * @brief Test whether a route path matches an incoming request path.
 *
 * Exact routes use strcmp (full-string equality).  Wildcard routes use
 * strncmp against the prefix up to (but not including) the trailing `*`.
 *
 * @param route       Registered route path, potentially ending in `*`.
 * @param is_wildcard True when the route was registered with a trailing `*`.
 * @param req_path    Incoming request path from the parsed HTTP request line.
 * @return True if the route matches the request path.
 */
bool DetWebServer::path_matches(const char *route, bool is_wildcard, const char *req_path)
{
    if (!is_wildcard)
        return strcmp(route, req_path) == 0;

    // Prefix match: compare everything up to (but not including) the '*'
    size_t prefix_len = strlen(route) - 1;
    return strncmp(route, req_path, prefix_len) == 0;
}

/**
 * @brief Main application tick — tick the session layer then dispatch completed requests.
 *
 * Call this repeatedly from loop().  Each call:
 *   1. Calls server_tick() which runs timeout sweeps + drains the event queue.
 *   2. Walks all slots; any in PARSE_COMPLETE is dispatched via match_and_execute().
 *   3. Any slot left in PARSE_COMPLETE after dispatch (i.e., callback did not
 *      send a response) is reset so it doesn't block the slot.
 *   4. Any slot in PARSE_ERROR receives an automatic 400 response.
 *   5. Any slot in PARSE_ENTITY_TOO_LARGE receives an automatic 413 response.
 *   6. Any slot in PARSE_URI_TOO_LONG receives an automatic 414 response.
 */
void DetWebServer::handle()
{
    server_tick();

    for (uint8_t i = 0; i < MAX_CONNS; i++)
    {
        if (http_pool[i].parse_state == PARSE_COMPLETE)
        {
            match_and_execute(i);
            if (http_pool[i].parse_state == PARSE_COMPLETE)
                http_reset(i);
        }
        else if (http_pool[i].parse_state == PARSE_ERROR)
        {
            send(i, 400, "text/plain", "Bad Request");
        }
        else if (http_pool[i].parse_state == PARSE_ENTITY_TOO_LARGE)
        {
            send(i, 413, "text/plain", "Payload Too Large");
        }
        else if (http_pool[i].parse_state == PARSE_URI_TOO_LONG)
        {
            send(i, 414, "text/plain", "URI Too Long");
        }
    }
}

/**
 * @brief Walk the route table and invoke the first matching handler.
 *
 * OPTIONS requests are intercepted before the route scan when CORS is enabled
 * so that preflight responses are returned without needing an explicit OPTIONS
 * route registered by the application.
 *
 * Route matching is first-match: insertion order determines priority.  An
 * exact route registered before a wildcard will therefore shadow the wildcard
 * for that specific path.
 *
 * @param slot_id Index into http_pool[] / conn_pool[].
 */
void DetWebServer::match_and_execute(uint8_t slot_id)
{
    HttpReq *req = &http_pool[slot_id];
    HttpMethod method = parse_method(req->method);

    // OPTIONS request: CORS preflight — respond immediately if CORS is enabled
    if (method == HTTP_OPTIONS && _cors_enabled)
    {
        send_empty(slot_id, 204);
        return;
    }

    // RFC 7230 §3.3.1: reject Transfer-Encoding — chunked decoding is not supported
    if (http_get_header(req, "Transfer-Encoding") != nullptr)
    {
        send(slot_id, 501, "text/plain", "Not Implemented");
        return;
    }

    for (uint8_t i = 0; i < _route_count; i++)
    {
        Route *r = &_routes[i];
        if (!r->is_active)
            continue;
        if (r->method != method)
            continue;
        if (!path_matches(r->path, r->is_wildcard, req->path))
            continue;

        r->callback(slot_id, req);
        return;
    }

    // No route matched
    if (_not_found_handler)
        _not_found_handler(slot_id, req);
    else
        send(slot_id, 404, "text/plain", "Not Found");
}

/**
 * @brief Build and transmit an HTTP response with a body.
 *
 * Uses a 512-byte stack buffer for headers.  CORS headers are appended when
 * `_cors_enabled`.  The slot is freed (state → CONN_FREE, pcb → nullptr)
 * *before* the tcp_write + tcp_close sequence to ensure any error callback
 * that lwIP fires during the write sees the slot as already released.
 *
 * If the slot's connection is not active (e.g., already timed-out or the
 * PCB is null) the slot is reset and the function returns without writing.
 *
 * @param slot_id      Connection slot index.
 * @param code         HTTP status code, e.g. 200.
 * @param content_type MIME type string, e.g. "application/json".
 * @param payload      Null-terminated body string to send.
 */
void DetWebServer::send(uint8_t slot_id, int code, const char *content_type, const char *payload)
{
    TcpConn *conn = &conn_pool[slot_id];
    if (conn->state != CONN_ACTIVE || conn->pcb == nullptr)
    {
        http_reset(slot_id);
        return;
    }

    int payload_len = (int)strlen(payload);

    char header[512];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 %d %s\r\n"
                        "Content-Type: %s\r\n"
                        "Content-Length: %d\r\n"
                        "%s"
                        "Connection: close\r\n\r\n",
                        code, status_text(code), content_type, payload_len, _cors_enabled ? _cors_header_buf : "");

    struct tcp_pcb *pcb = conn->pcb;
    /*
     * Detach and free the slot before writing.  Any lwIP error callback
     * firing during tcp_write will see CONN_FREE and take no action.
     */
    tcp_arg(pcb, nullptr);
    conn->state = CONN_FREE;
    conn->pcb = nullptr;

    tcp_write(pcb, header, (u16_t)hlen, TCP_WRITE_FLAG_COPY);
    tcp_write(pcb, payload, (u16_t)payload_len, TCP_WRITE_FLAG_COPY);
    tcp_output(pcb);

    if (tcp_close(pcb) != ERR_OK)
        tcp_abort(pcb);

    http_reset(slot_id);
}

/**
 * @brief Build and transmit an HTTP response with no body.
 *
 * Used for CORS preflight (204) and any response where only status headers
 * are needed.  Behaves identically to send() regarding slot lifecycle and
 * PCB ownership transfer — the slot is freed before the lwIP write call.
 *
 * @param slot_id Connection slot index.
 * @param code    HTTP status code, e.g. 204.
 */
void DetWebServer::send_empty(uint8_t slot_id, int code)
{
    TcpConn *conn = &conn_pool[slot_id];
    if (conn->state != CONN_ACTIVE || conn->pcb == nullptr)
    {
        http_reset(slot_id);
        return;
    }

    char header[512];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 %d %s\r\n"
                        "Content-Length: 0\r\n"
                        "%s"
                        "Connection: close\r\n\r\n",
                        code, status_text(code), _cors_enabled ? _cors_header_buf : "");

    struct tcp_pcb *pcb = conn->pcb;
    tcp_arg(pcb, nullptr);
    conn->state = CONN_FREE;
    conn->pcb = nullptr;

    tcp_write(pcb, header, (u16_t)hlen, TCP_WRITE_FLAG_COPY);
    tcp_output(pcb);

    if (tcp_close(pcb) != ERR_OK)
        tcp_abort(pcb);

    http_reset(slot_id);
}
