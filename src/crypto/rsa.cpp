// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file rsa.cpp
 * @brief RSA-2048 PKCS#1 v1.5 verify + software sign (see rsa.h).
 *
 * Verify: mbedtls on Arduino (hardware/mbedTLS), software full-width modexp on native. Software sign
 * (native) is the test reference path. Protocol-agnostic - raw big-endian key material in, no SSH.
 */

#include "crypto/rsa.h"
#include "crypto/sha256.h"
#include "crypto/sha512.h"
#include <string.h>

// ---------------------------------------------------------------------------
// DigestInfo for SHA-256 / SHA-512 (PKCS#1 v1.5, RFC 8017 §9.2, RFC 5754)
// ---------------------------------------------------------------------------

const uint8_t dws_pkcs1_sha256_digestinfo[DWS_PKCS1_DIGESTINFO_LEN] = {
    0x30, 0x31,                                           // SEQUENCE, length 49
    0x30, 0x0d,                                           // SEQUENCE, length 13 (AlgorithmIdentifier)
    0x06, 0x09,                                           // OID, length 9
    0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, // OID 2.16.840.1.101.3.4.2.1
    0x05, 0x00,                                           // NULL parameters
    0x04, 0x20                                            // OCTET STRING, length 32 (digest follows)
};

const uint8_t dws_pkcs1_sha512_digestinfo[DWS_PKCS1_SHA512_DIGESTINFO_LEN] = {
    0x30, 0x51,                                           // SEQUENCE, length 81
    0x30, 0x0d,                                           // SEQUENCE, length 13 (AlgorithmIdentifier)
    0x06, 0x09,                                           // OID, length 9
    0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03, // OID 2.16.840.1.101.3.4.2.3
    0x05, 0x00,                                           // NULL parameters
    0x04, 0x40                                            // OCTET STRING, length 64 (digest follows)
};

#ifdef ARDUINO

// ---------------------------------------------------------------------------
// Arduino - mbedtls verify path
// ---------------------------------------------------------------------------

#include <mbedtls/md.h>
#include <mbedtls/rsa.h>

int dws_rsa_verify(const uint8_t n_be[DWS_RSA_KEY_BYTES], const uint8_t e_be4[4], const uint8_t *msg, size_t msg_len,
                   const uint8_t *sig, size_t sig_len, DwsRsaHash hash)
{
    if (sig_len != DWS_RSA_KEY_BYTES)
        return -1;

    mbedtls_rsa_context rsa;
#if MBEDTLS_VERSION_MAJOR >= 3
    mbedtls_rsa_init(&rsa);
#else
    mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V15, 0);
#endif

    mbedtls_mpi N;
    mbedtls_mpi E;
    mbedtls_mpi_init(&N);
    mbedtls_mpi_init(&E);
    mbedtls_mpi_read_binary(&N, n_be, DWS_RSA_KEY_BYTES);
    mbedtls_mpi_read_binary(&E, e_be4, 4);

    int rc = mbedtls_rsa_import(&rsa, &N, nullptr, nullptr, nullptr, &E);
    if (rc == 0)
        rc = mbedtls_rsa_complete(&rsa);

    const bool sha512 = (hash == DwsRsaHash::SHA512);
    const mbedtls_md_type_t md = sha512 ? MBEDTLS_MD_SHA512 : MBEDTLS_MD_SHA256;
    const size_t dlen = sha512 ? DWS_SHA512_DIGEST_LEN : DWS_SHA256_DIGEST_LEN;
    uint8_t digest[DWS_SHA512_DIGEST_LEN];
    if (rc == 0)
    {
        if (sha512)
            dws_sha512(msg, msg_len, digest);
        else
            dws_sha256(msg, msg_len, digest);
#if MBEDTLS_VERSION_MAJOR >= 3
        rc = mbedtls_rsa_pkcs1_verify(&rsa, md, dlen, digest, sig);
#else
        rc = mbedtls_rsa_pkcs1_verify(&rsa, nullptr, nullptr, MBEDTLS_RSA_PUBLIC, md, dlen, digest, sig);
#endif
    }

    mbedtls_mpi_free(&N);
    mbedtls_mpi_free(&E);
    mbedtls_rsa_free(&rsa);
    return rc == 0 ? 0 : -1;
}

