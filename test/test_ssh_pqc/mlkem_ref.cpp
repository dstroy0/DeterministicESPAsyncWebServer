// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Independent ML-KEM-768 Decaps (test client side). Mirrors the pq-crystals reference structure; it
// deliberately duplicates the NTT/poly machinery rather than reaching into the src static internals so
// the test is a genuine second implementation. Validated against the pinned KAT in test_ssh_pqc.

#include "mlkem_ref.h"
#include "network_drivers/presentation/pqc/sha3.h"
#include <string.h>

#define N 256
#define Q 3329
#define K 3
#define QINV (-3327)

static const int16_t zetas[128] = {
    -1044, -758,  -359,  -1517, 1493,  1422,  287,   202,   -171,  622,  1577,  182,   962,   -1202, -1474, 1468,
    573,   -1325, 264,   383,   -829,  1458,  -1602, -130,  -681,  1017, 732,   608,   -1542, 411,   -205,  -1571,
    1223,  652,   -552,  1015,  -1293, 1491,  -282,  -1544, 516,   -8,   -320,  -666,  -1618, -1162, 126,   1469,
    -853,  -90,   -271,  830,   107,   -1421, -247,  -951,  -398,  961,  -1508, -725,  448,   -1065, 677,   -1275,
    -1103, 430,   555,   843,   -1251, 871,   1550,  105,   422,   587,  177,   -235,  -291,  -460,  1574,  1653,
    -246,  778,   1159,  -147,  -777,  1483,  -602,  1119,  -1590, 644,  -872,  349,   418,   329,   -156,  -75,
    817,   1097,  603,   610,   1322,  -1285, -1465, 384,   -1215, -136, 1218,  -1335, -874,  220,   -1187, -1659,
    -1185, -1530, -1278, 794,   -1510, -854,  -870,  478,   -108,  -308, 996,   991,   958,   -1460, 1522,  1628};

