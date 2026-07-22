// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_auth.cpp
 * @brief SSH user-authentication layer (RFC 4252) - password method.
 */

#include "network_drivers/presentation/ssh/auth/ssh_auth.h"
#include "network_drivers/presentation/ssh/crypto/ssh_ecdsa.h"        // ssh_ecdsa_p256_verify() (ecdsa-sha2-nistp256)
#include "network_drivers/presentation/ssh/crypto/ssh_ed25519.h"      // ssh_ed25519_verify() (ssh-ed25519 client keys)
#include "network_drivers/presentation/ssh/crypto/ssh_rsa.h"          // ssh_rsa_verify(), SSH_RSA_KEY_BYTES
#include "network_drivers/presentation/ssh/transport/ssh_packet.h"    // SSH_MSG_* constants
#include "network_drivers/presentation/ssh/transport/ssh_transport.h" // ssh_sess[], SshPhase
#include <string.h>

// ---------------------------------------------------------------------------
// Application password callback
// ---------------------------------------------------------------------------

// All SSH auth callbacks, owned by one instance (internal linkage): the application password
// and public-key verifiers. One named owner, unreachable from any other translation unit.
struct SshAuthCtx
{
    SshPasswordCb pw_cb = nullptr;
    SshPubkeyCb pk_cb = nullptr;
#if DWS_ENABLE_SSH_KEYBOARD_INTERACTIVE
    // Per-slot keyboard-interactive exchange state: armed by a "keyboard-interactive" USERAUTH_REQUEST
    // (we send one INFO_REQUEST), consumed by the matching INFO_RESPONSE. The user is remembered across
    // the round-trip since the INFO_RESPONSE does not carry it.
    struct
    {
        bool pending;
        char user[SSH_AUTH_USER_MAX];
    } ki[MAX_SSH_CONNS];
#endif
};
static SshAuthCtx s_auth;

void dws_ssh_auth_set_password_cb(SshPasswordCb cb)
{
    s_auth.pw_cb = cb;
}

void dws_ssh_auth_set_pubkey_cb(SshPubkeyCb cb)
{
    s_auth.pk_cb = cb;
}

// ---------------------------------------------------------------------------
// Wire helpers
// ---------------------------------------------------------------------------

// Read an SSH string into a fixed buffer, null-terminating it. Advances *off.
// Returns false on truncation or if the string does not fit (buffer too small).
static bool read_string(const uint8_t *p, size_t len, size_t *off, char *out, size_t outcap)
{
    if (*off + 4 > len)
        return false;
    uint32_t n = ((uint32_t)p[*off] << 24) | ((uint32_t)p[*off + 1] << 16) | ((uint32_t)p[*off + 2] << 8) |
                 (uint32_t)p[*off + 3];
    *off += 4;
    if (*off + n > len)
        return false;
    if (n >= outcap)
        return false; // does not fit our fixed buffer
    memcpy(out, p + *off, n);
    out[n] = '\0';
    *off += n;
    return true;
}

static void put_u32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}

// Read an SSH string by reference (no copy): *out points into p. Advances *off.
static bool read_string_ref(const uint8_t *p, size_t len, size_t *off, const uint8_t **out, uint32_t *slen)
{
    if (*off + 4 > len)
        return false;
    uint32_t n = ((uint32_t)p[*off] << 24) | ((uint32_t)p[*off + 1] << 16) | ((uint32_t)p[*off + 2] << 8) |
                 (uint32_t)p[*off + 3];
    *off += 4;
    if (*off + n > len)
        return false;
    *out = p + *off;
    *slen = n;
    *off += n;
    return true;
}

// Normalize an mpint (from a blob) into a fixed right-aligned big-endian buffer.
static bool mpint_to_fixed(const uint8_t *m, uint32_t mlen, uint8_t *out, size_t outlen)
{
    uint32_t off = 0;
    while (off < mlen && m[off] == 0) // strip sign/leading-zero bytes
        off++;
    uint32_t vlen = mlen - off;
    if (vlen > outlen)
        return false;
    memset(out, 0, outlen);
    memcpy(out + (outlen - vlen), m + off, vlen);
    return true;
}