#else

// ---------------------------------------------------------------------------
// Native - software RSA path (test reference; NOT constant-time)
// ---------------------------------------------------------------------------

#include "crypto/bignum.h"

// Secure-zero that the optimizer may not elide (crypto/ stays independent of the SSH wipe util).
static void secure_zero(void *p, size_t n)
{
    volatile uint8_t *v = (volatile uint8_t *)p;
    while (n--)
        *v++ = 0;
}

// Hash msg with the selected algorithm and return the matching DigestInfo.
//   digest must be >= DWS_SHA512_DIGEST_LEN bytes.
static void rsa_digest(const uint8_t *msg, size_t msg_len, DwsRsaHash hash, uint8_t digest[DWS_SHA512_DIGEST_LEN],
                       size_t *digest_len, const uint8_t **di, size_t *di_len)
{
    if (hash == DwsRsaHash::SHA512)
    {
        dws_sha512(msg, msg_len, digest);
        *digest_len = DWS_SHA512_DIGEST_LEN;
        *di = dws_pkcs1_sha512_digestinfo;
        *di_len = DWS_PKCS1_SHA512_DIGESTINFO_LEN;
    }
    else
    {
        dws_sha256(msg, msg_len, digest);
        *digest_len = DWS_SHA256_DIGEST_LEN;
        *di = dws_pkcs1_sha256_digestinfo;
        *di_len = DWS_PKCS1_DIGESTINFO_LEN;
    }
}

// Builds the 256-byte padded message:
//   0x00 0x01 [pad × 0xFF] 0x00 [DigestInfo] [digest]
static void pkcs1v15_encode(const uint8_t *digest, size_t digest_len, const uint8_t *di, size_t di_len,
                            uint8_t em[DWS_RSA_KEY_BYTES])
{
    const size_t total = di_len + digest_len;
    const size_t pad_len = DWS_RSA_KEY_BYTES - 3 - total;
    em[0] = 0x00;
    em[1] = 0x01;
    memset(em + 2, 0xFF, pad_len);
    em[2 + pad_len] = 0x00;
    memcpy(em + 3 + pad_len, di, di_len);
    memcpy(em + 3 + pad_len + di_len, digest, digest_len);
}

// ---------------------------------------------------------------------------
// Native full-width modular arithmetic (schoolbook multiply + bit-serial
// reduction: full-width and correct, but NOT constant-time - native is test-only).
// ---------------------------------------------------------------------------

// Full 128-limb product of two 64-limb little-endian integers.
static void bn_mul_full(const uint32_t a[DWS_BN_LIMBS], const uint32_t b[DWS_BN_LIMBS], uint32_t p[2 * DWS_BN_LIMBS])
{
    for (int k = 0; k < 2 * DWS_BN_LIMBS; k++)
        p[k] = 0;
    for (int i = 0; i < DWS_BN_LIMBS; i++)
    {
        uint64_t carry = 0;
        for (int j = 0; j < DWS_BN_LIMBS; j++)
        {
            uint64_t cur = (uint64_t)p[i + j] + (uint64_t)a[i] * b[j] + carry;
            p[i + j] = (uint32_t)cur;
            carry = cur >> 32;
        }
        int k = i + DWS_BN_LIMBS;
        // a and b are both DWS_BN_LIMBS (64) limbs, so their full product is bounded by 2*DWS_BN_LIMBS
        // (128) limbs; carry propagation out of the top half can never still be pending when k reaches
        // 2*DWS_BN_LIMBS. The "k < 2*DWS_BN_LIMBS" half of this guard is defensive and provably unreachable.
        while (carry && k < 2 * DWS_BN_LIMBS) // GCOVR_EXCL_BR_LINE
        {
            uint64_t cur = (uint64_t)p[k] + carry;
            p[k] = (uint32_t)cur;
            carry = cur >> 32;
            k++;
        }
    }
}

