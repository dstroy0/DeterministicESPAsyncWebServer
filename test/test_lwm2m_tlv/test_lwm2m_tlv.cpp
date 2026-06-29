// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the OMA LwM2M TLV codec (services/lwm2m): the writer (raw + typed value
// helpers), the cursor reader, and integer value decoding. Byte vectors checked against
// the LwM2M TLV type-byte layout. Pure host tests.

#include "services/lwm2m/lwm2m_tlv.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// A single-octet integer Resource: type 0xC1, id, value.
void test_write_int_1byte()
{
    uint8_t buf[16];
    Lwm2mTlvWriter w;
    lwm2m_tlv_init(&w, buf, sizeof(buf));
    lwm2m_tlv_write_int(&w, 3, 42);
    size_t n = lwm2m_tlv_finish(&w);
    const uint8_t expect[] = {0xC1, 0x03, 0x2A};
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);
}

// A value > 127 takes two octets (type length field = 2).
void test_write_int_2byte()
{
    uint8_t buf[16];
    Lwm2mTlvWriter w;
    lwm2m_tlv_init(&w, buf, sizeof(buf));
    lwm2m_tlv_write_int(&w, 1, 300);
    size_t n = lwm2m_tlv_finish(&w);
    const uint8_t expect[] = {0xC2, 0x01, 0x01, 0x2C}; // 300 = 0x012C
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);
}

// The spec's manufacturer example: resource 0 = "Open Mobile Alliance" -> C8 00 14 ...
void test_write_string_8bit_length()
{
    uint8_t buf[64];
    Lwm2mTlvWriter w;
    lwm2m_tlv_init(&w, buf, sizeof(buf));
    lwm2m_tlv_write_string(&w, 0, "Open Mobile Alliance");
    size_t n = lwm2m_tlv_finish(&w);
    TEST_ASSERT_EQUAL_HEX8(0xC8, buf[0]); // Resource, 8-bit id, 8-bit length
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[1]); // id 0
    TEST_ASSERT_EQUAL_HEX8(0x14, buf[2]); // length 20
    TEST_ASSERT_EQUAL_MEMORY("Open Mobile Alliance", buf + 3, 20);
    TEST_ASSERT_EQUAL_size_t(23, n);
}

// A 16-bit identifier sets type bit 5 and writes two id octets.
void test_write_16bit_id()
{
    uint8_t buf[16];
    Lwm2mTlvWriter w;
    lwm2m_tlv_init(&w, buf, sizeof(buf));
    lwm2m_tlv_write_int(&w, 0x0405, 1);
    size_t n = lwm2m_tlv_finish(&w);
    const uint8_t expect[] = {0xE1, 0x04, 0x05, 0x01}; // 0xC0 | 0x20(id16) | len 1
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);
}

void test_round_trip_and_value_int()
{
    uint8_t buf[64];
    Lwm2mTlvWriter w;
    lwm2m_tlv_init(&w, buf, sizeof(buf));
    lwm2m_tlv_write_int(&w, 0, 42);
    lwm2m_tlv_write_int(&w, 1, 300);
    lwm2m_tlv_write_int(&w, 2, -1);
    lwm2m_tlv_write_bool(&w, 3, true);
    lwm2m_tlv_write_string(&w, 4, "hi");
    size_t total = lwm2m_tlv_finish(&w);
    TEST_ASSERT_GREATER_THAN(0, (int)total);

    size_t pos = 0;
    Lwm2mTlv t;
    int64_t v;

    TEST_ASSERT_TRUE(lwm2m_tlv_read(buf, total, &pos, &t));
    TEST_ASSERT_EQUAL_UINT8(LWM2M_TLV_RESOURCE, t.id_type);
    TEST_ASSERT_EQUAL_UINT16(0, t.id);
    TEST_ASSERT_TRUE(lwm2m_tlv_value_int(t.value, t.value_len, &v));
    TEST_ASSERT_EQUAL_INT64(42, v);

    TEST_ASSERT_TRUE(lwm2m_tlv_read(buf, total, &pos, &t));
    TEST_ASSERT_EQUAL_UINT16(1, t.id);
    TEST_ASSERT_TRUE(lwm2m_tlv_value_int(t.value, t.value_len, &v));
    TEST_ASSERT_EQUAL_INT64(300, v);

    TEST_ASSERT_TRUE(lwm2m_tlv_read(buf, total, &pos, &t));
    TEST_ASSERT_EQUAL_UINT16(2, t.id);
    TEST_ASSERT_TRUE(lwm2m_tlv_value_int(t.value, t.value_len, &v));
    TEST_ASSERT_EQUAL_INT64(-1, v); // negative sign-extends

    TEST_ASSERT_TRUE(lwm2m_tlv_read(buf, total, &pos, &t));
    TEST_ASSERT_EQUAL_UINT16(3, t.id);
    TEST_ASSERT_EQUAL_size_t(1, t.value_len);
    TEST_ASSERT_EQUAL_HEX8(0x01, t.value[0]);

    TEST_ASSERT_TRUE(lwm2m_tlv_read(buf, total, &pos, &t));
    TEST_ASSERT_EQUAL_UINT16(4, t.id);
    TEST_ASSERT_EQUAL_MEMORY("hi", t.value, 2);

    TEST_ASSERT_FALSE(lwm2m_tlv_read(buf, total, &pos, &t)); // end
}