// Parse an "ssh-rsa" public-key blob: string("ssh-rsa") mpint(e) mpint(n).
static bool parse_ssh_rsa_blob(const uint8_t *blob, uint32_t blen, uint8_t n_be[SSH_RSA_KEY_BYTES], uint8_t e_be[4])
{
    size_t off = 0;
    const uint8_t *type;
    uint32_t type_len;
    if (!read_string_ref(blob, blen, &off, &type, &type_len))
        return false;
    if (type_len != 7 || memcmp(type, "ssh-rsa", 7) != 0)
        return false;

    const uint8_t *e_mp;
    uint32_t e_len;
    if (!read_string_ref(blob, blen, &off, &e_mp, &e_len))
        return false;
    if (!mpint_to_fixed(e_mp, e_len, e_be, 4))
        return false;

    const uint8_t *n_mp;
    uint32_t n_len;
    if (!read_string_ref(blob, blen, &off, &n_mp, &n_len))
        return false;
    if (!mpint_to_fixed(n_mp, n_len, n_be, SSH_RSA_KEY_BYTES))
        return false;

    return true;
}

// Parse an "ssh-ed25519" public-key blob: string("ssh-ed25519") string(pub32). (RFC 8709 §4)
static bool parse_ssh_ed25519_blob(const uint8_t *blob, uint32_t blen, uint8_t pub[32])
{
    size_t off = 0;
    const uint8_t *type;
    uint32_t type_len;
    // The caller only reaches here after matching the 15-byte string("ssh-ed25519") prefix on the blob,
    // so the type field is already proven present and correct.
    if (!read_string_ref(blob, blen, &off, &type, &type_len))   // GCOVR_EXCL_LINE  prefix match implies blen >= 15
        return false;                                           // GCOVR_EXCL_LINE
    if (type_len != 11 || memcmp(type, "ssh-ed25519", 11) != 0) // GCOVR_EXCL_LINE  prefix match implies this type
        return false;                                           // GCOVR_EXCL_LINE
    const uint8_t *pk;
    uint32_t pk_len;
    if (!read_string_ref(blob, blen, &off, &pk, &pk_len))
        return false;
    if (pk_len != 32)
        return false;
    memcpy(pub, pk, 32);
    return true;
}

// Parse an "ecdsa-sha2-nistp256" public-key blob (RFC 5656 §3.1):
//   string("ecdsa-sha2-nistp256") string("nistp256") string(Q = 0x04||X||Y, 65 bytes).
static bool parse_ssh_ecdsa_blob(const uint8_t *blob, uint32_t blen, uint8_t pub[SSH_ECDSA_P256_PUB_LEN])
{
    size_t off = 0;
    const uint8_t *type;
    uint32_t type_len;
    // As above: the caller matched the 23-byte string("ecdsa-sha2-nistp256") prefix before calling in.
    if (!read_string_ref(blob, blen, &off, &type, &type_len)) // GCOVR_EXCL_LINE  prefix match implies blen >= 23
        return false;                                         // GCOVR_EXCL_LINE
    if (type_len != 19 || memcmp(type, "ecdsa-sha2-nistp256", 19) != 0) // GCOVR_EXCL_LINE  prefix implies this type
        return false;                                                   // GCOVR_EXCL_LINE
    const uint8_t *curve;
    uint32_t curve_len;
    if (!read_string_ref(blob, blen, &off, &curve, &curve_len))
        return false;
    if (curve_len != 8 || memcmp(curve, "nistp256", 8) != 0)
        return false;
    const uint8_t *q;
    uint32_t q_len;
    if (!read_string_ref(blob, blen, &off, &q, &q_len))
        return false;
    if (q_len != SSH_ECDSA_P256_PUB_LEN || q[0] != 0x04) // uncompressed point only
        return false;
    memcpy(pub, q, SSH_ECDSA_P256_PUB_LEN);
    return true;
}

