// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the Siemens S7comm PDU codec (services/s7comm): the Setup Communication
// and Read Var request builders, the header parser, and the response data-item reader
// (length-in-bits + even-item padding). Constants per the Wireshark dissector. Pure host tests.

#include "services/s7comm/s7comm.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_build_setup()
{
    uint8_t buf[32];
    size_t n = s7_build_setup(buf, sizeof(buf), 0x0100, 1, 1, 480); // 480 = 0x01E0
    const uint8_t expect[] = {
        0x32, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x08, 0x00, 0x00, // header
        0xF0, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0xE0              // param
    };
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);
}

// Read 10 bytes of DB1 starting at byte 0.
void test_build_read_request()
{
    S7ReadItem item;
    item.area = S7_AREA_DB;
    item.db_number = 1;
    item.byte_address = 0;
    item.transport_size = S7_TS_BYTE;
    item.count = 10;
    uint8_t buf[48];
    size_t n = s7_build_read_request(buf, sizeof(buf), 0x0100, &item, 1);
    const uint8_t expect[] = {
        0x32, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x0E, 0x00, 0x00,            // header (param-len 0x0E)
        0x04, 0x01,                                                            // read var, 1 item
        0x12, 0x0A, 0x10, 0x02, 0x00, 0x0A, 0x00, 0x01, 0x84, 0x00, 0x00, 0x00 // S7-ANY item
    };
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);
}

// The byte address is encoded as a 24-bit bit-address (byte * 8).
void test_read_request_bit_address()
{
    S7ReadItem item;
    item.area = S7_AREA_DB;
    item.db_number = 2;
    item.byte_address = 10; // -> bit address 80 = 0x000050
    item.transport_size = S7_TS_BYTE;
    item.count = 1;
    uint8_t buf[48];
    size_t n = s7_build_read_request(buf, sizeof(buf), 1, &item, 1);
    TEST_ASSERT_GREATER_THAN(0, (int)n);
    // the 3 address octets are the last three of the item.
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[n - 3]);
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[n - 2]);
    TEST_ASSERT_EQUAL_HEX8(0x50, buf[n - 1]); // 10 * 8 = 80
}

// A single-item Ack_Data read response: return OK, byte transport (length in bits).
void test_parse_response_single()
{
    const uint8_t resp[] = {
        0x32, 0x03, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0x00, 0x0E, 0x00, 0x00, // 12-byte ack_data header
        0x04, 0x01,                                                             // param: read var, 1 item
        0xFF, 0x04, 0x00, 0x50, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
        0x99, 0xAA // data: OK, BYTE, 80 bits = 10 bytes
    };
    S7Header h;
    TEST_ASSERT_TRUE(s7_parse_header(resp, sizeof(resp), &h));
    TEST_ASSERT_EQUAL_HEX8(S7_ROSCTR_ACK_DATA, h.rosctr);
    TEST_ASSERT_EQUAL_size_t(12, h.header_len);
    TEST_ASSERT_EQUAL_UINT16(2, h.param_len);
    TEST_ASSERT_EQUAL_UINT16(14, h.data_len);

    size_t off = 0;
    S7DataItem it;
    TEST_ASSERT_TRUE(s7_read_next_item(h.data, h.data_len, &off, &it));
    TEST_ASSERT_EQUAL_HEX8(S7_RET_OK, it.return_code);
    TEST_ASSERT_EQUAL_HEX8(S7_DTS_BYTE, it.transport_size);
    TEST_ASSERT_EQUAL_size_t(10, it.data_len); // 80 bits -> 10 bytes
    const uint8_t expect_data[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect_data, it.data, 10);
    TEST_ASSERT_FALSE(s7_read_next_item(h.data, h.data_len, &off, &it)); // end
}

// Two items: the first is 3 bytes (odd) so it carries an even-pad byte; the second 2 bytes.
void test_parse_response_padding()
{
    const uint8_t data[] = {
        0xFF, 0x04, 0x00, 0x18, 0xAA, 0xBB, 0xCC, 0x00, // item 1: 24 bits = 3 bytes + 1 pad
        0xFF, 0x04, 0x00, 0x10, 0xDD, 0xEE              // item 2: 16 bits = 2 bytes (last, no pad)
    };
    size_t off = 0;
    S7DataItem it;
    TEST_ASSERT_TRUE(s7_read_next_item(data, sizeof(data), &off, &it));
    TEST_ASSERT_EQUAL_size_t(3, it.data_len);
    TEST_ASSERT_EQUAL_size_t(8, off); // 4 header + 3 data + 1 pad
    TEST_ASSERT_TRUE(s7_read_next_item(data, sizeof(data), &off, &it));
    TEST_ASSERT_EQUAL_size_t(2, it.data_len);
    TEST_ASSERT_EQUAL_HEX8(0xDD, it.data[0]);
    TEST_ASSERT_EQUAL_size_t(sizeof(data), off);
    TEST_ASSERT_FALSE(s7_read_next_item(data, sizeof(data), &off, &it));
}

// An octet-string transport size has its length in bytes (not bits); an error item has no data.
void test_parse_octet_and_error()
{
    const uint8_t octet[] = {0xFF, 0x09, 0x00, 0x05, 0x01, 0x02, 0x03, 0x04, 0x05}; // 5 bytes
    size_t off = 0;
    S7DataItem it;
    TEST_ASSERT_TRUE(s7_read_next_item(octet, sizeof(octet), &off, &it));
    TEST_ASSERT_EQUAL_size_t(5, it.data_len);

    const uint8_t err[] = {0x0A, 0x00, 0x00, 0x00}; // not found, NULL transport, 0 length
    off = 0;
    TEST_ASSERT_TRUE(s7_read_next_item(err, sizeof(err), &off, &it));
    TEST_ASSERT_EQUAL_HEX8(0x0A, it.return_code);
    TEST_ASSERT_EQUAL_size_t(0, it.data_len);
}

void test_parse_rejects_bad()
{
    S7Header h;
    const uint8_t bad_id[] = {0x33, 0x01, 0, 0, 0, 0, 0, 0, 0, 0};
    TEST_ASSERT_FALSE(s7_parse_header(bad_id, sizeof(bad_id), &h));
    // header claims more param/data than buffered.
    const uint8_t overrun[] = {0x32, 0x01, 0, 0, 0, 1, 0, 0x20, 0, 0};
    TEST_ASSERT_FALSE(s7_parse_header(overrun, sizeof(overrun), &h));

    size_t off = 0;
    S7DataItem it;
    const uint8_t trunc[] = {0xFF, 0x04, 0x00, 0x50, 0x11}; // says 10 bytes, only 1 present
    TEST_ASSERT_FALSE(s7_read_next_item(trunc, sizeof(trunc), &off, &it));
}

void test_build_overflow_fails_closed()
{
    S7ReadItem item = {};
    item.area = S7_AREA_DB;
    uint8_t small[16];
    TEST_ASSERT_EQUAL_size_t(0, s7_build_read_request(small, sizeof(small), 1, &item, 1)); // needs 24
    TEST_ASSERT_EQUAL_size_t(0, s7_build_setup(small, 8, 1, 1, 1, 480));                   // needs 18
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_build_setup);
    RUN_TEST(test_build_read_request);
    RUN_TEST(test_read_request_bit_address);
    RUN_TEST(test_parse_response_single);
    RUN_TEST(test_parse_response_padding);
    RUN_TEST(test_parse_octet_and_error);
    RUN_TEST(test_parse_rejects_bad);
    RUN_TEST(test_build_overflow_fails_closed);
    return UNITY_END();
}
