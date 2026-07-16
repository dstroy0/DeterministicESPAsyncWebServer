// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_aesgcm.cpp
 * @brief AES-256-GCM AEAD for SSH (aes256-gcm@openssh.com, RFC 5647) - see ssh_aesgcm.h.
 *
 * Arduino: the AES-256 block is mbedtls_aes_crypt_ecb() (ESP32 HW accelerator). Native: a compact
 * software AES-256 (shared forward S-box + GF(2^8) xtime, no large tables). GHASH and the counter
 * loop are software on both targets; the GF(2^128) reduction mirrors quic_aead.cpp (independent
 * module: AES-256 vs AES-128 and a different build gate, so it is kept self-contained here).
 */

#include "network_drivers/presentation/ssh/crypto/ssh_aesgcm.h"
#include <string.h>

// ===========================================================================
// AES-256 single-block primitive
// ===========================================================================

#ifdef ARDUINO

static inline void aes256_ecb(SshAesGcmCtx *ctx, const uint8_t in[16], uint8_t out[16])
{
    mbedtls_aes_crypt_ecb(&ctx->mbed, MBEDTLS_AES_ENCRYPT, in, out);
}
static inline void aes256_load_key(SshAesGcmCtx *ctx, const uint8_t key[32])
{
    mbedtls_aes_init(&ctx->mbed);
    mbedtls_aes_setkey_enc(&ctx->mbed, key, 256);
}
static inline void aes256_free_key(SshAesGcmCtx *ctx)
{
    mbedtls_aes_free(&ctx->mbed);
}

#else // Native software AES-256

#include "shared_primitives/aes_sbox.h"

namespace
{
// AES round constants (Rcon[1..7] are used by the 256-bit schedule; index 0 unused).
const uint8_t RCON[8] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40};

inline uint8_t xtime(uint8_t a)
{
    return (uint8_t)((a << 1) ^ ((a >> 7) ? 0x1bu : 0x00u));
}

// AES SubWord (FIPS 197 sec 5.2): apply the S-box to each of the four bytes of a 32-bit word.
uint32_t aes_sub_word(uint32_t w)
{
    return ((uint32_t)DET_AES_SBOX[w >> 24] << 24) | ((uint32_t)DET_AES_SBOX[(w >> 16) & 0xff] << 16) |
           ((uint32_t)DET_AES_SBOX[(w >> 8) & 0xff] << 8) | (uint32_t)DET_AES_SBOX[w & 0xff];
}
// AES RotWord (FIPS 197 sec 5.2): cyclically rotate a 32-bit word one byte left.
uint32_t aes_rot_word(uint32_t w)
{
    return (w << 8) | (w >> 24);
}

// AES-256 key schedule (FIPS 197 sec 5.2): Nk=8, Nr=14 -> 15 round keys x 4 words = 60 words.
void aes256_key_expand(const uint8_t key[32], uint32_t rk[60])
{
    for (int i = 0; i < 8; i++)
        rk[i] = ((uint32_t)key[4 * i] << 24) | ((uint32_t)key[4 * i + 1] << 16) | ((uint32_t)key[4 * i + 2] << 8) |
                (uint32_t)key[4 * i + 3];

    for (int i = 8; i < 60; i++)
    {
        uint32_t t = rk[i - 1];
        if (i % 8 == 0)
            t = aes_sub_word(aes_rot_word(t)) ^ ((uint32_t)RCON[i / 8] << 24);
        else if (i % 8 == 4) // AES-256 applies an extra SubWord at the mid-point of each 8-word run.
            t = aes_sub_word(t);
        rk[i] = rk[i - 8] ^ t;
    }
}

// AES-256 block encrypt (FIPS 197 sec 5.1), 14 rounds. State column-major: s[col*4 + row].
void aes256_encrypt_block(const uint32_t rk[60], const uint8_t in[16], uint8_t out[16])
{
    uint8_t s[16];
    for (int i = 0; i < 16; i++)
        s[i] = in[i] ^ (uint8_t)(rk[i / 4] >> (24 - (i % 4) * 8));

    for (int r = 1; r <= 13; r++)
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
        s[i] ^= (uint8_t)(rk[56 + i / 4] >> (24 - (i % 4) * 8));

    memcpy(out, s, 16);
}
} // namespace

// Non-const ctx for signature parity with the ARDUINO path (below), where aes256_ecb wraps
// mbedtls_aes_crypt_ecb() on a non-const mbedtls_aes_context. (S995 does not apply portably.)
static inline void aes256_ecb(SshAesGcmCtx *ctx, const uint8_t in[16], uint8_t out[16])
{
    aes256_encrypt_block(ctx->rk, in, out);
}
static inline void aes256_load_key(SshAesGcmCtx *ctx, const uint8_t key[32])
{
    aes256_key_expand(key, ctx->rk);
}
static inline void aes256_free_key(SshAesGcmCtx *)
{
    // no-op: the key schedule lives in-place in SshAesGcmCtx (a plain array), so there is
    // no external key handle to release. Kept for signature parity with the hardware path.
}

#endif // ARDUINO

// ===========================================================================
// AEAD_AES_256_GCM (NIST SP 800-38D) - software GHASH/GCTR on all targets
// ===========================================================================

