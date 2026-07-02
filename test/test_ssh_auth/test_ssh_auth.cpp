// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// SSH user-authentication tests (RFC 4252): service request/accept, request
// parsing, and the password method.

#include "network_drivers/presentation/ssh/ssh_auth.h"
#include "network_drivers/presentation/ssh/ssh_packet.h"
#include "network_drivers/presentation/ssh/ssh_transport.h"
#include <stdint.h>
#include <string.h>
#include <unity.h>

void setUp()
{
    ssh_transport_init(0);
    ssh_auth_set_password_cb(nullptr);
}
void tearDown()
{
}

// ---- wire builders --------------------------------------------------------

static size_t put_string(uint8_t *p, const char *s)
{
    uint32_t n = (uint32_t)strlen(s);
    p[0] = (uint8_t)(n >> 24);
    p[1] = (uint8_t)(n >> 16);
    p[2] = (uint8_t)(n >> 8);
    p[3] = (uint8_t)n;
    memcpy(p + 4, s, n);
    return 4 + n;
}

static bool check_uv(const char *u, const char *p)
{
    return strcmp(u, "alice") == 0 && strcmp(p, "s3cret") == 0;
}

// ---- service request ------------------------------------------------------

void test_service_request_accept()
{
    uint8_t pkt[64];
    size_t n = 0;
    pkt[n++] = SSH_MSG_SERVICE_REQUEST;
    n += put_string(pkt + n, "ssh-userauth");

    uint8_t out[64];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_auth_handle_service_request(pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_SERVICE_ACCEPT, out[0]);
    uint32_t sl = ((uint32_t)out[1] << 24) | ((uint32_t)out[2] << 16) | ((uint32_t)out[3] << 8) | out[4];
    TEST_ASSERT_EQUAL_UINT32(12, sl);
    TEST_ASSERT_EQUAL_MEMORY("ssh-userauth", out + 5, 12);
}

void test_service_request_rejects_unknown()
{
    uint8_t pkt[64];
    size_t n = 0;
    pkt[n++] = SSH_MSG_SERVICE_REQUEST;
    n += put_string(pkt + n, "ssh-connection");
    uint8_t out[64];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_handle_service_request(pkt, n, out, &olen, sizeof(out)));
}

// ---- request parsing ------------------------------------------------------

void test_parse_password_request()
{
    uint8_t pkt[128];
    size_t n = 0;
    pkt[n++] = SSH_MSG_USERAUTH_REQUEST;
    n += put_string(pkt + n, "alice");
    n += put_string(pkt + n, "ssh-connection");
    n += put_string(pkt + n, "password");
    pkt[n++] = 0; // change-password = FALSE
    n += put_string(pkt + n, "s3cret");

    SshAuthReq req;
    TEST_ASSERT_EQUAL_INT(0, ssh_auth_parse_request(pkt, n, &req));
    TEST_ASSERT_TRUE(req.is_password);
    TEST_ASSERT_EQUAL_STRING("alice", req.user);
    TEST_ASSERT_EQUAL_STRING("ssh-connection", req.service);
    TEST_ASSERT_EQUAL_STRING("password", req.method);
    TEST_ASSERT_EQUAL_STRING("s3cret", req.password);
}

void test_parse_none_request()
{
    uint8_t pkt[128];
    size_t n = 0;
    pkt[n++] = SSH_MSG_USERAUTH_REQUEST;
    n += put_string(pkt + n, "bob");
    n += put_string(pkt + n, "ssh-connection");
    n += put_string(pkt + n, "none");

    SshAuthReq req;
    TEST_ASSERT_EQUAL_INT(0, ssh_auth_parse_request(pkt, n, &req));
    TEST_ASSERT_FALSE(req.is_password);
    TEST_ASSERT_EQUAL_STRING("bob", req.user);
}

// ---- orchestration --------------------------------------------------------

