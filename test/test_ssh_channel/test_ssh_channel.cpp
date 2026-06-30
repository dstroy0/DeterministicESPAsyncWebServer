// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// SSH connection-protocol (channel) tests - RFC 4254, including multiplexing
// several channels over one connection (DETWS_SSH_MAX_CHANNELS > 1).

#include "network_drivers/presentation/ssh/ssh_channel.h"
#include "network_drivers/presentation/ssh/ssh_packet.h"
#include <stdint.h>
#include <string.h>
#include <unity.h>

static uint8_t last_data[256];
static size_t last_data_len;
static uint32_t last_channel;
static int data_cb_count;

static void data_cb(uint8_t slot, uint32_t channel, const uint8_t *d, size_t n)
{
    (void)slot;
    last_channel = channel;
    last_data_len = n < sizeof(last_data) ? n : sizeof(last_data);
    memcpy(last_data, d, last_data_len);
    data_cb_count++;
}

// direct-tcpip forward callbacks (port forwarding) ---------------------------
static char fwd_host[128];
static size_t fwd_host_len;
static uint16_t fwd_port;
static uint32_t fwd_open_channel;
static int fwd_open_count;
static int fwd_open_ret; // value the open cb returns (0 accept, <0 refuse)

static int fwd_open_cb(uint8_t slot, uint32_t channel, const char *host, size_t host_len, uint16_t port)
{
    (void)slot;
    fwd_open_channel = channel;
    fwd_host_len = host_len < sizeof(fwd_host) - 1 ? host_len : sizeof(fwd_host) - 1;
    memcpy(fwd_host, host, fwd_host_len);
    fwd_host[fwd_host_len] = 0;
    fwd_port = port;
    fwd_open_count++;
    return fwd_open_ret;
}

static uint8_t fwd_data[256];
static size_t fwd_data_len;
static uint32_t fwd_data_channel;
static int fwd_data_count;

static void fwd_data_cb(uint8_t slot, uint32_t channel, const uint8_t *d, size_t n)
{
    (void)slot;
    fwd_data_channel = channel;
    fwd_data_len = n < sizeof(fwd_data) ? n : sizeof(fwd_data);
    memcpy(fwd_data, d, fwd_data_len);
    fwd_data_count++;
}

