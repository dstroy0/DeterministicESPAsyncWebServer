// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Test-only ML-KEM-768 Decaps reference. The library ships Encaps only (the server is always the KEM
// responder), so to verify the SSH hybrid KEX the way a conforming client would - decapsulating the
// server's ciphertext to recover the shared secret - the test needs its own independent Decaps. This
// is a FIPS 203 K-PKE.Decrypt + K = G(m' || H(ek)); it omits the implicit-reject re-encryption because
// the test only ever feeds it a genuine ciphertext. Validated against the pinned KAT before use.

#ifndef DETWS_TEST_MLKEM_REF_H
#define DETWS_TEST_MLKEM_REF_H

#include <stdint.h>

// Recover the 32-octet shared secret from a decapsulation key (2400 B) and a genuine ciphertext.
void mlkem768_decaps_ref(const uint8_t dk[2400], const uint8_t ct[1088], uint8_t ss[32]);

#endif
