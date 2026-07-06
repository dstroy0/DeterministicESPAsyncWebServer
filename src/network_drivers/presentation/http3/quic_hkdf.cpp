// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file quic_hkdf.cpp
 * @brief HKDF-SHA256 and TLS 1.3 HKDF-Expand-Label (see quic_hkdf.h).
 */

#include "network_drivers/presentation/http3/quic_hkdf.h"

#if DETWS_ENABLE_HTTP3

#include "network_drivers/presentation/ssh/crypto/ssh_hmac_sha256.h"
#include <string.h>

void quic_hkdf_extract(const uint8_t *salt, size_t salt_len, const uint8_t *ikm, size_t ikm_len,
                       uint8_t prk[QUIC_HKDF_HASH_LEN])
{
    // RFC 5869 sec 2.2: PRK = HMAC-Hash(salt, IKM). ssh_hmac_sha256 pre-hashes keys > 64 bytes and
    // zero-pads shorter ones, which is exactly HMAC's own key handling, so the salt goes in as-is.
    ssh_hmac_sha256(salt, salt_len, ikm, ikm_len, prk);
}

namespace
{
// RFC 5869 sec 2.3 HKDF-Expand for the QUIC case: the info block is small and fixed and the
// requested length never exceeds one hash block, but the general N-block loop is written out so a
// future >32-byte caller stays correct. T(i) = HMAC(PRK, T(i-1) || info || i), i counts from 1.
void hkdf_expand(const uint8_t prk[QUIC_HKDF_HASH_LEN], const uint8_t *info, size_t info_len, uint8_t *out,
                 size_t out_len)
{
    uint8_t t[QUIC_HKDF_HASH_LEN];
    size_t t_len = 0; // 0 for T(0) (empty), QUIC_HKDF_HASH_LEN afterwards
    size_t done = 0;
    uint8_t counter = 0;
    while (done < out_len)
    {
        counter++;
        SshHmacCtx ctx;
        ssh_hmac_sha256_init(&ctx, prk, QUIC_HKDF_HASH_LEN);
        ssh_hmac_sha256_update(&ctx, t, t_len);
        ssh_hmac_sha256_update(&ctx, info, info_len);
        ssh_hmac_sha256_update(&ctx, &counter, 1);
        ssh_hmac_sha256_final(&ctx, t);
        t_len = QUIC_HKDF_HASH_LEN;

        size_t take = out_len - done;
        if (take > QUIC_HKDF_HASH_LEN)
            take = QUIC_HKDF_HASH_LEN;
        memcpy(out + done, t, take);
        done += take;
    }
}
} // namespace

void quic_hkdf_expand_label_ctx(const uint8_t secret[QUIC_HKDF_HASH_LEN], const char *label, const uint8_t *context,
                                size_t context_len, uint8_t *out, size_t out_len)
{
    // HkdfLabel (RFC 8446 sec 7.1): uint16 length | opaque label<..> = "tls13 " + label | opaque context.
    // Label length maxes out well under 255 (longest is "tls13 client in" = 15); the context is a
    // Transcript-Hash (<= 32) for Derive-Secret and empty for packet-protection keys. A fixed
    // 2 + 1 + 255 + 1 + 255 scratch buffer covers every caller.
    static const char PREFIX[6] = {'t', 'l', 's', '1', '3', ' '};
    size_t label_len = strlen(label);
    uint8_t info[2 + 1 + 255 + 1 + 255];
    size_t p = 0;
    info[p++] = (uint8_t)(out_len >> 8);
    info[p++] = (uint8_t)(out_len & 0xff);
    info[p++] = (uint8_t)(sizeof(PREFIX) + label_len); // full label length, prefix included
    memcpy(info + p, PREFIX, sizeof(PREFIX));
    p += sizeof(PREFIX);
    memcpy(info + p, label, label_len);
    p += label_len;
    info[p++] = (uint8_t)context_len;
    if (context_len)
    {
        memcpy(info + p, context, context_len);
        p += context_len;
    }

    hkdf_expand(secret, info, p, out, out_len);
}

void quic_hkdf_expand_label(const uint8_t secret[QUIC_HKDF_HASH_LEN], const char *label, uint8_t *out, size_t out_len)
{
    quic_hkdf_expand_label_ctx(secret, label, nullptr, 0, out, out_len);
}

#endif // DETWS_ENABLE_HTTP3
