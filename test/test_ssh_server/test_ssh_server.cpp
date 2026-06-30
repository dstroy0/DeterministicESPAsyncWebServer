// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// End-to-end SSH server dispatcher test: drives a full handshake
// (KEXINIT → KEXDH → NEWKEYS → userauth → channel) through ssh_server_dispatch
// and checks the emitted messages and resulting state at each step.

#include "network_drivers/presentation/ssh/ssh_auth.h"
#include "network_drivers/presentation/ssh/ssh_channel.h"
#include "network_drivers/presentation/ssh/ssh_dh.h"
#include "network_drivers/presentation/ssh/ssh_packet.h"
#include "network_drivers/presentation/ssh/ssh_rsa.h"
#include "network_drivers/presentation/ssh/ssh_server.h"
#include "network_drivers/presentation/ssh/ssh_transport.h"
#include <stdint.h>
#include <string.h>
#include <unity.h>

extern uint8_t _test_rsa_n[256];
extern uint8_t _test_rsa_d[256];
extern uint8_t _test_rsa_e[4];

// ---- emit recorder --------------------------------------------------------

static uint8_t emt_type[32];
static int emt_n;
static uint8_t emt_last[64]; // bytes of the most recent emit (for payload checks)
static size_t emt_last_len;
static void rec_emit(uint8_t slot, const uint8_t *p, size_t n)
{
    (void)slot;
    if (emt_n < 32 && n > 0)
        emt_type[emt_n++] = p[0];
    emt_last_len = n < sizeof(emt_last) ? n : sizeof(emt_last);
    memcpy(emt_last, p, emt_last_len);
}
static void emt_reset()
{
    emt_n = 0;
}

// ---- channel data recorder ------------------------------------------------

static int chan_data_count;
static uint8_t chan_data[64];
static size_t chan_data_len;
static void on_chan_data(uint8_t slot, uint32_t channel, const uint8_t *d, size_t n)
{
    (void)slot;
    (void)channel;
    chan_data_count++;
    chan_data_len = n < sizeof(chan_data) ? n : sizeof(chan_data);
    memcpy(chan_data, d, chan_data_len);
}

static bool pw_cb(const char *u, const char *p)
{
    return strcmp(u, "alice") == 0 && strcmp(p, "s3cret") == 0;
}

void setUp()
{
    ssh_transport_init(0);
    ssh_channel_init(0);
    emt_reset();
    chan_data_count = 0;
    chan_data_len = 0;

    memset(_test_rsa_n, 0, 256);
    _test_rsa_n[0] = 0xFF;
    _test_rsa_n[255] = 0x01;
    memset(_test_rsa_d, 0, 256);
    _test_rsa_d[255] = 0x01;
    _test_rsa_e[0] = 0;
    _test_rsa_e[1] = 1;
    _test_rsa_e[2] = 0;
    _test_rsa_e[3] = 1;
    ssh_rsa_load_pubkey();

    ssh_auth_set_password_cb(pw_cb);
    ssh_channel_set_data_cb(on_chan_data);
    ssh_server_set_emit_cb(rec_emit);
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
static void wr_u32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}
static size_t put_mpint(uint8_t *p, const uint8_t *be, size_t len)
{
    size_t off = 0;
    while (off < len && be[off] == 0)
        off++;
    bool pad = (off < len) && (be[off] & 0x80u);
    size_t mlen = (len - off) + (pad ? 1 : 0);
    wr_u32(p, (uint32_t)mlen);
    size_t o = 4;
    if (pad)
        p[o++] = 0;
    memcpy(p + o, be + off, len - off);
    return o + (len - off);
}

static size_t put_namelist(uint8_t *p, const char *s)
{
    return put_string(p, s);
}