static size_t build_pw_request(uint8_t *pkt, const char *u, const char *p)
{
    size_t n = 0;
    pkt[n++] = SSH_MSG_USERAUTH_REQUEST;
    n += put_string(pkt + n, u);
    n += put_string(pkt + n, "ssh-connection");
    n += put_string(pkt + n, "password");
    pkt[n++] = 0;
    n += put_string(pkt + n, p);
    return n;
}

void test_handle_request_success()
{
    ssh_auth_set_password_cb(check_uv);
    uint8_t pkt[128];
    size_t n = build_pw_request(pkt, "alice", "s3cret");

    uint8_t out[64];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_USERAUTH_SUCCESS, out[0]);
    TEST_ASSERT_TRUE(ssh_sess[0].authed);
    TEST_ASSERT_EQUAL(SSH_PHASE_OPEN, ssh_sess[0].phase);
}

void test_handle_request_wrong_password_fails()
{
    ssh_auth_set_password_cb(check_uv);
    uint8_t pkt[128];
    size_t n = build_pw_request(pkt, "alice", "wrong");

    uint8_t out[64];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_USERAUTH_FAILURE, out[0]);
    TEST_ASSERT_FALSE(ssh_sess[0].authed);
    // Failure advertises the "password" method.
    bool adv = false;
    for (size_t k = 0; k + 8 <= olen; k++)
        if (memcmp(out + k, "password", 8) == 0)
        {
            adv = true;
            break;
        }
    TEST_ASSERT_TRUE(adv);
}

void test_handle_none_request_fails_without_auth()
{
    ssh_auth_set_password_cb(check_uv);
    uint8_t pkt[64];
    size_t n = 0;
    pkt[n++] = SSH_MSG_USERAUTH_REQUEST;
    n += put_string(pkt + n, "alice");
    n += put_string(pkt + n, "ssh-connection");
    n += put_string(pkt + n, "none");

    uint8_t out[64];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_USERAUTH_FAILURE, out[0]);
    TEST_ASSERT_FALSE(ssh_sess[0].authed);
}

void test_handle_request_no_callback_fails()
{
    // No callback installed → all credentials rejected.
    uint8_t pkt[128];
    size_t n = build_pw_request(pkt, "alice", "s3cret");
    uint8_t out[64];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_USERAUTH_FAILURE, out[0]);
}

// ---- publickey method (RFC 4252 §7) ---------------------------------------

// Real "ssh-rsa" blob and a genuine signature over the exact publickey
// signed-data (session_id = bytes 0..31, user "alice", service
// "ssh-connection"), produced with openssl against a real RSA-2048 key.
static const char *PK_BLOB_HEX =
    "000000077373682d727361000000030100010000010100932e817a74435fdd1d7c7c5e8ec9b0f2a8b06b6b3db9b03e907"
    "d010d2c003985deaeb56d1ac5734116772da333131f8eb0b5bec0eff6f4dfd612068d2857acefa7dc75e84b0f2ffca57e"
    "82297f4f085b8584caebaa5a4be51c6f5887529f84fbee5f6940a31307d1224714705ca5cf47ca9e04d2a9faafe7b022b"
    "e41f426a1868f61410141a0a8b39110107f59d9c5a5920e8e2921ccaf85672ba0f860644f793f7c38425018c17e915a5f"
    "18ba5cfcf002a6a8fc50eeb08bc2f1e81f1df69704aacfce062bcd9ef678197e9778d411f3f364959637d09d56c6adc59"
    "8147a6f924d9075a3df5dd2c606a9e963afd49fe56b4633e5e905871d0b31093954ec57ff";
