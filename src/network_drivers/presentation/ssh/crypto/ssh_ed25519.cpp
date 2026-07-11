// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_ed25519.cpp
 * @brief Ed25519 (RFC 8032) sign + verify over edwards25519.
 *
 * Points are held in extended twisted-Edwards coordinates (X, Y, Z, T) as four field
 * elements (ssh_gf, from ssh_curve25519). Scalar multiplication is a constant-time
 * bit-by-bit double-and-add; scalars are reduced mod the group order L. SHA-512 hashes
 * the seed, the nonce input, and R||A||M. Deterministic (no RNG). Validated against the
 * RFC 8032 §7.1 vectors and a reference implementation (test_ssh_ed25519).
 */

#include "network_drivers/presentation/ssh/crypto/ssh_ed25519.h"
#include "network_drivers/presentation/ssh/crypto/ssh_curve25519.h" // ssh_gf + field ops (add/sub/mul/sq/inv/pack/unpack/cswap/copy)
#include "network_drivers/presentation/ssh/crypto/ssh_sha512.h"

// --- Curve constants (radix-2^16 field elements, little-endian limbs) --------

static const ssh_gf GF0 = {0};
static const ssh_gf GF1 = {1};
// d = -121665/121666 (the twisted-Edwards curve constant) and 2d.
static const ssh_gf ED_D = {0x78a3, 0x1359, 0x4dca, 0x75eb, 0xd8ab, 0x4141, 0x0a4d, 0x0070,
                            0xe898, 0x7779, 0x4079, 0x8cc7, 0xfe73, 0x2b6f, 0x6cee, 0x5203};
static const ssh_gf ED_D2 = {0xf159, 0x26b2, 0x9b94, 0xebd6, 0xb156, 0x8283, 0x149a, 0x00e0,
                             0xd130, 0xeef3, 0x80f2, 0x198e, 0xfce7, 0x56df, 0xd9dc, 0x2406};
// Base point B = (X, Y).
static const ssh_gf ED_X = {0xd51a, 0x8f25, 0x2d60, 0xc956, 0xa7b2, 0x9525, 0xc760, 0x692c,
                            0xdc5c, 0xfdd6, 0xe231, 0xc0a4, 0x53fe, 0xcd6e, 0x36d3, 0x2169};
static const ssh_gf ED_Y = {0x6658, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666,
                            0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666};
// sqrt(-1) mod p.
static const ssh_gf ED_I = {0xa0b0, 0x4a0e, 0x1b27, 0xc4ee, 0xe478, 0xad2f, 0x1806, 0x2f43,
                            0xd7a7, 0x3dfb, 0x0099, 0x2b4d, 0xdf0b, 0x4fc1, 0x2480, 0x2b83};
// The group order L = 2^252 + 27742317777372353535851937790883648493, little-endian.
static const int64_t ED_L[32] = {0xed, 0xd3, 0xf5, 0x5c, 0x1a, 0x63, 0x12, 0x58, 0xd6, 0x9c, 0xf7,
                                 0xa2, 0xde, 0xf9, 0xde, 0x14, 0,    0,    0,    0,    0,    0,
                                 0,    0,    0,    0,    0,    0,    0,    0,    0,    0x10};

// --- Small helpers ----------------------------------------------------------

// Constant-time 32-byte compare: 0 if equal, -1 otherwise.
static int ct_verify32(const uint8_t *x, const uint8_t *y)
{
    unsigned diff = 0;
    for (int i = 0; i < 32; i++)
        diff |= (unsigned)(x[i] ^ y[i]);
    return (int)((1 & ((diff - 1) >> 8)) - 1);
}

// Parity (low bit) of the canonical encoding of a field element.
static int gf_parity(const ssh_gf a)
{
    uint8_t d[32];
    ssh_gf_pack(d, a);
    return d[0] & 1;
}

// 0 if a and b encode the same field element, -1 otherwise.
static int gf_neq(const ssh_gf a, const ssh_gf b)
{
    uint8_t c[32];
    uint8_t d[32];
    ssh_gf_pack(c, a);
    ssh_gf_pack(d, b);
    return ct_verify32(c, d);
}

// out = a^(2^252 - 3) - used to take the square root during point decompression.
static void gf_pow2523(ssh_gf out, const ssh_gf a)
{
    ssh_gf c;
    ssh_gf_copy(c, a);
    for (int i = 250; i >= 0; i--)
    {
        ssh_gf_sq(c, c);
        if (i != 1)
            ssh_gf_mul(c, c, a);
    }
    ssh_gf_copy(out, c);
}

// --- Edwards point arithmetic (extended coords p[4] = X,Y,Z,T) --------------

