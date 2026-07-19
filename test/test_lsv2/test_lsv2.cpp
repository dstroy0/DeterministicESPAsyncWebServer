// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the Heidenhain LSV/2 telegram codec (services/lsv2): the framer (4-byte big-endian
// payload-length prefix + 4-char mnemonic + payload), the typed request builders (login / logout,
// filename commands, run-info), and the response readers (T_OK / T_ER error-class+code, data replies,
// stream re-framing). Byte-exact golden vectors per the LSV/2-over-TCP framing (length counts the
// payload only, so a bare telegram is exactly 8 bytes), cross-checked against the pyLSV2 reference.

#include "services/lsv2/lsv2.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// ── framer ───────────────────────────────────────────────────────────────────────────────────

void test_build_no_payload()
{
    uint8_t buf[16];
    // R_ST with no payload -> exactly 8 bytes: 00 00 00 00 'R' '_' 'S' 'T'
    size_t n = dws_lsv2_build(buf, sizeof(buf), DWS_LSV2_CMD_STATUS, nullptr, 0);
    TEST_ASSERT_EQUAL_size_t(8, n);
    const uint8_t want[] = {0x00, 0x00, 0x00, 0x00, 'R', '_', 'S', 'T'};
    TEST_ASSERT_EQUAL_MEMORY(want, buf, 8);
}

void test_build_with_payload()
{
    uint8_t buf[32];
    const uint8_t pay[] = {0xDE, 0xAD, 0xBE, 0xEF};
    size_t n = dws_lsv2_build(buf, sizeof(buf), DWS_LSV2_CMD_PARAM, pay, sizeof(pay));
    TEST_ASSERT_EQUAL_size_t(12, n);
    // length prefix counts payload only (4)
    const uint8_t want[] = {0x00, 0x00, 0x00, 0x04, 'R', '_', 'P', 'R', 0xDE, 0xAD, 0xBE, 0xEF};
    TEST_ASSERT_EQUAL_MEMORY(want, buf, 12);
    // overflow fails closed
    TEST_ASSERT_EQUAL_size_t(0, dws_lsv2_build(buf, 8, DWS_LSV2_CMD_PARAM, pay, sizeof(pay)));
}

void test_build_run_info()
{
    uint8_t buf[16];
    size_t n = dws_lsv2_build_run_info(buf, sizeof(buf), LSV2_RI_PGM_STATE); // 26 -> 0x001A
    TEST_ASSERT_EQUAL_size_t(10, n);
    const uint8_t want[] = {0x00, 0x00, 0x00, 0x02, 'R', '_', 'R', 'I', 0x00, 0x1A};
    TEST_ASSERT_EQUAL_MEMORY(want, buf, 10);
    // tiny buffer fails closed
    uint8_t tiny[9];
    TEST_ASSERT_EQUAL_size_t(0, dws_lsv2_build_run_info(tiny, sizeof(tiny), LSV2_RI_EXEC_STATE));
}

void test_build_login()
{
    uint8_t buf[64];
    // login "INSPECT", no password -> payload "INSPECT\0" (8 bytes)
    size_t n = dws_lsv2_build_login(buf, sizeof(buf), DWS_LSV2_LOGIN_INSPECT, nullptr);
    TEST_ASSERT_EQUAL_size_t(16, n);
    const uint8_t want[] = {0x00, 0x00, 0x00, 0x08, 'A', '_', 'L', 'G', 'I', 'N', 'S', 'P', 'E', 'C', 'T', 0x00};
    TEST_ASSERT_EQUAL_MEMORY(want, buf, 16);

    // login "DNC" with password "pw" -> payload "DNC\0pw\0" (7 bytes)
    n = dws_lsv2_build_login(buf, sizeof(buf), DWS_LSV2_LOGIN_DNC, "pw");
    TEST_ASSERT_EQUAL_size_t(15, n);
    const uint8_t want2[] = {0x00, 0x00, 0x00, 0x07, 'A', '_', 'L', 'G', 'D', 'N', 'C', 0x00, 'p', 'w', 0x00};
    TEST_ASSERT_EQUAL_MEMORY(want2, buf, 15);
}

