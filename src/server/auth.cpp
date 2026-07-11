// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file auth.cpp
 * @brief HTTP authentication for DetWebServer: Basic (RFC 7617) and stateless Digest
 *        (RFC 7616, SHA-256, qop=auth).
 *
 * Split out of dwserver.cpp (single-purpose server files). Holds the Basic credential check,
 * the Digest field parser, the keyed stateless-nonce mint/verify (no per-nonce server state),
 * and the 401 challenge builder. The route dispatcher (dwserver.cpp) calls these DetWebServer
 * methods when a matched route carries auth. Behaviour is identical to the pre-split code.
 */

#include "dwserver.h"
#include "network_drivers/presentation/base64/base64.h"         // base64_decode (Basic)
#include "network_drivers/presentation/ssh/crypto/ssh_sha256.h" // ssh_sha256, SSH_SHA256_DIGEST_LEN (Digest)
#include "network_drivers/transport/tcp.h"                      // conn_pool, det_conn_send, TcpConn/ConnState
#include "server/dwserver_internal.h"                           // req_is_head, resp helpers
#include "services/clock.h"                                     // detws_millis() for the stateless nonce
#include "shared_primitives/hex.h"                              // det_hex_encode/decode
#include <stdio.h>
#include <string.h>
#if DETWS_ENABLE_AUTH
#ifdef ARDUINO
#include <esp_system.h> // esp_random() for the Digest keying secret
#endif
#endif
// ---------------------------------------------------------------------------
// Basic Auth helpers
// ---------------------------------------------------------------------------

#if DETWS_ENABLE_AUTH
// One-shot SHA-256 of @p data, written as 64 lowercase hex chars + NUL.
static void sha256_hex(const uint8_t *data, size_t len, char out[65])
{
    uint8_t d[SSH_SHA256_DIGEST_LEN];
    ssh_sha256(data, len, d);
    det_hex_encode(d, SSH_SHA256_DIGEST_LEN, out);
}

// Extract the value of @p key from a Digest auth header into @p out.
// Handles both quoted ("value") and token (value) forms. The match must sit on
// a field boundary (start, or after ' '/',') and be immediately followed by '='
// so "nc" does not match inside "cnonce", etc.
static bool digest_field(const char *hdr, const char *key, char *out, size_t out_size)
{
    size_t klen = strnlen(key, 32);
    const char *p = hdr;
    while ((p = strstr(p, key)) != nullptr)
    {
        bool left_ok = (p == hdr) || p[-1] == ' ' || p[-1] == ',';
        const char *after = p + klen;
        if (left_ok && *after == '=')
        {
            after++;
            const char *vs;
            const char *ve;
            if (*after == '"')
            {
                vs = after + 1;
                ve = strchr(vs, '"');
                if (!ve)
                    return false;
            }
            else
            {
                vs = after;
                ve = vs;
                while (*ve && *ve != ',' && *ve != ' ')
                    ve++;
            }
            size_t vlen = (size_t)(ve - vs);
            if (vlen > out_size - 1)
                vlen = out_size - 1;
            memcpy(out, vs, vlen);
            out[vlen] = '\0';
            return true;
        }
        p = after;
    }
    return false;
}

void DetWebServer::regen_digest_secret()
{
    // Seed a 128-bit keying secret from the hardware CSPRNG (esp_random() on
    // ESP32; a non-crypto mock on native test builds), folded through SHA-256 with
    // a counter + millis() so even a weak host RNG yields distinct values across
    // calls. The secret keys every timestamped nonce this server issues; it lives
    // only in BSS and is never sent on the wire.
    static uint32_t counter = 0;
    counter++;
    uint8_t seed[24];
    for (int i = 0; i < 4; i++)
    {
        uint32_t r = esp_random();
        memcpy(seed + i * 4, &r, 4);
    }
    uint32_t c = counter;
    uint32_t t = (uint32_t)millis();
    memcpy(seed + 16, &c, 4);
    memcpy(seed + 20, &t, 4);
    uint8_t d[SSH_SHA256_DIGEST_LEN];
    ssh_sha256(seed, sizeof(seed), d);
    memcpy(_digest_secret, d, sizeof(_digest_secret)); // first 128 bits
}