// An Object Instance wrapping nested Resource TLVs (the writer emits raw nested bytes).
void test_object_instance_nested()
{
    uint8_t inner[16];
    Lwm2mTlvWriter iw;
    lwm2m_tlv_init(&iw, inner, sizeof(inner));
    lwm2m_tlv_write_int(&iw, 0, 1);
    lwm2m_tlv_write_int(&iw, 1, 2);
    size_t inner_len = lwm2m_tlv_finish(&iw);

    uint8_t buf[32];
    Lwm2mTlvWriter w;
    lwm2m_tlv_init(&w, buf, sizeof(buf));
    lwm2m_tlv_write(&w, LWM2M_TLV_OBJECT_INSTANCE, 0, inner, inner_len);
    size_t total = lwm2m_tlv_finish(&w);

    size_t pos = 0;
    Lwm2mTlv t;
    TEST_ASSERT_TRUE(lwm2m_tlv_read(buf, total, &pos, &t));
    TEST_ASSERT_EQUAL_UINT8(LWM2M_TLV_OBJECT_INSTANCE, t.id_type);
    TEST_ASSERT_EQUAL_size_t(inner_len, t.value_len);
    // The Object Instance value is itself a TLV stream.
    size_t ipos = 0;
    Lwm2mTlv it;
    TEST_ASSERT_TRUE(lwm2m_tlv_read(t.value, t.value_len, &ipos, &it));
    TEST_ASSERT_EQUAL_UINT16(0, it.id);
    TEST_ASSERT_TRUE(lwm2m_tlv_read(t.value, t.value_len, &ipos, &it));
    TEST_ASSERT_EQUAL_UINT16(1, it.id);
}

void test_overflow_and_malformed()
{
    uint8_t small[2];
    Lwm2mTlvWriter w;
    lwm2m_tlv_init(&w, small, sizeof(small));
    lwm2m_tlv_write_string(&w, 0, "too big for two bytes");
    TEST_ASSERT_EQUAL_size_t(0, lwm2m_tlv_finish(&w));

    // A TLV declaring more value than is buffered.
    const uint8_t trunc[] = {0xC8, 0x00, 0x14, 'a', 'b'}; // says length 20, only 2 follow
    size_t pos = 0;
    Lwm2mTlv t;
    TEST_ASSERT_FALSE(lwm2m_tlv_read(trunc, sizeof(trunc), &pos, &t));

    int64_t v;
    const uint8_t three[] = {1, 2, 3};
    TEST_ASSERT_FALSE(lwm2m_tlv_value_int(three, 3, &v)); // 3 octets is not a valid int width
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_write_int_1byte);
    RUN_TEST(test_write_int_2byte);
    RUN_TEST(test_write_string_8bit_length);
    RUN_TEST(test_write_16bit_id);
    RUN_TEST(test_round_trip_and_value_int);
    RUN_TEST(test_object_instance_nested);
    RUN_TEST(test_overflow_and_malformed);
    return UNITY_END();
}
