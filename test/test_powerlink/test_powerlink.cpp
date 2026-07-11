// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/powerlink: the Ethernet POWERLINK basic frame codec.

#include "services/powerlink/powerlink.h"
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

void test_soc(void)
{
    uint8_t out[8];
    size_t n = detws_epl_soc(Epl::EPL_NODE_MN, out, sizeof(out));
    const uint8_t expect[] = {Epl::EPL_MSG_SOC, Epl::EPL_NODE_BROADCAST, Epl::EPL_NODE_MN};
    TEST_ASSERT_EQUAL_size_t(3, n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, out, 3);
}

void test_preq_pres_roundtrip(void)
{
    uint8_t pdo[4] = {0x11, 0x22, 0x33, 0x44};
    uint8_t out[16];
    // PReq: MN (240) -> CN 5, carrying output PDO.
    size_t n = detws_epl_preq(5, Epl::EPL_NODE_MN, pdo, 4, out, sizeof(out));
    TEST_ASSERT_EQUAL_size_t(7, n);
    EplFrame f;
    TEST_ASSERT_TRUE(detws_epl_parse(out, n, &f));
    TEST_ASSERT_EQUAL_HEX8(Epl::EPL_MSG_PREQ, f.msg_type);
    TEST_ASSERT_EQUAL_HEX8(5, f.dest);
    TEST_ASSERT_EQUAL_HEX8(Epl::EPL_NODE_MN, f.source);
    TEST_ASSERT_EQUAL_size_t(4, f.payload_len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(pdo, f.payload, 4);

    // PRes: CN 5 -> broadcast, its input PDO.
    n = detws_epl_pres(5, pdo, 4, out, sizeof(out));
    TEST_ASSERT_TRUE(detws_epl_parse(out, n, &f));
    TEST_ASSERT_EQUAL_HEX8(Epl::EPL_MSG_PRES, f.msg_type);
    TEST_ASSERT_EQUAL_HEX8(Epl::EPL_NODE_BROADCAST, f.dest);
    TEST_ASSERT_EQUAL_HEX8(5, f.source);
}

void test_parse_rejects(void)
{
    EplFrame f;
    uint8_t tooshort[2] = {Epl::EPL_MSG_SOC, 0xFF};
    TEST_ASSERT_FALSE(detws_epl_parse(tooshort, 2, &f));
    // Unknown message type.
    uint8_t bad[3] = {0x99, 0xFF, 0xF0};
    TEST_ASSERT_FALSE(detws_epl_parse(bad, 3, &f));
}

void test_epl_build_guards()
{
    uint8_t out[64];
    uint8_t pdo[4] = {1, 2, 3, 4};
    TEST_ASSERT_EQUAL_size_t(0, detws_epl_build(0x01, 0, 0, nullptr, 4, out, sizeof(out))); // null payload with len
    TEST_ASSERT_EQUAL_size_t(0, detws_epl_build(0x01, 0, 0, pdo, sizeof(pdo), out, 2));     // cap too small
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_soc);
    RUN_TEST(test_preq_pres_roundtrip);
    RUN_TEST(test_parse_rejects);
    RUN_TEST(test_epl_build_guards);
    return UNITY_END();
}
