// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file tls13_msg.cpp
 * @brief TLS 1.3 handshake messages for the QUIC handshake (see tls13_msg.h).
 */

#include "network_drivers/presentation/http3/tls13_msg.h"

#if DETWS_ENABLE_HTTP3

#include "network_drivers/presentation/ssh/crypto/ssh_ed25519.h"
#include <string.h>

// TLS extension types used here (RFC 8446 sec 4.2 + RFC 9001).
enum
{
    TLS_EXT_SERVER_NAME = 0x0000,
    TLS_EXT_SUPPORTED_GROUPS = 0x000a,
    TLS_EXT_SIGNATURE_ALGORITHMS = 0x000d,
    TLS_EXT_ALPN = 0x0010,
    TLS_EXT_SUPPORTED_VERSIONS = 0x002b,
    TLS_EXT_KEY_SHARE = 0x0033,
};

// ---------------------------------------------------------------------------
// A minimal bounds-checked byte writer (back-patches length prefixes).
// ---------------------------------------------------------------------------
namespace
{
struct Writer
{
    uint8_t *buf;
    size_t cap;
    size_t pos;
    bool ok;
};

void w_u8(Writer *w, uint8_t v)
{
    // pos <= cap is an invariant (every write advances pos by exactly the checked amount), so the
    // comparison is written to avoid a size_t addition that could wrap (cpp:S3519).
    if (w->pos >= w->cap)
    {
        w->ok = false;
        return;
    }
    w->buf[w->pos++] = v;
}
void w_u16(Writer *w, uint16_t v)
{
    w_u8(w, (uint8_t)(v >> 8));
    w_u8(w, (uint8_t)v);
}
void w_u24(Writer *w, uint32_t v)
{
    w_u8(w, (uint8_t)(v >> 16));
    w_u8(w, (uint8_t)(v >> 8));
    w_u8(w, (uint8_t)v);
}
void w_bytes(Writer *w, const uint8_t *b, size_t n)
{
    if (n == 0)
        return;
    // Overflow-safe capacity check. The ternary keeps room in [0, cap] whatever pos is, so the
    // cap - pos subtraction provably cannot underflow (an underflow would defeat the bound and let the
    // memcpy overrun buf - exactly the S3519 case), and pos + n is never formed (no wrap).
    size_t room = w->pos <= w->cap ? (size_t)(w->cap - w->pos) : (size_t)0;
    if (n > room)
    {
        w->ok = false;
        return;
    }
    memcpy(w->buf + w->pos, b, n);
    w->pos += n;
}
// Reserve a 2- or 3-byte length placeholder; returns its position for w_patch16/24.
size_t w_mark(Writer *w, size_t nbytes)
{
    size_t at = w->pos;
    for (size_t i = 0; i < nbytes; i++)
        w_u8(w, 0);
    return at;
}
void w_patch16(Writer *w, size_t at)
{
    if (!w->ok)
        return;
    uint16_t len = (uint16_t)(w->pos - at - 2);
    w->buf[at] = (uint8_t)(len >> 8);
    w->buf[at + 1] = (uint8_t)len;
}
void w_patch24(Writer *w, size_t at)
{
    if (!w->ok)
        return;
    uint32_t len = (uint32_t)(w->pos - at - 3);
    w->buf[at] = (uint8_t)(len >> 16);
    w->buf[at + 1] = (uint8_t)(len >> 8);
    w->buf[at + 2] = (uint8_t)len;
}

// A bounds-checked reader.
struct Reader
{
    const uint8_t *buf;
    size_t len;
    size_t pos;
};
bool r_u8(Reader *r, uint8_t *v)
{
    if (r->pos + 1 > r->len)
        return false;
    *v = r->buf[r->pos++];
    return true;
}
bool r_u16(Reader *r, uint16_t *v)
{
    if (r->pos + 2 > r->len)
        return false;
    *v = (uint16_t)((r->buf[r->pos] << 8) | r->buf[r->pos + 1]);
    r->pos += 2;
    return true;
}
bool r_u24(Reader *r, uint32_t *v)
{
    if (r->pos + 3 > r->len)
        return false;
    *v = (uint32_t)((r->buf[r->pos] << 16) | (r->buf[r->pos + 1] << 8) | r->buf[r->pos + 2]);
    r->pos += 3;
    return true;
}
// Take a view of the next n bytes.
bool r_take(Reader *r, size_t n, const uint8_t **out)
{
    if (r->pos + n > r->len)
        return false;
    *out = r->buf + r->pos;
    r->pos += n;
    return true;
}
} // namespace

