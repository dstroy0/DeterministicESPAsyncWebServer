// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "network_drivers/presentation/pqc/mlkem.h"

#if DETWS_ENABLE_PQC_KEX

#include "network_drivers/presentation/pqc/sha3.h"
#include <string.h>

// ML-KEM-768 parameters (FIPS 203).
#define MK_N 256
#define MK_Q 3329
#define MK_K 3
#define MK_ETA 2 // eta1 == eta2 == 2 for ML-KEM-768
#define MK_DU 10
#define MK_DV 4
#define MK_POLYBYTES 384 // 12 bits * 256 / 8
#define MK_QINV (-3327)  // q^-1 mod 2^16, signed

// Twiddle factors zeta^BitRev7(i) in Montgomery form, reduced to (-q/2, q/2] (FIPS 203 / pq-crystals).
static const int16_t mk_zetas[128] = {
    -1044, -758,  -359,  -1517, 1493,  1422,  287,   202,   -171,  622,  1577,  182,   962,   -1202, -1474, 1468,
    573,   -1325, 264,   383,   -829,  1458,  -1602, -130,  -681,  1017, 732,   608,   -1542, 411,   -205,  -1571,
    1223,  652,   -552,  1015,  -1293, 1491,  -282,  -1544, 516,   -8,   -320,  -666,  -1618, -1162, 126,   1469,
    -853,  -90,   -271,  830,   107,   -1421, -247,  -951,  -398,  961,  -1508, -725,  448,   -1065, 677,   -1275,
    -1103, 430,   555,   843,   -1251, 871,   1550,  105,   422,   587,  177,   -235,  -291,  -460,  1574,  1653,
    -246,  778,   1159,  -147,  -777,  1483,  -602,  1119,  -1590, 644,  -872,  349,   418,   329,   -156,  -75,
    817,   1097,  603,   610,   1322,  -1285, -1465, 384,   -1215, -136, 1218,  -1335, -874,  220,   -1187, -1659,
    -1185, -1530, -1278, 794,   -1510, -854,  -870,  478,   -108,  -308, 996,   991,   958,   -1460, 1522,  1628};

// Montgomery reduction: given a = m*R, return m mod q in (-q, q). R = 2^16.
static int16_t mont_reduce(int32_t a)
{
    int16_t t = (int16_t)((int16_t)a * (int16_t)MK_QINV);
    t = (int16_t)((a - (int32_t)t * MK_Q) >> 16);
    return t;
}

static inline int16_t fqmul(int16_t a, int16_t b)
{
    return mont_reduce((int32_t)a * b);
}

// Barrett reduction: a mod q in (-q/2, q/2].
static int16_t barrett_reduce(int16_t a)
{
    const int16_t v = 20159; // round(2^26 / q)
    int16_t t = (int16_t)(((int32_t)v * a + (1 << 25)) >> 26);
    t = (int16_t)(t * MK_Q);
    return (int16_t)(a - t);
}

// Forward NTT (in place). Coefficients enter in normal order, leave in bit-reversed NTT order.
static void ntt(int16_t r[MK_N])
{
    unsigned k = 1;
    for (unsigned len = 128; len >= 2; len >>= 1)
    {
        for (unsigned start = 0; start < MK_N; start += (len << 1))
        {
            int16_t zeta = mk_zetas[k++];
            for (unsigned j = start; j < start + len; j++)
            {
                int16_t t = fqmul(zeta, r[j + len]);
                r[j + len] = (int16_t)(r[j] - t);
                r[j] = (int16_t)(r[j] + t);
            }
        }
    }
}

