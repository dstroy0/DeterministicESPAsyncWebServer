// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the BACnet/IP BVLC + NPDU codec (services/bacnet): the BVLC envelope and
// the NPDU header (control byte + optional addressing). Layout per ASHRAE 135. Pure host tests.

#include "services/bacnet/bacnet.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_bvlc_bytes()
{
    const uint8_t npdu[] = {0x01, 0x00, 0xAA};
    uint8_t buf[16];
    size_t n = dws_bvlc_build(buf, sizeof(buf), BVLC_FUNC_ORIGINAL_UNICAST, npdu, sizeof(npdu));
    const uint8_t expect[] = {0x81, 0x0A, 0x00, 0x07, 0x01, 0x00, 0xAA}; // type, func, len 7, npdu
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);

    uint8_t func;
    const uint8_t *p;
    size_t plen;
    TEST_ASSERT_TRUE(dws_bvlc_parse(buf, n, &func, &p, &plen));
    TEST_ASSERT_EQUAL_HEX8(BVLC_FUNC_ORIGINAL_UNICAST, func);
    TEST_ASSERT_EQUAL_size_t(3, plen);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(npdu, p, 3);
}

// A local APDU NPDU: version + control(0) + apdu, no addressing.
void test_npdu_local()
{
    const uint8_t apdu[] = {0x41, 0x42, 0x43};
    uint8_t buf[16];
    size_t n = dws_npdu_build(buf, sizeof(buf), false, NPDU_PRIO_NORMAL, false, 0, nullptr, 0, 0, apdu, sizeof(apdu));
    const uint8_t expect[] = {0x01, 0x00, 0x41, 0x42, 0x43};
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);

    NpduInfo info;
    TEST_ASSERT_TRUE(dws_npdu_parse(buf, n, &info));
    TEST_ASSERT_FALSE(info.dest_present);
    TEST_ASSERT_FALSE(info.network_message);
    TEST_ASSERT_EQUAL_size_t(3, info.apdu_len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(apdu, info.apdu, 3);
}

// A routed NPDU with a destination: control 0x20, DNET/DLEN/DADR + hop count.
void test_npdu_dest()
{
    const uint8_t dadr[] = {0x0A};
    const uint8_t apdu[] = {0x10};
    uint8_t buf[32];
    size_t n = dws_npdu_build(buf, sizeof(buf), true, NPDU_PRIO_NORMAL, true, 0x0005, dadr, sizeof(dadr), 0xFF, apdu,
                              sizeof(apdu));
    // 01, control(0x20|0x04 reply), 00 05 (dnet), 01 (dlen), 0A (dadr), FF (hop), 10 (apdu)
    const uint8_t expect[] = {0x01, 0x24, 0x00, 0x05, 0x01, 0x0A, 0xFF, 0x10};
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);

    NpduInfo info;
    TEST_ASSERT_TRUE(dws_npdu_parse(buf, n, &info));
    TEST_ASSERT_TRUE(info.dest_present);
    TEST_ASSERT_EQUAL_HEX16(0x0005, info.dnet);
    TEST_ASSERT_EQUAL_HEX8(0xFF, info.hop_count);
    TEST_ASSERT_EQUAL_size_t(1, info.apdu_len);
    TEST_ASSERT_EQUAL_HEX8(0x10, info.apdu[0]);
}

// A global broadcast: DNET 0xFFFF, DLEN 0.
void test_npdu_broadcast()
{
    const uint8_t apdu[] = {0x10, 0x08};
    uint8_t buf[16];
    size_t n =
        dws_npdu_build(buf, sizeof(buf), false, NPDU_PRIO_NORMAL, true, 0xFFFF, nullptr, 0, 0xFF, apdu, sizeof(apdu));
    const uint8_t expect[] = {0x01, 0x20, 0xFF, 0xFF, 0x00, 0xFF, 0x10, 0x08};
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);
}

// The parser skips both destination and source address fields before the APDU.
void test_npdu_parse_with_source()
{
    const uint8_t frame[] = {
        0x01, 0x28,             // version, control: dest + source present
        0x00, 0x05, 0x01, 0x0A, // DNET 5, DLEN 1, DADR 0A
        0x00, 0x03, 0x01, 0x0B, // SNET 3, SLEN 1, SADR 0B
        0xFF,                   // hop count
        0x30, 0x31              // apdu
    };
    NpduInfo info;
    TEST_ASSERT_TRUE(dws_npdu_parse(frame, sizeof(frame), &info));
    TEST_ASSERT_TRUE(info.dest_present);
    TEST_ASSERT_TRUE(info.src_present);
    TEST_ASSERT_EQUAL_HEX16(0x0005, info.dnet);
    TEST_ASSERT_EQUAL_HEX16(0x0003, info.snet);
    TEST_ASSERT_EQUAL_HEX8(0xFF, info.hop_count);
    TEST_ASSERT_EQUAL_size_t(2, info.apdu_len);
    TEST_ASSERT_EQUAL_HEX8(0x30, info.apdu[0]);
}

