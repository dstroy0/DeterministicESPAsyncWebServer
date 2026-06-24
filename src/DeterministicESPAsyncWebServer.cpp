// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file DeterministicESPAsyncWebServer.cpp
 * @brief Layer 7 (Application) - HTTP routing and request handler implementation.
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
 * pointer is nulled.  All TCP I/O goes through the transport-layer connection
 * API (det_conn_send / det_conn_flush / det_conn_close / det_conn_detach), so
 * this layer never calls lwIP directly:
 *   1. Save a local copy of the pcb pointer.
 *   2. Detach our slot from it (`det_conn_detach(pcb)`).
 *   3. Null out `conn->pcb` and set `conn->state = CONN_FREE`.
 *   4. Do the send + flush + close on the saved local pointer.
 *
 * This means any lwIP error callback that fires mid-write sees the slot as
 * already free and takes no action - preventing a double-free scenario.
 */

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/tls/det_tls.h"
#include "network_drivers/transport/listener.h"
#if DETWS_ENABLE_WEBSOCKET
#include "network_drivers/presentation/base64.h"
#include "network_drivers/presentation/sha1.h"
#elif DETWS_ENABLE_AUTH
#include "network_drivers/presentation/base64.h"
#endif
#if DETWS_ENABLE_AUTH
#include "network_drivers/presentation/ssh/ssh_sha256.h"
#ifdef ARDUINO
#include <esp_system.h> // esp_random() for the Digest nonce CSPRNG
#endif
#endif
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#if DETWS_ENABLE_WEBSOCKET
// Magic GUID concatenated to the client key for the WS accept hash (RFC 6455 §4.2.2)
static const char WS_MAGIC[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
#endif

/**
 * @brief Convert an HTTP status code to its standard reason phrase.
 *
 * Covers the 18 codes that arise in typical REST micro-server usage.
 * Unknown codes produce "Unknown" so callers never receive a null pointer.
 *
 * @param code HTTP status integer.
 * @return Pointer to a string-literal reason phrase; never null.
 */
// Response bytes go out via the transport-layer connection I/O API
// (det_conn_send / det_conn_flush / det_conn_close, see transport.h) so this
// application layer never calls lwIP directly.

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
    case 303:
        return "See Other";
    case 304:
        return "Not Modified";
    case 307:
        return "Temporary Redirect";
    case 308:
        return "Permanent Redirect";
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
 * Returns HTTP_UNKNOWN for any method the server does not implement, so the
 * dispatcher can answer 501 Not Implemented (RFC 7231 §6.5.2) instead of
 * silently treating it as GET.
 *
 * @param m Null-terminated method string, e.g. "POST".
 * @return Matching HttpMethod enum value, or HTTP_UNKNOWN.
 */
static HttpMethod parse_method(const char *m)
{
    if (strcmp(m, "GET") == 0)
        return HTTP_GET;
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
    return HTTP_METHOD_UNKNOWN;
}

/**
 * @brief Canonical method token for an HttpMethod (for the Allow header).
 */
static const char *method_name(HttpMethod m)
{
    switch (m)
    {
    case HTTP_GET:
        return "GET";
    case HTTP_POST:
        return "POST";
    case HTTP_PUT:
        return "PUT";
    case HTTP_DELETE:
        return "DELETE";
    case HTTP_PATCH:
        return "PATCH";
    case HTTP_HEAD:
        return "HEAD";
    case HTTP_OPTIONS:
        return "OPTIONS";
    default:
        return "";
    }
}

size_t DetWebServer::heap_needed()
{
    return DeterministicAsyncTCP::heap_needed();
}
bool DetWebServer::heap_available()
{
    return DeterministicAsyncTCP::heap_available();
}

DetWebServer::DetWebServer()
    : _route_count(0), _not_found_handler(nullptr), _cors_enabled(false), _log_cb(nullptr), _listener_count(0),
      _middleware_count(0), _rl_max(0), _rl_window_ms(0), _rl_window_start(0), _rl_count(0)
{
    for (int i = 0; i < MAX_ROUTES; i++)
        _routes[i] = {};
    for (int i = 0; i < MAX_MIDDLEWARE; i++)
        _middleware[i] = nullptr;
    _cors_header_buf[0] = '\0';
    for (int i = 0; i < MAX_CONNS; i++)
        _extra_hdr[i][0] = '\0';
#if DETWS_ENABLE_AUTH
    regen_digest_nonce();
#endif
#if DETWS_ENABLE_STATS
    _stat_requests = _stat_2xx = _stat_4xx = _stat_5xx = 0;
#endif
}

void DetWebServer::on_request_log(RequestLogCb cb)
{
    _log_cb = cb;
}

// Record a completed response: bump stats counters and fire the access-log hook.
// The request's method/path are still intact in http_pool[slot_id] (http_reset
// has not run yet at the call sites).
void DetWebServer::note_response(uint8_t slot_id, int code, int body_len)
{
#if DETWS_ENABLE_STATS
    _stat_requests++;
    if (code >= 500)
        _stat_5xx++;
    else if (code >= 400)
        _stat_4xx++;
    else if (code >= 200 && code < 300)
        _stat_2xx++;
#endif
    if (_log_cb)
    {
        const HttpReq *r = &http_pool[slot_id];
        _log_cb(r->method, r->path, code, body_len);
    }
}

// ---------------------------------------------------------------------------
// Middleware chain + built-in rate limiter
// ---------------------------------------------------------------------------

void DetWebServer::use(Middleware mw)
{
    if (mw == nullptr || _middleware_count >= MAX_MIDDLEWARE)
        return;
    _middleware[_middleware_count++] = mw;
}

// Run the chain in registration order. The first middleware to return MW_HALT
// stops dispatch; it is responsible for having sent a response.
bool DetWebServer::run_middleware(uint8_t slot_id, HttpReq *req)
{
    for (uint8_t i = 0; i < _middleware_count; i++)
    {
        if (_middleware[i] && _middleware[i](slot_id, req) == MW_HALT)
            return true;
    }
    return false;
}

void DetWebServer::enable_rate_limit(uint16_t max_requests, uint32_t window_ms)
{
    _rl_max = max_requests;
    _rl_window_ms = window_ms;
    _rl_window_start = millis();
    _rl_count = 0;
}

// Fixed-window counter. Unsigned subtraction is rollover-safe across the millis()
// wrap. On the request that tips past _rl_max, reply 429 + Retry-After and stop.
bool DetWebServer::rate_limit_check(uint8_t slot_id)
{
    if (_rl_max == 0 || _rl_window_ms == 0)
        return false; // disabled

    uint32_t now = millis();
    if ((uint32_t)(now - _rl_window_start) >= _rl_window_ms)
    {
        _rl_window_start = now; // new window
        _rl_count = 0;
    }

    _rl_count++;
    if (_rl_count <= _rl_max)
        return false; // within budget

    // Over budget: advertise how long until the window resets, then 429.
    uint32_t elapsed = (uint32_t)(now - _rl_window_start);
    uint32_t remain_ms = (_rl_window_ms > elapsed) ? (_rl_window_ms - elapsed) : 0;
    char secs[12];
    snprintf(secs, sizeof(secs), "%lu", (unsigned long)((remain_ms + 999) / 1000));
    add_response_header(slot_id, "Retry-After", secs);
    send(slot_id, 429, "text/plain", "Too Many Requests");
    return true;
}

int32_t DetWebServer::listen(uint16_t port, ConnProto proto)
{
    if (_listener_count >= MAX_LISTENERS)
        return DETWS_ERR_LISTENER_FULL;
    _listen_ports[_listener_count] = port;
    _listen_protos[_listener_count] = proto;
    _listen_tls[_listener_count] = false;
    _listener_count++;
    return DETWS_OK;
}

int32_t DetWebServer::begin(const WebServerConfig *cfg)
{
    if (_listener_count == 0)
        return DETWS_ERR_NO_LISTENERS;
    DeterministicAsyncTCP::pool_init(cfg);
#if DETWS_ENABLE_AUTH
    regen_digest_nonce(); // fresh server nonce per begin()
#endif
    for (uint8_t i = 0; i < MAX_CONNS; i++)
        http_reset(i);
#if DETWS_ENABLE_WEBSOCKET
    ws_init();
#endif
#if DETWS_ENABLE_SSE
    sse_init();
#endif
    for (uint8_t i = 0; i < _listener_count; i++)
    {
        if (listener_add(i, _listen_ports[i], _listen_protos[i], _listen_tls[i]) < 0)
            return DETWS_ERR_LISTEN_FAILED;
    }
    return DETWS_OK;
}

int32_t DetWebServer::begin(uint16_t port, const WebServerConfig *cfg)
{
    int32_t rc = listen(port);
    if (rc < 0)
        return rc;
    return begin(cfg);
}

#if DETWS_ENABLE_TLS
bool DetWebServer::tls_cert(const uint8_t *cert, size_t cert_len, const uint8_t *key, size_t key_len)
{
    return det_tls_global_init(cert, cert_len, key, key_len);
}

int32_t DetWebServer::listen_tls(uint16_t port)
{
    if (_listener_count >= MAX_LISTENERS)
        return DETWS_ERR_LISTENER_FULL;
    _listen_ports[_listener_count] = port;
    _listen_protos[_listener_count] = PROTO_HTTP;
    _listen_tls[_listener_count] = true;
    _listener_count++;
    return DETWS_OK;
}

int32_t DetWebServer::begin_tls(uint16_t port, const uint8_t *cert, size_t cert_len, const uint8_t *key, size_t key_len,
                                const WebServerConfig *cfg)
{
    if (!tls_cert(cert, cert_len, key, key_len))
        return DETWS_ERR_LISTEN_FAILED;
    int32_t rc = listen_tls(port);
    if (rc < 0)
        return rc;
    return begin(cfg);
}
#endif // DETWS_ENABLE_TLS

int32_t DetWebServer::restart(const WebServerConfig *cfg)
{
    if (_listener_count == 0)
        return DETWS_ERR_NO_LISTENERS;
    stop();
    return begin(cfg);
}

void DetWebServer::stop()
{
    listener_stop_all();
    DeterministicAsyncTCP::stop();
    for (uint8_t i = 0; i < MAX_CONNS; i++)
        http_reset(i);
#if DETWS_ENABLE_WEBSOCKET
    ws_init();
#endif
#if DETWS_ENABLE_SSE
    sse_init();
#endif
}

/**
 * @brief Register a route in the route table.
 *
 * Paths are stored null-terminated and truncated to MAX_PATH_LEN.  The
 * trailing character of the stored path is inspected to detect wildcard
 * routes: any path ending in `*` is treated as a prefix match.
 *
 * Registrations beyond MAX_ROUTES are silently ignored - callers should
 * verify return values if overflow is a concern.
 *
 * @param path     URL path to match, e.g. "/api/*".
 * @param method   HTTP method that triggers this route.
 * @param callback Handler invoked with (slot_id, request).
 */
static void fill_route_base(Route *r, const char *path)
{
    strncpy(r->path, path, MAX_PATH_LEN - 1);
    r->path[MAX_PATH_LEN - 1] = '\0';
    r->is_active = true;
    size_t len = strlen(r->path);
    r->is_wildcard = (len > 0 && r->path[len - 1] == '*');
    r->is_param = (strstr(r->path, "/:") != nullptr);
    r->is_regex = false;
    r->iface_filter = DETIFACE_ANY;
}

void DetWebServer::on(const char *path, HttpMethod method, Handler callback)
{
    if (_route_count >= MAX_ROUTES)
        return;
    Route *r = &_routes[_route_count++];
    fill_route_base(r, path);
    r->type = ROUTE_HTTP;
    r->method = method;
    r->callback = callback;
}

void DetWebServer::on(const char *path, HttpMethod method, Handler callback, DetIface iface)
{
    if (_route_count >= MAX_ROUTES)
        return;
    Route *r = &_routes[_route_count++];
    fill_route_base(r, path);
    r->type = ROUTE_HTTP;
    r->method = method;
    r->callback = callback;
    r->iface_filter = (uint8_t)iface;
}

void DetWebServer::set_ap_ip(uint32_t ap_ip)
{
    detws_ap_ip = ap_ip;
}

void DetWebServer::on_regex(const char *pattern, HttpMethod method, Handler callback)
{
    if (_route_count >= MAX_ROUTES)
        return;
    Route *r = &_routes[_route_count++];
    fill_route_base(r, pattern);
    r->type = ROUTE_HTTP;
    r->method = method;
    r->callback = callback;
    r->is_regex = true;
}

#if DETWS_ENABLE_AUTH
void DetWebServer::on(const char *path, HttpMethod method, Handler callback, const char *realm, const char *user,
                      const char *pass, bool digest)
{
    if (_route_count >= MAX_ROUTES)
        return;
    Route *r = &_routes[_route_count++];
    fill_route_base(r, path);
    r->type = ROUTE_HTTP;
    r->method = method;
    r->callback = callback;
    r->auth_required = true;
    r->auth_digest = digest;
    strncpy(r->auth_realm, realm, MAX_AUTH_LEN - 1);
    r->auth_realm[MAX_AUTH_LEN - 1] = '\0';
    strncpy(r->auth_user, user, MAX_AUTH_LEN - 1);
    r->auth_user[MAX_AUTH_LEN - 1] = '\0';
    strncpy(r->auth_pass, pass, MAX_AUTH_LEN - 1);
    r->auth_pass[MAX_AUTH_LEN - 1] = '\0';
}
#endif // DETWS_ENABLE_AUTH

#if DETWS_ENABLE_WEBSOCKET
void DetWebServer::on_ws(const char *path, WsConnectHandler on_connect, WsMessageHandler on_message,
                         WsCloseHandler on_close)
{
    if (_route_count >= MAX_ROUTES)
        return;
    Route *r = &_routes[_route_count++];
    fill_route_base(r, path);
    r->type = ROUTE_WS;
    r->ws_connect = on_connect;
    r->ws_message = on_message;
    r->ws_close = on_close;
}
#endif // DETWS_ENABLE_WEBSOCKET

#if DETWS_ENABLE_SSE
void DetWebServer::on_sse(const char *path, SseConnectHandler on_connect)
{
    if (_route_count >= MAX_ROUTES)
        return;
    Route *r = &_routes[_route_count++];
    fill_route_base(r, path);
    r->type = ROUTE_SSE;
    r->sse_connect = on_connect;
}
#endif // DETWS_ENABLE_SSE

void DetWebServer::on_not_found(Handler callback)
{
    _not_found_handler = callback;
}

/*
 * Enable CORS and pre-build the Access-Control response header block.
 *
 * The header string is constructed once here rather than at response time to
 * avoid repeated snprintf calls on the hot path.  It is stored in
 * `_cors_header_buf[]` and injected verbatim into every response when
 * `_cors_enabled` is true.
 *
 * Passing an empty or null origin disables CORS without clearing the buffer -
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
    snprintf(_cors_header_buf, CORS_HDR_BUF_SIZE,
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

// ---------------------------------------------------------------------------
// Bounded regex route matcher (see on_regex()).
//
// A small recursive backtracker over a single pattern (no heap, no groups, no
// alternation). Supported: literals, '.', quantifiers '*' '+' '?', character
// classes [..]/[^..] with a-z ranges, and '\' escapes incl. \d \w \s (\D \W \S).
// A step counter bounds total work so a pathological pattern fails closed
// (no match) instead of backtracking unboundedly - preserving determinism.
// ---------------------------------------------------------------------------

struct ReCtx
{
    uint32_t steps;
    uint32_t max_steps;
};

// Byte length of the atom at p: an escape (\x), a class ([...]), or one char.
static size_t re_atom_len(const char *p)
{
    if (*p == '\\')
        return p[1] ? 2 : 1;
    if (*p == '[')
    {
        const char *q = p + 1;
        if (*q == '^')
            q++;
        if (*q == ']') // a ']' right after '[' (or '[^') is a literal member
            q++;
        while (*q && *q != ']')
        {
            if (*q == '\\' && q[1])
                q += 2;
            else
                q++;
        }
        return (size_t)((*q == ']' ? q + 1 : q) - p);
    }
    return 1;
}

static bool re_class_member(char lo, char hi, char ch)
{
    return ch >= lo && ch <= hi;
}

// Does the atom [p, p+len) match the single character ch (ch != '\0')?
static bool re_atom_matches(const char *p, size_t len, char ch)
{
    if (ch == '\0')
        return false;
    if (*p == '\\')
    {
        char e = p[1];
        switch (e)
        {
        case 'd':
            return ch >= '0' && ch <= '9';
        case 'D':
            return !(ch >= '0' && ch <= '9');
        case 'w':
            return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_';
        case 'W':
            return !((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_');
        case 's':
            return ch == ' ' || ch == '\t';
        case 'S':
            return !(ch == ' ' || ch == '\t');
        default:
            return ch == e; // escaped literal (\. \* \\ ...)
        }
    }
    if (*p == '.')
        return true;
    if (*p == '[')
    {
        const char *q = p + 1;
        const char *end = p + len - 1; // points at the closing ']'
        bool neg = false;
        if (q < end && *q == '^')
        {
            neg = true;
            q++;
        }
        bool m = false;
        while (q < end)
        {
            char lo;
            if (*q == '\\' && (q + 1) < end)
            {
                lo = q[1];
                q += 2;
            }
            else
            {
                lo = *q;
                q++;
            }
            if (q < end && *q == '-' && (q + 1) < end && q[1] != ']')
            {
                q++; // consume '-'
                char hi;
                if (*q == '\\' && (q + 1) < end)
                {
                    hi = q[1];
                    q += 2;
                }
                else
                {
                    hi = *q;
                    q++;
                }
                if (re_class_member(lo, hi, ch))
                    m = true;
            }
            else if (ch == lo)
            {
                m = true;
            }
        }
        return neg ? !m : m;
    }
    return ch == *p; // literal
}

static bool re_match(ReCtx *c, const char *pat, const char *text);

// Greedy "(atom)* rest" against text.
static bool re_star(ReCtx *c, const char *atom, size_t al, const char *rest, const char *text)
{
    if (++c->steps > c->max_steps)
        return false;
    if (re_atom_matches(atom, al, *text) && re_star(c, atom, al, rest, text + 1))
        return true;
    return re_match(c, rest, text);
}

static bool re_match(ReCtx *c, const char *pat, const char *text)
{
    if (++c->steps > c->max_steps)
        return false;
    if (*pat == '\0')
        return *text == '\0'; // full-match: pattern and text end together

    size_t al = re_atom_len(pat);
    char quant = pat[al];
    const char *rest = (quant == '*' || quant == '+' || quant == '?') ? pat + al + 1 : pat + al;

    if (quant == '*')
        return re_star(c, pat, al, rest, text);
    if (quant == '+')
    {
        if (!re_atom_matches(pat, al, *text))
            return false;
        return re_star(c, pat, al, rest, text + 1);
    }
    if (quant == '?')
    {
        if (re_atom_matches(pat, al, *text) && re_match(c, rest, text + 1))
            return true;
        return re_match(c, rest, text);
    }
    // exactly one
    if (re_atom_matches(pat, al, *text))
        return re_match(c, rest, text + 1);
    return false;
}

// Whole-path regex match (implicitly anchored at both ends).
static bool regex_match(const char *pattern, const char *path)
{
    ReCtx c;
    c.steps = 0;
    c.max_steps = RE_MAX_STEPS;
    return re_match(&c, pattern, path);
}

/**
 * @brief Segment-by-segment match for routes containing `:name` parameters.
 *
 * Walks @p route and @p path one `/`-delimited segment at a time. Literal
 * segments must match exactly; a `:name` segment captures the corresponding
 * path segment into @p req->path_params. Both must contain the same number of
 * segments. No wildcard support (`:name` and trailing `*` are not combined).
 *
 * @return True on a full match (params captured); false otherwise.
 */
static bool match_path_params(const char *route, const char *path, HttpReq *req)
{
    req->path_param_count = 0;
    const char *r = route;
    const char *p = path;

    while (*r == '/' && *p == '/')
    {
        r++;
        p++;
        const char *rseg = r;
        while (*r && *r != '/')
            r++;
        size_t rlen = (size_t)(r - rseg);
        const char *pseg = p;
        while (*p && *p != '/')
            p++;
        size_t plen = (size_t)(p - pseg);

        if (rlen > 0 && rseg[0] == ':')
        {
            if (plen == 0)
                return false; // a `:name` segment must capture a non-empty value
            if (req->path_param_count < MAX_PATH_PARAMS)
            {
                QueryParam *qp = &req->path_params[req->path_param_count++];
                size_t klen = rlen - 1;
                if (klen > QUERY_KEY_LEN - 1)
                    klen = QUERY_KEY_LEN - 1;
                memcpy(qp->key, rseg + 1, klen);
                qp->key[klen] = '\0';
                size_t vlen = plen;
                if (vlen > QUERY_VAL_LEN - 1)
                    vlen = QUERY_VAL_LEN - 1;
                memcpy(qp->val, pseg, vlen);
                qp->val[vlen] = '\0';
            }
        }
        else if (rlen != plen || strncmp(rseg, pseg, rlen) != 0)
        {
            return false; // literal segment mismatch
        }
    }

    // Both strings must be fully consumed (identical segment counts).
    return (*r == '\0' && *p == '\0');
}

/**
 * @brief Main application tick - tick the session layer then dispatch completed requests.
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
#if DETWS_ENABLE_WEBSOCKET
        // WebSocket slot - drain ring buffer and dispatch ready frames
        WsConn *ws = ws_find(i);
        if (ws)
        {
            ws_parse(ws);

            if (ws->parse_state == WS_FRAME_READY)
            {
                for (uint8_t r = 0; r < _route_count; r++)
                {
                    if (_routes[r].type == ROUTE_WS && _routes[r].ws_message)
                    {
                        _routes[r].ws_message(ws->ws_id);
                        break;
                    }
                }
                ws_reset_frame(ws);
            }
            else if (ws->parse_state == WS_CLOSED || ws->parse_state == WS_ERROR)
            {
                for (uint8_t r = 0; r < _route_count; r++)
                {
                    if (_routes[r].type == ROUTE_WS && _routes[r].ws_close)
                    {
                        _routes[r].ws_close(ws->ws_id);
                        break;
                    }
                }
                ws_free(i);
                http_reset(i);
            }
            continue; // slot is owned by WS; skip HTTP dispatch
        }
#endif // DETWS_ENABLE_WEBSOCKET

#if DETWS_ENABLE_SSE
        // SSE slot - connection stays open, nothing to parse from client
        if (sse_find(i))
            continue;
#endif // DETWS_ENABLE_SSE

        // HTTP slot
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

// ---------------------------------------------------------------------------
// Diagnostic endpoint
// ---------------------------------------------------------------------------

#if DETWS_ENABLE_DIAG
void DetWebServer::diag(uint8_t slot_id)
{
    send(slot_id, 200, "application/json", DETWS_DIAG_JSON);
}
#endif

// ---------------------------------------------------------------------------
// WebSocket handshake helpers
// ---------------------------------------------------------------------------

#if DETWS_ENABLE_WEBSOCKET
/**
 * @brief Compute the Sec-WebSocket-Accept value for the HTTP 101 response.
 *
 * Concatenates the client key with the RFC 6455 magic GUID, SHA-1 hashes
 * the result, and base64-encodes the 20-byte digest into @p out.
 * @p out must be at least 29 bytes (28 base64 chars + null terminator).
 */
static const size_t WS_MAX_KEY_LEN = 64;

static bool ws_accept_key(const char *client_key, char *out)
{
    size_t key_len = strnlen(client_key, WS_MAX_KEY_LEN + 1);
    if (key_len > WS_MAX_KEY_LEN)
    {
        out[0] = '\0';
        return false;
    }
    size_t magic_len = sizeof(WS_MAGIC) - 1;
    char concat[WS_MAX_KEY_LEN + sizeof(WS_MAGIC)];
    memcpy(concat, client_key, key_len);
    memcpy(concat + key_len, WS_MAGIC, magic_len);

    uint8_t digest[SHA1_DIGEST_LEN];
    sha1((const uint8_t *)concat, key_len + magic_len, digest);
    base64_encode(digest, SHA1_DIGEST_LEN, out);
    return true;
}

/**
 * @brief Send the HTTP 101 Switching Protocols handshake and upgrade the slot.
 *
 * Does NOT close the TCP connection -- that is intentional.  The slot moves
 * from HTTP parse ownership to WS frame parse ownership.
 */
/**
 * @brief Send a 426 Upgrade Required for an unsupported Sec-WebSocket-Version.
 *
 * RFC 6455 §4.2.1: if the version is not 13 the server MUST respond with a
 * 426 and include a Sec-WebSocket-Version header listing the versions it
 * supports.  Closes the connection afterward.
 */
static void ws_send_version_required(uint8_t slot_id)
{
    TcpConn *conn = &conn_pool[slot_id];
    if (conn->state != CONN_ACTIVE || conn->pcb == nullptr)
    {
        http_reset(slot_id);
        return;
    }

    static const char resp[] = "HTTP/1.1 426 Upgrade Required\r\n"
                               "Sec-WebSocket-Version: 13\r\n"
                               "Content-Length: 0\r\n"
                               "Connection: close\r\n\r\n";

    struct tcp_pcb *pcb = conn->pcb;
    det_conn_detach(pcb);
    conn->state = CONN_FREE;
    conn->pcb = nullptr;

    det_conn_send(slot_id, pcb, resp, (u16_t)(sizeof(resp) - 1));
    det_conn_flush(slot_id, pcb);
    det_conn_close(pcb);

    http_reset(slot_id);
}

static bool ws_do_upgrade(uint8_t slot_id, HttpReq *req, WsConnectHandler on_connect)
{
    const char *client_key = http_get_header(req, "Sec-WebSocket-Key");
    if (!client_key)
        return false;

    char accept[32];
    if (!ws_accept_key(client_key, accept))
        return false;

    TcpConn *conn = &conn_pool[slot_id];
    if (conn->state != CONN_ACTIVE || !conn->pcb)
        return false;

    char hdr[WS_HDR_BUF_SIZE];
    int hlen = snprintf(hdr, sizeof(hdr),
                        "HTTP/1.1 101 Switching Protocols\r\n"
                        "Upgrade: websocket\r\n"
                        "Connection: Upgrade\r\n"
                        "Sec-WebSocket-Accept: %s\r\n\r\n",
                        accept);

    det_conn_send(slot_id, conn->pcb, hdr, (u16_t)hlen);
    det_conn_flush(slot_id, conn->pcb);

    // Reset HTTP parser but keep the TCP slot -- WS owns it now
    http_reset(slot_id);

    WsConn *ws = ws_alloc(slot_id);
    if (!ws)
    {
        // No WS slot available -- abort the connection
        det_conn_detach(conn->pcb);
        conn->state = CONN_FREE;
        conn->pcb = nullptr;
        return false;
    }

    if (on_connect)
        on_connect(ws->ws_id);

    return true;
}
#endif // DETWS_ENABLE_WEBSOCKET

// ---------------------------------------------------------------------------
// SSE upgrade helper
// ---------------------------------------------------------------------------

#if DETWS_ENABLE_SSE
/**
 * @brief Send the HTTP 200 + SSE headers and promote the slot to SSE mode.
 */
static bool sse_do_upgrade(uint8_t slot_id, HttpReq *req, SseConnectHandler on_connect)
{
    TcpConn *conn = &conn_pool[slot_id];
    if (conn->state != CONN_ACTIVE || !conn->pcb)
        return false;

    static const char SSE_HDR[] = "HTTP/1.1 200 OK\r\n"
                                  "Content-Type: text/event-stream\r\n"
                                  "Cache-Control: no-cache\r\n"
                                  "Connection: keep-alive\r\n\r\n";

    det_conn_send(slot_id, conn->pcb, SSE_HDR, (u16_t)(sizeof(SSE_HDR) - 1));
    det_conn_flush(slot_id, conn->pcb);

    // Reset HTTP parser; SSE keeps the TCP slot open
    const char *path = req->path;
    http_reset(slot_id);

    SseConn *sse = sse_alloc(slot_id, path);
    if (!sse)
    {
        det_conn_detach(conn->pcb);
        conn->state = CONN_FREE;
        conn->pcb = nullptr;
        return false;
    }

    if (on_connect)
        on_connect(sse->sse_id);

    return true;
}
#endif // DETWS_ENABLE_SSE

// ---------------------------------------------------------------------------
// Route dispatch
// ---------------------------------------------------------------------------

// True when the request on this slot used the HEAD method, whose response must
// carry the same headers as GET but no message body (RFC 7231 §4.3.2).
static inline bool req_is_head(uint8_t slot_id)
{
    return strcmp(http_pool[slot_id].method, "HEAD") == 0;
}

// Append a method token to a comma-separated Allow list, de-duplicating.
static void allow_append(char *buf, size_t cap, const char *m)
{
    if (!m[0] || strstr(buf, m))
        return;
    size_t len = strlen(buf);
    if (len == 0)
        snprintf(buf, cap, "%s", m);
    else
        snprintf(buf + len, cap - len, ", %s", m);
}

// Send 405 Method Not Allowed with the required Allow header (RFC 7231 §6.5.5).
static void send_method_not_allowed(uint8_t slot_id, const char *allow)
{
    TcpConn *conn = &conn_pool[slot_id];
    if (conn->state != CONN_ACTIVE || conn->pcb == nullptr)
    {
        http_reset(slot_id);
        return;
    }

    static const char body[] = "Method Not Allowed";
    char header[RESP_HDR_BUF_SIZE];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 405 Method Not Allowed\r\n"
                        "Allow: %s\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: %d\r\n"
                        "Connection: close\r\n\r\n",
                        allow, (int)(sizeof(body) - 1));

    struct tcp_pcb *pcb = conn->pcb;
    det_conn_detach(pcb);
    conn->state = CONN_FREE;
    conn->pcb = nullptr;

    det_conn_send(slot_id, pcb, header, (u16_t)hlen);
    if (!req_is_head(slot_id))
        det_conn_send(slot_id, pcb, body, (u16_t)(sizeof(body) - 1));
    det_conn_flush(slot_id, pcb);
    det_conn_close(pcb);
    http_reset(slot_id);
}

