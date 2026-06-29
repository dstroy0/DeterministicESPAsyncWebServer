// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the DeviceNet link-adaptation codec (services/devicenet): the 4-group 11-bit
// CAN identifier encode/decode, the explicit-message header octet, the fragmentation octet,
// single-frame explicit messages, and the fragmentation reassembler. Identifier allocation
// checked against the ODVA DeviceNet spec. Pure host tests.

#include "services/devicenet/devicenet.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_id_group1()
{
    uint32_t id = 0;
    TEST_ASSERT_TRUE(devicenet_encode_id(&id, DEVICENET_GROUP_1, 0x0A, 0x05)); // msgid 10, mac 5
    TEST_ASSERT_EQUAL_HEX32((0x0Au << 6) | 0x05u, id);
    TEST_ASSERT_TRUE(id < 0x400u);
    DeviceNetId d;
    TEST_ASSERT_TRUE(devicenet_decode_id(id, &d));
    TEST_ASSERT_EQUAL_INT(DEVICENET_GROUP_1, d.group);
    TEST_ASSERT_EQUAL_UINT8(0x0A, d.msg_id);
    TEST_ASSERT_EQUAL_UINT8(0x05, d.mac_id);
}

void test_id_group2()
{
    uint32_t id = 0;
    // Group 2: 10 MAC(6) MsgID(3). mac 0x21, unconnected explicit request.
    TEST_ASSERT_TRUE(devicenet_encode_id(&id, DEVICENET_GROUP_2, DEVICENET_G2_UNCONNECTED_EXPLICIT_REQ, 0x21));
    TEST_ASSERT_EQUAL_HEX32(0x400u | (0x21u << 3) | 4u, id);
    DeviceNetId d;
    TEST_ASSERT_TRUE(devicenet_decode_id(id, &d));
    TEST_ASSERT_EQUAL_INT(DEVICENET_GROUP_2, d.group);
    TEST_ASSERT_EQUAL_UINT8(0x21, d.mac_id);
    TEST_ASSERT_EQUAL_UINT8(4, d.msg_id);
}

void test_id_group3_and_4()
{
    uint32_t id = 0;
    TEST_ASSERT_TRUE(devicenet_encode_id(&id, DEVICENET_GROUP_3, 5, 0x09)); // 11 MsgID(3) MAC(6)
    TEST_ASSERT_EQUAL_HEX32(0x600u | (5u << 6) | 0x09u, id);
    DeviceNetId d;
    TEST_ASSERT_TRUE(devicenet_decode_id(id, &d));
    TEST_ASSERT_EQUAL_INT(DEVICENET_GROUP_3, d.group);
    TEST_ASSERT_EQUAL_UINT8(5, d.msg_id);
    TEST_ASSERT_EQUAL_UINT8(0x09, d.mac_id);

    TEST_ASSERT_TRUE(devicenet_encode_id(&id, DEVICENET_GROUP_4, 0x2A, 0)); // 11111 MsgID(6)
    TEST_ASSERT_EQUAL_HEX32(0x7C0u | 0x2Au, id);
    TEST_ASSERT_TRUE(devicenet_decode_id(id, &d));
    TEST_ASSERT_EQUAL_INT(DEVICENET_GROUP_4, d.group);
    TEST_ASSERT_EQUAL_UINT8(0x2A, d.msg_id);

    // 0x7F0..0x7FF are invalid identifiers.
    TEST_ASSERT_FALSE(devicenet_decode_id(0x7F5, &d));
    // out-of-range arguments fail closed.
    TEST_ASSERT_FALSE(devicenet_encode_id(&id, DEVICENET_GROUP_1, 16, 0));   // msgid > 4 bits
    TEST_ASSERT_FALSE(devicenet_encode_id(&id, DEVICENET_GROUP_2, 0, 64));   // mac > 6 bits
    TEST_ASSERT_FALSE(devicenet_encode_id(&id, DEVICENET_GROUP_4, 0x30, 0)); // msgid > 0x2F
}

void test_header_and_frag_octets()
{
    TEST_ASSERT_EQUAL_HEX8(0x80 | 0x21, devicenet_msg_header(true, false, 0x21));
    TEST_ASSERT_EQUAL_HEX8(0xC0 | 0x21, devicenet_msg_header(true, true, 0x21));
    TEST_ASSERT_EQUAL_HEX8(0x21, devicenet_msg_header(false, false, 0x21));
    TEST_ASSERT_EQUAL_HEX8(0x80 | 0x05, devicenet_frag_octet(DEVICENET_FRAG_LAST, 5));
    TEST_ASSERT_EQUAL_HEX8(0x40 | 0x01, devicenet_frag_octet(DEVICENET_FRAG_MIDDLE, 1));
}

