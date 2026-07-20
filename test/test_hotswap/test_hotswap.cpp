// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the removable-storage state machine (services/hotswap): the fault threshold and
// what resets it, probe pacing (including across a millis() rollover), present-but-unmountable,
// the removal/reinsertion cycle, and that the whole thing is fail-closed.

#include "services/hotswap/hotswap.h"
#include <stdio.h>
#include <string.h>
#include <unity.h>

static HotswapCore c;

void setUp()
{
    dws_hotswap_core_init(&c, 3, 2000, 100000);
}
void tearDown()
{
}

// Bring the volume up so a test can start from a healthy mount.
static void mount_it(uint32_t now)
{
    dws_hotswap_core_probe(&c, true, true, now);
}

// --- initial state --------------------------------------------------------

void test_starts_absent_not_ready()
{
    // Starting READY would let a caller write before anything was ever mounted.
    TEST_ASSERT_EQUAL_INT((int)StorageState::ABSENT, (int)c.state);
    TEST_ASSERT_EQUAL_UINT32(0, c.mounts);
    TEST_ASSERT_EQUAL_UINT32(0, c.faults);
}

void test_first_probe_is_due_immediately()
{
    // Back-dated last_probe: a card already present at boot must mount now, not one interval later.
    TEST_ASSERT_TRUE(dws_hotswap_core_due(&c, 100000));
}

void test_first_probe_is_due_when_init_time_is_near_zero()
{
    // Real case: begin() runs a few ms after boot, so `now - probe_interval` underflows past zero.
    // Unsigned arithmetic has to carry that correctly or a device would refuse to mount its card
    // for the first ~49 days of uptime.
    dws_hotswap_core_init(&c, 3, 2000, 5);
    TEST_ASSERT_TRUE(dws_hotswap_core_due(&c, 5));
    TEST_ASSERT_TRUE(dws_hotswap_core_due(&c, 6));
}

void test_zero_threshold_is_clamped_to_one()
{
    dws_hotswap_core_init(&c, 0, 2000, 0);
    TEST_ASSERT_EQUAL_UINT8(1, c.fail_threshold);
    mount_it(0);
    // With an unclamped 0 the volume would fault before any failure was reported.
    TEST_ASSERT_TRUE(dws_hotswap_core_io(&c, false));
    TEST_ASSERT_EQUAL_INT((int)StorageState::FAULTED, (int)c.state);
}

// --- the fault threshold --------------------------------------------------

void test_one_failure_does_not_fault_a_healthy_volume()
{
    mount_it(100000);
    TEST_ASSERT_FALSE(dws_hotswap_core_io(&c, false));
    TEST_ASSERT_EQUAL_INT((int)StorageState::READY, (int)c.state);
    TEST_ASSERT_EQUAL_UINT8(1, c.fail_run);
}

void test_threshold_run_faults_and_counts()
{
    mount_it(100000);
    TEST_ASSERT_FALSE(dws_hotswap_core_io(&c, false));
    TEST_ASSERT_FALSE(dws_hotswap_core_io(&c, false));
    TEST_ASSERT_TRUE(dws_hotswap_core_io(&c, false)); // 3rd == threshold -> state change
    TEST_ASSERT_EQUAL_INT((int)StorageState::FAULTED, (int)c.state);
    TEST_ASSERT_EQUAL_UINT32(1, c.faults);
}

void test_a_success_resets_the_failure_run()
{
    mount_it(100000);
    dws_hotswap_core_io(&c, false);
    dws_hotswap_core_io(&c, false);
    dws_hotswap_core_io(&c, true); // proves the medium is still there
    TEST_ASSERT_EQUAL_UINT8(0, c.fail_run);
    // So intermittent noise never accumulates into a false removal.
    TEST_ASSERT_FALSE(dws_hotswap_core_io(&c, false));
    TEST_ASSERT_FALSE(dws_hotswap_core_io(&c, false));
    TEST_ASSERT_EQUAL_INT((int)StorageState::READY, (int)c.state);
}