void DetWebServer::match_and_execute(uint8_t slot_id)
{
    HttpReq *req = &http_pool[slot_id];
    HttpMethod method = parse_method(req->method);

    // Start each request with no carried-over custom response headers or
    // captured path parameters.
    _extra_hdr[slot_id][0] = '\0';
    req->path_param_count = 0;

    // Built-in rate limiter first (cheapest rejection under flood), then the
    // user middleware chain. Either may short-circuit with a response.
    if (rate_limit_check(slot_id))
        return;
    if (run_middleware(slot_id, req))
        return;

    // CORS preflight
    if (method == HTTP_OPTIONS && _cors_enabled)
    {
        send_empty(slot_id, 204);
        return;
    }

    // RFC 7230 §3.3.1: reject Transfer-Encoding
    if (http_get_header(req, "Transfer-Encoding") != nullptr)
    {
        send(slot_id, 501, "text/plain", "Not Implemented");
        return;
    }

    // RFC 7231 §6.5.2: a method the server does not implement → 501.
    if (method == HTTP_METHOD_UNKNOWN)
    {
        send(slot_id, 501, "text/plain", "Not Implemented");
        return;
    }

#if DETWS_ENABLE_WEBSOCKET
    const char *upgrade_hdr = http_get_header(req, "Upgrade");
    bool is_ws_upgrade = (method == HTTP_GET) && upgrade_hdr && (strcasecmp(upgrade_hdr, "websocket") == 0);
#endif

    // For RFC 7231 §6.5.5: if a path matches but no method does, answer 405
    // with an Allow header listing the methods registered for that path.
    bool path_matched = false;
    char allow_buf[64];
    allow_buf[0] = '\0';

    for (uint8_t i = 0; i < _route_count; i++)
    {
        Route *r = &_routes[i];
        if (!r->is_active)
            continue;
        bool matched = r->is_regex   ? regex_match(r->path, req->path)
                       : r->is_param ? match_path_params(r->path, req->path, req)
                                     : path_matches(r->path, r->is_wildcard, req->path);
        if (!matched)
            continue;

        // Per-route interface gate: a route bound to STA/AP is invisible on the
        // other interface (falls through to other routes / 404).
        if (r->iface_filter != DETIFACE_ANY && r->iface_filter != conn_pool[slot_id].iface)
            continue;

#if DETWS_ENABLE_WEBSOCKET
        if (r->type == ROUTE_WS)
        {
#if DETWS_ENABLE_TLS
            // Encrypted WebSocket (wss) is not wired yet; reject cleanly so we
            // never emit a plaintext 101 over the TLS record layer.
            if (conn_pool[slot_id].tls)
            {
                send(slot_id, 501, "text/plain", "wss not supported");
                return;
            }
#endif
            if (!is_ws_upgrade)
            {
                send(slot_id, 400, "text/plain", "WebSocket upgrade required");
                return;
            }
            // RFC 6455 §4.2.1: only version 13 is supported; otherwise 426.
            const char *ws_ver = http_get_header(req, "Sec-WebSocket-Version");
            if (!ws_ver || strcmp(ws_ver, "13") != 0)
            {
                ws_send_version_required(slot_id);
                return;
            }
            if (!ws_do_upgrade(slot_id, req, r->ws_connect))
                send(slot_id, 503, "text/plain", "Service Unavailable");
            return;
        }
#endif // DETWS_ENABLE_WEBSOCKET

#if DETWS_ENABLE_SSE
        if (r->type == ROUTE_SSE)
        {
#if DETWS_ENABLE_TLS
            if (conn_pool[slot_id].tls)
            {
                send(slot_id, 501, "text/plain", "encrypted SSE not supported");
                return;
            }
#endif
            if (!sse_do_upgrade(slot_id, req, r->sse_connect))
                send(slot_id, 503, "text/plain", "Service Unavailable");
            return;
        }
#endif // DETWS_ENABLE_SSE

#if DETWS_ENABLE_FILE_SERVING
        if (r->type == ROUTE_STATIC)
        {
            // Static mounts answer GET (and HEAD via GET); other methods → 405.
            if (method != HTTP_GET && method != HTTP_HEAD)
            {
                path_matched = true;
                allow_append(allow_buf, sizeof(allow_buf), "GET");
                allow_append(allow_buf, sizeof(allow_buf), "HEAD");
                continue;
            }
            serve_static_request(slot_id, req, r);
            return;
        }
#endif // DETWS_ENABLE_FILE_SERVING

        // ROUTE_HTTP - a HEAD request is served by the GET handler with the
        // response body suppressed (RFC 7231 §4.3.2).
        bool method_ok = (r->method == method) || (method == HTTP_HEAD && r->method == HTTP_GET);
        if (!method_ok)
        {
            // Path matches but method differs - record it for a 405 + Allow.
            path_matched = true;
            allow_append(allow_buf, sizeof(allow_buf), method_name(r->method));
            // A GET route also answers HEAD, so advertise it in Allow.
            if (r->method == HTTP_GET)
                allow_append(allow_buf, sizeof(allow_buf), "HEAD");
            continue;
        }
#if DETWS_ENABLE_AUTH
        if (r->auth_required)
        {
            bool ok = r->auth_digest ? check_digest_auth(slot_id, req, r) : check_basic_auth(slot_id, req, r);
            if (!ok)
            {
                send_unauth(slot_id, r);
                return;
            }
        }
#endif // DETWS_ENABLE_AUTH
        r->callback(slot_id, req);
        return;
    }

    // Path existed but the method was not allowed (RFC 7231 §6.5.5).
    if (path_matched)
    {
        send_method_not_allowed(slot_id, allow_buf);
        return;
    }

    if (_not_found_handler)
        _not_found_handler(slot_id, req);
    else
        send(slot_id, 404, "text/plain", "Not Found");
}

