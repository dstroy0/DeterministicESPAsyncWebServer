// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_bignum.cpp
 * @brief 2048-bit Montgomery modular exponentiation for DH-group14.
 *
 * ─── Montgomery parameter for group-14 ───────────────────────────────────
 * The group-14 prime p ends in ...FFFFFFFF FFFFFFFF (little-endian d[0]=d[1]=
 * 0xFFFFFFFF).  The Montgomery parameter:
 *
 *   p_inv = (-(p mod 2^32))^(-1) mod 2^32
 *
 * p mod 2^32 = 0xFFFFFFFF
 * -(0xFFFFFFFF) mod 2^32 = 0x00000001
 * 0x00000001^(-1) mod 2^32 = 1
 *
 * So p_inv = 1 for the group-14 prime.
 * In the SOS reduction pass: m_i = t[i] * p_inv mod 2^32 = t[i].
 *
 * ─── R² mod p ────────────────────────────────────────────────────────────
 * R = 2^2048.  R² mod p = 2^4096 mod p.
 * It is computed once at startup by bn_init() via 4096 doublings mod p,
 * and stored in the static s_g14.r2 constant.
 * ─────────────────────────────────────────────────────────────────────────
 */

#include "network_drivers/presentation/ssh/crypto/ssh_bignum.h"
#include "network_drivers/presentation/ssh/transport/ssh_keymat.h" // for ssh_wipe()
#include <string.h>

// ---------------------------------------------------------------------------
// Scratch buffer (SSH_CRYPTO_WORK_SIZE bytes, zeroed after each crypto op)
// ---------------------------------------------------------------------------

uint8_t crypto_work[SSH_CRYPTO_WORK_SIZE];

// ---------------------------------------------------------------------------
// Group-14 prime and generator (RFC 3526, §3)
// Little-endian 32-bit limbs: d[0] = least significant.
// ---------------------------------------------------------------------------

const SshBigNum group14_p = {{
    // 2048-bit MODP group-14 prime
    0xFFFFFFFFu, 0xFFFFFFFFu, 0x8AACaa68u, 0x15728E5Au, 0x98FA0510u, 0x15D22618u, 0xEA956AE5u, 0x3995497Cu,
    0x95581718u, 0xDE2BCBF6u, 0x6F4C52C9u, 0xB5C55DF0u, 0xEC07A28Fu, 0x9B2783A2u, 0x180E8603u, 0xE39E772Cu,
    0x2E36CE3Bu, 0x32905E46u, 0xCA18217Cu, 0xF1746C08u, 0x4ABC9804u, 0x670C354Eu, 0x7096966Du, 0x9ED52907u,
    0x208552BBu, 0x1C62F356u, 0xDCA3AD96u, 0x83655D23u, 0xFD24CF5Fu, 0x69163FA8u, 0x1C55D39Au, 0x98DA4836u,
    0xA163BF05u, 0xC2007CB8u, 0xECE45B3Du, 0x49286651u, 0x7C4B1FE6u, 0xAE9F2411u, 0x5A899FA5u, 0xEE386BFBu,
    0xF406B7EDu, 0x0BFF5CB6u, 0xA637ED6Bu, 0xF44C42E9u, 0x625E7EC6u, 0xE485B576u, 0x6D51C245u, 0x4FE1356Du,
    0xF25F1437u, 0x302B0A6Du, 0xCD3A431Bu, 0xEF9519B3u, 0x8E3404DDu, 0x514A0879u, 0x3B139B22u, 0x020BBEa6u,
    0x8A67CC74u, 0x29024E08u, 0x80DC1CD1u, 0xC4C6628Bu, 0x2168C234u, 0xC90FDAA2u, 0xFFFFFFFFu, 0xFFFFFFFFu,
}};

const SshBigNum group14_g = {{
    2u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
    0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
    0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
}};

