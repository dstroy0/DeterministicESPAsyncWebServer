// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the accept-time connection gates (network_drivers/transport/listener):
// the global fixed-window accept throttle, the per-source-IP throttle bucket table
// (independent budgets, window rollover, the millis() wrap, and bounded eviction), and
// the CIDR source-IP allowlist. Every address-keyed gate keys on the FULL family-tagged
// address (DetIp) - never a hash or a uint32 flattening - so IPv4 and IPv6 peers are
// distinct buckets and a v6 peer cannot spray or collide its way past a per-address cap.
// These functions are always compiled so they can be host-tested; this env also compiles
// them with DETWS_ENABLE_ACCEPT_THROTTLE / PER_IP_THROTTLE / IP_ALLOWLIST set so the
// flag-guarded accept-callback paths build.
//
// The env overrides the budgets to small values so the boundaries are explicit:
//   ACCEPT_THROTTLE_MAX 3 / WINDOW 1000   PER_IP_MAX 2 / WINDOW 1000 / SLOTS 4   ALLOWLIST_SLOTS 4
// Pure host tests.

#include "network_drivers/network/ip.h"
#include "network_drivers/transport/listener.h"
#include <unity.h>

// Small builders so the tests read in terms of addresses, not byte plumbing.
static DetIp v4(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    return det_ip_from_v4_octets(a, b, c, d);
}
static DetIp v6(const char *s)
{
    DetIp ip;
    ip.family = DET_IP_NONE;
    det_ip_parse(s, &ip);
    return ip;
}

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
    DetIp a = v4(10, 0, 0, 1);
    DetIp b = v4(10, 0, 0, 2);
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(&a, 0));  // a:1
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(&a, 1));  // a:2 == MAX
    TEST_ASSERT_FALSE(listener_accept_allowed_ip(&a, 2)); // a over budget
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(&b, 2));  // b independent, fresh
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(&b, 3));  // b:2
    TEST_ASSERT_FALSE(listener_accept_allowed_ip(&b, 4)); // b over budget
}

// Distinct IPv6 peers are distinct buckets (no hash collapse); a v4 and a v6 never share one.
void test_per_ip_v6_distinct_buckets()
{
    listener_per_ip_throttle_reset();
    DetIp a = v6("2001:db8::1");
    DetIp b = v6("2001:db8::2");
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(&a, 0));  // a:1
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(&a, 1));  // a:2 == MAX
    TEST_ASSERT_FALSE(listener_accept_allowed_ip(&a, 2)); // a over budget
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(&b, 2));  // b:1 - a different v6 peer, own budget
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(&b, 3));  // b:2
    TEST_ASSERT_FALSE(listener_accept_allowed_ip(&b, 4)); // b over budget
}

// A per-IP bucket's window rolls over on its own clock.
void test_per_ip_window_rollover()
{
    listener_per_ip_throttle_reset();
    DetIp a = v4(192, 168, 1, 5);
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(&a, 0));
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(&a, 10));
    TEST_ASSERT_FALSE(listener_accept_allowed_ip(&a, 20));  // budget used
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(&a, 1000)); // window elapsed -> reset
}

// An unspecified address is untrackable; such sources defer to the global throttle (always true here).
void test_per_ip_unspecified_defers()
{
    listener_per_ip_throttle_reset();
    DetIp none;
    none.family = DET_IP_NONE;
    for (uint32_t i = 0; i < 10; i++)
        TEST_ASSERT_TRUE(listener_accept_allowed_ip(&none, i));
}

// More distinct addresses than buckets stays bounded: the oldest active bucket is evicted
// and the new address is still admitted (exercises the LRU-eviction branch).
void test_per_ip_eviction_bounded()
{
    listener_per_ip_throttle_reset();
    // Fill all 4 buckets at staggered start times, none yet expired at now=500.
    for (uint32_t i = 0; i < 4; i++)
    {
        DetIp ip = v4(10, 0, 0, (uint8_t)(i + 1));
        TEST_ASSERT_TRUE(listener_accept_allowed_ip(&ip, i * 100));
    }
    // A 5th distinct address must still be admitted by evicting the least-recently-started.
    DetIp fresh = v4(10, 0, 0, 99);
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(&fresh, 500));
}

// An empty allowlist allows everything, so enabling the feature without rules never locks out.
void test_ip_allowlist_empty_allows_all()
{
    listener_ip_allowlist_reset();
    DetIp any = v4(8, 8, 8, 8);
    TEST_ASSERT_TRUE(listener_ip_allowed(&any));
}

// A /24 rule matches its subnet only; host bits in the network argument are masked at compare time.
void test_ip_allowlist_cidr()
{
    listener_ip_allowlist_reset();
    DetIp net = v4(192, 168, 1, 0);
    TEST_ASSERT_TRUE(listener_ip_allow_add(&net, 24));
    DetIp in = v4(192, 168, 1, 55);
    DetIp out = v4(192, 168, 2, 55);
    TEST_ASSERT_TRUE(listener_ip_allowed(&in));
    TEST_ASSERT_FALSE(listener_ip_allowed(&out));

    listener_ip_allowlist_reset();
    DetIp net8 = v4(10, 1, 2, 3); // host bits masked -> 10.0.0.0/8
    TEST_ASSERT_TRUE(listener_ip_allow_add(&net8, 8));
    DetIp in8 = v4(10, 255, 255, 255);
    DetIp out8 = v4(11, 0, 0, 1);
    TEST_ASSERT_TRUE(listener_ip_allowed(&in8));
    TEST_ASSERT_FALSE(listener_ip_allowed(&out8));
}

