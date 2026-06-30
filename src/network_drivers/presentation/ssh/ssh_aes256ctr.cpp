// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_aes256ctr.cpp
 * @brief AES-256-CTR implementation.
 *
 * Arduino path: delegates block encryption to mbedtls_aes_crypt_ecb(), which
 * the ESP32 mbedtls port routes to the hardware AES accelerator.
 *
 * Native path: compact software AES-256 using only the 256-byte forward
 * S-box and GF(2^8) multiply-by-2/3 for MixColumns.  No large lookup tables.
 */

#include "ssh_aes256ctr.h"
#include "shared_primitives/shim.h"

// ============================================================================
// ARDUINO - hardware-accelerated path via mbedtls
// ============================================================================

#ifdef ARDUINO

void ssh_aes256ctr_init(SshAesCtrCtx *ctx, const uint8_t key[32], const uint8_t iv[16])
{
    mbedtls_aes_init(&ctx->_mbed);
    mbedtls_aes_setkey_enc(&ctx->_mbed, key, 256);
    memcpy(ctx->counter, iv, 16);
    memset(ctx->keystream, 0, 16);
    ctx->pos = 0;
}

void ssh_aes256ctr_crypt(SshAesCtrCtx *ctx, const uint8_t *in, uint8_t *out, size_t len)
{
    // Encrypt the whole buffer in one mbedtls call: it acquires the HW AES
    // engine / loads the key once and manages the big-endian counter, keystream
    // block, and intra-block offset itself (our fields map 1:1 to its
    // nonce_counter / stream_block / nc_off). This replaces the previous
    // per-16-byte-block mbedtls_aes_crypt_ecb() loop, whose per-block setup
    // dominated bulk throughput on ESP32.
    size_t nc_off = ctx->pos;
    mbedtls_aes_crypt_ctr(&ctx->_mbed, len, &nc_off, ctx->counter, ctx->keystream, in, out);
    ctx->pos = (uint8_t)nc_off; // 0..15
}

void ssh_aes256ctr_wipe(SshAesCtrCtx *ctx)
{
    mbedtls_aes_free(&ctx->_mbed);
    volatile uint8_t *p = (volatile uint8_t *)ctx;
    for (size_t i = 0; i < sizeof(SshAesCtrCtx); i++)
        p[i] = 0;
}

// ============================================================================
// NATIVE - software AES-256 (for host-side unit tests only)
// ============================================================================

#else

// ---------------------------------------------------------------------------
// AES S-box (FIPS 197 Figure 7)
// ---------------------------------------------------------------------------

static const uint8_t SBOX[256] = {
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

// AES round constants for key expansion (Rcon[1..10], index 0 unused)
static const uint8_t RCON[11] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};

// GF(2^8) multiply by 2 (x * 0x02 mod 0x1b irreducible polynomial)
static inline uint8_t xtime(uint8_t a)
{
    return (uint8_t)((a << 1) ^ ((a >> 7) ? 0x1bu : 0x00u));
}

// ---------------------------------------------------------------------------
// AES-256 key schedule expansion (FIPS 197 §5.2)
// AES-256 has Nk=8, Nr=14, producing 15 round keys × 4 words = 60 words.
// ---------------------------------------------------------------------------

static void aes256_key_expand(const uint8_t key[32], uint32_t rk[60])
{
    // SubWord: apply the S-box to each of the four bytes of a word.
    auto sub_word = [](uint32_t w) -> uint32_t {
        return ((uint32_t)SBOX[w >> 24] << 24) | ((uint32_t)SBOX[(w >> 16) & 0xff] << 16) |
               ((uint32_t)SBOX[(w >> 8) & 0xff] << 8) | (uint32_t)SBOX[w & 0xff];
    };
    // RotWord: cyclically rotate a word one byte left ([a,b,c,d] -> [b,c,d,a]).
    auto rot_word = [](uint32_t w) -> uint32_t { return (w << 8) | (w >> 24); };

    // The first Nk=8 words of the schedule are the cipher key verbatim, read as
    // big-endian 32-bit words.
    for (int i = 0; i < 8; i++)
    {
        rk[i] = ((uint32_t)key[4 * i] << 24) | ((uint32_t)key[4 * i + 1] << 16) | ((uint32_t)key[4 * i + 2] << 8) |
                (uint32_t)key[4 * i + 3];
    }

    // Each later word is the word Nk positions back XOR a transform of the
    // previous word. For AES-256 (Nk=8) there are two special positions per
    // group of 8 words (FIPS 197 §5.2):
    for (int i = 8; i < 60; i++)
    {
        uint32_t t = rk[i - 1];
        if (i % 8 == 0)
            // Start of a new key: RotWord, SubWord, then XOR the round constant.
            t = sub_word(rot_word(t)) ^ ((uint32_t)RCON[i / 8] << 24);
        else if (i % 8 == 4)
            // Mid-key SubWord step unique to 256-bit keys.
            t = sub_word(t);
        rk[i] = rk[i - 8] ^ t;
    }
}

