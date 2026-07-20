// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file sntrup761.cpp
 * @brief Streamlined NTRU Prime sntrup761 - full KEM: KeyGen, Encaps, Decaps (see sntrup761.h).
 *
 * Ported from OpenSSH's embedded sntrup761 reference (public domain; D. J. Bernstein,
 * C. Chuengsatiansup, T. Lange, C. van Vredendaal). We target a known set of platforms (the ESP32
 * variants + native), so the constant-time integer helpers are written directly with our int widths
 * instead of the reference's portable crypto_int layer. The generic Encode/Decode take a scratch
 * arena rather than the reference's variable-length recursion arrays, so the stack stays bounded.
 * SHA-512 and the RNG come from the SSH crypto seams; byte encodings and the hashing (prefix bytes
 * 1/2/3/4) match OpenSSH exactly, so a ciphertext produced here decapsulates on a real peer and a
 * public key generated here encapsulates on one - verified byte-exact against the reference both ways.
 */

#include "network_drivers/presentation/pqc/sntrup761.h"

#if DWS_ENABLE_PQC_KEX

#include "network_drivers/presentation/ssh/crypto/ssh_sha512.h"
#include <string.h>

// The CSPRNG seam (defined in ssh_dh.cpp) - forward-declared so this PQC primitive does not pull in
// the SSH key-exchange machinery (ServerConfig / bignum / keymat) that ssh_dh.h transitively includes.
void ssh_rng_fill(uint8_t *buf, size_t len);

