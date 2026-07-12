// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_curve25519.cpp
 * @brief Curve25519 field arithmetic (GF(2^255-19), radix-2^16) + X25519 (RFC 7748).
 *
 * The field element is sixteen int64 limbs of ~16 bits each; a full 16x16 schoolbook
 * product fits in int64 (no 128-bit type, so it builds on 32-bit xtensa). Reduction
 * folds anything at or above 2^256 back as *38 (2^256 = 2*2^255 = 2*19 mod p). The
 * X25519 scalar multiplication is the RFC 7748 §5 Montgomery ladder with constant-time
 * conditional swaps. Validated against the RFC 7748 §5.2 vectors (test_ssh_ed25519).
 *
 * On the ESP32-S3 (Arduino) X25519 has a second, byte-identical implementation that runs the
 * ladder in canonical uint32[8] and does each field multiply as one 256-bit modular multiply on
 * the RSA/MPI accelerator (~4.3x the software/PIE ladder); see the DETWS_X25519_MPI_HW block. It
 * shares the accelerator lock with mbedtls, so it is bracketed by esp_mpi_{enable,disable}_hardware_hw_op().
 */

#include "network_drivers/presentation/ssh/crypto/ssh_curve25519.h"
#ifdef ARDUINO
#include "sdkconfig.h"      // CONFIG_IDF_TARGET_ESP32S3 - selects the vector (PIE) field multiply
#include <mbedtls/bignum.h> // ESP32: field inversion on the MPI/RSA hardware accelerator
// On the S3, X25519 runs its whole Montgomery ladder in canonical uint32[8] and does each field multiply as
// one 256-bit modular multiply on the RSA/MPI accelerator (~4.3x the software/PIE ladder). Gated to Arduino
// because it shares the peripheral (and its lock) with mbedtls via esp_mpi_{enable,disable}_hardware_hw_op().
#if defined(CONFIG_IDF_TARGET_ESP32S3) && CONFIG_IDF_TARGET_ESP32S3
#define DETWS_X25519_MPI_HW 1
#include "soc/hwcrypto_reg.h" // RSA/MPI accelerator register map (MODMULT)
#include "soc/soc.h"          // DR_REG_RSA_BASE
#endif
#endif

// Small field constant (radix-2^16). Used only by the software X25519 ladder (the S3 MODMULT path carries its
// own canonical a24), so it would be unused there.
#ifndef DETWS_X25519_MPI_HW
static const ssh_gf GF_121665 = {0xDB41, 1}; // 121665 = 0x1DB41 (Montgomery a24)
#endif

// Normalize each limb toward 16 bits, folding the carry above 2^256 back in as *38.
// Two passes fully reduce a product's limbs; the +2^16 / -1 dance keeps it branch-free.
static void gf_carry(ssh_gf o)
{
    for (int i = 0; i < 16; i++)
    {
        o[i] += (int64_t)1 << 16;
        int64_t c = o[i] >> 16;
        o[(i + 1) * (i < 15)] += c - 1 + 37 * (c - 1) * (i == 15); // wrap: limb 15's carry *38 into limb 0
        o[i] -= c << 16;
    }
}

void ssh_gf_copy(ssh_gf out, const ssh_gf in)
{
    for (int i = 0; i < 16; i++)
        out[i] = in[i];
}

void ssh_gf_add(ssh_gf out, const ssh_gf a, const ssh_gf b)
{
    for (int i = 0; i < 16; i++)
        out[i] = a[i] + b[i];
}

void ssh_gf_sub(ssh_gf out, const ssh_gf a, const ssh_gf b)
{
    for (int i = 0; i < 16; i++)
        out[i] = a[i] - b[i];
}

#if defined(CONFIG_IDF_TARGET_ESP32S3) && CONFIG_IDF_TARGET_ESP32S3
// ---- ESP32-S3 vector (PIE) field multiply --------------------------------------------------------
// The S3 vector unit multiply-accumulates signed-16-bit lanes into a 40-bit accumulator
// (ee.vmulas.s16.accx), but radix-2^16 limbs run to ~2^18 and go negative after a subtraction, so they
// do not fit s16. Fix: balance each limb into signed-16-bit (a value-preserving carry redistribution
// mod p), after which a*b is a pure s16xs16 convolution - exactly the vector MAC. Device-validated
// byte-exact vs the scalar path (rig test, and test_gf_mul_s16_model_matches_scalar for the model);
// ~1.55x the scalar multiply on hardware.

