// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file http_client.cpp
 * @brief Outbound HTTP(S) client: URL parse + request build + response parse,
 *        and the raw-lwIP / mbedTLS transport (ESP32 only).
 */

#include "services/http_client/http_client.h"

#if DETWS_ENABLE_HTTP_CLIENT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Pure helpers (host-testable)
// ---------------------------------------------------------------------------

bool http_client_parse_url(const char *url, bool *is_https, char *host, size_t host_cap, uint16_t *port, char *path,
                           size_t path_cap)
{
    if (!url || !is_https || !host || !port || !path)
        return false;

    const char *p = url;
    if (strncmp(p, "https://", 8) == 0)
    {
        *is_https = true;
        *port = 443;
        p += 8;
    }
    else if (strncmp(p, "http://", 7) == 0)
    {
        *is_https = false;
        *port = 80;
        p += 7;
    }
    else
        return false;

    // host (up to ':' or '/')
    const char *h = p;
    while (*p && *p != ':' && *p != '/')
        p++;
    size_t hlen = (size_t)(p - h);
    if (hlen == 0 || hlen >= host_cap)
        return false;
    memcpy(host, h, hlen);
    host[hlen] = '\0';

    // optional :port
    if (*p == ':')
    {
        p++;
        if (*p < '0' || *p > '9')
            return false;
        uint32_t pn = 0;
        while (*p >= '0' && *p <= '9')
        {
            pn = pn * 10 + (uint32_t)(*p++ - '0');
            if (pn > 65535)
                return false;
        }
        *port = (uint16_t)pn;
    }

    // path ("/" if absent)
    if (*p == '\0')
    {
        if (path_cap < 2)
            return false;
        path[0] = '/';
        path[1] = '\0';
    }
    else
    {
        size_t plen = strlen(p);
        if (plen >= path_cap)
            return false;
        memcpy(path, p, plen + 1);
    }
    return true;
}

size_t http_client_build_request(const char *method, const char *host, uint16_t port, const char *path,
                                 const char *content_type, const uint8_t *body, size_t body_len, char *out, size_t cap)
{
    if (!method || !host || !path || !out || cap == 0)
        return 0;

    // Host header carries the port only when it is non-default.
    char hosthdr[80];
    if (port == 80 || port == 443)
        snprintf(hosthdr, sizeof(hosthdr), "%s", host);
    else
        snprintf(hosthdr, sizeof(hosthdr), "%s:%u", host, (unsigned)port);

    int n;
    if (body && body_len)
    {
        n = snprintf(out, cap,
                     "%s %s HTTP/1.1\r\n"
                     "Host: %s\r\n"
                     "User-Agent: DetWS\r\n"
                     "Content-Type: %s\r\n"
                     "Content-Length: %u\r\n"
                     "Connection: close\r\n\r\n",
                     method, path, hosthdr, content_type ? content_type : "application/octet-stream",
                     (unsigned)body_len);
        if (n < 0 || (size_t)n + body_len > cap)
            return 0;
        memcpy(out + n, body, body_len);
        return (size_t)n + body_len;
    }

    n = snprintf(out, cap,
                 "%s %s HTTP/1.1\r\n"
                 "Host: %s\r\n"
                 "User-Agent: DetWS\r\n"
                 "Connection: close\r\n\r\n",
                 method, path, hosthdr);
    if (n < 0 || (size_t)n >= cap)
        return 0;
    return (size_t)n;
}

// Case-insensitive search for header @p name in the header block [buf, end);
// returns a pointer to the value (past "name:" and spaces) or nullptr.
static const char *find_header(const uint8_t *buf, const uint8_t *end, const char *name)
{
    size_t nlen = strlen(name);
    const uint8_t *p = buf;
    while (p + nlen + 1 < end)
    {
        // at the start of a header line?
        if (strncasecmp((const char *)p, name, nlen) == 0 && p[nlen] == ':')
        {
            const uint8_t *v = p + nlen + 1;
            while (v < end && (*v == ' ' || *v == '\t'))
                v++;
            return (const char *)v;
        }
        // advance to the next line
        while (p < end && *p != '\n')
            p++;
        if (p < end)
            p++;
    }
    return nullptr;
}

