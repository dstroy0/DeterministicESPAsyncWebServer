// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file snmp_ber.cpp
 * @brief ASN.1 BER codec implementation for the SNMP agent.
 */

#include "services/snmp/snmp_ber.h"

#if DETWS_ENABLE_SNMP

// ---------------------------------------------------------------------------
// Encoder
// ---------------------------------------------------------------------------

void ber_enc_init(BerEnc *e, uint8_t *buf, size_t cap)
{
    e->buf = buf;
    e->cap = cap;
    e->len = 0;
    e->ok = (buf != nullptr && cap > 0);
}

static void enc_byte(BerEnc *e, uint8_t b)
{
    if (!e->ok)
        return;
    if (e->len >= e->cap)
    {
        e->ok = false;
        return;
    }
    e->buf[e->len++] = b;
}

static void enc_bytes(BerEnc *e, const uint8_t *p, size_t n)
{
    for (size_t i = 0; i < n; i++)
        enc_byte(e, p[i]);
}

// Definite length in minimal form (short < 128, else long form).
static void enc_len(BerEnc *e, size_t length)
{
    if (length < 0x80)
    {
        enc_byte(e, (uint8_t)length);
        return;
    }
    uint8_t tmp[4];
    int k = 0;
    size_t v = length;
    while (v)
    {
        tmp[k++] = (uint8_t)(v & 0xFF);
        v >>= 8;
    }
    enc_byte(e, (uint8_t)(0x80 | k));
    for (int i = k - 1; i >= 0; i--)
        enc_byte(e, tmp[i]);
}

bool ber_put_tlv(BerEnc *e, uint8_t tag, const uint8_t *val, size_t n)
{
    enc_byte(e, tag);
    enc_len(e, n);
    enc_bytes(e, val, n);
    return e->ok;
}

bool ber_put_raw(BerEnc *e, const uint8_t *bytes, size_t n)
{
    enc_bytes(e, bytes, n);
    return e->ok;
}

bool ber_put_integer(BerEnc *e, long v)
{
    // Minimal two's-complement encoding.
    uint8_t tmp[8];
    int k = 0;
    // Always emit at least one byte; strip redundant leading 0x00 / 0xFF while
    // preserving sign.
    long val = v;
    do
    {
        tmp[k++] = (uint8_t)(val & 0xFF);
        val >>= 8;
    } while (!((val == 0 && !(tmp[k - 1] & 0x80)) || (val == -1 && (tmp[k - 1] & 0x80))) && k < (int)sizeof(tmp));

    enc_byte(e, BER_INTEGER);
    enc_len(e, (size_t)k);
    for (int i = k - 1; i >= 0; i--)
        enc_byte(e, tmp[i]);
    return e->ok;
}

bool ber_put_uint(BerEnc *e, uint8_t tag, uint32_t v)
{
    // Non-negative integer: big-endian, minimal, with a leading 0x00 if the top
    // bit would otherwise make it look negative.
    uint8_t tmp[5];
    int k = 0;
    uint32_t val = v;
    do
    {
        tmp[k++] = (uint8_t)(val & 0xFF);
        val >>= 8;
    } while (val && k < 4);
    if (tmp[k - 1] & 0x80)
        tmp[k++] = 0x00; // keep it positive

    enc_byte(e, tag);
    enc_len(e, (size_t)k);
    for (int i = k - 1; i >= 0; i--)
        enc_byte(e, tmp[i]);
    return e->ok;
}

bool ber_put_octet_string(BerEnc *e, uint8_t tag, const uint8_t *d, size_t n)
{
    return ber_put_tlv(e, tag, d, n);
}

bool ber_put_null(BerEnc *e)
{
    enc_byte(e, BER_NULL);
    enc_byte(e, 0x00);
    return e->ok;
}

bool ber_put_oid(BerEnc *e, const uint32_t *arcs, size_t n)
{
    if (n < 2)
    {
        e->ok = false;
        return false;
    }
    uint8_t tmp[SNMP_MAX_OID_LEN * 5];
    size_t t = 0;
    // First two arcs combine into one byte/value: 40*arc0 + arc1.
    uint32_t first = 40u * arcs[0] + arcs[1];
    // Base-128, big-endian, high bit set on all but the last byte.
    auto emit = [&](uint32_t v) {
        uint8_t b[5];
        int k = 0;
        do
        {
            b[k++] = (uint8_t)(v & 0x7F);
            v >>= 7;
        } while (v);
        for (int i = k - 1; i >= 0; i--)
        {
            uint8_t byte = b[i];
            if (i != 0)
                byte |= 0x80;
            if (t < sizeof(tmp))
                tmp[t++] = byte;
            else
                e->ok = false;
        }
    };
    emit(first);
    for (size_t i = 2; i < n; i++)
        emit(arcs[i]);
    return ber_put_tlv(e, BER_OID, tmp, t);
}