/*
 * Build and transmit an HTTP response with a body.
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

    char header[RESP_HDR_BUF_SIZE];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 %d %s\r\n"
                        "Content-Type: %s\r\n"
                        "Content-Length: %d\r\n"
                        "%s"
                        "%s"
                        "Connection: close\r\n\r\n",
                        code, status_text(code), content_type, payload_len, _cors_enabled ? _cors_header_buf : "",
                        _extra_hdr[slot_id]);

    struct tcp_pcb *pcb = conn->pcb;
    /*
     * Detach and free the slot before writing.  Any lwIP error callback
     * firing during tcp_write will see CONN_FREE and take no action.
     */
    det_conn_detach(pcb);
    conn->state = CONN_FREE;
    conn->pcb = nullptr;

    bool head = req_is_head(slot_id);

    det_conn_send(slot_id, pcb, header, (u16_t)hlen);
    // HEAD responses carry the headers (incl. Content-Length) but no body.
    if (!head && payload_len > 0)
        det_conn_send(slot_id, pcb, payload, (u16_t)payload_len);
    det_conn_flush(slot_id, pcb);

    det_conn_close(pcb);

    note_response(slot_id, code, payload_len);
    http_reset(slot_id);
}

/*
 * Build and transmit an HTTP response with no body.
 *
 * Used for CORS preflight (204) and any response where only status headers
 * are needed.  Behaves identically to send() regarding slot lifecycle and
 * PCB ownership transfer - the slot is freed before the lwIP write call.
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

    char header[RESP_HDR_BUF_SIZE];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 %d %s\r\n"
                        "Content-Length: 0\r\n"
                        "%s"
                        "%s"
                        "Connection: close\r\n\r\n",
                        code, status_text(code), _cors_enabled ? _cors_header_buf : "", _extra_hdr[slot_id]);

    struct tcp_pcb *pcb = conn->pcb;
    det_conn_detach(pcb);
    conn->state = CONN_FREE;
    conn->pcb = nullptr;

    det_conn_send(slot_id, pcb, header, (u16_t)hlen);
    det_conn_flush(slot_id, pcb);

    det_conn_close(pcb);

    note_response(slot_id, code, 0);
    http_reset(slot_id);
}

void DetWebServer::redirect(uint8_t slot_id, int code, const char *location)
{
    TcpConn *conn = &conn_pool[slot_id];
    if (conn->state != CONN_ACTIVE || conn->pcb == nullptr)
    {
        http_reset(slot_id);
        return;
    }

    // Only the redirect status codes are valid here; anything else → 302.
    switch (code)
    {
    case 301:
    case 302:
    case 303:
    case 307:
    case 308:
        break;
    default:
        code = 302;
        break;
    }

    char header[RESP_HDR_BUF_SIZE];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 %d %s\r\n"
                        "Location: %s\r\n"
                        "Content-Length: 0\r\n"
                        "%s"
                        "%s"
                        "Connection: close\r\n\r\n",
                        code, status_text(code), location, _cors_enabled ? _cors_header_buf : "", _extra_hdr[slot_id]);

    struct tcp_pcb *pcb = conn->pcb;
    det_conn_detach(pcb);
    conn->state = CONN_FREE;
    conn->pcb = nullptr;

    det_conn_send(slot_id, pcb, header, (u16_t)hlen);
    det_conn_flush(slot_id, pcb);

    det_conn_close(pcb);

    note_response(slot_id, code, 0);
    http_reset(slot_id);
}

// ---------------------------------------------------------------------------
// Template rendering
//
// Walk a template once: when @p pcb is null only the output length is summed
// (pass 1); when @p pcb is set each literal run and resolved {{name}} value is
// written to it (pass 2). Walking twice avoids buffering the whole body, so
// memory use is constant. The resolver must be deterministic across the two
// passes. A "{{" with no matching "}}", or a name longer than 32 chars, is
// emitted literally.
// ---------------------------------------------------------------------------
static size_t tmpl_walk(uint8_t slot, const char *tmpl, TemplateVar resolver, struct tcp_pcb *pcb)
{
    size_t total = 0;
    const char *p = tmpl;
    while (*p)
    {
        if (p[0] == '{' && p[1] == '{')
        {
            const char *end = strstr(p + 2, "}}");
            size_t nlen = end ? (size_t)(end - (p + 2)) : 0;
            if (end && nlen <= 32)
            {
                char name[33];
                memcpy(name, p + 2, nlen);
                name[nlen] = '\0';
                const char *val = resolver ? resolver(name) : nullptr;
                if (!val)
                    val = "";
                size_t vlen = strlen(val);
                total += vlen;
                if (pcb && vlen)
                    det_conn_send(slot, pcb, val, (u16_t)vlen);
                p = end + 2;
                continue;
            }
            // Unterminated or over-long placeholder: emit "{{" literally.
            total += 2;
            if (pcb)
                det_conn_send(slot, pcb, "{{", 2);
            p += 2;
            continue;
        }

        // Literal run up to the next "{{".
        const char *run = p;
        while (*p && !(p[0] == '{' && p[1] == '{'))
            p++;
        size_t rlen = (size_t)(p - run);
        total += rlen;
        if (pcb && rlen)
            det_conn_send(slot, pcb, run, (u16_t)rlen);
    }
    return total;
}

void DetWebServer::send_template(uint8_t slot_id, int code, const char *content_type, const char *tmpl,
                                 TemplateVar resolver)
{
    TcpConn *conn = &conn_pool[slot_id];
    if (conn->state != CONN_ACTIVE || conn->pcb == nullptr)
    {
        http_reset(slot_id);
        return;
    }

    // Pass 1: size the rendered body (no writes).
    size_t body_len = tmpl_walk(slot_id, tmpl, resolver, nullptr);

    char header[RESP_HDR_BUF_SIZE];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 %d %s\r\n"
                        "Content-Type: %s\r\n"
                        "Content-Length: %d\r\n"
                        "%s"
                        "%s"
                        "Connection: close\r\n\r\n",
                        code, status_text(code), content_type, (int)body_len, _cors_enabled ? _cors_header_buf : "",
                        _extra_hdr[slot_id]);

    struct tcp_pcb *pcb = conn->pcb;
    det_conn_detach(pcb);
    conn->state = CONN_FREE;
    conn->pcb = nullptr;

    bool head = req_is_head(slot_id);

    det_conn_send(slot_id, pcb, header, (u16_t)hlen);
    // Pass 2: stream the rendered body (HEAD carries headers only).
    if (!head && body_len > 0)
        tmpl_walk(slot_id, tmpl, resolver, pcb);
    det_conn_flush(slot_id, pcb);

    det_conn_close(pcb);

    note_response(slot_id, code, (int)body_len);
    http_reset(slot_id);
}

// ---------------------------------------------------------------------------
// Chunked (streaming) responses
//
// Each write emits one HTTP/1.1 chunk - "<hexlen>\r\n<data>\r\n" - straight to
// the socket (RFC 7230 §4.1). send_chunked() writes the headers, runs the
// filler, then writes the terminating "0\r\n\r\n". The body is never buffered
// whole, so a response may be arbitrarily large in constant memory.
// ---------------------------------------------------------------------------

bool ChunkedResponse::write(const char *s)
{
    return write(s, s ? strlen(s) : 0);
}

bool ChunkedResponse::write(const char *data, size_t len)
{
    // A zero-length chunk is the terminator, so never emit one here; HEAD
    // responses suppress the body entirely.
    if (!_ok || _head || _pcb == nullptr || data == nullptr || len == 0)
        return _ok;

    char sz[12];
    int sn = snprintf(sz, sizeof(sz), "%x\r\n", (unsigned)len);
    det_conn_send(_slot, _pcb, sz, (u16_t)sn);
    det_conn_send(_slot, _pcb, data, (u16_t)len);
    det_conn_send(_slot, _pcb, "\r\n", 2);
    _total += len;
    return true;
}

bool ChunkedResponse::printf(const char *fmt, ...)
{
    if (!_ok || _head || _pcb == nullptr)
        return _ok;

    char buf[CHUNK_BUF_SIZE];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (n < 0)
        return _ok;
    size_t len = (n < (int)sizeof(buf)) ? (size_t)n : (sizeof(buf) - 1);
    return write(buf, len);
}

void DetWebServer::send_chunked(uint8_t slot_id, int code, const char *content_type, ChunkFiller filler)
{
    TcpConn *conn = &conn_pool[slot_id];
    if (conn->state != CONN_ACTIVE || conn->pcb == nullptr)
    {
        http_reset(slot_id);
        return;
    }

    char header[RESP_HDR_BUF_SIZE];
    int hlen =
        snprintf(header, sizeof(header),
                 "HTTP/1.1 %d %s\r\n"
                 "Content-Type: %s\r\n"
                 "Transfer-Encoding: chunked\r\n"
                 "%s"
                 "%s"
                 "Connection: close\r\n\r\n",
                 code, status_text(code), content_type, _cors_enabled ? _cors_header_buf : "", _extra_hdr[slot_id]);

    struct tcp_pcb *pcb = conn->pcb;
    det_conn_detach(pcb);
    conn->state = CONN_FREE;
    conn->pcb = nullptr;

    bool head = req_is_head(slot_id);

    det_conn_send(slot_id, pcb, header, (u16_t)hlen);

    ChunkedResponse res(slot_id, pcb, head);
    // HEAD carries the headers (incl. Transfer-Encoding) but no body, so the
    // filler and terminating chunk are skipped entirely.
    if (!head && filler)
    {
        filler(res, &http_pool[slot_id]);
        det_conn_send(slot_id, pcb, "0\r\n\r\n", 5);
    }

    det_conn_flush(slot_id, pcb);

    det_conn_close(pcb);

    note_response(slot_id, code, head ? 0 : (int)res.total());
    http_reset(slot_id);
}

// ---------------------------------------------------------------------------
// Custom response headers / cookies
//
// Appended to a fixed per-slot buffer during a handler and injected into the
// send paths above. A header that would overflow the buffer is dropped whole
// (the buffer is rewound to its prior length) so a malformed half-line never
// reaches the wire.
// ---------------------------------------------------------------------------

void DetWebServer::add_response_header(uint8_t slot_id, const char *name, const char *value)
{
    if (slot_id >= MAX_CONNS || name == nullptr || value == nullptr)
        return;

    char *buf = _extra_hdr[slot_id];
    size_t used = strlen(buf);
    size_t room = EXTRA_HDR_BUF_SIZE - used;
    int n = snprintf(buf + used, room, "%s: %s\r\n", name, value);
    if (n < 0 || (size_t)n >= room)
        buf[used] = '\0'; // would not fit: drop this header entirely
}

void DetWebServer::set_cookie(uint8_t slot_id, const char *name, const char *value, const char *attrs)
{
    if (slot_id >= MAX_CONNS || name == nullptr || value == nullptr)
        return;

    char *buf = _extra_hdr[slot_id];
    size_t used = strlen(buf);
    size_t room = EXTRA_HDR_BUF_SIZE - used;
    int n;
    if (attrs != nullptr && attrs[0] != '\0')
        n = snprintf(buf + used, room, "Set-Cookie: %s=%s; %s\r\n", name, value, attrs);
    else
        n = snprintf(buf + used, room, "Set-Cookie: %s=%s\r\n", name, value);
    if (n < 0 || (size_t)n >= room)
        buf[used] = '\0'; // would not fit: drop this cookie entirely
}

void DetWebServer::clear_response_headers(uint8_t slot_id)
{
    if (slot_id >= MAX_CONNS)
        return;
    _extra_hdr[slot_id][0] = '\0';
}

// ---------------------------------------------------------------------------
// MIME type lookup by extension
// ---------------------------------------------------------------------------

const char *DetWebServer::mime_type(const char *path)
{
    if (!path)
        return "application/octet-stream";

    // Find the last '.' after the last '/'.
    const char *dot = nullptr;
    for (const char *p = path; *p; p++)
    {
        if (*p == '/')
            dot = nullptr;
        else if (*p == '.')
            dot = p;
    }
    if (!dot || dot[1] == '\0')
        return "application/octet-stream";
    const char *ext = dot + 1;

    // Case-insensitive compare against a small static table.
    static const struct
    {
        const char *ext;
        const char *type;
    } table[] = {
        {"html", "text/html"},
        {"htm", "text/html"},
        {"css", "text/css"},
        {"js", "application/javascript"},
        {"mjs", "application/javascript"},
        {"json", "application/json"},
        {"xml", "application/xml"},
        {"txt", "text/plain"},
        {"csv", "text/csv"},
        {"svg", "image/svg+xml"},
        {"png", "image/png"},
        {"jpg", "image/jpeg"},
        {"jpeg", "image/jpeg"},
        {"gif", "image/gif"},
        {"ico", "image/x-icon"},
        {"webp", "image/webp"},
        {"wasm", "application/wasm"},
        {"woff", "font/woff"},
        {"woff2", "font/woff2"},
        {"ttf", "font/ttf"},
        {"pdf", "application/pdf"},
        {"gz", "application/gzip"},
    };
    for (size_t i = 0; i < sizeof(table) / sizeof(table[0]); i++)
    {
        const char *a = ext;
        const char *b = table[i].ext;
        bool eq = true;
        while (*a && *b)
        {
            char ca = (*a >= 'A' && *a <= 'Z') ? (char)(*a + 32) : *a;
            char cb = *b; // table is already lowercase
            if (ca != cb)
            {
                eq = false;
                break;
            }
            a++;
            b++;
        }
        if (eq && *a == '\0' && *b == '\0')
            return table[i].type;
    }
    return "application/octet-stream";
}

// ---------------------------------------------------------------------------
// Runtime stats endpoint
// ---------------------------------------------------------------------------

#if DETWS_ENABLE_STATS
void DetWebServer::stats(uint8_t slot_id)
{
    int active = 0;
    for (int i = 0; i < MAX_CONNS; i++)
        if (conn_pool[i].state == CONN_ACTIVE)
            active++;

    unsigned long up = millis();
#ifdef ARDUINO
    uint32_t heap = ESP.getFreeHeap();
#else
    uint32_t heap = 0;
#endif

    char body[192];
    snprintf(body, sizeof(body),
             "{\"uptime_ms\":%lu,\"requests\":%lu,\"http_2xx\":%lu,\"http_4xx\":%lu,"
             "\"http_5xx\":%lu,\"active_conns\":%d,\"free_heap\":%u}",
             up, (unsigned long)_stat_requests, (unsigned long)_stat_2xx, (unsigned long)_stat_4xx,
             (unsigned long)_stat_5xx, active, (unsigned)heap);
    send(slot_id, 200, "application/json", body);
}
#endif // DETWS_ENABLE_STATS

// ---------------------------------------------------------------------------
// WebSocket public API
// ---------------------------------------------------------------------------

#if DETWS_ENABLE_WEBSOCKET
void DetWebServer::ws_send_text(uint8_t ws_id, const char *text)
{
    if (ws_id >= MAX_WS_CONNS || !ws_pool[ws_id].active)
        return;
    WsConn *ws = &ws_pool[ws_id];
    if (ws->parse_state == WS_CLOSED || ws->parse_state == WS_ERROR)
        return;
    uint16_t len = (uint16_t)strlen(text);
    if (ws_send_frame(ws, WS_OP_TEXT, (const uint8_t *)text, len))
    {
        TcpConn *conn = &conn_pool[ws->slot_id];
        if (conn->pcb)
            det_conn_flush(conn->id, conn->pcb);
    }
}

void DetWebServer::ws_send_binary(uint8_t ws_id, const uint8_t *data, uint16_t len)
{
    if (ws_id >= MAX_WS_CONNS || !ws_pool[ws_id].active)
        return;
    WsConn *ws = &ws_pool[ws_id];
    if (ws->parse_state == WS_CLOSED || ws->parse_state == WS_ERROR)
        return;
    if (ws_send_frame(ws, WS_OP_BINARY, data, len))
    {
        TcpConn *conn = &conn_pool[ws->slot_id];
        if (conn->pcb)
            det_conn_flush(conn->id, conn->pcb);
    }
}

void DetWebServer::ws_disconnect(uint8_t ws_id)
{
    if (ws_id >= MAX_WS_CONNS || !ws_pool[ws_id].active)
        return;
    WsConn *ws = &ws_pool[ws_id];
    ws_close(ws, WS_CLOSE_NORMAL);
    TcpConn *conn = &conn_pool[ws->slot_id];
    if (conn->pcb)
        det_conn_flush(conn->id, conn->pcb);
    // handle() detects WS_CLOSED next tick and fires ws_close callback
}
#endif // DETWS_ENABLE_WEBSOCKET

// ---------------------------------------------------------------------------
// Server-Sent Events public API
// ---------------------------------------------------------------------------

#if DETWS_ENABLE_SSE
void DetWebServer::sse_send(uint8_t sse_id, const char *data, const char *event, const char *id)
{
    if (sse_id >= MAX_SSE_CONNS || !sse_pool[sse_id].active)
        return;
    SseConn *sse = &sse_pool[sse_id];
    if (sse_write(sse, data, event, id))
    {
        TcpConn *conn = &conn_pool[sse->slot_id];
        if (conn->pcb)
            det_conn_flush(conn->id, conn->pcb);
    }
}

void DetWebServer::sse_broadcast(const char *path, const char *data, const char *event, const char *id)
{
    for (int i = 0; i < MAX_SSE_CONNS; i++)
    {
        if (!sse_pool[i].active)
            continue;
        if (strcmp(sse_pool[i].path, path) != 0)
            continue;
        SseConn *sse = &sse_pool[i];
        if (sse_write(sse, data, event, id))
        {
            TcpConn *conn = &conn_pool[sse->slot_id];
            if (conn->pcb)
                det_conn_flush(conn->id, conn->pcb);
        }
    }
}
#endif // DETWS_ENABLE_SSE

// ---------------------------------------------------------------------------
// Basic Auth helpers
// ---------------------------------------------------------------------------

#if DETWS_ENABLE_AUTH
// Lowercase-hex-encode @p len bytes of @p in into @p out (needs len*2+1 bytes).
static void hex_encode(const uint8_t *in, size_t len, char *out)
{
    static const char hexd[] = "0123456789abcdef";
    for (size_t i = 0; i < len; i++)
    {
        out[i * 2] = hexd[in[i] >> 4];
        out[i * 2 + 1] = hexd[in[i] & 0x0f];
    }
    out[len * 2] = '\0';
}

// One-shot SHA-256 of @p data, written as 64 lowercase hex chars + NUL.
static void sha256_hex(const uint8_t *data, size_t len, char out[65])
{
    uint8_t d[SSH_SHA256_DIGEST_LEN];
    ssh_sha256(data, len, d);
    hex_encode(d, SSH_SHA256_DIGEST_LEN, out);
}

// Extract the value of @p key from a Digest auth header into @p out.
// Handles both quoted ("value") and token (value) forms. The match must sit on
// a field boundary (start, or after ' '/',') and be immediately followed by '='
// so "nc" does not match inside "cnonce", etc.
static bool digest_field(const char *hdr, const char *key, char *out, size_t out_size)
{
    size_t klen = strlen(key);
    const char *p = hdr;
    while ((p = strstr(p, key)) != nullptr)
    {
        bool left_ok = (p == hdr) || p[-1] == ' ' || p[-1] == ',';
        const char *after = p + klen;
        if (left_ok && *after == '=')
        {
            after++;
            const char *vs;
            const char *ve;
            if (*after == '"')
            {
                vs = after + 1;
                ve = strchr(vs, '"');
                if (!ve)
                    return false;
            }
            else
            {
                vs = after;
                ve = vs;
                while (*ve && *ve != ',' && *ve != ' ')
                    ve++;
            }
            size_t vlen = (size_t)(ve - vs);
            if (vlen > out_size - 1)
                vlen = out_size - 1;
            memcpy(out, vs, vlen);
            out[vlen] = '\0';
            return true;
        }
        p = after;
    }
    return false;
}

void DetWebServer::regen_digest_nonce()
{
    // Seed a 128-bit nonce from the hardware CSPRNG (esp_random() on ESP32; a
    // non-crypto mock on native test builds), folded through SHA-256 with a
    // counter + millis() so even a weak host RNG yields distinct values across
    // regenerations. Replaces the old counter+`this` derivation, which was
    // predictable and leaked an object address.
    static uint32_t counter = 0;
    counter++;
    uint8_t seed[24];
    for (int i = 0; i < 4; i++)
    {
        uint32_t r = esp_random();
        memcpy(seed + i * 4, &r, 4);
    }
    uint32_t c = counter;
    uint32_t t = (uint32_t)millis();
    memcpy(seed + 16, &c, 4);
    memcpy(seed + 20, &t, 4);
    uint8_t d[SSH_SHA256_DIGEST_LEN];
    ssh_sha256(seed, sizeof(seed), d);
    hex_encode(d, 16, _digest_nonce); // 16 bytes -> 32 hex chars
}

void DetWebServer::send_unauth(uint8_t slot_id, const Route *r)
{
    TcpConn *conn = &conn_pool[slot_id];
    if (conn->state != CONN_ACTIVE || !conn->pcb)
    {
        http_reset(slot_id);
        return;
    }

    char challenge[MAX_AUTH_LEN + 128];
    if (r->auth_digest)
        snprintf(challenge, sizeof(challenge),
                 "WWW-Authenticate: Digest realm=\"%s\", qop=\"auth\", algorithm=SHA-256, nonce=\"%s\"\r\n",
                 r->auth_realm, _digest_nonce);
    else
        snprintf(challenge, sizeof(challenge), "WWW-Authenticate: Basic realm=\"%s\"\r\n", r->auth_realm);

    static const char body[] = "Unauthorized";
    char header[RESP_HDR_BUF_SIZE];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 401 Unauthorized\r\n"
                        "%s"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: %d\r\n"
                        "%s"
                        "Connection: close\r\n\r\n",
                        challenge, (int)(sizeof(body) - 1), _cors_enabled ? _cors_header_buf : "");

    struct tcp_pcb *pcb = conn->pcb;
    det_conn_detach(pcb);
    conn->state = CONN_FREE;
    conn->pcb = nullptr;

    det_conn_send(slot_id, pcb, header, (u16_t)hlen);
    if (!req_is_head(slot_id))
        det_conn_send(slot_id, pcb, body, (u16_t)(sizeof(body) - 1));
    det_conn_flush(slot_id, pcb);

    det_conn_close(pcb);

    http_reset(slot_id);
}

bool DetWebServer::check_basic_auth(uint8_t /*slot_id*/, HttpReq *req, const Route *r)
{
    const char *auth_hdr = http_get_header(req, "Authorization");
    if (!auth_hdr || strncmp(auth_hdr, "Basic ", 6) != 0)
        return false;

    uint8_t decoded[MAX_AUTH_LEN * 2 + 2];
    // Bound the write to leave room for the null terminator at decoded[n]; an
    // over-long Authorization value now fails the decode instead of overrunning.
    size_t n = base64_decode(auth_hdr + 6, decoded, sizeof(decoded) - 1);
    if (n == 0)
        return false;
    decoded[n] = '\0';

    const char *colon = (const char *)memchr(decoded, ':', n);
    if (!colon)
        return false;

    size_t ulen = (size_t)(colon - (const char *)decoded);
    const char *pass = colon + 1;

    return (ulen == strlen(r->auth_user)) && (memcmp(decoded, r->auth_user, ulen) == 0) &&
           (strcmp(pass, r->auth_pass) == 0);
}

