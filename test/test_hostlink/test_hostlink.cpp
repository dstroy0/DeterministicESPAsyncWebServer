// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the Omron Host Link (C-mode) frame codec (services/hostlink): the FCS,
// the command builder, and the FCS-validating parser. Pure host tests.

#include "services/hostlink/hostlink.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// FCS of "@00RD00000010" is 0x57 (XOR of all ASCII values).
void test_fcs_vector()
{
    const char *body = "@00RD00000010";
    TEST_ASSERT_EQUAL_HEX8(0x57, hostlink_fcs(body, strlen(body)));
}

// The full DM-read command frame for node 0: @00RD00000010 57 *CR.
void test_build_dm_read()
{
    char buf[32];
    size_t n = hostlink_build(buf, sizeof(buf), 0, "RD", "00000010", 8);
    TEST_ASSERT_EQUAL_STRING("@00RD00000010"
                             "57"
                             "*\r",
                             buf);
    TEST_ASSERT_EQUAL_size_t(17, n);
}

// The node number renders as two digits.
void test_build_node_digits()
{
    char buf[32];
    size_t n = hostlink_build(buf, sizeof(buf), 13, "RD", "0000", 4);
    TEST_ASSERT_GREATER_THAN(0, (int)n);
    TEST_ASSERT_EQUAL_HEX8('@', buf[0]);
    TEST_ASSERT_EQUAL_HEX8('1', buf[1]);
    TEST_ASSERT_EQUAL_HEX8('3', buf[2]);
    TEST_ASSERT_EQUAL_HEX8('R', buf[3]);
    TEST_ASSERT_EQUAL_HEX8('D', buf[4]);
}

void test_round_trip()
{
    char buf[32];
    size_t n = hostlink_build(buf, sizeof(buf), 5, "RD", "00000010", 8);

    HostlinkFrame f;
    TEST_ASSERT_TRUE(hostlink_parse(buf, n, &f));
    TEST_ASSERT_EQUAL_UINT8(5, f.node);
    TEST_ASSERT_EQUAL_STRING("RD", f.header_code);
    TEST_ASSERT_EQUAL_size_t(8, f.text_len);
    TEST_ASSERT_EQUAL_MEMORY("00000010", f.text, 8);
}

// A response: @00RD00 <data> FCS *CR - the text starts with the end code "00".
void test_parse_response_end_code()
{
    char buf[64];
    // Build a "response-shaped" frame: header RD, text = end code "00" + 4 data digits.
    size_t n = hostlink_build(buf, sizeof(buf), 0, "RD", "001234", 6);
    HostlinkFrame f;
    TEST_ASSERT_TRUE(hostlink_parse(buf, n, &f));
    uint8_t code;
    TEST_ASSERT_TRUE(hostlink_end_code(&f, &code));
    TEST_ASSERT_EQUAL_HEX8(0x00, code); // normal completion
    TEST_ASSERT_EQUAL_MEMORY("001234", f.text, 6);
}

void test_parse_rejects_bad()
{
    char buf[32];
    size_t n = hostlink_build(buf, sizeof(buf), 0, "RD", "00000010", 8);

    HostlinkFrame f;
    // Corrupt a text char -> FCS no longer matches.
    char corrupt[32];
    memcpy(corrupt, buf, n);
    corrupt[6] ^= 0x01;
    TEST_ASSERT_FALSE(hostlink_parse(corrupt, n, &f));

    // Missing terminator.
    memcpy(corrupt, buf, n);
    corrupt[n - 1] = 'X';
    TEST_ASSERT_FALSE(hostlink_parse(corrupt, n, &f));

    // Not a Host Link frame.
    TEST_ASSERT_FALSE(hostlink_parse("hello", 5, &f));
    // Too short.
    TEST_ASSERT_FALSE(hostlink_parse("@00*\r", 5, &f));
}

void test_build_overflow_fails_closed()
{
    char small[8];
    TEST_ASSERT_EQUAL_size_t(0, hostlink_build(small, sizeof(small), 0, "RD", "00000010", 8));
    // A one-character header code is rejected.
    char buf[32];
    TEST_ASSERT_EQUAL_size_t(0, hostlink_build(buf, sizeof(buf), 0, "R", "0", 1));
}

// Builder/parser guards plus the hex-digit decoder's letter and invalid branches.
void test_guards_and_hex()
{
    char buf[32];
    // build guards
    TEST_ASSERT_EQUAL_size_t(0, hostlink_build(nullptr, sizeof(buf), 0, "RD", "0", 1)); // null buf
    TEST_ASSERT_EQUAL_size_t(0, hostlink_build(buf, sizeof(buf), 0, nullptr, "0", 1));  // null header code
    TEST_ASSERT_EQUAL_size_t(0, hostlink_build(buf, sizeof(buf), 100, "RD", "0", 1));   // node > 99
    TEST_ASSERT_EQUAL_size_t(0, hostlink_build(buf, sizeof(buf), 0, "RD", nullptr, 4)); // text_len but null text

    // parse: a non-digit node, and FCS characters that are not hex.
    HostlinkFrame f;
    TEST_ASSERT_FALSE(hostlink_parse("@A0RDFF*\r", 9, &f)); // node field not a digit
    TEST_ASSERT_FALSE(hostlink_parse("@00RDGG*\r", 9, &f)); // FCS chars not hex digits

    // end code: null, too-short text, then upper/lower/invalid hex through hex_val.
    uint8_t code = 0;
    TEST_ASSERT_FALSE(hostlink_end_code(nullptr, &code));
    HostlinkFrame g;
    g.text = "X";
    g.text_len = 1;
    TEST_ASSERT_FALSE(hostlink_end_code(&g, &code)); // text_len < 2
    g.text = "AB";
    g.text_len = 2;
    TEST_ASSERT_TRUE(hostlink_end_code(&g, &code)); // uppercase A-F
    TEST_ASSERT_EQUAL_HEX8(0xAB, code);
    g.text = "cd";
    g.text_len = 2;
    TEST_ASSERT_TRUE(hostlink_end_code(&g, &code)); // lowercase a-f
    TEST_ASSERT_EQUAL_HEX8(0xCD, code);
    g.text = "G!";
    g.text_len = 2;
    TEST_ASSERT_FALSE(hostlink_end_code(&g, &code)); // non-hex character
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_fcs_vector);
    RUN_TEST(test_build_dm_read);
    RUN_TEST(test_build_node_digits);
    RUN_TEST(test_round_trip);
    RUN_TEST(test_parse_response_end_code);
    RUN_TEST(test_parse_rejects_bad);
    RUN_TEST(test_build_overflow_fails_closed);
    RUN_TEST(test_guards_and_hex);
    return UNITY_END();
}