static size_t build_client_kexinit(uint8_t *out)
{
    size_t o = 0;
    out[o++] = SSH_MSG_KEXINIT;
    for (int j = 0; j < 16; j++)
        out[o++] = (uint8_t)j;
    o += put_namelist(out + o, "diffie-hellman-group14-sha256");
    o += put_namelist(out + o, "rsa-sha2-256");
    o += put_namelist(out + o, "aes256-ctr");
    o += put_namelist(out + o, "aes256-ctr");
    o += put_namelist(out + o, "hmac-sha2-256");
    o += put_namelist(out + o, "hmac-sha2-256");
    o += put_namelist(out + o, "none");
    o += put_namelist(out + o, "none");
    o += put_namelist(out + o, "");
    o += put_namelist(out + o, "");
    out[o++] = 0;
    for (int j = 0; j < 4; j++)
        out[o++] = 0;
    return o;
}

// ---- the full handshake ---------------------------------------------------

void test_full_handshake_to_channel_data()
{
    SshSession *s = &ssh_sess[0];
    // Banner exchange already done out-of-band; seed V_C and enter KEXINIT.
    strcpy(s->v_c, "SSH-2.0-TestClient");
    s->v_c_len = (uint16_t)strlen(s->v_c);
    s->phase = SSH_PHASE_KEXINIT;

    // 1. Client KEXINIT → server replies KEXINIT, generates ephemeral.
    uint8_t pkt[2048];
    size_t n = build_client_kexinit(pkt);
    emt_reset();
    TEST_ASSERT_EQUAL_INT(0, ssh_server_dispatch(0, pkt[0], pkt, n));
    TEST_ASSERT_EQUAL_INT(1, emt_n);
    TEST_ASSERT_EQUAL(SSH_MSG_KEXINIT, emt_type[0]);
    TEST_ASSERT_EQUAL(SSH_PHASE_DH_INIT, s->phase);

    // 2. KEXDH_INIT (e = 2) → KEXDH_REPLY + NEWKEYS.
    uint8_t e_be[256];
    memset(e_be, 0, sizeof(e_be));
    e_be[255] = 0x02;
    n = 0;
    pkt[n++] = SSH_MSG_KEXDH_INIT;
    n += put_mpint(pkt + n, e_be, 256);
    emt_reset();
    TEST_ASSERT_EQUAL_INT(0, ssh_server_dispatch(0, pkt[0], pkt, n));
    TEST_ASSERT_EQUAL_INT(2, emt_n);
    TEST_ASSERT_EQUAL(SSH_MSG_KEXDH_REPLY, emt_type[0]);
    TEST_ASSERT_EQUAL(SSH_MSG_NEWKEYS, emt_type[1]);
    TEST_ASSERT_TRUE(ssh_keys[0].active);

    // 3. Client NEWKEYS → encryption active, service phase.
    uint8_t nk = SSH_MSG_NEWKEYS;
    emt_reset();
    TEST_ASSERT_EQUAL_INT(0, ssh_server_dispatch(0, nk, &nk, 1));
    TEST_ASSERT_TRUE(ssh_pkt[0].encrypted);
    TEST_ASSERT_EQUAL(SSH_PHASE_SERVICE, s->phase);

    // 4. SERVICE_REQUEST → SERVICE_ACCEPT, auth phase.
    n = 0;
    pkt[n++] = SSH_MSG_SERVICE_REQUEST;
    n += put_string(pkt + n, "ssh-userauth");
    emt_reset();
    TEST_ASSERT_EQUAL_INT(0, ssh_server_dispatch(0, pkt[0], pkt, n));
    TEST_ASSERT_EQUAL(SSH_MSG_SERVICE_ACCEPT, emt_type[0]);
    TEST_ASSERT_EQUAL(SSH_PHASE_AUTH, s->phase);

    // 5. USERAUTH_REQUEST (password) → SUCCESS, open phase.
    n = 0;
    pkt[n++] = SSH_MSG_USERAUTH_REQUEST;
    n += put_string(pkt + n, "alice");
    n += put_string(pkt + n, "ssh-connection");
    n += put_string(pkt + n, "password");
    pkt[n++] = 0;
    n += put_string(pkt + n, "s3cret");
    emt_reset();
    TEST_ASSERT_EQUAL_INT(0, ssh_server_dispatch(0, pkt[0], pkt, n));
    TEST_ASSERT_EQUAL(SSH_MSG_USERAUTH_SUCCESS, emt_type[0]);
    TEST_ASSERT_TRUE(s->authed);
    TEST_ASSERT_EQUAL(SSH_PHASE_OPEN, s->phase);

    // 6. CHANNEL_OPEN (session) → CONFIRMATION.
    n = 0;
    pkt[n++] = SSH_MSG_CHANNEL_OPEN;
    n += put_string(pkt + n, "session");
    wr_u32(pkt + n, 11);
    wr_u32(pkt + n + 4, 4096);
    wr_u32(pkt + n + 8, 32768);
    n += 12;
    emt_reset();
    TEST_ASSERT_EQUAL_INT(0, ssh_server_dispatch(0, pkt[0], pkt, n));
    TEST_ASSERT_EQUAL(SSH_MSG_CHANNEL_OPEN_CONFIRM, emt_type[0]);
    TEST_ASSERT_TRUE(ssh_chan[0][0].open);

    // 7. CHANNEL_REQUEST (shell, want_reply) → SUCCESS.
    n = 0;
    pkt[n++] = SSH_MSG_CHANNEL_REQUEST;
    wr_u32(pkt + n, 0);
    n += 4;
    n += put_string(pkt + n, "shell");
    pkt[n++] = 1;
    emt_reset();
    TEST_ASSERT_EQUAL_INT(0, ssh_server_dispatch(0, pkt[0], pkt, n));
    TEST_ASSERT_EQUAL(SSH_MSG_CHANNEL_SUCCESS, emt_type[0]);

    // 8. CHANNEL_DATA → application callback receives the bytes.
    n = 0;
    pkt[n++] = SSH_MSG_CHANNEL_DATA;
    wr_u32(pkt + n, 0);
    n += 4;
    n += put_string(pkt + n, "hi");
    emt_reset();
    TEST_ASSERT_EQUAL_INT(0, ssh_server_dispatch(0, pkt[0], pkt, n));
    TEST_ASSERT_EQUAL_INT(1, chan_data_count);
    TEST_ASSERT_EQUAL_INT(2, (int)chan_data_len);
    TEST_ASSERT_EQUAL_MEMORY("hi", chan_data, 2);
}