namespace
{
inline void xor16(uint8_t *dst, const uint8_t *src)
{
    for (int i = 0; i < 16; i++)
        dst[i] ^= src[i];
}

// GHASH (acc *= H, and fold buffers into acc) is the shared 4-bit-table primitive in
// shared_primitives/ghash.h - ghash_key_init(&ctx->ghk, ctx->h) once at init, then ghash_mul /
// ghash_update on ctx->ghk. Replaced the old 128-iteration bitwise multiply (~37x faster on-device).

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
// advanced in place. @p in / @p out may alias.
void gctr(SshAesGcmCtx *ctx, uint8_t ctr[16], const uint8_t *in, size_t len, uint8_t *out)
{
    uint8_t ks[16];
    size_t off = 0;
    while (off < len)
    {
        aes256_ecb(ctx, ctr, ks);
        inc32(ctr);
        size_t take = len - off;
        if (take > 16)
            take = 16;
        for (size_t i = 0; i < take; i++)
            out[off + i] = in[off + i] ^ ks[i];
        off += take;
    }
}

// Compute J0 (96-bit nonce), the GHASH tag over aad || cipher, and the tag for one GCM operation.
// @p cipher is the ciphertext to authenticate (== output for seal, == input for open). Writes j0[16]
// (for the caller to derive the CTR start) and the 16-byte tag.
void gcm_core(SshAesGcmCtx *ctx, const uint8_t nonce[12], const uint8_t *aad, size_t aad_len, const uint8_t *cipher,
              size_t cipher_len, uint8_t j0[16], uint8_t tag[16])
{
    // 96-bit nonce: J0 = nonce || 0^31 || 1.
    memcpy(j0, nonce, 12);
    j0[12] = 0;
    j0[13] = 0;
    j0[14] = 0;
    j0[15] = 1;

    uint8_t acc[16] = {0};
    ghash_update(&ctx->ghk, acc, aad, aad_len);
    ghash_update(&ctx->ghk, acc, cipher, cipher_len);
    uint8_t lb[16];
    put_be64(lb, (uint64_t)aad_len * 8);
    put_be64(lb + 8, (uint64_t)cipher_len * 8);
    xor16(acc, lb);
    ghash_mul(&ctx->ghk, acc);

    uint8_t ej0[16];
    aes256_ecb(ctx, j0, ej0);
    for (int i = 0; i < 16; i++)
        tag[i] = acc[i] ^ ej0[i];
}

// Advance the RFC 5647 invocation counter: the low 8 bytes of the 12-byte nonce, big-endian; the
// 4-byte fixed field never changes.
inline void iv_increment(uint8_t iv[SSH_AESGCM_IV_LEN])
{
    for (int j = SSH_AESGCM_IV_LEN - 1; j >= 4; j--)
        if (++iv[j])
            break;
}
} // namespace

// ===========================================================================
// Public API
// ===========================================================================

void ssh_aesgcm_init(SshAesGcmCtx *ctx, const uint8_t key[SSH_AESGCM_KEY_LEN], const uint8_t iv[SSH_AESGCM_IV_LEN])
{
    aes256_load_key(ctx, key);
    uint8_t zero[16] = {0};
    aes256_ecb(ctx, zero, ctx->h); // H = E(K, 0^128)
    ghash_key_init(&ctx->ghk, ctx->h);
    memcpy(ctx->iv, iv, SSH_AESGCM_IV_LEN);
    ctx->ready = true;
}

void ssh_aesgcm_seal(SshAesGcmCtx *ctx, const uint8_t *aad, size_t aad_len, const uint8_t *pt, size_t pt_len,
                     uint8_t *out)
{
    // Encrypt with the CTR starting at inc32(J0), then GHASH the resulting ciphertext.
    uint8_t j0[16];
    memcpy(j0, ctx->iv, 12);
    j0[12] = 0;
    j0[13] = 0;
    j0[14] = 0;
    j0[15] = 1;
    uint8_t ctr[16];
    memcpy(ctr, j0, 16);
    inc32(ctr);
    gctr(ctx, ctr, pt, pt_len, out);

    uint8_t j0b[16];
    uint8_t tag[16];
    gcm_core(ctx, ctx->iv, aad, aad_len, out, pt_len, j0b, tag);
    memcpy(out + pt_len, tag, SSH_AESGCM_TAG_LEN);

    iv_increment(ctx->iv);
}

bool ssh_aesgcm_open(SshAesGcmCtx *ctx, const uint8_t *aad, size_t aad_len, const uint8_t *ct, size_t ct_len,
                     const uint8_t tag[SSH_AESGCM_TAG_LEN], uint8_t *out)
{
    // Authenticate over the received ciphertext BEFORE producing any plaintext.
    uint8_t j0[16];
    uint8_t exp_tag[16];
    gcm_core(ctx, ctx->iv, aad, aad_len, ct, ct_len, j0, exp_tag);

    uint8_t diff = 0;
    for (int i = 0; i < SSH_AESGCM_TAG_LEN; i++)
        diff |= (uint8_t)(exp_tag[i] ^ tag[i]);
    if (diff != 0)
        return false; // tag mismatch: nothing written, counter NOT advanced

    uint8_t ctr[16];
    memcpy(ctr, j0, 16);
    inc32(ctr);
    gctr(ctx, ctr, ct, ct_len, out);

    iv_increment(ctx->iv);
    return true;
}

void ssh_aesgcm_wipe(SshAesGcmCtx *ctx)
{
    aes256_free_key(ctx);
    volatile uint8_t *p = (volatile uint8_t *)ctx;
    for (size_t i = 0; i < sizeof(SshAesGcmCtx); i++)
        p[i] = 0;
}
