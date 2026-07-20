// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ikev2.cpp
 * @brief IKEv2 (RFC 7296) message + payload codec (pure, host-tested).
 */

#include "services/ikev2/ikev2.h"

#if DWS_ENABLE_IKEV2

#include <string.h> // memcpy / memset (framing is hand-rolled)

// ── big-endian scalar helpers ─────────────────────────────────────────────────────────────────
static inline void put16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)v;
}
static inline void put32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}
static inline uint16_t get16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}
static inline uint32_t get32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

// Write a generic payload header (next-payload, no critical bit, 2-byte total length); false on overflow.
static bool put_pl_hdr(uint8_t *buf, size_t cap, IkePayloadType next_payload, size_t total_len)
{
    if (total_len > 0xFFFF || cap < total_len)
        return false;
    buf[0] = (uint8_t)next_payload;
    buf[1] = 0x00;
    put16(buf + 2, (uint16_t)total_len);
    return true;
}

// ── IKE header ────────────────────────────────────────────────────────────────────────────────

size_t dws_ike_hdr_build(uint8_t *buf, size_t cap, const IkeHeader *h)
{
    if (!buf || !h || cap < DWS_IKE_HDR_LEN)
        return 0;
    memcpy(buf, h->init_spi, DWS_IKE_SPI_LEN);
    memcpy(buf + 8, h->resp_spi, DWS_IKE_SPI_LEN);
    buf[16] = (uint8_t)h->next_payload;
    buf[17] = h->version;
    buf[18] = (uint8_t)h->exchange;
    buf[19] = h->flags;
    put32(buf + 20, h->message_id);
    put32(buf + 24, h->length);
    return DWS_IKE_HDR_LEN;
}

bool dws_ike_hdr_parse(const uint8_t *buf, size_t len, IkeHeader *out)
{
    if (!out)
        return false;
    memset(out, 0, sizeof(*out));
    if (!buf || len < DWS_IKE_HDR_LEN)
        return false;
    memcpy(out->init_spi, buf, DWS_IKE_SPI_LEN);
    memcpy(out->resp_spi, buf + 8, DWS_IKE_SPI_LEN);
    out->next_payload = (IkePayloadType)buf[16];
    out->version = buf[17];
    out->exchange = (IkeExchange)buf[18];
    out->flags = buf[19];
    out->message_id = get32(buf + 20);
    out->length = get32(buf + 24);
    return true;
}

bool dws_ike_set_length(uint8_t *buf, size_t buf_cap, uint32_t total_len)
{
    if (!buf || buf_cap < DWS_IKE_HDR_LEN)
        return false;
    put32(buf + 24, total_len);
    return true;
}

// ── generic payload chain ─────────────────────────────────────────────────────────────────────

void dws_ike_payload_iter_init(IkePayloadIter *it, IkePayloadType first_type, const uint8_t *area, size_t area_len)
{
    if (!it)
        return;
    it->area = area;
    it->len = area_len;
    it->off = 0;
    it->next_type = first_type;
}

bool dws_ike_payload_next(IkePayloadIter *it, IkePayload *out)
{
    if (!out)
        return false;
    out->type = IkePayloadType::IKE_PL_NONE;
    out->next_payload = IkePayloadType::IKE_PL_NONE;
    out->critical = false;
    out->body = nullptr;
    out->body_len = 0;
    if (!it || !it->area || it->next_type == IkePayloadType::IKE_PL_NONE)
        return false;
    if (it->off + DWS_IKE_PAYLOAD_HDR_LEN > it->len)
        return false;
    const uint8_t *p = it->area + it->off;
    uint8_t next = p[0];
    bool critical = (p[1] & DWS_IKE_CRITICAL) != 0;
    uint16_t plen = get16(p + 2);
    if (plen < DWS_IKE_PAYLOAD_HDR_LEN || it->off + plen > it->len)
        return false;
    out->type = it->next_type;
    out->next_payload = (IkePayloadType)next;
    out->critical = critical;
    out->body = p + DWS_IKE_PAYLOAD_HDR_LEN;
    out->body_len = (size_t)plen - DWS_IKE_PAYLOAD_HDR_LEN;
    it->next_type = (IkePayloadType)next;
    it->off += plen;
    return true;
}

size_t dws_ike_payload_build(uint8_t *buf, size_t cap, IkePayloadType next_payload, bool critical, const uint8_t *body,
                             size_t body_len)
{
    if (!buf || (body_len && !body))
        return 0;
    size_t total = DWS_IKE_PAYLOAD_HDR_LEN + body_len;
    if (total > 0xFFFF || cap < total)
        return 0;
    buf[0] = (uint8_t)next_payload;
    buf[1] = critical ? DWS_IKE_CRITICAL : 0x00;
    put16(buf + 2, (uint16_t)total);
    if (body_len)
        memcpy(buf + DWS_IKE_PAYLOAD_HDR_LEN, body, body_len);
    return total;
}

