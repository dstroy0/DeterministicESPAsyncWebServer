// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host unit tests for the outbound SNMP notification builder (env:native_snmp_trap).

#include "services/snmp/snmp_ber.h"
#include "services/snmp/snmp_notify.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

static const uint32_t SYSUPTIME0[] = {1, 3, 6, 1, 2, 1, 1, 3, 0};
static const uint32_t SNMPTRAPOID0[] = {1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0};
static const uint32_t TRAP_OID[] = {1, 3, 6, 1, 4, 1, 9999, 2, 0, 1};

static void assert_oid(BerDec *d, const uint32_t *expect, size_t n)
{
    uint32_t oid[SNMP_MAX_OID_LEN];
    size_t on = 0;
    TEST_ASSERT_TRUE(ber_read_oid(d, oid, SNMP_MAX_OID_LEN, &on));
    TEST_ASSERT_EQUAL_size_t(n, on);
    for (size_t i = 0; i < n; i++)
        TEST_ASSERT_EQUAL_UINT32(expect[i], oid[i]);
}

// Build a v2c trap with one extra Gauge32 binding and decode the whole structure.
void test_trap_v2c_structure()
{
    SnmpVarbind vb;
    memset(&vb, 0, sizeof(vb));
    static const uint32_t vboid[] = {1, 3, 6, 1, 4, 1, 9999, 3, 0};
    vb.oid = vboid;
    vb.oid_len = sizeof(vboid) / sizeof(uint32_t);
    vb.type = SNMP_VB_GAUGE32;
    vb.ival = 42;

    uint8_t buf[256];
    size_t len = snmp_notify_build_v2c(buf, sizeof(buf), "public", SNMP_PDU_TRAPV2, 7, TRAP_OID,
                                       sizeof(TRAP_OID) / sizeof(uint32_t), 12345, &vb, 1);
    TEST_ASSERT_GREATER_THAN(0, len);
    TEST_ASSERT_EQUAL_HEX8(0x30, buf[0]);

    BerDec d;
    ber_dec_init(&d, buf, len);
    uint8_t tag;
    size_t l;
    long v;
    TEST_ASSERT_TRUE(ber_read_header(&d, &tag, &l)); // message SEQUENCE
    TEST_ASSERT_EQUAL_HEX8(0x30, tag);
    TEST_ASSERT_TRUE(ber_read_integer(&d, &v)); // version
    TEST_ASSERT_EQUAL(1, v);
    TEST_ASSERT_TRUE(ber_read_header(&d, &tag, &l)); // community OCTET STRING
    TEST_ASSERT_EQUAL_HEX8(0x04, tag);
    TEST_ASSERT_EQUAL_MEMORY("public", buf + d.pos, 6);
    ber_skip(&d, l);
    TEST_ASSERT_TRUE(ber_read_header(&d, &tag, &l)); // PDU
    TEST_ASSERT_EQUAL_HEX8(SNMP_PDU_TRAPV2, tag);
    TEST_ASSERT_TRUE(ber_read_integer(&d, &v)); // request-id
    TEST_ASSERT_EQUAL(7, v);
    TEST_ASSERT_TRUE(ber_read_integer(&d, &v)); // error-status
    TEST_ASSERT_EQUAL(0, v);
    TEST_ASSERT_TRUE(ber_read_integer(&d, &v)); // error-index
    TEST_ASSERT_EQUAL(0, v);
    TEST_ASSERT_TRUE(ber_read_header(&d, &tag, &l)); // varbinds SEQUENCE
    TEST_ASSERT_EQUAL_HEX8(0x30, tag);

    // vb1: sysUpTime.0 = TimeTicks
    TEST_ASSERT_TRUE(ber_read_header(&d, &tag, &l));
    TEST_ASSERT_EQUAL_HEX8(0x30, tag);
    assert_oid(&d, SYSUPTIME0, sizeof(SYSUPTIME0) / sizeof(uint32_t));
    TEST_ASSERT_TRUE(ber_read_header(&d, &tag, &l));
    TEST_ASSERT_EQUAL_HEX8(SNMP_TIMETICKS, tag);
    ber_skip(&d, l);

    // vb2: snmpTrapOID.0 = OID(trap_oid)
    TEST_ASSERT_TRUE(ber_read_header(&d, &tag, &l));
    TEST_ASSERT_EQUAL_HEX8(0x30, tag);
    assert_oid(&d, SNMPTRAPOID0, sizeof(SNMPTRAPOID0) / sizeof(uint32_t));
    assert_oid(&d, TRAP_OID, sizeof(TRAP_OID) / sizeof(uint32_t));

    // vb3: the extra Gauge32 binding
    TEST_ASSERT_TRUE(ber_read_header(&d, &tag, &l));
    TEST_ASSERT_EQUAL_HEX8(0x30, tag);
    assert_oid(&d, vboid, sizeof(vboid) / sizeof(uint32_t));
    TEST_ASSERT_TRUE(ber_read_header(&d, &tag, &l));
    TEST_ASSERT_EQUAL_HEX8(SNMP_GAUGE32, tag);
}

void test_inform_tag()
{
    uint8_t buf[128];
    size_t len = snmp_notify_build_v2c(buf, sizeof(buf), "public", 0xA6 /* InformRequest */, 1, TRAP_OID,
                                       sizeof(TRAP_OID) / sizeof(uint32_t), 1, nullptr, 0);
    TEST_ASSERT_GREATER_THAN(0, len);
    BerDec d;
    ber_dec_init(&d, buf, len);
    uint8_t tag;
    size_t l;
    long v;
    ber_read_header(&d, &tag, &l);
    ber_read_integer(&d, &v);
    ber_read_header(&d, &tag, &l);
    ber_skip(&d, l); // community
    TEST_ASSERT_TRUE(ber_read_header(&d, &tag, &l));
    TEST_ASSERT_EQUAL_HEX8(0xA6, tag); // InformRequest PDU
}

void test_buffer_too_small()
{
    uint8_t buf[8];
    size_t len = snmp_notify_build_v2c(buf, sizeof(buf), "public", SNMP_PDU_TRAPV2, 1, TRAP_OID,
                                       sizeof(TRAP_OID) / sizeof(uint32_t), 1, nullptr, 0);
    TEST_ASSERT_EQUAL_size_t(0, len);
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_trap_v2c_structure);
    RUN_TEST(test_inform_tag);
    RUN_TEST(test_buffer_too_small);
    return UNITY_END();
}