// ---------------------------------------------------------------------------
// ClientHello parsing
// ---------------------------------------------------------------------------
namespace
{
// Scan a list of 2-byte values for a target (used for versions/groups/sig algs).
bool list16_contains(const uint8_t *body, size_t body_len, size_t list_len, uint16_t target)
{
    if (list_len > body_len || (list_len % 2) != 0)
        return false;
    for (size_t i = 0; i + 1 < list_len; i += 2)
        if (((body[i] << 8) | body[i + 1]) == target)
            return true;
    return false;
}

void parse_extension(uint16_t type, const uint8_t *body, size_t blen, Tls13ClientHello *out)
{
    switch (type)
    {
    case TLS_EXT_SUPPORTED_VERSIONS: {
        // 1-byte list length, then 2-byte versions.
        if (blen < 1)
            return;
        size_t ll = body[0];
        out->offers_tls13 = list16_contains(body + 1, blen - 1, ll, TLS_VERSION_1_3);
        break;
    }
    case TLS_EXT_SUPPORTED_GROUPS: {
        if (blen < 2)
            return;
        size_t ll = (body[0] << 8) | body[1];
        out->offers_x25519 = list16_contains(body + 2, blen - 2, ll, TLS_GROUP_X25519);
        break;
    }
    case TLS_EXT_SIGNATURE_ALGORITHMS: {
        if (blen < 2)
            return;
        size_t ll = (body[0] << 8) | body[1];
        out->offers_ed25519 = list16_contains(body + 2, blen - 2, ll, TLS_SIG_ED25519);
        break;
    }
    case TLS_EXT_KEY_SHARE: {
        // 2-byte client_shares length, then KeyShareEntry { group(2), key_exchange<2> }.
        if (blen < 2)
            return;
        size_t ll = (body[0] << 8) | body[1];
        if (ll + 2 > blen)
            return;
        size_t i = 2, end = 2 + ll;
        while (i + 4 <= end)
        {
            uint16_t group = (uint16_t)((body[i] << 8) | body[i + 1]);
            uint16_t klen = (uint16_t)((body[i + 2] << 8) | body[i + 3]);
            i += 4;
            if (i + klen > end)
                return;
            if (group == TLS_GROUP_X25519 && klen == 32)
            {
                memcpy(out->client_x25519, body + i, 32);
                out->has_key_share = true;
            }
            i += klen;
        }
        break;
    }
    case TLS_EXT_ALPN: {
        // 2-byte list length, then entries: 1-byte name length + name.
        if (blen < 2)
            return;
        size_t ll = (body[0] << 8) | body[1];
        if (ll + 2 > blen)
            return;
        size_t i = 2, end = 2 + ll;
        while (i + 1 <= end)
        {
            size_t nl = body[i++];
            if (i + nl > end)
                return;
            if (nl == 2 && body[i] == 'h' && body[i + 1] == '3')
                out->offers_h3_alpn = true;
            i += nl;
        }
        break;
    }
    case TLS_EXT_QUIC_TRANSPORT_PARAMS:
        out->quic_tp = body;
        out->quic_tp_len = blen;
        break;
    case TLS_EXT_SERVER_NAME: {
        // ServerNameList: 2-byte length, then entries: type(1), name<2>. Take the first host_name.
        if (blen < 2)
            return;
        size_t ll = (body[0] << 8) | body[1];
        if (ll + 2 > blen)
            return;
        size_t i = 2, end = 2 + ll;
        if (i + 3 > end)
            return;
        uint8_t nt = body[i++];
        size_t nl = (body[i] << 8) | body[i + 1];
        i += 2;
        if (nt == 0 && i + nl <= end)
        {
            out->sni = body + i;
            out->sni_len = nl;
        }
        break;
    }
    default:
        break;
    }
}
} // namespace