// Validate an Authorization: Digest header (RFC 7616, SHA-256, qop=auth).
// HA1 = SHA256(user:realm:pass), HA2 = SHA256(method:uri),
// response = SHA256(HA1:nonce:nc:cnonce:qop:HA2).
bool DetWebServer::check_digest_auth(uint8_t /*slot_id*/, HttpReq *req, const Route *r)
{
    // Use the full-length Authorization capture (the scratch header value is
    // capped at MAX_VAL_LEN, far shorter than a Digest header).
    const char *hdr = req->authorization;
    if (strncmp(hdr, "Digest ", 7) != 0)
        return false;
    const char *d = hdr + 7;

    char username[MAX_AUTH_LEN];
    char nonce[40];
    char uri[MAX_PATH_LEN + MAX_QUERY_LEN + 2];
    char qop[16];
    char nc[16];
    char cnonce[64];
    char response[80];

    if (!digest_field(d, "username", username, sizeof(username)) || !digest_field(d, "nonce", nonce, sizeof(nonce)) ||
        !digest_field(d, "uri", uri, sizeof(uri)) || !digest_field(d, "qop", qop, sizeof(qop)) ||
        !digest_field(d, "nc", nc, sizeof(nc)) || !digest_field(d, "cnonce", cnonce, sizeof(cnonce)) ||
        !digest_field(d, "response", response, sizeof(response)))
        return false;

    // Identity + challenge binding must match before any hashing.
    if (strcmp(username, r->auth_user) != 0)
        return false;
    if (strcmp(nonce, _digest_nonce) != 0)
        return false;
    if (strcmp(qop, "auth") != 0)
        return false;

    char tmp[3 * MAX_AUTH_LEN + 4];
    char ha1[65];
    char ha2[65];
    char expected[65];

    int n = snprintf(tmp, sizeof(tmp), "%s:%s:%s", r->auth_user, r->auth_realm, r->auth_pass);
    sha256_hex((const uint8_t *)tmp, (size_t)n, ha1);

    char tmp2[sizeof(uri) + 16];
    n = snprintf(tmp2, sizeof(tmp2), "%s:%s", req->method, uri);
    sha256_hex((const uint8_t *)tmp2, (size_t)n, ha2);

    char tmp3[65 + 40 + 16 + 64 + 8 + 65 + 8];
    n = snprintf(tmp3, sizeof(tmp3), "%s:%s:%s:%s:%s:%s", ha1, nonce, nc, cnonce, qop, ha2);
    sha256_hex((const uint8_t *)tmp3, (size_t)n, expected);

    return strcasecmp(expected, response) == 0;
}
#endif // DETWS_ENABLE_AUTH

