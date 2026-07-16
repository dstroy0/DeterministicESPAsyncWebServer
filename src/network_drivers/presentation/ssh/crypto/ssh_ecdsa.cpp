// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_ecdsa.cpp
 * @brief ECDSA over NIST P-256 for ecdsa-sha2-nistp256 (RFC 5656 / FIPS 186-4).
 */

#include "network_drivers/presentation/ssh/crypto/ssh_ecdsa.h"
#include "network_drivers/presentation/ssh/crypto/ssh_sha256.h"
#include <string.h>

// ---------------------------------------------------------------------------
// Arduino - mbedTLS path (production, hardware-accelerated, side-channel hardened)
// ---------------------------------------------------------------------------

#ifdef ARDUINO

#include <esp_random.h> // esp_fill_random() for the ECDSA nonce / blinding RNG
#include <mbedtls/ecdh.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/ecp.h>

namespace
{
// RNG callback backed by the ESP32 hardware RNG.
int ecdsa_rng(void *ctx, unsigned char *buf, size_t len)
{
    (void)ctx;
    esp_fill_random(buf, len);
    return 0;
}
} // namespace

bool ssh_ecdsa_p256_pubkey(uint8_t pub[SSH_ECDSA_P256_PUB_LEN], const uint8_t priv[SSH_ECDSA_P256_PRIV_LEN])
{
    mbedtls_ecp_group grp;
    mbedtls_ecp_point Q;
    mbedtls_mpi d;
    mbedtls_ecp_group_init(&grp);
    mbedtls_ecp_point_init(&Q);
    mbedtls_mpi_init(&d);

    bool ok = false;
    if (mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1) == 0 &&
        mbedtls_mpi_read_binary(&d, priv, SSH_ECDSA_P256_PRIV_LEN) == 0 &&
        mbedtls_ecp_mul(&grp, &Q, &d, &grp.G, ecdsa_rng, nullptr) == 0)
    {
        size_t olen = 0;
        if (mbedtls_ecp_point_write_binary(&grp, &Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &olen, pub, SSH_ECDSA_P256_PUB_LEN) ==
                0 &&
            olen == SSH_ECDSA_P256_PUB_LEN)
            ok = true;
    }

    mbedtls_mpi_free(&d);
    mbedtls_ecp_point_free(&Q);
    mbedtls_ecp_group_free(&grp);
    return ok;
}

bool ssh_ecdsa_p256_sign(uint8_t sig[SSH_ECDSA_P256_SIG_LEN], const uint8_t *msg, size_t mlen,
                         const uint8_t priv[SSH_ECDSA_P256_PRIV_LEN])
{
    uint8_t h[SSH_SHA256_DIGEST_LEN];
    ssh_sha256(msg, mlen, h);

    mbedtls_ecp_group grp;
    mbedtls_mpi d;
    mbedtls_mpi r;
    mbedtls_mpi s;
    mbedtls_ecp_group_init(&grp);
    mbedtls_mpi_init(&d);
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);

    bool ok = false;
    if (mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1) == 0 &&
        mbedtls_mpi_read_binary(&d, priv, SSH_ECDSA_P256_PRIV_LEN) == 0 &&
        mbedtls_ecdsa_sign(&grp, &r, &s, &d, h, SSH_SHA256_DIGEST_LEN, ecdsa_rng, nullptr) == 0 &&
        mbedtls_mpi_write_binary(&r, sig, SSH_ECDSA_P256_COORD_LEN) == 0 &&
        mbedtls_mpi_write_binary(&s, sig + SSH_ECDSA_P256_COORD_LEN, SSH_ECDSA_P256_COORD_LEN) == 0)
        ok = true;

    mbedtls_mpi_free(&s);
    mbedtls_mpi_free(&r);
    mbedtls_mpi_free(&d);
    mbedtls_ecp_group_free(&grp);
    return ok;
}

