// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the AMQP 0-9-1 frame codec (services/amqp): the protocol header, the frame
// and method builders, the heartbeat, and the frame/method parsers. Pure host tests.

#include "services/amqp/amqp.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_protocol_header()
{
    uint8_t buf[8];
    size_t n = amqp_protocol_header(buf, sizeof(buf));
    const uint8_t expect[] = {'A', 'M', 'Q', 'P', 0, 0, 9, 1};
    TEST_ASSERT_EQUAL_size_t(8, n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, 8);
}

// A Connection.Start method (class 10, method 10) on channel 0.
void test_build_method_bytes()
{
    const uint8_t args[] = {0x00};
    uint8_t buf[32];
    size_t n = amqp_build_method(buf, sizeof(buf), 1, 10, 10, args, sizeof(args));
    const uint8_t expect[] = {
        0x01,                   // type METHOD
        0x00, 0x01,             // channel 1
        0x00, 0x00, 0x00, 0x05, // size 5
        0x00, 0x0A, 0x00, 0x0A, // class 10, method 10
        0x00,                   // args
        0xCE                    // frame-end
    };
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);
}

void test_method_round_trip()
{
    const uint8_t args[] = {0x0A, 0x0B, 0x0C};
    uint8_t buf[32];
    size_t n = amqp_build_method(buf, sizeof(buf), 7, 60, 40, args, sizeof(args)); // Basic.Publish-ish

    AmqpFrame f;
    size_t consumed;
    TEST_ASSERT_TRUE(amqp_parse_frame(buf, n, &f, &consumed));
    TEST_ASSERT_EQUAL_HEX8(AMQP_FRAME_METHOD, f.type);
    TEST_ASSERT_EQUAL_UINT16(7, f.channel);
    TEST_ASSERT_EQUAL_size_t(n, consumed);

    uint16_t cls, meth;
    const uint8_t *a;
    size_t alen;
    TEST_ASSERT_TRUE(amqp_parse_method(f.payload, f.payload_len, &cls, &meth, &a, &alen));
    TEST_ASSERT_EQUAL_UINT16(60, cls);
    TEST_ASSERT_EQUAL_UINT16(40, meth);
    TEST_ASSERT_EQUAL_size_t(sizeof(args), alen);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(args, a, sizeof(args));
}

void test_heartbeat()
{
    uint8_t buf[16];
    size_t n = amqp_build_heartbeat(buf, sizeof(buf));
    const uint8_t expect[] = {0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCE};
    TEST_ASSERT_EQUAL_size_t(8, n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);

    AmqpFrame f;
    size_t consumed;
    TEST_ASSERT_TRUE(amqp_parse_frame(buf, n, &f, &consumed));
    TEST_ASSERT_EQUAL_HEX8(AMQP_FRAME_HEARTBEAT, f.type);
    TEST_ASSERT_EQUAL_size_t(0, f.payload_len);
}

// A stream of two frames parses one at a time via the consumed cursor.
void test_parse_stream()
{
    uint8_t buf[64];
    size_t n = amqp_build_method(buf, sizeof(buf), 1, 10, 11, nullptr, 0);
    n += amqp_build_heartbeat(buf + n, sizeof(buf) - n);

    size_t pos = 0;
    AmqpFrame f;
    size_t c;
    TEST_ASSERT_TRUE(amqp_parse_frame(buf + pos, n - pos, &f, &c));
    TEST_ASSERT_EQUAL_HEX8(AMQP_FRAME_METHOD, f.type);
    pos += c;
    TEST_ASSERT_TRUE(amqp_parse_frame(buf + pos, n - pos, &f, &c));
    TEST_ASSERT_EQUAL_HEX8(AMQP_FRAME_HEARTBEAT, f.type);
    pos += c;
    TEST_ASSERT_EQUAL_size_t(n, pos);
}

void test_parse_rejects_bad()
{
    AmqpFrame f;
    size_t c;
    // A frame whose end octet is not 0xCE.
    const uint8_t bad_end[] = {0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0xAA, 0xFF};
    TEST_ASSERT_FALSE(amqp_parse_frame(bad_end, sizeof(bad_end), &f, &c));
    // Truncated (declares 5 payload octets, buffer too short).
    const uint8_t trunc[] = {0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00};
    TEST_ASSERT_FALSE(amqp_parse_frame(trunc, sizeof(trunc), &f, &c));
    // A method payload shorter than the 4-octet class/method id.
    const uint8_t short_method[] = {0x00, 0x0A};
    uint16_t cls, meth;
    const uint8_t *a;
    size_t alen;
    TEST_ASSERT_FALSE(amqp_parse_method(short_method, sizeof(short_method), &cls, &meth, &a, &alen));
}

void test_build_overflow_fails_closed()
{
    const uint8_t args[] = {1, 2, 3, 4};
    uint8_t small[8]; // needs 8 + 4 + args
    TEST_ASSERT_EQUAL_size_t(0, amqp_build_method(small, sizeof(small), 1, 10, 10, args, sizeof(args)));
    TEST_ASSERT_EQUAL_size_t(0, amqp_protocol_header(small, 4));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_protocol_header);
    RUN_TEST(test_build_method_bytes);
    RUN_TEST(test_method_round_trip);
    RUN_TEST(test_heartbeat);
    RUN_TEST(test_parse_stream);
    RUN_TEST(test_parse_rejects_bad);
    RUN_TEST(test_build_overflow_fails_closed);
    return UNITY_END();
}