namespace
{

// --- parameters (sntrup761) ---
constexpr int P = 761;
constexpr int Q = 4591;
constexpr int W = 286;
constexpr int Q12 = (Q - 1) / 2; // 2295
constexpr int HASH_BYTES = 32;
constexpr int SMALL_BYTES = (P + 3) / 4; // 191
constexpr int CONFIRM_BYTES = 32;
constexpr int CT_BYTES = DWS_SNTRUP761_CT_BYTES; // 1039
constexpr int PK_BYTES = DWS_SNTRUP761_PK_BYTES; // 1158

typedef int8_t small_t;
typedef int16_t Fq;

// Scratch arena for the Encode/Decode recursion (sum over levels of (len_i+1)/2 = 764; Decode carves
// 3 uint16 arrays + 1 uint32 array per level, Encode 2 uint16 arrays - both fit these).
constexpr int SCR16 = 2304;
constexpr int SCR32 = 768;

small_t F3_freeze(int16_t x)
{
    return (small_t)(x - 3 * ((10923 * x + 16384) >> 15));
}

Fq Fq_freeze(int32_t x)
{
    const int32_t q16 = (0x10000 + Q / 2) / Q;
    const int32_t q20 = (0x100000 + Q / 2) / Q;
    const int32_t q28 = (0x10000000 + Q / 2) / Q;
    x -= Q * ((q16 * x) >> 16);
    x -= Q * ((q20 * x) >> 20);
    return (Fq)(x - Q * ((q28 * x + 0x8000000) >> 28));
}

// sign bit of x broadcast to all 32 bits (portable, constant-time).
inline int32_t negative_mask(int32_t x)
{
    return -(int32_t)((uint32_t)x >> 31);
}

void uint32_divmod_uint14(uint32_t *Qout, uint16_t *rout, uint32_t x, uint16_t m)
{
    uint32_t qpart, mask, v = 0x80000000u / m;
    qpart = (uint32_t)((x * (uint64_t)v) >> 31);
    x -= qpart * m;
    *Qout = qpart;
    qpart = (uint32_t)((x * (uint64_t)v) >> 31);
    x -= qpart * m;
    *Qout += qpart;
    x -= m;
    *Qout += 1;
    mask = (uint32_t)negative_mask((int32_t)x);
    x += mask & (uint32_t)m;
    *Qout += mask;
    *rout = (uint16_t)x;
}

uint16_t uint32_mod_uint14(uint32_t x, uint16_t m)
{
    uint32_t Qq;
    uint16_t r;
    uint32_divmod_uint14(&Qq, &r, x, m);
    return r;
}

// Generic Encode: appends bytes at *out, halving (R,M) each level via a scratch arena.
uint8_t *Encode(uint8_t *out, const uint16_t *R, const uint16_t *M, int len, uint16_t *scr)
{
    if (len == 1)
    {
        uint16_t r = R[0], m = M[0];
        while (m > 1)
        {
            *out++ = (uint8_t)r;
            r >>= 8;
            m = (uint16_t)((m + 255) >> 8);
        }
        return out;
    }
    int half = (len + 1) / 2;
    uint16_t *R2 = scr, *M2 = scr + half;
    int i;
    for (i = 0; i + 1 < len; i += 2)
    {
        uint32_t m0 = M[i];
        uint32_t r = (uint32_t)R[i] + (uint32_t)R[i + 1] * m0;
        uint32_t m = (uint32_t)M[i + 1] * m0;
        while (m >= 16384)
        {
            *out++ = (uint8_t)r;
            r >>= 8;
            m = (m + 255) >> 8;
        }
        R2[i / 2] = (uint16_t)r;
        M2[i / 2] = (uint16_t)m;
    }
    if (i < len)
    {
        R2[i / 2] = R[i];
        M2[i / 2] = M[i];
    }
    return Encode(out, R2, M2, half, scr + 2 * half);
}

// Generic Decode: fills out[len] from S using moduli M, via a scratch arena.
void Decode(uint16_t *out, const uint8_t *S, const uint16_t *M, int len, uint16_t *scr, uint32_t *scr32)
{
    if (len == 1)
    {
        if (M[0] == 1)
            out[0] = 0;
        else if (M[0] <= 256)
            out[0] = uint32_mod_uint14(S[0], M[0]);
        else
            out[0] = uint32_mod_uint14((uint32_t)S[0] + ((uint32_t)(uint16_t)S[1] << 8), M[0]);
        return;
    }
    int half = (len + 1) / 2;
    uint16_t *R2 = scr, *M2 = scr + half, *bottomr = scr + 2 * half;
    uint32_t *bottomt = scr32;
    int i;
    for (i = 0; i + 1 < len; i += 2)
    {
        uint32_t m = (uint32_t)M[i] * (uint32_t)M[i + 1];
        if (m > 256 * 16383)
        {
            bottomt[i / 2] = 256 * 256;
            bottomr[i / 2] = (uint16_t)(S[0] + 256 * S[1]);
            S += 2;
            M2[i / 2] = (uint16_t)((((m + 255) >> 8) + 255) >> 8);
        }
        else if (m >= 16384)
        {
            bottomt[i / 2] = 256;
            bottomr[i / 2] = S[0];
            S += 1;
            M2[i / 2] = (uint16_t)((m + 255) >> 8);
        }
        else
        {
            bottomt[i / 2] = 1;
            bottomr[i / 2] = 0;
            M2[i / 2] = (uint16_t)m;
        }
    }
    if (i < len)
        M2[i / 2] = M[i];
    Decode(R2, S, M2, half, scr + 3 * half, scr32 + half);
    for (i = 0; i + 1 < len; i += 2)
    {
        uint32_t r1, r = bottomr[i / 2];
        uint16_t r0;
        r += bottomt[i / 2] * R2[i / 2];
        uint32_divmod_uint14(&r1, &r0, r, M[i]);
        r1 = uint32_mod_uint14(r1, M[i + 1]);
        *out++ = r0;
        *out++ = (uint16_t)r1;
    }
    if (i < len)
        *out++ = R2[i / 2];
}

// h = f * g in Rq (g small), mod x^p - x - 1.
void Rq_mult_small(Fq *h, const Fq *f, const small_t *g)
{
    int32_t fg[P + P - 1];
    int i, j;
    for (i = 0; i < P + P - 1; ++i)
        fg[i] = 0;
    for (i = 0; i < P; ++i)
        for (j = 0; j < P; ++j)
            fg[i + j] += f[i] * (int32_t)g[j];
    for (i = P; i < P + P - 1; ++i)
        fg[i - P] += fg[i];
    for (i = P; i < P + P - 1; ++i)
        fg[i - P + 1] += fg[i];
    for (i = 0; i < P; ++i)
        h[i] = Fq_freeze(fg[i]);
}

void Round(Fq *out, const Fq *a)
{
    for (int i = 0; i < P; ++i)
        out[i] = (Fq)(a[i] - F3_freeze(a[i]));
}

// --- constant-time uint32 sort (djbsort network), for Short_fromlist ---
inline void int32_minmax(int32_t *pp, int32_t *pq)
{
    int32_t x = *pp, y = *pq;
    int64_t d = (int64_t)y - (int64_t)x;                    // 64-bit to avoid overflow
    int32_t swap = (int32_t)-((uint64_t)d >> 63) & (x ^ y); // all-ones when y < x
    *pp = x ^ swap;
    *pq = y ^ swap;
}

void crypto_sort_int32(int32_t *x, long long n)
{
    long long top, p, q, r, i, j;
    if (n < 2)
        return;
    top = 1;
    while (top < n - top)
        top += top;
    for (p = top; p >= 1; p >>= 1)
    {
        i = 0;
        while (i + 2 * p <= n)
        {
            for (j = i; j < i + p; ++j)
                int32_minmax(&x[j], &x[j + p]);
            i += 2 * p;
        }
        for (j = i; j < n - p; ++j)
            int32_minmax(&x[j], &x[j + p]);
        i = 0;
        j = 0;
        for (q = top; q > p; q >>= 1)
        {
            if (j != i)
                for (;;)
                {
                    if (j == n - q)
                        goto done;
                    int32_t a = x[j + p];
                    for (r = q; r > p; r >>= 1)
                        int32_minmax(&a, &x[j + r]);
                    x[j + p] = a;
                    ++j;
                    if (j == i + p)
                    {
                        i += 2 * p;
                        break;
                    }
                }
            while (i + p <= n - q)
            {
                for (j = i; j < i + p; ++j)
                {
                    int32_t a = x[j + p];
                    for (r = q; r > p; r >>= 1)
                        int32_minmax(&a, &x[j + r]);
                    x[j + p] = a;
                }
                i += 2 * p;
            }
            j = i;
            while (j < n - q)
            {
                int32_t a = x[j + p];
                for (r = q; r > p; r >>= 1)
                    int32_minmax(&a, &x[j + r]);
                x[j + p] = a;
                ++j;
            }
        done:;
        }
    }
}

void crypto_sort_uint32(uint32_t *x, long long n)
{
    for (long long j = 0; j < n; ++j)
        x[j] ^= 0x80000000u;
    crypto_sort_int32((int32_t *)x, n);
    for (long long j = 0; j < n; ++j)
        x[j] ^= 0x80000000u;
}

void Short_fromlist(small_t *out, const uint32_t *in)
{
    uint32_t L[P];
    int i;
    for (i = 0; i < W; ++i)
        L[i] = in[i] & (uint32_t)-2;
    for (i = W; i < P; ++i)
        L[i] = (in[i] & (uint32_t)-3) | 1;
    crypto_sort_uint32(L, P);
    for (i = 0; i < P; ++i)
        out[i] = (small_t)((L[i] & 3) - 1);
}

void Short_random(small_t *out)
{
    uint32_t L[P];
    uint8_t rb[4];
    for (int i = 0; i < P; ++i)
    {
        ssh_rng_fill(rb, 4);
        L[i] = (uint32_t)rb[0] | ((uint32_t)rb[1] << 8) | ((uint32_t)rb[2] << 16) | ((uint32_t)rb[3] << 24);
    }
    Short_fromlist(out, L);
}

// out = SHA512(b || in)[0:32].
void Hash_prefix(uint8_t *out, int b, const uint8_t *in, size_t inlen)
{
    SshSha512Ctx ctx;
    uint8_t h[SSH_SHA512_DIGEST_LEN];
    uint8_t bb = (uint8_t)b;
    ssh_sha512_init(&ctx);
    ssh_sha512_update(&ctx, &bb, 1);
    ssh_sha512_update(&ctx, in, inlen);
    ssh_sha512_final(&ctx, h);
    memcpy(out, h, HASH_BYTES);
}

void Small_encode(uint8_t *s, const small_t *f)
{
    for (int i = 0; i < P / 4; ++i)
    {
        small_t x = 0;
        for (int j = 0; j < 4; ++j)
            x = (small_t)(x + ((*f++ + 1) << (2 * j)));
        *s++ = (uint8_t)x;
    }
    *s = (uint8_t)(*f + 1);
}

void Rq_decode(Fq *r, const uint8_t *s, uint16_t *scr, uint32_t *scr32)
{
    uint16_t Rr[P], M[P];
    for (int i = 0; i < P; ++i)
        M[i] = Q;
    Decode(Rr, s, M, P, scr, scr32);
    for (int i = 0; i < P; ++i)
        r[i] = (Fq)(((Fq)Rr[i]) - Q12);
}

void Rounded_encode(uint8_t *s, const Fq *r, uint16_t *scr)
{
    uint16_t Rr[P], M[P];
    for (int i = 0; i < P; ++i)
        Rr[i] = (uint16_t)(((r[i] + Q12) * 10923) >> 15);
    for (int i = 0; i < P; ++i)
        M[i] = (Q + 2) / 3;
    Encode(s, Rr, M, P, scr);
}

void HashConfirm(uint8_t *h, const uint8_t *r_enc, const uint8_t *cache)
{
    uint8_t x[HASH_BYTES * 2];
    Hash_prefix(x, 3, r_enc, SMALL_BYTES);
    memcpy(x + HASH_BYTES, cache, HASH_BYTES);
    Hash_prefix(h, 2, x, sizeof x);
}

void HashSession(uint8_t *k, int b, const uint8_t *r_enc, const uint8_t *c)
{
    uint8_t x[HASH_BYTES + CT_BYTES];
    Hash_prefix(x, 3, r_enc, SMALL_BYTES);
    memcpy(x + HASH_BYTES, c, CT_BYTES);
    Hash_prefix(k, b, x, sizeof x);
}

// Encapsulation reused for the Decapsulation FO re-encrypt check.
void Hide(uint8_t *c, uint8_t *r_enc, const small_t *r, const uint8_t *pk, const uint8_t *cache, uint16_t *scr,
          uint32_t *scr32)
{
    Small_encode(r_enc, r);
    Fq h[P], cp[P];
    Rq_decode(h, pk, scr, scr32);
    Rq_mult_small(cp, h, r); // c = Round(h * r)
    Round(cp, cp);
    Rounded_encode(c, cp, scr);
    HashConfirm(c + CT_BYTES - CONFIRM_BYTES, r_enc, cache);
}

// ===========================================================================
// KeyGen + Decapsulation (the reverse-SSH client / initiator side)
// ===========================================================================

// -1 (all ones) when the argument is nonzero / negative; 0 otherwise (our int widths are known).
inline int nonzero_mask16(int16_t x)
{
    uint32_t u = (uint16_t)x;
    return -(int)((u | (0u - u)) >> 31);
}
inline int negative_mask16(int16_t x)
{
    return -(int)((uint16_t)x >> 15);
}

void R3_fromRq(small_t *out, const Fq *r)
{
    for (int i = 0; i < P; ++i)
        out[i] = F3_freeze(r[i]);
}

void R3_mult(small_t *h, const small_t *f, const small_t *g)
{
    int16_t fg[P + P - 1];
    int i, j;
    for (i = 0; i < P + P - 1; ++i)
        fg[i] = 0;
    for (i = 0; i < P; ++i)
        for (j = 0; j < P; ++j)
            fg[i + j] = (int16_t)(fg[i + j] + f[i] * (int16_t)g[j]);
    for (i = P; i < P + P - 1; ++i)
        fg[i - P] = (int16_t)(fg[i - P] + fg[i]);
    for (i = P; i < P + P - 1; ++i)
        fg[i - P + 1] = (int16_t)(fg[i - P + 1] + fg[i]);
    for (i = 0; i < P; ++i)
        h[i] = F3_freeze(fg[i]);
}

// 1/in in R3 (mod 3); returns 0 on success (in invertible), -1 otherwise. Constant-time GCD.
int R3_recip(small_t *out, const small_t *in)
{
    small_t f[P + 1], g[P + 1], v[P + 1], r[P + 1];
    int sign, swap, t, i, loop, delta = 1;
    for (i = 0; i < P + 1; ++i)
        v[i] = 0;
    for (i = 0; i < P + 1; ++i)
        r[i] = 0;
    r[0] = 1;
    for (i = 0; i < P; ++i)
        f[i] = 0;
    f[0] = 1;
    f[P - 1] = f[P] = -1;
    for (i = 0; i < P; ++i)
        g[P - 1 - i] = in[i];
    g[P] = 0;
    for (loop = 0; loop < 2 * P - 1; ++loop)
    {
        for (i = P; i > 0; --i)
            v[i] = v[i - 1];
        v[0] = 0;
        sign = -g[0] * f[0];
        swap = negative_mask16((int16_t)-delta) & nonzero_mask16(g[0]);
        delta ^= swap & (delta ^ -delta);
        delta += 1;
        for (i = 0; i < P + 1; ++i)
        {
            t = swap & (f[i] ^ g[i]);
            f[i] = (small_t)(f[i] ^ t);
            g[i] = (small_t)(g[i] ^ t);
            t = swap & (v[i] ^ r[i]);
            v[i] = (small_t)(v[i] ^ t);
            r[i] = (small_t)(r[i] ^ t);
        }
        for (i = 0; i < P + 1; ++i)
            g[i] = F3_freeze((int16_t)(g[i] + sign * f[i]));
        for (i = 0; i < P + 1; ++i)
            r[i] = F3_freeze((int16_t)(r[i] + sign * v[i]));
        for (i = 0; i < P; ++i)
            g[i] = g[i + 1];
        g[P] = 0;
    }
    sign = f[0];
    for (i = 0; i < P; ++i)
        out[i] = (small_t)(sign * v[P - 1 - i]);
    return nonzero_mask16((int16_t)delta);
}

void Rq_mult3(Fq *h, const Fq *f)
{
    for (int i = 0; i < P; ++i)
        h[i] = Fq_freeze(3 * f[i]);
}

Fq Fq_recip(Fq a1)
{
    int i = 1;
    Fq ai = a1;
    while (i < Q - 2)
    {
        ai = Fq_freeze(a1 * (int32_t)ai);
        i += 1;
    }
    return ai;
}

// out = 1/(3*in) in Rq (used by KeyGen). Constant-time GCD over Fq.
int Rq_recip3(Fq *out, const small_t *in)
{
    Fq f[P + 1], g[P + 1], v[P + 1], r[P + 1], scale;
    int swap, i, loop, delta = 1;
    int32_t f0, g0;
    for (i = 0; i < P + 1; ++i)
        v[i] = 0;
    for (i = 0; i < P + 1; ++i)
        r[i] = 0;
    r[0] = Fq_recip(3);
    for (i = 0; i < P; ++i)
        f[i] = 0;
    f[0] = 1;
    f[P - 1] = f[P] = -1;
    for (i = 0; i < P; ++i)
        g[P - 1 - i] = in[i];
    g[P] = 0;
    for (loop = 0; loop < 2 * P - 1; ++loop)
    {
        for (i = P; i > 0; --i)
            v[i] = v[i - 1];
        v[0] = 0;
        swap = negative_mask16((int16_t)-delta) & nonzero_mask16(g[0]);
        delta ^= swap & (delta ^ -delta);
        delta += 1;
        Fq tmp;
        for (i = 0; i < P + 1; ++i)
        {
            tmp = (Fq)(swap & (f[i] ^ g[i]));
            f[i] ^= tmp;
            g[i] ^= tmp;
            tmp = (Fq)(swap & (v[i] ^ r[i]));
            v[i] ^= tmp;
            r[i] ^= tmp;
        }
        f0 = f[0];
        g0 = g[0];
        for (i = 0; i < P + 1; ++i)
            g[i] = Fq_freeze(f0 * g[i] - g0 * f[i]);
        for (i = 0; i < P + 1; ++i)
            r[i] = Fq_freeze(f0 * r[i] - g0 * v[i]);
        for (i = 0; i < P; ++i)
            g[i] = g[i + 1];
        g[P] = 0;
    }
    scale = Fq_recip(f[0]);
    for (i = 0; i < P; ++i)
        out[i] = Fq_freeze(scale * (int32_t)v[P - 1 - i]);
    return nonzero_mask16((int16_t)delta);
}

int Weightw_mask(const small_t *r)
{
    int weight = 0;
    for (int i = 0; i < P; ++i)
        weight += (r[i] & 1);
    return nonzero_mask16((int16_t)(weight - W));
}

void Small_random(small_t *out)
{
    uint8_t rb[4];
    for (int i = 0; i < P; ++i)
    {
        ssh_rng_fill(rb, 4);
        uint32_t u = (uint32_t)rb[0] | ((uint32_t)rb[1] << 8) | ((uint32_t)rb[2] << 16) | ((uint32_t)rb[3] << 24);
        out[i] = (small_t)((((u & 0x3fffffff) * 3) >> 30) - 1);
    }
}

void KeyGen(Fq *h, small_t *f, small_t *ginv)
{
    small_t g[P];
    Fq finv[P];
    for (;;)
    {
        Small_random(g);
        if (R3_recip(ginv, g) == 0)
            break;
    }
    Short_random(f);
    Rq_recip3(finv, f);
    Rq_mult_small(h, finv, g);
}

void Small_decode(small_t *f, const uint8_t *s)
{
    for (int i = 0; i < P / 4; ++i)
    {
        uint8_t x = *s++;
        for (int j = 0; j < 4; ++j)
            *f++ = (small_t)(((x >> (2 * j)) & 3) - 1);
    }
    *f = (small_t)((*s & 3) - 1);
}

void Rounded_decode(Fq *r, const uint8_t *s, uint16_t *scr, uint32_t *scr32)
{
    uint16_t Rr[P], M[P];
    for (int i = 0; i < P; ++i)
        M[i] = (Q + 2) / 3;
    Decode(Rr, s, M, P, scr, scr32);
    for (int i = 0; i < P; ++i)
        r[i] = (Fq)(Rr[i] * 3 - Q12);
}

void Rq_encode(uint8_t *s, const Fq *r, uint16_t *scr)
{
    uint16_t Rr[P], M[P];
    for (int i = 0; i < P; ++i)
        Rr[i] = (uint16_t)(r[i] + Q12);
    for (int i = 0; i < P; ++i)
        M[i] = Q;
    Encode(s, Rr, M, P, scr);
}

void Decrypt(small_t *r, const Fq *c, const small_t *f, const small_t *ginv)
{
    Fq cf[P], cf3[P];
    small_t e[P], ev[P];
    int mask, i;
    Rq_mult_small(cf, c, f);
    Rq_mult3(cf3, cf);
    R3_fromRq(e, cf3);
    R3_mult(ev, e, ginv);
    mask = Weightw_mask(ev);
    for (i = 0; i < W; ++i)
        r[i] = (small_t)(((ev[i] ^ 1) & ~mask) ^ 1);
    for (i = W; i < P; ++i)
        r[i] = (small_t)(ev[i] & ~mask);
}

// 0 when the two ciphertexts are equal, -1 otherwise.
int Ciphertexts_diff_mask(const uint8_t *c, const uint8_t *c2)
{
    uint16_t differentbits = 0;
    for (int i = 0; i < CT_BYTES; ++i)
        differentbits |= (uint16_t)(c[i] ^ c2[i]);
    return (int)((((uint16_t)(differentbits - 1)) >> 8) & 1) - 1;
}

} // namespace