bool ssh_ecdsa_p256_verify(const uint8_t pub[SSH_ECDSA_P256_PUB_LEN], const uint8_t *msg, size_t mlen,
                           const uint8_t sig[SSH_ECDSA_P256_SIG_LEN])
{
    uint8_t h[SSH_SHA256_DIGEST_LEN];
    ssh_sha256(msg, mlen, h);

    mbedtls_ecp_group grp;
    mbedtls_ecp_point Q;
    mbedtls_mpi r;
    mbedtls_mpi s;
    mbedtls_ecp_group_init(&grp);
    mbedtls_ecp_point_init(&Q);
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);

    bool ok = false;
    if (mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1) == 0 &&
        mbedtls_ecp_point_read_binary(&grp, &Q, pub, SSH_ECDSA_P256_PUB_LEN) == 0 &&
        mbedtls_ecp_check_pubkey(&grp, &Q) == 0 && mbedtls_mpi_read_binary(&r, sig, SSH_ECDSA_P256_COORD_LEN) == 0 &&
        mbedtls_mpi_read_binary(&s, sig + SSH_ECDSA_P256_COORD_LEN, SSH_ECDSA_P256_COORD_LEN) == 0 &&
        mbedtls_ecdsa_verify(&grp, h, SSH_SHA256_DIGEST_LEN, &Q, &r, &s) == 0)
        ok = true;

    mbedtls_mpi_free(&s);
    mbedtls_mpi_free(&r);
    mbedtls_ecp_point_free(&Q);
    mbedtls_ecp_group_free(&grp);
    return ok;
}

bool ssh_ecdsa_p256_ecdh(uint8_t shared_x[SSH_ECDSA_P256_COORD_LEN], const uint8_t peer_pub[SSH_ECDSA_P256_PUB_LEN],
                         const uint8_t priv[SSH_ECDSA_P256_PRIV_LEN])
{
    mbedtls_ecp_group grp;
    mbedtls_ecp_point Q;
    mbedtls_mpi d;
    mbedtls_mpi z; // shared secret = the X coordinate of d*Q
    mbedtls_ecp_group_init(&grp);
    mbedtls_ecp_point_init(&Q);
    mbedtls_mpi_init(&d);
    mbedtls_mpi_init(&z);

    bool ok = false;
    if (mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1) == 0 &&
        mbedtls_ecp_point_read_binary(&grp, &Q, peer_pub, SSH_ECDSA_P256_PUB_LEN) == 0 &&
        mbedtls_ecp_check_pubkey(&grp, &Q) == 0 && mbedtls_mpi_read_binary(&d, priv, SSH_ECDSA_P256_PRIV_LEN) == 0 &&
        mbedtls_ecdh_compute_shared(&grp, &z, &Q, &d, ecdsa_rng, nullptr) == 0 &&
        mbedtls_mpi_write_binary(&z, shared_x, SSH_ECDSA_P256_COORD_LEN) == 0)
        ok = true;

    mbedtls_mpi_free(&z);
    mbedtls_mpi_free(&d);
    mbedtls_ecp_point_free(&Q);
    mbedtls_ecp_group_free(&grp);
    return ok;
}

// ---------------------------------------------------------------------------
// Native - self-contained software P-256 (test-only; not compiled into firmware)
// ---------------------------------------------------------------------------

#else

#include "network_drivers/presentation/ssh/crypto/ssh_hmac_sha256.h"

namespace
{
// ---- 256-bit little-endian field/scalar arithmetic ----
// Elements are 8 x uint32 limbs, limb 0 least-significant.

// P-256 domain parameters (little-endian words).
const uint32_t P256_P[8] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000,
                            0x00000000, 0x00000000, 0x00000001, 0xFFFFFFFF};
const uint32_t P256_N[8] = {0xFC632551, 0xF3B9CAC2, 0xA7179E84, 0xBCE6FAAD,
                            0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF};
const uint32_t P256_B[8] = {0x27D2604B, 0x3BCE3C3E, 0xCC53B0F6, 0x651D06B0,
                            0x769886BC, 0xB3EBBD55, 0xAA3A93E7, 0x5AC635D8};
const uint32_t P256_GX[8] = {0xD898C296, 0xF4A13945, 0x2DEB33A0, 0x77037D81,
                             0x63A440F2, 0xF8BCE6E5, 0xE12C4247, 0x6B17D1F2};
