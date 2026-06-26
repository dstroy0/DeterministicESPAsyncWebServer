// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the per-IP brute-force auth lockout (services/auth_lockout).
// The state machine takes the millisecond clock as a parameter, so the host
// drives a synthetic clock - no real time, no ESP32. Uses the default sizing
// (THRESHOLD=5, BASE=1000 ms, MAX=300000 ms) from DetWebServerConfig.h.

#include "services/auth_lockout/auth_lockout.h"
#include <unity.h>

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
    const uint32_t ip = 0x0A000001u;
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD - 1; i++)
        auth_lockout_fail(ip, 0);
    TEST_ASSERT_EQUAL_UINT32(0, auth_lockout_remaining_ms(ip, 0));
}

// The threshold-th consecutive failure locks the address for the base duration.
void test_locks_at_threshold()
{
    auth_lockout_reset();
    const uint32_t ip = 0x0A000002u;
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD; i++)
        auth_lockout_fail(ip, 0);
    TEST_ASSERT_EQUAL_UINT32(DETWS_AUTH_LOCKOUT_BASE_MS, auth_lockout_remaining_ms(ip, 0));
}

// Each failure past the threshold doubles the lockout duration.
void test_exponential_backoff()
{
    auth_lockout_reset();
    const uint32_t ip = 0x0A000003u;
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD; i++)
        auth_lockout_fail(ip, 0);
    TEST_ASSERT_EQUAL_UINT32(DETWS_AUTH_LOCKOUT_BASE_MS, auth_lockout_remaining_ms(ip, 0));
    auth_lockout_fail(ip, 0);
    TEST_ASSERT_EQUAL_UINT32(2u * DETWS_AUTH_LOCKOUT_BASE_MS, auth_lockout_remaining_ms(ip, 0));
    auth_lockout_fail(ip, 0);
    TEST_ASSERT_EQUAL_UINT32(4u * DETWS_AUTH_LOCKOUT_BASE_MS, auth_lockout_remaining_ms(ip, 0));
}

// The backoff is capped at the configured maximum.
void test_caps_at_max()
{
    auth_lockout_reset();
    const uint32_t ip = 0x0A000004u;
    for (int i = 0; i < 40; i++)
        auth_lockout_fail(ip, 0);
    TEST_ASSERT_EQUAL_UINT32(DETWS_AUTH_LOCKOUT_MAX_MS, auth_lockout_remaining_ms(ip, 0));
}

// The lockout counts down and expires exactly at the window end (rollover-safe math).
void test_expires_after_window()
{
    auth_lockout_reset();
    const uint32_t ip = 0x0A000005u;
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD; i++)
        auth_lockout_fail(ip, 0);
    TEST_ASSERT_EQUAL_UINT32(1, auth_lockout_remaining_ms(ip, DETWS_AUTH_LOCKOUT_BASE_MS - 1));
    TEST_ASSERT_EQUAL_UINT32(0, auth_lockout_remaining_ms(ip, DETWS_AUTH_LOCKOUT_BASE_MS));
    TEST_ASSERT_EQUAL_UINT32(0, auth_lockout_remaining_ms(ip, DETWS_AUTH_LOCKOUT_BASE_MS + 100));
}

// A successful auth clears the failure count and lockout for that address.
void test_success_clears()
{
    auth_lockout_reset();
    const uint32_t ip = 0x0A000006u;
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD; i++)
        auth_lockout_fail(ip, 0);
    TEST_ASSERT_TRUE(auth_lockout_remaining_ms(ip, 0) > 0);
    auth_lockout_succeed(ip);
    TEST_ASSERT_EQUAL_UINT32(0, auth_lockout_remaining_ms(ip, 0));
    // The counter was reset: one short of the threshold is still not locked.
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD - 1; i++)
        auth_lockout_fail(ip, 0);
    TEST_ASSERT_EQUAL_UINT32(0, auth_lockout_remaining_ms(ip, 0));
}

// Locking one address does not affect a different one.
void test_isolates_addresses()
{
    auth_lockout_reset();
    const uint32_t a = 0x0A000007u, b = 0x0A000008u;
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD; i++)
        auth_lockout_fail(a, 0);
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD - 1; i++)
        auth_lockout_fail(b, 0);
    TEST_ASSERT_TRUE(auth_lockout_remaining_ms(a, 0) > 0);
    TEST_ASSERT_EQUAL_UINT32(0, auth_lockout_remaining_ms(b, 0));
}

// A zero source address is untrackable and is never locked.
void test_zero_ip_never_locked()
{
    auth_lockout_reset();
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD + 10; i++)
        auth_lockout_fail(0, 0);
    TEST_ASSERT_EQUAL_UINT32(0, auth_lockout_remaining_ms(0, 0));
}

// When the table is full of idle addresses, a new address is still tracked
// (an idle bucket is evicted) and can itself be locked out.
void test_table_full_tracks_new_address()
{
    auth_lockout_reset();
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_SLOTS; i++)
        auth_lockout_fail(0xC0000000u + (uint32_t)i, 0);
    const uint32_t fresh = 0xD0000000u;
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD; i++)
        auth_lockout_fail(fresh, 0);
    TEST_ASSERT_TRUE(auth_lockout_remaining_ms(fresh, 0) > 0);
}

// An active lockout is not evicted by eviction pressure from other addresses
// (the table evicts unlocked buckets first), so an attacker cannot clear their
// own lockout by flooding from other source addresses.
void test_active_lockout_survives_eviction()
{
    auth_lockout_reset();
    const uint32_t victim = 0x0A0000FFu;
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_THRESHOLD; i++)
        auth_lockout_fail(victim, 0);
    TEST_ASSERT_TRUE(auth_lockout_remaining_ms(victim, 0) > 0);
    for (uint32_t i = 1; i <= (uint32_t)DETWS_AUTH_LOCKOUT_SLOTS * 2u; i++)
        auth_lockout_fail(0xB0000000u + i, 1);
    TEST_ASSERT_TRUE(auth_lockout_remaining_ms(victim, 1) > 0);
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
    RUN_TEST(test_zero_ip_never_locked);
    RUN_TEST(test_table_full_tracks_new_address);
    RUN_TEST(test_active_lockout_survives_eviction);
    return UNITY_END();
}
