// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_bignum.h
 * @brief 2048-bit big-integer arithmetic for DH-group14 and RSA-2048.
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * DESIGN RATIONALE
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * SshBigNum is a fixed-width 2048-bit integer stored as 64 little-endian
 * 32-bit limbs (d[0] = least significant).  Fixed width means:
 *   - Struct size is a compile-time constant: 256 bytes.
 *   - No dynamic allocation - both DH scalars and RSA key fragments fit in
 *     the same type and can live in BSS or on the stack.
 *   - Array indexing is bounds-safe; no VLA or pointer arithmetic hazards.
 *
 * On Arduino (ESP32), DH uses mbedtls_mpi from ESP-IDF (heap-allocated,
 * variable-length bignum), which has hardware-accelerated multiplication.
 * On native builds, the software Montgomery path is used - correct but
 * slower (~200 ms for a 2048-bit exponentiation on x86 at test time).
 * Since DH happens once per connection and ESP32 uses the HW path in
 * production, the native speed is acceptable for testing.
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * MONTGOMERY MULTIPLICATION
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * For DH-group14 the modulus p ends in ...FFFFFFFF FFFFFFFF (little-endian
 * d[0]=0xFFFFFFFF).  This gives Montgomery parameter:
 *
 *   p_inv = (-(p mod 2^32))^(-1) mod 2^32
 *         = (-(0xFFFFFFFF))^(-1) mod 2^32
 *         = (0x00000001)^(-1) mod 2^32
 *         = 1
 *
 * p_inv = 1 simplifies the inner reduction loop: m_i = t[i] * 1 = t[i].
 *
 * Montgomery product:  MonPro(a,b) = a·b·R^-1 mod p
 *   where R = 2^2048.
 *
 * To compute a·b mod p normally:
 *   1. Convert a, b to Montgomery form: aR = a·R mod p (= MonPro(a, R²mod p))
 *   2. Compute MonPro(aR, bR) = a·b·R mod p
 *   3. Convert back: MonPro(a·b·R, 1) = a·b mod p
 *
 * R² mod p is a precomputed 2048-bit constant (see ssh_bignum.cpp).
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * SCRATCH BUFFER
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * The Montgomery SOS multiplication uses a 129-word (516-byte) temporary
 * array.  The expmod function needs three SshBigNum temporaries (768 bytes).
 * All of these live in crypto_work[] (SSH_CRYPTO_WORK_SIZE = 1536 bytes)
 * defined in DetWebServerConfig.h and allocated in ssh_bignum.cpp.
 *
 * Layout during bn_expmod_group14():
 *   [0..255]    base_mont  (SshBigNum)
 *   [256..511]  result     (SshBigNum)
 *   [512..767]  tmp        (SshBigNum)
 *   [768..1283] mont_t     (uint32_t[129])
 *
 * crypto_work[] is zeroed via ssh_wipe() immediately after bn_expmod_group14()
 * returns so intermediate DH/RSA products do not persist in memory.
 *
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SSH_BIGNUM_H
#define DETERMINISTICESPASYNCWEBSERVER_SSH_BIGNUM_H

#include "DetWebServerConfig.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Fixed-width 2048-bit integer
// ---------------------------------------------------------------------------

/** @brief Number of 32-bit limbs in a 2048-bit integer. */
#define SSH_BN_LIMBS 64

/**
 * @brief A 2048-bit unsigned integer stored as 64 little-endian 32-bit limbs.
 *
 * d[0] = least significant 32 bits.
 * d[63] = most significant 32 bits.
 */
struct SshBigNum
{
    uint32_t d[SSH_BN_LIMBS]; ///< 256 bytes of magnitude, little-endian limbs.
};

// ---------------------------------------------------------------------------
// Scratch buffer (defined in ssh_bignum.cpp)
// ---------------------------------------------------------------------------

/**
 * @brief Global scratch buffer for big-number temporaries.
 *
 * Only one SSH KEX or RSA-sign operation runs at a time (single Arduino
 * loop task, synchronous handshake).  Zeroed after every use via ssh_wipe().
 *
 * See the layout comment in the file header for the exact field map.
 */
extern uint8_t crypto_work[SSH_CRYPTO_WORK_SIZE];

// ---------------------------------------------------------------------------
// Conversion helpers
// ---------------------------------------------------------------------------

/**
 * @brief Read a big-endian byte array of @p len bytes into a SshBigNum.
 *
 * If @p len < 256 the most-significant limbs are zeroed.
 * If @p len > 256 only the least-significant 256 bytes are read.
 *
 * @param out   Destination bignum.
 * @param bytes Big-endian source bytes.
 * @param len   Number of source bytes (typically 256 for 2048-bit).
 */
void bn_from_bytes(SshBigNum *out, const uint8_t *bytes, size_t len);

/**
 * @brief Write a SshBigNum as a 256-byte big-endian array.
 *
 * @param bytes Destination buffer (exactly 256 bytes).
 * @param in    Source bignum.
 */
void bn_to_bytes(uint8_t bytes[256], const SshBigNum *in);

// ---------------------------------------------------------------------------
// Comparison
// ---------------------------------------------------------------------------

/**
 * @brief Compare two SshBigNum values.
 * @return -1 if a < b, 0 if a == b, 1 if a > b.
 */
int bn_cmp(const SshBigNum *a, const SshBigNum *b);

/**
 * @brief Return non-zero if @p a is zero (all limbs zero).
 */
int bn_is_zero(const SshBigNum *a);

// ---------------------------------------------------------------------------
// DH-group14 modular exponentiation
// ---------------------------------------------------------------------------

/**
 * @brief Compute out = base^exp mod group14_p.
 *
 * Uses Montgomery modular exponentiation with left-to-right binary scan.
 * Uses crypto_work[] for all temporaries; zeros crypto_work[] on exit.
 *
 * On Arduino the computation is delegated to mbedtls_mpi_exp_mod() which
 * uses hardware multiplication and blinding.
 *
 * @param out   Result (base^exp mod p, 2048-bit).
 * @param base  Base value; must satisfy 1 < base < p-1.
 * @param exp   Exponent (e.g. the 2048-bit private DH scalar y).
 */
void bn_expmod_group14(SshBigNum *out, const SshBigNum *base, const SshBigNum *exp);

/**
 * @brief Validate a received DH public value.
 *
 * RFC 4253 §8: the received value e (or f) must satisfy 1 < e < p-1.
 * Returns 0 if the value is valid, -1 otherwise.
 *
 * @param v  Received public DH value.
 */
int bn_dh_validate(const SshBigNum *v);

// ---------------------------------------------------------------------------
// Group-14 prime constant (exposed for key-derivation and validation)
// ---------------------------------------------------------------------------

/** @brief The RFC 3526 MODP group-14 prime (2048-bit). */
extern const SshBigNum group14_p;

/** @brief Generator for group-14: g = 2. */
extern const SshBigNum group14_g;

#endif // DETERMINISTICESPASYNCWEBSERVER_SSH_BIGNUM_H
