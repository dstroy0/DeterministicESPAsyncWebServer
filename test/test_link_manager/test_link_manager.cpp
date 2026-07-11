// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/link_manager: egress selection, graceful escalation, failover.

#include "services/link_manager/link_manager.h"
#include <unity.h>

// Eth (prio 20), WiFi STA (prio 10), softAP (prio 5).
static LinkIface g_ifaces[3];
static LinkManager g_m;

void setUp(void)
{
    g_ifaces[0] = {LinkKind::LINK_KIND_ETH, 20, false};
    g_ifaces[1] = {LinkKind::LINK_KIND_WIFI_STA, 10, false};
    g_ifaces[2] = {LinkKind::LINK_KIND_WIFI_AP, 5, false};
    detws_link_init(&g_m, g_ifaces, 3);
}
void tearDown(void)
{
}

void test_init_none_up(void)
{
    TEST_ASSERT_EQUAL_INT(-1, detws_link_active(&g_m));
}

void test_escalation_and_failover(void)
{
    int from = 0, to = 0;
    // WiFi STA comes up first -> it becomes active.
    TEST_ASSERT_TRUE(detws_link_set(&g_m, 1, true, &from, &to));
    TEST_ASSERT_EQUAL_INT(-1, from);
    TEST_ASSERT_EQUAL_INT(1, to);
    // Ethernet comes up (higher priority) -> escalate to it.
    TEST_ASSERT_TRUE(detws_link_set(&g_m, 0, true, &from, &to));
    TEST_ASSERT_EQUAL_INT(1, from);
    TEST_ASSERT_EQUAL_INT(0, to);
    // softAP comes up (lower priority) -> no change.
    TEST_ASSERT_FALSE(detws_link_set(&g_m, 2, true, &from, &to));
    TEST_ASSERT_EQUAL_INT(0, detws_link_active(&g_m));
    // Ethernet drops -> fail over to the next best up (WiFi STA).
    TEST_ASSERT_TRUE(detws_link_set(&g_m, 0, false, &from, &to));
    TEST_ASSERT_EQUAL_INT(0, from);
    TEST_ASSERT_EQUAL_INT(1, to);
    // WiFi STA drops too -> fail over to softAP.
    TEST_ASSERT_TRUE(detws_link_set(&g_m, 1, false, &from, &to));
    TEST_ASSERT_EQUAL_INT(2, to);
    // softAP drops -> nothing up.
    TEST_ASSERT_TRUE(detws_link_set(&g_m, 2, false, &from, &to));
    TEST_ASSERT_EQUAL_INT(-1, detws_link_active(&g_m));
}

void test_tie_break_lower_index(void)
{
    // Two interfaces at equal priority: the lower index wins.
    LinkIface pair[2] = {{LinkKind::LINK_KIND_ETH, 10, true}, {LinkKind::LINK_KIND_WIFI_STA, 10, true}};
    LinkManager m;
    detws_link_init(&m, pair, 2);
    TEST_ASSERT_EQUAL_INT(0, detws_link_active(&m));
}

void test_out_of_range_no_change(void)
{
    detws_link_set(&g_m, 1, true, nullptr, nullptr);
    int from = 5, to = 5;
    TEST_ASSERT_FALSE(detws_link_set(&g_m, 9, true, &from, &to));
    TEST_ASSERT_EQUAL_INT(1, from);
    TEST_ASSERT_EQUAL_INT(1, to);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_init_none_up);
    RUN_TEST(test_escalation_and_failover);
    RUN_TEST(test_tie_break_lower_index);
    RUN_TEST(test_out_of_range_no_change);
    return UNITY_END();
}
