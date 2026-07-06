// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/mms: the IEC 61850 MMS Read PDU codec.

#include "services/mms/mms.h"
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

static int find(const uint8_t *hay, size_t hlen, const uint8_t *needle, size_t nlen)
{
    if (nlen > hlen)
        return -1;
    for (size_t i = 0; i + nlen <= hlen; i++)
        if (memcmp(hay + i, needle, nlen) == 0)
            return (int)i;
    return -1;
}

void test_read_request_structure(void)
{
    uint8_t out[256];
    size_t n = detws_mms_read_request(42, "LD0/GGIO1$ST$Ind1$stVal", out, sizeof(out));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_HEX8(MMS_PDU_CONFIRMED_REQUEST, out[0]); // 0xA0
    // invokeID 42 -> 02 01 2A present near the front.
    const uint8_t iid[] = {0x02, 0x01, 0x2A};
    TEST_ASSERT_TRUE(find(out, n, iid, 3) >= 0);
    // The read service tag A4 is present.
    const uint8_t svc[] = {MMS_SERVICE_READ};
    TEST_ASSERT_TRUE(find(out, n, svc, 1) >= 0);
    // The VisibleString object name (1A) carrying the item.
    const uint8_t name[] = {0x1A, 0x17, 'L', 'D', '0', '/'};
    TEST_ASSERT_TRUE(find(out, n, name, sizeof(name)) >= 0);
}

void test_read_request_parse(void)
{
    uint8_t buf[256];
    size_t n = detws_mms_read_request(0x1234, "X", buf, sizeof(buf));
    MmsPdu p;
    TEST_ASSERT_TRUE(detws_mms_parse(buf, n, &p));
    TEST_ASSERT_EQUAL_HEX8(MMS_PDU_CONFIRMED_REQUEST, p.pdu_tag);
    TEST_ASSERT_EQUAL_UINT32(0x1234, p.invoke_id);
    TEST_ASSERT_EQUAL_HEX8(MMS_SERVICE_READ, p.service_tag);
    TEST_ASSERT_NOT_NULL(p.service_body);
}

void test_read_response_roundtrip(void)
{
    // A caller-encoded Data value: boolean-ish [3] BOOLEAN true -> 83 01 FF (context Data).
    uint8_t data[] = {0x83, 0x01, 0xFF};
    uint8_t buf[128];
    size_t n = detws_mms_read_response(7, data, sizeof(data), buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_HEX8(MMS_PDU_CONFIRMED_RESPONSE, buf[0]); // 0xA1
    MmsPdu p;
    TEST_ASSERT_TRUE(detws_mms_parse(buf, n, &p));
    TEST_ASSERT_EQUAL_UINT32(7, p.invoke_id);
    TEST_ASSERT_EQUAL_HEX8(MMS_SERVICE_READ, p.service_tag);
    // The data value survives inside the service body.
    TEST_ASSERT_TRUE(find(p.service_body, p.service_len, data, sizeof(data)) >= 0);
}

void test_parse_rejects_bad_tag(void)
{
    uint8_t bad[] = {0x30, 0x03, 0x02, 0x01, 0x01}; // SEQUENCE, not an MMS PDU tag
    MmsPdu p;
    TEST_ASSERT_FALSE(detws_mms_parse(bad, sizeof(bad), &p));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_read_request_structure);
    RUN_TEST(test_read_request_parse);
    RUN_TEST(test_read_response_roundtrip);
    RUN_TEST(test_parse_rejects_bad_tag);
    return UNITY_END();
}
