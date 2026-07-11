// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// SSH connection-protocol (channel) tests - RFC 4254, including multiplexing
// several channels over one connection (DETWS_SSH_MAX_CHANNELS > 1).

#include "network_drivers/presentation/ssh/connection/ssh_channel.h"
#include "network_drivers/presentation/ssh/transport/ssh_packet.h"
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

// tcpip-forward (ssh -R) remote-forward callbacks ---------------------------
static char rfwd_addr[64];
static size_t rfwd_addr_len;
static uint16_t rfwd_port;
static int rfwd_open_count;
static int rfwd_cancel_count;
static int rfwd_open_ret;   // value the open cb returns (>= 0 bound port, < 0 refuse)
static int rfwd_cancel_ret; // value the cancel cb returns (0 ok, < 0 unknown)

static int rfwd_open_cb(uint8_t slot, const char *addr, size_t addr_len, uint16_t port)
{
    (void)slot;
    rfwd_addr_len = addr_len < sizeof(rfwd_addr) - 1 ? addr_len : sizeof(rfwd_addr) - 1;
    memcpy(rfwd_addr, addr, rfwd_addr_len);
    rfwd_addr[rfwd_addr_len] = 0;
    rfwd_port = port;
    rfwd_open_count++;
    return rfwd_open_ret;
}
static int rfwd_cancel_cb(uint8_t slot, const char *addr, size_t addr_len, uint16_t port)
{
    (void)slot;
    (void)addr;
    (void)addr_len;
    rfwd_port = port;
    rfwd_cancel_count++;
    return rfwd_cancel_ret;
}

// forwarded-tcpip (ssh -R) open-confirmation callback -----------------------
static uint32_t confirm_channel;
static bool confirm_ok;
static int confirm_count;

static void confirm_cb(uint8_t slot, uint32_t channel, bool ok)
{
    (void)slot;
    confirm_channel = channel;
    confirm_ok = ok;
    confirm_count++;
}

