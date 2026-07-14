// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file response.cpp
 * @brief Response building for DetWebServer: template rendering, chunked/streaming responses,
 *        response headers + cookies, MIME typing, and the stats / Prometheus-metrics endpoints.
 *
 * Split out of dwserver.cpp (single-purpose server files). The two-pass {{name}} template walk,
 * the cross-loop chunked-send pump (paged like the file pump so a body is unbounded in constant
 * memory), the per-response header/cookie buffer API, mime_type(), and the built-in stats/metrics
 * handlers (rendered through the template engine from generated web assets). The chunked pump
 * shares the per-slot SendCtx owned by dwserver.cpp. Behaviour is identical to the pre-split code.
 */

#include "dwserver.h"
#include "network_drivers/transport/tcp.h" // conn_pool, det_conn_send, TcpConn/ConnState
#include "server/dwserver_internal.h"      // status_text, req_is_head, SendCtx s_send
#include "shared_primitives/mime.h"        // DET_MIME_*, mime tables
#include <stdio.h>
#include <string.h>
#if DETWS_ENABLE_METRICS || DETWS_ENABLE_STATS
#include "network_drivers/application/web_assets.h" // DETWS_STATS_JSON / DETWS_METRICS_PROM (generated)
#endif

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
    if (!det_conn_active(slot_id))
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
    if (!det_conn_active(slot_id))
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

    if (!det_conn_active(slot_id))
    {
        s.active = false; // connection gone mid-stream
        return;
    }

    // A body still being paged out is active, not idle: keep the CONN_TIMEOUT_MS idle sweep off
    // it so a transient send stall on a large stream cannot reap the slot mid-transfer.
    det_conn_touch_active(slot_id);

    // Frame each chunk in ONE buffer so it goes out in a single tcpip_thread round-trip (was three -
    // size line, body, CRLF - each a ~23 us marshal on-device). Reserve CHUNK_HDR_RESERVE bytes ahead
    // of the body for the "<hex>\r\n" size line and 2 after for the trailing CRLF, so the source writes
    // the body in place and the whole "<hex>\r\n<body>\r\n" is one det_conn_send with no extra copy.
    // FRAME reserves send-window room for that framing; the raw (HTTP/1.0) path sends the body verbatim.
    static const u16_t CHUNK_HDR_RESERVE = 8; // "<hex>\r\n" is <= 6 bytes for a chunk <= 0xFFFF
    const u16_t FRAME = s.raw ? 0 : 12;
    uint8_t framed[CHUNK_HDR_RESERVE + CHUNK_BUF_SIZE + 2];
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

        uint8_t *body = framed + CHUNK_HDR_RESERVE;
        size_t n = s.source(body, cap, s.ctx);
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
            det_conn_send(slot_id, body, (u16_t)n); // close-delimited: no chunk framing
        }
        else
        {
            // Prepend the size line (right-justified against the body) + append the trailing CRLF,
            // then send the framed chunk in one call.
            char sz[8];
            int sn = snprintf(sz, sizeof(sz), "%x\r\n", (unsigned)n);
            uint8_t *start = body - sn;
            memcpy(start, sz, (size_t)sn);
            body[n] = '\r';
            body[n + 1] = '\n';
            det_conn_send(slot_id, start, (u16_t)((size_t)sn + n + 2));
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
    int active = det_conn_active_count();

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
    int active = det_conn_active_count();

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
