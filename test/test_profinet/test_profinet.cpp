// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/profinet: the PROFINET DCP frame codec.

#include "services/profinet/profinet.h"
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

void test_header_roundtrip(void)
{
    uint8_t out[16];
    size_t n = dws_pn_dcp_header(Pn::PN_FRAMEID_DCP_IDENT_REQ, Pn::PN_DCP_SERVICE_IDENTIFY, Pn::PN_DCP_TYPE_REQUEST,
                                 0x11223344, 8, out, sizeof(out));
    TEST_ASSERT_EQUAL_size_t(10, n);
    const uint8_t expect[] = {0xFE, 0xFE, 0x05, 0x00, 0x11, 0x22, 0x33, 0x44, 0x00, 0x08};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, out, 10);

    PnDcpHeader h;
    TEST_ASSERT_TRUE(dws_pn_dcp_parse_header(out, n, &h));
    TEST_ASSERT_EQUAL_HEX16(Pn::PN_FRAMEID_DCP_IDENT_REQ, h.frame_id);
    TEST_ASSERT_EQUAL_HEX8(Pn::PN_DCP_SERVICE_IDENTIFY, h.service_id);
    TEST_ASSERT_EQUAL_HEX32(0x11223344, h.xid);
    TEST_ASSERT_EQUAL_UINT16(8, h.data_length);
}

void test_block_even_padding(void)
{
    // NameOfStation "plc" is 3 bytes (odd) -> padded to an even total, filler not counted in length.
    uint8_t out[16];
    size_t n = dws_pn_dcp_block(Pn::PN_DCP_OPT_DEVICE, Pn::PN_DCP_SUB_DEV_NAME_OF_STATION, (const uint8_t *)"plc", 3,
                                out, sizeof(out));
    TEST_ASSERT_EQUAL_size_t(4 + 3 + 1, n); // 4 header + 3 value + 1 pad
    const uint8_t expect[] = {0x02, 0x02, 0x00, 0x03, 'p', 'l', 'c', 0x00};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, out, n);
    // Even value -> no pad.
    n = dws_pn_dcp_block(Pn::PN_DCP_OPT_IP, Pn::PN_DCP_SUB_IP_PARAM, (const uint8_t *)"ABCD", 4, out, sizeof(out));
    TEST_ASSERT_EQUAL_size_t(8, n);
}

struct Seen
{
    int count;
    uint8_t opt[4];
    uint8_t sub[4];
    size_t len[4];
};
static void collect(uint8_t option, uint8_t suboption, const uint8_t *value, size_t value_len, void *arg)
{
    (void)value;
    Seen *s = (Seen *)arg;
    if (s->count < 4)
    {
        s->opt[s->count] = option;
        s->sub[s->count] = suboption;
        s->len[s->count] = value_len;
        s->count++;
    }
}

void test_walk_blocks(void)
{
    uint8_t buf[64];
    size_t n = 0;
    n += dws_pn_dcp_block(Pn::PN_DCP_OPT_DEVICE, Pn::PN_DCP_SUB_DEV_NAME_OF_STATION, (const uint8_t *)"plc", 3, buf + n,
                          sizeof(buf) - n);
    n += dws_pn_dcp_block(Pn::PN_DCP_OPT_IP, Pn::PN_DCP_SUB_IP_PARAM, (const uint8_t *)"ABCD", 4, buf + n,
                          sizeof(buf) - n);

    Seen s = {0, {0}, {0}, {0}};
    TEST_ASSERT_TRUE(dws_pn_dcp_walk(buf, n, collect, &s));
    TEST_ASSERT_EQUAL_INT(2, s.count);
    TEST_ASSERT_EQUAL_HEX8(Pn::PN_DCP_OPT_DEVICE, s.opt[0]);
    TEST_ASSERT_EQUAL_HEX8(Pn::PN_DCP_SUB_DEV_NAME_OF_STATION, s.sub[0]);
    TEST_ASSERT_EQUAL_size_t(3, s.len[0]); // pad not counted
    TEST_ASSERT_EQUAL_HEX8(Pn::PN_DCP_OPT_IP, s.opt[1]);
    TEST_ASSERT_EQUAL_size_t(4, s.len[1]);
}

void test_walk_rejects_truncated(void)
{
    // blockLength claims 10 but only 2 value bytes present.
    uint8_t bad[6] = {0x02, 0x02, 0x00, 0x0A, 0x01, 0x02};
    TEST_ASSERT_FALSE(dws_pn_dcp_walk(bad, sizeof(bad), nullptr, nullptr));
}

// Null-buffer / short-buffer / oversize guards on the header, block, and parse entry points.
void test_pn_guards(void)
{
    uint8_t out[16];
    TEST_ASSERT_EQUAL_size_t(0, dws_pn_dcp_header(0, 0, 0, 0, 0, nullptr, sizeof(out))); // null out
    TEST_ASSERT_EQUAL_size_t(0, dws_pn_dcp_header(0, 0, 0, 0, 0, out, 4));               // cap < header len
    TEST_ASSERT_EQUAL_size_t(0, dws_pn_dcp_block(0, 0, (const uint8_t *)"x", 1, nullptr, sizeof(out))); // null out
    TEST_ASSERT_EQUAL_size_t(0, dws_pn_dcp_block(0, 0, nullptr, 5, out, sizeof(out)));         // len but null value
    TEST_ASSERT_EQUAL_size_t(0, dws_pn_dcp_block(0, 0, (const uint8_t *)"sixval", 6, out, 4)); // block > cap

    PnDcpHeader h;
    TEST_ASSERT_FALSE(dws_pn_dcp_parse_header(nullptr, 10, &h)); // null frame
    uint8_t shortf[9] = {0};
    TEST_ASSERT_FALSE(dws_pn_dcp_parse_header(shortf, sizeof(shortf), &h)); // len < header len
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_header_roundtrip);
    RUN_TEST(test_block_even_padding);
    RUN_TEST(test_walk_blocks);
    RUN_TEST(test_walk_rejects_truncated);
    RUN_TEST(test_pn_guards);
    return UNITY_END();
}