// Parse an ECDSA signature blob (RFC 5656 §3.1.2): mpint(r) || mpint(s) -> raw r || s (32 + 32).
static bool parse_ecdsa_sig(const uint8_t *sig, uint32_t slen, uint8_t out[SSH_ECDSA_P256_SIG_LEN])
{
    size_t off = 0;
    const uint8_t *r;
    const uint8_t *s;
    uint32_t r_len;
    uint32_t s_len;
    if (!read_string_ref(sig, slen, &off, &r, &r_len) || !read_string_ref(sig, slen, &off, &s, &s_len))
        return false;
    return mpint_to_fixed(r, r_len, out, SSH_ECDSA_P256_COORD_LEN) &&
           mpint_to_fixed(s, s_len, out + SSH_ECDSA_P256_COORD_LEN, SSH_ECDSA_P256_COORD_LEN);
}

// ---------------------------------------------------------------------------
// Service request (RFC 4253 §10)
// ---------------------------------------------------------------------------

int dws_ssh_auth_handle_service_request(const uint8_t *payload, size_t len, uint8_t *out, size_t *out_len, size_t cap)
{
    if (len < 1 || payload[0] != SSH_MSG_SERVICE_REQUEST)
        return -1;

    size_t off = 1;
    char svc[32];
    if (!read_string(payload, len, &off, svc, sizeof(svc)))
        return -1;
    if (strcmp(svc, "ssh-userauth") != 0)
        return -1;

    // SERVICE_ACCEPT: byte(6) || string("ssh-userauth")
    static const char name[] = "ssh-userauth";
    uint32_t nl = (uint32_t)(sizeof(name) - 1);
    if (cap < 1 + 4 + nl)
        return -1;
    out[0] = SSH_MSG_SERVICE_ACCEPT;
    put_u32(out + 1, nl);
    memcpy(out + 5, name, nl);
    *out_len = 5 + nl;
    return 0;
}

// ---------------------------------------------------------------------------
// USERAUTH_REQUEST parse (RFC 4252 §5)
// ---------------------------------------------------------------------------

int dws_ssh_auth_parse_request(const uint8_t *payload, size_t len, SshAuthReq *req)
{
    memset(req, 0, sizeof(*req));
    if (len < 1 || payload[0] != SSH_MSG_USERAUTH_REQUEST)
        return -1;

    size_t off = 1;
    if (!read_string(payload, len, &off, req->user, sizeof(req->user)))
        return -1;
    if (!read_string(payload, len, &off, req->service, sizeof(req->service)))
        return -1;
    if (!read_string(payload, len, &off, req->method, sizeof(req->method)))
        return -1;

    if (strcmp(req->method, "password") == 0)
    {
        // boolean (FALSE = not a password change) || string password
        if (off >= len)
            return -1;
        off++; // skip the change-password boolean
        if (!read_string(payload, len, &off, req->password, sizeof(req->password)))
            return -1;
        req->is_password = true;
    }
    else if (strcmp(req->method, "publickey") == 0)
    {
        // boolean has_signature || string algo || string pubkey-blob [|| string signature]
        if (off >= len)
            return -1;
        req->has_signature = payload[off++] != 0;
        if (!read_string(payload, len, &off, req->pk_algo, sizeof(req->pk_algo)))
            return -1;
        if (!read_string_ref(payload, len, &off, &req->pk_blob, &req->pk_blob_len))
            return -1;

        // Everything parsed so far is exactly the data the signature covers.
        req->signed_prefix = payload;
        req->signed_prefix_len = off;

        if (req->has_signature)
        {
            const uint8_t *sigblob;
            uint32_t sigblob_len;
            if (!read_string_ref(payload, len, &off, &sigblob, &sigblob_len))
                return -1;
            // signature blob = string(sig-algo) || string(raw-signature)
            size_t so = 0;
            const uint8_t *salgo;
            uint32_t salgo_len;
            if (!read_string_ref(sigblob, sigblob_len, &so, &salgo, &salgo_len))
                return -1;
            if (!read_string_ref(sigblob, sigblob_len, &so, &req->signature, &req->signature_len))
                return -1;
        }
        req->is_pubkey = true;
    }
#if DWS_ENABLE_SSH_KEYBOARD_INTERACTIVE
    else if (strcmp(req->method, "keyboard-interactive") == 0)
    {
        // RFC 4256 §3.1: string(language tag, deprecated) || string(submethods). Both are ignored -
        // this server always drives a single "Password:" prompt.
        req->is_kbdint = true;
    }
#endif
    return 0;
}