// ---------------------------------------------------------------------------
// AES block encrypt (FIPS 197 §5.1) - CTR mode only needs Cipher (encrypt).
// State is column-major: s[col*4 + row].
// ---------------------------------------------------------------------------

static void aes256_encrypt_block(const uint32_t rk[60], const uint8_t in[16], uint8_t out[16])
{
    uint8_t s[16];

    // AddRoundKey (round 0)
    for (int i = 0; i < 16; i++)
        s[i] = in[i] ^ (uint8_t)(rk[i / 4] >> (24 - (i % 4) * 8));

    // Rounds 1–13: SubBytes + ShiftRows + MixColumns + AddRoundKey
    for (int r = 1; r <= 13; r++)
    {
        // SubBytes
        for (int i = 0; i < 16; i++)
            s[i] = SBOX[s[i]];

        // ShiftRows: the state is column-major (s[col*4 + row]), so row r is
        // {s[r], s[4+r], s[8+r], s[12+r]}. Row 0 is unchanged; rows 1/2/3 are
        // cyclically left-rotated by 1/2/3 bytes respectively.
        uint8_t t;
        t = s[1]; // row 1 <<< 1
        s[1] = s[5];
        s[5] = s[9];
        s[9] = s[13];
        s[13] = t;
        t = s[2]; // row 2 <<< 2 (swap the two pairs)
        s[2] = s[10];
        s[10] = t;
        t = s[6];
        s[6] = s[14];
        s[14] = t;
        t = s[15]; // row 3 <<< 3 (== >>> 1)
        s[15] = s[11];
        s[11] = s[7];
        s[7] = s[3];
        s[3] = t;

        // MixColumns: multiply each column by the fixed AES polynomial matrix
        // over GF(2^8). Using e = a^b^c^d, the standard identity gives each
        // output byte as in ^ e ^ xtime(in ^ next), which needs only one
        // GF multiply-by-2 (xtime) per term.
        for (int c = 0; c < 4; c++)
        {
            uint8_t a = s[c * 4], b = s[c * 4 + 1], cc = s[c * 4 + 2], d = s[c * 4 + 3];
            uint8_t e = a ^ b ^ cc ^ d;
            s[c * 4] = a ^ e ^ xtime(a ^ b);
            s[c * 4 + 1] = b ^ e ^ xtime(b ^ cc);
            s[c * 4 + 2] = cc ^ e ^ xtime(cc ^ d);
            s[c * 4 + 3] = d ^ e ^ xtime(d ^ a);
        }

        // AddRoundKey
        for (int i = 0; i < 16; i++)
            s[i] ^= (uint8_t)(rk[r * 4 + i / 4] >> (24 - (i % 4) * 8));
    }

    // Final round: SubBytes + ShiftRows + AddRoundKey (no MixColumns)
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
        s[i] ^= (uint8_t)(rk[56 + i / 4] >> (24 - (i % 4) * 8));

    memcpy(out, s, 16);
}

// ---------------------------------------------------------------------------
// Public API (native software path)
// ---------------------------------------------------------------------------

void ssh_aes256ctr_init(SshAesCtrCtx *ctx, const uint8_t key[32], const uint8_t iv[16])
{
    aes256_key_expand(key, ctx->rk);
    memcpy(ctx->counter, iv, 16);
    memset(ctx->keystream, 0, 16);
    ctx->pos = 0;
}

static void aes_block_sw(SshAesCtrCtx *ctx)
{
    aes256_encrypt_block(ctx->rk, ctx->counter, ctx->keystream);
    for (int j = 15; j >= 0; j--)
        if (++ctx->counter[j])
            break;
}

void ssh_aes256ctr_crypt(SshAesCtrCtx *ctx, const uint8_t *in, uint8_t *out, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        if (ctx->pos == 0)
            aes_block_sw(ctx);
        out[i] = in[i] ^ ctx->keystream[ctx->pos];
        ctx->pos = (uint8_t)((ctx->pos + 1u) & 0x0fu);
    }
}

void ssh_aes256ctr_wipe(SshAesCtrCtx *ctx)
{
    volatile uint8_t *p = (volatile uint8_t *)ctx;
    for (size_t i = 0; i < sizeof(SshAesCtrCtx); i++)
        p[i] = 0;
}

#endif // ARDUINO