// Balance a[16] into signed-16-bit limbs of the same value mod p (round-to-nearest carry; limb-15
// overflow wraps *38 into limb 0 since 2^256 == 38; three passes settle the wrap).
static void gf_balance_s16(int16_t o[16], const ssh_gf a)
{
    // int32 throughout: limbs stay ~+-2^18 and carries ~+-2, so no value exceeds int32 - which avoids the
    // emulated 64-bit carry-propagation math (48 steps per operand) that dominated the field multiply.
    int32_t c[16];
    for (int i = 0; i < 16; i++)
        c[i] = (int32_t)a[i];
    for (int pass = 0; pass < 3; pass++)
    {
        int32_t carry = 0;
        for (int i = 0; i < 16; i++)
        {
            int32_t v = c[i] + carry;
            carry = (v + 0x8000) >> 16;
            c[i] = v - (carry << 16);
        }
        c[0] += 38 * carry;
    }
    for (int i = 0; i < 16; i++)
        o[i] = (int16_t)c[i];
}

// One output limb t[k] = as[0..15] . window[0..15] on the ACCX. as is 16-byte aligned; the reversed-b
// window w may be unaligned (loaded via ee.ld.128.usar + ee.src.q). ACCX is 40-bit: sign-extend bit 39.
static inline int64_t gf_accx_dot_win(const int16_t *as, const int16_t *w)
{
    uint32_t lo, hi;
    const int16_t *pa = as, *pw = w;
    asm volatile("ee.zero.accx\n"
                 "ee.vld.128.ip q3, %[a], 16\n"
                 "ee.ld.128.usar.ip q0, %[w], 16\n"
                 "ee.ld.128.usar.ip q1, %[w], 16\n"
                 "ee.src.q q2, q0, q1\n"
                 "ee.vmulas.s16.accx q3, q2\n"
                 "ee.vld.128.ip q3, %[a], 16\n"
                 "ee.ld.128.usar.ip q0, %[w], 16\n"
                 "ee.src.q q2, q1, q0\n"
                 "ee.vmulas.s16.accx q3, q2\n"
                 "rur.accx_0 %[lo]\n"
                 "rur.accx_1 %[hi]\n"
                 : [lo] "=&r"(lo), [hi] "=&r"(hi), [a] "+r"(pa), [w] "+r"(pw)
                 :
                 : "memory");
    uint64_t raw = (uint64_t)lo | ((uint64_t)(uint8_t)hi << 32);
    return ((int64_t)(raw << 24)) >> 24;
}

// Shared tail: build the reversed-bs window array, run the ACCX convolution, fold + carry. as points at
// a 16-byte-aligned int16[16]; bs is read scalar-only (no alignment needed).
static void gf_conv_finish(ssh_gf out, const int16_t *as, const int16_t *bs)
{
    // bp = [15 zeros][bs reversed: bs15..bs0][zeros]; output k's window starts at bp[30-k].
    __attribute__((aligned(16))) int16_t bp[64];
    for (int i = 0; i < 64; i++)
        bp[i] = 0;
    for (int m = 0; m < 16; m++)
        bp[15 + m] = bs[15 - m];
    int64_t t[31];
    for (int k = 0; k < 31; k++)
        t[k] = gf_accx_dot_win(as, bp + (30 - k));
    for (int i = 0; i < 15; i++)
        t[i] += 38 * t[i + 16];
    for (int i = 0; i < 16; i++)
        out[i] = t[i];
    gf_carry(out);
    gf_carry(out);
}

void ssh_gf_mul(ssh_gf out, const ssh_gf a, const ssh_gf b)
{
    __attribute__((aligned(16))) int16_t as[16];
    int16_t bs[16];
    gf_balance_s16(as, a);
    gf_balance_s16(bs, b);
    gf_conv_finish(out, as, bs);
}