// ---------------------------------------------------------------------------
// Response builders
// ---------------------------------------------------------------------------

int dws_ssh_auth_build_failure(uint8_t *out, size_t *out_len, size_t cap, bool partial)
{
    // SSH_MSG_USERAUTH_FAILURE || name-list(authentications) || boolean(partial)
#if DWS_SSH_ALLOW_PASSWORD
#if DWS_ENABLE_SSH_KEYBOARD_INTERACTIVE
    static const char methods[] = "publickey,password,keyboard-interactive";
#else
    static const char methods[] = "publickey,password";
#endif
#else
    static const char methods[] = "publickey"; // password auth disabled for hardening
#endif
    uint32_t ml = (uint32_t)(sizeof(methods) - 1);
    if (cap < 1 + 4 + ml + 1)
        return -1;
    out[0] = SSH_MSG_USERAUTH_FAILURE;
    put_u32(out + 1, ml);
    memcpy(out + 5, methods, ml);
    out[5 + ml] = partial ? 1 : 0;
    *out_len = 5 + ml + 1;
    return 0;
}

int dws_ssh_auth_build_success(uint8_t *out, size_t *out_len, size_t cap)
{
    if (cap < 1)
        return -1;
    out[0] = SSH_MSG_USERAUTH_SUCCESS;
    *out_len = 1;
    return 0;
}

// SSH_MSG_USERAUTH_PK_OK || string(algo) || string(blob) - the "this key would
// be accepted, send a signature" probe response (RFC 4252 §7).
static int build_pk_ok(const SshAuthReq *req, uint8_t *out, size_t *out_len, size_t cap)
{
    uint32_t al = (uint32_t)strnlen(req->pk_algo, sizeof(req->pk_algo));
    if (cap < (size_t)1 + 4 + al + 4 + req->pk_blob_len)
        return -1;
    size_t o = 0;
    out[o++] = SSH_MSG_USERAUTH_PK_OK;
    put_u32(out + o, al);
    o += 4;
    memcpy(out + o, req->pk_algo, al);
    o += al;
    put_u32(out + o, req->pk_blob_len);
    o += 4;
    memcpy(out + o, req->pk_blob, req->pk_blob_len);
    o += req->pk_blob_len;
    *out_len = o;
    return 0;
}

#if DWS_ENABLE_SSH_KEYBOARD_INTERACTIVE
// SSH_MSG_USERAUTH_INFO_REQUEST (RFC 4256 §3.2): empty name/instruction/language, one non-echoed
// "Password: " prompt. This is the challenge-response face of password auth (a single prompt).
static int build_info_request(uint8_t *out, size_t *out_len, size_t cap)
{
    static const char prompt[] = "Password: ";
    const uint32_t pl = (uint32_t)(sizeof(prompt) - 1);
    const size_t need = 1 + 4 + 4 + 4 + 4 + 4 + pl + 1; // msg,name,instr,lang,num-prompts,prompt,echo
    if (cap < need)
        return -1;
    size_t o = 0;
    out[o++] = SSH_MSG_USERAUTH_INFO_REQUEST;
    put_u32(out + o, 0); // name = ""
    o += 4;
    put_u32(out + o, 0); // instruction = ""
    o += 4;
    put_u32(out + o, 0); // language tag = "" (deprecated)
    o += 4;
    put_u32(out + o, 1); // num-prompts = 1
    o += 4;
    put_u32(out + o, pl);
    o += 4;
    memcpy(out + o, prompt, pl);
    o += pl;
    out[o++] = 0; // echo = FALSE (the response is a password)
    *out_len = o;
    return 0;
}
#endif

// ---------------------------------------------------------------------------
// Orchestration
// ---------------------------------------------------------------------------

