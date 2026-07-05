// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file tls13_kdf.cpp
 * @brief TLS 1.3 key schedule (see tls13_kdf.h).
 */

#include "network_drivers/presentation/http3/tls13_kdf.h"

#if DETWS_ENABLE_HTTP3

#include "network_drivers/presentation/http3/quic_hkdf.h"
#include "network_drivers/presentation/ssh/crypto/ssh_hmac_sha256.h"
#include "network_drivers/presentation/ssh/crypto/ssh_sha256.h"
#include <string.h>

namespace
{
// Transcript-Hash of the empty string: SHA-256(""), the context for the two "derived" steps.
void empty_hash(uint8_t out[TLS13_SECRET_LEN])
{
    ssh_sha256(nullptr, 0, out);
}
} // namespace

void tls13_derive_secret(const uint8_t secret[TLS13_SECRET_LEN], const char *label,
                         const uint8_t transcript_hash[TLS13_SECRET_LEN], uint8_t out[TLS13_SECRET_LEN])
{
    // Derive-Secret(Secret, Label, Messages) = HKDF-Expand-Label(Secret, Label, Hash(Messages), L).
    quic_hkdf_expand_label_ctx(secret, label, transcript_hash, TLS13_SECRET_LEN, out, TLS13_SECRET_LEN);
}

void tls13_ks_early(Tls13KeySchedule *ks)
{
    // No PSK: Early Secret = HKDF-Extract(salt=0, IKM=0^Hash.length). HMAC zero-pads a short/absent
    // key, so an empty salt and a 32-zero-byte IKM reproduce the RFC 8448 early secret exactly.
    uint8_t zeros[TLS13_SECRET_LEN];
    memset(zeros, 0, sizeof(zeros));
    quic_hkdf_extract(nullptr, 0, zeros, sizeof(zeros), ks->early_secret);
}

void tls13_ks_handshake(Tls13KeySchedule *ks, const uint8_t ecdhe[TLS13_SECRET_LEN],
                        const uint8_t ch_sh_hash[TLS13_SECRET_LEN])
{
    // Handshake Secret = HKDF-Extract(Derive-Secret(Early, "derived", ""), (EC)DHE).
    uint8_t eh[TLS13_SECRET_LEN];
    empty_hash(eh);
    uint8_t derived[TLS13_SECRET_LEN];
    tls13_derive_secret(ks->early_secret, "derived", eh, derived);
    quic_hkdf_extract(derived, sizeof(derived), ecdhe, TLS13_SECRET_LEN, ks->handshake_secret);

    tls13_derive_secret(ks->handshake_secret, "c hs traffic", ch_sh_hash, ks->client_hs_traffic);
    tls13_derive_secret(ks->handshake_secret, "s hs traffic", ch_sh_hash, ks->server_hs_traffic);
}

void tls13_ks_master(Tls13KeySchedule *ks, const uint8_t ch_sfin_hash[TLS13_SECRET_LEN])
{
    // Master Secret = HKDF-Extract(Derive-Secret(Handshake, "derived", ""), 0^Hash.length).
    uint8_t eh[TLS13_SECRET_LEN];
    empty_hash(eh);
    uint8_t derived[TLS13_SECRET_LEN];
    tls13_derive_secret(ks->handshake_secret, "derived", eh, derived);
    uint8_t zeros[TLS13_SECRET_LEN];
    memset(zeros, 0, sizeof(zeros));
    quic_hkdf_extract(derived, sizeof(derived), zeros, sizeof(zeros), ks->master_secret);

    tls13_derive_secret(ks->master_secret, "c ap traffic", ch_sfin_hash, ks->client_ap_traffic);
    tls13_derive_secret(ks->master_secret, "s ap traffic", ch_sfin_hash, ks->server_ap_traffic);
}

void tls13_finished_mac(const uint8_t base_secret[TLS13_SECRET_LEN], const uint8_t transcript_hash[TLS13_SECRET_LEN],
                        uint8_t out[TLS13_SECRET_LEN])
{
    // finished_key = HKDF-Expand-Label(base_secret, "finished", "", L); verify_data = HMAC(fk, Hash).
    uint8_t finished_key[TLS13_SECRET_LEN];
    quic_hkdf_expand_label(base_secret, "finished", finished_key, sizeof(finished_key));
    ssh_hmac_sha256(finished_key, sizeof(finished_key), transcript_hash, TLS13_SECRET_LEN, out);
}

#endif // DETWS_ENABLE_HTTP3
