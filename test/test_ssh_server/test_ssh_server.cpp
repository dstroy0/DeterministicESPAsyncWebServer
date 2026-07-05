// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// End-to-end SSH server dispatcher test: drives a full handshake
// (KEXINIT → KEXDH → NEWKEYS → userauth → channel) through ssh_server_dispatch
// and checks the emitted messages and resulting state at each step.

#include "network_drivers/presentation/ssh/auth/ssh_auth.h"
#include "network_drivers/presentation/ssh/connection/ssh_channel.h"
#include "network_drivers/presentation/ssh/transport/ssh_dh.h"
#include "network_drivers/presentation/ssh/transport/ssh_packet.h"
#include "network_drivers/presentation/ssh/crypto/ssh_rsa.h"
#include "network_drivers/presentation/ssh/connection/ssh_server.h"
#include "network_drivers/presentation/ssh/transport/ssh_transport.h"
#include <stdint.h>
#include <stdio.h>
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
static uint32_t rd_u32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
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

static size_t build_client_kexinit(uint8_t *out, bool ext_info_c = true)
{
    size_t o = 0;
    out[o++] = SSH_MSG_KEXINIT;
    for (int j = 0; j < 16; j++)
        out[o++] = (uint8_t)j;
    o += put_namelist(out + o,
                      ext_info_c ? "diffie-hellman-group14-sha256,ext-info-c" // RFC 8308
                                 : "diffie-hellman-group14-sha256");
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

    // 3. Client NEWKEYS → encryption active, service phase. Because the client
    //    KEXINIT advertised ext-info-c, the server now sends EXT_INFO (RFC 8308).
    uint8_t nk = SSH_MSG_NEWKEYS;
    emt_reset();
    TEST_ASSERT_EQUAL_INT(0, ssh_server_dispatch(0, nk, &nk, 1));
    TEST_ASSERT_TRUE(ssh_pkt[0].encrypted);
    TEST_ASSERT_EQUAL(SSH_PHASE_SERVICE, s->phase);
    TEST_ASSERT_EQUAL_INT(1, emt_n);
    TEST_ASSERT_EQUAL(SSH_MSG_EXT_INFO, emt_type[0]);

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

// An inbound CHANNEL_CLOSE must be answered with CHANNEL_EOF and CHANNEL_CLOSE as
// two separate binary packets (RFC 4253 6) - not both bytes in one packet, which a
// strict peer (openssh packet_check_eom()) rejects.
void test_inbound_close_emits_eof_then_close_separately()
{
    // Open a channel so the close path has something to close (peer id 21).
    uint8_t op[64];
    size_t on = 0;
    op[on++] = SSH_MSG_CHANNEL_OPEN;
    on += put_string(op + on, "session");
    wr_u32(op + on, 21);
    wr_u32(op + on + 4, 4096);
    wr_u32(op + on + 8, 32768);
    on += 12;
    uint8_t obuf[64];
    size_t ol = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_handle_open(0, op, on, obuf, &ol, sizeof(obuf)));

    uint8_t pkt[8];
    pkt[0] = SSH_MSG_CHANNEL_CLOSE;
    wr_u32(pkt + 1, 0); // recipient = local channel 0
    emt_reset();
    TEST_ASSERT_EQUAL_INT(0, ssh_server_dispatch(0, SSH_MSG_CHANNEL_CLOSE, pkt, 5));
    TEST_ASSERT_EQUAL_INT(2, emt_n); // two distinct binary packets, not one
    TEST_ASSERT_EQUAL(SSH_MSG_CHANNEL_EOF, emt_type[0]);
    TEST_ASSERT_EQUAL(SSH_MSG_CHANNEL_CLOSE, emt_type[1]);
    TEST_ASSERT_FALSE(ssh_chan[0][0].open);
}

