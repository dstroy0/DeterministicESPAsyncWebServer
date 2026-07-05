// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_sha512.h
 * @brief SHA-512 (FIPS 180-4) - streaming context and one-shot API.
 *
 * Needed by Ed25519 (RFC 8032), whose signing and verification hash with SHA-512.
 * On Arduino (ESP32) both the streaming context and the one-shot delegate to mbedtls
 * (hardware-accelerated where available); on native builds the software FIPS-180-4
 * implementation is used. Mirrors the ssh_sha256 dual-path structure.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SSH_SHA512_H
#define DETERMINISTICESPASYNCWEBSERVER_SSH_SHA512_H

#include <stddef.h>
#include <stdint.h>

/** @brief SHA-512 digest length in bytes. */
#define SSH_SHA512_DIGEST_LEN 64

/** @brief SHA-512 block size in bytes. */
#define SSH_SHA512_BLOCK_LEN 128

/** @brief Streaming SHA-512 context. */
#ifdef ARDUINO
#include <mbedtls/sha512.h>
typedef struct
{
    mbedtls_sha512_context mbed; ///< mbedtls SHA-512 state (ESP32).
} SshSha512Ctx;
#else
typedef struct
{
    uint64_t s[8];    ///< Running hash words (H0..H7).
    uint64_t n;       ///< Total bytes processed so far.
    uint8_t buf[128]; ///< Partial block accumulator.
    uint32_t buflen;  ///< Bytes valid in buf[].
} SshSha512Ctx;
#endif

/** @brief Initialize a streaming SHA-512 context (@p ctx must not be NULL). */
void ssh_sha512_init(SshSha512Ctx *ctx);

/** @brief Feed @p len bytes of @p data into the running hash. */
void ssh_sha512_update(SshSha512Ctx *ctx, const uint8_t *data, size_t len);

/**
 * @brief Finalize the hash and write the 64-byte digest. The context is undefined
 *        afterwards; call init() again before reuse.
 * @param digest  Output buffer, SSH_SHA512_DIGEST_LEN bytes.
 */
void ssh_sha512_final(SshSha512Ctx *ctx, uint8_t digest[SSH_SHA512_DIGEST_LEN]);

/** @brief One-shot SHA-512: hash @p len bytes of @p data into @p digest (64 bytes). */
void ssh_sha512(const uint8_t *data, size_t len, uint8_t digest[SSH_SHA512_DIGEST_LEN]);

#endif // DETERMINISTICESPASYNCWEBSERVER_SSH_SHA512_H
