// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the MessagePack encoder and decoder
// (network_drivers/presentation/msgpack). Expected byte sequences follow the
// MessagePack spec encodings; the decoder tests parse spec-form bytes and
// round-trip the encoder output.

#include "network_drivers/presentation/msgpack/msgpack.h"
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

static void check(const uint8_t *got, size_t got_len, const uint8_t *exp, size_t exp_len)
{
    TEST_ASSERT_EQUAL_size_t(exp_len, got_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(exp, got, exp_len);
}

void test_uint()
{
    uint8_t b[16];
    MsgpackWriter w;
    msgpack_init(&w, b, sizeof(b));
    msgpack_uint(&w, 0);
    uint8_t e0[] = {0x00};
    check(b, msgpack_len(&w), e0, 1);
    msgpack_init(&w, b, sizeof(b));
    msgpack_uint(&w, 127);
    uint8_t e7f[] = {0x7f};
    check(b, msgpack_len(&w), e7f, 1);
    msgpack_init(&w, b, sizeof(b));
    msgpack_uint(&w, 128);
    uint8_t e80[] = {0xcc, 0x80};
    check(b, msgpack_len(&w), e80, 2);
    msgpack_init(&w, b, sizeof(b));
    msgpack_uint(&w, 256);
    uint8_t e256[] = {0xcd, 0x01, 0x00};
    check(b, msgpack_len(&w), e256, 3);
    msgpack_init(&w, b, sizeof(b));
    msgpack_uint(&w, 65536);
    uint8_t e64k[] = {0xce, 0x00, 0x01, 0x00, 0x00};
    check(b, msgpack_len(&w), e64k, 5);
}

void test_int()
{
    uint8_t b[16];
    MsgpackWriter w;
    msgpack_init(&w, b, sizeof(b));
    msgpack_int(&w, -1);
    uint8_t e[] = {0xff};
    check(b, msgpack_len(&w), e, 1);
    msgpack_init(&w, b, sizeof(b));
    msgpack_int(&w, -32);
    uint8_t e32[] = {0xe0};
    check(b, msgpack_len(&w), e32, 1);
    msgpack_init(&w, b, sizeof(b));
    msgpack_int(&w, -33);
    uint8_t e33[] = {0xd0, 0xdf};
    check(b, msgpack_len(&w), e33, 2);
    msgpack_init(&w, b, sizeof(b));
    msgpack_int(&w, -129);
    uint8_t e129[] = {0xd1, 0xff, 0x7f};
    check(b, msgpack_len(&w), e129, 3);
    msgpack_init(&w, b, sizeof(b));
    msgpack_int(&w, 42); // positive via msgpack_int -> fixint
    uint8_t e42[] = {0x2a};
    check(b, msgpack_len(&w), e42, 1);
}

void test_str()
{
    uint8_t b[40];
    MsgpackWriter w;
    msgpack_init(&w, b, sizeof(b));
    msgpack_str(&w, "");
    uint8_t e[] = {0xa0};
    check(b, msgpack_len(&w), e, 1);
    msgpack_init(&w, b, sizeof(b));
    msgpack_str(&w, "a");
    uint8_t e2[] = {0xa1, 0x61};
    check(b, msgpack_len(&w), e2, 2);
    msgpack_init(&w, b, sizeof(b));
    msgpack_str(&w, "hello");
    uint8_t e3[] = {0xa5, 0x68, 0x65, 0x6c, 0x6c, 0x6f};
    check(b, msgpack_len(&w), e3, 6);
}

void test_bytes()
{
    uint8_t b[16];
    MsgpackWriter w;
    msgpack_init(&w, b, sizeof(b));
    uint8_t data[] = {1, 2, 3};
    msgpack_bytes(&w, data, 3);
    uint8_t e[] = {0xc4, 0x03, 0x01, 0x02, 0x03};
    check(b, msgpack_len(&w), e, 5);
}

void test_simple()
{
    uint8_t b[8];
    MsgpackWriter w;
    msgpack_init(&w, b, sizeof(b));
    msgpack_bool(&w, false);
    uint8_t e[] = {0xc2};
    check(b, msgpack_len(&w), e, 1);
    msgpack_init(&w, b, sizeof(b));
    msgpack_bool(&w, true);
    uint8_t e2[] = {0xc3};
    check(b, msgpack_len(&w), e2, 1);
    msgpack_init(&w, b, sizeof(b));
    msgpack_nil(&w);
    uint8_t e3[] = {0xc0};
    check(b, msgpack_len(&w), e3, 1);
}

void test_float()
{
    uint8_t b[8];
    MsgpackWriter w;
    msgpack_init(&w, b, sizeof(b));
    msgpack_float(&w, 1.0f);
    uint8_t e[] = {0xca, 0x3f, 0x80, 0x00, 0x00};
    check(b, msgpack_len(&w), e, 5);
}

void test_array_and_map()
{
    uint8_t b[16];
    MsgpackWriter w;
    msgpack_init(&w, b, sizeof(b));
    msgpack_array(&w, 3);
    msgpack_uint(&w, 1);
    msgpack_uint(&w, 2);
    msgpack_uint(&w, 3);
    uint8_t e[] = {0x93, 0x01, 0x02, 0x03};
    check(b, msgpack_len(&w), e, 4);
    msgpack_init(&w, b, sizeof(b));
    msgpack_map(&w, 1);
    msgpack_uint(&w, 1);
    msgpack_uint(&w, 2);
    uint8_t e2[] = {0x81, 0x01, 0x02};
    check(b, msgpack_len(&w), e2, 3);
}

void test_overflow_fails_closed()
{
    uint8_t b[2];
    MsgpackWriter w;
    msgpack_init(&w, b, sizeof(b));
    msgpack_uint(&w, 65536); // needs 5 bytes
    TEST_ASSERT_FALSE(msgpack_ok(&w));
    TEST_ASSERT_EQUAL_size_t(5, msgpack_len(&w));
}

// ---------------------------------------------------------------------------
// Decoder
// ---------------------------------------------------------------------------

void test_decode_uint()
{
    // positive fixint, uint8, uint16, uint32, uint64
    uint8_t in[] = {0x00, 0x7f, 0xcc, 0x80, 0xcd, 0x01, 0x00, 0xce, 0x00, 0x01, 0x00,
                    0x00, 0xcf, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
    MsgpackReader r;
    msgpack_reader_init(&r, in, sizeof(in));
    uint64_t v;
    TEST_ASSERT_EQUAL(MSGPACK_TYPE_UINT, msgpack_peek(&r));
    TEST_ASSERT_TRUE(msgpack_read_uint(&r, &v));
    TEST_ASSERT_EQUAL_UINT64(0, v);
    TEST_ASSERT_TRUE(msgpack_read_uint(&r, &v));
    TEST_ASSERT_EQUAL_UINT64(127, v);
    TEST_ASSERT_TRUE(msgpack_read_uint(&r, &v));
    TEST_ASSERT_EQUAL_UINT64(128, v);
    TEST_ASSERT_TRUE(msgpack_read_uint(&r, &v));
    TEST_ASSERT_EQUAL_UINT64(256, v);
    TEST_ASSERT_TRUE(msgpack_read_uint(&r, &v));
    TEST_ASSERT_EQUAL_UINT64(65536, v);
    TEST_ASSERT_TRUE(msgpack_read_uint(&r, &v));
    TEST_ASSERT_EQUAL_UINT64(0x100000000ULL, v);
    TEST_ASSERT_TRUE(msgpack_reader_ok(&r));
    TEST_ASSERT_EQUAL(MSGPACK_TYPE_INVALID, msgpack_peek(&r)); // exhausted
}

void test_decode_int()
{
    // negative fixint (-1, -32), int8 (-128), int16 (-32768), int32 (-2147483648)
    uint8_t in[] = {0xff, 0xe0, 0xd0, 0x80, 0xd1, 0x80, 0x00, 0xd2, 0x80, 0x00, 0x00, 0x00};
    MsgpackReader r;
    msgpack_reader_init(&r, in, sizeof(in));
    int64_t v;
    TEST_ASSERT_EQUAL(MSGPACK_TYPE_INT, msgpack_peek(&r));
    TEST_ASSERT_TRUE(msgpack_read_int(&r, &v));
    TEST_ASSERT_EQUAL_INT64(-1, v);
    TEST_ASSERT_TRUE(msgpack_read_int(&r, &v));
    TEST_ASSERT_EQUAL_INT64(-32, v);
    TEST_ASSERT_TRUE(msgpack_read_int(&r, &v));
    TEST_ASSERT_EQUAL_INT64(-128, v);
    TEST_ASSERT_TRUE(msgpack_read_int(&r, &v));
    TEST_ASSERT_EQUAL_INT64(-32768, v);
    TEST_ASSERT_TRUE(msgpack_read_int(&r, &v));
    TEST_ASSERT_EQUAL_INT64(-2147483648LL, v);
    TEST_ASSERT_TRUE(msgpack_reader_ok(&r));
    // read_int also accepts an unsigned encoding
    uint8_t u[] = {0xcc, 0x80};
    msgpack_reader_init(&r, u, sizeof(u));
    TEST_ASSERT_TRUE(msgpack_read_int(&r, &v));
    TEST_ASSERT_EQUAL_INT64(128, v);
}

void test_decode_str_and_bytes()
{
    uint8_t in[] = {0xa3, 'a', 'b', 'c', 0xc4, 0x02, 0xde, 0xad}; // fixstr "abc", bin8 {de ad}
    MsgpackReader r;
    msgpack_reader_init(&r, in, sizeof(in));
    const char *s;
    size_t n;
    TEST_ASSERT_EQUAL(MSGPACK_TYPE_STR, msgpack_peek(&r));
    TEST_ASSERT_TRUE(msgpack_read_str(&r, &s, &n));
    TEST_ASSERT_EQUAL_size_t(3, n);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("abc", s, 3);
    const uint8_t *bin;
    TEST_ASSERT_EQUAL(MSGPACK_TYPE_BIN, msgpack_peek(&r));
    TEST_ASSERT_TRUE(msgpack_read_bytes(&r, &bin, &n));
    TEST_ASSERT_EQUAL_size_t(2, n);
    TEST_ASSERT_EQUAL_UINT8(0xde, bin[0]);
    TEST_ASSERT_EQUAL_UINT8(0xad, bin[1]);
}

void test_decode_simple_and_float()
{
    uint8_t in[] = {0xc0, 0xc2, 0xc3, 0xca, 0x3f, 0x80, 0x00, 0x00}; // nil false true float32(1.0)
    MsgpackReader r;
    msgpack_reader_init(&r, in, sizeof(in));
    bool bv;
    float fv;
    TEST_ASSERT_EQUAL(MSGPACK_TYPE_NIL, msgpack_peek(&r));
    TEST_ASSERT_TRUE(msgpack_read_nil(&r));
    TEST_ASSERT_EQUAL(MSGPACK_TYPE_BOOL, msgpack_peek(&r));
    TEST_ASSERT_TRUE(msgpack_read_bool(&r, &bv));
    TEST_ASSERT_FALSE(bv);
    TEST_ASSERT_TRUE(msgpack_read_bool(&r, &bv));
    TEST_ASSERT_TRUE(bv);
    TEST_ASSERT_EQUAL(MSGPACK_TYPE_FLOAT, msgpack_peek(&r));
    TEST_ASSERT_TRUE(msgpack_read_float(&r, &fv));
    TEST_ASSERT_EQUAL_FLOAT(1.0f, fv);
    // float64 (0xcb) narrows to float
    uint8_t d[] = {0xcb, 0x40, 0x09, 0x21, 0xfb, 0x54, 0x44, 0x2d, 0x18}; // pi as double
    msgpack_reader_init(&r, d, sizeof(d));
    TEST_ASSERT_TRUE(msgpack_read_float(&r, &fv));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 3.14159f, fv);
}

void test_decode_array_and_map()
{
    uint8_t in[] = {0x93, 0x01, 0x02, 0x03, 0x81, 0xa1, 'k', 0x2a}; // [1,2,3] {"k":42}
    MsgpackReader r;
    msgpack_reader_init(&r, in, sizeof(in));
    size_t count;
    TEST_ASSERT_EQUAL(MSGPACK_TYPE_ARRAY, msgpack_peek(&r));
    TEST_ASSERT_TRUE(msgpack_read_array(&r, &count));
    TEST_ASSERT_EQUAL_size_t(3, count);
    uint64_t v;
    for (size_t i = 0; i < count; i++)
    {
        TEST_ASSERT_TRUE(msgpack_read_uint(&r, &v));
        TEST_ASSERT_EQUAL_UINT64(i + 1, v);
    }
    TEST_ASSERT_EQUAL(MSGPACK_TYPE_MAP, msgpack_peek(&r));
    TEST_ASSERT_TRUE(msgpack_read_map(&r, &count));
    TEST_ASSERT_EQUAL_size_t(1, count);
    const char *s;
    size_t n;
    TEST_ASSERT_TRUE(msgpack_read_str(&r, &s, &n));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("k", s, 1);
    TEST_ASSERT_TRUE(msgpack_read_uint(&r, &v));
    TEST_ASSERT_EQUAL_UINT64(42, v);
}

void test_decode_roundtrip()
{
    // Encode a small document, then decode it back and check each field.
    uint8_t b[64];
    MsgpackWriter w;
    msgpack_init(&w, b, sizeof(b));
    msgpack_map(&w, 3);
    msgpack_str(&w, "id");
    msgpack_uint(&w, 4242);
    msgpack_str(&w, "t");
    msgpack_float(&w, 21.5f);
    msgpack_str(&w, "ok");
    msgpack_bool(&w, true);
    TEST_ASSERT_TRUE(msgpack_ok(&w));

    MsgpackReader r;
    msgpack_reader_init(&r, b, msgpack_len(&w));
    size_t count;
    TEST_ASSERT_TRUE(msgpack_read_map(&r, &count));
    TEST_ASSERT_EQUAL_size_t(3, count);
    const char *k;
    size_t kn;
    uint64_t uv;
    float fv;
    bool bv;
    TEST_ASSERT_TRUE(msgpack_read_str(&r, &k, &kn));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("id", k, 2);
    TEST_ASSERT_TRUE(msgpack_read_uint(&r, &uv));
    TEST_ASSERT_EQUAL_UINT64(4242, uv);
    TEST_ASSERT_TRUE(msgpack_read_str(&r, &k, &kn));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("t", k, 1);
    TEST_ASSERT_TRUE(msgpack_read_float(&r, &fv));
    TEST_ASSERT_EQUAL_FLOAT(21.5f, fv);
    TEST_ASSERT_TRUE(msgpack_read_str(&r, &k, &kn));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("ok", k, 2);
    TEST_ASSERT_TRUE(msgpack_read_bool(&r, &bv));
    TEST_ASSERT_TRUE(bv);
    TEST_ASSERT_TRUE(msgpack_reader_ok(&r));
}

void test_decode_fails_closed()
{
    MsgpackReader r;
    // truncated uint16 (header says read 2 more bytes, only 1 present)
    uint8_t t1[] = {0xcd, 0x01};
    msgpack_reader_init(&r, t1, sizeof(t1));
    uint64_t v;
    TEST_ASSERT_FALSE(msgpack_read_uint(&r, &v));
    TEST_ASSERT_FALSE(msgpack_reader_ok(&r));
    // truncated str (len 5, only 2 bytes present)
    uint8_t t2[] = {0xa5, 'h', 'i'};
    msgpack_reader_init(&r, t2, sizeof(t2));
    const char *s;
    size_t n;
    TEST_ASSERT_FALSE(msgpack_read_str(&r, &s, &n));
    TEST_ASSERT_FALSE(msgpack_reader_ok(&r));
    // type mismatch: read_uint on a bool
    uint8_t t3[] = {0xc3};
    msgpack_reader_init(&r, t3, sizeof(t3));
    TEST_ASSERT_FALSE(msgpack_read_uint(&r, &v));
    TEST_ASSERT_FALSE(msgpack_reader_ok(&r));
    // unsupported byte (0xc1) peeks INVALID and any read fails
    uint8_t t4[] = {0xc1};
    msgpack_reader_init(&r, t4, sizeof(t4));
    TEST_ASSERT_EQUAL(MSGPACK_TYPE_INVALID, msgpack_peek(&r));
    TEST_ASSERT_FALSE(msgpack_read_int(&r, (int64_t *)&v));
    // empty buffer
    msgpack_reader_init(&r, t4, 0);
    TEST_ASSERT_EQUAL(MSGPACK_TYPE_INVALID, msgpack_peek(&r));
    TEST_ASSERT_FALSE(msgpack_read_nil(&r));
}

// Round-trip every wide encoding so both the encoder width branches (u64, i32,
// i64, str8/str16, bin16, array16, map16, str_n) and the matching decoder paths run.
void test_wide_roundtrip()
{
    static uint8_t b[2048];
    MsgpackWriter w;
    MsgpackReader r;
    uint64_t uv;
    int64_t iv;
    size_t n, cnt;
    const char *sp;
    const uint8_t *bp;

    msgpack_init(&w, b, sizeof(b));
    msgpack_uint(&w, 0x123456789ULL); // uint64 (0xcf)
    TEST_ASSERT_EQUAL_UINT8(0xcf, b[0]);
    msgpack_reader_init(&r, b, msgpack_len(&w));
    TEST_ASSERT_TRUE(msgpack_read_uint(&r, &uv));
    TEST_ASSERT_EQUAL_UINT64(0x123456789ULL, uv);

    msgpack_init(&w, b, sizeof(b));
    msgpack_int(&w, -70000); // int32 (0xd2)
    TEST_ASSERT_EQUAL_UINT8(0xd2, b[0]);
    msgpack_reader_init(&r, b, msgpack_len(&w));
    TEST_ASSERT_TRUE(msgpack_read_int(&r, &iv));
    TEST_ASSERT_EQUAL_INT64(-70000, iv);

    msgpack_init(&w, b, sizeof(b));
    msgpack_int(&w, -5000000000LL); // int64 (0xd3)
    TEST_ASSERT_EQUAL_UINT8(0xd3, b[0]);
    msgpack_reader_init(&r, b, msgpack_len(&w));
    TEST_ASSERT_TRUE(msgpack_read_int(&r, &iv));
    TEST_ASSERT_EQUAL_INT64(-5000000000LL, iv);

    msgpack_init(&w, b, sizeof(b));
    msgpack_int(&w, 5000000000LL); // positive crossing into a wide int
    msgpack_reader_init(&r, b, msgpack_len(&w));
    TEST_ASSERT_TRUE(msgpack_read_int(&r, &iv));
    TEST_ASSERT_EQUAL_INT64(5000000000LL, iv);

    char s40[41];
    for (int i = 0; i < 40; i++)
        s40[i] = 'x';
    s40[40] = '\0';
    msgpack_init(&w, b, sizeof(b));
    msgpack_str(&w, s40); // str8 (0xd9)
    TEST_ASSERT_EQUAL_UINT8(0xd9, b[0]);
    msgpack_reader_init(&r, b, msgpack_len(&w));
    TEST_ASSERT_TRUE(msgpack_read_str(&r, &sp, &n));
    TEST_ASSERT_EQUAL_size_t(40, n);

    char s300[301];
    for (int i = 0; i < 300; i++)
        s300[i] = 'y';
    s300[300] = '\0';
    msgpack_init(&w, b, sizeof(b));
    msgpack_str(&w, s300); // str16 (0xda)
    TEST_ASSERT_EQUAL_UINT8(0xda, b[0]);
    msgpack_reader_init(&r, b, msgpack_len(&w));
    TEST_ASSERT_TRUE(msgpack_read_str(&r, &sp, &n));
    TEST_ASSERT_EQUAL_size_t(300, n);

    msgpack_init(&w, b, sizeof(b));
    msgpack_str_n(&w, "hi", 2); // explicit-length fixstr
    msgpack_reader_init(&r, b, msgpack_len(&w));
    TEST_ASSERT_TRUE(msgpack_read_str(&r, &sp, &n));
    TEST_ASSERT_EQUAL_size_t(2, n);

    static uint8_t big[300];
    for (int i = 0; i < 300; i++)
        big[i] = (uint8_t)i;
    msgpack_init(&w, b, sizeof(b));
    msgpack_bytes(&w, big, 300); // bin16 (0xc5)
    TEST_ASSERT_EQUAL_UINT8(0xc5, b[0]);
    msgpack_reader_init(&r, b, msgpack_len(&w));
    TEST_ASSERT_TRUE(msgpack_read_bytes(&r, &bp, &n));
    TEST_ASSERT_EQUAL_size_t(300, n);

    msgpack_init(&w, b, sizeof(b));
    msgpack_array(&w, 20); // array16 (0xdc)
    for (int i = 0; i < 20; i++)
        msgpack_uint(&w, (uint64_t)i);
    TEST_ASSERT_EQUAL_UINT8(0xdc, b[0]);
    msgpack_reader_init(&r, b, msgpack_len(&w));
    TEST_ASSERT_TRUE(msgpack_read_array(&r, &cnt));
    TEST_ASSERT_EQUAL_size_t(20, cnt);
    for (int i = 0; i < 20; i++)
    {
        TEST_ASSERT_TRUE(msgpack_read_uint(&r, &uv));
        TEST_ASSERT_EQUAL_UINT64((uint64_t)i, uv);
    }

    msgpack_init(&w, b, sizeof(b));
    msgpack_map(&w, 20); // map16 (0xde)
    for (int i = 0; i < 20; i++)
    {
        msgpack_uint(&w, (uint64_t)i);
        msgpack_uint(&w, (uint64_t)i);
    }
    TEST_ASSERT_EQUAL_UINT8(0xde, b[0]);
    msgpack_reader_init(&r, b, msgpack_len(&w));
    TEST_ASSERT_TRUE(msgpack_read_map(&r, &cnt));
    TEST_ASSERT_EQUAL_size_t(20, cnt);
}

// Wide-type decoder error paths: truncated str16/bin16/array16 headers + bodies.
void test_decode_wide_fails_closed()
{
    MsgpackReader r;
    const char *s;
    const uint8_t *bp;
    size_t n;
    // str16 header claims 300 bytes, body absent
    uint8_t t1[] = {0xda, 0x01, 0x2c, 'a'};
    msgpack_reader_init(&r, t1, sizeof(t1));
    TEST_ASSERT_FALSE(msgpack_read_str(&r, &s, &n));
    TEST_ASSERT_FALSE(msgpack_reader_ok(&r));
    // bin16 truncated header (only one length byte)
    uint8_t t2[] = {0xc5, 0x01};
    msgpack_reader_init(&r, t2, sizeof(t2));
    TEST_ASSERT_FALSE(msgpack_read_bytes(&r, &bp, &n));
    // array16 header truncated
    uint8_t t3[] = {0xdc, 0x00};
    msgpack_reader_init(&r, t3, sizeof(t3));
    size_t cnt;
    TEST_ASSERT_FALSE(msgpack_read_array(&r, &cnt));
    // uint64 truncated
    uint8_t t4[] = {0xcf, 0x00, 0x00};
    msgpack_reader_init(&r, t4, sizeof(t4));
    uint64_t uv;
    TEST_ASSERT_FALSE(msgpack_read_uint(&r, &uv));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_uint);
    RUN_TEST(test_wide_roundtrip);
    RUN_TEST(test_decode_wide_fails_closed);
    RUN_TEST(test_int);
    RUN_TEST(test_str);
    RUN_TEST(test_bytes);
    RUN_TEST(test_simple);
    RUN_TEST(test_float);
    RUN_TEST(test_array_and_map);
    RUN_TEST(test_overflow_fails_closed);
    RUN_TEST(test_decode_uint);
    RUN_TEST(test_decode_int);
    RUN_TEST(test_decode_str_and_bytes);
    RUN_TEST(test_decode_simple_and_float);
    RUN_TEST(test_decode_array_and_map);
    RUN_TEST(test_decode_roundtrip);
    RUN_TEST(test_decode_fails_closed);
    return UNITY_END();
}
