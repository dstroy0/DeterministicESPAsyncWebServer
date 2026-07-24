// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the UBX codec (services/ubx): the 8-bit Fletcher checksum, frame build / poll,
// one-shot parse (sync + length + checksum validation), the ACK + little-endian readers, and the
// streaming NMEA/UBX demultiplexer. Independent known-answer vectors are the published u-blox poll
// frames UBX-MON-VER (B5 62 0A 04 00 00 0E 34) and UBX-CFG-PRT (B5 62 06 00 00 00 06 18). Pure host
// tests.

#include "services/ubx/ubx.h"
#include <stdio.h>
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// -- checksum + build (external KAT: the MON-VER / CFG-PRT poll frames) --

void test_checksum_known_vector()
{
    const uint8_t body[] = {0x0A, 0x04, 0x00, 0x00}; // MON-VER poll: class id len
    uint8_t a = 0, b = 0;
    dws_ubx_checksum(body, sizeof(body), &a, &b);
    TEST_ASSERT_EQUAL_HEX8(0x0E, a);
    TEST_ASSERT_EQUAL_HEX8(0x34, b);
}

void test_build_poll_mon_ver()
{
    const uint8_t want[] = {0xB5, 0x62, 0x0A, 0x04, 0x00, 0x00, 0x0E, 0x34};
    uint8_t buf[16];
    size_t n = dws_ubx_build_poll(buf, sizeof(buf), 0x0A, 0x04);
    TEST_ASSERT_EQUAL_UINT(sizeof(want), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(want, buf, sizeof(want));
}

void test_build_poll_cfg_prt()
{
    const uint8_t want[] = {0xB5, 0x62, 0x06, 0x00, 0x00, 0x00, 0x06, 0x18};
    uint8_t buf[16];
    size_t n = dws_ubx_build_poll(buf, sizeof(buf), 0x06, 0x00);
    TEST_ASSERT_EQUAL_UINT(sizeof(want), n);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(want, buf, sizeof(want));
}

void test_build_with_payload()
{
    // UBX-CFG-MSG set-rate: enable UBX-NAV-PVT (01 07) at rate 1.
    const uint8_t pay[] = {0x01, 0x07, 0x01};
    uint8_t buf[32];
    size_t n = dws_ubx_build(buf, sizeof(buf), 0x06, 0x01, pay, sizeof(pay));
    TEST_ASSERT_EQUAL_UINT(8u + sizeof(pay), n);
    TEST_ASSERT_EQUAL_HEX8(0xB5, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x62, buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0x06, buf[2]);
    TEST_ASSERT_EQUAL_HEX8(0x01, buf[3]);
    TEST_ASSERT_EQUAL_HEX8((uint8_t)sizeof(pay), buf[4]); // len LE low
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[5]);                 // len LE high
    // Independent checksum over class..payload end.
    uint8_t a = 0, b = 0;
    dws_ubx_checksum(buf + 2, sizeof(pay) + 4u, &a, &b);
    TEST_ASSERT_EQUAL_HEX8(a, buf[n - 2]);
    TEST_ASSERT_EQUAL_HEX8(b, buf[n - 1]);
}

void test_build_rejects_bad_args()
{
    uint8_t buf[8];
    TEST_ASSERT_EQUAL_UINT(0, dws_ubx_build(nullptr, sizeof(buf), 0, 0, nullptr, 0));
    TEST_ASSERT_EQUAL_UINT(0, dws_ubx_build(buf, 4, 0, 0, nullptr, 0)); // cap too small
    const uint8_t p[] = {1};
    TEST_ASSERT_EQUAL_UINT(0, dws_ubx_build(buf, sizeof(buf), 0, 0, nullptr, 1)); // len>0, null payload
    TEST_ASSERT_EQUAL_UINT(0, dws_ubx_build(buf, sizeof(buf), 0, 0, p, 1));       // needs 9, cap 8
}

// -- one-shot parse --

