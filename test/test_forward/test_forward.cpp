// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the interface forwarding plane (services/forward): default-deny, an
// ALLOW rule forwards, a DENY wins, multi-destination fan-out, no reflection to the
// source, the per-rule rate cap (driven by the host test clock), send-failure counting,
// and the interface / rule table limits. Pure host tests. The DMA-driven wiring (DMA-
// complete -> FORWARD lane -> ingress -> egress DMA) is HW-verified separately.
//
// The env sizes DETWS_FWD_MAX_IFACES = 4, DETWS_FWD_MAX_RULES = 4.

#include "services/forward/det_forward.h"
#include <map>
#include <string.h>
#include <unity.h>
#include <vector>

struct Cap
{
    std::vector<std::vector<uint8_t>> frames;
    bool accept = true;
};
static std::map<uint8_t, Cap> g_cap; // capture, keyed by interface id

static bool cap_send(uint8_t id, const uint8_t *d, uint16_t n, void *)
{
    Cap &c = g_cap[id];
    if (!c.accept)
        return false; // simulate a full / failed interface
    c.frames.push_back(std::vector<uint8_t>(d, d + n));
    return true;
}

static bool add_if(uint8_t id)
{
    g_cap[id]; // ensure a capture entry exists (accept = true)
    return det_forward_add_if(id, DET_IF_OTHER, cap_send, nullptr);
}

static uint8_t ingress(uint8_t src, const char *s)
{
    return det_forward_ingress(src, (const uint8_t *)s, (uint16_t)strlen(s));
}

static det_forward_stats stats()
{
    det_forward_stats st;
    det_forward_get_stats(&st);
    return st;
}

void setUp()
{
    g_cap.clear();
    det_forward_reset();
    det_forward_test_set_now(0);
}
void tearDown()
{
    det_forward_reset();
}

void test_default_deny()
{
    TEST_ASSERT_TRUE(add_if(1));
    TEST_ASSERT_TRUE(add_if(2));
    TEST_ASSERT_EQUAL_UINT8(0, ingress(1, "hi")); // no rule -> nothing forwarded
    TEST_ASSERT_EQUAL_size_t(0, g_cap[2].frames.size());
    TEST_ASSERT_EQUAL_UINT32(1, stats().frames_in);
    TEST_ASSERT_EQUAL_UINT32(0, stats().forwarded);
}

void test_allow_forwards()
{
    add_if(1);
    add_if(2);
    TEST_ASSERT_TRUE(det_forward_add_rule(1, 2, DET_FWD_ALLOW, 0));
    TEST_ASSERT_EQUAL_UINT8(1, ingress(1, "abc"));
    TEST_ASSERT_EQUAL_size_t(1, g_cap[2].frames.size());
    TEST_ASSERT_EQUAL_size_t(0, g_cap[1].frames.size()); // source not touched
    TEST_ASSERT_EQUAL_MEMORY("abc", g_cap[2].frames[0].data(), 3);
    TEST_ASSERT_EQUAL_UINT32(1, stats().forwarded);
}

void test_no_self_forward()
{
    add_if(1);
    det_forward_add_rule(1, 1, DET_FWD_ALLOW, 0); // even an explicit self rule
    TEST_ASSERT_EQUAL_UINT8(0, ingress(1, "loop"));
    TEST_ASSERT_EQUAL_size_t(0, g_cap[1].frames.size());
}

void test_deny_wins_over_allow()
{
    add_if(1);
    add_if(2);
    det_forward_add_rule(1, 2, DET_FWD_ALLOW, 0);
    det_forward_add_rule(1, 2, DET_FWD_DENY, 0); // deny wins regardless of order
    TEST_ASSERT_EQUAL_UINT8(0, ingress(1, "x"));
    TEST_ASSERT_EQUAL_size_t(0, g_cap[2].frames.size());
    TEST_ASSERT_EQUAL_UINT32(1, stats().blocked);
}

void test_multi_destination_fanout()
{
    add_if(1);
    add_if(2);
    add_if(3);
    det_forward_add_rule(1, 2, DET_FWD_ALLOW, 0);
    det_forward_add_rule(1, 3, DET_FWD_ALLOW, 0);
    TEST_ASSERT_EQUAL_UINT8(2, ingress(1, "bcast"));
    TEST_ASSERT_EQUAL_size_t(1, g_cap[2].frames.size());
    TEST_ASSERT_EQUAL_size_t(1, g_cap[3].frames.size());
}

void test_rate_cap_drops_then_reopens()
{
    add_if(1);
    add_if(2);
    det_forward_add_rule(1, 2, DET_FWD_ALLOW, 2); // 2 frames / second
    TEST_ASSERT_EQUAL_UINT8(1, ingress(1, "a"));
    TEST_ASSERT_EQUAL_UINT8(1, ingress(1, "b"));
    TEST_ASSERT_EQUAL_UINT8(0, ingress(1, "c")); // 3rd in the window -> dropped
    TEST_ASSERT_EQUAL_size_t(2, g_cap[2].frames.size());
    TEST_ASSERT_EQUAL_UINT32(1, stats().rate_dropped);
    det_forward_test_set_now(1000); // next window
    TEST_ASSERT_EQUAL_UINT8(1, ingress(1, "d"));
    TEST_ASSERT_EQUAL_size_t(3, g_cap[2].frames.size());
}

void test_send_failure_counted()
{
    add_if(1);
    add_if(2);
    det_forward_add_rule(1, 2, DET_FWD_ALLOW, 0);
    g_cap[2].accept = false; // interface refuses
    TEST_ASSERT_EQUAL_UINT8(0, ingress(1, "x"));
    TEST_ASSERT_EQUAL_UINT32(1, stats().send_fail);
    TEST_ASSERT_EQUAL_UINT32(0, stats().forwarded);
}

void test_add_if_validation_and_table_full()
{
    TEST_ASSERT_TRUE(add_if(1));
    TEST_ASSERT_FALSE(add_if(1));                                             // duplicate id
    TEST_ASSERT_FALSE(det_forward_add_if(9, DET_IF_OTHER, nullptr, nullptr)); // null send
    TEST_ASSERT_TRUE(add_if(2));
    TEST_ASSERT_TRUE(add_if(3));
    TEST_ASSERT_TRUE(add_if(4));
    TEST_ASSERT_FALSE(add_if(5)); // table full (DETWS_FWD_MAX_IFACES = 4)
}

void test_add_rule_table_full()
{
    for (int i = 0; i < DETWS_FWD_MAX_RULES; i++)
        TEST_ASSERT_TRUE(det_forward_add_rule(1, 2, DET_FWD_ALLOW, 0));
    TEST_ASSERT_FALSE(det_forward_add_rule(1, 3, DET_FWD_ALLOW, 0)); // full
}

void test_unregistered_destination_is_inert()
{
    add_if(1);
    det_forward_add_rule(1, 9, DET_FWD_ALLOW, 0); // dst 9 never registered
    TEST_ASSERT_EQUAL_UINT8(0, ingress(1, "x"));  // nothing to forward to
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_default_deny);
    RUN_TEST(test_allow_forwards);
    RUN_TEST(test_no_self_forward);
    RUN_TEST(test_deny_wins_over_allow);
    RUN_TEST(test_multi_destination_fanout);
    RUN_TEST(test_rate_cap_drops_then_reopens);
    RUN_TEST(test_send_failure_counted);
    RUN_TEST(test_add_if_validation_and_table_full);
    RUN_TEST(test_add_rule_table_full);
    RUN_TEST(test_unregistered_destination_is_inert);
    return UNITY_END();
}