void dws_sntrup761_enc(const uint8_t pk[DWS_SNTRUP761_PK_BYTES], uint8_t ct[DWS_SNTRUP761_CT_BYTES],
                       uint8_t ss[DWS_SNTRUP761_SS_BYTES])
{
    uint16_t scr16[SCR16];
    uint32_t scr32[SCR32];
    small_t r[P];
    uint8_t r_enc[SMALL_BYTES], cache[HASH_BYTES];

    Hash_prefix(cache, 4, pk, PK_BYTES);
    Short_random(r);
    Hide(ct, r_enc, r, pk, cache, scr16, scr32);
    HashSession(ss, 1, r_enc, ct);
}

void dws_sntrup761_keypair(uint8_t pk[DWS_SNTRUP761_PK_BYTES], uint8_t sk[DWS_SNTRUP761_SK_BYTES])
{
    uint16_t scr16[SCR16];
    Fq h[P];
    small_t f[P], ginv[P];

    KeyGen(h, f, ginv);
    Rq_encode(pk, h, scr16);
    Small_encode(sk, f);
    Small_encode(sk + SMALL_BYTES, ginv);
    // ...then the pk copy, a random rho for implicit reject, and the cached H(4||pk).
    uint8_t *tail = sk + 2 * SMALL_BYTES; // SecretKeys_bytes = 2 * Small_bytes
    memcpy(tail, pk, PK_BYTES);
    ssh_rng_fill(tail + PK_BYTES, SMALL_BYTES);
    Hash_prefix(tail + PK_BYTES + SMALL_BYTES, 4, pk, PK_BYTES);
}