// Software Montgomery state and helpers below are used ONLY by the native
// software modexp; on ARDUINO bn_expmod_group14() delegates to mbedtls (HW),
// so this (data-dependent, non-constant-time) machinery must never be compiled
// into firmware.  Guarded out on ARDUINO accordingly (SECURITY.md §timing).
#ifndef ARDUINO
// Group14 Montgomery constants, owned by one instance (internal linkage): R mod p, R^2 mod p,
// and the init flag (all filled by bn_init()). One named owner, unreachable cross-TU.
struct Group14Ctx
{
    SshBigNum r1; // R mod p = 2^2048 - p (two's complement of p in 2048 bits)
    SshBigNum r2; // R^2 mod p = 2^4096 mod p (bn_init() via repeated doubling)
    bool initialized = false;
};
static Group14Ctx s_g14;
#endif // !ARDUINO

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

static int bn_cmp_raw(const uint32_t *a, const uint32_t *b, int n)
{
    for (int i = n - 1; i >= 0; i--)
    {
        if (a[i] < b[i])
            return -1;
        if (a[i] > b[i])
            return 1;
    }
    return 0;
}

#ifndef ARDUINO // native-only Montgomery helpers (see guard note above)
// Subtract b from a in place (a -= b).  Assumes a >= b.  Both are n limbs.
static void bn_sub_inplace(uint32_t *a, const uint32_t *b, int n)
{
    uint64_t borrow = 0;
    for (int i = 0; i < n; i++)
    {
        uint64_t v = (uint64_t)a[i] - b[i] - borrow;
        a[i] = (uint32_t)v;
        borrow = (v >> 32) & 1u;
    }
}

// Left-shift n-limb value by 1 bit.  Returns the shifted-out MSB.
static uint32_t bn_shl1(uint32_t *a, int n)
{
    uint32_t carry = 0;
    for (int i = 0; i < n; i++)
    {
        uint32_t nc = a[i] >> 31;
        a[i] = (a[i] << 1) | carry;
        carry = nc;
    }
    return carry;
}

// ---------------------------------------------------------------------------
// Montgomery initialization (compute R mod p and R^2 mod p for group-14)
// ---------------------------------------------------------------------------

static void bn_init(void)
{
    if (s_g14.initialized)
        return;

    // R mod p = 2^2048 mod p = 2^2048 - p
    // (Since 2^2047 <= p < 2^2048, R mod p = 2^2048 - p which is positive and < p.)
    // Compute via borrow subtraction: 0 - p with 2048-bit wrap.
    {
        uint64_t borrow = 0;
        for (int i = 0; i < SSH_BN_LIMBS; i++)
        {
            uint64_t v = (uint64_t)0 - group14_p.d[i] - borrow;
            s_g14.r1.d[i] = (uint32_t)v;
            borrow = (v >> 32) & 1u;
        }
        // borrow == 1 here (expected - we wrapped around 2^2048), which is
        // the carry that represents the implicit 2^2048 in R mod p. Correct.
    }

    // R^2 mod p = 2^4096 mod p.
    // Compute by starting from R mod p and doubling it 2048 times mod p.
    memcpy(s_g14.r2.d, s_g14.r1.d, sizeof(SshBigNum));
    for (int i = 0; i < 2048; i++)
    {
        uint32_t overflow = bn_shl1(s_g14.r2.d, SSH_BN_LIMBS);
        // If overflow bit set OR result >= p, subtract p.
        if (overflow || bn_cmp_raw(s_g14.r2.d, group14_p.d, SSH_BN_LIMBS) >= 0)
            bn_sub_inplace(s_g14.r2.d, group14_p.d, SSH_BN_LIMBS);
    }

    s_g14.initialized = true;
}

// ---------------------------------------------------------------------------
// Montgomery SOS multiplication: out = a * b * R^-1 mod p
// Requires: 0 <= a, b < p.
// Uses crypto_work[768..1283] as the 129-limb temporary t[].
// p_inv = 1 for group-14 (see file header).
// ---------------------------------------------------------------------------

