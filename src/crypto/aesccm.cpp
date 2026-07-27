// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file aesccm.cpp
 * @brief AEAD AES-CCM (NIST SP 800-38C) - see aesccm.h.
 *
 * Arduino: mbedtls_ccm (AES routed to the ESP32 HW accelerator), detached tag natively. Native host: a
 * software CCM built on the shared table-free AES block (crypto/aes_block.h) - CBC-MAC for authentication
 * (formatting function per SP 800-38C Appendix A) and AES-CTR for encryption, both under the one key.
 */

#include "crypto/aesccm.h"

#if DWS_ENABLE_SMB

#include "crypto/crypto_opt.h"
#include <string.h>
#ifndef ARDUINO
#include "crypto/aes_block.h" // native software AES-128/256 (mbedtls path uses its own on Arduino)
#endif
DWS_CRYPTO_HOT

#ifdef ARDUINO
// ===========================================================================
// Hardware path: mbedtls_ccm -> ESP32 AES peripheral. Detached tag is native.
// ===========================================================================

bool dws_aesccm_seal_tag(const uint8_t *key, size_t key_len, const uint8_t *nonce, size_t nonce_len, const uint8_t *aad,
                         size_t aad_len, const uint8_t *pt, size_t pt_len, uint8_t *ct_out,
                         uint8_t tag_out[DWS_AESCCM_TAG_LEN])
{
    if (!key || !nonce || !ct_out || !tag_out || (key_len != 16 && key_len != 32))
        return false;
    mbedtls_ccm_context c;
    mbedtls_ccm_init(&c);
    if (mbedtls_ccm_setkey(&c, MBEDTLS_CIPHER_ID_AES, key, (unsigned)(key_len * 8)) != 0)
    {
        mbedtls_ccm_free(&c);
        return false;
    }
    int rc = mbedtls_ccm_encrypt_and_tag(&c, pt_len, nonce, nonce_len, aad, aad_len, pt, ct_out, tag_out,
                                         DWS_AESCCM_TAG_LEN);
    mbedtls_ccm_free(&c);
    return rc == 0;
}

bool dws_aesccm_open_tag(const uint8_t *key, size_t key_len, const uint8_t *nonce, size_t nonce_len, const uint8_t *aad,
                         size_t aad_len, const uint8_t *ct, size_t ct_len, const uint8_t tag[DWS_AESCCM_TAG_LEN],
                         uint8_t *out)
{
    if (!key || !nonce || !ct || !out || !tag || (key_len != 16 && key_len != 32))
        return false;
    mbedtls_ccm_context c;
    mbedtls_ccm_init(&c);
    if (mbedtls_ccm_setkey(&c, MBEDTLS_CIPHER_ID_AES, key, (unsigned)(key_len * 8)) != 0)
    {
        mbedtls_ccm_free(&c);
        return false;
    }
    // mbedtls verifies the tag in constant time and only then keeps the plaintext; non-zero => bad tag.
    int rc = mbedtls_ccm_auth_decrypt(&c, ct_len, nonce, nonce_len, aad, aad_len, ct, out, tag, DWS_AESCCM_TAG_LEN);
    mbedtls_ccm_free(&c);
    if (rc != 0)
    {
        memset(out, 0, ct_len);
        return false;
    }
    return true;
}

#else // native software CCM
// ===========================================================================
// Software path (NIST SP 800-38C): CBC-MAC over B0 || fmt(AAD) || fmt(PT), CTR encryption from A1, and
// the tag encrypted with the counter block A0.
// ===========================================================================

