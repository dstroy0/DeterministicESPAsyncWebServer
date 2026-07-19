// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the SCPI / IEEE 488.2 instrument-control codec (services/scpi): the command
// builder, the response parsers (numeric / boolean / string / arbitrary block), the STB/ESR/ESE/SRE
// + error-queue status model, and the SCPI short/long-form header matcher. Pure host tests.

#include "services/scpi/scpi.h"
#include <math.h>
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// ── common commands ──────────────────────────────────────────────────────────────────────────

void test_common_commands()
{
    TEST_ASSERT_EQUAL_STRING("*CLS", dws_scpi_common(ScpiCommon::SCPI_CLS));
    TEST_ASSERT_EQUAL_STRING("*IDN?", dws_scpi_common(ScpiCommon::SCPI_IDN_Q));
    TEST_ASSERT_EQUAL_STRING("*OPC?", dws_scpi_common(ScpiCommon::SCPI_OPC_Q));
    TEST_ASSERT_EQUAL_STRING("*RST", dws_scpi_common(ScpiCommon::SCPI_RST));
    TEST_ASSERT_EQUAL_STRING("*ESR?", dws_scpi_common(ScpiCommon::SCPI_ESR_Q));
    TEST_ASSERT_EQUAL_STRING("*STB?", dws_scpi_common(ScpiCommon::SCPI_STB_Q));
    TEST_ASSERT_EQUAL_STRING("*WAI", dws_scpi_common(ScpiCommon::SCPI_WAI));
}

// ── command builder ──────────────────────────────────────────────────────────────────────────

void test_build_no_args()
{
    char buf[32];
    size_t n = dws_scpi_build(buf, sizeof(buf), "*RST", nullptr, 0);
    TEST_ASSERT_EQUAL_STRING("*RST\n", buf);
    TEST_ASSERT_EQUAL_size_t(5, n);
}

void test_build_one_arg()
{
    char buf[64];
    const char *args[] = {"1.5"};
    size_t n = dws_scpi_build(buf, sizeof(buf), "SOURce:VOLTage", args, 1);
    TEST_ASSERT_EQUAL_STRING("SOURce:VOLTage 1.5\n", buf);
    TEST_ASSERT_EQUAL_size_t(19, n);
}

void test_build_multi_arg()
{
    char buf[64];
    const char *args[] = {"1.5", "MAX"};
    size_t n = dws_scpi_build(buf, sizeof(buf), "SOUR:VOLT", args, 2);
    TEST_ASSERT_EQUAL_STRING("SOUR:VOLT 1.5,MAX\n", buf);
    TEST_ASSERT_EQUAL_size_t(18, n);
}

void test_build_overflow_and_guards()
{
    char small[8];
    // header alone longer than the buffer
    TEST_ASSERT_EQUAL_size_t(0, dws_scpi_build(small, sizeof(small), "SOURce:VOLTage", nullptr, 0));
    char buf[16];
    const char *args[] = {"1234567890"};
    TEST_ASSERT_EQUAL_size_t(0, dws_scpi_build(buf, sizeof(buf), "VOLT", args, 1)); // arg pushes past cap
    // null guards
    TEST_ASSERT_EQUAL_size_t(0, dws_scpi_build(nullptr, sizeof(buf), "X", nullptr, 0));
    TEST_ASSERT_EQUAL_size_t(0, dws_scpi_build(buf, sizeof(buf), nullptr, nullptr, 0));
    const char *bad[] = {nullptr};
    TEST_ASSERT_EQUAL_size_t(0, dws_scpi_build(buf, sizeof(buf), "V", bad, 1)); // null arg element
}

void test_fmt_real()
{
    char buf[32];
    TEST_ASSERT_GREATER_THAN(0, (int)dws_scpi_fmt_real(buf, sizeof(buf), 1.5));
    TEST_ASSERT_EQUAL_STRING("1.5", buf);
    dws_scpi_fmt_real(buf, sizeof(buf), 0.0);
    TEST_ASSERT_EQUAL_STRING("0", buf);
    dws_scpi_fmt_real(buf, sizeof(buf), -12.25);
    TEST_ASSERT_EQUAL_STRING("-12.25", buf);
    // a tiny buffer fails closed
    char tiny[2];
    TEST_ASSERT_EQUAL_size_t(0, dws_scpi_fmt_real(tiny, sizeof(tiny), 123456.789));
}

