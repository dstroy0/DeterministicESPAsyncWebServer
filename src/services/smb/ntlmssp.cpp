// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ntlmssp.cpp
 * @brief NTLMSSP message codec implementation (see ntlmssp.h). Little-endian; text UTF-16LE.
 */

#include "ntlmssp.h"

#if DETWS_ENABLE_SMB

#include <string.h>

static const uint8_t NTLMSSP_SIG[8] = {'N', 'T', 'L', 'M', 'S', 'S', 'P', 0};

static uint16_t rd16(const uint8_t *p)
{
    return (uint16_t)(p[0] | (p[1] << 8));
}
static uint32_t rd32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static void wr16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
}
static void wr32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

// Write a Len/MaxLen/BufferOffset field triplet at @p f.
static void wr_field(uint8_t *f, uint16_t len, uint32_t off)
{
    wr16(f + 0, len);
    wr16(f + 2, len); // MaxLen == Len
    wr32(f + 4, off);
}

size_t ntlmssp_build_negotiate(uint8_t *buf, size_t cap, uint32_t flags)
{
    if (!buf || cap < 32)
        return 0;
    memset(buf, 0, 32);
    memcpy(buf + 0, NTLMSSP_SIG, 8); // Signature
    wr32(buf + 8, 1);                // MessageType = NEGOTIATE
    wr32(buf + 12, flags);           // NegotiateFlags
    wr_field(buf + 16, 0, 32);       // DomainNameFields (empty; offset = end of header)
    wr_field(buf + 24, 0, 32);       // WorkstationFields (empty)
    return 32;
}

bool ntlmssp_parse_challenge(const uint8_t *msg, size_t len, NtlmChallenge *out)
{
    if (!msg || !out || len < 48) // through TargetInfoFields
        return false;
    if (memcmp(msg, NTLMSSP_SIG, 8) != 0 || rd32(msg + 8) != 2)
        return false;
    out->flags = rd32(msg + 20);
    memcpy(out->server_challenge, msg + 24, 8);
    uint16_t ti_len = rd16(msg + 40);
    uint32_t ti_off = rd32(msg + 44);
    if (ti_len == 0)
    {
        out->target_info = nullptr;
        out->target_info_len = 0;
        return true;
    }
    if ((size_t)ti_off + ti_len > len) // target info out of bounds -> fail closed
        return false;
    out->target_info = msg + ti_off;
    out->target_info_len = ti_len;
    return true;
}

// Append the UTF-16LE encoding of @p s to buf[at..]; returns the byte count (2 * strlen).
static size_t put_utf16le(uint8_t *buf, const char *s)
{
    size_t n = 0;
    if (s)
        for (const char *p = s; *p; p++)
        {
            buf[n++] = (uint8_t)*p;
            buf[n++] = 0;
        }
    return n;
}
static size_t utf16_len(const char *s)
{
    size_t n = 0;
    if (s)
        while (s[n])
            n++;
    return n * 2;
}

size_t ntlmssp_build_authenticate(uint8_t *buf, size_t cap, const uint8_t *lm_resp, size_t lm_len,
                                  const uint8_t *nt_resp, size_t nt_len, const char *domain, const char *user,
                                  const char *workstation, uint32_t flags)
{
    const size_t HDR = 64; // fixed part (no Version, no MIC)
    size_t dlen = utf16_len(domain);
    size_t ulen = utf16_len(user);
    size_t wlen = utf16_len(workstation);
    size_t total = HDR + lm_len + nt_len + dlen + ulen + wlen; // session key empty
    if (!buf || total > cap)
        return 0;

    memset(buf, 0, HDR);
    memcpy(buf + 0, NTLMSSP_SIG, 8); // Signature
    wr32(buf + 8, 3);                // MessageType = AUTHENTICATE

    // Lay out the payload after the fixed header, then point each field at it.
    size_t off = HDR;
    size_t lm_off = off;
    if (lm_resp && lm_len)
        memcpy(buf + off, lm_resp, lm_len);
    off += lm_len;
    size_t nt_off = off;
    if (nt_resp && nt_len)
        memcpy(buf + off, nt_resp, nt_len);
    off += nt_len;
    size_t dom_off = off;
    off += put_utf16le(buf + off, domain);
    size_t usr_off = off;
    off += put_utf16le(buf + off, user);
    size_t wks_off = off;
    off += put_utf16le(buf + off, workstation);
    size_t key_off = off; // EncryptedRandomSessionKey empty

    wr_field(buf + 12, (uint16_t)lm_len, (uint32_t)lm_off); // LmChallengeResponseFields
    wr_field(buf + 20, (uint16_t)nt_len, (uint32_t)nt_off); // NtChallengeResponseFields
    wr_field(buf + 28, (uint16_t)dlen, (uint32_t)dom_off);  // DomainNameFields
    wr_field(buf + 36, (uint16_t)ulen, (uint32_t)usr_off);  // UserNameFields
    wr_field(buf + 44, (uint16_t)wlen, (uint32_t)wks_off);  // WorkstationFields
    wr_field(buf + 52, 0, (uint32_t)key_off);               // EncryptedRandomSessionKeyFields
    wr32(buf + 60, flags);                                  // NegotiateFlags
    return total;
}

#endif // DETWS_ENABLE_SMB