void test_build_logout()
{
    uint8_t buf[32];
    // no login -> log out of everything -> empty payload, 8 bytes
    size_t n = dws_lsv2_build_logout(buf, sizeof(buf), nullptr);
    TEST_ASSERT_EQUAL_size_t(8, n);
    const uint8_t want[] = {0x00, 0x00, 0x00, 0x00, 'A', '_', 'L', 'O'};
    TEST_ASSERT_EQUAL_MEMORY(want, buf, 8);
    // empty string is treated the same as no login
    TEST_ASSERT_EQUAL_size_t(8, dws_lsv2_build_logout(buf, sizeof(buf), ""));

    // named group -> payload "DNC\0"
    n = dws_lsv2_build_logout(buf, sizeof(buf), DWS_LSV2_LOGIN_DNC);
    TEST_ASSERT_EQUAL_size_t(12, n);
    const uint8_t want2[] = {0x00, 0x00, 0x00, 0x04, 'A', '_', 'L', 'O', 'D', 'N', 'C', 0x00};
    TEST_ASSERT_EQUAL_MEMORY(want2, buf, 12);
}

void test_build_filename()
{
    uint8_t buf[32];
    // R_FL "PGM.H" -> payload "PGM.H\0" (6 bytes)
    size_t n = dws_lsv2_build_filename(buf, sizeof(buf), DWS_LSV2_CMD_FILE_LOAD, "PGM.H");
    TEST_ASSERT_EQUAL_size_t(14, n);
    const uint8_t want[] = {0x00, 0x00, 0x00, 0x06, 'R', '_', 'F', 'L', 'P', 'G', 'M', '.', 'H', 0x00};
    TEST_ASSERT_EQUAL_MEMORY(want, buf, 14);
}

// ── parser ───────────────────────────────────────────────────────────────────────────────────

void test_parse_ok()
{
    const uint8_t frame[] = {0x00, 0x00, 0x00, 0x00, 'T', '_', 'O', 'K'};
    Lsv2Telegram t;
    size_t consumed = 0;
    TEST_ASSERT_TRUE(dws_lsv2_parse(frame, sizeof(frame), &t, &consumed));
    TEST_ASSERT_EQUAL_MEMORY("T_OK", t.mnemonic, 4);
    TEST_ASSERT_EQUAL_size_t(0, t.payload_len);
    TEST_ASSERT_NULL(t.payload);
    TEST_ASSERT_EQUAL_size_t(8, consumed);
    TEST_ASSERT_TRUE(dws_lsv2_is_ok(&t));
    TEST_ASSERT_FALSE(dws_lsv2_is_error(&t));
    TEST_ASSERT_TRUE(dws_lsv2_is(&t, DWS_LSV2_RSP_OK));
}

void test_parse_error()
{
    // T_ER with a 2-byte error-class + error-code payload
    const uint8_t frame[] = {0x00, 0x00, 0x00, 0x02, 'T', '_', 'E', 'R', 0x01, 0x31};
    Lsv2Telegram t;
    TEST_ASSERT_TRUE(dws_lsv2_parse(frame, sizeof(frame), &t, nullptr));
    TEST_ASSERT_TRUE(dws_lsv2_is_error(&t));
    TEST_ASSERT_FALSE(dws_lsv2_is_ok(&t));
    uint8_t ec = 0, code = 0;
    TEST_ASSERT_TRUE(dws_lsv2_error(&t, &ec, &code));
    TEST_ASSERT_EQUAL_UINT8(0x01, ec);
    TEST_ASSERT_EQUAL_UINT8(0x31, code);

    // T_BD (transfer error) is also an error
    const uint8_t xfer[] = {0x00, 0x00, 0x00, 0x02, 'T', '_', 'B', 'D', 0x02, 0x69};
    TEST_ASSERT_TRUE(dws_lsv2_parse(xfer, sizeof(xfer), &t, nullptr));
    TEST_ASSERT_TRUE(dws_lsv2_is_error(&t));
    // a T_OK is not an error and has no 2-byte error payload
    const uint8_t ok[] = {0x00, 0x00, 0x00, 0x00, 'T', '_', 'O', 'K'};
    TEST_ASSERT_TRUE(dws_lsv2_parse(ok, sizeof(ok), &t, nullptr));
    TEST_ASSERT_FALSE(dws_lsv2_error(&t, &ec, &code));
}

