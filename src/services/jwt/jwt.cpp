// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file jwt.cpp
 * @brief JWT HS256 verification + claim extraction (base64url over base64).
 */

#include "services/jwt/jwt.h"

#if DETWS_ENABLE_JWT

#include "network_drivers/presentation/base64.h"
#include "network_drivers/presentation/ssh/ssh_hmac_sha256.h"
#include <stdio.h>
#include <string.h>

// Constant-time equality over @p n bytes (no early-out timing oracle).
static bool ct_eq(const char *a, const char *b, size_t n)
{
    uint8_t diff = 0;
    for (size_t i = 0; i < n; i++)
        diff |= (uint8_t)(a[i] ^ b[i]);
    return diff == 0;
}

// base64url-encode @p in_len bytes into @p out (no '=' padding). @p out must hold
// 4*ceil(in_len/3)+1 bytes. Returns the number of characters written.
static size_t b64url_encode(const uint8_t *in, size_t in_len, char *out)
{
    base64_encode(in, in_len, out); // standard base64, '='-padded, NUL-terminated
    size_t n = 0;
    for (size_t i = 0; out[i]; i++)
    {
        char c = out[i];
        if (c == '=')
        {
            out[i] = '\0';
            break;
        }
        if (c == '+')
            out[i] = '-';
        else if (c == '/')
            out[i] = '_';
        n = i + 1;
    }
    out[n] = '\0';
    return n;
}

// base64url-decode @p in_len chars into @p out. Returns bytes decoded (0 on error).
static size_t b64url_decode(const char *in, size_t in_len, uint8_t *out, size_t out_cap)
{
    char tmp[DETWS_JWT_MAX_LEN + 4];
    if (in_len + 4 > sizeof(tmp))
        return 0;
    size_t j = 0;
    for (size_t i = 0; i < in_len; i++)
    {
        char c = in[i];
        if (c == '-')
            c = '+';
        else if (c == '_')
            c = '/';
        tmp[j++] = c;
    }
    while (j % 4)
        tmp[j++] = '=';
    tmp[j] = '\0';
    return base64_decode(tmp, out, out_cap);
}

// Split a compact JWT into header.payload (signing input) and the signature.
// Requires exactly two '.' separators. Returns false on a malformed shape.
static bool jwt_split(const char *token, size_t token_len, size_t *signing_len, const char **sig, size_t *sig_len)
{
    const char *d1 = (const char *)memchr(token, '.', token_len);
    if (!d1)
        return false;
    size_t rem = token_len - (size_t)(d1 + 1 - token);
    const char *d2 = (const char *)memchr(d1 + 1, '.', rem);
    if (!d2)
        return false;
    size_t rem2 = token_len - (size_t)(d2 + 1 - token);
    if (memchr(d2 + 1, '.', rem2)) // a third '.' is not a valid JWT
        return false;
    *signing_len = (size_t)(d2 - token);
    *sig = d2 + 1;
    *sig_len = rem2;
    return true;
}

bool jwt_verify_hs256(const char *token, size_t token_len, const uint8_t *secret, size_t secret_len)
{
    if (!token || token_len < 5 || token_len > DETWS_JWT_MAX_LEN)
        return false;

    size_t signing_len, sig_len;
    const char *sig;
    if (!jwt_split(token, token_len, &signing_len, &sig, &sig_len))
        return false;

    // HS256 -> 32-byte MAC -> 43 base64url chars (no padding).
    if (sig_len != 43)
        return false;

    uint8_t mac[SSH_HMAC_SHA256_LEN];
    ssh_hmac_sha256(secret, secret_len, (const uint8_t *)token, signing_len, mac);

    char computed[48];
    if (b64url_encode(mac, sizeof(mac), computed) != 43)
        return false;
    return ct_eq(computed, sig, 43);
}

bool jwt_bearer_valid(const char *auth_header, const uint8_t *secret, size_t secret_len)
{
    if (!auth_header || strncasecmp(auth_header, "Bearer ", 7) != 0)
        return false;
    const char *tok = auth_header + 7;
    while (*tok == ' ')
        tok++;
    return jwt_verify_hs256(tok, strlen(tok), secret, secret_len);
}

bool jwt_claim_int(const char *token, size_t token_len, const char *name, long *out)
{
    if (!token || !name || !out)
        return false;

    const char *d1 = (const char *)memchr(token, '.', token_len);
    if (!d1)
        return false;
    size_t rem = token_len - (size_t)(d1 + 1 - token);
    const char *d2 = (const char *)memchr(d1 + 1, '.', rem);
    if (!d2)
        return false;
    const char *payload = d1 + 1;
    size_t payload_len = (size_t)(d2 - payload);

    uint8_t buf[DETWS_JWT_MAX_LEN];
    size_t n = b64url_decode(payload, payload_len, buf, sizeof(buf) - 1);
    if (n == 0)
        return false;
    buf[n] = '\0';

    char key[40];
    int kn = snprintf(key, sizeof(key), "\"%s\"", name);
    if (kn <= 0 || kn >= (int)sizeof(key))
        return false;
    const char *p = strstr((const char *)buf, key);
    if (!p)
        return false;
    p += kn;
    while (*p == ' ' || *p == ':' || *p == '\t')
        p++;
    bool neg = false;
    if (*p == '-')
    {
        neg = true;
        p++;
    }
    if (*p < '0' || *p > '9')
        return false;
    long v = 0;
    while (*p >= '0' && *p <= '9')
        v = v * 10 + (*p++ - '0');
    *out = neg ? -v : v;
    return true;
}

#endif // DETWS_ENABLE_JWT
