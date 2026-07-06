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
 * **PCB lifecycle / teardown ownership**
 * All TCP I/O and teardown go through the transport-layer connection API
 * (det_conn_send / det_conn_flush / det_conn_begin_close / det_conn_close /
 * det_conn_abort_slot), so this layer never calls lwIP or touches the raw
 * `tcp_pcb` directly. The transport owns the teardown order for every close:
 * it detaches the pcb from its lwIP callbacks and sets the slot `CONN_FREE`
 * (pcb nulled) BEFORE the FIN/RST, on the captured pcb pointer. This means any
 * lwIP error callback that fires mid-teardown sees the slot as already free and
 * takes no action - preventing a double-free. L7 passes only the slot index:
 * det_conn_close(slot) for a graceful local close, det_conn_abort_slot(slot)
 * for a hard RST, det_conn_begin_close(slot) for the drain-then-close dwell.
 */

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/presentation/presentation.h" // http_proto_set_poll (install the instance-bound HTTP poll)
#include "network_drivers/session/proto_handler.h"
#include "network_drivers/session/worker.h"
#include "network_drivers/tls/det_tls.h"
#include "network_drivers/transport/listener.h"
#include "shared_primitives/det_hex.h"
#include "shared_primitives/det_mime.h"
#if DETWS_ENABLE_HTTP2
#include "network_drivers/presentation/http2/h2_server.h"
#endif
#if DETWS_ENABLE_HTTP3
#include "network_drivers/presentation/http3/quic_server.h"
#endif
#if DETWS_ENABLE_WEBSOCKET
#include "network_drivers/presentation/base64/base64.h"
#include "network_drivers/presentation/sha1/sha1.h"
#elif DETWS_ENABLE_AUTH
#include "network_drivers/presentation/base64/base64.h"
#endif
#if DETWS_ENABLE_AUTH
#include "network_drivers/presentation/ssh/crypto/ssh_sha256.h"
#include "services/det_clock.h" // detws_millis() for the stateless Digest nonce timestamp
#if DETWS_ENABLE_AUTH_LOCKOUT
#include "services/auth_lockout/auth_lockout.h"
#endif
#ifdef ARDUINO
#include <esp_system.h> // esp_random() for the Digest nonce CSPRNG
#endif
#endif
#if DETWS_ENABLE_CSRF
#include "services/csrf/csrf.h"
#ifdef ARDUINO
#include <esp_system.h> // esp_random() for the CSRF HMAC secret
#endif
#endif
#if DETWS_ENABLE_WEBDAV
#include "services/webdav/webdav.h"
#include <time.h> // RFC 1123 Last-Modified formatting
#endif
#if DETWS_ENABLE_METRICS || DETWS_ENABLE_STATS
#include "network_drivers/application/web_assets.h" // DETWS_METRICS_PROM / DETWS_STATS_JSON (generated)
#endif
#if DETWS_HTTP_EMIT_DATE
#include "services/ntp_service.h" // detws_ntp_http_date() for the optional Date header
#endif
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#if DETWS_ENABLE_WEBSOCKET
// Magic GUID concatenated to the client key for the WS accept hash (RFC 6455 §4.2.2)
static const char WS_MAGIC[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
#endif

#if DETWS_ENABLE_FILE_SERVING
// Cross-loop file-send continuation. A file response larger than the TCP send
// buffer cannot be blasted out in one dispatch (tcp_write returns ERR_MEM once the
// window fills and the rest would be dropped). Instead serve_file_internal sends
// the headers, opens the file, and hands it to this per-slot state; file_send_pump
// pages out at most det_conn_sndbuf() bytes per worker loop and resumes on the next
// loop (woken promptly by the sent callback) as the window drains - no truncation,
// no blocking the worker. One transfer per slot at a time.
struct FileSend
{
    fs::File file;    ///< open source file (held across loops).
    size_t off;       ///< absolute file offset of the next byte to send.
    size_t remaining; ///< body bytes still to send.
    int status;       ///< response status (200 / 206) for note_response.
    int total;        ///< total body length, for the access log.
    bool keep;        ///< keep-alive vs close at completion.
    bool active;      ///< a transfer is in progress on this slot.
};
static FileSend g_file_send[MAX_CONNS];
#endif

// Per-slot chunked-send continuation. Mirrors FileSend but pulls body pieces from
// a ChunkSource generator and adds the HTTP chunk framing; paged across loops.
struct ChunkSend
{
    ChunkSource source; ///< body generator (active==false means none).
    void *ctx;          ///< caller state passed to source (must outlive the send).
    int status;         ///< response status, for note_response.
    int total;          ///< body bytes emitted so far (excludes framing).
    bool keep;          ///< keep-alive vs close at completion.
    bool active;        ///< a chunked response is in progress on this slot.
    bool raw;           ///< HTTP/1.0 client: stream the body unframed, close-delimited (no chunk wrapping).
};
static ChunkSend g_chunk_send[MAX_CONNS];

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
    case 206:
        return "Partial Content";
#if DETWS_ENABLE_WEBDAV
    case 207:
        return "Multi-Status";
#endif
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
#if DETWS_ENABLE_WEBDAV
    case 412:
        return "Precondition Failed";
    case 423:
        return "Locked";
    case 502:
        return "Bad Gateway";
#endif
    case 413:
        return "Payload Too Large";
    case 414:
        return "URI Too Long";
    case 416:
        return "Range Not Satisfiable";
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

DetWebServer::DetWebServer()
    : _route_count(0), _not_found_handler(nullptr), _cors_enabled(false), _log_cb(nullptr), _listener_count(0),
      _middleware_count(0), _rl_max(0), _rl_window_ms(0), _rl_window_start(0), _rl_count(0)
{
    for (int i = 0; i < MAX_ROUTES; i++)
        _routes[i] = {};
    for (int i = 0; i < MAX_MIDDLEWARE; i++)
        _middleware[i] = nullptr;
    _cors_header_buf[0] = '\0';
    _cache_control_buf[0] = '\0';
    for (int i = 0; i < MAX_CONNS; i++)
        _extra_hdr[i][0] = '\0';
#if DETWS_ENABLE_AUTH
    regen_digest_secret();
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

#if DETWS_ENABLE_KEEPALIVE || DETWS_ENABLE_WEBSOCKET
// Case-insensitive search for @p token as a comma/space-delimited element of a
// Connection header value (e.g. "keep-alive" in "Keep-Alive, Upgrade"). Shared by
// keep-alive evaluation and the WebSocket Upgrade-token check.
static bool conn_has_token(const char *hdr, const char *token)
{
    if (!hdr)
        return false;
    size_t tlen = strlen(token);
    const char *p = hdr;
    while (*p)
    {
        while (*p == ' ' || *p == ',' || *p == '\t')
            p++;
        const char *start = p;
        while (*p && *p != ',')
            p++;
        size_t len = (size_t)(p - start);
        while (len && (start[len - 1] == ' ' || start[len - 1] == '\t'))
            len--;
        if (len == tlen && strncasecmp(start, token, tlen) == 0)
            return true;
        if (*p == ',')
            p++;
    }
    return false;
}
#endif // DETWS_ENABLE_KEEPALIVE || DETWS_ENABLE_WEBSOCKET

#if DETWS_ENABLE_KEEPALIVE
bool DetWebServer::keepalive_eval(uint8_t slot_id)
{
    HttpReq *req = &http_pool[slot_id];
    // Only a cleanly-parsed request has a known message boundary; errors close.
    if (req->parse_state != PARSE_COMPLETE)
        return false;

    const char *c = http_get_header(req, "Connection");
    bool keep;
    if (req->version == HTTP_11)
        keep = !conn_has_token(c, "close"); // 1.1 default: persistent
    else
        keep = conn_has_token(c, "keep-alive"); // 1.0/unknown default: close
    if (!keep)
        return false;

    // Fairness bound: serve at most DETWS_KEEPALIVE_MAX_REQUESTS, then close.
    if (++http_req_count[slot_id] >= DETWS_KEEPALIVE_MAX_REQUESTS)
        return false;
    return true;
}
#endif // DETWS_ENABLE_KEEPALIVE

// Finish a response: flush, then either begin the graceful CONN_CLOSING dwell
// (close path) or leave the slot active for reuse (keep-alive). The HTTP parser
// is reset either way, returning a kept-alive slot to PARSE_METHOD ready for the
// next request. The slot stays CONN_ACTIVE through the write for BOTH paths
// (callbacks live - the model keep-alive has always used); the close path then
// dwells in CONN_CLOSING from here, so the slot is reclaimed only once the peer
// ACKs the response (or the CLOSING timeout fires), not before it is delivered.
// Every response path now addresses the connection by slot alone - the transport
// resolves the pcb internally, the same way the RX read path does (no pcb is
// threaded through the app layer, so the send target can never disagree).
void DetWebServer::resp_end(uint8_t slot_id, int code, int body_len, bool keep)
{
    det_conn_flush(slot_id);
    if (!keep)
        det_conn_begin_close(slot_id); // ACTIVE -> CONN_CLOSING; finalizes on ACK
    note_response(slot_id, code, body_len);
    http_reset(slot_id);
}

// Resolve the Connection response header (and report keep-alive intent) in one
// place so every response path agrees. Keep-alive compiled out always closes.
const char *DetWebServer::resp_conn_hdr(uint8_t slot_id, bool *keep_out)
{
    bool keep = false;
#if DETWS_ENABLE_KEEPALIVE
    keep = keepalive_eval(slot_id);
#else
    (void)slot_id;
#endif
    if (keep_out)
        *keep_out = keep;
    return keep ? "Connection: keep-alive\r\n" : "Connection: close\r\n";
}

// Append the shared response trailer (CORS block + custom headers + Connection +
// the terminating blank line) to a header buffer already holding the status line
// and per-response headers. One owner for the trailer every dynamic response ends
// with. Returns the new total length.
int DetWebServer::append_resp_trailer(char *buf, size_t cap, int hlen, uint8_t slot_id, const char *cl)
{
    if (hlen < 0)
        return 0;
    if ((size_t)hlen >= cap)
        return (int)cap - 1; // status line already filled the buffer (truncated); clamp in-bounds
#if DETWS_HTTP_EMIT_DATE
    // RFC 7231 7.1.1.2: emit Date only when a real wall-clock time exists; a
    // clock-less device (or one whose NTP has not synced yet) omits it.
    char date_hdr[48] = "";
    char imf[40];
    if (detws_ntp_http_date(imf, sizeof(imf)) > 0)
        snprintf(date_hdr, sizeof(date_hdr), "Date: %s\r\n", imf);
#else
    const char *date_hdr = "";
#endif
    int n = snprintf(buf + hlen, cap - (size_t)hlen, "%s%s%s%s\r\n", date_hdr, _cors_enabled ? _cors_header_buf : "",
                     _extra_hdr[slot_id], cl);
    if (n < 0)
        return hlen;
    // snprintf returns the would-be length; if the trailer truncated, clamp to the
    // bytes actually written so the returned length never exceeds the buffer (else a
    // caller would send/copy past the end - a stack over-read on large extra headers).
    if ((size_t)n >= cap - (size_t)hlen)
        return (int)cap - 1;
    return hlen + n;
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
    send(slot_id, 429, DET_MIME_TEXT_PLAIN, "Too Many Requests");
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

// Server instance whose pipeline the worker task pumps, plus the trampoline the
// generic worker layer calls (it has no DetWebServer type). Set in begin().
static DetWebServer *s_worker_server = nullptr;
static void detws_pump_trampoline(int worker_id)
{
    if (s_worker_server)
        s_worker_server->service_once(worker_id);
}

#if DETWS_ENABLE_HTTP3
// The quic_server request seam has no DetWebServer type; this trampoline forwards a completed
// HTTP/3 request into the instance's shared route dispatcher (app == the DetWebServer *).
static bool s_h3_running = false;
static void detws_h3_request_trampoline(void *app, uint32_t conn_id, uint64_t stream_id, const char *method,
                                        const char *path, const char *authority, const uint8_t *body, size_t body_len)
{
    if (app)
        ((DetWebServer *)app)->dispatch_h3_request(conn_id, stream_id, method, path, authority, body, body_len);
}

// Randomness for the QUIC ephemeral X25519 key, the ServerHello random, and our connection IDs: the
// hardware TRNG on device; a deterministic PRNG on host (test builds carry no security context and
// have no esp_random).
static void detws_h3_rng(uint8_t *out, size_t len)
{
#ifdef ARDUINO
    size_t i = 0;
    while (i < len)
    {
        uint32_t r = esp_random();
        size_t n = (len - i) < 4 ? (len - i) : 4;
        memcpy(out + i, &r, n);
        i += n;
    }
#else
    static uint32_t s = 0x9e3779b9u;
    for (size_t i = 0; i < len; i++)
    {
        s = s * 1664525u + 1013904223u;
        out[i] = (uint8_t)(s >> 24);
    }
#endif
}
#endif // DETWS_ENABLE_HTTP3

// HTTP's poll is instance-bound (it dispatches into this server's routes), so it cannot be a plain
// global on_poll like the singleton protocols. begin() records the serving instance here and installs
// this forwarder as the HTTP ProtoHandler's on_poll, so the worker loop pumps HTTP through the one
// uniform seam. The library serves from a single DetWebServer (the slot pools are global singletons),
// which is exactly what this one instance pointer models.
static DetWebServer *s_http_instance = nullptr;
static void detws_http_on_poll(uint8_t slot)
{
    if (s_http_instance)
        s_http_instance->http_poll_slot(slot);
}

int32_t DetWebServer::begin(const WebServerConfig *cfg)
{
    if (_listener_count == 0
#if DETWS_ENABLE_HTTP3
        && !_h3_enabled // an HTTP/3-only server binds UDP, not a TCP listener
#endif
    )
        return DETWS_ERR_NO_LISTENERS;
    DeterministicAsyncTCP::pool_init(cfg);
#if DETWS_ENABLE_AUTH
    regen_digest_secret(); // fresh server keying secret per begin()
#endif
#if DETWS_ENABLE_CSRF
    {
        // Seed the CSRF HMAC secret from the hardware RNG (a fixed dev secret on
        // native/test builds, which have no esp_random).
        uint8_t sec[32];
#ifdef ARDUINO
        for (int i = 0; i < 8; i++)
        {
            uint32_t r = esp_random();
            memcpy(sec + i * 4, &r, 4);
        }
#else
        for (int i = 0; i < 32; i++)
            sec[i] = (uint8_t)(0xA5 ^ i);
#endif
        csrf_set_secret(sec, sizeof(sec));
    }
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
#if DETWS_ENABLE_HTTP3
    // Bind the HTTP/3 QUIC server (UDP on device; on host it is fed via quic_server_ingest). Requests
    // dispatch through this instance's routes via the trampoline; quic_server_poll() runs in service_once.
    if (_h3_enabled)
    {
        QuicServerConfig h3cfg;
        memset(&h3cfg, 0, sizeof(h3cfg));
        h3cfg.cert_der = _h3_cert;
        h3cfg.cert_len = _h3_cert_len;
        memcpy(h3cfg.ed25519_seed, _h3_seed, sizeof(h3cfg.ed25519_seed));
        h3cfg.rng = detws_h3_rng;
        s_h3_running = quic_server_begin(_h3_port, &h3cfg, detws_h3_request_trampoline, this);
    }
#endif
#ifdef ARDUINO
    // Routes/listeners are now fixed; start the worker task(s) that drive the
    // pipeline off the user's loop(). On host the pipeline runs inline via handle().
    s_worker_server = this;
    detws_workers_start(detws_pump_trampoline);
#endif
    return DETWS_OK;
}

int32_t DetWebServer::begin(uint16_t port, const WebServerConfig *cfg)
{
    int32_t rc = listen(port);
    if (rc < 0)
        return rc;
    return begin(cfg);
}

#if DETWS_ENABLE_HTTP3
bool DetWebServer::h3_cert(const uint8_t *cert_der, size_t cert_len, const uint8_t ed25519_seed[32], uint16_t port)
{
    if (!cert_der || cert_len == 0 || !ed25519_seed)
        return false;
    _h3_cert = cert_der;
    _h3_cert_len = cert_len;
    memcpy(_h3_seed, ed25519_seed, sizeof(_h3_seed));
    _h3_port = port;
    _h3_enabled = true;
    return true;
}

// Response sink for the HTTP/3 dispatch slot: route (code, content_type, body) onto the QUIC stream
// the request arrived on (ids stashed on the slot by dispatch_h3_request). Installed as conn->resp_sink
// so send()/send_empty() stay protocol-agnostic.
static bool h3_resp_sink(uint8_t slot, int code, const char *content_type, const char *body, size_t len)
{
    TcpConn *c = &conn_pool[slot];
    return quic_server_respond(c->h3_conn_id, c->h3_stream, code, content_type, (const uint8_t *)body, len);
}

void DetWebServer::dispatch_h3_request(uint32_t conn_id, uint64_t stream_id, const char *method, const char *path,
                                       const char *authority, const uint8_t *body, size_t body_len)
{
    const uint8_t slot = DETWS_H3_DISPATCH_SLOT;
    HttpReq *r = &http_pool[slot];
    http_reset(slot);

    // Map the semantic request fields into the shared HttpReq (as h2_server does per stream).
    size_t mn = strlen(method);
    if (mn >= sizeof(r->method))
        mn = sizeof(r->method) - 1;
    memcpy(r->method, method, mn);
    r->method[mn] = 0;

    const char *q = strchr(path, '?');
    size_t plen = q ? (size_t)(q - path) : strlen(path);
    if (plen >= sizeof(r->path))
        plen = sizeof(r->path) - 1;
    memcpy(r->path, path, plen);
    r->path[plen] = 0;
    r->path_idx = strlen(r->path);
    if (q)
    {
        size_t ql = strlen(q + 1);
        if (ql >= sizeof(r->query))
            ql = sizeof(r->query) - 1;
        memcpy(r->query, q + 1, ql);
        r->query[ql] = 0;
        r->query_idx = strlen(r->query);
    }

    // :authority maps to Host, the way the h2 bridge does.
    if (authority && authority[0] && r->header_count < MAX_HEADERS)
    {
        Header *h = &r->headers[r->header_count++];
        memcpy(h->key, "host", 5);
        size_t vl = strlen(authority);
        if (vl >= sizeof(h->val))
            vl = sizeof(h->val) - 1;
        memcpy(h->val, authority, vl);
        h->val[vl] = 0;
    }

    if (body && body_len)
    {
        size_t n = body_len > BODY_BUF_SIZE ? BODY_BUF_SIZE : body_len;
        memcpy(r->body, body, n);
        r->body_len = n;
        r->body[r->body_len] = 0;
        r->body_bytes_read = body_len;
        r->content_length = body_len;
    }
    r->parse_state = PARSE_COMPLETE;

    // Mark the reserved slot as HTTP/3 and install the response sink so send() / send_empty() route the
    // response back onto this stream (no TCP pcb here - the sink owns the QUIC framing).
    TcpConn *c = &conn_pool[slot];
    c->h3 = 1;
    c->h3_conn_id = conn_id;
    c->h3_stream = stream_id;
    c->resp_sink = h3_resp_sink;
    c->iface = DETIFACE_STA;
    c->state = CONN_ACTIVE;
    c->pcb = nullptr;

    match_and_execute(slot); // -> handler -> send() -> resp_sink -> quic_server_respond()

    // Release the dispatch slot for the next request (a no-response handler simply leaves the stream open).
    c->h3 = 0;
    c->resp_sink = nullptr;
    c->state = CONN_FREE;
    http_reset(slot);
}
#endif // DETWS_ENABLE_HTTP3

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

#if DETWS_ENABLE_MTLS
bool DetWebServer::tls_require_client_cert(const uint8_t *ca, size_t ca_len)
{
    return det_tls_set_client_ca(ca, ca_len);
}

int DetWebServer::tls_client_subject(uint8_t slot_id, char *out, size_t out_len)
{
    return det_tls_peer_subject(slot_id, out, out_len);
}
#endif // DETWS_ENABLE_MTLS
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
#ifdef ARDUINO
    // Stop the worker task(s) before tearing down the slots they service.
    detws_workers_stop();
#endif
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

void DetWebServer::set_cache_control(const char *value)
{
    if (!value || value[0] == '\0')
    {
        _cache_control_buf[0] = '\0';
        return;
    }
    snprintf(_cache_control_buf, CACHE_CONTROL_BUF_SIZE, "Cache-Control: %s\r\n", value);
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
#if DETWS_ENABLE_WEBSOCKET
void DetWebServer::ws_dispatch_message(WsConn *ws)
{
    for (uint8_t r = 0; r < _route_count; r++)
        if (_routes[r].type == ROUTE_WS && _routes[r].ws_message)
        {
            _routes[r].ws_message(ws->ws_id);
            break;
        }
}

void DetWebServer::ws_dispatch_close(WsConn *ws)
{
    for (uint8_t r = 0; r < _route_count; r++)
        if (_routes[r].type == ROUTE_WS && _routes[r].ws_close)
        {
            _routes[r].ws_close(ws->ws_id);
            break;
        }
}
#endif // DETWS_ENABLE_WEBSOCKET

void DetWebServer::handle()
{
#ifdef ARDUINO
    // The worker task drives the pipeline on its own core; loop() is freed.
    if (detws_workers_running())
        return;
#endif
    service_once();
}

void DetWebServer::service_once(int worker_id)
{
    // Wire HTTP's instance-bound poll to the server currently being serviced, so the dispatch loop
    // pumps HTTP through the uniform ProtoHandler::on_poll seam (see http_poll_slot). Done here (not
    // just begin()) so test paths that drive service_once() directly also install it, and so it always
    // targets the running instance. Two pointer stores; negligible at poll cadence.
    s_http_instance = this;
    http_proto_set_poll(detws_http_on_poll);

    server_tick(worker_id);

#if DETWS_ENABLE_HTTP3
    // Drive the QUIC/HTTP-3 server: ingest queued datagrams, run the engines (which dispatch requests
    // through this instance's routes), flush replies. One worker owns it, so requests stay single-threaded.
    if (worker_id == 0 && s_h3_running)
        quic_server_poll(detws_millis());
#endif

    for (uint8_t i = 0; i < MAX_CONNS; i++)
    {
        // This worker services only the slots it owns (all of them at N=1).
        if (conn_pool[i].owner != worker_id)
            continue;

        // Ack-on-consume: reopen the TCP receive window by whatever any consumer
        // (HTTP/WS/TLS/service) drained from this slot's ring on the previous pass.
        // Transport owns the window math; we just nudge it once per slot per loop.
        det_conn_ack_consumed(i);

        // Every protocol - HTTP included - is pumped through the one uniform ProtoHandler::on_poll
        // seam (no per-protocol branch here). HTTP's poll is instance-bound (it dispatches into this
        // server's routes), installed at begin() via http_proto_set_poll() -> http_poll_slot(); the
        // singleton pollers (SSH etc.) gate on CONN_ACTIVE inside their own on_poll.
        const ProtoHandler *ph = proto_get(conn_pool[i].proto);
        if (ph && ph->on_poll)
            ph->on_poll(i);
    }

    // Run any callbacks app code deferred to this worker (race-free push path).
    detws_worker_run_deferred(worker_id);
}

// HTTP's instance-bound poll pump. Installed as the HTTP ProtoHandler's on_poll at begin() (via
// http_proto_set_poll) so the worker dispatch loop pumps HTTP through the same uniform seam as every
// other protocol - no HTTP special case in the loop. Runs the file/chunk send pumps, the WebSocket +
// SSE drains, the keep-alive re-parse, and dispatches a completed request into this server's routes.
void DetWebServer::http_poll_slot(uint8_t i)
{
#if DETWS_ENABLE_FILE_SERVING
    // A file response in flight owns the slot: page out the next window and
    // skip the rest of the pipeline until the whole body has been sent.
    if (g_file_send[i].active)
    {
        file_send_pump(i);
        return;
    }
#endif
    // Likewise a chunked response in flight: pull + frame the next window.
    if (g_chunk_send[i].active)
    {
        chunk_send_pump(i);
        return;
    }

#if DETWS_ENABLE_WEBSOCKET
    // WebSocket slot - drain ring buffer and dispatch ready frames
    WsConn *ws = ws_find(i);
    if (ws)
    {
#if DETWS_ENABLE_TLS
        if (conn_pool[i].tls)
        {
            // wss://: the rx ring holds ciphertext, so decrypt records here and
            // feed the frame parser, dispatching each completed frame as it
            // finishes (one TLS record may carry several WS frames).
            uint8_t tbuf[256];
            int n;
            while ((n = det_tls_read(i, tbuf, sizeof(tbuf))) > 0)
            {
                for (int k = 0; k < n; k++)
                {
                    ws_feed_byte(ws, tbuf[k]);
                    if (ws->parse_state == WS_FRAME_READY)
                    {
                        ws_dispatch_message(ws);
                        ws_reset_frame(ws);
                    }
                    else if (ws->parse_state == WS_CLOSED || ws->parse_state == WS_ERROR)
                        break;
                }
                if (ws->parse_state == WS_CLOSED || ws->parse_state == WS_ERROR)
                    break;
            }
            if (ws->parse_state == WS_CLOSED || ws->parse_state == WS_ERROR || n < 0)
            {
                ws_dispatch_close(ws);
                ws_free(i);
                det_conn_abort_slot(i); // transport owns TLS-free + detach + reset + RST
                http_reset(i);
            }
            return;
        }
#endif // DETWS_ENABLE_TLS

        ws_parse(ws);

        if (ws->parse_state == WS_FRAME_READY)
        {
            ws_dispatch_message(ws);
            ws_reset_frame(ws);
        }
        else if (ws->parse_state == WS_CLOSED || ws->parse_state == WS_ERROR)
        {
            ws_dispatch_close(ws);
            ws_free(i);
            // RFC 6455 5.5.1: close the underlying TCP connection after the close
            // handshake. begin_close moves the slot out of CONN_ACTIVE so the
            // post-close bytes are NOT re-parsed as a new HTTP request (the
            // close-frame the WS layer queued still flushes during the dwell).
            det_conn_begin_close(i);
            http_reset(i);
        }
        return; // slot is owned by WS; skip HTTP dispatch
    }
#endif // DETWS_ENABLE_WEBSOCKET

#if DETWS_ENABLE_SSE
    // SSE slot - connection stays open, nothing to parse from client
    if (sse_find(i))
        return;
#endif // DETWS_ENABLE_SSE

#if DETWS_ENABLE_KEEPALIVE
    // Keep-alive: a slot recycled after a response may already hold the next
    // (pipelined) request in its ring buffer with no new EVT_DATA to trigger a
    // parse. Drain it here each tick so it gets dispatched. TLS slots are
    // skipped - their ring holds ciphertext, decrypted in the session layer.
    if (conn_pool[i].state == CONN_ACTIVE && http_pool[i].parse_state != PARSE_COMPLETE
#if DETWS_ENABLE_TLS
        && !conn_pool[i].tls
#endif
    )
        http_parse(i);
#endif

    // HTTP slot
    if (http_pool[i].parse_state == PARSE_COMPLETE)
    {
        match_and_execute(i);
        if (http_pool[i].parse_state == PARSE_COMPLETE)
            http_reset(i);
    }
    else if (http_pool[i].parse_state == PARSE_ERROR)
    {
        send(i, 400, DET_MIME_TEXT_PLAIN, "Bad Request");
    }
    else if (http_pool[i].parse_state == PARSE_ENTITY_TOO_LARGE)
    {
        send(i, 413, DET_MIME_TEXT_PLAIN, "Payload Too Large");
    }
    else if (http_pool[i].parse_state == PARSE_URI_TOO_LONG)
    {
        send(i, 414, DET_MIME_TEXT_PLAIN, "URI Too Long");
    }
}

bool DetWebServer::defer(uint8_t slot, detws_deferred_fn fn, void *arg)
{
    if (slot >= MAX_CONNS)
        return false;
    // Route to the worker that owns the slot so the callback runs single-threaded
    // alongside that slot's own processing.
    return detws_defer(conn_pool[slot].owner, fn, arg);
}

// ---------------------------------------------------------------------------
// Diagnostic endpoint
// ---------------------------------------------------------------------------

#if DETWS_ENABLE_DIAG
void DetWebServer::diag(uint8_t slot_id)
{
    send(slot_id, 200, DET_MIME_JSON, DETWS_DIAG_JSON);
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
    // RFC 6455 4.2.1: the Sec-WebSocket-Key must base64-decode to exactly 16 bytes.
    uint8_t raw[24];
    if (base64_decode(client_key, raw, sizeof(raw)) != 16)
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

    det_conn_send(slot_id, resp, (u16_t)(sizeof(resp) - 1));
    det_conn_flush(slot_id);
    det_conn_begin_close(slot_id); // dwell in CONN_CLOSING until the response drains

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
    int hlen;
#if DETWS_ENABLE_WS_DEFLATE
    // Negotiate permessage-deflate (RFC 7692) if the client offered it. We force
    // no_context_takeover in both directions so each message decompresses
    // independently (the INFLATE window is the message buffer, not a kept window).
    const char *ws_ext = http_get_header(req, "Sec-WebSocket-Extensions");
    bool pmd = ws_ext && strstr(ws_ext, "permessage-deflate");
    hlen = snprintf(hdr, sizeof(hdr),
                    "HTTP/1.1 101 Switching Protocols\r\n"
                    "Upgrade: websocket\r\n"
                    "Connection: Upgrade\r\n"
                    "Sec-WebSocket-Accept: %s\r\n"
                    "%s"
                    "\r\n",
                    accept,
                    pmd ? "Sec-WebSocket-Extensions: permessage-deflate; client_no_context_takeover; "
                          "server_no_context_takeover\r\n"
                        : "");
#else
    hlen = snprintf(hdr, sizeof(hdr),
                    "HTTP/1.1 101 Switching Protocols\r\n"
                    "Upgrade: websocket\r\n"
                    "Connection: Upgrade\r\n"
                    "Sec-WebSocket-Accept: %s\r\n\r\n",
                    accept);
#endif

    det_conn_send(slot_id, hdr, (u16_t)hlen);
    det_conn_flush(slot_id);

    // Reset HTTP parser but keep the TCP slot -- WS owns it now
    http_reset(slot_id);

    WsConn *ws = ws_alloc(slot_id);
    if (!ws)
    {
        // No WS slot available -- abort the connection (transport owns the teardown)
        det_conn_abort_slot(slot_id);
        return false;
    }

#if DETWS_ENABLE_WS_DEFLATE
    ws->pmd = pmd;
#endif
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

    det_conn_send(slot_id, SSE_HDR, (u16_t)(sizeof(SSE_HDR) - 1));
    det_conn_flush(slot_id);

    // Copy the path BEFORE resetting the parser: http_reset() zeroes the whole
    // HttpReq (including req->path), so a pointer into it would dangle. The saved
    // path is what sse_broadcast() matches against.
    char path[MAX_PATH_LEN];
    strncpy(path, req->path, sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';
    http_reset(slot_id);

    SseConn *sse = sse_alloc(slot_id, path);
    if (!sse)
    {
        det_conn_abort_slot(slot_id); // transport owns detach + reset + RST
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

// Send a terminal text/plain error response that closes the connection: the
// status reason (e.g. "405 Method Not Allowed"), one optional pre-formatted extra
// header (CRLF-terminated, e.g. "Allow: GET\r\n"), then Content-Type/Length and
// "Connection: close". Begins the CONN_CLOSING dwell so the bytes drain before
// teardown; HEAD omits the body. One owner for the error-and-close path.
static void send_error_close(uint8_t slot_id, const char *status, const char *extra_hdr, const char *body)
{
    TcpConn *conn = &conn_pool[slot_id];
    if (conn->state != CONN_ACTIVE || conn->pcb == nullptr)
    {
        http_reset(slot_id);
        return;
    }

    int blen = (int)strlen(body);
    char header[RESP_HDR_BUF_SIZE];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 %s\r\n"
                        "%s"
                        "Content-Type: %s\r\n"
                        "Content-Length: %d\r\n"
                        "Connection: close\r\n\r\n",
                        status, extra_hdr ? extra_hdr : "", DET_MIME_TEXT_PLAIN, blen);

    det_conn_send(slot_id, header, (u16_t)hlen);
    if (blen > 0 && !req_is_head(slot_id))
        det_conn_send(slot_id, body, (u16_t)blen);
    det_conn_flush(slot_id);
    det_conn_begin_close(slot_id); // dwell in CONN_CLOSING until the response drains
    http_reset(slot_id);
}

// Send 405 Method Not Allowed with the required Allow header (RFC 7231 §6.5.5).
static void send_method_not_allowed(uint8_t slot_id, const char *allow)
{
    char extra[80];
    snprintf(extra, sizeof(extra), "Allow: %s\r\n", allow);
    send_error_close(slot_id, "405 Method Not Allowed", extra, "Method Not Allowed");
}

#if DETWS_ENABLE_AUTH_LOCKOUT
// The peer's family-tagged source address for the connection in slot_id (unspecified on native /
// no pcb). Used as the auth-lockout bucket key - the full IPv4 or IPv6 address, so a v6 peer is
// never flattened onto a shared v4 bucket nor folded into a collideable hash.
static DetIp lockout_client_ip(uint8_t slot_id)
{
    DetIp ip;
    ip.family = DET_IP_NONE;
    det_conn_remote_addr(slot_id, &ip);
    return ip;
}

// 429 Too Many Requests with Retry-After (auth lockout active). Closes the
// connection - mirrors send_method_not_allowed's PCB lifecycle.
static void send_too_many_requests(uint8_t slot_id, uint32_t retry_after_s)
{
    char extra[40];
    snprintf(extra, sizeof(extra), "Retry-After: %lu\r\n", (unsigned long)retry_after_s);
    send_error_close(slot_id, "429 Too Many Requests", extra, "Too Many Requests");
}
#endif // DETWS_ENABLE_AUTH_LOCKOUT

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

#if DETWS_ENABLE_WEBDAV
    // A WebDAV mount owns its whole subtree and every method on it (including
    // PROPFIND/MKCOL/etc., which parse_method() does not recognize), so intercept
    // before the unknown-method 501 and the normal route loop.
    if (try_serve_dav(slot_id, req))
        return;
#endif

    // CORS preflight
    if (method == HTTP_OPTIONS && _cors_enabled)
    {
        send_empty(slot_id, 204);
        return;
    }

#if DETWS_ENABLE_CSRF
    // Built-in token endpoint: GET /csrf issues a signed token (also set as the
    // csrf cookie) for clients to echo in X-CSRF-Token on state-changing requests.
    if (method == HTTP_GET && strcmp(req->path, "/csrf") == 0)
    {
        char tok[CSRF_TOKEN_BUF];
        if (csrf_issue(tok, sizeof(tok)) > 0)
        {
            set_cookie(slot_id, "csrf", tok, "Path=/; SameSite=Strict");
            char body[CSRF_TOKEN_BUF + 16];
            snprintf(body, sizeof(body), "{\"token\":\"%s\"}", tok);
            send(slot_id, 200, DET_MIME_JSON, body);
        }
        else
        {
            send(slot_id, 500, DET_MIME_TEXT_PLAIN, "CSRF unavailable");
        }
        return;
    }

    // Enforce CSRF on every state-changing method: require a valid signed
    // X-CSRF-Token header (GET / HEAD / OPTIONS are exempt - not state-changing).
    if (method == HTTP_POST || method == HTTP_PUT || method == HTTP_PATCH || method == HTTP_DELETE)
    {
        const char *tok = http_get_header(req, "X-CSRF-Token");
        if (!tok || !csrf_verify(tok))
        {
            send(slot_id, 403, DET_MIME_TEXT_PLAIN, "CSRF token missing or invalid");
            return;
        }
    }
#endif

    // RFC 7230 §3.3.1: reject Transfer-Encoding
    if (http_get_header(req, "Transfer-Encoding") != nullptr)
    {
        send(slot_id, 501, DET_MIME_TEXT_PLAIN, "Not Implemented");
        return;
    }

    // RFC 7231 §6.5.2: a method the server does not implement → 501.
    if (method == HTTP_METHOD_UNKNOWN)
    {
        send(slot_id, 501, DET_MIME_TEXT_PLAIN, "Not Implemented");
        return;
    }

#if DETWS_ENABLE_WEBSOCKET
    const char *upgrade_hdr = http_get_header(req, "Upgrade");
    // RFC 6455 4.2.1: a valid handshake needs Upgrade: websocket AND a Connection
    // header that includes the "Upgrade" token.
    bool is_ws_upgrade = (method == HTTP_GET) && upgrade_hdr && (strcasecmp(upgrade_hdr, "websocket") == 0) &&
                         conn_has_token(http_get_header(req, "Connection"), "upgrade");
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
            if (!is_ws_upgrade)
            {
                send(slot_id, 400, DET_MIME_TEXT_PLAIN, "WebSocket upgrade required");
                return;
            }
            // RFC 6455 §4.2.1: only version 13 is supported; otherwise 426.
            const char *ws_ver = http_get_header(req, "Sec-WebSocket-Version");
            if (!ws_ver || strcmp(ws_ver, "13") != 0)
            {
                ws_send_version_required(slot_id);
                return;
            }
            // A failed upgrade here means a malformed/oversized Sec-WebSocket-Key (a
            // client error, RFC 6455 4.2.1), so answer 400 rather than 503.
            if (!ws_do_upgrade(slot_id, req, r->ws_connect))
                send(slot_id, 400, DET_MIME_TEXT_PLAIN, "Bad WebSocket handshake");
            return;
        }
#endif // DETWS_ENABLE_WEBSOCKET

#if DETWS_ENABLE_SSE
        if (r->type == ROUTE_SSE)
        {
            if (!sse_do_upgrade(slot_id, req, r->sse_connect))
                send(slot_id, 503, DET_MIME_TEXT_PLAIN, "Service Unavailable");
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
#if DETWS_ENABLE_AUTH_LOCKOUT
            DetIp cip = lockout_client_ip(slot_id);
            uint32_t now = (uint32_t)millis();
            uint32_t remain = auth_lockout_remaining_ms(&cip, now);
            if (remain > 0)
            {
                // Address is locked out: 429 + Retry-After, no credential check.
                send_too_many_requests(slot_id, (remain + 999) / 1000);
                return;
            }
#endif
            bool stale = false;
            bool ok = r->auth_digest ? check_digest_auth(slot_id, req, r, &stale) : check_basic_auth(slot_id, req, r);
#if DETWS_ENABLE_AUTH_LOCKOUT
            // A stale-nonce retry carries valid credentials, so it is not a failed
            // attempt: don't count it toward the lockout (nor reset the counter).
            if (ok)
                auth_lockout_succeed(&cip);
            else if (!stale)
                auth_lockout_fail(&cip, now);
#endif
            if (!ok)
            {
                send_unauth(slot_id, r, stale);
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
        send(slot_id, 404, DET_MIME_TEXT_PLAIN, "Not Found");
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
    if (slot_id >= CONN_POOL_SLOTS)
        return; // guard the public entry: never index conn_pool out of range
    TcpConn *conn = &conn_pool[slot_id];
#if DETWS_ENABLE_HTTP2 || DETWS_ENABLE_HTTP3
    // A self-framing protocol (HTTP/2, HTTP/3) installed its own response sink at negotiation /
    // dispatch time; route through it and let it own its framing + connection lifecycle. This runs
    // before the HTTP/1.1 pcb check because that check is a TCP-transport concern (the HTTP/3 slot
    // has no pcb by design, and an h2 connection manages its own).
    if (conn->resp_sink)
    {
        conn->resp_sink(slot_id, code, content_type, payload, strlen(payload));
        return;
    }
#endif
    if (conn->state != CONN_ACTIVE || conn->pcb == nullptr)
    {
        http_reset(slot_id);
        return;
    }

    int payload_len = (int)strlen(payload);

    bool keep;
    const char *cl = resp_conn_hdr(slot_id, &keep);

    char header[RESP_HDR_BUF_SIZE];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 %d %s\r\n"
                        "Content-Type: %s\r\n"
                        "Content-Length: %d\r\n",
                        code, status_text(code), content_type, payload_len);
    hlen = append_resp_trailer(header, sizeof(header), hlen, slot_id, cl);

    // The slot stays CONN_ACTIVE through the write for both paths; resp_end then
    // begins the CONN_CLOSING dwell on the close path (finalized once ACKed).

    bool head = req_is_head(slot_id);

    // HEAD responses carry the headers (incl. Content-Length) but no body. For a
    // body that fits the header scratch, coalesce headers+body into a single send
    // so the response costs one tcpip_thread round-trip instead of two.
    if (!head && payload_len > 0 && (size_t)hlen + (size_t)payload_len <= sizeof(header))
    {
        memcpy(header + hlen, payload, (size_t)payload_len);
        det_conn_send(slot_id, header, (u16_t)(hlen + payload_len));
    }
    else
    {
        det_conn_send(slot_id, header, (u16_t)hlen);
        if (!head && payload_len > 0)
            det_conn_send(slot_id, payload, (u16_t)payload_len);
    }

    resp_end(slot_id, code, payload_len, keep);
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
    if (slot_id >= CONN_POOL_SLOTS)
        return;
    TcpConn *conn = &conn_pool[slot_id];
#if DETWS_ENABLE_HTTP2 || DETWS_ENABLE_HTTP3
    if (conn->resp_sink)
    {
        conn->resp_sink(slot_id, code, "text/plain", "", 0);
        return;
    }
#endif
    if (conn->state != CONN_ACTIVE || conn->pcb == nullptr)
    {
        http_reset(slot_id);
        return;
    }

    bool keep;
    const char *cl = resp_conn_hdr(slot_id, &keep);

    char header[RESP_HDR_BUF_SIZE];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 %d %s\r\n"
                        "Content-Length: 0\r\n",
                        code, status_text(code));
    hlen = append_resp_trailer(header, sizeof(header), hlen, slot_id, cl);

    det_conn_send(slot_id, header, (u16_t)hlen);

    resp_end(slot_id, code, 0, keep);
}

void DetWebServer::redirect(uint8_t slot_id, int code, const char *location)
{
    if (slot_id >= MAX_CONNS)
        return;
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

    bool keep;
    const char *cl = resp_conn_hdr(slot_id, &keep);

    char header[RESP_HDR_BUF_SIZE];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 %d %s\r\n"
                        "Location: %s\r\n"
                        "Content-Length: 0\r\n",
                        code, status_text(code), location);
    hlen = append_resp_trailer(header, sizeof(header), hlen, slot_id, cl);

    det_conn_send(slot_id, header, (u16_t)hlen);

    resp_end(slot_id, code, 0, keep);
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
// Two-pass: pass 1 sizes the body (emit=false), pass 2 streams it (emit=true).
static size_t tmpl_walk(uint8_t slot, const char *tmpl, TemplateVar resolver, bool emit)
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
                if (emit && vlen)
                    det_conn_send(slot, val, (u16_t)vlen);
                p = end + 2;
                continue;
            }
            // Unterminated or over-long placeholder: emit "{{" literally.
            total += 2;
            if (emit)
                det_conn_send(slot, "{{", 2);
            p += 2;
            continue;
        }

        // Literal run up to the next "{{".
        const char *run = p;
        while (*p && !(p[0] == '{' && p[1] == '{'))
            p++;
        size_t rlen = (size_t)(p - run);
        total += rlen;
        if (emit && rlen)
            det_conn_send(slot, run, (u16_t)rlen);
    }
    return total;
}

