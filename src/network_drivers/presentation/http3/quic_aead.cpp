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

namespace
{
// AES S-box (FIPS 197 Figure 7).
const uint8_t SBOX[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76, 0xca, 0x82, 0xc9,
    0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0, 0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f,
    0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15, 0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07,
    0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75, 0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3,
    0x29, 0xe3, 0x2f, 0x84, 0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58,
    0xcf, 0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8, 0x51, 0xa3,
    0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, 0xcd, 0x0c, 0x13, 0xec, 0x5f,
    0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73, 0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88,
    0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb, 0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac,
    0x62, 0x91, 0x95, 0xe4, 0x79, 0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a,
    0xae, 0x08, 0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a, 0x70,
    0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e, 0xe1, 0xf8, 0x98, 0x11,
    0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf, 0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42,
    0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16,
};

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
        return ((uint32_t)SBOX[w >> 24] << 24) | ((uint32_t)SBOX[(w >> 16) & 0xff] << 16) |
               ((uint32_t)SBOX[(w >> 8) & 0xff] << 8) | (uint32_t)SBOX[w & 0xff];
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
            s[i] = SBOX[s[i]];

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
        s[i] = SBOX[s[i]];

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
