// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_hmac_sha512.h
 * @brief HMAC-SHA2-512 (RFC 2104 + FIPS 198-1).
 *
 * The MAC for the hmac-sha2-512 and hmac-sha2-512-etm@openssh.com SSH integrity algorithms.
 * Implemented over the ssh_sha512 streaming functions (SHA-512 block size 128 bytes). SSH-derived
 * MAC keys are 64 bytes (<= the block size), so the key is zero-padded, not pre-hashed. Pure.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SSH_HMAC_SHA512_H
#define DETERMINISTICESPASYNCWEBSERVER_SSH_HMAC_SHA512_H

#include "network_drivers/presentation/ssh/crypto/ssh_sha512.h"
#include <stddef.h>
#include <stdint.h>

/** @brief HMAC-SHA2-512 output length in bytes. */
#define SSH_HMAC_SHA512_LEN 64

/** @brief Streaming HMAC-SHA2-512 context (stores the opad key block + inner hash state). */
typedef struct
{
    uint8_t okey[SSH_SHA512_BLOCK_LEN]; ///< (key XOR opad), applied in the final step
    SshSha512Ctx inner;                 ///< inner hash: H((key XOR ipad) || message)
} SshHmacSha512Ctx;

/** @brief Begin an HMAC-SHA2-512 over @p key (keys > 128 bytes are pre-hashed per RFC 2104). */
void ssh_hmac_sha512_init(SshHmacSha512Ctx *ctx, const uint8_t *key, size_t key_len);
/** @brief Feed @p len message bytes. */
void ssh_hmac_sha512_update(SshHmacSha512Ctx *ctx, const uint8_t *data, size_t len);
/** @brief Finish, writing the 64-byte MAC. */
void ssh_hmac_sha512_final(SshHmacSha512Ctx *ctx, uint8_t mac[SSH_HMAC_SHA512_LEN]);

/** @brief One-shot HMAC-SHA2-512 over a single buffer. */
void ssh_hmac_sha512(const uint8_t *key, size_t key_len, const uint8_t *data, size_t len,
                     uint8_t mac[SSH_HMAC_SHA512_LEN]);

#endif // DETERMINISTICESPASYNCWEBSERVER_SSH_HMAC_SHA512_H
