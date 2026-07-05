// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file quic_crypto.cpp
 * @brief QUIC packet protection and Initial secrets (see quic_crypto.h).
 */

#include "network_drivers/presentation/http3/quic_crypto.h"

#if DETWS_ENABLE_HTTP3

#include "network_drivers/presentation/http3/quic_aead.h"
#include "network_drivers/presentation/http3/quic_hkdf.h"
#include "network_drivers/presentation/http3/quic_packet.h"
#include <string.h>

namespace
{
// RFC 9001 sec 5.2: the version-1 Initial salt.
const uint8_t INITIAL_SALT[20] = {0x38, 0x76, 0x2c, 0xf7, 0xf5, 0x59, 0x34, 0xb3, 0x4d, 0x17,
                                  0x9a, 0xe6, 0xa4, 0xc8, 0x0c, 0xad, 0xcc, 0xbb, 0x7f, 0x0a};

// RFC 9001 sec 5.8: the version-1 Retry integrity key and nonce.
const uint8_t RETRY_KEY[16] = {0xbe, 0x0c, 0x69, 0x0b, 0x9f, 0x66, 0x57, 0x5a,
                               0x1d, 0x76, 0x6b, 0x54, 0xe3, 0x68, 0xc8, 0x4e};
const uint8_t RETRY_NONCE[12] = {0x46, 0x15, 0x99, 0xd3, 0x5d, 0x63, 0x2b, 0xf2, 0x23, 0x98, 0x25, 0xbb};

// Build the AEAD nonce: the packet number, left-padded to the 12-byte IV width, XOR the IV.
void build_nonce(const uint8_t iv[12], uint64_t full_pn, uint8_t nonce[12])
{
    memcpy(nonce, iv, 12);
    for (int i = 0; i < 8; i++)
        nonce[11 - i] ^= (uint8_t)(full_pn >> (8 * i));
}
} // namespace

void quic_keys_from_secret(const uint8_t secret[QUIC_HKDF_HASH_LEN], QuicPacketKeys *out)
{
    // RFC 9001 sec 5.1: every encryption level's packet keys are these three Expand-Labels of the
    // level's traffic secret (the Initial secrets below, or the TLS handshake / application secrets).
    quic_hkdf_expand_label(secret, "quic key", out->key, sizeof(out->key));
    quic_hkdf_expand_label(secret, "quic iv", out->iv, sizeof(out->iv));
    quic_hkdf_expand_label(secret, "quic hp", out->hp, sizeof(out->hp));
}

void quic_derive_initial_secrets(const uint8_t *dcid, size_t dcid_len, QuicInitialSecrets *out)
{
    uint8_t initial_secret[QUIC_HKDF_HASH_LEN];
    quic_hkdf_extract(INITIAL_SALT, sizeof(INITIAL_SALT), dcid, dcid_len, initial_secret);

    uint8_t client_secret[QUIC_HKDF_HASH_LEN];
    uint8_t server_secret[QUIC_HKDF_HASH_LEN];
    quic_hkdf_expand_label(initial_secret, "client in", client_secret, sizeof(client_secret));
    quic_hkdf_expand_label(initial_secret, "server in", server_secret, sizeof(server_secret));

    quic_keys_from_secret(client_secret, &out->client);
    quic_keys_from_secret(server_secret, &out->server);
}