// Reduce a 128-limb value mod a 64-limb modulus, bit-serial. out = p mod m.
static void bn_reduce_full(const uint32_t p[2 * DWS_BN_LIMBS], const uint32_t m[DWS_BN_LIMBS],
                           uint32_t out[DWS_BN_LIMBS])
{
    uint32_t r[DWS_BN_LIMBS + 1];
    for (int k = 0; k <= DWS_BN_LIMBS; k++)
        r[k] = 0;

    for (int bit = 2 * DWS_BN_LIMBS * 32 - 1; bit >= 0; bit--)
    {
        uint32_t carry = 0;
        for (int k = 0; k <= DWS_BN_LIMBS; k++)
        {
            uint32_t nc = r[k] >> 31;
            r[k] = (r[k] << 1) | carry;
            carry = nc;
        }
        r[0] |= (p[bit >> 5] >> (bit & 31)) & 1u;

        bool ge = r[DWS_BN_LIMBS] != 0;
        if (!ge)
        {
            ge = true;
            for (int k = DWS_BN_LIMBS - 1; k >= 0; k--)
            {
                if (r[k] != m[k])
                {
                    ge = (r[k] > m[k]);
                    break;
                }
            }
        }
        if (ge)
        {
            uint64_t borrow = 0;
            for (int k = 0; k < DWS_BN_LIMBS; k++)
            {
                uint64_t v = (uint64_t)r[k] - m[k] - borrow;
                r[k] = (uint32_t)v;
                borrow = (v >> 32) & 1u;
            }
            r[DWS_BN_LIMBS] -= (uint32_t)borrow;
        }
    }
    for (int k = 0; k < DWS_BN_LIMBS; k++)
        out[k] = r[k];
}

// out = base^e mod n, e a small public exponent.
static void bn_modexp_pub(const DwsBigNum *base, uint32_t e, const DwsBigNum *n, DwsBigNum *out)
{
    uint32_t prod[2 * DWS_BN_LIMBS];

    DwsBigNum b;
    for (int k = 0; k < DWS_BN_LIMBS; k++)
    {
        prod[k] = base->d[k];
        prod[k + DWS_BN_LIMBS] = 0;
    }
    bn_reduce_full(prod, n->d, b.d);

    DwsBigNum r;
    memset(r.d, 0, sizeof(r.d));
    r.d[0] = 1; // r = 1

    int top = 31;
    while (top >= 0 && !((e >> top) & 1u))
        top--;
    for (int i = top; i >= 0; i--)
    {
        bn_mul_full(r.d, r.d, prod); // r = r^2 mod n
        bn_reduce_full(prod, n->d, r.d);
        if ((e >> i) & 1u)
        {
            bn_mul_full(r.d, b.d, prod); // r = r*base mod n
            bn_reduce_full(prod, n->d, r.d);
        }
    }
    *out = r;
}

// out = base^exp mod n, exp a full-width 2048-bit private exponent.
static void bn_modexp_full(const DwsBigNum *base, const DwsBigNum *exp, const DwsBigNum *n, DwsBigNum *out)
{
    uint32_t prod[2 * DWS_BN_LIMBS];

    DwsBigNum b;
    for (int k = 0; k < DWS_BN_LIMBS; k++)
    {
        prod[k] = base->d[k];
        prod[k + DWS_BN_LIMBS] = 0;
    }
    bn_reduce_full(prod, n->d, b.d);

    DwsBigNum r;
    memset(r.d, 0, sizeof(r.d));
    r.d[0] = 1; // r = 1

    int top_limb = DWS_BN_LIMBS - 1;
    while (top_limb >= 0 && exp->d[top_limb] == 0)
        top_limb--;
    if (top_limb < 0)
    {
        *out = r; // exp == 0 -> result is 1
        return;
    }
    int top_bit = 31;
    // exp->d[top_limb] != 0 by construction, so this scan finds a set bit before top_bit passes 0;
    // the "top_bit >= 0" half of the guard is defensive, not reachable.
    while (top_bit >= 0 && !((exp->d[top_limb] >> top_bit) & 1u)) // GCOVR_EXCL_BR_LINE
        top_bit--;

    for (int limb = top_limb; limb >= 0; limb--)
    {
        int start = (limb == top_limb) ? top_bit : 31;
        for (int bit = start; bit >= 0; bit--)
        {
            bn_mul_full(r.d, r.d, prod); // r = r^2 mod n
            bn_reduce_full(prod, n->d, r.d);
            if ((exp->d[limb] >> bit) & 1u)
            {
                bn_mul_full(r.d, b.d, prod); // r = r*base mod n
                bn_reduce_full(prod, n->d, r.d);
            }
        }
    }
    *out = r;
}