int http_client_parse_response(uint8_t *buf, size_t len, size_t *body_off, size_t *body_len)
{
    if (!buf || len < 12 || memcmp(buf, "HTTP/1.", 7) != 0)
        return HTTP_CLIENT_ERR_RESPONSE;

    // Status code: first space, then 3 digits.
    const uint8_t *sp = (const uint8_t *)memchr(buf, ' ', len);
    if (!sp || sp + 4 > buf + len)
        return HTTP_CLIENT_ERR_RESPONSE;
    int status = (sp[1] - '0') * 100 + (sp[2] - '0') * 10 + (sp[3] - '0');
    if (status < 100 || status > 599)
        return HTTP_CLIENT_ERR_RESPONSE;

    // Header terminator "\r\n\r\n".
    uint8_t *hdr_end = nullptr;
    for (size_t i = 0; i + 3 < len; i++)
        if (buf[i] == '\r' && buf[i + 1] == '\n' && buf[i + 2] == '\r' && buf[i + 3] == '\n')
        {
            hdr_end = buf + i;
            break;
        }
    if (!hdr_end)
        return HTTP_CLIENT_ERR_RESPONSE;

    size_t off = (size_t)(hdr_end + 4 - buf);
    size_t avail = len - off;

    const char *te = find_header(buf, hdr_end, "Transfer-Encoding");
    if (te && strncasecmp(te, "chunked", 7) == 0)
    {
        // Decode chunked transfer-encoding in place into the body region.
        size_t in = off, out = off;
        while (in < len)
        {
            // chunk size line (hex) up to CRLF
            size_t csz = 0;
            bool any = false;
            while (in < len && buf[in] != '\r')
            {
                uint8_t c = buf[in++];
                int d;
                if (c >= '0' && c <= '9')
                    d = c - '0';
                else if (c >= 'a' && c <= 'f')
                    d = c - 'a' + 10;
                else if (c >= 'A' && c <= 'F')
                    d = c - 'A' + 10;
                else
                    break; // stop at ';' (chunk ext) or junk
                csz = csz * 16 + (size_t)d;
                any = true;
            }
            // skip to end of the size line
            while (in < len && buf[in] != '\n')
                in++;
            if (in < len)
                in++; // past '\n'
            if (!any)
                break;
            if (csz == 0)
                break; // last chunk
            if (in + csz > len)
                csz = (in < len) ? (len - in) : 0; // truncated; take what we have
            memmove(buf + out, buf + in, csz);
            out += csz;
            in += csz;
            // skip trailing CRLF after chunk data
            while (in < len && (buf[in] == '\r' || buf[in] == '\n'))
                in++;
        }
        *body_off = off;
        *body_len = out - off;
        return status;
    }

    const char *cl = find_header(buf, hdr_end, "Content-Length");
    if (cl)
    {
        long n = atol(cl);
        if (n < 0)
            n = 0;
        size_t want = (size_t)n;
        *body_off = off;
        *body_len = (want < avail) ? want : avail; // bounded by what we received
        return status;
    }

    // No framing header: connection-close delimited (rest of the buffer).
    *body_off = off;
    *body_len = avail;
    return status;
}

// ---------------------------------------------------------------------------
// Transport (ESP32 only): raw-lwIP TCP client + DNS, optional client mbedTLS.
// ---------------------------------------------------------------------------
#if defined(ARDUINO)

#include "lwip/dns.h"
#include "lwip/priv/tcpip_priv.h"
#include "lwip/tcp.h"
#include <Arduino.h>

#if DETWS_ENABLE_HTTP_CLIENT_TLS
#include "network_drivers/tls/det_tls.h"
#include <mbedtls/ssl.h> // MBEDTLS_ERR_SSL_WANT_READ for the BIO recv callback
#endif

// Optional stage tracing: build with -DDETWS_HTTP_CLIENT_DEBUG to print where a
// request stalls (DNS / connect / send / receive). Goes to the console UART.
#ifdef DETWS_HTTP_CLIENT_DEBUG
#define CL_DBG(...) printf(__VA_ARGS__)
#else
#define CL_DBG(...) ((void)0)
#endif

// Single in-flight request (one loop task). All buffers static (no heap).
// s_rx holds the *response to parse*: the raw wire bytes for http, or (for https)
// the plaintext decrypted by the TLS engine.
static uint8_t s_rx[DETWS_HTTP_CLIENT_BUF_SIZE];
static volatile size_t s_rx_len;
static volatile bool s_connected;
static volatile bool s_closed;
static volatile bool s_aborted;
static struct tcp_pcb *s_pcb;

