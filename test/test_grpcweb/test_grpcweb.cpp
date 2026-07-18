// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the gRPC-Web message framing codec (services/grpcweb): the message and
// trailer frame builders and the frame parser. Pure host tests.

#include "services/grpcweb/grpcweb.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// A message frame is [flags=0][len BE32][body].
void test_frame_message_bytes()
{
    const uint8_t msg[] = {0x08, 0x96, 0x01}; // a protobuf message (field 1 = 150)
    uint8_t buf[16];
    size_t n = dws_grpcweb_frame_message(buf, sizeof(buf), msg, sizeof(msg), false);
    TEST_ASSERT_EQUAL_size_t(GRPCWEB_PREFIX_LEN + sizeof(msg), n);
    const uint8_t expect[] = {0x00, 0x00, 0x00, 0x00, 0x03, 0x08, 0x96, 0x01};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);
}

void test_compressed_flag()
{
    const uint8_t msg[] = {0xFF};
    uint8_t buf[16];
    size_t n = dws_grpcweb_frame_message(buf, sizeof(buf), msg, sizeof(msg), true);
    TEST_ASSERT_EQUAL_size_t(6, n);
    TEST_ASSERT_EQUAL_HEX8(GRPCWEB_FLAG_COMPRESSED, buf[0]);
}

void test_trailer_frame()
{
    uint8_t buf[64];
    size_t n = dws_grpcweb_frame_trailer(buf, sizeof(buf), 0, "OK");
    TEST_ASSERT_GREATER_THAN(0, (int)n);
    TEST_ASSERT_EQUAL_HEX8(GRPCWEB_FLAG_TRAILER, buf[0]);
    // body = "grpc-status:0\r\ngrpc-message:OK\r\n" (32 octets)
    const char *body = "grpc-status:0\r\ngrpc-message:OK\r\n";
    size_t blen = strlen(body);
    TEST_ASSERT_EQUAL_size_t(GRPCWEB_PREFIX_LEN + blen, n);
    uint32_t len = ((uint32_t)buf[1] << 24) | ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 8) | buf[4];
    TEST_ASSERT_EQUAL_UINT32(blen, len);
    TEST_ASSERT_EQUAL_MEMORY(body, buf + GRPCWEB_PREFIX_LEN, blen);
}

void test_trailer_status_only()
{
    uint8_t buf[64];
    size_t n = dws_grpcweb_frame_trailer(buf, sizeof(buf), 5, nullptr); // no message
    GrpcWebFrame f;
    size_t c;
    TEST_ASSERT_TRUE(dws_grpcweb_parse(buf, n, &f, &c));
    TEST_ASSERT_TRUE(f.trailer);
    int status;
    TEST_ASSERT_TRUE(dws_grpcweb_trailer_status(f.body, f.body_len, &status));
    TEST_ASSERT_EQUAL_INT(5, status);
}

