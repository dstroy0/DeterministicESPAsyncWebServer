// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the IEEE C37.118.2 synchrophasor frame codec (services/c37118): the
// CRC-CCITT, the frame builder, the Command frame, and the CRC-validating parser.
// Pure host tests.

#include "services/c37118/c37118.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// CRC-CCITT (0x1021, init 0xFFFF, no reflection, no final mask): the canonical check
// value for the ASCII string "123456789" is 0x29B1.
void test_crc_check_value()
{
    TEST_ASSERT_EQUAL_HEX16(0x29B1, c37118_crc((const uint8_t *)"123456789", 9));
}

// A Command frame has the documented header bytes and an 18-octet size.
void test_build_command_bytes()
{
    uint8_t buf[32];
    size_t n = c37118_build_command(buf, sizeof(buf), 7, 0x5F000000, 0x00000000, C37118_CMD_DATA_ON);
    TEST_ASSERT_EQUAL_size_t(18, n);
    TEST_ASSERT_EQUAL_HEX8(0xAA, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x42, buf[1]); // type CMD (4<<4) | version 2
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[2]);
    TEST_ASSERT_EQUAL_HEX8(0x12, buf[3]); // framesize 18
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[4]);
    TEST_ASSERT_EQUAL_HEX8(0x07, buf[5]);  // idcode 7
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[14]); // cmd word high byte
    TEST_ASSERT_EQUAL_HEX8(0x02, buf[15]); // cmd word low byte (DATA_ON = 2)
    // The appended CHK (octets 16..17) matches a fresh CRC over the first 16 octets.
    uint16_t chk = (uint16_t)((buf[16] << 8) | buf[17]);
    TEST_ASSERT_EQUAL_HEX16(c37118_crc(buf, 16), chk);
}

void test_command_round_trip()
{
    uint8_t buf[32];
    size_t n = c37118_build_command(buf, sizeof(buf), 0x1234, 0x5F5E1100, 0x00ABCDEF, C37118_CMD_SEND_CFG2);
    TEST_ASSERT_GREATER_THAN(0, (int)n);

    C37118Frame f;
    TEST_ASSERT_TRUE(c37118_parse_frame(buf, n, &f));
    TEST_ASSERT_EQUAL_UINT8(C37118_TYPE_CMD, f.type);
    TEST_ASSERT_EQUAL_UINT8(C37118_VERSION_2011, f.version);
    TEST_ASSERT_EQUAL_UINT16(18, f.framesize);
    TEST_ASSERT_EQUAL_HEX16(0x1234, f.idcode);
    TEST_ASSERT_EQUAL_HEX32(0x5F5E1100, f.soc);
    TEST_ASSERT_EQUAL_HEX32(0x00ABCDEF, f.fracsec);
    TEST_ASSERT_EQUAL_size_t(2, f.data_len);
    uint16_t cmd;
    TEST_ASSERT_TRUE(c37118_parse_command(&f, &cmd));
    TEST_ASSERT_EQUAL_UINT16(C37118_CMD_SEND_CFG2, cmd);
}

// A data frame carries an arbitrary payload (e.g. STAT + one phasor) and round-trips.
void test_data_frame_payload()
{
    const uint8_t payload[] = {0x00, 0x00, 0x12, 0x34, 0x56, 0x78}; // STAT + a 4-byte value
    uint8_t buf[64];
    size_t n = c37118_build_frame(buf, sizeof(buf), C37118_TYPE_DATA, C37118_VERSION_2011, 60, 100, 200, payload,
                                  sizeof(payload));
    TEST_ASSERT_EQUAL_size_t(C37118_MIN_FRAME + sizeof(payload), n);
    TEST_ASSERT_EQUAL_HEX8(0x02, buf[1]); // type DATA (0) | version 2

    C37118Frame f;
    TEST_ASSERT_TRUE(c37118_parse_frame(buf, n, &f));
    TEST_ASSERT_EQUAL_UINT8(C37118_TYPE_DATA, f.type);
    TEST_ASSERT_EQUAL_size_t(sizeof(payload), f.data_len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(payload, f.data, sizeof(payload));
    uint16_t cmd;
    TEST_ASSERT_FALSE(c37118_parse_command(&f, &cmd)); // not a command frame
}

void test_parse_rejects_bad()
{
    uint8_t buf[32];
    size_t n = c37118_build_command(buf, sizeof(buf), 7, 1, 2, C37118_CMD_DATA_ON);
    C37118Frame f;

    // A flipped payload bit must fail the CRC check.
    uint8_t corrupt[32];
    memcpy(corrupt, buf, n);
    corrupt[14] ^= 0x01;
    TEST_ASSERT_FALSE(c37118_parse_frame(corrupt, n, &f));

    // Wrong SYNC leader.
    memcpy(corrupt, buf, n);
    corrupt[0] = 0xAB;
    TEST_ASSERT_FALSE(c37118_parse_frame(corrupt, n, &f));

    // Truncated buffer (framesize claims more than is present).
    TEST_ASSERT_FALSE(c37118_parse_frame(buf, n - 1, &f));

    // Too short to be a frame at all.
    TEST_ASSERT_FALSE(c37118_parse_frame(buf, 4, &f));
}

void test_build_overflow_fails_closed()
{
    uint8_t small[8];
    TEST_ASSERT_EQUAL_size_t(0, c37118_build_command(small, sizeof(small), 1, 0, 0, C37118_CMD_DATA_ON));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_crc_check_value);
    RUN_TEST(test_build_command_bytes);
    RUN_TEST(test_command_round_trip);
    RUN_TEST(test_data_frame_payload);
    RUN_TEST(test_parse_rejects_bad);
    RUN_TEST(test_build_overflow_fails_closed);
    return UNITY_END();
}