void setUp()
{
    ssh_channel_init(0);
    ssh_channel_set_data_cb(data_cb);
    ssh_channel_set_forward_open_cb(nullptr); // forwarding off by default
    ssh_channel_set_forward_data_cb(nullptr);
    ssh_channel_set_rforward_open_cb(nullptr); // remote forwarding off by default
    ssh_channel_set_rforward_cancel_cb(nullptr);
    ssh_channel_set_forward_confirm_cb(nullptr);
    memset(rfwd_addr, 0, sizeof(rfwd_addr));
    rfwd_addr_len = 0;
    rfwd_port = 0;
    rfwd_open_count = 0;
    rfwd_cancel_count = 0;
    rfwd_open_ret = 0;
    rfwd_cancel_ret = 0;
    confirm_channel = 0xFFFFFFFFu;
    confirm_ok = false;
    confirm_count = 0;
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
    TEST_ASSERT_EQUAL_UINT8(SshChanType::SSH_CHAN_DIRECT_TCPIP, ssh_chan[0][id].type);
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

// ---- global request (RFC 4254 §4; §7.1 tcpip-forward, ssh -R) --------------

// GLOBAL_REQUEST tcpip-forward / cancel-tcpip-forward: name, want_reply, bind addr, port.
static size_t make_global_fwd(uint8_t *pkt, const char *name, bool want_reply, const char *bind_addr, uint16_t port)
{
    size_t n = 0;
    pkt[n++] = SSH_MSG_GLOBAL_REQUEST;
    n += put_string(pkt + n, name);
    pkt[n++] = want_reply ? 1 : 0;
    n += put_string(pkt + n, bind_addr);
    wr_u32(pkt + n, port);
    n += 4;
    return n;
}

// GLOBAL_REQUEST with an arbitrary (non-forward) name and no request-specific data.
static size_t make_global_other(uint8_t *pkt, const char *name, bool want_reply)
{
    size_t n = 0;
    pkt[n++] = SSH_MSG_GLOBAL_REQUEST;
    n += put_string(pkt + n, name);
    pkt[n++] = want_reply ? 1 : 0;
    return n;
}

// With no remote-forward owner installed, tcpip-forward is refused (REQUEST_FAILURE).
void test_rforward_no_cb_refused()
{
    uint8_t pkt[64], out[16];
    size_t olen = 99;
    size_t n = make_global_fwd(pkt, "tcpip-forward", true, "", 8080);
    TEST_ASSERT_EQUAL_INT(0, ssh_global_request_handle(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(1u, olen);
    TEST_ASSERT_EQUAL(SSH_MSG_REQUEST_FAILURE, out[0]);
    TEST_ASSERT_EQUAL_INT(0, rfwd_open_count);
}

// An accepted specific-port forward replies with a bare REQUEST_SUCCESS.
void test_rforward_accept_specific_port()
{
    ssh_channel_set_rforward_open_cb(rfwd_open_cb);
    rfwd_open_ret = 8080;
    uint8_t pkt[64], out[16];
    size_t olen = 99;
    size_t n = make_global_fwd(pkt, "tcpip-forward", true, "0.0.0.0", 8080);
    TEST_ASSERT_EQUAL_INT(0, ssh_global_request_handle(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(1u, olen);
    TEST_ASSERT_EQUAL(SSH_MSG_REQUEST_SUCCESS, out[0]);
    TEST_ASSERT_EQUAL_INT(1, rfwd_open_count);
    TEST_ASSERT_EQUAL_STRING("0.0.0.0", rfwd_addr);
    TEST_ASSERT_EQUAL_UINT16(8080, rfwd_port);
}

// A port-0 request that is accepted echoes the allocated port (RFC 4254 §7.1).
void test_rforward_port0_echoes_allocated()
{
    ssh_channel_set_rforward_open_cb(rfwd_open_cb);
    rfwd_open_ret = 54321;
    uint8_t pkt[64], out[16];
    size_t olen = 99;
    size_t n = make_global_fwd(pkt, "tcpip-forward", true, "", 0);
    TEST_ASSERT_EQUAL_INT(0, ssh_global_request_handle(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(5u, olen);
    TEST_ASSERT_EQUAL(SSH_MSG_REQUEST_SUCCESS, out[0]);
    TEST_ASSERT_EQUAL_UINT32(54321u, rd_u32(out + 1));
}

// Accepted but want_reply = false -> the callback still runs, but no reply is emitted.
void test_rforward_no_reply_silent()
{
    ssh_channel_set_rforward_open_cb(rfwd_open_cb);
    rfwd_open_ret = 8080;
    uint8_t pkt[64], out[16];
    size_t olen = 99;
    size_t n = make_global_fwd(pkt, "tcpip-forward", false, "", 8080);
    TEST_ASSERT_EQUAL_INT(0, ssh_global_request_handle(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(0u, olen);
    TEST_ASSERT_EQUAL_INT(1, rfwd_open_count);
}

// cancel-tcpip-forward routes to the cancel callback and replies REQUEST_SUCCESS.
void test_rforward_cancel()
{
    ssh_channel_set_rforward_cancel_cb(rfwd_cancel_cb);
    rfwd_cancel_ret = 0;
    uint8_t pkt[64], out[16];
    size_t olen = 99;
    size_t n = make_global_fwd(pkt, "cancel-tcpip-forward", true, "", 8080);
    TEST_ASSERT_EQUAL_INT(0, ssh_global_request_handle(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(1u, olen);
    TEST_ASSERT_EQUAL(SSH_MSG_REQUEST_SUCCESS, out[0]);
    TEST_ASSERT_EQUAL_INT(1, rfwd_cancel_count);
    TEST_ASSERT_EQUAL_UINT16(8080, rfwd_port);
}

// An unrecognized global request answers REQUEST_FAILURE when want_reply is set
// (RFC 4254 §4, never UNIMPLEMENTED), and is silent otherwise.
void test_global_unknown_request()
{
    uint8_t pkt[64], out[16];
    size_t olen = 99;
    size_t n = make_global_other(pkt, "keepalive@openssh.com", true);
    TEST_ASSERT_EQUAL_INT(0, ssh_global_request_handle(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(1u, olen);
    TEST_ASSERT_EQUAL(SSH_MSG_REQUEST_FAILURE, out[0]);

    olen = 99;
    n = make_global_other(pkt, "hostkeys-00@openssh.com", false);
    TEST_ASSERT_EQUAL_INT(0, ssh_global_request_handle(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(0u, olen); // no reply when want_reply is unset
}

// A truncated GLOBAL_REQUEST (missing the request-name string) is rejected.
void test_global_malformed()
{
    uint8_t pkt[4], out[16];
    pkt[0] = SSH_MSG_GLOBAL_REQUEST;
    size_t olen = 99;
    TEST_ASSERT_EQUAL_INT(-1, ssh_global_request_handle(0, pkt, 1, out, &olen, sizeof(out)));
}

// ---- forwarded-tcpip: server-initiated channel (ssh -R) -------------------

// Build a CHANNEL_OPEN_CONFIRMATION for our local channel @p recipient.
static size_t make_open_confirm(uint8_t *pkt, uint32_t recipient, uint32_t sender, uint32_t window, uint32_t maxpkt)
{
    pkt[0] = SSH_MSG_CHANNEL_OPEN_CONFIRM;
    wr_u32(pkt + 1, recipient);
    wr_u32(pkt + 5, sender);
    wr_u32(pkt + 9, window);
    wr_u32(pkt + 13, maxpkt);
    return 17;
}

// ssh_channel_open_forwarded builds a valid forwarded-tcpip CHANNEL_OPEN and marks
// the channel pending (a session open cannot reuse the pending slot).
void test_forwarded_open_builds_channel()
{
    uint8_t out[128];
    size_t olen = 0;
    int ch = ssh_channel_open_forwarded(0, "10.0.0.1", 8080, "192.168.1.9", 51000, out, &olen, sizeof(out));
    TEST_ASSERT_TRUE(ch >= 0);
    TEST_ASSERT_EQUAL(SSH_MSG_CHANNEL_OPEN, out[0]);
    // string "forwarded-tcpip" then our sender channel id.
    TEST_ASSERT_EQUAL_UINT32(15u, rd_u32(out + 1));
    TEST_ASSERT_EQUAL_MEMORY("forwarded-tcpip", out + 5, 15);
    TEST_ASSERT_EQUAL_UINT32((uint32_t)ch, rd_u32(out + 20)); // sender == local channel id
    // The channel is pending, so a subsequent session open must claim a different slot.
    uint32_t sess = open_session(7, 4096);
    TEST_ASSERT_NOT_EQUAL((uint32_t)ch, sess);
}

// A CONFIRMATION marks the channel open, records the peer window, and fires the cb.
void test_forwarded_confirm_opens_channel()
{
    ssh_channel_set_forward_confirm_cb(confirm_cb);
    uint8_t out[128];
    size_t olen = 0;
    int ch = ssh_channel_open_forwarded(0, "10.0.0.1", 22, "1.2.3.4", 40000, out, &olen, sizeof(out));
    TEST_ASSERT_TRUE(ch >= 0);

    uint8_t pkt[17];
    size_t n = make_open_confirm(pkt, (uint32_t)ch, 99, 5000, 16384);
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_handle_open_confirm(0, pkt, n));
    TEST_ASSERT_EQUAL_INT(1, confirm_count);
    TEST_ASSERT_TRUE(confirm_ok);
    TEST_ASSERT_EQUAL_UINT32((uint32_t)ch, confirm_channel);

    // Now that it is open, the server can frame outbound data toward the peer window.
    uint8_t dout[64];
    size_t dlen = 0;
    TEST_ASSERT_EQUAL_INT(0,
                          ssh_channel_build_data(0, (uint32_t)ch, (const uint8_t *)"hi", 2, dout, &dlen, sizeof(dout)));
    TEST_ASSERT_EQUAL(SSH_MSG_CHANNEL_DATA, dout[0]);
    TEST_ASSERT_EQUAL_UINT32(99u, rd_u32(dout + 1)); // addressed to the peer's channel id
}

// A FAILURE frees the pending channel (its slot is reusable) and fires cb(ok=false).
void test_forwarded_failure_frees_channel()
{
    ssh_channel_set_forward_confirm_cb(confirm_cb);
    uint8_t out[128];
    size_t olen = 0;
    int ch = ssh_channel_open_forwarded(0, "10.0.0.1", 22, "1.2.3.4", 40000, out, &olen, sizeof(out));
    TEST_ASSERT_TRUE(ch >= 0);

    uint8_t pkt[17];
    pkt[0] = SSH_MSG_CHANNEL_OPEN_FAILURE;
    wr_u32(pkt + 1, (uint32_t)ch);
    wr_u32(pkt + 5, 2u); // reason: connect failed
    wr_u32(pkt + 9, 0);  // empty description
    wr_u32(pkt + 13, 0); // empty language
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_handle_open_failure(0, pkt, 17));
    TEST_ASSERT_EQUAL_INT(1, confirm_count);
    TEST_ASSERT_FALSE(confirm_ok);

    // The freed slot is reusable: a session open may now claim the same channel id.
    uint32_t sess = open_session(3, 4096);
    TEST_ASSERT_EQUAL_UINT32((uint32_t)ch, sess);
}

// A CONFIRMATION / FAILURE for a channel we did not open (no pending) is rejected.
void test_forwarded_confirm_unknown_rejected()
{
    uint8_t pkt[17];
    size_t n = make_open_confirm(pkt, 0, 5, 1000, 8192);
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_handle_open_confirm(0, pkt, n)); // nothing pending
    pkt[0] = SSH_MSG_CHANNEL_OPEN_FAILURE;
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_handle_open_failure(0, pkt, 17));
}

// Inbound data on a confirmed forwarded-tcpip channel routes to the forward owner,
// not the session data callback (ssh -R return path).
void test_forwarded_inbound_data_routes_to_forward_cb()
{
    ssh_channel_set_forward_data_cb(fwd_data_cb);
    uint8_t out[128];
    size_t olen = 0;
    int ch = ssh_channel_open_forwarded(0, "10.0.0.1", 22, "1.2.3.4", 40000, out, &olen, sizeof(out));
    TEST_ASSERT_TRUE(ch >= 0);
    uint8_t cpkt[17];
    make_open_confirm(cpkt, (uint32_t)ch, 42, 5000, 16384);
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_handle_open_confirm(0, cpkt, 17));

    uint8_t pkt[64];
    size_t n = make_data(pkt, (uint32_t)ch, "payload");
    uint8_t dout[16];
    size_t dolen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_channel_handle_data(0, pkt, n, dout, &dolen, sizeof(dout)));
    TEST_ASSERT_EQUAL_INT(1, fwd_data_count);
    TEST_ASSERT_EQUAL_INT(0, data_cb_count); // NOT delivered as session data
    TEST_ASSERT_EQUAL_UINT32((uint32_t)ch, fwd_data_channel);
    TEST_ASSERT_EQUAL_MEMORY("payload", fwd_data, 7);
}

// ---- guard / error-branch coverage ----------------------------------------

// Every entry point rejects an out-of-range slot or a wrong leading message byte.
void test_chan_slot_and_msgtype_guards()
{
    uint8_t out[32];
    size_t ol = 0;
    uint8_t z[17] = {0};             // leading byte 0 != any handled message type
    ssh_channel_init(MAX_SSH_CONNS); // no-op
    TEST_ASSERT_EQUAL_INT(-1, ssh_global_request_handle(0, z, sizeof(z), out, &ol, sizeof(out)));      // 175
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_handle_open(MAX_SSH_CONNS, z, 1, out, &ol, sizeof(out)));    // 346
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_handle_open_confirm(MAX_SSH_CONNS, z, 17));                  // 312
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_handle_open_failure(0, z, 5));                               // 331
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_handle_request(MAX_SSH_CONNS, z, 1, out, &ol, sizeof(out))); // 412
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_handle_data(MAX_SSH_CONNS, z, 1, out, &ol, sizeof(out)));    // 454
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_handle_window_adjust(0, z, 9));                              // 533
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_build_close(MAX_SSH_CONNS, 0, out, &ol, sizeof(out)));       // 567
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_handle_close(0, z, 5, out, &ol, sizeof(out)));               // 575
}

// Malformed payloads (over-long strings, missing trailing fields, unknown channel) are rejected.
void test_chan_malformed_payloads()
{
    uint8_t out[64];
    size_t ol = 0;
    size_t n = 0;

    uint8_t over[5] = {SSH_MSG_CHANNEL_OPEN, 0x00, 0x00, 0x00, 0xFF}; // type len 255 overruns -> rd_string (96)
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_handle_open(0, over, sizeof(over), out, &ol, sizeof(out)));

    uint8_t shortpkt[16];
    n = 0;
    shortpkt[n++] = SSH_MSG_CHANNEL_OPEN;
    n += put_string(shortpkt + n, "x"); // type ok but < 12 trailing bytes (354)
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_handle_open(0, shortpkt, n, out, &ol, sizeof(out)));

    ssh_channel_set_forward_open_cb(fwd_open_cb); // forwarding on so direct-tcpip reaches the host parse
    uint8_t dt[40];
    n = 0;
    dt[n++] = SSH_MSG_CHANNEL_OPEN;
    n += put_string(dt + n, "direct-tcpip");
    wr_u32(dt + n, 7);
    wr_u32(dt + n + 4, 32768);
    wr_u32(dt + n + 8, 32768);
    n += 12;
    dt[n++] = 0;
    dt[n++] = 0;
    dt[n++] = 0;
    dt[n++] = 0xFF; // host string len 255, truncated (374)
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_handle_open(0, dt, n, out, &ol, sizeof(out)));

    uint8_t rq[32];
    n = 0;
    rq[n++] = SSH_MSG_CHANNEL_REQUEST;
    wr_u32(rq + n, 0);
    n += 4;
    rq[n++] = 0;
    rq[n++] = 0;
    rq[n++] = 0;
    rq[n++] = 0xFF; // rtype len 255 -> rd_string fail (422)
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_handle_request(0, rq, n, out, &ol, sizeof(out)));
    n = 0;
    rq[n++] = SSH_MSG_CHANNEL_REQUEST;
    wr_u32(rq + n, 0);
    n += 4;
    n += put_string(rq + n, "shell"); // no want_reply byte (424)
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_handle_request(0, rq, n, out, &ol, sizeof(out)));
    n = 0;
    rq[n++] = SSH_MSG_CHANNEL_REQUEST;
    wr_u32(rq + n, 99);
    n += 4;
    n += put_string(rq + n, "shell");
    rq[n++] = 1; // recipient 99 not open (429)
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_handle_request(0, rq, n, out, &ol, sizeof(out)));

    uint8_t dp[16];
    n = 0;
    dp[n++] = SSH_MSG_CHANNEL_DATA;
    wr_u32(dp + n, 0);
    n += 4;
    dp[n++] = 0;
    dp[n++] = 0;
    dp[n++] = 0;
    dp[n++] = 0xFF; // data len 255 truncated (464)
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_handle_data(0, dp, n, out, &ol, sizeof(out)));

    uint8_t g[40];
    n = 0;
    g[n++] = SSH_MSG_GLOBAL_REQUEST;
    n += put_string(g + n, "x"); // name but no want_reply byte (183)
    TEST_ASSERT_EQUAL_INT(-1, ssh_global_request_handle(0, g, n, out, &ol, sizeof(out)));
    n = 0;
    g[n++] = SSH_MSG_GLOBAL_REQUEST;
    n += put_string(g + n, "tcpip-forward");
    g[n++] = 1;
    g[n++] = 0;
    g[n++] = 0;
    g[n++] = 0;
    g[n++] = 0xFF; // truncated bind address (195)
    TEST_ASSERT_EQUAL_INT(-1, ssh_global_request_handle(0, g, n, out, &ol, sizeof(out)));
}