// Squaring balances the operand ONCE (a == b, and gf_balance_s16 is deterministic, so the second balance
// in mul(a,a) is pure waste). ~2/3 of the Montgomery-ladder field ops are squarings, so this matters.
// Byte-exact with ssh_gf_mul(out, a, a) by construction.
void ssh_gf_sq(ssh_gf out, const ssh_gf a)
{
    __attribute__((aligned(16))) int16_t as[16];
    gf_balance_s16(as, a);
    gf_conv_finish(out, as, as);
}
#else
void ssh_gf_mul(ssh_gf out, const ssh_gf a, const ssh_gf b)
{
    int64_t t[31];
    for (int i = 0; i < 31; i++)
        t[i] = 0;
    // Every limb fits in int32 (~16-18 bits after carry / add / sub), but a[i] and b[j] are int64, so a
    // plain a[i]*b[j] compiles to a full emulated 64x64 multiply on the 32-bit xtensa core. Casting both
    // operands to int32 makes gcc emit a single widening 32x32->64 multiply (mull+mulsh) instead - the
    // products (~34 bits) and their sums (~38 bits) still accumulate in the int64 t[].
    for (int i = 0; i < 16; i++)
    {
        int32_t ai = (int32_t)a[i];
        for (int j = 0; j < 16; j++)
            t[i + j] += (int64_t)ai * (int32_t)b[j];
    }
    for (int i = 0; i < 15; i++)
        t[i] += 38 * t[i + 16]; // fold the upper half (weight >= 2^256) down
    for (int i = 0; i < 16; i++)
        out[i] = t[i];
    gf_carry(out);
    gf_carry(out);
}
#endif

#if !(defined(CONFIG_IDF_TARGET_ESP32S3) && CONFIG_IDF_TARGET_ESP32S3)
void ssh_gf_sq(ssh_gf out, const ssh_gf a)
{
    ssh_gf_mul(out, a, a);
}
#endif

// Software field inversion out = a^-1 = a^(p-2). Fixed addition chain: square 255 times,
// multiplying in a at every bit except positions 2 and 4 (which are 0 in p-2 = 2^255 - 21).
// The reference path (native builds) and the fallback if the hardware modexp ever fails.
static void gf_inv_sw(ssh_gf out, const ssh_gf a)
{
    ssh_gf c;
    ssh_gf_copy(c, a);
    for (int i = 253; i >= 0; i--)
    {
        ssh_gf_sq(c, c);
        if (i != 2 && i != 4)
            ssh_gf_mul(c, c, a);
    }
    ssh_gf_copy(out, c);
}

#ifdef ARDUINO
// p = 2^255 - 19 and the inversion exponent p-2 = 2^255 - 21, big-endian for mbedtls.
static const uint8_t P25519_BE[32] = {0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xed};
static const uint8_t P25519_MINUS2_BE[32] = {0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                             0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                             0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xeb};

// out = a^(p-2) mod p on the ESP32 MPI/RSA hardware accelerator - the same modexp engine
// that runs RSA sign and DH-group14. mbedtls_mpi_exp_mod takes an arbitrary modulus, so
// 2^255-19 runs on the accelerator. Only the inversion is offloaded (one big modexp);
// the 255-round Montgomery ladder multiply stays in the software radix-2^16 core, where
// per-multiply marshalling to the peripheral would cost more than it saves. The exponent
// is a public constant; the base is packed to its canonical residue first.
void ssh_gf_inv(ssh_gf out, const ssh_gf a)
{
    uint8_t le[32];
    uint8_t be[32];
    ssh_gf_pack(le, a); // canonical little-endian residue in [0, p)
    for (int i = 0; i < 32; i++)
        be[i] = le[31 - i]; // to big-endian for mbedtls

    mbedtls_mpi A;
    mbedtls_mpi E;
    mbedtls_mpi N;
    mbedtls_mpi X;
    mbedtls_mpi_init(&A);
    mbedtls_mpi_init(&E);
    mbedtls_mpi_init(&N);
    mbedtls_mpi_init(&X);
    bool ok = mbedtls_mpi_read_binary(&A, be, 32) == 0 && mbedtls_mpi_read_binary(&E, P25519_MINUS2_BE, 32) == 0 &&
              mbedtls_mpi_read_binary(&N, P25519_BE, 32) == 0 && mbedtls_mpi_exp_mod(&X, &A, &E, &N, nullptr) == 0 &&
              mbedtls_mpi_write_binary(&X, be, 32) == 0;
    mbedtls_mpi_free(&A);
    mbedtls_mpi_free(&E);
    mbedtls_mpi_free(&N);
    mbedtls_mpi_free(&X);
    if (!ok)
    {
        gf_inv_sw(out, a); // never expected; keep correctness if the peripheral path fails
        return;
    }
    for (int i = 0; i < 32; i++)
        le[i] = be[31 - i];
    ssh_gf_unpack(out, le);
}
#else
void ssh_gf_inv(ssh_gf out, const ssh_gf a)
{
    gf_inv_sw(out, a);
}
#endif