void DetWebServer::send_template(uint8_t slot_id, int code, const char *content_type, const char *tmpl,
                                 TemplateVar resolver)
{
    if (slot_id >= MAX_CONNS)
        return;
    TcpConn *conn = &conn_pool[slot_id];
    if (conn->state != CONN_ACTIVE || conn->pcb == nullptr)
    {
        http_reset(slot_id);
        return;
    }

    // Pass 1: size the rendered body (no writes).
    size_t body_len = tmpl_walk(slot_id, tmpl, resolver, false);

    bool keep;
    const char *cl = resp_conn_hdr(slot_id, &keep);

    char header[RESP_HDR_BUF_SIZE];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 %d %s\r\n"
                        "Content-Type: %s\r\n"
                        "Content-Length: %d\r\n",
                        code, status_text(code), content_type, (int)body_len);
    hlen = append_resp_trailer(header, sizeof(header), hlen, slot_id, cl);

    bool head = req_is_head(slot_id);

    det_conn_send(slot_id, header, (u16_t)hlen);
    // Pass 2: stream the rendered body (HEAD carries headers only).
    if (!head && body_len > 0)
        tmpl_walk(slot_id, tmpl, resolver, true);

    resp_end(slot_id, code, (int)body_len, keep);
}

// ---------------------------------------------------------------------------
// Chunked (streaming) responses
//
// send_chunked() writes the headers, then pulls the body from a ChunkSource one
// piece at a time, emitting each as an HTTP/1.1 chunk ("<hexlen>\r\n<data>\r\n",
// RFC 7230 §4.1) and finally the terminating "0\r\n\r\n". Like the file pump, the
// body pages across worker loops as the TCP send window drains (chunk_send_pump,
// resumed by the sent callback), so a response is unbounded in constant memory and
// never truncated at the window. The source's ctx must outlive the response (see
// ChunkSource). One chunked response per slot at a time.
// ---------------------------------------------------------------------------

