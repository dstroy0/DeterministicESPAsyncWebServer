// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file hmac_sha256.cpp
 * @brief HMAC-SHA2-256 implementation (RFC 2104).
 *
 * Implemented in terms of the dws_sha256 streaming functions so it compiles identically on Arduino and
 * native. The inner SHA-256 hardware acceleration (where present) is transparent through those calls.
 *
 * RFC 2104 construction: HMAC(K, m) = H((K XOR opad) || H((K XOR ipad) || m)), H = SHA-256, ipad = 0x36
 * repeated, opad = 0x5c repeated. Keys > 64 bytes are pre-hashed; keys <= 64 are zero-padded to the
 * 64-byte block. SSH-derived MAC keys are 32 bytes, so they are padded, not pre-hashed.
 */

#include "crypto/hmac_sha256.h"
#include <string.h>

// Build one 64-byte HMAC key block from a variable-length key (RFC 2104 §2).
static void build_key_block(const uint8_t *key, size_t key_len, uint8_t block[64], uint8_t pad_byte)
{
    uint8_t k[64] = {0};
    if (key_len > 64)
    {
        dws_sha256(key, key_len, k); // keys longer than the block are replaced by their SHA-256 hash
    }
    else
    {
        for (size_t i = 0; i < key_len; i++)
            k[i] = key[i];
    }
    for (int i = 0; i < 64; i++)
        block[i] = k[i] ^ pad_byte;
}

void dws_hmac_sha256_init(DwsHmacSha256Ctx *ctx, const uint8_t *key, size_t key_len)
{
    uint8_t ikey[64];
    build_key_block(key, key_len, ikey, 0x36u);      // ipad
    build_key_block(key, key_len, ctx->okey, 0x5cu); // opad (stored for final step)

    dws_sha256_init(&ctx->inner);
    dws_sha256_update(&ctx->inner, ikey, 64);
}

void dws_hmac_sha256_update(DwsHmacSha256Ctx *ctx, const uint8_t *data, size_t len)
{
    dws_sha256_update(&ctx->inner, data, len);
}

void dws_hmac_sha256_final(DwsHmacSha256Ctx *ctx, uint8_t mac[DWS_HMAC_SHA256_LEN])
{
    uint8_t inner_digest[DWS_SHA256_DIGEST_LEN];
    dws_sha256_final(&ctx->inner, inner_digest);

    // Outer hash: H(okey || inner_digest)
    DwsSha256Ctx outer;
    dws_sha256_init(&outer);
    dws_sha256_update(&outer, ctx->okey, 64);
    dws_sha256_update(&outer, inner_digest, DWS_SHA256_DIGEST_LEN);
    dws_sha256_final(&outer, mac);
}

void dws_hmac_sha256(const uint8_t *key, size_t key_len, const uint8_t *data, size_t len,
                     uint8_t mac[DWS_HMAC_SHA256_LEN])
{
    DwsHmacSha256Ctx ctx;
    dws_hmac_sha256_init(&ctx, key, key_len);
    dws_hmac_sha256_update(&ctx, data, len);
    dws_hmac_sha256_final(&ctx, mac);
}
