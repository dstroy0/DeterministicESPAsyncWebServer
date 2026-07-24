// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the PTP / IEEE 1588-2008 codec (services/ptp): the 10-octet timestamp, the 34-octet
// common header, the Sync / Delay_Req / Follow_Up / Delay_Resp / Announce build+parse round-trips
// (with the type-specific messageType / control / length), and the slave offset / mean-path-delay
// math with a known symmetric-delay case (offset 25 ns, delay 50 ns). Pure host tests.

#include "services/ptp/ptp.h"
#include <stdio.h>
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

static const uint8_t CID[8] = {0x00, 0x11, 0x22, 0xFF, 0xFE, 0x33, 0x44, 0x55};

static void putbe16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)v;
}

// -- timestamp --

void test_timestamp_roundtrip()
{
    DwsPtpTimestamp ts = {0x112233445566ULL, 123456789u}, got;
    uint8_t b[DWS_PTP_TS_LEN];
    dws_ptp_ts_write(b, &ts);
    // 48-bit seconds are big-endian in the first 6 octets.
    TEST_ASSERT_EQUAL_HEX8(0x11, b[0]);
    TEST_ASSERT_EQUAL_HEX8(0x66, b[5]);
    dws_ptp_ts_read(b, &got);
    TEST_ASSERT_EQUAL_UINT64(ts.seconds, got.seconds);
    TEST_ASSERT_EQUAL_UINT32(ts.nanoseconds, got.nanoseconds);
}

void test_timestamp_ns_conversion()
{
    DwsPtpTimestamp ts = {5u, 250000000u};
    TEST_ASSERT_EQUAL_INT64(5250000000LL, dws_ptp_ts_to_ns(&ts));
    DwsPtpTimestamp back;
    dws_ptp_ts_from_ns(5250000000LL, &back);
    TEST_ASSERT_EQUAL_UINT64(5u, back.seconds);
    TEST_ASSERT_EQUAL_UINT32(250000000u, back.nanoseconds);
    // negative clamps to zero (on-wire timestamps are unsigned).
    dws_ptp_ts_from_ns(-1, &back);
    TEST_ASSERT_EQUAL_UINT64(0u, back.seconds);
    TEST_ASSERT_EQUAL_UINT32(0u, back.nanoseconds);
}

// -- header --

void test_header_roundtrip()
{
    DwsPtpHeader h;
    memset(&h, 0, sizeof(h));
    h.message_type = DWS_PTP_SYNC;
    h.transport_specific = 0x1;
    h.version = 2;
    h.domain = 3;
    h.flags = 0x0200; // twoStepFlag
    h.correction = 0x0001020304050607LL;
    memcpy(h.clock_identity, CID, 8);
    h.port_number = 1;
    h.sequence_id = 0x1234;
    h.control = 0x00;
    h.log_interval = -3;

    uint8_t buf[64];
    TEST_ASSERT_EQUAL_UINT(DWS_PTP_HEADER_LEN, dws_ptp_build_header(buf, sizeof(buf), &h, 10));
    // messageLength = header + body.
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[2]);
    TEST_ASSERT_EQUAL_HEX8(44, buf[3]);
    // octet 0 = transportSpecific<<4 | messageType.
    TEST_ASSERT_EQUAL_HEX8(0x10, buf[0]);

    DwsPtpHeader g;
    memset(&g, 0, sizeof(g));
    TEST_ASSERT_TRUE(dws_ptp_parse_header(buf, sizeof(buf), &g));
    TEST_ASSERT_EQUAL_UINT8(DWS_PTP_SYNC, g.message_type);
    TEST_ASSERT_EQUAL_UINT8(0x1, g.transport_specific);
    TEST_ASSERT_EQUAL_UINT8(2, g.version);
    TEST_ASSERT_EQUAL_UINT16(44, g.message_length);
    TEST_ASSERT_EQUAL_UINT8(3, g.domain);
    TEST_ASSERT_EQUAL_HEX16(0x0200, g.flags);
    TEST_ASSERT_EQUAL_HEX64(0x0001020304050607LL, g.correction);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(CID, g.clock_identity, 8);
    TEST_ASSERT_EQUAL_UINT16(1, g.port_number);
    TEST_ASSERT_EQUAL_HEX16(0x1234, g.sequence_id);
    TEST_ASSERT_EQUAL_INT8(-3, g.log_interval);
}

