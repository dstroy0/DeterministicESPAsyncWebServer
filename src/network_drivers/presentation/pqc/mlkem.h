// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file mlkem.h
 * @brief ML-KEM-768 encapsulation (FIPS 203), responder side only.
 *
 * The post-quantum half of the mlkem768x25519-sha256 (SSH) and X25519MLKEM768 (TLS 1.3) hybrid key
 * exchanges. This server is always the KEM *responder*: the peer sends its ML-KEM encapsulation key,
 * we run Encaps to produce a ciphertext + shared secret. Only Encaps is implemented - no KeyGen or
 * Decaps - so none of the constant-time Fujisaki-Okamoto re-encryption/compare surface is present.
 *
 * Encaps is the FIPS 203 "internal" (derandomized) form: the 32-octet message @p m is supplied by the
 * caller (drawn from the platform RNG in production, fixed in known-answer tests). Deterministic given
 * (@p ek, @p m), which is exactly what the ACVP encapsulation test vectors pin.
 *
 * Arithmetic is a software NTT over q=3329 with Montgomery reduction (the twiddle factors are fixed
 * constants premultiplied into Montgomery form, so each butterfly is two int16 multiplies and a
 * shift - no division, and the hardware MPI, which targets RSA/DH-sized operands, would only add
 * marshaling overhead). Zero heap; peak stack ~7 KB.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_PQC_MLKEM_H
#define DETERMINISTICESPASYNCWEBSERVER_PQC_MLKEM_H

#include "ServerConfig.h"

#if DETWS_ENABLE_PQC_KEX

#include <stddef.h>
#include <stdint.h>

#define MLKEM768_EK_BYTES 1184 ///< encapsulation key (public key): 384*k + 32
#define MLKEM768_CT_BYTES 1088 ///< ciphertext: 32*(du*k + dv) = 32*(30+4)
#define MLKEM768_SS_BYTES 32   ///< shared secret
#define MLKEM768_MSG_BYTES 32  ///< the random message m fed to Encaps

/**
 * @brief ML-KEM-768 Encaps (FIPS 203, derandomized): (ct, ss) from an encapsulation key and message.
 *
 * Validates @p ek (FIPS 203 modulus check: every decoded coefficient must be < q); on a malformed key
 * it writes nothing and returns false. Otherwise derives K = ss and encrypts @p m under @p ek.
 *
 * @param[in]  ek  peer encapsulation key (MLKEM768_EK_BYTES).
 * @param[in]  m   32-octet message (the encapsulation randomness).
 * @param[out] ct  ciphertext (MLKEM768_CT_BYTES).
 * @param[out] ss  32-octet shared secret.
 * @return true on success, false if @p ek fails the modulus check.
 */
bool mlkem768_encaps(const uint8_t ek[MLKEM768_EK_BYTES], const uint8_t m[MLKEM768_MSG_BYTES],
                     uint8_t ct[MLKEM768_CT_BYTES], uint8_t ss[MLKEM768_SS_BYTES]);

#endif // DETWS_ENABLE_PQC_KEX

#endif // DETERMINISTICESPASYNCWEBSERVER_PQC_MLKEM_H