namespace
{
struct CcmKey
{
    uint32_t rk[60]; ///< schedule (44 words used for AES-128, 60 for AES-256)
    int nr;          ///< rounds (10 or 14)
};

inline void ccm_key_init(CcmKey *k, const uint8_t *key, size_t key_len)
{
    if (key_len == 32)
    {
        dws_aes_key_expand(key, 8, k->rk);
        k->nr = 14;
    }
    else
    {
        dws_aes_key_expand(key, 4, k->rk);
        k->nr = 10;
    }
}

inline void ecb(const CcmKey *k, const uint8_t in[16], uint8_t out[16])
{
    dws_aes_encrypt_block(k->rk, k->nr, in, out);
}

// Build the counter block A_i = flags(L-1) || nonce || [i]_L (SP 800-38C Appendix A, the CTR formatting).
inline void ctr_block(uint8_t A[16], const uint8_t *nonce, size_t nonce_len, size_t i)
{
    const size_t L = 15 - nonce_len;
    memset(A, 0, 16);
    A[0] = (uint8_t)(L - 1);
    memcpy(A + 1, nonce, nonce_len);
    for (size_t j = 0; j < L; j++)
        A[15 - j] = (uint8_t)((i >> (8 * j)) & 0xff);
}

// CBC-MAC of B0 || fmt(aad) || fmt(pt) -> raw MAC T[16] (SP 800-38C §6.1 / Appendix A formatting).
void cbc_mac(const CcmKey *k, const uint8_t *nonce, size_t nonce_len, const uint8_t *aad, size_t aad_len,
             const uint8_t *pt, size_t pt_len, uint8_t T[16])
{
    const size_t L = 15 - nonce_len;
    uint8_t X[16] = {0};

    // B0: flags = 64*Adata + 8*((M-2)/2) + (L-1); then nonce; then Q = pt_len big-endian in L bytes.
    uint8_t B0[16] = {0};
    B0[0] = (uint8_t)((aad_len > 0 ? 0x40 : 0x00) | (((DWS_AESCCM_TAG_LEN - 2) / 2) << 3) | (L - 1));
    memcpy(B0 + 1, nonce, nonce_len);
    for (size_t j = 0; j < L; j++)
        B0[15 - j] = (uint8_t)((pt_len >> (8 * j)) & 0xff);
    for (int i = 0; i < 16; i++)
        X[i] ^= B0[i];
    ecb(k, X, X);

    // Associated data: a 2-byte big-endian length prefix (for 0 < aad_len < 0xFF00) then the AAD, packed
    // into 16-byte blocks and zero-padded.
    if (aad_len > 0)
    {
        uint8_t blk[16];
        memset(blk, 0, 16);
        blk[0] = (uint8_t)((aad_len >> 8) & 0xff);
        blk[1] = (uint8_t)(aad_len & 0xff);
        size_t fill = 2;
        size_t off = 0;
        while (off < aad_len)
        {
            size_t take = 16 - fill;
            if (take > aad_len - off)
                take = aad_len - off;
            memcpy(blk + fill, aad + off, take);
            off += take;
            fill += take;
            for (int i = 0; i < 16; i++)
                X[i] ^= blk[i];
            ecb(k, X, X);
            memset(blk, 0, 16);
            fill = 0;
        }
    }

    // Payload blocks, zero-padded to 16.
    size_t off = 0;
    while (off < pt_len)
    {
        uint8_t blk[16];
        memset(blk, 0, 16);
        size_t take = pt_len - off;
        if (take > 16)
            take = 16;
        memcpy(blk, pt + off, take);
        for (int i = 0; i < 16; i++)
            X[i] ^= blk[i];
        ecb(k, X, X);
        off += take;
    }
    memcpy(T, X, 16);
}

// AES-CTR from counter block index @p i0 (A_{i0}, A_{i0+1}, ...). @p in / @p out may alias.
void ctr_crypt(const CcmKey *k, const uint8_t *nonce, size_t nonce_len, size_t i0, const uint8_t *in, size_t len,
               uint8_t *out)
{
    uint8_t A[16];
    uint8_t S[16];
    size_t off = 0;
    size_t i = i0;
    while (off < len)
    {
        ctr_block(A, nonce, nonce_len, i);
        ecb(k, A, S);
        size_t take = len - off;
        if (take > 16)
            take = 16;
        for (size_t j = 0; j < take; j++)
            out[off + j] = in[off + j] ^ S[j];
        off += take;
        i++;
    }
}

// Encrypted tag U = T XOR AES(A0) (the counter block for i = 0 protects the MAC).
void tag_encrypt(const CcmKey *k, const uint8_t *nonce, size_t nonce_len, const uint8_t T[16],
                 uint8_t out_tag[DWS_AESCCM_TAG_LEN])
{
    uint8_t A0[16];
    uint8_t S0[16];
    ctr_block(A0, nonce, nonce_len, 0);
    ecb(k, A0, S0);
    for (int i = 0; i < DWS_AESCCM_TAG_LEN; i++)
        out_tag[i] = (uint8_t)(T[i] ^ S0[i]);
}
} // namespace

bool dws_aesccm_seal_tag(const uint8_t *key, size_t key_len, const uint8_t *nonce, size_t nonce_len, const uint8_t *aad,
                         size_t aad_len, const uint8_t *pt, size_t pt_len, uint8_t *ct_out,
                         uint8_t tag_out[DWS_AESCCM_TAG_LEN])
{
    if (!key || !nonce || !ct_out || !tag_out || (key_len != 16 && key_len != 32) || nonce_len < 7 || nonce_len > 13)
        return false;
    CcmKey k;
    ccm_key_init(&k, key, key_len);
    uint8_t T[16];
    cbc_mac(&k, nonce, nonce_len, aad, aad_len, pt, pt_len, T);
    ctr_crypt(&k, nonce, nonce_len, 1, pt, pt_len, ct_out); // payload from A1
    tag_encrypt(&k, nonce, nonce_len, T, tag_out);          // MAC protected by A0
    return true;
}

bool dws_aesccm_open_tag(const uint8_t *key, size_t key_len, const uint8_t *nonce, size_t nonce_len, const uint8_t *aad,
                         size_t aad_len, const uint8_t *ct, size_t ct_len, const uint8_t tag[DWS_AESCCM_TAG_LEN],
                         uint8_t *out)
{
    if (!key || !nonce || !ct || !out || !tag || (key_len != 16 && key_len != 32) || nonce_len < 7 || nonce_len > 13)
        return false;
    CcmKey k;
    ccm_key_init(&k, key, key_len);
    ctr_crypt(&k, nonce, nonce_len, 1, ct, ct_len, out); // recover plaintext into out
    uint8_t T[16];
    cbc_mac(&k, nonce, nonce_len, aad, aad_len, out, ct_len, T);
    uint8_t exp[DWS_AESCCM_TAG_LEN];
    tag_encrypt(&k, nonce, nonce_len, T, exp);
    uint8_t diff = 0;
    for (int i = 0; i < DWS_AESCCM_TAG_LEN; i++)
        diff |= (uint8_t)(exp[i] ^ tag[i]);
    if (diff != 0)
    {
        memset(out, 0, ct_len); // fail closed: no unauthenticated plaintext escapes
        return false;
    }
    return true;
}

#endif // ARDUINO
#endif // DWS_ENABLE_SMB
