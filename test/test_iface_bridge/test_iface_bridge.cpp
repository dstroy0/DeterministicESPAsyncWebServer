// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the interface-bridge pure core (services/iface_bridge): the address:port -> bus rule
// table (register / find / dedup / capacity, with the full DWSIp bind address preserved) and the
// write-then-read transaction frame codec (big-endian header, partial-frame handling, round-trip). The
// bus I/O and listener are ESP32-only; these tests cover the portable logic.

#include "services/iface_bridge/iface_bridge.h"
#include <string.h>
#include <unity.h>

void setUp()
{
    dws_iface_bridge_clear();
}
void tearDown()
{
}

static BridgeTarget uart_target()
{
    BridgeTarget t;
    memset(&t, 0, sizeof(t));
    t.bus = BridgeBus::uart;
    t.mode = BridgeMode::stream;
    t.unit = 1;
    t.rate = 115200;
    return t;
}

static BridgeTarget i2c_target(uint16_t addr)
{
    BridgeTarget t;
    memset(&t, 0, sizeof(t));
    t.bus = BridgeBus::i2c;
    t.mode = BridgeMode::transaction;
    t.unit = 0;
    t.addr_cs = addr;
    t.rate = 400000;
    return t;
}

void test_map_and_find()
{
    BridgeTarget u = uart_target();
    TEST_ASSERT_TRUE(dws_iface_bridge_map("192.168.1.50", 4001, BridgeProto::tcp, &u));
    TEST_ASSERT_EQUAL_UINT8(1, dws_iface_bridge_count());

    const BridgeRule *r = dws_iface_bridge_find(4001, BridgeProto::tcp);
    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT_EQUAL(BridgeBus::uart, r->target.bus);
    TEST_ASSERT_EQUAL_UINT16(4001, r->listen_port);
    TEST_ASSERT_EQUAL(DWSIpFamily::DWS_IP_V4, r->listen_ip.family); // full bind address preserved
    TEST_ASSERT_EQUAL_UINT8(192, r->listen_ip.bytes[0]);
    TEST_ASSERT_EQUAL_UINT8(50, r->listen_ip.bytes[3]);

    TEST_ASSERT_NULL(dws_iface_bridge_find(4001, BridgeProto::udp)); // proto must match
    TEST_ASSERT_NULL(dws_iface_bridge_find(4002, BridgeProto::tcp)); // port must match
}

void test_any_interface_and_dedup()
{
    BridgeTarget i = i2c_target(0x40);
    TEST_ASSERT_TRUE(dws_iface_bridge_map(nullptr, 5000, BridgeProto::tcp, &i)); // any interface
    const BridgeRule *r = dws_iface_bridge_find(5000, BridgeProto::tcp);
    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT_EQUAL(DWSIpFamily::DWS_IP_NONE, r->listen_ip.family);
    TEST_ASSERT_EQUAL_UINT16(0x40, r->target.addr_cs);

    // Same port+proto is a duplicate -> rejected.
    BridgeTarget u = uart_target();
    TEST_ASSERT_FALSE(dws_iface_bridge_map("10.0.0.1", 5000, BridgeProto::tcp, &u));
    // Same port on the other transport is fine.
    TEST_ASSERT_TRUE(dws_iface_bridge_map(nullptr, 5000, BridgeProto::udp, &i));
    TEST_ASSERT_EQUAL_UINT8(2, dws_iface_bridge_count());
}

void test_bad_address_rejected()
{
    BridgeTarget u = uart_target();
    TEST_ASSERT_FALSE(dws_iface_bridge_map("not.an.ip", 6000, BridgeProto::tcp, &u));
    TEST_ASSERT_EQUAL_UINT8(0, dws_iface_bridge_count());
}

void test_table_full()
{
    BridgeTarget u = uart_target();
    for (uint16_t p = 0; p < DWS_BRIDGE_MAX_RULES; p++)
        TEST_ASSERT_TRUE(dws_iface_bridge_map(nullptr, (uint16_t)(7000 + p), BridgeProto::tcp, &u));
    TEST_ASSERT_EQUAL_UINT8(DWS_BRIDGE_MAX_RULES, dws_iface_bridge_count());
    TEST_ASSERT_FALSE(dws_iface_bridge_map(nullptr, 9999, BridgeProto::tcp, &u)); // full
    dws_iface_bridge_clear();
    TEST_ASSERT_EQUAL_UINT8(0, dws_iface_bridge_count());
}

void test_txn_roundtrip()
{
    const uint8_t wr[3] = {0xAA, 0xBB, 0xCC};
    uint8_t frame[16];
    size_t n = dws_iface_bridge_txn_build(frame, sizeof(frame), wr, 3, 5);
    TEST_ASSERT_EQUAL_size_t(DWS_BRIDGE_TXN_HDR + 3, n);
    const uint8_t expect[] = {0x00, 0x03, 0x00, 0x05, 0xAA, 0xBB, 0xCC}; // write_len=3, read_len=5
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, frame, n);

    uint16_t wl = 0;
    uint16_t rl = 0;
    const uint8_t *wd = nullptr;
    size_t used = dws_iface_bridge_txn_parse(frame, n, &wl, &rl, &wd);
    TEST_ASSERT_EQUAL_size_t(n, used);
    TEST_ASSERT_EQUAL_UINT16(3, wl);
    TEST_ASSERT_EQUAL_UINT16(5, rl);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(wr, wd, 3);
}

void test_txn_partial_and_readonly()
{
    // Partial header (< 4 bytes) -> need more.
    const uint8_t hdr2[2] = {0x00, 0x03};
    TEST_ASSERT_EQUAL_size_t(0, dws_iface_bridge_txn_parse(hdr2, sizeof(hdr2), nullptr, nullptr, nullptr));

    // Header claims write_len 3 but only 2 payload bytes present -> need more.
    const uint8_t partial[6] = {0x00, 0x03, 0x00, 0x00, 0xAA, 0xBB};
    TEST_ASSERT_EQUAL_size_t(0, dws_iface_bridge_txn_parse(partial, sizeof(partial), nullptr, nullptr, nullptr));

    // Read-only transaction (write_len 0) completes at just the header.
    const uint8_t readonly[4] = {0x00, 0x00, 0x00, 0x08};
    uint16_t wl = 9;
    uint16_t rl = 0;
    const uint8_t *wd = nullptr;
    TEST_ASSERT_EQUAL_size_t(DWS_BRIDGE_TXN_HDR, dws_iface_bridge_txn_parse(readonly, sizeof(readonly), &wl, &rl, &wd));
    TEST_ASSERT_EQUAL_UINT16(0, wl);
    TEST_ASSERT_EQUAL_UINT16(8, rl);
}

void test_build_overflow_fails_closed()
{
    const uint8_t wr[4] = {1, 2, 3, 4};
    uint8_t small[6]; // needs 4 + 4 = 8
    TEST_ASSERT_EQUAL_size_t(0, dws_iface_bridge_txn_build(small, sizeof(small), wr, 4, 0));
    TEST_ASSERT_EQUAL_size_t(0, dws_iface_bridge_txn_parse(nullptr, 10, nullptr, nullptr, nullptr));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_map_and_find);
    RUN_TEST(test_any_interface_and_dedup);
    RUN_TEST(test_bad_address_rejected);
    RUN_TEST(test_table_full);
    RUN_TEST(test_txn_roundtrip);
    RUN_TEST(test_txn_partial_and_readonly);
    RUN_TEST(test_build_overflow_fails_closed);
    return UNITY_END();
}
