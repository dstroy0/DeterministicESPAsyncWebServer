// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_transport.cpp
 * @brief SSH transport handshake - banner exchange and KEXINIT negotiation.
 */

#include "ssh_transport.h"
#include "ssh_bignum.h"     // bn_*, SshBigNum
#include "ssh_curve25519.h" // ssh_x25519 (curve25519-sha256 KEX)
#include "ssh_dh.h"         // ssh_rng_fill(), ssh_dh[], ssh_dh_generate/derive_keys
#include "ssh_ed25519.h"    // ssh_ed25519 host-key sign
#include "ssh_packet.h"     // SSH_MSG_KEXINIT, ssh_pkt[]
#include "ssh_rsa.h"        // ssh_rsa_encode_pubkey/sign, ssh_host_pubkey, SSH_RSA_*
#include "ssh_sha256.h"
#include <stdio.h> // snprintf (name-list assembly)
#include <string.h>

SshSession ssh_sess[MAX_SSH_CONNS];

// ---------------------------------------------------------------------------
// Negotiable algorithms. The server supports two KEX methods and two host-key
// types; cipher / MAC / compression are fixed. Negotiation is crypto-agnostic and
// steers toward whichever suite ssh_kex_set_prefer_rsa() selects (default: RSA/DH,
// hardware-accelerated on ESP32), while still advertising both suites so a client
// that supports only one still connects.
// ---------------------------------------------------------------------------

static const char *const KEX_DH = "diffie-hellman-group14-sha256";
static const char *const KEX_C25519 = "curve25519-sha256";
static const char *const KEX_C25519_LIBSSH = "curve25519-sha256@libssh.org"; // identical wire protocol
static const char *const HOSTKEY_RSA = "rsa-sha2-256";
static const char *const HOSTKEY_ED = "ssh-ed25519";
static const char *const ALG_CIPHER = "aes256-ctr";
static const char *const ALG_MAC = "hmac-sha2-256";
static const char *const ALG_COMP = "none";
// RFC 8308 indicator a client sets in its kex_algorithms to request EXT_INFO.
static const char *const EXT_INFO_C = "ext-info-c";

// Runtime negotiation preference (true = prefer the hardware-accelerated RSA/DH suite).
static bool g_prefer_rsa = true;

void ssh_kex_set_prefer_rsa(bool prefer)
{
    g_prefer_rsa = prefer;
}
bool ssh_kex_prefer_rsa(void)
{
    return g_prefer_rsa;
}

// --- ssh-ed25519 host key (optional; the RSA host key is loaded via ssh_rsa) --
static uint8_t g_ed_seed[32];
static uint8_t g_ed_pub[32];
static bool g_ed_have = false;

void ssh_hostkey_ed25519_set(const uint8_t seed[32])
{
    memcpy(g_ed_seed, seed, 32);
    ssh_ed25519_pubkey(g_ed_pub, g_ed_seed);
    g_ed_have = true;
}
bool ssh_hostkey_ed25519_available(void)
{
    return g_ed_have;
}
static bool hostkey_rsa_available(void)
{
    return ssh_host_pubkey.loaded;
}

// Build the KEX / host-key advertise lists in preference order (RFC 4253 §7.1 name-
// lists), filtering host-key types to the keys we actually hold. server-sig-algs
// (RFC 8308) uses the same host-key ordering. Written into a caller buffer.
static void build_kex_list(char *out, size_t cap)
{
    const char *c1 = KEX_C25519, *c2 = KEX_C25519_LIBSSH, *dh = KEX_DH;
    if (g_prefer_rsa)
        snprintf(out, cap, "%s,%s,%s,ext-info-s", dh, c1, c2);
    else
        snprintf(out, cap, "%s,%s,%s,ext-info-s", c1, c2, dh);
}
static void build_hostkey_list(char *out, size_t cap)
{
    const char *first = g_prefer_rsa ? HOSTKEY_RSA : HOSTKEY_ED;
    const char *second = g_prefer_rsa ? HOSTKEY_ED : HOSTKEY_RSA;
    bool first_ok = g_prefer_rsa ? hostkey_rsa_available() : ssh_hostkey_ed25519_available();
    bool second_ok = g_prefer_rsa ? ssh_hostkey_ed25519_available() : hostkey_rsa_available();
    out[0] = '\0';
    if (first_ok)
        snprintf(out, cap, "%s", first);
    if (second_ok)
    {
        size_t l = strlen(out);
        snprintf(out + l, cap - l, "%s%s", l ? "," : "", second);
    }
}

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

