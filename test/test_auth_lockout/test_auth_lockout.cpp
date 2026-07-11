// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the per-peer brute-force auth lockout (services/auth_lockout).
// Every bucket keys on the FULL family-tagged address (DetIp) - never a hash or a
// uint32 flattening - so IPv4 and IPv6 peers are distinct buckets and no address can
// share or poison another's lockout. The state machine takes the millisecond clock as
// a parameter, so the host drives a synthetic clock - no real time, no ESP32. Uses the
// default sizing (THRESHOLD=5, BASE=1000 ms, MAX=300000 ms) from ServerConfig.h.

#include "network_drivers/network/ip.h"
#include "services/auth_lockout/auth_lockout.h"
#include <unity.h>

// Build a v4 DetIp from a host-order word (0x0A000001 -> 10.0.0.1).
static DetIp v4w(uint32_t host_order)
{
    return det_ip_from_v4_octets((uint8_t)(host_order >> 24), (uint8_t)(host_order >> 16), (uint8_t)(host_order >> 8),
                                 (uint8_t)host_order);
}

void setUp()
{
}
void tearDown()
{
}

// One short of the threshold leaves the address unlocked.
void test_below_threshold_not_locked()
{
    auth_lockout_reset();
    DetIp ip = v4w(0x0A000001u);
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD - 1; i++)
        auth_lockout_fail(&ip, 0);
    TEST_ASSERT_EQUAL_UINT32(0, auth_lockout_remaining_ms(&ip, 0));
}

// The threshold-th consecutive failure locks the address for the base duration.
void test_locks_at_threshold()
{
    auth_lockout_reset();
    DetIp ip = v4w(0x0A000002u);
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD; i++)
        auth_lockout_fail(&ip, 0);
    TEST_ASSERT_EQUAL_UINT32(DETWS_AUTH_LOCKOUT_BASE_MS, auth_lockout_remaining_ms(&ip, 0));
}

// Each failure past the threshold doubles the lockout duration.
void test_exponential_backoff()
{
    auth_lockout_reset();
    DetIp ip = v4w(0x0A000003u);
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD; i++)
        auth_lockout_fail(&ip, 0);
    TEST_ASSERT_EQUAL_UINT32(DETWS_AUTH_LOCKOUT_BASE_MS, auth_lockout_remaining_ms(&ip, 0));
    auth_lockout_fail(&ip, 0);
    TEST_ASSERT_EQUAL_UINT32(2u * DETWS_AUTH_LOCKOUT_BASE_MS, auth_lockout_remaining_ms(&ip, 0));
    auth_lockout_fail(&ip, 0);
    TEST_ASSERT_EQUAL_UINT32(4u * DETWS_AUTH_LOCKOUT_BASE_MS, auth_lockout_remaining_ms(&ip, 0));
}

// The backoff is capped at the configured maximum.
void test_caps_at_max()
{
    auth_lockout_reset();
    DetIp ip = v4w(0x0A000004u);
    for (int i = 0; i < 40; i++)
        auth_lockout_fail(&ip, 0);
    TEST_ASSERT_EQUAL_UINT32(DETWS_AUTH_LOCKOUT_MAX_MS, auth_lockout_remaining_ms(&ip, 0));
}

// The lockout counts down and expires exactly at the window end (rollover-safe math).
void test_expires_after_window()
{
    auth_lockout_reset();
    DetIp ip = v4w(0x0A000005u);
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD; i++)
        auth_lockout_fail(&ip, 0);
    TEST_ASSERT_EQUAL_UINT32(1, auth_lockout_remaining_ms(&ip, DETWS_AUTH_LOCKOUT_BASE_MS - 1));
    TEST_ASSERT_EQUAL_UINT32(0, auth_lockout_remaining_ms(&ip, DETWS_AUTH_LOCKOUT_BASE_MS));
    TEST_ASSERT_EQUAL_UINT32(0, auth_lockout_remaining_ms(&ip, DETWS_AUTH_LOCKOUT_BASE_MS + 100));
}

// A successful auth clears the failure count and lockout for that address.
void test_success_clears()
{
    auth_lockout_reset();
    DetIp ip = v4w(0x0A000006u);
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD; i++)
        auth_lockout_fail(&ip, 0);
    TEST_ASSERT_TRUE(auth_lockout_remaining_ms(&ip, 0) > 0);
    auth_lockout_succeed(&ip);
    TEST_ASSERT_EQUAL_UINT32(0, auth_lockout_remaining_ms(&ip, 0));
    // The counter was reset: one short of the threshold is still not locked.
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD - 1; i++)
        auth_lockout_fail(&ip, 0);
    TEST_ASSERT_EQUAL_UINT32(0, auth_lockout_remaining_ms(&ip, 0));
}

// Locking one address does not affect a different one.
void test_isolates_addresses()
{
    auth_lockout_reset();
    DetIp a = v4w(0x0A000007u), b = v4w(0x0A000008u);
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD; i++)
        auth_lockout_fail(&a, 0);
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD - 1; i++)
        auth_lockout_fail(&b, 0);
    TEST_ASSERT_TRUE(auth_lockout_remaining_ms(&a, 0) > 0);
    TEST_ASSERT_EQUAL_UINT32(0, auth_lockout_remaining_ms(&b, 0));
}