#if DETWS_ENABLE_HTTP_CLIENT_TLS
// For https the lwIP recv callback feeds *ciphertext* into a draining ring (the
// TLS engine pulls from it during the handshake/read), so a modest buffer holds
// the multi-KB handshake flight without loss: when the ring is full the callback
// refuses the segment (ERR_MEM, no pbuf_free) and lwIP redelivers it once the
// engine drains some bytes. The ring must exceed one TCP segment (TCP_MSS) or a
// full segment could never fit. Plaintext output goes to s_rx (above).
static uint8_t s_ct[DETWS_HTTP_CLIENT_CT_BUF_SIZE];
static volatile size_t s_ct_head;
static volatile size_t s_ct_tail;
static volatile bool s_tls_mode; // recv routes wire bytes to the ciphertext ring
#endif

// DNS resolution result (filled from tcpip_thread).
static volatile bool s_dns_done;
static volatile bool s_dns_ok;
static ip_addr_t s_dns_addr;

// --- tcpip_thread-marshaled raw-lwIP ops (see transport.cpp for the rationale) ---
struct ClConnCall
{
    struct tcpip_api_call_data base;
    ip_addr_t addr;
    uint16_t port;
    err_t result;
};
struct ClSendCall
{
    struct tcpip_api_call_data base;
    const void *data;
    u16_t len;
    err_t result;
};

static err_t cl_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

static err_t cl_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    (void)arg;
    (void)tpcb;
    CL_DBG("[hc] connected cb err=%d\n", (int)err);
    if (err == ERR_OK)
        s_connected = true;
    else
        s_aborted = true;
    return ERR_OK;
}

static void cl_err(void *arg, err_t err)
{
    (void)arg;
    CL_DBG("[hc] err cb err=%d\n", (int)err);
    s_pcb = nullptr; // lwIP already freed the pcb
    s_aborted = true;
}

static err_t cl_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    (void)arg;
    (void)err;
    if (p == nullptr)
    {
        CL_DBG("[hc] recv cb: peer closed\n");
        s_closed = true; // peer closed
        return ERR_OK;
    }
    CL_DBG("[hc] recv cb: %u bytes\n", (unsigned)p->tot_len);
#if DETWS_ENABLE_HTTP_CLIENT_TLS
    if (s_tls_mode)
    {
        // Ciphertext -> draining ring with lossless backpressure: if the whole
        // segment will not fit the free space, refuse it (lwIP retains and
        // redelivers) so no handshake bytes are dropped.
        size_t used = (s_ct_head + sizeof(s_ct) - s_ct_tail) % sizeof(s_ct);
        size_t freesp = (sizeof(s_ct) - 1) - used;
        if (p->tot_len > freesp)
            return ERR_MEM;
        struct pbuf *q = p;
        while (q)
        {
            const uint8_t *d = (const uint8_t *)q->payload;
            for (u16_t i = 0; i < q->len; i++)
            {
                s_ct[s_ct_head] = d[i];
                s_ct_head = (s_ct_head + 1) % sizeof(s_ct);
            }
            q = q->next;
        }
        tcp_recved(tpcb, p->tot_len);
        pbuf_free(p);
        return ERR_OK;
    }