void DetWebServer::send_chunked(uint8_t slot_id, int code, const char *content_type, ChunkSource source, void *ctx)
{
    if (slot_id >= MAX_CONNS)
        return;
    TcpConn *conn = &conn_pool[slot_id];
    if (conn->state != CONN_ACTIVE || conn->pcb == nullptr)
    {
        http_reset(slot_id);
        return;
    }

    bool keep;
    const char *cl = resp_conn_hdr(slot_id, &keep);

    // RFC 7230 3.3.1: chunked is an HTTP/1.1 transfer-coding - it MUST NOT be sent
    // to an HTTP/1.0 (or unknown-version) client. Fall back to a close-delimited
    // body: omit Transfer-Encoding, force Connection: close, stream the body
    // unframed, and signal its end by closing the connection (RFC 7230 3.3.3).
    bool raw = (http_pool[slot_id].version != HTTP_11);

    char header[RESP_HDR_BUF_SIZE];
    int hlen;
    if (raw)
    {
        keep = false; // close-delimited: the connection close IS the message boundary
        cl = "Connection: close\r\n";
        hlen = snprintf(header, sizeof(header),
                        "HTTP/1.0 %d %s\r\n"
                        "Content-Type: %s\r\n",
                        code, status_text(code), content_type);
    }
    else
        hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 %d %s\r\n"
                        "Content-Type: %s\r\n"
                        "Transfer-Encoding: chunked\r\n",
                        code, status_text(code), content_type);
    hlen = append_resp_trailer(header, sizeof(header), hlen, slot_id, cl);

    det_conn_send(slot_id, header, (u16_t)hlen);

    // HEAD carries the headers but no body or terminator.
    if (req_is_head(slot_id) || !source)
    {
        resp_end(slot_id, code, 0, keep);
        return;
    }

    ChunkSend &s = g_chunk_send[slot_id];
    s.source = source;
    s.ctx = ctx;
    s.status = code;
    s.total = 0;
    s.keep = keep;
    s.active = true;
    s.raw = raw;
    chunk_send_pump(slot_id);
}

