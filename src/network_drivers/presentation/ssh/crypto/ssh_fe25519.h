// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_fe25519.h
 * @brief ESP32-S3 GF(2^255-19) field layer on the RSA/MPI hardware accelerator (X25519 + Ed25519).
 *
 * Field elements are canonical `uint32[8]` (< p = 2^255-19) so every field multiply is a single
 * 256-bit modular multiply on the S3 RSA accelerator (~1,386 cycles vs 7,955 for the software SIMD
 * `ssh_gf_mul`). add/sub are native 32-bit (carry + one conditional subtract of p); bytes<->fe is a
 * per-scalar-mult conversion, not per multiply. This is the shared engine behind both the X25519 KEX
 * (`ssh_curve25519.cpp`) and the Ed25519 host-key signature (`ssh_ed25519.cpp`) on the S3; the
 * radix-2^16 `ssh_gf` path is the native / non-S3 fallback in both.
 *
 * The accelerator (and its lock) are shared with mbedTLS RSA/DH, so a scalar-mult brackets itself with
 * `ssh_fe_hw_enable()` / `ssh_fe_hw_disable()` (mbedTLS's own `esp_mpi_{enable,disable}_hardware_hw_op`,
 * i.e. acquire the MPI lock + clock/power the peripheral) and holds the lock for its whole run.
 *
 * Header-only `static inline` on purpose: the cheap ops (add/sub/cswap) inline into the ladder in each
 * translation unit with no cross-TU call overhead, and the whole layer stays one source of truth.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SSH_FE25519_H
#define DETERMINISTICESPASYNCWEBSERVER_SSH_FE25519_H

#include <stdint.h>

#ifdef ARDUINO
#include "sdkconfig.h" // CONFIG_IDF_TARGET_ESP32S3
#endif

// Gated to Arduino on the S3: the field layer drives the RSA peripheral through mbedTLS's port
// (esp_mpi_*), which only exists in the on-device toolchain.
#if defined(ARDUINO) && defined(CONFIG_IDF_TARGET_ESP32S3) && CONFIG_IDF_TARGET_ESP32S3
#define DWS_FE25519_MPI_HW 1
#endif

#ifdef DWS_FE25519_MPI_HW

#include "soc/hwcrypto_reg.h" // RSA/MPI accelerator register map (MODMULT)
#include "soc/soc.h"          // DR_REG_RSA_BASE

extern "C"
{
    void esp_mpi_enable_hardware_hw_op(void);  // mbedTLS port: acquire the MPI lock + clock/power the peripheral
    void esp_mpi_disable_hardware_hw_op(void); // release the lock + power down
}

#define SSH_RSA_REG(a) (*(volatile uint32_t *)(a))

/** @brief A field element of GF(2^255-19): canonical, eight little-endian 32-bit limbs (< p). */
typedef uint32_t fe[8];

// Constants for the 256-bit modular multiply mod p = 2^255-19 (scratchpad/montconst.py): Montgomery m'
// and R^2 mod p (= 38^2 = 1444 = 0x5a4). Preloading R^2 into the result block makes the accelerator
// return a plain residue X*Y mod p rather than a Montgomery form (the esp_mpi_mul_mpi_mod convention).
static const uint32_t FE_MOD_MPRIME = 0x286bca1bu;
static const uint32_t FE_MOD_P[8] = {0xffffffedu, 0xffffffffu, 0xffffffffu, 0xffffffffu,
                                     0xffffffffu, 0xffffffffu, 0xffffffffu, 0x7fffffffu};
static const uint32_t FE_MOD_R2[8] = {0x000005a4u, 0, 0, 0, 0, 0, 0, 0};

// Acquire the accelerator (lock + power) for a scalar-mult, and drop it after. Bracket every run.
static inline void ssh_fe_hw_enable(void)
{
    esp_mpi_enable_hardware_hw_op();    // lock + clock/power the peripheral
    SSH_RSA_REG(RSA_INTERRUPT_REG) = 0; // poll only, no completion IRQ
}
static inline void ssh_fe_hw_disable(void)
{
    esp_mpi_disable_hardware_hw_op(); // release the lock + power down
}

// z = x*y mod p (8 words / 256-bit). Requires ssh_fe_hw_enable() first. Output is always canonical (< p).
static inline void fe_mul(fe z, const fe x, const fe y) // safe if z aliases x/y
{
    volatile uint32_t *M = (volatile uint32_t *)RSA_MEM_M_BLOCK_BASE;
    volatile uint32_t *X = (volatile uint32_t *)RSA_MEM_X_BLOCK_BASE;
    volatile uint32_t *Y = (volatile uint32_t *)RSA_MEM_Y_BLOCK_BASE;
    volatile uint32_t *Z = (volatile uint32_t *)RSA_MEM_Z_BLOCK_BASE;
    SSH_RSA_REG(RSA_LENGTH_REG) = 8 - 1; // mode = words - 1
    SSH_RSA_REG(RSA_M_DASH_REG) = FE_MOD_MPRIME;
    for (int i = 0; i < 8; i++)
    {
        M[i] = FE_MOD_P[i];
        X[i] = x[i];
        Y[i] = y[i];
        Z[i] = FE_MOD_R2[i]; // r = R^2 mod p in the result block -> plain (non-Montgomery) output
    }
    SSH_RSA_REG(RSA_CLEAR_INTERRUPT_REG) = 1; // clear any stale done flag before starting
    SSH_RSA_REG(RSA_MOD_MULT_START_REG) = 1;
    while (SSH_RSA_REG(RSA_QUERY_INTERRUPT_REG) == 0)
        ;
    SSH_RSA_REG(RSA_CLEAR_INTERRUPT_REG) = 1;
    for (int i = 0; i < 8; i++)
        z[i] = Z[i];
}
static inline void fe_sq(fe o, const fe x)
{
    fe_mul(o, x, x);
}