// publickey method (RFC 4252 §7): validate the offered key (a signature-less probe -> PK_OK) or verify
// the signature over string(session_id) || signed_prefix, keying success to connection i.
static int dws_ssh_auth_handle_pubkey(uint8_t i, const SshAuthReq *req, uint8_t *out, size_t *out_len, size_t cap)
{
    // Key type is taken from the blob (the algo name only steers the RSA signature hash).
    bool is_ed = req->pk_blob_len >= 4 + 11 && memcmp(req->pk_blob,
                                                      "\x00\x00\x00\x0b"
                                                      "ssh-ed25519",
                                                      4 + 11) == 0;
    bool is_ecdsa = req->pk_blob_len >= 4 + 19 && memcmp(req->pk_blob,
                                                         "\x00\x00\x00\x13"
                                                         "ecdsa-sha2-nistp256",
                                                         4 + 19) == 0;
    uint8_t n_be[SSH_RSA_KEY_BYTES];
    uint8_t e_be[4];
    uint8_t ed_pub[32];
    uint8_t ec_pub[SSH_ECDSA_P256_PUB_LEN];
    bool parsed = false;
    if (is_ed)
        parsed = parse_ssh_ed25519_blob(req->pk_blob, req->pk_blob_len, ed_pub);
    else if (is_ecdsa)
        parsed = parse_ssh_ecdsa_blob(req->pk_blob, req->pk_blob_len, ec_pub);
    else
        parsed = parse_ssh_rsa_blob(req->pk_blob, req->pk_blob_len, n_be, e_be);
    bool key_ok = parsed && s_auth.pk_cb && s_auth.pk_cb(req->user, req->pk_blob, req->pk_blob_len);
    if (!key_ok)
        return dws_ssh_auth_build_failure(out, out_len, cap, false);

    if (!req->has_signature)
        return build_pk_ok(req, out, out_len, cap); // probe: ask for a signature

    // Verify the signature over string(session_id) || signed_prefix. The session_id is the first KEX's
    // exchange hash: 32 bytes (SHA-256 methods) or 64 (sntrup761x25519-sha512).
    const size_t sid_len = ssh_sess[i].session_id_len;
    uint8_t signed_data[SSH_PKT_BUF_SIZE + 4 + SSH_KEXHASH_MAX_LEN];
    size_t sd = 0;
    put_u32(signed_data + sd, (uint32_t)sid_len);
    sd += 4;
    memcpy(signed_data + sd, ssh_sess[i].session_id, sid_len);
    sd += sid_len;
    if (req->signed_prefix_len > SSH_PKT_BUF_SIZE)
        return dws_ssh_auth_build_failure(out, out_len, cap, false);
    memcpy(signed_data + sd, req->signed_prefix, req->signed_prefix_len);
    sd += req->signed_prefix_len;

    // For RSA the signature hash is chosen by the client's algorithm name (RFC 8332),
    // not the key blob: rsa-sha2-512 -> SHA-512, otherwise SHA-256.
    const SshRsaHash rh = (strcmp(req->pk_algo, SSH_RSA_SIG_ALG_SHA512) == 0) ? SshRsaHash::SHA512 : SshRsaHash::SHA256;
    bool sig_ok;
    if (is_ed)
    {
        sig_ok = req->signature_len == 64 && ssh_ed25519_verify(ed_pub, signed_data, sd, req->signature);
    }
    else if (is_ecdsa)
    {
        uint8_t ec_sig[SSH_ECDSA_P256_SIG_LEN];
        sig_ok = parse_ecdsa_sig(req->signature, req->signature_len, ec_sig) &&
                 ssh_ecdsa_p256_verify(ec_pub, signed_data, sd, ec_sig);
    }
    else
    {
        sig_ok = ssh_rsa_verify(n_be, e_be, signed_data, sd, req->signature, req->signature_len, rh) == 0;
    }
    if (sig_ok)
    {
        ssh_sess[i].authed = true;
        ssh_sess[i].phase = SshPhase::SSH_PHASE_OPEN;
        return dws_ssh_auth_build_success(out, out_len, cap);
    }
    return dws_ssh_auth_build_failure(out, out_len, cap, false);
}

