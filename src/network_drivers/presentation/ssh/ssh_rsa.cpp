// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_rsa.cpp
 * @brief RSA-SHA2-256 host-key signing (stack-only private key, PKCS#1 v1.5).
 */

#include "ssh_rsa.h"
#include "ssh_keymat.h"
#include <string.h>

// ---------------------------------------------------------------------------
// DigestInfo for SHA-256 (PKCS#1 v1.5, RFC 8017 §9.2)
// ---------------------------------------------------------------------------

const uint8_t ssh_pkcs1_sha256_digestinfo[SSH_PKCS1_DIGESTINFO_LEN] = {
    0x30, 0x31,                                           // SEQUENCE, length 49
    0x30, 0x0d,                                           // SEQUENCE, length 13 (AlgorithmIdentifier)
    0x06, 0x09,                                           // OID, length 9
    0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, // OID 2.16.840.1.101.3.4.2.1
    0x05, 0x00,                                           // NULL parameters
    0x04, 0x20                                            // OCTET STRING, length 32 (digest follows)
};

// ---------------------------------------------------------------------------
// Public host key (BSS - no secret material)
// ---------------------------------------------------------------------------

SshRsaPubKey ssh_host_pubkey;

// ---------------------------------------------------------------------------
// PKCS#1 v1.5 pad-and-encode
// ---------------------------------------------------------------------------

// Builds the 256-byte padded message:
//   0x00 0x01 [202 × 0xFF] 0x00 [19-byte DigestInfo] [32-byte digest]
// Returns 0 on success.
static int pkcs1v15_encode(const uint8_t digest[SSH_SHA256_DIGEST_LEN], uint8_t em[SSH_RSA_KEY_BYTES])
{
    // DigestInfo total = 19 + 32 = 51 bytes
    // Padding = 256 - 3 - 51 = 202 bytes of 0xFF
    const size_t di_len = SSH_PKCS1_DIGESTINFO_LEN + SSH_SHA256_DIGEST_LEN; // 51
    const size_t pad_len = SSH_RSA_KEY_BYTES - 3 - di_len;                  // 202

    em[0] = 0x00;
    em[1] = 0x01;
    memset(em + 2, 0xFF, pad_len);
    em[2 + pad_len] = 0x00;
    memcpy(em + 3 + pad_len, ssh_pkcs1_sha256_digestinfo, SSH_PKCS1_DIGESTINFO_LEN);
    memcpy(em + 3 + pad_len + SSH_PKCS1_DIGESTINFO_LEN, digest, SSH_SHA256_DIGEST_LEN);
    return 0;
}

// ---------------------------------------------------------------------------
// Arduino - mbedtls path
// ---------------------------------------------------------------------------

#ifdef ARDUINO

#include <Preferences.h> // ESP-IDF NVS wrapper
#include <mbedtls/md.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>

// Load the NVS DER blob into a stack buffer, parse it with mbedtls, and
// extract n and e into ssh_host_pubkey.
int ssh_rsa_load_pubkey(void)
{
    Preferences prefs;
    if (!prefs.begin("ssh_host_key", true))
        return -1;

    uint8_t der[SSH_RSA_KEY_DER_MAX];
    size_t der_len = prefs.getBytesLength("priv_der");
    if (der_len == 0 || der_len > SSH_RSA_KEY_DER_MAX)
    {
        prefs.end();
        return -1;
    }
    prefs.getBytes("priv_der", der, der_len);
    prefs.end();

    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);
    int rc = mbedtls_pk_parse_key(&pk, der, der_len, nullptr, 0
#if MBEDTLS_VERSION_MAJOR >= 3
                                  ,
                                  nullptr, 0
#endif
    );
    // Wipe the DER stack buffer whether or not parse succeeded.
    ssh_wipe(der, der_len);

    if (rc != 0)
    {
        mbedtls_pk_free(&pk);
        return -1;
    }

    mbedtls_rsa_context *rsa = mbedtls_pk_rsa(pk);
    if (mbedtls_rsa_get_len(rsa) != SSH_RSA_KEY_BYTES)
    {
        mbedtls_pk_free(&pk);
        return -1;
    }

    // Write n and e into the public-only BSS struct.
    mbedtls_mpi n_mpi, e_mpi;
    mbedtls_mpi_init(&n_mpi);
    mbedtls_mpi_init(&e_mpi);
    mbedtls_rsa_export(rsa, &n_mpi, nullptr, nullptr, nullptr, &e_mpi);
    mbedtls_mpi_write_binary(&n_mpi, ssh_host_pubkey.n, SSH_RSA_KEY_BYTES);
    uint32_t e_val = (uint32_t)mbedtls_mpi_get_bit(&e_mpi, 0) | ((uint32_t)mbedtls_mpi_get_bit(&e_mpi, 16) << 16);
    // Full e value
    mbedtls_mpi_write_binary(&e_mpi, ssh_host_pubkey.e_bytes + 4 - sizeof(ssh_host_pubkey.e_bytes),
                             sizeof(ssh_host_pubkey.e_bytes));
    mbedtls_mpi_free(&n_mpi);
    mbedtls_mpi_free(&e_mpi);
    mbedtls_pk_free(&pk);

    ssh_host_pubkey.loaded = true;
    (void)e_val;
    return 0;
}

