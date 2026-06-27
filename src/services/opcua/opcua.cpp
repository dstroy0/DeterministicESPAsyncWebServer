// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file opcua.cpp
 * @brief OPC UA Binary codec + UACP framing + Hello/Acknowledge (implementation).
 *
 * Pure little-endian codec and handshake logic; the ESP32 section pumps the
 * PROTO_OPCUA rx ring and answers HEL with ACK. No heap, no stdlib.
 */

#include "services/opcua/opcua.h"

#if DETWS_ENABLE_OPCUA

#include <string.h>

// ---------------------------------------------------------------------------
// Built-in type codec
// ---------------------------------------------------------------------------
static void w_bytes(UaWriter *w, const void *src, size_t n)
{
    if (!w->ok || w->n + n > w->cap)
    {
        w->ok = false;
        return;
    }
    memcpy(w->o + w->n, src, n);
    w->n += n;
}

void ua_w_u8(UaWriter *w, uint8_t v)
{
    w_bytes(w, &v, 1);
}
void ua_w_u16(UaWriter *w, uint16_t v)
{
    uint8_t b[2] = {(uint8_t)v, (uint8_t)(v >> 8)};
    w_bytes(w, b, 2);
}
void ua_w_u32(UaWriter *w, uint32_t v)
{
    uint8_t b[4] = {(uint8_t)v, (uint8_t)(v >> 8), (uint8_t)(v >> 16), (uint8_t)(v >> 24)};
    w_bytes(w, b, 4);
}
void ua_w_u64(UaWriter *w, uint64_t v)
{
    uint8_t b[8];
    for (int i = 0; i < 8; i++)
        b[i] = (uint8_t)(v >> (8 * i));
    w_bytes(w, b, 8);
}
void ua_w_i32(UaWriter *w, int32_t v)
{
    ua_w_u32(w, (uint32_t)v);
}
void ua_w_f32(UaWriter *w, float v)
{
    uint32_t u;
    memcpy(&u, &v, 4);
    ua_w_u32(w, u);
}
void ua_w_f64(UaWriter *w, double v)
{
    uint64_t u;
    memcpy(&u, &v, 8);
    ua_w_u64(w, u);
}
void ua_w_bool(UaWriter *w, bool v)
{
    ua_w_u8(w, v ? 1 : 0);
}
void ua_w_string(UaWriter *w, const char *s, int32_t len)
{
    ua_w_i32(w, len);
    if (len > 0 && s)
        w_bytes(w, s, (size_t)len);
}

static bool r_take(UaReader *r, void *dst, size_t n)
{
    if (r->err || r->off + n > r->len)
    {
        r->err = true;
        return false;
    }
    memcpy(dst, r->p + r->off, n);
    r->off += n;
    return true;
}

