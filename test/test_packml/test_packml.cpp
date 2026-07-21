// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the PackML / OMAC state model (ISA-TR88.00.02): the pure transition engine
// (command / state-complete / execute-complete + command validity) and the owned PackTags service
// (state advance, production counters, unit-mode rules, machine speed, and the state/reset timers).

#include "services/clock.h"
#include "services/packml/packml.h"
#include <unity.h>

// Host clock seam so the timer tags (StateCurrentTime, AccTimeSinceReset) are deterministic.
static uint32_t g_ms = 0;
static uint32_t test_clock()
{
    return g_ms;
}

void setUp()
{
    g_ms = 0;
    dws_set_clock(test_clock, 1000);
    dws_packml_svc_init(PackMlMode::PRODUCING);
}
void tearDown()
{
}

// ---- Pure engine: happy production path -----------------------------------

void test_engine_startup_to_execute()
{
    PackMlState s = PackMlState::STOPPED;
    s = dws_packml_command(s, PackMlCommand::RESET);
    TEST_ASSERT_EQUAL(PackMlState::RESETTING, s);
    s = dws_packml_state_complete(s);
    TEST_ASSERT_EQUAL(PackMlState::IDLE, s);
    s = dws_packml_command(s, PackMlCommand::START);
    TEST_ASSERT_EQUAL(PackMlState::STARTING, s);
    s = dws_packml_state_complete(s);
    TEST_ASSERT_EQUAL(PackMlState::EXECUTE, s);
}

void test_engine_execute_to_complete_and_back()
{
    PackMlState s = PackMlState::EXECUTE;
    s = dws_packml_execute_complete(s);
    TEST_ASSERT_EQUAL(PackMlState::COMPLETING, s);
    s = dws_packml_state_complete(s);
    TEST_ASSERT_EQUAL(PackMlState::COMPLETE, s);
    // Complete -> Reset -> Resetting -> Idle, ready for the next run.
    s = dws_packml_command(s, PackMlCommand::RESET);
    TEST_ASSERT_EQUAL(PackMlState::RESETTING, s);
    TEST_ASSERT_EQUAL(PackMlState::IDLE, dws_packml_state_complete(s));
}

void test_engine_hold_unhold()
{
    PackMlState s = PackMlState::EXECUTE;
    s = dws_packml_command(s, PackMlCommand::HOLD);
    TEST_ASSERT_EQUAL(PackMlState::HOLDING, s);
    s = dws_packml_state_complete(s);
    TEST_ASSERT_EQUAL(PackMlState::HELD, s);
    s = dws_packml_command(s, PackMlCommand::UNHOLD);
    TEST_ASSERT_EQUAL(PackMlState::UNHOLDING, s);
    TEST_ASSERT_EQUAL(PackMlState::EXECUTE, dws_packml_state_complete(s));
}

void test_engine_suspend_unsuspend()
{
    PackMlState s = PackMlState::EXECUTE;
    s = dws_packml_command(s, PackMlCommand::SUSPEND);
    TEST_ASSERT_EQUAL(PackMlState::SUSPENDING, s);
    s = dws_packml_state_complete(s);
    TEST_ASSERT_EQUAL(PackMlState::SUSPENDED, s);
    s = dws_packml_command(s, PackMlCommand::UNSUSPEND);
    TEST_ASSERT_EQUAL(PackMlState::UNSUSPENDING, s);
    TEST_ASSERT_EQUAL(PackMlState::EXECUTE, dws_packml_state_complete(s));
}

// ---- Universal Stop / Abort branches --------------------------------------

void test_engine_stop_from_many_states()
{
    const PackMlState from[] = {PackMlState::IDLE,      PackMlState::EXECUTE,  PackMlState::HELD,
                                PackMlState::SUSPENDED, PackMlState::COMPLETE, PackMlState::RESETTING};
    for (unsigned i = 0; i < sizeof(from) / sizeof(from[0]); i++)
        TEST_ASSERT_EQUAL(PackMlState::STOPPING, dws_packml_command(from[i], PackMlCommand::STOP));
    // Stopping -> Stopped.
    TEST_ASSERT_EQUAL(PackMlState::STOPPED, dws_packml_state_complete(PackMlState::STOPPING));
    // Stop is a no-op once already Stopped / Stopping / in the abort branch.
    TEST_ASSERT_EQUAL(PackMlState::STOPPED, dws_packml_command(PackMlState::STOPPED, PackMlCommand::STOP));
    TEST_ASSERT_EQUAL(PackMlState::ABORTED, dws_packml_command(PackMlState::ABORTED, PackMlCommand::STOP));
}