bool tls13_parse_client_hello(const uint8_t *msg, size_t len, Tls13ClientHello *out)
{
    memset(out, 0, sizeof(*out));

    Reader r = {msg, len, 0};
    uint8_t type = 0;
    uint32_t body_len = 0;
    if (!r_u8(&r, &type) || type != TLS_HS_CLIENT_HELLO || !r_u24(&r, &body_len))
        return false;
    // The handshake body must fit; trailing bytes past it are not part of this message.
    if (r.pos + body_len > len)
        return false;
    r.len = r.pos + body_len;

    uint16_t legacy_version = 0;
    const uint8_t *random = nullptr;
    if (!r_u16(&r, &legacy_version) || !r_take(&r, 32, &random))
        return false;

    uint8_t sid_len = 0;
    if (!r_u8(&r, &sid_len) || sid_len > 32)
        return false;
    if (!r_take(&r, sid_len, &out->session_id))
        return false;
    out->session_id_len = sid_len;

    uint16_t cs_len = 0;
    const uint8_t *cs = nullptr;
    if (!r_u16(&r, &cs_len) || (cs_len % 2) != 0 || !r_take(&r, cs_len, &cs))
        return false;

    uint8_t comp_len = 0;
    const uint8_t *comp = nullptr;
    if (!r_u8(&r, &comp_len) || !r_take(&r, comp_len, &comp))
        return false;

    // Extensions (a ClientHello for TLS 1.3 always has them).
    uint16_t ext_total = 0;
    if (!r_u16(&r, &ext_total))
        return false;
    size_t ext_end = r.pos + ext_total;
    if (ext_end > r.len)
        return false;
    while (r.pos < ext_end)
    {
        uint16_t etype = 0, elen = 0;
        const uint8_t *ebody = nullptr;
        if (!r_u16(&r, &etype) || !r_u16(&r, &elen) || !r_take(&r, elen, &ebody))
            return false;
        parse_extension(etype, ebody, elen, out);
    }
    return true;
}

// ---------------------------------------------------------------------------
// Builders
// ---------------------------------------------------------------------------
size_t tls13_build_server_hello(uint8_t *out, size_t cap, const uint8_t random[32], const uint8_t *session_id,
                                uint8_t session_id_len, const uint8_t server_x25519[32])
{
    Writer w = {out, cap, 0, true};
    w_u8(&w, TLS_HS_SERVER_HELLO);
    size_t hs_len = w_mark(&w, 3);

    w_u16(&w, 0x0303); // legacy_version
    w_bytes(&w, random, 32);
    w_u8(&w, session_id_len);
    w_bytes(&w, session_id, session_id_len);
    w_u16(&w, TLS_CIPHER_AES_128_GCM_SHA256);
    w_u8(&w, 0x00); // legacy_compression_method

    size_t ext_len = w_mark(&w, 2);
    // key_share -> server KeyShareEntry { x25519, 32-byte key }. (Ordered key_share then
    // supported_versions to match the RFC 8448 sec 3 ServerHello; extension order is not significant.)
    w_u16(&w, TLS_EXT_KEY_SHARE);
    w_u16(&w, 4 + 32);
    w_u16(&w, TLS_GROUP_X25519);
    w_u16(&w, 32);
    w_bytes(&w, server_x25519, 32);
    // supported_versions -> selected 0x0304.
    w_u16(&w, TLS_EXT_SUPPORTED_VERSIONS);
    w_u16(&w, 2);
    w_u16(&w, TLS_VERSION_1_3);
    w_patch16(&w, ext_len);

    w_patch24(&w, hs_len);
    return w.ok ? w.pos : 0;
}

