// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the SenML (RFC 8428) pack builders (services/senml): SenML-JSON (exact
// string assertions, anchored on the RFC example) and SenML-CBOR (read back with the CBOR
// reader). Pure host tests.

#include "network_drivers/presentation/cbor/cbor.h"
#include "services/senml/senml.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// The RFC 8428 single-record example.
void test_json_canonical()
{
    SenmlRecord r = {};
    r.base_name = "urn:dev:ow:10e2073a01080063";
    r.unit = "Cel";
    r.value_kind = SENML_V_FLOAT;
    r.value = 23.1;
    char buf[128];
    size_t n = senml_json_build(buf, sizeof(buf), &r, 1);
    TEST_ASSERT_GREATER_THAN(0, (int)n);
    TEST_ASSERT_EQUAL_STRING("[{\"bn\":\"urn:dev:ow:10e2073a01080063\",\"u\":\"Cel\",\"v\":23.1}]", buf);
}

// A multi-record pack: a base name on the first record, plain names after.
void test_json_multi_record()
{
    SenmlRecord r[2] = {};
    r[0].base_name = "urn:dev:ow:1;";
    r[0].name = "voltage";
    r[0].unit = "V";
    r[0].value_kind = SENML_V_FLOAT;
    r[0].value = 120.1;
    r[1].name = "current";
    r[1].unit = "A";
    r[1].value_kind = SENML_V_FLOAT;
    r[1].value = 1.25;
    char buf[160];
    size_t n = senml_json_build(buf, sizeof(buf), r, 2);
    TEST_ASSERT_GREATER_THAN(0, (int)n);
    TEST_ASSERT_EQUAL_STRING(
        "[{\"bn\":\"urn:dev:ow:1;\",\"n\":\"voltage\",\"u\":\"V\",\"v\":120.1},{\"n\":\"current\",\"u\":\"A\",\"v\":1."
        "25}]",
        buf);
}

// String / boolean values, and an integral time emitted as an integer (full precision).
void test_json_string_bool_time()
{
    SenmlRecord rs = {};
    rs.name = "status";
    rs.value_kind = SENML_V_STRING;
    rs.value_str = "ok";
    rs.has_time = true;
    rs.time = 1600000000; // integral -> "1600000000", not "1.6e+09"
    char buf[128];
    TEST_ASSERT_GREATER_THAN(0, (int)senml_json_build(buf, sizeof(buf), &rs, 1));
    TEST_ASSERT_EQUAL_STRING("[{\"n\":\"status\",\"vs\":\"ok\",\"t\":1600000000}]", buf);

    SenmlRecord rb = {};
    rb.name = "open";
    rb.value_kind = SENML_V_BOOL;
    rb.value_bool = true;
    TEST_ASSERT_GREATER_THAN(0, (int)senml_json_build(buf, sizeof(buf), &rb, 1));
    TEST_ASSERT_EQUAL_STRING("[{\"n\":\"open\",\"vb\":true}]", buf);
}

// SenML-CBOR: a CBOR array of integer-keyed maps; read it back with the CBOR reader.
void test_cbor_round_trip()
{
    SenmlRecord r = {};
    r.name = "temp";
    r.unit = "Cel";
    r.value_kind = SENML_V_FLOAT;
    r.value = 42; // integral -> emitted as a CBOR integer
    uint8_t buf[64];
    size_t n = senml_cbor_build(buf, sizeof(buf), &r, 1);
    TEST_ASSERT_GREATER_THAN(0, (int)n);

    CborReader rd;
    cbor_reader_init(&rd, buf, n);
    size_t arr;
    TEST_ASSERT_TRUE(cbor_read_array(&rd, &arr));
    TEST_ASSERT_EQUAL_size_t(1, arr);
    size_t fields;
    TEST_ASSERT_TRUE(cbor_read_map(&rd, &fields));
    TEST_ASSERT_EQUAL_size_t(3, fields);

    int64_t key;
    const char *s;
    size_t slen;
    TEST_ASSERT_TRUE(cbor_read_int(&rd, &key));
    TEST_ASSERT_EQUAL_INT64(0, key); // n
    TEST_ASSERT_TRUE(cbor_read_text(&rd, &s, &slen));
    TEST_ASSERT_EQUAL_MEMORY("temp", s, slen);

    TEST_ASSERT_TRUE(cbor_read_int(&rd, &key));
    TEST_ASSERT_EQUAL_INT64(1, key); // u
    TEST_ASSERT_TRUE(cbor_read_text(&rd, &s, &slen));
    TEST_ASSERT_EQUAL_MEMORY("Cel", s, slen);

    TEST_ASSERT_TRUE(cbor_read_int(&rd, &key));
    TEST_ASSERT_EQUAL_INT64(2, key); // v
    int64_t v;
    TEST_ASSERT_TRUE(cbor_read_int(&rd, &v));
    TEST_ASSERT_EQUAL_INT64(42, v);
    TEST_ASSERT_TRUE(cbor_reader_ok(&rd));
}

// A negative base-label is encoded (bn = -2) and reads back as a negative key.
void test_cbor_base_name_key()
{
    SenmlRecord r = {};
    r.base_name = "dev1";
    r.value_kind = SENML_V_NONE;
    uint8_t buf[32];
    size_t n = senml_cbor_build(buf, sizeof(buf), &r, 1);
    CborReader rd;
    cbor_reader_init(&rd, buf, n);
    size_t arr, fields;
    TEST_ASSERT_TRUE(cbor_read_array(&rd, &arr));
    TEST_ASSERT_TRUE(cbor_read_map(&rd, &fields));
    TEST_ASSERT_EQUAL_size_t(1, fields);
    int64_t key;
    TEST_ASSERT_TRUE(cbor_read_int(&rd, &key));
    TEST_ASSERT_EQUAL_INT64(-2, key); // bn
}

void test_overflow_fails_closed()
{
    SenmlRecord r = {};
    r.base_name = "urn:dev:ow:10e2073a01080063";
    r.unit = "Cel";
    r.value_kind = SENML_V_FLOAT;
    r.value = 23.1;
    char small[16];
    TEST_ASSERT_EQUAL_size_t(0, senml_json_build(small, sizeof(small), &r, 1));
    uint8_t csmall[4];
    TEST_ASSERT_EQUAL_size_t(0, senml_cbor_build(csmall, sizeof(csmall), &r, 1));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_json_canonical);
    RUN_TEST(test_json_multi_record);
    RUN_TEST(test_json_string_bool_time);
    RUN_TEST(test_cbor_round_trip);
    RUN_TEST(test_cbor_base_name_key);
    RUN_TEST(test_overflow_fails_closed);
    return UNITY_END();
}