static void bn_monpro(SshBigNum *out, const SshBigNum *a, const SshBigNum *b)
{
    // t lives at offset 768 in crypto_work (after base_mont, result, tmp).
    uint32_t *t = (uint32_t *)(crypto_work + 768);
    memset(t, 0, 129 * sizeof(uint32_t));

    for (int i = 0; i < SSH_BN_LIMBS; i++)
    {
        // Multiply step: t[0..63] += a[i] * b[0..63]
        uint64_t carry = 0;
        for (int j = 0; j < SSH_BN_LIMBS; j++)
        {
            uint64_t uv = (uint64_t)t[i + j] + (uint64_t)a->d[i] * (uint64_t)b->d[j] + carry;
            t[i + j] = (uint32_t)uv;
            carry = uv >> 32;
        }
        t[i + SSH_BN_LIMBS] += (uint32_t)carry;

        // Reduction step: m = t[i] * p_inv = t[i] * 1 = t[i]
        uint32_t m = t[i];
        carry = 0;
        for (int j = 0; j < SSH_BN_LIMBS; j++)
        {
            uint64_t uv = (uint64_t)t[i + j] + (uint64_t)m * (uint64_t)group14_p.d[j] + carry;
            t[i + j] = (uint32_t)uv;
            carry = uv >> 32;
        }
        // Add carry into the high word (t[i+64]); t[128] absorbs final overflow.
        uint64_t hi = (uint64_t)t[i + SSH_BN_LIMBS] + carry;
        t[i + SSH_BN_LIMBS] = (uint32_t)hi;
        t[i + SSH_BN_LIMBS + 1] += (uint32_t)(hi >> 32);
    }

    // Result is in t[64..127].  Conditionally subtract p if result >= p.
    uint32_t *res = t + SSH_BN_LIMBS;
    if (t[128] || bn_cmp_raw(res, group14_p.d, SSH_BN_LIMBS) >= 0)
        bn_sub_inplace(res, group14_p.d, SSH_BN_LIMBS);

    memcpy(out->d, res, SSH_BN_LIMBS * sizeof(uint32_t));
}
#endif // !ARDUINO (native-only Montgomery helpers)

// ---------------------------------------------------------------------------
// Public API (bn_from_bytes / bn_to_bytes / bn_cmp / ... shared both platforms)
// ---------------------------------------------------------------------------

void bn_from_bytes(SshBigNum *out, const uint8_t *bytes, size_t len)
{
    memset(out->d, 0, sizeof(SshBigNum));
    // bytes are big-endian; map to little-endian limbs.
    size_t blen = len < 256 ? len : 256;
    for (size_t i = 0; i < blen; i++)
    {
        size_t byte_pos = i; // byte i from LSB (bytes[len-1-i] is the i-th byte from the end)
        out->d[byte_pos / 4] |= (uint32_t)bytes[len - 1 - i] << ((byte_pos % 4) * 8);
    }
}

void bn_to_bytes(uint8_t bytes[256], const SshBigNum *in)
{
    for (int i = 0; i < SSH_BN_LIMBS; i++)
    {
        uint32_t v = in->d[SSH_BN_LIMBS - 1 - i];
        bytes[i * 4 + 0] = (uint8_t)(v >> 24);
        bytes[i * 4 + 1] = (uint8_t)(v >> 16);
        bytes[i * 4 + 2] = (uint8_t)(v >> 8);
        bytes[i * 4 + 3] = (uint8_t)(v);
    }
}

int bn_cmp(const SshBigNum *a, const SshBigNum *b)
{
    return bn_cmp_raw(a->d, b->d, SSH_BN_LIMBS);
}

int bn_is_zero(const SshBigNum *a)
{
    for (int i = 0; i < SSH_BN_LIMBS; i++)
        if (a->d[i])
            return 0;
    return 1;
}

int bn_dh_validate(const SshBigNum *v)
{
    // Must be > 1
    int ok = 0;
    for (int i = 1; i < SSH_BN_LIMBS; i++)
        if (v->d[i])
        {
            ok = 1;
            break;
        }
    if (!ok && v->d[0] <= 1u)
        return -1;
    // Must be < p-1
    // p-1: subtract 1 from p
    SshBigNum pm1 = group14_p;
    pm1.d[0]--;
    if (bn_cmp(v, &pm1) >= 0)
        return -1;
    return 0;
}