const uint32_t P256_GY[8] = {0x37BF51F5, 0xCBB64068, 0x6B315ECE, 0x2BCE3357,
                             0x7C0F9E16, 0x8EE7EB4A, 0xFE1A7F9B, 0x4FE342E2};

void fp_set(uint32_t r[8], const uint32_t a[8])
{
    for (int i = 0; i < 8; i++)
        r[i] = a[i];
}
void fp_zero(uint32_t r[8])
{
    for (int i = 0; i < 8; i++)
        r[i] = 0;
}
bool fp_is_zero(const uint32_t a[8])
{
    uint32_t x = 0;
    for (int i = 0; i < 8; i++)
        x |= a[i];
    return x == 0;
}
// -1 if a<b, 0 if a==b, 1 if a>b.
int fp_cmp(const uint32_t a[8], const uint32_t b[8])
{
    for (int i = 7; i >= 0; i--)
        if (a[i] != b[i])
            return a[i] > b[i] ? 1 : -1;
    return 0;
}
// r = a - b (mod 2^256); returns borrow.
uint32_t sub_raw(uint32_t r[8], const uint32_t a[8], const uint32_t b[8])
{
    uint64_t brw = 0;
    for (int i = 0; i < 8; i++)
    {
        uint64_t t = (uint64_t)a[i] - b[i] - brw;
        r[i] = (uint32_t)t;
        brw = (t >> 32) & 1u;
    }
    return (uint32_t)brw;
}
// r = a + b (mod m), inputs < m.
void fp_addm(uint32_t r[8], const uint32_t a[8], const uint32_t b[8], const uint32_t m[8])
{
    uint32_t s[9];
    uint64_t c = 0;
    for (int i = 0; i < 8; i++)
    {
        uint64_t t = (uint64_t)a[i] + b[i] + c;
        s[i] = (uint32_t)t;
        c = t >> 32;
    }
    s[8] = (uint32_t)c;
    // if s >= m (9-limb vs 8-limb m), subtract m.
    bool ge = s[8] != 0;
    if (!ge)
    {
        ge = true;
        for (int k = 7; k >= 0; k--)
            if (s[k] != m[k])
            {
                ge = s[k] > m[k];
                break;
            }
    }
    if (ge)
    {
        uint64_t brw = 0;
        for (int k = 0; k < 8; k++)
        {
            uint64_t t = (uint64_t)s[k] - m[k] - brw;
            s[k] = (uint32_t)t;
            brw = (t >> 32) & 1u;
        }
    }
    for (int k = 0; k < 8; k++)
        r[k] = s[k];
}
// r = a - b (mod m), inputs < m.
void fp_subm(uint32_t r[8], const uint32_t a[8], const uint32_t b[8], const uint32_t m[8])
{
    uint32_t t[8];
    if (sub_raw(t, a, b))
    {
        uint32_t c = 0; // borrow: add m back
        uint64_t cc = 0;
        for (int i = 0; i < 8; i++)
        {
            uint64_t v = (uint64_t)t[i] + m[i] + cc;
            t[i] = (uint32_t)v;
            cc = v >> 32;
        }
        (void)c;
    }
    fp_set(r, t);
}
// acc[0..7] >= m[0..7]? Compares the low 8 limbs from the most significant down.
bool reduce_low8_ge(const uint32_t acc[8], const uint32_t m[8])
{
    for (int k = 7; k >= 0; k--)
        if (acc[k] != m[k])
            return acc[k] > m[k];
    return true; // all limbs equal
}
// Reduce a 512-bit product mod m (bit-serial, MSB to LSB). Correct but slow; native is test-only.
void reduce_mod(uint32_t r[8], const uint32_t prod[16], const uint32_t m[8])
{
    uint32_t acc[9];
    for (int k = 0; k < 9; k++)
        acc[k] = 0;
    for (int bit = 511; bit >= 0; bit--)
    {
        uint32_t carry = 0;
        for (int k = 0; k < 9; k++)
        {
            uint32_t nc = acc[k] >> 31;
            acc[k] = (acc[k] << 1) | carry;
            carry = nc;
        }
        acc[0] |= (prod[bit >> 5] >> (bit & 31)) & 1u;
        bool ge = acc[8] != 0;
        if (!ge)
            ge = reduce_low8_ge(acc, m);
        if (ge)
        {
            uint64_t brw = 0;
            for (int k = 0; k < 8; k++)
            {
                uint64_t t = (uint64_t)acc[k] - m[k] - brw;
                acc[k] = (uint32_t)t;
                brw = (t >> 32) & 1u;
            }
            acc[8] -= (uint32_t)brw;
        }
    }
    for (int k = 0; k < 8; k++)
        r[k] = acc[k];
}
// r = (a * b) mod m.
void fp_mul(uint32_t r[8], const uint32_t a[8], const uint32_t b[8], const uint32_t m[8])
{
    uint32_t prod[16];
    for (int k = 0; k < 16; k++)
        prod[k] = 0;
    for (int i = 0; i < 8; i++)
    {
        uint64_t carry = 0;
        for (int j = 0; j < 8; j++)
        {
            uint64_t t = (uint64_t)prod[i + j] + (uint64_t)a[i] * b[j] + carry;
            prod[i + j] = (uint32_t)t;
            carry = t >> 32;
        }
        int k = i + 8;
        while (carry)
        {
            uint64_t t = (uint64_t)prod[k] + carry;
            prod[k] = (uint32_t)t;
            carry = t >> 32;
            k++;
        }
    }
    reduce_mod(r, prod, m);
}
void fp_sqr(uint32_t r[8], const uint32_t a[8], const uint32_t m[8])
{
    fp_mul(r, a, a, m);
}
// r = a^(m-2) mod m  (Fermat inverse; m prime).
void fp_inv(uint32_t r[8], const uint32_t a[8], const uint32_t m[8])
{
    const uint32_t two[8] = {2, 0, 0, 0, 0, 0, 0, 0};
    uint32_t e[8];
    sub_raw(e, m, two); // e = m - 2
    uint32_t res[8] = {1, 0, 0, 0, 0, 0, 0, 0};
    uint32_t base[8];
    fp_set(base, a);
    for (int i = 0; i < 256; i++)
    {
        if ((e[i >> 5] >> (i & 31)) & 1u)
            fp_mul(res, res, base, m);
        fp_mul(base, base, base, m);
    }
    fp_set(r, res);
}
// Reduce a single 256-bit value < 2m into [0, m).
void fp_reduce_once(uint32_t r[8], const uint32_t a[8], const uint32_t m[8])
{
    uint32_t t[8];
    if (sub_raw(t, a, m))
        fp_set(r, a);
    else
        fp_set(r, t);
}
void load_be(uint32_t r[8], const uint8_t b[32])
{
    for (int i = 0; i < 8; i++)
    {
        const uint8_t *p = b + (28 - 4 * i);
        r[i] = ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
    }
}
void store_be(uint8_t b[32], const uint32_t r[8])
{
    for (int i = 0; i < 8; i++)
    {
        uint8_t *p = b + (28 - 4 * i);
        p[0] = (uint8_t)(r[i] >> 24);
        p[1] = (uint8_t)(r[i] >> 16);
        p[2] = (uint8_t)(r[i] >> 8);
        p[3] = (uint8_t)r[i];
    }
}

