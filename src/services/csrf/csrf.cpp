// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file csrf.cpp
 * @brief Stateless HMAC-signed CSRF token implementation (DETWS_ENABLE_CSRF).
 *
 * The token is `<nonce_hex>.<sig_hex>`; the signature is the first CSRF_SIG_BYTES
 * of HMAC-SHA256(secret, nonce). Verification recomputes the HMAC over the
 * embedded nonce and constant-time compares - no server-side session state.
 */

#include "csrf.h"

#if DETWS_ENABLE_CSRF

#include "network_drivers/presentation/ssh/ssh_hmac_sha256.h"
#include "shared_primitives/det_hex.h"
#include <stdio.h>
#include <string.h>

namespace
{

uint8_t g_secret[32];
size_t g_secret_len = 0;
uint64_t g_counter = 0;

// hex encode/decode now via the shared det_hex.h primitive.

// Constant-time compare of n characters (no early exit on mismatch).
bool ct_equal(const char *a, const char *b, size_t n)
{
    uint8_t diff = 0;
    for (size_t i = 0; i < n; i++)
        diff |= (uint8_t)(a[i] ^ b[i]);
    return diff == 0;
}

// Hex of the truncated HMAC-SHA256(secret, nonce) into sig_hex (2*CSRF_SIG_BYTES + 1).
void sign_nonce(const uint8_t *nonce, size_t nlen, char *sig_hex)
{
    uint8_t mac[SSH_HMAC_SHA256_LEN];
    ssh_hmac_sha256(g_secret, g_secret_len, nonce, nlen, mac);
    det_hex_encode(mac, CSRF_SIG_BYTES, sig_hex); // truncate the MAC to CSRF_SIG_BYTES
}

} // namespace

void csrf_set_secret(const uint8_t *secret, size_t len)
{
    if (!secret)
    {
        g_secret_len = 0;
        return;
    }
    g_secret_len = len > sizeof(g_secret) ? sizeof(g_secret) : len;
    memcpy(g_secret, secret, g_secret_len);
}

int csrf_issue(char *out, size_t cap)
{
    if (g_secret_len == 0 || !out || cap < CSRF_TOKEN_BUF)
        return 0;

    uint8_t nonce[CSRF_NONCE_BYTES];
    uint64_t c = ++g_counter;
    for (size_t i = 0; i < CSRF_NONCE_BYTES; i++)
        nonce[i] = (uint8_t)(c >> (8 * i));

    char nhex[CSRF_NONCE_BYTES * 2 + 1];
    char shex[CSRF_SIG_BYTES * 2 + 1];
    det_hex_encode(nonce, CSRF_NONCE_BYTES, nhex);
    sign_nonce(nonce, CSRF_NONCE_BYTES, shex);

    int n = snprintf(out, cap, "%s.%s", nhex, shex);
    return (n > 0 && (size_t)n < cap) ? n : 0;
}

bool csrf_verify(const char *token)
{
    if (g_secret_len == 0 || !token)
        return false;

    const char *dot = strchr(token, '.');
    if (!dot)
        return false;

    size_t nhexlen = (size_t)(dot - token);
    if (nhexlen != CSRF_NONCE_BYTES * 2)
        return false;

    uint8_t nonce[CSRF_NONCE_BYTES];
    if (det_hex_decode(token, nhexlen, nonce, sizeof(nonce)) != CSRF_NONCE_BYTES)
        return false;

    const char *sig = dot + 1;
    if (strlen(sig) != CSRF_SIG_BYTES * 2)
        return false;

    char expect[CSRF_SIG_BYTES * 2 + 1];
    sign_nonce(nonce, CSRF_NONCE_BYTES, expect);
    return ct_equal(sig, expect, CSRF_SIG_BYTES * 2);
}

void csrf_reset(void)
{
    memset(g_secret, 0, sizeof(g_secret));
    g_secret_len = 0;
    g_counter = 0;
}

#endif // DETWS_ENABLE_CSRF
