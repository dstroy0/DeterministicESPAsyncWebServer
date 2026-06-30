// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file jwt.cpp
 * @brief JWT HS256 verification + claim extraction (base64url over base64).
 */

#include "services/jwt/jwt.h"

#if DETWS_ENABLE_JWT

#include "network_drivers/presentation/base64/base64.h"
#include "network_drivers/presentation/ssh/ssh_hmac_sha256.h"
#include "shared_primitives/shim.h"

// Constant-time equality over @p n bytes (no early-out timing oracle).
static bool ct_eq(const char *a, const char *b, size_t n)
{
    uint8_t diff = 0;
    for (size_t i = 0; i < n; i++)
        diff |= (uint8_t)(a[i] ^ b[i]);
    return diff == 0;
}

// base64url encode/decode are shared with OIDC in the base64 module
// (base64url_encode / base64url_decode).

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

// RFC 7515 §5.2: the algorithm used MUST be the one named by the JWS header "alg".
// Decode the header segment and require alg == "HS256" - this rejects "none",
// RS256, HS384, and any other algorithm-substitution attempt before the HMAC check.
static bool jwt_header_alg_is_hs256(const char *header, size_t hlen)
{
    uint8_t buf[96];
    size_t n = base64url_decode(header, hlen, buf, sizeof(buf) - 1);
    if (n == 0)
        return false;
    buf[n] = '\0';
    const char *p = strstr((const char *)buf, "\"alg\"");
    if (!p)
        return false;
    p += 5;
    while (*p == ' ' || *p == ':' || *p == '\t')
        p++;
    if (*p != '"')
        return false;
    p++;
    return strncmp(p, "HS256", 5) == 0 && p[5] == '"';
}

bool jwt_verify_hs256(const char *token, size_t token_len, const uint8_t *secret, size_t secret_len)
{
    if (!token || token_len < 5 || token_len > DETWS_JWT_MAX_LEN)
        return false;

    size_t signing_len, sig_len;
    const char *sig;
    if (!jwt_split(token, token_len, &signing_len, &sig, &sig_len))
        return false;

    // Validate the declared algorithm matches what we verify (RFC 7515 §5.2).
    const char *d1 = (const char *)memchr(token, '.', token_len);
    if (!jwt_header_alg_is_hs256(token, (size_t)(d1 - token)))
        return false;

    // HS256 -> 32-byte MAC -> 43 base64url chars (no padding).
    if (sig_len != 43)
        return false;

    uint8_t mac[SSH_HMAC_SHA256_LEN];
    ssh_hmac_sha256(secret, secret_len, (const uint8_t *)token, signing_len, mac);

    char computed[48];
    if (base64url_encode(mac, sizeof(mac), computed) != 43)
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
    size_t n = base64url_decode(payload, payload_len, buf, sizeof(buf) - 1);
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

bool jwt_claim_str(const char *token, size_t token_len, const char *name, char *out, size_t out_cap)
{
    if (!token || !name || !out || out_cap == 0)
        return false;
    out[0] = '\0';

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
    size_t n = base64url_decode(payload, payload_len, buf, sizeof(buf) - 1);
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
    if (*p != '"') // not a string-valued claim
        return false;
    p++;
    size_t i = 0;
    while (*p && *p != '"' && i + 1 < out_cap)
    {
        if (*p == '\\' && p[1]) // minimal unescape: drop the backslash, copy the next char
            p++;
        out[i++] = *p++;
    }
    if (*p != '"') // unterminated string or value too long for out
    {
        out[0] = '\0';
        return false;
    }
    out[i] = '\0';
    return true;
}

bool jwt_scope_allows(const char *scope_claim, const char *required)
{
    if (!scope_claim || !required || !*required)
        return false;
    size_t rlen = strlen(required);
    const char *p = scope_claim;
    while (*p)
    {
        while (*p == ' ')
            p++;
        const char *start = p;
        while (*p && *p != ' ')
            p++;
        if ((size_t)(p - start) == rlen && memcmp(start, required, rlen) == 0)
            return true;
    }
    return false;
}

#endif // DETWS_ENABLE_JWT
