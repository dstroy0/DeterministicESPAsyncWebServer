// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// SSH transport-glue test: drives a PROTO_SSH connection through the real
// conn_pool ring buffer + packet layer (banner exchange → KEXINIT) and checks
// the bytes written back to the socket via the tcp_write capture mock.

#include "lwip/tcp.h"
#include "network_drivers/presentation/ssh/connection/ssh_conn.h"
#include "network_drivers/presentation/ssh/crypto/ssh_rsa.h"
#include "network_drivers/presentation/ssh/transport/ssh_packet.h"
#include "network_drivers/presentation/ssh/transport/ssh_transport.h"
#include "network_drivers/transport/tcp.h"
#include <stdint.h>
#include <string.h>
#include <unity.h>

// Native RSA test fixture (defined in ssh_rsa.cpp native path).
extern uint8_t _test_rsa_n[256];
extern uint8_t _test_rsa_d[256];
extern uint8_t _test_rsa_e[4];

void setUp()
{
    for (int i = 0; i < MAX_CONNS; i++)
    {
        conn_pool[i] = {};
        conn_pool[i].id = (uint8_t)i;
        conn_pool[i].state = CONN_ACTIVE;
        conn_pool[i].pcb = &_mock_pcb;
        conn_pool[i].proto = PROTO_SSH;
        conn_pool[i].proto_slot = DETWS_PROTO_SLOT_NONE;
    }
    ssh_conn_setup();
    // A host key must be available for host-key negotiation to succeed (RSA here).
    memset(_test_rsa_n, 0, 256);
    _test_rsa_n[0] = 0xFF;
    _test_rsa_n[255] = 0x01;
    memset(_test_rsa_d, 0, 256);
    _test_rsa_d[255] = 0x01;
    _test_rsa_e[0] = 0x00;
    _test_rsa_e[1] = 0x01;
    _test_rsa_e[2] = 0x00;
    _test_rsa_e[3] = 0x01;
    ssh_rsa_load_pubkey();
    tcp_capture_reset();
}
void tearDown()
{
    // Release the SSH slot (mirrors a real disconnect) so it is free next test.
    ssh_conn_close(0);
    tcp_capture_disable();
}

static void push_bytes(uint8_t slot, const uint8_t *d, size_t n)
{
    TcpConn *c = &conn_pool[slot];
    for (size_t i = 0; i < n; i++)
    {
        c->rx_buffer[c->rx_head] = d[i];
        c->rx_head = (c->rx_head + 1) % RX_BUF_SIZE;
    }
}

static size_t put_namelist(uint8_t *p, const char *s)
{
    uint32_t n = (uint32_t)strlen(s);
    p[0] = (uint8_t)(n >> 24);
    p[1] = (uint8_t)(n >> 16);
    p[2] = (uint8_t)(n >> 8);
    p[3] = (uint8_t)n;
    memcpy(p + 4, s, n);
    return 4 + n;
}

static size_t build_kexinit_payload(uint8_t *out)
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

// Frame an unencrypted SSH binary packet around a payload (RFC 4253 §6).
static size_t frame_packet(uint8_t *out, const uint8_t *payload, size_t plen)
{
    size_t total = 5 + plen;
    size_t pad = 8 - (total % 8);
    if (pad < 4)
        pad += 8;
    size_t pkt_len = 1 + plen + pad;
    out[0] = (uint8_t)(pkt_len >> 24);
    out[1] = (uint8_t)(pkt_len >> 16);
    out[2] = (uint8_t)(pkt_len >> 8);
    out[3] = (uint8_t)pkt_len;
    out[4] = (uint8_t)pad;
    memcpy(out + 5, payload, plen);
    memset(out + 5 + plen, 0, pad);
    return 4 + pkt_len;
}

void test_accept_sends_server_banner()
{
    ssh_conn_accept(0);
    const char *resp = tcp_captured();
    TEST_ASSERT_TRUE(tcp_captured_len() >= 8);
    TEST_ASSERT_EQUAL_MEMORY("SSH-2.0-", resp, 8);
    TEST_ASSERT_NOT_EQUAL(DETWS_PROTO_SLOT_NONE, conn_pool[0].proto_slot);
}

