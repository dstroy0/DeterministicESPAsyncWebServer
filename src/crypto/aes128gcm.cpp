// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file aes128gcm.cpp
 * @brief AES-128 block cipher and AEAD_AES_128_GCM (see aes128gcm.h).
 *
 * Arduino: the AES block is mbedtls_aes_crypt_ecb() (ESP32 HW accelerator). Native: a compact
 * software AES-128 (forward S-box + GF(2^8) xtime, no large tables). GHASH and the counter loop
 * are software on both targets. The AEAD's working memory (key schedule, GHASH table, keystream,
 * accumulator, tag mask, counters) lives in an Aes128GcmWork over the shared crypto scratch and is
 * wiped on exit; no cipher state on the stack.
 */

#include "crypto/aes128gcm.h"

#if (DWS_ENABLE_HTTP3 || DWS_ENABLE_DTLS || DWS_ENABLE_SMB)

#include "crypto/crypto_scratch.h" // crypto_work + dws_crypto_wipe + dws_ct_eq (the shared crypto scratch)
#include "crypto/ghash.h"
#include <string.h>
#ifdef ARDUINO
#include <mbedtls/aes.h> // DwsAes128 wraps mbedtls_aes_context on ESP32 (definition private to this TU)
#else
#include "crypto/aes_block.h" // native software AES-128 (Arduino uses the mbedtls HW block below)
#endif

// DwsAes128 - definition private to this TU (aes128gcm.h forward-declares the symbol only): mbedtls key
// schedule on ESP32, a software round-key schedule on host.
#ifdef ARDUINO
struct DwsAes128
{
    mbedtls_aes_context mbed; ///< mbedtls context (HW-accelerated on ESP32), key schedule loaded.
};
#else
struct DwsAes128
{
    uint32_t rk[44]; ///< AES-128 expanded round-key schedule (11 round keys x 4 words).
};
#endif

// ===========================================================================
// AES-128 single-block primitive
// ===========================================================================

#ifdef ARDUINO

void dws_aes128_init(DwsAes128 *ctx, const uint8_t key[16])
{
    mbedtls_aes_init(&ctx->mbed);
    mbedtls_aes_setkey_enc(&ctx->mbed, key, 128);
}

void dws_aes128_encrypt_block(DwsAes128 *ctx, const uint8_t in[16], uint8_t out[16])
{
    mbedtls_aes_crypt_ecb(&ctx->mbed, MBEDTLS_AES_ENCRYPT, in, out);
}

void dws_aes128_wipe(DwsAes128 *ctx)
{
    mbedtls_aes_free(&ctx->mbed);
}

#else // Native software AES-128

void dws_aes128_init(DwsAes128 *ctx, const uint8_t key[16])
{
    dws_aes_key_expand(key, 4, ctx->rk);
}

void dws_aes128_encrypt_block(DwsAes128 *ctx, const uint8_t in[16], uint8_t out[16])
{
    dws_aes_encrypt_block(ctx->rk, 10, in, out);
}

void dws_aes128_wipe(DwsAes128 *ctx)
{
    dws_crypto_wipe(ctx, sizeof(DwsAes128));
}

#endif // ARDUINO

// ===========================================================================
// AEAD_AES_128_GCM (NIST SP 800-38D) - software GHASH/GCTR on all targets. All working memory lives in an
// Aes128GcmWork laid over the shared crypto scratch (crypto_work) and is wiped after each op.
// ===========================================================================

struct Aes128GcmWork
{
    DwsAes128 aes;   ///< AES-128 key schedule.
    uint8_t h[16];   ///< GHASH subkey H = E(K, 0^128).
    GhashKey ghk;    ///< 4-bit GHASH table built from H.
    uint8_t ks[16];  ///< GCTR keystream block.
    uint8_t acc[16]; ///< GHASH accumulator.
    uint8_t lb[16];  ///< length block (aad_len || cipher_len, in bits).
    uint8_t ej0[16]; ///< E(K, J0), the tag mask.
    uint8_t j0[16];  ///< pre-counter block J0 = nonce || 0^31 || 1.
    uint8_t ctr[16]; ///< running GCTR counter.
    uint8_t tag[16]; ///< computed tag (open: compared; seal writes the caller's buffer directly).
};
static_assert(sizeof(Aes128GcmWork) <= DWS_CRYPTO_WORK_SIZE, "Aes128GcmWork must fit the shared crypto scratch");

