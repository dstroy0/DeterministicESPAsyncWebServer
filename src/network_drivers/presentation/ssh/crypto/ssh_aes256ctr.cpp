// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_aes256ctr.cpp
 * @brief AES-256-CTR implementation.
 *
 * Arduino path: delegates block encryption to mbedtls_aes_crypt_ecb(), which
 * the ESP32 mbedtls port routes to the hardware AES accelerator.
 *
 * Native path: compact software AES-256 using only the 256-byte forward
 * S-box and GF(2^8) multiply-by-2/3 for MixColumns.  No large lookup tables.
 */

#include "network_drivers/presentation/ssh/crypto/ssh_aes256ctr.h"
#include <string.h>

// ============================================================================
// ARDUINO - hardware-accelerated path via mbedtls
// ============================================================================

#ifdef ARDUINO

void ssh_aes256ctr_init(SshAesCtrCtx *ctx, const uint8_t key[32], const uint8_t iv[16])
{
    mbedtls_aes_init(&ctx->_mbed);
    mbedtls_aes_setkey_enc(&ctx->_mbed, key, 256);
    memcpy(ctx->counter, iv, 16);
    memset(ctx->keystream, 0, 16);
    ctx->pos = 0;
}

void ssh_aes256ctr_crypt(SshAesCtrCtx *ctx, const uint8_t *in, uint8_t *out, size_t len)
{
    // Encrypt the whole buffer in one mbedtls call: it acquires the HW AES
    // engine / loads the key once and manages the big-endian counter, keystream
    // block, and intra-block offset itself (our fields map 1:1 to its
    // nonce_counter / stream_block / nc_off). This replaces the previous
    // per-16-byte-block mbedtls_aes_crypt_ecb() loop, whose per-block setup
    // dominated bulk throughput on ESP32.
    size_t nc_off = ctx->pos;
    mbedtls_aes_crypt_ctr(&ctx->_mbed, len, &nc_off, ctx->counter, ctx->keystream, in, out);
    ctx->pos = (uint8_t)nc_off; // 0..15
}

void ssh_aes256ctr_wipe(SshAesCtrCtx *ctx)
{
    mbedtls_aes_free(&ctx->_mbed);
    volatile uint8_t *p = (volatile uint8_t *)ctx;
    for (size_t i = 0; i < sizeof(SshAesCtrCtx); i++)
        p[i] = 0;
}

// ============================================================================
// NATIVE - software AES-256 (for host-side unit tests only)
// ============================================================================

#else

// ---------------------------------------------------------------------------
// AES S-box (FIPS 197 Figure 7)
// ---------------------------------------------------------------------------

#include "shared_primitives/aes_block.h"

// ---------------------------------------------------------------------------
// Public API (native software path)
// ---------------------------------------------------------------------------

void ssh_aes256ctr_init(SshAesCtrCtx *ctx, const uint8_t key[32], const uint8_t iv[16])
{
    det_aes_key_expand(key, 8, ctx->rk);
    memcpy(ctx->counter, iv, 16);
    memset(ctx->keystream, 0, 16);
    ctx->pos = 0;
}

static void aes_block_sw(SshAesCtrCtx *ctx)
{
    det_aes_encrypt_block(ctx->rk, 14, ctx->counter, ctx->keystream);
    for (int j = 15; j >= 0; j--)
        if (++ctx->counter[j])
            break;
}

void ssh_aes256ctr_crypt(SshAesCtrCtx *ctx, const uint8_t *in, uint8_t *out, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        if (ctx->pos == 0)
            aes_block_sw(ctx);
        out[i] = in[i] ^ ctx->keystream[ctx->pos];
        ctx->pos = (uint8_t)((ctx->pos + 1u) & 0x0fu);
    }
}

void ssh_aes256ctr_wipe(SshAesCtrCtx *ctx)
{
    volatile uint8_t *p = (volatile uint8_t *)ctx;
    for (size_t i = 0; i < sizeof(SshAesCtrCtx); i++)
        p[i] = 0;
}

#endif // ARDUINO
