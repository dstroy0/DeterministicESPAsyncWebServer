// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ntlm.cpp
 * @brief NTLMv2 response computation (see ntlm.h).
 */

#include "ntlm.h"

#if DWS_ENABLE_SMB

#include "services/smb/smb_md.h"
#include <string.h>

void ntlm_nt_hash(const char *password, uint8_t nt_hash[16])
{
    MdCtx c;
    md4_init(&c);
    for (const char *p = password; *p; p++)
    {
        uint8_t pair[2] = {(uint8_t)*p, 0}; // UTF-16LE (ASCII/UTF-8 code unit + high byte 0)
        md4_update(&c, pair, 2);
    }
    md4_final(&c, nt_hash);
}

bool ntlm_ntowfv2(const uint8_t nt_hash[16], const char *user, const char *domain, uint8_t owf[16])
{
    uint8_t buf[512]; // UTF-16LE of Uppercase(user) + domain; 256 chars max
    size_t n = 0;
    for (const char *p = user; *p; p++)
    {
        char up = *p;
        if (up >= 'a' && up <= 'z')
            up = (char)(up - 32); // ASCII uppercase (only the user, per MS-NLMP)
        if (n + 2 > sizeof(buf))
            return false;
        buf[n++] = (uint8_t)up;
        buf[n++] = 0;
    }
    for (const char *p = domain; *p; p++)
    {
        if (n + 2 > sizeof(buf))
            return false;
        buf[n++] = (uint8_t)*p;
        buf[n++] = 0;
    }
    hmac_md5(nt_hash, 16, buf, n, owf);
    return true;
}

// HMAC-MD5 over a two-part message (the key here is always the 16-byte NTOWFv2, < 64 bytes,
// so no key-shortening is needed).
static void hmac_md5_2(const uint8_t key[16], const uint8_t *m1, size_t l1, const uint8_t *m2, size_t l2,
                       uint8_t out[16])
{
    uint8_t ipad[64];
    uint8_t opad[64];
    for (int i = 0; i < 64; i++)
    {
        uint8_t k = (i < 16) ? key[i] : 0;
        ipad[i] = (uint8_t)(k ^ 0x36);
        opad[i] = (uint8_t)(k ^ 0x5c);
    }
    uint8_t inner[16];
    MdCtx c;
    md5_init(&c);
    md5_update(&c, ipad, 64);
    md5_update(&c, m1, l1);
    if (m2 && l2)
        md5_update(&c, m2, l2);
    md5_final(&c, inner);
    md5_init(&c);
    md5_update(&c, opad, 64);
    md5_update(&c, inner, 16);
    md5_final(&c, out);
}

size_t ntlm_v2_response(const uint8_t owf[16], const uint8_t server_challenge[8], const uint8_t client_challenge[8],
                        const uint8_t timestamp[8], const uint8_t *target_info, size_t ti_len, uint8_t *out,
                        size_t out_cap, uint8_t session_key[16])
{
    const size_t temp_len = 2 + 6 + 8 + 8 + 4 + ti_len + 4; // MS-NLMP temp layout
    const size_t resp_len = 16 + temp_len;                  // NTProofStr(16) + temp
    if (!out || resp_len > out_cap)
        return 0;

    // Build temp in place at out+16, so the result is NTProofStr(16) || temp contiguously.
    uint8_t *temp = out + 16;
    size_t k = 0;
    temp[k++] = 0x01;       // Responserversion
    temp[k++] = 0x01;       // HiResponserversion
    memset(temp + k, 0, 6); // Z(6)
    k += 6;
    memcpy(temp + k, timestamp, 8);
    k += 8;
    memcpy(temp + k, client_challenge, 8);
    k += 8;
    memset(temp + k, 0, 4); // Z(4)
    k += 4;
    memcpy(temp + k, target_info, ti_len);
    k += ti_len;
    memset(temp + k, 0, 4); // Z(4) trailer; temp_len (line 83) already accounts for it, so k is done

    uint8_t ntproof[16];
    hmac_md5_2(owf, server_challenge, 8, temp, temp_len, ntproof);
    memcpy(out, ntproof, 16); // out = NTProofStr || temp
    if (session_key)
        hmac_md5(owf, 16, ntproof, 16, session_key);
    return resp_len;
}

#endif // DWS_ENABLE_SMB