// RFC 8308: EXT_INFO advertises server-sig-algs listing every client public-key
// signature algorithm the server can verify (both rsa-sha2-256 and ssh-ed25519, ordered
// by the negotiation preference) so a modern OpenSSH client picks a key type it can offer.
void test_extinfo_build_advertises_server_sig_algs()
{
    ssh_kex_set_prefer_rsa(true); // deterministic ordering: rsa first
    uint8_t out[64];
    size_t n = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_extinfo_build(out, &n, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_EXT_INFO, out[0]);
    TEST_ASSERT_EQUAL_UINT32(1u, rd_u32(out + 1));  // one extension
    TEST_ASSERT_EQUAL_UINT32(15u, rd_u32(out + 5)); // strlen("server-sig-algs")
    TEST_ASSERT_EQUAL_MEMORY("server-sig-algs", out + 9, 15);
    size_t off = 9 + 15;
    const char *want = "rsa-sha2-256,ssh-ed25519";
    TEST_ASSERT_EQUAL_UINT32((uint32_t)strlen(want), rd_u32(out + off));
    TEST_ASSERT_EQUAL_MEMORY(want, out + off + 4, strlen(want));
}

// A real OpenSSH client KEXINIT is ~1.5 KB; the parser must accept one well past
// the old 512-byte bound (a smaller bound reset the connection at key exchange).
void test_large_client_kexinit_accepted()
{
    uint8_t pkt[2048];
    size_t o = 0;
    pkt[o++] = SSH_MSG_KEXINIT;
    for (int j = 0; j < 16; j++)
        pkt[o++] = 0; // cookie

    char kex[1200];
    size_t k = 0;
    for (int j = 0; j < 40; j++)
        k += (size_t)sprintf(kex + k, "filler-alg-%02d@example.com,", j);
    k += (size_t)sprintf(kex + k, "diffie-hellman-group14-sha256,ext-info-c");
    o += put_namelist(pkt + o, kex);
    o += put_namelist(pkt + o, "rsa-sha2-256");
    o += put_namelist(pkt + o, "aes256-ctr");
    o += put_namelist(pkt + o, "aes256-ctr");
    o += put_namelist(pkt + o, "hmac-sha2-256");
    o += put_namelist(pkt + o, "hmac-sha2-256");
    o += put_namelist(pkt + o, "none");
    o += put_namelist(pkt + o, "none");
    o += put_namelist(pkt + o, "");
    o += put_namelist(pkt + o, "");
    pkt[o++] = 0; // first_kex_packet_follows
    for (int j = 0; j < 4; j++)
        pkt[o++] = 0; // reserved

    TEST_ASSERT_TRUE(o > 512); // larger than the old SSH_KEXINIT_MAX
    SshSession *s = &ssh_sess[0];
    s->phase = SSH_PHASE_KEXINIT;
    TEST_ASSERT_EQUAL_INT(0, ssh_kexinit_parse(0, pkt, o));
    TEST_ASSERT_TRUE(s->ext_info_c); // ext-info-c detected in the big list
}

// A client that did NOT advertise ext-info-c must not be sent EXT_INFO.
void test_extinfo_not_sent_without_ext_info_c()
{
    SshSession *s = &ssh_sess[0];
    strcpy(s->v_c, "SSH-2.0-NoExt");
    s->v_c_len = (uint16_t)strlen(s->v_c);
    s->phase = SSH_PHASE_KEXINIT;

    uint8_t pkt[2048];
    size_t n = build_client_kexinit(pkt, /*ext_info_c=*/false);
    TEST_ASSERT_EQUAL_INT(0, ssh_server_dispatch(0, pkt[0], pkt, n));

    uint8_t e_be[256];
    memset(e_be, 0, sizeof(e_be));
    e_be[255] = 0x02;
    n = 0;
    pkt[n++] = SSH_MSG_KEXDH_INIT;
    n += put_mpint(pkt + n, e_be, 256);
    TEST_ASSERT_EQUAL_INT(0, ssh_server_dispatch(0, pkt[0], pkt, n));

    uint8_t nk = SSH_MSG_NEWKEYS;
    emt_reset();
    TEST_ASSERT_EQUAL_INT(0, ssh_server_dispatch(0, nk, &nk, 1));
    TEST_ASSERT_FALSE(ssh_sess[0].ext_info_c);
    TEST_ASSERT_EQUAL_INT(0, emt_n); // no EXT_INFO emitted
}

