// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file aesgcm.cpp
 * @brief AES-256-GCM AEAD - stateless implementation (see aesgcm.h).
 *
 * All working memory (AES key schedule, GHASH table, keystream, accumulator, tag mask, counters) lives in
 * the shared crypto scratch (crypto_work), laid out as a GcmWork on the software path or an
 * mbedtls_gcm_context on the hardware-GCM path, and the whole region is wiped on the way out. No cipher
 * state ever touches the stack or BSS.
 *
 * Arduino: the AES-256 block is mbedtls_aes_crypt_ecb() (ESP32 HW accelerator); dies with a hardware GCM
 * mode do the whole AEAD in one mbedtls_gcm call. Native: a compact software AES-256 (shared forward S-box
 * + GF(2^8) xtime) + software GHASH. The GF(2^128) reduction mirrors aes128gcm.cpp.
 */

#include "crypto/aesgcm.h"
#include "crypto/crypto_opt.h"
#include "crypto/crypto_scratch.h" // crypto_work + dws_crypto_wipe (the shared crypto scratch)
#include "crypto/ghash.h"
#include <string.h>
#ifdef ARDUINO
#include "soc/soc_caps.h" // SOC_AES_SUPPORT_GCM - does this die's AES peripheral have a hardware GCM mode?
#endif

// Chips whose AES peripheral has a hardware GCM mode (e.g. ESP32-P4, SOC_AES_SUPPORT_GCM) do the whole
// AEAD in one hardware call - far faster than driving the block cipher one 16-byte block at a time and
// folding GHASH in software (measured 3x-33x on the P4). The ESP32-S3 has no GCM mode, so it keeps the
// manual HW-AES + software-GHASH path.
#if defined(ARDUINO) && defined(SOC_AES_SUPPORT_GCM) && SOC_AES_SUPPORT_GCM
#define DWS_AESGCM_HW_GCM 1
#else
#define DWS_AESGCM_HW_GCM 0
#endif

#if DWS_AESGCM_HW_GCM
#include <mbedtls/gcm.h> // hardware GCM peripheral: AES-CTR + GHASH in one call
#elif defined(ARDUINO)
#include <mbedtls/aes.h> // HW AES block + software GHASH
#else
#include "crypto/aes_block.h" // native software AES-256
#endif
DWS_CRYPTO_HOT

// Advance the RFC 5647 invocation counter: the low 8 bytes of the 12-byte nonce, big-endian; the 4-byte
// fixed field never changes. Shared by both paths and exposed publicly for the SSH packet layer.
void dws_aesgcm_iv_increment(uint8_t iv[DWS_AESGCM_IV_LEN])
{
    for (int j = DWS_AESGCM_IV_LEN - 1; j >= 4; j--)
        if (++iv[j])
            break;
}

#if DWS_AESGCM_HW_GCM
// ===========================================================================
// Hardware GCM path (mbedtls_gcm -> the ESP32 AES peripheral's GCM mode).
// ===========================================================================

static_assert(sizeof(mbedtls_gcm_context) <= DWS_CRYPTO_WORK_SIZE, "mbedtls_gcm_context must fit crypto_work");

void dws_aesgcm_seal_tag(const uint8_t key[DWS_AESGCM_KEY_LEN], const uint8_t nonce[DWS_AESGCM_IV_LEN],
                         const uint8_t *aad, size_t aad_len, const uint8_t *pt, size_t pt_len, uint8_t *ct_out,
                         uint8_t tag_out[DWS_AESGCM_TAG_LEN])
{
    // GCM context (AES-256 key schedule) lives in the shared crypto scratch, never on the stack.
    mbedtls_gcm_context *g = reinterpret_cast<mbedtls_gcm_context *>(crypto_work);
    mbedtls_gcm_init(g);
    mbedtls_gcm_setkey(g, MBEDTLS_CIPHER_ID_AES, key, 256);
    mbedtls_gcm_crypt_and_tag(g, MBEDTLS_GCM_ENCRYPT, pt_len, nonce, DWS_AESGCM_IV_LEN, aad, aad_len, pt, ct_out,
                              DWS_AESGCM_TAG_LEN, tag_out);
    mbedtls_gcm_free(g);
    dws_crypto_wipe(crypto_work, sizeof(mbedtls_gcm_context));
}

