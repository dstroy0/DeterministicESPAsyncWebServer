// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file sntrup761.h
 * @brief Streamlined NTRU Prime sntrup761 KEM - responder (encapsulation) only.
 *
 * The second post-quantum KEM OpenSSH ships (alongside ML-KEM-768), used by the
 * sntrup761x25519-sha512@openssh.com hybrid key exchange. Both KEM roles are provided:
 *   - Encapsulation (SSH server / responder): given the peer's public key, produce a ciphertext
 *     and a shared secret.
 *   - KeyGen + Decapsulation (the reverse-SSH client / initiator): generate a keypair, send the
 *     public key, then recover the shared secret from the server's ciphertext.
 *
 * Streamlined NTRU Prime, parameter set sntrup761 (p=761, q=4591, w=286): a lattice KEM over
 * the ring Z_q[x]/(x^761 - x - 1). The algorithm and the byte encodings match OpenSSH's embedded
 * sntrup761 reference (public domain; D. J. Bernstein et al.) so the ciphertext this produces
 * decapsulates byte-for-byte on a real OpenSSH peer. Zero heap; SHA-512 via the SSH sha512 seam,
 * randomness via ssh_rng_fill.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_PQC_SNTRUP761_H
#define DETERMINISTICESPASYNCWEBSERVER_PQC_SNTRUP761_H

#include "ServerConfig.h"

#if DWS_ENABLE_PQC_KEX

#include <stddef.h>
#include <stdint.h>

#define DWS_SNTRUP761_PK_BYTES 1158 ///< public key (Rq-encoded h)
#define DWS_SNTRUP761_SK_BYTES 1763 ///< secret key (f, 1/g, pk, rho, cache)
#define DWS_SNTRUP761_CT_BYTES 1039 ///< ciphertext (Rounded-encoded c || 32-byte Confirm)
#define DWS_SNTRUP761_SS_BYTES 32   ///< shared secret (session key)

/**
 * @brief sntrup761 key generation (initiator). Produces a public/secret keypair; the caller sends
 *        @p pk and holds @p sk until the peer's ciphertext arrives (then dws_sntrup761_dec).
 */
void dws_sntrup761_keypair(uint8_t pk[DWS_SNTRUP761_PK_BYTES], uint8_t sk[DWS_SNTRUP761_SK_BYTES]);

/**
 * @brief sntrup761 Encapsulation (responder). Draws a fresh short polynomial via ssh_rng_fill,
 *        encrypts it under @p pk, and derives the session key.
 * @param pk  the peer's public key (DWS_SNTRUP761_PK_BYTES).
 * @param ct  out: the ciphertext (DWS_SNTRUP761_CT_BYTES).
 * @param ss  out: the shared secret (DWS_SNTRUP761_SS_BYTES).
 */
void dws_sntrup761_enc(const uint8_t pk[DWS_SNTRUP761_PK_BYTES], uint8_t ct[DWS_SNTRUP761_CT_BYTES],
                       uint8_t ss[DWS_SNTRUP761_SS_BYTES]);

/**
 * @brief sntrup761 Decapsulation (initiator). Recovers the shared secret from the peer's ciphertext
 *        using the secret key from dws_sntrup761_keypair. Implicit-rejection (FO): on a bad
 *        ciphertext it returns a deterministic pseudo-random secret rather than failing.
 */
void dws_sntrup761_dec(const uint8_t sk[DWS_SNTRUP761_SK_BYTES], const uint8_t ct[DWS_SNTRUP761_CT_BYTES],
                       uint8_t ss[DWS_SNTRUP761_SS_BYTES]);

#endif // DWS_ENABLE_PQC_KEX

#endif // DETERMINISTICESPASYNCWEBSERVER_PQC_SNTRUP761_H
