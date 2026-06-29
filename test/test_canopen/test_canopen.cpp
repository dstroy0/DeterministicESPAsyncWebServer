// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the CANopen (CiA 301) message codec (services/canopen): NMT, SYNC,
// heartbeat, EMCY, PDO, and expedited SDO read/write/abort + response parsing, plus the
// COB-ID classifier. Values checked against the CiA 301 default identifier allocation.
// Pure host tests.

#include "services/canopen/canopen.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// NMT "start node 5": COB-ID 0, [command, node].
void test_nmt_start_node()
{
    CanFrame f;
    TEST_ASSERT_TRUE(canopen_build_nmt(&f, CANOPEN_NMT_START, 5));
    TEST_ASSERT_EQUAL_UINT32(0x000, f.id);
    TEST_ASSERT_FALSE(f.extended);
    TEST_ASSERT_EQUAL_UINT8(2, f.dlc);
    TEST_ASSERT_EQUAL_HEX8(0x01, f.data[0]);
    TEST_ASSERT_EQUAL_HEX8(5, f.data[1]);

    // node 0 = all nodes is allowed; node 200 is out of range.
    TEST_ASSERT_TRUE(canopen_build_nmt(&f, CANOPEN_NMT_RESET_NODE, 0));
    TEST_ASSERT_FALSE(canopen_build_nmt(&f, CANOPEN_NMT_START, 200));
}

void test_sync()
{
    CanFrame f;
    TEST_ASSERT_TRUE(canopen_build_sync(&f));
    TEST_ASSERT_EQUAL_UINT32(0x080, f.id);
    TEST_ASSERT_EQUAL_UINT8(0, f.dlc);

    CanopenMsg m;
    TEST_ASSERT_TRUE(canopen_parse(&f, &m));
    TEST_ASSERT_EQUAL_INT(CANOPEN_T_SYNC, m.type);
    TEST_ASSERT_EQUAL_UINT8(0, m.node_id);
}

void test_heartbeat_roundtrip()
{
    CanFrame f;
    TEST_ASSERT_TRUE(canopen_build_heartbeat(&f, 10, CANOPEN_STATE_OPERATIONAL));
    TEST_ASSERT_EQUAL_UINT32(0x70A, f.id); // 0x700 + 10
    TEST_ASSERT_EQUAL_UINT8(1, f.dlc);

    CanopenMsg m;
    TEST_ASSERT_TRUE(canopen_parse(&f, &m));
    TEST_ASSERT_EQUAL_INT(CANOPEN_T_HEARTBEAT, m.type);
    TEST_ASSERT_EQUAL_UINT8(10, m.node_id);

    uint8_t node = 0, state = 0;
    TEST_ASSERT_TRUE(canopen_parse_heartbeat(&f, &node, &state));
    TEST_ASSERT_EQUAL_UINT8(10, node);
    TEST_ASSERT_EQUAL_HEX8(CANOPEN_STATE_OPERATIONAL, state);
}

void test_emcy_roundtrip()
{
    const uint8_t msef[5] = {0xDE, 0xAD, 0xBE, 0xEF, 0x42};
    CanFrame f;
    TEST_ASSERT_TRUE(canopen_build_emcy(&f, 3, 0x8130, 0x11, msef));
    TEST_ASSERT_EQUAL_UINT32(0x083, f.id); // 0x080 + 3
    TEST_ASSERT_EQUAL_UINT8(8, f.dlc);
    TEST_ASSERT_EQUAL_HEX8(0x30, f.data[0]); // error code LE
    TEST_ASSERT_EQUAL_HEX8(0x81, f.data[1]);

    uint8_t node = 0, reg = 0, out_msef[5] = {0};
    uint16_t code = 0;
    TEST_ASSERT_TRUE(canopen_parse_emcy(&f, &node, &code, &reg, out_msef));
    TEST_ASSERT_EQUAL_UINT8(3, node);
    TEST_ASSERT_EQUAL_HEX16(0x8130, code);
    TEST_ASSERT_EQUAL_HEX8(0x11, reg);
    TEST_ASSERT_EQUAL_MEMORY(msef, out_msef, 5);

    // 0x080 with node 0 is SYNC, not EMCY: parse_emcy must reject it.
    CanFrame sync;
    canopen_build_sync(&sync);
    sync.dlc = 8;
    TEST_ASSERT_FALSE(canopen_parse_emcy(&sync, &node, &code, &reg, out_msef));
}