// ---- Jacobian point arithmetic on y^2 = x^3 - 3x + b over F_p ----
struct Jac
{
    uint32_t X[8];
    uint32_t Y[8];
    uint32_t Z[8]; // point at infinity iff Z == 0
};

void fp_mul2(uint32_t r[8], const uint32_t a[8])
{
    fp_addm(r, a, a, P256_P);
}

// R = 2*P  (a = -3 optimized doubling; alias-safe).
void jac_double(Jac *R, const Jac *P)
{
    if (fp_is_zero(P->Z))
    {
        *R = *P;
        return;
    }
    uint32_t delta[8];
    uint32_t gamma[8];
    uint32_t beta[8];
    uint32_t alpha[8];
    uint32_t t0[8];
    uint32_t t1[8];
    fp_sqr(delta, P->Z, P256_P);       // delta = Z^2
    fp_sqr(gamma, P->Y, P256_P);       // gamma = Y^2
    fp_mul(beta, P->X, gamma, P256_P); // beta = X*gamma
    fp_subm(t0, P->X, delta, P256_P);  // X - delta
    fp_addm(t1, P->X, delta, P256_P);  // X + delta
    fp_mul(t0, t0, t1, P256_P);        // (X-delta)(X+delta)
    fp_addm(alpha, t0, t0, P256_P);
    fp_addm(alpha, alpha, t0, P256_P); // alpha = 3*(X-delta)(X+delta)

    uint32_t x3[8];
    uint32_t y3[8];
    uint32_t z3[8];
    uint32_t eight_beta[8];
    fp_mul2(eight_beta, beta);
    fp_mul2(eight_beta, eight_beta);
    fp_mul2(eight_beta, eight_beta);     // 8*beta
    fp_sqr(x3, alpha, P256_P);           // alpha^2
    fp_subm(x3, x3, eight_beta, P256_P); // X3 = alpha^2 - 8*beta

    fp_addm(z3, P->Y, P->Z, P256_P); // Y+Z
    fp_sqr(z3, z3, P256_P);          // (Y+Z)^2
    fp_subm(z3, z3, gamma, P256_P);
    fp_subm(z3, z3, delta, P256_P); // Z3 = (Y+Z)^2 - gamma - delta

    uint32_t four_beta[8];
    fp_mul2(four_beta, beta);
    fp_mul2(four_beta, four_beta);             // 4*beta
    fp_subm(four_beta, four_beta, x3, P256_P); // 4*beta - X3
    fp_mul(y3, alpha, four_beta, P256_P);      // alpha*(4*beta - X3)
    uint32_t g2[8];
    fp_sqr(g2, gamma, P256_P); // gamma^2
    fp_mul2(g2, g2);
    fp_mul2(g2, g2);
    fp_mul2(g2, g2);             // 8*gamma^2
    fp_subm(y3, y3, g2, P256_P); // Y3 = alpha*(4*beta - X3) - 8*gamma^2

    fp_set(R->X, x3);
    fp_set(R->Y, y3);
    fp_set(R->Z, z3);
}