uint8_t ua_r_u8(UaReader *r)
{
    uint8_t v = 0;
    r_take(r, &v, 1);
    return v;
}
uint16_t ua_r_u16(UaReader *r)
{
    uint8_t b[2] = {0, 0};
    r_take(r, b, 2);
    return (uint16_t)(b[0] | (b[1] << 8));
}
uint32_t ua_r_u32(UaReader *r)
{
    uint8_t b[4] = {0, 0, 0, 0};
    r_take(r, b, 4);
    return (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}
uint64_t ua_r_u64(UaReader *r)
{
    uint8_t b[8];
    if (!r_take(r, b, 8))
        return 0;
    uint64_t v = 0;
    for (int i = 0; i < 8; i++)
        v |= (uint64_t)b[i] << (8 * i);
    return v;
}
int32_t ua_r_i32(UaReader *r)
{
    return (int32_t)ua_r_u32(r);
}
float ua_r_f32(UaReader *r)
{
    uint32_t u = ua_r_u32(r);
    float v;
    memcpy(&v, &u, 4);
    return v;
}
double ua_r_f64(UaReader *r)
{
    uint64_t u = ua_r_u64(r);
    double v;
    memcpy(&v, &u, 8);
    return v;
}
bool ua_r_bool(UaReader *r)
{
    return ua_r_u8(r) != 0;
}
bool ua_r_string(UaReader *r, char *out, size_t cap, int32_t *out_len)
{
    int32_t len = ua_r_i32(r);
    if (r->err)
        return false;
    if (out_len)
        *out_len = len;
    if (len < 0) // null string
    {
        if (cap)
            out[0] = '\0';
        return true;
    }
    if ((size_t)len + 1 > cap || r->off + (size_t)len > r->len)
    {
        r->err = true;
        return false;
    }
    memcpy(out, r->p + r->off, (size_t)len);
    out[len] = '\0';
    r->off += (size_t)len;
    return true;
}

// ---------------------------------------------------------------------------
// UACP framing + handshake
// ---------------------------------------------------------------------------
bool opcua_parse_header(const uint8_t *buf, size_t len, UaMsgHeader *h)
{
    if (!buf || len < 8 || !h)
        return false;
    h->type[0] = (char)buf[0];
    h->type[1] = (char)buf[1];
    h->type[2] = (char)buf[2];
    h->chunk = (char)buf[3];
    h->size = (uint32_t)buf[4] | ((uint32_t)buf[5] << 8) | ((uint32_t)buf[6] << 16) | ((uint32_t)buf[7] << 24);
    return true;
}

bool opcua_parse_hello(const uint8_t *msg, size_t len, OpcUaHello *out)
{
    UaMsgHeader h;
    if (!opcua_parse_header(msg, len, &h) || memcmp(h.type, "HEL", 3) != 0)
        return false;
    if (h.size != len || h.size < 8 + 20) // 8-byte header + at least the five sizes
        return false;
    UaReader r = {msg + 8, len - 8, 0, false};
    out->protocol_version = ua_r_u32(&r);
    out->recv_buf_size = ua_r_u32(&r);
    out->send_buf_size = ua_r_u32(&r);
    out->max_msg_size = ua_r_u32(&r);
    out->max_chunk_count = ua_r_u32(&r);
    return !r.err; // EndpointUrl (a String) follows; not needed for negotiation
}

static uint32_t neg(uint32_t client, uint32_t server)
{
    if (client == 0)
        return server;
    return client < server ? client : server;
}

size_t opcua_build_ack(const OpcUaHello *hello, uint8_t *out, size_t cap)
{
    if (!hello || !out)
        return 0;
    const uint32_t total = 8 + 20; // header + 5 x UInt32
    UaWriter w = {out, cap, 0, true};
    ua_w_u8(&w, 'A');
    ua_w_u8(&w, 'C');
    ua_w_u8(&w, 'K');
    ua_w_u8(&w, 'F');
    ua_w_u32(&w, total);
    ua_w_u32(&w, 0);                                          // ProtocolVersion
    ua_w_u32(&w, neg(hello->send_buf_size, DETWS_OPCUA_BUF)); // our ReceiveBufferSize
    ua_w_u32(&w, neg(hello->recv_buf_size, DETWS_OPCUA_BUF)); // our SendBufferSize
    ua_w_u32(&w, neg(hello->max_msg_size, DETWS_OPCUA_BUF));  // MaxMessageSize
    ua_w_u32(&w, 1);                                          // MaxChunkCount (single-chunk)
    return w.ok ? w.n : 0;
}

// ---------------------------------------------------------------------------
// ESP32 TCP server (PROTO_OPCUA)
// ---------------------------------------------------------------------------
#ifdef ARDUINO

#include "network_drivers/transport/transport.h"

namespace
{
size_t ring_avail(const TcpConn *c)
{
    return (size_t)((c->rx_head - c->rx_tail + RX_BUF_SIZE) % RX_BUF_SIZE);
}
void ring_peek(const TcpConn *c, size_t off, uint8_t *dst, size_t n)
{
    size_t idx = (c->rx_tail + off) % RX_BUF_SIZE;
    for (size_t i = 0; i < n; i++)
    {
        dst[i] = c->rx_buffer[idx];
        idx = (idx + 1) % RX_BUF_SIZE;
    }
}
void ring_consume(TcpConn *c, size_t n)
{
    c->rx_tail = (c->rx_tail + n) % RX_BUF_SIZE;
}
void raw_send(uint8_t slot, const void *data, size_t n)
{
    TcpConn *c = &conn_pool[slot];
    if (c->state != CONN_ACTIVE || !c->pcb || n == 0)
        return;
    det_conn_send(c->id, c->pcb, data, (u16_t)n);
    det_conn_flush(c->id, c->pcb);
}
void close_conn(uint8_t slot)
{
    TcpConn *c = &conn_pool[slot];
    if (c->pcb)
    {
        struct tcp_pcb *p = c->pcb;
        det_conn_detach(p);
        c->state = CONN_FREE;
        c->pcb = nullptr;
        det_conn_close(slot, p);
    }
}

uint8_t s_msg[DETWS_OPCUA_BUF]; // single-accessor reassembly buffer
} // namespace

void opcua_rx(uint8_t slot)
{
    TcpConn *c = &conn_pool[slot];
    if (c->state != CONN_ACTIVE)
        return;
    if (ring_avail(c) < 8)
        return; // need the UACP header

    uint8_t hdr[8];
    ring_peek(c, 0, hdr, 8);
    UaMsgHeader h;
    if (!opcua_parse_header(hdr, 8, &h) || h.size < 8 || h.size > sizeof(s_msg))
    {
        close_conn(slot);
        return;
    }
    if (ring_avail(c) < h.size)
        return; // wait for the full message

    ring_peek(c, 0, s_msg, h.size);
    ring_consume(c, h.size);

    if (memcmp(h.type, "HEL", 3) == 0)
    {
        OpcUaHello hello;
        uint8_t ack[32];
        size_t n;
        if (opcua_parse_hello(s_msg, h.size, &hello) && (n = opcua_build_ack(&hello, ack, sizeof(ack))) > 0)
            raw_send(slot, ack, n);
        else
            close_conn(slot);
    }
    // OPN / MSG / CLO are later increments; ignore until then.
}

#else // host build: the codec/handshake are tested directly; rx is a no-op stub

void opcua_rx(uint8_t slot)
{
    (void)slot;
}

#endif // ARDUINO

#endif // DETWS_ENABLE_OPCUA