int ssh_rsa_sign(const uint8_t *msg, size_t msg_len, uint8_t sig[SSH_RSA_SIG_BYTES])
{
    // 1. Load private key from NVS directly into a stack-local context.
    //    mbedtls_pk_context / mbedtls_rsa_context heap-allocate their MPI
    //    limbs internally; we free them immediately after signing.
    Preferences prefs;
    if (!prefs.begin("ssh_host_key", true))
        return -1;

    uint8_t der[SSH_RSA_KEY_DER_MAX];
    size_t der_len = prefs.getBytesLength("priv_der");
    if (der_len == 0 || der_len > SSH_RSA_KEY_DER_MAX)
    {
        prefs.end();
        return -1;
    }
    prefs.getBytes("priv_der", der, der_len);
    prefs.end();

    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);
    int rc = mbedtls_pk_parse_key(&pk, der, der_len, nullptr, 0
#if MBEDTLS_VERSION_MAJOR >= 3
                                  ,
                                  nullptr, 0
#endif
    );
    ssh_wipe(der, der_len); // wipe the stack DER copy immediately
    if (rc != 0)
    {
        mbedtls_pk_free(&pk);
        return -1;
    }

    // 2. Sign: mbedtls handles SHA-256 hashing + PKCS#1 v1.5 pad internally.
    size_t sig_len = 0;
    rc = mbedtls_pk_sign(&pk, MBEDTLS_MD_SHA256, msg, msg_len, sig, SSH_RSA_SIG_BYTES, &sig_len, nullptr, nullptr);
    mbedtls_pk_free(&pk); // frees all MPI limb memory (private key)

    return (rc == 0 && sig_len == SSH_RSA_SIG_BYTES) ? 0 : -1;
}

int ssh_rsa_verify(const uint8_t n_be[SSH_RSA_KEY_BYTES], const uint8_t e_be4[4], const uint8_t *msg, size_t msg_len,
                   const uint8_t *sig, size_t sig_len)
{
    if (sig_len != SSH_RSA_KEY_BYTES)
        return -1;

    mbedtls_rsa_context rsa;
    mbedtls_rsa_init(&rsa);

    mbedtls_mpi N, E;
    mbedtls_mpi_init(&N);
    mbedtls_mpi_init(&E);
    mbedtls_mpi_read_binary(&N, n_be, SSH_RSA_KEY_BYTES);
    mbedtls_mpi_read_binary(&E, e_be4, 4);

    int rc = mbedtls_rsa_import(&rsa, &N, nullptr, nullptr, nullptr, &E);
    if (rc == 0)
        rc = mbedtls_rsa_complete(&rsa);

    uint8_t digest[SSH_SHA256_DIGEST_LEN];
    if (rc == 0)
    {
        ssh_sha256(msg, msg_len, digest);
        rc = mbedtls_rsa_pkcs1_verify(&rsa, MBEDTLS_MD_SHA256, SSH_SHA256_DIGEST_LEN, digest, sig);
    }

    mbedtls_mpi_free(&N);
    mbedtls_mpi_free(&E);
    mbedtls_rsa_free(&rsa);
    return rc == 0 ? 0 : -1;
}

// ---------------------------------------------------------------------------
// Native - software RSA path (test-only; private key injected by fixture)
// ---------------------------------------------------------------------------

#else

// The native test fixture sets these before calling ssh_rsa_sign().
// They are plain arrays (no pointers to heap); the test is responsible for
// wiping them after the test if desired.
uint8_t _test_rsa_n[SSH_RSA_KEY_BYTES];
uint8_t _test_rsa_d[SSH_RSA_KEY_BYTES];
uint8_t _test_rsa_e[4];

