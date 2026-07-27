// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file aes128gcm.h
 * @brief AES-128 block cipher + AEAD_AES_128_GCM (RFC 5116 / NIST SP 800-38D).
 *
 * The generic 128-bit AES primitives: encrypt one 16-byte block under a 128-bit key (ECB - used for
 * GCM's counter mode and for keystream sampling), and the one-shot AEAD_AES_128_GCM seal/open (96-bit
 * nonce, 128-bit tag). Consumed by QUIC Initial packet protection (RFC 9001 sec 5.3/5.4), the DTLS 1.3
 * record layer, and SMB 3.x transport encryption - a single home for the primitive, not per-protocol copies.
 *
 * On Arduino (ESP32) the AES block is mbedtls, routed to the hardware AES accelerator; on native host
 * builds a compact software AES-128 is used so the whole AEAD is unit-testable off-target. GHASH and the
 * counter loop are the same software on both. Pure, zero heap, host-tested against the NIST GCM vectors
 * and RFC 9001 Appendix A.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_CRYPTO_AES128GCM_H
#define DETERMINISTICESPASYNCWEBSERVER_CRYPTO_AES128GCM_H

#include "ServerConfig.h"

// Shared by the HTTP/3 (QUIC) packet protection and the DTLS 1.3 record layer.
#if (DWS_ENABLE_HTTP3 || DWS_ENABLE_DTLS || DWS_ENABLE_SMB)

#include <stddef.h>
#include <stdint.h>

/** @brief AEAD_AES_128_GCM authentication tag length in bytes. */
#define DWS_AES128GCM_TAG_LEN 16

// ---------------------------------------------------------------------------
// AES-128 single-block primitive (used by GCM and by header protection)
// ---------------------------------------------------------------------------

// Opaque: the definition (mbedtls_aes_context on ESP32, a software key schedule on host) is private to
// aes128gcm.cpp - consumers know only the symbol and hold it via DwsAes128*.
struct DwsAes128;

/** @brief Load a 128-bit key and expand the encryption key schedule. */
void dws_aes128_init(DwsAes128 *ctx, const uint8_t key[16]);

/** @brief Encrypt one 16-byte block (ECB). @p in and @p out may alias. */
void dws_aes128_encrypt_block(DwsAes128 *ctx, const uint8_t in[16], uint8_t out[16]);

/** @brief Wipe the key schedule (and release mbedtls state on Arduino). */
void dws_aes128_wipe(DwsAes128 *ctx);

// ---------------------------------------------------------------------------
// AEAD_AES_128_GCM (96-bit nonce, 128-bit tag)
// ---------------------------------------------------------------------------

/**
 * @brief Seal: AEAD_AES_128_GCM encrypt-and-authenticate.
 *
 * Writes @p pt_len ciphertext bytes followed by the 16-byte tag into @p out, so @p out must hold
 * at least @p pt_len + DWS_AES128GCM_TAG_LEN bytes. @p out may alias @p pt (in-place encryption).
 *
 * @param key      16-byte key.
 * @param nonce    12-byte nonce.
 * @param aad      Additional authenticated data (may be NULL when @p aad_len is 0).
 * @param aad_len  AAD length.
 * @param pt       Plaintext.
 * @param pt_len   Plaintext length.
 * @param out      Output: ciphertext || tag (@p pt_len + 16 bytes).
 */
void dws_aes128gcm_seal(const uint8_t key[16], const uint8_t nonce[12], const uint8_t *aad, size_t aad_len,
                        const uint8_t *pt, size_t pt_len, uint8_t *out);

/**
 * @brief Open: AEAD_AES_128_GCM verify-and-decrypt.
 *
 * @p ct is ciphertext followed by the 16-byte tag (so @p ct_len >= DWS_AES128GCM_TAG_LEN). The tag is
 * verified in constant time before any plaintext is exposed; on mismatch nothing is written and the
 * function returns false. @p out receives @p ct_len - 16 plaintext bytes and may alias @p ct.
 *
 * @return true if the tag is valid (plaintext written), false otherwise.
 */
bool dws_aes128gcm_open(const uint8_t key[16], const uint8_t nonce[12], const uint8_t *aad, size_t aad_len,
                        const uint8_t *ct, size_t ct_len, uint8_t *out);

// ---------------------------------------------------------------------------
// Detached-tag variants (tag kept separate from the ciphertext, e.g. SMB 3.x TRANSFORM_HEADER, RFC 5116 §5.1)
// ---------------------------------------------------------------------------

/**
 * @brief Seal with the authentication tag written to a separate buffer (not appended to the ciphertext).
 *
 * @p ct_out receives @p pt_len ciphertext bytes (may alias @p pt); @p tag_out receives the 16-byte tag. Used
 * where the wire format carries the tag apart from the ciphertext, such as the SMB 3.x TRANSFORM_HEADER
 * Signature field.
 */
void dws_aes128gcm_seal_tag(const uint8_t key[16], const uint8_t nonce[12], const uint8_t *aad, size_t aad_len,
                            const uint8_t *pt, size_t pt_len, uint8_t *ct_out, uint8_t tag_out[16]);

/**
 * @brief Open with a detached authentication tag. The tag is verified in constant time over @p aad || @p ct
 *        before any plaintext is exposed; on mismatch nothing is written and false is returned. @p out
 *        receives @p ct_len plaintext bytes and may alias @p ct.
 * @return true iff the tag is valid.
 */
bool dws_aes128gcm_open_tag(const uint8_t key[16], const uint8_t nonce[12], const uint8_t *aad, size_t aad_len,
                            const uint8_t *ct, size_t ct_len, const uint8_t tag[16], uint8_t *out);

#endif // DWS_ENABLE_HTTP3 || DWS_ENABLE_DTLS
#endif // DETERMINISTICESPASYNCWEBSERVER_CRYPTO_AES128GCM_H