// p += q (twisted-Edwards addition, RFC 8032 §5.1.4 / the unified add formula).
static void ed_add(ssh_gf p[4], ssh_gf q[4])
{
    ssh_gf a;
    ssh_gf b;
    ssh_gf c;
    ssh_gf d;
    ssh_gf t;
    ssh_gf e;
    ssh_gf f;
    ssh_gf g;
    ssh_gf h;
    ssh_gf_sub(a, p[1], p[0]);
    ssh_gf_sub(t, q[1], q[0]);
    ssh_gf_mul(a, a, t);
    ssh_gf_add(b, p[0], p[1]);
    ssh_gf_add(t, q[0], q[1]);
    ssh_gf_mul(b, b, t);
    ssh_gf_mul(c, p[3], q[3]);
    ssh_gf_mul(c, c, ED_D2);
    ssh_gf_mul(d, p[2], q[2]);
    ssh_gf_add(d, d, d);
    ssh_gf_sub(e, b, a);
    ssh_gf_sub(f, d, c);
    ssh_gf_add(g, d, c);
    ssh_gf_add(h, b, a);
    ssh_gf_mul(p[0], e, f);
    ssh_gf_mul(p[1], h, g);
    ssh_gf_mul(p[2], g, f);
    ssh_gf_mul(p[3], e, h);
}

// Constant-time conditional swap of points p and q when b == 1.
static void ed_cswap(ssh_gf p[4], ssh_gf q[4], int b)
{
    for (int i = 0; i < 4; i++)
        ssh_gf_cswap(p[i], q[i], b);
}

// Encode a point to 32 bytes: y with x's low bit in the top bit.
static void ed_pack(uint8_t r[32], ssh_gf p[4])
{
    ssh_gf tx;
    ssh_gf ty;
    ssh_gf zi;
    ssh_gf_inv(zi, p[2]);
    ssh_gf_mul(tx, p[0], zi);
    ssh_gf_mul(ty, p[1], zi);
    ssh_gf_pack(r, ty);
    r[31] ^= (uint8_t)(gf_parity(tx) << 7);
}

// p = s * q (variable-base scalar mult), s is 32 bytes little-endian.
static void ed_scalarmult(ssh_gf p[4], ssh_gf q[4], const uint8_t *s)
{
    ssh_gf_copy(p[0], GF0);
    ssh_gf_copy(p[1], GF1);
    ssh_gf_copy(p[2], GF1);
    ssh_gf_copy(p[3], GF0);
    for (int i = 255; i >= 0; i--)
    {
        int b = (s[i >> 3] >> (i & 7)) & 1;
        ed_cswap(p, q, b);
        ed_add(q, p);
        ed_add(p, p);
        ed_cswap(p, q, b);
    }
}

// p = s * B (base-point scalar mult).
static void ed_scalarbase(ssh_gf p[4], const uint8_t *s)
{
    ssh_gf q[4];
    ssh_gf_copy(q[0], ED_X);
    ssh_gf_copy(q[1], ED_Y);
    ssh_gf_copy(q[2], GF1);
    ssh_gf_mul(q[3], ED_X, ED_Y);
    ed_scalarmult(p, q, s);
}

// Decode a point and negate it (r = -A) for the verification equation; returns 0 on
// success, -1 if the encoding is not a valid curve point.
static int ed_unpackneg(ssh_gf r[4], const uint8_t p[32])
{
    ssh_gf t;
    ssh_gf chk;
    ssh_gf num;
    ssh_gf den;
    ssh_gf den2;
    ssh_gf den4;
    ssh_gf den6;
    ssh_gf_copy(r[2], GF1);
    ssh_gf_unpack(r[1], p); // y (top/sign bit masked off)
    ssh_gf_sq(num, r[1]);   // y^2
    ssh_gf_mul(den, num, ED_D);
    ssh_gf_sub(num, num, r[2]); // u = y^2 - 1
    ssh_gf_add(den, r[2], den); // v = d*y^2 + 1

    ssh_gf_sq(den2, den);
    ssh_gf_sq(den4, den2);
    ssh_gf_mul(den6, den4, den2);
    ssh_gf_mul(t, den6, num);
    ssh_gf_mul(t, t, den);
    gf_pow2523(t, t); // t = (u v^7)^((p-5)/8)
    ssh_gf_mul(t, t, num);
    ssh_gf_mul(t, t, den);
    ssh_gf_mul(t, t, den);
    ssh_gf_mul(r[0], t, den); // x candidate = u v^3 (u v^7)^((p-5)/8)

    ssh_gf_sq(chk, r[0]);
    ssh_gf_mul(chk, chk, den);
    if (gf_neq(chk, num))
        ssh_gf_mul(r[0], r[0], ED_I); // multiply by sqrt(-1)
    ssh_gf_sq(chk, r[0]);
    ssh_gf_mul(chk, chk, den);
    if (gf_neq(chk, num))
        return -1; // no square root: invalid point

    if (gf_parity(r[0]) == (p[31] >> 7))
        ssh_gf_sub(r[0], GF0, r[0]); // pick the correct sign, then negate for -A
    ssh_gf_mul(r[3], r[0], r[1]);
    return 0;
}

// --- Scalar reduction mod L -------------------------------------------------