void test_header_rejects()
{
    DwsPtpHeader h;
    uint8_t buf[34];
    TEST_ASSERT_EQUAL_UINT(0, dws_ptp_build_header(nullptr, sizeof(buf), &h, 0));
    TEST_ASSERT_EQUAL_UINT(0, dws_ptp_build_header(buf, 33, &h, 0)); // cap too small
    TEST_ASSERT_FALSE(dws_ptp_parse_header(buf, 33, &h));            // short
    TEST_ASSERT_FALSE(dws_ptp_parse_header(nullptr, 34, &h));
}

// -- timestamp messages (Sync / Delay_Req / Follow_Up) --

void test_sync_delay_req_follow_up()
{
    DwsPtpHeader h;
    memset(&h, 0, sizeof(h));
    memcpy(h.clock_identity, CID, 8);
    h.port_number = 1;
    h.sequence_id = 42;
    // version left 0 -> build must default it to 2.
    DwsPtpTimestamp ts = {1000u, 500u}, got;
    uint8_t buf[64];

    size_t n = dws_ptp_build_sync(buf, sizeof(buf), &h, &ts);
    TEST_ASSERT_EQUAL_UINT(DWS_PTP_HEADER_LEN + DWS_PTP_TS_LEN, n);
    DwsPtpHeader g;
    TEST_ASSERT_TRUE(dws_ptp_parse_timestamp_msg(buf, n, &g, &got));
    TEST_ASSERT_EQUAL_UINT8(DWS_PTP_SYNC, g.message_type);
    TEST_ASSERT_EQUAL_UINT8(2, g.version); // defaulted
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[32]); // control Sync = 0
    TEST_ASSERT_EQUAL_UINT64(1000u, got.seconds);

    n = dws_ptp_build_delay_req(buf, sizeof(buf), &h, &ts);
    TEST_ASSERT_TRUE(dws_ptp_parse_timestamp_msg(buf, n, &g, &got));
    TEST_ASSERT_EQUAL_UINT8(DWS_PTP_DELAY_REQ, g.message_type);
    TEST_ASSERT_EQUAL_HEX8(0x01, buf[32]); // control Delay_Req = 1

    n = dws_ptp_build_follow_up(buf, sizeof(buf), &h, &ts);
    TEST_ASSERT_TRUE(dws_ptp_parse_timestamp_msg(buf, n, &g, &got));
    TEST_ASSERT_EQUAL_UINT8(DWS_PTP_FOLLOW_UP, g.message_type);
    TEST_ASSERT_EQUAL_HEX8(0x02, buf[32]); // control Follow_Up = 2

    // version already 2 -> build leaves it (covers the non-default branch).
    h.version = 2;
    n = dws_ptp_build_sync(buf, sizeof(buf), &h, &ts);
    TEST_ASSERT_TRUE(dws_ptp_parse_timestamp_msg(buf, n, &g, &got));
    TEST_ASSERT_EQUAL_UINT8(2, g.version);
}

void test_timestamp_msg_rejects()
{
    DwsPtpHeader h;
    memset(&h, 0, sizeof(h));
    DwsPtpTimestamp ts = {1u, 2u}, got;
    uint8_t buf[64];
    // bad args to build
    TEST_ASSERT_EQUAL_UINT(0, dws_ptp_build_sync(nullptr, sizeof(buf), &h, &ts));
    TEST_ASSERT_EQUAL_UINT(0, dws_ptp_build_sync(buf, 10, &h, &ts)); // cap too small
    // A Delay_Resp is not a timestamp message -> parse_timestamp_msg rejects it.
    size_t n = dws_ptp_build_delay_resp(buf, sizeof(buf), &h, &ts, CID, 1);
    TEST_ASSERT_FALSE(dws_ptp_parse_timestamp_msg(buf, n, &h, &got));
    // short frame + null out
    n = dws_ptp_build_sync(buf, sizeof(buf), &h, &ts);
    TEST_ASSERT_FALSE(dws_ptp_parse_timestamp_msg(buf, DWS_PTP_HEADER_LEN + 5, &h, &got));
    TEST_ASSERT_FALSE(dws_ptp_parse_timestamp_msg(buf, n, &h, nullptr));
}

// -- Delay_Resp --

