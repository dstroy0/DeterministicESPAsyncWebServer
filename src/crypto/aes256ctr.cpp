// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file aes256ctr.cpp
 * @brief AES-256-CTR implementation (stateless, see aes256ctr.h).
 *
 * The ephemeral key schedule and keystream block are borrowed from the shared crypto scratch
 * (crypto_work) - never a stack or BSS local - and wiped on the way out, so expanded key material is
 * funneled through the one hardened, single-op region rather than scattered across the address space.
 *
 * Arduino path: mbedtls_aes_crypt_ctr(), which the ESP32 mbedtls port routes to the hardware AES
 * accelerator. Native path: a compact software AES-256 (256-byte forward S-box + GF(2^8) MixColumns),
 * for host-side unit tests only.
 */

#include "crypto/aes256ctr.h"
#include "crypto/crypto_opt.h"
#include "crypto/crypto_scratch.h" // crypto_work (ephemeral schedule/keystream) + dws_crypto_wipe
#include <string.h>
#ifdef ARDUINO
#include <mbedtls/aes.h>
#else
#include "crypto/aes_block.h" // native software AES S-box/blocks (ARDUINO uses the mbedtls block above)
#endif
DWS_CRYPTO_HOT

// ============================================================================
// ARDUINO - hardware-accelerated path via mbedtls
// ============================================================================

#ifdef ARDUINO

static_assert(sizeof(mbedtls_aes_context) + 16 <= DWS_CRYPTO_WORK_SIZE,
              "AES-256-CTR schedule + keystream must fit the shared crypto scratch (crypto_work)");

void dws_aes256ctr_crypt(const uint8_t key[DWS_AES256CTR_KEY_LEN], uint8_t counter[DWS_AES256CTR_CTR_LEN],
                         const uint8_t *in, uint8_t *out, size_t len)
{
    // Schedule + keystream block live in the shared crypto scratch (crypto_scratch.h), not a local.
    mbedtls_aes_context *aes = reinterpret_cast<mbedtls_aes_context *>(crypto_work);
    uint8_t *ks = crypto_work + sizeof(mbedtls_aes_context);
    mbedtls_aes_init(aes);
    mbedtls_aes_setkey_enc(aes, key, 256);
    size_t nc_off = 0; // block-aligned callers (SSH) leave this 0; the counter alone carries the stream state
    mbedtls_aes_crypt_ctr(aes, len, &nc_off, counter, ks, in, out);
    mbedtls_aes_free(aes);
    dws_crypto_wipe(crypto_work, sizeof(mbedtls_aes_context) + 16);
}

// ============================================================================
// NATIVE - software AES-256 (for host-side unit tests only)
// ============================================================================

#else

static_assert(60 * sizeof(uint32_t) + 16 <= DWS_CRYPTO_WORK_SIZE,
              "AES-256-CTR schedule + keystream must fit the shared crypto scratch (crypto_work)");

void dws_aes256ctr_crypt(const uint8_t key[DWS_AES256CTR_KEY_LEN], uint8_t counter[DWS_AES256CTR_CTR_LEN],
                         const uint8_t *in, uint8_t *out, size_t len)
{
    // Round-key schedule (60 words) and the keystream block live in the shared crypto scratch, not a local.
    uint32_t *rk = reinterpret_cast<uint32_t *>(crypto_work);
    uint8_t *ks = crypto_work + 60 * sizeof(uint32_t);
    dws_aes_key_expand(key, 8, rk);
    uint8_t pos = 0;
    for (size_t i = 0; i < len; i++)
    {
        if (pos == 0)
        {
            dws_aes_encrypt_block(rk, 14, counter, ks); // keystream = AES(counter)
            for (int j = 15; j >= 0; j--)               // then advance the big-endian counter by one block
                if (++counter[j])
                    break;
        }
        out[i] = in[i] ^ ks[pos];
        pos = (uint8_t)((pos + 1u) & 0x0fu);
    }
    dws_crypto_wipe(crypto_work, 60 * sizeof(uint32_t) + 16);
}

#endif // ARDUINO

// ---------------------------------------------------------------------------
// Length peek (used by the SSH recv path; mirrors dws_chachapoly_get_length)
// ---------------------------------------------------------------------------

uint32_t dws_aes256ctr_get_length(const uint8_t key[DWS_AES256CTR_KEY_LEN],
                                  const uint8_t counter[DWS_AES256CTR_CTR_LEN], const uint8_t enc4[4])
{
    // Produce the keystream block for @p counter (AES-ECB) in the shared crypto scratch, then XOR the first
    // 4 bytes to recover the length. @p counter is not advanced and no cipher state touches the stack.
#ifdef ARDUINO
    mbedtls_aes_context *aes = reinterpret_cast<mbedtls_aes_context *>(crypto_work);
    uint8_t *ks = crypto_work + sizeof(mbedtls_aes_context);
    mbedtls_aes_init(aes);
    mbedtls_aes_setkey_enc(aes, key, 256);
    mbedtls_aes_crypt_ecb(aes, MBEDTLS_AES_ENCRYPT, counter, ks);
    mbedtls_aes_free(aes);
    const size_t wipe_len = sizeof(mbedtls_aes_context) + 16;
#else
    uint32_t *rk = reinterpret_cast<uint32_t *>(crypto_work);
    uint8_t *ks = crypto_work + 60 * sizeof(uint32_t);
    dws_aes_key_expand(key, 8, rk);
    dws_aes_encrypt_block(rk, 14, counter, ks);
    const size_t wipe_len = 60 * sizeof(uint32_t) + 16;
#endif
    uint32_t len = ((uint32_t)(enc4[0] ^ ks[0]) << 24) | ((uint32_t)(enc4[1] ^ ks[1]) << 16) |
                   ((uint32_t)(enc4[2] ^ ks[2]) << 8) | (uint32_t)(enc4[3] ^ ks[3]);
    dws_crypto_wipe(crypto_work, wipe_len);
    return len;
}
