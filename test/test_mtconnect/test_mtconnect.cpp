// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/mtconnect: the MTConnectStreams + MTConnectError document builders.

#include "services/mtconnect/mtconnect.h"
#include <stdio.h>
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

static bool contains(const char *hay, const char *needle)
{
    return strstr(hay, needle) != nullptr;
}

void test_streams_document(void)
{
    char buf[1024];
    DetwsMtcStreams s;
    detws_mtc_streams_begin(&s, buf, sizeof(buf), 1500, 42, "cnc1");
    detws_mtc_streams_add(&s, DETWS_MTC_EVENT, "Availability", "avail", 40, "2026-07-06T00:00:00Z", "AVAILABLE");
    detws_mtc_streams_add(&s, DETWS_MTC_SAMPLE, "Position", "xpos", 41, "2026-07-06T00:00:01Z", "12.5");
    detws_mtc_streams_add(&s, DETWS_MTC_CONDITION, "SystemCondition", "sys", 42, "2026-07-06T00:00:02Z", "Fault");
    size_t n = detws_mtc_streams_end(&s);
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_size_t(strlen(buf), n);

    TEST_ASSERT_TRUE(contains(buf, "<MTConnectStreams"));
    TEST_ASSERT_TRUE(contains(buf, "instanceId=\"1500\""));
    TEST_ASSERT_TRUE(contains(buf, "nextSequence=\"42\""));
    TEST_ASSERT_TRUE(contains(buf, "<DeviceStream name=\"cnc1\">"));
    TEST_ASSERT_TRUE(contains(buf, "<Events><Availability dataItemId=\"avail\" sequence=\"40\""));
    TEST_ASSERT_TRUE(contains(buf, ">AVAILABLE</Availability></Events>"));
    TEST_ASSERT_TRUE(contains(buf, "<Samples><Position dataItemId=\"xpos\" sequence=\"41\""));
    TEST_ASSERT_TRUE(contains(buf, ">12.5</Position></Samples>"));
    TEST_ASSERT_TRUE(contains(buf, "<Condition><Fault type=\"SystemCondition\" dataItemId=\"sys\""));
    // Properly closed.
    TEST_ASSERT_TRUE(contains(buf, "</ComponentStream></DeviceStream></Streams></MTConnectStreams>"));
}

void test_streams_escapes_value(void)
{
    char buf[512];
    DetwsMtcStreams s;
    detws_mtc_streams_begin(&s, buf, sizeof(buf), 1, 1, "d");
    detws_mtc_streams_add(&s, DETWS_MTC_EVENT, "Message", "msg", 1, "T", "a<b&c");
    detws_mtc_streams_end(&s);
    TEST_ASSERT_TRUE(contains(buf, ">a&lt;b&amp;c</Message>"));
}