namespace
{
inline void xor16(uint8_t *dst, const uint8_t *src)
{
    for (int i = 0; i < 16; i++)
        dst[i] ^= src[i];
}

// GHASH (acc *= H, and fold buffers into acc) is the shared 4-bit-table primitive in crypto/ghash.h:
// ghash_key_init(&w->ghk, w->h) once, then ghash_update / ghash_mul on w->ghk.

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
    // A single-byte carry (i=15 wrapping into i=14, etc.) only needs ctr[15] to roll 0xff -> 0x00,
    // i.e. ~256 GCTR blocks (~4 KiB of plaintext) through the public seal()/open() API - well within
    // reach of a host test, so that carry-continue arm is exercised below and is NOT excluded.
    // What genuinely cannot be reached is the loop running all 4 iterations to exhaustion with no
    // break at all, i.e. every one of the 4 bytes carrying in the SAME inc32() call, which only
    // happens when the full 32-bit counter was 0xffffffff before this call. ctr always starts at
    // inc32(J0) with J0[12..15] fixed to 0,0,0,1 (96-bit-nonce J0, NIST SP 800-38D) - not
    // caller-controlled - so reaching that requires one seal()/open() call over ~2^32 contiguous
    // GCTR blocks (~64 GiB of plaintext in one call): infeasible in a host test.
    for (int i = 15; i >= 12; // GCOVR_EXCL_BR_LINE  loop-exhausted-with-no-break needs the ~64GiB full wrap; see above
         i--)
        if (++ctr[i])
            break;
}

// GCTR (NIST SP 800-38D sec 6.5): out = in XOR AES-CTR keystream from w->ctr, advanced in place. Uses
// w->ks. @p in / @p out may alias.
void gctr(Aes128GcmWork *w, const uint8_t *in, size_t len, uint8_t *out)
{
    size_t off = 0;
    while (off < len)
    {
        dws_aes128_encrypt_block(&w->aes, w->ctr, w->ks);
        inc32(w->ctr);
        size_t take = len - off;
        if (take > 16)
            take = 16;
        for (size_t i = 0; i < take; i++)
            out[off + i] = in[off + i] ^ w->ks[i];
        off += take;
    }
}

// Compute H, the GHASH table, J0, GHASH(aad || cipher) and the 16-byte tag for a 96-bit-nonce GCM
// operation. @p cipher is the ciphertext to authenticate (== output for seal, == input for open). Uses
// w->h/ghk/j0/acc/lb/ej0; writes @p tag_out.
void gcm_core(Aes128GcmWork *w, const uint8_t nonce[12], const uint8_t *aad, size_t aad_len, const uint8_t *cipher,
              size_t cipher_len, uint8_t tag_out[16])
{
    memset(w->h, 0, 16);
    dws_aes128_encrypt_block(&w->aes, w->h, w->h); // H = E(K, 0^128)
    ghash_key_init(&w->ghk, w->h);

    // 96-bit nonce: J0 = nonce || 0^31 || 1.
    memcpy(w->j0, nonce, 12);
    w->j0[12] = 0;
    w->j0[13] = 0;
    w->j0[14] = 0;
    w->j0[15] = 1;

    memset(w->acc, 0, 16);
    ghash_update(&w->ghk, w->acc, aad, aad_len);
    ghash_update(&w->ghk, w->acc, cipher, cipher_len);
    put_be64(w->lb, (uint64_t)aad_len * 8);
    put_be64(w->lb + 8, (uint64_t)cipher_len * 8);
    xor16(w->acc, w->lb);
    ghash_mul(&w->ghk, w->acc);

    dws_aes128_encrypt_block(&w->aes, w->j0, w->ej0);
    for (int i = 0; i < 16; i++)
        tag_out[i] = w->acc[i] ^ w->ej0[i];
}