// An inbound EXT_INFO from the client is accepted and ignored (no UNIMPLEMENTED).
void test_inbound_ext_info_ignored()
{
    uint8_t pkt[1] = {SSH_MSG_EXT_INFO};
    emt_reset();
    TEST_ASSERT_EQUAL_INT(0, ssh_server_dispatch(0, SSH_MSG_EXT_INFO, pkt, 1));
    TEST_ASSERT_EQUAL_INT(0, emt_n);
}

// ---------------------------------------------------------------------------
// Packet-layer (ssh_packet.cpp) framing edge cases
// ---------------------------------------------------------------------------

#include "network_drivers/presentation/ssh/crypto/ssh_aes256ctr.h"
#include "network_drivers/presentation/ssh/transport/ssh_keymat.h"

static int g_pkt_calls = 0;
static void pkt_rec_handler(uint8_t slot, uint8_t msg_type, const uint8_t *payload, size_t len)
{
    (void)slot;
    (void)msg_type;
    (void)payload;
    (void)len;
    g_pkt_calls++;
}

// Every packet entry point rejects an out-of-range slot, and a send whose wire form
// does not fit the output buffer.
void test_ssh_pkt_index_and_cap_guards()
{
    uint8_t out[64];
    size_t out_len = 0;
    ssh_pkt_init(MAX_SSH_CONNS); // out-of-range slot: no-op, no crash
    TEST_ASSERT_EQUAL_INT(-1, ssh_pkt_send(MAX_SSH_CONNS, (const uint8_t *)"x", 1, out, &out_len, sizeof(out)));
    TEST_ASSERT_EQUAL_INT(-1, ssh_pkt_recv(MAX_SSH_CONNS, (const uint8_t *)"x", 1, pkt_rec_handler));
    TEST_ASSERT_EQUAL_INT(-1, ssh_pkt_disconnect(MAX_SSH_CONNS, 11, out, &out_len, sizeof(out)));

    ssh_pkt_init(0);
    uint8_t big_payload[64];
    memset(big_payload, 'a', sizeof(big_payload));
    TEST_ASSERT_EQUAL_INT(-1, ssh_pkt_send(0, big_payload, sizeof(big_payload), out, &out_len, 8)); // cap too small
}

// Unencrypted receive rejects a buffer overflow, an invalid packet length and an
// over-large padding length, and stalls (returns 0, consumes nothing) on a partial packet.
void test_ssh_pkt_recv_unencrypted_errors()
{
    static uint8_t overflow[SSH_PKT_BUF_SIZE + 1];
    memset(overflow, 0, sizeof(overflow));
    ssh_pkt_init(0);
    TEST_ASSERT_EQUAL_INT(-1, ssh_pkt_recv(0, overflow, sizeof(overflow), pkt_rec_handler)); // buffer overflow

    ssh_pkt_init(0);
    uint8_t zero_len[4] = {0, 0, 0, 0}; // packet_length == 0
    TEST_ASSERT_EQUAL_INT(-1, ssh_pkt_recv(0, zero_len, 4, pkt_rec_handler));

    ssh_pkt_init(0);
    uint8_t bad_pad[10] = {0, 0, 0, 6, 6, 1, 0, 0, 0, 0}; // padding_length 6 >= packet_length 6
    TEST_ASSERT_EQUAL_INT(-1, ssh_pkt_recv(0, bad_pad, sizeof(bad_pad), pkt_rec_handler));

    ssh_pkt_init(0);
    uint8_t partial[5] = {0, 0, 0, 6, 4}; // announces 6, only 5 present -> buffered, not consumed
    g_pkt_calls = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_pkt_recv(0, partial, sizeof(partial), pkt_rec_handler));
    TEST_ASSERT_EQUAL_INT(0, g_pkt_calls);
}

