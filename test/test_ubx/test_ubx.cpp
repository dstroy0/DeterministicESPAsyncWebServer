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
    return UNITY_END();
}
