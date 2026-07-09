// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/dds: the RTPS message + submessage framing codec.

#include "services/dds/dds.h"
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

static const uint8_t GUID[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
static const uint8_t VENDOR[2] = {0x01, 0x03};

void test_header(void)
{
    uint8_t out[24];
    size_t n = detws_rtps_header(GUID, VENDOR, out, sizeof(out));
    TEST_ASSERT_EQUAL_size_t(20, n);
    const uint8_t expect[20] = {'R', 'T', 'P', 'S', 2, 4, 0x01, 0x03, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, out, 20);
    // Too small a buffer -> 0.
    TEST_ASSERT_EQUAL_size_t(0, detws_rtps_header(GUID, VENDOR, out, 10));
}

void test_submessage_endianness(void)
{
    uint8_t body[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    uint8_t out[16];
    // Little-endian (E flag set): octetsToNextHeader = 0x0008 -> 08 00.
    size_t n = detws_rtps_submessage(RTPS_SM_INFO_TS, RTPS_FLAG_ENDIAN, body, 8, out, sizeof(out));
    const uint8_t le[] = {0x09, 0x01, 0x08, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    TEST_ASSERT_EQUAL_size_t(sizeof(le), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(le, out, n);
    // Big-endian (E flag clear): 00 08.
    n = detws_rtps_submessage(RTPS_SM_DATA, 0x00, body, 8, out, sizeof(out));
    TEST_ASSERT_EQUAL_HEX8(0x00, out[2]);
    TEST_ASSERT_EQUAL_HEX8(0x08, out[3]);
}

struct Seen
{
    int count;
    uint8_t ids[8];
    size_t lens[8];
};
static void collect(uint8_t id, uint8_t flags, const uint8_t *body, size_t body_len, void *arg)
{
    (void)flags;
    (void)body;
    Seen *s = (Seen *)arg;
    if (s->count < 8)
    {
        s->ids[s->count] = id;
        s->lens[s->count] = body_len;
        s->count++;
    }
}

void test_parse_message(void)
{
    uint8_t msg[64];
    size_t n = detws_rtps_header(GUID, VENDOR, msg, sizeof(msg));
    uint8_t ts[8] = {0};
    n += detws_rtps_submessage(RTPS_SM_INFO_TS, RTPS_FLAG_ENDIAN, ts, 8, msg + n, sizeof(msg) - n);
    uint8_t data[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    n += detws_rtps_submessage(RTPS_SM_DATA, RTPS_FLAG_ENDIAN, data, 4, msg + n, sizeof(msg) - n);

    Seen s = {0, {0}, {0}};
    TEST_ASSERT_TRUE(detws_rtps_parse(msg, n, collect, &s));
    TEST_ASSERT_EQUAL_INT(2, s.count);
    TEST_ASSERT_EQUAL_HEX8(RTPS_SM_INFO_TS, s.ids[0]);
    TEST_ASSERT_EQUAL_size_t(8, s.lens[0]);
    TEST_ASSERT_EQUAL_HEX8(RTPS_SM_DATA, s.ids[1]);
    TEST_ASSERT_EQUAL_size_t(4, s.lens[1]);
}

void test_parse_rejects(void)
{
    uint8_t msg[24];
    detws_rtps_header(GUID, VENDOR, msg, sizeof(msg));
    // Bad magic.
    msg[0] = 'X';
    TEST_ASSERT_FALSE(detws_rtps_parse(msg, 20, nullptr, nullptr));
    msg[0] = 'R';
    // A newer minor version than ours is rejected.
    msg[5] = 99;
    TEST_ASSERT_FALSE(detws_rtps_parse(msg, 20, nullptr, nullptr));
    // Too short for a header.
    TEST_ASSERT_FALSE(detws_rtps_parse(msg, 10, nullptr, nullptr));
}

void test_rtps_build_guards()
{
    uint8_t out[8];
    uint8_t guid[12] = {0};
    uint8_t vendor[2] = {0};
    TEST_ASSERT_EQUAL_size_t(0, detws_rtps_header(guid, vendor, out, 4)); // cap too small
    uint8_t body[4] = {1, 2, 3, 4};
    TEST_ASSERT_EQUAL_size_t(0, detws_rtps_submessage(0x15, 0, body, sizeof(body), out, 2)); // cap too small
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_header);
    RUN_TEST(test_submessage_endianness);
    RUN_TEST(test_parse_message);
    RUN_TEST(test_parse_rejects);
    RUN_TEST(test_rtps_build_guards);
    return UNITY_END();
}