// The send and receive sequence-number overflow guards fire at the rekey threshold.
void test_ssh_pkt_seq_overflow_guards()
{
    uint8_t out[64];
    size_t out_len = 0;
    ssh_pkt_init(0);
    ssh_pkt[0].seq_no_send = SSH_SEQ_CLOSE_THRESHOLD;
    TEST_ASSERT_EQUAL_INT(-1, ssh_pkt_send(0, (const uint8_t *)"x", 1, out, &out_len, sizeof(out)));

    ssh_pkt_init(0);
    ssh_pkt[0].seq_no_recv = SSH_SEQ_CLOSE_THRESHOLD;
    uint8_t pkt[10] = {0, 0, 0, 6, 4, 42, 0, 0, 0, 0}; // a well-formed unencrypted packet
    TEST_ASSERT_EQUAL_INT(-1, ssh_pkt_recv(0, pkt, sizeof(pkt), pkt_rec_handler));
}

// An encrypted send round-trips through an encrypted receive (loopback keys), and a
// corrupted MAC is rejected.
void test_ssh_pkt_encrypted_roundtrip_and_mac_fail()
{
    uint8_t key[32], iv[16];
    memset(key, 0x11, sizeof(key));
    memset(iv, 0x22, sizeof(iv));

    ssh_pkt_init(0);
    ssh_pkt[0].encrypted = true;
    ssh_aes256ctr_init(&ssh_keys[0].s2c_ctx, key, iv);
    ssh_aes256ctr_init(&ssh_keys[0].c2s_ctx, key, iv); // identical -> loopback
    memset(ssh_keys[0].mac_key_s2c, 0x33, 32);
    memset(ssh_keys[0].mac_key_c2s, 0x33, 32);

    uint8_t payload[8];
    memset(payload, 0xAB, sizeof(payload));
    payload[0] = 50; // msg_type
    uint8_t out[256];
    size_t out_len = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_pkt_send(0, payload, sizeof(payload), out, &out_len, sizeof(out)));
    TEST_ASSERT_TRUE(out_len > 0);

    // Good packet: decrypt + MAC verify + deliver.
    ssh_aes256ctr_init(&ssh_keys[0].c2s_ctx, key, iv); // reset decrypt cipher to the send's start
    ssh_pkt[0].seq_no_recv = 0;
    ssh_pkt[0].rx_len = 0;
    uint8_t rx[256];
    memcpy(rx, out, out_len);
    g_pkt_calls = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_pkt_recv(0, rx, out_len, pkt_rec_handler));
    TEST_ASSERT_EQUAL_INT(1, g_pkt_calls);

    // Corrupted MAC (last byte) is rejected.
    ssh_aes256ctr_init(&ssh_keys[0].c2s_ctx, key, iv);
    ssh_pkt[0].seq_no_recv = 0;
    ssh_pkt[0].rx_len = 0;
    memcpy(rx, out, out_len);
    rx[out_len - 1] ^= 0xFF;
    TEST_ASSERT_EQUAL_INT(-1, ssh_pkt_recv(0, rx, out_len, pkt_rec_handler));

    ssh_pkt_init(0); // leave the slot clean for later tests
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_ssh_pkt_index_and_cap_guards);
    RUN_TEST(test_ssh_pkt_recv_unencrypted_errors);
    RUN_TEST(test_ssh_pkt_seq_overflow_guards);
    RUN_TEST(test_ssh_pkt_encrypted_roundtrip_and_mac_fail);
    RUN_TEST(test_full_handshake_to_channel_data);
    RUN_TEST(test_extinfo_build_advertises_server_sig_algs);
    RUN_TEST(test_extinfo_not_sent_without_ext_info_c);
    RUN_TEST(test_inbound_ext_info_ignored);
    RUN_TEST(test_large_client_kexinit_accepted);
    RUN_TEST(test_channel_open_before_auth_rejected);
    RUN_TEST(test_disconnect_closes);
    RUN_TEST(test_ignore_is_noop);
    RUN_TEST(test_auth_bruteforce_disconnect);
    RUN_TEST(test_auth_success_after_failures);
    RUN_TEST(test_unimplemented_reply_for_unknown_message);
    RUN_TEST(test_inbound_close_emits_eof_then_close_separately);
    return UNITY_END();
}