// Inverse NTT (in place), with the 1/128 * Montgomery scaling folded into the final multiply.
static void invntt(int16_t r[MK_N])
{
    const int16_t f = 1441; // mont^2 / 128
    unsigned k = 127;
    for (unsigned len = 2; len <= 128; len <<= 1)
    {
        for (unsigned start = 0; start < MK_N; start += (len << 1))
        {
            int16_t zeta = mk_zetas[k--];
            for (unsigned j = start; j < start + len; j++)
            {
                int16_t t = r[j];
                r[j] = barrett_reduce((int16_t)(t + r[j + len]));
                r[j + len] = (int16_t)(r[j + len] - t);
                r[j + len] = fqmul(zeta, r[j + len]);
            }
        }
    }
    for (unsigned j = 0; j < MK_N; j++)
        r[j] = fqmul(r[j], f);
}

// Multiply two degree-1 residues mod (X^2 - zeta) in the NTT domain.
static void basemul(int16_t r[2], const int16_t a[2], const int16_t b[2], int16_t zeta)
{
    r[0] = fqmul(a[1], b[1]);
    r[0] = fqmul(r[0], zeta);
    r[0] = (int16_t)(r[0] + fqmul(a[0], b[0]));
    r[1] = fqmul(a[0], b[1]);
    r[1] = (int16_t)(r[1] + fqmul(a[1], b[0]));
}

static void poly_basemul(int16_t r[MK_N], const int16_t a[MK_N], const int16_t b[MK_N])
{
    for (unsigned i = 0; i < MK_N / 4; i++)
    {
        basemul(&r[4 * i], &a[4 * i], &b[4 * i], mk_zetas[64 + i]);
        basemul(&r[4 * i + 2], &a[4 * i + 2], &b[4 * i + 2], (int16_t)(-mk_zetas[64 + i]));
    }
}

// r = sum_i a[i] o b[i]  (pointwise in the NTT domain), then Barrett-reduced.
static void polyvec_basemul_acc(int16_t r[MK_N], const int16_t a[MK_K][MK_N], const int16_t b[MK_K][MK_N])
{
    int16_t t[MK_N];
    poly_basemul(r, a[0], b[0]);
    for (unsigned i = 1; i < MK_K; i++)
    {
        poly_basemul(t, a[i], b[i]);
        for (unsigned j = 0; j < MK_N; j++)
            r[j] = (int16_t)(r[j] + t[j]);
    }
    for (unsigned j = 0; j < MK_N; j++)
        r[j] = barrett_reduce(r[j]);
}

// ByteDecode_12: 384 octets -> 256 coefficients in [0, 2^12).
static void poly_frombytes(int16_t r[MK_N], const uint8_t a[MK_POLYBYTES])
{
    for (unsigned i = 0; i < MK_N / 2; i++)
    {
        r[2 * i] = (int16_t)(((a[3 * i + 0] >> 0) | ((uint16_t)a[3 * i + 1] << 8)) & 0xFFF);
        r[2 * i + 1] = (int16_t)(((a[3 * i + 1] >> 4) | ((uint16_t)a[3 * i + 2] << 4)) & 0xFFF);
    }
}

// Decompress_1: each message bit b -> b ? (q+1)/2 : 0.
static void poly_frommsg(int16_t r[MK_N], const uint8_t msg[32])
{
    for (unsigned i = 0; i < 32; i++)
        for (unsigned j = 0; j < 8; j++)
        {
            int16_t mask = (int16_t)(-(int16_t)((msg[i] >> j) & 1));
            r[8 * i + j] = (int16_t)(mask & ((MK_Q + 1) / 2));
        }
}

static inline uint32_t load32_le(const uint8_t *x)
{
    return (uint32_t)x[0] | ((uint32_t)x[1] << 8) | ((uint32_t)x[2] << 16) | ((uint32_t)x[3] << 24);
}

// Centered binomial distribution, eta = 2: 128 octets of PRF output -> 256 coefficients in [-2, 2].
static void cbd2(int16_t r[MK_N], const uint8_t buf[128])
{
    for (unsigned i = 0; i < MK_N / 8; i++)
    {
        uint32_t t = load32_le(buf + 4 * i);
        uint32_t d = t & 0x55555555u;
        d += (t >> 1) & 0x55555555u;
        for (unsigned j = 0; j < 8; j++)
        {
            int16_t a = (int16_t)((d >> (4 * j)) & 0x3);
            int16_t b = (int16_t)((d >> (4 * j + 2)) & 0x3);
            r[8 * i + j] = (int16_t)(a - b);
        }
    }
}

