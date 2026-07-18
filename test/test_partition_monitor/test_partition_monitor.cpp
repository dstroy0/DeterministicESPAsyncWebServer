// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the partition-map core (services/partition_monitor): the
// type/subtype -> kind classifier and the JSON serializer. The flash walk is
// ESP32-only and a no-op on the host.

#include "services/partition_monitor/partition_monitor.h"
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_kind_app()
{
    TEST_ASSERT_EQUAL_STRING("factory", dws_partition_kind(0, 0x00));
    TEST_ASSERT_EQUAL_STRING("ota", dws_partition_kind(0, 0x10));
    TEST_ASSERT_EQUAL_STRING("ota", dws_partition_kind(0, 0x11));
    TEST_ASSERT_EQUAL_STRING("test", dws_partition_kind(0, 0x20));
    TEST_ASSERT_EQUAL_STRING("app", dws_partition_kind(0, 0x05)); // app, not a known subtype
}

void test_kind_data()
{
    TEST_ASSERT_EQUAL_STRING("otadata", dws_partition_kind(1, 0x00));
    TEST_ASSERT_EQUAL_STRING("phy", dws_partition_kind(1, 0x01));
    TEST_ASSERT_EQUAL_STRING("nvs", dws_partition_kind(1, 0x02));
    TEST_ASSERT_EQUAL_STRING("coredump", dws_partition_kind(1, 0x03));
    TEST_ASSERT_EQUAL_STRING("littlefs", dws_partition_kind(1, 0x83));
    TEST_ASSERT_EQUAL_STRING("spiffs", dws_partition_kind(1, 0x82));
    TEST_ASSERT_EQUAL_STRING("data", dws_partition_kind(1, 0x77)); // unknown data subtype
}

void test_json()
{
    DWSPartitionInfo p[2] = {
        {"nvs", 1, 0x02, 0x9000, 0x6000, false},
        {"app0", 0, 0x10, 0x10000, 0x140000, true},
    };
    char buf[512];
    int n = dws_partition_json(p, 2, buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_STRING(
        "{\"partitions\":[{\"label\":\"nvs\",\"kind\":\"nvs\",\"type\":1,\"subtype\":2,\"addr\":36864,\"size\":24576,"
        "\"running\":false},"
        "{\"label\":\"app0\",\"kind\":\"ota\",\"type\":0,\"subtype\":16,\"addr\":65536,\"size\":1310720,"
        "\"running\":true}]}",
        buf);
}

void test_json_small_buffer_fails_closed()
{
    DWSPartitionInfo p[1] = {{"nvs", 1, 0x02, 0x9000, 0x6000, false}};
    char buf[8];
    TEST_ASSERT_EQUAL_INT(0, dws_partition_json(p, 1, buf, sizeof(buf)));
}

void test_collect_host_stub()
{
    DWSPartitionInfo p[4];
    TEST_ASSERT_EQUAL_UINT8(0, dws_partition_collect(p, 4));
}

void test_partition_kind_data_subtypes()
{
    TEST_ASSERT_EQUAL_STRING("nvs_keys", dws_partition_kind(0x01, 0x04));
    TEST_ASSERT_EQUAL_STRING("fat", dws_partition_kind(0x01, 0x81));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_kind_app);
    RUN_TEST(test_kind_data);
    RUN_TEST(test_json);
    RUN_TEST(test_json_small_buffer_fails_closed);
    RUN_TEST(test_collect_host_stub);
    RUN_TEST(test_partition_kind_data_subtypes);
    return UNITY_END();
}
