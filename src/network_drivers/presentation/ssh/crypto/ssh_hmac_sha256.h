// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_hmac_sha256.h
 * @brief HMAC-SHA2-256 (RFC 2104 + FIPS 198-1).
 *
 * Used for SSH binary packet MAC in both directions after key exchange.
 *
 * MAC computation (RFC 4253 §6.4):
 *   mac = HMAC-SHA256(mac_key, uint32_be(seq_num) || plaintext_packet)
 *
 * where plaintext_packet = packet_length || padding_length || payload || padding
 * (the unencrypted bytes - MAC is computed over plaintext before encryption).
 *
 * SECURITY NOTE - MAC must be verified before plaintext is acted upon.
 * ssh_packet.cpp enforces this order: decrypt → verify → dispatch.  The
 * functions in this file are pure crypto; the ordering guarantee lives in
 * the packet layer.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SSH_HMAC_SHA256_H
#define DETERMINISTICESPASYNCWEBSERVER_SSH_HMAC_SHA256_H

#include "network_drivers/presentation/ssh/crypto/ssh_sha256.h"
#include <stddef.h>
#include <stdint.h>

/** @brief HMAC-SHA2-256 output length in bytes. */
#define SSH_HMAC_SHA256_LEN 32

/**
 * @brief Compute HMAC-SHA2-256 over a single contiguous buffer.
 *
 * Equivalent to calling the streaming variant with one update call.
 *
 * @param key      MAC key bytes.
 * @param key_len  Key length in bytes (must be ≤ 64; keys > 64 bytes are
 *                 pre-hashed per RFC 2104; keys ≤ 64 bytes are zero-padded
 *                 to the block length).  SSH-derived keys are always 32 bytes.
 * @param data    Input bytes.
 * @param len     Input length.
 * @param mac     Output buffer, must be SSH_HMAC_SHA256_LEN bytes.
 */
void ssh_hmac_sha256(const uint8_t *key, size_t key_len, const uint8_t *data, size_t len,
                     uint8_t mac[SSH_HMAC_SHA256_LEN]);

/**
 * @brief Streaming HMAC-SHA2-256 context.
 *
 * For the packet MAC where data arrives in two separate pieces:
 *   1. 4-byte big-endian sequence number
 *   2. plaintext packet bytes
 *
 * Usage:
 *   ssh_hmac_sha256_init(&ctx, key);
 *   ssh_hmac_sha256_update(&ctx, seq_bytes, 4);
 *   ssh_hmac_sha256_update(&ctx, packet, pkt_len);
 *   ssh_hmac_sha256_final(&ctx, mac_out);
 */
struct SshHmacCtx
{
    SshSha256Ctx inner; ///< Inner hash context (key XOR ipad prepended).
    uint8_t okey[64];   ///< Outer key block (key XOR opad), stored for final step.
};

/**
 * @brief Initialize a streaming HMAC-SHA2-256 context.
 *
 * @param ctx      Context to initialize.
 * @param key      Key bytes (≤ 64; keys > 64 are pre-hashed per RFC 2104).
 * @param key_len  Length of key in bytes.
 */
void ssh_hmac_sha256_init(SshHmacCtx *ctx, const uint8_t *key, size_t key_len);

/** @brief Feed @p len bytes into the running HMAC. */
void ssh_hmac_sha256_update(SshHmacCtx *ctx, const uint8_t *data, size_t len);

/** @brief Finalize and write the 32-byte MAC. */
void ssh_hmac_sha256_final(SshHmacCtx *ctx, uint8_t mac[SSH_HMAC_SHA256_LEN]);

#endif // DETERMINISTICESPASYNCWEBSERVER_SSH_HMAC_SHA256_H