void test_delay_resp()
{
    DwsPtpHeader h;
    memset(&h, 0, sizeof(h));
    memcpy(h.clock_identity, CID, 8);
    h.port_number = 1;
    DwsPtpTimestamp t4 = {2000u, 900u};
    const uint8_t reqid[8] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x11, 0x22, 0x33};
    uint8_t buf[64];
    size_t n = dws_ptp_build_delay_resp(buf, sizeof(buf), &h, &t4, reqid, 7);
    TEST_ASSERT_EQUAL_UINT(DWS_PTP_HEADER_LEN + DWS_PTP_TS_LEN + 10, n);
    TEST_ASSERT_EQUAL_HEX8(0x03, buf[32]); // control Delay_Resp = 3

    DwsPtpHeader g;
    DwsPtpDelayResp r;
    TEST_ASSERT_TRUE(dws_ptp_parse_delay_resp(buf, n, &g, &r));
    TEST_ASSERT_EQUAL_UINT8(DWS_PTP_DELAY_RESP, g.message_type);
    TEST_ASSERT_EQUAL_UINT64(2000u, r.receive.seconds);
    TEST_ASSERT_EQUAL_UINT32(900u, r.receive.nanoseconds);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(reqid, r.req_clock_id, 8);
    TEST_ASSERT_EQUAL_UINT16(7, r.req_port);

    // rejects: bad args, wrong type, short, null
    TEST_ASSERT_EQUAL_UINT(0, dws_ptp_build_delay_resp(buf, 20, &h, &t4, reqid, 7));
    TEST_ASSERT_EQUAL_UINT(0, dws_ptp_build_delay_resp(buf, sizeof(buf), &h, &t4, nullptr, 7));
    uint8_t sync[44];
    size_t sn = dws_ptp_build_sync(sync, sizeof(sync), &h, &t4);
    TEST_ASSERT_FALSE(dws_ptp_parse_delay_resp(sync, sn, &g, &r));                     // wrong type
    TEST_ASSERT_FALSE(dws_ptp_parse_delay_resp(buf, DWS_PTP_HEADER_LEN + 12, &g, &r)); // short
    TEST_ASSERT_FALSE(dws_ptp_parse_delay_resp(buf, n, &g, nullptr));
}

// -- Announce --

