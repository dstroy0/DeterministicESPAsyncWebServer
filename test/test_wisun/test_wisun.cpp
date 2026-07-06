// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/wisun: the CoAP client request builder (RFC 7252) + the FAN node registry.

#include "services/wisun/wisun.h"
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

void test_build_coap_get(void)
{
    uint8_t buf[64];
    // CON GET "sensors/temp", msg id 0x1234, no token.
    size_t n = wisun_build_coap(WISUN_COAP_CON, WISUN_COAP_GET, 0x1234, nullptr, 0, "sensors/temp", nullptr, 0, buf,
                                sizeof(buf));
    // Header: 0x40 (ver1, CON, tkl0), code 0x01, mid 0x12 0x34.
    TEST_ASSERT_EQUAL_HEX8(0x40, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0x12, buf[2]);
    TEST_ASSERT_EQUAL_HEX8(0x34, buf[3]);
    // Option 1: Uri-Path "sensors" -> delta 11, len 7 -> 0xB7.
    TEST_ASSERT_EQUAL_HEX8(0xB7, buf[4]);
    TEST_ASSERT_EQUAL_MEMORY("sensors", buf + 5, 7);
    // Option 2: Uri-Path "temp" -> delta 0, len 4 -> 0x04.
    TEST_ASSERT_EQUAL_HEX8(0x04, buf[12]);
    TEST_ASSERT_EQUAL_MEMORY("temp", buf + 13, 4);
    TEST_ASSERT_EQUAL_size_t(17, n);
}

void test_build_coap_put_with_token_and_payload(void)
{
    uint8_t buf[64];
    const uint8_t token[2] = {0xAB, 0xCD};
    const uint8_t body[3] = {0x31, 0x32, 0x33};
    size_t n = wisun_build_coap(WISUN_COAP_NON, WISUN_COAP_PUT, 0x0005, token, 2, "led", body, 3, buf, sizeof(buf));
    // Header: 0x52 (ver=01, type NON=01, tkl=0010), code 0x03 (PUT), mid 0x00 0x05.
    TEST_ASSERT_EQUAL_HEX8(0x52, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x03, buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[2]);
    TEST_ASSERT_EQUAL_HEX8(0x05, buf[3]);
    // Token.
    TEST_ASSERT_EQUAL_HEX8(0xAB, buf[4]);
    TEST_ASSERT_EQUAL_HEX8(0xCD, buf[5]);
    // Uri-Path "led" -> 0xB3.
    TEST_ASSERT_EQUAL_HEX8(0xB3, buf[6]);
    TEST_ASSERT_EQUAL_MEMORY("led", buf + 7, 3);
    // Payload marker + body.
    TEST_ASSERT_EQUAL_HEX8(0xFF, buf[10]);
    TEST_ASSERT_EQUAL_MEMORY(body, buf + 11, 3);
    TEST_ASSERT_EQUAL_size_t(14, n);
}

void test_build_coap_long_segment_extended_length(void)
{
    // A 13-char path segment forces the extended-length nibble (0xD).
    uint8_t buf[64];
    size_t n =
        wisun_build_coap(WISUN_COAP_CON, WISUN_COAP_GET, 1, nullptr, 0, "abcdefghijklm", nullptr, 0, buf, sizeof(buf));
    // Option header: delta 11 (0xB), length 13 -> nibble 0xD, ext byte 13-13=0.
    TEST_ASSERT_EQUAL_HEX8(0xBD, buf[4]);
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[5]); // extended length = 0
    TEST_ASSERT_EQUAL_MEMORY("abcdefghijklm", buf + 6, 13);
    TEST_ASSERT_EQUAL_size_t(4 + 2 + 13, n);
}

void test_build_coap_rejects_bad_args(void)
{
    uint8_t buf[64];
    uint8_t tok[9] = {0};
    TEST_ASSERT_EQUAL_size_t(0, wisun_build_coap(WISUN_COAP_CON, WISUN_COAP_GET, 1, tok, 9, "x", nullptr, 0, buf,
                                                 sizeof(buf))); // tkl > 8
    uint8_t tiny[3];
    TEST_ASSERT_EQUAL_size_t(0, wisun_build_coap(WISUN_COAP_CON, WISUN_COAP_GET, 1, nullptr, 0, "x", nullptr, 0, tiny,
                                                 sizeof(tiny))); // too small
}

void test_node_registry(void)
{
    WisunNode storage[3];
    WisunFan fan;
    DetIp br = det_ip_from_v6_bytes((const uint8_t[16]){0xfd, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1});
    wisun_init(&fan, &br, storage, 3);

    DetIp n1, n2;
    det_ip_parse("fd00::10", &n1);
    det_ip_parse("fd00::11", &n2);
    TEST_ASSERT_EQUAL_INT(0, wisun_node_register(&fan, &n1, 100));
    TEST_ASSERT_EQUAL_INT(1, wisun_node_register(&fan, &n2, 101));
    // Re-register n1 refreshes, does not add.
    TEST_ASSERT_EQUAL_INT(0, wisun_node_register(&fan, &n1, 200));
    TEST_ASSERT_EQUAL_size_t(2, wisun_joined_count(&fan));
    size_t idx = 99;
    TEST_ASSERT_TRUE(wisun_node_find(&fan, &n2, &idx));
    TEST_ASSERT_EQUAL_size_t(1, idx);

    char js[128];
    size_t jn = wisun_nodes_json(&fan, js, sizeof(js));
    TEST_ASSERT_EQUAL_size_t(strlen(js), jn);
    TEST_ASSERT_EQUAL_STRING("[{\"addr\":\"fd00::10\",\"joined\":true},{\"addr\":\"fd00::11\",\"joined\":true}]", js);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_build_coap_get);
    RUN_TEST(test_build_coap_put_with_token_and_payload);
    RUN_TEST(test_build_coap_long_segment_extended_length);
    RUN_TEST(test_build_coap_rejects_bad_args);
    RUN_TEST(test_node_registry);
    return UNITY_END();
}
