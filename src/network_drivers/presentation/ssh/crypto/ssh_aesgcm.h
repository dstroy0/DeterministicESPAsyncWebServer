// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_aesgcm.h
 * @brief AES-256-GCM AEAD for SSH (aes256-gcm@openssh.com, RFC 5647).
 *
 * The OpenSSH AES-GCM cipher is an AEAD: the 4-byte SSH packet_length is sent in the clear and
 * authenticated as additional data (AAD), the rest of the binary packet (padding_length || payload
 * || padding) is encrypted, and a 16-byte GCM tag follows. No separate MAC is negotiated - the AEAD
 * tag *is* the integrity check (RFC 5647 sec 7.3).
 *
 * NONCE / INVOCATION COUNTER (RFC 5647 sec 7.1)
 * The 12-byte GCM nonce is `fixed_field(4) || invocation_counter(8)`. The initial nonce is the first
 * 12 bytes of the IV derived from the key exchange (RFC 4253 sec 7.2, labels 'A'/'B'). After every
 * packet the 8-byte invocation_counter is incremented as a big-endian integer; the 4-byte fixed field
 * never changes. The counter therefore lives in the context and advances once per sealed/opened packet
 * - this is what makes the cipher stateful and requires whole-packet atomicity in the packet layer.
 *
 * PLATFORM SELECTION (mirrors ssh_aes256ctr / dws_quic_aead)
 * On Arduino (ESP32) the AES-256 block is mbedtls, routed to the hardware AES accelerator. On native
 * host builds a compact software AES-256 is used so the whole AEAD is unit-testable off-target. GHASH
 * and the counter loop are the same software on both targets.
 *
 * Pure, zero heap; host-tested against the NIST/McGrew AES-256-GCM test vectors and round-tripped
 * through the SSH packet layer.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SSH_AESGCM_H
#define DETERMINISTICESPASYNCWEBSERVER_SSH_AESGCM_H

#include "shared_primitives/ghash.h"
#include <stddef.h>
#include <stdint.h>

/** @brief AES-256-GCM key length (bytes). */
static constexpr size_t SSH_AESGCM_KEY_LEN = 32;
/** @brief GCM nonce length (bytes) = fixed_field(4) || invocation_counter(8). */
static constexpr size_t SSH_AESGCM_IV_LEN = 12;
/** @brief GCM authentication tag length (bytes). */
static constexpr size_t SSH_AESGCM_TAG_LEN = 16;

#ifdef ARDUINO
#include <mbedtls/aes.h>
/** @brief AES-256-GCM context for one SSH direction (HW AES on ESP32). */
struct SshAesGcmCtx
{
    mbedtls_aes_context mbed;      ///< mbedtls context (HW-accelerated on ESP32), encrypt key schedule.
    uint8_t h[16];                 ///< GHASH subkey H = E(K, 0^128).
    GhashKey ghk;                  ///< 4-bit GHASH table built from H (once at init).
    uint8_t iv[SSH_AESGCM_IV_LEN]; ///< current nonce; low 8 bytes (invocation counter) ++ per packet.
    bool ready;                    ///< true once a key/IV is installed.
};
#else
/** @brief AES-256-GCM context for one SSH direction (software AES on host). */
struct SshAesGcmCtx
{
    uint32_t rk[60];               ///< AES-256 expanded round-key schedule (60 words, 240 bytes).
    uint8_t h[16];                 ///< GHASH subkey H = E(K, 0^128).
    GhashKey ghk;                  ///< 4-bit GHASH table built from H (once at init).
    uint8_t iv[SSH_AESGCM_IV_LEN]; ///< current nonce; low 8 bytes (invocation counter) ++ per packet.
    bool ready;                    ///< true once a key/IV is installed.
};
#endif

/**
 * @brief Initialize an AES-256-GCM context: expand the key, precompute H, latch the initial nonce.
 * @param ctx  Uninitialized context.
 * @param key  32-byte AES-256 key (KEX label 'C'/'D').
 * @param iv   12-byte initial nonce (first 12 bytes of the KEX 'A'/'B' IV).
 */
void ssh_aesgcm_init(SshAesGcmCtx *ctx, const uint8_t key[SSH_AESGCM_KEY_LEN], const uint8_t iv[SSH_AESGCM_IV_LEN]);

/**
 * @brief Seal one packet: AES-256-GCM encrypt @p pt (@p pt_len bytes) and authenticate it together
 *        with @p aad. Writes @p pt_len ciphertext bytes then the 16-byte tag into @p out (so @p out
 *        must hold @p pt_len + ::SSH_AESGCM_TAG_LEN bytes). @p out may alias @p pt (in place).
 *        The context's invocation counter is advanced by one afterwards.
 */
void ssh_aesgcm_seal(SshAesGcmCtx *ctx, const uint8_t *aad, size_t aad_len, const uint8_t *pt, size_t pt_len,
                     uint8_t *out);

/**
 * @brief Open one packet: verify the 16-byte @p tag over @p aad || @p ct in constant time, and only
 *        on success decrypt @p ct (@p ct_len bytes) into @p out (@p out may alias @p ct). The
 *        invocation counter is advanced by one on success. @return true iff the tag is valid.
 */
bool ssh_aesgcm_open(SshAesGcmCtx *ctx, const uint8_t *aad, size_t aad_len, const uint8_t *ct, size_t ct_len,
                     const uint8_t tag[SSH_AESGCM_TAG_LEN], uint8_t *out);

/** @brief Zero the key schedule, H, and nonce (volatile wipe). Call on disconnect. */
void ssh_aesgcm_wipe(SshAesGcmCtx *ctx);

#endif // DETERMINISTICESPASYNCWEBSERVER_SSH_AESGCM_H
