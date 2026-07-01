// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_transport.cpp
 * @brief SSH transport handshake - banner exchange and KEXINIT negotiation.
 */

#include "ssh_transport.h"
#include "ssh_bignum.h" // bn_*, SshBigNum
#include "ssh_dh.h"     // ssh_rng_fill(), ssh_dh[], ssh_dh_generate/derive_keys
#include "ssh_packet.h" // SSH_MSG_KEXINIT, ssh_pkt[]
#include "ssh_rsa.h"    // ssh_rsa_encode_pubkey/sign, SSH_RSA_*
#include "ssh_sha256.h"
#include <string.h>

SshSession ssh_sess[MAX_SSH_CONNS];

// ---------------------------------------------------------------------------
// Algorithms this server supports (exactly one per category).
// ---------------------------------------------------------------------------

static const char *const ALG_KEX = "diffie-hellman-group14-sha256";
// Our advertised kex_algorithms: the real KEX plus the RFC 8308 indicator
// ext-info-s, which tells the client we will send SSH_MSG_EXT_INFO (server-sig-algs).
static const char *const ALG_KEX_ADVERTISED = "diffie-hellman-group14-sha256,ext-info-s";
static const char *const ALG_HOSTKEY = "rsa-sha2-256";
static const char *const ALG_CIPHER = "aes256-ctr";
static const char *const ALG_MAC = "hmac-sha2-256";
static const char *const ALG_COMP = "none";
// Public-key signature algorithms we accept for userauth (RFC 8308 server-sig-algs).
static const char *const ALG_SIG = "rsa-sha2-256";
// RFC 8308 indicator a client sets in its kex_algorithms to request EXT_INFO.
static const char *const EXT_INFO_C = "ext-info-c";

// ---------------------------------------------------------------------------
// Byte-writer helpers
// ---------------------------------------------------------------------------

namespace
{
struct Writer
{
    uint8_t *p;
    size_t cap;
    size_t len;
    bool ok;
};

void w_bytes(Writer &w, const void *src, size_t n)
{
    if (!w.ok || w.len + n > w.cap)
    {
        w.ok = false;
        return;
    }
    memcpy(w.p + w.len, src, n);
    w.len += n;
}

void w_u8(Writer &w, uint8_t v)
{
    w_bytes(w, &v, 1);
}

void w_u32(Writer &w, uint32_t v)
{
    uint8_t b[4] = {(uint8_t)(v >> 24), (uint8_t)(v >> 16), (uint8_t)(v >> 8), (uint8_t)v};
    w_bytes(w, b, 4);
}

// Write an SSH name-list: uint32 length + comma-separated names.
void w_namelist(Writer &w, const char *list)
{
    uint32_t n = (uint32_t)strlen(list);
    w_u32(w, n);
    w_bytes(w, list, n);
}

// Write an SSH string: uint32 length + raw bytes.
void w_string(Writer &w, const uint8_t *data, size_t n)
{
    w_u32(w, (uint32_t)n);
    w_bytes(w, data, n);
}

// Write an SSH mpint from a fixed-width big-endian integer: strip leading zero
// bytes, prepend 0x00 if the top bit is set.
void w_mpint(Writer &w, const uint8_t *be, size_t len)
{
    size_t off = 0;
    while (off < len && be[off] == 0)
        off++;
    if (off == len)
    {
        w_u32(w, 0); // zero → empty string
        return;
    }
    bool pad = (be[off] & 0x80u) != 0;
    w_u32(w, (uint32_t)(len - off) + (pad ? 1u : 0u));
    if (pad)
        w_u8(w, 0x00);
    w_bytes(w, be + off, len - off);
}
} // namespace

// ---------------------------------------------------------------------------
// name-list membership test (RFC 4253 §7.1 - comma-separated, no spaces)
// ---------------------------------------------------------------------------