void dws_sntrup761_dec(const uint8_t sk[DWS_SNTRUP761_SK_BYTES], const uint8_t ct[DWS_SNTRUP761_CT_BYTES],
                       uint8_t ss[DWS_SNTRUP761_SS_BYTES])
{
    uint16_t scr16[SCR16];
    uint32_t scr32[SCR32];
    const uint8_t *pk = sk + 2 * SMALL_BYTES;
    const uint8_t *rho = pk + PK_BYTES;
    const uint8_t *cache = rho + SMALL_BYTES;
    small_t f[P], ginv[P], r[P];
    Fq cp[P];
    uint8_t r_enc[SMALL_BYTES], cnew[CT_BYTES];

    Small_decode(f, sk);
    Small_decode(ginv, sk + SMALL_BYTES);
    Rounded_decode(cp, ct, scr16, scr32);
    Decrypt(r, cp, f, ginv);
    Hide(cnew, r_enc, r, pk, cache, scr16, scr32); // re-encrypt: FO check
    int mask = Ciphertexts_diff_mask(ct, cnew);
    for (int i = 0; i < SMALL_BYTES; ++i)
        r_enc[i] = (uint8_t)(r_enc[i] ^ (mask & (r_enc[i] ^ rho[i]))); // implicit reject -> rho
    HashSession(ss, 1 + mask, r_enc, ct);
}

#endif // DWS_ENABLE_PQC_KEX