// Constant-time conditional swap of p and q when b == 1 (b must be 0 or 1).
void ssh_gf_cswap(ssh_gf p, ssh_gf q, int b)
{
    int64_t mask = ~((int64_t)b - 1); // all ones when b==1, zero when b==0
    for (int i = 0; i < 16; i++)
    {
        int64_t t = mask & (p[i] ^ q[i]);
        p[i] ^= t;
        q[i] ^= t;
    }
}

// Canonical little-endian encoding: fully reduce mod p (conditional subtract twice),
// then emit 16-bit limbs low byte first.
void ssh_gf_pack(uint8_t out[32], const ssh_gf a)
{
    ssh_gf t;
    ssh_gf m;
    ssh_gf_copy(t, a);
    gf_carry(t);
    gf_carry(t);
    gf_carry(t);
    for (int j = 0; j < 2; j++)
    {
        m[0] = t[0] - 0xffed;
        for (int i = 1; i < 15; i++)
        {
            m[i] = t[i] - 0xffff - ((m[i - 1] >> 16) & 1);
            m[i - 1] &= 0xffff;
        }
        m[15] = t[15] - 0x7fff - ((m[14] >> 16) & 1);
        int b = (int)((m[15] >> 16) & 1);
        m[14] &= 0xffff;
        ssh_gf_cswap(t, m, 1 - b); // keep the subtracted value only if it did not borrow
    }
    for (int i = 0; i < 16; i++)
    {
        out[2 * i] = (uint8_t)(t[i] & 0xff);
        out[2 * i + 1] = (uint8_t)(t[i] >> 8);
    }
}

// Decode 32 little-endian bytes into a field element; the top bit is masked off (255-bit).
void ssh_gf_unpack(ssh_gf out, const uint8_t in[32])
{
    for (int i = 0; i < 16; i++)
        out[i] = (int64_t)in[2 * i] + ((int64_t)in[2 * i + 1] << 8);
    out[15] &= 0x7fff;
}

#ifdef DETWS_X25519_MPI_HW
// ============================= ESP32-S3 X25519 on the RSA/MPI accelerator =================================
// Field elements are canonical uint32[8] (< p = 2^255-19) THROUGHOUT the ladder, so every field multiply is a
// single hardware 256-bit modular multiply and add/sub are native 32-bit with one conditional subtract of p.
// Only the byte<->fe conversion happens per scalar-mult (not per multiply, as a drop-in would). Byte-exact
// with the software radix-2^16 ladder, validated against the RFC 7748 §5.2 vectors on hardware.
//
// The accelerator (and its lock) are shared with mbedtls' RSA/DH; esp_mpi_enable_hardware_hw_op() takes that
// lock and powers/clocks the peripheral (its mpi_hal_enable_hardware_hw_op() clears the RSA-mem power-down and
// waits for the block to be ready). The ladder brackets itself with enable/disable and holds the lock for its
// whole run: a KEX is infrequent, and per-multiply lock/power toggling would cost far more than it saves.
extern "C"
{
    void esp_mpi_enable_hardware_hw_op(void);  // mbedtls port: acquire the MPI lock + clock/power the peripheral
    void esp_mpi_disable_hardware_hw_op(void); // release the lock + power down
}

#define RSA_REG(a) (*(volatile uint32_t *)(a))

// Constants for the 256-bit modular multiply mod p = 2^255-19 (scratchpad/montconst.py): Montgomery m' and
// R^2 mod p (= 38^2 = 1444 = 0x5a4). Preloading R^2 into the result (Z/RB) block makes the accelerator return
// a plain residue X*Y mod p rather than a Montgomery form - the esp_mpi_mul_mpi_mod convention.
static const uint32_t MOD_MPRIME = 0x286bca1bu;
static const uint32_t MOD_P[8] = {0xffffffedu, 0xffffffffu, 0xffffffffu, 0xffffffffu,
                                  0xffffffffu, 0xffffffffu, 0xffffffffu, 0x7fffffffu};
static const uint32_t MOD_R2[8] = {0x000005a4u, 0, 0, 0, 0, 0, 0, 0};

