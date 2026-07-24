// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file esp.cpp
 * @brief ESP (RFC 4303) packet transform with AES-256-GCM (RFC 4106) - see esp.h.
 */

#include "services/esp/esp.h"

#if DWS_ENABLE_IKEV2

#include "network_drivers/presentation/ssh/crypto/ssh_aesgcm.h"
#include <string.h>

namespace
{
void put32be(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}
uint32_t get32be(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}
void esp_nonce(uint8_t nonce[SSH_AESGCM_IV_LEN], const uint8_t *salt, const uint8_t *iv)
{
    memcpy(nonce, salt, DWS_ESP_SALT_LEN);
    memcpy(nonce + DWS_ESP_SALT_LEN, iv, DWS_ESP_IV_LEN);
}
// Bytes the ESP header + IV occupy before the ciphertext (also the AAD length = SPI|Seq).
constexpr size_t ESP_CT_OFF = DWS_ESP_HDR_LEN + DWS_ESP_IV_LEN;
} // namespace

size_t dws_esp_gcm_encapsulate(uint32_t spi, uint32_t seq, const uint8_t key[DWS_ESP_KEY_LEN],
                               const uint8_t salt[DWS_ESP_SALT_LEN], const uint8_t iv[DWS_ESP_IV_LEN],
                               uint8_t next_header, const uint8_t *payload, size_t payload_len, uint8_t *out,
                               size_t out_cap)
{
    if (!key || !salt || !iv || !out || (payload_len && !payload))
        return 0;

    // Plaintext = Payload | Padding | Pad Length | Next Header, padded so Pad Length + Next Header (the
    // 2-octet trailer) land on a 4-octet boundary (RFC 4303 §2.4).
    size_t padn = (4 - (payload_len + 2) % 4) % 4;
    size_t pt_len = payload_len + padn + 2;
    size_t total = ESP_CT_OFF + pt_len + DWS_ESP_ICV_LEN;
    if (out_cap < total)
        return 0;

    put32be(out, spi);
    put32be(out + 4, seq);
    memcpy(out + DWS_ESP_HDR_LEN, iv, DWS_ESP_IV_LEN);

    uint8_t *pt = out + ESP_CT_OFF;
    if (payload_len)
        memcpy(pt, payload, payload_len);
    for (size_t i = 0; i < padn; i++)
        pt[payload_len + i] = (uint8_t)(i + 1); // RFC 4303 monotonic padding 1, 2, 3 ...
    pt[payload_len + padn] = (uint8_t)padn;     // Pad Length
    pt[payload_len + padn + 1] = next_header;   // Next Header

    // Encrypt in place: AAD = SPI | Seq (the first 8 octets), plaintext -> ciphertext || ICV at ESP_CT_OFF.
    uint8_t nonce[SSH_AESGCM_IV_LEN];
    esp_nonce(nonce, salt, iv);
    SshAesGcmCtx ctx;
    ssh_aesgcm_init(&ctx, key, nonce);
    ssh_aesgcm_seal(&ctx, out, DWS_ESP_HDR_LEN, pt, pt_len, pt);
    ssh_aesgcm_wipe(&ctx);
    return total;
}

bool dws_esp_gcm_decapsulate(const uint8_t key[DWS_ESP_KEY_LEN], const uint8_t salt[DWS_ESP_SALT_LEN], uint8_t *packet,
                             size_t len, uint32_t *spi_out, uint32_t *seq_out, uint8_t *next_header_out,
                             const uint8_t **payload_out, size_t *payload_len_out)
{
    if (!key || !salt || !packet || !payload_out || !payload_len_out)
        return false;
    // Minimum: header + IV + at least the 2-octet trailer (Pad Length + Next Header) + ICV.
    if (len < ESP_CT_OFF + 2 + DWS_ESP_ICV_LEN)
        return false;

    const uint8_t *iv = packet + DWS_ESP_HDR_LEN;
    uint8_t *ct = packet + ESP_CT_OFF;
    size_t ct_len = len - ESP_CT_OFF - DWS_ESP_ICV_LEN;
    const uint8_t *tag = ct + ct_len;

    uint8_t nonce[SSH_AESGCM_IV_LEN];
    esp_nonce(nonce, salt, iv);
    SshAesGcmCtx ctx;
    ssh_aesgcm_init(&ctx, key, nonce);
    bool ok = ssh_aesgcm_open(&ctx, packet, DWS_ESP_HDR_LEN, ct, ct_len, tag, ct); // AAD = SPI | Seq
    ssh_aesgcm_wipe(&ctx);
    if (!ok)
        return false;

    // Trailer: the last octet is Next Header, the one before it is Pad Length.
    uint8_t next_header = ct[ct_len - 1];
    uint8_t pad_len = ct[ct_len - 2];
    if ((size_t)pad_len + 2 > ct_len) // padding + trailer cannot exceed the plaintext
        return false;

    if (spi_out)
        *spi_out = get32be(packet);
    if (seq_out)
        *seq_out = get32be(packet + 4);
    if (next_header_out)
        *next_header_out = next_header;
    *payload_out = ct;
    *payload_len_out = ct_len - 2 - pad_len;
    return true;
}

// ── ESP anti-replay window (RFC 4303 §3.4.3) ───────────────────────────────────────────────────

void dws_esp_replay_init(EspReplay *r)
{
    if (!r)
        return;
    r->highest = 0;
    r->bitmap = 0;
    r->seen_any = false;
}

bool dws_esp_replay_check(EspReplay *r, uint32_t seq)
{
    if (!r || seq == 0) // sequence 0 is never valid (ESP counts from 1)
        return false;

    if (!r->seen_any)
    {
        r->highest = seq;
        r->bitmap = 1; // bit 0 = this (the new highest)
        r->seen_any = true;
        return true;
    }

    if (seq > r->highest)
    {
        // A new highest: slide the window up, then mark the new top bit. A jump >= the window clears it.
        uint32_t shift = seq - r->highest;
        r->bitmap = (shift >= DWS_ESP_REPLAY_WINDOW) ? 0u : (r->bitmap << shift);
        r->bitmap |= 1u;
        r->highest = seq;
        return true;
    }

    uint32_t offset = r->highest - seq;
    if (offset >= DWS_ESP_REPLAY_WINDOW) // left of the window -> too old
        return false;
    uint64_t mask = (uint64_t)1 << offset;
    if (r->bitmap & mask) // already accepted -> replay
        return false;
    r->bitmap |= mask;
    return true;
}

#endif // DWS_ENABLE_IKEV2
