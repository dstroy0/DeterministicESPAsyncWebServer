// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the SunSpec Modbus codec (services/sunspec): the map writer, the
// marker check, the model-chain walker, and the typed point readers. Pure host tests.

#include "services/sunspec/sunspec.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// Build a small map (marker + a tiny model + end model), then walk it back.
void test_build_and_walk()
{
    uint8_t buf[64];
    SunSpecWriter w;
    sunspec_writer_init(&w, buf, sizeof(buf));
    sunspec_write_marker(&w);
    // A made-up model id 99 with 4 body registers: u16=0x1234, i16=-2, u32=0x00BEEF01
    sunspec_write_model_header(&w, 99, 4);
    sunspec_write_u16(&w, 0x1234);
    sunspec_write_i16(&w, -2);
    sunspec_write_u32(&w, 0x00BEEF01);
    sunspec_write_end_model(&w);
    size_t n = sunspec_writer_finish(&w);
    // marker(4) + header(4) + body(8) + end(4) = 20
    TEST_ASSERT_EQUAL_size_t(20, n);

    TEST_ASSERT_TRUE(sunspec_check_marker(buf, n));
    const uint8_t marker[] = {0x53, 0x75, 0x6E, 0x53}; // "SunS"
    TEST_ASSERT_EQUAL_HEX8_ARRAY(marker, buf, 4);

    size_t off;
    TEST_ASSERT_TRUE(sunspec_begin(buf, n, &off));
    TEST_ASSERT_EQUAL_size_t(4, off);

    SunSpecModel m;
    TEST_ASSERT_TRUE(sunspec_next_model(buf, n, &off, &m));
    TEST_ASSERT_EQUAL_UINT16(99, m.id);
    TEST_ASSERT_EQUAL_UINT16(4, m.length);
    TEST_ASSERT_EQUAL_size_t(8, m.body_len);
    TEST_ASSERT_EQUAL_HEX16(0x1234, sunspec_u16(m.body, 0));
    TEST_ASSERT_EQUAL_INT16(-2, sunspec_i16(m.body, 1));
    TEST_ASSERT_EQUAL_HEX32(0x00BEEF01, sunspec_u32(m.body, 2));

    // The next walk hits the end model and stops.
    TEST_ASSERT_FALSE(sunspec_next_model(buf, n, &off, &m));
}

// A two-model map walks in order.
void test_two_models()
{
    uint8_t buf[64];
    SunSpecWriter w;
    sunspec_writer_init(&w, buf, sizeof(buf));
    sunspec_write_marker(&w);
    sunspec_write_model_header(&w, 1, 1); // common-ish, 1 body register
    sunspec_write_u16(&w, 0xAAAA);
    sunspec_write_model_header(&w, 103, 2); // inverter-ish, 2 body registers
    sunspec_write_i16(&w, 100);
    sunspec_write_i16(&w, -3); // a scale factor (sunssf)
    sunspec_write_end_model(&w);
    size_t n = sunspec_writer_finish(&w);

    size_t off;
    SunSpecModel m;
    TEST_ASSERT_TRUE(sunspec_begin(buf, n, &off));
    TEST_ASSERT_TRUE(sunspec_next_model(buf, n, &off, &m));
    TEST_ASSERT_EQUAL_UINT16(1, m.id);
    TEST_ASSERT_TRUE(sunspec_next_model(buf, n, &off, &m));
    TEST_ASSERT_EQUAL_UINT16(103, m.id);
    TEST_ASSERT_EQUAL_INT16(100, sunspec_i16(m.body, 0));
    TEST_ASSERT_EQUAL_INT16(-3, sunspec_i16(m.body, 1));
    TEST_ASSERT_FALSE(sunspec_next_model(buf, n, &off, &m));
}

void test_string_point()
{
    uint8_t buf[64];
    SunSpecWriter w;
    sunspec_writer_init(&w, buf, sizeof(buf));
    sunspec_write_marker(&w);
    sunspec_write_model_header(&w, 1, 8);     // 8 registers = 16 chars of body
    sunspec_write_string(&w, "Acme Corp", 8); // "Acme Corp" + NUL padding to 16 bytes
    sunspec_write_end_model(&w);
    size_t n = sunspec_writer_finish(&w);

    size_t off;
    SunSpecModel m;
    sunspec_begin(buf, n, &off);
    TEST_ASSERT_TRUE(sunspec_next_model(buf, n, &off, &m));
    char mfg[32];
    TEST_ASSERT_TRUE(sunspec_string(m.body, 0, 8, mfg, sizeof(mfg)));
    TEST_ASSERT_EQUAL_STRING("Acme Corp", mfg);
}