void test_pdo_roundtrip()
{
    const uint8_t payload[6] = {1, 2, 3, 4, 5, 6};
    CanFrame f;
    TEST_ASSERT_TRUE(canopen_build_tpdo(&f, 2, 7, payload, 6));
    TEST_ASSERT_EQUAL_UINT32(0x287, f.id); // TPDO2 base 0x280 + 7
    TEST_ASSERT_EQUAL_UINT8(6, f.dlc);
    TEST_ASSERT_EQUAL_MEMORY(payload, f.data, 6);

    CanopenMsg m;
    TEST_ASSERT_TRUE(canopen_parse(&f, &m));
    TEST_ASSERT_EQUAL_INT(CANOPEN_T_TPDO, m.type);
    TEST_ASSERT_EQUAL_UINT8(2, m.pdo_num);
    TEST_ASSERT_EQUAL_UINT8(7, m.node_id);

    TEST_ASSERT_TRUE(canopen_build_rpdo(&f, 4, 1, payload, 8));
    TEST_ASSERT_EQUAL_UINT32(0x501, f.id);                       // RPDO4 base 0x500 + 1
    TEST_ASSERT_FALSE(canopen_build_tpdo(&f, 5, 1, payload, 1)); // pdo_num out of range
    TEST_ASSERT_FALSE(canopen_build_tpdo(&f, 1, 1, payload, 9)); // len > 8
}

void test_sdo_read_request()
{
    CanFrame f;
    TEST_ASSERT_TRUE(canopen_build_sdo_read(&f, 0x20, 0x1018, 1)); // identity object, vendor id
    TEST_ASSERT_EQUAL_UINT32(0x620, f.id);                         // 0x600 + 0x20
    TEST_ASSERT_EQUAL_UINT8(8, f.dlc);
    TEST_ASSERT_EQUAL_HEX8(0x40, f.data[0]); // upload initiate
    TEST_ASSERT_EQUAL_HEX8(0x18, f.data[1]); // index LE
    TEST_ASSERT_EQUAL_HEX8(0x10, f.data[2]);
    TEST_ASSERT_EQUAL_HEX8(0x01, f.data[3]); // sub
}

void test_sdo_write_expedited()
{
    const uint8_t val[2] = {0x34, 0x12}; // 0x1234, little-endian
    CanFrame f;
    TEST_ASSERT_TRUE(canopen_build_sdo_write(&f, 5, 0x6040, 0, val, 2));
    TEST_ASSERT_EQUAL_UINT32(0x605, f.id);
    TEST_ASSERT_EQUAL_HEX8(0x2B, f.data[0]); // download, expedited, size indicated, 2 bytes
    TEST_ASSERT_EQUAL_HEX8(0x40, f.data[1]); // index LE
    TEST_ASSERT_EQUAL_HEX8(0x60, f.data[2]);
    TEST_ASSERT_EQUAL_HEX8(0x00, f.data[3]);
    TEST_ASSERT_EQUAL_HEX8(0x34, f.data[4]);
    TEST_ASSERT_EQUAL_HEX8(0x12, f.data[5]);

    // 1-byte write -> command 0x2F; 4-byte -> 0x23; bad lengths fail.
    TEST_ASSERT_TRUE(canopen_build_sdo_write(&f, 5, 0x6040, 0, val, 1));
    TEST_ASSERT_EQUAL_HEX8(0x2F, f.data[0]);
    const uint8_t four[4] = {1, 2, 3, 4};
    TEST_ASSERT_TRUE(canopen_build_sdo_write(&f, 5, 0x6040, 0, four, 4));
    TEST_ASSERT_EQUAL_HEX8(0x23, f.data[0]);
    TEST_ASSERT_FALSE(canopen_build_sdo_write(&f, 5, 0x6040, 0, four, 0));
    TEST_ASSERT_FALSE(canopen_build_sdo_write(&f, 5, 0x6040, 0, four, 5));
}

