// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dwserver.cpp
 * @brief Layer 7 (Application) - HTTP routing and request handler implementation.
 *
 * **Dispatch pipeline (called from DWS::handle())**
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
 * (dws_conn_send / dws_conn_flush / dws_conn_begin_close / dws_conn_close /
 * dws_conn_abort_slot), so this layer never calls lwIP or touches the raw
 * `tcp_pcb` directly. The transport owns the teardown order for every close:
 * it detaches the pcb from its lwIP callbacks and sets the slot `ConnState::CONN_FREE`
 * (pcb nulled) BEFORE the FIN/RST, on the captured pcb pointer. This means any
 * lwIP error callback that fires mid-teardown sees the slot as already free and
 * takes no action - preventing a double-free. L7 passes only the slot index:
 * dws_conn_close(slot) for a graceful local close, dws_conn_abort_slot(slot)
 * for a hard RST, dws_conn_begin_close(slot) for the drain-then-close dwell.
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
#if DWS_ENABLE_HTTP2
#include "network_drivers/presentation/http2/h2_server.h"
#endif
#if DWS_ENABLE_HTTP3
#include "network_drivers/presentation/http3/quic_server.h"
#endif
#if DWS_ENABLE_WEBSOCKET
#include "network_drivers/presentation/base64/base64.h"
#include "network_drivers/presentation/sha1/sha1.h"
#elif DWS_ENABLE_AUTH
#include "network_drivers/presentation/base64/base64.h"
#endif
#if DWS_ENABLE_AUTH
#include "network_drivers/presentation/ssh/crypto/ssh_sha256.h"
#include "services/clock.h" // dws_millis() for the stateless Digest nonce timestamp
#if DWS_ENABLE_AUTH_LOCKOUT
#include "services/auth_lockout/auth_lockout.h"
#endif
#if DWS_ENABLE_FORWARDED_TRUST
#include "services/forwarded_trust/forwarded_trust.h"
#endif
#ifdef ARDUINO
#include <esp_system.h> // esp_random() for the Digest nonce CSPRNG
#endif
#endif
#if DWS_ENABLE_CSRF
#include "services/csrf/csrf.h"
#ifdef ARDUINO
#include <esp_system.h> // esp_random() for the CSRF HMAC secret
#endif
#endif
#if DWS_ENABLE_WEBDAV
#include "services/webdav/webdav.h"
#include <time.h> // RFC 1123 Last-Modified formatting
#endif
#if DWS_ENABLE_METRICS || DWS_ENABLE_STATS
#include "network_drivers/application/web_assets.h" // DWS_METRICS_PROM / DWS_STATS_JSON (generated)
#endif
#if DWS_HTTP_EMIT_DATE
#if DWS_ENABLE_TIME_SOURCE
#include "services/time_source/time_source.h" // dws_time_http_date() - any NTP/GPS/RTC/... source
#else
#include "services/ntp_service/ntp_service.h" // dws_ntp_http_date() - direct NTP (or the host test seam)
#endif
#endif
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

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
// (dws_conn_send / dws_conn_flush / dws_conn_close, see tcp.h) so this
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
#if DWS_ENABLE_WEBDAV
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
#if DWS_ENABLE_WEBDAV
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

DWS::DWS()
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
#if DWS_ENABLE_AUTH
    regen_digest_secret();
#endif
#if DWS_ENABLE_STATS
    _stat_requests = 0;
    _stat_2xx = 0;
    _stat_4xx = 0;
    _stat_5xx = 0;
#endif
}

void DWS::on_request_log(RequestLogCb cb)
{
    _log_cb = cb;
}

// Record a completed response: bump stats counters and fire the access-log hook.
// The request's method/path are still intact in http_pool[slot_id] (http_reset
// has not run yet at the call sites).
void DWS::note_response(uint8_t slot_id, int code, int body_len)
{
#if DWS_ENABLE_STATS
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

#if DWS_ENABLE_KEEPALIVE || DWS_ENABLE_WEBSOCKET
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
#endif // DWS_ENABLE_KEEPALIVE || DWS_ENABLE_WEBSOCKET

#if DWS_ENABLE_KEEPALIVE
bool DWS::keepalive_eval(uint8_t slot_id)
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

    // Fairness bound: serve at most DWS_KEEPALIVE_MAX_REQUESTS, then close.
    if (++http_req_count[slot_id] >= DWS_KEEPALIVE_MAX_REQUESTS)
        return false;
    return true;
}
#endif // DWS_ENABLE_KEEPALIVE

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
void DWS::dws_resp_end(uint8_t slot_id, int code, int body_len, bool keep, bool pre_flushed)
{
    if (!pre_flushed)
        dws_conn_flush(slot_id); // a pre_flushed caller already did tcp_output in its final send
    if (!keep)
        dws_conn_begin_close(slot_id); // ACTIVE -> ConnState::CONN_CLOSING; finalizes on ACK
    note_response(slot_id, code, body_len);
    http_reset(slot_id);
}

