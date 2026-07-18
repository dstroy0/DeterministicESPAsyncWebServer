// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file goose.cpp
 * @brief IEC 61850 GOOSE publisher codec (see goose.h).
 */

#include "services/goose/goose.h"

#if DWS_ENABLE_GOOSE

#include <string.h>

namespace
{
// Number of octets to encode a BER definite length.
size_t len_octets(size_t len)
{
    if (len < 0x80)
        return 1;
    size_t n = 0;
    for (size_t v = len; v; v >>= 8)
        n++;
    return 1 + n;
}

// Write a BER length at p; returns octets written.
size_t write_len(uint8_t *p, size_t len)
{
    if (len < 0x80)
    {
        p[0] = (uint8_t)len;
        return 1;
    }
    size_t nb = 0;
    for (size_t v = len; v; v >>= 8)
        nb++;
    p[0] = (uint8_t)(0x80 | nb);
    for (size_t i = 0; i < nb; i++)
        p[1 + i] = (uint8_t)(len >> (8 * (nb - 1 - i)));
    return 1 + nb;
}

// Append a TLV. Returns false on overflow.
bool tlv(uint8_t *out, size_t cap, size_t *n, uint8_t tag, const uint8_t *val, size_t val_len)
{
    size_t need = 1 + len_octets(val_len) + val_len;
    if (*n + need > cap)
        return false;
    out[(*n)++] = tag;
    *n += write_len(out + *n, val_len);
    if (val_len)
    {
        memcpy(out + *n, val, val_len);
        *n += val_len;
    }
    return true;
}

// Append a BER INTEGER (unsigned value, minimal length, leading 0x00 if the MSB is set).
bool dws_ber_int(uint8_t *out, size_t cap, size_t *n, uint8_t tag, uint32_t v)
{
    uint8_t buf[5];
    size_t k = 0;
    // big-endian minimal
    if (v == 0)
        buf[k++] = 0;
    else
    {
        uint8_t tmp[4];
        size_t t = 0;
        for (uint32_t x = v; x; x >>= 8)
            tmp[t++] = (uint8_t)x;
        if (tmp[t - 1] & 0x80)
            buf[k++] = 0x00; // keep it positive
        for (size_t i = 0; i < t; i++)
            buf[k++] = tmp[t - 1 - i];
    }
    return tlv(out, cap, n, tag, buf, k);
}

bool dws_ber_str(uint8_t *out, size_t cap, size_t *n, uint8_t tag, const char *s)
{
    return tlv(out, cap, n, tag, (const uint8_t *)(s ? s : ""), s ? strnlen(s, cap + 1) : 0);
}

bool dws_ber_bool(uint8_t *out, size_t cap, size_t *n, uint8_t tag, bool v)
{
    uint8_t b = v ? 0xFF : 0x00;
    return tlv(out, cap, n, tag, &b, 1);
}
} // namespace

size_t dws_goose_pdu(const DetwsGoose *g, uint8_t *out, size_t cap)
{
    if (!g || !out)
        return 0;
    const size_t RESERVE = 4; // 0x61 tag + up to 3 length octets
    if (cap < RESERVE)
        return 0;

    size_t n = RESERVE; // build the content after the reserved header area
    static const uint8_t ZT[8] = {0};
    bool ok = dws_ber_str(out, cap, &n, 0x80, g->gocb_ref) &&
              dws_ber_int(out, cap, &n, 0x81, g->time_allowed_to_live) && dws_ber_str(out, cap, &n, 0x82, g->dat_set) &&
              dws_ber_str(out, cap, &n, 0x83, g->go_id) && tlv(out, cap, &n, 0x84, g->t ? g->t : ZT, 8) &&
              dws_ber_int(out, cap, &n, 0x85, g->st_num) && dws_ber_int(out, cap, &n, 0x86, g->sq_num) &&
              dws_ber_bool(out, cap, &n, 0x87, g->simulation) && dws_ber_int(out, cap, &n, 0x88, g->conf_rev) &&
              dws_ber_bool(out, cap, &n, 0x89, g->nds_com) && dws_ber_int(out, cap, &n, 0x8A, g->num_entries) &&
              tlv(out, cap, &n, 0xAB, g->all_data, g->all_data_len);
    if (!ok)
        return 0;

    size_t content_len = n - RESERVE;
    size_t hdr = 1 + len_octets(content_len);
    // Move the content down so the 0x61 tag + length sit immediately before it (no gap).
    memmove(out + hdr, out + RESERVE, content_len);
    out[0] = 0x61;
    write_len(out + 1, content_len);
    return hdr + content_len;
}

size_t dws_goose_frame(const uint8_t *dst, const uint8_t *src, uint16_t appid, const DetwsGoose *g, uint8_t *out,
                       size_t cap)
{
    if (!dst || !src || !g || !out || cap < 22)
        return 0;
    // Ethernet II header (ethertype 0x88B8 = GOOSE).
    memcpy(out, dst, 6);
    memcpy(out + 6, src, 6);
    out[12] = 0x88;
    out[13] = 0xB8;
    // GOOSE header: APPID(2), length(2, filled below), reserved1(2), reserved2(2).
    out[14] = (uint8_t)(appid >> 8);
    out[15] = (uint8_t)appid;
    out[18] = 0;
    out[19] = 0;
    out[20] = 0;
    out[21] = 0;

    size_t pdu = dws_goose_pdu(g, out + 22, cap - 22);
    if (!pdu)
        return 0;
    uint16_t goose_len = (uint16_t)(8 + pdu); // GOOSE header (8) + APDU
    out[16] = (uint8_t)(goose_len >> 8);
    out[17] = (uint8_t)goose_len;
    return 22 + pdu;
}

#endif // DWS_ENABLE_GOOSE