void test_banner_then_kexinit_advances_and_replies()
{
    ssh_conn_accept(0);
    uint8_t j = conn_pool[0].proto_slot;
    tcp_capture_reset();

    // Client banner followed by a framed KEXINIT, delivered together.
    const char *banner = "SSH-2.0-TestClient\r\n";
    push_bytes(0, (const uint8_t *)banner, strlen(banner));

    uint8_t payload[700];
    size_t plen = build_kexinit_payload(payload);
    uint8_t pkt[768];
    size_t pktlen = frame_packet(pkt, payload, plen);
    push_bytes(0, pkt, pktlen);

    ssh_conn_rx(0);

    // The client identification string was captured.
    TEST_ASSERT_EQUAL_STRING("SSH-2.0-TestClient", ssh_sess[j].v_c);
    // Negotiation succeeded and the server replied + generated its ephemeral.
    TEST_ASSERT_EQUAL(SSH_PHASE_DH_INIT, ssh_sess[j].phase);
    // A server packet (KEXINIT) was written back.
    TEST_ASSERT_TRUE(tcp_captured_len() > 0);
}

void test_poll_triggers_server_rekey()
{
    ssh_conn_accept(0);
    uint8_t j = conn_pool[0].proto_slot;
    // An authenticated, open session that has spent its volume budget (packet-count proxy).
    ssh_sess[j].authed = true;
    ssh_sess[j].phase = SSH_PHASE_OPEN;
    ssh_sess[j].last_kex_ms = 0;
    ssh_pkt[j].kex_active = false;
    ssh_pkt[j].enc_out = true;
    ssh_pkt[j].enc_in = true;
    ssh_pkt[j].seq_no_send = SSH_REKEY_PACKET_THRESHOLD;
    tcp_capture_reset();

    ssh_conn_poll(0);
    // The server emitted a fresh KEXINIT and entered the re-key handshake (RFC 4253 §9).
    TEST_ASSERT_TRUE(tcp_captured_len() > 0);
    TEST_ASSERT_EQUAL(SSH_PHASE_KEXINIT, ssh_sess[j].phase);

    // A poll on an already-re-keying / under-budget session does not fire again.
    tcp_capture_reset();
    ssh_conn_poll(0);
    TEST_ASSERT_EQUAL(0, tcp_captured_len());
}

// The L5 ProtoHandler accessor exposes the installed dispatch table.
void test_proto_handler_accessor()
{
    TEST_ASSERT_NOT_NULL(ssh_proto_handler());
}

// The channel-send entry points fail closed on an out-of-range slot and on a mapped
// slot whose TCP conn has lost its pcb.
void test_send_entrypoints_reject()
{
    uint8_t data[4] = {1, 2, 3, 4};
    TEST_ASSERT_EQUAL_INT(-1, ssh_conn_send(250, 0, data, sizeof(data)));
    TEST_ASSERT_EQUAL_INT(-1, ssh_conn_close_channel(250, 0));
    TEST_ASSERT_EQUAL_INT(-1, ssh_conn_open_forwarded(250, "h", 22, "o", 1));

    ssh_conn_accept(0);
    uint8_t j = conn_pool[0].proto_slot;
    conn_pool[0].pcb = nullptr; // live SSH slot but the socket is gone
    TEST_ASSERT_EQUAL_INT(-1, ssh_conn_send(j, 0, data, sizeof(data)));
    TEST_ASSERT_EQUAL_INT(-1, ssh_conn_close_channel(j, 0));
    TEST_ASSERT_EQUAL_INT(-1, ssh_conn_open_forwarded(j, "h", 22, "o", 1));
}

// poll and rx ignore a live conn that is not a mapped SSH session, and rx on an empty
// ring reads nothing; a partial client banner leaves the session in the banner phase.
void test_poll_rx_banner_guards()
{
    ssh_conn_poll(1); // conn 1 is ACTIVE but never accepted (proto_slot == NONE)
    ssh_conn_rx(1);

    ssh_conn_accept(0);
    ssh_conn_rx(0); // empty rx ring -> reads nothing

    const char *partial = "SSH-2.0-Incomplete"; // no CRLF yet
    push_bytes(0, (const uint8_t *)partial, strlen(partial));
    ssh_conn_rx(0);
    uint8_t j = conn_pool[0].proto_slot;
    TEST_ASSERT_EQUAL(SSH_PHASE_BANNER, ssh_sess[j].phase);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_accept_sends_server_banner);
    RUN_TEST(test_banner_then_kexinit_advances_and_replies);
    RUN_TEST(test_poll_triggers_server_rekey);
    RUN_TEST(test_proto_handler_accessor);
    RUN_TEST(test_send_entrypoints_reject);
    RUN_TEST(test_poll_rx_banner_guards);
    return UNITY_END();
}
