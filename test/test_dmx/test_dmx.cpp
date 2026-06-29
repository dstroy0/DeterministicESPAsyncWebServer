// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the DMX512 + RDM codec (services/dmx): the DMX512 slot packet, and the RDM
// (ANSI E1.20) packet build/parse with 48-bit UIDs and the 16-bit additive checksum. Pure
// host tests.

#include "services/dmx/dmx.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_dmx_build_and_get()
{
    uint8_t ch[4] = {10, 20, 30, 255};
    uint8_t buf[8];
    size_t n = dmx_build(buf, sizeof(buf), DMX_SC_DIMMER, ch, 4);
    TEST_ASSERT_EQUAL_size_t(5, n); // start code + 4 channels
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(10, buf[1]);

    TEST_ASSERT_EQUAL_UINT8(10, dmx_get_channel(buf, n, 1)); // channel 1 = slot at buf[1]
    TEST_ASSERT_EQUAL_UINT8(255, dmx_get_channel(buf, n, 4));
    TEST_ASSERT_EQUAL_UINT8(0, dmx_get_channel(buf, n, 5)); // out of range
    TEST_ASSERT_EQUAL_UINT8(0, dmx_get_channel(buf, n, 0)); // channels are 1-based

    // 512 channels is the max; 513 is rejected.
    static uint8_t big[513];
    TEST_ASSERT_EQUAL_size_t(0, dmx_build(big, sizeof(big), DMX_SC_DIMMER, big, 513));
}

void test_rdm_uid()
{
    uint64_t uid = rdm_uid(0x4444, 0x12345678);
    TEST_ASSERT_EQUAL_HEX64(0x444412345678ULL, uid);
}

// Build a GET DEVICE_INFO (no parameter data), parse it back, verify the checksum holds.
void test_rdm_get_roundtrip()
{
    RdmPacket p;
    memset(&p, 0, sizeof(p));
    p.dest_uid = rdm_uid(0x4444, 0x00000001);
    p.src_uid = rdm_uid(0x7A70, 0x000000AA);
    p.tn = 5;
    p.port_id = 1;
    p.msg_count = 0;
    p.sub_device = 0;
    p.cc = RDM_CC_GET;
    p.pid = RDM_PID_DEVICE_INFO;
    p.pdl = 0;

    uint8_t buf[64];
    size_t n = rdm_build(buf, sizeof(buf), &p, nullptr, 0);
    TEST_ASSERT_EQUAL_size_t(RDM_OVERHEAD, n); // 24-octet message + 2 checksum, pdl 0
    TEST_ASSERT_EQUAL_HEX8(0xCC, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, buf[1]);
    TEST_ASSERT_EQUAL_HEX8(24, buf[2]);

    RdmPacket g;
    size_t c;
    TEST_ASSERT_TRUE(rdm_parse(buf, n, &g, &c));
    TEST_ASSERT_EQUAL_HEX64(p.dest_uid, g.dest_uid);
    TEST_ASSERT_EQUAL_HEX64(p.src_uid, g.src_uid);
    TEST_ASSERT_EQUAL_UINT8(5, g.tn);
    TEST_ASSERT_EQUAL_HEX8(RDM_CC_GET, g.cc);
    TEST_ASSERT_EQUAL_HEX16(RDM_PID_DEVICE_INFO, g.pid);
    TEST_ASSERT_EQUAL_UINT8(0, g.pdl);
    TEST_ASSERT_EQUAL_size_t(n, c);
}

// A SET with parameter data round-trips, and the parameter slice matches.
void test_rdm_set_with_data()
{
    RdmPacket p;
    memset(&p, 0, sizeof(p));
    p.dest_uid = rdm_uid(0x4444, 0x00000001);
    p.src_uid = rdm_uid(0x7A70, 0x000000AA);
    p.tn = 9;
    p.port_id = 1;
    p.cc = RDM_CC_SET;
    p.pid = RDM_PID_DMX_START_ADDRESS;
    const uint8_t addr[2] = {0x00, 0x64}; // start address 100, big-endian

    uint8_t buf[64];
    size_t n = rdm_build(buf, sizeof(buf), &p, addr, 2);
    TEST_ASSERT_EQUAL_size_t(RDM_OVERHEAD + 2, n);
    TEST_ASSERT_EQUAL_HEX8(26, buf[2]); // message length 24 + pdl 2

    RdmPacket g;
    size_t c;
    TEST_ASSERT_TRUE(rdm_parse(buf, n, &g, &c));
    TEST_ASSERT_EQUAL_HEX8(RDM_CC_SET, g.cc);
    TEST_ASSERT_EQUAL_HEX16(RDM_PID_DMX_START_ADDRESS, g.pid);
    TEST_ASSERT_EQUAL_UINT8(2, g.pdl);
    TEST_ASSERT_EQUAL_MEMORY(addr, g.pdata, 2);
}

void test_rdm_parse_rejects_bad()
{
    RdmPacket p;
    memset(&p, 0, sizeof(p));
    p.cc = RDM_CC_GET;
    p.pid = RDM_PID_IDENTIFY_DEVICE;
    uint8_t buf[64];
    size_t n = rdm_build(buf, sizeof(buf), &p, nullptr, 0);

    RdmPacket g;
    size_t c;
    uint8_t bad_cs[64];
    memcpy(bad_cs, buf, n);
    bad_cs[n - 1] ^= 0xFF; // corrupt the checksum
    TEST_ASSERT_FALSE(rdm_parse(bad_cs, n, &g, &c));

    uint8_t bad_sc[64];
    memcpy(bad_sc, buf, n);
    bad_sc[0] = 0xAA; // not an RDM start code
    TEST_ASSERT_FALSE(rdm_parse(bad_sc, n, &g, &c));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_dmx_build_and_get);
    RUN_TEST(test_rdm_uid);
    RUN_TEST(test_rdm_get_roundtrip);
    RUN_TEST(test_rdm_set_with_data);
    RUN_TEST(test_rdm_parse_rejects_bad);
    return UNITY_END();
}
