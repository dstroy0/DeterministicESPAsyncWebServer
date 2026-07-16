// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file h3_conn.cpp
 * @brief HTTP/3 application engine over QUIC streams (see h3_conn.h).
 */

#include "network_drivers/presentation/http3/h3_conn.h"

#if DETWS_ENABLE_HTTP3

#include "network_drivers/presentation/http3/qpack.h"
#include "network_drivers/presentation/http3/quic_varint.h"
#include <string.h>

namespace
{
H3Stream *h3_stream_get(H3Conn *h3, uint64_t id, bool create)
{
    H3Stream *free_slot = nullptr;
    for (size_t i = 0; i < DETWS_H3_MAX_STREAMS; i++)
    {
        if (h3->streams[i].role != H3StreamRole::H3_ROLE_FREE && h3->streams[i].id == id)
            return &h3->streams[i];
        if (!free_slot && h3->streams[i].role == H3StreamRole::H3_ROLE_FREE)
            free_slot = &h3->streams[i];
    }
    if (!create || !free_slot)
        return nullptr;
    memset(free_slot, 0, sizeof(*free_slot));
    free_slot->id = id;
    return free_slot;
}

// Copy a bounded, NUL-terminated field.
void set_field(char *dst, size_t cap, const char *src, size_t len)
{
    if (len >= cap)
        len = cap - 1;
    memcpy(dst, src, len);
    dst[len] = '\0';
}

// QPACK emit target: capture the request pseudo-headers.
struct ReqEmit
{
    H3Stream *st;
};
bool req_emit(void *ctx, const char *name, size_t nlen, const char *value, size_t vlen)
{
    H3Stream *st = ((ReqEmit *)ctx)->st;
    if (nlen == 7 && memcmp(name, ":method", 7) == 0)
        set_field(st->method, sizeof(st->method), value, vlen);
    else if (nlen == 5 && memcmp(name, ":path", 5) == 0)
        set_field(st->path, sizeof(st->path), value, vlen);
    else if (nlen == 10 && memcmp(name, ":authority", 10) == 0)
        set_field(st->authority, sizeof(st->authority), value, vlen);
    return true; // ignore regular headers for now (routing is by method + path)
}

// Parse the accumulated request stream: decode HEADERS, coalesce DATA into a body, and dispatch.
void dispatch_request(H3Conn *h3, H3Stream *st)
{
    static uint8_t body[DETWS_H3_STREAM_BUF];
    static char scratch[DETWS_H3_PATH_LEN + DETWS_H3_AUTHORITY_LEN + 64];
    size_t body_len = 0;

    size_t off = 0;
    while (off < st->buf_len)
    {
        H3Frame fr;
        if (!h3_frame_parse(st->buf + off, st->buf_len - off, &fr))
            break;
        size_t payload = off + fr.header_len;
        if (payload + fr.length > st->buf_len)
            break; // incomplete frame
        const uint8_t *fp = st->buf + payload;
        if (fr.type == H3FrameType::H3_HEADERS)
        {
            ReqEmit e = {st};
            qpack_decode(fp, (size_t)fr.length, scratch, sizeof(scratch), req_emit, &e);
            st->have_headers = true;
        }
        else if (fr.type == H3FrameType::H3_DATA)
        {
            // Copy only while there is room left in body. room is 0 once body is full (no underflow),
            // and take is clamped to it, so body_len + take <= sizeof(body).
            size_t room = (body_len < sizeof(body)) ? sizeof(body) - body_len : 0;
            size_t take = (size_t)fr.length;
            if (take > room)
                take = room;
            if (take)
                memcpy(body + body_len, fp, take);
            body_len += take;
        }
        off = payload + (size_t)fr.length;
    }

    if (st->have_headers && h3->on_request)
        h3->on_request(h3->app, h3, st->id, st->method, st->path, st->authority, body, body_len);
}

void append(H3Stream *st, const uint8_t *data, size_t len)
{
    if (len > sizeof(st->buf) - st->buf_len)
        len = sizeof(st->buf) - st->buf_len;
    memcpy(st->buf + st->buf_len, data, len);
    st->buf_len += len;
}

void on_stream_data(void *app, QuicConn *, uint64_t stream_id, const uint8_t *data, size_t len, bool fin)
{
    H3Conn *h3 = (H3Conn *)app;
    H3Stream *st = h3_stream_get(h3, stream_id, true);
    if (!st)
        return;

    if (st->role == H3StreamRole::H3_ROLE_FREE)
        st->role = (stream_id & 0x03) == 0x00 ? H3StreamRole::H3_ROLE_REQUEST : H3StreamRole::H3_ROLE_OTHER_UNI;

    append(st, data, len);

    // A unidirectional stream begins with a stream-type varint; classify it once.
    if (st->role != H3StreamRole::H3_ROLE_REQUEST && !st->type_read && st->buf_len >= 1)
    {
        uint64_t type = 0;
        size_t c = 0;
        if (!quic_varint_decode(st->buf, st->buf_len, &type, &c))
            return; // need more bytes for the varint
        st->type_read = true;
        if (type == 0x00)
            st->role = H3StreamRole::H3_ROLE_CONTROL;
        else if (type == 0x02)
            st->role = H3StreamRole::H3_ROLE_QPACK_ENC;
        else if (type == 0x03)
            st->role = H3StreamRole::H3_ROLE_QPACK_DEC;
        else
            st->role = H3StreamRole::H3_ROLE_OTHER_UNI;
        memmove(st->buf, st->buf + c, st->buf_len - c);
        st->buf_len -= c;
    }

    if (st->role == H3StreamRole::H3_ROLE_CONTROL)
    {
        // The control stream carries SETTINGS first; parse whatever complete frames we have.
        size_t off = 0;
        while (off < st->buf_len)
        {
            H3Frame fr;
            if (!h3_frame_parse(st->buf + off, st->buf_len - off, &fr))
                break;
            if (off + fr.header_len + fr.length > st->buf_len)
                break;
            if (fr.type == H3FrameType::H3_SETTINGS)
            {
                h3_settings_defaults(&h3->peer_settings);
                h3_parse_settings(st->buf + off + fr.header_len, (size_t)fr.length, &h3->peer_settings);
            }
            off += fr.header_len + (size_t)fr.length;
        }
        memmove(st->buf, st->buf + off, st->buf_len - off);
        st->buf_len -= off;
        return;
    }
    if (st->role != H3StreamRole::H3_ROLE_REQUEST)
    {
        st->buf_len = 0; // QPACK/other uni streams: nothing to do (static-table only)
        return;
    }

    if (fin)
        dispatch_request(h3, st);
}

void on_handshake_done(void *app, QuicConn *qc)
{
    H3Conn *h3 = (H3Conn *)app;
    if (h3->control_opened)
        return;
    h3->control_opened = true;

    // Server control stream (id 3): stream type 0x00 + SETTINGS.
    uint8_t buf[64];
    size_t p = quic_varint_encode(buf, sizeof(buf), 0x00);
    const uint64_t ids[] = {H3Setting::H3_SETTINGS_QPACK_MAX_TABLE_CAPACITY,
                            H3Setting::H3_SETTINGS_QPACK_BLOCKED_STREAMS};
    const uint64_t vals[] = {0, 0};
    p += h3_build_settings(buf + p, sizeof(buf) - p, ids, vals, 2);
    quic_conn_stream_send(qc, 3, buf, p, false);

    // QPACK encoder (id 7, type 0x02) and decoder (id 11, type 0x03) streams: type byte only.
    uint8_t t;
    size_t n = quic_varint_encode(&t, 1, 0x02);
    quic_conn_stream_send(qc, 7, &t, n, false);
    n = quic_varint_encode(&t, 1, 0x03);
    quic_conn_stream_send(qc, 11, &t, n, false);
    h3->next_uni_id = 15;
}
} // namespace

