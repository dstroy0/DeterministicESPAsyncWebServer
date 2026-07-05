// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_curve25519.h
 * @brief Curve25519 field arithmetic + X25519 (RFC 7748) for the curve25519-sha256 KEX.
 *
 * Field elements are GF(2^255 - 19) in a portable radix-2^16 representation (sixteen
 * int64 limbs), so no 128-bit integer type is needed - important because 32-bit xtensa
 * (ESP32) gcc has no __int128. The same field arithmetic backs Ed25519 (ssh_ed25519),
 * so the field ops are exported here. Correctness is pinned to the RFC 7748 §5.2 test
 * vectors (test_ssh_ed25519). No heap; all state is on the stack.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SSH_CURVE25519_H
#define DETERMINISTICESPASYNCWEBSERVER_SSH_CURVE25519_H

#include <stddef.h>
#include <stdint.h>

/** @brief A field element of GF(2^255 - 19): 16 limbs, radix 2^16 (limb i weighs 2^(16i)). */
typedef int64_t ssh_gf[16];

// --- Field arithmetic (shared with ssh_ed25519) ----------------------------

void ssh_gf_copy(ssh_gf out, const ssh_gf in);               ///< out = in
void ssh_gf_add(ssh_gf out, const ssh_gf a, const ssh_gf b); ///< out = a + b (unreduced)
void ssh_gf_sub(ssh_gf out, const ssh_gf a, const ssh_gf b); ///< out = a - b (unreduced)
void ssh_gf_mul(ssh_gf out, const ssh_gf a, const ssh_gf b); ///< out = a * b mod p
void ssh_gf_sq(ssh_gf out, const ssh_gf a);                  ///< out = a^2 mod p
void ssh_gf_inv(ssh_gf out, const ssh_gf a);                 ///< out = a^-1 mod p (= a^(p-2))
void ssh_gf_pack(uint8_t out[32], const ssh_gf a);           ///< canonical little-endian 32-byte encoding
void ssh_gf_unpack(ssh_gf out, const uint8_t in[32]);        ///< decode 32 bytes (high bit ignored)
void ssh_gf_cswap(ssh_gf p, ssh_gf q, int b);                ///< constant-time conditional swap of p,q when b==1

// --- X25519 (RFC 7748) -----------------------------------------------------

/**
 * @brief X25519 scalar multiplication: @p out = @p scalar * @p point (RFC 7748 §5).
 *
 * @p scalar and @p point are 32-byte little-endian; the scalar is clamped internally.
 * @p out may alias neither input. Constant-time in the scalar (Montgomery ladder with
 * conditional swaps).
 */
void ssh_x25519(uint8_t out[32], const uint8_t scalar[32], const uint8_t point[32]);

/** @brief X25519 with the standard base point u=9: @p out = @p scalar * G. */
void ssh_x25519_base(uint8_t out[32], const uint8_t scalar[32]);

#endif // DETERMINISTICESPASYNCWEBSERVER_SSH_CURVE25519_H