// Full BACnet/IP stack: BVLC wrapping an NPDU wrapping an APDU.
void test_full_stack()
{
    const uint8_t apdu[] = {0x10, 0x08, 0x12, 0x34};
    uint8_t npdu[16];
    size_t nlen =
        dws_npdu_build(npdu, sizeof(npdu), false, NPDU_PRIO_NORMAL, false, 0, nullptr, 0, 0, apdu, sizeof(apdu));
    uint8_t buf[32];
    size_t n = dws_bvlc_build(buf, sizeof(buf), BVLC_FUNC_ORIGINAL_BROADCAST, npdu, nlen);

    uint8_t func;
    const uint8_t *p;
    size_t plen;
    TEST_ASSERT_TRUE(dws_bvlc_parse(buf, n, &func, &p, &plen));
    TEST_ASSERT_EQUAL_HEX8(BVLC_FUNC_ORIGINAL_BROADCAST, func);
    NpduInfo info;
    TEST_ASSERT_TRUE(dws_npdu_parse(p, plen, &info));
    TEST_ASSERT_EQUAL_size_t(sizeof(apdu), info.apdu_len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(apdu, info.apdu, sizeof(apdu));
}

void test_parse_rejects_bad()
{
    uint8_t func;
    const uint8_t *p;
    size_t plen;
    const uint8_t bad_type[] = {0x82, 0x0A, 0x00, 0x04}; // not BACnet/IP
    TEST_ASSERT_FALSE(dws_bvlc_parse(bad_type, sizeof(bad_type), &func, &p, &plen));
    const uint8_t short_bvlc[] = {0x81, 0x0A, 0x00, 0x08, 0x01}; // declares 8, only 5 buffered
    TEST_ASSERT_FALSE(dws_bvlc_parse(short_bvlc, sizeof(short_bvlc), &func, &p, &plen));

    NpduInfo info;
    const uint8_t bad_ver[] = {0x02, 0x00, 0x41};
    TEST_ASSERT_FALSE(dws_npdu_parse(bad_ver, sizeof(bad_ver), &info));
    const uint8_t trunc_dest[] = {0x01, 0x20, 0x00, 0x05, 0x05, 0x0A}; // DLEN 5 overruns
    TEST_ASSERT_FALSE(dws_npdu_parse(trunc_dest, sizeof(trunc_dest), &info));
}

void test_overflow_fails_closed()
{
    const uint8_t apdu[] = {1, 2, 3, 4};
    uint8_t small[4];
    TEST_ASSERT_EQUAL_size_t(0, dws_bvlc_build(small, sizeof(small), BVLC_FUNC_ORIGINAL_UNICAST, apdu, sizeof(apdu)));
    uint8_t nsmall[4];
    TEST_ASSERT_EQUAL_size_t(0, dws_npdu_build(nsmall, sizeof(nsmall), false, 0, false, 0, nullptr, 0, 0, apdu, 4));
}

// BVLC / NPDU builders fail closed on null args, and NPDU parsing rejects a header
// that claims a destination/source but is truncated.
void test_bacnet_guards_and_truncations()
{
    uint8_t buf[64], npdu[4] = {NPDU_VERSION, 0, 0, 0};
    TEST_ASSERT_EQUAL_UINT(0, dws_bvlc_build(nullptr, sizeof(buf), 0x0A, npdu, 4)); // null buffer
    TEST_ASSERT_EQUAL_UINT(0, dws_bvlc_build(buf, sizeof(buf), 0x0A, nullptr, 5));  // dws_npdu_len w/o npdu
    TEST_ASSERT_EQUAL_UINT(
        0, dws_npdu_build(nullptr, sizeof(buf), false, 0, false, 0, nullptr, 0, 0, npdu, 4)); // null buf
    TEST_ASSERT_EQUAL_UINT(
        0, dws_npdu_build(buf, sizeof(buf), false, 0, false, 0, nullptr, 0, 0, nullptr, 5)); // apdu w/o ptr

    NpduInfo info;
    uint8_t dest_trunc[2] = {NPDU_VERSION, NPCI_DEST_PRESENT};
    TEST_ASSERT_FALSE(dws_npdu_parse(dest_trunc, 2, &info)); // destination announced, none present
    uint8_t src_trunc[2] = {NPDU_VERSION, NPCI_SRC_PRESENT};
    TEST_ASSERT_FALSE(dws_npdu_parse(src_trunc, 2, &info));                            // source announced, none present
    uint8_t src_overrun[6] = {NPDU_VERSION, NPCI_SRC_PRESENT, 0x00, 0x01, 0xFF, 0x00}; // SLEN overruns the buffer
    TEST_ASSERT_FALSE(dws_npdu_parse(src_overrun, 6, &info));
    uint8_t no_hop[5] = {NPDU_VERSION, NPCI_DEST_PRESENT, 0x00, 0x01, 0x00}; // valid dest, then no hop-count byte
    TEST_ASSERT_FALSE(dws_npdu_parse(no_hop, 5, &info));
}

// dws_bvlc_build: a zero-length NPDU payload (e.g. a header-only BVLC-Result) exercises the
// "no npdu" side of the length guard and skips the memcpy; a declared length that itself
// exceeds the 16-bit BVLL length field trips the overflow guard before the cap is even checked.
void test_bvlc_build_zero_len_and_giant_overflow()
{
    uint8_t buf[16];
    size_t n = dws_bvlc_build(buf, sizeof(buf), BVLC_FUNC_RESULT, nullptr, 0);
    const uint8_t expect[] = {0x81, 0x00, 0x00, 0x04}; // type, func, len 4 (header only)
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);

    uint8_t dummy_npdu[1] = {0};
    TEST_ASSERT_EQUAL_size_t(
        0, dws_bvlc_build(buf, sizeof(buf), BVLC_FUNC_ORIGINAL_UNICAST, dummy_npdu, 0x10000)); // total > 0xFFFF
}