void test_parse_data_reply()
{
    // S_RI run-info reply carrying 3 payload bytes
    const uint8_t frame[] = {0x00, 0x00, 0x00, 0x03, 'S', '_', 'R', 'I', 0xAA, 0xBB, 0xCC};
    Lsv2Telegram t;
    size_t consumed = 0;
    TEST_ASSERT_TRUE(dws_lsv2_parse(frame, sizeof(frame), &t, &consumed));
    TEST_ASSERT_TRUE(dws_lsv2_is(&t, DWS_LSV2_RSP_RUN_INFO));
    TEST_ASSERT_EQUAL_size_t(3, t.payload_len);
    TEST_ASSERT_EQUAL_MEMORY("\xAA\xBB\xCC", t.payload, 3);
    TEST_ASSERT_EQUAL_size_t(11, consumed);
}

void test_parse_incomplete()
{
    Lsv2Telegram t;
    // fewer than 8 header bytes -> false, and out is cleared
    const uint8_t partial[] = {0x00, 0x00, 0x00};
    TEST_ASSERT_FALSE(dws_lsv2_parse(partial, sizeof(partial), &t, nullptr));
    TEST_ASSERT_EQUAL_size_t(0, t.payload_len);
    TEST_ASSERT_NULL(t.payload);
    // header declares 4 payload bytes but only 2 are present -> incomplete
    const uint8_t truncated[] = {0x00, 0x00, 0x00, 0x04, 'S', '_', 'F', 'L', 0x11, 0x22};
    TEST_ASSERT_FALSE(dws_lsv2_parse(truncated, sizeof(truncated), &t, nullptr));
}

void test_parse_stream_multi()
{
    // two telegrams back-to-back: T_OK then S_RI(2 bytes)
    const uint8_t stream[] = {0x00, 0x00, 0x00, 0x00, 'T', '_', 'O', 'K',  0x00,
                              0x00, 0x00, 0x02, 'S',  '_', 'R', 'I', 0x07, 0x08};
    Lsv2Telegram t;
    size_t consumed = 0;
    TEST_ASSERT_TRUE(dws_lsv2_parse(stream, sizeof(stream), &t, &consumed));
    TEST_ASSERT_TRUE(dws_lsv2_is_ok(&t));
    TEST_ASSERT_EQUAL_size_t(8, consumed);

    size_t off = consumed;
    TEST_ASSERT_TRUE(dws_lsv2_parse(stream + off, sizeof(stream) - off, &t, &consumed));
    TEST_ASSERT_TRUE(dws_lsv2_is(&t, DWS_LSV2_RSP_RUN_INFO));
    TEST_ASSERT_EQUAL_size_t(2, t.payload_len);
    TEST_ASSERT_EQUAL_MEMORY("\x07\x08", t.payload, 2);
    TEST_ASSERT_EQUAL_size_t(10, consumed);
    TEST_ASSERT_EQUAL_size_t(sizeof(stream), off + consumed);
}

void test_roundtrip()
{
    // build then parse: run-info request survives a frame/parse round trip
    uint8_t buf[16];
    size_t n = dws_lsv2_build_run_info(buf, sizeof(buf), LSV2_RI_OVERRIDE);
    Lsv2Telegram t;
    size_t consumed = 0;
    TEST_ASSERT_TRUE(dws_lsv2_parse(buf, n, &t, &consumed));
    TEST_ASSERT_TRUE(dws_lsv2_is(&t, DWS_LSV2_CMD_RUN_INFO));
    TEST_ASSERT_EQUAL_size_t(2, t.payload_len);
    TEST_ASSERT_EQUAL_UINT8(0x00, t.payload[0]);
    TEST_ASSERT_EQUAL_UINT8(25, t.payload[1]); // LSV2_RI_OVERRIDE
    TEST_ASSERT_EQUAL_size_t(n, consumed);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_build_no_payload);
    RUN_TEST(test_build_with_payload);
    RUN_TEST(test_build_run_info);
    RUN_TEST(test_build_login);
    RUN_TEST(test_build_logout);
    RUN_TEST(test_build_filename);
    RUN_TEST(test_parse_ok);
    RUN_TEST(test_parse_error);
    RUN_TEST(test_parse_data_reply);
    RUN_TEST(test_parse_incomplete);
    RUN_TEST(test_parse_stream_multi);
    RUN_TEST(test_roundtrip);
    return UNITY_END();
}
