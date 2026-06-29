// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the accept-time connection gates (network_drivers/transport/listener):
// the global fixed-window accept throttle, the per-source-IP throttle bucket table
// (independent budgets, window rollover, the millis() wrap, and bounded eviction), and
// the CIDR source-IP allowlist. These three functions are always compiled so they can be
// host-tested; this env also compiles them with DETWS_ENABLE_ACCEPT_THROTTLE /
// PER_IP_THROTTLE / IP_ALLOWLIST set so the flag-guarded accept-callback paths build.
//
// The env overrides the budgets to small values so the boundaries are explicit:
//   ACCEPT_THROTTLE_MAX 3 / WINDOW 1000   PER_IP_MAX 2 / WINDOW 1000 / SLOTS 4   ALLOWLIST_SLOTS 4
// Pure host tests.

#include "network_drivers/transport/listener.h"
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// The global throttle allows up to MAX per window, then denies until the window rolls.
void test_accept_throttle_window()
{
    listener_accept_throttle_reset();
    TEST_ASSERT_TRUE(listener_accept_allowed(0));   // 1
    TEST_ASSERT_TRUE(listener_accept_allowed(10));  // 2
    TEST_ASSERT_TRUE(listener_accept_allowed(20));  // 3 == MAX
    TEST_ASSERT_FALSE(listener_accept_allowed(30)); // 4 over budget, same window
    // A timestamp a full window later opens a fresh budget.
    TEST_ASSERT_TRUE(listener_accept_allowed(1000));
    TEST_ASSERT_TRUE(listener_accept_allowed(1100));
}

// The fixed-window math uses unsigned subtraction, so it survives the millis() rollover.
void test_accept_throttle_rollover()
{
    listener_accept_throttle_reset();
    uint32_t base = 0xFFFFFE00u;                           // ~512 ticks before wrap
    TEST_ASSERT_TRUE(listener_accept_allowed(base));       // window starts here, 1
    TEST_ASSERT_TRUE(listener_accept_allowed(base + 100)); // 2
    TEST_ASSERT_TRUE(listener_accept_allowed(5));          // wrapped; elapsed ~517 < 1000, 3
    TEST_ASSERT_FALSE(listener_accept_allowed(10));        // over budget, still the same window
}

// Each source IP gets its own budget; one noisy client cannot exhaust another's.
void test_per_ip_independent_budgets()
{
    listener_per_ip_throttle_reset();
    uint32_t a = DETWS_IPV4(10, 0, 0, 1);
    uint32_t b = DETWS_IPV4(10, 0, 0, 2);
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(a, 0));  // a:1
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(a, 1));  // a:2 == MAX
    TEST_ASSERT_FALSE(listener_accept_allowed_ip(a, 2)); // a over budget
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(b, 2));  // b independent, fresh
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(b, 3));  // b:2
    TEST_ASSERT_FALSE(listener_accept_allowed_ip(b, 4)); // b over budget
}

// A per-IP bucket's window rolls over on its own clock.
void test_per_ip_window_rollover()
{
    listener_per_ip_throttle_reset();
    uint32_t a = DETWS_IPV4(192, 168, 1, 5);
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(a, 0));
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(a, 10));
    TEST_ASSERT_FALSE(listener_accept_allowed_ip(a, 20));  // budget used
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(a, 1000)); // window elapsed -> reset
}

// ip == 0 is the empty-bucket sentinel; such sources defer to the global throttle (always true here).
void test_per_ip_zero_defers()
{
    listener_per_ip_throttle_reset();
    for (uint32_t i = 0; i < 10; i++)
        TEST_ASSERT_TRUE(listener_accept_allowed_ip(0, i));
}