// ---------------------------------------------------------------------------
// File serving
// ---------------------------------------------------------------------------

#if DETWS_ENABLE_FILE_SERVING
void DetWebServer::serve_file_internal(uint8_t slot_id, bool head, fs::FS &file_sys, const char *fs_path,
                                       const char *content_type, const char *content_encoding)
{
    fs::File f = file_sys.open(fs_path, "r");
    if (!f)
    {
        send(slot_id, 404, "text/plain", "Not Found");
        return;
    }

    TcpConn *conn = &conn_pool[slot_id];
    if (conn->state != CONN_ACTIVE || !conn->pcb)
    {
        f.close();
        http_reset(slot_id);
        return;
    }

    size_t file_size = f.size();

    // Optional Content-Encoding line (e.g. gzip for pre-compressed assets).
    char enc_line[40];
    enc_line[0] = '\0';
    if (content_encoding)
        snprintf(enc_line, sizeof(enc_line), "Content-Encoding: %s\r\n", content_encoding);

#if DETWS_ENABLE_ETAG
    // Strong validator from size + last-modified time. If the client's
    // If-None-Match matches, the resource is unchanged -> 304 (no body).
    char etag[40];
    snprintf(etag, sizeof(etag), "\"%x-%lx\"", (unsigned)file_size, (unsigned long)f.getLastWrite());
    const char *inm = http_get_header(&http_pool[slot_id], "If-None-Match");
    if (inm && strcmp(inm, etag) == 0)
    {
        f.close();
        struct tcp_pcb *p304 = conn->pcb;
        det_conn_detach(p304);
        conn->state = CONN_FREE;
        conn->pcb = nullptr;
        char h304[RESP_HDR_BUF_SIZE];
        int n304 = snprintf(h304, sizeof(h304), "HTTP/1.1 304 Not Modified\r\nETag: %s\r\n%sConnection: close\r\n\r\n",
                            etag, _cors_enabled ? _cors_header_buf : "");
        det_conn_send(slot_id, p304, h304, (u16_t)n304);
        det_conn_flush(slot_id, p304);
        det_conn_close(p304);
        note_response(slot_id, 304, 0);
        http_reset(slot_id);
        return;
    }
    char etag_line[48];
    snprintf(etag_line, sizeof(etag_line), "ETag: %s\r\n", etag);
#else
    const char *etag_line = "";
#endif

    char header[RESP_HDR_BUF_SIZE];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: %s\r\n"
                        "Content-Length: %d\r\n"
                        "%s%s%s"
                        "Connection: close\r\n\r\n",
                        content_type, (int)file_size, enc_line, etag_line, _cors_enabled ? _cors_header_buf : "");

    struct tcp_pcb *pcb = conn->pcb;
    det_conn_detach(pcb);
    conn->state = CONN_FREE;
    conn->pcb = nullptr;

    det_conn_send(slot_id, pcb, header, (u16_t)hlen);

    // HEAD: headers only (Content-Length reflects the body that would be sent).
    if (!head)
    {
        uint8_t chunk[FILE_CHUNK_SIZE];
        size_t n;
        while ((n = f.read(chunk, sizeof(chunk))) > 0)
            det_conn_send(slot_id, pcb, chunk, (u16_t)n);
    }

    det_conn_flush(slot_id, pcb);

    det_conn_close(pcb);

    f.close();
    note_response(slot_id, 200, head ? 0 : (int)file_size);
    http_reset(slot_id);
}