// PRF_eta(seed, nonce) = SHAKE256(seed || nonce), then sample CBD_eta (eta = 2).
static void poly_getnoise(int16_t r[MK_N], const uint8_t seed[32], uint8_t nonce)
{
    uint8_t extseed[33];
    memcpy(extseed, seed, 32);
    extseed[32] = nonce;
    uint8_t buf[MK_ETA * MK_N / 4]; // 128
    shake256(buf, sizeof(buf), extseed, sizeof(extseed));
    cbd2(r, buf);
}

// One transposed matrix entry: A^T[i][j] = SampleNTT(XOF(rho || i || j)) (FIPS 203 gen with (i,j)).
static void gen_matrix_entry(int16_t out[MK_N], const uint8_t rho[32], uint8_t i, uint8_t j)
{
    uint8_t seed[34];
    memcpy(seed, rho, 32);
    seed[32] = i;
    seed[33] = j;
    KeccakCtx ctx;
    shake128_absorb(&ctx, seed, sizeof(seed));

    unsigned count = 0;
    while (count < MK_N)
    {
        uint8_t buf[KECCAK_RATE_SHAKE128]; // 168 = 56*3, no 3-octet group straddles a block
        keccak_squeeze(&ctx, buf, sizeof(buf));
        for (unsigned p = 0; p + 3 <= sizeof(buf) && count < MK_N; p += 3)
        {
            uint16_t d1 = (uint16_t)(buf[p] | ((uint16_t)(buf[p + 1] & 0xF) << 8));
            uint16_t d2 = (uint16_t)((buf[p + 1] >> 4) | ((uint16_t)buf[p + 2] << 4));
            if (d1 < MK_Q)
                out[count++] = (int16_t)d1;
            if (count < MK_N && d2 < MK_Q)
                out[count++] = (int16_t)d2;
        }
    }
}

// Compress_10 + ByteEncode_10 for one polynomial: 256 coefficients -> 320 octets.
static void poly_compress10(uint8_t r[320], const int16_t a[MK_N])
{
    unsigned k = 0;
    for (unsigned i = 0; i < MK_N / 4; i++)
    {
        uint16_t t[4];
        for (unsigned j = 0; j < 4; j++)
        {
            int16_t u = a[4 * i + j];
            u = (int16_t)(u + ((u >> 15) & MK_Q)); // to [0, q)
            t[j] = (uint16_t)(((((uint32_t)u << 10) + MK_Q / 2) / MK_Q) & 0x3FF);
        }
        r[k + 0] = (uint8_t)(t[0]);
        r[k + 1] = (uint8_t)((t[0] >> 8) | (t[1] << 2));
        r[k + 2] = (uint8_t)((t[1] >> 6) | (t[2] << 4));
        r[k + 3] = (uint8_t)((t[2] >> 4) | (t[3] << 6));
        r[k + 4] = (uint8_t)(t[3] >> 2);
        k += 5;
    }
}

// Compress_4 + ByteEncode_4 for one polynomial: 256 coefficients -> 128 octets.
static void poly_compress4(uint8_t r[128], const int16_t a[MK_N])
{
    for (unsigned i = 0; i < MK_N / 8; i++)
    {
        uint8_t t[8];
        for (unsigned j = 0; j < 8; j++)
        {
            int16_t u = a[8 * i + j];
            u = (int16_t)(u + ((u >> 15) & MK_Q));
            t[j] = (uint8_t)(((((uint16_t)u << 4) + MK_Q / 2) / MK_Q) & 15);
        }
        r[4 * i + 0] = (uint8_t)(t[0] | (t[1] << 4));
        r[4 * i + 1] = (uint8_t)(t[2] | (t[3] << 4));
        r[4 * i + 2] = (uint8_t)(t[4] | (t[5] << 4));
        r[4 * i + 3] = (uint8_t)(t[6] | (t[7] << 4));
    }
}