// ── typed payload builders ────────────────────────────────────────────────────────────────────

size_t dws_ike_sa_build(uint8_t *buf, size_t cap, IkePayloadType next_payload, uint8_t proposal_num,
                        IkeProtocol protocol_id, const uint8_t *spi, uint8_t spi_size, const IkeTransform *transforms,
                        uint8_t num_transforms)
{
    if (!buf || !transforms || num_transforms == 0)
        return 0;
    if (spi_size && !spi)
        return 0;

    const size_t prop_hdr = 8; // last+res+len(2)+num+proto+spisize+numtrans
    size_t off = DWS_IKE_PAYLOAD_HDR_LEN + prop_hdr + spi_size;
    if (cap < off)
        return 0;

    size_t tstart = off;
    for (uint8_t i = 0; i < num_transforms; i++)
    {
        bool has_key = transforms[i].key_length >= 0;
        size_t tlen = 8 + (has_key ? 4 : 0);
        if (off + tlen > cap)
            return 0;
        buf[off + 0] = (i + 1 == num_transforms) ? 0 : 3; // 0 = last, 3 = more
        buf[off + 1] = 0;
        put16(buf + off + 2, (uint16_t)tlen);
        buf[off + 4] = (uint8_t)transforms[i].type;
        buf[off + 5] = 0;
        put16(buf + off + 6, transforms[i].id);
        if (has_key)
        {
            put16(buf + off + 8, (uint16_t)(0x8000u | IKE_ATTR_KEY_LENGTH)); // TV form, AF bit set
            put16(buf + off + 10, (uint16_t)transforms[i].key_length);
        }
        off += tlen;
    }

    size_t prop_len = prop_hdr + spi_size + (off - tstart);
    size_t sa_total = DWS_IKE_PAYLOAD_HDR_LEN + prop_len;
    if (prop_len > 0xFFFF || sa_total > 0xFFFF)
        return 0;

    buf[0] = (uint8_t)next_payload;
    buf[1] = 0;
    put16(buf + 2, (uint16_t)sa_total);
    uint8_t *pr = buf + DWS_IKE_PAYLOAD_HDR_LEN;
    pr[0] = 0; // last (only) proposal
    pr[1] = 0;
    put16(pr + 2, (uint16_t)prop_len);
    pr[4] = proposal_num;
    pr[5] = (uint8_t)protocol_id;
    pr[6] = spi_size;
    pr[7] = num_transforms;
    if (spi_size)
        memcpy(pr + prop_hdr, spi, spi_size);
    return sa_total;
}

size_t dws_ike_ke_build(uint8_t *buf, size_t cap, IkePayloadType next_payload, uint16_t dh_group, const uint8_t *data,
                        size_t data_len)
{
    if (!buf || (data_len && !data))
        return 0;
    size_t total = DWS_IKE_PAYLOAD_HDR_LEN + 4 + data_len;
    if (!put_pl_hdr(buf, cap, next_payload, total))
        return 0;
    put16(buf + 4, dh_group);
    buf[6] = 0;
    buf[7] = 0;
    if (data_len)
        memcpy(buf + 8, data, data_len);
    return total;
}

size_t dws_ike_nonce_build(uint8_t *buf, size_t cap, IkePayloadType next_payload, const uint8_t *nonce,
                           size_t nonce_len)
{
    if (!buf || (nonce_len && !nonce))
        return 0;
    size_t total = DWS_IKE_PAYLOAD_HDR_LEN + nonce_len;
    if (!put_pl_hdr(buf, cap, next_payload, total))
        return 0;
    if (nonce_len)
        memcpy(buf + 4, nonce, nonce_len);
    return total;
}

size_t dws_ike_id_build(uint8_t *buf, size_t cap, IkePayloadType next_payload, IkeIdType id_type, const uint8_t *data,
                        size_t data_len)
{
    if (!buf || (data_len && !data))
        return 0;
    size_t total = DWS_IKE_PAYLOAD_HDR_LEN + 4 + data_len;
    if (!put_pl_hdr(buf, cap, next_payload, total))
        return 0;
    buf[4] = (uint8_t)id_type;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    if (data_len)
        memcpy(buf + 8, data, data_len);
    return total;
}