void test_further_failures_while_faulted_are_ignored()
{
    mount_it(100000);
    for (int i = 0; i < 3; i++)
        dws_hotswap_core_io(&c, false);
    TEST_ASSERT_EQUAL_UINT32(1, c.faults);
    // A caller honoring ready() stops here; a stray report must not re-fire the transition.
    TEST_ASSERT_FALSE(dws_hotswap_core_io(&c, false));
    TEST_ASSERT_FALSE(dws_hotswap_core_io(&c, true));
    TEST_ASSERT_EQUAL_UINT32(1, c.faults);
    TEST_ASSERT_EQUAL_INT((int)StorageState::FAULTED, (int)c.state);
}

void test_io_while_absent_is_ignored()
{
    TEST_ASSERT_FALSE(dws_hotswap_core_io(&c, false));
    TEST_ASSERT_EQUAL_INT((int)StorageState::ABSENT, (int)c.state);
    TEST_ASSERT_EQUAL_UINT32(0, c.faults);
}

void test_fail_run_saturates_instead_of_wrapping()
{
    dws_hotswap_core_init(&c, 255, 2000, 0);
    mount_it(0);
    for (int i = 0; i < 400; i++)
        dws_hotswap_core_io(&c, false);
    // A wrapping counter would drop back under the threshold and un-fault the volume.
    TEST_ASSERT_EQUAL_INT((int)StorageState::FAULTED, (int)c.state);
}

// --- probe pacing ---------------------------------------------------------

void test_no_probe_while_ready()
{
    mount_it(100000);
    TEST_ASSERT_FALSE(dws_hotswap_core_due(&c, 100000 + 999999));
}

void test_probe_is_rate_limited_while_absent()
{
    dws_hotswap_core_probe(&c, false, false, 100000); // nothing there
    TEST_ASSERT_FALSE(dws_hotswap_core_due(&c, 100000 + 1999));
    TEST_ASSERT_TRUE(dws_hotswap_core_due(&c, 100000 + 2000));
}

void test_probe_pacing_is_wrapsafe_across_rollover()
{
    // Last probe just before the 32-bit millis rollover; "now" just after it.
    dws_hotswap_core_probe(&c, false, false, 0xFFFFF000u);
    TEST_ASSERT_FALSE(dws_hotswap_core_due(&c, 0xFFFFF000u + 1999));
    TEST_ASSERT_TRUE(dws_hotswap_core_due(&c, 0xFFFFF000u + 2000)); // wraps past 0
}

// --- mounting -------------------------------------------------------------

void test_present_but_unmountable_stays_absent()
{
    // A card that will not mount is not storage, however present the detect pin says it is.
    TEST_ASSERT_FALSE(dws_hotswap_core_probe(&c, true, false, 100000));
    TEST_ASSERT_EQUAL_INT((int)StorageState::ABSENT, (int)c.state);
    TEST_ASSERT_EQUAL_UINT32(0, c.mounts);
}

void test_mount_counts_only_on_transition()
{
    TEST_ASSERT_TRUE(dws_hotswap_core_probe(&c, true, true, 100000));
    TEST_ASSERT_EQUAL_UINT32(1, c.mounts);
    // A redundant probe of an already-mounted volume is not another insertion.
    TEST_ASSERT_FALSE(dws_hotswap_core_probe(&c, true, true, 101000));
    TEST_ASSERT_EQUAL_UINT32(1, c.mounts);
}

void test_full_removal_and_reinsertion_cycle()
{
    mount_it(100000);
    TEST_ASSERT_EQUAL_UINT32(1, c.mounts);

    // Card pulled: writes start failing.
    for (int i = 0; i < 3; i++)
        dws_hotswap_core_io(&c, false);
    TEST_ASSERT_EQUAL_INT((int)StorageState::FAULTED, (int)c.state);

    // Probe while it is still out.
    TEST_ASSERT_TRUE(dws_hotswap_core_due(&c, 102000));
    dws_hotswap_core_probe(&c, false, false, 102000);
    TEST_ASSERT_EQUAL_INT((int)StorageState::ABSENT, (int)c.state);

    // Card back in.
    TEST_ASSERT_TRUE(dws_hotswap_core_due(&c, 104000));
    TEST_ASSERT_TRUE(dws_hotswap_core_probe(&c, true, true, 104000));
    TEST_ASSERT_EQUAL_INT((int)StorageState::READY, (int)c.state);
    TEST_ASSERT_EQUAL_UINT32(2, c.mounts);
    TEST_ASSERT_EQUAL_UINT32(1, c.faults);
    TEST_ASSERT_EQUAL_UINT8(0, c.fail_run); // a fresh mount starts with a clean run

    // And it is usable again: the old failures must not carry over.
    TEST_ASSERT_FALSE(dws_hotswap_core_io(&c, false));
    TEST_ASSERT_EQUAL_INT((int)StorageState::READY, (int)c.state);
}

