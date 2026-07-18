// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dws_quic_aead.cpp
 * @brief AES-128 block cipher and AEAD_AES_128_GCM (see dws_quic_aead.h).
 *
 * Arduino: the AES block is mbedtls_aes_crypt_ecb() (ESP32 HW accelerator). Native: a compact
 * software AES-128 (forward S-box + GF(2^8) xtime, no large tables). GHASH and the counter loop
 * are software on both targets.
 */

#include "network_drivers/presentation/http3/quic_aead.h"

#if (DWS_ENABLE_HTTP3 || DWS_ENABLE_DTLS)

#include "shared_primitives/ghash.h"
#include <string.h>

// ===========================================================================
// AES-128 single-block primitive
// ===========================================================================

#ifdef ARDUINO

void dws_quic_aes128_init(QuicAes128 *ctx, const uint8_t key[16])
{
    mbedtls_aes_init(&ctx->mbed);
    mbedtls_aes_setkey_enc(&ctx->mbed, key, 128);
}

void dws_quic_aes128_encrypt_block(QuicAes128 *ctx, const uint8_t in[16], uint8_t out[16])
{
    mbedtls_aes_crypt_ecb(&ctx->mbed, MBEDTLS_AES_ENCRYPT, in, out);
}

void dws_quic_aes128_wipe(QuicAes128 *ctx)
{
    mbedtls_aes_free(&ctx->mbed);
}

#else // Native software AES-128

#include "shared_primitives/aes_block.h"

void dws_quic_aes128_init(QuicAes128 *ctx, const uint8_t key[16])
{
    dws_aes_key_expand(key, 4, ctx->rk);
}

void dws_quic_aes128_encrypt_block(QuicAes128 *ctx, const uint8_t in[16], uint8_t out[16])
{
    dws_aes_encrypt_block(ctx->rk, 10, in, out);
}

void dws_quic_aes128_wipe(QuicAes128 *ctx)
{
    volatile uint8_t *p = (volatile uint8_t *)ctx;
    for (size_t i = 0; i < sizeof(QuicAes128); i++)
        p[i] = 0;
}

#endif // ARDUINO

// ===========================================================================
// AEAD_AES_128_GCM (NIST SP 800-38D) - software on all targets
// ===========================================================================

namespace
{
inline void xor16(uint8_t *dst, const uint8_t *src)
{
    for (int i = 0; i < 16; i++)
        dst[i] ^= src[i];
}

// GHASH (acc *= H, and fold buffers into acc) is the shared 4-bit-table primitive in
// shared_primitives/ghash.h: build a GhashKey from H per packet (ghash_key_init), then ghash_update /
// ghash_mul. Replaced the old 128-iteration bitwise multiply (~37x faster on-device).

inline void put_be64(uint8_t *p, uint64_t v)
{
    for (int i = 7; i >= 0; i--)
    {
        p[i] = (uint8_t)(v & 0xff);
        v >>= 8;
    }
}

// Increment the low 32 bits of a 16-byte counter block, big-endian, mod 2^32 (GCM inc32).
inline void inc32(uint8_t ctr[16])
{
    for (int i = 15; i >= 12; i--)
        if (++ctr[i])
            break;
}

// GCTR (NIST SP 800-38D sec 6.5): out = in XOR AES-CTR keystream starting from counter @p ctr,
// which is advanced in place. @p in / @p out may alias.
void gctr(QuicAes128 *aes, uint8_t ctr[16], const uint8_t *in, size_t len, uint8_t *out)
{
    uint8_t ks[16];
    size_t off = 0;
    while (off < len)
    {
        dws_quic_aes128_encrypt_block(aes, ctr, ks);
        inc32(ctr);
        size_t take = len - off;
        if (take > 16)
            take = 16;
        for (size_t i = 0; i < take; i++)
            out[off + i] = in[off + i] ^ ks[i];
        off += take;
    }
}

// Compute H, J0, the GHASH tag and the ciphertext for a 96-bit-nonce GCM operation. @p cipher is
// the ciphertext to authenticate (== output for seal, == input for open). Writes the 16-byte tag.
void gcm_core(QuicAes128 *aes, const uint8_t nonce[12], const uint8_t *aad, size_t aad_len, const uint8_t *cipher,
              size_t cipher_len, uint8_t j0[16], uint8_t tag[16])
{
    uint8_t h[16] = {0};
    dws_quic_aes128_encrypt_block(aes, h, h); // H = E(K, 0^128)
    GhashKey ghk;
    ghash_key_init(&ghk, h);

    // 96-bit nonce: J0 = nonce || 0^31 || 1.
    memcpy(j0, nonce, 12);
    j0[12] = 0;
    j0[13] = 0;
    j0[14] = 0;
    j0[15] = 1;

    uint8_t s[16] = {0};
    ghash_update(&ghk, s, aad, aad_len);
    ghash_update(&ghk, s, cipher, cipher_len);
    uint8_t lb[16];
    put_be64(lb, (uint64_t)aad_len * 8);
    put_be64(lb + 8, (uint64_t)cipher_len * 8);
    xor16(s, lb);
    ghash_mul(&ghk, s);

    uint8_t ej0[16];
    dws_quic_aes128_encrypt_block(aes, j0, ej0);
    for (int i = 0; i < 16; i++)
        tag[i] = s[i] ^ ej0[i];
}
} // namespace

void dws_quic_aes128_gcm_seal(const uint8_t key[16], const uint8_t nonce[12], const uint8_t *aad, size_t aad_len,
                              const uint8_t *pt, size_t pt_len, uint8_t *out)
{
    QuicAes128 aes;
    dws_quic_aes128_init(&aes, key);

    // Encrypt first (counter starts at inc32(J0)), then GHASH the resulting ciphertext.
    uint8_t j0[16];
    memcpy(j0, nonce, 12);
    j0[12] = 0;
    j0[13] = 0;
    j0[14] = 0;
    j0[15] = 1;
    uint8_t ctr[16];
    memcpy(ctr, j0, 16);
    inc32(ctr);
    gctr(&aes, ctr, pt, pt_len, out);

    uint8_t j0b[16];
    uint8_t tag[16];
    gcm_core(&aes, nonce, aad, aad_len, out, pt_len, j0b, tag);
    memcpy(out + pt_len, tag, QUIC_AEAD_TAG_LEN);

    dws_quic_aes128_wipe(&aes);
}

bool dws_quic_aes128_gcm_open(const uint8_t key[16], const uint8_t nonce[12], const uint8_t *aad, size_t aad_len,
                              const uint8_t *ct, size_t ct_len, uint8_t *out)
{
    if (ct_len < QUIC_AEAD_TAG_LEN)
        return false;
    size_t pt_len = ct_len - QUIC_AEAD_TAG_LEN;

    QuicAes128 aes;
    dws_quic_aes128_init(&aes, key);

    // Authenticate over the received ciphertext before producing any plaintext.
    uint8_t j0[16];
    uint8_t tag[16];
    gcm_core(&aes, nonce, aad, aad_len, ct, pt_len, j0, tag);

    uint8_t diff = 0;
    for (int i = 0; i < QUIC_AEAD_TAG_LEN; i++)
        diff |= (uint8_t)(tag[i] ^ ct[pt_len + i]);
    if (diff != 0)
    {
        dws_quic_aes128_wipe(&aes);
        return false;
    }

    uint8_t ctr[16];
    memcpy(ctr, j0, 16);
    inc32(ctr);
    gctr(&aes, ctr, ct, pt_len, out);

    dws_quic_aes128_wipe(&aes);
    return true;
}

#endif // DWS_ENABLE_HTTP3 || DWS_ENABLE_DTLS
