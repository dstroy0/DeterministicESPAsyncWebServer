// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the HiSLIP (IVI-6.1) message codec (services/hislip): the fixed 16-byte header
// build/parse, the Initialize / AsyncInitialize handshake, and the Data / DataEND framing. The
// byte-exact vectors are built from the IVI-6.1 spec worked examples. Pure host tests.

#include "services/hislip/hislip.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// ── header ────────────────────────────────────────────────────────────────────────────────────

void test_header_roundtrip()
{
    uint8_t buf[16];
    TEST_ASSERT_EQUAL_size_t(
        16, dws_hislip_build_header(buf, sizeof(buf), HislipMsg::DATA_END, 0x01, 0xDEADBEEF, 0x0102030405060708ULL));
    // prologue + fields on the wire
    TEST_ASSERT_EQUAL_HEX8('H', buf[0]);
    TEST_ASSERT_EQUAL_HEX8('S', buf[1]);
    TEST_ASSERT_EQUAL_HEX8(7, buf[2]); // DATA_END
    TEST_ASSERT_EQUAL_HEX8(0x01, buf[3]);
    const uint8_t param_be[] = {0xDE, 0xAD, 0xBE, 0xEF};
    TEST_ASSERT_EQUAL_MEMORY(param_be, buf + 4, 4);
    const uint8_t len_be[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    TEST_ASSERT_EQUAL_MEMORY(len_be, buf + 8, 8);

    HislipHeader h;
    TEST_ASSERT_TRUE(dws_hislip_parse_header(buf, sizeof(buf), &h));
    TEST_ASSERT_EQUAL_UINT8((uint8_t)HislipMsg::DATA_END, (uint8_t)h.type);
    TEST_ASSERT_EQUAL_HEX8(0x01, h.control);
    TEST_ASSERT_EQUAL_HEX32(0xDEADBEEF, h.parameter);
    TEST_ASSERT_EQUAL_HEX64(0x0102030405060708ULL, h.payload_len);
}

void test_header_rejects()
{
    HislipHeader h;
    const uint8_t good[16] = {'H', 'S', 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    TEST_ASSERT_TRUE(dws_hislip_parse_header(good, 16, &h));
    // short buffer
    TEST_ASSERT_FALSE(dws_hislip_parse_header(good, 15, &h));
    // bad prologue
    const uint8_t bad[16] = {'X', 'S', 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    TEST_ASSERT_FALSE(dws_hislip_parse_header(bad, 16, &h));
    // build fails closed on a too-small buffer
    uint8_t small[15];
    TEST_ASSERT_EQUAL_size_t(0, dws_hislip_build_header(small, sizeof(small), HislipMsg::DATA, 0, 0, 0));
}

// ── message type codes (spot-check the enum against the IVI-6.1 table) ──────────────────────────

void test_message_type_codes()
{
    TEST_ASSERT_EQUAL_UINT8(0, (uint8_t)HislipMsg::INITIALIZE);
    TEST_ASSERT_EQUAL_UINT8(1, (uint8_t)HislipMsg::INITIALIZE_RESPONSE);
    TEST_ASSERT_EQUAL_UINT8(6, (uint8_t)HislipMsg::DATA);
    TEST_ASSERT_EQUAL_UINT8(7, (uint8_t)HislipMsg::DATA_END);
    TEST_ASSERT_EQUAL_UINT8(12, (uint8_t)HislipMsg::TRIGGER);
    TEST_ASSERT_EQUAL_UINT8(17, (uint8_t)HislipMsg::ASYNC_INITIALIZE);
    TEST_ASSERT_EQUAL_UINT8(18, (uint8_t)HislipMsg::ASYNC_INITIALIZE_RESPONSE);
    TEST_ASSERT_EQUAL_UINT8(23, (uint8_t)HislipMsg::ASYNC_DEVICE_CLEAR_ACKNOWLEDGE);
    TEST_ASSERT_EQUAL_UINT8(24, (uint8_t)HislipMsg::ASYNC_LOCK_INFO);
    TEST_ASSERT_EQUAL_UINT8(38, (uint8_t)HislipMsg::AUTHENTICATION_RESULT);
}

// ── Initialize (byte-exact IVI-6.1 worked example) ──────────────────────────────────────────────

// Client offers v1.0, vendor "XY" (0x5859), sub-address "hislip0".
void test_build_initialize_vector()
{
    uint8_t buf[64];
    size_t n = dws_hislip_build_initialize(buf, sizeof(buf), DWS_HISLIP_VERSION_1_0, 0x5859, "hislip0");
    const uint8_t expected[] = {'H',  'S',  0x00, 0x00, 0x01, 0x00, 0x58, 0x59, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x07, 'h',  'i',  's',  'l',  'i',  'p',  '0'};
    TEST_ASSERT_EQUAL_size_t(sizeof(expected), n);
    TEST_ASSERT_EQUAL_MEMORY(expected, buf, sizeof(expected));
}

void test_parse_initialize()
{
    uint8_t buf[64];
    size_t n = dws_hislip_build_initialize(buf, sizeof(buf), DWS_HISLIP_VERSION_2_0, 0x4142, "hislip0");
    HislipInitialize init;
    TEST_ASSERT_TRUE(dws_hislip_parse_initialize(buf, n, &init));
    TEST_ASSERT_EQUAL_HEX16(DWS_HISLIP_VERSION_2_0, init.protocol_version);
    TEST_ASSERT_EQUAL_HEX16(0x4142, init.vendor_id);
    TEST_ASSERT_EQUAL_size_t(7, init.sub_address_len);
    TEST_ASSERT_EQUAL_MEMORY("hislip0", init.sub_address, 7);

    // a truncated payload (header claims 7 bytes, only 3 present) is rejected
    TEST_ASSERT_FALSE(dws_hislip_parse_initialize(buf, DWS_HISLIP_HEADER_LEN + 3, &init));
}

void test_initialize_response()
{
    uint8_t buf[16];
    size_t n = dws_hislip_build_initialize_response(buf, sizeof(buf),
                                                    DWS_HISLIP_INITRESP_OVERLAP | DWS_HISLIP_INITRESP_ENC_MANDATORY,
                                                    DWS_HISLIP_VERSION_1_1, 0x0042);
    TEST_ASSERT_EQUAL_size_t(16, n);
    HislipInitializeResponse resp;
    TEST_ASSERT_TRUE(dws_hislip_parse_initialize_response(buf, n, &resp));
    TEST_ASSERT_EQUAL_HEX16(DWS_HISLIP_VERSION_1_1, resp.protocol_version);
    TEST_ASSERT_EQUAL_HEX16(0x0042, resp.session_id);
    TEST_ASSERT_TRUE(resp.overlap);
    TEST_ASSERT_TRUE(resp.encryption_mandatory);
}

void test_async_initialize()
{
    uint8_t buf[16];
    TEST_ASSERT_EQUAL_size_t(16, dws_hislip_build_async_initialize(buf, sizeof(buf), 0x0042));
    HislipHeader h;
    TEST_ASSERT_TRUE(dws_hislip_parse_header(buf, 16, &h));
    TEST_ASSERT_EQUAL_UINT8((uint8_t)HislipMsg::ASYNC_INITIALIZE, (uint8_t)h.type);
    TEST_ASSERT_EQUAL_HEX32(0x0042, h.parameter); // session id in the low 16 bits
    TEST_ASSERT_EQUAL_HEX64(0, h.payload_len);

    // AsyncInitializeResponse carries the server vendor id
    TEST_ASSERT_EQUAL_size_t(16, dws_hislip_build_async_initialize_response(buf, sizeof(buf), 0x01, 0x5859));
    TEST_ASSERT_TRUE(dws_hislip_parse_header(buf, 16, &h));
    TEST_ASSERT_EQUAL_UINT8((uint8_t)HislipMsg::ASYNC_INITIALIZE_RESPONSE, (uint8_t)h.type);
    TEST_ASSERT_EQUAL_HEX32(0x5859, h.parameter);
    TEST_ASSERT_EQUAL_HEX8(0x01, h.control);
}

// ── Data / DataEND (byte-exact IVI-6.1 worked example) ──────────────────────────────────────────

// DataEND carrying "*IDN?\n" at the initial client MessageID 0xFFFFFF00.
void test_build_dataend_vector()
{
    uint8_t buf[64];
    const char *scpi = "*IDN?\n";
    size_t n = dws_hislip_build_data(buf, sizeof(buf), true, 0, DWS_HISLIP_MESSAGE_ID_INIT, (const uint8_t *)scpi,
                                     strlen(scpi));
    const uint8_t expected[] = {'H',  'S',  0x07, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x06, '*',  'I',  'D',  'N',  '?',  '\n'};
    TEST_ASSERT_EQUAL_size_t(sizeof(expected), n);
    TEST_ASSERT_EQUAL_MEMORY(expected, buf, sizeof(expected));
}

void test_data_roundtrip()
{
    uint8_t buf[64];
    const uint8_t payload[] = {'V', 'O', 'L', 'T', '?'};
    size_t n = dws_hislip_build_data(buf, sizeof(buf), false, 0, 0x00001000, payload, sizeof(payload));
    TEST_ASSERT_EQUAL_size_t(16 + sizeof(payload), n);
    HislipHeader h;
    TEST_ASSERT_TRUE(dws_hislip_parse_header(buf, n, &h));
    TEST_ASSERT_EQUAL_UINT8((uint8_t)HislipMsg::DATA, (uint8_t)h.type); // not END
    TEST_ASSERT_EQUAL_HEX32(0x00001000, h.parameter);
    TEST_ASSERT_EQUAL_HEX64(sizeof(payload), h.payload_len);
    TEST_ASSERT_EQUAL_MEMORY(payload, buf + 16, sizeof(payload));
}

void test_message_id_increment()
{
    TEST_ASSERT_EQUAL_HEX32(0xFFFFFF02, dws_hislip_next_message_id(DWS_HISLIP_MESSAGE_ID_INIT));
    TEST_ASSERT_EQUAL_HEX32(0xFFFFFF04, dws_hislip_next_message_id(0xFFFFFF02));
    // unsigned 32-bit wrap
    TEST_ASSERT_EQUAL_HEX32(0x00000001, dws_hislip_next_message_id(0xFFFFFFFF));
    TEST_ASSERT_EQUAL_HEX32(0x00000000, dws_hislip_next_message_id(0xFFFFFFFE));
}

void test_build_overflow()
{
    uint8_t small[20];
    // a 6-byte payload needs 22 bytes; a 20-byte buffer fails closed
    TEST_ASSERT_EQUAL_size_t(0, dws_hislip_build_data(small, sizeof(small), true, 0, 0, (const uint8_t *)"*IDN?\n", 6));
    // null payload with non-zero length is rejected
    uint8_t buf[32];
    TEST_ASSERT_EQUAL_size_t(0, dws_hislip_build_data(buf, sizeof(buf), false, 0, 0, nullptr, 4));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_header_roundtrip);
    RUN_TEST(test_header_rejects);
    RUN_TEST(test_message_type_codes);
    RUN_TEST(test_build_initialize_vector);
    RUN_TEST(test_parse_initialize);
    RUN_TEST(test_initialize_response);
    RUN_TEST(test_async_initialize);
    RUN_TEST(test_build_dataend_vector);
    RUN_TEST(test_data_roundtrip);
    RUN_TEST(test_message_id_increment);
    RUN_TEST(test_build_overflow);
    return UNITY_END();
}
