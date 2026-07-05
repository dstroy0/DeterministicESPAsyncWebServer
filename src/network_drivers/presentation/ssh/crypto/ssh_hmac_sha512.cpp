// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_hmac_sha512.cpp
 * @brief HMAC-SHA2-512 implementation (RFC 2104). See ssh_hmac_sha512.h.
 *
 * HMAC(K, m) = H((K XOR opad) || H((K XOR ipad) || m)), H = SHA-512, block = 128 bytes,
 * ipad = 0x36 repeated, opad = 0x5c repeated.
 */

#include "network_drivers/presentation/ssh/crypto/ssh_hmac_sha512.h"

namespace
{
// One 128-byte HMAC key block: keys > 128 bytes are pre-hashed (RFC 2104), else zero-padded.
void build_key_block(const uint8_t *key, size_t key_len, uint8_t block[SSH_SHA512_BLOCK_LEN], uint8_t pad_byte)
{
    uint8_t k[SSH_SHA512_BLOCK_LEN] = {0};
    if (key_len > SSH_SHA512_BLOCK_LEN)
    {
        ssh_sha512(key, key_len, k); // 64-byte digest; the remaining 64 bytes stay zero
    }
    else
    {
        for (size_t i = 0; i < key_len; i++)
            k[i] = key[i];
    }
    for (int i = 0; i < SSH_SHA512_BLOCK_LEN; i++)
        block[i] = (uint8_t)(k[i] ^ pad_byte);
}
} // namespace

void ssh_hmac_sha512_init(SshHmacSha512Ctx *ctx, const uint8_t *key, size_t key_len)
{
    uint8_t ikey[SSH_SHA512_BLOCK_LEN];
    build_key_block(key, key_len, ikey, 0x36u);      // ipad
    build_key_block(key, key_len, ctx->okey, 0x5cu); // opad (kept for the final step)
    ssh_sha512_init(&ctx->inner);
    ssh_sha512_update(&ctx->inner, ikey, SSH_SHA512_BLOCK_LEN);
}

void ssh_hmac_sha512_update(SshHmacSha512Ctx *ctx, const uint8_t *data, size_t len)
{
    ssh_sha512_update(&ctx->inner, data, len);
}

void ssh_hmac_sha512_final(SshHmacSha512Ctx *ctx, uint8_t mac[SSH_HMAC_SHA512_LEN])
{
    uint8_t inner_digest[SSH_SHA512_DIGEST_LEN];
    ssh_sha512_final(&ctx->inner, inner_digest);
    SshSha512Ctx outer;
    ssh_sha512_init(&outer);
    ssh_sha512_update(&outer, ctx->okey, SSH_SHA512_BLOCK_LEN);
    ssh_sha512_update(&outer, inner_digest, SSH_SHA512_DIGEST_LEN);
    ssh_sha512_final(&outer, mac);
}

void ssh_hmac_sha512(const uint8_t *key, size_t key_len, const uint8_t *data, size_t len,
                     uint8_t mac[SSH_HMAC_SHA512_LEN])
{
    SshHmacSha512Ctx ctx;
    ssh_hmac_sha512_init(&ctx, key, key_len);
    ssh_hmac_sha512_update(&ctx, data, len);
    ssh_hmac_sha512_final(&ctx, mac);
}