static const char *PK_SIG_HEX =
    "25044aeea46af01d38e0b09b626f43be788a30a156ff8b16bf5f6a637ef47f0066f6dda53fd2fe60ea8f4fe82f94988d3"
    "dfc0da02aea3d578a884069904124385e7bd16297135b86ee4d44071493edc1b9f18528f0c03c16069ece8a552da8ccab"
    "f72a846d684c43d97fc55003553f8978ded240b63b2b50bc37f1f7aa3e9148018a5e3a3a1823288b8ae003dc45498ab8f"
    "ced85bf14899c47f590b22dfdf78758be99d281f790d86c9d84bc629e26ddfb6b95cb0cb5b8b1ef6daec18480091eb364"
    "f594edcde4f02cd9a1e8a08c6267ff7221e279159d59a0502d261499ed7b9b96c51da7db03b421d316e14f994ecba90d3"
    "7f0e1841747182863d0433f964a";

static size_t hexdec(const char *h, uint8_t *out)
{
    size_t n = 0;
    for (; h[0] && h[1]; h += 2)
    {
        auto nib = [](char c) -> int { return c >= 'a' ? c - 'a' + 10 : c - '0'; };
        out[n++] = (uint8_t)((nib(h[0]) << 4) | nib(h[1]));
    }
    return n;
}

static void wr_u32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}

static bool pk_cb_alice(const char *u, const uint8_t *blob, size_t n)
{
    (void)blob;
    (void)n;
    return strcmp(u, "alice") == 0;
}

static size_t build_pubkey_req(uint8_t *pkt, const uint8_t *blob, size_t blob_len, const uint8_t *sig, size_t sig_len,
                               bool with_sig)
{
    size_t n = 0;
    pkt[n++] = SSH_MSG_USERAUTH_REQUEST;
    n += put_string(pkt + n, "alice");
    n += put_string(pkt + n, "ssh-connection");
    n += put_string(pkt + n, "publickey");
    pkt[n++] = with_sig ? 1 : 0;
    n += put_string(pkt + n, "rsa-sha2-256");
    wr_u32(pkt + n, (uint32_t)blob_len);
    n += 4;
    memcpy(pkt + n, blob, blob_len);
    n += blob_len;
    if (with_sig)
    {
        uint32_t inner = 4 + 12 + 4 + (uint32_t)sig_len;
        wr_u32(pkt + n, inner);
        n += 4;
        n += put_string(pkt + n, "rsa-sha2-256");
        wr_u32(pkt + n, (uint32_t)sig_len);
        n += 4;
        memcpy(pkt + n, sig, sig_len);
        n += sig_len;
    }
    return n;
}

static void set_session_id_0_to_31()
{
    for (int j = 0; j < 32; j++)
        ssh_sess[0].session_id[j] = (uint8_t)j;
    ssh_sess[0].have_session_id = true;
}

void test_pubkey_probe_returns_pk_ok()
{
    ssh_auth_set_pubkey_cb(pk_cb_alice);
    set_session_id_0_to_31();
    uint8_t blob[512];
    size_t blob_len = hexdec(PK_BLOB_HEX, blob);

    uint8_t pkt[1024];
    size_t n = build_pubkey_req(pkt, blob, blob_len, nullptr, 0, false);
    uint8_t out[1024];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_USERAUTH_PK_OK, out[0]);
    TEST_ASSERT_FALSE(ssh_sess[0].authed);
}

void test_pubkey_valid_signature_succeeds()
{
    ssh_auth_set_pubkey_cb(pk_cb_alice);
    set_session_id_0_to_31();
    uint8_t blob[512], sig[256];
    size_t blob_len = hexdec(PK_BLOB_HEX, blob);
    size_t sig_len = hexdec(PK_SIG_HEX, sig);

    uint8_t pkt[1024];
    size_t n = build_pubkey_req(pkt, blob, blob_len, sig, sig_len, true);
    uint8_t out[64];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_USERAUTH_SUCCESS, out[0]);
    TEST_ASSERT_TRUE(ssh_sess[0].authed);
    TEST_ASSERT_EQUAL(SSH_PHASE_OPEN, ssh_sess[0].phase);
}