// The CIDR-string public entry point parses v4 and v6, bare hosts, and rejects garbage.
void test_ip_allowlist_cidr_string()
{
    listener_ip_allowlist_reset();
    TEST_ASSERT_TRUE(listener_ip_allow_add_cidr("192.168.1.0/24"));
    TEST_ASSERT_TRUE(listener_ip_allow_add_cidr("2001:db8::/32"));
    TEST_ASSERT_TRUE(listener_ip_allow_add_cidr("10.0.0.5")); // bare host -> /32

    DetIp v4in = v4(192, 168, 1, 200);
    DetIp v4host = v4(10, 0, 0, 5);
    DetIp v4no = v4(10, 0, 0, 6);
    DetIp v6in = v6("2001:db8:0:0:1234::abcd");
    DetIp v6no = v6("2001:db9::1");
    TEST_ASSERT_TRUE(listener_ip_allowed(&v4in));
    TEST_ASSERT_TRUE(listener_ip_allowed(&v4host));
    TEST_ASSERT_FALSE(listener_ip_allowed(&v4no));
    TEST_ASSERT_TRUE(listener_ip_allowed(&v6in));
    TEST_ASSERT_FALSE(listener_ip_allowed(&v6no)); // v6 peer outside every v6 rule (and v4 rules never match)

    // Malformed input fails closed.
    TEST_ASSERT_FALSE(listener_ip_allow_add_cidr("not-an-ip"));
    TEST_ASSERT_FALSE(listener_ip_allow_add_cidr("192.168.1.0/33")); // prefix > 32
    TEST_ASSERT_FALSE(listener_ip_allow_add_cidr("2001:db8::/129")); // prefix > 128
    TEST_ASSERT_FALSE(listener_ip_allow_add_cidr("192.168.1.0/"));   // empty prefix
}

// A v4 allowlist rule must never admit a v6 peer (and vice versa) - families are isolated.
void test_ip_allowlist_family_isolation()
{
    listener_ip_allowlist_reset();
    DetIp v4net = v4(192, 168, 1, 0);
    TEST_ASSERT_TRUE(listener_ip_allow_add(&v4net, 24));
    DetIp v6peer = v6("2001:db8::1");
    TEST_ASSERT_FALSE(listener_ip_allowed(&v6peer)); // rules exist but none match this family
}

// /32 is a single host; /0 matches everything (the full-width-shift edge is handled apart).
void test_ip_allowlist_host_and_zero_prefix()
{
    listener_ip_allowlist_reset();
    DetIp host = v4(203, 0, 113, 7);
    TEST_ASSERT_TRUE(listener_ip_allow_add(&host, 32));
    DetIp other = v4(203, 0, 113, 8);
    TEST_ASSERT_TRUE(listener_ip_allowed(&host));
    TEST_ASSERT_FALSE(listener_ip_allowed(&other));

    listener_ip_allowlist_reset();
    DetIp z = v4(0, 0, 0, 0);
    TEST_ASSERT_TRUE(listener_ip_allow_add(&z, 0)); // /0 -> matches all v4
    DetIp anyone = v4(1, 2, 3, 4);
    TEST_ASSERT_TRUE(listener_ip_allowed(&anyone));
}

// add() fails closed on an out-of-range prefix and on a full table.
void test_ip_allowlist_rejects_bad_and_full()
{
    listener_ip_allowlist_reset();
    DetIp bad = v4(1, 0, 0, 0);
    TEST_ASSERT_FALSE(listener_ip_allow_add(&bad, 33)); // prefix > 32
    for (int i = 0; i < 4; i++)                         // SLOTS == 4
    {
        DetIp r = v4(10, 0, 0, (uint8_t)i);
        TEST_ASSERT_TRUE(listener_ip_allow_add(&r, 32));
    }
    DetIp overflow = v4(10, 0, 0, 9);
    TEST_ASSERT_FALSE(listener_ip_allow_add(&overflow, 32)); // table full
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_accept_throttle_window);
    RUN_TEST(test_accept_throttle_rollover);
    RUN_TEST(test_per_ip_independent_budgets);
    RUN_TEST(test_per_ip_v6_distinct_buckets);
    RUN_TEST(test_per_ip_window_rollover);
    RUN_TEST(test_per_ip_unspecified_defers);
    RUN_TEST(test_per_ip_eviction_bounded);
    RUN_TEST(test_ip_allowlist_empty_allows_all);
    RUN_TEST(test_ip_allowlist_cidr);
    RUN_TEST(test_ip_allowlist_cidr_string);
    RUN_TEST(test_ip_allowlist_family_isolation);
    RUN_TEST(test_ip_allowlist_host_and_zero_prefix);
    RUN_TEST(test_ip_allowlist_rejects_bad_and_full);
    return UNITY_END();
}