// Page a pending chunked response: pull pieces from the source and frame them into
// the send window each worker loop, resuming on later loops as the window drains.
void DetWebServer::chunk_send_pump(uint8_t slot_id)
{
    ChunkSend &s = g_chunk_send[slot_id];
    if (!s.active)
        return;

    TcpConn *conn = &conn_pool[slot_id];
    if (conn->state != CONN_ACTIVE || !conn->pcb)
    {
        s.active = false; // connection gone mid-stream
        return;
    }

    // Reserve room for the largest framing around one CHUNK_BUF_SIZE chunk:
    // "<hex>\r\n" (size line) + "\r\n" trailer. 12 bytes covers a 4-hex-digit size.
    // The raw (HTTP/1.0) path writes the body bytes verbatim, so it needs no reserve.
    const u16_t FRAME = s.raw ? 0 : 12;
    for (;;)
    {
        u16_t avail = det_conn_sndbuf(slot_id);
        if (avail <= FRAME)
        {
            det_conn_flush(slot_id); // no room for a useful chunk; resume next loop
            return;
        }
        size_t cap = (size_t)(avail - FRAME);
        if (cap > CHUNK_BUF_SIZE)
            cap = CHUNK_BUF_SIZE;

        uint8_t buf[CHUNK_BUF_SIZE];
        size_t n = s.source(buf, cap, s.ctx);
        if (n == 0)
        {
            if (!s.raw)
                det_conn_send(slot_id, "0\r\n\r\n", 5); // terminating chunk (1.1 only)
            det_conn_flush(slot_id);
            s.active = false;
            resp_end(slot_id, s.status, s.total, s.keep); // raw: keep==false -> connection close ends the body
            return;
        }
        if (n > cap)
            n = cap; // defensive: a misbehaving source must not overrun the window

        if (s.raw)
        {
            det_conn_send(slot_id, buf, (u16_t)n); // close-delimited: no chunk framing
        }
        else
        {
            char szhdr[12];
            int sn = snprintf(szhdr, sizeof(szhdr), "%x\r\n", (unsigned)n);
            det_conn_send(slot_id, szhdr, (u16_t)sn);
            det_conn_send(slot_id, buf, (u16_t)n);
            det_conn_send(slot_id, "\r\n", 2);
        }
        s.total += (int)n;
    }
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
        return DET_MIME_OCTET_STREAM;

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
        return DET_MIME_OCTET_STREAM;
    const char *ext = dot + 1;

    // Case-insensitive compare against a small static table.
    static const struct
    {
        const char *ext;
        const char *type;
    } table[] = {
        {"html", DET_MIME_TEXT_HTML}, {"htm", DET_MIME_TEXT_HTML},  {"css", "text/css"},
        {"js", DET_MIME_JAVASCRIPT},  {"mjs", DET_MIME_JAVASCRIPT}, {"json", DET_MIME_JSON},
        {"xml", "application/xml"},   {"txt", DET_MIME_TEXT_PLAIN}, {"csv", "text/csv"},
        {"svg", "image/svg+xml"},     {"png", "image/png"},         {"jpg", "image/jpeg"},
        {"jpeg", "image/jpeg"},       {"gif", "image/gif"},         {"ico", "image/x-icon"},
        {"webp", "image/webp"},       {"wasm", "application/wasm"}, {"woff", "font/woff"},
        {"woff2", "font/woff2"},      {"ttf", "font/ttf"},          {"pdf", "application/pdf"},
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
    return DET_MIME_OCTET_STREAM;
}

// ---------------------------------------------------------------------------
// Runtime stats endpoint
// ---------------------------------------------------------------------------

#if DETWS_ENABLE_STATS
// The stats body is an editable template asset (src/web/input/DETWS_STATS_JSON.json)
// rendered through the {{name}} engine, like /metrics - values are substituted by
// name, with no printf-format coupling. Snapshot into statics just before the
// (twice-invoked, size + emit) resolver runs.
static char s_s_uptime[12], s_s_requests[12], s_s_2xx[12], s_s_4xx[12], s_s_5xx[12], s_s_active[8], s_s_heap[12];

static const char *stats_var(const char *name)
{
    if (!strcmp(name, "uptime_ms"))
        return s_s_uptime;
    if (!strcmp(name, "requests"))
        return s_s_requests;
    if (!strcmp(name, "http_2xx"))
        return s_s_2xx;
    if (!strcmp(name, "http_4xx"))
        return s_s_4xx;
    if (!strcmp(name, "http_5xx"))
        return s_s_5xx;
    if (!strcmp(name, "active_conns"))
        return s_s_active;
    if (!strcmp(name, "free_heap"))
        return s_s_heap;
    return nullptr;
}

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

    snprintf(s_s_uptime, sizeof(s_s_uptime), "%lu", up);
    snprintf(s_s_requests, sizeof(s_s_requests), "%lu", (unsigned long)_stat_requests);
    snprintf(s_s_2xx, sizeof(s_s_2xx), "%lu", (unsigned long)_stat_2xx);
    snprintf(s_s_4xx, sizeof(s_s_4xx), "%lu", (unsigned long)_stat_4xx);
    snprintf(s_s_5xx, sizeof(s_s_5xx), "%lu", (unsigned long)_stat_5xx);
    snprintf(s_s_active, sizeof(s_s_active), "%d", active);
    snprintf(s_s_heap, sizeof(s_s_heap), "%u", (unsigned)heap);

    send_template(slot_id, 200, DET_MIME_JSON, DETWS_STATS_JSON, stats_var);
}
#endif // DETWS_ENABLE_STATS

