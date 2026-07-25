// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file hmac_sha512.h
 * @brief HMAC-SHA2-512 (RFC 2104 + FIPS 198-1) - streaming context and one-shot API.
 *
 * The shared HMAC-SHA512 primitive. Backs the SSH hmac-sha2-512 / hmac-sha2-512-etm@openssh.com
 * integrity algorithms. Built over the dws_sha512 streaming functions (SHA-512 block size 128 bytes).
 * SSH-derived MAC keys are 64 bytes (<= the block size), so the key is zero-padded, not pre-hashed.
 * Pure crypto; the protocol layer that uses it owns the verify-before-act ordering.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_CRYPTO_HMAC_SHA512_H
#define DETERMINISTICESPASYNCWEBSERVER_CRYPTO_HMAC_SHA512_H

#include "crypto/sha512.h"
#include <stddef.h>
#include <stdint.h>

/** @brief HMAC-SHA2-512 output length in bytes. */
#define DWS_HMAC_SHA512_LEN 64

/** @brief Streaming HMAC-SHA2-512 context (stores the opad key block + inner hash state). */
typedef struct
{
    uint8_t okey[DWS_SHA512_BLOCK_LEN]; ///< (key XOR opad), applied in the final step
    DwsSha512Ctx inner;                 ///< inner hash: H((key XOR ipad) || message)
} DwsHmacSha512Ctx;

/** @brief Begin an HMAC-SHA2-512 over @p key (keys > 128 bytes are pre-hashed per RFC 2104). */
void dws_hmac_sha512_init(DwsHmacSha512Ctx *ctx, const uint8_t *key, size_t key_len);
/** @brief Feed @p len message bytes. */
void dws_hmac_sha512_update(DwsHmacSha512Ctx *ctx, const uint8_t *data, size_t len);
/** @brief Finish, writing the 64-byte MAC. */
void dws_hmac_sha512_final(DwsHmacSha512Ctx *ctx, uint8_t mac[DWS_HMAC_SHA512_LEN]);

/** @brief One-shot HMAC-SHA2-512 over a single buffer. */
void dws_hmac_sha512(const uint8_t *key, size_t key_len, const uint8_t *data, size_t len,
                     uint8_t mac[DWS_HMAC_SHA512_LEN]);

#endif // DETERMINISTICESPASYNCWEBSERVER_CRYPTO_HMAC_SHA512_H