// build_open_failure / build_open_confirm reject an output buffer smaller than 17 bytes.
void test_chan_open_cap_guards()
{
    uint8_t out[64];
    size_t ol = 0, n = 0;
    uint8_t unk[32];
    unk[n++] = SSH_MSG_CHANNEL_OPEN;
    n += put_string(unk + n, "bogus");
    wr_u32(unk + n, 1);
    wr_u32(unk + n + 4, 32768);
    wr_u32(unk + n + 8, 32768);
    n += 12;
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_handle_open(0, unk, n, out, &ol, 10)); // unknown type -> failure cap<17 (143)

    n = 0;
    uint8_t ses[32];
    ses[n++] = SSH_MSG_CHANNEL_OPEN;
    n += put_string(ses + n, "session");
    wr_u32(ses + n, 1);
    wr_u32(ses + n + 4, 32768);
    wr_u32(ses + n + 8, 32768);
    n += 12;
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_handle_open(0, ses, n, out, &ol, 10)); // session -> confirm cap<17 (157)
}

// open_forwarded guards (null addr, tiny cap, pool full) + per-channel cap guards.
void test_chan_forward_and_channel_guards()
{
    uint8_t out[64];
    size_t ol = 0, n = 0;
    // While a slot is free: null address (262) and a too-small buffer (273).
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_open_forwarded(0, nullptr, 80, "x", 90, out, &ol, sizeof(out))); // 262
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_open_forwarded(0, "10.0.0.1", 80, "1.2.3.4", 90, out, &ol, 10)); // 273

    uint32_t ch = open_session(5, 32768);
    uint8_t rq[32];
    n = 0;
    rq[n++] = SSH_MSG_CHANNEL_REQUEST;
    wr_u32(rq + n, ch);
    n += 4;
    n += put_string(rq + n, "shell");
    rq[n++] = 1;
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_handle_request(0, rq, n, out, &ol, 3)); // want_reply cap<5 (439)

    ssh_chan[0][ch].peer_window = 1000;
    ssh_chan[0][ch].peer_max_pkt = 1000;
    TEST_ASSERT_EQUAL_INT(-1,
                          ssh_channel_build_data(0, ch, (const uint8_t *)"hello", 5, out, &ol, 5)); // cap<9+len (515)
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_build_close(0, 99, out, &ol, sizeof(out)));               // null chan (553)

    // With every channel slot occupied, a server-initiated open is refused (265).
    for (int c = 0; c < DETWS_SSH_MAX_CHANNELS; c++)
        ssh_chan[0][c].open = true;
    TEST_ASSERT_EQUAL_INT(-1, ssh_channel_open_forwarded(0, "10.0.0.1", 80, "1.2.3.4", 90, out, &ol, sizeof(out)));
}