void test_pubkey_tampered_signature_fails()
{
    ssh_auth_set_pubkey_cb(pk_cb_alice);
    set_session_id_0_to_31();
    uint8_t blob[512], sig[256];
    size_t blob_len = hexdec(PK_BLOB_HEX, blob);
    size_t sig_len = hexdec(PK_SIG_HEX, sig);
    sig[100] ^= 0x01;

    uint8_t pkt[1024];
    size_t n = build_pubkey_req(pkt, blob, blob_len, sig, sig_len, true);
    uint8_t out[64];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_USERAUTH_FAILURE, out[0]);
    TEST_ASSERT_FALSE(ssh_sess[0].authed);
}

void test_pubkey_unauthorized_key_fails()
{
    ssh_auth_set_pubkey_cb(nullptr); // no key authorized
    set_session_id_0_to_31();
    uint8_t blob[512], sig[256];
    size_t blob_len = hexdec(PK_BLOB_HEX, blob);
    size_t sig_len = hexdec(PK_SIG_HEX, sig);

    uint8_t pkt[1024];
    size_t n = build_pubkey_req(pkt, blob, blob_len, sig, sig_len, true);
    uint8_t out[64];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_USERAUTH_FAILURE, out[0]);
}

// SERVICE_REQUEST parsing rejects a wrong/empty message, a truncated service string,
// and an output buffer too small for SERVICE_ACCEPT.
void test_service_request_errors()
{
    uint8_t out[64], p[64];
    size_t olen = 0;
    p[0] = 99;
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_handle_service_request(p, 1, out, &olen, sizeof(out))); // wrong type
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_handle_service_request(p, 0, out, &olen, sizeof(out))); // len 0
    p[0] = SSH_MSG_SERVICE_REQUEST;
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_handle_service_request(p, 1, out, &olen, sizeof(out))); // truncated service
    size_t n = 0;
    p[n++] = SSH_MSG_SERVICE_REQUEST;
    n += put_string(p + n, "ssh-userauth");
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_handle_service_request(p, n, out, &olen, 3)); // cap too small
}

// The response builders reject an output buffer too small for their fixed messages.
void test_build_response_guards()
{
    uint8_t out[8];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_build_failure(out, &olen, 2, false)); // < 1+4+methods+1
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_build_success(out, &olen, 0));        // cap < 1

    // build_pk_ok via a pubkey probe with a tiny output buffer.
    ssh_auth_set_pubkey_cb(pk_cb_alice);
    set_session_id_0_to_31();
    uint8_t blob[512];
    size_t blob_len = hexdec(PK_BLOB_HEX, blob);
    uint8_t pkt[1024];
    size_t pn = build_pubkey_req(pkt, blob, blob_len, nullptr, 0, false);
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_handle_request(0, pkt, pn, out, &olen, 4));
}