void test_faulted_volume_can_go_straight_back_to_ready()
{
    // A card reseated quickly enough that the probe finds it mounted without an ABSENT step.
    mount_it(100000);
    for (int i = 0; i < 3; i++)
        dws_hotswap_core_io(&c, false);
    TEST_ASSERT_TRUE(dws_hotswap_core_probe(&c, true, true, 102000));
    TEST_ASSERT_EQUAL_INT((int)StorageState::READY, (int)c.state);
    TEST_ASSERT_EQUAL_UINT32(2, c.mounts);
}

// --- null safety + serialization -----------------------------------------

void test_null_core_is_not_a_crash()
{
    dws_hotswap_core_init(nullptr, 3, 2000, 0);
    TEST_ASSERT_FALSE(dws_hotswap_core_io(nullptr, false));
    TEST_ASSERT_FALSE(dws_hotswap_core_due(nullptr, 0));
    TEST_ASSERT_FALSE(dws_hotswap_core_probe(nullptr, true, true, 0));
}

void test_state_names()
{
    TEST_ASSERT_EQUAL_STRING("absent", dws_hotswap_state_name(StorageState::ABSENT));
    TEST_ASSERT_EQUAL_STRING("ready", dws_hotswap_state_name(StorageState::READY));
    TEST_ASSERT_EQUAL_STRING("faulted", dws_hotswap_state_name(StorageState::FAULTED));
}

void test_json_and_overflow_is_fail_closed()
{
    char buf[64];
    size_t n = dws_hotswap_json(buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_STRING("{\"storage\":\"absent\",\"mounts\":0,\"faults\":0}", buf);

    char tiny[8];
    TEST_ASSERT_EQUAL_UINT32(0, dws_hotswap_json(tiny, sizeof(tiny)));
    TEST_ASSERT_EQUAL_STRING("", tiny); // never a truncated half-object
    TEST_ASSERT_EQUAL_UINT32(0, dws_hotswap_json(nullptr, 16));
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_starts_absent_not_ready);
    RUN_TEST(test_first_probe_is_due_immediately);
    RUN_TEST(test_first_probe_is_due_when_init_time_is_near_zero);
    RUN_TEST(test_zero_threshold_is_clamped_to_one);
    RUN_TEST(test_one_failure_does_not_fault_a_healthy_volume);
    RUN_TEST(test_threshold_run_faults_and_counts);
    RUN_TEST(test_a_success_resets_the_failure_run);
    RUN_TEST(test_further_failures_while_faulted_are_ignored);
    RUN_TEST(test_io_while_absent_is_ignored);
    RUN_TEST(test_fail_run_saturates_instead_of_wrapping);
    RUN_TEST(test_no_probe_while_ready);
    RUN_TEST(test_probe_is_rate_limited_while_absent);
    RUN_TEST(test_probe_pacing_is_wrapsafe_across_rollover);
    RUN_TEST(test_present_but_unmountable_stays_absent);
    RUN_TEST(test_mount_counts_only_on_transition);
    RUN_TEST(test_full_removal_and_reinsertion_cycle);
    RUN_TEST(test_faulted_volume_can_go_straight_back_to_ready);
    RUN_TEST(test_null_core_is_not_a_crash);
    RUN_TEST(test_state_names);
    RUN_TEST(test_json_and_overflow_is_fail_closed);
    return UNITY_END();
}