// Distinct IPv6 peers are distinct buckets, and a v6 peer never shares a v4 peer's
// lockout (the whole point of keying on the full address instead of a hash).
void test_v6_distinct_from_v4_and_each_other()
{
    auth_lockout_reset();
    DetIp v6a;
    v6a.family = DetIpFamily::DET_IP_NONE;
    DetIp v6b;
    v6b.family = DetIpFamily::DET_IP_NONE;
    TEST_ASSERT_TRUE(det_ip_parse("2001:db8::1", &v6a));
    TEST_ASSERT_TRUE(det_ip_parse("2001:db8::2", &v6b));
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD; i++)
        auth_lockout_fail(&v6a, 0);
    TEST_ASSERT_TRUE(auth_lockout_remaining_ms(&v6a, 0) > 0);        // v6a locked
    TEST_ASSERT_EQUAL_UINT32(0, auth_lockout_remaining_ms(&v6b, 0)); // a different v6 peer is not

    // A v4 host that "looks like" the v6 tail must not inherit the lockout.
    DetIp v4 = v4w(0x00000001u); // 0.0.0.1 - distinct family, distinct bucket
    TEST_ASSERT_EQUAL_UINT32(0, auth_lockout_remaining_ms(&v4, 0));
}

// An unspecified source address is untrackable and is never locked.
void test_zero_ip_never_locked()
{
    auth_lockout_reset();
    DetIp none;
    none.family = DetIpFamily::DET_IP_NONE;
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD + 10; i++)
        auth_lockout_fail(&none, 0);
    TEST_ASSERT_EQUAL_UINT32(0, auth_lockout_remaining_ms(&none, 0));
}

// When the table is full of idle addresses, a new address is still tracked
// (an idle bucket is evicted) and can itself be locked out.
void test_table_full_tracks_new_address()
{
    auth_lockout_reset();
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_SLOTS; i++)
    {
        DetIp ip = v4w(0xC0000000u + (uint32_t)i);
        auth_lockout_fail(&ip, 0);
    }
    DetIp fresh = v4w(0xD0000000u);
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD; i++)
        auth_lockout_fail(&fresh, 0);
    TEST_ASSERT_TRUE(auth_lockout_remaining_ms(&fresh, 0) > 0);
}

// An active lockout is not evicted by eviction pressure from other addresses
// (the table evicts unlocked buckets first), so an attacker cannot clear their
// own lockout by flooding from other source addresses.
void test_active_lockout_survives_eviction()
{
    auth_lockout_reset();
    DetIp victim = v4w(0x0A0000FFu);
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD; i++)
        auth_lockout_fail(&victim, 0);
    TEST_ASSERT_TRUE(auth_lockout_remaining_ms(&victim, 0) > 0);
    for (uint32_t i = 1; i <= (uint32_t)DETWS_AUTH_LOCKOUT_SLOTS * 2u; i++)
    {
        DetIp other = v4w(0xB0000000u + i);
        auth_lockout_fail(&other, 1);
    }
    TEST_ASSERT_TRUE(auth_lockout_remaining_ms(&victim, 1) > 0);
}

void test_succeed_unspecified_and_table_full_eviction()
{
    auth_lockout_reset();
    DetIp none = {};             // unspecified (family DetIpFamily::DET_IP_NONE)
    auth_lockout_succeed(&none); // no-op guard on an unspecified address
    // Fill every bucket with an active lockout.
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_SLOTS; i++)
    {
        DetIp ip = v4w(0x0B000000u + (uint32_t)i);
        uint32_t t = 100000u - (uint32_t)i; // decreasing so a later-indexed bucket is more stale (LRU)
        for (int f = 0; f < DETWS_AUTH_LOCKOUT_THRESHOLD; f++)
            auth_lockout_fail(&ip, t);
    }
    // A new distinct IP: the table is full of active lockouts, so allocation tracks LRU and evicts.
    DetIp extra = v4w(0x0C000001u);
    for (int f = 0; f < DETWS_AUTH_LOCKOUT_THRESHOLD; f++)
        auth_lockout_fail(&extra, 100000u); // full table -> allocation tracks LRU and evicts the stalest bucket
    TEST_ASSERT_TRUE(auth_lockout_remaining_ms(&extra, 100000u) > 0);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_below_threshold_not_locked);
    RUN_TEST(test_locks_at_threshold);
    RUN_TEST(test_exponential_backoff);
    RUN_TEST(test_caps_at_max);
    RUN_TEST(test_expires_after_window);
    RUN_TEST(test_success_clears);
    RUN_TEST(test_isolates_addresses);
    RUN_TEST(test_v6_distinct_from_v4_and_each_other);
    RUN_TEST(test_zero_ip_never_locked);
    RUN_TEST(test_table_full_tracks_new_address);
    RUN_TEST(test_active_lockout_survives_eviction);
    RUN_TEST(test_succeed_unspecified_and_table_full_eviction);
    return UNITY_END();
}