#endif
    struct pbuf *q = p;
    while (q)
    {
        const uint8_t *d = (const uint8_t *)q->payload;
        for (u16_t i = 0; i < q->len; i++)
            if (s_rx_len < sizeof(s_rx))
                s_rx[s_rx_len++] = d[i];
        q = q->next;
    }
    tcp_recved(tpcb, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

static err_t cl_do_connect(struct tcpip_api_call_data *c)
{
    ClConnCall *k = (ClConnCall *)c;
    // Outbound IPv4 connection (DNS resolves an IPv4 address); match the pcb type.
    s_pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
    if (!s_pcb)
    {
        k->result = ERR_MEM;
        return ERR_OK;
    }
    tcp_recv(s_pcb, cl_recv);
    tcp_err(s_pcb, cl_err);
    k->result = tcp_connect(s_pcb, &k->addr, k->port, cl_connected);
    return ERR_OK;
}

static err_t cl_do_send(struct tcpip_api_call_data *c)
{
    ClSendCall *k = (ClSendCall *)c;
    if (!s_pcb)
    {
        k->result = ERR_CONN;
        return ERR_OK;
    }
    k->result = tcp_write(s_pcb, k->data, k->len, TCP_WRITE_FLAG_COPY);
    if (k->result == ERR_OK)
        tcp_output(s_pcb);
    return ERR_OK;
}

static err_t cl_do_close(struct tcpip_api_call_data *c)
{
    (void)c;
    if (s_pcb)
    {
        tcp_recv(s_pcb, nullptr);
        tcp_err(s_pcb, nullptr);
        if (tcp_close(s_pcb) != ERR_OK)
            tcp_abort(s_pcb);
        s_pcb = nullptr;
    }
    return ERR_OK;
}

// Close from the main loop: marshal the raw tcp_* teardown into tcpip_thread so
// it never races the stack (the same cross-thread rule as the server transport).
static void cl_close()
{
    ClConnCall k;
    memset(&k, 0, sizeof(k));
    tcpip_api_call(cl_do_close, &k.base);
}

static void cl_dns_cb(const char *name, const ip_addr_t *addr, void *arg)
{
    (void)name;
    (void)arg;
    if (addr)
    {
        s_dns_addr = *addr;
        s_dns_ok = true;
    }
    s_dns_done = true;
}

static err_t cl_do_dns(struct tcpip_api_call_data *c)
{
    const char *host = (const char *)((ClSendCall *)c)->data;
    err_t e = dns_gethostbyname(host, &s_dns_addr, cl_dns_cb, nullptr);
    if (e == ERR_OK)
    {
        s_dns_ok = true;
        s_dns_done = true;
    }
    else if (e != ERR_INPROGRESS)
        s_dns_done = true; // hard failure
    return ERR_OK;
}

// Resolve @p host (dotted-quad fast path, else DNS) into s_dns_addr.
static bool cl_resolve(const char *host, uint32_t deadline)
{
    if (ipaddr_aton(host, &s_dns_addr))
        return true;
    s_dns_done = false;
    s_dns_ok = false;
    ClSendCall k;
    memset(&k, 0, sizeof(k));
    k.data = host;
    tcpip_api_call(cl_do_dns, &k.base);
    while (!s_dns_done && (int32_t)(deadline - millis()) > 0)
        delay(5);
    return s_dns_ok;
}

#if DETWS_ENABLE_HTTP_CLIENT_TLS
// mbedTLS BIO over the raw client pcb: send via the marshaled tcp_write, recv by
// draining the ciphertext ring the lwIP recv callback fills.
static int cl_tls_send(void *ctx, const unsigned char *buf, size_t len)
{
    (void)ctx;
    ClSendCall k;
    memset(&k, 0, sizeof(k));
    k.data = buf;
    k.len = (u16_t)(len > 0xFFFF ? 0xFFFF : len);
    tcpip_api_call(cl_do_send, &k.base);
    return (k.result == ERR_OK) ? (int)k.len : -1;
}
static int cl_tls_recv(void *ctx, unsigned char *buf, size_t len)
{
    (void)ctx;
    size_t used = (s_ct_head + sizeof(s_ct) - s_ct_tail) % sizeof(s_ct);
    if (used == 0)
        return s_closed ? 0 : MBEDTLS_ERR_SSL_WANT_READ;
    size_t n = used < len ? used : len;
    for (size_t i = 0; i < n; i++)
    {
        buf[i] = s_ct[s_ct_tail];
        s_ct_tail = (s_ct_tail + 1) % sizeof(s_ct);
    }
    return (int)n;
}
#endif // DETWS_ENABLE_HTTP_CLIENT_TLS

// Core request: build, connect, send, receive, parse.
static int http_request(const char *method, const char *url, const char *content_type, const uint8_t *body,
                        size_t body_len, HttpClientResult *out)
{
    if (out)
    {
        out->status = 0;
        out->body = nullptr;
        out->body_len = 0;
    }

    bool is_https;
    char host[80], path[160];
    uint16_t port;
    if (!http_client_parse_url(url, &is_https, host, sizeof(host), &port, path, sizeof(path)))
        return HTTP_CLIENT_ERR_URL;
    CL_DBG("[hc] url host=%s port=%u https=%d path=%s\n", host, (unsigned)port, (int)is_https, path);
#if !DETWS_ENABLE_HTTP_CLIENT_TLS
    if (is_https)
        return HTTP_CLIENT_ERR_TLS;
#endif

    // Build the request line + headers (+ optional body).
    char req[768];
    size_t reqlen = http_client_build_request(method, host, port, path, content_type, body, body_len, req, sizeof(req));
    if (reqlen == 0)
        return HTTP_CLIENT_ERR_URL;

    uint32_t deadline = millis() + DETWS_HTTP_CLIENT_TIMEOUT_MS;

    if (!cl_resolve(host, deadline))
        return HTTP_CLIENT_ERR_DNS;
    CL_DBG("[hc] resolved ip=%s\n", ipaddr_ntoa(&s_dns_addr));

    // Reset connection state and connect.
    s_rx_len = 0;
    s_connected = false;
    s_closed = false;
    s_aborted = false;
    s_pcb = nullptr;
#if DETWS_ENABLE_HTTP_CLIENT_TLS
    s_tls_mode = is_https; // route wire bytes to the ciphertext ring for https
    s_ct_head = 0;
    s_ct_tail = 0;
#endif
    ClConnCall cc;
    memset(&cc, 0, sizeof(cc));
    cc.addr = s_dns_addr;
    cc.port = port;
    tcpip_api_call(cl_do_connect, &cc.base);
    CL_DBG("[hc] connect dispatch result=%d\n", (int)cc.result);
    if (cc.result != ERR_OK)
        return HTTP_CLIENT_ERR_CONNECT;
    while (!s_connected && !s_aborted && (int32_t)(deadline - millis()) > 0)
        delay(5);
    CL_DBG("[hc] connect poll connected=%d aborted=%d\n", (int)s_connected, (int)s_aborted);
    if (!s_connected)
    {
        cl_close();
        return s_aborted ? HTTP_CLIENT_ERR_CONNECT : HTTP_CLIENT_ERR_TIMEOUT;
    }

    // The response to parse: raw wire bytes (http) or decrypted plaintext (https),
    // both land in s_rx.
    size_t resp_len = 0;

    if (is_https)
    {
#if DETWS_ENABLE_HTTP_CLIENT_TLS
        int rc = det_tls_client_run(host, (const uint8_t *)req, reqlen, s_rx, sizeof(s_rx), &resp_len, cl_tls_send,
                                    cl_tls_recv, deadline);
        CL_DBG("[hc] tls rc=%d pt_len=%u\n", rc, (unsigned)resp_len);
        if (rc < 0)
        {
            cl_close();
            return HTTP_CLIENT_ERR_TLS;
        }
#else
        cl_close();
        return HTTP_CLIENT_ERR_TLS;
#endif
    }
    else
    {
        // Plaintext: send the request, then read until close / timeout / buffer full.
        ClSendCall sc;
        memset(&sc, 0, sizeof(sc));
        sc.data = req;
        sc.len = (u16_t)reqlen;
        tcpip_api_call(cl_do_send, &sc.base);
        CL_DBG("[hc] send result=%d reqlen=%u\n", (int)sc.result, (unsigned)reqlen);
        if (sc.result != ERR_OK)
        {
            cl_close();
            return HTTP_CLIENT_ERR_SEND;
        }
        while (!s_closed && s_rx_len < sizeof(s_rx) && (int32_t)(deadline - millis()) > 0)
            delay(5);
        resp_len = s_rx_len;
    }

    CL_DBG("[hc] done rx_len=%u closed=%d resp_len=%u\n", (unsigned)s_rx_len, (int)s_closed, (unsigned)resp_len);
    cl_close();

    if (resp_len == 0)
        return HTTP_CLIENT_ERR_TIMEOUT;

    size_t body_off = 0, blen = 0;
    int status = http_client_parse_response(s_rx, resp_len, &body_off, &blen);
    if (status < 0)
        return HTTP_CLIENT_ERR_RESPONSE;
    if (out)
    {
        out->status = status;
        out->body = s_rx + body_off;
        out->body_len = blen;
    }
    return status;
}

int http_get(const char *url, HttpClientResult *out)
{
    return http_request("GET", url, nullptr, nullptr, 0, out);
}

int http_post(const char *url, const char *content_type, const uint8_t *body, size_t body_len, HttpClientResult *out)
{
    return http_request("POST", url, content_type, body, body_len, out);
}

#else // host build: transport is a stub

int http_get(const char *, HttpClientResult *)
{
    return HTTP_CLIENT_ERR_CONNECT;
}
int http_post(const char *, const char *, const uint8_t *, size_t, HttpClientResult *)
{
    return HTTP_CLIENT_ERR_CONNECT;
}

#endif // ARDUINO

#endif // DETWS_ENABLE_HTTP_CLIENT