// dws_bvlc_parse: a null buffer, a buffer shorter than the fixed header, a declared BVLL
// length that is itself smaller than the header, and all-null optional outputs.
void test_bvlc_parse_edge_branches()
{
    uint8_t func;
    const uint8_t *p;
    size_t plen;
    TEST_ASSERT_FALSE(dws_bvlc_parse(nullptr, 8, &func, &p, &plen)); // null buffer

    const uint8_t too_short[2] = {0x81, 0x0A};
    TEST_ASSERT_FALSE(dws_bvlc_parse(too_short, sizeof(too_short), &func, &p, &plen)); // len < header

    const uint8_t small_total[4] = {0x81, 0x0A, 0x00, 0x03}; // declares total 3, less than the header
    TEST_ASSERT_FALSE(dws_bvlc_parse(small_total, sizeof(small_total), &func, &p, &plen));

    const uint8_t header_only[4] = {0x81, 0x0A, 0x00, 0x04}; // valid, no npdu payload
    TEST_ASSERT_TRUE(
        dws_bvlc_parse(header_only, sizeof(header_only), nullptr, nullptr, nullptr)); // all outputs optional
}

// dws_npdu_build: a zero-length APDU (header-only NPDU) exercises the "no apdu" side of the
// length guard and skips the trailing memcpy; a destination announced (dadr_len != 0) with a
// null DADR pointer trips the guard and fails closed.
void test_npdu_build_zero_apdu_and_null_dadr()
{
    uint8_t buf[16];
    size_t n = dws_npdu_build(buf, sizeof(buf), false, NPDU_PRIO_NORMAL, false, 0, nullptr, 0, 0, nullptr, 0);
    const uint8_t expect[] = {0x01, 0x00};
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);

    TEST_ASSERT_EQUAL_size_t(
        0, dws_npdu_build(buf, sizeof(buf), false, NPDU_PRIO_NORMAL, true, 0x0005, nullptr, 3, 0xFF, nullptr, 0));
}

// dws_npdu_parse: a null buffer, a null output pointer, and a length too short to hold the
// fixed version+control octets.
void test_npdu_parse_null_buf_out_and_short()
{
    const uint8_t frame[2] = {NPDU_VERSION, 0x00};
    NpduInfo info;
    TEST_ASSERT_FALSE(dws_npdu_parse(nullptr, sizeof(frame), &info)); // null buffer
    TEST_ASSERT_FALSE(dws_npdu_parse(frame, sizeof(frame), nullptr)); // null output
    TEST_ASSERT_FALSE(dws_npdu_parse(frame, 1, &info));               // len < 2
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_bacnet_guards_and_truncations);
    RUN_TEST(test_bvlc_bytes);
    RUN_TEST(test_npdu_local);
    RUN_TEST(test_npdu_dest);
    RUN_TEST(test_npdu_broadcast);
    RUN_TEST(test_npdu_parse_with_source);
    RUN_TEST(test_full_stack);
    RUN_TEST(test_parse_rejects_bad);
    RUN_TEST(test_overflow_fails_closed);
    RUN_TEST(test_bvlc_build_zero_len_and_giant_overflow);
    RUN_TEST(test_bvlc_parse_edge_branches);
    RUN_TEST(test_npdu_build_zero_apdu_and_null_dadr);
    RUN_TEST(test_npdu_parse_null_buf_out_and_short);
    return UNITY_END();
}
