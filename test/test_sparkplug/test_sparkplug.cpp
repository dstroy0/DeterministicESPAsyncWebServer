// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the Sparkplug B codec (services/sparkplug): the topic builder, the Metric
// serializer (exact protobuf bytes), and a Payload round-trip read with the protobuf cursor.
// Field numbers per the Eclipse Tahu sparkplug_b.proto. Pure host tests.

#include "services/protobuf/protobuf.h"
#include "services/sparkplug/sparkplug.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_topic()
{
    char buf[64];
    size_t n = spb_build_topic(buf, sizeof(buf), "group1", "NDATA", "edge1", nullptr);
    TEST_ASSERT_EQUAL_STRING("spBv1.0/group1/NDATA/edge1", buf);
    TEST_ASSERT_EQUAL_size_t(strlen(buf), n);

    n = spb_build_topic(buf, sizeof(buf), "group1", "DDATA", "edge1", "dev1");
    TEST_ASSERT_EQUAL_STRING("spBv1.0/group1/DDATA/edge1/dev1", buf);
    TEST_ASSERT_GREATER_THAN(0, (int)n);
}

// A double metric "temperature" = 23.5 -> exact Tahu Metric protobuf bytes.
void test_metric_bytes()
{
    SpbMetric m = {};
    m.name = "temperature";
    m.datatype = SPB_DT_DOUBLE;
    m.kind = SPB_M_DOUBLE;
    m.double_value = 23.5;
    uint8_t buf[64];
    size_t n = spb_build_metric(buf, sizeof(buf), &m);
    const uint8_t expect[] = {
        0x0A, 0x0B, 't',  'e',  'm',  'p',  'e',  'r',  'a', 't', 'u', 'r', 'e', // name (field 1)
        0x20, 0x0A,                                                              // datatype (field 4) = 10 (Double)
        0x69, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x37, 0x40                     // double_value (field 13) = 23.5 LE
    };
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);
}

// Build a Payload and walk it back with the protobuf cursor.
void test_payload_round_trip()
{
    SpbMetric m = {};
    m.name = "temperature";
    m.datatype = SPB_DT_DOUBLE;
    m.kind = SPB_M_DOUBLE;
    m.double_value = 23.5;
    uint8_t buf[128];
    size_t n = spb_build_payload(buf, sizeof(buf), 1000, 5, &m, 1);
    TEST_ASSERT_GREATER_THAN(0, (int)n);

    size_t pos = 0;
    PbField f;
    // field 1: timestamp.
    TEST_ASSERT_TRUE(pb_read_field(buf, n, &pos, &f));
    TEST_ASSERT_EQUAL_UINT32(1, f.field_number);
    TEST_ASSERT_EQUAL_UINT64(1000, f.value);
    // field 2: the metric submessage.
    TEST_ASSERT_TRUE(pb_read_field(buf, n, &pos, &f));
    TEST_ASSERT_EQUAL_UINT32(2, f.field_number);
    TEST_ASSERT_EQUAL_UINT8(PB_WT_LEN, f.wire_type);
    const uint8_t *metric = f.data;
    size_t metric_len = f.len;
    // field 3: seq.
    TEST_ASSERT_TRUE(pb_read_field(buf, n, &pos, &f));
    TEST_ASSERT_EQUAL_UINT32(3, f.field_number);
    TEST_ASSERT_EQUAL_UINT64(5, f.value);

    // Walk the metric: name (1), datatype (4), double_value (13).
    size_t mp = 0;
    TEST_ASSERT_TRUE(pb_read_field(metric, metric_len, &mp, &f));
    TEST_ASSERT_EQUAL_UINT32(1, f.field_number);
    TEST_ASSERT_EQUAL_MEMORY("temperature", f.data, f.len);
    TEST_ASSERT_TRUE(pb_read_field(metric, metric_len, &mp, &f));
    TEST_ASSERT_EQUAL_UINT32(4, f.field_number);
    TEST_ASSERT_EQUAL_UINT64(SPB_DT_DOUBLE, f.value);
    TEST_ASSERT_TRUE(pb_read_field(metric, metric_len, &mp, &f));
    TEST_ASSERT_EQUAL_UINT32(13, f.field_number);
    TEST_ASSERT_EQUAL_HEX64(0x4037800000000000ULL, f.value); // bits of 23.5
    TEST_ASSERT_TRUE(pb_double_bits(f.value) == 23.5);
}

void test_metric_int_and_string()
{
    SpbMetric mi = {};
    mi.name = "count";
    mi.datatype = SPB_DT_INT32;
    mi.kind = SPB_M_INT;
    mi.int_value = 42;
    uint8_t buf[64];
    size_t n = spb_build_metric(buf, sizeof(buf), &mi);
    size_t pos = 0;
    PbField f;
    // skip name + datatype, read the int value (field 10).
    pb_read_field(buf, n, &pos, &f); // name
    pb_read_field(buf, n, &pos, &f); // datatype
    TEST_ASSERT_TRUE(pb_read_field(buf, n, &pos, &f));
    TEST_ASSERT_EQUAL_UINT32(10, f.field_number);
    TEST_ASSERT_EQUAL_UINT64(42, f.value);

    SpbMetric ms = {};
    ms.name = "status";
    ms.datatype = SPB_DT_STRING;
    ms.kind = SPB_M_STRING;
    ms.string_value = "ok";
    n = spb_build_metric(buf, sizeof(buf), &ms);
    pos = 0;
    pb_read_field(buf, n, &pos, &f); // name
    pb_read_field(buf, n, &pos, &f); // datatype
    TEST_ASSERT_TRUE(pb_read_field(buf, n, &pos, &f));
    TEST_ASSERT_EQUAL_UINT32(15, f.field_number); // string_value
    TEST_ASSERT_EQUAL_MEMORY("ok", f.data, f.len);
}

// A metric carrying an alias instead of a name (DATA messages after birth).
void test_metric_alias()
{
    SpbMetric m = {};
    m.has_alias = true;
    m.alias = 7;
    m.datatype = SPB_DT_BOOLEAN;
    m.kind = SPB_M_BOOL;
    m.bool_value = true;
    uint8_t buf[32];
    size_t n = spb_build_metric(buf, sizeof(buf), &m);
    size_t pos = 0;
    PbField f;
    TEST_ASSERT_TRUE(pb_read_field(buf, n, &pos, &f));
    TEST_ASSERT_EQUAL_UINT32(2, f.field_number); // alias (no name field)
    TEST_ASSERT_EQUAL_UINT64(7, f.value);
}

void test_overflow_fails_closed()
{
    SpbMetric m = {};
    m.name = "temperature";
    m.datatype = SPB_DT_DOUBLE;
    m.kind = SPB_M_DOUBLE;
    m.double_value = 23.5;
    uint8_t small[8];
    TEST_ASSERT_EQUAL_size_t(0, spb_build_metric(small, sizeof(small), &m));
    char tsmall[8];
    TEST_ASSERT_EQUAL_size_t(0, spb_build_topic(tsmall, sizeof(tsmall), "group1", "NDATA", "edge1", nullptr));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_topic);
    RUN_TEST(test_metric_bytes);
    RUN_TEST(test_payload_round_trip);
    RUN_TEST(test_metric_int_and_string);
    RUN_TEST(test_metric_alias);
    RUN_TEST(test_overflow_fails_closed);
    return UNITY_END();
}