// One negotiation candidate: an algorithm name, the tag we store if it is chosen, and
// whether we can actually perform it (e.g. we hold the matching host key).
struct AlgCand
{
    const char *name;
    int tag;
    bool avail;
};

// Steer-to-preferred negotiation: return the tag of the FIRST candidate (in OUR
// preference order) that the client also offers and that we can perform, or -1.
static int negotiate_alg(const uint8_t *client_list, uint32_t nlen, const AlgCand *cands, int n)
{
    for (int i = 0; i < n; i++)
        if (cands[i].avail && namelist_contains(client_list, nlen, cands[i].name))
            return cands[i].tag;
    return -1;
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

    char kexlist[160], hklist[48];
    build_kex_list(kexlist, sizeof(kexlist));
    build_hostkey_list(hklist, sizeof(hklist));
    w_namelist(w, kexlist);    // kex_algorithms (preference-ordered, + ext-info-s)
    w_namelist(w, hklist);     // server_host_key_algorithms (only keys we hold)
    w_namelist(w, ALG_CIPHER); // encryption c2s
    w_namelist(w, ALG_CIPHER); // encryption s2c
    w_namelist(w, ALG_MAC);    // mac c2s
    w_namelist(w, ALG_MAC);    // mac s2c
    w_namelist(w, ALG_COMP);   // compression c2s
    w_namelist(w, ALG_COMP);   // compression s2c
    w_namelist(w, "");         // languages c2s
    w_namelist(w, "");         // languages s2c
    w_u8(w, 0);                // first_kex_packet_follows = false
    w_u32(w, 0);               // reserved

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

    // kex_algorithms: negotiate the KEX method in our preference order.
    if (!read_namelist(payload, len, &off, &list, &nlen))
        return -1;
    // RFC 8308: if the client offers ext-info-c we will send SSH_MSG_EXT_INFO.
    s->ext_info_c = namelist_contains(list, nlen, EXT_INFO_C);
    {
        AlgCand kc[3];
        if (g_prefer_rsa)
        {
            kc[0] = {KEX_DH, SSH_KEX_DH_GROUP14, true};
            kc[1] = {KEX_C25519, SSH_KEX_CURVE25519, true};
            kc[2] = {KEX_C25519_LIBSSH, SSH_KEX_CURVE25519, true};
        }
        else
        {
            kc[0] = {KEX_C25519, SSH_KEX_CURVE25519, true};
            kc[1] = {KEX_C25519_LIBSSH, SSH_KEX_CURVE25519, true};
            kc[2] = {KEX_DH, SSH_KEX_DH_GROUP14, true};
        }
        int k = negotiate_alg(list, nlen, kc, 3);
        if (k < 0)
            return -1; // no mutual KEX
        s->kex_alg = (uint8_t)k;
    }
    // server_host_key_algorithms: negotiate, restricted to keys we actually hold.
    if (!read_namelist(payload, len, &off, &list, &nlen))
        return -1;
    {
        AlgCand hc[2];
        if (g_prefer_rsa)
        {
            hc[0] = {HOSTKEY_RSA, SSH_HOSTKEY_RSA, hostkey_rsa_available()};
            hc[1] = {HOSTKEY_ED, SSH_HOSTKEY_ED25519, ssh_hostkey_ed25519_available()};
        }
        else
        {
            hc[0] = {HOSTKEY_ED, SSH_HOSTKEY_ED25519, ssh_hostkey_ed25519_available()};
            hc[1] = {HOSTKEY_RSA, SSH_HOSTKEY_RSA, hostkey_rsa_available()};
        }
        int h = negotiate_alg(list, nlen, hc, 2);
        if (h < 0)
            return -1; // no mutual host-key algorithm
        s->hostkey_alg = (uint8_t)h;
    }
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
    // Accepted client public-key signature algorithms for userauth. Both are always
    // verifiable (independent of which host key we hold); ordered by our preference so a
    // modern client picks the steered-to type. A client uses this to choose a key to offer.
    const char *siglist = g_prefer_rsa ? "rsa-sha2-256,ssh-ed25519" : "ssh-ed25519,rsa-sha2-256";
    w_namelist(w, siglist); // value: accepted client-sig algorithms
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

// Method-neutral exchange hash. The client/server public values are hashed as SSH
// strings for an ECDH KEX (Q_C, Q_S; RFC 8731) or as mpints for a finite-field DH KEX
// (e, f; RFC 4253 §8). K is always an mpint. cpub/spub are big-endian, right-aligned in
// their buffers, so hash_mpint / hash_string produce the canonical minimal encoding.
static int compute_exchange_hash(uint8_t i, bool pub_is_string, const uint8_t *cpub, size_t cpub_len,
                                 const uint8_t *spub, size_t spub_len, const uint8_t *k_be, size_t k_len,
                                 const uint8_t *ks, size_t ks_len, uint8_t out[SSH_SHA256_DIGEST_LEN])
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
    if (pub_is_string)
    {
        hash_string(&ctx, cpub, cpub_len); // Q_C
        hash_string(&ctx, spub, spub_len); // Q_S
    }
    else
    {
        hash_mpint(&ctx, cpub, cpub_len); // e
        hash_mpint(&ctx, spub, spub_len); // f
    }
    hash_mpint(&ctx, k_be, k_len); // K
    ssh_sha256_final(&ctx, out);
    return 0;
}