void test_build_explicit_single_frame()
{
    const uint8_t cip[3] = {0x0E, 0x20, 0x01}; // a tiny CIP get-attribute-ish body
    CanFrame f;
    TEST_ASSERT_TRUE(
        devicenet_build_explicit(&f, DEVICENET_GROUP_2, DEVICENET_G2_UNCONNECTED_EXPLICIT_REQ, 0x21, cip, 3));
    TEST_ASSERT_FALSE(f.extended);
    TEST_ASSERT_EQUAL_UINT8(4, f.dlc);       // 1 header + 3 body
    TEST_ASSERT_EQUAL_HEX8(0x21, f.data[0]); // header, not fragmented, mac 0x21
    TEST_ASSERT_EQUAL_MEMORY(cip, f.data + 1, 3);
    // a body that does not fit in one frame is rejected (use fragmentation instead).
    uint8_t big[8] = {0};
    TEST_ASSERT_FALSE(devicenet_build_explicit(&f, DEVICENET_GROUP_2, 4, 0x21, big, 8));
}

void test_frag_non_fragmented()
{
    // header octet with FRAG clear -> the body is complete in one frame.
    const uint8_t body[4] = {0x21, 0xAA, 0xBB, 0xCC};
    DeviceNetFragRx rx;
    devicenet_frag_reset(&rx);
    TEST_ASSERT_EQUAL_INT(DEVICENET_FRAG_COMPLETE, devicenet_frag_feed(&rx, body, 4));
    TEST_ASSERT_EQUAL_UINT16(3, rx.len);
    TEST_ASSERT_EQUAL_HEX8(0xAA, rx.buf[0]);
}

void test_frag_reassembly_roundtrip()
{
    DeviceNetFragRx rx;
    devicenet_frag_reset(&rx);

    // First fragment (count 0): header(FRAG|mac) + frag(FIRST,0) + 6 data.
    uint8_t f0[8] = {0x80 | 0x21, devicenet_frag_octet(DEVICENET_FRAG_FIRST, 0), 1, 2, 3, 4, 5, 6};
    TEST_ASSERT_EQUAL_INT(DEVICENET_FRAG_STARTED, devicenet_frag_feed(&rx, f0, 8));

    // Middle fragment (count 1): + 6 more data.
    uint8_t f1[8] = {0x80 | 0x21, devicenet_frag_octet(DEVICENET_FRAG_MIDDLE, 1), 7, 8, 9, 10, 11, 12};
    TEST_ASSERT_EQUAL_INT(DEVICENET_FRAG_PROGRESS, devicenet_frag_feed(&rx, f1, 8));

    // Last fragment (count 2): + 2 data.
    uint8_t f2[4] = {0x80 | 0x21, devicenet_frag_octet(DEVICENET_FRAG_LAST, 2), 13, 14};
    TEST_ASSERT_EQUAL_INT(DEVICENET_FRAG_COMPLETE, devicenet_frag_feed(&rx, f2, 4));

    TEST_ASSERT_EQUAL_UINT16(14, rx.len);
    uint8_t expect[14] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
    TEST_ASSERT_EQUAL_MEMORY(expect, rx.buf, 14);
}

void test_frag_out_of_order_errors()
{
    DeviceNetFragRx rx;
    devicenet_frag_reset(&rx);
    uint8_t f0[3] = {0x80 | 0x21, devicenet_frag_octet(DEVICENET_FRAG_FIRST, 0), 0xAA};
    TEST_ASSERT_EQUAL_INT(DEVICENET_FRAG_STARTED, devicenet_frag_feed(&rx, f0, 3));
    // jump straight to a middle count 2 (expected 1) -> error, session reset.
    uint8_t bad[3] = {0x80 | 0x21, devicenet_frag_octet(DEVICENET_FRAG_MIDDLE, 2), 0xBB};
    TEST_ASSERT_EQUAL_INT(DEVICENET_FRAG_ERR, devicenet_frag_feed(&rx, bad, 3));
    TEST_ASSERT_FALSE(rx.active);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_id_group1);
    RUN_TEST(test_id_group2);
    RUN_TEST(test_id_group3_and_4);
    RUN_TEST(test_header_and_frag_octets);
    RUN_TEST(test_build_explicit_single_frame);
    RUN_TEST(test_frag_non_fragmented);
    RUN_TEST(test_frag_reassembly_roundtrip);
    RUN_TEST(test_frag_out_of_order_errors);
    return UNITY_END();
}