// z = x*y mod p (8 words / 256-bit). Requires esp_mpi_enable_hardware_hw_op() first (peripheral up, lock held).
// The result is always canonical (< p), so callers need no follow-up reduce.
static void mpi_modmul256(uint32_t z[8], const uint32_t x[8], const uint32_t y[8])
{
    volatile uint32_t *M = (volatile uint32_t *)RSA_MEM_M_BLOCK_BASE;
    volatile uint32_t *X = (volatile uint32_t *)RSA_MEM_X_BLOCK_BASE;
    volatile uint32_t *Y = (volatile uint32_t *)RSA_MEM_Y_BLOCK_BASE;
    volatile uint32_t *Z = (volatile uint32_t *)RSA_MEM_Z_BLOCK_BASE;
    RSA_REG(RSA_LENGTH_REG) = 8 - 1; // mode = words - 1
    RSA_REG(RSA_M_DASH_REG) = MOD_MPRIME;
    for (int i = 0; i < 8; i++)
    {
        M[i] = MOD_P[i];
        X[i] = x[i];
        Y[i] = y[i];
        Z[i] = MOD_R2[i]; // r = R^2 mod p in the result block -> plain (non-Montgomery) output
    }
    RSA_REG(RSA_CLEAR_INTERRUPT_REG) = 1; // clear any stale done flag before starting
    RSA_REG(RSA_MOD_MULT_START_REG) = 1;
    while (RSA_REG(RSA_QUERY_INTERRUPT_REG) == 0)
        ;
    RSA_REG(RSA_CLEAR_INTERRUPT_REG) = 1;
    for (int i = 0; i < 8; i++)
        z[i] = Z[i];
}

typedef uint32_t fe[8];
static const uint32_t FE_A24[8] = {121665u, 0, 0, 0, 0, 0, 0, 0}; // X25519 a24 = (486662-2)/4 (RFC 7748 §5)

static void fe_copy(fe o, const fe a)
{
    for (int i = 0; i < 8; i++)
        o[i] = a[i];
}
static void fe_0(fe o)
{
    for (int i = 0; i < 8; i++)
        o[i] = 0;
}
static void fe_1(fe o)
{
    o[0] = 1;
    for (int i = 1; i < 8; i++)
        o[i] = 0;
}
// If o >= p (o is in [p, 2p)), subtract p. Constant-time: the borrow out of o-p selects o or o-p.
static void fe_reduce_once(fe o)
{
    uint32_t t[8];
    int64_t b = 0;
    for (int i = 0; i < 8; i++)
    {
        b += (int64_t)o[i] - (int64_t)MOD_P[i];
        t[i] = (uint32_t)b;
        b >>= 32;
    }
    uint32_t keep = (uint32_t)b; // 0 if o>=p (take t=o-p), 0xffffffff if o<p (keep o)
    for (int i = 0; i < 8; i++)
        o[i] = (o[i] & keep) | (t[i] & ~keep);
}
static void fe_add(fe o, const fe x, const fe y) // x,y < p -> o = x+y mod p
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
static void fe_sub(fe o, const fe x, const fe y) // x,y < p -> o = x-y mod p
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
        c += (uint64_t)t[i] + (MOD_P[i] & borrow);
        o[i] = (uint32_t)c;
        c >>= 32;
    }
}
static void fe_mul(fe o, const fe x, const fe y) // o = x*y mod p (HW MODMULT; safe if o aliases x/y)
{
    mpi_modmul256(o, x, y);
}
static void fe_sq(fe o, const fe x)
{
    mpi_modmul256(o, x, x);
}
static void fe_cswap(fe x, fe y, uint32_t swap) // constant-time swap of x,y when swap==1
{
    uint32_t mask = (uint32_t)(-(int32_t)swap);
    for (int i = 0; i < 8; i++)
    {
        uint32_t t = mask & (x[i] ^ y[i]);
        x[i] ^= t;
        y[i] ^= t;
    }
}
static void fe_frombytes(fe o, const uint8_t b[32])
{
    for (int i = 0; i < 8; i++)
        o[i] = (uint32_t)b[4 * i] | ((uint32_t)b[4 * i + 1] << 8) | ((uint32_t)b[4 * i + 2] << 16) |
               ((uint32_t)b[4 * i + 3] << 24);
    o[7] &= 0x7fffffffu; // RFC 7748: mask bit 255 of u
    fe_reduce_once(o);   // the masked value can still be in [p, 2^255) -> canonicalize
}
static void fe_tobytes(uint8_t b[32], const fe a)
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
static void fe_invert(fe o, const fe a)
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