// FIPS 203 modulus check: ek's decoded coefficients must all be < q.
static bool check_ek(const uint8_t ek[MLKEM768_EK_BYTES])
{
    for (unsigned i = 0; i < MK_K; i++)
    {
        int16_t p[MK_N];
        poly_frombytes(p, ek + i * MK_POLYBYTES);
        for (unsigned j = 0; j < MK_N; j++)
            if ((uint16_t)p[j] >= MK_Q)
                return false;
    }
    return true;
}

// K-PKE.Encrypt(ek, m, r) -> ct. u is streamed and compressed one row at a time to bound stack.
static void k_pke_encrypt(uint8_t ct[MLKEM768_CT_BYTES], const uint8_t ek[MLKEM768_EK_BYTES], const uint8_t m[32],
                          const uint8_t coins[32])
{
    int16_t that[MK_K][MK_N];
    for (unsigned i = 0; i < MK_K; i++)
        poly_frombytes(that[i], ek + i * MK_POLYBYTES);
    const uint8_t *rho = ek + MK_K * MK_POLYBYTES;

    int16_t sp[MK_K][MK_N];
    for (unsigned i = 0; i < MK_K; i++)
    {
        poly_getnoise(sp[i], coins, (uint8_t)i); // y, nonce 0..k-1
        ntt(sp[i]);
    }

    // u = NTT^-1(A^T o y) + e1, compressed with du = 10 (320 octets/row).
    for (unsigned i = 0; i < MK_K; i++)
    {
        int16_t at_row[MK_K][MK_N];
        for (unsigned j = 0; j < MK_K; j++)
            gen_matrix_entry(at_row[j], rho, (uint8_t)i, (uint8_t)j);
        int16_t u_row[MK_N];
        polyvec_basemul_acc(u_row, at_row, sp);
        invntt(u_row);
        int16_t e1[MK_N];
        poly_getnoise(e1, coins, (uint8_t)(MK_K + i)); // e1, nonce k..2k-1
        for (unsigned x = 0; x < MK_N; x++)
            u_row[x] = barrett_reduce((int16_t)(u_row[x] + e1[x]));
        poly_compress10(ct + i * 320, u_row);
    }

    // v = NTT^-1(t^T o y) + e2 + Decompress_1(m), compressed with dv = 4 (128 octets).
    int16_t v[MK_N];
    polyvec_basemul_acc(v, that, sp);
    invntt(v);
    int16_t e2[MK_N];
    poly_getnoise(e2, coins, (uint8_t)(2 * MK_K)); // e2, nonce 2k
    int16_t mu[MK_N];
    poly_frommsg(mu, m);
    for (unsigned x = 0; x < MK_N; x++)
        v[x] = barrett_reduce((int16_t)(v[x] + e2[x] + mu[x]));
    poly_compress4(ct + MK_K * 320, v);
}

bool mlkem768_encaps(const uint8_t ek[MLKEM768_EK_BYTES], const uint8_t m[MLKEM768_MSG_BYTES],
                     uint8_t ct[MLKEM768_CT_BYTES], uint8_t ss[MLKEM768_SS_BYTES])
{
    if (!check_ek(ek))
        return false;

    // (K, r) = G(m || H(ek)); ss = K.
    uint8_t g_in[64];
    memcpy(g_in, m, 32);
    sha3_256(g_in + 32, ek, MLKEM768_EK_BYTES); // H(ek)
    uint8_t g_out[64];
    sha3_512(g_out, g_in, sizeof(g_in));
    memcpy(ss, g_out, 32);

    k_pke_encrypt(ct, ek, m, g_out + 32);
    return true;
}

#endif // DETWS_ENABLE_PQC_KEX
