// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file quic_aead.h
 * @brief AES-128 block cipher and AEAD_AES_128_GCM (RFC 5116 / NIST SP 800-38D).
 *
 * The QUIC Initial packet protection AEAD is AEAD_AES_128_GCM (RFC 9001 sec 5.3), and header
 * protection samples a keystream block from AES-128 in ECB mode (sec 5.4). Both need a single
 * primitive: encrypt one 16-byte block under a 128-bit key. GCM's counter mode and its GHASH
 * authenticator are then layered on top in software.
 *
 * This mirrors the SSH cipher split: on Arduino (ESP32) the AES block is mbedtls, routed to the
 * hardware AES accelerator; on native host builds a compact software AES-128 is used so the whole
 * AEAD is unit-testable off-target. GHASH and the counter loop are the same software on both.
 *
 * Pure, zero heap, host-tested against the NIST GCM test vectors and RFC 9001 Appendix A.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_QUIC_AEAD_H
#define DETERMINISTICESPASYNCWEBSERVER_QUIC_AEAD_H

#include "DetWebServerConfig.h"

#if DETWS_ENABLE_HTTP3

#include <stddef.h>
#include <stdint.h>

/** @brief AEAD_AES_128_GCM authentication tag length in bytes. */
#define QUIC_AEAD_TAG_LEN 16

// ---------------------------------------------------------------------------
// AES-128 single-block primitive (used by GCM and by header protection)
// ---------------------------------------------------------------------------

#ifdef ARDUINO
#include <mbedtls/aes.h>
struct QuicAes128
{
    mbedtls_aes_context mbed; ///< mbedtls context (HW-accelerated on ESP32), key schedule loaded.
};
#else
struct QuicAes128
{
    uint32_t rk[44]; ///< AES-128 expanded round-key schedule (11 round keys x 4 words).
};
#endif

/** @brief Load a 128-bit key and expand the encryption key schedule. */
void quic_aes128_init(QuicAes128 *ctx, const uint8_t key[16]);

/** @brief Encrypt one 16-byte block (ECB). @p in and @p out may alias. */
void quic_aes128_encrypt_block(QuicAes128 *ctx, const uint8_t in[16], uint8_t out[16]);

/** @brief Wipe the key schedule (and release mbedtls state on Arduino). */
void quic_aes128_wipe(QuicAes128 *ctx);

// ---------------------------------------------------------------------------
// AEAD_AES_128_GCM (96-bit nonce, 128-bit tag)
// ---------------------------------------------------------------------------

/**
 * @brief Seal: AEAD_AES_128_GCM encrypt-and-authenticate.
 *
 * Writes @p pt_len ciphertext bytes followed by the 16-byte tag into @p out, so @p out must hold
 * at least @p pt_len + QUIC_AEAD_TAG_LEN bytes. @p out may alias @p pt (in-place encryption).
 *
 * @param key      16-byte key.
 * @param nonce    12-byte nonce.
 * @param aad      Additional authenticated data (may be NULL when @p aad_len is 0).
 * @param aad_len  AAD length.
 * @param pt       Plaintext.
 * @param pt_len   Plaintext length.
 * @param out      Output: ciphertext || tag (@p pt_len + 16 bytes).
 */
void quic_aes128_gcm_seal(const uint8_t key[16], const uint8_t nonce[12], const uint8_t *aad, size_t aad_len,
                          const uint8_t *pt, size_t pt_len, uint8_t *out);

/**
 * @brief Open: AEAD_AES_128_GCM verify-and-decrypt.
 *
 * @p ct is ciphertext followed by the 16-byte tag (so @p ct_len >= QUIC_AEAD_TAG_LEN). The tag is
 * verified in constant time before any plaintext is exposed; on mismatch nothing is written and the
 * function returns false. @p out receives @p ct_len - 16 plaintext bytes and may alias @p ct.
 *
 * @return true if the tag is valid (plaintext written), false otherwise.
 */
bool quic_aes128_gcm_open(const uint8_t key[16], const uint8_t nonce[12], const uint8_t *aad, size_t aad_len,
                          const uint8_t *ct, size_t ct_len, uint8_t *out);

#endif // DETWS_ENABLE_HTTP3
#endif // DETERMINISTICESPASYNCWEBSERVER_QUIC_AEAD_H