void test_marker_and_truncation()
{
    const uint8_t no_marker[] = {0x00, 0x01, 0x02, 0x03};
    TEST_ASSERT_FALSE(sunspec_check_marker(no_marker, sizeof(no_marker)));
    size_t off;
    TEST_ASSERT_FALSE(sunspec_begin(no_marker, sizeof(no_marker), &off));

    // Marker + a header that claims more body than is present -> truncation.
    const uint8_t trunc[] = {0x53, 0x75, 0x6E, 0x53, 0x00, 0x63, 0x00, 0x04, 0x12, 0x34};
    SunSpecModel m;
    TEST_ASSERT_TRUE(sunspec_begin(trunc, sizeof(trunc), &off));
    TEST_ASSERT_FALSE(sunspec_next_model(trunc, sizeof(trunc), &off, &m));
}

void test_writer_overflow_fails_closed()
{
    uint8_t small[6];
    SunSpecWriter w;
    sunspec_writer_init(&w, small, sizeof(small));
    sunspec_write_marker(&w);             // 4 bytes ok
    sunspec_write_model_header(&w, 1, 1); // would need 4 more -> overflow
    TEST_ASSERT_EQUAL_size_t(0, sunspec_writer_finish(&w));
}

// Reader guards (next_model null args + no-room-for-header), the i32 point reader, and
// the string reader's argument guards.
void test_reader_guards_and_i32()
{
    uint8_t buf[16] = {0};
    size_t off = 0;
    SunSpecModel m;
    TEST_ASSERT_FALSE(sunspec_next_model(nullptr, 16, &off, &m));  // null regs
    TEST_ASSERT_FALSE(sunspec_next_model(buf, 16, nullptr, &m));   // null offset
    TEST_ASSERT_FALSE(sunspec_next_model(buf, 16, &off, nullptr)); // null out
    off = 14;
    TEST_ASSERT_FALSE(sunspec_next_model(buf, 16, &off, &m)); // no room for the [id][length] header

    const uint8_t body[4] = {0xFF, 0xFF, 0xFF, 0xFE}; // big-endian -2
    TEST_ASSERT_EQUAL_INT32(-2, sunspec_i32(body, 0));

    char out[8];
    TEST_ASSERT_FALSE(sunspec_string(nullptr, 0, 1, out, sizeof(out)));  // null body
    TEST_ASSERT_FALSE(sunspec_string(body, 0, 1, nullptr, sizeof(out))); // null out
    TEST_ASSERT_FALSE(sunspec_string(body, 0, 1, out, 0));               // zero out_cap
}

// The i32 writer, ss_put's error-flag short-circuit, and every sunspec_write_string
// reject (null string, already-errored writer, and a field that overflows the buffer).
void test_writer_error_and_string_paths()
{
    uint8_t buf[16];
    SunSpecWriter w;
    sunspec_writer_init(&w, buf, sizeof(buf));
    TEST_ASSERT_TRUE(sunspec_write_i32(&w, -123456));
    TEST_ASSERT_EQUAL_size_t(4, sunspec_writer_finish(&w));

    // Once a write overflows, the next ss_put bails on the sticky error flag.
    uint8_t two[2];
    SunSpecWriter e;
    sunspec_writer_init(&e, two, sizeof(two));
    TEST_ASSERT_FALSE(sunspec_write_u32(&e, 0)); // needs 4 > cap 2 -> sets error
    TEST_ASSERT_FALSE(sunspec_write_u16(&e, 0)); // ss_put sees the error flag

    sunspec_writer_init(&w, buf, sizeof(buf));
    TEST_ASSERT_FALSE(sunspec_write_string(&w, nullptr, 1)); // null string

    SunSpecWriter serr;
    sunspec_writer_init(&serr, two, sizeof(two));
    TEST_ASSERT_FALSE(sunspec_write_u32(&serr, 0));         // set the error flag
    TEST_ASSERT_FALSE(sunspec_write_string(&serr, "x", 1)); // write_string sees it

    uint8_t four[4];
    SunSpecWriter sof;
    sunspec_writer_init(&sof, four, sizeof(four));
    TEST_ASSERT_FALSE(sunspec_write_string(&sof, "abcd", 4)); // field 8 > cap 4
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_build_and_walk);
    RUN_TEST(test_two_models);
    RUN_TEST(test_string_point);
    RUN_TEST(test_marker_and_truncation);
    RUN_TEST(test_writer_overflow_fails_closed);
    RUN_TEST(test_reader_guards_and_i32);
    RUN_TEST(test_writer_error_and_string_paths);
    return UNITY_END();
}
