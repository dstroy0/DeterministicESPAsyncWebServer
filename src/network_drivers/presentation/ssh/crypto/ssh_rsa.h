// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_rsa.h
 * @brief RSA-SHA2-256/512 host-key signing and public-key serialization.
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * SECURITY MODEL - PRIVATE KEY LIFETIME
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * The RSA-2048 private key (d, p, q, dp, dq, qinv) MUST NEVER live in static
 * or global memory.  Reasons:
 *
 *   1. A linear heap/BSS overflow from any direction can reach a static
 *      variable; the stack is a separate region with its own growth direction
 *      and is far harder to reach from a single-direction overflow.
 *
 *   2. A use-after-free or dangling-pointer bug that reads static memory can
 *      silently expose a key that "should have been zeroed" but was not.
 *
 *   3. The private key is only needed for the KEX (once per connection, during
 *      the handshake).  There is no reason to keep it resident between uses.
 *
 * MANDATED LIFETIME:
 *   load (from NVS) → stack → sign → volatile-wipe → return
 *
 * The struct SshRsaPrivKey is declared here for documentation purposes; it
 * must only ever be declared as a local variable inside ssh_rsa_sign().
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * PKCS#1 v1.5 SIGNATURE SCHEME
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * The signature produced by ssh_rsa_sign() follows PKCS#1 v1.5 (RFC 8017
 * §8.2), which is what SSH uses for "rsa-sha2-256" and "rsa-sha2-512"
 * (RFC 8332) - the only difference is the hash and its DigestInfo OID:
 *
 *   1. Compute digest  = SHA256(msg) or SHA512(msg), per @p hash.
 *   2. Encode digest in a DER DigestInfo wrapper:
 *        SHA-256: 30 31 30 0d 06 09 60 86 48 01 65 03 04 02 01 05 00 04 20 <32 bytes>
 *        SHA-512: 30 51 30 0d 06 09 60 86 48 01 65 03 04 02 03 05 00 04 40 <64 bytes>
 *      The OID 2.16.840.1.101.3.4.2.1 identifies SHA-256, .2.3 SHA-512 (RFC 5754 §3.2).
 *   3. Pad to the RSA modulus length (256 bytes for RSA-2048):
 *        0x00 0x01 &lt;0xFF padding&gt; 0x00 &lt;DigestInfo&gt;
 *      The 0xFF padding fills bytes [2 .. 256-1-len(DigestInfo)-1].
 *      SHA-256 DigestInfo = 51 bytes (padding 202); SHA-512 DigestInfo = 83 bytes (padding 170).
 *   4. Interpret the 256-byte padded message M as a bignum m.
 *   5. Compute s = m^d mod n  (RSA private-key operation).
 *   6. Output s as a 256-byte big-endian integer.
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * ARDUINO VS NATIVE
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Arduino: uses mbedtls_pk_sign() with MBEDTLS_MD_SHA256 or MBEDTLS_MD_SHA512,
 *   which performs the full PKCS#1 v1.5 pad-and-sign in hardware-accelerated
 *   multiprecision arithmetic via ESP-IDF.
 *
 * Native:  software path - SHA-256/512 via ssh_sha256()/ssh_sha512(), PKCS#1
 *   v1.5 padding built by hand, RSA exponentiation via bn_expmod_group14()
 *   (same Montgomery path used for DH).  Both paths use the same key layout.
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * NVS KEY FORMAT
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * On Arduino the private key is stored in NVS namespace "ssh_host_key" under
 * key "priv_der", as a DER-encoded PKCS#1 RSAPrivateKey (RFC 8017 App. C):
 *
 *   RSAPrivateKey ::= SEQUENCE {
 *     version           Version (INTEGER: 0),
 *     modulus           INTEGER,   -- n
 *     publicExponent    INTEGER,   -- e
 *     privateExponent   INTEGER,   -- d
 *     prime1            INTEGER,   -- p
 *     prime2            INTEGER,   -- q
 *     exponent1         INTEGER,   -- dp = d mod (p-1)
 *     exponent2         INTEGER,   -- dq = d mod (q-1)
 *     coefficient       INTEGER,   -- qinv = q^(-1) mod p
 *   }
 *
 * The DER blob is parsed by mbedtls_rsa_parse_key() on Arduino.  On native
 * builds, the test fixture injects n, e, d directly as raw 256-byte arrays.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SSH_RSA_H
#define DETERMINISTICESPASYNCWEBSERVER_SSH_RSA_H

#include "network_drivers/presentation/ssh/crypto/ssh_bignum.h"
#include "network_drivers/presentation/ssh/crypto/ssh_sha256.h"
#include "network_drivers/presentation/ssh/crypto/ssh_sha512.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Hash algorithm selecting the RSA signature scheme (RFC 8332).
 *
 * The RSA public-key blob is "ssh-rsa" for both; only the message hash and its
 * DigestInfo OID differ. SHA256 = "rsa-sha2-256", SHA512 = "rsa-sha2-512".
 */