void test_parse_roundtrip()
{
    const uint8_t pay[] = {0xDE, 0xAD, 0xBE, 0xEF};
    uint8_t buf[32];
    size_t n = dws_ubx_build(buf, sizeof(buf), 0x01, 0x07, pay, sizeof(pay));
    DwsUbx m;
    TEST_ASSERT_TRUE(dws_ubx_parse(buf, n, &m));
    TEST_ASSERT_EQUAL_HEX8(0x01, m.cls);
    TEST_ASSERT_EQUAL_HEX8(0x07, m.id);
    TEST_ASSERT_EQUAL_UINT16(sizeof(pay), m.len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(pay, m.payload, sizeof(pay));
}

void test_parse_rejects()
{
    uint8_t buf[16];
    size_t n = dws_ubx_build(buf, sizeof(buf), 0x0A, 0x04, nullptr, 0);
    DwsUbx m;
    TEST_ASSERT_FALSE(dws_ubx_parse(buf, 7, &m));     // too short
    TEST_ASSERT_FALSE(dws_ubx_parse(nullptr, n, &m)); // null
    uint8_t bad_sync[8];
    memcpy(bad_sync, buf, 8);
    bad_sync[1] = 0x63; // wrong sync2
    TEST_ASSERT_FALSE(dws_ubx_parse(bad_sync, 8, &m));
    uint8_t bad_ck[8];
    memcpy(bad_ck, buf, 8);
    bad_ck[7] ^= 0xFF; // corrupt CK_B
    TEST_ASSERT_FALSE(dws_ubx_parse(bad_ck, 8, &m));
    // Declared length longer than the buffer.
    uint8_t long_len[8];
    memcpy(long_len, buf, 8);
    long_len[4] = 0x10; // claim 16-byte payload but only 8 bytes present
    TEST_ASSERT_FALSE(dws_ubx_parse(long_len, 8, &m));
}

// -- ACK + LE readers --

void test_ack()
{
    uint8_t buf[16];
    const uint8_t acked[] = {0x06, 0x01};
    uint8_t ac = 0, ai = 0;
    // ACK-ACK (05 01)
    size_t n = dws_ubx_build(buf, sizeof(buf), 0x05, 0x01, acked, 2);
    DwsUbx m;
    TEST_ASSERT_TRUE(dws_ubx_parse(buf, n, &m));
    TEST_ASSERT_EQUAL_INT(1, dws_ubx_ack(&m, &ac, &ai));
    TEST_ASSERT_EQUAL_HEX8(0x06, ac);
    TEST_ASSERT_EQUAL_HEX8(0x01, ai);
    // ACK-NAK (05 00)
    n = dws_ubx_build(buf, sizeof(buf), 0x05, 0x00, acked, 2);
    TEST_ASSERT_TRUE(dws_ubx_parse(buf, n, &m));
    TEST_ASSERT_EQUAL_INT(0, dws_ubx_ack(&m, nullptr, nullptr));
    // Not an ACK: wrong class.
    n = dws_ubx_build(buf, sizeof(buf), 0x01, 0x07, acked, 2);
    TEST_ASSERT_TRUE(dws_ubx_parse(buf, n, &m));
    TEST_ASSERT_EQUAL_INT(-1, dws_ubx_ack(&m, nullptr, nullptr));
    // Class 05 but unknown id, and too-short payload.
    n = dws_ubx_build(buf, sizeof(buf), 0x05, 0x02, acked, 2);
    TEST_ASSERT_TRUE(dws_ubx_parse(buf, n, &m));
    TEST_ASSERT_EQUAL_INT(-1, dws_ubx_ack(&m, nullptr, nullptr));
    TEST_ASSERT_EQUAL_INT(-1, dws_ubx_ack(nullptr, nullptr, nullptr));
}

void test_le_readers()
{
    const uint8_t p[] = {0x78, 0x56, 0x34, 0x12};
    TEST_ASSERT_EQUAL_HEX16(0x5678, dws_ubx_u16(p, 0));
    TEST_ASSERT_EQUAL_HEX32(0x12345678u, dws_ubx_u32(p, 0));
    const uint8_t neg[] = {0xFF, 0xFF, 0xFF, 0xFF};
    TEST_ASSERT_EQUAL_INT32(-1, dws_ubx_i32(neg, 0));
}

// -- streaming demultiplexer --

// Feed a byte buffer through the demux; collect passthrough bytes and the last decoded frame.
static int drive(DwsUbxStream *st, const uint8_t *in, size_t n, char *pass, size_t *pass_n, DwsUbx *last, int *overflow)
{
    int frames = 0;
    *pass_n = 0;
    *overflow = 0;
    for (size_t i = 0; i < n; i++)
    {
        uint8_t pb = 0;
        int r = dws_ubx_stream_feed(st, in[i], last, &pb);
        if (r == DWS_UBX_FRAME)
            frames++;
        else if (r == DWS_UBX_PASSTHROUGH)
            pass[(*pass_n)++] = (char)pb;
        else if (r == DWS_UBX_OVERFLOW)
            (*overflow)++;
    }
    pass[*pass_n] = 0;
    return frames;
}

void test_stream_demux_mixed()
{
    uint8_t frame[16];
    const uint8_t acked[] = {0x06, 0x01};
    size_t fn = dws_ubx_build(frame, sizeof(frame), 0x05, 0x01, acked, 2);

    // ASCII NMEA, then a UBX frame, then more ASCII.
    uint8_t stream[128];
    size_t sn = 0;
    const char *pre = "$GPGGA,1\r\n";
    const char *post = "$GPRMC,2\r\n";
    for (const char *c = pre; *c; c++)
        stream[sn++] = (uint8_t)*c;
    memcpy(stream + sn, frame, fn);
    sn += fn;
    for (const char *c = post; *c; c++)
        stream[sn++] = (uint8_t)*c;

    DwsUbxStream st;
    dws_ubx_stream_init(&st);
    char pass[128];
    size_t pn = 0;
    DwsUbx last;
    int ov = 0;
    int frames = drive(&st, stream, sn, pass, &pn, &last, &ov);

    TEST_ASSERT_EQUAL_INT(1, frames);
    TEST_ASSERT_EQUAL_INT(0, ov);
    TEST_ASSERT_EQUAL_HEX8(0x05, last.cls);
    TEST_ASSERT_EQUAL_HEX8(0x01, last.id);
    TEST_ASSERT_EQUAL_UINT16(2, last.len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(acked, last.payload, 2);
    // Every non-UBX byte passed through, reconstructing the two NMEA lines verbatim.
    char want[64];
    snprintf(want, sizeof(want), "%s%s", pre, post);
    TEST_ASSERT_EQUAL_STRING(want, pass);
}

void test_stream_bad_checksum_resyncs()
{
    uint8_t frame[16];
    size_t fn = dws_ubx_build(frame, sizeof(frame), 0x0A, 0x04, nullptr, 0);
    frame[fn - 1] ^= 0xFF; // corrupt CK_B

    DwsUbxStream st;
    dws_ubx_stream_init(&st);
    char pass[64];
    size_t pn = 0;
    DwsUbx last;
    int ov = 0;
    int frames = drive(&st, frame, fn, pass, &pn, &last, &ov);
    TEST_ASSERT_EQUAL_INT(0, frames); // bad checksum: no frame emitted
    TEST_ASSERT_EQUAL_INT(0, ov);

    // After the bad frame the demux must still accept a good one.
    uint8_t good[16];
    size_t gn = dws_ubx_build(good, sizeof(good), 0x0A, 0x04, nullptr, 0);
    frames = drive(&st, good, gn, pass, &pn, &last, &ov);
    TEST_ASSERT_EQUAL_INT(1, frames);
}

void test_stream_overflow_skips()
{
    // Hand-craft a frame declaring a payload length above DWS_UBX_MAX_PAYLOAD.
    uint8_t big[8];
    big[0] = DWS_UBX_SYNC1;
    big[1] = DWS_UBX_SYNC2;
    big[2] = 0x01;
    big[3] = 0x02;
    uint16_t big_len = (uint16_t)(DWS_UBX_MAX_PAYLOAD + 1);
    big[4] = (uint8_t)(big_len & 0xFF);
    big[5] = (uint8_t)(big_len >> 8);
    // payload + checksum are just discarded; supply that many filler bytes.
    uint8_t stream[8 + DWS_UBX_MAX_PAYLOAD + 1 + 2];
    memcpy(stream, big, 6);
    size_t sn = 6;
    for (uint16_t i = 0; i < big_len + 2; i++)
        stream[sn++] = 0xAA;

    DwsUbxStream st;
    dws_ubx_stream_init(&st);
    char pass[8];
    size_t pn = 0;
    DwsUbx last;
    int ov = 0;
    int frames = drive(&st, stream, sn, pass, &pn, &last, &ov);
    TEST_ASSERT_EQUAL_INT(0, frames);
    TEST_ASSERT_EQUAL_INT(1, ov); // reported exactly one overflow, then resynced

    // A good frame right after is still parsed.
    uint8_t good[16];
    size_t gn = dws_ubx_build(good, sizeof(good), 0x0A, 0x04, nullptr, 0);
    frames = drive(&st, good, gn, pass, &pn, &last, &ov);
    TEST_ASSERT_EQUAL_INT(1, frames);
}

void test_stream_false_and_double_sync()
{
    DwsUbxStream st;
    dws_ubx_stream_init(&st);
    char pass[16];
    size_t pn = 0;
    DwsUbx last;
    int ov = 0;
    // 0xB5 followed by a non-0x62 => the stray byte passes through; no frame.
    const uint8_t s1[] = {DWS_UBX_SYNC1, 'A'};
    int frames = drive(&st, s1, sizeof(s1), pass, &pn, &last, &ov);
    TEST_ASSERT_EQUAL_INT(0, frames);
    TEST_ASSERT_EQUAL_STRING("A", pass);

    // 0xB5 0xB5 0x62 ... : the doubled sync1 must still open a frame.
    uint8_t frame[8];
    size_t fn = dws_ubx_build(frame, sizeof(frame), 0x0A, 0x04, nullptr, 0);
    uint8_t doubled[16];
    doubled[0] = DWS_UBX_SYNC1;
    memcpy(doubled + 1, frame, fn); // frame already starts with SYNC1 SYNC2 ...
    dws_ubx_stream_init(&st);
    frames = drive(&st, doubled, fn + 1, pass, &pn, &last, &ov);
    TEST_ASSERT_EQUAL_INT(1, frames);
    TEST_ASSERT_EQUAL_INT(0, (int)pn); // nothing passed through
}

void test_stream_null_safe()
{
    TEST_ASSERT_EQUAL_INT(DWS_UBX_NONE, dws_ubx_stream_feed(nullptr, 0x00, nullptr, nullptr));
    dws_ubx_stream_init(nullptr); // must not crash
}

// A full UBX-NAV-PVT frame from a Python u-blox-8 reference (San Francisco-ish fix, 2026-07-24 12:30:45).
static const uint8_t navpvt_frame[100] = {
    0xb5, 0x62, 0x01, 0x07, 0x5c, 0x00, 0x15, 0xcd, 0x5b, 0x07, 0xea, 0x07, 0x07, 0x18, 0x0c, 0x1e, 0x2d,
    0x07, 0x19, 0x00, 0x00, 0x00, 0x20, 0xa1, 0x07, 0x00, 0x03, 0x01, 0x00, 0x09, 0xd8, 0x71, 0x3b, 0xb7,
    0x08, 0xf5, 0x46, 0x16, 0x24, 0x77, 0x00, 0x00, 0x60, 0x6d, 0x00, 0x00, 0xb0, 0x04, 0x00, 0x00, 0x08,
    0x07, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0xce, 0xff, 0xff, 0xff, 0x05, 0x00, 0x00, 0x00, 0x70, 0x00,
    0x00, 0x00, 0x20, 0xaa, 0x44, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x80, 0x84, 0x1e, 0x00, 0xb4, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x69, 0xbe};

void test_nav_pvt_decode()
{
    DwsUbx m;
    TEST_ASSERT_TRUE(dws_ubx_parse(navpvt_frame, sizeof(navpvt_frame), &m));
    TEST_ASSERT_EQUAL_HEX8(DWS_UBX_CLASS_NAV, m.cls);
    TEST_ASSERT_EQUAL_HEX8(DWS_UBX_NAV_PVT, m.id);

    DwsUbxNavPvt pvt;
    TEST_ASSERT_TRUE(dws_ubx_parse_nav_pvt(&m, &pvt));
    TEST_ASSERT_EQUAL_UINT32(123456789u, pvt.itow_ms);
    TEST_ASSERT_EQUAL_UINT16(2026, pvt.year);
    TEST_ASSERT_EQUAL_UINT8(7, pvt.month);
    TEST_ASSERT_EQUAL_UINT8(24, pvt.day);
    TEST_ASSERT_EQUAL_UINT8(12, pvt.hour);
    TEST_ASSERT_EQUAL_UINT8(30, pvt.minute);
    TEST_ASSERT_EQUAL_UINT8(45, pvt.second);
    TEST_ASSERT_EQUAL_HEX8(0x07, pvt.valid);
    TEST_ASSERT_EQUAL_INT32(500000, pvt.nano);
    TEST_ASSERT_EQUAL_UINT32(25, pvt.time_acc_ns);
    TEST_ASSERT_EQUAL_UINT8(DWS_UBX_FIX_3D, pvt.fix_type);
    TEST_ASSERT_TRUE(pvt.flags & DWS_UBX_PVT_FIX_OK);
    TEST_ASSERT_EQUAL_UINT8(9, pvt.num_sv);
    TEST_ASSERT_EQUAL_INT32(-1220841000, pvt.lon_1e7); // -122.0841 deg
    TEST_ASSERT_EQUAL_INT32(373749000, pvt.lat_1e7);   // 37.3749 deg
    TEST_ASSERT_EQUAL_INT32(30500, pvt.height_mm);
    TEST_ASSERT_EQUAL_INT32(28000, pvt.hmsl_mm);
    TEST_ASSERT_EQUAL_UINT32(1200, pvt.h_acc_mm);
    TEST_ASSERT_EQUAL_UINT32(1800, pvt.v_acc_mm);
    TEST_ASSERT_EQUAL_INT32(100, pvt.vel_n_mm_s);
    TEST_ASSERT_EQUAL_INT32(-50, pvt.vel_e_mm_s);
    TEST_ASSERT_EQUAL_INT32(5, pvt.vel_d_mm_s);
    TEST_ASSERT_EQUAL_INT32(112, pvt.gspeed_mm_s);
    TEST_ASSERT_EQUAL_INT32(4500000, pvt.head_mot_1e5);
    TEST_ASSERT_EQUAL_UINT32(30, pvt.s_acc_mm_s);
    TEST_ASSERT_EQUAL_UINT32(2000000, pvt.head_acc_1e5);
    TEST_ASSERT_EQUAL_UINT16(180, pvt.pdop_1e2);
}

void test_nav_pvt_rejects()
{
    DwsUbxNavPvt pvt;
    // Wrong class/id: an ACK-ACK frame is not a NAV-PVT.
    uint8_t ackpl[2] = {0x06, 0x01};
    uint8_t ackbuf[16];
    size_t an = dws_ubx_build(ackbuf, sizeof(ackbuf), 0x05, 0x01, ackpl, 2);
    DwsUbx ack;
    TEST_ASSERT_TRUE(dws_ubx_parse(ackbuf, an, &ack));
    TEST_ASSERT_FALSE(dws_ubx_parse_nav_pvt(&ack, &pvt));

    // Right class/id but a short payload (e.g. an older 84-byte protocol) is rejected.
    uint8_t shortpl[84] = {0};
    uint8_t sbuf[128];
    size_t sn = dws_ubx_build(sbuf, sizeof(sbuf), DWS_UBX_CLASS_NAV, DWS_UBX_NAV_PVT, shortpl, sizeof(shortpl));
    DwsUbx sm;
    TEST_ASSERT_TRUE(dws_ubx_parse(sbuf, sn, &sm));
    TEST_ASSERT_FALSE(dws_ubx_parse_nav_pvt(&sm, &pvt));

    // Null guards.
    TEST_ASSERT_FALSE(dws_ubx_parse_nav_pvt(nullptr, &pvt));
    DwsUbx m;
    dws_ubx_parse(navpvt_frame, sizeof(navpvt_frame), &m);
    TEST_ASSERT_FALSE(dws_ubx_parse_nav_pvt(&m, nullptr));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_checksum_known_vector);
    RUN_TEST(test_build_poll_mon_ver);
    RUN_TEST(test_build_poll_cfg_prt);
    RUN_TEST(test_build_with_payload);
    RUN_TEST(test_build_rejects_bad_args);
    RUN_TEST(test_parse_roundtrip);
    RUN_TEST(test_parse_rejects);
    RUN_TEST(test_ack);
    RUN_TEST(test_le_readers);
    RUN_TEST(test_stream_demux_mixed);
    RUN_TEST(test_stream_bad_checksum_resyncs);
    RUN_TEST(test_stream_overflow_skips);
    RUN_TEST(test_stream_false_and_double_sync);
    RUN_TEST(test_stream_null_safe);
    RUN_TEST(test_nav_pvt_decode);
    RUN_TEST(test_nav_pvt_rejects);
    return UNITY_END();
}