// Reduce the 512-bit little-endian value x[0..63] modulo L into r[0..31].
static void ed_modL(uint8_t r[32], int64_t x[64])
{
    int64_t carry;
    int i;
    int j;
    for (i = 63; i >= 32; --i)
    {
        carry = 0;
        for (j = i - 32; j < i - 12; ++j)
        {
            x[j] += carry - 16 * x[i] * ED_L[j - (i - 32)];
            carry = (x[j] + 128) >> 8;
            x[j] -= carry << 8;
        }
        x[j] += carry;
        x[i] = 0;
    }
    carry = 0;
    for (j = 0; j < 32; ++j)
    {
        x[j] += carry - (x[31] >> 4) * ED_L[j];
        carry = x[j] >> 8;
        x[j] &= 255;
    }
    for (j = 0; j < 32; ++j)
        x[j] -= carry * ED_L[j];
    for (i = 0; i < 32; ++i)
    {
        x[i + 1] += x[i] >> 8;
        r[i] = (uint8_t)(x[i] & 255);
    }
}

// Reduce a 64-byte hash in place: r[0..31] = r mod L.
static void ed_reduce(uint8_t r[64])
{
    int64_t x[64];
    for (int i = 0; i < 64; i++)
        x[i] = (int64_t)(uint64_t)r[i];
    for (int i = 0; i < 64; i++)
        r[i] = 0;
    ed_modL(r, x);
}

// --- Public API -------------------------------------------------------------

void ssh_ed25519_pubkey(uint8_t pub[32], const uint8_t seed[32])
{
    uint8_t d[64];
    ssh_sha512(seed, 32, d);
    d[0] &= 248;
    d[31] &= 127;
    d[31] |= 64; // clamp -> secret scalar a = d[0..31]
    ssh_gf p[4];
    ed_scalarbase(p, d);
    ed_pack(pub, p);
}

void ssh_ed25519_sign(uint8_t sig[64], const uint8_t *msg, size_t mlen, const uint8_t seed[32])
{
    uint8_t d[64];
    uint8_t pub[32];
    uint8_t r[64];
    uint8_t h[64];
    ssh_sha512(seed, 32, d);
    d[0] &= 248;
    d[31] &= 127;
    d[31] |= 64; // a = d[0..31]; prefix = d[32..63]

    // A = a * B
    ssh_gf p[4];
    ed_scalarbase(p, d);
    ed_pack(pub, p);

    // r = SHA-512(prefix || M) mod L
    SshSha512Ctx c;
    ssh_sha512_init(&c);
    ssh_sha512_update(&c, d + 32, 32);
    ssh_sha512_update(&c, msg, mlen);
    ssh_sha512_final(&c, r);
    ed_reduce(r);

    // R = r * B
    ssh_gf rp[4];
    ed_scalarbase(rp, r);
    ed_pack(sig, rp); // sig[0..31] = R

    // h = SHA-512(R || A || M) mod L
    ssh_sha512_init(&c);
    ssh_sha512_update(&c, sig, 32);
    ssh_sha512_update(&c, pub, 32);
    ssh_sha512_update(&c, msg, mlen);
    ssh_sha512_final(&c, h);
    ed_reduce(h);

    // S = (r + h*a) mod L
    int64_t x[64];
    for (int i = 0; i < 64; i++)
        x[i] = 0;
    for (int i = 0; i < 32; i++)
        x[i] = (int64_t)(uint64_t)r[i];
    for (int i = 0; i < 32; i++)
        for (int j = 0; j < 32; j++)
            x[i + j] += (int64_t)(uint64_t)h[i] * (int64_t)(uint64_t)d[j];
    ed_modL(sig + 32, x); // sig[32..63] = S
}

// True iff the little-endian 32-byte scalar S is canonical (0 <= S < L). RFC 8032
// 5.1.7 requires this: S and S+L both satisfy the group equation (L*B is the
// identity), so without the range check the signature is malleable. Verification
// operates only on public data, so a plain compare from the top byte down is fine.
static bool ed_scalar_canonical(const uint8_t s[32])
{
    for (int i = 31; i >= 0; i--)
    {
        uint8_t li = (uint8_t)ED_L[i];
        if (s[i] < li)
            return true;
        if (s[i] > li)
            return false;
    }
    return false; // S == L is out of range
}

bool ssh_ed25519_verify(const uint8_t pub[32], const uint8_t *msg, size_t mlen, const uint8_t sig[64])
{
    if (!ed_scalar_canonical(sig + 32))
        return false; // non-canonical S (RFC 8032 5.1.7): reject to prevent malleability

    ssh_gf p[4];
    ssh_gf q[4];
    if (ed_unpackneg(q, pub) != 0)
        return false; // q = -A

    // h = SHA-512(R || A || M) mod L
    uint8_t h[64];
    SshSha512Ctx c;
    ssh_sha512_init(&c);
    ssh_sha512_update(&c, sig, 32); // R
    ssh_sha512_update(&c, pub, 32); // A
    ssh_sha512_update(&c, msg, mlen);
    ssh_sha512_final(&c, h);
    ed_reduce(h);

    ed_scalarmult(p, q, h); // p = h * (-A)
    ssh_gf sb[4];
    ed_scalarbase(sb, sig + 32); // sb = S * B
    ed_add(p, sb);               // p = S*B - h*A

    uint8_t t[32];
    ed_pack(t, p);
    return ct_verify32(sig, t) == 0; // R == S*B - h*A ?
}