static int16_t montred(int32_t a)
{
    int16_t t = (int16_t)((int16_t)a * (int16_t)QINV);
    return (int16_t)((a - (int32_t)t * Q) >> 16);
}
static inline int16_t fqmul(int16_t a, int16_t b)
{
    return montred((int32_t)a * b);
}
static int16_t barrett(int16_t a)
{
    const int16_t v = 20159;
    int16_t t = (int16_t)(((int32_t)v * a + (1 << 25)) >> 26);
    return (int16_t)(a - (int16_t)(t * Q));
}
static void ntt(int16_t r[N])
{
    unsigned k = 1;
    for (unsigned len = 128; len >= 2; len >>= 1)
        for (unsigned start = 0; start < N; start += (len << 1))
        {
            int16_t z = zetas[k++];
            for (unsigned j = start; j < start + len; j++)
            {
                int16_t t = fqmul(z, r[j + len]);
                r[j + len] = (int16_t)(r[j] - t);
                r[j] = (int16_t)(r[j] + t);
            }
        }
}
static void invntt(int16_t r[N])
{
    const int16_t f = 1441;
    unsigned k = 127;
    for (unsigned len = 2; len <= 128; len <<= 1)
        for (unsigned start = 0; start < N; start += (len << 1))
        {
            int16_t z = zetas[k--];
            for (unsigned j = start; j < start + len; j++)
            {
                int16_t t = r[j];
                r[j] = barrett((int16_t)(t + r[j + len]));
                r[j + len] = (int16_t)(r[j + len] - t);
                r[j + len] = fqmul(z, r[j + len]);
            }
        }
    for (unsigned j = 0; j < N; j++)
        r[j] = fqmul(r[j], f);
}
static void basemul(int16_t r[2], const int16_t a[2], const int16_t b[2], int16_t z)
{
    r[0] = fqmul(a[1], b[1]);
    r[0] = fqmul(r[0], z);
    r[0] = (int16_t)(r[0] + fqmul(a[0], b[0]));
    r[1] = fqmul(a[0], b[1]);
    r[1] = (int16_t)(r[1] + fqmul(a[1], b[0]));
}
static void polyvec_basemul_acc(int16_t r[N], const int16_t a[K][N], const int16_t b[K][N])
{
    int16_t t[N];
    for (unsigned p = 0; p < N / 4; p++)
        basemul(&r[4 * p], &a[0][4 * p], &b[0][4 * p], zetas[64 + p]),
            basemul(&r[4 * p + 2], &a[0][4 * p + 2], &b[0][4 * p + 2], (int16_t)(-zetas[64 + p]));
    for (unsigned i = 1; i < K; i++)
    {
        for (unsigned p = 0; p < N / 4; p++)
            basemul(&t[4 * p], &a[i][4 * p], &b[i][4 * p], zetas[64 + p]),
                basemul(&t[4 * p + 2], &a[i][4 * p + 2], &b[i][4 * p + 2], (int16_t)(-zetas[64 + p]));
        for (unsigned j = 0; j < N; j++)
            r[j] = (int16_t)(r[j] + t[j]);
    }
    for (unsigned j = 0; j < N; j++)
        r[j] = barrett(r[j]);
}
static void poly_frombytes(int16_t r[N], const uint8_t a[384])
{
    for (unsigned i = 0; i < N / 2; i++)
    {
        r[2 * i] = (int16_t)(((a[3 * i] >> 0) | ((uint16_t)a[3 * i + 1] << 8)) & 0xFFF);
        r[2 * i + 1] = (int16_t)(((a[3 * i + 1] >> 4) | ((uint16_t)a[3 * i + 2] << 4)) & 0xFFF);
    }
}
static void decompress10(int16_t r[N], const uint8_t a[320])
{
    for (unsigned i = 0; i < N / 4; i++)
    {
        uint16_t t[4];
        t[0] = (uint16_t)((a[0] >> 0) | ((uint16_t)a[1] << 8));
        t[1] = (uint16_t)((a[1] >> 2) | ((uint16_t)a[2] << 6));
        t[2] = (uint16_t)((a[2] >> 4) | ((uint16_t)a[3] << 4));
        t[3] = (uint16_t)((a[3] >> 6) | ((uint16_t)a[4] << 2));
        a += 5;
        for (unsigned j = 0; j < 4; j++)
            r[4 * i + j] = (int16_t)((((uint32_t)(t[j] & 0x3FF) * Q) + 512) >> 10);
    }
}
static void decompress4(int16_t r[N], const uint8_t a[128])
{
    for (unsigned i = 0; i < N / 2; i++)
    {
        r[2 * i] = (int16_t)(((uint16_t)(a[i] & 15) * Q + 8) >> 4);
        r[2 * i + 1] = (int16_t)(((uint16_t)(a[i] >> 4) * Q + 8) >> 4);
    }
}
static void poly_tomsg(uint8_t msg[32], const int16_t a[N])
{
    for (unsigned i = 0; i < N / 8; i++)
    {
        msg[i] = 0;
        for (unsigned j = 0; j < 8; j++)
        {
            int16_t t = a[8 * i + j];
            t = (int16_t)(t + ((t >> 15) & Q));
            t = (int16_t)((((t << 1) + Q / 2) / Q) & 1);
            msg[i] = (uint8_t)(msg[i] | (t << j));
        }
    }
}

void mlkem768_decaps_ref(const uint8_t dk[2400], const uint8_t ct[1088], uint8_t ss[32])
{
    // dk = dk_pke(1152) || ek(1184) || H(ek)(32) || z(32); s_hat is stored in NTT domain.
    int16_t shat[K][N];
    for (unsigned i = 0; i < K; i++)
        poly_frombytes(shat[i], dk + i * 384);

    int16_t u[K][N];
    for (unsigned i = 0; i < K; i++)
    {
        decompress10(u[i], ct + i * 320);
        ntt(u[i]);
    }
    int16_t v[N];
    decompress4(v, ct + K * 320);

    int16_t w[N];
    polyvec_basemul_acc(w, shat, u);
    invntt(w);
    for (unsigned j = 0; j < N; j++)
        w[j] = (int16_t)(v[j] - w[j]); // m-carrying polynomial = v - s^T u

    uint8_t m2[32];
    poly_tomsg(m2, w);

    const uint8_t *h = dk + 1152 + 1184; // H(ek)
    uint8_t gin[64];
    memcpy(gin, m2, 32);
    memcpy(gin + 32, h, 32);
    uint8_t gout[64];
    sha3_512(gout, gin, sizeof(gin)); // (K, r) = G(m' || H(ek)); shared secret = K
    memcpy(ss, gout, 32);
}
