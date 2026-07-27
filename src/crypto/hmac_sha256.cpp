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
 *
 * The transient working memory that touches the key (the padded ipad/opad blocks, the intermediate inner
 * digest, and the one outer / one-shot hash context) lives in the shared crypto scratch (HMAC-256 region)
 * and is wiped on the way out - never on the stack. The caller-owned streaming context (DwsHmacSha256Ctx:
 * the opad key block + the inner hash state) is per-session state the caller wipes at teardown, so it is
 * NOT kept in the scratch (a long-lived value there would be clobbered by the next op).
 */

#include "crypto/hmac_sha256.h"
#include "crypto/crypto_opt.h"
#include "crypto/crypto_scratch.h" // crypto_work HMAC-256 region + dws_crypto_wipe
#include <string.h>
// HMAC-SHA256 is HW-SHA-dominated; the only -O lever is its SW key-block glue. On the P4 that rides the per-die
// -O3 default (whose win is -O3's loop-unroll parameter budget). The S3's ~4% O3 edge is the same parameter
// class (bisected on-device: -fpeel-loops and -funswitch-loops both no-op), not a single transform, and not
// worth -O3's code-size / miscompile baggage on a HW-dominated MAC - so the S3 keeps the -O2 default. Capturing
// that 4% deliberately would take a source #pragma GCC unroll on the key-block loops (a code change, not a flag).
// See crypto_opt.h caveat 1.
DWS_CRYPTO_HOT

namespace
{
// Transient HMAC-SHA256 working set in the shared crypto scratch (HMAC-256 region). One op at a time, wiped
// after each call. The two 64-byte key blocks double as key-padding scratch for build_key_block.
struct HmacWork
{
    uint8_t opad[64];                            ///< one-shot opad block (persists inner->outer); else key-pad scratch
    uint8_t ipad[64];                            ///< ipad block; also key-pad scratch once its ipad is consumed
    uint8_t inner_digest[DWS_SHA256_DIGEST_LEN]; ///< H((K XOR ipad) || m)
    DwsSha256Ctx hash;                           ///< transient hash: one-shot inner then outer; streaming final outer
};
static_assert(sizeof(HmacWork) <= DWS_CW_HMAC256_SZ, "HmacWork must fit its crypto_work region");
inline HmacWork *hm_work()
{
    return reinterpret_cast<HmacWork *>(crypto_work + DWS_CW_HMAC256_OFF);
}

// Build one 64-byte HMAC key block into @p block (RFC 2104 sec 2), using @p scratch (64 bytes) to hold the
// zero-padded / pre-hashed key. Both @p block and @p scratch are crypto_work-resident, never the stack.
void build_key_block(const uint8_t *key, size_t key_len, uint8_t block[64], uint8_t pad_byte, uint8_t scratch[64])
{
    memset(scratch, 0, 64);
    if (key_len > 64)
    {
        dws_sha256(key, key_len, scratch); // keys longer than the block are replaced by their SHA-256 hash
    }
    else
    {
        for (size_t i = 0; i < key_len; i++)
            scratch[i] = key[i];
    }
    for (int i = 0; i < 64; i++)
        block[i] = scratch[i] ^ pad_byte;
}
} // namespace

void dws_hmac_sha256_init(DwsHmacSha256Ctx *ctx, const uint8_t *key, size_t key_len)
{
    HmacWork *w = hm_work();
    build_key_block(key, key_len, w->ipad, 0x36u, w->opad);   // ipad -> scratch (opad slot holds the padded key)
    build_key_block(key, key_len, ctx->okey, 0x5cu, w->opad); // opad -> caller ctx (stored for the final step)

    dws_sha256_init(&ctx->inner);
    dws_sha256_update(&ctx->inner, w->ipad, 64);
    dws_crypto_wipe(crypto_work + DWS_CW_HMAC256_OFF, DWS_CW_HMAC256_SZ);
}

void dws_hmac_sha256_update(DwsHmacSha256Ctx *ctx, const uint8_t *data, size_t len)
{
    dws_sha256_update(&ctx->inner, data, len);
}

void dws_hmac_sha256_final(DwsHmacSha256Ctx *ctx, uint8_t mac[DWS_HMAC_SHA256_LEN])
{
    HmacWork *w = hm_work();
    dws_sha256_final(&ctx->inner, w->inner_digest);

    // Outer hash: H(okey || inner_digest)
    dws_sha256_init(&w->hash);
    dws_sha256_update(&w->hash, ctx->okey, 64);
    dws_sha256_update(&w->hash, w->inner_digest, DWS_SHA256_DIGEST_LEN);
    dws_sha256_final(&w->hash, mac);
    dws_crypto_wipe(crypto_work + DWS_CW_HMAC256_OFF, DWS_CW_HMAC256_SZ);
}

void dws_hmac_sha256(const uint8_t *key, size_t key_len, const uint8_t *data, size_t len,
                     uint8_t mac[DWS_HMAC_SHA256_LEN])
{
    // Self-contained (does not build a caller-facing context): ipad block first, fold it into the inner hash,
    // then reuse its slot as the opad key-padding scratch - so no key block ever lands on the stack.
    HmacWork *w = hm_work();
    build_key_block(key, key_len, w->ipad, 0x36u, w->opad); // ipad block (opad slot as key-pad scratch)
    dws_sha256_init(&w->hash);
    dws_sha256_update(&w->hash, w->ipad, 64);
    dws_sha256_update(&w->hash, data, len);
    dws_sha256_final(&w->hash, w->inner_digest); // inner = H((K XOR ipad) || m)

    build_key_block(key, key_len, w->opad, 0x5cu, w->ipad); // opad block (ipad slot now free as scratch)
    dws_sha256_init(&w->hash);
    dws_sha256_update(&w->hash, w->opad, 64);
    dws_sha256_update(&w->hash, w->inner_digest, DWS_SHA256_DIGEST_LEN);
    dws_sha256_final(&w->hash, mac); // HMAC = H((K XOR opad) || inner)
    dws_crypto_wipe(crypto_work + DWS_CW_HMAC256_OFF, DWS_CW_HMAC256_SZ);
}
