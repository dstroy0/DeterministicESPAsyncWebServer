// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for egress-interface reporting (network_drivers/physical). The lwIP
// default-route + WiFi lookups are ESP32-only; the pure classifier that maps an
// egress IP against the WiFi station / softAP IPs is host-tested here.

#include "network_drivers/physical/physical.h"
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// Egress IP matching the station IP -> WiFi station.
void test_classify_sta()
{
    TEST_ASSERT_EQUAL_INT(DETIFACE_STA, det_net_classify_ip(0x0A000005u, 0x0A000005u, 0xC0A80401u));
}

// Egress IP matching the softAP IP -> softAP.
void test_classify_ap()
{
    TEST_ASSERT_EQUAL_INT(DETIFACE_AP, det_net_classify_ip(0xC0A80401u, 0x0A000005u, 0xC0A80401u));
}

// A live egress IP that is neither WiFi IP -> wired (Ethernet).
void test_classify_eth()
{
    TEST_ASSERT_EQUAL_INT(DETIFACE_ETH, det_net_classify_ip(0xC0A80105u, 0x0A000005u, 0));
    TEST_ASSERT_EQUAL_INT(DETIFACE_ETH, det_net_classify_ip(0xC0A80105u, 0, 0)); // ETH only, no WiFi
}

// No route -> ANY, regardless of the WiFi IPs.
void test_classify_none()
{
    TEST_ASSERT_EQUAL_INT(DETIFACE_ANY, det_net_classify_ip(0, 0x0A000005u, 0xC0A80401u));
}

// On a host build there is no default route, so egress reports ANY / 0.
void test_egress_host_stub()
{
    TEST_ASSERT_EQUAL_INT(DETIFACE_ANY, det_net_egress());
    TEST_ASSERT_EQUAL_UINT32(0, det_net_egress_ip());
}

// Ethernet bring-up is ESP32-only; on host (and when disabled) it reports not-ready.
void test_eth_host_stub()
{
    TEST_ASSERT_FALSE(init_eth_physical());
    TEST_ASSERT_FALSE(eth_ready());
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_classify_sta);
    RUN_TEST(test_classify_ap);
    RUN_TEST(test_classify_eth);
    RUN_TEST(test_classify_none);
    RUN_TEST(test_egress_host_stub);
    RUN_TEST(test_eth_host_stub);
    return UNITY_END();
}