// Resolve the Connection response header (and report keep-alive intent) in one
// place so every response path agrees. Keep-alive compiled out always closes.
const char *DWS::dws_resp_conn_hdr(uint8_t slot_id, bool *keep_out)
{
    bool keep = false;
#if DWS_ENABLE_KEEPALIVE
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
int DWS::append_resp_trailer(char *buf, size_t cap, int hlen, uint8_t slot_id, const char *cl)
{
    if (hlen < 0)
        return 0;
    if ((size_t)hlen >= cap)
        return (int)cap - 1; // status line already filled the buffer (truncated); clamp in-bounds
#if DWS_HTTP_EMIT_DATE
    // RFC 7231 7.1.1.2: emit Date only when a real wall-clock time exists; a clock-less device (no
    // synced/valid time source yet) omits it. The time comes from the multi-source registry (any
    // enabled NTP / GPS / RTC / ... by priority) when DWS_ENABLE_TIME_SOURCE is set, else straight
    // from NTP.
    char date_hdr[48] = "";
    char imf[40];
#if DWS_ENABLE_TIME_SOURCE
    if (dws_time_http_date(imf, sizeof(imf)) > 0)
#else
    if (dws_ntp_http_date(imf, sizeof(imf)) > 0)
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

int32_t DWS::listen(uint16_t port, ConnProto proto)
{
    if (_listener_count >= MAX_LISTENERS)
        return (int32_t)DWSResult::DWS_ERR_LISTENER_FULL;
    _listen_ports[_listener_count] = port;
    _listen_protos[_listener_count] = proto;
    _listen_tls[_listener_count] = false;
    _listener_count++;
    // Return the listener id (its index), not DWSResult::DWS_OK: begin() binds listener_pool[i] from
    // _listen_ports[i] and the accept path stamps that same index onto the slot, so this id is what
    // dws_relay_publish() / dws_ssh_forward_begin() must match against. (Errors are negative.)
    return (int32_t)(_listener_count - 1);
}

// Server instance bindings, owned by one instance (internal linkage): the instance whose
// pipeline the worker task pumps, the HTTP/3-running flag, and the instance the HTTP on_poll
// forwarder dispatches into. The library serves from a single DWS (the slot pools are
// global singletons), which is exactly what these instance pointers model. One named owner,
// unreachable from any other translation unit. Set in begin().
struct InstanceCtx
{
    DWS *worker_server = nullptr;
#if DWS_ENABLE_HTTP3
    bool dws_h3_running = false;
#endif
    DWS *http_instance = nullptr;
};
static InstanceCtx s_inst;
#ifdef ARDUINO
// The worker task's per-tick entry (registered with dws_workers_start below); ESP32-only, so it is
// compiled only where it is used - on host the pipeline runs inline via handle().
static void dws_pump_trampoline(int worker_id)
{
    if (s_inst.worker_server)
        s_inst.worker_server->service_once(worker_id);
}
#endif

#if DWS_ENABLE_HTTP3
// The dws_quic_server request seam has no DWS type; this trampoline forwards a completed
// HTTP/3 request into the instance's shared route dispatcher (app == the DWS *).
static void dws_h3_request_trampoline(void *app, uint32_t conn_id, uint64_t stream_id, const char *method,
                                      const char *path, const char *authority, const uint8_t *body, size_t body_len)
{
    if (app)
        ((DWS *)app)->dispatch_h3_request(conn_id, stream_id, method, path, authority, body, body_len);
}

// Randomness for the QUIC ephemeral X25519 key, the ServerHello random, and our connection IDs: the
// hardware TRNG on device; a deterministic PRNG on host (test builds carry no security context and
// have no esp_random).
static void dws_h3_rng(uint8_t *out, size_t len)
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
#endif // DWS_ENABLE_HTTP3

// HTTP's poll is instance-bound (it dispatches into this server's routes), so it cannot be a plain
// global on_poll like the singleton protocols. begin() records the serving instance here and installs
// this forwarder as the HTTP ProtoHandler's on_poll, so the worker loop pumps HTTP through the one
// uniform seam. The library serves from a single DWS (the slot pools are global singletons),
// which is exactly what this one instance pointer models.
static void dws_http_on_poll(uint8_t slot)
{
    if (s_inst.http_instance)
        s_inst.http_instance->http_poll_slot(slot);
}

int32_t DWS::begin(const WebServerConfig *cfg)
{
    if (_listener_count == 0
#if DWS_ENABLE_HTTP3
        && !_h3_enabled // an HTTP/3-only server binds UDP, not a TCP listener
#endif
    )
        return (int32_t)DWSResult::DWS_ERR_NO_LISTENERS;
    DeterministicAsyncTCP::pool_init(cfg);
#if DWS_ENABLE_AUTH
    regen_digest_secret(); // fresh server keying secret per begin()
#endif
#if DWS_ENABLE_CSRF
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
        dws_csrf_set_secret(sec, sizeof(sec));
    }
#endif
    for (uint8_t i = 0; i < MAX_CONNS; i++)
        http_reset(i);
#if DWS_ENABLE_WEBSOCKET
    ws_init();
#endif
#if DWS_ENABLE_SSE
    dws_sse_init();
#endif
    for (uint8_t i = 0; i < _listener_count; i++)
    {
        if (listener_add(i, _listen_ports[i], _listen_protos[i], _listen_tls[i]) < 0)
            return (int32_t)DWSResult::DWS_ERR_LISTEN_FAILED;
    }
#if DWS_ENABLE_HTTP3
    // Bind the HTTP/3 QUIC server (UDP on device; on host it is fed via dws_quic_server_ingest). Requests
    // dispatch through this instance's routes via the trampoline; dws_quic_server_poll() runs in service_once.
    if (_h3_enabled)
    {
        QuicServerConfig h3cfg;
        memset(&h3cfg, 0, sizeof(h3cfg));
        h3cfg.cert_der = _h3_cert;
        h3cfg.cert_len = _h3_cert_len;
        memcpy(h3cfg.ed25519_seed, _h3_seed, sizeof(h3cfg.ed25519_seed));
        h3cfg.rng = dws_h3_rng;
        s_inst.dws_h3_running = dws_quic_server_begin(_h3_port, &h3cfg, dws_h3_request_trampoline, this);
    }
#endif
#ifdef ARDUINO
    // Routes/listeners are now fixed; start the worker task(s) that drive the
    // pipeline off the user's loop(). On host the pipeline runs inline via handle().
    s_inst.worker_server = this;
    dws_workers_start(dws_pump_trampoline);
#endif
    return (int32_t)DWSResult::DWS_OK;
}

int32_t DWS::begin(uint16_t port, const WebServerConfig *cfg)
{
    int32_t rc = listen(port);
    if (rc < 0)
        return rc;
    return begin(cfg);
}

#if DWS_ENABLE_HTTP3
bool DWS::dws_h3_cert(const uint8_t *cert_der, size_t cert_len, const uint8_t ed25519_seed[32], uint16_t port)
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
// the request arrived on (ids stashed on the slot by dispatch_h3_request). Installed as conn->dws_resp_sink
// so send()/send_empty() stay protocol-agnostic.
static bool dws_h3_resp_sink(uint8_t slot, int code, const char *content_type, const char *body, size_t len)
{
    TcpConn *c = &conn_pool[slot];
    return dws_quic_server_respond(c->dws_h3_conn_id, c->dws_h3_stream, code, content_type, (const uint8_t *)body, len);
}

void DWS::dispatch_h3_request(uint32_t conn_id, uint64_t stream_id, const char *method, const char *path,
                              const char *authority, const uint8_t *body, size_t body_len)
{
    const uint8_t slot = DWS_H3_DISPATCH_SLOT;
    HttpReq *r = &http_pool[slot];
    http_reset(slot);

    // Map the semantic request fields into the shared HttpReq (as dws_h2_server does per stream).
    size_t mn = strnlen(method, sizeof(r->method));
    if (mn >= sizeof(r->method))
        mn = sizeof(r->method) - 1;
    memcpy(r->method, method, mn);
    r->method[mn] = 0;

    const char *q = strchr(path, '?');
    size_t plen = q ? (size_t)(q - path) : strnlen(path, sizeof(r->path));
    if (plen >= sizeof(r->path))
        plen = sizeof(r->path) - 1;
    memcpy(r->path, path, plen);
    r->path[plen] = 0;
    r->path_idx = strnlen(r->path, sizeof(r->path));
    if (q)
    {
        size_t ql = strnlen(q + 1, sizeof(r->query));
        if (ql >= sizeof(r->query))
            ql = sizeof(r->query) - 1;
        memcpy(r->query, q + 1, ql);
        r->query[ql] = 0;
        r->query_idx = strnlen(r->query, sizeof(r->query));
    }

    // :authority maps to Host, the way the h2 bridge does.
    if (authority && authority[0] && r->header_count < MAX_HEADERS)
    {
        Header *h = &r->headers[r->header_count++];
        memcpy(h->key, "host", 5);
        size_t vl = strnlen(authority, sizeof(h->val));
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
    c->dws_h3_conn_id = conn_id;
    c->dws_h3_stream = stream_id;
    c->dws_resp_sink = dws_h3_resp_sink;
    c->iface = DWSIface::DETIFACE_STA;
    c->state = ConnState::CONN_ACTIVE;
    c->pcb = nullptr;

    match_and_execute(slot); // -> handler -> send() -> dws_resp_sink -> dws_quic_server_respond()

    // Release the dispatch slot for the next request (a no-response handler simply leaves the stream open).
    c->h3 = 0;
    c->dws_resp_sink = nullptr;
    c->state = ConnState::CONN_FREE;
    http_reset(slot);
}
#endif // DWS_ENABLE_HTTP3

#if DWS_ENABLE_TLS
bool DWS::tls_cert(const uint8_t *cert, size_t cert_len, const uint8_t *key, size_t key_len)
{
    return dws_tls_global_init(cert, cert_len, key, key_len);
}

int32_t DWS::listen_tls(uint16_t port)
{
    if (_listener_count >= MAX_LISTENERS)
        return (int32_t)DWSResult::DWS_ERR_LISTENER_FULL;
    _listen_ports[_listener_count] = port;
    _listen_protos[_listener_count] = ConnProto::PROTO_HTTP;
    _listen_tls[_listener_count] = true;
    _listener_count++;
    return (int32_t)DWSResult::DWS_OK;
}

int32_t DWS::begin_tls(uint16_t port, const uint8_t *cert, size_t cert_len, const uint8_t *key, size_t key_len,
                       const WebServerConfig *cfg)
{
    if (!tls_cert(cert, cert_len, key, key_len))
        return (int32_t)DWSResult::DWS_ERR_LISTEN_FAILED;
    int32_t rc = listen_tls(port);
    if (rc < 0)
        return rc;
    return begin(cfg);
}

#if DWS_ENABLE_MTLS
bool DWS::tls_require_client_cert(const uint8_t *ca, size_t ca_len)
{
    return dws_tls_set_client_ca(ca, ca_len);
}

int DWS::tls_client_subject(uint8_t slot_id, char *out, size_t out_len)
{
    return dws_tls_peer_subject(slot_id, out, out_len);
}
#endif // DWS_ENABLE_MTLS
#endif // DWS_ENABLE_TLS

int32_t DWS::restart(const WebServerConfig *cfg)
{
    if (_listener_count == 0)
        return (int32_t)DWSResult::DWS_ERR_NO_LISTENERS;
    stop();
    return begin(cfg);
}

void DWS::stop()
{
#ifdef ARDUINO
    // Stop the worker task(s) before tearing down the slots they service.
    dws_workers_stop();
#endif
    listener_stop_all();
    DeterministicAsyncTCP::stop();
    for (uint8_t i = 0; i < MAX_CONNS; i++)
        http_reset(i);
#if DWS_ENABLE_WEBSOCKET
    ws_init();
#endif
#if DWS_ENABLE_SSE
    dws_sse_init();
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
    r->iface_filter = DWSIface::DETIFACE_ANY;
}

void DWS::on(const char *path, HttpMethod method, Handler callback)
{
    if (_route_count >= MAX_ROUTES)
        return;
    Route *r = &_routes[_route_count++];
    fill_route_base(r, path);
    r->type = RouteType::ROUTE_HTTP;
    r->method = method;
    r->callback = callback;
}

void DWS::on(const char *path, HttpMethod method, Handler callback, DWSIface iface)
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

void DWS::set_ap_ip(uint32_t ap_ip)
{
    dws_ap_ip = ap_ip;
}

void DWS::on_regex(const char *pattern, HttpMethod method, Handler callback)
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

#if DWS_ENABLE_AUTH
void DWS::on(const char *path, HttpMethod method, Handler callback, const char *realm, const char *user,
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
#endif // DWS_ENABLE_AUTH

#if DWS_ENABLE_WEBSOCKET
void DWS::on_ws(const char *path, WsConnectHandler on_connect, WsMessageHandler on_message, WsCloseHandler on_close)
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
#endif // DWS_ENABLE_WEBSOCKET

#if DWS_ENABLE_SSE
void DWS::on_sse(const char *path, SseConnectHandler on_connect)
{
    if (_route_count >= MAX_ROUTES)
        return;
    Route *r = &_routes[_route_count++];
    fill_route_base(r, path);
    r->type = RouteType::ROUTE_SSE;
    r->dws_sse_connect = on_connect;
}
#endif // DWS_ENABLE_SSE

void DWS::on_not_found(Handler callback)
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
void DWS::set_cors(const char *origin)
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

void DWS::set_cache_control(const char *value)
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
bool DWS::path_matches(const char *route, bool is_wildcard, const char *req_path)
{
    if (!is_wildcard)
        return strcmp(route, req_path) == 0;

    // Prefix match: compare everything up to (but not including) the '*'
    size_t prefix_len = strnlen(route, MAX_PATH_LEN) - 1;
    return strncmp(route, req_path, prefix_len) == 0;
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
// Record one `:name` path parameter (key from the route segment, value from the path segment), each
// truncated to its buffer. No-op once the param table is full. Extracted to keep the matcher loop flat.
static void capture_path_param(HttpReq *req, const char *key, size_t klen, const char *val, size_t vlen)
{
    if (req->path_param_count >= MAX_PATH_PARAMS)
        return;
    QueryParam *qp = &req->path_params[req->path_param_count++];
    if (klen > QUERY_KEY_LEN - 1)
        klen = QUERY_KEY_LEN - 1;
    memcpy(qp->key, key, klen);
    qp->key[klen] = '\0';
    if (vlen > QUERY_VAL_LEN - 1)
        vlen = QUERY_VAL_LEN - 1;
    memcpy(qp->val, val, vlen);
    qp->val[vlen] = '\0';
}

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
            capture_path_param(req, rseg + 1, rlen - 1, pseg, plen);
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
#if DWS_ENABLE_WEBSOCKET
void DWS::ws_dispatch_message(const WsConn *ws) const
{
    for (uint8_t r = 0; r < _route_count; r++)
        if (_routes[r].type == RouteType::ROUTE_WS && _routes[r].ws_message)
        {
            _routes[r].ws_message(ws->ws_id);
            break;
        }
}

void DWS::ws_dispatch_close(const WsConn *ws) const
{
    for (uint8_t r = 0; r < _route_count; r++)
        if (_routes[r].type == RouteType::ROUTE_WS && _routes[r].ws_close)
        {
            _routes[r].ws_close(ws->ws_id);
            break;
        }
}
#endif // DWS_ENABLE_WEBSOCKET

void DWS::handle()
{
#ifdef ARDUINO
    // The worker task drives the pipeline on its own core; loop() is freed.
    if (dws_workers_running())
        return;
#endif
    service_once();
}

void DWS::service_once(int worker_id)
{
    // Wire HTTP's instance-bound poll to the server currently being serviced, so the dispatch loop
    // pumps HTTP through the uniform ProtoHandler::on_poll seam (see http_poll_slot). Done here (not
    // just begin()) so test paths that drive service_once() directly also install it, and so it always
    // targets the running instance. Two pointer stores; negligible at poll cadence.
    s_inst.http_instance = this;
    http_proto_set_poll(dws_http_on_poll);

    server_tick(worker_id);

#if DWS_ENABLE_HTTP3
    // Drive the QUIC/HTTP-3 server: ingest queued datagrams, run the engines (which dispatch requests
    // through this instance's routes), flush replies. One worker owns it, so requests stay single-threaded.
    if (worker_id == 0 && s_inst.dws_h3_running)
        dws_quic_server_poll(dws_millis());
#endif

    for (uint8_t i = 0; i < MAX_CONNS; i++)
    {
        // This worker services only the slots it owns (all of them at N=1).
        if (conn_pool[i].owner != worker_id)
            continue;

        // Ack-on-consume: reopen the TCP receive window by whatever any consumer
        // (HTTP/WS/TLS/service) drained from this slot's ring on the previous pass.
        // Transport owns the window math; we just nudge it once per slot per loop.
        dws_conn_ack_consumed(i);

        // Every protocol - HTTP included - is pumped through the one uniform ProtoHandler::on_poll
        // seam (no per-protocol branch here). HTTP's poll is instance-bound (it dispatches into this
        // server's routes), installed at begin() via http_proto_set_poll() -> http_poll_slot(); the
        // singleton pollers (SSH etc.) gate on ConnState::CONN_ACTIVE inside their own on_poll.
        const ProtoHandler *ph = proto_get(conn_pool[i].proto);
        if (ph && ph->on_poll)
            ph->on_poll(i);
    }

    // Run any callbacks app code deferred to this worker (race-free push path).
    dws_worker_run_deferred(worker_id);
}

// HTTP's instance-bound poll pump. Installed as the HTTP ProtoHandler's on_poll at begin() (via
// http_proto_set_poll) so the worker dispatch loop pumps HTTP through the same uniform seam as every
// other protocol - no HTTP special case in the loop. Runs the file/chunk send pumps, the WebSocket +
// SSE drains, the keep-alive re-parse, and dispatches a completed request into this server's routes.
#if DWS_ENABLE_EDGE_CACHE
// Edge-cache async-fetch pump seam (see dws_http_set_edge_poll / services/edge_cache/edge_cache_proxy):
// a cache miss suspends the client request and drives the non-blocking origin fetch from this slot's poll.
static bool (*s_edge_poll)(uint8_t slot) = nullptr;
void dws_http_set_edge_poll(bool (*fn)(uint8_t slot))
{
    s_edge_poll = fn;
}
#endif

void DWS::http_poll_slot(uint8_t i)
{
#if DWS_ENABLE_EDGE_CACHE
    // An edge-cache origin fetch in flight for this slot owns it: pump the fetch and skip the rest of the
    // HTTP pipeline until it completes (and hands off to send_chunked for the cached response).
    if (s_edge_poll && s_edge_poll(i))
        return;
#endif
#if DWS_ENABLE_FILE_SERVING
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

#if DWS_ENABLE_WEBSOCKET
    // WebSocket slot - drain ring buffer and dispatch ready frames
    WsConn *ws = ws_find(i);
    if (ws)
    {
#if DWS_ENABLE_TLS
        if (conn_pool[i].tls)
        {
            // wss://: the rx ring holds ciphertext, so decrypt records here and
            // feed the frame parser, dispatching each completed frame as it
            // finishes (one TLS record may carry several WS frames).
            uint8_t tbuf[256];
            int n;
            while ((n = dws_tls_read(i, tbuf, sizeof(tbuf))) > 0)
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
                dws_conn_abort_slot(i); // transport owns TLS-free + detach + reset + RST
                http_reset(i);
            }
            return;
        }
#endif // DWS_ENABLE_TLS

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
            dws_conn_begin_close(i);
            http_reset(i);
        }
        return; // slot is owned by WS; skip HTTP dispatch
    }
#endif // DWS_ENABLE_WEBSOCKET

#if DWS_ENABLE_SSE
    // SSE slot - connection stays open, nothing to parse from client
    if (dws_sse_find(i))
        return;
#endif // DWS_ENABLE_SSE

#if DWS_ENABLE_KEEPALIVE
    // Keep-alive: a slot recycled after a response may already hold the next
    // (pipelined) request in its ring buffer with no new EvtType::EVT_DATA to trigger a
    // parse. Drain it here each tick so it gets dispatched. TLS slots are
    // skipped - their ring holds ciphertext, decrypted in the session layer.
    if (conn_pool[i].state == ConnState::CONN_ACTIVE && http_pool[i].parse_state != ParseState::PARSE_COMPLETE
#if DWS_ENABLE_TLS
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
        send(i, 400, DWS_MIME_TEXT_PLAIN, "Bad Request");
    }
    else if (http_pool[i].parse_state == ParseState::PARSE_ENTITY_TOO_LARGE)
    {
        send(i, 413, DWS_MIME_TEXT_PLAIN, "Payload Too Large");
    }
    else if (http_pool[i].parse_state == ParseState::PARSE_URI_TOO_LONG)
    {
        send(i, 414, DWS_MIME_TEXT_PLAIN, "URI Too Long");
    }
}

bool DWS::defer(uint8_t slot, dws_deferred_fn fn, void *arg) const
{
    if (slot >= MAX_CONNS)
        return false;
    // Route to the worker that owns the slot so the callback runs single-threaded
    // alongside that slot's own processing.
    return dws_defer(conn_pool[slot].owner, fn, arg);
}

// ---------------------------------------------------------------------------
// Diagnostic endpoint
// ---------------------------------------------------------------------------

#if DWS_ENABLE_DIAG
void DWS::diag(uint8_t slot_id)
{
    send(slot_id, 200, DWS_MIME_JSON, DWS_DIAG_JSON);
}
#endif

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
                        status, extra_hdr ? extra_hdr : "", DWS_MIME_TEXT_PLAIN, blen);

    // The last write carries the flush (dws_conn_send_flush = write+tcp_output in one marshal), so
    // an error-and-close costs one round-trip fewer - and 4xx/5xx closes are the hot path under a
    // flood (rate-limit / auth-lockout 429s).
    if (blen > 0 && !req_is_head(slot_id))
    {
        dws_conn_send(slot_id, header, (u16_t)hlen);
        dws_conn_send_flush(slot_id, body, (u16_t)blen);
    }
    else
    {
        dws_conn_send_flush(slot_id, header, (u16_t)hlen);
    }
    dws_conn_begin_close(slot_id); // dwell in ConnState::CONN_CLOSING until the response drains
    http_reset(slot_id);
}