void test_channel_open_before_auth_rejected()
{
    SshSession *s = &ssh_sess[0];
    s->authed = false;
    s->phase = SSH_PHASE_SERVICE;
    uint8_t pkt[64];
    size_t n = 0;
    pkt[n++] = SSH_MSG_CHANNEL_OPEN;
    n += put_string(pkt + n, "session");
    wr_u32(pkt + n, 1);
    wr_u32(pkt + n + 4, 4096);
    wr_u32(pkt + n + 8, 32768);
    n += 12;
    TEST_ASSERT_EQUAL_INT(-1, ssh_server_dispatch(0, pkt[0], pkt, n));
}

void test_disconnect_closes()
{
    uint8_t pkt[1] = {SSH_MSG_DISCONNECT};
    TEST_ASSERT_EQUAL_INT(-1, ssh_server_dispatch(0, pkt[0], pkt, 1));
}

void test_ignore_is_noop()
{
    uint8_t pkt[1] = {SSH_MSG_IGNORE};
    emt_reset();
    TEST_ASSERT_EQUAL_INT(0, ssh_server_dispatch(0, pkt[0], pkt, 1));
    TEST_ASSERT_EQUAL_INT(0, emt_n);
}

// Build a password USERAUTH_REQUEST with the given (wrong/right) password.
static size_t build_password_auth(uint8_t *p, const char *user, const char *pass)
{
    size_t n = 0;
    p[n++] = SSH_MSG_USERAUTH_REQUEST;
    n += put_string(p + n, user);
    n += put_string(p + n, "ssh-connection");
    n += put_string(p + n, "password");
    p[n++] = 0; // change-password boolean = FALSE
    n += put_string(p + n, pass);
    return n;
}

