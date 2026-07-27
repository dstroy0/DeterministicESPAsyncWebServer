// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file aesgcm.h
 * @brief AES-256-GCM AEAD (RFC 5116) - stateless, detached-tag API.
 *
 * There is no context object and no cipher state on the stack: the caller keeps only the raw 32-byte key
 * and the 12-byte nonce (as plain bytes), and every call rebuilds the AES key schedule + GHASH table in
 * the shared crypto scratch (crypto_work) and wipes the whole region on the way out. This mirrors the
 * chacha20-poly1305 and AES-256-CTR APIs and keeps all expanded key material funneled through the one
 * hardened, wiped region - nothing lingers in BSS or on the stack.
 *
 * Used by SSH aes256-gcm@openssh.com (RFC 5647: the caller advances the invocation counter with
 * dws_aesgcm_iv_increment after every packet) and SMB 3.x transport encryption (fresh nonce per message).
 *
 * On Arduino (ESP32) the AES-256 block is mbedtls, routed to the hardware AES accelerator (and on dies
 * with a hardware GCM mode the whole AEAD is one mbedtls_gcm call); native host builds use a compact
 * software AES-256 + software GHASH so the AEAD is unit-testable off-target. Host-tested byte-exact
 * against the NIST/McGrew AES-256-GCM vectors.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_CRYPTO_AESGCM_H
#define DETERMINISTICESPASYNCWEBSERVER_CRYPTO_AESGCM_H

#include <stddef.h>
#include <stdint.h>

/** @brief AES-256-GCM key length (bytes). */
static constexpr size_t DWS_AESGCM_KEY_LEN = 32;
/** @brief GCM nonce length (bytes) = fixed_field(4) || invocation_counter(8). */
static constexpr size_t DWS_AESGCM_IV_LEN = 12;
/** @brief GCM authentication tag length (bytes). */
static constexpr size_t DWS_AESGCM_TAG_LEN = 16;

/**
 * @brief Seal one message with AES-256-GCM under @p key and @p nonce.
 *
 * @p ct_out receives @p pt_len ciphertext bytes (may alias @p pt) and @p tag_out the 16-byte tag. All AES
 * key-schedule / GHASH working memory lives in the shared crypto scratch; none of it touches the stack.
 * No state is kept or advanced (the caller owns the nonce).
 */
void dws_aesgcm_seal_tag(const uint8_t key[DWS_AESGCM_KEY_LEN], const uint8_t nonce[DWS_AESGCM_IV_LEN],
                         const uint8_t *aad, size_t aad_len, const uint8_t *pt, size_t pt_len, uint8_t *ct_out,
                         uint8_t tag_out[DWS_AESGCM_TAG_LEN]);

/**
 * @brief Open one AES-256-GCM message: verify @p tag over @p aad || @p ct in constant time, then (only on
 *        success) decrypt @p ct into @p out (may alias @p ct). @return true iff the tag is valid.
 */
bool dws_aesgcm_open_tag(const uint8_t key[DWS_AESGCM_KEY_LEN], const uint8_t nonce[DWS_AESGCM_IV_LEN],
                         const uint8_t *aad, size_t aad_len, const uint8_t *ct, size_t ct_len,
                         const uint8_t tag[DWS_AESGCM_TAG_LEN], uint8_t *out);

/**
 * @brief Advance the RFC 5647 invocation counter: the low 8 bytes of the 12-byte nonce as a big-endian
 *        integer; the 4-byte fixed field never changes. SSH calls this after each sealed/opened packet.
 */
void dws_aesgcm_iv_increment(uint8_t iv[DWS_AESGCM_IV_LEN]);

#endif // DETERMINISTICESPASYNCWEBSERVER_CRYPTO_AESGCM_H