void test_engine_abort_and_clear()
{
    // Abort from any non-abort state -> Aborting -> Aborted.
    TEST_ASSERT_EQUAL(PackMlState::ABORTING, dws_packml_command(PackMlState::EXECUTE, PackMlCommand::ABORT));
    TEST_ASSERT_EQUAL(PackMlState::ABORTING, dws_packml_command(PackMlState::STOPPED, PackMlCommand::ABORT));
    TEST_ASSERT_EQUAL(PackMlState::ABORTED, dws_packml_state_complete(PackMlState::ABORTING));
    // Abort is a no-op once aborting/aborted.
    TEST_ASSERT_EQUAL(PackMlState::ABORTED, dws_packml_command(PackMlState::ABORTED, PackMlCommand::ABORT));
    // Aborted -> Clear -> Clearing -> Stopped.
    PackMlState s = dws_packml_command(PackMlState::ABORTED, PackMlCommand::CLEAR);
    TEST_ASSERT_EQUAL(PackMlState::CLEARING, s);
    TEST_ASSERT_EQUAL(PackMlState::STOPPED, dws_packml_state_complete(s));
}

// ---- Command validity ------------------------------------------------------

void test_engine_invalid_commands_are_noops()
{
    // Start only from Idle; Hold only from Execute; Reset only from Stopped/Complete; etc.
    TEST_ASSERT_EQUAL(PackMlState::STOPPED, dws_packml_command(PackMlState::STOPPED, PackMlCommand::START));
    TEST_ASSERT_EQUAL(PackMlState::IDLE, dws_packml_command(PackMlState::IDLE, PackMlCommand::HOLD));
    TEST_ASSERT_EQUAL(PackMlState::EXECUTE, dws_packml_command(PackMlState::EXECUTE, PackMlCommand::RESET));
    TEST_ASSERT_EQUAL(PackMlState::IDLE, dws_packml_command(PackMlState::IDLE, PackMlCommand::RESET));
    TEST_ASSERT_FALSE(dws_packml_command_valid(PackMlState::STOPPED, PackMlCommand::START));
    TEST_ASSERT_TRUE(dws_packml_command_valid(PackMlState::STOPPED, PackMlCommand::RESET));
    TEST_ASSERT_TRUE(dws_packml_command_valid(PackMlState::EXECUTE, PackMlCommand::HOLD));
}

void test_engine_acting_classification()
{
    TEST_ASSERT_TRUE(dws_packml_is_acting(PackMlState::STARTING));
    TEST_ASSERT_TRUE(dws_packml_is_acting(PackMlState::ABORTING));
    TEST_ASSERT_TRUE(dws_packml_is_acting(PackMlState::COMPLETING));
    TEST_ASSERT_FALSE(dws_packml_is_acting(PackMlState::EXECUTE));
    TEST_ASSERT_FALSE(dws_packml_is_acting(PackMlState::STOPPED));
    TEST_ASSERT_FALSE(dws_packml_is_acting(PackMlState::ABORTED));
    // Wait states do not auto-advance.
    TEST_ASSERT_EQUAL(PackMlState::EXECUTE, dws_packml_state_complete(PackMlState::EXECUTE));
    TEST_ASSERT_EQUAL(PackMlState::IDLE, dws_packml_state_complete(PackMlState::IDLE));
}

void test_state_wire_numbers()
{
    // Status.StateCurrent carries the ISA-TR88.00.02 numbers an HMI expects.
    TEST_ASSERT_EQUAL_UINT8(2, (uint8_t)PackMlState::STOPPED);
    TEST_ASSERT_EQUAL_UINT8(6, (uint8_t)PackMlState::EXECUTE);
    TEST_ASSERT_EQUAL_UINT8(9, (uint8_t)PackMlState::ABORTED);
    TEST_ASSERT_EQUAL_UINT8(17, (uint8_t)PackMlState::COMPLETE);
    TEST_ASSERT_EQUAL_STRING("Execute", dws_packml_state_name(PackMlState::EXECUTE));
    TEST_ASSERT_EQUAL_STRING("Abort", dws_packml_command_name(PackMlCommand::ABORT));
}

// ---- Owned service ---------------------------------------------------------

void test_svc_init_is_stopped()
{
    TEST_ASSERT_EQUAL(PackMlState::STOPPED, dws_packml_svc_state());
    PackMlStatus st;
    dws_packml_svc_status(&st);
    TEST_ASSERT_EQUAL(PackMlState::STOPPED, st.state_current);
    TEST_ASSERT_EQUAL(PackMlMode::PRODUCING, st.unit_mode_current);
    TEST_ASSERT_EQUAL_UINT32(0, st.prod_processed);
}

