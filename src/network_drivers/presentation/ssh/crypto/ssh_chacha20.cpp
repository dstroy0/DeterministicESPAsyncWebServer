// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_chacha20.cpp
 * @brief ChaCha20 (RFC 8439) - implementation. See ssh_chacha20.h.
 */

#include "network_drivers/presentation/ssh/crypto/ssh_chacha20.h"

// ChaCha20 is a hot, pure-integer (add/xor/rotate) keystream generator. The ESP32-S3 has no usable
// vector path (its PIE unit has only a *saturating* 32-bit add, `ee.vadds.s32`; ChaCha needs modular
// wrap-around, so it cannot be vectorized). The real lever is optimization level: the library ships at
// the arduino framework's -Os, and ChaCha runs ~2.36x faster at -O2 (measured on-device, CCOUNT). Force
// -O2 for just this translation unit so the cipher is fast regardless of the consumer's size-optimized
// build. Byte-exact (no logic change); the SIMD investigation is in docs/FEATURE_PERFORMANCE.md.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("O2")
#endif

namespace
{
uint32_t rd_le32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
uint32_t rotl32(uint32_t v, int c)
{
    return (v << c) | (v >> (32 - c));
}

#define QR(a, b, c, d)                                                                                                 \
    a += b;                                                                                                            \
    d ^= a;                                                                                                            \
    d = rotl32(d, 16);                                                                                                 \
    c += d;                                                                                                            \
    b ^= c;                                                                                                            \
    b = rotl32(b, 12);                                                                                                 \
    a += b;                                                                                                            \
    d ^= a;                                                                                                            \
    d = rotl32(d, 8);                                                                                                  \
    c += d;                                                                                                            \
    b ^= c;                                                                                                            \
    b = rotl32(b, 7)

// The ChaCha20 core: 20 rounds over the 16-word state, add the original state, serialize LE.
void chacha_core(const uint32_t in[16], uint8_t out[64])
{
    uint32_t x[16];
    for (int i = 0; i < 16; i++)
        x[i] = in[i];
    for (int i = 0; i < 10; i++) // 10 double-rounds = 20 rounds
    {
        QR(x[0], x[4], x[8], x[12]);
        QR(x[1], x[5], x[9], x[13]);
        QR(x[2], x[6], x[10], x[14]);
        QR(x[3], x[7], x[11], x[15]);
        QR(x[0], x[5], x[10], x[15]);
        QR(x[1], x[6], x[11], x[12]);
        QR(x[2], x[7], x[8], x[13]);
        QR(x[3], x[4], x[9], x[14]);
    }
    for (int i = 0; i < 16; i++)
    {
        uint32_t v = x[i] + in[i];
        out[4 * i + 0] = (uint8_t)v;
        out[4 * i + 1] = (uint8_t)(v >> 8);
        out[4 * i + 2] = (uint8_t)(v >> 16);
        out[4 * i + 3] = (uint8_t)(v >> 24);
    }
}

// "expand 32-byte k"
const uint32_t SIGMA0 = 0x61707865;
const uint32_t SIGMA1 = 0x3320646e;
const uint32_t SIGMA2 = 0x79622d32;
const uint32_t SIGMA3 = 0x6b206574;
} // namespace

void ssh_chacha20_xor(const uint8_t key[SSH_CHACHA20_KEY_LEN], const uint8_t iv[8], uint64_t counter, const uint8_t *in,
                      uint8_t *out, size_t len)
{
    uint32_t st[16];
    st[0] = SIGMA0;
    st[1] = SIGMA1;
    st[2] = SIGMA2;
    st[3] = SIGMA3;
    for (int i = 0; i < 8; i++)
        st[4 + i] = rd_le32(key + 4 * i);
    st[14] = rd_le32(iv + 0); // OpenSSH layout: 64-bit nonce in words 14-15
    st[15] = rd_le32(iv + 4);
    uint8_t ks[64];
    size_t off = 0;
    while (off < len)
    {
        st[12] = (uint32_t)(counter & 0xffffffffu); // 64-bit little-endian counter in words 12-13
        st[13] = (uint32_t)(counter >> 32);
        chacha_core(st, ks);
        size_t n = (len - off < 64) ? (len - off) : 64;
        for (size_t i = 0; i < n; i++)
            out[off + i] = (uint8_t)((in ? in[off + i] : 0) ^ ks[i]);
        off += n;
        counter++;
    }
}

void ssh_chacha20_block_ietf(const uint8_t key[SSH_CHACHA20_KEY_LEN], uint32_t counter, const uint8_t nonce[12],
                             uint8_t out[SSH_CHACHA20_BLOCK_LEN])
{
    uint32_t st[16];
    st[0] = SIGMA0;
    st[1] = SIGMA1;
    st[2] = SIGMA2;
    st[3] = SIGMA3;
    for (int i = 0; i < 8; i++)
        st[4 + i] = rd_le32(key + 4 * i);
    st[12] = counter; // RFC 8439 layout: 32-bit counter, 96-bit nonce
    st[13] = rd_le32(nonce + 0);
    st[14] = rd_le32(nonce + 4);
    st[15] = rd_le32(nonce + 8);
    chacha_core(st, out);
}