void ssh_x25519(uint8_t out[32], const uint8_t scalar[32], const uint8_t point[32])
{
    uint8_t e[32];
    for (int i = 0; i < 32; i++)
        e[i] = scalar[i];
    e[0] &= 248; // clamp the scalar (RFC 7748 §5)
    e[31] &= 127;
    e[31] |= 64;

    esp_mpi_enable_hardware_hw_op(); // lock + power the accelerator for the whole ladder
    RSA_REG(RSA_INTERRUPT_REG) = 0;  // poll only, no completion IRQ

    fe x1;
    fe x2;
    fe z2;
    fe x3;
    fe z3;
    fe A;
    fe AA;
    fe B;
    fe BB;
    fe E;
    fe C;
    fe D;
    fe DA;
    fe CB;
    fe t0;
    fe t1;
    fe_frombytes(x1, point);
    fe_1(x2);
    fe_0(z2);
    fe_copy(x3, x1);
    fe_1(z3);
    uint32_t swap = 0;

    // Montgomery ladder over the 255 scalar bits, high to low (RFC 7748 §5).
    for (int t = 254; t >= 0; t--)
    {
        uint32_t k_t = (e[t >> 3] >> (t & 7)) & 1;
        swap ^= k_t;
        fe_cswap(x2, x3, swap);
        fe_cswap(z2, z3, swap);
        swap = k_t;
        fe_add(A, x2, z2);
        fe_sq(AA, A);
        fe_sub(B, x2, z2);
        fe_sq(BB, B);
        fe_sub(E, AA, BB);
        fe_add(C, x3, z3);
        fe_sub(D, x3, z3);
        fe_mul(DA, D, A);
        fe_mul(CB, C, B);
        fe_add(t0, DA, CB);
        fe_sq(x3, t0);
        fe_sub(t1, DA, CB);
        fe_sq(t1, t1);
        fe_mul(z3, x1, t1);
        fe_mul(x2, AA, BB);
        fe_mul(t0, FE_A24, E);
        fe_add(t0, AA, t0);
        fe_mul(z2, E, t0);
    }
    fe_cswap(x2, x3, swap);
    fe_cswap(z2, z3, swap);

    // Result = X / Z = x2 * z2^-1.
    fe_invert(z2, z2);
    fe_mul(x2, x2, z2);
    fe_tobytes(out, x2);
    esp_mpi_disable_hardware_hw_op(); // release the lock + power down
}
#else
void ssh_x25519(uint8_t out[32], const uint8_t scalar[32], const uint8_t point[32])
{
    uint8_t z[32];
    for (int i = 0; i < 31; i++)
        z[i] = scalar[i];
    z[31] = (uint8_t)((scalar[31] & 127) | 64); // clamp the scalar (RFC 7748 §5)
    z[0] &= 248;

    ssh_gf x;
    ssh_gf a;
    ssh_gf b;
    ssh_gf c;
    ssh_gf d;
    ssh_gf e;
    ssh_gf f;
    ssh_gf_unpack(x, point);
    for (int i = 0; i < 16; i++)
    {
        b[i] = x[i];
        a[i] = c[i] = d[i] = 0;
    }
    a[0] = d[0] = 1;

    // Montgomery ladder over the 255 scalar bits, high to low.
    for (int i = 254; i >= 0; i--)
    {
        int r = (z[i >> 3] >> (i & 7)) & 1;
        ssh_gf_cswap(a, b, r);
        ssh_gf_cswap(c, d, r);
        ssh_gf_add(e, a, c);
        ssh_gf_sub(a, a, c);
        ssh_gf_add(c, b, d);
        ssh_gf_sub(b, b, d);
        ssh_gf_sq(d, e);
        ssh_gf_sq(f, a);
        ssh_gf_mul(a, c, a);
        ssh_gf_mul(c, b, e);
        ssh_gf_add(e, a, c);
        ssh_gf_sub(a, a, c);
        ssh_gf_sq(b, a);
        ssh_gf_sub(c, d, f);
        ssh_gf_mul(a, c, GF_121665);
        ssh_gf_add(a, a, d);
        ssh_gf_mul(c, c, a);
        ssh_gf_mul(a, d, f);
        ssh_gf_mul(d, b, x);
        ssh_gf_sq(b, e);
        ssh_gf_cswap(a, b, r);
        ssh_gf_cswap(c, d, r);
    }

    // Result = X / Z = a * c^-1.
    ssh_gf_inv(c, c);
    ssh_gf_mul(a, a, c);
    ssh_gf_pack(out, a);
}
#endif // DETWS_X25519_MPI_HW

void ssh_x25519_base(uint8_t out[32], const uint8_t scalar[32])
{
    uint8_t base[32] = {9};
    ssh_x25519(out, scalar, base);
}
