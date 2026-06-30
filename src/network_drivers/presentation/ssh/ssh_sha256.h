// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_sha256.h
 * @brief SHA-256 (FIPS 180-4) - streaming context and one-shot API.
 *
 * On Arduino (ESP32) targets BOTH the one-shot and the streaming context
 * (init / update / final) delegate to mbedtls, which routes SHA-256 to the
 * hardware accelerator. The streaming path backs the per-packet HMAC
 * (ssh_hmac_sha256, run on every inbound and outbound SSH packet) as well as
 * the KEX exchange-hash, so hardware acceleration matters for bulk throughput,
 * not just the handshake. On native builds the software implementation is used.
 *
 * The context therefore holds an mbedtls_sha256_context on Arduino (pulling in
 * <mbedtls/sha256.h>) and the software hash state on native.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SSH_SHA256_H
#define DETERMINISTICESPASYNCWEBSERVER_SSH_SHA256_H

#include "shared_primitives/shim.h"

/** @brief SHA-256 digest length in bytes. */
#define SSH_SHA256_DIGEST_LEN 32

/** @brief SHA-256 block size in bytes. */
#define SSH_SHA256_BLOCK_LEN 64

/**
 * @brief Streaming SHA-256 context.
 *
 * Holds the running hash state so data can be fed in multiple chunks.
 * Used for the KEX exchange-hash which is assembled from several
 * separately-encoded fields (V_C, V_S, I_C, I_S, K_S, e, f, K).
 */
#ifdef ARDUINO
typedef struct
{
    mbedtls_sha256_context mbed; ///< HW-accelerated SHA-256 state (ESP32 mbedtls).
} SshSha256Ctx;
#else
typedef struct
{
    uint32_t s[8];   ///< Running hash words (H0..H7).
    uint64_t n;      ///< Total bytes processed so far.
    uint8_t buf[64]; ///< Partial block accumulator.
    uint32_t buflen; ///< Bytes valid in buf[].
} SshSha256Ctx;
#endif

/**
 * @brief Initialize a streaming SHA-256 context.
 * @param ctx  Must not be NULL.
 */
void ssh_sha256_init(SshSha256Ctx *ctx);

/**
 * @brief Feed @p len bytes of @p data into the running hash.
 * @param ctx   Initialized context.
 * @param data  Input bytes.
 * @param len   Number of input bytes.
 */
void ssh_sha256_update(SshSha256Ctx *ctx, const uint8_t *data, size_t len);

/**
 * @brief Finalize the hash and write the 32-byte digest.
 *
 * The context is in an undefined state after this call; do not call
 * update() again without first calling init().
 *
 * @param ctx     Initialized context with all data already fed.
 * @param digest  Output buffer, must be SSH_SHA256_DIGEST_LEN bytes.
 */
void ssh_sha256_final(SshSha256Ctx *ctx, uint8_t digest[SSH_SHA256_DIGEST_LEN]);

/**
 * @brief One-shot SHA-256: hash @p len bytes and write the digest.
 *
 * On Arduino uses the mbedtls hardware-accelerated path.
 * On native uses the software streaming implementation.
 *
 * @param data    Input bytes.
 * @param len     Number of input bytes.
 * @param digest  Output buffer, must be SSH_SHA256_DIGEST_LEN bytes.
 */
void ssh_sha256(const uint8_t *data, size_t len, uint8_t digest[SSH_SHA256_DIGEST_LEN]);

#endif // DETERMINISTICESPASYNCWEBSERVER_SSH_SHA256_H
