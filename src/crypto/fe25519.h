// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file fe25519.h
 * @brief Per-variant GF(2^255-19) field layer on the RSA/MPI hardware accelerator (X25519 + Ed25519).
 *
 * Field elements are canonical `uint32[8]` (< p = 2^255-19) so every field multiply is a single
 * 256-bit modular multiply on the RSA accelerator (S3: ~1,386 cycles vs 7,955 for the software SIMD
 * `dws_gf_mul`; P4: ~2,118 cycles / 5.9 us). add/sub are native 32-bit (carry + one conditional subtract
 * of p); bytes<->fe is a per-scalar-mult conversion, not per multiply. This is the shared engine behind
 * both the X25519 KEX (`dws_curve25519.cpp`) and the Ed25519 host-key signature (`dws_ed25519.cpp`) on
 * every die with a single-shot hardware MODMULT (S3 hw_ver1, P4 and newer hw_ver3 - see the gate below);
 * the radix-2^16 `dws_gf` path is the native / classic-ESP32 fallback in both.
 *
 * The accelerator (and its lock) are shared with mbedTLS RSA/DH, so a scalar-mult brackets itself with
 * `dws_fe_hw_enable()` / `dws_fe_hw_disable()` (mbedTLS's own `esp_mpi_{enable,disable}_hardware_hw_op`,
 * i.e. acquire the MPI lock + clock/power the peripheral) and holds the lock for its whole run.
 *
 * Header-only `static inline` on purpose: the cheap ops (add/sub/cswap) inline into the ladder in each
 * translation unit with no cross-TU call overhead, and the whole layer stays one source of truth.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_CRYPTO_FE25519_H
#define DETERMINISTICESPASYNCWEBSERVER_CRYPTO_FE25519_H

#include <stdint.h>

#ifdef ARDUINO
#include "sdkconfig.h" // CONFIG_IDF_TARGET_* selects the die
#endif

// The GF(2^255-19) field layer runs each multiply as one 256-bit modular multiply on the RSA/MPI
// accelerator (esp_mpi_*, which exists only in the on-device toolchain). Enabled PER-VARIANT on every
// die with a single-shot hardware MODMULT op: the ESP32-S3 (older "hw_ver1" RSA register names) and the
// ESP32-P4 and newer (the "hw_ver3" names). The classic ESP32 has no single-shot MODMULT - it needs two
// MULT passes (see esp_mpi_mul_mpi_mod_hw_op) - so it keeps the software radix-2^16 ladder. Add a die to
// this list once its rig has passed the on-device RFC KAT (test/interop + main_cryptobench).
#if defined(ARDUINO) && ((defined(CONFIG_IDF_TARGET_ESP32S3) && CONFIG_IDF_TARGET_ESP32S3) ||                          \
                         (defined(CONFIG_IDF_TARGET_ESP32P4) && CONFIG_IDF_TARGET_ESP32P4))
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

// The RSA MODMULT register set was renamed between silicon generations. Key off the macro the die's
// soc/rsa_reg.h actually defines so one code path serves both: the modular-multiply sequence, the
// Montgomery constants, and the "poll until the done/idle bit reads 1" completion are identical - only
// the register names (and the done bit's name: INTERRUPT vs IDLE) differ. Verified byte-exact against
// the RFC 8032 / RFC 6979 KATs on both an S3 (hw_ver1) and a P4 (hw_ver3).
#if defined(RSA_SET_START_MODMULT_REG) // hw_ver3: ESP32-P4 and newer
#define DWS_RSA_MEM_M RSA_M_MEM
#define DWS_RSA_MEM_X RSA_X_MEM
#define DWS_RSA_MEM_Y RSA_Y_MEM
#define DWS_RSA_MEM_Z RSA_Z_MEM
#define DWS_RSA_MODE RSA_MODE_REG      // operand length in words, minus 1
#define DWS_RSA_MPRIME RSA_M_PRIME_REG // Montgomery m' (mod 2^32)
#define DWS_RSA_START RSA_SET_START_MODMULT_REG
#define DWS_RSA_DONE RSA_QUERY_IDLE_REG // reads 1 once the accelerator is idle (op complete)
#define DWS_RSA_INTCLR RSA_INT_CLR_REG
#define DWS_RSA_INTENA RSA_INT_ENA_REG
#elif defined(RSA_MOD_MULT_START_REG) // hw_ver1: ESP32-S3 / S2
#define DWS_RSA_MEM_M RSA_MEM_M_BLOCK_BASE
#define DWS_RSA_MEM_X RSA_MEM_X_BLOCK_BASE
#define DWS_RSA_MEM_Y RSA_MEM_Y_BLOCK_BASE
#define DWS_RSA_MEM_Z RSA_MEM_Z_BLOCK_BASE
#define DWS_RSA_MODE RSA_LENGTH_REG
#define DWS_RSA_MPRIME RSA_M_DASH_REG
#define DWS_RSA_START RSA_MOD_MULT_START_REG
#define DWS_RSA_DONE RSA_QUERY_INTERRUPT_REG // reads 1 once the op raises its completion bit
#define DWS_RSA_INTCLR RSA_CLEAR_INTERRUPT_REG
#define DWS_RSA_INTENA RSA_INTERRUPT_REG
#else
#error "DWS_FE25519_MPI_HW: no known RSA MODMULT register set for this target - add its die to the gate"
#endif

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
static inline void dws_fe_hw_enable(void)
{
    esp_mpi_enable_hardware_hw_op(); // lock + clock/power the peripheral (waits for its memory-init)
    SSH_RSA_REG(DWS_RSA_INTENA) = 0; // poll only, no completion IRQ
}
static inline void dws_fe_hw_disable(void)
{
    esp_mpi_disable_hardware_hw_op(); // release the lock + power down
}

// z = x*y mod p (8 words / 256-bit). Requires dws_fe_hw_enable() first. Output is always canonical (< p).
static inline void fe_mul(fe z, const fe x, const fe y) // safe if z aliases x/y
{
    volatile uint32_t *M = (volatile uint32_t *)DWS_RSA_MEM_M;
    volatile uint32_t *X = (volatile uint32_t *)DWS_RSA_MEM_X;
    volatile uint32_t *Y = (volatile uint32_t *)DWS_RSA_MEM_Y;
    volatile uint32_t *Z = (volatile uint32_t *)DWS_RSA_MEM_Z;
    SSH_RSA_REG(DWS_RSA_MODE) = 8 - 1; // mode = words - 1
    SSH_RSA_REG(DWS_RSA_MPRIME) = FE_MOD_MPRIME;
    for (int i = 0; i < 8; i++)
    {
        M[i] = FE_MOD_P[i];
        X[i] = x[i];
        Y[i] = y[i];
        Z[i] = FE_MOD_R2[i]; // Rinv = R^2 mod p in the result block -> plain (non-Montgomery) output
    }
    SSH_RSA_REG(DWS_RSA_INTCLR) = 1; // clear any stale done flag before starting
    SSH_RSA_REG(DWS_RSA_START) = 1;
    while (SSH_RSA_REG(DWS_RSA_DONE) == 0) // wait until the done/idle bit reads 1
        ;
    SSH_RSA_REG(DWS_RSA_INTCLR) = 1;
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
#endif // DETERMINISTICESPASYNCWEBSERVER_CRYPTO_FE25519_H