enum class SshRsaHash : uint8_t
{
    SHA256 = 0, ///< rsa-sha2-256
    SHA512 = 1  ///< rsa-sha2-512
};

// ---------------------------------------------------------------------------
// Key-blob sizes
// ---------------------------------------------------------------------------

/** @brief Maximum DER size for a PKCS#1 RSAPrivateKey with 2048-bit fields. */
#define SSH_RSA_KEY_DER_MAX 1700

/** @brief RSA modulus and private exponent size in bytes (RSA-2048). */
#define SSH_RSA_KEY_BYTES 256

/** @brief PKCS#1 v1.5 signature size for RSA-2048 in bytes. */
#define SSH_RSA_SIG_BYTES 256

/**
 * @brief Key-blob type string for an RSA host key.
 *
 * Per RFC 8332 §3, the RSA *public-key blob* always carries the type string
 * "ssh-rsa" - even when the negotiated *signature* algorithm is
 * "rsa-sha2-256" or "rsa-sha2-512".  Only the signature and authentication
 * algorithm-name fields use "rsa-sha2-256"; the key blob format is unchanged
 * from RFC 4253 §6.6.  Emitting "rsa-sha2-256" here would make compliant
 * clients (e.g. OpenSSH) fail to parse the host key.
 */
#define SSH_RSA_PUBKEY_ALG "ssh-rsa"

/** @brief Length of SSH_RSA_PUBKEY_ALG ("ssh-rsa" = 7 bytes). */
#define SSH_RSA_PUBKEY_ALG_LEN 7

/** @brief Signature algorithm name for SHA-256 (RFC 8332). Used in the signature blob. */
#define SSH_RSA_SIG_ALG_SHA256 "rsa-sha2-256"

/** @brief Signature algorithm name for SHA-512 (RFC 8332). Used in the signature blob. */
#define SSH_RSA_SIG_ALG_SHA512 "rsa-sha2-512"

// ---------------------------------------------------------------------------
// PKCS#1 v1.5 DigestInfo headers (RFC 8017, RFC 5754)
//   SHA-256: 30 31 30 0d 06 09 60 86 48 01 65 03 04 02 01 05 00 04 20
//   SHA-512: 30 51 30 0d 06 09 60 86 48 01 65 03 04 02 03 05 00 04 40
// ---------------------------------------------------------------------------

/** @brief Length of the DER DigestInfo wrapper for SHA-256. */
#define SSH_PKCS1_DIGESTINFO_LEN 19

/** @brief Length of the DER DigestInfo wrapper for SHA-512. */
#define SSH_PKCS1_SHA512_DIGESTINFO_LEN 19

/**
 * @brief The DER-encoded DigestInfo wrapper for SHA-256.
 *
 * Prepend this to the 32-byte SHA-256 digest to form the 51-byte
 * DigestInfo structure for PKCS#1 v1.5.
 */
extern const uint8_t ssh_pkcs1_sha256_digestinfo[SSH_PKCS1_DIGESTINFO_LEN];

/**
 * @brief The DER-encoded DigestInfo wrapper for SHA-512.
 *
 * Prepend this to the 64-byte SHA-512 digest to form the 83-byte
 * DigestInfo structure for PKCS#1 v1.5.
 */
extern const uint8_t ssh_pkcs1_sha512_digestinfo[SSH_PKCS1_SHA512_DIGESTINFO_LEN];

// ---------------------------------------------------------------------------
// RSA private key - stack-local only, never static or global
// ---------------------------------------------------------------------------

/**
 * @brief RSA-2048 private key parameters.
 *
 * WARNING: This struct is intentionally NOT to be declared as a global or
 * static variable.  The ONLY valid use is as an automatic (stack) variable
 * inside ssh_rsa_sign().  After the signature is computed, every byte is
 * overwritten via ssh_wipe() before the function returns.
 *
 * On Arduino, these fields are populated from mbedtls_rsa_context by reading
 * the mpi limbs into 256-byte big-endian buffers.
 *
 * On native, the fields are set directly by the test fixture or loaded from
 * a DER blob via a minimal DER parser included in ssh_rsa.cpp.
 */
struct SshRsaPrivKey
{
    uint8_t n[SSH_RSA_KEY_BYTES]; ///< Modulus n (256 bytes, big-endian).
    uint8_t d[SSH_RSA_KEY_BYTES]; ///< Private exponent d (SENSITIVE).
    uint8_t e_bytes[4];           ///< Public exponent e (typically 65537).
};

// ---------------------------------------------------------------------------
// RSA public key (safe to keep in static/flash - no secret material)
// ---------------------------------------------------------------------------

/**
 * @brief RSA-2048 public key parameters for the host key blob.
 *
 * Allocated in BSS at link time.  Contains only n and e; no secret material.
 */
