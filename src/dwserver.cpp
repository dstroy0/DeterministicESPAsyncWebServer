// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dwserver.cpp
 * @brief Layer 7 (Application) - HTTP routing and request handler implementation.
 *
 * **Dispatch pipeline (called from DetWebServer::handle())**
 * ```
 * handle()
 *   └─ server_tick()                 ← drain FreeRTOS event queue
 *   └─ for each slot:
 *        ParseState::PARSE_COMPLETE          → match_and_execute()
 *        ParseState::PARSE_ERROR             → send(400)
 *        ParseState::PARSE_ENTITY_TOO_LARGE  → send(413)
 *        ParseState::PARSE_URI_TOO_LONG      → send(414)
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
 * it detaches the pcb from its lwIP callbacks and sets the slot `ConnState::CONN_FREE`
 * (pcb nulled) BEFORE the FIN/RST, on the captured pcb pointer. This means any
 * lwIP error callback that fires mid-teardown sees the slot as already free and
 * takes no action - preventing a double-free. L7 passes only the slot index:
 * det_conn_close(slot) for a graceful local close, det_conn_abort_slot(slot)
 * for a hard RST, det_conn_begin_close(slot) for the drain-then-close dwell.
 */

#include "dwserver.h"
#include "server/dwserver_internal.h"                  // shared helpers exposed to the src/server/*.cpp handlers
#include "network_drivers/presentation/presentation.h" // http_proto_set_poll (install the instance-bound HTTP poll)
#include "network_drivers/session/proto_handler.h"
#include "network_drivers/session/worker.h"
#include "network_drivers/tls/tls.h"
#include "network_drivers/transport/listener.h"
#include "shared_primitives/hex.h"
#include "shared_primitives/mime.h"
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
#include "services/clock.h" // detws_millis() for the stateless Digest nonce timestamp
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
#if DETWS_ENABLE_TIME_SOURCE
#include "services/time_source/time_source.h" // detws_time_http_date() - any NTP/GPS/RTC/... source
#else
#include "services/ntp_service/ntp_service.h" // detws_ntp_http_date() - direct NTP (or the host test seam)
#endif
#endif
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#if DETWS_ENABLE_WEBSOCKET
// Magic GUID concatenated to the client key for the WS accept hash (RFC 6455 §4.2.2)
static const char WS_MAGIC[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
#endif

// The outbound-transfer continuation types (FileSend/ChunkSend/SendCtx) live in
// server/dwserver_internal.h so the split file_serving / chunked handler TUs share the same
// per-slot state. This is the single owning definition of that state (external linkage, but the
// sole definition - the one named owner).
SendCtx s_send;

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
// (det_conn_send / det_conn_flush / det_conn_close, see tcp.h) so this
// application layer never calls lwIP directly.

const char *status_text(int code)
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
 * Returns HttpVersion::HTTP_UNKNOWN for any method the server does not implement, so the
 * dispatcher can answer 501 Not Implemented (RFC 7231 §6.5.2) instead of
 * silently treating it as GET.
 *
 * @param m Null-terminated method string, e.g. "POST".
 * @return Matching HttpMethod enum value, or HttpVersion::HTTP_UNKNOWN.
 */
static HttpMethod parse_method(const char *m)
{
    if (strcmp(m, "GET") == 0)
        return HttpMethod::HTTP_GET;
    if (strcmp(m, "POST") == 0)
        return HttpMethod::HTTP_POST;
    if (strcmp(m, "PUT") == 0)
        return HttpMethod::HTTP_PUT;
    if (strcmp(m, "DELETE") == 0)
        return HttpMethod::HTTP_DELETE;
    if (strcmp(m, "PATCH") == 0)
        return HttpMethod::HTTP_PATCH;
    if (strcmp(m, "HEAD") == 0)
        return HttpMethod::HTTP_HEAD;
    if (strcmp(m, "OPTIONS") == 0)
        return HttpMethod::HTTP_OPTIONS;
    return HttpMethod::HTTP_METHOD_UNKNOWN;
}

/**
 * @brief Canonical method token for an HttpMethod (for the Allow header).
 */
static const char *method_name(HttpMethod m)
{
    switch (m)
    {
    case HttpMethod::HTTP_GET:
        return "GET";
    case HttpMethod::HTTP_POST:
        return "POST";
    case HttpMethod::HTTP_PUT:
        return "PUT";
    case HttpMethod::HTTP_DELETE:
        return "DELETE";
    case HttpMethod::HTTP_PATCH:
        return "PATCH";
    case HttpMethod::HTTP_HEAD:
        return "HEAD";
    case HttpMethod::HTTP_OPTIONS:
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
    _stat_requests = 0;
    _stat_2xx = 0;
    _stat_4xx = 0;
    _stat_5xx = 0;
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
    size_t tlen = strnlen(token, 32);
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
    if (req->parse_state != ParseState::PARSE_COMPLETE)
        return false;

    const char *c = http_get_header(req, "Connection");
    bool keep;
    if (req->version == HttpVersion::HTTP_11)
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

// Finish a response: flush, then either begin the graceful ConnState::CONN_CLOSING dwell
// (close path) or leave the slot active for reuse (keep-alive). The HTTP parser
// is reset either way, returning a kept-alive slot to ParseState::PARSE_METHOD ready for the
// next request. The slot stays ConnState::CONN_ACTIVE through the write for BOTH paths
// (callbacks live - the model keep-alive has always used); the close path then
// dwells in ConnState::CONN_CLOSING from here, so the slot is reclaimed only once the peer
// ACKs the response (or the CLOSING timeout fires), not before it is delivered.
// Every response path now addresses the connection by slot alone - the transport
// resolves the pcb internally, the same way the RX read path does (no pcb is
// threaded through the app layer, so the send target can never disagree).
void DetWebServer::resp_end(uint8_t slot_id, int code, int body_len, bool keep)
{
    det_conn_flush(slot_id);
    if (!keep)
        det_conn_begin_close(slot_id); // ACTIVE -> ConnState::CONN_CLOSING; finalizes on ACK
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
    // RFC 7231 7.1.1.2: emit Date only when a real wall-clock time exists; a clock-less device (no
    // synced/valid time source yet) omits it. The time comes from the multi-source registry (any
    // enabled NTP / GPS / RTC / ... by priority) when DETWS_ENABLE_TIME_SOURCE is set, else straight
    // from NTP.
    char date_hdr[48] = "";
    char imf[40];
#if DETWS_ENABLE_TIME_SOURCE
    if (detws_time_http_date(imf, sizeof(imf)) > 0)
#else
    if (detws_ntp_http_date(imf, sizeof(imf)) > 0)
#endif
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

// Run the chain in registration order. The first middleware to return MwResult::MW_HALT
// stops dispatch; it is responsible for having sent a response.
bool DetWebServer::run_middleware(uint8_t slot_id, HttpReq *req)
{
    for (uint8_t i = 0; i < _middleware_count; i++)
    {
        if (_middleware[i] && _middleware[i](slot_id, req) == MwResult::MW_HALT)
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
        return (int32_t)DetWebServerResult::DETWS_ERR_LISTENER_FULL;
    _listen_ports[_listener_count] = port;
    _listen_protos[_listener_count] = proto;
    _listen_tls[_listener_count] = false;
    _listener_count++;
    // Return the listener id (its index), not DetWebServerResult::DETWS_OK: begin() binds listener_pool[i] from
    // _listen_ports[i] and the accept path stamps that same index onto the slot, so this id is what
    // det_relay_publish() / ssh_forward_begin() must match against. (Errors are negative.)
    return (int32_t)(_listener_count - 1);
}

// Server instance bindings, owned by one instance (internal linkage): the instance whose
// pipeline the worker task pumps, the HTTP/3-running flag, and the instance the HTTP on_poll
// forwarder dispatches into. The library serves from a single DetWebServer (the slot pools are
// global singletons), which is exactly what these instance pointers model. One named owner,
// unreachable from any other translation unit. Set in begin().
struct InstanceCtx
{
    DetWebServer *worker_server = nullptr;
#if DETWS_ENABLE_HTTP3
    bool h3_running = false;
#endif
    DetWebServer *http_instance = nullptr;
};
static InstanceCtx s_inst;
#ifdef ARDUINO
// The worker task's per-tick entry (registered with detws_workers_start below); ESP32-only, so it is
// compiled only where it is used - on host the pipeline runs inline via handle().
static void detws_pump_trampoline(int worker_id)
{
    if (s_inst.worker_server)
        s_inst.worker_server->service_once(worker_id);
}
#endif

#if DETWS_ENABLE_HTTP3
// The quic_server request seam has no DetWebServer type; this trampoline forwards a completed
// HTTP/3 request into the instance's shared route dispatcher (app == the DetWebServer *).
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
static void detws_http_on_poll(uint8_t slot)
{
    if (s_inst.http_instance)
        s_inst.http_instance->http_poll_slot(slot);
}

int32_t DetWebServer::begin(const WebServerConfig *cfg)
{
    if (_listener_count == 0
#if DETWS_ENABLE_HTTP3
        && !_h3_enabled // an HTTP/3-only server binds UDP, not a TCP listener
#endif
    )
        return (int32_t)DetWebServerResult::DETWS_ERR_NO_LISTENERS;
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
            return (int32_t)DetWebServerResult::DETWS_ERR_LISTEN_FAILED;
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
        s_inst.h3_running = quic_server_begin(_h3_port, &h3cfg, detws_h3_request_trampoline, this);
    }
#endif
#ifdef ARDUINO
    // Routes/listeners are now fixed; start the worker task(s) that drive the
    // pipeline off the user's loop(). On host the pipeline runs inline via handle().
    s_inst.worker_server = this;
    detws_workers_start(detws_pump_trampoline);
#endif
    return (int32_t)DetWebServerResult::DETWS_OK;
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
    r->parse_state = ParseState::PARSE_COMPLETE;

    // Mark the reserved slot as HTTP/3 and install the response sink so send() / send_empty() route the
    // response back onto this stream (no TCP pcb here - the sink owns the QUIC framing).
    TcpConn *c = &conn_pool[slot];
    c->h3 = 1;
    c->h3_conn_id = conn_id;
    c->h3_stream = stream_id;
    c->resp_sink = h3_resp_sink;
    c->iface = DetIface::DETIFACE_STA;
    c->state = ConnState::CONN_ACTIVE;
    c->pcb = nullptr;

    match_and_execute(slot); // -> handler -> send() -> resp_sink -> quic_server_respond()

    // Release the dispatch slot for the next request (a no-response handler simply leaves the stream open).
    c->h3 = 0;
    c->resp_sink = nullptr;
    c->state = ConnState::CONN_FREE;
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
        return (int32_t)DetWebServerResult::DETWS_ERR_LISTENER_FULL;
    _listen_ports[_listener_count] = port;
    _listen_protos[_listener_count] = ConnProto::PROTO_HTTP;
    _listen_tls[_listener_count] = true;
    _listener_count++;
    return (int32_t)DetWebServerResult::DETWS_OK;
}

int32_t DetWebServer::begin_tls(uint16_t port, const uint8_t *cert, size_t cert_len, const uint8_t *key, size_t key_len,
                                const WebServerConfig *cfg)
{
    if (!tls_cert(cert, cert_len, key, key_len))
        return (int32_t)DetWebServerResult::DETWS_ERR_LISTEN_FAILED;
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
        return (int32_t)DetWebServerResult::DETWS_ERR_NO_LISTENERS;
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
void fill_route_base(Route *r, const char *path)
{
    strncpy(r->path, path, MAX_PATH_LEN - 1);
    r->path[MAX_PATH_LEN - 1] = '\0';
    r->is_active = true;
    size_t len = strnlen(r->path, MAX_PATH_LEN);
    r->is_wildcard = (len > 0 && r->path[len - 1] == '*');
    r->is_param = (strstr(r->path, "/:") != nullptr);
    r->is_regex = false;
    r->iface_filter = DetIface::DETIFACE_ANY;
}

void DetWebServer::on(const char *path, HttpMethod method, Handler callback)
{
    if (_route_count >= MAX_ROUTES)
        return;
    Route *r = &_routes[_route_count++];
    fill_route_base(r, path);
    r->type = RouteType::ROUTE_HTTP;
    r->method = method;
    r->callback = callback;
}

void DetWebServer::on(const char *path, HttpMethod method, Handler callback, DetIface iface)
{
    if (_route_count >= MAX_ROUTES)
        return;
    Route *r = &_routes[_route_count++];
    fill_route_base(r, path);
    r->type = RouteType::ROUTE_HTTP;
    r->method = method;
    r->callback = callback;
    r->iface_filter = iface;
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
    r->type = RouteType::ROUTE_HTTP;
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
    r->type = RouteType::ROUTE_HTTP;
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
    r->type = RouteType::ROUTE_WS;
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
    r->type = RouteType::ROUTE_SSE;
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
    size_t prefix_len = strnlen(route, MAX_PATH_LEN) - 1;
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
 *   2. Walks all slots; any in ParseState::PARSE_COMPLETE is dispatched via match_and_execute().
 *   3. Any slot left in ParseState::PARSE_COMPLETE after dispatch (i.e., callback did not
 *      send a response) is reset so it doesn't block the slot.
 *   4. Any slot in ParseState::PARSE_ERROR receives an automatic 400 response.
 *   5. Any slot in ParseState::PARSE_ENTITY_TOO_LARGE receives an automatic 413 response.
 *   6. Any slot in ParseState::PARSE_URI_TOO_LONG receives an automatic 414 response.
 */
#if DETWS_ENABLE_WEBSOCKET
void DetWebServer::ws_dispatch_message(WsConn *ws)
{
    for (uint8_t r = 0; r < _route_count; r++)
        if (_routes[r].type == RouteType::ROUTE_WS && _routes[r].ws_message)
        {
            _routes[r].ws_message(ws->ws_id);
            break;
        }
}

void DetWebServer::ws_dispatch_close(WsConn *ws)
{
    for (uint8_t r = 0; r < _route_count; r++)
        if (_routes[r].type == RouteType::ROUTE_WS && _routes[r].ws_close)
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
    s_inst.http_instance = this;
    http_proto_set_poll(detws_http_on_poll);

    server_tick(worker_id);

#if DETWS_ENABLE_HTTP3
    // Drive the QUIC/HTTP-3 server: ingest queued datagrams, run the engines (which dispatch requests
    // through this instance's routes), flush replies. One worker owns it, so requests stay single-threaded.
    if (worker_id == 0 && s_inst.h3_running)
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
        // singleton pollers (SSH etc.) gate on ConnState::CONN_ACTIVE inside their own on_poll.
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
    if (s_send.file[i].active)
    {
        file_send_pump(i);
        return;
    }
#endif
    // Likewise a chunked response in flight: pull + frame the next window.
    if (s_send.chunk[i].active)
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
                    if (ws->parse_state == WsParseState::WS_FRAME_READY)
                    {
                        ws_dispatch_message(ws);
                        ws_reset_frame(ws);
                    }
                    else if (ws->parse_state == WsParseState::WS_CLOSED || ws->parse_state == WsParseState::WS_ERROR)
                        break;
                }
                if (ws->parse_state == WsParseState::WS_CLOSED || ws->parse_state == WsParseState::WS_ERROR)
                    break;
            }
            if (ws->parse_state == WsParseState::WS_CLOSED || ws->parse_state == WsParseState::WS_ERROR || n < 0)
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

        if (ws->parse_state == WsParseState::WS_FRAME_READY)
        {
            ws_dispatch_message(ws);
            ws_reset_frame(ws);
        }
        else if (ws->parse_state == WsParseState::WS_CLOSED || ws->parse_state == WsParseState::WS_ERROR)
        {
            ws_dispatch_close(ws);
            ws_free(i);
            // RFC 6455 5.5.1: close the underlying TCP connection after the close
            // handshake. begin_close moves the slot out of ConnState::CONN_ACTIVE so the
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
    // (pipelined) request in its ring buffer with no new EvtType::EVT_DATA to trigger a
    // parse. Drain it here each tick so it gets dispatched. TLS slots are
    // skipped - their ring holds ciphertext, decrypted in the session layer.
    if (conn_pool[i].state == ConnState::CONN_ACTIVE && http_pool[i].parse_state != ParseState::PARSE_COMPLETE
#if DETWS_ENABLE_TLS
        && !conn_pool[i].tls
#endif
    )
        http_parse(i);
#endif

    // HTTP slot
    if (http_pool[i].parse_state == ParseState::PARSE_COMPLETE)
    {
        match_and_execute(i);
        if (http_pool[i].parse_state == ParseState::PARSE_COMPLETE)
            http_reset(i);
    }
    else if (http_pool[i].parse_state == ParseState::PARSE_ERROR)
    {
        send(i, 400, DET_MIME_TEXT_PLAIN, "Bad Request");
    }
    else if (http_pool[i].parse_state == ParseState::PARSE_ENTITY_TOO_LARGE)
    {
        send(i, 413, DET_MIME_TEXT_PLAIN, "Payload Too Large");
    }
    else if (http_pool[i].parse_state == ParseState::PARSE_URI_TOO_LONG)
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
    if (conn->state != ConnState::CONN_ACTIVE || conn->pcb == nullptr)
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
    det_conn_begin_close(slot_id); // dwell in ConnState::CONN_CLOSING until the response drains

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
    if (conn->state != ConnState::CONN_ACTIVE || !conn->pcb)
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
    if (conn->state != ConnState::CONN_ACTIVE || !conn->pcb)
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
// carry the same headers as GET but no message body (RFC 7231 §4.3.2). External
// linkage (declared in server/dwserver_internal.h): the split handler TUs call it.
bool req_is_head(uint8_t slot_id)
{
    return strcmp(http_pool[slot_id].method, "HEAD") == 0;
}

// Append a method token to a comma-separated Allow list, de-duplicating.
static void allow_append(char *buf, size_t cap, const char *m)
{
    if (!m[0] || strstr(buf, m))
        return;
    size_t len = strnlen(buf, cap);
    if (len == 0)
        snprintf(buf, cap, "%s", m);
    else
        snprintf(buf + len, cap - len, ", %s", m);
}

// Send a terminal text/plain error response that closes the connection: the
// status reason (e.g. "405 Method Not Allowed"), one optional pre-formatted extra
// header (CRLF-terminated, e.g. "Allow: GET\r\n"), then Content-Type/Length and
// "Connection: close". Begins the ConnState::CONN_CLOSING dwell so the bytes drain before
// teardown; HEAD omits the body. One owner for the error-and-close path.
static void send_error_close(uint8_t slot_id, const char *status, const char *extra_hdr, const char *body)
{
    TcpConn *conn = &conn_pool[slot_id];
    if (conn->state != ConnState::CONN_ACTIVE || conn->pcb == nullptr)
    {
        http_reset(slot_id);
        return;
    }

    int blen = (int)strnlen(body, 0xFFFF);
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
    det_conn_begin_close(slot_id); // dwell in ConnState::CONN_CLOSING until the response drains
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
    ip.family = DetIpFamily::DET_IP_NONE;
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
    if (method == HttpMethod::HTTP_OPTIONS && _cors_enabled)
    {
        send_empty(slot_id, 204);
        return;
    }

#if DETWS_ENABLE_CSRF
    // Built-in token endpoint: GET /csrf issues a signed token (also set as the
    // csrf cookie) for clients to echo in X-CSRF-Token on state-changing requests.
    if (method == HttpMethod::HTTP_GET && strcmp(req->path, "/csrf") == 0)
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
    if (method == HttpMethod::HTTP_POST || method == HttpMethod::HTTP_PUT || method == HttpMethod::HTTP_PATCH ||
        method == HttpMethod::HTTP_DELETE)
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
    if (method == HttpMethod::HTTP_METHOD_UNKNOWN)
    {
        send(slot_id, 501, DET_MIME_TEXT_PLAIN, "Not Implemented");
        return;
    }

#if DETWS_ENABLE_WEBSOCKET
    const char *upgrade_hdr = http_get_header(req, "Upgrade");
    // RFC 6455 4.2.1: a valid handshake needs Upgrade: websocket AND a Connection
    // header that includes the "Upgrade" token.
    bool is_ws_upgrade = (method == HttpMethod::HTTP_GET) && upgrade_hdr &&
                         (strcasecmp(upgrade_hdr, "websocket") == 0) &&
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
        if (r->iface_filter != DetIface::DETIFACE_ANY && r->iface_filter != conn_pool[slot_id].iface)
            continue;

#if DETWS_ENABLE_WEBSOCKET
        if (r->type == RouteType::ROUTE_WS)
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
        if (r->type == RouteType::ROUTE_SSE)
        {
            if (!sse_do_upgrade(slot_id, req, r->sse_connect))
                send(slot_id, 503, DET_MIME_TEXT_PLAIN, "Service Unavailable");
            return;
        }
#endif // DETWS_ENABLE_SSE

#if DETWS_ENABLE_FILE_SERVING
        if (r->type == RouteType::ROUTE_STATIC)
        {
            // Static mounts answer GET (and HEAD via GET); other methods → 405.
            if (method != HttpMethod::HTTP_GET && method != HttpMethod::HTTP_HEAD)
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

        // RouteType::ROUTE_HTTP - a HEAD request is served by the GET handler with the
        // response body suppressed (RFC 7231 §4.3.2).
        bool method_ok =
            (r->method == method) || (method == HttpMethod::HTTP_HEAD && r->method == HttpMethod::HTTP_GET);
        if (!method_ok)
        {
            // Path matches but method differs - record it for a 405 + Allow.
            path_matched = true;
            allow_append(allow_buf, sizeof(allow_buf), method_name(r->method));
            // A GET route also answers HEAD, so advertise it in Allow.
            if (r->method == HttpMethod::HTTP_GET)
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
 * `_cors_enabled`.  The slot is freed (state → ConnState::CONN_FREE, pcb → nullptr)
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
    if (conn->state != ConnState::CONN_ACTIVE || conn->pcb == nullptr)
    {
        http_reset(slot_id);
        return;
    }

    int payload_len = (int)strnlen(payload, 0xFFFF);

    bool keep;
    const char *cl = resp_conn_hdr(slot_id, &keep);

    char header[RESP_HDR_BUF_SIZE];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 %d %s\r\n"
                        "Content-Type: %s\r\n"
                        "Content-Length: %d\r\n",
                        code, status_text(code), content_type, payload_len);
    hlen = append_resp_trailer(header, sizeof(header), hlen, slot_id, cl);

    // The slot stays ConnState::CONN_ACTIVE through the write for both paths; resp_end then
    // begins the ConnState::CONN_CLOSING dwell on the close path (finalized once ACKed).

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
    if (conn->state != ConnState::CONN_ACTIVE || conn->pcb == nullptr)
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
    if (conn->state != ConnState::CONN_ACTIVE || conn->pcb == nullptr)
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
                size_t vlen = strnlen(val, 0xFFFF);
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
    if (conn->state != ConnState::CONN_ACTIVE || conn->pcb == nullptr)
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
    if (conn->state != ConnState::CONN_ACTIVE || conn->pcb == nullptr)
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
    bool raw = (http_pool[slot_id].version != HttpVersion::HTTP_11);

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

    ChunkSend &s = s_send.chunk[slot_id];
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
    ChunkSend &s = s_send.chunk[slot_id];
    if (!s.active)
        return;

    TcpConn *conn = &conn_pool[slot_id];
    if (conn->state != ConnState::CONN_ACTIVE || !conn->pcb)
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
    size_t used = strnlen(buf, EXTRA_HDR_BUF_SIZE);
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
    size_t used = strnlen(buf, EXTRA_HDR_BUF_SIZE);
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
struct StatsCtx
{
    char uptime[12];
    char requests[12];
    char n2xx[12];
    char n4xx[12];
    char n5xx[12];
    char active[8];
    char heap[12];
};
static StatsCtx s_stats;

static const char *stats_var(const char *name)
{
    if (!strcmp(name, "uptime_ms"))
        return s_stats.uptime;
    if (!strcmp(name, "requests"))
        return s_stats.requests;
    if (!strcmp(name, "http_2xx"))
        return s_stats.n2xx;
    if (!strcmp(name, "http_4xx"))
        return s_stats.n4xx;
    if (!strcmp(name, "http_5xx"))
        return s_stats.n5xx;
    if (!strcmp(name, "active_conns"))
        return s_stats.active;
    if (!strcmp(name, "free_heap"))
        return s_stats.heap;
    return nullptr;
}

void DetWebServer::stats(uint8_t slot_id)
{
    int active = 0;
    for (int i = 0; i < MAX_CONNS; i++)
        if (conn_pool[i].state == ConnState::CONN_ACTIVE)
            active++;

    unsigned long up = millis();
#ifdef ARDUINO
    uint32_t heap = ESP.getFreeHeap();
#else
    uint32_t heap = 0;
#endif

    snprintf(s_stats.uptime, sizeof(s_stats.uptime), "%lu", up);
    snprintf(s_stats.requests, sizeof(s_stats.requests), "%lu", (unsigned long)_stat_requests);
    snprintf(s_stats.n2xx, sizeof(s_stats.n2xx), "%lu", (unsigned long)_stat_2xx);
    snprintf(s_stats.n4xx, sizeof(s_stats.n4xx), "%lu", (unsigned long)_stat_4xx);
    snprintf(s_stats.n5xx, sizeof(s_stats.n5xx), "%lu", (unsigned long)_stat_5xx);
    snprintf(s_stats.active, sizeof(s_stats.active), "%d", active);
    snprintf(s_stats.heap, sizeof(s_stats.heap), "%u", (unsigned)heap);

    send_template(slot_id, 200, DET_MIME_JSON, DETWS_STATS_JSON, stats_var);
}
#endif // DETWS_ENABLE_STATS

#if DETWS_ENABLE_METRICS
// The Prometheus exposition is an editable template asset (src/web/input/
// DETWS_METRICS_PROM.txt) rendered through the {{name}} engine, so values are
// substituted by name (no printf format coupling). metrics() snapshots the live
// values into these statics just before send_template(), which invokes the
// resolver twice (size + emit) - deterministic because the snapshot is fixed.
struct MetricsCtx
{
    char uptime[12];
    char requests[12];
    char n2xx[12];
    char n4xx[12];
    char n5xx[12];
    char active[8];
    char max[8];
    char heap[12];
    char minheap[12];
    char heapsize[12];
    char maxalloc[12];
};
static MetricsCtx s_metrics;

static const char *metrics_var(const char *name)
{
    if (!strcmp(name, "uptime_seconds"))
        return s_metrics.uptime;
    if (!strcmp(name, "requests_total"))
        return s_metrics.requests;
    if (!strcmp(name, "resp_2xx"))
        return s_metrics.n2xx;
    if (!strcmp(name, "resp_4xx"))
        return s_metrics.n4xx;
    if (!strcmp(name, "resp_5xx"))
        return s_metrics.n5xx;
    if (!strcmp(name, "active_conns"))
        return s_metrics.active;
    if (!strcmp(name, "max_conns"))
        return s_metrics.max;
    if (!strcmp(name, "free_heap"))
        return s_metrics.heap;
    if (!strcmp(name, "min_free_heap"))
        return s_metrics.minheap;
    if (!strcmp(name, "heap_size"))
        return s_metrics.heapsize;
    if (!strcmp(name, "max_alloc_heap"))
        return s_metrics.maxalloc;
    return nullptr;
}

void DetWebServer::metrics(uint8_t slot_id)
{
    int active = 0;
    for (int i = 0; i < MAX_CONNS; i++)
        if (conn_pool[i].state == ConnState::CONN_ACTIVE)
            active++;

    unsigned long up = millis();
#ifdef ARDUINO
    uint32_t heap = ESP.getFreeHeap();
    uint32_t min_heap = ESP.getMinFreeHeap();
    uint32_t heap_size = ESP.getHeapSize();
    uint32_t max_alloc = ESP.getMaxAllocHeap();
#else
    uint32_t heap = 0;
    uint32_t min_heap = 0;
    uint32_t heap_size = 0;
    uint32_t max_alloc = 0;
#endif

    snprintf(s_metrics.uptime, sizeof(s_metrics.uptime), "%lu", up / 1000UL);
    snprintf(s_metrics.requests, sizeof(s_metrics.requests), "%lu", (unsigned long)_stat_requests);
    snprintf(s_metrics.n2xx, sizeof(s_metrics.n2xx), "%lu", (unsigned long)_stat_2xx);
    snprintf(s_metrics.n4xx, sizeof(s_metrics.n4xx), "%lu", (unsigned long)_stat_4xx);
    snprintf(s_metrics.n5xx, sizeof(s_metrics.n5xx), "%lu", (unsigned long)_stat_5xx);
    snprintf(s_metrics.active, sizeof(s_metrics.active), "%d", active);
    snprintf(s_metrics.max, sizeof(s_metrics.max), "%d", (int)MAX_CONNS);
    snprintf(s_metrics.heap, sizeof(s_metrics.heap), "%u", (unsigned)heap);
    snprintf(s_metrics.minheap, sizeof(s_metrics.minheap), "%u", (unsigned)min_heap);
    snprintf(s_metrics.heapsize, sizeof(s_metrics.heapsize), "%u", (unsigned)heap_size);
    snprintf(s_metrics.maxalloc, sizeof(s_metrics.maxalloc), "%u", (unsigned)max_alloc);

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
    if (ws->parse_state == WsParseState::WS_CLOSED || ws->parse_state == WsParseState::WS_ERROR)
        return;
    uint16_t len = (uint16_t)strnlen(text, 0xFFFF);
    if (ws_send_frame(ws, WsOpcode::WS_OP_TEXT, (const uint8_t *)text, len))
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
    if (ws->parse_state == WsParseState::WS_CLOSED || ws->parse_state == WsParseState::WS_ERROR)
        return;
    if (ws_send_frame(ws, WsOpcode::WS_OP_BINARY, data, len))
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
    ws_close(ws, WsCloseCode::WS_CLOSE_NORMAL);
    TcpConn *conn = &conn_pool[ws->slot_id];
    if (conn->pcb)
        det_conn_flush(conn->id);
    // handle() detects WsParseState::WS_CLOSED next tick and fires ws_close callback
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
    size_t klen = strnlen(key, 32);
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
    if (strnlen(nonce, 42) != 8 + 1 + 32 || nonce[8] != '.')
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
    if (conn->state != ConnState::CONN_ACTIVE || !conn->pcb)
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

    return (ulen == strnlen(r->auth_user, MAX_AUTH_LEN)) && (memcmp(decoded, r->auth_user, ulen) == 0) &&
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