// ── numeric response parser ──────────────────────────────────────────────────────────────────

void test_parse_number()
{
    double v = 0;
    TEST_ASSERT_TRUE(dws_scpi_parse_number("42", 2, &v)); // NR1
    TEST_ASSERT_EQUAL_DOUBLE(42.0, v);
    TEST_ASSERT_TRUE(dws_scpi_parse_number("-3.14", 5, &v)); // NR2, sign
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, -3.14, v);
    TEST_ASSERT_TRUE(dws_scpi_parse_number("1.5E3", 5, &v)); // NR3
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, 1500.0, v);
    TEST_ASSERT_TRUE(dws_scpi_parse_number("2.5e-2", 6, &v)); // negative exponent, lowercase e
    TEST_ASSERT_DOUBLE_WITHIN(1e-12, 0.025, v);
    TEST_ASSERT_TRUE(dws_scpi_parse_number("+.5", 3, &v)); // leading '+' and no integer digits
    TEST_ASSERT_DOUBLE_WITHIN(1e-12, 0.5, v);
    TEST_ASSERT_TRUE(dws_scpi_parse_number("9.9E37", 6, &v)); // SCPI +INFinity special value parses
    TEST_ASSERT_TRUE(v > 1e37);
}

void test_parse_number_rejects()
{
    double v = 0;
    TEST_ASSERT_FALSE(dws_scpi_parse_number("", 0, &v));
    TEST_ASSERT_FALSE(dws_scpi_parse_number("abc", 3, &v));
    TEST_ASSERT_FALSE(dws_scpi_parse_number("1.5V", 4, &v)); // trailing unit
    TEST_ASSERT_FALSE(dws_scpi_parse_number("1.5E", 4, &v)); // exponent with no digits
    TEST_ASSERT_FALSE(dws_scpi_parse_number("+", 1, &v));    // sign only
    TEST_ASSERT_FALSE(dws_scpi_parse_number("1 2", 3, &v));  // embedded space
}

// ── boolean response parser ──────────────────────────────────────────────────────────────────

void test_parse_bool()
{
    bool b = false;
    TEST_ASSERT_TRUE(dws_scpi_parse_bool("1", 1, &b));
    TEST_ASSERT_TRUE(b);
    TEST_ASSERT_TRUE(dws_scpi_parse_bool("0", 1, &b));
    TEST_ASSERT_FALSE(b);
    TEST_ASSERT_TRUE(dws_scpi_parse_bool("ON", 2, &b));
    TEST_ASSERT_TRUE(b);
    TEST_ASSERT_TRUE(dws_scpi_parse_bool("off", 3, &b));
    TEST_ASSERT_FALSE(b);
    TEST_ASSERT_TRUE(dws_scpi_parse_bool("On", 2, &b));
    TEST_ASSERT_TRUE(b);
    TEST_ASSERT_FALSE(dws_scpi_parse_bool("YES", 3, &b));
    TEST_ASSERT_FALSE(dws_scpi_parse_bool("", 0, &b));
}

// ── string response parser ───────────────────────────────────────────────────────────────────