int ssh_rsa_load_pubkey(void)
{
    memcpy(ssh_host_pubkey.n, _test_rsa_n, SSH_RSA_KEY_BYTES);
    memcpy(ssh_host_pubkey.e_bytes, _test_rsa_e, 4);
    ssh_host_pubkey.loaded = true;
    return 0;
}

int ssh_rsa_sign(const uint8_t *msg, size_t msg_len, uint8_t sig[SSH_RSA_SIG_BYTES])
{
    // SECURITY: private key lives only in this stack frame.
    // Volatile wipe at end ensures the compiler cannot elide the zero-out.
    SshRsaPrivKey priv;

    // Load from test fixture arrays.
    memcpy(priv.n, _test_rsa_n, SSH_RSA_KEY_BYTES);
    memcpy(priv.d, _test_rsa_d, SSH_RSA_KEY_BYTES);
    memcpy(priv.e_bytes, _test_rsa_e, 4);

    // 1. SHA-256 digest of the message.
    uint8_t digest[SSH_SHA256_DIGEST_LEN];
    ssh_sha256(msg, msg_len, digest);

    // 2. PKCS#1 v1.5 encode: 0x00 0x01 0xFF... 0x00 DigestInfo digest
    uint8_t em[SSH_RSA_KEY_BYTES];
    pkcs1v15_encode(digest, em);
    ssh_wipe(digest, sizeof(digest));

    // 3. RSA private-key operation: s = em^d mod n.
    //    Portable square-and-multiply; test-only, NOT constant-time.
    //    Test fixtures must use d=1 (sig = em mod n = em for em < n).
    SshBigNum n_bn, d_bn, m_bn, s_bn;
    bn_from_bytes(&n_bn, priv.n, SSH_RSA_KEY_BYTES);
    bn_from_bytes(&d_bn, priv.d, SSH_RSA_KEY_BYTES);
    bn_from_bytes(&m_bn, em, SSH_RSA_KEY_BYTES);
    ssh_wipe(em, sizeof(em));

    // Square-and-multiply mod n.
    // Uses uint64_t for intermediate products of 32-bit limbs.
    memset(s_bn.d, 0, sizeof(s_bn.d));
    s_bn.d[0] = 1; // s = 1

    // Iterating from MSB to LSB of d.
    for (int limb = SSH_BN_LIMBS - 1; limb >= 0; limb--)
    {
        for (int bit = 31; bit >= 0; bit--)
        {
            // Square: s = s*s mod n
            // (Uses crypto_work as scratch via the existing bn_monpro machinery)
            // For simplicity in test: use naive O(n^2) reduction.
            // This is 64-limb multiply with 64-limb modular reduction.
            uint64_t prod[128] = {0};
            for (int a = 0; a < SSH_BN_LIMBS; a++)
                for (int b2 = 0; b2 < SSH_BN_LIMBS; b2++)
                {
                    uint64_t carry = (uint64_t)s_bn.d[a] * s_bn.d[b2];
                    int idx = a + b2;
                    prod[idx] += carry;
                    if (prod[idx] < carry)
                        prod[idx + 1]++;
                }
            // Normalize carries
            for (int k = 0; k < 127; k++)
            {
                prod[k + 1] += prod[k] >> 32;
                prod[k] &= 0xFFFFFFFFu;
            }
            // Reduce mod n (multi-precision division by repeated subtraction
            // is too slow; use shift-based schoolbook reduction).
            // For test purposes, truncate to 64 limbs (mod 2^2048), then
            // reduce mod n via repeated subtraction of n<<k.
            SshBigNum sq;
            for (int k = 0; k < SSH_BN_LIMBS; k++)
                sq.d[k] = (uint32_t)prod[k];
            // Reduce mod n using bn_cmp + subtract loop.
            while (bn_cmp(&sq, &n_bn) >= 0)
            {
                int64_t borrow = 0;
                for (int k = 0; k < SSH_BN_LIMBS; k++)
                {
                    int64_t diff = (int64_t)sq.d[k] - (int64_t)n_bn.d[k] + borrow;
                    sq.d[k] = (uint32_t)(diff & 0xFFFFFFFFLL);
                    borrow = diff >> 32;
                }
            }
            s_bn = sq;

            // Conditional multiply: if bit set, s = s * m mod n
            if ((d_bn.d[limb] >> bit) & 1u)
            {
                memset(prod, 0, sizeof(prod));
                for (int a = 0; a < SSH_BN_LIMBS; a++)
                    for (int b2 = 0; b2 < SSH_BN_LIMBS; b2++)
                    {
                        uint64_t carry = (uint64_t)s_bn.d[a] * m_bn.d[b2];
                        int idx = a + b2;
                        prod[idx] += carry;
                        if (prod[idx] < carry)
                            prod[idx + 1]++;
                    }
                for (int k = 0; k < 127; k++)
                {
                    prod[k + 1] += prod[k] >> 32;
                    prod[k] &= 0xFFFFFFFFu;
                }
                SshBigNum mul;
                for (int k = 0; k < SSH_BN_LIMBS; k++)
                    mul.d[k] = (uint32_t)prod[k];
                while (bn_cmp(&mul, &n_bn) >= 0)
                {
                    int64_t borrow = 0;
                    for (int k = 0; k < SSH_BN_LIMBS; k++)
                    {
                        int64_t diff = (int64_t)mul.d[k] - (int64_t)n_bn.d[k] + borrow;
                        mul.d[k] = (uint32_t)(diff & 0xFFFFFFFFLL);
                        borrow = diff >> 32;
                    }
                }
                s_bn = mul;
            }
        }
    }

    // Write result as big-endian signature.
    bn_to_bytes(sig, &s_bn);

    // Wipe all sensitive stack material.
    ssh_wipe(&priv, sizeof(priv));
    ssh_wipe(&n_bn, sizeof(n_bn));
    ssh_wipe(&d_bn, sizeof(d_bn));
    ssh_wipe(&m_bn, sizeof(m_bn));
    ssh_wipe(&s_bn, sizeof(s_bn));

    return 0;
}