void h3_conn_init(H3Conn *h3, QuicConn *qc, H3RequestFn on_request, void *app)
{
    memset(h3, 0, sizeof(*h3));
    h3->qc = qc;
    h3->on_request = on_request;
    h3->app = app;
    h3->next_uni_id = 3;
    for (size_t i = 0; i < DETWS_H3_MAX_STREAMS; i++)
        h3->streams[i].id = UINT64_MAX;
    h3_settings_defaults(&h3->peer_settings);

    QuicConnCallbacks cb = {on_stream_data, on_handshake_done, h3};
    qc->cb = cb;
}

bool h3_conn_respond(H3Conn *h3, uint64_t stream_id, int status, const char *content_type, const uint8_t *body,
                     size_t body_len)
{
    H3Stream *st = h3_stream_get(h3, stream_id, false);
    if (st)
        st->responded = true;

    // QPACK field section: prefix + :status + optional content-type + content-length.
    uint8_t block[256];
    size_t bp = qpack_encode_prefix(block, sizeof(block));
    char st3[4];
    st3[0] = (char)('0' + (status / 100) % 10);
    st3[1] = (char)('0' + (status / 10) % 10);
    st3[2] = (char)('0' + status % 10);
    st3[3] = '\0';
    bp += qpack_encode_header(block + bp, sizeof(block) - bp, ":status", 7, st3, 3);
    if (content_type)
        bp +=
            qpack_encode_header(block + bp, sizeof(block) - bp, "content-type", 12, content_type, strlen(content_type));
    char clen[16];
    size_t cl = 0;
    {
        // decimal content-length without stdlib
        char tmp[16];
        size_t n = 0;
        size_t v = body_len;
        do
        {
            tmp[n++] = (char)('0' + v % 10);
            v /= 10;
        } while (v);
        while (n)
            clen[cl++] = tmp[--n];
    }
    bp += qpack_encode_header(block + bp, sizeof(block) - bp, "content-length", 14, clen, cl);

    // HEADERS frame + DATA frame, sent on the request stream with FIN.
    uint8_t out[DETWS_H3_STREAM_BUF];
    size_t op = h3_build_headers(out, sizeof(out), block, bp);
    if (!op)
        return false;
    if (body_len)
    {
        size_t dn = h3_build_data(out + op, sizeof(out) - op, body, body_len);
        if (!dn)
            return false;
        op += dn;
    }
    return quic_conn_stream_send(h3->qc, stream_id, out, op, true) == op;
}

#endif // DETWS_ENABLE_HTTP3