// A server's expedited upload response (0x580+node) decodes to the value.
void test_sdo_upload_response_expedited()
{
    CanFrame f;
    memset(&f, 0, sizeof(f));
    f.id = 0x580 + 0x20;
    f.dlc = 8;
    f.data[0] = 0x4B; // upload response, expedited, size indicated, 2 bytes (n=2 -> 4-2)
    f.data[1] = 0x18; // index 0x1018
    f.data[2] = 0x10;
    f.data[3] = 0x01; // sub 1
    f.data[4] = 0x9A; // value 0x029A, little-endian
    f.data[5] = 0x02;

    CanopenSdoResponse r;
    TEST_ASSERT_TRUE(canopen_parse_sdo_response(&f, &r));
    TEST_ASSERT_TRUE(r.is_upload);
    TEST_ASSERT_TRUE(r.expedited);
    TEST_ASSERT_FALSE(r.is_abort);
    TEST_ASSERT_EQUAL_HEX16(0x1018, r.index);
    TEST_ASSERT_EQUAL_UINT8(1, r.sub);
    TEST_ASSERT_EQUAL_UINT8(2, r.len);
    TEST_ASSERT_EQUAL_HEX8(0x9A, r.data[0]);
    TEST_ASSERT_EQUAL_HEX8(0x02, r.data[1]);
}

void test_sdo_abort_roundtrip()
{
    CanFrame f;
    TEST_ASSERT_TRUE(canopen_build_sdo_abort(&f, 0x20, 0x1000, 0, CANOPEN_ABORT_NO_OBJECT, false));
    TEST_ASSERT_EQUAL_UINT32(0x580 + 0x20, f.id); // server -> client
    TEST_ASSERT_EQUAL_HEX8(0x80, f.data[0]);

    CanopenSdoResponse r;
    TEST_ASSERT_TRUE(canopen_parse_sdo_response(&f, &r));
    TEST_ASSERT_TRUE(r.is_abort);
    TEST_ASSERT_EQUAL_HEX32(CANOPEN_ABORT_NO_OBJECT, r.abort_code);
    TEST_ASSERT_EQUAL_HEX16(0x1000, r.index);
}

// The download (write) acknowledge: scs=3, no payload.
void test_sdo_download_ack()
{
    CanFrame f;
    memset(&f, 0, sizeof(f));
    f.id = 0x580 + 5;
    f.dlc = 8;
    f.data[0] = 0x60; // download initiate response
    f.data[1] = 0x40;
    f.data[2] = 0x60; // index 0x6040
    CanopenSdoResponse r;
    TEST_ASSERT_TRUE(canopen_parse_sdo_response(&f, &r));
    TEST_ASSERT_FALSE(r.is_abort);
    TEST_ASSERT_FALSE(r.is_upload);
    TEST_ASSERT_EQUAL_UINT8(0, r.len);
}

void test_parse_classifies()
{
    CanFrame f;
    memset(&f, 0, sizeof(f));
    CanopenMsg m;

    f.id = 0x18A; // TPDO1 + node 10
    TEST_ASSERT_TRUE(canopen_parse(&f, &m));
    TEST_ASSERT_EQUAL_INT(CANOPEN_T_TPDO, m.type);
    TEST_ASSERT_EQUAL_UINT8(1, m.pdo_num);
    TEST_ASSERT_EQUAL_UINT8(10, m.node_id);

    f.id = 0x60F; // SDO_RX + node 15
    TEST_ASSERT_TRUE(canopen_parse(&f, &m));
    TEST_ASSERT_EQUAL_INT(CANOPEN_T_SDO_RX, m.type);
    TEST_ASSERT_EQUAL_UINT8(15, m.node_id);

    // Extended frames are not CANopen default-profile.
    f.extended = true;
    TEST_ASSERT_FALSE(canopen_parse(&f, &m));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_nmt_start_node);
    RUN_TEST(test_sync);
    RUN_TEST(test_heartbeat_roundtrip);
    RUN_TEST(test_emcy_roundtrip);
    RUN_TEST(test_pdo_roundtrip);
    RUN_TEST(test_sdo_read_request);
    RUN_TEST(test_sdo_write_expedited);
    RUN_TEST(test_sdo_upload_response_expedited);
    RUN_TEST(test_sdo_abort_roundtrip);
    RUN_TEST(test_sdo_download_ack);
    RUN_TEST(test_parse_classifies);
    return UNITY_END();
}