// Free the AES key schedule (mbedtls on ARDUINO) and wipe the whole working set from the shared scratch.
inline void work_wipe(Aes128GcmWork *w)
{
    dws_aes128_wipe(&w->aes);
    dws_crypto_wipe(crypto_work, sizeof(Aes128GcmWork));
}
} // namespace

void dws_aes128gcm_seal(const uint8_t key[16], const uint8_t nonce[12], const uint8_t *aad, size_t aad_len,
                        const uint8_t *pt, size_t pt_len, uint8_t *out)
{
    Aes128GcmWork *w = reinterpret_cast<Aes128GcmWork *>(crypto_work);
    dws_aes128_init(&w->aes, key);

    // Encrypt first (counter starts at inc32(J0)), then GHASH the resulting ciphertext.
    memcpy(w->j0, nonce, 12);
    w->j0[12] = 0;
    w->j0[13] = 0;
    w->j0[14] = 0;
    w->j0[15] = 1;
    memcpy(w->ctr, w->j0, 16);
    inc32(w->ctr);
    gctr(w, pt, pt_len, out);

    gcm_core(w, nonce, aad, aad_len, out, pt_len, out + pt_len); // tag appended after the ciphertext
    work_wipe(w);
}

bool dws_aes128gcm_open(const uint8_t key[16], const uint8_t nonce[12], const uint8_t *aad, size_t aad_len,
                        const uint8_t *ct, size_t ct_len, uint8_t *out)
{
    if (ct_len < DWS_AES128GCM_TAG_LEN)
        return false;
    size_t pt_len = ct_len - DWS_AES128GCM_TAG_LEN;

    Aes128GcmWork *w = reinterpret_cast<Aes128GcmWork *>(crypto_work);
    dws_aes128_init(&w->aes, key);

    // Authenticate over the received ciphertext before producing any plaintext.
    gcm_core(w, nonce, aad, aad_len, ct, pt_len, w->tag);
    if (!dws_ct_eq(w->tag, ct + pt_len, DWS_AES128GCM_TAG_LEN))
    {
        work_wipe(w);
        return false;
    }

    memcpy(w->ctr, w->j0, 16);
    inc32(w->ctr);
    gctr(w, ct, pt_len, out);

    work_wipe(w);
    return true;
}

void dws_aes128gcm_seal_tag(const uint8_t key[16], const uint8_t nonce[12], const uint8_t *aad, size_t aad_len,
                            const uint8_t *pt, size_t pt_len, uint8_t *ct_out, uint8_t tag_out[16])
{
    Aes128GcmWork *w = reinterpret_cast<Aes128GcmWork *>(crypto_work);
    dws_aes128_init(&w->aes, key);

    // Encrypt (CTR from inc32(J0)), then GHASH the ciphertext - identical to dws_aes128gcm_seal, except the
    // tag lands in a separate buffer instead of being appended (SMB 3.x carries it in the TRANSFORM_HEADER).
    memcpy(w->j0, nonce, 12);
    w->j0[12] = 0;
    w->j0[13] = 0;
    w->j0[14] = 0;
    w->j0[15] = 1;
    memcpy(w->ctr, w->j0, 16);
    inc32(w->ctr);
    gctr(w, pt, pt_len, ct_out);

    gcm_core(w, nonce, aad, aad_len, ct_out, pt_len, tag_out);
    work_wipe(w);
}

bool dws_aes128gcm_open_tag(const uint8_t key[16], const uint8_t nonce[12], const uint8_t *aad, size_t aad_len,
                            const uint8_t *ct, size_t ct_len, const uint8_t tag[16], uint8_t *out)
{
    Aes128GcmWork *w = reinterpret_cast<Aes128GcmWork *>(crypto_work);
    dws_aes128_init(&w->aes, key);

    // Authenticate over the received ciphertext before producing any plaintext.
    gcm_core(w, nonce, aad, aad_len, ct, ct_len, w->tag);
    if (!dws_ct_eq(w->tag, tag, DWS_AES128GCM_TAG_LEN))
    {
        work_wipe(w);
        return false;
    }

    memcpy(w->ctr, w->j0, 16);
    inc32(w->ctr);
    gctr(w, ct, ct_len, out);

    work_wipe(w);
    return true;
}

#endif // DWS_ENABLE_HTTP3 || DWS_ENABLE_DTLS
