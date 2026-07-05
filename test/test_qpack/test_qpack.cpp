// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the QPACK codec (network_drivers/presentation/http3/qpack, RFC 9204): the
// Appendix B.1 worked example (literal field line with a static name reference), the encoder's
// exact bytes for an indexed / name-reference / literal-name field line, a round-trip through all
// three representations, and rejection of any dynamic-table reference (non-zero Required Insert
// Count, dynamic index/name, post-base representations). Static-table-only, pure host codec.

#include "network_drivers/presentation/http3/qpack.h"
#include <string.h>
#include <string>
#include <unity.h>
#include <utility>
#include <vector>

struct Sink
{
    std::vector<std::pair<std::string, std::string>> hdrs;
};

static bool sink_emit(void *ctx, const char *n, size_t nl, const char *v, size_t vl)
{
    ((Sink *)ctx)->hdrs.push_back({std::string(n, nl), std::string(v, vl)});
    return true;
}

static bool decode_all(const uint8_t *block, size_t len, Sink *s)
{
    char scratch[512];
    return qpack_decode(block, len, scratch, sizeof scratch, sink_emit, s);
}

void setUp()
{
}
void tearDown()
{
}

// RFC 9204 Appendix B.1: 0000 51 0b /index.html -> :path=/index.html (static name index 1).
void test_appendix_b1_decode()
{
    const uint8_t block[] = {0x00, 0x00, 0x51, 0x0b, '/', 'i', 'n', 'd', 'e', 'x', '.', 'h', 't', 'm', 'l'};
    Sink s;
    TEST_ASSERT_TRUE(decode_all(block, sizeof block, &s));
    TEST_ASSERT_EQUAL_UINT(1, (unsigned)s.hdrs.size());
    TEST_ASSERT_EQUAL_STRING(":path", s.hdrs[0].first.c_str());
    TEST_ASSERT_EQUAL_STRING("/index.html", s.hdrs[0].second.c_str());
}

// Full static match -> Indexed Field Line. :status=200 is static index 25 -> 0xC0|25 = 0xD9.
void test_encode_indexed()
{
    uint8_t out[8];
    size_t n = qpack_encode_header(out, sizeof out, ":status", 7, "200", 3);
    TEST_ASSERT_EQUAL_INT(1, (int)n);
    TEST_ASSERT_EQUAL_HEX8(0xD9, out[0]);
    Sink s;
    const uint8_t block[3] = {0x00, 0x00, out[0]};
    TEST_ASSERT_TRUE(decode_all(block, 3, &s));
    TEST_ASSERT_EQUAL_STRING(":status", s.hdrs[0].first.c_str());
    TEST_ASSERT_EQUAL_STRING("200", s.hdrs[0].second.c_str());
}

// Name-only static match -> Literal Field Line with Name Reference (static index 1 = :path -> 0x51).
void test_encode_nameref_roundtrip()
{
    uint8_t out[32];
    size_t n = qpack_encode_header(out, sizeof out, ":path", 5, "/index.html", 11);
    TEST_ASSERT_TRUE(n > 1);
    TEST_ASSERT_EQUAL_HEX8(0x51, out[0]); // 01 N=0 T=1 index=1

    uint8_t block[34] = {0x00, 0x00};
    memcpy(block + 2, out, n);
    Sink s;
    TEST_ASSERT_TRUE(decode_all(block, n + 2, &s));
    TEST_ASSERT_EQUAL_STRING(":path", s.hdrs[0].first.c_str());
    TEST_ASSERT_EQUAL_STRING("/index.html", s.hdrs[0].second.c_str());
}

// No static match -> Literal Field Line with Literal Name; plus a hand-built raw (H=0) name path.
void test_literal_name()
{
    uint8_t out[32];
    size_t n = qpack_encode_header(out, sizeof out, "x-test", 6, "hi", 2);
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_HEX8(0x20, out[0] & 0xE0); // 001 pattern
    uint8_t block[34] = {0x00, 0x00};
    memcpy(block + 2, out, n);
    Sink s;
    TEST_ASSERT_TRUE(decode_all(block, n + 2, &s));
    TEST_ASSERT_EQUAL_STRING("x-test", s.hdrs[0].first.c_str());
    TEST_ASSERT_EQUAL_STRING("hi", s.hdrs[0].second.c_str());

    // Hand-built raw literal name (H=0, name len 6) + raw value "hi" (H=0, len 2).
    const uint8_t raw[] = {0x00, 0x00, 0x26, 'x', '-', 't', 'e', 's', 't', 0x02, 'h', 'i'};
    Sink s2;
    TEST_ASSERT_TRUE(decode_all(raw, sizeof raw, &s2));
    TEST_ASSERT_EQUAL_STRING("x-test", s2.hdrs[0].first.c_str());
    TEST_ASSERT_EQUAL_STRING("hi", s2.hdrs[0].second.c_str());
}

// A whole field section: prefix + several fields of each representation kind.
void test_full_section()
{
    uint8_t out[128];
    size_t o = qpack_encode_prefix(out, sizeof out);
    TEST_ASSERT_EQUAL_INT(2, (int)o);
    TEST_ASSERT_EQUAL_HEX8(0x00, out[0]);
    TEST_ASSERT_EQUAL_HEX8(0x00, out[1]);
    o += qpack_encode_header(out + o, sizeof out - o, ":status", 7, "200", 3);       // indexed
    o += qpack_encode_header(out + o, sizeof out - o, "content-type", 12, "x/y", 3); // name ref
    o += qpack_encode_header(out + o, sizeof out - o, "x-test", 6, "hello", 5);      // literal name
    Sink s;
    TEST_ASSERT_TRUE(decode_all(out, o, &s));
    TEST_ASSERT_EQUAL_UINT(3, (unsigned)s.hdrs.size());
    TEST_ASSERT_EQUAL_STRING(":status", s.hdrs[0].first.c_str());
    TEST_ASSERT_EQUAL_STRING("200", s.hdrs[0].second.c_str());
    TEST_ASSERT_EQUAL_STRING("content-type", s.hdrs[1].first.c_str());
    TEST_ASSERT_EQUAL_STRING("x/y", s.hdrs[1].second.c_str());
    TEST_ASSERT_EQUAL_STRING("x-test", s.hdrs[2].first.c_str());
    TEST_ASSERT_EQUAL_STRING("hello", s.hdrs[2].second.c_str());
}

// Any dynamic-table reference or non-zero Required Insert Count is a decode error for us.
void test_reject_dynamic()
{
    Sink s;
    const uint8_t ric_nonzero[] = {0x01, 0x00}; // Required Insert Count = 1
    TEST_ASSERT_FALSE(decode_all(ric_nonzero, 2, &s));
    const uint8_t indexed_dyn[] = {0x00, 0x00, 0x80}; // Indexed Field Line, T=0 (dynamic)
    TEST_ASSERT_FALSE(decode_all(indexed_dyn, 3, &s));
    const uint8_t nameref_dyn[] = {0x00, 0x00, 0x40, 0x00}; // Literal w/ Name Ref, T=0 (dynamic)
    TEST_ASSERT_FALSE(decode_all(nameref_dyn, 4, &s));
    const uint8_t postbase[] = {0x00, 0x00, 0x10}; // Indexed Post-Base (dynamic)
    TEST_ASSERT_FALSE(decode_all(postbase, 3, &s));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_appendix_b1_decode);
    RUN_TEST(test_encode_indexed);
    RUN_TEST(test_encode_nameref_roundtrip);
    RUN_TEST(test_literal_name);
    RUN_TEST(test_full_section);
    RUN_TEST(test_reject_dynamic);
    return UNITY_END();
}