void test_svc_full_run_with_counts()
{
    TEST_ASSERT_TRUE(dws_packml_svc_command(PackMlCommand::RESET));
    dws_packml_svc_state_complete(); // -> Idle
    TEST_ASSERT_EQUAL(PackMlState::IDLE, dws_packml_svc_state());
    TEST_ASSERT_TRUE(dws_packml_svc_command(PackMlCommand::START));
    dws_packml_svc_state_complete(); // -> Execute
    TEST_ASSERT_EQUAL(PackMlState::EXECUTE, dws_packml_svc_state());

    dws_packml_svc_count(false);
    dws_packml_svc_count(false);
    dws_packml_svc_count(true); // one defective
    PackMlStatus st;
    dws_packml_svc_status(&st);
    TEST_ASSERT_EQUAL_UINT32(3, st.prod_processed);
    TEST_ASSERT_EQUAL_UINT32(1, st.prod_defective);

    TEST_ASSERT_TRUE(dws_packml_svc_complete_run()); // -> Completing
    dws_packml_svc_state_complete();                 // -> Complete
    TEST_ASSERT_EQUAL(PackMlState::COMPLETE, dws_packml_svc_state());
}

void test_svc_count_only_in_execute()
{
    // Not executing (Stopped) -> counts are ignored.
    dws_packml_svc_count(false);
    PackMlStatus st;
    dws_packml_svc_status(&st);
    TEST_ASSERT_EQUAL_UINT32(0, st.prod_processed);
}

void test_svc_rejects_illegal_command()
{
    // Start is illegal in Stopped; the service reports no change.
    TEST_ASSERT_FALSE(dws_packml_svc_command(PackMlCommand::START));
    TEST_ASSERT_EQUAL(PackMlState::STOPPED, dws_packml_svc_state());
}

void test_svc_mode_change_rules()
{
    // Allowed in Stopped.
    TEST_ASSERT_TRUE(dws_packml_svc_set_mode(PackMlMode::MAINTENANCE));
    // Drive into Execute, where a mode change must be refused.
    dws_packml_svc_command(PackMlCommand::RESET);
    dws_packml_svc_state_complete();
    dws_packml_svc_command(PackMlCommand::START);
    dws_packml_svc_state_complete();
    TEST_ASSERT_EQUAL(PackMlState::EXECUTE, dws_packml_svc_state());
    TEST_ASSERT_FALSE(dws_packml_svc_set_mode(PackMlMode::MANUAL));
    PackMlStatus st;
    dws_packml_svc_status(&st);
    TEST_ASSERT_EQUAL(PackMlMode::MAINTENANCE, st.unit_mode_current); // unchanged
}

void test_svc_speed_actual_tracks_execute()
{
    dws_packml_svc_set_speed(120.0f);
    PackMlStatus st;
    dws_packml_svc_status(&st);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, st.mach_speed_actual); // Stopped -> 0
    dws_packml_svc_command(PackMlCommand::RESET);
    dws_packml_svc_state_complete();
    dws_packml_svc_command(PackMlCommand::START);
    dws_packml_svc_state_complete(); // Execute
    dws_packml_svc_status(&st);
    TEST_ASSERT_EQUAL_FLOAT(120.0f, st.mach_speed_actual);
}

void test_svc_timers()
{
    g_ms = 1000;
    dws_packml_svc_command(PackMlCommand::RESET); // reset stamps AccTimeSinceReset base + enters Resetting
    g_ms = 1500;
    PackMlStatus st;
    dws_packml_svc_status(&st);
    TEST_ASSERT_EQUAL_UINT32(500, st.state_current_ms);        // 1500 - 1000 (entered Resetting at 1000)
    TEST_ASSERT_EQUAL_UINT32(500, st.acc_time_since_reset_ms); // reset at 1000
    dws_packml_svc_state_complete();                           // -> Idle at 1500
    g_ms = 1800;
    dws_packml_svc_status(&st);
    TEST_ASSERT_EQUAL_UINT32(300, st.state_current_ms);        // 1800 - 1500
    TEST_ASSERT_EQUAL_UINT32(800, st.acc_time_since_reset_ms); // 1800 - 1000
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_engine_startup_to_execute);
    RUN_TEST(test_engine_execute_to_complete_and_back);
    RUN_TEST(test_engine_hold_unhold);
    RUN_TEST(test_engine_suspend_unsuspend);
    RUN_TEST(test_engine_stop_from_many_states);
    RUN_TEST(test_engine_abort_and_clear);
    RUN_TEST(test_engine_invalid_commands_are_noops);
    RUN_TEST(test_engine_acting_classification);
    RUN_TEST(test_state_wire_numbers);
    RUN_TEST(test_svc_init_is_stopped);
    RUN_TEST(test_svc_full_run_with_counts);
    RUN_TEST(test_svc_count_only_in_execute);
    RUN_TEST(test_svc_rejects_illegal_command);
    RUN_TEST(test_svc_mode_change_rules);
    RUN_TEST(test_svc_speed_actual_tracks_execute);
    RUN_TEST(test_svc_timers);
    return UNITY_END();
}