int ssh_kex_exchange_hash(uint8_t i, const uint8_t *e_be, const uint8_t *f_be, const uint8_t *k_be, const uint8_t *ks,
                          size_t ks_len, uint8_t out[SSH_SHA256_DIGEST_LEN])
{
    return compute_exchange_hash(i, false, e_be, 256, f_be, 256, k_be, 256, ks, ks_len, out);
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

// Parse SSH_MSG_KEX_ECDH_INIT (msg 30, shares the number with KEXDH_INIT):
// byte(30) || string(Q_C). Q_C must be exactly 32 bytes for X25519 (RFC 8731).
static int parse_ecdh_init(const uint8_t *payload, size_t len, uint8_t qc[32])
{
    if (len < 1 + 4 || payload[0] != SSH_MSG_KEXDH_INIT)
        return -1;
    uint32_t n = ((uint32_t)payload[1] << 24) | ((uint32_t)payload[2] << 16) | ((uint32_t)payload[3] << 8) |
                 (uint32_t)payload[4];
    if (n != 32 || (size_t)5 + n > len)
        return -1;
    memcpy(qc, payload + 5, 32);
    return 0;
}

// Encode the server host-key blob K_S for the negotiated host-key algorithm.
//   rsa-sha2-256 → "ssh-rsa" blob (ssh_rsa_encode_pubkey)
//   ssh-ed25519  → string("ssh-ed25519") || string(pub32)   (RFC 8709 §4)
static int encode_hostkey(uint8_t i, uint8_t *ks, size_t *ks_len, size_t cap)
{
    if (ssh_sess[i].hostkey_alg == SSH_HOSTKEY_ED25519)
    {
        Writer w = {ks, cap, 0, true};
        w_string(w, (const uint8_t *)HOSTKEY_ED, strlen(HOSTKEY_ED));
        w_string(w, g_ed_pub, 32);
        if (!w.ok)
            return -1;
        *ks_len = w.len;
        return 0;
    }
    return ssh_rsa_encode_pubkey(ks, ks_len, cap);
}

// Sign the exchange hash H with the negotiated host key. Writes the raw signature (no
// SSH framing) plus its algorithm name; the caller wraps it as the signature blob.
static int sign_hash(uint8_t i, const uint8_t H[SSH_SHA256_DIGEST_LEN], uint8_t *sig, size_t *sig_len, size_t sig_cap,
                     const char **sig_name)
{
    if (ssh_sess[i].hostkey_alg == SSH_HOSTKEY_ED25519)
    {
        if (sig_cap < 64)
            return -1;
        ssh_ed25519_sign(sig, H, SSH_SHA256_DIGEST_LEN, g_ed_seed);
        *sig_len = 64;
        *sig_name = HOSTKEY_ED; // "ssh-ed25519"
        return 0;
    }
    if (sig_cap < SSH_RSA_SIG_BYTES || ssh_rsa_sign(H, SSH_SHA256_DIGEST_LEN, sig) != 0)
        return -1;
    *sig_len = SSH_RSA_SIG_BYTES;
    *sig_name = HOSTKEY_RSA; // "rsa-sha2-256"
    return 0;
}

// Assemble SSH_MSG_KEXDH_REPLY (== KEX_ECDH_REPLY, msg 31):
//   byte(31) || string(K_S) || (mpint f | string Q_S) || string( string(sig_name) || string(sig) )
static int build_kex_reply(uint8_t i, const uint8_t *ks, size_t ks_len, const uint8_t *spub, size_t spub_len,
                           const char *sig_name, const uint8_t *sig, size_t sig_len, uint8_t *out, size_t *out_len,
                           size_t cap)
{
    Writer w = {out, cap, 0, true};
    w_u8(w, SSH_MSG_KEXDH_REPLY);
    w_string(w, ks, ks_len); // K_S
    if (ssh_sess[i].kex_alg == SSH_KEX_CURVE25519)
        w_string(w, spub, spub_len); // Q_S (raw string)
    else
        w_mpint(w, spub, spub_len); // f (mpint)
    uint32_t nl = (uint32_t)strlen(sig_name);
    w_u32(w, 4 + nl + 4 + (uint32_t)sig_len); // signature blob length
    w_string(w, (const uint8_t *)sig_name, nl);
    w_string(w, sig, sig_len);
    if (!w.ok)
        return -1;
    *out_len = w.len;
    return 0;
}

int ssh_kex_generate(uint8_t i)
{
    if (i >= MAX_SSH_CONNS)
        return -1;
    if (ssh_sess[i].kex_alg == SSH_KEX_CURVE25519)
    {
        // X25519 ephemeral: random 32-byte scalar, public = X25519(scalar, base point).
        ssh_rng_fill(ssh_sess[i].ecdh_sk, 32);
        ssh_x25519_base(ssh_sess[i].ecdh_pk, ssh_sess[i].ecdh_sk);
        return 0;
    }
    return ssh_dh_generate(i);
}

int ssh_kexdh_handle(uint8_t i, const uint8_t *payload, size_t len, uint8_t *reply_out, size_t *reply_len, size_t cap)
{
    if (i >= MAX_SSH_CONNS)
        return -1;
    SshSession *s = &ssh_sess[i];

    // 1. Shared secret K + the two public values, per negotiated KEX method.
    //    k_be holds K big-endian, right-aligned so hash_mpint / the KDF strip to minimal.
    uint8_t k_be[256];
    memset(k_be, 0, sizeof(k_be));
    uint8_t cpub[256], spub[256]; // client / server public value (right-aligned)
    size_t cpub_len = 256, spub_len = 256;
    bool pub_is_string = false;

    if (s->kex_alg == SSH_KEX_CURVE25519)
    {
        // curve25519-sha256 (RFC 8731): K = X25519(sk, Q_C); Q_C/Q_S hashed as strings.
        uint8_t qc[32], kk[32];
        if (parse_ecdh_init(payload, len, qc) != 0)
            return -1;
        ssh_x25519(kk, s->ecdh_sk, qc);
        // Reject a low-order client point (all-zero shared secret) - RFC 7748 §6.1.
        uint8_t zacc = 0;
        for (int b = 0; b < 32; b++)
            zacc |= kk[b];
        if (zacc == 0)
        {
            ssh_wipe(kk, sizeof(kk));
            return -1;
        }
        memcpy(k_be + (256 - 32), kk, 32);
        memcpy(cpub, qc, 32);
        memcpy(spub, s->ecdh_pk, 32);
        cpub_len = spub_len = 32;
        pub_is_string = true;
        ssh_wipe(kk, sizeof(kk));
    }
    else
    {
        // diffie-hellman-group14-sha256 (RFC 4253 §8): K = e^y mod p; e/f are mpints.
        uint8_t e_be[256];
        if (ssh_kexdh_parse_init(payload, len, e_be) != 0)
            return -1;
        SshBigNum e;
        bn_from_bytes(&e, e_be, 256);
        if (bn_dh_validate(&e) != 0)
            return -1;
        SshBigNum K;
        bn_expmod_group14(&K, &e, &ssh_dh[i].y);
        bn_to_bytes(k_be, &K);
        ssh_wipe(&K, sizeof(K));
        memcpy(cpub, e_be, 256);
        bn_to_bytes(spub, &ssh_dh[i].f);
    }

    // 2. Host-key blob K_S (per negotiated host-key algorithm).
    uint8_t ks[SSH_RSA_PUBKEY_BLOB_MAX];
    size_t ks_len = 0;
    if (encode_hostkey(i, ks, &ks_len, sizeof(ks)) != 0)
    {
        ssh_wipe(k_be, sizeof(k_be));
        return -1;
    }

    // 3. Exchange hash H; capture the session id on the first KEX.
    uint8_t H[SSH_SHA256_DIGEST_LEN];
    compute_exchange_hash(i, pub_is_string, cpub, cpub_len, spub, spub_len, k_be, 256, ks, ks_len, H);
    if (!s->have_session_id)
    {
        memcpy(s->session_id, H, SSH_SHA256_DIGEST_LEN);
        s->have_session_id = true;
    }

    // 4. Sign H with the negotiated host key (rsa-sha2-256 or ssh-ed25519).
    uint8_t sig[SSH_RSA_SIG_BYTES]; // 256 bytes: fits an RSA-2048 sig and a 64-byte ed25519 sig
    size_t sig_len = 0;
    const char *sig_name = 0;
    if (sign_hash(i, H, sig, &sig_len, sizeof(sig), &sig_name) != 0)
    {
        ssh_wipe(k_be, sizeof(k_be));
        return -1;
    }

    // 5. Assemble the reply, then derive the six session keys (id fixed at first KEX's H).
    if (build_kex_reply(i, ks, ks_len, spub, spub_len, sig_name, sig, sig_len, reply_out, reply_len, cap) != 0)
    {
        ssh_wipe(k_be, sizeof(k_be));
        return -1;
    }
    ssh_dh_derive_keys_sid(i, k_be, H, s->session_id);
    ssh_wipe(k_be, sizeof(k_be));

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
    // New ephemeral for forward secrecy across the re-key (re-generated for the finally
    // negotiated method once the peer's KEXINIT arrives; see the KEXINIT dispatch).
    if (ssh_kex_generate(i) != 0)
        return -1;
    ssh_sess[i].phase = SSH_PHASE_KEXINIT;
    return 0;
}
