// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the Haas Machine Data Collection (MDC) Q-command codec (services/haas_mdc): the ?Q
// query builders, the STX/ETB frame + CSV parser, the typed Q500 (program/status/parts) and Q600
// (macro) decoders, the UNKNOWN error, and the DPRNT push de-multiplexer. Byte-exact response vectors
// per the Haas MDC framing (payload between STX 0x02 and ETB 0x17, then CR LF and a '>' prompt).

#include "services/haas_mdc/haas_mdc.h"
#include <string.h>
#include <unity.h>

// Frame delimiters as string-literal pieces (concatenation keeps the \x.. escapes from greedily
// swallowing the following payload character).
#define STX "\x02"
#define ETB "\x17"

void setUp()
{
}
void tearDown()
{
}

// ── query builders ───────────────────────────────────────────────────────────────────────────

void test_build_q()
{
    char buf[32];
    TEST_ASSERT_GREATER_THAN(0, (int)dws_haas_mdc_build_q(buf, sizeof(buf), HAAS_Q_SERIAL));
    TEST_ASSERT_EQUAL_STRING("?Q100\r", buf);
    dws_haas_mdc_build_q(buf, sizeof(buf), HAAS_Q_MODE);
    TEST_ASSERT_EQUAL_STRING("?Q104\r", buf);
    dws_haas_mdc_build_q(buf, sizeof(buf), HAAS_Q_PROGRAM_STATUS);
    TEST_ASSERT_EQUAL_STRING("?Q500\r", buf);
    // overflow fails closed
    char tiny[4];
    TEST_ASSERT_EQUAL_size_t(0, dws_haas_mdc_build_q(tiny, sizeof(tiny), HAAS_Q_SERIAL));
}

void test_build_var()
{
    char buf[32];
    dws_haas_mdc_build_var(buf, sizeof(buf), 100);
    TEST_ASSERT_EQUAL_STRING("?Q600 100\r", buf);
    dws_haas_mdc_build_var(buf, sizeof(buf), 5021);
    TEST_ASSERT_EQUAL_STRING("?Q600 5021\r", buf);
    char tiny[6];
    TEST_ASSERT_EQUAL_size_t(0, dws_haas_mdc_build_var(tiny, sizeof(tiny), 5021));
}

// ── response parsing ─────────────────────────────────────────────────────────────────────────

void test_parse_simple_and_value()
{
    // Q100 -> serial number
    const char *frame = STX "SERIAL NUMBER, 1234567" ETB "\r\n>";
    HaasMdcResp r;
    TEST_ASSERT_TRUE(dws_haas_mdc_parse(frame, strlen(frame), &r));
    TEST_ASSERT_EQUAL_UINT8(2, r.n_fields);
    TEST_ASSERT_EQUAL_MEMORY("SERIAL NUMBER", r.field[0], 13);
    TEST_ASSERT_EQUAL_size_t(13, r.field_len[0]);
    const char *v = nullptr;
    size_t vl = 0;
    TEST_ASSERT_TRUE(dws_haas_mdc_value(&r, &v, &vl)); // leading space trimmed
    TEST_ASSERT_EQUAL_size_t(7, vl);
    TEST_ASSERT_EQUAL_MEMORY("1234567", v, 7);

    // Q104 -> mode token
    const char *mode = STX "MODE, MDI" ETB "\r\n>";
    TEST_ASSERT_TRUE(dws_haas_mdc_parse(mode, strlen(mode), &r));
    TEST_ASSERT_TRUE(dws_haas_mdc_value(&r, &v, &vl));
    TEST_ASSERT_EQUAL_size_t(3, vl);
    TEST_ASSERT_EQUAL_MEMORY("MDI", v, 3);
    TEST_ASSERT_FALSE(dws_haas_mdc_is_error(&r));
}

void test_parse_status_idle()
{
    const char *frame = STX "PROGRAM, O00010, IDLE, PARTS, 42" ETB "\r\n>";
    HaasMdcResp r;
    TEST_ASSERT_TRUE(dws_haas_mdc_parse(frame, strlen(frame), &r));
    TEST_ASSERT_EQUAL_UINT8(5, r.n_fields);
    HaasMdcStatus s;
    TEST_ASSERT_TRUE(dws_haas_mdc_parse_status(&r, &s));
    TEST_ASSERT_FALSE(s.busy);
    TEST_ASSERT_EQUAL_MEMORY("O00010", s.program, 6);
    TEST_ASSERT_EQUAL_size_t(6, s.program_len);
    TEST_ASSERT_EQUAL_MEMORY("IDLE", s.status, 4);
    TEST_ASSERT_EQUAL_size_t(4, s.status_len);
    TEST_ASSERT_TRUE(s.parts_valid);
    TEST_ASSERT_EQUAL_UINT32(42, s.parts);
}

void test_parse_status_busy()
{
    const char *frame = STX "STATUS, BUSY" ETB "\r\n>";
    HaasMdcResp r;
    TEST_ASSERT_TRUE(dws_haas_mdc_parse(frame, strlen(frame), &r));
    HaasMdcStatus s;
    TEST_ASSERT_TRUE(dws_haas_mdc_parse_status(&r, &s));
    TEST_ASSERT_TRUE(s.busy);
    TEST_ASSERT_EQUAL_MEMORY("BUSY", s.status, 4);
    TEST_ASSERT_FALSE(s.parts_valid);
    TEST_ASSERT_NULL(s.program);
}