static inline void fe_copy(fe o, const fe a)
{
    for (int i = 0; i < 8; i++)
        o[i] = a[i];
}
static inline void fe_0(fe o)
{
    for (int i = 0; i < 8; i++)
        o[i] = 0;
}
static inline void fe_1(fe o)
{
    o[0] = 1;
    for (int i = 1; i < 8; i++)
        o[i] = 0;
}
// If o >= p (o is in [p, 2p)), subtract p. Constant-time: the borrow out of o-p selects o or o-p.
static inline void fe_reduce_once(fe o)
{
    uint32_t t[8];
    int64_t b = 0;
    for (int i = 0; i < 8; i++)
    {
        b += (int64_t)o[i] - (int64_t)FE_MOD_P[i];
        t[i] = (uint32_t)b;
        b >>= 32;
    }
    uint32_t keep = (uint32_t)b; // 0 if o>=p (take t=o-p), 0xffffffff if o<p (keep o)
    for (int i = 0; i < 8; i++)
        o[i] = (o[i] & keep) | (t[i] & ~keep);
}
static inline void fe_add(fe o, const fe x, const fe y) // x,y < p -> o = x+y mod p
{
    uint64_t c = 0;
    for (int i = 0; i < 8; i++)
    {
        c += (uint64_t)x[i] + y[i];
        o[i] = (uint32_t)c;
        c >>= 32;
    }
    fe_reduce_once(o); // x+y < 2p, one conditional subtract
}
static inline void fe_sub(fe o, const fe x, const fe y) // x,y < p -> o = x-y mod p
{
    int64_t b = 0;
    uint32_t t[8];
    for (int i = 0; i < 8; i++)
    {
        b += (int64_t)x[i] - (int64_t)y[i];
        t[i] = (uint32_t)b;
        b >>= 32;
    }
    uint32_t borrow = (uint32_t)b; // 0xffffffff if x<y -> add p back
    uint64_t c = 0;
    for (int i = 0; i < 8; i++)
    {
        c += (uint64_t)t[i] + (FE_MOD_P[i] & borrow);
        o[i] = (uint32_t)c;
        c >>= 32;
    }
}
static inline void fe_cswap(fe x, fe y, uint32_t swap) // constant-time swap of x,y when swap==1
{
    uint32_t mask = (uint32_t)(-(int32_t)swap);
    for (int i = 0; i < 8; i++)
    {
        uint32_t t = mask & (x[i] ^ y[i]);
        x[i] ^= t;
        y[i] ^= t;
    }
}
static inline void fe_frombytes(fe o, const uint8_t b[32])
{
    for (int i = 0; i < 8; i++)
        o[i] = (uint32_t)b[4 * i] | ((uint32_t)b[4 * i + 1] << 8) | ((uint32_t)b[4 * i + 2] << 16) |
               ((uint32_t)b[4 * i + 3] << 24);
    o[7] &= 0x7fffffffu; // Ed25519/X25519 both ignore bit 255 of the y/u coordinate
    fe_reduce_once(o);   // the masked value can still be in [p, 2^255) -> canonicalize
}
static inline void fe_tobytes(uint8_t b[32], const fe a)
{
    fe t;
    fe_copy(t, a);
    fe_reduce_once(t); // freeze to the canonical residue
    for (int i = 0; i < 8; i++)
    {
        b[4 * i] = (uint8_t)t[i];
        b[4 * i + 1] = (uint8_t)(t[i] >> 8);
        b[4 * i + 2] = (uint8_t)(t[i] >> 16);
        b[4 * i + 3] = (uint8_t)(t[i] >> 24);
    }
}
// o = a^(p-2) = a^-1 mod p (tweetnacl square-and-multiply chain for the exponent 2^255-21).
static inline void fe_invert(fe o, const fe a)
{
    fe c;
    fe_copy(c, a);
    for (int i = 253; i >= 0; i--)
    {
        fe_sq(c, c);
        if (i != 2 && i != 4)
            fe_mul(c, c, a);
    }
    fe_copy(o, c);
}
// o = a^((p-5)/8) = a^(2^252-3) - the square-root exponent for Ed25519 point decompression.
static inline void fe_pow2523(fe o, const fe a)
{
    fe c;
    fe_copy(c, a);
    for (int i = 250; i >= 0; i--)
    {
        fe_sq(c, c);
        if (i != 1)
            fe_mul(c, c, a);
    }
    fe_copy(o, c);
}
// Low bit of the canonical encoding (Ed25519 x-coordinate sign).
static inline int fe_parity(const fe a)
{
    uint8_t d[32];
    fe_tobytes(d, a);
    return d[0] & 1;
}
// 0 if a and b encode the same field element, -1 otherwise (constant-time over the 32 bytes).
static inline int fe_neq(const fe a, const fe b)
{
    uint8_t c[32];
    uint8_t d[32];
    fe_tobytes(c, a);
    fe_tobytes(d, b);
    unsigned diff = 0;
    for (int i = 0; i < 32; i++)
        diff |= (unsigned)(c[i] ^ d[i]);
    return (int)((1 & ((diff - 1) >> 8)) - 1);
}

#endif // DWS_FE25519_MPI_HW
#endif // DETERMINISTICESPASYNCWEBSERVER_SSH_FE25519_H