struct SshRsaPubKey
{
    uint8_t n[SSH_RSA_KEY_BYTES]; ///< Modulus n (256 bytes, big-endian).
    uint8_t e_bytes[4];           ///< Public exponent e (big-endian uint32).
    bool loaded;                  ///< True after ssh_rsa_load_pubkey() succeeds.
};

/** @brief Static host public key (BSS). Set by ssh_rsa_load_pubkey(). */
extern SshRsaPubKey ssh_host_pubkey;

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

/**
 * @brief Load the public portion of the RSA host key into ssh_host_pubkey.
 *
 * Arduino: reads the DER blob from NVS ("ssh_host_key"/"priv_der"), parses
 * n and e via mbedtls, stores them in ssh_host_pubkey.
 *
 * Native:  reads n and e from the test fixture arrays (see ssh_rsa.cpp).
 *
 * Call once at startup (or after begin()).  Must succeed before any SSH
 * connections are accepted.
 *
 * @return 0 on success, -1 if the key is absent or malformed.
 */
int ssh_rsa_load_pubkey(void);

/**
 * @brief Sign @p msg using the RSA host key (PKCS#1 v1.5, rsa-sha2-256/512).
 *
 * The private key is loaded from NVS directly into a stack-local
 * SshRsaPrivKey struct, used, then zeroed before this function returns.
 * The key NEVER touches static or global memory.
 *
 * On Arduino: delegates to mbedtls_pk_sign() (hardware-accelerated).
 * On native:  software SHA-256/512 + hand-built PKCS#1 pad + bn_expmod_group14().
 *
 * @param[in]  msg     Message to sign (typically the exchange hash H, 32 bytes).
 * @param[in]  msg_len Length of @p msg.
 * @param[in]  hash    Signature hash: SHA256 (rsa-sha2-256) or SHA512 (rsa-sha2-512).
 * @param[out] sig     Output buffer, must be SSH_RSA_SIG_BYTES (256) bytes.
 * @return 0 on success, -1 on failure (NVS read error, RSA error).
 */
int ssh_rsa_sign(const uint8_t *msg, size_t msg_len, SshRsaHash hash, uint8_t sig[SSH_RSA_SIG_BYTES]);

/**
 * @brief Serialize the RSA public host key into an SSH "ssh-rsa" key blob.
 *
 * Format (RFC 4253 §6.6, RFC 8332 §3):
 *   uint32  len("ssh-rsa")   = 7
 *   byte[7] "ssh-rsa"
 *   mpint   e
 *   mpint   n
 *
 * Writes into @p out; sets *@p out_len to the number of bytes written.
 * @p out must be at least SSH_RSA_PUBKEY_BLOB_MAX bytes.
 *
 * @param[out] out      Destination buffer.
 * @param[out] out_len  Number of bytes written.
 * @param[in]  out_cap  Capacity of @p out.
 * @return 0 on success, -1 if pubkey not loaded or buffer too small.
 */
int ssh_rsa_encode_pubkey(uint8_t *out, size_t *out_len, size_t out_cap);

/**
 * @brief Verify an RSA PKCS#1 v1.5 signature (rsa-sha2-256/512) with a public key.
 *
 * Used for client publickey authentication (RFC 4252 §7): @p n_be / @p e_be4
 * come from the client-supplied key blob, not the host key. The public exponent
 * is small (typically 65537), so the modular exponentiation s^e mod n is cheap
 * and is performed for real on both platforms (native: schoolbook bignum;
 * Arduino: mbedTLS). The hash is selected by the client's signature algorithm
 * name (rsa-sha2-256 -> SHA256, rsa-sha2-512 -> SHA512), not by the key blob.
 *
 * @param[in] n_be    RSA modulus n, big-endian, 256 bytes.
 * @param[in] e_be4   RSA public exponent e, big-endian, 4 bytes.
 * @param[in] msg     Signed message.
 * @param[in] msg_len Length of @p msg.
 * @param[in] sig     Signature bytes (256 for RSA-2048).
 * @param[in] sig_len Length of @p sig.
 * @param[in] hash    Signature hash: SHA256 (rsa-sha2-256) or SHA512 (rsa-sha2-512).
 * @return 0 if the signature is valid, -1 otherwise.
 */
int ssh_rsa_verify(const uint8_t n_be[SSH_RSA_KEY_BYTES], const uint8_t e_be4[4], const uint8_t *msg, size_t msg_len,
                   const uint8_t *sig, size_t sig_len, SshRsaHash hash);

/**
 * @brief Maximum byte length of the serialized RSA public key blob.
 *
 *   uint32 len + "ssh-rsa"(7) + mpint e (4 len + 1 pad + up to 4 bytes)
 *   + mpint n (4 len + 1 pad + 256 bytes)
 */
#define SSH_RSA_PUBKEY_BLOB_MAX (4 + 7 + 4 + 1 + 4 + 4 + 1 + 256)

#endif // DETERMINISTICESPASYNCWEBSERVER_SSH_RSA_H