#if DETWS_ENABLE_METRICS
// The Prometheus exposition is an editable template asset (src/web/input/
// DETWS_METRICS_PROM.txt) rendered through the {{name}} engine, so values are
// substituted by name (no printf format coupling). metrics() snapshots the live
// values into these statics just before send_template(), which invokes the
// resolver twice (size + emit) - deterministic because the snapshot is fixed.
static char s_m_uptime[12], s_m_requests[12], s_m_2xx[12], s_m_4xx[12], s_m_5xx[12];
static char s_m_active[8], s_m_max[8], s_m_heap[12], s_m_minheap[12], s_m_heapsize[12], s_m_maxalloc[12];

static const char *metrics_var(const char *name)
{
    if (!strcmp(name, "uptime_seconds"))
        return s_m_uptime;
    if (!strcmp(name, "requests_total"))
        return s_m_requests;
    if (!strcmp(name, "resp_2xx"))
        return s_m_2xx;
    if (!strcmp(name, "resp_4xx"))
        return s_m_4xx;
    if (!strcmp(name, "resp_5xx"))
        return s_m_5xx;
    if (!strcmp(name, "active_conns"))
        return s_m_active;
    if (!strcmp(name, "max_conns"))
        return s_m_max;
    if (!strcmp(name, "free_heap"))
        return s_m_heap;
    if (!strcmp(name, "min_free_heap"))
        return s_m_minheap;
    if (!strcmp(name, "heap_size"))
        return s_m_heapsize;
    if (!strcmp(name, "max_alloc_heap"))
        return s_m_maxalloc;
    return nullptr;
}

void DetWebServer::metrics(uint8_t slot_id)
{
    int active = 0;
    for (int i = 0; i < MAX_CONNS; i++)
        if (conn_pool[i].state == CONN_ACTIVE)
            active++;

    unsigned long up = millis();
#ifdef ARDUINO
    uint32_t heap = ESP.getFreeHeap();
    uint32_t min_heap = ESP.getMinFreeHeap();
    uint32_t heap_size = ESP.getHeapSize();
    uint32_t max_alloc = ESP.getMaxAllocHeap();
#else
    uint32_t heap = 0, min_heap = 0, heap_size = 0, max_alloc = 0;
#endif

    snprintf(s_m_uptime, sizeof(s_m_uptime), "%lu", up / 1000UL);
    snprintf(s_m_requests, sizeof(s_m_requests), "%lu", (unsigned long)_stat_requests);
    snprintf(s_m_2xx, sizeof(s_m_2xx), "%lu", (unsigned long)_stat_2xx);
    snprintf(s_m_4xx, sizeof(s_m_4xx), "%lu", (unsigned long)_stat_4xx);
    snprintf(s_m_5xx, sizeof(s_m_5xx), "%lu", (unsigned long)_stat_5xx);
    snprintf(s_m_active, sizeof(s_m_active), "%d", active);
    snprintf(s_m_max, sizeof(s_m_max), "%d", (int)MAX_CONNS);
    snprintf(s_m_heap, sizeof(s_m_heap), "%u", (unsigned)heap);
    snprintf(s_m_minheap, sizeof(s_m_minheap), "%u", (unsigned)min_heap);
    snprintf(s_m_heapsize, sizeof(s_m_heapsize), "%u", (unsigned)heap_size);
    snprintf(s_m_maxalloc, sizeof(s_m_maxalloc), "%u", (unsigned)max_alloc);

    send_template(slot_id, 200, "text/plain; version=0.0.4; charset=utf-8", DETWS_METRICS_PROM, metrics_var);
}
#endif // DETWS_ENABLE_METRICS

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
            det_conn_flush(conn->id);
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
            det_conn_flush(conn->id);
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
        det_conn_flush(conn->id);
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
            det_conn_flush(conn->id);
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
                det_conn_flush(conn->id);
        }
    }
}
#endif // DETWS_ENABLE_SSE

// ---------------------------------------------------------------------------
// Basic Auth helpers
// ---------------------------------------------------------------------------