// Returns true if @p want appears as a complete element of the comma-separated
// list [list, list+len).
static bool namelist_contains(const uint8_t *list, uint32_t len, const char *want)
{
    size_t wl = strlen(want);
    uint32_t start = 0;
    for (uint32_t i = 0; i <= len; i++)
    {
        if (i == len || list[i] == ',')
        {
            uint32_t elen = i - start;
            if (elen == wl && memcmp(list + start, want, wl) == 0)
                return true;
            start = i + 1;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Init
// ---------------------------------------------------------------------------

void ssh_transport_init(uint8_t i)
{
    if (i >= MAX_SSH_CONNS)
        return;
    SshSession *s = &ssh_sess[i];
    memset(s, 0, sizeof(*s));
    s->phase = SSH_PHASE_BANNER;
}

// ---------------------------------------------------------------------------
// Identification string exchange (RFC 4253 §4.2)
// ---------------------------------------------------------------------------

int ssh_transport_server_banner(uint8_t *out, size_t *out_len, size_t cap)
{
    size_t vlen = strlen(SSH_SERVER_VERSION);
    if (vlen + 2 > cap)
        return -1;
    memcpy(out, SSH_SERVER_VERSION, vlen);
    out[vlen] = '\r';
    out[vlen + 1] = '\n';
    *out_len = vlen + 2;
    return 0;
}

int ssh_transport_recv_banner(uint8_t i, const uint8_t *data, size_t len, size_t *consumed)
{
    if (i >= MAX_SSH_CONNS)
        return -1;
    SshSession *s = &ssh_sess[i];

    size_t k = 0;
    while (k < len)
    {
        uint8_t c = data[k++];
        if (c == '\n')
        {
            // End of a line. Strip a trailing CR if present.
            uint16_t n = s->banner_len;
            if (n > 0 && s->banner_buf[n - 1] == '\r')
                n--;

            // RFC 4253 §4.2: the server may receive other lines before the
            // identification string; only the line starting with "SSH-" counts.
            if (n >= 4 && memcmp(s->banner_buf, "SSH-", 4) == 0)
            {
                if (n >= SSH_VERSION_MAX)
                    return -1;
                memcpy(s->v_c, s->banner_buf, n);
                s->v_c[n] = '\0';
                s->v_c_len = n;
                s->banner_len = 0;
                s->phase = SSH_PHASE_KEXINIT;
                *consumed = k;
                return 1;
            }
            // Not the SSH line - discard and keep scanning.
            s->banner_len = 0;
            continue;
        }

        if (s->banner_len >= SSH_VERSION_MAX)
            return -1; // line too long
        s->banner_buf[s->banner_len++] = c;
    }

    *consumed = k;
    return 0; // need more data
}

// ---------------------------------------------------------------------------
// KEXINIT (RFC 4253 §7.1)
// ---------------------------------------------------------------------------

int ssh_kexinit_build(uint8_t i, uint8_t *payload, size_t *len, size_t cap)
{
    if (i >= MAX_SSH_CONNS)
        return -1;
    SshSession *s = &ssh_sess[i];

    Writer w = {payload, cap, 0, true};
    w_u8(w, SSH_MSG_KEXINIT);

    uint8_t cookie[16];
    ssh_rng_fill(cookie, sizeof(cookie));
    w_bytes(w, cookie, sizeof(cookie));

    w_namelist(w, ALG_KEX_ADVERTISED); // kex_algorithms (+ ext-info-s)
    w_namelist(w, ALG_HOSTKEY);        // server_host_key_algorithms
    w_namelist(w, ALG_CIPHER);         // encryption c2s
    w_namelist(w, ALG_CIPHER);         // encryption s2c
    w_namelist(w, ALG_MAC);            // mac c2s
    w_namelist(w, ALG_MAC);            // mac s2c
    w_namelist(w, ALG_COMP);           // compression c2s
    w_namelist(w, ALG_COMP);           // compression s2c
    w_namelist(w, "");                 // languages c2s
    w_namelist(w, "");                 // languages s2c
    w_u8(w, 0);                        // first_kex_packet_follows = false
    w_u32(w, 0);                       // reserved

    if (!w.ok)
        return -1;

    // Retain a copy as I_S for the exchange hash.
    if (w.len > SSH_KEXINIT_S_MAX)
        return -1;
    memcpy(s->i_s, payload, w.len);
    s->i_s_len = (uint16_t)w.len;

    *len = w.len;
    return 0;
}

// Read a name-list field at offset *off; set *list/*nlen to point into payload.
// Returns true on success and advances *off past the field.
static bool read_namelist(const uint8_t *p, size_t len, size_t *off, const uint8_t **list, uint32_t *nlen)
{
    if (*off + 4 > len)
        return false;
    uint32_t n = ((uint32_t)p[*off] << 24) | ((uint32_t)p[*off + 1] << 16) | ((uint32_t)p[*off + 2] << 8) |
                 (uint32_t)p[*off + 3];
    *off += 4;
    if (*off + n > len)
        return false;
    *list = p + *off;
    *nlen = n;
    *off += n;
    return true;
}

int ssh_kexinit_parse(uint8_t i, const uint8_t *payload, size_t len)
{
    if (i >= MAX_SSH_CONNS)
        return -1;
    SshSession *s = &ssh_sess[i];

    if (len < 1 + 16 || payload[0] != SSH_MSG_KEXINIT)
        return -1;

    // Retain a copy as I_C for the exchange hash.
    if (len > SSH_KEXINIT_MAX)
        return -1;
    memcpy(s->i_c, payload, len);
    s->i_c_len = (uint16_t)len;

    size_t off = 1 + 16; // skip msg type + 16-byte cookie

    const uint8_t *list;
    uint32_t nlen;

    // kex_algorithms
    if (!read_namelist(payload, len, &off, &list, &nlen) || !namelist_contains(list, nlen, ALG_KEX))
        return -1;
    // RFC 8308: if the client offers ext-info-c we will send SSH_MSG_EXT_INFO.
    s->ext_info_c = namelist_contains(list, nlen, EXT_INFO_C);
    // server_host_key_algorithms
    if (!read_namelist(payload, len, &off, &list, &nlen) || !namelist_contains(list, nlen, ALG_HOSTKEY))
        return -1;
    // encryption c2s
    if (!read_namelist(payload, len, &off, &list, &nlen) || !namelist_contains(list, nlen, ALG_CIPHER))
        return -1;
    // encryption s2c
    if (!read_namelist(payload, len, &off, &list, &nlen) || !namelist_contains(list, nlen, ALG_CIPHER))
        return -1;
    // mac c2s
    if (!read_namelist(payload, len, &off, &list, &nlen) || !namelist_contains(list, nlen, ALG_MAC))
        return -1;
    // mac s2c
    if (!read_namelist(payload, len, &off, &list, &nlen) || !namelist_contains(list, nlen, ALG_MAC))
        return -1;
    // compression c2s
    if (!read_namelist(payload, len, &off, &list, &nlen) || !namelist_contains(list, nlen, ALG_COMP))
        return -1;
    // compression s2c
    if (!read_namelist(payload, len, &off, &list, &nlen) || !namelist_contains(list, nlen, ALG_COMP))
        return -1;

    s->phase = SSH_PHASE_DH_INIT;
    return 0;
}

int ssh_extinfo_build(uint8_t *out, size_t *len, size_t cap)
{
    // byte SSH_MSG_EXT_INFO || uint32 nr-extensions || (string name, string value)*
    Writer w = {out, cap, 0, true};
    w_u8(w, SSH_MSG_EXT_INFO);
    w_u32(w, 1);                      // one extension
    w_namelist(w, "server-sig-algs"); // extension name
    w_namelist(w, ALG_SIG);           // value: accepted client-sig algorithms
    if (!w.ok)
        return -1;
    *len = w.len;
    return 0;
}

// ---------------------------------------------------------------------------
// Exchange hash H (RFC 4253 §8) - streamed into SHA-256, no large buffer.
// ---------------------------------------------------------------------------

// Hash a 4-byte big-endian length prefix.
static void hash_u32(SshSha256Ctx *ctx, uint32_t v)
{
    uint8_t b[4] = {(uint8_t)(v >> 24), (uint8_t)(v >> 16), (uint8_t)(v >> 8), (uint8_t)v};
    ssh_sha256_update(ctx, b, 4);
}

// Hash an SSH string: uint32 length + raw bytes.
static void hash_string(SshSha256Ctx *ctx, const uint8_t *data, size_t len)
{
    hash_u32(ctx, (uint32_t)len);
    ssh_sha256_update(ctx, data, len);
}

// Hash an SSH mpint from a fixed-width big-endian integer: strip leading zero
// bytes, prepend a 0x00 if the top bit is set (to keep it positive).
static void hash_mpint(SshSha256Ctx *ctx, const uint8_t *be, size_t len)
{
    size_t off = 0;
    while (off < len && be[off] == 0)
        off++;
    if (off == len)
    {
        // Value is zero → mpint is an empty string.
        hash_u32(ctx, 0);
        return;
    }
    bool pad = (be[off] & 0x80u) != 0;
    uint32_t mlen = (uint32_t)(len - off) + (pad ? 1u : 0u);
    hash_u32(ctx, mlen);
    if (pad)
    {
        uint8_t zero = 0;
        ssh_sha256_update(ctx, &zero, 1);
    }
    ssh_sha256_update(ctx, be + off, len - off);
}

int ssh_kex_exchange_hash(uint8_t i, const uint8_t *e_be, const uint8_t *f_be, const uint8_t *k_be, const uint8_t *ks,
                          size_t ks_len, uint8_t out[SSH_SHA256_DIGEST_LEN])
{
    if (i >= MAX_SSH_CONNS)
        return -1;
    SshSession *s = &ssh_sess[i];

    static const char *const v_s = SSH_SERVER_VERSION;

    SshSha256Ctx ctx;
    ssh_sha256_init(&ctx);
    hash_string(&ctx, (const uint8_t *)s->v_c, s->v_c_len); // V_C
    hash_string(&ctx, (const uint8_t *)v_s, strlen(v_s));   // V_S
    hash_string(&ctx, s->i_c, s->i_c_len);                  // I_C
    hash_string(&ctx, s->i_s, s->i_s_len);                  // I_S
    hash_string(&ctx, ks, ks_len);                          // K_S
    hash_mpint(&ctx, e_be, 256);                            // e
    hash_mpint(&ctx, f_be, 256);                            // f
    hash_mpint(&ctx, k_be, 256);                            // K
    ssh_sha256_final(&ctx, out);
    return 0;
}

// ---------------------------------------------------------------------------
// KEXDH (RFC 4253 §8)
// ---------------------------------------------------------------------------

int ssh_kexdh_parse_init(const uint8_t *payload, size_t len, uint8_t e_be[256])
{
    if (len < 1 + 4 || payload[0] != SSH_MSG_KEXDH_INIT)
        return -1;

    uint32_t n = ((uint32_t)payload[1] << 24) | ((uint32_t)payload[2] << 16) | ((uint32_t)payload[3] << 8) |
                 (uint32_t)payload[4];
    if ((size_t)5 + n > len)
        return -1;

    const uint8_t *m = payload + 5;
    size_t off = 0;
    while (off < n && m[off] == 0) // strip sign/leading-zero bytes
        off++;
    size_t vlen = n - off;
    if (vlen > 256)
        return -1; // e exceeds 2048 bits

    memset(e_be, 0, 256);
    memcpy(e_be + (256 - vlen), m + off, vlen);
    return 0;
}

int ssh_kexdh_build_reply(const uint8_t *ks, size_t ks_len, const uint8_t *f_be, const uint8_t *sig, size_t sig_len,
                          uint8_t *out, size_t *out_len, size_t cap)
{
    Writer w = {out, cap, 0, true};
    w_u8(w, SSH_MSG_KEXDH_REPLY);
    w_string(w, ks, ks_len); // K_S
    w_mpint(w, f_be, 256);   // f

    // signature = string( string("rsa-sha2-256") || string(sig) )
    uint32_t inner = 4 + 12 + 4 + (uint32_t)sig_len;
    w_u32(w, inner);
    w_string(w, (const uint8_t *)"rsa-sha2-256", 12);
    w_string(w, sig, sig_len);

    if (!w.ok)
        return -1;
    *out_len = w.len;
    return 0;
}

int ssh_kexdh_handle(uint8_t i, const uint8_t *payload, size_t len, uint8_t *reply_out, size_t *reply_len, size_t cap)
{
    if (i >= MAX_SSH_CONNS)
        return -1;
    SshSession *s = &ssh_sess[i];

    // 1. Parse and validate the client public value e (1 < e < p-1).
    uint8_t e_be[256];
    if (ssh_kexdh_parse_init(payload, len, e_be) != 0)
        return -1;
    SshBigNum e;
    bn_from_bytes(&e, e_be, 256);
    if (bn_dh_validate(&e) != 0)
        return -1;

    // 2. Shared secret K = e^y mod p.
    SshBigNum K;
    bn_expmod_group14(&K, &e, &ssh_dh[i].y);
    uint8_t k_be[256];
    bn_to_bytes(k_be, &K);

    // 3. Server public value f (already computed by ssh_dh_generate).
    uint8_t f_be[256];
    bn_to_bytes(f_be, &ssh_dh[i].f);

    // 4. Host-key blob K_S.
    uint8_t ks[SSH_RSA_PUBKEY_BLOB_MAX];
    size_t ks_len = 0;
    if (ssh_rsa_encode_pubkey(ks, &ks_len, sizeof(ks)) != 0)
    {
        ssh_wipe(k_be, sizeof(k_be));
        ssh_wipe(&K, sizeof(K));
        return -1;
    }

    // 5. Exchange hash H, and capture the session id on the first KEX.
    uint8_t H[SSH_SHA256_DIGEST_LEN];
    ssh_kex_exchange_hash(i, e_be, f_be, k_be, ks, ks_len, H);
    if (!s->have_session_id)
    {
        memcpy(s->session_id, H, SSH_SHA256_DIGEST_LEN);
        s->have_session_id = true;
    }

    // 6. Sign H with the RSA host key.
    uint8_t sig[SSH_RSA_SIG_BYTES];
    if (ssh_rsa_sign(H, SSH_SHA256_DIGEST_LEN, sig) != 0)
    {
        ssh_wipe(k_be, sizeof(k_be));
        ssh_wipe(&K, sizeof(K));
        return -1;
    }

    // 7. Assemble KEXDH_REPLY.
    if (ssh_kexdh_build_reply(ks, ks_len, f_be, sig, sizeof(sig), reply_out, reply_len, cap) != 0)
    {
        ssh_wipe(k_be, sizeof(k_be));
        ssh_wipe(&K, sizeof(K));
        return -1;
    }

    // 8. Derive the six session keys (installed; encryption activates at NEWKEYS).
    //    The session id stays fixed at the first KEX's H across re-keys.
    ssh_dh_derive_keys_sid(i, k_be, H, s->session_id);

    // 9. Wipe the shared secret from the stack.
    ssh_wipe(k_be, sizeof(k_be));
    ssh_wipe(&K, sizeof(K));

    s->phase = SSH_PHASE_NEWKEYS;
    return 0;
}

void ssh_newkeys_complete(uint8_t i)
{
    if (i >= MAX_SSH_CONNS)
        return;
    ssh_pkt[i].encrypted = true;
    ssh_pkt[i].kex_active = false;
    // On the first KEX advance to the service phase; on a re-key the connection
    // is already authenticated, so resume the open (channel) phase.
    ssh_sess[i].phase = ssh_sess[i].authed ? SSH_PHASE_OPEN : SSH_PHASE_SERVICE;
}

bool ssh_rekey_needed(uint8_t i)
{
    if (i >= MAX_SSH_CONNS)
        return false;
    return ssh_pkt[i].seq_no_send >= SSH_REKEY_PACKET_THRESHOLD || ssh_pkt[i].seq_no_recv >= SSH_REKEY_PACKET_THRESHOLD;
}

int ssh_transport_begin_rekey(uint8_t i, uint8_t *out, size_t *out_len, size_t cap)
{
    if (i >= MAX_SSH_CONNS)
        return -1;
    // Fresh server KEXINIT (re-stores I_S for the new exchange hash).
    if (ssh_kexinit_build(i, out, out_len, cap) != 0)
        return -1;
    // New ephemeral DH key pair for forward secrecy across the re-key.
    if (ssh_dh_generate(i) != 0)
        return -1;
    ssh_sess[i].phase = SSH_PHASE_KEXINIT;
    return 0;
}
