// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/nts: the NTS-KE record + NTS NTP extension-field wire codec (RFC 8915).

#include "services/nts/nts.h"
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

void test_ke_record(void)
{
    uint8_t body[2] = {0x00, 0x00};
    uint8_t out[8];
    size_t n = detws_nts_ke_record(true, Nts::NTS_KE_NEXT_PROTOCOL, body, 2, out, sizeof(out));
    const uint8_t expect[] = {0x80, 0x01, 0x00, 0x02, 0x00, 0x00}; // critical|type1, len2, NTPv4
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, out, n);
}

void test_ke_request(void)
{
    uint8_t out[32];
    size_t n = detws_nts_ke_request(out, sizeof(out));
    // Next-Protocol(NTPv4) + AEAD(AES-SIV-CMAC-256=15) + End-of-Message, all critical.
    const uint8_t expect[] = {0x80, 0x01, 0x00, 0x02, 0x00, 0x00, // next protocol = 0
                              0x80, 0x04, 0x00, 0x02, 0x00, 0x0F, // aead = 15
                              0x80, 0x00, 0x00, 0x00};            // end of message
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, out, n);
}

struct Collected
{
    int count;
    uint16_t types[8];
    bool crit[8];
};
static void collect(bool critical, uint16_t type, const uint8_t *body, size_t body_len, void *arg)
{
    (void)body;
    (void)body_len;
    Collected *c = (Collected *)arg;
    if (c->count < 8)
    {
        c->types[c->count] = type;
        c->crit[c->count] = critical;
        c->count++;
    }
}

void test_ke_parse(void)
{
    uint8_t req[32];
    size_t n = detws_nts_ke_request(req, sizeof(req));
    Collected c = {0, {0}, {false}};
    TEST_ASSERT_TRUE(detws_nts_ke_parse(req, n, collect, &c));
    TEST_ASSERT_EQUAL_INT(3, c.count);
    TEST_ASSERT_EQUAL_UINT16(Nts::NTS_KE_NEXT_PROTOCOL, c.types[0]);
    TEST_ASSERT_EQUAL_UINT16(Nts::NTS_KE_AEAD_ALGORITHM, c.types[1]);
    TEST_ASSERT_EQUAL_UINT16(Nts::NTS_KE_END_OF_MESSAGE, c.types[2]);
    TEST_ASSERT_TRUE(c.crit[0]);

    // No End-of-Message -> not well-formed.
    uint8_t bad[6] = {0x80, 0x01, 0x00, 0x02, 0x00, 0x00};
    TEST_ASSERT_FALSE(detws_nts_ke_parse(bad, sizeof(bad), nullptr, nullptr));
    // Truncated body -> false.
    uint8_t trunc[5] = {0x80, 0x01, 0x00, 0x02, 0x00};
    TEST_ASSERT_FALSE(detws_nts_ke_parse(trunc, sizeof(trunc), nullptr, nullptr));
}

void test_extension_field_padding(void)
{
    // 32-byte unique id: 4 + 32 = 36, already a multiple of 4.
    uint8_t uid[32];
    memset(uid, 0xAB, sizeof(uid));
    uint8_t out[64];
    size_t n = detws_nts_ef_unique_id(uid, sizeof(uid), out, sizeof(out));
    TEST_ASSERT_EQUAL_size_t(36, n);
    TEST_ASSERT_EQUAL_HEX8(0x01, out[0]);
    TEST_ASSERT_EQUAL_HEX8(0x04, out[1]);
    TEST_ASSERT_EQUAL_UINT16(36, (uint16_t)((out[2] << 8) | out[3])); // Length field

    // 5-byte value: 4 + 5 = 9 -> padded to 12, last 3 bytes zeroed.
    uint8_t v[5] = {1, 2, 3, 4, 5};
    n = detws_nts_ef(NtsEf::NTS_EF_COOKIE, v, 5, out, sizeof(out));
    TEST_ASSERT_EQUAL_size_t(12, n);
    TEST_ASSERT_EQUAL_UINT16(12, (uint16_t)((out[2] << 8) | out[3]));
    TEST_ASSERT_EQUAL_HEX8(0x00, out[9]);
    TEST_ASSERT_EQUAL_HEX8(0x00, out[11]);
}

void test_ef_wrappers_and_guards()
{
    uint8_t out[64];
    uint8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    TEST_ASSERT_TRUE(detws_nts_ef_cookie(data, sizeof(data), out, sizeof(out)) > 0);
    TEST_ASSERT_TRUE(detws_nts_ef_unique_id(data, sizeof(data), out, sizeof(out)) > 0);
    TEST_ASSERT_EQUAL_size_t(0, detws_nts_ef(0x0304, data, sizeof(data), nullptr, sizeof(out))); // null out
    TEST_ASSERT_EQUAL_size_t(0, detws_nts_ef(0x0304, data, sizeof(data), out, 2));               // overflow
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_ke_record);
    RUN_TEST(test_ke_request);
    RUN_TEST(test_ke_parse);
    RUN_TEST(test_extension_field_padding);
    RUN_TEST(test_ef_wrappers_and_guards);
    return UNITY_END();
}
