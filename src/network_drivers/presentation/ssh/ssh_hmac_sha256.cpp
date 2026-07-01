// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_hmac_sha256.cpp
 * @brief HMAC-SHA2-256 implementation (RFC 2104).
 *
 * Implemented in terms of ssh_sha256 streaming functions so it compiles
 * identically on Arduino and native.  The inner SHA-256 hardware acceleration
 * (where present) is transparent through the ssh_sha256_* calls.
 *
 * RFC 2104 construction:
 *   HMAC(K, m) = H((K XOR opad) || H((K XOR ipad) || m))
 *
 * where ipad = 0x36 repeated, opad = 0x5c repeated, H = SHA-256.
 *
 * Since SSH-derived MAC keys are exactly 32 bytes (< SHA-256 block size of
 * 64 bytes), the key is NOT pre-hashed - it is padded with zeros to 64 bytes
 * internally and used directly as the HMAC key.
 */

#include "ssh_hmac_sha256.h"
#include <string.h>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Build one 64-byte HMAC key block from a variable-length key.
// Keys > 64 bytes are pre-hashed per RFC 2104 §2; keys ≤ 64 are zero-padded.
static void build_key_block(const uint8_t *key, size_t key_len, uint8_t block[64], uint8_t pad_byte)
{
    uint8_t k[64] = {0};
    if (key_len > 64)
    {
        // Keys longer than the block size are replaced by their SHA-256 hash.
        ssh_sha256(key, key_len, k);
        // SHA-256 output is 32 bytes; remaining 32 bytes stay zero.
    }
    else
    {
        for (size_t i = 0; i < key_len; i++)
            k[i] = key[i];
    }
    for (int i = 0; i < 64; i++)
        block[i] = k[i] ^ pad_byte;
}

// ---------------------------------------------------------------------------
// Streaming API
// ---------------------------------------------------------------------------

void ssh_hmac_sha256_init(SshHmacCtx *ctx, const uint8_t *key, size_t key_len)
{
    uint8_t ikey[64];
    build_key_block(key, key_len, ikey, 0x36u);      // ipad
    build_key_block(key, key_len, ctx->okey, 0x5cu); // opad (stored for final step)

    ssh_sha256_init(&ctx->inner);
    ssh_sha256_update(&ctx->inner, ikey, 64);
}

void ssh_hmac_sha256_update(SshHmacCtx *ctx, const uint8_t *data, size_t len)
{
    ssh_sha256_update(&ctx->inner, data, len);
}

void ssh_hmac_sha256_final(SshHmacCtx *ctx, uint8_t mac[SSH_HMAC_SHA256_LEN])
{
    uint8_t inner_digest[SSH_SHA256_DIGEST_LEN];
    ssh_sha256_final(&ctx->inner, inner_digest);

    // Outer hash: H(okey || inner_digest)
    SshSha256Ctx outer;
    ssh_sha256_init(&outer);
    ssh_sha256_update(&outer, ctx->okey, 64);
    ssh_sha256_update(&outer, inner_digest, SSH_SHA256_DIGEST_LEN);
    ssh_sha256_final(&outer, mac);
}

// ---------------------------------------------------------------------------
// One-shot convenience
// ---------------------------------------------------------------------------

void ssh_hmac_sha256(const uint8_t *key, size_t key_len, const uint8_t *data, size_t len,
                     uint8_t mac[SSH_HMAC_SHA256_LEN])
{
    SshHmacCtx ctx;
    ssh_hmac_sha256_init(&ctx, key, key_len);
    ssh_hmac_sha256_update(&ctx, data, len);
    ssh_hmac_sha256_final(&ctx, mac);
}
