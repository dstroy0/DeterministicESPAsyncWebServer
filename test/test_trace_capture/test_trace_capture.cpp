// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the pre/post-trigger sample-window assembler (services/trace_capture):
// the pre-trigger ring wrap + freeze on trigger(), post-trigger fill + sink firing, one
// capture in flight fail-closed, feed() before begin()/after end() counted dropped, the
// zero-pretrigger edge case, and a multi-window trace_id sequence. Pure host tests.
// The env sizes DWS_TC_MAX_WINDOW_SAMPLES = 32.

#include "services/trace_capture/trace_capture.h"
#include <unity.h>
#include <vector>

struct CapturedWindow
{
    std::vector<uint16_t> samples;
    uint16_t pretrigger_samples;
    uint32_t trace_id;
};

static std::vector<CapturedWindow> g_windows;

static void on_window(const dws_tc_window *w, void *)
{
    CapturedWindow cw;
    cw.samples.assign(w->samples, w->samples + w->n_samples);
    cw.pretrigger_samples = w->pretrigger_samples;
    cw.trace_id = w->trace_id;
    g_windows.push_back(cw);
}

void setUp()
{
    g_windows.clear();
    dws_tc_end();
}
void tearDown()
{
    dws_tc_end();
}

static bool begin(uint16_t pre, uint16_t post)
{
    dws_tc_config cfg = {};
    cfg.pretrigger_samples = pre;
    cfg.posttrigger_samples = post;
    cfg.sink = on_window;
    cfg.ctx = nullptr;
    return dws_tc_begin(&cfg);
}

void test_begin_validates()
{
    TEST_ASSERT_FALSE(dws_tc_begin(nullptr));
    dws_tc_config cfg = {};
    cfg.pretrigger_samples = 4;
    cfg.posttrigger_samples = 4;
    cfg.sink = nullptr; // missing sink
    TEST_ASSERT_FALSE(dws_tc_begin(&cfg));
    cfg.sink = on_window;
    cfg.pretrigger_samples = 0;
    cfg.posttrigger_samples = 0; // both zero -> nothing to capture
    TEST_ASSERT_FALSE(dws_tc_begin(&cfg));
    cfg.posttrigger_samples = DWS_TC_MAX_WINDOW_SAMPLES + 1; // exceeds static storage
    TEST_ASSERT_FALSE(dws_tc_begin(&cfg));
    TEST_ASSERT_TRUE(begin(4, 4));
}

void test_pretrigger_ring_wraps_and_freezes_on_trigger()
{
    TEST_ASSERT_TRUE(begin(4, 4));
    // Feed 6 samples into a 4-deep pre-trigger ring: only the last 4 (2,3,4,5) survive.
    const uint16_t pre[] = {0, 1, 2, 3, 4, 5};
    TEST_ASSERT_EQUAL_UINT16(6, dws_tc_feed(pre, 6));
    TEST_ASSERT_FALSE(dws_tc_capturing());
    TEST_ASSERT_TRUE(dws_tc_trigger());
    TEST_ASSERT_TRUE(dws_tc_capturing());

    const uint16_t post[] = {100, 101, 102, 103};
    TEST_ASSERT_EQUAL_UINT16(4, dws_tc_feed(post, 4));
    TEST_ASSERT_FALSE(dws_tc_capturing()); // window completed, fired

    TEST_ASSERT_EQUAL_size_t(1, g_windows.size());
    const CapturedWindow &w = g_windows[0];
    TEST_ASSERT_EQUAL_UINT16(4, w.pretrigger_samples);
    TEST_ASSERT_EQUAL_size_t(8, w.samples.size());
    const uint16_t expect[] = {2, 3, 4, 5, 100, 101, 102, 103};
    for (int i = 0; i < 8; i++)
        TEST_ASSERT_EQUAL_UINT16(expect[i], w.samples[i]);
    TEST_ASSERT_EQUAL_UINT32(0, w.trace_id);

    dws_tc_stats st;
    dws_tc_get_stats(&st);
    TEST_ASSERT_EQUAL_UINT32(1, st.windows_completed);
    TEST_ASSERT_EQUAL_UINT32(0, st.triggers_dropped);
}

void test_trigger_fail_closed_while_capturing()
{
    TEST_ASSERT_TRUE(begin(2, 4));
    const uint16_t pre[] = {9, 8};
    dws_tc_feed(pre, 2);
    TEST_ASSERT_TRUE(dws_tc_trigger());
    TEST_ASSERT_FALSE(dws_tc_trigger()); // a window is already filling -> rejected

    dws_tc_stats st;
    dws_tc_get_stats(&st);
    TEST_ASSERT_EQUAL_UINT32(1, st.triggers_dropped);
    TEST_ASSERT_EQUAL_size_t(0, g_windows.size()); // still filling, no sink call yet
}

void test_feed_before_begin_or_after_end_drops()
{
    const uint16_t s[] = {1, 2, 3};
    TEST_ASSERT_EQUAL_UINT16(0, dws_tc_feed(s, 3)); // never began
    dws_tc_stats st;
    dws_tc_get_stats(&st);
    TEST_ASSERT_EQUAL_UINT32(3, st.samples_dropped);

    TEST_ASSERT_TRUE(begin(2, 2));
    dws_tc_end();
    TEST_ASSERT_EQUAL_UINT16(0, dws_tc_feed(s, 3)); // ended
    TEST_ASSERT_FALSE(dws_tc_trigger());            // ended
}

void test_zero_pretrigger_edge_case()
{
    TEST_ASSERT_TRUE(begin(0, 3)); // trigger-only, no pre-roll
    TEST_ASSERT_TRUE(dws_tc_trigger());
    const uint16_t post[] = {7, 8, 9};
    TEST_ASSERT_EQUAL_UINT16(3, dws_tc_feed(post, 3));
    TEST_ASSERT_EQUAL_size_t(1, g_windows.size());
    TEST_ASSERT_EQUAL_UINT16(0, g_windows[0].pretrigger_samples);
    TEST_ASSERT_EQUAL_size_t(3, g_windows[0].samples.size());
    TEST_ASSERT_EQUAL_UINT16(7, g_windows[0].samples[0]);
    TEST_ASSERT_EQUAL_UINT16(9, g_windows[0].samples[2]);
}

void test_multiple_sequential_windows_increment_trace_id()
{
    TEST_ASSERT_TRUE(begin(1, 1));
    for (int i = 0; i < 3; i++)
    {
        const uint16_t pre[] = {(uint16_t)(100 + i)};
        dws_tc_feed(pre, 1);
        TEST_ASSERT_TRUE(dws_tc_trigger());
        const uint16_t post[] = {(uint16_t)(200 + i)};
        dws_tc_feed(post, 1);
    }
    TEST_ASSERT_EQUAL_size_t(3, g_windows.size());
    for (int i = 0; i < 3; i++)
        TEST_ASSERT_EQUAL_UINT32((uint32_t)i, g_windows[i].trace_id);

    dws_tc_stats st;
    dws_tc_get_stats(&st);
    TEST_ASSERT_EQUAL_UINT32(3, st.windows_completed);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_begin_validates);
    RUN_TEST(test_pretrigger_ring_wraps_and_freezes_on_trigger);
    RUN_TEST(test_trigger_fail_closed_while_capturing);
    RUN_TEST(test_feed_before_begin_or_after_end_drops);
    RUN_TEST(test_zero_pretrigger_edge_case);
    RUN_TEST(test_multiple_sequential_windows_increment_trace_id);
    return UNITY_END();
}