// GLOBAL_REQUEST reply paths that cannot fit the (tiny) output buffer.
void test_chan_global_request_reply_caps()
{
    uint8_t out[64];
    size_t ol = 0, n = 0;
    uint8_t g[64];
    // Unknown request name, want_reply, no room for the 1-byte reply (246).
    g[n++] = SSH_MSG_GLOBAL_REQUEST;
    n += put_string(g + n, "unknown-req");
    g[n++] = 1;
    TEST_ASSERT_EQUAL_INT(-1, ssh_global_request_handle(0, g, n, out, &ol, 0)); // 246

    // tcpip-forward refused (no cb), want_reply, no room (210).
    n = 0;
    g[n++] = SSH_MSG_GLOBAL_REQUEST;
    n += put_string(g + n, "tcpip-forward");
    g[n++] = 1;
    n += put_string(g + n, "0.0.0.0");
    wr_u32(g + n, 8080);
    n += 4;
    TEST_ASSERT_EQUAL_INT(-1, ssh_global_request_handle(0, g, n, out, &ol, 0)); // 210

    // tcpip-forward accepted with port 0 (echo), want_reply, no room for the 5-byte reply (224).
    ssh_channel_set_rforward_open_cb(rfwd_open_cb);
    rfwd_open_ret = 9000;
    n = 0;
    g[n++] = SSH_MSG_GLOBAL_REQUEST;
    n += put_string(g + n, "tcpip-forward");
    g[n++] = 1;
    n += put_string(g + n, "0.0.0.0");
    wr_u32(g + n, 0);
    n += 4;
    TEST_ASSERT_EQUAL_INT(-1, ssh_global_request_handle(0, g, n, out, &ol, 3)); // 224

    // cancel accepted, want_reply, no room for the bare success (232).
    ssh_channel_set_rforward_cancel_cb(rfwd_cancel_cb);
    rfwd_cancel_ret = 0;
    n = 0;
    g[n++] = SSH_MSG_GLOBAL_REQUEST;
    n += put_string(g + n, "cancel-tcpip-forward");
    g[n++] = 1;
    n += put_string(g + n, "0.0.0.0");
    wr_u32(g + n, 8080);
    n += 4;
    TEST_ASSERT_EQUAL_INT(-1, ssh_global_request_handle(0, g, n, out, &ol, 0)); // 232
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_chan_slot_and_msgtype_guards);
    RUN_TEST(test_chan_malformed_payloads);
    RUN_TEST(test_chan_open_cap_guards);
    RUN_TEST(test_chan_forward_and_channel_guards);
    RUN_TEST(test_chan_global_request_reply_caps);
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
    RUN_TEST(test_rforward_no_cb_refused);
    RUN_TEST(test_rforward_accept_specific_port);
    RUN_TEST(test_rforward_port0_echoes_allocated);
    RUN_TEST(test_rforward_no_reply_silent);
    RUN_TEST(test_rforward_cancel);
    RUN_TEST(test_global_unknown_request);
    RUN_TEST(test_global_malformed);
    RUN_TEST(test_forwarded_open_builds_channel);
    RUN_TEST(test_forwarded_confirm_opens_channel);
    RUN_TEST(test_forwarded_failure_frees_channel);
    RUN_TEST(test_forwarded_confirm_unknown_rejected);
    RUN_TEST(test_forwarded_inbound_data_routes_to_forward_cb);
    return UNITY_END();
}