void setUp()
{
    ssh_channel_init(0);
    ssh_channel_set_data_cb(data_cb);
    ssh_channel_set_forward_open_cb(nullptr); // forwarding off by default
    ssh_channel_set_forward_data_cb(nullptr);
    memset(last_data, 0, sizeof(last_data));
    last_data_len = 0;
    last_channel = 0xFFFFFFFFu;
    data_cb_count = 0;
    memset(fwd_host, 0, sizeof(fwd_host));
    fwd_host_len = 0;
    fwd_port = 0;
    fwd_open_channel = 0xFFFFFFFFu;
    fwd_open_count = 0;
    fwd_open_ret = 0;
    memset(fwd_data, 0, sizeof(fwd_data));
    fwd_data_len = 0;
    fwd_data_channel = 0xFFFFFFFFu;
    fwd_data_count = 0;
}
void tearDown()
{
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
static size_t put_string(uint8_t *p, const char *s)
{
    uint32_t n = (uint32_t)strlen(s);
    wr_u32(p, n);
    memcpy(p + 4, s, n);
    return 4 + n;
}

// Open a session channel; returns the local channel id from the confirmation.
static uint32_t open_session(uint32_t peer_id, uint32_t peer_window)
{
    uint8_t pkt[64];
    size_t n = 0;
    pkt[n++] = SSH_MSG_CHANNEL_OPEN;
    n += put_string(pkt + n, "session");
    wr_u32(pkt + n, peer_id);
    wr_u32(pkt + n + 4, peer_window);
    wr_u32(pkt + n + 8, 32768);
    n += 12;

    uint8_t out[64];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_handle_open(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_CHANNEL_OPEN_CONFIRM, out[0]);
    return rd_u32(out + 5); // local channel id
}

// Build a CHANNEL_DATA packet addressed to local channel @p recipient.
static size_t make_data(uint8_t *pkt, uint32_t recipient, const char *s)
{
    size_t n = 0;
    pkt[n++] = SSH_MSG_CHANNEL_DATA;
    wr_u32(pkt + n, recipient);
    n += 4;
    n += put_string(pkt + n, s);
    return n;
}

// Build a "direct-tcpip" CHANNEL_OPEN: connect host:port, origin advisory.
static size_t make_direct_tcpip(uint8_t *pkt, uint32_t sender, const char *host, uint16_t port)
{
    size_t n = 0;
    pkt[n++] = SSH_MSG_CHANNEL_OPEN;
    n += put_string(pkt + n, "direct-tcpip");
    wr_u32(pkt + n, sender);
    wr_u32(pkt + n + 4, 32768); // init window
    wr_u32(pkt + n + 8, 32768); // max packet
    n += 12;
    n += put_string(pkt + n, host); // host to connect
    wr_u32(pkt + n, port);          // port to connect
    n += 4;
    n += put_string(pkt + n, "127.0.0.1"); // originator host (advisory)
    wr_u32(pkt + n, 12345);                // originator port (advisory)
    n += 4;
    return n;
}

// ---- open -----------------------------------------------------------------

void test_open_session_confirms()
{
    uint32_t id = open_session(42, 1000);
    TEST_ASSERT_TRUE(ssh_chan[0][id].open);
    TEST_ASSERT_EQUAL_UINT32(42, ssh_chan[0][id].peer_id);
    TEST_ASSERT_EQUAL_UINT32(1000, ssh_chan[0][id].peer_window);
}

void test_open_unknown_type_fails()
{
    uint8_t pkt[64];
    size_t n = 0;
    pkt[n++] = SSH_MSG_CHANNEL_OPEN;
    n += put_string(pkt + n, "x11"); // not an accepted open type
    wr_u32(pkt + n, 7);
    wr_u32(pkt + n + 4, 1000);
    wr_u32(pkt + n + 8, 16384);
    n += 12;

    uint8_t out[64];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_handle_open(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_CHANNEL_OPEN_FAILURE, out[0]);
    TEST_ASSERT_EQUAL_UINT32(3u, rd_u32(out + 5)); // unknown channel type
    TEST_ASSERT_FALSE(ssh_chan[0][0].open);
}

// ---- direct-tcpip forward (port forwarding, ssh -L) -----------------------

void test_direct_tcpip_no_cb_prohibited()
{
    // Forwarding is opt-in: with no open callback installed it is refused.
    uint8_t pkt[96];
    size_t n = make_direct_tcpip(pkt, 7, "example.com", 80);
    uint8_t out[64];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_handle_open(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_CHANNEL_OPEN_FAILURE, out[0]);
    TEST_ASSERT_EQUAL_UINT32(1u, rd_u32(out + 5)); // administratively prohibited
    TEST_ASSERT_FALSE(ssh_chan[0][0].open);
}

void test_direct_tcpip_accept_confirms()
{
    ssh_channel_set_forward_open_cb(fwd_open_cb);
    fwd_open_ret = 0; // owner accepts (connected)
    uint8_t pkt[96];
    size_t n = make_direct_tcpip(pkt, 9, "example.com", 443);
    uint8_t out[64];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_handle_open(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_CHANNEL_OPEN_CONFIRM, out[0]);
    uint32_t id = rd_u32(out + 5);
    TEST_ASSERT_TRUE(ssh_chan[0][id].open);
    TEST_ASSERT_EQUAL_UINT8(SSH_CHAN_DIRECT_TCPIP, ssh_chan[0][id].type);
    TEST_ASSERT_EQUAL_INT(1, fwd_open_count);
    TEST_ASSERT_EQUAL_UINT32(id, fwd_open_channel);
    TEST_ASSERT_EQUAL_STRING("example.com", fwd_host);
    TEST_ASSERT_EQUAL_UINT16(443, fwd_port);
}

void test_direct_tcpip_refused_connect_failed()
{
    ssh_channel_set_forward_open_cb(fwd_open_cb);
    fwd_open_ret = -1; // owner could not connect
    uint8_t pkt[96];
    size_t n = make_direct_tcpip(pkt, 9, "10.0.0.9", 22);
    uint8_t out[64];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_handle_open(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_CHANNEL_OPEN_FAILURE, out[0]);
    TEST_ASSERT_EQUAL_UINT32(2u, rd_u32(out + 5)); // connect failed
    TEST_ASSERT_FALSE(ssh_chan[0][0].open);        // channel freed on refusal
}

void test_forward_data_routes_to_forward_cb()
{
    ssh_channel_set_forward_open_cb(fwd_open_cb);
    ssh_channel_set_forward_data_cb(fwd_data_cb);
    fwd_open_ret = 0;
    uint8_t pkt[96];
    size_t n = make_direct_tcpip(pkt, 9, "h", 80);
    uint8_t out[64];
    size_t olen = 0;
    ssh_channel_handle_open(0, pkt, n, out, &olen, sizeof(out));
    uint32_t id = rd_u32(out + 5);

    uint8_t dpkt[32];
    size_t dn = make_data(dpkt, id, "GET /");
    olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_handle_data(0, dpkt, dn, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL_INT(1, fwd_data_count); // routed to the forward owner
    TEST_ASSERT_EQUAL_INT(0, data_cb_count);  // NOT the session callback
    TEST_ASSERT_EQUAL_UINT32(id, fwd_data_channel);
    TEST_ASSERT_EQUAL_MEMORY("GET /", fwd_data, 5);
}

// ---- request --------------------------------------------------------------

void test_shell_request_success_with_reply()
{
    open_session(5, 1000);
    uint8_t pkt[64];
    size_t n = 0;
    pkt[n++] = SSH_MSG_CHANNEL_REQUEST;
    wr_u32(pkt + n, 0);
    n += 4;
    n += put_string(pkt + n, "shell");
    pkt[n++] = 1; // want_reply

    uint8_t out[16];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_handle_request(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_CHANNEL_SUCCESS, out[0]);
    TEST_ASSERT_EQUAL_UINT32(5, rd_u32(out + 1));
}

void test_unknown_request_failure()
{
    open_session(5, 1000);
    uint8_t pkt[64];
    size_t n = 0;
    pkt[n++] = SSH_MSG_CHANNEL_REQUEST;
    wr_u32(pkt + n, 0);
    n += 4;
    n += put_string(pkt + n, "x11-req");
    pkt[n++] = 1;

    uint8_t out[16];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_handle_request(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_CHANNEL_FAILURE, out[0]);
}

void test_request_no_reply_produces_nothing()
{
    open_session(5, 1000);
    uint8_t pkt[64];
    size_t n = 0;
    pkt[n++] = SSH_MSG_CHANNEL_REQUEST;
    wr_u32(pkt + n, 0);
    n += 4;
    n += put_string(pkt + n, "shell");
    pkt[n++] = 0; // want_reply = false

    uint8_t out[16];
    size_t olen = 99;
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_handle_request(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT32(0, (uint32_t)olen);
}

// ---- data -----------------------------------------------------------------

void test_inbound_data_invokes_callback()
{
    uint32_t id = open_session(5, 1000);
    uint8_t pkt[64];
    size_t n = make_data(pkt, id, "hello");

    uint8_t out[16];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_handle_data(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL_INT(1, data_cb_count);
    TEST_ASSERT_EQUAL_UINT32(id, last_channel);
    TEST_ASSERT_EQUAL_INT(5, (int)last_data_len);
    TEST_ASSERT_EQUAL_MEMORY("hello", last_data, 5);
}

void test_inbound_data_window_replenish()
{
    uint32_t id = open_session(5, 1000);
    uint8_t big[20000];
    memset(big, 'x', sizeof(big));
    uint8_t pkt[20100];
    size_t n = 0;
    pkt[n++] = SSH_MSG_CHANNEL_DATA;
    wr_u32(pkt + n, id);
    n += 4;
    wr_u32(pkt + n, (uint32_t)sizeof(big));
    n += 4;
    memcpy(pkt + n, big, sizeof(big));
    n += sizeof(big);

    uint8_t out[16];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_handle_data(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_CHANNEL_WINDOW_ADJUST, out[0]);
    TEST_ASSERT_EQUAL_UINT32(5, rd_u32(out + 1)); // peer channel
    TEST_ASSERT_EQUAL_UINT32(SSH_CHAN_WINDOW, ssh_chan[0][id].local_window);
}

void test_inbound_data_exceeding_window_rejected()
{
    uint32_t id = open_session(5, 1000);
    ssh_chan[0][id].local_window = 4; // shrink artificially
    uint8_t pkt[32];
    size_t n = make_data(pkt, id, "toolong"); // 7 bytes > 4
    uint8_t out[16];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_handle_data(0, pkt, n, out, &olen, sizeof(out)));
}

void test_outbound_data_frames_and_decrements_window()
{
    uint32_t id = open_session(5, 1000);
    uint8_t out[64];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_build_data(0, id, (const uint8_t *)"abc", 3, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_CHANNEL_DATA, out[0]);
    TEST_ASSERT_EQUAL_UINT32(5, rd_u32(out + 1)); // peer channel
    TEST_ASSERT_EQUAL_UINT32(3, rd_u32(out + 5)); // data length
    TEST_ASSERT_EQUAL_MEMORY("abc", out + 9, 3);
    TEST_ASSERT_EQUAL_UINT32(997, ssh_chan[0][id].peer_window);
}

void test_outbound_data_exceeding_peer_window_rejected()
{
    uint32_t id = open_session(5, 2); // tiny peer window
    uint8_t out[64];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_build_data(0, id, (const uint8_t *)"abc", 3, out, &olen, sizeof(out)));
}

void test_window_adjust_grows_peer_window()
{
    uint32_t id = open_session(5, 100);
    uint8_t pkt[9];
    pkt[0] = SSH_MSG_CHANNEL_WINDOW_ADJUST;
    wr_u32(pkt + 1, id);
    wr_u32(pkt + 5, 500);
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_handle_window_adjust(0, pkt, sizeof(pkt)));
    TEST_ASSERT_EQUAL_UINT32(600, ssh_chan[0][id].peer_window);
}

void test_build_close_emits_eof_and_close()
{
    uint32_t id = open_session(5, 1000);
    uint8_t out[16];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_build_close(0, id, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT32(10, (uint32_t)olen);
    TEST_ASSERT_EQUAL(SSH_MSG_CHANNEL_EOF, out[0]);
    TEST_ASSERT_EQUAL_UINT32(5, rd_u32(out + 1));
    TEST_ASSERT_EQUAL(SSH_MSG_CHANNEL_CLOSE, out[5]);
    TEST_ASSERT_EQUAL_UINT32(5, rd_u32(out + 6));
    TEST_ASSERT_FALSE(ssh_chan[0][id].open);
}

void test_inbound_close_routes_to_channel()
{
    uint32_t id = open_session(5, 1000);
    uint8_t pkt[8];
    pkt[0] = SSH_MSG_CHANNEL_CLOSE;
    wr_u32(pkt + 1, id);
    uint8_t out[16];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_handle_close(0, pkt, 5, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_CHANNEL_EOF, out[0]);
    TEST_ASSERT_EQUAL_UINT32(5, rd_u32(out + 1));
    TEST_ASSERT_FALSE(ssh_chan[0][id].open);
}

// ---- multiplexing (DETWS_SSH_MAX_CHANNELS > 1) ----------------------------

void test_multiplex_two_channels_route_independently()
{
    uint32_t a = open_session(5, 1000); // peer 5
    uint32_t b = open_session(7, 1000); // peer 7
    TEST_ASSERT_NOT_EQUAL(a, b);
    TEST_ASSERT_TRUE(ssh_chan[0][a].open);
    TEST_ASSERT_TRUE(ssh_chan[0][b].open);
    TEST_ASSERT_EQUAL_UINT32(5, ssh_chan[0][a].peer_id);
    TEST_ASSERT_EQUAL_UINT32(7, ssh_chan[0][b].peer_id);

    // Inbound data on channel b is delivered tagged with b.
    uint8_t pkt[32];
    size_t n = make_data(pkt, b, "to-b");
    uint8_t out[16];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_handle_data(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT32(b, last_channel);
    TEST_ASSERT_EQUAL_MEMORY("to-b", last_data, 4);

    // Outbound on channel a targets peer 5, on b targets peer 7.
    olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_build_data(0, a, (const uint8_t *)"x", 1, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT32(5, rd_u32(out + 1));
    olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_build_data(0, b, (const uint8_t *)"y", 1, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT32(7, rd_u32(out + 1));

    // Closing a leaves b open and routable.
    olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_build_close(0, a, out, &olen, sizeof(out)));
    TEST_ASSERT_FALSE(ssh_chan[0][a].open);
    TEST_ASSERT_TRUE(ssh_chan[0][b].open);
    olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_build_data(0, b, (const uint8_t *)"z", 1, out, &olen, sizeof(out)));
    // a is now closed: addressing it fails.
    olen = 0;
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_build_data(0, a, (const uint8_t *)"z", 1, out, &olen, sizeof(out)));
}

void test_pool_full_open_fails()
{
    for (int k = 0; k < DETWS_SSH_MAX_CHANNELS; k++)
        open_session((uint32_t)(10 + k), 1000);
    // One past the pool: CHANNEL_OPEN_FAILURE (resource shortage).
    uint8_t pkt[64];
    size_t n = 0;
    pkt[n++] = SSH_MSG_CHANNEL_OPEN;
    n += put_string(pkt + n, "session");
    wr_u32(pkt + n, 99);
    wr_u32(pkt + n + 4, 1000);
    wr_u32(pkt + n + 8, 16384);
    n += 12;
    uint8_t out[64];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_handle_open(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_CHANNEL_OPEN_FAILURE, out[0]);
    TEST_ASSERT_EQUAL_UINT32(4u, rd_u32(out + 5)); // reason 4 = resource shortage
}

void test_data_to_unknown_channel_rejected()
{
    open_session(5, 1000); // local id 0
    uint8_t pkt[32];
    size_t n = make_data(pkt, DETWS_SSH_MAX_CHANNELS + 5, "x"); // out-of-range recipient
    uint8_t out[16];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_handle_data(0, pkt, n, out, &olen, sizeof(out)));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_open_session_confirms);
    RUN_TEST(test_open_unknown_type_fails);
    RUN_TEST(test_direct_tcpip_no_cb_prohibited);
    RUN_TEST(test_direct_tcpip_accept_confirms);
    RUN_TEST(test_direct_tcpip_refused_connect_failed);
    RUN_TEST(test_forward_data_routes_to_forward_cb);
    RUN_TEST(test_shell_request_success_with_reply);
    RUN_TEST(test_unknown_request_failure);
    RUN_TEST(test_request_no_reply_produces_nothing);
    RUN_TEST(test_inbound_data_invokes_callback);
    RUN_TEST(test_inbound_data_window_replenish);
    RUN_TEST(test_inbound_data_exceeding_window_rejected);
    RUN_TEST(test_outbound_data_frames_and_decrements_window);
    RUN_TEST(test_outbound_data_exceeding_peer_window_rejected);
    RUN_TEST(test_window_adjust_grows_peer_window);
    RUN_TEST(test_build_close_emits_eof_and_close);
    RUN_TEST(test_inbound_close_routes_to_channel);
    RUN_TEST(test_multiplex_two_channels_route_independently);
    RUN_TEST(test_pool_full_open_fails);
    RUN_TEST(test_data_to_unknown_channel_rejected);
    return UNITY_END();
}