void DetWebServer::serve_file(uint8_t slot_id, fs::FS &file_sys, const char *fs_path, const char *content_type)
{
    serve_file_internal(slot_id, req_is_head(slot_id), file_sys, fs_path, content_type, nullptr);
}

void DetWebServer::serve_static(const char *url_prefix, fs::FS &file_sys, const char *fs_root)
{
    if (_route_count >= MAX_ROUTES)
        return;
    Route *r = &_routes[_route_count++];

    // Store the pattern as a wildcard so path_matches() does a prefix match.
    char pat[MAX_PATH_LEN];
    size_t n = strlen(url_prefix);
    if (n > 0 && url_prefix[n - 1] == '*')
        snprintf(pat, sizeof(pat), "%s", url_prefix); // already a wildcard
    else
        snprintf(pat, sizeof(pat), "%s*", url_prefix); // append the wildcard
    fill_route_base(r, pat);
    r->type = ROUTE_STATIC;
    r->method = HTTP_GET;
    r->static_fs = &file_sys;
    r->static_root = fs_root;
}

void DetWebServer::serve_static_request(uint8_t slot_id, HttpReq *req, const Route *r)
{
    if (!r->static_fs)
    {
        send(slot_id, 404, "text/plain", "Not Found");
        return;
    }

    // Request path beyond the mount prefix (route path minus its trailing '*').
    size_t plen = strlen(r->path);
    if (plen > 0 && r->path[plen - 1] == '*')
        plen--;
    const char *sub = (strlen(req->path) >= plen) ? req->path + plen : "";

    // Reject path traversal before touching the filesystem.
    if (strstr(sub, ".."))
    {
        send(slot_id, 404, "text/plain", "Not Found");
        return;
    }

    const char *root = r->static_root ? r->static_root : "";
    size_t rlen = strlen(root);
    bool root_slash = (rlen > 0 && root[rlen - 1] == '/');
    if (root_slash && sub[0] == '/') // avoid a doubled separator
        sub++;
    bool sub_slash = (sub[0] == '/');
    const char *sep = (root_slash || sub_slash) ? "" : "/";

    // Directory or bare-prefix request → index.html.
    size_t slen = strlen(sub);
    bool dir = (slen == 0) || (sub[slen - 1] == '/');

    char fs_path[256];
    int wn = dir ? snprintf(fs_path, sizeof(fs_path), "%s%s%sindex.html", root, sep, sub)
                 : snprintf(fs_path, sizeof(fs_path), "%s%s%s", root, sep, sub);
    if (wn <= 0 || wn >= (int)sizeof(fs_path))
    {
        send(slot_id, 404, "text/plain", "Not Found");
        return;
    }

    const char *ctype = mime_type(fs_path);
    bool head = req_is_head(slot_id);

    // Pre-compressed variant: serve <path>.gz if the client accepts gzip and it
    // exists. Content-Type stays that of the original (uncompressed) resource.
    const char *ae = http_get_header(req, "Accept-Encoding");
    if (ae && strstr(ae, "gzip"))
    {
        char gz[260];
        int gn = snprintf(gz, sizeof(gz), "%s.gz", fs_path);
        if (gn > 0 && gn < (int)sizeof(gz) && r->static_fs->exists(gz))
        {
            serve_file_internal(slot_id, head, *r->static_fs, gz, ctype, "gzip");
            return;
        }
    }

    serve_file_internal(slot_id, head, *r->static_fs, fs_path, ctype, nullptr);
}
#endif // DETWS_ENABLE_FILE_SERVING
