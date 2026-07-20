// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the abstract logging layer (shared_primitives/log.h). Built at
// DWS_LOG_LEVEL_INFO with DWS_ENABLE_LOGBUF, so the interesting property is testable at runtime:
// DEBUG sits below the floor and must be *absent* from the binary, while INFO and above emit.
//
// The compile-time half of the guarantee (no code, no flash string for a discarded level) is not a
// runtime property, so it is asserted the only way it can be - by observing that a discarded call
// reaches neither the sink nor the ring, with the argument-evaluation probe below proving the
// arguments were never even evaluated.

#include "services/logbuf/logbuf.h"
#include "shared_primitives/log.h"
#include <stdio.h>
#include <string.h>
#include <unity.h>

static uint8_t s_last_level = 0xFF;
static char s_last_line[256];
static int s_sink_calls = 0;
static int s_eval_count = 0;

static void test_sink(uint8_t level, const char *line)
{
    s_last_level = level;
    snprintf(s_last_line, sizeof(s_last_line), "%s", line);
    s_sink_calls++;
}

// Named in a log argument; a discarded level must never call it.
static int counting_arg(void)
{
    s_eval_count++;
    return 42;
}

void setUp()
{
    s_last_level = 0xFF;
    s_last_line[0] = '\0';
    s_sink_calls = 0;
    s_eval_count = 0;
    dws_logbuf_reset();
    dws_log_set_sink(test_sink);
}

void tearDown()
{
    dws_log_set_sink(nullptr);
}

// --- the floor ------------------------------------------------------------

void test_debug_is_below_the_floor_and_emits_nothing()
{
    DWS_LOGD("debug %d", counting_arg());
    TEST_ASSERT_EQUAL_INT(0, s_sink_calls);
    TEST_ASSERT_EQUAL_UINT16(0, dws_log_count());
}

void test_discarded_call_does_not_evaluate_its_arguments()
{
    // The whole point of a preprocessor filter rather than a runtime `if`: a discarded log must not
    // pay for building its own arguments either.
    DWS_LOGD("expensive %d", counting_arg());
    TEST_ASSERT_EQUAL_INT(0, s_eval_count);
}

void test_info_and_above_emit()
{
    DWS_LOGI("hello %d", 7);
    TEST_ASSERT_EQUAL_INT(1, s_sink_calls);
    TEST_ASSERT_EQUAL_UINT8(DWS_LOG_LEVEL_INFO, s_last_level);
    TEST_ASSERT_EQUAL_STRING("hello 7", s_last_line);

    DWS_LOGW("warn %s", "here");
    TEST_ASSERT_EQUAL_UINT8(DWS_LOG_LEVEL_WARN, s_last_level);
    TEST_ASSERT_EQUAL_STRING("warn here", s_last_line);

    DWS_LOGE("err %u", 3u);
    TEST_ASSERT_EQUAL_UINT8(DWS_LOG_LEVEL_ERROR, s_last_level);
    TEST_ASSERT_EQUAL_STRING("err 3", s_last_line);

    TEST_ASSERT_EQUAL_INT(3, s_sink_calls);
}

void test_enabled_call_does_evaluate_its_arguments()
{
    DWS_LOGI("value %d", counting_arg());
    TEST_ASSERT_EQUAL_INT(1, s_eval_count);
    TEST_ASSERT_EQUAL_STRING("value 42", s_last_line);
}

// --- routing --------------------------------------------------------------

void test_emitted_line_also_reaches_the_logbuf_ring()
{
    DWS_LOGW("to the ring");
    TEST_ASSERT_EQUAL_UINT16(1, dws_log_count());
    TEST_ASSERT_EQUAL_STRING("W to the ring", dws_log_at(0));
}

void test_levels_match_the_logbuf_letters()
{
    // The DWS_LOG_LEVEL_* preprocessor values and DWSLogLevel's constexprs are two spellings of one
    // scale; if they ever drift, the stored letter is what goes wrong, so assert on that.
    DWS_LOGI("i");
    DWS_LOGW("w");
    DWS_LOGE("e");
    TEST_ASSERT_EQUAL_STRING("I i", dws_log_at(0));
    TEST_ASSERT_EQUAL_STRING("W w", dws_log_at(1));
    TEST_ASSERT_EQUAL_STRING("E e", dws_log_at(2));
    TEST_ASSERT_EQUAL_INT(DWS_LOG_LEVEL_INFO, (int)DWSLogLevel::DWS_LOG_INFO);
    TEST_ASSERT_EQUAL_INT(DWS_LOG_LEVEL_WARN, (int)DWSLogLevel::DWS_LOG_WARN);
    TEST_ASSERT_EQUAL_INT(DWS_LOG_LEVEL_ERROR, (int)DWSLogLevel::DWS_LOG_ERROR);
    TEST_ASSERT_EQUAL_INT(DWS_LOG_LEVEL_DEBUG, (int)DWSLogLevel::DWS_LOG_DEBUG);
}

void test_no_sink_is_not_a_crash()
{
    dws_log_set_sink(nullptr);
    DWS_LOGE("still goes to the ring");
    TEST_ASSERT_EQUAL_INT(0, s_sink_calls);
    TEST_ASSERT_EQUAL_UINT16(1, dws_log_count());
}

// --- formatting edges -----------------------------------------------------

void test_long_line_is_truncated_not_overflowed()
{
    char big[DWS_LOG_LINE_LEN * 3];
    memset(big, 'x', sizeof(big) - 1);
    big[sizeof(big) - 1] = '\0';
    DWS_LOGE("%s", big);
    TEST_ASSERT_EQUAL_INT(1, s_sink_calls);
    TEST_ASSERT_TRUE(strlen(s_last_line) < DWS_LOG_LINE_LEN);
}

void test_null_format_is_ignored()
{
    const char *fmt = nullptr;
    dws_log_printf(DWS_LOG_LEVEL_ERROR, fmt);
    TEST_ASSERT_EQUAL_INT(0, s_sink_calls);
    TEST_ASSERT_EQUAL_UINT16(0, dws_log_count());
}

void test_empty_message_is_still_a_line()
{
    DWS_LOGI("%s", "");
    TEST_ASSERT_EQUAL_INT(1, s_sink_calls);
    TEST_ASSERT_EQUAL_STRING("", s_last_line);
    TEST_ASSERT_EQUAL_STRING("I ", dws_log_at(0));
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_debug_is_below_the_floor_and_emits_nothing);
    RUN_TEST(test_discarded_call_does_not_evaluate_its_arguments);
    RUN_TEST(test_info_and_above_emit);
    RUN_TEST(test_enabled_call_does_evaluate_its_arguments);
    RUN_TEST(test_emitted_line_also_reaches_the_logbuf_ring);
    RUN_TEST(test_levels_match_the_logbuf_letters);
    RUN_TEST(test_no_sink_is_not_a_crash);
    RUN_TEST(test_long_line_is_truncated_not_overflowed);
    RUN_TEST(test_null_format_is_ignored);
    RUN_TEST(test_empty_message_is_still_a_line);
    return UNITY_END();
}