size_t dws_ike_auth_build(uint8_t *buf, size_t cap, IkePayloadType next_payload, IkeAuthMethod auth_method,
                          const uint8_t *data, size_t data_len)
{
    if (!buf || (data_len && !data))
        return 0;
    size_t total = DWS_IKE_PAYLOAD_HDR_LEN + 4 + data_len;
    if (!put_pl_hdr(buf, cap, next_payload, total))
        return 0;
    buf[4] = (uint8_t)auth_method;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    if (data_len)
        memcpy(buf + 8, data, data_len);
    return total;
}

size_t dws_ike_cert_build(uint8_t *buf, size_t cap, IkePayloadType next_payload, uint8_t cert_encoding,
                          const uint8_t *data, size_t data_len)
{
    if (!buf || (data_len && !data))
        return 0;
    size_t total = DWS_IKE_PAYLOAD_HDR_LEN + 1 + data_len;
    if (!put_pl_hdr(buf, cap, next_payload, total))
        return 0;
    buf[4] = cert_encoding;
    if (data_len)
        memcpy(buf + 5, data, data_len);
    return total;
}

size_t dws_ike_notify_build(uint8_t *buf, size_t cap, IkePayloadType next_payload, IkeProtocol protocol_id,
                            const uint8_t *spi, uint8_t spi_size, uint16_t notify_type, const uint8_t *data,
                            size_t data_len)
{
    if (!buf || (spi_size && !spi) || (data_len && !data))
        return 0;
    size_t total = DWS_IKE_PAYLOAD_HDR_LEN + 4 + spi_size + data_len;
    if (!put_pl_hdr(buf, cap, next_payload, total))
        return 0;
    buf[4] = (uint8_t)protocol_id;
    buf[5] = spi_size;
    put16(buf + 6, notify_type);
    size_t off = 8;
    if (spi_size)
    {
        memcpy(buf + off, spi, spi_size);
        off += spi_size;
    }
    if (data_len)
        memcpy(buf + off, data, data_len);
    return total;
}

size_t dws_ike_delete_build(uint8_t *buf, size_t cap, IkePayloadType next_payload, IkeProtocol protocol_id,
                            uint8_t spi_size, const uint8_t *spis, uint16_t num_spis)
{
    if (!buf)
        return 0;
    size_t spis_len = (size_t)spi_size * num_spis;
    if (spis_len && !spis)
        return 0;
    size_t total = DWS_IKE_PAYLOAD_HDR_LEN + 4 + spis_len;
    if (!put_pl_hdr(buf, cap, next_payload, total))
        return 0;
    buf[4] = (uint8_t)protocol_id;
    buf[5] = spi_size;
    put16(buf + 6, num_spis);
    if (spis_len)
        memcpy(buf + 8, spis, spis_len);
    return total;
}

size_t dws_ike_ts_build(uint8_t *buf, size_t cap, IkePayloadType next_payload, const IkeTrafficSelector *sels,
                        uint8_t num)
{
    if (!buf || !sels || num == 0)
        return 0;
    size_t off = DWS_IKE_PAYLOAD_HDR_LEN + 4; // generic hdr + num(1) + reserved(3)
    if (cap < off)
        return 0;
    for (uint8_t i = 0; i < num; i++)
    {
        const IkeTrafficSelector *s = &sels[i];
        if ((s->addr_len != 4 && s->addr_len != 16) || !s->start_addr || !s->end_addr)
            return 0;
        size_t sel_len = 8 + 2 * s->addr_len;
        if (off + sel_len > cap)
            return 0;
        buf[off + 0] = (uint8_t)s->ts_type;
        buf[off + 1] = s->ip_protocol;
        put16(buf + off + 2, (uint16_t)sel_len);
        put16(buf + off + 4, s->start_port);
        put16(buf + off + 6, s->end_port);
        memcpy(buf + off + 8, s->start_addr, s->addr_len);
        memcpy(buf + off + 8 + s->addr_len, s->end_addr, s->addr_len);
        off += sel_len;
    }
    if (off > 0xFFFF)
        return 0;
    buf[0] = (uint8_t)next_payload;
    buf[1] = 0;
    put16(buf + 2, (uint16_t)off);
    buf[4] = num;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    return off;
}

