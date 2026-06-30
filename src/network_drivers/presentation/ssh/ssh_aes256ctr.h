// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_aes256ctr.h
 * @brief AES-256-CTR stream cipher context and API.
 *
 * AES-256-CTR is the mandatory cipher for this SSH implementation
 * (aes256-ctr, RFC 4344 §4).  CTR mode turns AES into a stream cipher:
 * each 16-byte counter block is encrypted with AES-ECB to produce a
 * keystream block, and data is XOR'd with the keystream.  Encrypt and
 * decrypt are identical operations.
 *
 * COUNTER FORMAT (RFC 4344 §4)
 * The 16-byte IV/counter block increments as a big-endian 128-bit integer
 * after each 16 bytes of output.  The initial IV is derived per RFC 4253 §7.2:
 *   IV = SHA256(K || H || 'A' || session_id)   (C→S)
 *   IV = SHA256(K || H || 'B' || session_id)   (S→C)
 *
 * PLATFORM SELECTION
 * ──────────────────
 * On Arduino (ESP32) the struct embeds an actual mbedtls_aes_context so that
 * mbedtls_aes_setkey_enc() / mbedtls_aes_crypt_ecb() are used for every AES
 * block encryption.  The ESP32 mbedtls port routes these calls to the
 * hardware AES accelerator transparently.  The compiler knows the true size
 * of mbedtls_aes_context at compile time - no guessing, no overflow possible.
 *
 * On native (x86 test builds) the struct stores a software-expanded 60-word
 * AES-256 round key schedule (240 bytes) and a software AES block cipher is
 * used.  The software path exists only to allow host-side unit testing; it is
 * never compiled into the production ESP32 firmware.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SSH_AES256CTR_H
#define DETERMINISTICESPASYNCWEBSERVER_SSH_AES256CTR_H

#include "shared_primitives/shim.h"

// ---------------------------------------------------------------------------
// Platform-specific AES context storage
// ---------------------------------------------------------------------------

#ifdef ARDUINO
// On ESP32/Arduino, include the real mbedtls header.  The struct stores the
// actual mbedtls_aes_context so the compiler knows its size exactly.
#include <mbedtls/aes.h>

struct SshAesCtrCtx
{
    mbedtls_aes_context _mbed; ///< mbedtls context with pre-expanded key schedule.
    uint8_t counter[16];       ///< Current CTR block (big-endian 128-bit counter).
    uint8_t keystream[16];     ///< Buffered keystream from last AES-ECB call.
    uint8_t pos;               ///< Next byte position within keystream[].
};

#else // Native - software AES, no mbedtls dependency

struct SshAesCtrCtx
{
    uint32_t rk[60];       ///< AES-256 expanded round key schedule (60 words, 240 bytes).
    uint8_t counter[16];   ///< Current CTR block (big-endian 128-bit counter).
    uint8_t keystream[16]; ///< Buffered keystream from last AES block encrypt.
    uint8_t pos;           ///< Next byte position within keystream[].
};

#endif // ARDUINO

// ---------------------------------------------------------------------------
// API
// ---------------------------------------------------------------------------

/**
 * @brief Initialize an AES-256-CTR context.
 *
 * Expands the key schedule (hardware on Arduino, software on native) and
 * stores the initial counter/IV.
 *
 * @param ctx  Uninitialized context.
 * @param key  32-byte AES-256 key (derived from KEX via RFC 4253 §7.2).
 * @param iv   16-byte initial counter block (derived from KEX).
 */
void ssh_aes256ctr_init(SshAesCtrCtx *ctx, const uint8_t key[32], const uint8_t iv[16]);

/**
 * @brief Encrypt or decrypt @p len bytes in-place (or src→dst).
 *
 * AES-CTR is symmetric: XOR with keystream is its own inverse.
 * Calling this function for encryption and decryption is identical.
 *
 * The counter advances by ceil(len/16) blocks.  If @p in == @p out the
 * operation is in-place (the only mode used by ssh_packet.cpp).
 *
 * @param ctx  Initialized AES-256-CTR context.
 * @param in   Input bytes.
 * @param out  Output bytes (may equal @p in for in-place).
 * @param len  Number of bytes to process.
 */
void ssh_aes256ctr_crypt(SshAesCtrCtx *ctx, const uint8_t *in, uint8_t *out, size_t len);

/**
 * @brief Zero the key schedule and all context fields.
 *
 * Call on disconnect.  Uses a volatile wipe so the compiler cannot elide it.
 * On Arduino also calls mbedtls_aes_free() to release any internal resources.
 *
 * @param ctx  Context to wipe.
 */
void ssh_aes256ctr_wipe(SshAesCtrCtx *ctx);

#endif // DETERMINISTICESPASYNCWEBSERVER_SSH_AES256CTR_H
