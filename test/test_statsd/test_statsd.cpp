// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the StatsD client (services/statsd): the pure line formatter
// (statsd_format - types, sample rate, tags, overflow, bad args) and the emit helpers
// (statsd_count/gauge/timing/set), whose formatted-and-sent bytes are captured through the
// transport UDP service's host capture seam.

#include "network_drivers/transport/udp.h" // det_udp_capture_* (host seam)
#include "services/statsd/statsd.h"
#include <string>
#include <unity.h>

static std::string captured()
{
    const uint8_t *p = det_udp_captured();
    return p ? std::string((const char *)p, det_udp_captured_len()) : std::string();
}

void setUp()
{
    det_udp_capture_enable();
    det_udp_capture_reset();
}
void tearDown()
{
}

// ---- pure formatter ----

void test_format_types()
{
    char out[64];
    TEST_ASSERT_TRUE(statsd_format(out, sizeof(out), "api.hits", "1", STATSD_COUNTER, 1.0f, nullptr));
    TEST_ASSERT_EQUAL_STRING("api.hits:1|c", out);
    statsd_format(out, sizeof(out), "temp", "42", STATSD_GAUGE, 1.0f, nullptr);
    TEST_ASSERT_EQUAL_STRING("temp:42|g", out);
    statsd_format(out, sizeof(out), "req.latency", "120", STATSD_TIMING, 1.0f, nullptr);
    TEST_ASSERT_EQUAL_STRING("req.latency:120|ms", out); // timing renders as "ms"
    statsd_format(out, sizeof(out), "users", "u42", STATSD_SET, 1.0f, nullptr);
    TEST_ASSERT_EQUAL_STRING("users:u42|s", out);
}

void test_format_sample_rate()
{
    char out[64];
    statsd_format(out, sizeof(out), "x", "1", STATSD_COUNTER, 0.1f, nullptr);
    TEST_ASSERT_EQUAL_STRING("x:1|c|@0.1", out);
    statsd_format(out, sizeof(out), "x", "1", STATSD_COUNTER, 0.5f, nullptr);
    TEST_ASSERT_EQUAL_STRING("x:1|c|@0.5", out);
    statsd_format(out, sizeof(out), "x", "1", STATSD_COUNTER, 0.01f, nullptr);
    TEST_ASSERT_EQUAL_STRING("x:1|c|@0.01", out);
    statsd_format(out, sizeof(out), "x", "1", STATSD_COUNTER, 1.0f, nullptr); // >=1 -> no annotation
    TEST_ASSERT_EQUAL_STRING("x:1|c", out);
}

void test_format_tags_and_both()
{
    char out[80];
    statsd_format(out, sizeof(out), "x", "1", STATSD_COUNTER, 1.0f, "env:prod,host:a");
    TEST_ASSERT_EQUAL_STRING("x:1|c|#env:prod,host:a", out);
    statsd_format(out, sizeof(out), "x", "1", STATSD_COUNTER, 0.1f, "env:prod");
    TEST_ASSERT_EQUAL_STRING("x:1|c|@0.1|#env:prod", out); // rate before tags
}

void test_format_guards()
{
    char out[64];
    TEST_ASSERT_EQUAL_UINT(0, statsd_format(out, sizeof(out), "x", "1", 'z', 1.0f, nullptr)); // bad type
    TEST_ASSERT_EQUAL_UINT(0, statsd_format(out, sizeof(out), nullptr, "1", STATSD_COUNTER, 1.0f, nullptr));
    TEST_ASSERT_EQUAL_UINT(0, statsd_format(out, sizeof(out), "", "1", STATSD_COUNTER, 1.0f, nullptr));
    TEST_ASSERT_EQUAL_UINT(0, statsd_format(out, sizeof(out), "x", nullptr, STATSD_COUNTER, 1.0f, nullptr));
    TEST_ASSERT_EQUAL_UINT(0, statsd_format(out, 5, "toolongname", "1", STATSD_COUNTER, 1.0f, nullptr)); // overflow
}

// ---- emit helpers (formatted + "sent", captured via the UDP seam) ----

void test_emit_counter_and_negative()
{
    statsd_begin("collector.local", 8125, nullptr);
    statsd_count("api.hits", 3);
    TEST_ASSERT_EQUAL_STRING("api.hits:3|c", captured().c_str());
    det_udp_capture_reset();
    statsd_count("api.hits", -4); // counters may go negative
    TEST_ASSERT_EQUAL_STRING("api.hits:-4|c", captured().c_str());
}

void test_emit_gauge_and_delta()
{
    statsd_begin("h", 0, nullptr); // 0 -> default port
    statsd_gauge("heap.free", 200000);
    TEST_ASSERT_EQUAL_STRING("heap.free:200000|g", captured().c_str());
    det_udp_capture_reset();
    statsd_gauge_delta("conns", 5);
    TEST_ASSERT_EQUAL_STRING("conns:+5|g", captured().c_str());
    det_udp_capture_reset();
    statsd_gauge_delta("conns", -2);
    TEST_ASSERT_EQUAL_STRING("conns:-2|g", captured().c_str());
}

void test_emit_timing_set_sampled()
{
    statsd_begin("h", 8125, nullptr);
    statsd_timing("db.query", 120);
    TEST_ASSERT_EQUAL_STRING("db.query:120|ms", captured().c_str());
    det_udp_capture_reset();
    statsd_set("uniques", "device-7");
    TEST_ASSERT_EQUAL_STRING("uniques:device-7|s", captured().c_str());
    det_udp_capture_reset();
    statsd_count_sampled("rare", 1, 0.25f);
    TEST_ASSERT_EQUAL_STRING("rare:1|c|@0.25", captured().c_str());
}

void test_emit_global_tags()
{
    statsd_begin("h", 8125, "env:prod,region:us");
    statsd_count("x", 1);
    TEST_ASSERT_EQUAL_STRING("x:1|c|#env:prod,region:us", captured().c_str());
}

void test_emit_noop_until_begin()
{
    statsd_begin(nullptr, 0, nullptr); // clears the target
    det_udp_capture_reset();
    statsd_count("x", 1); // no target -> nothing sent, no crash
    TEST_ASSERT_EQUAL_UINT(0, det_udp_captured_len());
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_format_types);
    RUN_TEST(test_format_sample_rate);
    RUN_TEST(test_format_tags_and_both);
    RUN_TEST(test_format_guards);
    RUN_TEST(test_emit_counter_and_negative);
    RUN_TEST(test_emit_gauge_and_delta);
    RUN_TEST(test_emit_timing_set_sampled);
    RUN_TEST(test_emit_global_tags);
    RUN_TEST(test_emit_noop_until_begin);
    return UNITY_END();
}
