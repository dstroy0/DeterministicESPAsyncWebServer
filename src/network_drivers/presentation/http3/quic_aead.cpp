// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file quic_aead.cpp
 * @brief AES-128 block cipher and AEAD_AES_128_GCM (see quic_aead.h).
 *
 * Arduino: the AES block is mbedtls_aes_crypt_ecb() (ESP32 HW accelerator). Native: a compact
 * software AES-128 (forward S-box + GF(2^8) xtime, no large tables). GHASH and the counter loop
 * are software on both targets.
 */

#include "network_drivers/presentation/http3/quic_aead.h"

#if DETWS_ENABLE_HTTP3

#include <string.h>

// ===========================================================================
// AES-128 single-block primitive
// ===========================================================================

#ifdef ARDUINO

void quic_aes128_init(QuicAes128 *ctx, const uint8_t key[16])
{
    mbedtls_aes_init(&ctx->mbed);
    mbedtls_aes_setkey_enc(&ctx->mbed, key, 128);
}

void quic_aes128_encrypt_block(QuicAes128 *ctx, const uint8_t in[16], uint8_t out[16])
{
    mbedtls_aes_crypt_ecb(&ctx->mbed, MBEDTLS_AES_ENCRYPT, in, out);
}

void quic_aes128_wipe(QuicAes128 *ctx)
{
    mbedtls_aes_free(&ctx->mbed);
}

#else // Native software AES-128

#include "shared_primitives/aes_sbox.h"

namespace
{

// AES round constants (Rcon[1..10]; index 0 unused).
const uint8_t RCON[11] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};

inline uint8_t xtime(uint8_t a)
{
    return (uint8_t)((a << 1) ^ ((a >> 7) ? 0x1bu : 0x00u));
}

// AES-128 key schedule (FIPS 197 sec 5.2): Nk=4, Nr=10 -> 11 round keys x 4 words = 44 words.
void aes128_key_expand(const uint8_t key[16], uint32_t rk[44])
{
    auto sub_word = [](uint32_t w) -> uint32_t {
        return ((uint32_t)DET_AES_SBOX[w >> 24] << 24) | ((uint32_t)DET_AES_SBOX[(w >> 16) & 0xff] << 16) |
               ((uint32_t)DET_AES_SBOX[(w >> 8) & 0xff] << 8) | (uint32_t)DET_AES_SBOX[w & 0xff];
    };
    auto rot_word = [](uint32_t w) -> uint32_t { return (w << 8) | (w >> 24); };

    for (int i = 0; i < 4; i++)
        rk[i] = ((uint32_t)key[4 * i] << 24) | ((uint32_t)key[4 * i + 1] << 16) | ((uint32_t)key[4 * i + 2] << 8) |
                (uint32_t)key[4 * i + 3];

    for (int i = 4; i < 44; i++)
    {
        uint32_t t = rk[i - 1];
        if (i % 4 == 0)
            t = sub_word(rot_word(t)) ^ ((uint32_t)RCON[i / 4] << 24);
        rk[i] = rk[i - 4] ^ t;
    }
}

// AES-128 block encrypt (FIPS 197 sec 5.1). State column-major: s[col*4 + row].
void aes128_encrypt_block(const uint32_t rk[44], const uint8_t in[16], uint8_t out[16])
{
    uint8_t s[16];
    for (int i = 0; i < 16; i++)
        s[i] = in[i] ^ (uint8_t)(rk[i / 4] >> (24 - (i % 4) * 8));

    for (int r = 1; r <= 9; r++)
    {
        for (int i = 0; i < 16; i++)
            s[i] = DET_AES_SBOX[s[i]];

        uint8_t t;
        t = s[1]; // row 1 <<< 1
        s[1] = s[5];
        s[5] = s[9];
        s[9] = s[13];
        s[13] = t;
        t = s[2]; // row 2 <<< 2
        s[2] = s[10];
        s[10] = t;
        t = s[6];
        s[6] = s[14];
        s[14] = t;
        t = s[15]; // row 3 <<< 3
        s[15] = s[11];
        s[11] = s[7];
        s[7] = s[3];
        s[3] = t;

        for (int c = 0; c < 4; c++)
        {
            uint8_t a = s[c * 4], b = s[c * 4 + 1], cc = s[c * 4 + 2], d = s[c * 4 + 3];
            uint8_t e = a ^ b ^ cc ^ d;
            s[c * 4] = a ^ e ^ xtime(a ^ b);
            s[c * 4 + 1] = b ^ e ^ xtime(b ^ cc);
            s[c * 4 + 2] = cc ^ e ^ xtime(cc ^ d);
            s[c * 4 + 3] = d ^ e ^ xtime(d ^ a);
        }

        for (int i = 0; i < 16; i++)
            s[i] ^= (uint8_t)(rk[r * 4 + i / 4] >> (24 - (i % 4) * 8));
    }

    for (int i = 0; i < 16; i++)
        s[i] = DET_AES_SBOX[s[i]];

    uint8_t t;
    t = s[1];
    s[1] = s[5];
    s[5] = s[9];
    s[9] = s[13];
    s[13] = t;
    t = s[2];
    s[2] = s[10];
    s[10] = t;
    t = s[6];
    s[6] = s[14];
    s[14] = t;
    t = s[15];
    s[15] = s[11];
    s[11] = s[7];
    s[7] = s[3];
    s[3] = t;

    for (int i = 0; i < 16; i++)
        s[i] ^= (uint8_t)(rk[40 + i / 4] >> (24 - (i % 4) * 8));

    memcpy(out, s, 16);
}
} // namespace

