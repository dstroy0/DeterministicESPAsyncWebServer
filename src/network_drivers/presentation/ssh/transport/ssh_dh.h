// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_dh.h
 * @brief DH-group14-SHA256 key exchange (RFC 4253 §8 + RFC 8268).
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * PROTOCOL FLOW (server side)
 * ═══════════════════════════════════════════════════════════════════════════
 *
 *  Client                          Server
 *  ──────                          ──────
 *  SSH_MSG_KEXDH_INIT  ──e──►    ssh_dh_generate(slot):
 *                                   y  = random 2048-bit scalar
 *                                   f  = 2^y mod p        (server public)
 *                                   (y, f stored in ssh_dh[slot])
 *
 *                                 ssh_dh_finish(slot, e):
 *                                   validate e: 1 < e < p-1
 *                                   K  = e^y mod p        (shared secret)
 *                                   H  = SHA256(V_C||V_S||I_C||I_S||K_S||e||f||K)
 *                                   sig = RSA-SHA2-256(host_key, H)
 *                          ◄──────  SSH_MSG_KEXDH_REPLY (K_S, f, sig)
 *                                   y zeroed; K → key derivation → K zeroed
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * SECURITY NOTES
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * 1. Private scalar y is generated fresh for each connection.  It is stored
 *    in ssh_dh[slot].y and zeroed by ssh_dh_wipe() (in ssh_keymat.h) as
 *    soon as K is derived.  NEVER reuse y.
 *
 * 2. The received value e is validated (1 < e < p-1) before any computation.
 *    A value of 1 or p-1 is a known small-subgroup attack; reject both
 *    (RFC 4253 §8 requires rejection of e outside [2, p-2]).
 *
 * 3. The exchange hash H is computed over all four handshake strings
 *    (V_C, V_S, I_C, I_S), the host key blob K_S, and the DH values.
 *    Omitting any field or reordering them breaks the binding between the
 *    cryptographic material and the identity of both sides.
 *
 * 4. Key material derivation follows RFC 4253 §7.2.  K and H are the only
 *    inputs; the session_id equals H from the first KEX.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SSH_DH_H
#define DETERMINISTICESPASYNCWEBSERVER_SSH_DH_H

#include "DetWebServerConfig.h"
#include "network_drivers/presentation/ssh/crypto/ssh_bignum.h"
#include "network_drivers/presentation/ssh/transport/ssh_keymat.h"
#include "network_drivers/presentation/ssh/crypto/ssh_sha256.h"
#include <stddef.h>
#include <stdint.h>

// ---------------------------------------------------------------------------
// RNG
// ---------------------------------------------------------------------------

/**
 * @brief Fill @p len bytes of @p buf with cryptographically random data.
 *
 * On Arduino: uses esp_fill_random() (hardware RNG seeded by analog noise).
 * On native:  uses esp_fill_random() from the Arduino.h mock, which is a
 *             time-seeded PRNG - NOT secure, for testing only.
 */
void ssh_rng_fill(uint8_t *buf, size_t len);

// ---------------------------------------------------------------------------
// DH key exchange
// ---------------------------------------------------------------------------

/**
 * @brief Generate the server ephemeral DH key pair for connection slot @p i.
 *
 * Fills ssh_dh[i].y with random bytes (2048-bit private scalar), then
 * computes ssh_dh[i].f = 2^y mod p (the server's public DH value).
 *
 * The caller must send f in SSH_MSG_KEXDH_REPLY after this returns.
 *
 * @param i  SSH connection slot index.
 * @return 0 on success, -1 if @p i is out of range.
 */
int ssh_dh_generate(uint8_t i);