void test_error_document(void)
{
    char buf[512];
    size_t n = detws_mtc_error(1500, "UNSUPPORTED", "bad path", buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_TRUE(contains(buf, "<MTConnectError"));
    TEST_ASSERT_TRUE(contains(buf, "instanceId=\"1500\""));
    TEST_ASSERT_TRUE(contains(buf, "<Error errorCode=\"UNSUPPORTED\">bad path</Error>"));
}

void test_overflow_returns_zero(void)
{
    char buf[40];
    DetwsMtcStreams s;
    detws_mtc_streams_begin(&s, buf, sizeof(buf), 1, 1, "device");
    detws_mtc_streams_add(&s, DETWS_MTC_SAMPLE, "Position", "x", 1, "T", "1.0");
    TEST_ASSERT_EQUAL_size_t(0, detws_mtc_streams_end(&s)); // did not fit
}

void test_escape_gt_quote_and_overflow()
{
    char buf[512];
    DetwsMtcStreams s;
    detws_mtc_streams_begin(&s, buf, sizeof(buf), 1, 1, "d");
    detws_mtc_streams_add(&s, DETWS_MTC_EVENT, "Message", "msg", 1, "T", "a>b\"c");
    detws_mtc_streams_end(&s);
    TEST_ASSERT_NOT_NULL(strstr(buf, "&gt;"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "&quot;"));
    // Stream overflow closes at end(); error document overflow fails closed.
    char tiny[64];
    DetwsMtcStreams s2;
    detws_mtc_streams_begin(&s2, tiny, sizeof(tiny), 1, 1, "device-with-a-very-long-name");
    detws_mtc_streams_add(&s2, DETWS_MTC_SAMPLE, "Position", "xpos", 1, "2026-07-06T00:00:01Z", "12.5");
    TEST_ASSERT_EQUAL_size_t(0, detws_mtc_streams_end(&s2));
    char t2[8];
    TEST_ASSERT_EQUAL_size_t(0, detws_mtc_error(1, "X", "y", t2, sizeof(t2)));
}

void test_devices_probe_document(void)
{
    char buf[1024];
    DetwsMtcStreams s;
    detws_mtc_devices_begin(&s, buf, sizeof(buf), 1500, "dev1", "cnc1", "uuid-abc");
    detws_mtc_devices_add_item(&s, DETWS_MTC_EVENT, "avail", "Availability", nullptr, nullptr);
    detws_mtc_devices_add_item(&s, DETWS_MTC_SAMPLE, "xpos", "Position", "Xabs", "MILLIMETER");
    detws_mtc_devices_add_item(&s, DETWS_MTC_CONDITION, "sys", "SystemCondition", nullptr, nullptr);
    size_t n = detws_mtc_devices_end(&s);
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_size_t(strlen(buf), n);

    TEST_ASSERT_TRUE(contains(buf, "<MTConnectDevices"));
    TEST_ASSERT_TRUE(contains(buf, "instanceId=\"1500\""));
    TEST_ASSERT_TRUE(contains(buf, "<Devices><Device id=\"dev1\" name=\"cnc1\" uuid=\"uuid-abc\"><DataItems>"));
    TEST_ASSERT_TRUE(contains(buf, "<DataItem category=\"EVENT\" id=\"avail\" type=\"Availability\"/>"));
    // name + units are emitted only when present.
    TEST_ASSERT_TRUE(contains(
        buf, "<DataItem category=\"SAMPLE\" id=\"xpos\" type=\"Position\" name=\"Xabs\" units=\"MILLIMETER\"/>"));
    TEST_ASSERT_TRUE(contains(buf, "<DataItem category=\"CONDITION\" id=\"sys\" type=\"SystemCondition\"/>"));
    TEST_ASSERT_TRUE(contains(buf, "</DataItems></Device></Devices></MTConnectDevices>"));
}

void test_devices_escape_and_overflow(void)
{
    char buf[1024];
    DetwsMtcStreams s;
    detws_mtc_devices_begin(&s, buf, sizeof(buf), 1, "d<1", "n&m", "u");
    detws_mtc_devices_add_item(&s, DETWS_MTC_EVENT, "i\"d", "T>y", nullptr, nullptr);
    TEST_ASSERT_TRUE(detws_mtc_devices_end(&s) > 0);
    TEST_ASSERT_TRUE(contains(buf, "id=\"d&lt;1\" name=\"n&amp;m\""));
    TEST_ASSERT_TRUE(contains(buf, "id=\"i&quot;d\" type=\"T&gt;y\""));
    // A device document that does not fit fails closed.
    char tiny[40];
    DetwsMtcStreams s2;
    detws_mtc_devices_begin(&s2, tiny, sizeof(tiny), 1, "dev", "device-with-a-very-long-name", "uuid");
    detws_mtc_devices_add_item(&s2, DETWS_MTC_SAMPLE, "xpos", "Position", "Xabs", "MILLIMETER");
    TEST_ASSERT_EQUAL_size_t(0, detws_mtc_devices_end(&s2));
}

void test_assets_document(void)
{
    char buf[1024];
    DetwsMtcStreams s;
    detws_mtc_assets_begin(&s, buf, sizeof(buf), 1500, 2, 1024);
    detws_mtc_assets_cutting_tool_begin(&s, "tool-1", "SN-42", "T17", "uuid-abc", "2026-07-09T00:00:00Z");
    detws_mtc_assets_tool_life(&s, "MINUTES", "DOWN", "100", "42");
    detws_mtc_assets_cutting_tool_end(&s);
    // A second tool with only the required assetId - optional attrs omitted.
    detws_mtc_assets_cutting_tool_begin(&s, "tool-2", nullptr, nullptr, nullptr, nullptr);
    detws_mtc_assets_tool_life(&s, "PART_COUNT", "UP", nullptr, "7");
    detws_mtc_assets_cutting_tool_end(&s);
    size_t n = detws_mtc_assets_end(&s);
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_size_t(strlen(buf), n);

    TEST_ASSERT_TRUE(contains(buf, "<MTConnectAssets"));
    TEST_ASSERT_TRUE(contains(buf, "instanceId=\"1500\" version=\"1.4\" assetBufferSize=\"1024\" assetCount=\"2\""));
    TEST_ASSERT_TRUE(contains(
        buf, "<Assets><CuttingTool assetId=\"tool-1\" serialNumber=\"SN-42\" toolId=\"T17\" deviceUuid=\"uuid-abc\" "
             "timestamp=\"2026-07-09T00:00:00Z\"><CuttingToolLifeCycle>"));
    TEST_ASSERT_TRUE(contains(buf, "<ToolLife type=\"MINUTES\" countDirection=\"DOWN\" limit=\"100\">42</ToolLife>"));
    TEST_ASSERT_TRUE(contains(buf, "</CuttingToolLifeCycle></CuttingTool>"));
    // Second tool: no optional attrs, no limit.
    TEST_ASSERT_TRUE(contains(buf, "<CuttingTool assetId=\"tool-2\"><CuttingToolLifeCycle>"));
    TEST_ASSERT_TRUE(contains(buf, "<ToolLife type=\"PART_COUNT\" countDirection=\"UP\">7</ToolLife>"));
    TEST_ASSERT_TRUE(contains(buf, "</Assets></MTConnectAssets>"));
}

void test_assets_escape_and_overflow(void)
{
    char buf[1024];
    DetwsMtcStreams s;
    detws_mtc_assets_begin(&s, buf, sizeof(buf), 1, 1, 8);
    detws_mtc_assets_cutting_tool_begin(&s, "a<1", "s&n", nullptr, nullptr, nullptr);
    detws_mtc_assets_tool_life(&s, "WEAR", "UP", nullptr, "1>2");
    detws_mtc_assets_cutting_tool_end(&s);
    TEST_ASSERT_TRUE(detws_mtc_assets_end(&s) > 0);
    TEST_ASSERT_TRUE(contains(buf, "assetId=\"a&lt;1\" serialNumber=\"s&amp;n\""));
    TEST_ASSERT_TRUE(contains(buf, ">1&gt;2</ToolLife>"));
    // An asset document that does not fit fails closed.
    char tiny[40];
    DetwsMtcStreams s2;
    detws_mtc_assets_begin(&s2, tiny, sizeof(tiny), 1, 1, 8);
    detws_mtc_assets_cutting_tool_begin(&s2, "a-tool-with-a-very-long-asset-id", nullptr, nullptr, nullptr, nullptr);
    detws_mtc_assets_cutting_tool_end(&s2);
    TEST_ASSERT_EQUAL_size_t(0, detws_mtc_assets_end(&s2));
}

void test_sample_buffer_and_query(void)
{
    DetwsMtcSampleBuffer b;
    detws_mtc_sample_buffer_init(&b, 1);
    TEST_ASSERT_EQUAL_UINT64(1, detws_mtc_sample_buffer_add(&b, DETWS_MTC_SAMPLE, "Position", "xpos", "T1", "1.0"));
    TEST_ASSERT_EQUAL_UINT64(2, detws_mtc_sample_buffer_add(&b, DETWS_MTC_SAMPLE, "Position", "xpos", "T2", "2.0"));
    TEST_ASSERT_EQUAL_UINT64(3, detws_mtc_sample_buffer_add(&b, DETWS_MTC_EVENT, "Execution", "exec", "T3", "ACTIVE"));

    char buf[1024];
    size_t n = detws_mtc_sample_query(&b, buf, sizeof(buf), 1500, "cnc1", 1, 10);
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_size_t(strlen(buf), n);
    // Full sample-cursor header.
    TEST_ASSERT_TRUE(contains(buf, "instanceId=\"1500\" version=\"1.4\" bufferSize=\""));
    TEST_ASSERT_TRUE(contains(buf, "firstSequence=\"1\" lastSequence=\"3\" nextSequence=\"4\""));
    TEST_ASSERT_TRUE(contains(buf, "<Position dataItemId=\"xpos\" sequence=\"1\" timestamp=\"T1\">1.0</Position>"));
    TEST_ASSERT_TRUE(
        contains(buf, "<Execution dataItemId=\"exec\" sequence=\"3\" timestamp=\"T3\">ACTIVE</Execution>"));

    // A windowed request: one observation from sequence 2, so nextSequence advances to 3.
    n = detws_mtc_sample_query(&b, buf, sizeof(buf), 1500, "cnc1", 2, 1);
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_TRUE(contains(buf, "firstSequence=\"1\" lastSequence=\"3\" nextSequence=\"3\""));
    TEST_ASSERT_TRUE(contains(buf, "sequence=\"2\" timestamp=\"T2\">2.0</Position>"));
    TEST_ASSERT_FALSE(contains(buf, "timestamp=\"T3\"")); // count=1 stopped before seq 3
}

void test_sample_buffer_eviction(void)
{
    DetwsMtcSampleBuffer b;
    detws_mtc_sample_buffer_init(&b, 1);
    // Overfill the ring: DETWS_MTC_SAMPLE_BUFFER + 8 observations, so the oldest 8 are evicted.
    const uint32_t total = DETWS_MTC_SAMPLE_BUFFER + 8;
    for (uint32_t i = 1; i <= total; i++)
    {
        char ts[16];
        int p = 0;
        ts[p++] = 'T';
        // tiny uint->decimal
        char d[12];
        int q = 0;
        uint32_t v = i;
        do
        {
            d[q++] = (char)('0' + v % 10);
            v /= 10;
        } while (v);
        while (q > 0)
            ts[p++] = d[--q];
        ts[p] = '\0';
        detws_mtc_sample_buffer_add(&b, DETWS_MTC_SAMPLE, "Position", "xpos", ts, "9.9");
    }
    // Retained window is [total-BUFFER+1, total]; first advanced by 8.
    char buf[8192];
    size_t n = detws_mtc_sample_query(&b, buf, sizeof(buf), 1, "d", 1 /* stale -> clamps up */, 1000);
    TEST_ASSERT_TRUE(n > 0);
    char expect[96];
    snprintf(expect, sizeof(expect), "firstSequence=\"%u\" lastSequence=\"%u\" nextSequence=\"%u\"",
             (unsigned)(total - DETWS_MTC_SAMPLE_BUFFER + 1), (unsigned)total, (unsigned)(total + 1));
    TEST_ASSERT_TRUE(contains(buf, expect));
    // The oldest kept observation carries the sliding first sequence, not sequence 1.
    char oldest[48];
    snprintf(oldest, sizeof(oldest), "sequence=\"%u\"", (unsigned)(total - DETWS_MTC_SAMPLE_BUFFER + 1));
    TEST_ASSERT_TRUE(contains(buf, oldest));
    TEST_ASSERT_FALSE(contains(buf, "sequence=\"1\"")); // evicted
}

void test_sample_query_future_and_empty(void)
{
    DetwsMtcSampleBuffer b;
    detws_mtc_sample_buffer_init(&b, 5);
    detws_mtc_sample_buffer_add(&b, DETWS_MTC_SAMPLE, "Position", "xpos", "T", "1.0"); // seq 5

    char buf[512];
    // A `from` past the newest yields no observations and nextSequence = the buffer's next (6).
    size_t n = detws_mtc_sample_query(&b, buf, sizeof(buf), 1, "d", 100, 10);
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_TRUE(contains(buf, "firstSequence=\"5\" lastSequence=\"5\" nextSequence=\"6\""));
    TEST_ASSERT_FALSE(contains(buf, "<Position"));

    // An empty buffer answers with lastSequence = first-1 and no observations.
    DetwsMtcSampleBuffer e;
    detws_mtc_sample_buffer_init(&e, 1);
    n = detws_mtc_sample_query(&e, buf, sizeof(buf), 1, "d", 1, 10);
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_TRUE(contains(buf, "firstSequence=\"1\" lastSequence=\"0\" nextSequence=\"1\""));

    // Fail-closed on a buffer too small for even the header.
    char tiny[32];
    TEST_ASSERT_EQUAL_size_t(0, detws_mtc_sample_query(&b, tiny, sizeof(tiny), 1, "device-name", 5, 10));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_streams_document);
    RUN_TEST(test_streams_escapes_value);
    RUN_TEST(test_error_document);
    RUN_TEST(test_overflow_returns_zero);
    RUN_TEST(test_escape_gt_quote_and_overflow);
    RUN_TEST(test_devices_probe_document);
    RUN_TEST(test_devices_escape_and_overflow);
    RUN_TEST(test_assets_document);
    RUN_TEST(test_assets_escape_and_overflow);
    RUN_TEST(test_sample_buffer_and_query);
    RUN_TEST(test_sample_buffer_eviction);
    RUN_TEST(test_sample_query_future_and_empty);
    return UNITY_END();
}