void test_parse_macro()
{
    // documented 6-decimal form
    const char *f1 = STX "MACRO, 100, 0.000000" ETB "\r\n>";
    HaasMdcResp r;
    TEST_ASSERT_TRUE(dws_haas_mdc_parse(f1, strlen(f1), &r));
    uint32_t var = 0;
    const char *val = nullptr;
    size_t vl = 0;
    TEST_ASSERT_TRUE(dws_haas_mdc_parse_macro(&r, &var, &val, &vl));
    TEST_ASSERT_EQUAL_UINT32(100, var);
    TEST_ASSERT_EQUAL_MEMORY("0.000000", val, 8);
    TEST_ASSERT_EQUAL_size_t(8, vl);

    // space-padded, negative value (value field trimmed of leading padding)
    const char *f2 = STX "MACRO, 5021,       -234.567" ETB "\r\n>";
    TEST_ASSERT_TRUE(dws_haas_mdc_parse(f2, strlen(f2), &r));
    TEST_ASSERT_TRUE(dws_haas_mdc_parse_macro(&r, &var, &val, &vl));
    TEST_ASSERT_EQUAL_UINT32(5021, var);
    TEST_ASSERT_EQUAL_size_t(8, vl);
    TEST_ASSERT_EQUAL_MEMORY("-234.567", val, 8);
}

void test_error_and_no_frame()
{
    const char *unk = STX "UNKNOWN" ETB "\r\n>";
    HaasMdcResp r;
    TEST_ASSERT_TRUE(dws_haas_mdc_parse(unk, strlen(unk), &r));
    TEST_ASSERT_TRUE(dws_haas_mdc_is_error(&r));

    // no STX at all -> not a complete frame yet
    const char *partial = "PROGRAM, O00010";
    TEST_ASSERT_FALSE(dws_haas_mdc_parse(partial, strlen(partial), &r));
    // STX but no ETB -> incomplete
    const char *noetb = STX "MODE, MDI";
    TEST_ASSERT_FALSE(dws_haas_mdc_parse(noetb, strlen(noetb), &r));
}

void test_leading_prompt()
{
    // previous response's trailing '>' prompt precedes this frame in the stream
    const char *frame = ">" STX "MODE, JOG" ETB "\r\n";
    HaasMdcResp r;
    TEST_ASSERT_TRUE(dws_haas_mdc_parse(frame, strlen(frame), &r));
    const char *v = nullptr;
    size_t vl = 0;
    TEST_ASSERT_TRUE(dws_haas_mdc_value(&r, &v, &vl));
    TEST_ASSERT_EQUAL_MEMORY("JOG", v, 3);
}

void test_field_access()
{
    const char *frame = STX "PROGRAM, O00010, FEED HOLD, PARTS, 7" ETB "\r\n>";
    HaasMdcResp r;
    TEST_ASSERT_TRUE(dws_haas_mdc_parse(frame, strlen(frame), &r));
    const char *p = nullptr;
    size_t l = 0;
    TEST_ASSERT_TRUE(dws_haas_mdc_field(&r, 2, &p, &l));
    TEST_ASSERT_EQUAL_size_t(9, l); // "FEED HOLD" (interior space kept)
    TEST_ASSERT_EQUAL_MEMORY("FEED HOLD", p, 9);
    TEST_ASSERT_FALSE(dws_haas_mdc_field(&r, 99, &p, &l)); // out of range
}

void test_dprnt()
{
    // a pushed DPRNT line: raw text + CRLF, no STX/ETB
    const char *line = "X-12.3456\r\n";
    const char *t = nullptr;
    size_t tl = 0;
    TEST_ASSERT_TRUE(dws_haas_mdc_dprnt_line(line, strlen(line), &t, &tl));
    TEST_ASSERT_EQUAL_size_t(9, tl);
    TEST_ASSERT_EQUAL_MEMORY("X-12.3456", t, 9);

    // bracketed by POPEN (DC2 0x12) / PCLOS (DC4 0x14)
    const char *bracket = "\x12"
                          "COUNT 5\r\n"
                          "\x14";
    TEST_ASSERT_TRUE(dws_haas_mdc_dprnt_line(bracket, strlen(bracket), &t, &tl));
    TEST_ASSERT_EQUAL_size_t(7, tl); // interior space (a DPRNT '*') preserved
    TEST_ASSERT_EQUAL_MEMORY("COUNT 5", t, 7);

    // a framed Q response is NOT a DPRNT push (contains STX)
    const char *framed = STX "MODE, MDI" ETB "\r\n>";
    TEST_ASSERT_FALSE(dws_haas_mdc_dprnt_line(framed, strlen(framed), &t, &tl));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_build_q);
    RUN_TEST(test_build_var);
    RUN_TEST(test_parse_simple_and_value);
    RUN_TEST(test_parse_status_idle);
    RUN_TEST(test_parse_status_busy);
    RUN_TEST(test_parse_macro);
    RUN_TEST(test_error_and_no_frame);
    RUN_TEST(test_leading_prompt);
    RUN_TEST(test_field_access);
    RUN_TEST(test_dprnt);
    return UNITY_END();
}