void quic_aes128_init(QuicAes128 *ctx, const uint8_t key[16])
{
    aes128_key_expand(key, ctx->rk);
}

void quic_aes128_encrypt_block(QuicAes128 *ctx, const uint8_t in[16], uint8_t out[16])
{
    aes128_encrypt_block(ctx->rk, in, out);
}

void quic_aes128_wipe(QuicAes128 *ctx)
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

// Multiply x by y in GF(2^128) with the GCM reduction polynomial, result into x (NIST SP 800-38D
// sec 6.3). Bit 0 is the MSB of byte 0; the field element is shifted right one bit at a time and
// reduced by R = 0xe1 || 0^120 whenever a 1 falls out of the low end. 128 iterations, no tables.
void gf_mul(uint8_t x[16], const uint8_t y[16])
{
    uint8_t z[16] = {0};
    uint8_t v[16];
    memcpy(v, y, 16);
    for (int i = 0; i < 128; i++)
    {
        if ((x[i >> 3] >> (7 - (i & 7))) & 1)
            xor16(z, v);
        uint8_t lsb = v[15] & 1;
        for (int j = 15; j > 0; j--)
            v[j] = (uint8_t)((v[j] >> 1) | (v[j - 1] << 7));
        v[0] >>= 1;
        if (lsb)
            v[0] ^= 0xe1;
    }
    memcpy(x, z, 16);
}

// GHASH update: fold @p len bytes of @p data into accumulator @p acc, MSB-zero-padding a final
// short block, acc = (acc XOR block) * H per 16 bytes.
void ghash_update(const uint8_t h[16], uint8_t acc[16], const uint8_t *data, size_t len)
{
    size_t off = 0;
    while (off < len)
    {
        uint8_t block[16] = {0};
        size_t take = len - off;
        if (take > 16)
            take = 16;
        memcpy(block, data + off, take);
        xor16(acc, block);
        gf_mul(acc, h);
        off += take;
    }
}

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
        quic_aes128_encrypt_block(aes, ctr, ks);
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
    quic_aes128_encrypt_block(aes, h, h); // H = E(K, 0^128)

    // 96-bit nonce: J0 = nonce || 0^31 || 1.
    memcpy(j0, nonce, 12);
    j0[12] = 0;
    j0[13] = 0;
    j0[14] = 0;
    j0[15] = 1;

    uint8_t s[16] = {0};
    ghash_update(h, s, aad, aad_len);
    ghash_update(h, s, cipher, cipher_len);
    uint8_t lb[16];
    put_be64(lb, (uint64_t)aad_len * 8);
    put_be64(lb + 8, (uint64_t)cipher_len * 8);
    xor16(s, lb);
    gf_mul(s, h);

    uint8_t ej0[16];
    quic_aes128_encrypt_block(aes, j0, ej0);
    for (int i = 0; i < 16; i++)
        tag[i] = s[i] ^ ej0[i];
}
} // namespace

void quic_aes128_gcm_seal(const uint8_t key[16], const uint8_t nonce[12], const uint8_t *aad, size_t aad_len,
                          const uint8_t *pt, size_t pt_len, uint8_t *out)
{
    QuicAes128 aes;
    quic_aes128_init(&aes, key);

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

    uint8_t j0b[16], tag[16];
    gcm_core(&aes, nonce, aad, aad_len, out, pt_len, j0b, tag);
    memcpy(out + pt_len, tag, QUIC_AEAD_TAG_LEN);

    quic_aes128_wipe(&aes);
}

bool quic_aes128_gcm_open(const uint8_t key[16], const uint8_t nonce[12], const uint8_t *aad, size_t aad_len,
                          const uint8_t *ct, size_t ct_len, uint8_t *out)
{
    if (ct_len < QUIC_AEAD_TAG_LEN)
        return false;
    size_t pt_len = ct_len - QUIC_AEAD_TAG_LEN;

    QuicAes128 aes;
    quic_aes128_init(&aes, key);

    // Authenticate over the received ciphertext before producing any plaintext.
    uint8_t j0[16], tag[16];
    gcm_core(&aes, nonce, aad, aad_len, ct, pt_len, j0, tag);

    uint8_t diff = 0;
    for (int i = 0; i < QUIC_AEAD_TAG_LEN; i++)
        diff |= (uint8_t)(tag[i] ^ ct[pt_len + i]);
    if (diff != 0)
    {
        quic_aes128_wipe(&aes);
        return false;
    }

    uint8_t ctr[16];
    memcpy(ctr, j0, 16);
    inc32(ctr);
    gctr(&aes, ctr, ct, pt_len, out);

    quic_aes128_wipe(&aes);
    return true;
}

#endif // DETWS_ENABLE_HTTP3
