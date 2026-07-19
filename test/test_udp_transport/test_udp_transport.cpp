// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for the UDP transport's multicast receive path (dws_udp_listen_multicast /
// dws_udp_leave_multicast): group validation, datagram delivery to the handler, and teardown.
// The device path joins the group via IGMP; on the host the mock records the join so the same
// call sequence a service makes (join -> receive -> leave) is exercised end to end.

#include "network_drivers/transport/udp.h"
#include <string.h>
#include <unity.h>

static int g_calls = 0;
static char g_last[64];
static size_t g_last_len = 0;
static char g_src_ip[16];
static uint16_t g_src_port = 0;
static void *g_ctx_seen = nullptr;

static void on_datagram(const uint8_t *data, size_t len, const struct DWSUdpPeer *peer, void *ctx)
{
    g_calls++;
    g_ctx_seen = ctx;
    g_last_len = (len < sizeof(g_last)) ? len : sizeof(g_last) - 1;
    memcpy(g_last, data, g_last_len);
    g_last[g_last_len] = '\0';
    dws_udp_peer_addr(peer, g_src_ip, sizeof(g_src_ip), &g_src_port);
}

void setUp(void)
{
    dws_udp_reset_listeners();
    g_calls = 0;
    g_last_len = 0;
    g_last[0] = '\0';
    g_src_ip[0] = '\0';
    g_src_port = 0;
    g_ctx_seen = nullptr;
}
void tearDown(void)
{
}

void test_join_records_the_group()
{
    TEST_ASSERT_TRUE(dws_udp_listen_multicast("224.0.0.251", 5353, on_datagram, nullptr));
    TEST_ASSERT_EQUAL_STRING("224.0.0.251", dws_udp_joined_group(5353));
    // A port with no multicast listener has no group.
    TEST_ASSERT_NULL(dws_udp_joined_group(1900));
}

void test_group_datagram_reaches_the_handler()
{
    int marker = 42;
    TEST_ASSERT_TRUE(dws_udp_listen_multicast("224.0.0.251", 5353, on_datagram, &marker));
    const char *pkt = "\x00\x00\x84\x00mdns-announce";
    dws_udp_inject(5353, "192.168.1.77", 5353, (const uint8_t *)pkt, 17);
    TEST_ASSERT_EQUAL_INT(1, g_calls);
    TEST_ASSERT_EQUAL_UINT(17, (unsigned)g_last_len);
    TEST_ASSERT_EQUAL_PTR(&marker, g_ctx_seen);
    TEST_ASSERT_EQUAL_STRING("192.168.1.77", g_src_ip);
    TEST_ASSERT_EQUAL_UINT16(5353, g_src_port);
}

void test_counts_repeated_announcements()
{
    // The contention-counting use case: many announcements land on one joined group.
    TEST_ASSERT_TRUE(dws_udp_listen_multicast("224.0.0.251", 5353, on_datagram, nullptr));
    for (int i = 0; i < 12; i++)
        dws_udp_inject(5353, "192.168.1.5", 5353, (const uint8_t *)"x", 1);
    TEST_ASSERT_EQUAL_INT(12, g_calls);
}

void test_rejects_non_multicast_group()
{
    // A unicast address would bind but never deliver - fail loudly instead.
    TEST_ASSERT_FALSE(dws_udp_listen_multicast("192.168.1.10", 5353, on_datagram, nullptr));
    TEST_ASSERT_FALSE(dws_udp_listen_multicast("223.255.255.255", 5353, on_datagram, nullptr)); // just below /4
    TEST_ASSERT_FALSE(dws_udp_listen_multicast("240.0.0.1", 5353, on_datagram, nullptr));       // just above /4
    TEST_ASSERT_NULL(dws_udp_joined_group(5353));
}

void test_accepts_group_range_edges()
{
    TEST_ASSERT_TRUE(dws_udp_listen_multicast("224.0.0.1", 5000, on_datagram, nullptr));
    TEST_ASSERT_TRUE(dws_udp_leave_multicast(5000));
    TEST_ASSERT_TRUE(dws_udp_listen_multicast("239.255.255.250", 1900, on_datagram, nullptr)); // SSDP
    TEST_ASSERT_EQUAL_STRING("239.255.255.250", dws_udp_joined_group(1900));
}

void test_rejects_malformed_group()
{
    TEST_ASSERT_FALSE(dws_udp_listen_multicast(nullptr, 5353, on_datagram, nullptr));
    TEST_ASSERT_FALSE(dws_udp_listen_multicast("", 5353, on_datagram, nullptr));
    TEST_ASSERT_FALSE(dws_udp_listen_multicast("224.0.0", 5353, on_datagram, nullptr));     // too few octets
    TEST_ASSERT_FALSE(dws_udp_listen_multicast("224.0.0.1.2", 5353, on_datagram, nullptr)); // too many
    TEST_ASSERT_FALSE(dws_udp_listen_multicast("224.0.0.256", 5353, on_datagram, nullptr)); // octet overflow
    TEST_ASSERT_FALSE(dws_udp_listen_multicast("224.0.0.", 5353, on_datagram, nullptr));    // trailing dot
    TEST_ASSERT_FALSE(dws_udp_listen_multicast("224.0..1", 5353, on_datagram, nullptr));    // empty octet
    TEST_ASSERT_FALSE(dws_udp_listen_multicast("224.0.0.abc", 5353, on_datagram, nullptr)); // non-digit
    TEST_ASSERT_NULL(dws_udp_joined_group(5353));
}

void test_leave_releases_the_slot()
{
    TEST_ASSERT_TRUE(dws_udp_listen_multicast("224.0.0.251", 5353, on_datagram, nullptr));
    TEST_ASSERT_TRUE(dws_udp_leave_multicast(5353));
    TEST_ASSERT_NULL(dws_udp_joined_group(5353));
    // Leaving twice, or leaving a port that never joined, is a no-op failure not a crash.
    TEST_ASSERT_FALSE(dws_udp_leave_multicast(5353));
    TEST_ASSERT_FALSE(dws_udp_leave_multicast(9999));
    // After leaving, the group no longer delivers.
    dws_udp_inject(5353, "192.168.1.5", 5353, (const uint8_t *)"x", 1);
    TEST_ASSERT_EQUAL_INT(0, g_calls);
}

void test_leave_ignores_a_plain_listener()
{
    // A non-multicast listener on the same port must not be torn down by a leave.
    TEST_ASSERT_TRUE(dws_udp_listen(5353, on_datagram, nullptr));
    TEST_ASSERT_FALSE(dws_udp_leave_multicast(5353));
    dws_udp_inject(5353, "192.168.1.5", 5353, (const uint8_t *)"y", 1);
    TEST_ASSERT_EQUAL_INT(1, g_calls); // still bound
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_join_records_the_group);
    RUN_TEST(test_group_datagram_reaches_the_handler);
    RUN_TEST(test_counts_repeated_announcements);
    RUN_TEST(test_rejects_non_multicast_group);
    RUN_TEST(test_accepts_group_range_edges);
    RUN_TEST(test_rejects_malformed_group);
    RUN_TEST(test_leave_releases_the_slot);
    RUN_TEST(test_leave_ignores_a_plain_listener);
    return UNITY_END();
}