// R = P + Q  (general Jacobian addition; alias-safe).
void jac_add(Jac *R, const Jac *P, const Jac *Q)
{
    if (fp_is_zero(P->Z))
    {
        *R = *Q;
        return;
    }
    if (fp_is_zero(Q->Z))
    {
        *R = *P;
        return;
    }
    uint32_t z1z1[8];
    uint32_t z2z2[8];
    uint32_t u1[8];
    uint32_t u2[8];
    uint32_t s1[8];
    uint32_t s2[8];
    uint32_t t[8];
    fp_sqr(z1z1, P->Z, P256_P);
    fp_sqr(z2z2, Q->Z, P256_P);
    fp_mul(u1, P->X, z2z2, P256_P); // U1 = X1*Z2^2
    fp_mul(u2, Q->X, z1z1, P256_P); // U2 = X2*Z1^2
    fp_mul(s1, P->Y, Q->Z, P256_P);
    fp_mul(s1, s1, z2z2, P256_P); // S1 = Y1*Z2^3
    fp_mul(s2, Q->Y, P->Z, P256_P);
    fp_mul(s2, s2, z1z1, P256_P); // S2 = Y2*Z1^3

    uint32_t h[8];
    uint32_t rr[8];
    fp_subm(h, u2, u1, P256_P);  // H = U2 - U1
    fp_subm(rr, s2, s1, P256_P); // S2 - S1
    if (fp_is_zero(h))
    {
        if (fp_is_zero(rr))
        {
            jac_double(R, P);
            return;
        }
        fp_zero(R->Z); // P == -Q -> infinity
        return;
    }
    fp_addm(rr, rr, rr, P256_P); // r = 2*(S2 - S1)

    uint32_t i[8];
    uint32_t j[8];
    uint32_t v[8];
    fp_mul2(i, h);
    fp_sqr(i, i, P256_P);     // I = (2H)^2
    fp_mul(j, h, i, P256_P);  // J = H*I
    fp_mul(v, u1, i, P256_P); // V = U1*I

    uint32_t x3[8];
    uint32_t y3[8];
    uint32_t z3[8];
    fp_sqr(x3, rr, P256_P); // r^2
    fp_subm(x3, x3, j, P256_P);
    fp_mul2(t, v);
    fp_subm(x3, x3, t, P256_P); // X3 = r^2 - J - 2V

    fp_subm(y3, v, x3, P256_P);
    fp_mul(y3, rr, y3, P256_P); // r*(V - X3)
    fp_mul(t, s1, j, P256_P);
    fp_mul2(t, t);              // 2*S1*J
    fp_subm(y3, y3, t, P256_P); // Y3 = r*(V - X3) - 2*S1*J

    fp_addm(z3, P->Z, Q->Z, P256_P);
    fp_sqr(z3, z3, P256_P);
    fp_subm(z3, z3, z1z1, P256_P);
    fp_subm(z3, z3, z2z2, P256_P);
    fp_mul(z3, z3, h, P256_P); // Z3 = ((Z1+Z2)^2 - Z1^2 - Z2^2)*H

    fp_set(R->X, x3);
    fp_set(R->Y, y3);
    fp_set(R->Z, z3);
}

