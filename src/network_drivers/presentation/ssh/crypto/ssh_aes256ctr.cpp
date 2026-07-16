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

#include "network_drivers/presentation/ssh/crypto/ssh_aes256ctr.h"
#include <string.h>

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

#include "shared_primitives/aes_sbox.h"

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

// SubWord: apply the S-box to each of the four bytes of a word.
static uint32_t aes_sub_word(uint32_t w)
{
    return ((uint32_t)DET_AES_SBOX[w >> 24] << 24) | ((uint32_t)DET_AES_SBOX[(w >> 16) & 0xff] << 16) |
           ((uint32_t)DET_AES_SBOX[(w >> 8) & 0xff] << 8) | (uint32_t)DET_AES_SBOX[w & 0xff];
}
// RotWord: cyclically rotate a word one byte left ([a,b,c,d] -> [b,c,d,a]).
static uint32_t aes_rot_word(uint32_t w)
{
    return (w << 8) | (w >> 24);
}

static void aes256_key_expand(const uint8_t key[32], uint32_t rk[60])
{
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
            t = aes_sub_word(aes_rot_word(t)) ^ ((uint32_t)RCON[i / 8] << 24);
        else if (i % 8 == 4)
            // Mid-key SubWord step unique to 256-bit keys.
            t = aes_sub_word(t);
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
            s[i] = DET_AES_SBOX[s[i]];

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
            uint8_t a = s[c * 4];
            uint8_t b = s[c * 4 + 1];
            uint8_t cc = s[c * 4 + 2];
            uint8_t d = s[c * 4 + 3];
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