// ---------------------------------------------------------------------------
// Modular exponentiation: out = base^exp mod group14_p
// ---------------------------------------------------------------------------

#ifdef ARDUINO
// On ESP32 delegate to mbedtls which uses HW bignum acceleration.
#include <mbedtls/bignum.h>

void bn_expmod_group14(SshBigNum *out, const SshBigNum *base, const SshBigNum *exp)
{
    uint8_t base_be[256];
    uint8_t exp_be[256];
    uint8_t p_be[256];
    uint8_t res_be[256];
    bn_to_bytes(base_be, base);
    bn_to_bytes(exp_be, exp);
    bn_to_bytes(p_be, &group14_p);

    mbedtls_mpi B;
    mbedtls_mpi E;
    mbedtls_mpi P;
    mbedtls_mpi R;
    mbedtls_mpi_init(&B);
    mbedtls_mpi_init(&E);
    mbedtls_mpi_init(&P);
    mbedtls_mpi_init(&R);

    mbedtls_mpi_read_binary(&B, base_be, 256);
    mbedtls_mpi_read_binary(&E, exp_be, 256);
    mbedtls_mpi_read_binary(&P, p_be, 256);

    mbedtls_mpi_exp_mod(&R, &B, &E, &P, NULL);
    mbedtls_mpi_write_binary(&R, res_be, 256);

    mbedtls_mpi_free(&B);
    mbedtls_mpi_free(&E);
    mbedtls_mpi_free(&P);
    mbedtls_mpi_free(&R);

    bn_from_bytes(out, res_be, 256);
    // Wipe the big-endian temporaries (they held the private exponent).
    ssh_wipe(exp_be, 256);
    ssh_wipe(res_be, 256);
}

#else // Native software path

void bn_expmod_group14(SshBigNum *out, const SshBigNum *base, const SshBigNum *exp)
{
    bn_init(); // ensure s_g14.r1, s_g14.r2 are computed

    // Map crypto_work regions (layout documented in ssh_bignum.h):
    SshBigNum *base_mont = (SshBigNum *)(crypto_work + 0);
    SshBigNum *result = (SshBigNum *)(crypto_work + 256);
    SshBigNum *tmp = (SshBigNum *)(crypto_work + 512);
    // t[] at offset 768 is used inside bn_monpro().

    // Convert base to Montgomery form: base_mont = base * R mod p
    //   = MonPro(base, R^2 mod p)
    bn_monpro(base_mont, base, &s_g14.r2);

    // result = 1 in Montgomery form = R mod p = s_g14.r1
    memcpy(result->d, s_g14.r1.d, sizeof(SshBigNum));

    // Left-to-right binary square-and-multiply (MSB first: d[63]..d[0], bit 31..0)
    for (int i = SSH_BN_LIMBS - 1; i >= 0; i--)
    {
        for (int b = 31; b >= 0; b--)
        {
            // Square: result = result^2 * R^-1 mod p
            bn_monpro(tmp, result, result);
            memcpy(result->d, tmp->d, sizeof(SshBigNum));

            if ((exp->d[i] >> b) & 1u)
            {
                // Multiply: result = result * base_mont * R^-1 mod p
                bn_monpro(tmp, result, base_mont);
                memcpy(result->d, tmp->d, sizeof(SshBigNum));
            }
        }
    }

    // Convert back from Montgomery form: out = result * R^-1 mod p
    //   = MonPro(result, 1)
    SshBigNum one;
    memset(one.d, 0, sizeof(SshBigNum));
    one.d[0] = 1u;
    bn_monpro(out, result, &one);

    // Wipe all temporaries in crypto_work (they contained DH private key fragments).
    ssh_wipe(crypto_work, SSH_CRYPTO_WORK_SIZE);
}

#endif // ARDUINO