// Stateless Digest nonce (RFC 7616 3.3): "<issue_ms_hex>.<mac_hex>" where the MAC
// is SHA-256(secret || issue_ms) truncated to 128 bits. The server holds no
// per-nonce state - it recomputes the MAC to authenticate a returned nonce and
// reads the embedded issue time to age it - so the scheme is safe under the
// shared-nothing worker model (the secret is set once at begin(), read-only after).
static uint32_t digest_nonce_mac(const uint8_t *secret, uint32_t issue, char *mac_hex)
{
    uint8_t material[20];
    memcpy(material, secret, 16);
    memcpy(material + 16, &issue, 4); // endian-symmetric: minted and verified the same way
    uint8_t d[SSH_SHA256_DIGEST_LEN];
    ssh_sha256(material, sizeof(material), d);
    det_hex_encode(d, 16, mac_hex); // 16 bytes -> 32 hex chars + NUL
    return issue;
}

void DetWebServer::make_digest_nonce(char *out, size_t cap)
{
    uint32_t issue = detws_millis();
    char issue_hex[9];
    det_hex_encode((const uint8_t *)&issue, 4, issue_hex); // 4 bytes -> 8 hex chars
    char mac_hex[33];
    digest_nonce_mac(_digest_secret, issue, mac_hex);
    snprintf(out, cap, "%s.%s", issue_hex, mac_hex);
}

bool DetWebServer::verify_digest_nonce(const char *nonce, bool *expired)
{
    *expired = false;
    // Expected shape: 8 hex (issue) + '.' + 32 hex (MAC).
    if (strnlen(nonce, 42) != 8 + 1 + 32 || nonce[8] != '.')
        return false;
    uint32_t issue;
    if (det_hex_decode(nonce, 8, (uint8_t *)&issue, 4) != 4)
        return false;
    char mac_hex[33];
    digest_nonce_mac(_digest_secret, issue, mac_hex);
    // Constant-time compare of the 32 MAC hex chars: a forged nonce never reveals
    // how many leading characters matched.
    const char *got = nonce + 9;
    uint8_t diff = 0;
    for (int i = 0; i < 32; i++)
        diff |= (uint8_t)(mac_hex[i] ^ got[i]);
    if (diff != 0)
        return false;                      // not a nonce this server minted
    uint32_t age = detws_millis() - issue; // unsigned: tolerant of the 32-bit millis wrap
    *expired = (age > DETWS_DIGEST_NONCE_LIFETIME_MS);
    return true;
}

void DetWebServer::send_unauth(uint8_t slot_id, const Route *r, bool stale)
{
    TcpConn *conn = &conn_pool[slot_id];
    if (conn->state != ConnState::CONN_ACTIVE || !conn->pcb)
    {
        http_reset(slot_id);
        return;
    }

    char challenge[MAX_AUTH_LEN + 128];
    if (r->auth_digest)
    {
        char nonce[48];
        make_digest_nonce(nonce, sizeof(nonce)); // a fresh, timestamped nonce per challenge
        snprintf(challenge, sizeof(challenge),
                 "WWW-Authenticate: Digest realm=\"%s\", qop=\"auth\", algorithm=SHA-256, nonce=\"%s\"%s\r\n",
                 r->auth_realm, nonce, stale ? ", stale=true" : "");
    }
    else
        snprintf(challenge, sizeof(challenge), "WWW-Authenticate: Basic realm=\"%s\"\r\n", r->auth_realm);

    bool keep;
    const char *cl = resp_conn_hdr(slot_id, &keep);

    static const char body[] = "Unauthorized";
    char header[RESP_HDR_BUF_SIZE];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 401 Unauthorized\r\n"
                        "%s"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: %d\r\n"
                        "%s"
                        "%s\r\n",
                        challenge, (int)(sizeof(body) - 1), _cors_enabled ? _cors_header_buf : "", cl);

    det_conn_send(slot_id, header, (u16_t)hlen);
    if (!req_is_head(slot_id))
        det_conn_send(slot_id, body, (u16_t)(sizeof(body) - 1));

    resp_end(slot_id, 401, (int)(sizeof(body) - 1), keep);
}