size_t ber_seq_begin(BerEnc *e, uint8_t tag)
{
    enc_byte(e, tag);
    size_t token = e->len; // position of the (reserved) length field
    enc_byte(e, 0x82);     // long-form, 2 length bytes - back-patched at close
    enc_byte(e, 0x00);
    enc_byte(e, 0x00);
    return token;
}

void ber_seq_end(BerEnc *e, size_t token)
{
    if (!e->ok)
        return;
    size_t content = e->len - (token + 3);
    if (content > 0xFFFF)
    {
        e->ok = false;
        return;
    }
    e->buf[token] = 0x82;
    e->buf[token + 1] = (uint8_t)((content >> 8) & 0xFF);
    e->buf[token + 2] = (uint8_t)(content & 0xFF);
}

// ---------------------------------------------------------------------------
// Decoder
// ---------------------------------------------------------------------------

void ber_dec_init(BerDec *d, const uint8_t *buf, size_t len)
{
    d->buf = buf;
    d->len = len;
    d->pos = 0;
    d->ok = (buf != nullptr);
}

bool ber_read_header(BerDec *d, uint8_t *tag, size_t *length)
{
    if (!d->ok || d->pos >= d->len)
    {
        d->ok = false;
        return false;
    }
    *tag = d->buf[d->pos++];

    if (d->pos >= d->len)
    {
        d->ok = false;
        return false;
    }
    uint8_t l0 = d->buf[d->pos++];
    size_t length_val;
    if (l0 < 0x80)
    {
        length_val = l0;
    }
    else
    {
        int k = l0 & 0x7F;
        if (k == 0 || k > 4 || d->pos + (size_t)k > d->len)
        {
            d->ok = false;
            return false;
        }
        length_val = 0;
        for (int i = 0; i < k; i++)
            length_val = (length_val << 8) | d->buf[d->pos++];
    }
    // Wrap-safe bound: a 4-octet long-form length can be up to 0xFFFFFFFF, so on a 32-bit
    // target d->pos + length_val would overflow and slip under d->len, letting an
    // attacker-supplied length past the check. Compare against the remaining capacity
    // instead (d->pos <= d->len holds here: the count-byte check above bounded d->pos).
    if (length_val > d->len - d->pos)
    {
        d->ok = false;
        return false;
    }
    *length = length_val;
    return true;
}

bool ber_read_integer(BerDec *d, long *out)
{
    uint8_t tag;
    size_t len;
    if (!ber_read_header(d, &tag, &len) || tag != BER_INTEGER || len == 0 || len > 8)
    {
        d->ok = false;
        return false;
    }
    // Sign-extend from the first byte.
    long v = (d->buf[d->pos] & 0x80) ? -1 : 0;
    for (size_t i = 0; i < len; i++)
        v = (v << 8) | d->buf[d->pos + i];
    d->pos += len;
    *out = v;
    return true;
}

bool ber_read_oid(BerDec *d, uint32_t *arcs, size_t max, size_t *n)
{
    uint8_t tag;
    size_t len;
    if (!ber_read_header(d, &tag, &len) || tag != BER_OID || len == 0 || max < 2)
    {
        d->ok = false;
        return false;
    }
    const uint8_t *p = d->buf + d->pos;
    size_t count = 0;
    uint32_t acc = 0;
    bool first_done = false;
    // X.690 8.19: each subidentifier is base-128 (high bit = continuation). The FIRST
    // subidentifier is itself multi-octet-capable and encodes 40*arc0 + arc1.
    for (size_t i = 0; i < len; i++)
    {
        acc = (acc << 7) | (uint32_t)(p[i] & 0x7F);
        if (p[i] & 0x80)
            continue; // more octets in this subidentifier
        if (!first_done)
        {
            uint32_t arc0 = acc / 40u;
            if (arc0 > 2u)
                arc0 = 2u;        // arc0 is 0..2; the remainder goes to arc1 (may exceed 39)
            arcs[count++] = arc0; // count==0 here, max>=2 guaranteed above
            arcs[count++] = acc - 40u * arc0;
            first_done = true;
        }
        else
        {
            if (count >= max)
            {
                d->ok = false;
                return false;
            }
            arcs[count++] = acc;
        }
        acc = 0;
    }
    d->pos += len;
    *n = count;
    return true;
}

bool ber_skip(BerDec *d, size_t length)
{
    if (!d->ok || d->pos > d->len || length > d->len - d->pos) // wrap-safe (see ber_read_header)
    {
        d->ok = false;
        return false;
    }
    d->pos += length;
    return true;
}

#endif // DETWS_ENABLE_SNMP