// ---------------------------------------------------------------------------
// Native RSA verification - correct full-width modular exponentiation.
// Only the public operation s^e mod n is needed (e is small, ~17 bits), so a
// straightforward schoolbook multiply + bit-serial reduction is fast enough
// and, unlike the d=1 signing stub, mathematically complete.
// ---------------------------------------------------------------------------

// Full 128-limb product of two 64-limb little-endian integers.
static void bn_mul_full(const uint32_t a[SSH_BN_LIMBS], const uint32_t b[SSH_BN_LIMBS], uint32_t p[2 * SSH_BN_LIMBS])
{
    for (int k = 0; k < 2 * SSH_BN_LIMBS; k++)
        p[k] = 0;
    for (int i = 0; i < SSH_BN_LIMBS; i++)
    {
        uint64_t carry = 0;
        for (int j = 0; j < SSH_BN_LIMBS; j++)
        {
            uint64_t cur = (uint64_t)p[i + j] + (uint64_t)a[i] * b[j] + carry;
            p[i + j] = (uint32_t)cur;
            carry = cur >> 32;
        }
        // Propagate the final carry into the upper half.
        int k = i + SSH_BN_LIMBS;
        while (carry && k < 2 * SSH_BN_LIMBS)
        {
            uint64_t cur = (uint64_t)p[k] + carry;
            p[k] = (uint32_t)cur;
            carry = cur >> 32;
            k++;
        }
    }
}

// Reduce a 128-limb value mod a 64-limb modulus, bit-serial. out = p mod m.
static void bn_reduce_full(const uint32_t p[2 * SSH_BN_LIMBS], const uint32_t m[SSH_BN_LIMBS],
                           uint32_t out[SSH_BN_LIMBS])
{
    uint32_t r[SSH_BN_LIMBS + 1];
    for (int k = 0; k <= SSH_BN_LIMBS; k++)
        r[k] = 0;

    for (int bit = 2 * SSH_BN_LIMBS * 32 - 1; bit >= 0; bit--)
    {
        // r <<= 1
        uint32_t carry = 0;
        for (int k = 0; k <= SSH_BN_LIMBS; k++)
        {
            uint32_t nc = r[k] >> 31;
            r[k] = (r[k] << 1) | carry;
            carry = nc;
        }
        // bring in the next bit of p
        r[0] |= (p[bit >> 5] >> (bit & 31)) & 1u;

        // if r >= m, subtract m. r has one guard limb (r[SSH_BN_LIMBS]).
        bool ge = r[SSH_BN_LIMBS] != 0;
        if (!ge)
        {
            ge = true; // assume equal/greater until a limb says otherwise
            for (int k = SSH_BN_LIMBS - 1; k >= 0; k--)
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
            for (int k = 0; k < SSH_BN_LIMBS; k++)
            {
                uint64_t v = (uint64_t)r[k] - m[k] - borrow;
                r[k] = (uint32_t)v;
                borrow = (v >> 32) & 1u;
            }
            r[SSH_BN_LIMBS] -= (uint32_t)borrow;
        }
    }
    for (int k = 0; k < SSH_BN_LIMBS; k++)
        out[k] = r[k];
}