bool dws_aesgcm_open_tag(const uint8_t key[DWS_AESGCM_KEY_LEN], const uint8_t nonce[DWS_AESGCM_IV_LEN],
                         const uint8_t *aad, size_t aad_len, const uint8_t *ct, size_t ct_len,
                         const uint8_t tag[DWS_AESGCM_TAG_LEN], uint8_t *out)
{
    mbedtls_gcm_context *g = reinterpret_cast<mbedtls_gcm_context *>(crypto_work);
    mbedtls_gcm_init(g);
    mbedtls_gcm_setkey(g, MBEDTLS_CIPHER_ID_AES, key, 256);
    int rc =
        mbedtls_gcm_auth_decrypt(g, ct_len, nonce, DWS_AESGCM_IV_LEN, aad, aad_len, tag, DWS_AESGCM_TAG_LEN, ct, out);
    mbedtls_gcm_free(g);
    dws_crypto_wipe(crypto_work, sizeof(mbedtls_gcm_context));
    return rc == 0;
}

#else // !DWS_AESGCM_HW_GCM - software GHASH/GCTR (all state in a GcmWork over crypto_work)

// ===========================================================================
// GcmWork: the entire AES-256-GCM working set, laid over the shared crypto scratch. No cipher state on
// the stack; the whole struct is wiped after each operation.
// ===========================================================================
struct GcmWork
{
#ifdef ARDUINO
    mbedtls_aes_context aes; ///< AES-256 key schedule (HW-accelerated block on ESP32).
#else
    uint32_t rk[60]; ///< AES-256 expanded round-key schedule (software).
#endif
    uint8_t h[16];   ///< GHASH subkey H = E(K, 0^128).
    GhashKey ghk;    ///< 4-bit GHASH table built from H.
    uint8_t ks[16];  ///< GCTR keystream block (also the zero input used to derive H).
    uint8_t acc[16]; ///< GHASH accumulator.
    uint8_t lb[16];  ///< length block (aad_len || cipher_len, in bits).
    uint8_t ej0[16]; ///< E(K, J0), the tag mask.
    uint8_t j0[16];  ///< pre-counter block J0 = nonce || 0^31 || 1.
    uint8_t ctr[16]; ///< running GCTR counter.
};
static_assert(sizeof(GcmWork) <= DWS_CRYPTO_WORK_SIZE, "GcmWork must fit the shared crypto scratch (crypto_work)");

