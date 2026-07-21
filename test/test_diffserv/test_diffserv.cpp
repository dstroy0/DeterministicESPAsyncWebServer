// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

// DiffServ QoS marking (DWS_ENABLE_DIFFSERV) host tests: the DSCP->TOS encode, the server-wide + UDP
// DSCP defaults, the per-connection setter (writes pcb->tos), and the per-listener override incl. the
// UNSET sentinel + 6-bit mask + unknown-port miss. The accept-callback apply (static, driven only through
// a real lwIP accept) is HW-verified on-wire via tcpdump; here the reachable API surface is asserted.

#include "lwip/tcp.h" // mock struct tcp_pcb (carries the tos field)
#include "network_drivers/transport/diffserv.h"
#include "network_drivers/transport/listener.h"
#include "network_drivers/transport/tcp.h"
#include <unity.h>

void setUp()
{
    dws_set_default_dscp(0); // best-effort baseline so each test starts clean
    dws_udp_set_dscp(0);
    for (uint8_t i = 0; i < MAX_CONNS; i++)
        conn_pool[i].pcb = nullptr;
}

void tearDown()
{
}

// DSCP -> DS field: DSCP occupies the high 6 bits, ECN the low 2 (left 0). TOS == DSCP << 2.
static void test_dscp_to_tos_encode()
{
    TEST_ASSERT_EQUAL_UINT8(0, dws_dscp_to_tos(DWS_DSCP_CS0));     // best-effort
    TEST_ASSERT_EQUAL_UINT8(0xB8, dws_dscp_to_tos(DWS_DSCP_EF));   // 46 << 2 = 0xB8
    TEST_ASSERT_EQUAL_UINT8(0xC0, dws_dscp_to_tos(DWS_DSCP_CS6));  // 48 << 2 = 0xC0
    TEST_ASSERT_EQUAL_UINT8(0x88, dws_dscp_to_tos(DWS_DSCP_AF41)); // 34 << 2 = 0x88
    TEST_ASSERT_EQUAL_UINT8(0xFC, dws_dscp_to_tos(63));            // max DSCP, ECN still 0
    TEST_ASSERT_EQUAL_UINT8(0x04, dws_dscp_to_tos(0x41));          // 0x41 masked to 6 bits -> 1
}

static void test_default_dscp_roundtrip()
{
    TEST_ASSERT_EQUAL_UINT8(0, dws_diffserv_default_dscp());
    dws_set_default_dscp(DWS_DSCP_EF);
    TEST_ASSERT_EQUAL_UINT8(46, dws_diffserv_default_dscp());
    dws_set_default_dscp(0xFF); // masked to the low 6 bits, not stored raw
    TEST_ASSERT_EQUAL_UINT8(63, dws_diffserv_default_dscp());
}

static void test_udp_dscp_roundtrip()
{
    TEST_ASSERT_EQUAL_UINT8(0, dws_diffserv_udp_dscp());
    dws_udp_set_dscp(DWS_DSCP_AF31);
    TEST_ASSERT_EQUAL_UINT8(26, dws_diffserv_udp_dscp());
    dws_udp_set_dscp(0);
    TEST_ASSERT_EQUAL_UINT8(0, dws_diffserv_udp_dscp());
}

static void test_conn_set_dscp_writes_pcb_tos()
{
    struct tcp_pcb pcb;
    pcb.tos = 0;
    conn_pool[0].pcb = &pcb;

    TEST_ASSERT_TRUE(dws_conn_set_dscp(0, DWS_DSCP_EF));
    TEST_ASSERT_EQUAL_UINT8(0xB8, pcb.tos); // EF stamped into the DS field

    TEST_ASSERT_TRUE(dws_conn_set_dscp(0, DWS_DSCP_CS0)); // live re-tag to best-effort
    TEST_ASSERT_EQUAL_UINT8(0, pcb.tos);
}

static void test_conn_set_dscp_rejects_bad_slot()
{
    conn_pool[0].pcb = nullptr;
    TEST_ASSERT_FALSE(dws_conn_set_dscp(0, DWS_DSCP_EF));   // no live pcb
    TEST_ASSERT_FALSE(dws_conn_set_dscp(255, DWS_DSCP_EF)); // slot out of range
}

static void test_listen_set_dscp_override_and_sentinel()
{
    TEST_ASSERT_EQUAL(1, listener_add(0, 8080, ConnProto::PROTO_HTTP));
    TEST_ASSERT_EQUAL_UINT8(DWS_DSCP_UNSET, listener_pool[0].dscp); // no override until set

    TEST_ASSERT_TRUE(dws_listen_set_dscp(8080, DWS_DSCP_EF));
    TEST_ASSERT_EQUAL_UINT8(46, listener_pool[0].dscp);

    TEST_ASSERT_TRUE(dws_listen_set_dscp(8080, 0x7E)); // wide value masked to 6 bits (0x3E = 62)
    TEST_ASSERT_EQUAL_UINT8(62, listener_pool[0].dscp);

    TEST_ASSERT_TRUE(dws_listen_set_dscp(8080, DWS_DSCP_UNSET)); // sentinel preserved, not masked to 63
    TEST_ASSERT_EQUAL_UINT8(DWS_DSCP_UNSET, listener_pool[0].dscp);

    TEST_ASSERT_FALSE(dws_listen_set_dscp(9999, DWS_DSCP_EF)); // no listener on that port
    listener_stop(0);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_dscp_to_tos_encode);
    RUN_TEST(test_default_dscp_roundtrip);
    RUN_TEST(test_udp_dscp_roundtrip);
    RUN_TEST(test_conn_set_dscp_writes_pcb_tos);
    RUN_TEST(test_conn_set_dscp_rejects_bad_slot);
    RUN_TEST(test_listen_set_dscp_override_and_sentinel);
    return UNITY_END();
}