bool DetWebServer::check_basic_auth(uint8_t /*slot_id*/, HttpReq *req, const Route *r)
{
    const char *auth_hdr = http_get_header(req, "Authorization");
    if (!auth_hdr || strncmp(auth_hdr, "Basic ", 6) != 0)
        return false;

    uint8_t decoded[MAX_AUTH_LEN * 2 + 2];
    // Bound the write to leave room for the null terminator at decoded[n]; an
    // over-long Authorization value now fails the decode instead of overrunning.
    size_t n = base64_decode(auth_hdr + 6, decoded, sizeof(decoded) - 1);
    if (n == 0)
        return false;
    decoded[n] = '\0';

    const char *colon = (const char *)memchr(decoded, ':', n);
    if (!colon)
        return false;

    size_t ulen = (size_t)(colon - (const char *)decoded);
    const char *pass = colon + 1;

    return (ulen == strnlen(r->auth_user, MAX_AUTH_LEN)) && (memcmp(decoded, r->auth_user, ulen) == 0) &&
           (strcmp(pass, r->auth_pass) == 0);
}

// Validate an Authorization: Digest header (RFC 7616, SHA-256, qop=auth).
// HA1 = SHA256(user:realm:pass), HA2 = SHA256(method:uri),
// response = SHA256(HA1:nonce:nc:cnonce:qop:HA2).
bool DetWebServer::check_digest_auth(uint8_t /*slot_id*/, HttpReq *req, const Route *r, bool *stale)
{
    // Use the full-length Authorization capture (the scratch header value is
    // capped at MAX_VAL_LEN, far shorter than a Digest header).
    const char *hdr = req->authorization;
    if (strncmp(hdr, "Digest ", 7) != 0)
        return false;
    const char *d = hdr + 7;

    char username[MAX_AUTH_LEN];
    char nonce[48];
    char uri[MAX_PATH_LEN + MAX_QUERY_LEN + 2];
    char qop[16];
    char nc[16];
    char cnonce[64];
    char response[80];

    if (!digest_field(d, "username", username, sizeof(username)) || !digest_field(d, "nonce", nonce, sizeof(nonce)) ||
        !digest_field(d, "uri", uri, sizeof(uri)) || !digest_field(d, "qop", qop, sizeof(qop)) ||
        !digest_field(d, "nc", nc, sizeof(nc)) || !digest_field(d, "cnonce", cnonce, sizeof(cnonce)) ||
        !digest_field(d, "response", response, sizeof(response)))
        return false;

    // Identity + challenge binding must match before any hashing.
    if (strcmp(username, r->auth_user) != 0)
        return false;
    // The nonce must be one this server minted (authentic MAC). A stale (expired)
    // nonce is still authentic - we finish the credential check below and let the
    // caller reissue with stale=true rather than rejecting outright (RFC 7616 3.3).
    bool nonce_expired = false;
    if (!verify_digest_nonce(nonce, &nonce_expired))
        return false;
    if (strcmp(qop, "auth") != 0)
        return false;

    // RFC 7616 3.4: the resource named by the "uri" parameter MUST be the same as the
    // request target; otherwise a Digest response captured for one route could be
    // replayed against another route under the same realm/nonce.
    char target[MAX_PATH_LEN + MAX_QUERY_LEN + 2];
    if (req->query[0])
        snprintf(target, sizeof(target), "%s?%s", req->path, req->query);
    else
        snprintf(target, sizeof(target), "%s", req->path);
    if (strcmp(uri, target) != 0)
        return false;

    char tmp[3 * MAX_AUTH_LEN + 4];
    char ha1[65];
    char ha2[65];
    char expected[65];

    int n = snprintf(tmp, sizeof(tmp), "%s:%s:%s", r->auth_user, r->auth_realm, r->auth_pass);
    sha256_hex((const uint8_t *)tmp, (size_t)n, ha1);

    char tmp2[sizeof(uri) + 16];
    n = snprintf(tmp2, sizeof(tmp2), "%s:%s", req->method, uri);
    sha256_hex((const uint8_t *)tmp2, (size_t)n, ha2);

    char tmp3[65 + 48 + 16 + 64 + 8 + 65 + 8];
    n = snprintf(tmp3, sizeof(tmp3), "%s:%s:%s:%s:%s:%s", ha1, nonce, nc, cnonce, qop, ha2);
    sha256_hex((const uint8_t *)tmp3, (size_t)n, expected);

    if (strcasecmp(expected, response) != 0)
        return false; // wrong credentials - leave *stale untouched (no transparent retry)
    if (nonce_expired)
    {
        // Correct credentials but an aged nonce: signal a transparent retry so the
        // client recomputes against a fresh challenge without re-prompting the user.
        *stale = true;
        return false;
    }
    return true;
}
#endif // DETWS_ENABLE_AUTH