// ---------------------------------------------------------------------------
// AES-256 single-block primitive (operates on the schedule inside GcmWork)
// ---------------------------------------------------------------------------
namespace
{
inline void aes256_ecb(GcmWork *w, const uint8_t in[16], uint8_t out[16])
{
#ifdef ARDUINO
    mbedtls_aes_crypt_ecb(&w->aes, MBEDTLS_AES_ENCRYPT, in, out);
#else
    dws_aes_encrypt_block(w->rk, 14, in, out);
#endif
}
inline void aes256_load_key(GcmWork *w, const uint8_t key[32])
{
#ifdef ARDUINO
    mbedtls_aes_init(&w->aes);
    mbedtls_aes_setkey_enc(&w->aes, key, 256);
#else
    dws_aes_key_expand(key, 8, w->rk);
#endif
}
inline void aes256_free_key(GcmWork *w)
{
#ifdef ARDUINO
    mbedtls_aes_free(&w->aes);
#else
    (void)w; // software schedule lives in-place in GcmWork; nothing external to release
#endif
}

inline void xor16(uint8_t *dst, const uint8_t *src)
{
    for (int i = 0; i < 16; i++)
        dst[i] ^= src[i];
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
    // A single-byte carry (ctr[15] 0xff -> 0x00 into ctr[14]) is cheap to reach and exercised by
    // test_aesgcm_gctr_counter_byte_carry; the full 2^32 wrap (~64 GiB in one call) is the only branch a
    // host test cannot reach.
    for (int i = 15; i >= 12; // GCOVR_EXCL_BR_LINE  full 2^32 counter wrap (~64 GiB/call) unreachable
         i--)
        if (++ctr[i])
            break;
}

// Build H + the GHASH table and set J0 = nonce || 0^31 || 1 for a freshly key-loaded @p w.
void gcm_prepare(GcmWork *w, const uint8_t nonce[12])
{
    memset(w->ks, 0, 16);       // zero input for H (reuses the keystream slot; overwritten by gctr below)
    aes256_ecb(w, w->ks, w->h); // H = E(K, 0^128)
    ghash_key_init(&w->ghk, w->h);
    memcpy(w->j0, nonce, 12);
    w->j0[12] = 0;
    w->j0[13] = 0;
    w->j0[14] = 0;
    w->j0[15] = 1;
}

// GCTR (NIST SP 800-38D sec 6.5): out = in XOR AES-CTR keystream from @p w->ctr, advanced in place.
void gctr(GcmWork *w, const uint8_t *in, size_t len, uint8_t *out)
{
    size_t off = 0;
    while (off < len)
    {
        aes256_ecb(w, w->ctr, w->ks);
        inc32(w->ctr);
        size_t take = len - off;
        if (take > 16)
            take = 16;
        for (size_t i = 0; i < take; i++)
            out[off + i] = in[off + i] ^ w->ks[i];
        off += take;
    }
}

// GHASH over aad || cipher, fold in the lengths, and produce the 16-byte tag (acc XOR E(K, J0)).
void gcm_tag(GcmWork *w, const uint8_t *aad, size_t aad_len, const uint8_t *cipher, size_t cipher_len,
             uint8_t tag_out[16])
{
    memset(w->acc, 0, 16);
    ghash_update(&w->ghk, w->acc, aad, aad_len);
    ghash_update(&w->ghk, w->acc, cipher, cipher_len);
    put_be64(w->lb, (uint64_t)aad_len * 8);
    put_be64(w->lb + 8, (uint64_t)cipher_len * 8);
    xor16(w->acc, w->lb);
    ghash_mul(&w->ghk, w->acc);
    aes256_ecb(w, w->j0, w->ej0);
    for (int i = 0; i < 16; i++)
        tag_out[i] = w->acc[i] ^ w->ej0[i];
}
} // namespace

// ===========================================================================
// Public API (stateless, detached tag)
// ===========================================================================

void dws_aesgcm_seal_tag(const uint8_t key[DWS_AESGCM_KEY_LEN], const uint8_t nonce[DWS_AESGCM_IV_LEN],
                         const uint8_t *aad, size_t aad_len, const uint8_t *pt, size_t pt_len, uint8_t *ct_out,
                         uint8_t tag_out[DWS_AESGCM_TAG_LEN])
{
    GcmWork *w = reinterpret_cast<GcmWork *>(crypto_work);
    aes256_load_key(w, key);
    gcm_prepare(w, nonce);
    // Encrypt with the CTR starting at inc32(J0), then GHASH the resulting ciphertext.
    memcpy(w->ctr, w->j0, 16);
    inc32(w->ctr);
    gctr(w, pt, pt_len, ct_out);
    gcm_tag(w, aad, aad_len, ct_out, pt_len, tag_out);
    aes256_free_key(w);
    dws_crypto_wipe(crypto_work, sizeof(GcmWork));
}

bool dws_aesgcm_open_tag(const uint8_t key[DWS_AESGCM_KEY_LEN], const uint8_t nonce[DWS_AESGCM_IV_LEN],
                         const uint8_t *aad, size_t aad_len, const uint8_t *ct, size_t ct_len,
                         const uint8_t tag[DWS_AESGCM_TAG_LEN], uint8_t *out)
{
    GcmWork *w = reinterpret_cast<GcmWork *>(crypto_work);
    aes256_load_key(w, key);
    gcm_prepare(w, nonce);
    // Authenticate over the received ciphertext BEFORE producing any plaintext (tag reuses the ej0 slot).
    gcm_tag(w, aad, aad_len, ct, ct_len, w->ej0);
    if (!dws_ct_eq(w->ej0, tag, DWS_AESGCM_TAG_LEN))
    {
        aes256_free_key(w);
        dws_crypto_wipe(crypto_work, sizeof(GcmWork));
        return false; // tag mismatch: nothing written
    }
    memcpy(w->ctr, w->j0, 16);
    inc32(w->ctr);
    gctr(w, ct, ct_len, out);
    aes256_free_key(w);
    dws_crypto_wipe(crypto_work, sizeof(GcmWork));
    return true;
}

#endif // DWS_AESGCM_HW_GCM