size_t dws_ike_sk_build(uint8_t *buf, size_t cap, IkePayloadType next_payload, const uint8_t *iv, size_t iv_len,
                        const uint8_t *ciphertext, size_t ct_len, const uint8_t *icv, size_t icv_len)
{
    if (!buf || (iv_len && !iv) || (ct_len && !ciphertext) || (icv_len && !icv))
        return 0;
    size_t total = DWS_IKE_PAYLOAD_HDR_LEN + iv_len + ct_len + icv_len;
    if (!put_pl_hdr(buf, cap, next_payload, total))
        return 0;
    size_t off = DWS_IKE_PAYLOAD_HDR_LEN;
    if (iv_len)
    {
        memcpy(buf + off, iv, iv_len);
        off += iv_len;
    }
    if (ct_len)
    {
        memcpy(buf + off, ciphertext, ct_len);
        off += ct_len;
    }
    if (icv_len)
        memcpy(buf + off, icv, icv_len);
    return total;
}

// ── typed payload parsers ─────────────────────────────────────────────────────────────────────

bool dws_ike_ke_parse(const uint8_t *body, size_t body_len, uint16_t *dh_group, const uint8_t **data, size_t *data_len)
{
    if (dh_group)
        *dh_group = 0;
    if (data)
        *data = nullptr;
    if (data_len)
        *data_len = 0;
    if (!body || body_len < 4)
        return false;
    if (dh_group)
        *dh_group = get16(body);
    if (data)
        *data = body + 4;
    if (data_len)
        *data_len = body_len - 4;
    return true;
}

bool dws_ike_id_parse(const uint8_t *body, size_t body_len, IkeIdType *id_type, const uint8_t **data, size_t *data_len)
{
    if (id_type)
        *id_type = IkeIdType::IKE_ID_RESERVED;
    if (data)
        *data = nullptr;
    if (data_len)
        *data_len = 0;
    if (!body || body_len < 4)
        return false;
    if (id_type)
        *id_type = (IkeIdType)body[0];
    if (data)
        *data = body + 4;
    if (data_len)
        *data_len = body_len - 4;
    return true;
}

bool dws_ike_auth_parse(const uint8_t *body, size_t body_len, IkeAuthMethod *auth_method, const uint8_t **data,
                        size_t *data_len)
{
    if (auth_method)
        *auth_method = IkeAuthMethod::IKE_AUTH_RESERVED;
    if (data)
        *data = nullptr;
    if (data_len)
        *data_len = 0;
    if (!body || body_len < 4)
        return false;
    if (auth_method)
        *auth_method = (IkeAuthMethod)body[0];
    if (data)
        *data = body + 4;
    if (data_len)
        *data_len = body_len - 4;
    return true;
}

bool dws_ike_notify_parse(const uint8_t *body, size_t body_len, IkeProtocol *protocol_id, uint16_t *notify_type,
                          const uint8_t **spi, uint8_t *spi_size, const uint8_t **data, size_t *data_len)
{
    if (protocol_id)
        *protocol_id = IkeProtocol::IKE_PROTO_NONE;
    if (notify_type)
        *notify_type = 0;
    if (spi)
        *spi = nullptr;
    if (spi_size)
        *spi_size = 0;
    if (data)
        *data = nullptr;
    if (data_len)
        *data_len = 0;
    if (!body || body_len < 4)
        return false;
    uint8_t ss = body[1];
    if (body_len < (size_t)4 + ss)
        return false;
    if (protocol_id)
        *protocol_id = (IkeProtocol)body[0];
    if (spi_size)
        *spi_size = ss;
    if (notify_type)
        *notify_type = get16(body + 2);
    if (spi)
        *spi = ss ? body + 4 : nullptr;
    if (data)
        *data = body + 4 + ss;
    if (data_len)
        *data_len = body_len - 4 - ss;
    return true;
}

bool dws_ike_delete_parse(const uint8_t *body, size_t body_len, IkeProtocol *protocol_id, uint8_t *spi_size,
                          uint16_t *num_spis, const uint8_t **spis)
{
    if (protocol_id)
        *protocol_id = IkeProtocol::IKE_PROTO_NONE;
    if (spi_size)
        *spi_size = 0;
    if (num_spis)
        *num_spis = 0;
    if (spis)
        *spis = nullptr;
    if (!body || body_len < 4)
        return false;
    uint8_t ss = body[1];
    uint16_t num = get16(body + 2);
    if (body_len < (size_t)4 + (size_t)ss * num)
        return false;
    if (protocol_id)
        *protocol_id = (IkeProtocol)body[0];
    if (spi_size)
        *spi_size = ss;
    if (num_spis)
        *num_spis = num;
    if (spis)
        *spis = (ss && num) ? body + 4 : nullptr;
    return true;
}

