// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the Omron FINS frame codec (services/fins): the command builder, the
// Memory Area Read convenience, and the command / response parsers. Pure host tests.

#include "services/fins/fins.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

static FinsHeader make_header()
{
    FinsHeader h = {};
    h.icf = FINS_ICF_COMMAND; // 0x80
    h.rsv = 0x00;
    h.gct = 0x02;
    h.dna = 0x00;
    h.da1 = 0x01; // destination node 1
    h.da2 = 0x00;
    h.sna = 0x00;
    h.sa1 = 0x02; // source node 2
    h.sa2 = 0x00;
    h.sid = 0x2A;
    return h;
}

void test_build_command_bytes()
{
    FinsHeader h = make_header();
    const uint8_t params[] = {0xAB, 0xCD};
    uint8_t buf[32];
    size_t n = dws_fins_build_command(buf, sizeof(buf), &h, 0x05, 0x01, params, sizeof(params));
    TEST_ASSERT_EQUAL_size_t(FINS_HEADER_SIZE + 2 + 2, n);
    const uint8_t expect[] = {0x80, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x00, 0x2A, // header
                              0x05, 0x01,                                                 // MRC, SRC
                              0xAB, 0xCD};                                                // params
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);
}

// Memory Area Read (0101): area + 2-octet word address + bit + 2-octet count.
void test_memory_area_read()
{
    FinsHeader h = make_header();
    uint8_t buf[32];
    // area 0xB0 (DM), word 100 = 0x0064, bit 0, read 10 words.
    size_t n = dws_fins_build_memory_area_read(buf, sizeof(buf), &h, 0xB0, 100, 0, 10);
    TEST_ASSERT_EQUAL_size_t(FINS_HEADER_SIZE + 2 + 6, n);
    TEST_ASSERT_EQUAL_HEX8(0x01, buf[10]); // MRC
    TEST_ASSERT_EQUAL_HEX8(0x01, buf[11]); // SRC
    TEST_ASSERT_EQUAL_HEX8(0xB0, buf[12]); // area
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[13]); // addr hi
    TEST_ASSERT_EQUAL_HEX8(0x64, buf[14]); // addr lo (100)
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[15]); // bit
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[16]); // count hi
    TEST_ASSERT_EQUAL_HEX8(0x0A, buf[17]); // count lo (10)
}

void test_parse_command()
{
    FinsHeader h = make_header();
    const uint8_t params[] = {0xB0, 0x00, 0x64, 0x00, 0x00, 0x0A};
    uint8_t buf[32];
    size_t n = dws_fins_build_command(buf, sizeof(buf), &h, 0x01, 0x01, params, sizeof(params));

    FinsCommand c;
    TEST_ASSERT_TRUE(dws_fins_parse_command(buf, n, &c));
    TEST_ASSERT_EQUAL_HEX8(FINS_ICF_COMMAND, c.header.icf);
    TEST_ASSERT_EQUAL_HEX8(0x01, c.header.da1);
    TEST_ASSERT_EQUAL_HEX8(0x2A, c.header.sid);
    TEST_ASSERT_EQUAL_HEX8(0x01, c.mrc);
    TEST_ASSERT_EQUAL_HEX8(0x01, c.src);
    TEST_ASSERT_EQUAL_size_t(sizeof(params), c.params_len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(params, c.params, sizeof(params));
}

// A normal-completion response: end code 0x0000, then two data words.
void test_parse_response_ok()
{
    const uint8_t resp[] = {
        0xC0, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00, 0x2A, // response header
        0x01, 0x01,                                                 // echoed MRC/SRC
        0x00, 0x00,                                                 // MRES/SRES = normal
        0x12, 0x34, 0x56, 0x78                                      // 2 data words
    };
    FinsResponse r;
    TEST_ASSERT_TRUE(dws_fins_parse_response(resp, sizeof(resp), &r));
    TEST_ASSERT_EQUAL_HEX8(FINS_ICF_RESPONSE, r.header.icf);
    TEST_ASSERT_EQUAL_HEX8(0x2A, r.header.sid); // echoes the request SID
    TEST_ASSERT_EQUAL_HEX8(0x01, r.mrc);
    TEST_ASSERT_EQUAL_HEX8(0x00, r.mres);
    TEST_ASSERT_EQUAL_HEX8(0x00, r.sres);
    TEST_ASSERT_EQUAL_size_t(4, r.data_len);
    const uint8_t data[] = {0x12, 0x34, 0x56, 0x78};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(data, r.data, 4);
}

// An error response carries a non-zero end code and no data.
void test_parse_response_error()
{
    const uint8_t resp[] = {0xC0, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00,
                            0x01, 0x00, 0x2A, 0x01, 0x01, 0x01, 0x01}; // MRES/SRES = 0x0101 (e.g. local node error)
    FinsResponse r;
    TEST_ASSERT_TRUE(dws_fins_parse_response(resp, sizeof(resp), &r));
    TEST_ASSERT_EQUAL_HEX8(0x01, r.mres);
    TEST_ASSERT_EQUAL_HEX8(0x01, r.sres);
    TEST_ASSERT_EQUAL_size_t(0, r.data_len);
}

void test_overflow_and_truncation()
{
    FinsHeader h = make_header();
    uint8_t small[8]; // smaller than even the header
    TEST_ASSERT_EQUAL_size_t(0, dws_fins_build_command(small, sizeof(small), &h, 1, 1, nullptr, 0));
    // Null destination / header / a param length with no params array.
    uint8_t big[32];
    TEST_ASSERT_EQUAL_size_t(0, dws_fins_build_command(nullptr, sizeof(big), &h, 1, 1, nullptr, 0));
    TEST_ASSERT_EQUAL_size_t(0, dws_fins_build_command(big, sizeof(big), nullptr, 1, 1, nullptr, 0));
    TEST_ASSERT_EQUAL_size_t(0, dws_fins_build_command(big, sizeof(big), &h, 1, 1, nullptr, 4));

    FinsCommand c;
    const uint8_t short_buf[] = {0x80, 0x00, 0x02, 0x00}; // too short for header + MRC/SRC
    TEST_ASSERT_FALSE(dws_fins_parse_command(short_buf, sizeof(short_buf), &c));

    FinsResponse r;
    const uint8_t short_resp[] = {0xC0, 0x00, 0x02, 0x00, 0x02, 0x00,
                                  0x00, 0x01, 0x00, 0x2A, 0x01, 0x01}; // no end code
    TEST_ASSERT_FALSE(dws_fins_parse_response(short_resp, sizeof(short_resp), &r));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_build_command_bytes);
    RUN_TEST(test_memory_area_read);
    RUN_TEST(test_parse_command);
    RUN_TEST(test_parse_response_ok);
    RUN_TEST(test_parse_response_error);
    RUN_TEST(test_overflow_and_truncation);
    return UNITY_END();
}