// Repeated auth failures must be bounded: after SSH_MAX_AUTH_ATTEMPTS failed
// USERAUTH_REQUESTs the server sends SSH_MSG_DISCONNECT and signals close (-1).
void test_auth_bruteforce_disconnect()
{
    SshSession *s = &ssh_sess[0];
    s->phase = SSH_PHASE_AUTH;
    s->authed = false;
    s->auth_failures = 0;

    uint8_t pkt[128];
    size_t n = build_password_auth(pkt, "alice", "wrong");

    // The first SSH_MAX_AUTH_ATTEMPTS-1 failures keep the connection open.
    for (int k = 0; k < SSH_MAX_AUTH_ATTEMPTS - 1; k++)
    {
        emt_reset();
        TEST_ASSERT_EQUAL_INT(0, ssh_server_dispatch(0, pkt[0], pkt, n));
        TEST_ASSERT_EQUAL(SSH_MSG_USERAUTH_FAILURE, emt_type[0]);
        TEST_ASSERT_FALSE(s->authed);
    }
    TEST_ASSERT_EQUAL_INT(SSH_MAX_AUTH_ATTEMPTS - 1, s->auth_failures);

    // The final failure trips the limit: FAILURE then DISCONNECT, return -1.
    emt_reset();
    TEST_ASSERT_EQUAL_INT(-1, ssh_server_dispatch(0, pkt[0], pkt, n));
    TEST_ASSERT_EQUAL_INT(2, emt_n);
    TEST_ASSERT_EQUAL(SSH_MSG_USERAUTH_FAILURE, emt_type[0]);
    TEST_ASSERT_EQUAL(SSH_MSG_DISCONNECT, emt_type[1]);
}

// A successful auth before the limit does not count toward the failure budget.
void test_auth_success_after_failures()
{
    SshSession *s = &ssh_sess[0];
    s->phase = SSH_PHASE_AUTH;
    s->authed = false;
    s->auth_failures = 0;

    uint8_t pkt[128];
    size_t n = build_password_auth(pkt, "alice", "wrong");
    emt_reset();
    TEST_ASSERT_EQUAL_INT(0, ssh_server_dispatch(0, pkt[0], pkt, n));
    TEST_ASSERT_EQUAL(SSH_MSG_USERAUTH_FAILURE, emt_type[0]);

    n = build_password_auth(pkt, "alice", "s3cret");
    emt_reset();
    TEST_ASSERT_EQUAL_INT(0, ssh_server_dispatch(0, pkt[0], pkt, n));
    TEST_ASSERT_EQUAL(SSH_MSG_USERAUTH_SUCCESS, emt_type[0]);
    TEST_ASSERT_TRUE(s->authed);
    TEST_ASSERT_EQUAL_INT(1, s->auth_failures); // unchanged by the success
}

// An unrecognized message gets SSH_MSG_UNIMPLEMENTED with the rejected packet's
// sequence number (RFC 4253 §11.4). seq_no_recv has already advanced past it.
void test_unimplemented_reply_for_unknown_message()
{
    ssh_pkt_init(0);
    ssh_pkt[0].seq_no_recv = 7; // pretend recv() just processed packet #6
    emt_reset();
    uint8_t pkt[1] = {200}; // 200 is not a handled message type
    TEST_ASSERT_EQUAL_INT(0, ssh_server_dispatch(0, pkt[0], pkt, 1));
    TEST_ASSERT_EQUAL(SSH_MSG_UNIMPLEMENTED, emt_type[0]);
    TEST_ASSERT_EQUAL_INT(5, (int)emt_last_len);
    uint32_t seq = ((uint32_t)emt_last[1] << 24) | ((uint32_t)emt_last[2] << 16) | ((uint32_t)emt_last[3] << 8) |
                   (uint32_t)emt_last[4];
    TEST_ASSERT_EQUAL_UINT32(6, seq); // rejected packet = seq_no_recv - 1
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_full_handshake_to_channel_data);
    RUN_TEST(test_channel_open_before_auth_rejected);
    RUN_TEST(test_disconnect_closes);
    RUN_TEST(test_ignore_is_noop);
    RUN_TEST(test_auth_bruteforce_disconnect);
    RUN_TEST(test_auth_success_after_failures);
    RUN_TEST(test_unimplemented_reply_for_unknown_message);
    return UNITY_END();
}