bool dws_ike_sk_parse(const uint8_t *body, size_t body_len, size_t iv_len, size_t icv_len, const uint8_t **iv,
                      const uint8_t **ciphertext, size_t *ct_len, const uint8_t **icv)
{
    if (iv)
        *iv = nullptr;
    if (ciphertext)
        *ciphertext = nullptr;
    if (ct_len)
        *ct_len = 0;
    if (icv)
        *icv = nullptr;
    if (!body || body_len < iv_len + icv_len)
        return false;
    if (iv)
        *iv = iv_len ? body : nullptr;
    if (ciphertext)
        *ciphertext = body + iv_len;
    if (ct_len)
        *ct_len = body_len - iv_len - icv_len;
    if (icv)
        *icv = icv_len ? body + body_len - icv_len : nullptr;
    return true;
}

// ── SA / proposal / transform parsing ─────────────────────────────────────────────────────────

bool dws_ike_sa_first_proposal(const uint8_t *body, size_t body_len, IkeProposalRef *out)
{
    if (!out)
        return false;
    memset(out, 0, sizeof(*out));
    if (!body || body_len < 8)
        return false;
    uint16_t plen = get16(body + 2);
    uint8_t ss = body[6];
    if (plen < 8 || (size_t)plen > body_len || (size_t)8 + ss > plen)
        return false;
    out->last = (body[0] == 0); // 0 = last, 2 = more proposals follow
    out->proposal_num = body[4];
    out->protocol_id = (IkeProtocol)body[5];
    out->spi_size = ss;
    out->num_transforms = body[7];
    out->spi = ss ? body + 8 : nullptr;
    out->transforms = body + 8 + ss;
    out->transforms_len = (size_t)plen - 8 - ss;
    return true;
}

void dws_ike_transform_iter_init(IkeTransformIter *it, const IkeProposalRef *p)
{
    if (!it)
        return;
    it->area = p ? p->transforms : nullptr;
    it->len = p ? p->transforms_len : 0;
    it->off = 0;
}

bool dws_ike_transform_next(IkeTransformIter *it, IkeTransformRef *out)
{
    if (!out)
        return false;
    out->type = IkeTransformType::IKE_TRANSFORM_ENCR;
    out->id = 0;
    out->key_length = -1;
    out->last = true;
    if (!it || !it->area || it->off + 8 > it->len)
        return false;
    const uint8_t *t = it->area + it->off;
    uint16_t tlen = get16(t + 2);
    if (tlen < 8 || it->off + tlen > it->len)
        return false;
    out->last = (t[0] == 0); // 0 = last, 3 = more
    out->type = (IkeTransformType)t[4];
    out->id = get16(t + 6);

    // Walk the transform attributes (RFC 7296 3.3.5): a 2-byte AF|type; if AF (0x8000) set it is TV
    // (a 2-byte value follows), else TLV (a 2-byte length then that many value bytes).
    size_t ao = 8;
    while (ao + 4 <= tlen)
    {
        uint16_t af_type = get16(t + ao);
        uint16_t atype = af_type & 0x7FFF;
        if (af_type & 0x8000)
        {
            if (atype == IKE_ATTR_KEY_LENGTH)
                out->key_length = get16(t + ao + 2);
            ao += 4;
        }
        else
        {
            uint16_t alen = get16(t + ao + 2);
            ao += (size_t)4 + alen;
        }
    }
    it->off += tlen;
    return true;
}

// ── traffic selector parsing ──────────────────────────────────────────────────────────────────

uint8_t dws_ike_ts_count(const uint8_t *body, size_t body_len)
{
    if (!body || body_len < 4)
        return 0;
    return body[0];
}

bool dws_ike_ts_get(const uint8_t *body, size_t body_len, uint8_t index, IkeTrafficSelector *out)
{
    if (!out)
        return false;
    memset(out, 0, sizeof(*out));
    if (!body || body_len < 4)
        return false;
    uint8_t num = body[0];
    if (index >= num)
        return false;
    size_t off = 4;
    for (uint8_t i = 0; i < num; i++)
    {
        if (off + 8 > body_len)
            return false;
        uint16_t sel_len = get16(body + off + 2);
        if (sel_len < 8 || off + sel_len > body_len || ((sel_len - 8) % 2) != 0)
            return false;
        if (i == index)
        {
            size_t addr_len = (size_t)(sel_len - 8) / 2;
            out->ts_type = (IkeTsType)body[off];
            out->ip_protocol = body[off + 1];
            out->start_port = get16(body + off + 4);
            out->end_port = get16(body + off + 6);
            out->start_addr = body + off + 8;
            out->end_addr = body + off + 8 + addr_len;
            out->addr_len = addr_len;
            return true;
        }
        off += sel_len;
    }
    return false;
}

#endif // DWS_ENABLE_IKEV2