// R = k*P  (double-and-add, MSB->LSB; not constant-time - native test path only).
void jac_scalar_mul(Jac *R, const uint32_t k[8], const Jac *P)
{
    Jac acc;
    fp_zero(acc.X);
    fp_zero(acc.Y);
    fp_zero(acc.Z); // infinity
    for (int bit = 255; bit >= 0; bit--)
    {
        jac_double(&acc, &acc);
        if ((k[bit >> 5] >> (bit & 31)) & 1u)
            jac_add(&acc, &acc, P);
    }
    *R = acc;
}

// Convert Jacobian to affine (x, y). Caller must ensure P is not the infinity point.
void jac_to_affine(uint32_t x[8], uint32_t y[8], const Jac *P)
{
    uint32_t zinv[8];
    uint32_t zinv2[8];
    uint32_t zinv3[8];
    fp_inv(zinv, P->Z, P256_P);
    fp_sqr(zinv2, zinv, P256_P);
    fp_mul(zinv3, zinv2, zinv, P256_P);
    fp_mul(x, P->X, zinv2, P256_P);
    fp_mul(y, P->Y, zinv3, P256_P);
}

void jac_set_affine(Jac *P, const uint32_t x[8], const uint32_t y[8])
{
    fp_set(P->X, x);
    fp_set(P->Y, y);
    fp_zero(P->Z);
    P->Z[0] = 1;
}

// Check (x, y) is on y^2 = x^3 - 3x + b (mod p).
bool on_curve(const uint32_t x[8], const uint32_t y[8])
{
    if (fp_cmp(x, P256_P) >= 0 || fp_cmp(y, P256_P) >= 0)
        return false;
    uint32_t lhs[8];
    uint32_t rhs[8];
    uint32_t t[8];
    fp_sqr(lhs, y, P256_P); // y^2
    fp_sqr(rhs, x, P256_P);
    fp_mul(rhs, rhs, x, P256_P); // x^3
    fp_addm(t, x, x, P256_P);
    fp_addm(t, t, x, P256_P);     // 3x
    fp_subm(rhs, rhs, t, P256_P); // x^3 - 3x
    fp_addm(rhs, rhs, P256_B, P256_P);
    return fp_cmp(lhs, rhs) == 0;
}

// ---- RFC 6979 deterministic nonce (HMAC-SHA256 DRBG, hlen = qlen = 256) ----

// out = HMAC-SHA256(key(32), a || tag_present?tag : nothing || extra1 || extra2).
void hmac_cat(uint8_t out[32], const uint8_t key[32], const uint8_t *v, size_t vlen, const int tag, const uint8_t *x,
              const uint8_t *e)
{
    uint8_t buf[97]; // 32 (V) + 1 (tag) + 32 (x) + 32 (e)
    size_t n = 0;
    memcpy(buf + n, v, vlen);
    n += vlen;
    if (tag >= 0)
    {
        buf[n++] = (uint8_t)tag;
        memcpy(buf + n, x, 32);
        n += 32;
        memcpy(buf + n, e, 32);
        n += 32;
    }
    ssh_hmac_sha256(key, 32, buf, n, out);
}