#if DETWS_ENABLE_AUTH
// One-shot SHA-256 of @p data, written as 64 lowercase hex chars + NUL.
static void sha256_hex(const uint8_t *data, size_t len, char out[65])
{
    uint8_t d[SSH_SHA256_DIGEST_LEN];
    ssh_sha256(data, len, d);
    det_hex_encode(d, SSH_SHA256_DIGEST_LEN, out);
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

void DetWebServer::regen_digest_secret()
{
    // Seed a 128-bit keying secret from the hardware CSPRNG (esp_random() on
    // ESP32; a non-crypto mock on native test builds), folded through SHA-256 with
    // a counter + millis() so even a weak host RNG yields distinct values across
    // calls. The secret keys every timestamped nonce this server issues; it lives
    // only in BSS and is never sent on the wire.
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
    memcpy(_digest_secret, d, sizeof(_digest_secret)); // first 128 bits
}

// Stateless Digest nonce (RFC 7616 3.3): "<issue_ms_hex>.<mac_hex>" where the MAC
// is SHA-256(secret || issue_ms) truncated to 128 bits. The server holds no
// per-nonce state - it recomputes the MAC to authenticate a returned nonce and
// reads the embedded issue time to age it - so the scheme is safe under the
// shared-nothing worker model (the secret is set once at begin(), read-only after).
static uint32_t digest_nonce_mac(const uint8_t *secret, uint32_t issue, char *mac_hex)
{
    uint8_t material[20];
    memcpy(material, secret, 16);
    memcpy(material + 16, &issue, 4); // endian-symmetric: minted and verified the same way
    uint8_t d[SSH_SHA256_DIGEST_LEN];
    ssh_sha256(material, sizeof(material), d);
    det_hex_encode(d, 16, mac_hex); // 16 bytes -> 32 hex chars + NUL
    return issue;
}

void DetWebServer::make_digest_nonce(char *out, size_t cap)
{
    uint32_t issue = detws_millis();
    char issue_hex[9];
    det_hex_encode((const uint8_t *)&issue, 4, issue_hex); // 4 bytes -> 8 hex chars
    char mac_hex[33];
    digest_nonce_mac(_digest_secret, issue, mac_hex);
    snprintf(out, cap, "%s.%s", issue_hex, mac_hex);
}

bool DetWebServer::verify_digest_nonce(const char *nonce, bool *expired)
{
    *expired = false;
    // Expected shape: 8 hex (issue) + '.' + 32 hex (MAC).
    if (strlen(nonce) != 8 + 1 + 32 || nonce[8] != '.')
        return false;
    uint32_t issue;
    if (det_hex_decode(nonce, 8, (uint8_t *)&issue, 4) != 4)
        return false;
    char mac_hex[33];
    digest_nonce_mac(_digest_secret, issue, mac_hex);
    // Constant-time compare of the 32 MAC hex chars: a forged nonce never reveals
    // how many leading characters matched.
    const char *got = nonce + 9;
    uint8_t diff = 0;
    for (int i = 0; i < 32; i++)
        diff |= (uint8_t)(mac_hex[i] ^ got[i]);
    if (diff != 0)
        return false;                      // not a nonce this server minted
    uint32_t age = detws_millis() - issue; // unsigned: tolerant of the 32-bit millis wrap
    *expired = (age > DETWS_DIGEST_NONCE_LIFETIME_MS);
    return true;
}

void DetWebServer::send_unauth(uint8_t slot_id, const Route *r, bool stale)
{
    TcpConn *conn = &conn_pool[slot_id];
    if (conn->state != CONN_ACTIVE || !conn->pcb)
    {
        http_reset(slot_id);
        return;
    }

    char challenge[MAX_AUTH_LEN + 128];
    if (r->auth_digest)
    {
        char nonce[48];
        make_digest_nonce(nonce, sizeof(nonce)); // a fresh, timestamped nonce per challenge
        snprintf(challenge, sizeof(challenge),
                 "WWW-Authenticate: Digest realm=\"%s\", qop=\"auth\", algorithm=SHA-256, nonce=\"%s\"%s\r\n",
                 r->auth_realm, nonce, stale ? ", stale=true" : "");
    }
    else
        snprintf(challenge, sizeof(challenge), "WWW-Authenticate: Basic realm=\"%s\"\r\n", r->auth_realm);

    bool keep;
    const char *cl = resp_conn_hdr(slot_id, &keep);

    static const char body[] = "Unauthorized";
    char header[RESP_HDR_BUF_SIZE];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 401 Unauthorized\r\n"
                        "%s"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: %d\r\n"
                        "%s"
                        "%s\r\n",
                        challenge, (int)(sizeof(body) - 1), _cors_enabled ? _cors_header_buf : "", cl);

    det_conn_send(slot_id, header, (u16_t)hlen);
    if (!req_is_head(slot_id))
        det_conn_send(slot_id, body, (u16_t)(sizeof(body) - 1));

    resp_end(slot_id, 401, (int)(sizeof(body) - 1), keep);
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
bool DetWebServer::check_digest_auth(uint8_t /*slot_id*/, HttpReq *req, const Route *r, bool *stale)
{
    // Use the full-length Authorization capture (the scratch header value is
    // capped at MAX_VAL_LEN, far shorter than a Digest header).
    const char *hdr = req->authorization;
    if (strncmp(hdr, "Digest ", 7) != 0)
        return false;
    const char *d = hdr + 7;

    char username[MAX_AUTH_LEN];
    char nonce[48];
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
    // The nonce must be one this server minted (authentic MAC). A stale (expired)
    // nonce is still authentic - we finish the credential check below and let the
    // caller reissue with stale=true rather than rejecting outright (RFC 7616 3.3).
    bool nonce_expired = false;
    if (!verify_digest_nonce(nonce, &nonce_expired))
        return false;
    if (strcmp(qop, "auth") != 0)
        return false;

    // RFC 7616 3.4: the resource named by the "uri" parameter MUST be the same as the
    // request target; otherwise a Digest response captured for one route could be
    // replayed against another route under the same realm/nonce.
    char target[MAX_PATH_LEN + MAX_QUERY_LEN + 2];
    if (req->query[0])
        snprintf(target, sizeof(target), "%s?%s", req->path, req->query);
    else
        snprintf(target, sizeof(target), "%s", req->path);
    if (strcmp(uri, target) != 0)
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

    char tmp3[65 + 48 + 16 + 64 + 8 + 65 + 8];
    n = snprintf(tmp3, sizeof(tmp3), "%s:%s:%s:%s:%s:%s", ha1, nonce, nc, cnonce, qop, ha2);
    sha256_hex((const uint8_t *)tmp3, (size_t)n, expected);

    if (strcasecmp(expected, response) != 0)
        return false; // wrong credentials - leave *stale untouched (no transparent retry)
    if (nonce_expired)
    {
        // Correct credentials but an aged nonce: signal a transparent retry so the
        // client recomputes against a fresh challenge without re-prompting the user.
        *stale = true;
        return false;
    }
    return true;
}
#endif // DETWS_ENABLE_AUTH

// ---------------------------------------------------------------------------
// File serving
// ---------------------------------------------------------------------------

#if DETWS_ENABLE_RANGE
// Parse a single-range `Range: bytes=...` header value against a resource of
// @p size bytes. Supported forms: "bytes=A-B", "bytes=A-" (A to end) and
// "bytes=-N" (last N bytes). Returns:
//   0  no usable Range header (caller sends a full 200) - absent, malformed, or
//      a multi-range request (RFC 7233 §3.1 permits ignoring it),
//   1  a satisfiable range (writes inclusive [*out_start, *out_end]),
//  -1  a syntactically valid but unsatisfiable range (caller sends 416).
static int parse_byte_range(const char *hdr, size_t size, size_t *out_start, size_t *out_end)
{
    if (!hdr)
        return 0;
    // Require the "bytes=" unit (case-insensitive).
    if (strncasecmp(hdr, "bytes=", 6) != 0)
        return 0;
    const char *p = hdr + 6;
    while (*p == ' ')
        p++;
    if (strchr(p, ',')) // multi-range not supported -> fall back to full 200
        return 0;

    bool have_start = false, have_end = false;
    size_t start = 0, end = 0;
    const size_t SZMAX = (size_t)-1;
    if (*p >= '0' && *p <= '9')
    {
        have_start = true;
        while (*p >= '0' && *p <= '9')
        {
            size_t d = (size_t)(*p++ - '0');
            // Saturate on overflow: a start past SIZE_MAX is past EOF -> 416, never wraps.
            start = (start > (SZMAX - d) / 10) ? SZMAX : start * 10 + d;
        }
    }
    if (*p != '-')
        return 0; // malformed
    p++;
    if (*p >= '0' && *p <= '9')
    {
        have_end = true;
        end = 0;
        while (*p >= '0' && *p <= '9')
        {
            size_t d = (size_t)(*p++ - '0');
            end = (end > (SZMAX - d) / 10) ? SZMAX : end * 10 + d; // saturate -> clamps to last byte
        }
    }
    while (*p == ' ')
        p++;
    if (*p != '\0')
        return 0; // trailing garbage -> ignore the header

    if (!have_start)
    {
        // Suffix form "bytes=-N": the last N bytes.
        if (!have_end || end == 0)
            return -1; // "-" alone, or "-0" -> unsatisfiable
        if (size == 0)
            return -1;
        start = (end >= size) ? 0 : (size - end);
        end = size - 1;
    }
    else
    {
        if (start >= size)
            return -1; // start past EOF -> unsatisfiable
        if (!have_end || end >= size)
            end = size - 1; // open-ended or clamped to last byte
        if (start > end)
            return -1;
    }
    *out_start = start;
    *out_end = end;
    return 1;
}
#endif // DETWS_ENABLE_RANGE

#if DETWS_ENABLE_FILE_SERVING
// HTTP-date helpers (shared by file serving's Last-Modified / If-Modified-Since and
// WebDAV's getlastmodified / creationdate). WEBDAV requires FILE_SERVING, so this is
// the single home for both. Format a time_t as an RFC 1123 GMT date; leaves @p out
// empty when the timestamp is zero/unavailable.
static void http_rfc1123(time_t t, char *out, size_t cap)
{
    out[0] = '\0';
    if (t <= 0)
        return;
    struct tm tmv;
    if (!gmtime_r(&t, &tmv)) // reentrant: never the shared static buffer (worker-safe)
        return;
    strftime(out, cap, "%a, %d %b %Y %H:%M:%S GMT", &tmv);
}

// True if a resource last modified at @p mtime is NOT newer than the client's
// If-Modified-Since date @p ims (RFC 1123 form), i.e. a conditional GET should
// answer 304. Parses the date by hand (sscanf, no stdlib) and compares the two
// broken-down times field by field, so no timegm()/epoch round-trip is needed.
// Returns false (serve 200) when there is no usable date - mtime is 0 (no clock),
// @p ims is absent, or it does not parse.
static bool http_not_modified_since(time_t mtime, const char *ims)
{
    if (mtime <= 0 || !ims)
        return false;
    char mon[4] = {0};
    int day = 0, year = 0, hh = 0, mm = 0, ss = 0;
    // "Sun, 06 Nov 1994 08:49:37 GMT" - skip the weekday, read the rest.
    if (sscanf(ims, "%*3s, %d %3s %d %d:%d:%d", &day, mon, &year, &hh, &mm, &ss) != 6)
        return false;
    static const char MONTHS[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    const char *mp = strstr(MONTHS, mon);
    // Must align to a 3-char month boundary: a malformed token like "ebM" appears in
    // the table at a non-multiple-of-3 offset and would otherwise mis-parse as a month.
    if (!mp || ((mp - MONTHS) % 3) != 0)
        return false;
    int imon = (int)(mp - MONTHS) / 3; // 0-based, matches struct tm tm_mon

    struct tm tf;
    if (!gmtime_r(&mtime, &tf)) // reentrant: never the shared static buffer (worker-safe)
        return false;
    // Compare file (tf) vs If-Modified-Since fields, most significant first.
    int fy = tf.tm_year + 1900;
    if (fy != year)
        return fy < year;
    if (tf.tm_mon != imon)
        return tf.tm_mon < imon;
    if (tf.tm_mday != day)
        return tf.tm_mday < day;
    if (tf.tm_hour != hh)
        return tf.tm_hour < hh;
    if (tf.tm_min != mm)
        return tf.tm_min < mm;
    return tf.tm_sec <= ss;
}

// RFC 9110 13.1.2: If-None-Match comparison. Supports "*" (matches any current
// representation), a comma-separated list of entity-tags, and weak comparison
// (an inbound W/"x" matches our strong "x"). @p etag is our tag, quotes included.
static bool inm_matches(const char *inm, const char *etag)
{
    while (*inm == ' ' || *inm == '\t')
        inm++;
    if (inm[0] == '*')
        return true; // "*" matches the existing representation
    size_t etlen = strlen(etag);
    const char *p = inm;
    while (*p)
    {
        while (*p == ' ' || *p == '\t' || *p == ',')
            p++;
        if (!*p)
            break;
        const char *tag = p;
        if (tag[0] == 'W' && tag[1] == '/') // weak validator: ignore the W/ prefix
            tag += 2;
        if (tag[0] == '"')
        {
            const char *end = strchr(tag + 1, '"');
            if (end)
            {
                size_t tlen = (size_t)(end - tag + 1);
                if (tlen == etlen && strncmp(tag, etag, etlen) == 0)
                    return true;
            }
        }
        const char *comma = strchr(p, ',');
        if (!comma)
            break;
        p = comma + 1;
    }
    return false;
}

void DetWebServer::serve_file_internal(uint8_t slot_id, bool head, fs::FS &file_sys, const char *fs_path,
                                       const char *content_type, const char *content_encoding)
{
    fs::File f = file_sys.open(fs_path, "r");
    if (!f)
    {
        send(slot_id, 404, DET_MIME_TEXT_PLAIN, "Not Found");
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

    bool keep;
    const char *cl = resp_conn_hdr(slot_id, &keep);

    // Optional Content-Encoding line (e.g. gzip for pre-compressed assets).
    char enc_line[40];
    enc_line[0] = '\0';
    if (content_encoding)
        snprintf(enc_line, sizeof(enc_line), "Content-Encoding: %s\r\n", content_encoding);

#if DETWS_ENABLE_ETAG
    // Conditional GET. Strong validator (ETag) from size + mtime; plus a
    // Last-Modified date validator. A conditional request answers 304 when either
    // the client's If-None-Match matches the ETag, or - per RFC 9110, only when no
    // If-None-Match is present - its If-Modified-Since is not older than the file.
    time_t mtime = f.getLastWrite();
    char etag[40];
    snprintf(etag, sizeof(etag), "\"%x-%lx\"", (unsigned)file_size, (unsigned long)mtime);

    char lastmod_line[56];
    lastmod_line[0] = '\0';
    char lm_date[40];
    http_rfc1123(mtime, lm_date, sizeof(lm_date));
    if (lm_date[0])
        snprintf(lastmod_line, sizeof(lastmod_line), "Last-Modified: %s\r\n", lm_date);

    const char *inm = http_get_header(&http_pool[slot_id], "If-None-Match");
    bool not_modified = inm ? inm_matches(inm, etag)
                            : http_not_modified_since(mtime, http_get_header(&http_pool[slot_id], "If-Modified-Since"));
    if (not_modified)
    {
        f.close();
        char h304[RESP_HDR_BUF_SIZE];
        int n304 = snprintf(h304, sizeof(h304), "HTTP/1.1 304 Not Modified\r\nETag: %s\r\n%s%s%s%s\r\n", etag,
                            lastmod_line, _cache_control_buf, _cors_enabled ? _cors_header_buf : "", cl);
        det_conn_send(slot_id, h304, (u16_t)n304);
        resp_end(slot_id, 304, 0, keep);
        return;
    }
    char etag_line[48];
    snprintf(etag_line, sizeof(etag_line), "ETag: %s\r\n", etag);
#else
    const char *etag_line = "";
    const char *lastmod_line = "";
#endif

    // Default: full 200 response covering the whole file.
    int status = 200;
    size_t body_len = file_size;
    size_t body_off = 0; // file offset the body starts at (nonzero for a Range)
    const char *accept_ranges = "";
    char range_line[64];
    range_line[0] = '\0';

#if DETWS_ENABLE_RANGE
    accept_ranges = "Accept-Ranges: bytes\r\n"; // advertise range support on every file response
    size_t r_start = 0, r_end = 0;
    int rr = parse_byte_range(http_get_header(&http_pool[slot_id], "Range"), file_size, &r_start, &r_end);
    if (rr < 0)
    {
        // Unsatisfiable range -> 416 with Content-Range: bytes */<size>.
        f.close();
        char h416[RESP_HDR_BUF_SIZE];
        int n416 = snprintf(h416, sizeof(h416),
                            "HTTP/1.1 416 Range Not Satisfiable\r\n"
                            "Content-Range: bytes */%u\r\n"
                            "Content-Length: 0\r\n"
                            "%s%s\r\n",
                            (unsigned)file_size, _cors_enabled ? _cors_header_buf : "", cl);
        det_conn_send(slot_id, h416, (u16_t)n416);
        resp_end(slot_id, 416, 0, keep);
        return;
    }
    if (rr > 0)
    {
        status = 206;
        body_len = r_end - r_start + 1;
        snprintf(range_line, sizeof(range_line), "Content-Range: bytes %u-%u/%u\r\n", (unsigned)r_start,
                 (unsigned)r_end, (unsigned)file_size);
        f.seek((uint32_t)r_start);
        body_off = r_start;
    }
#endif

    char header[RESP_HDR_BUF_SIZE];
    int hlen =
        snprintf(header, sizeof(header),
                 "HTTP/1.1 %d %s\r\n"
                 "Content-Type: %s\r\n"
                 "Content-Length: %u\r\n"
                 "%s%s%s%s%s%s%s"
                 "%s\r\n",
                 status, status_text(status), content_type, (unsigned)body_len, accept_ranges, range_line, enc_line,
                 etag_line, lastmod_line, _cache_control_buf, _cors_enabled ? _cors_header_buf : "", cl);

    det_conn_send(slot_id, header, (u16_t)hlen);

    // HEAD or empty body: headers only, finish now.
    if (head || body_len == 0)
    {
        f.close();
        resp_end(slot_id, status, 0, keep);
        return;
    }

    // Hand the body to the cross-loop pump: it pages out at most one send-buffer
    // window now and resumes on later loops as the window drains, so a file larger
    // than TCP_SND_BUF is never truncated. The pump owns the file and calls
    // resp_end() at completion - do not close f or end the response here.
    FileSend &s = g_file_send[slot_id];
    s.file = f; // shared handle on ARDUINO; the local f going out of scope keeps it open
    s.off = body_off;
    s.remaining = body_len;
    s.status = status;
    s.total = (int)body_len;
    s.keep = keep;
    s.active = true;
    file_send_pump(slot_id);
}

// Page out a pending file response across worker loops: send up to det_conn_sndbuf()
// bytes now and return; the next loop resumes (woken by the sent callback) until the
// whole body has been queued, then finish the response. Bounded per loop, never
// truncates, never blocks the worker.
void DetWebServer::file_send_pump(uint8_t slot_id)
{
    FileSend &s = g_file_send[slot_id];
    if (!s.active)
        return;

    TcpConn *conn = &conn_pool[slot_id];
    if (conn->state != CONN_ACTIVE || !conn->pcb)
    {
        // Connection went away mid-transfer: drop the source and the continuation.
        s.file.close();
        s.active = false;
        return;
    }

    uint8_t chunk[FILE_CHUNK_SIZE];
    while (s.remaining > 0)
    {
        u16_t avail = det_conn_sndbuf(slot_id);
        if (avail == 0)
        {
            det_conn_flush(slot_id); // push what is queued; resume on a later loop
            return;
        }
        size_t want = s.remaining < sizeof(chunk) ? s.remaining : sizeof(chunk);
        if (want > avail)
            want = avail;
        size_t n = s.file.read(chunk, want);
        if (n == 0)
        {
            s.remaining = 0; // read error / short file: stop (response will be short)
            break;
        }
        if (!det_conn_send(slot_id, chunk, (u16_t)n))
        {
            s.file.seek((uint32_t)s.off); // un-read the bytes that did not go out; retry next loop
            det_conn_flush(slot_id);
            return;
        }
        s.off += n;
        s.remaining -= n;
    }

    // Whole body queued: finish the response (flush, keep-alive/close, log, reset).
    s.file.close();
    s.active = false;
    det_conn_flush(slot_id);
    resp_end(slot_id, s.status, s.total, s.keep);
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
        send(slot_id, 404, DET_MIME_TEXT_PLAIN, "Not Found");
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
        send(slot_id, 404, DET_MIME_TEXT_PLAIN, "Not Found");
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
        send(slot_id, 404, DET_MIME_TEXT_PLAIN, "Not Found");
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

#if DETWS_ENABLE_WEBDAV
// ---------------------------------------------------------------------------
// WebDAV (RFC 4918) - filesystem-backed request handling. The pure core
// (method classification + 207 XML builder + header parsing) lives in
// services/webdav/webdav.{h,cpp} and is host-tested; this part needs a real FS.
// ---------------------------------------------------------------------------

static char g_dav_buf[DETWS_WEBDAV_BUF_SIZE]; // 207 Multi-Status scratch (BSS)

// http_rfc1123() lives in the FILE_SERVING section above (WEBDAV requires
// FILE_SERVING), shared by both; used here for getlastmodified / creationdate.

// Join an FS root and a subpath into @p out (mirrors serve_static_request's
// separator handling). Returns false on overflow.
static bool dav_join(const char *root, const char *sub, char *out, size_t cap)
{
    size_t rlen = strlen(root);
    bool root_slash = (rlen > 0 && root[rlen - 1] == '/');
    if (root_slash && sub[0] == '/')
        sub++;
    bool sub_slash = (sub[0] == '/');
    const char *sep = (root_slash || sub_slash) ? "" : "/";
    int wn = snprintf(out, cap, "%s%s%s", root, sep, sub);
    return wn > 0 && wn < (int)cap;
}

// The basename of an FS entry name (cores differ: name() may be a full path or a
// bare name).
static const char *dav_basename(const char *name)
{
    const char *slash = strrchr(name, '/');
    return slash ? slash + 1 : name;
}

// Recursively delete a file or directory tree (bounded depth). Re-opens the
// directory after each child removal so iteration is never mutated underneath us.
static bool dav_rm_recursive(fs::FS &fsys, const char *path, int depth)
{
    if (depth > 8)
        return false; // refuse pathologically deep trees rather than overflow the stack
    fs::File d = fsys.open(path, "r");
    if (!d)
        return false;
    if (!d.isDirectory())
    {
        d.close();
        return fsys.remove(path);
    }
    for (;;)
    {
        fs::File c = d.openNextFile();
        if (!c)
            break;
        char cp[256];
        int wn = snprintf(cp, sizeof(cp), "%s/%s", path, dav_basename(c.name()));
        c.close();
        if (wn <= 0 || wn >= (int)sizeof(cp))
        {
            d.close();
            return false;
        }
        if (!dav_rm_recursive(fsys, cp, depth + 1))
        {
            d.close();
            return false;
        }
        d.close();
        d = fsys.open(path, "r"); // reset the directory cursor after the deletion
        if (!d)
            return false;
    }
    d.close();
    return fsys.rmdir(path);
}

// Recursively copy a file or directory tree from @p src to @p dst (bounded depth).
// Unlike dav_rm_recursive we cannot re-open + take the first child each step (the
// source is not consumed, so that would loop forever); instead we re-open and skip
// to child #idx, which is safe even if a core invalidates an open dir handle across
// the writes the copy makes to the destination tree.
static bool dav_copy_recursive(fs::FS &fsys, const char *src, const char *dst, int depth)
{
    if (depth > 8)
        return false; // refuse pathologically deep trees rather than overflow the stack

    fs::File s = fsys.open(src, "r");
    if (!s)
        return false;
    if (!s.isDirectory())
    {
        fs::File d = fsys.open(dst, "w");
        if (!d)
        {
            s.close();
            return false;
        }
        uint8_t cbuf[FILE_CHUNK_SIZE];
        size_t cn;
        while ((cn = s.read(cbuf, sizeof(cbuf))) > 0)
            d.write(cbuf, cn);
        s.close();
        d.close();
        return true;
    }
    s.close();

    if (!fsys.mkdir(dst)) // create the destination collection (caller cleared any existing dst)
        return false;

    for (int idx = 0;; idx++)
    {
        fs::File d = fsys.open(src, "r");
        if (!d)
            return false;
        fs::File c;
        for (int i = 0; i <= idx; i++)
        {
            c = d.openNextFile();
            if (!c)
                break;
        }
        if (!c)
        {
            d.close();
            break; // no child at this index - done
        }
        char base[128];
        snprintf(base, sizeof(base), "%s", dav_basename(c.name()));
        c.close();
        d.close();

        char sp[256], dp[256];
        int wn1 = snprintf(sp, sizeof(sp), "%s/%s", src, base);
        int wn2 = snprintf(dp, sizeof(dp), "%s/%s", dst, base);
        if (wn1 <= 0 || wn1 >= (int)sizeof(sp) || wn2 <= 0 || wn2 >= (int)sizeof(dp))
            return false;
        if (!dav_copy_recursive(fsys, sp, dp, depth + 1))
            return false;
    }
    return true;
}

// Map a WebDAV request path to its on-disk path under the mount @p r. Strips the
// mount prefix, rejects traversal, joins onto the FS root, and drops a trailing
// '/'. Returns 0 on success, else the HTTP error code (403 traversal, 414 too
// long) - the single source of truth for the path check, shared by the request
// handler and the streaming-PUT begin hook.
static int dav_resolve_path(const Route *r, const char *reqpath, char *out, size_t cap)
{
    size_t plen = strlen(r->path);
    if (plen > 0 && r->path[plen - 1] == '*')
        plen--;
    const char *sub = (strlen(reqpath) >= plen) ? reqpath + plen : "";
    if (strstr(sub, ".."))
        return 403;
    const char *root = r->static_root ? r->static_root : "";
    if (!dav_join(root, sub, out, cap))
        return 414;
    size_t fpl = strlen(out);
    if (fpl > 1 && out[fpl - 1] == '/')
        out[fpl - 1] = '\0';
    return 0;
}

#if DETWS_ENABLE_STREAM_BODY
// Per-connection streaming-PUT state for WebDAV: each slot streams its body to its
// own file, so concurrent PUTs never clobber one another, and a transfer is never
// bounded by BODY_BUF_SIZE. Indexed by the request's slot (req - http_pool).
static DetWebServer *g_dav_stream_srv = nullptr;
struct DavPut
{
    fs::File file;  ///< destination file for this slot's PUT.
    bool active;    ///< file opened for the current PUT.
    bool error;     ///< a write (or the open) failed.
    bool existed;   ///< target existed before this PUT (204 vs 201).
    size_t written; ///< bytes written so far.
};
static DavPut g_dav_put[MAX_CONNS];

bool DetWebServer::dav_put_begin_tramp(HttpReq *req)
{
    return g_dav_stream_srv && g_dav_stream_srv->dav_stream_put_begin(req);
}
void DetWebServer::dav_put_data_tramp(HttpReq *req, const uint8_t *data, size_t len)
{
    if (g_dav_stream_srv)
        g_dav_stream_srv->dav_stream_put_data(req, data, len);
}
void DetWebServer::dav_put_abort_tramp(HttpReq *req)
{
    // The PUT was torn down before the handler ran: close the half-written file so
    // the handle is not leaked (a leak eventually exhausts LittleFS's open slots).
    uint8_t slot = (uint8_t)(req - http_pool);
    if (slot < MAX_CONNS && g_dav_put[slot].active)
    {
        g_dav_put[slot].file.close();
        g_dav_put[slot].active = false;
    }
}

bool DetWebServer::dav_stream_put_begin(HttpReq *req)
{
    if (strcmp(req->method, "PUT") != 0)
        return false;
    uint8_t slot = (uint8_t)(req - http_pool);
    for (uint8_t i = 0; i < _route_count; i++)
    {
        Route *r = &_routes[i];
        if (!r->is_active || r->type != ROUTE_DAV)
            continue;
        if (!path_matches(r->path, r->is_wildcard, req->path))
            continue;
        if (r->iface_filter != DETIFACE_ANY && r->iface_filter != conn_pool[slot].iface)
            continue;
        if (!r->static_fs)
            return false;
        char fs_path[256];
        if (dav_resolve_path(r, req->path, fs_path, sizeof(fs_path)) != 0)
            return false; // traversal / too long - let it buffer; the handler answers 403/414
        DavPut *d = &g_dav_put[slot];
        d->active = false;
        d->error = false;
        d->written = 0;
        d->existed = r->static_fs->exists(fs_path);
        d->file = r->static_fs->open(fs_path, "w");
        if (d->file)
            d->active = true;
        else
            d->error = true;
        return true; // stream regardless so the body is consumed and the handler replies
    }
    return false;
}

void DetWebServer::dav_stream_put_data(HttpReq *req, const uint8_t *data, size_t len)
{
    uint8_t slot = (uint8_t)(req - http_pool);
    if (slot >= MAX_CONNS)
        return;
    DavPut *d = &g_dav_put[slot];
    if (d->active && !d->error)
    {
        if (d->file.write(data, len) != len)
            d->error = true;
        else
            d->written += len;
    }
}
#endif // DETWS_ENABLE_STREAM_BODY

void DetWebServer::dav(const char *url_prefix, fs::FS &file_sys, const char *fs_root)
{
    if (_route_count >= MAX_ROUTES)
        return;
    Route *r = &_routes[_route_count++];

    char pat[MAX_PATH_LEN];
    size_t n = strlen(url_prefix);
    if (n > 0 && url_prefix[n - 1] == '*')
        snprintf(pat, sizeof(pat), "%s", url_prefix);
    else
        snprintf(pat, sizeof(pat), "%s*", url_prefix);
    fill_route_base(r, pat);
    r->type = ROUTE_DAV;
    r->method = HTTP_GET; // unused: WebDAV dispatch keys off the raw method token
    r->static_fs = &file_sys;
    r->static_root = fs_root;

#if DETWS_ENABLE_STREAM_BODY
    // Stream PUT bodies straight to the file (one global sink; see DETWS_ENABLE_STREAM_BODY).
    g_dav_stream_srv = this;
    http_parser_set_stream_hooks(dav_put_begin_tramp, dav_put_data_tramp, dav_put_abort_tramp);
#endif
}

void DetWebServer::dav_send_status(uint8_t slot_id, int code, const char *extra_headers)
{
    TcpConn *conn = &conn_pool[slot_id];
    if (conn->state != CONN_ACTIVE || conn->pcb == nullptr)
    {
        http_reset(slot_id);
        return;
    }
    bool keep;
    const char *cl = resp_conn_hdr(slot_id, &keep);
    char header[RESP_HDR_BUF_SIZE];
    int hlen = snprintf(header, sizeof(header), "HTTP/1.1 %d %s\r\n%sContent-Length: 0\r\n%s\r\n", code,
                        status_text(code), extra_headers ? extra_headers : "", cl);
    det_conn_send(slot_id, header, (u16_t)hlen);
    resp_end(slot_id, code, 0, keep);
}

bool DetWebServer::try_serve_dav(uint8_t slot_id, HttpReq *req)
{
    for (uint8_t i = 0; i < _route_count; i++)
    {
        Route *r = &_routes[i];
        if (!r->is_active || r->type != ROUTE_DAV)
            continue;
        if (!path_matches(r->path, r->is_wildcard, req->path))
            continue;
        if (r->iface_filter != DETIFACE_ANY && r->iface_filter != conn_pool[slot_id].iface)
            continue;
        serve_dav_request(slot_id, req, r);
        return true;
    }
    return false;
}

void DetWebServer::serve_dav_request(uint8_t slot_id, HttpReq *req, const Route *r)
{
    if (!r->static_fs)
    {
        dav_send_status(slot_id, 404, "");
        return;
    }
    fs::FS &fsys = *r->static_fs;

    char fs_path[256];
    int rc = dav_resolve_path(r, req->path, fs_path, sizeof(fs_path));
    if (rc != 0)
    {
        dav_send_status(slot_id, rc, ""); // 403 traversal / 414 too long
        return;
    }

    // Mount-prefix length and FS root, used by COPY/MOVE to resolve the Destination.
    size_t plen = strlen(r->path);
    if (plen > 0 && r->path[plen - 1] == '*')
        plen--;
    const char *root = r->static_root ? r->static_root : "";

    switch (webdav_method(req->method))
    {
    case DAV_M_OPTIONS:
        add_response_header(slot_id, "DAV", "1, 2");
        add_response_header(slot_id, "Allow",
                            "OPTIONS, GET, HEAD, PUT, DELETE, PROPFIND, PROPPATCH, MKCOL, COPY, MOVE, LOCK, UNLOCK");
        add_response_header(slot_id, "MS-Author-Via", "DAV");
        send_empty(slot_id, 200);
        return;

    case DAV_M_GET:
    case DAV_M_HEAD: {
        fs::File f = fsys.open(fs_path, "r");
        if (!f)
        {
            dav_send_status(slot_id, 404, "");
            return;
        }
        bool isdir = f.isDirectory();
        f.close();
        if (isdir)
        {
            dav_send_status(slot_id, 405, ""); // GET on a collection is not a download
            return;
        }
        serve_file_internal(slot_id, webdav_method(req->method) == DAV_M_HEAD, fsys, fs_path, mime_type(fs_path),
                            nullptr);
        return;
    }

    case DAV_M_PUT: {
#if DETWS_ENABLE_STREAM_BODY
        if (req->body_streaming)
        {
            // The body was written to this slot's file as it arrived (dav_stream_put_*).
            DavPut *d = &g_dav_put[slot_id];
            if (d->active)
            {
                d->file.close();
                d->active = false; // closed here: the abort hook must not double-close
            }
            else
            {
                dav_send_status(slot_id, 409, ""); // parent missing / not writable
                return;
            }
            if (d->error)
            {
                dav_send_status(slot_id, 507, ""); // a write failed (e.g. disk full)
                return;
            }
            dav_send_status(slot_id, d->existed ? 204 : 201, "");
            return;
        }
#endif
        // Buffered fallback (streaming disabled): body bounded by BODY_BUF_SIZE.
        bool existed = fsys.exists(fs_path);
        fs::File f = fsys.open(fs_path, "w");
        if (!f)
        {
            dav_send_status(slot_id, 409, ""); // parent missing / not writable
            return;
        }
        if (req->body_len)
            f.write(req->body, req->body_len);
        f.close();
        dav_send_status(slot_id, existed ? 204 : 201, "");
        return;
    }

    case DAV_M_DELETE: {
        if (!fsys.exists(fs_path))
        {
            dav_send_status(slot_id, 404, "");
            return;
        }
        dav_send_status(slot_id, dav_rm_recursive(fsys, fs_path, 0) ? 204 : 403, "");
        return;
    }

    case DAV_M_MKCOL:
        if (fsys.exists(fs_path))
        {
            dav_send_status(slot_id, 405, ""); // already exists
            return;
        }
        dav_send_status(slot_id, fsys.mkdir(fs_path) ? 201 : 409, "");
        return;

    case DAV_M_COPY:
    case DAV_M_MOVE: {
        const char *dest_hdr = http_get_header(req, "Destination");
        char dest_url[256];
        if (!dest_hdr || !webdav_dest_path(dest_hdr, dest_url, sizeof(dest_url)))
        {
            dav_send_status(slot_id, 400, "");
            return;
        }
        // The destination must live under this same mount.
        if (strncmp(dest_url, r->path, plen) != 0)
        {
            dav_send_status(slot_id, 502, "");
            return;
        }
        const char *dest_sub = dest_url + plen;
        if (strstr(dest_sub, ".."))
        {
            dav_send_status(slot_id, 403, "");
            return;
        }
        char dest_fs[256];
        if (!dav_join(root, dest_sub, dest_fs, sizeof(dest_fs)))
        {
            dav_send_status(slot_id, 414, "");
            return;
        }
        size_t dpl = strlen(dest_fs);
        if (dpl > 1 && dest_fs[dpl - 1] == '/')
            dest_fs[dpl - 1] = '\0';

        const char *ow = http_get_header(req, "Overwrite");
        bool overwrite = !(ow && (ow[0] == 'F' || ow[0] == 'f'));
        bool dest_exists = fsys.exists(dest_fs);
        if (dest_exists && !overwrite)
        {
            dav_send_status(slot_id, 412, "");
            return;
        }

        if (webdav_method(req->method) == DAV_M_MOVE)
        {
            if (dest_exists)
                dav_rm_recursive(fsys, dest_fs, 0); // replace
            bool ok = fsys.rename(fs_path, dest_fs);
            dav_send_status(slot_id, ok ? (dest_exists ? 204 : 201) : 409, "");
            return;
        }

        // COPY: a file or a whole collection (RFC 4918 9.8). Depth applies to a
        // collection source: "0" copies just the collection itself, "infinity"
        // (the default, also when absent) copies the entire tree.
        fs::File src = fsys.open(fs_path, "r");
        if (!src)
        {
            dav_send_status(slot_id, 404, "");
            return;
        }
        bool src_is_dir = src.isDirectory();
        src.close();

        const char *depth_h = http_get_header(req, "Depth");
        bool shallow = depth_h && depth_h[0] == '0'; // Depth: 0

        if (dest_exists)
            dav_rm_recursive(fsys, dest_fs, 0); // overwrite: clear the target first

        bool ok;
        if (src_is_dir && shallow)
            ok = fsys.mkdir(dest_fs); // collection, Depth:0 - just the collection, no members
        else
            ok = dav_copy_recursive(fsys, fs_path, dest_fs, 0);
        dav_send_status(slot_id, ok ? (dest_exists ? 204 : 201) : 409, "");
        return;
    }

    case DAV_M_LOCK: {
        // Advisory lock: issue a synthetic exclusive-write token (NOT enforced).
        unsigned long tok = (unsigned long)millis();
#ifdef ARDUINO
        tok ^= (unsigned long)esp_random();
#endif
        char token[48];
        snprintf(token, sizeof(token), "opaquelocktoken:%08lx-detws", tok);
        snprintf(g_dav_buf, sizeof(g_dav_buf),
                 "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                 "<D:prop xmlns:D=\"DAV:\"><D:lockdiscovery><D:activelock>"
                 "<D:locktype><D:write/></D:locktype>"
                 "<D:lockscope><D:exclusive/></D:lockscope>"
                 "<D:depth>infinity</D:depth><D:timeout>Second-3600</D:timeout>"
                 "<D:locktoken><D:href>%s</D:href></D:locktoken>"
                 "</D:activelock></D:lockdiscovery></D:prop>\n",
                 token);
        // RFC 4918 §10.5: Lock-Token uses a Coded-URL (angle-bracketed).
        char lt[64];
        snprintf(lt, sizeof(lt), "<%s>", token);
        add_response_header(slot_id, "Lock-Token", lt);
        send(slot_id, 200, "application/xml; charset=utf-8", g_dav_buf);
        return;
    }

    case DAV_M_UNLOCK:
        dav_send_status(slot_id, 204, ""); // advisory: nothing to release
        return;

    case DAV_M_PROPFIND: {
        fs::File f = fsys.open(fs_path, "r");
        if (!f)
        {
            dav_send_status(slot_id, 404, "");
            return;
        }
        bool isdir = f.isDirectory();
        uint32_t fsize = (uint32_t)f.size();
        time_t mtime = f.getLastWrite();

        int depth = webdav_depth(http_get_header(req, "Depth"), 1);

        // RFC 4918 9.1.1: this server lists at most one level, so a Depth: infinity
        // PROPFIND is rejected with 403 + the propfind-finite-depth precondition rather
        // than silently returning a partial (one-level) 207 the client would read as
        // complete. Clients wanting a listing use Depth: 0 or 1.
        if (depth == DAV_DEPTH_INFINITY)
        {
            f.close();
            static const char body[] = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
                                       "<D:error xmlns:D=\"DAV:\"><D:propfind-finite-depth/></D:error>\r\n";
            send(slot_id, 403, "application/xml", body);
            return;
        }

        // Self href: the request path, with a trailing '/' for a collection.
        char self_href[MAX_PATH_LEN + 2];
        snprintf(self_href, sizeof(self_href), "%s", req->path);
        size_t sl = strlen(self_href);
        if (isdir && (sl == 0 || self_href[sl - 1] != '/'))
        {
            if (sl + 1 < sizeof(self_href))
            {
                self_href[sl++] = '/';
                self_href[sl] = '\0';
            }
        }

        size_t cap = sizeof(g_dav_buf), len = 0;
        len = webdav_ms_begin(g_dav_buf, cap, len);
        char mt[40];
        http_rfc1123(mtime, mt, sizeof(mt));
        len = webdav_ms_entry(g_dav_buf, cap, len, self_href, isdir, fsize, mt, isdir ? "" : mime_type(fs_path));

        if (isdir && depth >= 1)
        {
            int count = 0;
            for (;;)
            {
                fs::File c = f.openNextFile();
                if (!c)
                    break;
                if (count >= DETWS_WEBDAV_MAX_ENTRIES)
                {
                    c.close();
                    break;
                }
                const char *base = dav_basename(c.name());
                bool cdir = c.isDirectory();
                uint32_t csize = (uint32_t)c.size();
                time_t cmt = c.getLastWrite();
                char chref[MAX_PATH_LEN + 80];
                snprintf(chref, sizeof(chref), "%s%s%s", self_href, base, cdir ? "/" : "");
                char cmtbuf[40];
                http_rfc1123(cmt, cmtbuf, sizeof(cmtbuf));
                c.close();
                size_t before = len;
                len = webdav_ms_entry(g_dav_buf, cap, len, chref, cdir, csize, cmtbuf, cdir ? "" : mime_type(base));
                if (len == before)
                    break; // buffer full - stop listing
                count++;
            }
        }
        f.close();
        len = webdav_ms_end(g_dav_buf, cap, len);
        send(slot_id, 207, "application/xml; charset=utf-8", g_dav_buf);
        return;
    }

    case DAV_M_PROPPATCH: {
        // Read-only properties (no dead-property store): answer 207 with each
        // requested property refused 403, rather than 405 - keeps Explorer/Finder,
        // which PROPPATCH a timestamp right after a PUT, from erroring.
        if (!fsys.exists(fs_path))
        {
            dav_send_status(slot_id, 404, "");
            return;
        }
        size_t n = webdav_proppatch_ms(g_dav_buf, sizeof(g_dav_buf), req->path, (const char *)req->body, req->body_len);
        if (!n)
        {
            dav_send_status(slot_id, 507, ""); // Insufficient Storage: response did not fit the buffer
            return;
        }
        send(slot_id, 207, "application/xml; charset=utf-8", g_dav_buf);
        return;
    }

    case DAV_M_UNSUPPORTED:
    default:
        dav_send_status(
            slot_id, 405,
            "Allow: OPTIONS, GET, HEAD, PUT, DELETE, PROPFIND, PROPPATCH, MKCOL, COPY, MOVE, LOCK, UNLOCK\r\n");
        return;
    }
}
#endif // DETWS_ENABLE_WEBDAV