// Send 405 Method Not Allowed with the required Allow header (RFC 7231 §6.5.5).
static void send_method_not_allowed(uint8_t slot_id, const char *allow)
{
    char extra[80];
    snprintf(extra, sizeof(extra), "Allow: %s\r\n", allow);
    send_error_close(slot_id, "405 Method Not Allowed", extra, "Method Not Allowed");
}

#if DWS_ENABLE_AUTH_LOCKOUT
// The peer's family-tagged source address for the connection in slot_id (unspecified on native /
// no pcb). Used as the auth-lockout bucket key - the full IPv4 or IPv6 address, so a v6 peer is
// never flattened onto a shared v4 bucket nor folded into a collideable hash.
static DWSIp lockout_client_ip(uint8_t slot_id)
{
    DWSIp ip;
    ip.family = DWSIpFamily::DWS_IP_NONE;
    dws_conn_remote_addr(slot_id, &ip);
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
#endif // DWS_ENABLE_AUTH_LOCKOUT

bool DWS::route_admits(const Route *r, uint8_t slot_id, HttpReq *req) const
{
    if (!r->is_active)
        return false;
    bool matched = r->is_regex   ? regex_match(r->path, req->path)
                   : r->is_param ? match_path_params(r->path, req->path, req)
                                 : path_matches(r->path, r->is_wildcard, req->path);
    if (!matched)
        return false;
    // Per-route interface gate: a route bound to STA/AP is invisible on the
    // other interface (falls through to other routes / 404).
    if (r->iface_filter != DWSIface::DETIFACE_ANY && r->iface_filter != dws_conn_iface(slot_id))
        return false;
    return true;
}

#if DWS_ENABLE_CSRF
bool DWS::dws_csrf_gate(uint8_t slot_id, HttpReq *req, HttpMethod method)
{
    // Built-in token endpoint: GET /csrf issues a signed token (also set as the
    // csrf cookie) for clients to echo in X-CSRF-Token on state-changing requests.
    if (method == HttpMethod::HTTP_GET && strcmp(req->path, "/csrf") == 0)
    {
        char tok[CSRF_TOKEN_BUF];
        if (dws_csrf_issue(tok, sizeof(tok)) > 0)
        {
            set_cookie(slot_id, "csrf", tok, "Path=/; SameSite=Strict");
            char body[CSRF_TOKEN_BUF + 16];
            snprintf(body, sizeof(body), "{\"token\":\"%s\"}", tok);
            send(slot_id, 200, DWS_MIME_JSON, body);
        }
        else
        {
            send(slot_id, 500, DWS_MIME_TEXT_PLAIN, "CSRF unavailable");
        }
        return true;
    }

    // Enforce CSRF on every state-changing method: require a valid signed
    // X-CSRF-Token header (GET / HEAD / OPTIONS are exempt - not state-changing).
    if (method == HttpMethod::HTTP_POST || method == HttpMethod::HTTP_PUT || method == HttpMethod::HTTP_PATCH ||
        method == HttpMethod::HTTP_DELETE)
    {
        const char *tok = http_get_header(req, "X-CSRF-Token");
        if (!tok || !dws_csrf_verify(tok))
        {
            send(slot_id, 403, DWS_MIME_TEXT_PLAIN, "CSRF token missing or invalid");
            return true;
        }
    }
    return false;
}
#endif // DWS_ENABLE_CSRF

#if DWS_ENABLE_WEBSOCKET
void DWS::handle_ws_route(uint8_t slot_id, HttpReq *req, HttpMethod method, const Route *r)
{
    const char *upgrade_hdr = http_get_header(req, "Upgrade");
    // RFC 6455 4.2.1: a valid handshake needs Upgrade: websocket AND a Connection
    // header that includes the "Upgrade" token.
    bool is_ws_upgrade = (method == HttpMethod::HTTP_GET) && upgrade_hdr &&
                         (strcasecmp(upgrade_hdr, "websocket") == 0) &&
                         conn_has_token(http_get_header(req, "Connection"), "upgrade");
    if (!is_ws_upgrade)
    {
        send(slot_id, 400, DWS_MIME_TEXT_PLAIN, "WebSocket upgrade required");
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
        send(slot_id, 400, DWS_MIME_TEXT_PLAIN, "Bad WebSocket handshake");
}
#endif // DWS_ENABLE_WEBSOCKET

#if DWS_ENABLE_AUTH
bool DWS::authorize_request(uint8_t slot_id, HttpReq *req, const Route *r)
{
#if DWS_ENABLE_AUTH_LOCKOUT
    DWSIp cip = lockout_client_ip(slot_id);
#if DWS_ENABLE_FORWARDED_TRUST
    // Behind a trusted reverse proxy, key the lockout on the original client (the proxy's Forwarded /
    // X-Forwarded-For), not the proxy's shared TCP address. Ignored for a direct/untrusted peer, so a
    // spoofed header can neither evade a lockout nor frame another address.
    {
        char fbuf[DWS_IP_STR_MAX];
        const char *fwd = http_forwarded_client(req, fbuf, sizeof(fbuf), nullptr) ? fbuf : nullptr;
        DWSIp eff;
        dws_forwarded_effective_ip(&cip, fwd, &eff);
        cip = eff;
    }
#endif
    uint32_t now = (uint32_t)millis();
    uint32_t remain = auth_lockout_remaining_ms(&cip, now);
    if (remain > 0)
    {
        // Address is locked out: 429 + Retry-After, no credential check.
        send_too_many_requests(slot_id, (remain + 999) / 1000);
        return false;
    }
#endif
    bool stale = false;
    bool ok = r->auth_digest ? check_digest_auth(slot_id, req, r, &stale) : check_basic_auth(slot_id, req, r);
#if DWS_ENABLE_AUTH_LOCKOUT
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
        return false;
    }
    return true;
}
#endif // DWS_ENABLE_AUTH

bool DWS::dispatch_matched_route(uint8_t slot_id, HttpReq *req, HttpMethod method, Route *r, bool *path_matched,
                                 char *allow_buf, size_t allow_cap)
{
#if DWS_ENABLE_WEBSOCKET
    if (r->type == RouteType::ROUTE_WS)
    {
        handle_ws_route(slot_id, req, method, r);
        return true;
    }
#endif // DWS_ENABLE_WEBSOCKET

#if DWS_ENABLE_SSE
    if (r->type == RouteType::ROUTE_SSE)
    {
        if (!dws_sse_do_upgrade(slot_id, req, r->dws_sse_connect))
            send(slot_id, 503, DWS_MIME_TEXT_PLAIN, "Service Unavailable");
        return true;
    }
#endif // DWS_ENABLE_SSE

#if DWS_ENABLE_FILE_SERVING
    if (r->type == RouteType::ROUTE_STATIC)
    {
        // Static mounts answer GET (and HEAD via GET); other methods → 405.
        if (method != HttpMethod::HTTP_GET && method != HttpMethod::HTTP_HEAD)
        {
            *path_matched = true;
            allow_append(allow_buf, allow_cap, "GET");
            allow_append(allow_buf, allow_cap, "HEAD");
            return false;
        }
        serve_static_request(slot_id, req, r);
        return true;
    }
#endif // DWS_ENABLE_FILE_SERVING

    // RouteType::ROUTE_HTTP - a HEAD request is served by the GET handler with the
    // response body suppressed (RFC 7231 §4.3.2).
    bool method_ok = (r->method == method) || (method == HttpMethod::HTTP_HEAD && r->method == HttpMethod::HTTP_GET);
    if (!method_ok)
    {
        // Path matches but method differs - record it for a 405 + Allow.
        *path_matched = true;
        allow_append(allow_buf, allow_cap, method_name(r->method));
        // A GET route also answers HEAD, so advertise it in Allow.
        if (r->method == HttpMethod::HTTP_GET)
            allow_append(allow_buf, allow_cap, "HEAD");
        return false;
    }
#if DWS_ENABLE_AUTH
    if (r->auth_required && !authorize_request(slot_id, req, r))
        return true; // 401/429 already sent
#endif               // DWS_ENABLE_AUTH
    r->callback(slot_id, req);
    return true;
}

void DWS::match_and_execute(uint8_t slot_id)
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

#if DWS_ENABLE_WEBDAV
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

#if DWS_ENABLE_CSRF
    if (dws_csrf_gate(slot_id, req, method))
        return;
#endif

    // RFC 7230 §3.3.1: reject Transfer-Encoding
    if (http_get_header(req, "Transfer-Encoding") != nullptr)
    {
        send(slot_id, 501, DWS_MIME_TEXT_PLAIN, "Not Implemented");
        return;
    }

    // RFC 7231 §6.5.2: a method the server does not implement → 501.
    if (method == HttpMethod::HTTP_METHOD_UNKNOWN)
    {
        send(slot_id, 501, DWS_MIME_TEXT_PLAIN, "Not Implemented");
        return;
    }

    // For RFC 7231 §6.5.5: if a path matches but no method does, answer 405
    // with an Allow header listing the methods registered for that path.
    bool path_matched = false;
    char allow_buf[64];
    allow_buf[0] = '\0';

    for (uint8_t i = 0; i < _route_count; i++)
    {
        Route *r = &_routes[i];
        if (!route_admits(r, slot_id, req))
            continue;
        if (dispatch_matched_route(slot_id, req, method, r, &path_matched, allow_buf, sizeof(allow_buf)))
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
        send(slot_id, 404, DWS_MIME_TEXT_PLAIN, "Not Found");
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
void DWS::send(uint8_t slot_id, int code, const char *content_type, const char *payload)
{
    // Null-terminated convenience wrapper over the explicit-length send.
    send(slot_id, code, content_type, (const uint8_t *)payload, payload ? strnlen(payload, 0xFFFF) : 0);
}

void DWS::send(uint8_t slot_id, int code, const char *content_type, const uint8_t *body, size_t body_len)
{
    if (slot_id >= CONN_POOL_SLOTS)
        return; // guard the public entry: never index conn_pool out of range
    const char *payload = (const char *)body;
    TcpConn *conn = &conn_pool[slot_id];
#if DWS_ENABLE_HTTP2 || DWS_ENABLE_HTTP3
    // A self-framing protocol (HTTP/2, HTTP/3) installed its own response sink at negotiation /
    // dispatch time; route through it and let it own its framing + connection lifecycle. This runs
    // before the HTTP/1.1 pcb check because that check is a TCP-transport concern (the HTTP/3 slot
    // has no pcb by design, and an h2 connection manages its own).
    if (conn->dws_resp_sink)
    {
        conn->dws_resp_sink(slot_id, code, content_type, payload, body_len);
        return;
    }
#endif
    if (conn->state != ConnState::CONN_ACTIVE || conn->pcb == nullptr)
    {
        http_reset(slot_id);
        return;
    }

    int payload_len = (int)(body_len > 0xFFFF ? 0xFFFF : body_len);

    bool keep;
    const char *cl = dws_resp_conn_hdr(slot_id, &keep);

    char header[RESP_HDR_BUF_SIZE];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 %d %s\r\n"
                        "Content-Type: %s\r\n"
                        "Content-Length: %d\r\n",
                        code, status_text(code), content_type, payload_len);
    hlen = append_resp_trailer(header, sizeof(header), hlen, slot_id, cl);

    // The slot stays ConnState::CONN_ACTIVE through the write for both paths; dws_resp_end then
    // begins the ConnState::CONN_CLOSING dwell on the close path (finalized once ACKed).

    bool head = req_is_head(slot_id);

    // HEAD responses carry the headers (incl. Content-Length) but no body. For a
    // body that fits the header scratch, coalesce headers+body into a single send
    // so the response costs one tcpip_thread round-trip instead of two. The final
    // write also carries the flush (dws_conn_send_flush), so dws_resp_end skips it -
    // a keep-alive small response is now one marshal (write+output) instead of two.
    if (!head && payload_len > 0 && (size_t)hlen + (size_t)payload_len <= sizeof(header))
    {
        memcpy(header + hlen, payload, (size_t)payload_len);
        dws_conn_send_flush(slot_id, header, (u16_t)(hlen + payload_len));
    }
    else if (!head && payload_len > 0)
    {
        dws_conn_send(slot_id, header, (u16_t)hlen);
        dws_conn_send_flush(slot_id, payload, (u16_t)payload_len);
    }
    else
    {
        dws_conn_send_flush(slot_id, header, (u16_t)hlen);
    }

    dws_resp_end(slot_id, code, payload_len, keep, /*pre_flushed=*/true);
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
void DWS::send_empty(uint8_t slot_id, int code)
{
    if (slot_id >= CONN_POOL_SLOTS)
        return;
    TcpConn *conn = &conn_pool[slot_id];
#if DWS_ENABLE_HTTP2 || DWS_ENABLE_HTTP3
    if (conn->dws_resp_sink)
    {
        conn->dws_resp_sink(slot_id, code, "text/plain", "", 0);
        return;
    }
#endif
    if (conn->state != ConnState::CONN_ACTIVE || conn->pcb == nullptr)
    {
        http_reset(slot_id);
        return;
    }

    bool keep;
    const char *cl = dws_resp_conn_hdr(slot_id, &keep);

    char header[RESP_HDR_BUF_SIZE];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 %d %s\r\n"
                        "Content-Length: 0\r\n",
                        code, status_text(code));
    hlen = append_resp_trailer(header, sizeof(header), hlen, slot_id, cl);

    dws_conn_send_flush(slot_id, header, (u16_t)hlen);

    dws_resp_end(slot_id, code, 0, keep, /*pre_flushed=*/true);
}

void DWS::redirect(uint8_t slot_id, int code, const char *location)
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
    const char *cl = dws_resp_conn_hdr(slot_id, &keep);

    char header[RESP_HDR_BUF_SIZE];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 %d %s\r\n"
                        "Location: %s\r\n"
                        "Content-Length: 0\r\n",
                        code, status_text(code), location);
    hlen = append_resp_trailer(header, sizeof(header), hlen, slot_id, cl);

    dws_conn_send_flush(slot_id, header, (u16_t)hlen);

    dws_resp_end(slot_id, code, 0, keep, /*pre_flushed=*/true);
}