void test_parse_string()
{
    char out[32];
    TEST_ASSERT_EQUAL_size_t(5, dws_scpi_parse_string("\"hello\"", 7, out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("hello", out);
    // single quotes
    TEST_ASSERT_EQUAL_size_t(3, dws_scpi_parse_string("'abc'", 5, out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("abc", out);
    // a doubled quote collapses to one
    TEST_ASSERT_EQUAL_size_t(3, dws_scpi_parse_string("\"a\"\"b\"", 6, out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("a\"b", out);
    // empty string
    TEST_ASSERT_EQUAL_size_t(0, dws_scpi_parse_string("\"\"", 2, out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("", out);
    // missing / mismatched quotes -> rejected
    TEST_ASSERT_EQUAL_size_t(0, dws_scpi_parse_string("hello", 5, out, sizeof(out)));
    TEST_ASSERT_EQUAL_size_t(0, dws_scpi_parse_string("\"hello'", 7, out, sizeof(out)));
    // overflow fails closed
    char tiny[3];
    TEST_ASSERT_EQUAL_size_t(0, dws_scpi_parse_string("\"hello\"", 7, tiny, sizeof(tiny)));
}

// ── arbitrary block parser ───────────────────────────────────────────────────────────────────

void test_parse_block_definite()
{
    const uint8_t blk[] = "#14DATA"; // # 1 4 then 4 bytes "DATA"
    const uint8_t *data = nullptr;
    size_t dlen = 0, consumed = 0;
    TEST_ASSERT_TRUE(dws_scpi_parse_block(blk, 7, &data, &dlen, &consumed));
    TEST_ASSERT_EQUAL_size_t(4, dlen);
    TEST_ASSERT_EQUAL_MEMORY("DATA", data, 4);
    TEST_ASSERT_EQUAL_size_t(7, consumed);

    // two length digits: #205HELLO -> length "05" = 5, "HELLO"
    const uint8_t blk2[] = "#205HELLO";
    TEST_ASSERT_TRUE(dws_scpi_parse_block(blk2, 9, &data, &dlen, &consumed));
    TEST_ASSERT_EQUAL_size_t(5, dlen);
    TEST_ASSERT_EQUAL_MEMORY("HELLO", data, 5);
    TEST_ASSERT_EQUAL_size_t(9, consumed);
}

void test_parse_block_indefinite()
{
    const uint8_t blk[] = "#0HELLO\n"; // #0 <data> NL
    const uint8_t *data = nullptr;
    size_t dlen = 0, consumed = 0;
    TEST_ASSERT_TRUE(dws_scpi_parse_block(blk, 8, &data, &dlen, &consumed));
    TEST_ASSERT_EQUAL_size_t(5, dlen);
    TEST_ASSERT_EQUAL_MEMORY("HELLO", data, 5);
    TEST_ASSERT_EQUAL_size_t(8, consumed);
}

void test_parse_block_rejects()
{
    const uint8_t *data = nullptr;
    size_t dlen = 0, consumed = 0;
    // truncated definite block (says 4 bytes, only 3 present)
    const uint8_t trunc[] = "#14DAT";
    TEST_ASSERT_FALSE(dws_scpi_parse_block(trunc, 6, &data, &dlen, &consumed));
    // not a block
    TEST_ASSERT_FALSE(dws_scpi_parse_block((const uint8_t *)"hello", 5, &data, &dlen, &consumed));
    // bad length-count digit
    const uint8_t bad[] = "#X4DATA";
    TEST_ASSERT_FALSE(dws_scpi_parse_block(bad, 7, &data, &dlen, &consumed));
    // indefinite with no terminating newline
    const uint8_t noeol[] = "#0HELLO";
    TEST_ASSERT_FALSE(dws_scpi_parse_block(noeol, 7, &data, &dlen, &consumed));
}

// ── status model ─────────────────────────────────────────────────────────────────────────────

void test_status_error_queue_fifo()
{
    ScpiStatus s;
    dws_scpi_status_init(&s);
    ScpiError e;
    // empty queue -> 0,"No error"
    TEST_ASSERT_FALSE(dws_scpi_pop_error(&s, &e));
    TEST_ASSERT_EQUAL_INT16(0, e.number);
    TEST_ASSERT_EQUAL_STRING("No error", e.msg);

    dws_scpi_push_error(&s, -113, nullptr); // std text filled in
    dws_scpi_push_error(&s, -222, nullptr);
    // FIFO order
    TEST_ASSERT_TRUE(dws_scpi_pop_error(&s, &e));
    TEST_ASSERT_EQUAL_INT16(-113, e.number);
    TEST_ASSERT_EQUAL_STRING("Undefined header", e.msg);
    TEST_ASSERT_TRUE(dws_scpi_pop_error(&s, &e));
    TEST_ASSERT_EQUAL_INT16(-222, e.number);
    TEST_ASSERT_EQUAL_STRING("Data out of range", e.msg);
    TEST_ASSERT_FALSE(dws_scpi_pop_error(&s, &e)); // drained
}

void test_status_esr_class_bits()
{
    ScpiStatus s;
    dws_scpi_status_init(&s);
    dws_scpi_push_error(&s, -100, nullptr); // command error -> CME
    TEST_ASSERT_BITS_HIGH(SCPI_ESR_CME, s.esr);
    dws_scpi_push_error(&s, -200, nullptr); // execution error -> EXE
    TEST_ASSERT_BITS_HIGH(SCPI_ESR_EXE, s.esr);
    dws_scpi_push_error(&s, -310, nullptr); // device-specific -> DDE
    TEST_ASSERT_BITS_HIGH(SCPI_ESR_DDE, s.esr);
    dws_scpi_push_error(&s, -410, nullptr); // query error -> QYE
    TEST_ASSERT_BITS_HIGH(SCPI_ESR_QYE, s.esr);
}

void test_status_stb_and_mss()
{
    ScpiStatus s;
    dws_scpi_status_init(&s);
    // an error in the queue raises EAV
    dws_scpi_push_error(&s, -100, nullptr);
    TEST_ASSERT_BITS_HIGH(SCPI_STB_EAV, dws_scpi_stb(&s));

    // ESR & ESE -> ESB; ESB & SRE -> MSS
    dws_scpi_status_init(&s);
    dws_scpi_event(&s, SCPI_ESR_OPC);
    s.ese = SCPI_ESR_OPC; // enable OPC into the summary
    uint8_t stb = dws_scpi_stb(&s);
    TEST_ASSERT_BITS_HIGH(SCPI_STB_ESB, stb);
    TEST_ASSERT_BITS_LOW(SCPI_STB_MSS, stb); // not requested yet
    s.sre = SCPI_STB_ESB;                    // request service on the event summary
    stb = dws_scpi_stb(&s);
    TEST_ASSERT_BITS_HIGH(SCPI_STB_ESB | SCPI_STB_MSS, stb);

    // ESR event NOT enabled by ESE does not summarize
    dws_scpi_status_init(&s);
    dws_scpi_event(&s, SCPI_ESR_CME);
    TEST_ASSERT_BITS_LOW(SCPI_STB_ESB, dws_scpi_stb(&s));
}

void test_status_cls()
{
    ScpiStatus s;
    dws_scpi_status_init(&s);
    dws_scpi_push_error(&s, -222, nullptr);
    dws_scpi_event(&s, SCPI_ESR_OPC);
    s.ese = SCPI_ESR_OPC; // enables survive *CLS
    s.sre = SCPI_STB_EAV;
    dws_scpi_cls(&s);
    TEST_ASSERT_EQUAL_UINT8(0, s.esr);
    TEST_ASSERT_EQUAL_UINT8(0, s.count);
    TEST_ASSERT_EQUAL_UINT8(SCPI_ESR_OPC, s.ese); // untouched
    TEST_ASSERT_EQUAL_UINT8(SCPI_STB_EAV, s.sre); // untouched
    ScpiError e;
    TEST_ASSERT_FALSE(dws_scpi_pop_error(&s, &e));
}

void test_status_queue_overflow()
{
    ScpiStatus s;
    dws_scpi_status_init(&s);
    for (int i = 0; i < DWS_SCPI_ERR_QUEUE; i++)
        dws_scpi_push_error(&s, (int16_t)(-100 - i), "e");
    dws_scpi_push_error(&s, -222, "one too many"); // overflow -> tail becomes -350
    ScpiError e;
    int16_t last = 0;
    while (dws_scpi_pop_error(&s, &e))
        last = e.number;
    TEST_ASSERT_EQUAL_INT16(-350, last);
}

void test_std_error_lookup()
{
    TEST_ASSERT_EQUAL_STRING("No error", dws_scpi_std_error(0));
    TEST_ASSERT_EQUAL_STRING("Undefined header", dws_scpi_std_error(-113));
    TEST_ASSERT_EQUAL_STRING("Queue overflow", dws_scpi_std_error(-350));
    TEST_ASSERT_EQUAL_STRING("Query UNTERMINATED", dws_scpi_std_error(-420));
    TEST_ASSERT_EQUAL_STRING("", dws_scpi_std_error(-999)); // unknown
}

// ── header matcher ───────────────────────────────────────────────────────────────────────────

static bool match(const char *in, const char *pat)
{
    return dws_scpi_match(in, strlen(in), pat);
}

void test_match_short_long_form()
{
    TEST_ASSERT_TRUE(match("SYST:ERR?", "SYSTem:ERRor?"));     // short form
    TEST_ASSERT_TRUE(match("system:error?", "SYSTem:ERRor?")); // long form, lowercase
    TEST_ASSERT_TRUE(match("SYSTEM:ERROR?", "SYSTem:ERRor?")); // long form, uppercase
    TEST_ASSERT_TRUE(match("MEAS:VOLT:DC?", "MEASure:VOLTage:DC?"));
    // a header may carry parameters after a space - only the header is matched
    TEST_ASSERT_TRUE(match("SOUR:VOLT 1.5", "SOURce:VOLTage"));
}

void test_match_query_suffix()
{
    TEST_ASSERT_FALSE(match("SYST:ERR", "SYSTem:ERRor?")); // pattern is a query, input is not
    TEST_ASSERT_FALSE(match("SYST:ERR?", "SYSTem:ERRor")); // input is a query, pattern is not
    TEST_ASSERT_TRUE(match("SYST:ERR", "SYSTem:ERRor"));   // both non-query
}

void test_match_numeric_suffix()
{
    TEST_ASSERT_TRUE(match("OUTP2:STAT", "OUTPut2:STATe"));  // suffix matches
    TEST_ASSERT_TRUE(match("OUTP:STAT", "OUTPut:STATe"));    // both default to 1
    TEST_ASSERT_FALSE(match("OUTP:STAT", "OUTPut2:STATe"));  // 1 vs 2
    TEST_ASSERT_FALSE(match("OUTP3:STAT", "OUTPut2:STATe")); // 3 vs 2
}

void test_match_common_and_root()
{
    TEST_ASSERT_TRUE(match("*IDN?", "*IDN?"));
    TEST_ASSERT_TRUE(match("*idn?", "*IDN?")); // case-insensitive
    TEST_ASSERT_FALSE(match("*RST", "*IDN?"));
    TEST_ASSERT_TRUE(match(":SYST:ERR?", "SYSTem:ERRor?")); // absolute-root leading ':'
}

void test_match_negatives()
{
    TEST_ASSERT_FALSE(match("MEAS:CURR?", "MEASure:VOLTage?"));  // different node
    TEST_ASSERT_FALSE(match("SYST:ERR:NEXT?", "SYSTem:ERRor?")); // extra depth
    TEST_ASSERT_FALSE(match("SYST?", "SYSTem:ERRor?"));          // too shallow
    TEST_ASSERT_FALSE(match("SYSTE:ERR?", "SYSTem:ERRor?"));     // partial (not short nor long form)
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_common_commands);
    RUN_TEST(test_build_no_args);
    RUN_TEST(test_build_one_arg);
    RUN_TEST(test_build_multi_arg);
    RUN_TEST(test_build_overflow_and_guards);
    RUN_TEST(test_fmt_real);
    RUN_TEST(test_parse_number);
    RUN_TEST(test_parse_number_rejects);
    RUN_TEST(test_parse_bool);
    RUN_TEST(test_parse_string);
    RUN_TEST(test_parse_block_definite);
    RUN_TEST(test_parse_block_indefinite);
    RUN_TEST(test_parse_block_rejects);
    RUN_TEST(test_status_error_queue_fifo);
    RUN_TEST(test_status_esr_class_bits);
    RUN_TEST(test_status_stb_and_mss);
    RUN_TEST(test_status_cls);
    RUN_TEST(test_status_queue_overflow);
    RUN_TEST(test_std_error_lookup);
    RUN_TEST(test_match_short_long_form);
    RUN_TEST(test_match_query_suffix);
    RUN_TEST(test_match_numeric_suffix);
    RUN_TEST(test_match_common_and_root);
    RUN_TEST(test_match_negatives);
    return UNITY_END();
}
