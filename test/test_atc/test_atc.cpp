// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/atc: the ATC field-I/O interop snapshot.

#include "services/atc/atc.h"
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

void test_snapshot_json(void)
{
    AtcPoint pts[3] = {
        {"det.1", false, 1},        // input
        {"phase.2.green", true, 0}, // output
        {"det.2", false, 0},        // input
    };
    AtcFieldIo io = {pts, 3};
    char buf[256];
    size_t n = detws_atc_snapshot_json(&io, buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_size_t(strlen(buf), n);
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"inputs\":[{\"name\":\"det.1\",\"value\":1}"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "{\"name\":\"det.2\",\"value\":0}"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"outputs\":[{\"name\":\"phase.2.green\",\"value\":0}]"));
}

void test_set_output(void)
{
    AtcPoint pts[2] = {{"det.1", false, 0}, {"phase.1.green", true, 0}};
    AtcFieldIo io = {pts, 2};
    // Set an output.
    TEST_ASSERT_TRUE(detws_atc_set_output(&io, "phase.1.green", 1));
    TEST_ASSERT_EQUAL_UINT8(1, pts[1].value);
    // Cannot set an input.
    TEST_ASSERT_FALSE(detws_atc_set_output(&io, "det.1", 1));
    // Unknown name.
    TEST_ASSERT_FALSE(detws_atc_set_output(&io, "nope", 1));
}

void test_get(void)
{
    AtcPoint pts[2] = {{"det.1", false, 42}, {"out.1", true, 7}};
    AtcFieldIo io = {pts, 2};
    bool found = false;
    TEST_ASSERT_EQUAL_UINT8(42, detws_atc_get(&io, "det.1", &found));
    TEST_ASSERT_TRUE(found);
    detws_atc_get(&io, "missing", &found);
    TEST_ASSERT_FALSE(found);
}

void test_empty_and_overflow(void)
{
    AtcFieldIo io = {nullptr, 0};
    char buf[64];
    size_t n = detws_atc_snapshot_json(&io, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("{\"inputs\":[],\"outputs\":[]}", buf);
    TEST_ASSERT_EQUAL_size_t(strlen("{\"inputs\":[],\"outputs\":[]}"), n);

    AtcPoint pts[1] = {{"a-long-input-name", false, 1}};
    AtcFieldIo io2 = {pts, 1};
    char small[16];
    TEST_ASSERT_EQUAL_size_t(0, detws_atc_snapshot_json(&io2, small, sizeof(small)));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_snapshot_json);
    RUN_TEST(test_set_output);
    RUN_TEST(test_get);
    RUN_TEST(test_empty_and_overflow);
    return UNITY_END();
}