// USERAUTH_REQUEST parsing rejects a truncation at every field, for both the password
// and publickey methods (including the signature blob).
void test_parse_request_truncations()
{
    SshAuthReq req;
    uint8_t p[512];
    size_t n;

    p[0] = 99;
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_parse_request(p, 1, &req)); // wrong type
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_parse_request(p, 0, &req)); // len 0

    p[0] = SSH_MSG_USERAUTH_REQUEST;
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_parse_request(p, 1, &req)); // user length header truncated

    n = 0;
    p[n++] = SSH_MSG_USERAUTH_REQUEST;
    wr_u32(p + n, 20);
    n += 4; // user claims 20 bytes, none present
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_parse_request(p, n, &req));

    n = 0;
    p[n++] = SSH_MSG_USERAUTH_REQUEST;
    char big[300];
    memset(big, 'u', sizeof(big) - 1);
    big[sizeof(big) - 1] = '\0';
    n += put_string(p + n, big); // user longer than the fixed buffer
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_parse_request(p, n, &req));

    n = 0;
    p[n++] = SSH_MSG_USERAUTH_REQUEST;
    n += put_string(p + n, "alice");
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_parse_request(p, n, &req)); // service missing

    n = 0;
    p[n++] = SSH_MSG_USERAUTH_REQUEST;
    n += put_string(p + n, "alice");
    n += put_string(p + n, "ssh-connection");
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_parse_request(p, n, &req)); // method missing

    // password: boolean missing, then password string missing.
    n = 0;
    p[n++] = SSH_MSG_USERAUTH_REQUEST;
    n += put_string(p + n, "alice");
    n += put_string(p + n, "ssh-connection");
    n += put_string(p + n, "password");
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_parse_request(p, n, &req)); // no boolean
    p[n++] = 0;
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_parse_request(p, n, &req)); // password missing

    // publickey: has-signature byte missing, then algo, then blob (header and body).
    n = 0;
    p[n++] = SSH_MSG_USERAUTH_REQUEST;
    n += put_string(p + n, "alice");
    n += put_string(p + n, "ssh-connection");
    n += put_string(p + n, "publickey");
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_parse_request(p, n, &req)); // no has-sig byte
    p[n++] = 0;
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_parse_request(p, n, &req)); // algo missing
    n += put_string(p + n, "rsa-sha2-256");
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_parse_request(p, n, &req)); // blob header missing
    wr_u32(p + n, 100);
    n += 4; // blob claims 100 bytes, none present
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_parse_request(p, n, &req));

    // publickey with signature: outer sig blob missing, then inner algo, then raw sig.
    n = 0;
    p[n++] = SSH_MSG_USERAUTH_REQUEST;
    n += put_string(p + n, "alice");
    n += put_string(p + n, "ssh-connection");
    n += put_string(p + n, "publickey");
    p[n++] = 1; // has signature
    n += put_string(p + n, "rsa-sha2-256");
    n += put_string(p + n, "blob");
    size_t base = n;
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_parse_request(p, n, &req)); // outer sig blob missing

    n = base;
    wr_u32(p + n, 2);
    n += 4;
    p[n++] = 0;
    p[n++] = 0; // sig blob len 2: cannot hold the inner sig-algo string
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_parse_request(p, n, &req));

    n = base;
    uint8_t sb[32];
    size_t sblen = put_string(sb, "rsa-sha2-256"); // sig blob = algo only, no raw signature
    wr_u32(p + n, (uint32_t)sblen);
    n += 4;
    memcpy(p + n, sb, sblen);
    n += sblen;
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_parse_request(p, n, &req));
}

