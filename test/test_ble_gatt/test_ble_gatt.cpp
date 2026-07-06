// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/ble_gatt: the ATT PDU codec + GATT characteristic JSON.

#include "services/ble_gatt/ble_gatt.h"
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

void test_build_pdus(void)
{
    uint8_t buf[16];
    // Read Request handle 0x0025.
    TEST_ASSERT_EQUAL_size_t(3, att_read_req(0x0025, buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_HEX8(ATT_OP_READ_REQ, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x25, buf[1]); // LE low
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[2]); // LE high
    // Write Request.
    const uint8_t val[3] = {0xDE, 0xAD, 0xBE};
    size_t n = att_write_req(0x0031, val, 3, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_size_t(6, n);
    TEST_ASSERT_EQUAL_HEX8(ATT_OP_WRITE_REQ, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x31, buf[1]);
    TEST_ASSERT_EQUAL_MEMORY(val, buf + 3, 3);
    // Notification.
    n = att_notify(0x0031, val, 3, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_HEX8(ATT_OP_HANDLE_VALUE_NTF, buf[0]);
    // Error Response.
    n = att_error_rsp(ATT_OP_READ_REQ, 0x0025, 0x0A, buf, sizeof(buf)); // 0x0A = Attribute Not Found
    TEST_ASSERT_EQUAL_size_t(5, n);
    TEST_ASSERT_EQUAL_HEX8(ATT_OP_ERROR_RSP, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(ATT_OP_READ_REQ, buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0x0A, buf[4]);
}

void test_build_overflow(void)
{
    uint8_t tiny[2];
    TEST_ASSERT_EQUAL_size_t(0, att_read_req(0x0025, tiny, sizeof(tiny)));
}

void test_parse(void)
{
    AttPdu p;
    // Write Request with value.
    const uint8_t w[] = {ATT_OP_WRITE_REQ, 0x31, 0x00, 0x01, 0x02};
    TEST_ASSERT_TRUE(att_parse(w, sizeof(w), &p));
    TEST_ASSERT_EQUAL_HEX8(ATT_OP_WRITE_REQ, p.opcode);
    TEST_ASSERT_EQUAL_HEX16(0x0031, p.handle);
    TEST_ASSERT_EQUAL_size_t(2, p.value_len);
    TEST_ASSERT_EQUAL_HEX8(0x01, p.value[0]);
    // Error Response.
    const uint8_t e[] = {ATT_OP_ERROR_RSP, ATT_OP_READ_REQ, 0x25, 0x00, 0x0A};
    TEST_ASSERT_TRUE(att_parse(e, sizeof(e), &p));
    TEST_ASSERT_EQUAL_HEX8(ATT_OP_READ_REQ, p.req_op);
    TEST_ASSERT_EQUAL_HEX16(0x0025, p.handle);
    TEST_ASSERT_EQUAL_HEX8(0x0A, p.error);
    // Read Response value.
    const uint8_t r[] = {ATT_OP_READ_RSP, 0xAA, 0xBB};
    TEST_ASSERT_TRUE(att_parse(r, sizeof(r), &p));
    TEST_ASSERT_EQUAL_size_t(2, p.value_len);
    // Truncated write req -> false.
    const uint8_t bad[] = {ATT_OP_WRITE_REQ, 0x31};
    TEST_ASSERT_FALSE(att_parse(bad, sizeof(bad), &p));
}

void test_char_json(void)
{
    GattChar chars[2] = {{0x0025, 0x2A37, GATT_PROP_READ | GATT_PROP_NOTIFY}, // Heart Rate Measurement
                         {0x0031, 0x2A6E, GATT_PROP_READ}};                   // Temperature
    char buf[160];
    size_t n = gatt_char_json(chars, 2, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_size_t(strlen(buf), n);
    TEST_ASSERT_EQUAL_STRING(
        "[{\"handle\":37,\"uuid\":\"0x2a37\",\"props\":18},{\"handle\":49,\"uuid\":\"0x2a6e\",\"props\":2}]", buf);
    // Empty.
    gatt_char_json(nullptr, 0, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("[]", buf);
    // Overflow: a buffer too small returns 0.
    char tiny[8];
    TEST_ASSERT_EQUAL_size_t(0, gatt_char_json(chars, 2, tiny, sizeof(tiny)));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_build_pdus);
    RUN_TEST(test_build_overflow);
    RUN_TEST(test_parse);
    RUN_TEST(test_char_json);
    return UNITY_END();
}