// One RFC 6979 candidate k: if it yields a valid r and s, write the 64-byte signature and return true.
bool ecdsa_try_sign(const uint32_t k[8], const Jac *G, const uint32_t d[8], const uint32_t e[8], uint8_t sig[64])
{
    if (fp_is_zero(k) || fp_cmp(k, P256_N) >= 0)
        return false;
    Jac R;
    jac_scalar_mul(&R, k, G);
    if (fp_is_zero(R.Z))
        return false;
    uint32_t rx[8];
    uint32_t ry[8];
    jac_to_affine(rx, ry, &R);
    uint32_t r[8];
    fp_reduce_once(r, rx, P256_N); // r = Rx mod n
    if (fp_is_zero(r))
        return false;
    uint32_t kinv[8];
    uint32_t s[8];
    fp_inv(kinv, k, P256_N);
    fp_mul(s, r, d, P256_N);    // r*d
    fp_addm(s, s, e, P256_N);   // e + r*d
    fp_mul(s, kinv, s, P256_N); // k^-1 (e + r*d)
    if (fp_is_zero(s))
        return false;
    store_be(sig, r);
    store_be(sig + 32, s);
    return true;
}

// ECDSA core: sign hash e_bytes (32) with scalar d, deterministic k per RFC 6979.
bool ecdsa_sign_core(uint8_t sig[64], const uint8_t h1[32], const uint32_t d[8])
{
    uint32_t e[8];
    uint32_t etmp[8];
    load_be(etmp, h1);
    fp_reduce_once(e, etmp, P256_N); // bits2int(h1) mod n

    uint8_t x_oct[32];
    uint8_t h_oct[32];
    store_be(x_oct, d);
    store_be(h_oct, e); // bits2octets(h1)

    uint8_t V[32];
    uint8_t K[32];
    memset(V, 0x01, 32);
    memset(K, 0x00, 32);
    // RFC 6979 first pass: rekey K from V, tag byte 0x00, x_oct and h_oct, then refresh V from K.
    hmac_cat(K, K, V, 32, 0x00, x_oct, h_oct);
    hmac_cat(V, K, V, 32, -1, nullptr, nullptr);
    // RFC 6979 second pass: rekey K from V, tag byte 0x01, x_oct and h_oct, then refresh V from K.
    hmac_cat(K, K, V, 32, 0x01, x_oct, h_oct);
    hmac_cat(V, K, V, 32, -1, nullptr, nullptr);

    Jac G;
    jac_set_affine(&G, P256_GX, P256_GY);

    for (int guard = 0; guard < 64; guard++)
    {
        // T = HMAC_K(V) (exactly 32 bytes -> one block).
        hmac_cat(V, K, V, 32, -1, nullptr, nullptr);
        uint32_t k[8];
        load_be(k, V); // bits2int(T)
        if (ecdsa_try_sign(k, &G, d, e, sig))
            return true;
        // Retry: K = HMAC_K(V || 0x00); V = HMAC_K(V).
        uint8_t buf[33];
        memcpy(buf, V, 32);
        buf[32] = 0x00;
        ssh_hmac_sha256(K, 32, buf, 33, K);
        hmac_cat(V, K, V, 32, -1, nullptr, nullptr);
    }
    return false;
}

} // namespace

bool ssh_ecdsa_p256_pubkey(uint8_t pub[SSH_ECDSA_P256_PUB_LEN], const uint8_t priv[SSH_ECDSA_P256_PRIV_LEN])
{
    uint32_t d[8];
    load_be(d, priv);
    if (fp_is_zero(d) || fp_cmp(d, P256_N) >= 0)
        return false;
    Jac G;
    Jac Q;
    jac_set_affine(&G, P256_GX, P256_GY);
    jac_scalar_mul(&Q, d, &G);
    if (fp_is_zero(Q.Z))
        return false;
    uint32_t qx[8];
    uint32_t qy[8];
    jac_to_affine(qx, qy, &Q);
    pub[0] = 0x04;
    store_be(pub + 1, qx);
    store_be(pub + 33, qy);
    return true;
}