// More distinct addresses than buckets stays bounded: the oldest active bucket is evicted
// and the new address is still admitted (exercises the LRU-eviction branch).
void test_per_ip_eviction_bounded()
{
    listener_per_ip_throttle_reset();
    // Fill all 4 buckets at staggered start times, none yet expired at now=500.
    for (uint32_t i = 0; i < 4; i++)
        TEST_ASSERT_TRUE(listener_accept_allowed_ip(DETWS_IPV4(10, 0, 0, (uint8_t)(i + 1)), i * 100));
    // A 5th distinct address must still be admitted by evicting the least-recently-started.
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(DETWS_IPV4(10, 0, 0, 99), 500));
}

// An empty allowlist allows everything, so enabling the feature without rules never locks out.
void test_ip_allowlist_empty_allows_all()
{
    listener_ip_allowlist_reset();
    TEST_ASSERT_TRUE(listener_ip_allowed(DETWS_IPV4(8, 8, 8, 8)));
}

// A /24 rule matches its subnet only; host bits in the network argument are masked off.
void test_ip_allowlist_cidr()
{
    listener_ip_allowlist_reset();
    TEST_ASSERT_TRUE(listener_ip_allow_add(DETWS_IPV4(192, 168, 1, 0), 24));
    TEST_ASSERT_TRUE(listener_ip_allowed(DETWS_IPV4(192, 168, 1, 55)));
    TEST_ASSERT_FALSE(listener_ip_allowed(DETWS_IPV4(192, 168, 2, 55)));

    listener_ip_allowlist_reset();
    TEST_ASSERT_TRUE(listener_ip_allow_add(DETWS_IPV4(10, 1, 2, 3), 8)); // host bits masked -> 10.0.0.0/8
    TEST_ASSERT_TRUE(listener_ip_allowed(DETWS_IPV4(10, 255, 255, 255)));
    TEST_ASSERT_FALSE(listener_ip_allowed(DETWS_IPV4(11, 0, 0, 1)));
}

// /32 is a single host; /0 matches everything (the full-width-shift edge is handled apart).
void test_ip_allowlist_host_and_zero_prefix()
{
    listener_ip_allowlist_reset();
    TEST_ASSERT_TRUE(listener_ip_allow_add(DETWS_IPV4(203, 0, 113, 7), 32));
    TEST_ASSERT_TRUE(listener_ip_allowed(DETWS_IPV4(203, 0, 113, 7)));
    TEST_ASSERT_FALSE(listener_ip_allowed(DETWS_IPV4(203, 0, 113, 8)));

    listener_ip_allowlist_reset();
    TEST_ASSERT_TRUE(listener_ip_allow_add(0, 0)); // /0 -> mask 0 -> matches all
    TEST_ASSERT_TRUE(listener_ip_allowed(DETWS_IPV4(1, 2, 3, 4)));
}

// add() fails closed on an out-of-range prefix and on a full table.
void test_ip_allowlist_rejects_bad_and_full()
{
    listener_ip_allowlist_reset();
    TEST_ASSERT_FALSE(listener_ip_allow_add(DETWS_IPV4(1, 0, 0, 0), 33)); // prefix > 32
    for (int i = 0; i < 4; i++)                                           // SLOTS == 4
        TEST_ASSERT_TRUE(listener_ip_allow_add(DETWS_IPV4(10, 0, 0, (uint8_t)i), 32));
    TEST_ASSERT_FALSE(listener_ip_allow_add(DETWS_IPV4(10, 0, 0, 9), 32)); // table full
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_accept_throttle_window);
    RUN_TEST(test_accept_throttle_rollover);
    RUN_TEST(test_per_ip_independent_budgets);
    RUN_TEST(test_per_ip_window_rollover);
    RUN_TEST(test_per_ip_zero_defers);
    RUN_TEST(test_per_ip_eviction_bounded);
    RUN_TEST(test_ip_allowlist_empty_allows_all);
    RUN_TEST(test_ip_allowlist_cidr);
    RUN_TEST(test_ip_allowlist_host_and_zero_prefix);
    RUN_TEST(test_ip_allowlist_rejects_bad_and_full);
    return UNITY_END();
}
