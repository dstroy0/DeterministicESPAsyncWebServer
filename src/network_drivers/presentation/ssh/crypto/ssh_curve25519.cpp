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
 */

#include "network_drivers/presentation/ssh/crypto/ssh_curve25519.h"
#ifdef ARDUINO
#include <mbedtls/bignum.h> // ESP32: field inversion on the MPI/RSA hardware accelerator
#endif

// Small field constants.
static const ssh_gf GF_1 = {1};
static const ssh_gf GF_121665 = {0xDB41, 1}; // 121665 = 0x1DB41 (Montgomery a24)

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

void ssh_gf_mul(ssh_gf out, const ssh_gf a, const ssh_gf b)
{
    int64_t t[31];
    for (int i = 0; i < 31; i++)
        t[i] = 0;
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++)
            t[i + j] += a[i] * b[j];
    for (int i = 0; i < 15; i++)
        t[i] += 38 * t[i + 16]; // fold the upper half (weight >= 2^256) down
    for (int i = 0; i < 16; i++)
        out[i] = t[i];
    gf_carry(out);
    gf_carry(out);
}

void ssh_gf_sq(ssh_gf out, const ssh_gf a)
{
    ssh_gf_mul(out, a, a);
}

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
    (void)GF_1;
}

void ssh_x25519_base(uint8_t out[32], const uint8_t scalar[32])
{
    uint8_t base[32] = {9};
    ssh_x25519(out, scalar, base);
}