void test_announce()
{
    DwsPtpHeader h;
    memset(&h, 0, sizeof(h));
    memcpy(h.clock_identity, CID, 8);
    h.port_number = 1;
    h.message_type = DWS_PTP_ANNOUNCE;
    h.control = 0x05;
    h.version = 2;
    uint8_t buf[64];
    dws_ptp_build_header(buf, sizeof(buf), &h, 30);

    const uint8_t gmid[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t *p = buf + DWS_PTP_HEADER_LEN;
    DwsPtpTimestamp origin = {0x0000AABBCCDDULL, 111u};
    dws_ptp_ts_write(p, &origin);
    p += DWS_PTP_TS_LEN;
    putbe16(p, (uint16_t)37);
    p += 2;      // currentUtcOffset
    *p++ = 0;    // reserved
    *p++ = 128;  // priority1
    *p++ = 6;    // clockClass
    *p++ = 0x21; // clockAccuracy
    putbe16(p, 0x4321);
    p += 2;     // variance
    *p++ = 128; // priority2
    memcpy(p, gmid, 8);
    p += 8;
    putbe16(p, 5);
    p += 2;    // stepsRemoved
    *p = 0x20; // timeSource = GPS

    DwsPtpHeader g;
    DwsPtpAnnounce a;
    TEST_ASSERT_TRUE(dws_ptp_parse_announce(buf, DWS_PTP_HEADER_LEN + 30, &g, &a));
    TEST_ASSERT_EQUAL_UINT8(DWS_PTP_ANNOUNCE, g.message_type);
    TEST_ASSERT_EQUAL_UINT64(0x0000AABBCCDDULL, a.origin.seconds);
    TEST_ASSERT_EQUAL_INT16(37, a.utc_offset);
    TEST_ASSERT_EQUAL_UINT8(128, a.gm_priority1);
    TEST_ASSERT_EQUAL_UINT8(6, a.gm_clock_class);
    TEST_ASSERT_EQUAL_UINT8(0x21, a.gm_clock_accuracy);
    TEST_ASSERT_EQUAL_HEX16(0x4321, a.gm_variance);
    TEST_ASSERT_EQUAL_UINT8(128, a.gm_priority2);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(gmid, a.gm_identity, 8);
    TEST_ASSERT_EQUAL_UINT16(5, a.steps_removed);
    TEST_ASSERT_EQUAL_UINT8(0x20, a.time_source);

    // rejects: wrong type, short, null
    uint8_t sync[44];
    size_t sn = dws_ptp_build_sync(sync, sizeof(sync), &h, &origin); // message_type overwritten to SYNC
    TEST_ASSERT_FALSE(dws_ptp_parse_announce(sync, sn, &g, &a));
    TEST_ASSERT_FALSE(dws_ptp_parse_announce(buf, DWS_PTP_HEADER_LEN + 20, &g, &a));
    TEST_ASSERT_FALSE(dws_ptp_parse_announce(buf, DWS_PTP_HEADER_LEN + 30, &g, nullptr));
}

void test_build_announce()
{
    DwsPtpHeader h;
    memset(&h, 0, sizeof(h));
    memcpy(h.clock_identity, CID, 8);
    h.port_number = 1;
    h.sequence_id = 9;
    DwsPtpAnnounce a;
    memset(&a, 0, sizeof(a));
    a.origin.seconds = 0x0000AABBCCDDULL;
    a.origin.nanoseconds = 111u;
    a.utc_offset = 37;
    a.gm_priority1 = 128;
    a.gm_clock_class = 6;
    a.gm_clock_accuracy = 0x21;
    a.gm_variance = 0x4321;
    a.gm_priority2 = 128;
    const uint8_t gmid[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    memcpy(a.gm_identity, gmid, 8);
    a.steps_removed = 5;
    a.time_source = 0x20;

    uint8_t buf[64];
    size_t n = dws_ptp_build_announce(buf, sizeof(buf), &h, &a);
    TEST_ASSERT_EQUAL_UINT(DWS_PTP_HEADER_LEN + 30, n);
    TEST_ASSERT_EQUAL_HEX8(0x05, buf[32]); // control Announce = 5

    DwsPtpHeader g;
    DwsPtpAnnounce b;
    TEST_ASSERT_TRUE(dws_ptp_parse_announce(buf, n, &g, &b));
    TEST_ASSERT_EQUAL_UINT8(DWS_PTP_ANNOUNCE, g.message_type);
    TEST_ASSERT_EQUAL_UINT64(a.origin.seconds, b.origin.seconds);
    TEST_ASSERT_EQUAL_INT16(37, b.utc_offset);
    TEST_ASSERT_EQUAL_UINT8(6, b.gm_clock_class);
    TEST_ASSERT_EQUAL_HEX16(0x4321, b.gm_variance);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(gmid, b.gm_identity, 8);
    TEST_ASSERT_EQUAL_UINT16(5, b.steps_removed);
    TEST_ASSERT_EQUAL_UINT8(0x20, b.time_source);

    TEST_ASSERT_EQUAL_UINT(0, dws_ptp_build_announce(buf, 20, &h, &a)); // cap too small
    TEST_ASSERT_EQUAL_UINT(0, dws_ptp_build_announce(buf, sizeof(buf), &h, nullptr));
}

// -- slave math --

void test_compute_symmetric()
{
    // Symmetric path delay d = 50 ns, true offset o = 25 ns:
    //   t2 = t1 + d + o ; t4 = t3 + d - o
    DwsPtpSync s;
    dws_ptp_compute(0, 75, 200, 225, &s);
    TEST_ASSERT_EQUAL_INT64(25, s.offset_ns);
    TEST_ASSERT_EQUAL_INT64(50, s.delay_ns);

    // Slave ahead of master (negative offset).
    dws_ptp_compute(1000, 1040, 2000, 2060, &s); // ms=40, sm=60 -> offset=-10, delay=50
    TEST_ASSERT_EQUAL_INT64(-10, s.offset_ns);
    TEST_ASSERT_EQUAL_INT64(50, s.delay_ns);

    dws_ptp_compute(0, 0, 0, 0, nullptr); // null-safe
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_timestamp_roundtrip);
    RUN_TEST(test_timestamp_ns_conversion);
    RUN_TEST(test_header_roundtrip);
    RUN_TEST(test_header_rejects);
    RUN_TEST(test_sync_delay_req_follow_up);
    RUN_TEST(test_timestamp_msg_rejects);
    RUN_TEST(test_delay_resp);
    RUN_TEST(test_announce);
    RUN_TEST(test_build_announce);
    RUN_TEST(test_compute_symmetric);
    return UNITY_END();
}