// Two concatenated frames (a message then trailers) parse in sequence.
void test_parse_stream()
{
    const uint8_t msg[] = {0x12, 0x02, 'h', 'i'};
    uint8_t buf[128];
    size_t n = dws_grpcweb_frame_message(buf, sizeof(buf), msg, sizeof(msg), false);
    n += dws_grpcweb_frame_trailer(buf + n, sizeof(buf) - n, 0, "OK");

    size_t pos = 0;
    GrpcWebFrame f;
    size_t c;
    // frame 1: the message
    TEST_ASSERT_TRUE(dws_grpcweb_parse(buf + pos, n - pos, &f, &c));
    TEST_ASSERT_FALSE(f.trailer);
    TEST_ASSERT_FALSE(f.compressed);
    TEST_ASSERT_EQUAL_size_t(sizeof(msg), f.body_len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(msg, f.body, sizeof(msg));
    pos += c;
    // frame 2: the trailers
    TEST_ASSERT_TRUE(dws_grpcweb_parse(buf + pos, n - pos, &f, &c));
    TEST_ASSERT_TRUE(f.trailer);
    int status = -1;
    TEST_ASSERT_TRUE(dws_grpcweb_trailer_status(f.body, f.body_len, &status));
    TEST_ASSERT_EQUAL_INT(0, status);
    pos += c;
    TEST_ASSERT_EQUAL_size_t(n, pos); // consumed exactly the whole stream
}

void test_parse_incomplete()
{
    const uint8_t msg[] = {0xAA, 0xBB, 0xCC};
    uint8_t buf[16];
    size_t n = dws_grpcweb_frame_message(buf, sizeof(buf), msg, sizeof(msg), false);
    GrpcWebFrame f;
    size_t c;
    TEST_ASSERT_FALSE(dws_grpcweb_parse(buf, GRPCWEB_PREFIX_LEN - 1, &f, &c)); // prefix incomplete
    TEST_ASSERT_FALSE(dws_grpcweb_parse(buf, n - 1, &f, &c));                  // body short by one
}

void test_frame_overflow_fails_closed()
{
    const uint8_t msg[] = {1, 2, 3, 4};
    uint8_t small[6]; // 5 prefix + 4 body needs 9
    TEST_ASSERT_EQUAL_size_t(0, dws_grpcweb_frame_message(small, sizeof(small), msg, sizeof(msg), false));
    TEST_ASSERT_EQUAL_size_t(0, dws_grpcweb_frame_trailer(small, 3, 0, "x")); // smaller than the prefix
}

// Frame builder null guards and every trailer-builder overflow/reject: the status-key
// put, a negative status, the status digits, and the grpc-message line.
void test_frame_and_trailer_guards()
{
    uint8_t buf[64];
    const uint8_t msg[] = {1, 2, 3};
    TEST_ASSERT_EQUAL_size_t(0, dws_grpcweb_frame(nullptr, sizeof(buf), 0, msg, 3)); // null buf
    TEST_ASSERT_EQUAL_size_t(0, dws_grpcweb_frame(buf, sizeof(buf), 0, nullptr, 3)); // body_len but null body

    TEST_ASSERT_EQUAL_size_t(0, dws_grpcweb_frame_trailer(buf, 8, 0, nullptr));            // status key overflows
    TEST_ASSERT_EQUAL_size_t(0, dws_grpcweb_frame_trailer(buf, sizeof(buf), -1, nullptr)); // negative status
    TEST_ASSERT_EQUAL_size_t(0, dws_grpcweb_frame_trailer(buf, 17, 5, nullptr));           // status digits overflow
    TEST_ASSERT_EQUAL_size_t(0, dws_grpcweb_frame_trailer(buf, 24, 0, "msg")); // grpc-message line overflows
}

// The status extractor rejects a null body, a key not followed by a digit, and a body
// with no grpc-status line at all.
void test_trailer_status_parse_paths()
{
    int status = -1;
    TEST_ASSERT_FALSE(dws_grpcweb_trailer_status(nullptr, 10, &status)); // null body
    const char *nondigit = "grpc-status:X";
    TEST_ASSERT_FALSE(dws_grpcweb_trailer_status((const uint8_t *)nondigit, strlen(nondigit), &status));
    const char *nokey = "foo:1\r\n";
    TEST_ASSERT_FALSE(dws_grpcweb_trailer_status((const uint8_t *)nokey, strlen(nokey), &status));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_frame_message_bytes);
    RUN_TEST(test_compressed_flag);
    RUN_TEST(test_trailer_frame);
    RUN_TEST(test_trailer_status_only);
    RUN_TEST(test_parse_stream);
    RUN_TEST(test_parse_incomplete);
    RUN_TEST(test_frame_overflow_fails_closed);
    RUN_TEST(test_frame_and_trailer_guards);
    RUN_TEST(test_trailer_status_parse_paths);
    return UNITY_END();
}