// A malformed ssh-rsa public-key blob fails to load at each field, yielding a
// USERAUTH_FAILURE rather than a crash.
void test_pubkey_blob_parse_failures()
{
    ssh_auth_set_pubkey_cb(pk_cb_alice);
    set_session_id_0_to_31();
    uint8_t out[512], pkt[1024], b[400];
    size_t olen = 0, bl, pn;

    // empty blob: type string cannot be read.
    pn = build_pubkey_req(pkt, b, 0, nullptr, 0, false);
    TEST_ASSERT_EQUAL_INT(0, ssh_auth_handle_request(0, pkt, pn, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_USERAUTH_FAILURE, out[0]);

    // wrong key type.
    bl = put_string(b, "ssh-dss");
    pn = build_pubkey_req(pkt, b, bl, nullptr, 0, false);
    ssh_auth_handle_request(0, pkt, pn, out, &olen, sizeof(out));
    TEST_ASSERT_EQUAL(SSH_MSG_USERAUTH_FAILURE, out[0]);

    // ssh-rsa with no exponent.
    bl = put_string(b, "ssh-rsa");
    pn = build_pubkey_req(pkt, b, bl, nullptr, 0, false);
    ssh_auth_handle_request(0, pkt, pn, out, &olen, sizeof(out));
    TEST_ASSERT_EQUAL(SSH_MSG_USERAUTH_FAILURE, out[0]);

    // exponent wider than 4 bytes.
    bl = put_string(b, "ssh-rsa");
    wr_u32(b + bl, 5);
    bl += 4;
    memcpy(b + bl, "\x01\x02\x03\x04\x05", 5);
    bl += 5;
    pn = build_pubkey_req(pkt, b, bl, nullptr, 0, false);
    ssh_auth_handle_request(0, pkt, pn, out, &olen, sizeof(out));
    TEST_ASSERT_EQUAL(SSH_MSG_USERAUTH_FAILURE, out[0]);

    // valid exponent but no modulus.
    bl = put_string(b, "ssh-rsa");
    wr_u32(b + bl, 1);
    bl += 4;
    b[bl++] = 3;
    pn = build_pubkey_req(pkt, b, bl, nullptr, 0, false);
    ssh_auth_handle_request(0, pkt, pn, out, &olen, sizeof(out));
    TEST_ASSERT_EQUAL(SSH_MSG_USERAUTH_FAILURE, out[0]);

    // modulus wider than the key buffer.
    bl = put_string(b, "ssh-rsa");
    wr_u32(b + bl, 1);
    bl += 4;
    b[bl++] = 3;
    uint32_t nlen = 300; // > SSH_RSA_KEY_BYTES (256)
    wr_u32(b + bl, nlen);
    bl += 4;
    memset(b + bl, 0x11, nlen);
    bl += nlen;
    pn = build_pubkey_req(pkt, b, bl, nullptr, 0, false);
    ssh_auth_handle_request(0, pkt, pn, out, &olen, sizeof(out));
    TEST_ASSERT_EQUAL(SSH_MSG_USERAUTH_FAILURE, out[0]);
}

static bool pk_cb_any(const char *u, const uint8_t *blob, size_t n)
{
    (void)u;
    (void)blob;
    (void)n;
    return true;
}

// A publickey request whose signed prefix exceeds SSH_PKT_BUF_SIZE is rejected before
// the signature buffer is assembled (a DoS guard), yielding USERAUTH_FAILURE.
void test_pubkey_oversized_signed_prefix()
{
    ssh_auth_set_pubkey_cb(pk_cb_any); // accept any key so the length guard is reached
    set_session_id_0_to_31();
    static uint8_t blob[2200];
    size_t base = hexdec(PK_BLOB_HEX, blob);     // a valid ssh-rsa blob prefix
    memset(blob + base, 0, sizeof(blob) - base); // trailing padding (parse ignores it)
    static uint8_t pkt[4096], out[4096];
    uint8_t sig[8] = {0};
    size_t n = build_pubkey_req(pkt, blob, sizeof(blob), sig, sizeof(sig), true);
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_USERAUTH_FAILURE, out[0]);
}

// handle_request rejects an out-of-range slot and a payload that does not parse.
void test_handle_request_index_and_parse_guards()
{
    uint8_t out[64], p[8];
    size_t olen = 0;
    p[0] = 99;
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_handle_request(MAX_SSH_CONNS, p, 1, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL_INT(-1, ssh_auth_handle_request(0, p, 1, out, &olen, sizeof(out))); // parse fails
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_service_request_errors);
    RUN_TEST(test_build_response_guards);
    RUN_TEST(test_parse_request_truncations);
    RUN_TEST(test_pubkey_blob_parse_failures);
    RUN_TEST(test_pubkey_oversized_signed_prefix);
    RUN_TEST(test_handle_request_index_and_parse_guards);
    RUN_TEST(test_service_request_accept);
    RUN_TEST(test_service_request_rejects_unknown);
    RUN_TEST(test_parse_password_request);
    RUN_TEST(test_parse_none_request);
    RUN_TEST(test_handle_request_success);
    RUN_TEST(test_handle_request_wrong_password_fails);
    RUN_TEST(test_handle_none_request_fails_without_auth);
    RUN_TEST(test_handle_request_no_callback_fails);
    RUN_TEST(test_pubkey_probe_returns_pk_ok);
    RUN_TEST(test_pubkey_valid_signature_succeeds);
    RUN_TEST(test_pubkey_tampered_signature_fails);
    RUN_TEST(test_pubkey_unauthorized_key_fails);
    return UNITY_END();
}
