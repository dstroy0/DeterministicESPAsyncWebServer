// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_rsa.cpp
 * @brief RSA-SHA2-256/512 host-key signing (stack-only private key, PKCS#1 v1.5).
 */

#include "network_drivers/presentation/ssh/crypto/ssh_rsa.h"
#include "network_drivers/presentation/ssh/transport/ssh_keymat.h"
#include <string.h>

// ---------------------------------------------------------------------------
// DigestInfo for SHA-256 / SHA-512 (PKCS#1 v1.5, RFC 8017 §9.2, RFC 5754)
// ---------------------------------------------------------------------------

const uint8_t ssh_pkcs1_sha256_digestinfo[SSH_PKCS1_DIGESTINFO_LEN] = {
    0x30, 0x31,                                           // SEQUENCE, length 49
    0x30, 0x0d,                                           // SEQUENCE, length 13 (AlgorithmIdentifier)
    0x06, 0x09,                                           // OID, length 9
    0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, // OID 2.16.840.1.101.3.4.2.1
    0x05, 0x00,                                           // NULL parameters
    0x04, 0x20                                            // OCTET STRING, length 32 (digest follows)
};

const uint8_t ssh_pkcs1_sha512_digestinfo[SSH_PKCS1_SHA512_DIGESTINFO_LEN] = {
    0x30, 0x51,                                           // SEQUENCE, length 81
    0x30, 0x0d,                                           // SEQUENCE, length 13 (AlgorithmIdentifier)
    0x06, 0x09,                                           // OID, length 9
    0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03, // OID 2.16.840.1.101.3.4.2.3
    0x05, 0x00,                                           // NULL parameters
    0x04, 0x40                                            // OCTET STRING, length 64 (digest follows)
};

// ---------------------------------------------------------------------------
// Public host key (BSS - no secret material)
// ---------------------------------------------------------------------------

SshRsaPubKey ssh_host_pubkey;

// ---------------------------------------------------------------------------
// Arduino - mbedtls path
// ---------------------------------------------------------------------------

#ifdef ARDUINO

#include <Preferences.h> // ESP-IDF NVS wrapper
#include <esp_random.h>  // esp_fill_random() for the RSA blinding RNG
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h> // serialise signs on the shared cached key context
#include <mbedtls/md.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>

// RNG callback for mbedtls private-key operations. mbedtls v3 requires a real
// f_rng for RSA signing (blinding); v2 uses it harmlessly. Backed by the ESP32
// hardware RNG.
static int ssh_mbedtls_rng(void *ctx, unsigned char *buf, size_t len)
{
    (void)ctx;
    esp_fill_random(buf, len);
    return 0;
}

// Cached RSA host-key signer. Re-parsing the PKCS#8 key per handshake also re-ran mbedtls's first-use
// blinding setup (a software r^-1 mod n), ~170 ms wasted on every sign; the parsed context caches the
// blinding state (Vf/Vi), so keeping it resident means each sign pays only the CRT modexp (~440 -> ~270 ms
// on an ESP32-S3). The private key therefore stays in RAM for the server lifetime (as an SSH host key
// normally does); the mutex serialises signs because mbedtls mutates the blinding values per operation, so
// two worker cores must not enter mbedtls_pk_sign on the shared context at once. Loaded once at startup by
// ssh_rsa_load_pubkey() (called from the sketch's setup(), single-threaded).
struct SshRsaCtx
{
    mbedtls_pk_context pk;  ///< parsed host key + cached blinding state
    SemaphoreHandle_t lock; ///< serialises signs on the shared context
    bool ready;             ///< pk holds a valid parsed key
};
static SshRsaCtx s_rsa;

// Load the NVS DER blob, parse it once into the cached signer context, and extract n and e into
// ssh_host_pubkey. Idempotent (re-parses / frees any previously loaded key), so calling it again reloads.
int ssh_rsa_load_pubkey(void)
{
    if (!s_rsa.lock)
        s_rsa.lock = xSemaphoreCreateMutex();

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

    // (Re)parse into the persistent context. Free any prior key first.
    if (s_rsa.ready)
    {
        mbedtls_pk_free(&s_rsa.pk);
        s_rsa.ready = false;
    }
    mbedtls_pk_init(&s_rsa.pk);
    int rc = mbedtls_pk_parse_key(&s_rsa.pk, der, der_len, nullptr, 0
#if MBEDTLS_VERSION_MAJOR >= 3
                                  ,
                                  ssh_mbedtls_rng, nullptr
#endif
    );
    // Wipe the DER stack buffer whether or not parse succeeded; the parsed key stays in s_rsa.pk.
    ssh_wipe(der, der_len);

    if (rc != 0)
    {
        mbedtls_pk_free(&s_rsa.pk);
        return -1;
    }

    mbedtls_rsa_context *rsa = mbedtls_pk_rsa(s_rsa.pk);
    if (mbedtls_rsa_get_len(rsa) != SSH_RSA_KEY_BYTES)
    {
        mbedtls_pk_free(&s_rsa.pk);
        return -1;
    }

    // Write n and e into the public-only BSS struct.
    mbedtls_mpi n_mpi;
    mbedtls_mpi e_mpi;
    mbedtls_mpi_init(&n_mpi);
    mbedtls_mpi_init(&e_mpi);
    mbedtls_rsa_export(rsa, &n_mpi, nullptr, nullptr, nullptr, &e_mpi);
    mbedtls_mpi_write_binary(&n_mpi, ssh_host_pubkey.n, SSH_RSA_KEY_BYTES);
    mbedtls_mpi_write_binary(&e_mpi, ssh_host_pubkey.e_bytes + 4 - sizeof(ssh_host_pubkey.e_bytes),
                             sizeof(ssh_host_pubkey.e_bytes));
    mbedtls_mpi_free(&n_mpi);
    mbedtls_mpi_free(&e_mpi);

    s_rsa.ready = true;
    ssh_host_pubkey.loaded = true;
    return 0;
}

int ssh_rsa_sign(const uint8_t *msg, size_t msg_len, SshRsaHash hash, uint8_t sig[SSH_RSA_SIG_BYTES])
{
    // Reuse the key parsed once at startup. Lazy-load as a fallback if the sketch never did (e.g. a sign
    // before ssh_rsa_load_pubkey()); load runs single-threaded at boot so first-use here is the edge case.
    if (!s_rsa.ready && ssh_rsa_load_pubkey() != 0)
        return -1;

    // Hash the message, then sign the digest. mbedtls_pk_sign() does NOT hash its input - it PKCS#1-pads
    // the supplied digest - so for rsa-sha2-256/512 (RFC 8332) we pass SHA-256(msg) / SHA-512(msg), not msg.
    const bool sha512 = (hash == SshRsaHash::SHA512);
    const mbedtls_md_type_t md = sha512 ? MBEDTLS_MD_SHA512 : MBEDTLS_MD_SHA256;
    const size_t dlen = sha512 ? SSH_SHA512_DIGEST_LEN : SSH_SHA256_DIGEST_LEN;
    uint8_t digest[SSH_SHA512_DIGEST_LEN];
    if (sha512)
        ssh_sha512(msg, msg_len, digest);
    else
        ssh_sha256(msg, msg_len, digest);

    // Serialise: mbedtls mutates the context's blinding state (Vf/Vi) on each private op.
    if (s_rsa.lock)
        xSemaphoreTake(s_rsa.lock, portMAX_DELAY);
    size_t sig_len = 0;
#if MBEDTLS_VERSION_MAJOR >= 3
    int rc = mbedtls_pk_sign(&s_rsa.pk, md, digest, dlen, sig, SSH_RSA_SIG_BYTES, &sig_len, ssh_mbedtls_rng, nullptr);
#else
    int rc = mbedtls_pk_sign(&s_rsa.pk, md, digest, dlen, sig, &sig_len, ssh_mbedtls_rng, nullptr);
#endif
    if (s_rsa.lock)
        xSemaphoreGive(s_rsa.lock);
    ssh_wipe(digest, sizeof(digest));

    return (rc == 0 && sig_len == SSH_RSA_SIG_BYTES) ? 0 : -1;
}

int ssh_rsa_verify(const uint8_t n_be[SSH_RSA_KEY_BYTES], const uint8_t e_be4[4], const uint8_t *msg, size_t msg_len,
                   const uint8_t *sig, size_t sig_len, SshRsaHash hash)
{
    if (sig_len != SSH_RSA_KEY_BYTES)
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
    mbedtls_mpi_read_binary(&N, n_be, SSH_RSA_KEY_BYTES);
    mbedtls_mpi_read_binary(&E, e_be4, 4);

    int rc = mbedtls_rsa_import(&rsa, &N, nullptr, nullptr, nullptr, &E);
    if (rc == 0)
        rc = mbedtls_rsa_complete(&rsa);

    const bool sha512 = (hash == SshRsaHash::SHA512);
    const mbedtls_md_type_t md = sha512 ? MBEDTLS_MD_SHA512 : MBEDTLS_MD_SHA256;
    const size_t dlen = sha512 ? SSH_SHA512_DIGEST_LEN : SSH_SHA256_DIGEST_LEN;
    uint8_t digest[SSH_SHA512_DIGEST_LEN];
    if (rc == 0)
    {
        if (sha512)
            ssh_sha512(msg, msg_len, digest);
        else
            ssh_sha256(msg, msg_len, digest);
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

// ---------------------------------------------------------------------------
// PKCS#1 v1.5 pad-and-encode (software; Arduino delegates padding to mbedtls)
// ---------------------------------------------------------------------------

// Hash msg with the selected algorithm and return the matching DigestInfo.
//   digest must be >= SSH_SHA512_DIGEST_LEN bytes.
static void rsa_digest(const uint8_t *msg, size_t msg_len, SshRsaHash hash, uint8_t digest[SSH_SHA512_DIGEST_LEN],
                       size_t *digest_len, const uint8_t **di, size_t *di_len)
{
    if (hash == SshRsaHash::SHA512)
    {
        ssh_sha512(msg, msg_len, digest);
        *digest_len = SSH_SHA512_DIGEST_LEN;
        *di = ssh_pkcs1_sha512_digestinfo;
        *di_len = SSH_PKCS1_SHA512_DIGESTINFO_LEN;
    }
    else
    {
        ssh_sha256(msg, msg_len, digest);
        *digest_len = SSH_SHA256_DIGEST_LEN;
        *di = ssh_pkcs1_sha256_digestinfo;
        *di_len = SSH_PKCS1_DIGESTINFO_LEN;
    }
}

// Builds the 256-byte padded message:
//   0x00 0x01 [pad × 0xFF] 0x00 [DigestInfo] [digest]
// pad = 256 - 3 - di_len - digest_len (202 for SHA-256, 170 for SHA-512).
static void pkcs1v15_encode(const uint8_t *digest, size_t digest_len, const uint8_t *di, size_t di_len,
                            uint8_t em[SSH_RSA_KEY_BYTES])
{
    const size_t total = di_len + digest_len;
    const size_t pad_len = SSH_RSA_KEY_BYTES - 3 - total;
    em[0] = 0x00;
    em[1] = 0x01;
    memset(em + 2, 0xFF, pad_len);
    em[2 + pad_len] = 0x00;
    memcpy(em + 3 + pad_len, di, di_len);
    memcpy(em + 3 + pad_len + di_len, digest, digest_len);
}

// ---------------------------------------------------------------------------
// Native full-width modular arithmetic.
// These helpers back both the private signing operation (s = em^d mod n) and
// the public verify operation (s^e mod n).  Schoolbook multiply + bit-serial
// reduction: correct and full-width (no truncation), but NOT constant-time -
// the native path is test-only (ESP32/mbedTLS is the real one).
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

// out = base^exp mod n, exp a full-width 2048-bit private exponent.
// Left-to-right square-and-multiply over every bit of exp (MSB to LSB,
// skipping leading zero limbs/bits).  Same helpers as the public path, so the
// reduction is full-width and correct for any d (not just d=1).
static void bn_modexp_full(const SshBigNum *base, const SshBigNum *exp, const SshBigNum *n, SshBigNum *out)
{
    uint32_t prod[2 * SSH_BN_LIMBS];

    // Reduce the base mod n up front.
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

    // Locate the most-significant set bit of exp.
    int top_limb = SSH_BN_LIMBS - 1;
    while (top_limb >= 0 && exp->d[top_limb] == 0)
        top_limb--;
    if (top_limb < 0)
    {
        *out = r; // exp == 0 -> result is 1
        return;
    }
    int top_bit = 31;
    while (top_bit >= 0 && !((exp->d[top_limb] >> top_bit) & 1u))
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

int ssh_rsa_sign(const uint8_t *msg, size_t msg_len, SshRsaHash hash, uint8_t sig[SSH_RSA_SIG_BYTES])
{
    // SECURITY: private key lives only in this stack frame.
    SshRsaPrivKey priv;
    memcpy(priv.n, _test_rsa_n, SSH_RSA_KEY_BYTES);
    memcpy(priv.d, _test_rsa_d, SSH_RSA_KEY_BYTES);
    memcpy(priv.e_bytes, _test_rsa_e, 4);

    // 1. SHA-256/512 digest of the message + matching DigestInfo.
    uint8_t digest[SSH_SHA512_DIGEST_LEN];
    size_t digest_len = 0;
    const uint8_t *di = nullptr;
    size_t di_len = 0;
    rsa_digest(msg, msg_len, hash, digest, &digest_len, &di, &di_len);

    // 2. PKCS#1 v1.5 encode: 0x00 0x01 0xFF... 0x00 DigestInfo digest
    uint8_t em[SSH_RSA_KEY_BYTES];
    pkcs1v15_encode(digest, digest_len, di, di_len, em);
    ssh_wipe(digest, sizeof(digest));

    // 3. RSA private-key operation: s = em^d mod n (full-width, correct for
    //    any private exponent - no longer a d=1 stub).
    SshBigNum n_bn;
    SshBigNum d_bn;
    SshBigNum m_bn;
    SshBigNum s_bn;
    bn_from_bytes(&n_bn, priv.n, SSH_RSA_KEY_BYTES);
    bn_from_bytes(&d_bn, priv.d, SSH_RSA_KEY_BYTES);
    bn_from_bytes(&m_bn, em, SSH_RSA_KEY_BYTES);
    ssh_wipe(em, sizeof(em));

    bn_modexp_full(&m_bn, &d_bn, &n_bn, &s_bn);

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

int ssh_rsa_verify(const uint8_t n_be[SSH_RSA_KEY_BYTES], const uint8_t e_be4[4], const uint8_t *msg, size_t msg_len,
                   const uint8_t *sig, size_t sig_len, SshRsaHash hash)
{
    if (sig_len != SSH_RSA_KEY_BYTES)
        return -1;

    SshBigNum n;
    SshBigNum s;
    SshBigNum m;
    bn_from_bytes(&n, n_be, SSH_RSA_KEY_BYTES);
    bn_from_bytes(&s, sig, SSH_RSA_KEY_BYTES);
    if (bn_cmp(&s, &n) >= 0)
        return -1; // signature must be reduced mod n

    uint32_t e = ((uint32_t)e_be4[0] << 24) | ((uint32_t)e_be4[1] << 16) | ((uint32_t)e_be4[2] << 8) | e_be4[3];
    bn_modexp_pub(&s, e, &n, &m);

    uint8_t em[SSH_RSA_KEY_BYTES];
    bn_to_bytes(em, &m);

    // Recompute the expected PKCS#1 v1.5 block and compare in constant time.
    uint8_t digest[SSH_SHA512_DIGEST_LEN];
    size_t digest_len = 0;
    const uint8_t *di = nullptr;
    size_t di_len = 0;
    rsa_digest(msg, msg_len, hash, digest, &digest_len, &di, &di_len);
    uint8_t expected[SSH_RSA_KEY_BYTES];
    pkcs1v15_encode(digest, digest_len, di, di_len, expected);

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
