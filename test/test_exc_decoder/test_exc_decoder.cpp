// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/exc_decoder: parsing a real ESP32 Guru Meditation panic dump.

#include "services/exc_decoder/exc_decoder.h"
#include <string.h>
#include <unity.h>

static const char *PANIC = "Guru Meditation Error: Core  1 panic'ed (LoadProhibited). Exception was unhandled.\n"
                           "\n"
                           "Core  1 register dump:\n"
                           "PC      : 0x400d1234  PS      : 0x00060730  A0      : 0x800d5678  A1      : 0x3ffb2200\n"
                           "A2      : 0x00000000  A3      : 0x00000001  A4      : 0x00000000  A5      : 0x00000000\n"
                           "EXCVADDR: 0x0000002a  LBEG    : 0x40085d5c  LEND    : 0x40085d6a  LCOUNT  : 0x00000000\n"
                           "\n"
                           "Backtrace: 0x400d1234:0x3ffb2200 0x400d5678:0x3ffb2220 0x400d9abc:0x3ffb2240\n";

void setUp(void)
{
}
void tearDown(void)
{
}

void test_parse_full(void)
{
    ExcInfo info;
    TEST_ASSERT_TRUE(detws_exc_parse(PANIC, &info));
    TEST_ASSERT_EQUAL_INT(1, info.core);
    TEST_ASSERT_EQUAL_STRING("LoadProhibited", info.cause);
    TEST_ASSERT_EQUAL_HEX32(0x400d1234, info.pc);
    TEST_ASSERT_TRUE(info.has_excvaddr);
    TEST_ASSERT_EQUAL_HEX32(0x2a, info.excvaddr);
    TEST_ASSERT_EQUAL_size_t(3, info.frame_count);
    TEST_ASSERT_EQUAL_HEX32(0x400d1234, info.frames[0].pc);
    TEST_ASSERT_EQUAL_HEX32(0x3ffb2200, info.frames[0].sp);
    TEST_ASSERT_EQUAL_HEX32(0x400d9abc, info.frames[2].pc);
}

void test_json(void)
{
    ExcInfo info;
    detws_exc_parse(PANIC, &info);
    char buf[512];
    size_t n = detws_exc_json(&info, buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_size_t(strlen(buf), n);
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"core\":1"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"cause\":\"LoadProhibited\""));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"pc\":\"0x400d1234\""));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"excvaddr\":\"0x0000002a\""));
    TEST_ASSERT_NOT_NULL(strstr(buf, "\"backtrace\":[\"0x400d1234\",\"0x400d5678\",\"0x400d9abc\"]"));
}

void test_backtrace_only_and_corrupted(void)
{
    // No register dump: PC must fall back to the first backtrace frame. Trailing corruption marker.
    const char *bt = "Backtrace: 0x400e1111:0x3ffc0000 0x400e2222:0x3ffc0020 |<-CORRUPTED\n";
    ExcInfo info;
    TEST_ASSERT_TRUE(detws_exc_parse(bt, &info));
    TEST_ASSERT_EQUAL_INT(-1, info.core);
    TEST_ASSERT_EQUAL_STRING("", info.cause);
    TEST_ASSERT_FALSE(info.has_excvaddr);
    TEST_ASSERT_EQUAL_size_t(2, info.frame_count);
    TEST_ASSERT_EQUAL_HEX32(0x400e1111, info.pc); // fell back to frame[0]
}

void test_garbage_returns_false(void)
{
    ExcInfo info;
    TEST_ASSERT_FALSE(detws_exc_parse("nothing to see here\n", &info));
    TEST_ASSERT_FALSE(detws_exc_parse("", &info));
}

void test_json_omits_core_when_absent_and_overflow(void)
{
    const char *bt = "Backtrace: 0x400e1111:0x3ffc0000\n";
    ExcInfo info;
    detws_exc_parse(bt, &info);
    char buf[128];
    size_t n = detws_exc_json(&info, buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_NULL(strstr(buf, "\"core\"")); // omitted when core == -1
    TEST_ASSERT_NULL(strstr(buf, "excvaddr")); // omitted when absent

    char tiny[8];
    TEST_ASSERT_EQUAL_size_t(0, detws_exc_json(&info, tiny, sizeof(tiny)));
}

void test_upper_hex_and_json_overflow()
{
    ExcInfo info;
    // Uppercase hex addresses exercise the A-F branch of the nibble parser.
    const char *up = "Guru Meditation Error: Core  0 panic'ed (StoreProhibited).\n"
                     "Backtrace: 0x400D1234:0x3FFB2200 0x400DABCD:0x3FFB2220\n";
    TEST_ASSERT_TRUE(detws_exc_parse(up, &info));
    TEST_ASSERT_EQUAL_HEX32(0x400D1234, info.frames[0].pc);
    TEST_ASSERT_EQUAL_HEX32(0x400DABCD, info.frames[1].pc);
    char buf[256];
    TEST_ASSERT_TRUE(detws_exc_json(&info, buf, sizeof(buf)) > 0);
    TEST_ASSERT_EQUAL_size_t(0, detws_exc_json(&info, buf, 8)); // tiny cap fails closed
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_parse_full);
    RUN_TEST(test_json);
    RUN_TEST(test_backtrace_only_and_corrupted);
    RUN_TEST(test_garbage_returns_false);
    RUN_TEST(test_json_omits_core_when_absent_and_overflow);
    RUN_TEST(test_upper_hex_and_json_overflow);
    return UNITY_END();
}