/**
 * @brief Complete the DH exchange given the client's public value @p e.
 *
 * Steps:
 *   1. Validate e: 1 < e < p-1.  Returns -1 on failure (close connection).
 *   2. Compute K = e^y mod p.
 *   3. Compute exchange hash H = SHA256(hash_input).
 *   4. Derive session keys into ssh_keys[i].
 *   5. Zero ssh_dh[i].y and ssh_dh[i].K via ssh_dh_wipe().
 *      ssh_dh[i].f and ssh_dh[i].H are retained (f for KEXDH_REPLY if
 *      called before send, H becomes the session_id).
 *
 * @param i           SSH connection slot index.
 * @param e           Client's DH public value (2048-bit, big-endian, 256 bytes).
 * @param hash_input  Pre-assembled hash input buffer (see RFC 4253 §8).
 * @param hi_len      Length of hash_input in bytes.
 * @return 0 on success, -1 if e is invalid or slot is out of range.
 */
int ssh_dh_finish(uint8_t i, const uint8_t e[256], const uint8_t *hash_input, size_t hi_len);

/**
 * @brief Derive the six session keys from shared secret K and exchange hash H.
 *
 * Implements RFC 4253 §7.2 key derivation:
 *   IV_c2s  = SHA256(K || H || 'A' || session_id)  [first 16 bytes]
 *   IV_s2c  = SHA256(K || H || 'B' || session_id)  [first 16 bytes]
 *   key_c2s = SHA256(K || H || 'C' || session_id)  [32 bytes]
 *   key_s2c = SHA256(K || H || 'D' || session_id)  [32 bytes]
 *   mac_c2s = SHA256(K || H || 'E' || session_id)  [32 bytes]
 *   mac_s2c = SHA256(K || H || 'F' || session_id)  [32 bytes]
 *
 * Installs the derived keys into ssh_keys[i].
 * Does NOT zero K or H - the caller (ssh_dh_finish) does that.
 *
 * @param i           Slot index.
 * @param K_be        Shared secret K, big-endian, 256 bytes.
 * @param H           Exchange hash, 32 bytes (also used as session_id).
 */
void ssh_dh_derive_keys(uint8_t i, const uint8_t K_be[256], const uint8_t H[SSH_SHA256_DIGEST_LEN]);

/**
 * @brief Derive session keys with an explicit session id (RFC 4253 §7.2).
 *
 * Same as ssh_dh_derive_keys() but uses @p session_id for the session-id
 * component of every key, which on a re-key must remain the exchange hash H
 * from the *first* KEX (while @p H is the current re-key exchange hash).
 *
 * @param i           Slot index.
 * @param K_be        Shared secret K, big-endian, 256 bytes.
 * @param H           Current exchange hash, 32 bytes.
 * @param session_id  Session id (H of the first KEX), 32 bytes.
 */
void ssh_dh_derive_keys_sid(uint8_t i, const uint8_t K_be[256], const uint8_t H[SSH_SHA256_DIGEST_LEN],
                            const uint8_t session_id[SSH_SHA256_DIGEST_LEN], uint8_t cipher_alg, uint8_t mac_alg);

/** @brief Max bytes ssh_kdf_derive() can produce (4 SHA-256 blocks). */
#define SSH_KDF_MAX (4 * SSH_SHA256_DIGEST_LEN)

/**
 * @brief RFC 4253 §7.2 key derivation for any length up to @ref SSH_KDF_MAX.
 *
 * Produces K1 || K2 || ... where K1 = HASH(mpint(K) || H || @p label || session_id)
 * and each Ki+1 = HASH(mpint(K) || H || K1..Ki), filling @p out (@p out_len bytes).
 * Every algorithm negotiated today needs <= 32 B (one block); the chain exists for
 * spec-completeness / future ciphers needing longer key material. @p out_len is
 * clamped to SSH_KDF_MAX.
 */
void ssh_kdf_derive(const uint8_t K_be[256], const uint8_t H[SSH_SHA256_DIGEST_LEN],
                    const uint8_t session_id[SSH_SHA256_DIGEST_LEN], char label, uint8_t *out, size_t out_len);

#endif // DETERMINISTICESPASYNCWEBSERVER_SSH_DH_H