// out = base^e mod n, e a small public exponent.
static void bn_modexp_pub(const SshBigNum *base, uint32_t e, const SshBigNum *n, SshBigNum *out)
{
    uint32_t prod[2 * SSH_BN_LIMBS];

    // Reduce the base mod n (signatures are < n, but be safe).
    SshBigNum b;
    for (int k = 0; k < SSH_BN_LIMBS; k++)
    {
        prod[k] = base->d[k];
        prod[k + SSH_BN_LIMBS] = 0;
    }
    bn_reduce_full(prod, n->d, b.d);

    SshBigNum r;
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

int ssh_rsa_verify(const uint8_t n_be[SSH_RSA_KEY_BYTES], const uint8_t e_be4[4], const uint8_t *msg, size_t msg_len,
                   const uint8_t *sig, size_t sig_len)
{
    if (sig_len != SSH_RSA_KEY_BYTES)
        return -1;

    SshBigNum n, s, m;
    bn_from_bytes(&n, n_be, SSH_RSA_KEY_BYTES);
    bn_from_bytes(&s, sig, SSH_RSA_KEY_BYTES);
    if (bn_cmp(&s, &n) >= 0)
        return -1; // signature must be reduced mod n

    uint32_t e = ((uint32_t)e_be4[0] << 24) | ((uint32_t)e_be4[1] << 16) | ((uint32_t)e_be4[2] << 8) | e_be4[3];
    bn_modexp_pub(&s, e, &n, &m);

    uint8_t em[SSH_RSA_KEY_BYTES];
    bn_to_bytes(em, &m);

    // Recompute the expected PKCS#1 v1.5 block and compare in constant time.
    uint8_t digest[SSH_SHA256_DIGEST_LEN];
    ssh_sha256(msg, msg_len, digest);
    uint8_t expected[SSH_RSA_KEY_BYTES];
    pkcs1v15_encode(digest, expected);

    uint8_t diff = 0;
    for (size_t k = 0; k < SSH_RSA_KEY_BYTES; k++)
        diff |= (uint8_t)(em[k] ^ expected[k]);
    return diff == 0 ? 0 : -1;
}

#endif // ARDUINO

// ---------------------------------------------------------------------------
// Public-key blob serialization (both platforms)
// ---------------------------------------------------------------------------

// Write a 4-byte big-endian uint32 to p and advance p by 4.
static uint8_t *put_u32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)(v);
    return p + 4;
}

// Write an SSH mpint (4-byte length + optional 0x00 prefix + data).
// data is big-endian, data_len bytes.  Returns pointer past the written data.
static uint8_t *put_mpint(uint8_t *p, const uint8_t *data, size_t data_len)
{
    // Skip leading zeros to find first non-zero byte.
    size_t off = 0;
    while (off < data_len && data[off] == 0)
        off++;
    const uint8_t *src = data + off;
    size_t src_len = data_len - off;
    bool need_pad = (src_len > 0) && (src[0] & 0x80u);
    uint32_t mpint_len = (uint32_t)src_len + (need_pad ? 1u : 0u);
    p = put_u32(p, mpint_len);
    if (need_pad)
        *p++ = 0x00;
    memcpy(p, src, src_len);
    return p + src_len;
}

int ssh_rsa_encode_pubkey(uint8_t *out, size_t *out_len, size_t out_cap)
{
    if (!ssh_host_pubkey.loaded)
        return -1;

    // Conservative upper bound already defined: SSH_RSA_PUBKEY_BLOB_MAX
    if (out_cap < SSH_RSA_PUBKEY_BLOB_MAX)
        return -1;

    const char *alg = SSH_RSA_PUBKEY_ALG; // "ssh-rsa" (RFC 8332 §3)
    size_t alg_len = SSH_RSA_PUBKEY_ALG_LEN;

    uint8_t *p = out;

    // uint32 len(alg) + alg bytes
    p = put_u32(p, (uint32_t)alg_len);
    memcpy(p, alg, alg_len);
    p += alg_len;

    // mpint e
    p = put_mpint(p, ssh_host_pubkey.e_bytes, sizeof(ssh_host_pubkey.e_bytes));

    // mpint n
    p = put_mpint(p, ssh_host_pubkey.n, SSH_RSA_KEY_BYTES);

    *out_len = (size_t)(p - out);
    return 0;
}
