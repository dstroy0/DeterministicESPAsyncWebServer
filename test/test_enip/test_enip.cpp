// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the EtherNet/IP encapsulation codec (services/enip): the header, the
// RegisterSession + SendRRData builders, and the SendRRData (CPF) reply extractor.
// Little-endian; constants per the Wireshark ENIP dissector. Pure host tests.

#include "services/enip/enip.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_header_round_trip()
{
    EipHeader h;
    memset(&h, 0, sizeof(h));
    h.command = EIP_CMD_REGISTER_SESSION;
    h.session_handle = 0x12345678;
    h.status = EIP_STATUS_SUCCESS;
    const uint8_t ctx[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    memcpy(h.sender_context, ctx, 8);
    const uint8_t data[] = {0xAA, 0xBB};
    uint8_t buf[32];
    size_t n = eip_build(buf, sizeof(buf), &h, data, sizeof(data));
    TEST_ASSERT_EQUAL_size_t(EIP_HEADER_SIZE + 2, n);
    // command + length, little-endian.
    TEST_ASSERT_EQUAL_HEX8(0x65, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0x02, buf[2]);
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[3]);
    TEST_ASSERT_EQUAL_HEX8(0x78, buf[4]); // session handle LSB

    EipHeader p;
    const uint8_t *d;
    size_t dlen;
    TEST_ASSERT_TRUE(eip_parse(buf, n, &p, &d, &dlen));
    TEST_ASSERT_EQUAL_HEX16(EIP_CMD_REGISTER_SESSION, p.command);
    TEST_ASSERT_EQUAL_HEX32(0x12345678, p.session_handle);
    TEST_ASSERT_EQUAL_MEMORY(ctx, p.sender_context, 8);
    TEST_ASSERT_EQUAL_size_t(2, dlen);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(data, d, 2);
}

void test_register_session()
{
    uint8_t buf[32];
    size_t n = eip_build_register_session(buf, sizeof(buf), nullptr);
    TEST_ASSERT_EQUAL_size_t(EIP_HEADER_SIZE + 4, n);
    TEST_ASSERT_EQUAL_HEX8(0x65, buf[0]); // RegisterSession
    TEST_ASSERT_EQUAL_HEX8(0x04, buf[2]); // length 4
    // data = protocol version 1 (LE) + options flags 0.
    TEST_ASSERT_EQUAL_HEX8(0x01, buf[24]);
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[25]);
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[26]);
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[27]);
}

void test_send_rr_data_bytes()
{
    const uint8_t cip[] = {0x4C, 0x02}; // a (stub) CIP request
    uint8_t buf[64];
    size_t n = eip_build_send_rr_data(buf, sizeof(buf), 0x12345678, nullptr, 10, cip, sizeof(cip));
    TEST_ASSERT_EQUAL_size_t(EIP_HEADER_SIZE + 18, n); // data = 6 + 2 + 4(null) + 4(hdr) + 2(cip) = 18
    TEST_ASSERT_EQUAL_HEX8(0x6F, buf[0]);              // SendRRData
    TEST_ASSERT_EQUAL_HEX8(0x12, buf[2]);              // length 18 (0x0012) LSB
    // command data (CPF) starting at offset 24.
    const uint8_t expect_data[] = {
        0x00, 0x00, 0x00, 0x00, // interface handle
        0x0A, 0x00,             // timeout 10
        0x02, 0x00,             // CPF item count 2
        0x00, 0x00, 0x00, 0x00, // null address item (type 0x0000, len 0)
        0xB2, 0x00, 0x02, 0x00, // unconnected data item (type 0x00B2, len 2)
        0x4C, 0x02              // the CIP message
    };
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect_data, buf + EIP_HEADER_SIZE, sizeof(expect_data));
}

void test_send_rr_data_round_trip()
{
    const uint8_t cip[] = {0x4C, 0x20, 0x01, 0x24, 0x01};
    uint8_t buf[64];
    size_t n = eip_build_send_rr_data(buf, sizeof(buf), 0x01, nullptr, 5, cip, sizeof(cip));

    EipHeader p;
    const uint8_t *d;
    size_t dlen;
    TEST_ASSERT_TRUE(eip_parse(buf, n, &p, &d, &dlen));
    TEST_ASSERT_EQUAL_HEX16(EIP_CMD_SEND_RR_DATA, p.command);

    const uint8_t *out_cip;
    size_t out_len;
    TEST_ASSERT_TRUE(eip_parse_send_rr_data(d, dlen, &out_cip, &out_len));
    TEST_ASSERT_EQUAL_size_t(sizeof(cip), out_len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(cip, out_cip, sizeof(cip));
}

void test_parse_rejects_bad()
{
    EipHeader p;
    const uint8_t *d;
    size_t dlen;
    const uint8_t short_hdr[] = {0x65, 0x00, 0x00, 0x00}; // < 24 octets
    TEST_ASSERT_FALSE(eip_parse(short_hdr, sizeof(short_hdr), &p, &d, &dlen));

    // A 24-octet header that declares 8 data octets but has none.
    uint8_t hdr[EIP_HEADER_SIZE] = {0};
    hdr[0] = 0x6F;
    hdr[2] = 0x08; // length 8
    TEST_ASSERT_FALSE(eip_parse(hdr, sizeof(hdr), &p, &d, &dlen));

    // A SendRRData block with no unconnected-data item.
    const uint8_t no_item[] = {0, 0, 0, 0, 0, 0, 0x00, 0x00}; // interface + timeout + item count 0
    const uint8_t *cip;
    size_t clen;
    TEST_ASSERT_FALSE(eip_parse_send_rr_data(no_item, sizeof(no_item), &cip, &clen));
}

void test_build_overflow_fails_closed()
{
    const uint8_t cip[] = {0x4C, 0x02};
    uint8_t small[24]; // room for the header but not the CPF data
    TEST_ASSERT_EQUAL_size_t(0, eip_build_send_rr_data(small, sizeof(small), 1, nullptr, 1, cip, sizeof(cip)));
    TEST_ASSERT_EQUAL_size_t(0, eip_build_register_session(small, 16, nullptr));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_header_round_trip);
    RUN_TEST(test_register_session);
    RUN_TEST(test_send_rr_data_bytes);
    RUN_TEST(test_send_rr_data_round_trip);
    RUN_TEST(test_parse_rejects_bad);
    RUN_TEST(test_build_overflow_fails_closed);
    return UNITY_END();
}