int dws_rsa_sign_sw(const uint8_t n_be[DWS_RSA_KEY_BYTES], const uint8_t d_be[DWS_RSA_KEY_BYTES], const uint8_t *msg,
                    size_t msg_len, DwsRsaHash hash, uint8_t sig[DWS_RSA_SIG_BYTES])
{
    // 1. SHA-256/512 digest of the message + matching DigestInfo.
    uint8_t digest[DWS_SHA512_DIGEST_LEN];
    size_t digest_len = 0;
    const uint8_t *di = nullptr;
    size_t di_len = 0;
    rsa_digest(msg, msg_len, hash, digest, &digest_len, &di, &di_len);

    // 2. PKCS#1 v1.5 encode: 0x00 0x01 0xFF... 0x00 DigestInfo digest
    uint8_t em[DWS_RSA_KEY_BYTES];
    pkcs1v15_encode(digest, digest_len, di, di_len, em);
    secure_zero(digest, sizeof(digest));

    // 3. RSA private-key operation: s = em^d mod n (full-width).
    DwsBigNum n_bn;
    DwsBigNum d_bn;
    DwsBigNum m_bn;
    DwsBigNum s_bn;
    bn_from_bytes(&n_bn, n_be, DWS_RSA_KEY_BYTES);
    bn_from_bytes(&d_bn, d_be, DWS_RSA_KEY_BYTES);
    bn_from_bytes(&m_bn, em, DWS_RSA_KEY_BYTES);
    secure_zero(em, sizeof(em));

    bn_modexp_full(&m_bn, &d_bn, &n_bn, &s_bn);

    bn_to_bytes(sig, &s_bn);

    secure_zero(&n_bn, sizeof(n_bn));
    secure_zero(&d_bn, sizeof(d_bn));
    secure_zero(&m_bn, sizeof(m_bn));
    secure_zero(&s_bn, sizeof(s_bn));
    return 0;
}

int dws_rsa_verify(const uint8_t n_be[DWS_RSA_KEY_BYTES], const uint8_t e_be4[4], const uint8_t *msg, size_t msg_len,
                   const uint8_t *sig, size_t sig_len, DwsRsaHash hash)
{
    if (sig_len != DWS_RSA_KEY_BYTES)
        return -1;

    DwsBigNum n;
    DwsBigNum s;
    DwsBigNum m;
    bn_from_bytes(&n, n_be, DWS_RSA_KEY_BYTES);
    bn_from_bytes(&s, sig, DWS_RSA_KEY_BYTES);
    if (bn_cmp(&s, &n) >= 0)
        return -1; // signature must be reduced mod n

    uint32_t e = ((uint32_t)e_be4[0] << 24) | ((uint32_t)e_be4[1] << 16) | ((uint32_t)e_be4[2] << 8) | e_be4[3];
    bn_modexp_pub(&s, e, &n, &m);

    uint8_t em[DWS_RSA_KEY_BYTES];
    bn_to_bytes(em, &m);

    // Recompute the expected PKCS#1 v1.5 block and compare in constant time.
    uint8_t digest[DWS_SHA512_DIGEST_LEN];
    size_t digest_len = 0;
    const uint8_t *di = nullptr;
    size_t di_len = 0;
    rsa_digest(msg, msg_len, hash, digest, &digest_len, &di, &di_len);
    uint8_t expected[DWS_RSA_KEY_BYTES];
    pkcs1v15_encode(digest, digest_len, di, di_len, expected);

    uint8_t diff = 0;
    for (size_t k = 0; k < DWS_RSA_KEY_BYTES; k++)
        diff |= (uint8_t)(em[k] ^ expected[k]);
    return diff == 0 ? 0 : -1;
}

#endif // ARDUINO