bool ssh_ecdsa_p256_sign(uint8_t sig[SSH_ECDSA_P256_SIG_LEN], const uint8_t *msg, size_t mlen,
                         const uint8_t priv[SSH_ECDSA_P256_PRIV_LEN])
{
    uint32_t d[8];
    load_be(d, priv);
    if (fp_is_zero(d) || fp_cmp(d, P256_N) >= 0)
        return false;
    uint8_t h1[SSH_SHA256_DIGEST_LEN];
    ssh_sha256(msg, mlen, h1);
    return ecdsa_sign_core(sig, h1, d);
}

bool ssh_ecdsa_p256_verify(const uint8_t pub[SSH_ECDSA_P256_PUB_LEN], const uint8_t *msg, size_t mlen,
                           const uint8_t sig[SSH_ECDSA_P256_SIG_LEN])
{
    if (pub[0] != 0x04)
        return false;
    uint32_t qx[8];
    uint32_t qy[8];
    load_be(qx, pub + 1);
    load_be(qy, pub + 33);
    if (!on_curve(qx, qy))
        return false;

    uint32_t r[8];
    uint32_t s[8];
    load_be(r, sig);
    load_be(s, sig + 32);
    if (fp_is_zero(r) || fp_cmp(r, P256_N) >= 0 || fp_is_zero(s) || fp_cmp(s, P256_N) >= 0)
        return false;

    uint8_t h1[SSH_SHA256_DIGEST_LEN];
    ssh_sha256(msg, mlen, h1);
    uint32_t e[8];
    uint32_t etmp[8];
    load_be(etmp, h1);
    fp_reduce_once(e, etmp, P256_N);

    uint32_t w[8];
    uint32_t u1[8];
    uint32_t u2[8];
    fp_inv(w, s, P256_N);
    fp_mul(u1, e, w, P256_N);
    fp_mul(u2, r, w, P256_N);

    Jac G;
    Jac Q;
    Jac Rg;
    Jac Rq;
    Jac R;
    jac_set_affine(&G, P256_GX, P256_GY);
    jac_set_affine(&Q, qx, qy);
    jac_scalar_mul(&Rg, u1, &G);
    jac_scalar_mul(&Rq, u2, &Q);
    jac_add(&R, &Rg, &Rq);
    if (fp_is_zero(R.Z))
        return false;
    uint32_t rx[8];
    uint32_t ry[8];
    uint32_t rxn[8];
    jac_to_affine(rx, ry, &R);
    fp_reduce_once(rxn, rx, P256_N);
    return fp_cmp(rxn, r) == 0;
}

bool ssh_ecdsa_p256_ecdh(uint8_t shared_x[SSH_ECDSA_P256_COORD_LEN], const uint8_t peer_pub[SSH_ECDSA_P256_PUB_LEN],
                         const uint8_t priv[SSH_ECDSA_P256_PRIV_LEN])
{
    if (peer_pub[0] != 0x04)
        return false;
    uint32_t qx[8];
    uint32_t qy[8];
    load_be(qx, peer_pub + 1);
    load_be(qy, peer_pub + 33);
    if (!on_curve(qx, qy)) // rejects off-curve / out-of-range peer points
        return false;
    uint32_t d[8];
    load_be(d, priv);
    if (fp_is_zero(d) || fp_cmp(d, P256_N) >= 0)
        return false;
    Jac Q;
    Jac R;
    jac_set_affine(&Q, qx, qy);
    jac_scalar_mul(&R, d, &Q);
    if (fp_is_zero(R.Z)) // d*Q is the identity -> invalid shared secret
        return false;
    uint32_t rx[8];
    uint32_t ry[8];
    jac_to_affine(rx, ry, &R);
    store_be(shared_x, rx); // K = X coordinate (big-endian)
    return true;
}

#endif // ARDUINO