size_t tls13_build_encrypted_extensions(uint8_t *out, size_t cap, const uint8_t *quic_tp, size_t quic_tp_len)
{
    Writer w = {out, cap, 0, true};
    w_u8(&w, TLS_HS_ENCRYPTED_EXTENSIONS);
    size_t hs_len = w_mark(&w, 3);

    size_t ext_len = w_mark(&w, 2);
    // ALPN -> ProtocolNameList [ "h3" ].
    w_u16(&w, TLS_EXT_ALPN);
    w_u16(&w, 5); // ext body length: 2 (list len) + 1 + 2
    w_u16(&w, 3); // ProtocolNameList length
    w_u8(&w, 2);  // name length
    w_bytes(&w, (const uint8_t *)"h3", 2);
    // quic_transport_parameters.
    w_u16(&w, TLS_EXT_QUIC_TRANSPORT_PARAMS);
    w_u16(&w, (uint16_t)quic_tp_len);
    w_bytes(&w, quic_tp, quic_tp_len);
    w_patch16(&w, ext_len);

    w_patch24(&w, hs_len);
    return w.ok ? w.pos : 0;
}

size_t tls13_build_certificate(uint8_t *out, size_t cap, const uint8_t *cert_der, size_t cert_len)
{
    Writer w = {out, cap, 0, true};
    w_u8(&w, TLS_HS_CERTIFICATE);
    size_t hs_len = w_mark(&w, 3);

    w_u8(&w, 0); // certificate_request_context: empty
    size_t list_len = w_mark(&w, 3);
    w_u24(&w, (uint32_t)cert_len); // CertificateEntry cert_data length
    w_bytes(&w, cert_der, cert_len);
    w_u16(&w, 0); // entry extensions: empty
    w_patch24(&w, list_len);

    w_patch24(&w, hs_len);
    return w.ok ? w.pos : 0;
}

size_t tls13_cert_verify_content(uint8_t *out, size_t cap, const uint8_t transcript_hash[32], bool is_server)
{
    // RFC 8446 sec 4.4.3: 64 spaces || context string || 0x00 || transcript hash.
    static const char *SRV = "TLS 1.3, server CertificateVerify";
    static const char *CLI = "TLS 1.3, client CertificateVerify";
    const char *ctx = is_server ? SRV : CLI;
    size_t ctx_len = strlen(ctx);
    size_t total = 64 + ctx_len + 1 + 32;
    if (total > cap)
        return 0;
    memset(out, 0x20, 64);
    memcpy(out + 64, ctx, ctx_len);
    out[64 + ctx_len] = 0x00;
    memcpy(out + 64 + ctx_len + 1, transcript_hash, 32);
    return total;
}

size_t tls13_build_cert_verify(uint8_t *out, size_t cap, const uint8_t transcript_hash[32], const uint8_t seed[32])
{
    uint8_t content[64 + 33 + 1 + 32];
    size_t clen = tls13_cert_verify_content(content, sizeof(content), transcript_hash, true);
    if (!clen)
        return 0;
    uint8_t sig[SSH_ED25519_SIG_LEN];
    ssh_ed25519_sign(sig, content, clen, seed);

    Writer w = {out, cap, 0, true};
    w_u8(&w, TLS_HS_CERTIFICATE_VERIFY);
    size_t hs_len = w_mark(&w, 3);
    w_u16(&w, TLS_SIG_ED25519);
    w_u16(&w, SSH_ED25519_SIG_LEN);
    w_bytes(&w, sig, SSH_ED25519_SIG_LEN);
    w_patch24(&w, hs_len);
    return w.ok ? w.pos : 0;
}

size_t tls13_build_finished(uint8_t *out, size_t cap, const uint8_t verify_data[32])
{
    Writer w = {out, cap, 0, true};
    w_u8(&w, TLS_HS_FINISHED);
    size_t hs_len = w_mark(&w, 3);
    w_bytes(&w, verify_data, 32);
    w_patch24(&w, hs_len);
    return w.ok ? w.pos : 0;
}

#endif // DETWS_ENABLE_HTTP3