int dws_ssh_auth_handle_request(uint8_t i, const uint8_t *payload, size_t len, uint8_t *out, size_t *out_len,
                                size_t cap)
{
    if (i >= MAX_SSH_CONNS)
        return -1;

    SshAuthReq req;
    if (dws_ssh_auth_parse_request(payload, len, &req) != 0)
        return -1;

    // ---- publickey method (RFC 4252 §7) ----
    if (req.is_pubkey)
        return dws_ssh_auth_handle_pubkey(i, &req, out, out_len, cap);

#if DWS_ENABLE_SSH_KEYBOARD_INTERACTIVE
    // ---- keyboard-interactive method (RFC 4256): arm the exchange and send one "Password:" prompt.
    if (req.is_kbdint)
    {
        if (!s_auth.pw_cb) // no verifier installed -> cannot challenge
            return dws_ssh_auth_build_failure(out, out_len, cap, false);
        s_auth.ki[i].pending = true;
        size_t ul = strnlen(req.user, sizeof(s_auth.ki[i].user) - 1);
        memcpy(s_auth.ki[i].user, req.user, ul);
        s_auth.ki[i].user[ul] = '\0';
        return build_info_request(out, out_len, cap);
    }
#endif

    // ---- password method (RFC 4252 §8) ----
    // Password auth can be compiled out for publickey-only hardening.
#if DWS_SSH_ALLOW_PASSWORD
    bool ok = req.is_password && s_auth.pw_cb && s_auth.pw_cb(req.user, req.password);
#else
    bool ok = false;
#endif

    // Wipe the password from the stack regardless of the outcome.
    volatile char *p = req.password;
    for (size_t k = 0; k < sizeof(req.password); k++)
        p[k] = 0;

    if (ok)
    {
        ssh_sess[i].authed = true;
        ssh_sess[i].phase = SshPhase::SSH_PHASE_OPEN;
        return dws_ssh_auth_build_success(out, out_len, cap);
    }
    return dws_ssh_auth_build_failure(out, out_len, cap, false);
}

#if DWS_ENABLE_SSH_KEYBOARD_INTERACTIVE
int dws_ssh_auth_handle_info_response(uint8_t i, const uint8_t *payload, size_t len, uint8_t *out, size_t *out_len,
                                      size_t cap)
{
    if (i >= MAX_SSH_CONNS)
        return -1;
    if (!s_auth.ki[i].pending) // no keyboard-interactive exchange armed for this slot
        return -1;
    s_auth.ki[i].pending = false; // consume the exchange regardless of outcome

    // SSH_MSG_USERAUTH_INFO_RESPONSE (RFC 4256 §3.4): byte(61) || uint32 num-responses || string[num].
    // We sent one prompt, so exactly one response is expected.
    if (len < 1 || payload[0] != SSH_MSG_USERAUTH_INFO_RESPONSE)
        return -1;
    size_t off = 1;
    if (off + 4 > len)
        return -1;
    uint32_t nr = ((uint32_t)payload[off] << 24) | ((uint32_t)payload[off + 1] << 16) |
                  ((uint32_t)payload[off + 2] << 8) | (uint32_t)payload[off + 3];
    off += 4;

    char resp[SSH_AUTH_PASS_MAX];
    bool ok = false;
    if (nr == 1 && read_string(payload, len, &off, resp, sizeof(resp)))
        ok = s_auth.pw_cb && s_auth.pw_cb(s_auth.ki[i].user, resp);

    // Wipe the response and the remembered user from memory regardless of outcome.
    volatile char *rp = resp;
    for (size_t k = 0; k < sizeof(resp); k++)
        rp[k] = 0;
    volatile char *up = s_auth.ki[i].user;
    for (size_t k = 0; k < sizeof(s_auth.ki[i].user); k++)
        up[k] = 0;

    if (ok)
    {
        ssh_sess[i].authed = true;
        ssh_sess[i].phase = SshPhase::SSH_PHASE_OPEN;
        return dws_ssh_auth_build_success(out, out_len, cap);
    }
    return dws_ssh_auth_build_failure(out, out_len, cap, false);
}
#endif
