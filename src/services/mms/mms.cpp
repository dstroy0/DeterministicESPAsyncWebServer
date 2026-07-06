// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file mms.cpp
 * @brief IEC 61850 MMS PDU codec (see mms.h).
 */

#include "services/mms/mms.h"

#if DETWS_ENABLE_MMS

#include <string.h>

namespace
{
// BER definite-length octet count for a length value < 64 KiB.
size_t len_octets(size_t len)
{
    if (len < 0x80)
        return 1;
    if (len < 0x100)
        return 2;
    return 3;
}

// Write a BER length; returns octets written.
size_t write_len(uint8_t *p, size_t len)
{
    if (len < 0x80)
    {
        p[0] = (uint8_t)len;
        return 1;
    }
    if (len < 0x100)
    {
        p[0] = 0x81;
        p[1] = (uint8_t)len;
        return 2;
    }
    p[0] = 0x82;
    p[1] = (uint8_t)(len >> 8);
    p[2] = (uint8_t)len;
    return 3;
}

// Write a full TLV (tag + length + value) at out; returns total length, or 0 on overflow.
size_t tlv(uint8_t tag, const uint8_t *val, size_t val_len, uint8_t *out, size_t cap)
{
    // One source of truth for the length-octet count: the value offset (k) and the total (n) both derive
    // from it, so k + val_len == n is provable (write_len writes exactly this many octets).
    size_t lo = len_octets(val_len);
    size_t k = 1 + lo;      // value offset: tag + length octets
    size_t n = k + val_len; // total = offset + value
    if (n > cap)
        return 0;
    out[0] = tag;
    write_len(out + 1, val_len); // writes exactly lo octets
    // The copy writes val_len bytes at out+k, ending at out+n-1 < out+cap (n <= cap, restated at the
    // copy site so the bound is explicit to a static analyzer, not just implied through k and n).
    if (val_len && k + val_len <= cap)
        memcpy(out + k, val, val_len);
    return n;
}

// Minimal big-endian unsigned INTEGER content (with a leading 0 if the MSB is set), into buf; returns len.
size_t int_content(uint32_t v, uint8_t *buf)
{
    uint8_t tmp[4];
    size_t t = 0;
    if (v == 0)
        tmp[t++] = 0;
    else
        for (uint32_t x = v; x; x >>= 8)
            tmp[t++] = (uint8_t)x;
    size_t k = 0;
    if (tmp[t - 1] & 0x80)
        buf[k++] = 0x00;
    for (size_t i = 0; i < t; i++)
        buf[k++] = tmp[t - 1 - i];
    return k;
}
} // namespace

size_t detws_mms_read_request(uint32_t invoke_id, const char *item_name, uint8_t *out, size_t cap)
{
    if (!out || !item_name)
        return 0;
    size_t name_len = strlen(item_name);
    if (name_len > 128)
        return 0;

    uint8_t scratch[256];
    // innermost: objectName VisibleString (0x1A) with the item name.
    size_t n = tlv(0x1A, (const uint8_t *)item_name, name_len, scratch, sizeof(scratch));
    if (!n)
        return 0;
    // A0 name [0]
    uint8_t a0[256];
    n = tlv(0xA0, scratch, n, a0, sizeof(a0));
    if (!n)
        return 0;
    // 30 SEQUENCE (one VariableSpecification)
    n = tlv(0x30, a0, n, scratch, sizeof(scratch));
    if (!n)
        return 0;
    // A0 listOfVariable [0]
    n = tlv(0xA0, scratch, n, a0, sizeof(a0));
    if (!n)
        return 0;
    // A1 variableAccessSpecification [1]
    n = tlv(0xA1, a0, n, scratch, sizeof(scratch));
    if (!n)
        return 0;
    // A4 read [4]
    n = tlv(MMS_SERVICE_READ, scratch, n, a0, sizeof(a0));
    if (!n)
        return 0;

    // Prepend the invokeID INTEGER, then wrap in the confirmed-request PDU.
    uint8_t idc[5];
    size_t idlen = int_content(invoke_id, idc);
    uint8_t body[256];
    size_t bn = tlv(MMS_TAG_INVOKE_ID, idc, idlen, body, sizeof(body));
    if (!bn || bn + n > sizeof(body))
        return 0;
    memcpy(body + bn, a0, n); // append the A4 read
    bn += n;
    return tlv(MMS_PDU_CONFIRMED_REQUEST, body, bn, out, cap);
}