size_t quic_packet_protect(uint8_t *pkt, size_t cap, size_t pn_offset, uint8_t pn_len, uint64_t full_pn,
                           size_t payload_len, const QuicPacketKeys *keys, bool is_long)
{
    if (pn_len < 1 || pn_len > 4)
        return 0;
    size_t hdr_len = pn_offset + pn_len;
    size_t total = hdr_len + payload_len + QUIC_AEAD_TAG_LEN;
    if (total > cap)
        return 0;

    // AEAD-seal the payload in place; associated data is the unprotected header.
    uint8_t nonce[12];
    build_nonce(keys->iv, full_pn, nonce);
    quic_aes128_gcm_seal(keys->key, nonce, pkt, hdr_len, pkt + hdr_len, payload_len, pkt + hdr_len);

    // Header protection (RFC 9001 sec 5.4): sample 16 bytes at pn_offset + 4 (always inside the
    // ciphertext because pn_len <= 4), AES-ECB it under hp, mask the low first-byte bits and the PN.
    QuicAes128 hp;
    quic_aes128_init(&hp, keys->hp);
    uint8_t mask[16];
    quic_aes128_encrypt_block(&hp, pkt + pn_offset + 4, mask);
    quic_aes128_wipe(&hp);

    pkt[0] ^= mask[0] & (is_long ? 0x0f : 0x1f);
    for (uint8_t i = 0; i < pn_len; i++)
        pkt[pn_offset + i] ^= mask[1 + i];

    return total;
}

size_t quic_packet_unprotect(uint8_t *pkt, size_t pn_offset, size_t length, uint64_t largest_pn,
                             const QuicPacketKeys *keys, bool is_long, uint8_t *out, uint64_t *out_pn)
{
    // Header protection needs a full 16-byte sample starting at pn_offset + 4, and the AEAD region
    // must carry at least the 16-byte tag once the (<=4-byte) packet number is removed.
    if (length < 4 + QUIC_AEAD_TAG_LEN)
        return (size_t)-1;

    QuicAes128 hp;
    quic_aes128_init(&hp, keys->hp);
    uint8_t mask[16];
    quic_aes128_encrypt_block(&hp, pkt + pn_offset + 4, mask);
    quic_aes128_wipe(&hp);

    pkt[0] ^= mask[0] & (is_long ? 0x0f : 0x1f);
    uint8_t pn_len = (uint8_t)((pkt[0] & 0x03) + 1);

    uint64_t truncated_pn = 0;
    for (uint8_t i = 0; i < pn_len; i++)
    {
        pkt[pn_offset + i] ^= mask[1 + i];
        truncated_pn = (truncated_pn << 8) | pkt[pn_offset + i];
    }
    uint64_t full_pn = quic_pn_decode(largest_pn, truncated_pn, (uint8_t)(pn_len * 8));
    if (out_pn)
        *out_pn = full_pn;

    size_t hdr_len = pn_offset + pn_len;
    size_t ct_len = length - pn_len; // ciphertext + tag
    uint8_t nonce[12];
    build_nonce(keys->iv, full_pn, nonce);
    if (!quic_aes128_gcm_open(keys->key, nonce, pkt, hdr_len, pkt + hdr_len, ct_len, out))
        return (size_t)-1;

    return ct_len - QUIC_AEAD_TAG_LEN;
}

void quic_retry_integrity_tag(const uint8_t *odcid, size_t odcid_len, const uint8_t *retry, size_t retry_len,
                              uint8_t tag[16])
{
    // AAD = Retry Pseudo-Packet: ODCID Length (1 byte) || ODCID || Retry packet (sans tag).
    // Assemble it into a scratch buffer; a Retry is small (short token), so a fixed cap suffices.
    uint8_t aad[1 + QUIC_MAX_CID_LEN + 256];
    size_t p = 0;
    aad[p++] = (uint8_t)odcid_len;
    if (odcid_len > QUIC_MAX_CID_LEN || 1 + odcid_len + retry_len > sizeof(aad))
    {
        memset(tag, 0, 16);
        return;
    }
    memcpy(aad + p, odcid, odcid_len);
    p += odcid_len;
    memcpy(aad + p, retry, retry_len);
    p += retry_len;

    // Empty plaintext: seal writes only the 16-byte tag.
    quic_aes128_gcm_seal(RETRY_KEY, RETRY_NONCE, aad, p, nullptr, 0, tag);
}

#endif // DETWS_ENABLE_HTTP3
