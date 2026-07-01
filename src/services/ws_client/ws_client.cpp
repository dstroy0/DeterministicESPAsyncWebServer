// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ws_client.cpp
 * @brief WebSocket (RFC 6455) client codec (host-testable) + the raw-lwIP /
 *        mbedTLS persistent transport (ESP32 only).
 */

#include "services/ws_client/ws_client.h"

#if DETWS_ENABLE_WS_CLIENT

#include "network_drivers/presentation/base64/base64.h"
#include "network_drivers/presentation/sha1/sha1.h"
#include <stdio.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Pure codec (host-testable)
// ---------------------------------------------------------------------------

static const char WS_MAGIC[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

void ws_client_accept_for_key(const char *key_b64, char *out, size_t out_cap)
{
    if (!out || out_cap == 0)
        return;
    out[0] = '\0';
    if (!key_b64)
        return;
    char concat[64];
    size_t klen = strlen(key_b64);
    size_t mlen = sizeof(WS_MAGIC) - 1;
    if (klen + mlen >= sizeof(concat))
        return;
    memcpy(concat, key_b64, klen);
    memcpy(concat + klen, WS_MAGIC, mlen);
    uint8_t digest[SHA1_DIGEST_LEN];
    sha1((const uint8_t *)concat, klen + mlen, digest);
    if (out_cap < 29) // 28 base64 chars + NUL
        return;
    base64_encode(digest, SHA1_DIGEST_LEN, out);
}

size_t ws_client_build_handshake(uint8_t *out, size_t cap, const char *host, const char *path, const char *key_b64)
{
    if (!out || !host || !path || !key_b64)
        return 0;
    int n = snprintf((char *)out, cap,
                     "GET %s HTTP/1.1\r\n"
                     "Host: %s\r\n"
                     "Upgrade: websocket\r\n"
                     "Connection: Upgrade\r\n"
                     "Sec-WebSocket-Key: %s\r\n"
                     "Sec-WebSocket-Version: 13\r\n\r\n",
                     path, host, key_b64);
    if (n < 0 || (size_t)n >= cap)
        return 0;
    return (size_t)n;
}

// Case-insensitive header-value lookup within [buf, buf+len); returns a pointer
// to the value (past "name:" and OWS) and its length via *vlen, or nullptr.
static const char *find_header(const uint8_t *buf, size_t len, const char *name, size_t *vlen)
{
    size_t nlen = strlen(name);
    const uint8_t *p = buf;
    const uint8_t *end = buf + len;
    while (p + nlen + 1 < end)
    {
        if (strncasecmp((const char *)p, name, nlen) == 0 && p[nlen] == ':')
        {
            const uint8_t *v = p + nlen + 1;
            while (v < end && (*v == ' ' || *v == '\t'))
                v++;
            const uint8_t *e = v;
            while (e < end && *e != '\r' && *e != '\n')
                e++;
            *vlen = (size_t)(e - v);
            return (const char *)v;
        }
        while (p < end && *p != '\n')
            p++;
        if (p < end)
            p++;
    }
    return nullptr;
}

bool ws_client_check_response(const uint8_t *buf, size_t len, const char *expected_accept)
{
    if (!buf || len < 12 || !expected_accept)
        return false;
    // Status line must be an HTTP 101.
    const uint8_t *eol = (const uint8_t *)memchr(buf, '\n', len);
    if (!eol)
        return false;
    bool ok101 = false;
    for (const uint8_t *q = buf; q + 3 < eol; q++)
        if (q[0] == '1' && q[1] == '0' && q[2] == '1')
        {
            ok101 = true;
            break;
        }
    if (!ok101)
        return false;
    size_t vlen = 0;
    const char *acc = find_header(buf, len, "Sec-WebSocket-Accept", &vlen);
    if (!acc)
        return false;
    return vlen == strlen(expected_accept) && memcmp(acc, expected_accept, vlen) == 0;
}

size_t ws_client_build_frame(uint8_t *out, size_t cap, uint8_t opcode, const uint8_t *payload, size_t len,
                             const uint8_t mask[4])
{
    if (!out || !mask)
        return 0;
    size_t hdr = 2 + 4; // byte0 + len-byte + 4-byte mask (short form)
    if (len >= 126 && len < 65536)
        hdr += 2;
    else if (len >= 65536)
        hdr += 8;
    if (hdr + len > cap)
        return 0;

    size_t i = 0;
    out[i++] = (uint8_t)(0x80 | (opcode & 0x0F)); // FIN + opcode
    if (len < 126)
        out[i++] = (uint8_t)(0x80 | len);
    else if (len < 65536)
    {
        out[i++] = 0x80 | 126;
        out[i++] = (uint8_t)(len >> 8);
        out[i++] = (uint8_t)(len & 0xFF);
    }
    else
    {
        out[i++] = 0x80 | 127;
        for (int s = 56; s >= 0; s -= 8)
            out[i++] = (uint8_t)((uint64_t)len >> s);
    }
    memcpy(out + i, mask, 4);
    i += 4;
    for (size_t j = 0; j < len; j++)
        out[i + j] = (uint8_t)(payload[j] ^ mask[j & 3]);
    return i + len;
}

bool ws_client_parse_frame(const uint8_t *buf, size_t avail, uint8_t *opcode, bool *fin, size_t *payload_off,
                           size_t *payload_len, size_t *consumed)
{
    if (!buf || avail < 2)
        return false;
    uint8_t b0 = buf[0];
    uint8_t b1 = buf[1];
    bool masked = (b1 & 0x80) != 0;
    uint64_t len = b1 & 0x7F;
    size_t off = 2;
    if (len == 126)
    {
        if (avail < off + 2)
            return false;
        len = ((uint64_t)buf[off] << 8) | buf[off + 1];
        off += 2;
    }
    else if (len == 127)
    {
        if (avail < off + 8)
            return false;
        uint64_t v = 0;
        for (int s = 0; s < 8; s++)
            v = (v << 8) | buf[off + s];
        if (v > 0xFFFFFFFFu) // absurd frame length on a constrained device
            return false;
        len = v;
        off += 8;
    }
    if (masked)
        off += 4; // server frames should not be masked, but stay aligned if they are
    if (avail < off + (size_t)len)
        return false;
    *opcode = (uint8_t)(b0 & 0x0F);
    *fin = (b0 & 0x80) != 0;
    *payload_off = off;
    *payload_len = (size_t)len;
    *consumed = off + (size_t)len;
    return true;
}

// ---------------------------------------------------------------------------
// Transport (ESP32 only): persistent raw-lwIP client + RFC 6455 framing,
// with wss:// over a persistent client TLS session (det_tls csess).
// ---------------------------------------------------------------------------
#if defined(ARDUINO)

#include "network_drivers/transport/det_client.h" // shared outbound TCP client (L4)
#include <Arduino.h>
#include <esp_system.h> // esp_fill_random (per-frame masking key)

#if DETWS_ENABLE_WS_CLIENT_TLS
#include "network_drivers/tls/det_tls.h"
#include <mbedtls/ssl.h>
#endif

#ifdef DETWS_WS_CLIENT_DEBUG
#define WSC_DBG(...) printf(__VA_ARGS__)
#else
#define WSC_DBG(...) ((void)0)
#endif

static WsClientMessageCb s_cb;
static int s_cid = -1;         // outbound connection id (det_client pool)
static volatile bool s_closed; // peer closed / error (set when the pump sees it)
static bool s_ws_up;
static bool s_use_tls;

// Inbound plaintext ring, fed by a pump in the loop: from det_client_read for
// plain ws, from the TLS session (det_tls_csess_read) for wss.
static uint8_t s_rx[DETWS_WS_CLIENT_BUF_SIZE];
static volatile size_t s_rx_head;
static volatile size_t s_rx_tail;
static uint8_t s_pkt[DETWS_WS_CLIENT_BUF_SIZE]; // a frame copied out to parse
static uint8_t s_tx[DETWS_WS_CLIENT_BUF_SIZE];  // outgoing frame scratch

// Fragmented-message reassembly (continuation frames -> one delivered message).
static uint8_t s_msg[DETWS_WS_CLIENT_BUF_SIZE];
static size_t s_msg_len;
static uint8_t s_msg_op;

static inline size_t ring_avail()
{
    return (s_rx_head + sizeof(s_rx) - s_rx_tail) % sizeof(s_rx);
}
static inline uint8_t ring_peek(size_t i)
{
    return s_rx[(s_rx_tail + i) % sizeof(s_rx)];
}
static inline void ring_advance(size_t n)
{
    s_rx_tail = (s_rx_tail + n) % sizeof(s_rx);
}
static void ring_copy(uint8_t *dst, size_t n)
{
    for (size_t i = 0; i < n; i++)
        dst[i] = s_rx[(s_rx_tail + i) % sizeof(s_rx)];
}

// --- transport over the shared outbound client (det_client) ---

static bool ws_tx_plain(const uint8_t *data, size_t len)
{
    return det_client_send(s_cid, data, len);
}

// Drain plaintext wire bytes from the client into the s_rx ring (plain ws).
static void ws_pump_plain()
{
    uint8_t tmp[256];
    for (;;)
    {
        size_t freey = (sizeof(s_rx) - 1) - ring_avail();
        if (freey == 0)
            break;
        size_t want = freey < sizeof(tmp) ? freey : sizeof(tmp);
        size_t n = det_client_read(s_cid, tmp, want);
        if (n == 0)
        {
            if (det_client_is_closed(s_cid))
                s_closed = true;
            break;
        }
        for (size_t i = 0; i < n; i++)
        {
            s_rx[s_rx_head] = tmp[i];
            s_rx_head = (s_rx_head + 1) % sizeof(s_rx);
        }
    }
}

#if DETWS_ENABLE_WS_CLIENT_TLS
// TLS BIO over the shared client: write ciphertext through the pool, read
// ciphertext by draining the client's wire ring.
static int ws_tls_send(void *ctx, const unsigned char *buf, size_t len)
{
    (void)ctx;
    size_t cap = len > 0xFFFF ? 0xFFFF : len;
    return det_client_send(s_cid, buf, cap) ? (int)cap : MBEDTLS_ERR_SSL_WANT_WRITE;
}
static int ws_tls_recv(void *ctx, unsigned char *buf, size_t len)
{
    (void)ctx;
    size_t n = det_client_read(s_cid, buf, len);
    if (n == 0)
        return det_client_is_closed(s_cid) ? 0 : MBEDTLS_ERR_SSL_WANT_READ;
    return (int)n;
}
static void ws_pump_tls()
{
    uint8_t tmp[256];
    for (;;)
    {
        size_t freey = (sizeof(s_rx) - 1) - ring_avail();
        if (freey == 0)
            break;
        size_t want = freey < sizeof(tmp) ? freey : sizeof(tmp);
        int n = det_tls_csess_read(tmp, want);
        if (n <= 0)
        {
            if (n < 0)
                s_closed = true;
            break;
        }
        for (int i = 0; i < n; i++)
        {
            s_rx[s_rx_head] = tmp[i];
            s_rx_head = (s_rx_head + 1) % sizeof(s_rx);
        }
    }
}
#endif // DETWS_ENABLE_WS_CLIENT_TLS

// Send already-framed bytes (plaintext or TLS-encrypted per the mode).
static bool ws_tx(const uint8_t *data, size_t len)
{
#if DETWS_ENABLE_WS_CLIENT_TLS
    if (s_use_tls)
        return det_tls_csess_write(data, len) == (int)len;
#endif
    return ws_tx_plain(data, len);
}

// Frame and send a message with a fresh random masking key (RFC 6455 client rule).
static bool ws_send_frame(uint8_t opcode, const uint8_t *payload, size_t len)
{
    if (!s_ws_up)
        return false;
    uint8_t mask[4];
    esp_fill_random(mask, 4);
    size_t n = ws_client_build_frame(s_tx, sizeof(s_tx), opcode, payload, len, mask);
    return n && ws_tx(s_tx, n);
}

static void ws_close_tcp()
{
#if DETWS_ENABLE_WS_CLIENT_TLS
    if (s_use_tls)
        det_tls_csess_end();
#endif
    if (s_cid >= 0)
        det_client_close(s_cid);
    s_cid = -1;
    s_ws_up = false;
}

static void deliver(uint8_t op, const uint8_t *payload, size_t len)
{
    if (s_cb && (op == WSC_OP_TEXT || op == WSC_OP_BINARY))
        s_cb(op, payload, len);
}

// Dispatch one parsed frame (handles fragmentation, ping/pong, close).
static void handle_frame(uint8_t op, bool fin, const uint8_t *payload, size_t len)
{
    switch (op)
    {
    case WSC_OP_TEXT:
    case WSC_OP_BINARY:
        if (fin)
        {
            deliver(op, payload, len); // common case: unfragmented
        }
        else
        {
            s_msg_op = op; // first fragment
            s_msg_len = len < sizeof(s_msg) ? len : sizeof(s_msg);
            memcpy(s_msg, payload, s_msg_len);
        }
        break;
    case WSC_OP_CONT:
        if (s_msg_len + len <= sizeof(s_msg))
        {
            memcpy(s_msg + s_msg_len, payload, len);
            s_msg_len += len;
        }
        if (fin)
        {
            deliver(s_msg_op, s_msg, s_msg_len);
            s_msg_len = 0;
        }
        break;
    case WSC_OP_PING:
        ws_send_frame(WSC_OP_PONG, payload, len); // echo the application data
        break;
    case WSC_OP_CLOSE:
        ws_send_frame(WSC_OP_CLOSE, nullptr, 0);
        s_closed = true;
        break;
    case WSC_OP_PONG:
    default:
        break;
    }
}

static void process_rx()
{
#if DETWS_ENABLE_WS_CLIENT_TLS
    if (s_use_tls)
        ws_pump_tls();
    else
#endif
        ws_pump_plain();
    for (;;)
    {
        size_t avail = ring_avail();
        if (avail < 2)
            return;
        // Peek the header bytes (a frame header is at most 14 bytes); parse_frame
        // reads only the header and uses the real ring count to test completeness.
        uint8_t hdr[14];
        size_t hn = avail < sizeof(hdr) ? avail : sizeof(hdr);
        for (size_t i = 0; i < hn; i++)
            hdr[i] = ring_peek(i);
        uint8_t op;
        bool fin;
        size_t off, plen, consumed;
        if (!ws_client_parse_frame(hdr, avail, &op, &fin, &off, &plen, &consumed))
            return; // header incomplete or full frame not yet arrived
        if (consumed > sizeof(s_pkt))
        {
            ring_advance(consumed); // oversized frame: drop it
            continue;
        }
        ring_copy(s_pkt, consumed);
        ring_advance(consumed);
        handle_frame(op, fin, s_pkt + off, plen);
    }
}

void ws_client_on_message(WsClientMessageCb cb)
{
    s_cb = cb;
}

bool ws_client_connect(const char *host, uint16_t port, bool use_tls, const char *path)
{
    if (!host || !path)
        return false;
#if !DETWS_ENABLE_WS_CLIENT_TLS
    if (use_tls)
        return false;
#endif
    s_rx_head = s_rx_tail = 0;
    s_closed = s_ws_up = false;
    s_msg_len = 0;
    s_use_tls = use_tls;

    uint32_t deadline = millis() + 8000;

    // Open the TCP connection (DNS + connect) via the shared client transport.
    s_cid = det_client_open(host, port, 8000);
    if (s_cid < 0)
    {
        WSC_DBG("[wsc] det_client_open failed (%d)\n", s_cid);
        return false;
    }

#if DETWS_ENABLE_WS_CLIENT_TLS
    if (s_use_tls)
    {
        if (!det_tls_csess_begin(host, ws_tls_send, ws_tls_recv))
        {
            WSC_DBG("[wsc] csess_begin failed\n");
            ws_close_tcp();
            return false;
        }
        int h;
        while ((h = det_tls_csess_handshake()) == 0 && !s_closed && (int32_t)(deadline - millis()) > 0)
            delay(5);
        if (h != 1)
        {
            WSC_DBG("[wsc] TLS handshake h=%d closed=%d\n", h, (int)s_closed);
            ws_close_tcp();
            return false;
        }
        WSC_DBG("[wsc] TLS handshake ok\n");
    }
#endif

    // Generate a random 16-byte key, base64 it, send the opening handshake.
    uint8_t keyraw[16];
    esp_fill_random(keyraw, sizeof(keyraw));
    char key_b64[25];
    base64_encode(keyraw, sizeof(keyraw), key_b64);
    char expect[32];
    ws_client_accept_for_key(key_b64, expect, sizeof(expect));

    size_t n = ws_client_build_handshake(s_tx, sizeof(s_tx), host, path, key_b64);
    if (n == 0 || !ws_tx(s_tx, n))
    {
        ws_close_tcp();
        return false;
    }

    // Read the response header (up to "\r\n\r\n") out of the rx ring.
    uint8_t hs[512];
    size_t hl = 0;
    bool done = false;
    while (!done && !s_closed && (int32_t)(deadline - millis()) > 0)
    {
#if DETWS_ENABLE_WS_CLIENT_TLS
        if (s_use_tls)
            ws_pump_tls();
        else
#endif
            ws_pump_plain();
        while (ring_avail() > 0 && hl < sizeof(hs))
        {
            hs[hl++] = ring_peek(0);
            ring_advance(1);
            if (hl >= 4 && hs[hl - 4] == '\r' && hs[hl - 3] == '\n' && hs[hl - 2] == '\r' && hs[hl - 1] == '\n')
            {
                done = true;
                break;
            }
        }
        if (!done)
            delay(5);
    }
    if (!done || !ws_client_check_response(hs, hl, expect))
    {
        WSC_DBG("[wsc] handshake fail done=%d hl=%u resp:\n%.*s\n", (int)done, (unsigned)hl, (int)hl, (const char *)hs);
        ws_close_tcp();
        return false;
    }
    s_ws_up = true;
    return true;
}

bool ws_client_send_text(const char *text)
{
    return ws_send_frame(WSC_OP_TEXT, (const uint8_t *)text, text ? strlen(text) : 0);
}
bool ws_client_send_binary(const uint8_t *data, size_t len)
{
    return ws_send_frame(WSC_OP_BINARY, data, len);
}

bool ws_client_loop()
{
    if (!s_ws_up)
        return false;
    process_rx();
    if (s_closed)
    {
        ws_close_tcp();
        return false;
    }
    return true;
}

bool ws_client_connected()
{
    return s_ws_up;
}

void ws_client_close()
{
    if (s_ws_up)
        ws_send_frame(WSC_OP_CLOSE, nullptr, 0);
    ws_close_tcp();
}

#else // host build: transport is a stub

void ws_client_on_message(WsClientMessageCb)
{
}
bool ws_client_connect(const char *, uint16_t, bool, const char *)
{
    return false;
}
bool ws_client_send_text(const char *)
{
    return false;
}
bool ws_client_send_binary(const uint8_t *, size_t)
{
    return false;
}
bool ws_client_loop()
{
    return false;
}
bool ws_client_connected()
{
    return false;
}
void ws_client_close()
{
}

#endif // ARDUINO

#endif // DETWS_ENABLE_WS_CLIENT