size_t detws_mms_read_response(uint32_t invoke_id, const uint8_t *data, size_t data_len, uint8_t *out, size_t cap)
{
    if (!out || (data_len && !data))
        return 0;
    uint8_t scratch[256];
    // listOfAccessResult SEQUENCE (0xA1 in Read response) wrapping the caller's Data value(s).
    size_t n = tlv(0xA1, data, data_len, scratch, sizeof(scratch));
    if (!n)
        return 0;
    // A4 read response [4]
    uint8_t svc[256];
    n = tlv(MMS_SERVICE_READ, scratch, n, svc, sizeof(svc));
    if (!n)
        return 0;

    uint8_t idc[5];
    size_t idlen = int_content(invoke_id, idc);
    uint8_t body[256];
    size_t bn = tlv(MMS_TAG_INVOKE_ID, idc, idlen, body, sizeof(body));
    if (!bn || bn + n > sizeof(body))
        return 0;
    memcpy(body + bn, svc, n);
    bn += n;
    return tlv(MMS_PDU_CONFIRMED_RESPONSE, body, bn, out, cap);
}

bool detws_mms_parse(const uint8_t *pdu, size_t len, MmsPdu *out)
{
    if (!pdu || !out || len < 2)
        return false;
    out->pdu_tag = pdu[0];
    if (out->pdu_tag != MMS_PDU_CONFIRMED_REQUEST && out->pdu_tag != MMS_PDU_CONFIRMED_RESPONSE &&
        out->pdu_tag != MMS_PDU_CONFIRMED_ERROR)
        return false;
    // Decode the outer length.
    size_t off = 1;
    size_t body_len = pdu[off];
    if (body_len & 0x80)
    {
        size_t nb = body_len & 0x7F;
        if (nb == 0 || nb > 2 || off + 1 + nb > len)
            return false;
        body_len = 0;
        for (size_t i = 0; i < nb; i++)
            body_len = (body_len << 8) | pdu[off + 1 + i];
        off += 1 + nb;
    }
    else
        off += 1;
    if (off + body_len > len)
        return false;

    // First inner element must be the invokeID INTEGER.
    if (off + 2 > len || pdu[off] != MMS_TAG_INVOKE_ID)
        return false;
    size_t idlen = pdu[off + 1];
    if (idlen > 4 || off + 2 + idlen > len)
        return false;
    uint32_t id = 0;
    for (size_t i = 0; i < idlen; i++)
        id = (id << 8) | pdu[off + 2 + i];
    out->invoke_id = id;
    size_t p = off + 2 + idlen;

    // The next element is the confirmedService (A4 read, A5 write, ...).
    if (p < off + body_len && p < len)
    {
        out->service_tag = pdu[p];
        size_t sp = p + 1;
        size_t slen = pdu[sp];
        size_t hdr = 1;
        if (slen & 0x80)
        {
            size_t nb = slen & 0x7F;
            if (nb == 0 || nb > 2 || sp + 1 + nb > len)
                return false;
            slen = 0;
            for (size_t i = 0; i < nb; i++)
                slen = (slen << 8) | pdu[sp + 1 + i];
            hdr = 1 + nb;
        }
        if (sp + hdr + slen > len)
            return false;
        out->service_body = pdu + sp + hdr;
        out->service_len = slen;
    }
    else
    {
        out->service_tag = 0;
        out->service_body = nullptr;
        out->service_len = 0;
    }
    return true;
}

#endif // DETWS_ENABLE_MMS
