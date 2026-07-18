// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the Thread spinel / HDLC-lite framing codec (services/thread): the FCS
// (CRC-16/X-25) against its catalog check value (0x906E), an encode -> decode round trip,
// the byte-stuffing of reserved bytes, and malformed framing (bad FCS, dangling escape,
// buffer-too-small, no flag). Pure host tests.
//
// The env sizes DWS_THREAD_MAX_DATA = 64.

#include "services/thread/thread.h"
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_fcs_x25_check_value()
{
    // CRC-16/X-25 (poly 0x8408, init 0xFFFF, reflected, xorout 0xFFFF) of "123456789" = 0x906E.
    const uint8_t check[9] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    TEST_ASSERT_EQUAL_HEX16(0x906E, dws_spinel_fcs(check, 9));
}

void test_encode_decode_round_trip()
{
    // A tiny spinel frame: header (flag|iid|tid) + command (PROP_VALUE_GET) + property.
    const uint8_t payload[3] = {0x81, 0x02, 0x02};
    uint8_t frame[32];
    uint16_t n = dws_spinel_frame_encode(payload, 3, frame, sizeof(frame));
    TEST_ASSERT_GREATER_THAN_UINT16(0, n);
    TEST_ASSERT_EQUAL_HEX8(ThreadHdlc::HDLC_FLAG, frame[n - 1]);

    uint8_t pay[16];
    uint16_t plen = 0;
    int c = dws_spinel_frame_decode(frame, n, pay, sizeof(pay), &plen);
    TEST_ASSERT_EQUAL_INT((int)n, c);
    TEST_ASSERT_EQUAL_UINT16(3, plen);
    TEST_ASSERT_EQUAL_MEMORY(payload, pay, 3);
}

void test_byte_stuffing_round_trip()
{
    const uint8_t payload[4] = {0x7E, 0x7D, 0x11, 0x13}; // all reserved
    uint8_t frame[32];
    uint16_t n = dws_spinel_frame_encode(payload, 4, frame, sizeof(frame));
    TEST_ASSERT_GREATER_THAN_UINT16(0, n);
    for (uint16_t i = 0; i + 1 < n; i++) // no raw reserved byte in the body
    {
        TEST_ASSERT_NOT_EQUAL_HEX8(0x7E, frame[i]);
        TEST_ASSERT_NOT_EQUAL_HEX8(0x11, frame[i]);
        TEST_ASSERT_NOT_EQUAL_HEX8(0x13, frame[i]);
    }
    uint8_t pay[16];
    uint16_t plen = 0;
    TEST_ASSERT_EQUAL_INT((int)n, dws_spinel_frame_decode(frame, n, pay, sizeof(pay), &plen));
    TEST_ASSERT_EQUAL_UINT16(4, plen);
    TEST_ASSERT_EQUAL_MEMORY(payload, pay, 4);
}

void test_decode_needs_more_without_flag()
{
    const uint8_t partial[3] = {0x81, 0x02, 0x02};
    uint8_t pay[8];
    uint16_t plen = 0;
    TEST_ASSERT_EQUAL_INT(0, dws_spinel_frame_decode(partial, sizeof(partial), pay, sizeof(pay), &plen));
}

void test_decode_rejects_bad_fcs()
{
    const uint8_t payload[3] = {0x81, 0x02, 0x02};
    uint8_t frame[32];
    uint16_t n = dws_spinel_frame_encode(payload, 3, frame, sizeof(frame));
    frame[0] ^= 0xFF; // corrupt the payload so the FCS no longer matches
    uint8_t pay[8];
    uint16_t plen = 0;
    TEST_ASSERT_EQUAL_INT(-1, dws_spinel_frame_decode(frame, n, pay, sizeof(pay), &plen));
}

void test_decode_rejects_dangling_escape()
{
    const uint8_t frame[2] = {ThreadHdlc::HDLC_ESCAPE, ThreadHdlc::HDLC_FLAG}; // escape with nothing before the flag
    uint8_t pay[8];
    uint16_t plen = 0;
    TEST_ASSERT_EQUAL_INT(-1, dws_spinel_frame_decode(frame, sizeof(frame), pay, sizeof(pay), &plen));
}

void test_decode_rejects_small_payload_buffer()
{
    const uint8_t payload[6] = {1, 2, 3, 4, 5, 6};
    uint8_t frame[32];
    uint16_t n = dws_spinel_frame_encode(payload, 6, frame, sizeof(frame));
    uint8_t tiny[3];
    uint16_t plen = 0;
    TEST_ASSERT_EQUAL_INT(-1, dws_spinel_frame_decode(frame, n, tiny, sizeof(tiny), &plen));
}

void test_encode_bounds()
{
    uint8_t data[80] = {0};
    uint8_t out[256];
    TEST_ASSERT_EQUAL_UINT16(0, dws_spinel_frame_encode(data, 65, out, sizeof(out))); // > MAX_DATA 64
    uint8_t small[3];
    TEST_ASSERT_EQUAL_UINT16(0, dws_spinel_frame_encode(data, 8, small, sizeof(small))); // will not fit
}

// --- Spinel command layer ---------------------------------------------------------------

void test_spinel_pack_uint_kats()
{
    uint8_t out[8];
    TEST_ASSERT_EQUAL_UINT8(1, dws_spinel_pack_uint(0, out, sizeof(out)));
    TEST_ASSERT_EQUAL_HEX8(0x00, out[0]);
    TEST_ASSERT_EQUAL_UINT8(1, dws_spinel_pack_uint(127, out, sizeof(out)));
    TEST_ASSERT_EQUAL_HEX8(0x7F, out[0]);
    TEST_ASSERT_EQUAL_UINT8(2, dws_spinel_pack_uint(128, out, sizeof(out))); // 0x80 0x01
    TEST_ASSERT_EQUAL_HEX8(0x80, out[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, out[1]);
    TEST_ASSERT_EQUAL_UINT8(2, dws_spinel_pack_uint(1337, out, sizeof(out))); // 0xB9 0x0A
    TEST_ASSERT_EQUAL_HEX8(0xB9, out[0]);
    TEST_ASSERT_EQUAL_HEX8(0x0A, out[1]);
}

void test_spinel_pack_unpack_round_trip()
{
    const uint32_t values[5] = {0, 42, 127, 128, 200000};
    for (int i = 0; i < 5; i++)
    {
        uint8_t out[8];
        uint8_t n = dws_spinel_pack_uint(values[i], out, sizeof(out));
        TEST_ASSERT_GREATER_THAN_UINT8(0, n);
        uint32_t v = 0;
        int c = dws_spinel_unpack_uint(out, n, &v);
        TEST_ASSERT_EQUAL_INT((int)n, c);
        TEST_ASSERT_EQUAL_UINT32(values[i], v);
    }
}

void test_spinel_unpack_needs_more_and_overflow()
{
    const uint8_t truncated[2] = {0x80, 0x80}; // continuation with no terminator
    uint32_t v = 0;
    TEST_ASSERT_EQUAL_INT(0, dws_spinel_unpack_uint(truncated, 2, &v)); // need more
    const uint8_t overflow[6] = {0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
    TEST_ASSERT_EQUAL_INT(-1, dws_spinel_unpack_uint(overflow, 6, &v)); // > uint32
}

void test_spinel_command_build_parse_round_trip()
{
    const uint8_t val[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    uint8_t buf[32];
    // header 0x81, CMD_PROP_VALUE_SET, a large property id (multi-byte packed), a value.
    uint16_t n = dws_spinel_command_build(0x81, SpinelCmd::SPINEL_CMD_PROP_VALUE_SET, 1337, val, 4, buf, sizeof(buf));
    TEST_ASSERT_GREATER_THAN_UINT16(0, n);

    uint8_t header = 0;
    uint32_t cmd = 0, prop = 0;
    const uint8_t *v = nullptr;
    uint16_t vlen = 0;
    int off = dws_spinel_command_parse(buf, n, &header, &cmd, &prop, &v, &vlen);
    TEST_ASSERT_GREATER_THAN_INT(0, off);
    TEST_ASSERT_EQUAL_HEX8(0x81, header);
    TEST_ASSERT_EQUAL_UINT32(SpinelCmd::SPINEL_CMD_PROP_VALUE_SET, cmd);
    TEST_ASSERT_EQUAL_UINT32(1337, prop);
    TEST_ASSERT_EQUAL_UINT16(4, vlen);
    TEST_ASSERT_EQUAL_MEMORY(val, v, 4);
}

void test_spinel_command_through_hdlc()
{
    // The command payload rides inside an HDLC frame: build the command, frame it, decode
    // the frame, then parse the command back out - the full Thread codec stack.
    uint8_t payload[16];
    uint16_t plen = dws_spinel_command_build(0x82, SpinelCmd::SPINEL_CMD_PROP_VALUE_GET, 2 /*NCP_VERSION*/, nullptr, 0,
                                             payload, sizeof(payload));
    uint8_t frame[32];
    uint16_t fn = dws_spinel_frame_encode(payload, plen, frame, sizeof(frame));
    TEST_ASSERT_GREATER_THAN_UINT16(0, fn);

    uint8_t got[16];
    uint16_t glen = 0;
    TEST_ASSERT_EQUAL_INT((int)fn, dws_spinel_frame_decode(frame, fn, got, sizeof(got), &glen));
    uint8_t header = 0;
    uint32_t cmd = 0, prop = 0;
    const uint8_t *v = nullptr;
    uint16_t vlen = 0;
    TEST_ASSERT_GREATER_THAN_INT(0, dws_spinel_command_parse(got, glen, &header, &cmd, &prop, &v, &vlen));
    TEST_ASSERT_EQUAL_UINT32(SpinelCmd::SPINEL_CMD_PROP_VALUE_GET, cmd);
    TEST_ASSERT_EQUAL_UINT32(2, prop);
    TEST_ASSERT_EQUAL_UINT16(0, vlen);
}

void test_spinel_guards()
{
    uint8_t out[8];
    TEST_ASSERT_EQUAL_UINT8(0, dws_spinel_pack_uint(0x12345, out, 0)); // zero cap
    uint8_t trunc[1] = {0x81};                                         // continuation bit set, truncated
    uint32_t v = 0;
    TEST_ASSERT_TRUE(dws_spinel_unpack_uint(trunc, sizeof(trunc), &v) <= 0); // truncated -> non-positive
    uint8_t pay[4] = {1, 2, 3, 4};
    TEST_ASSERT_EQUAL_UINT16(0, dws_spinel_frame_encode(pay, sizeof(pay), out, 2)); // overflow
    uint8_t fpay[8];
    uint16_t fl = 0;
    uint8_t short_frame[2] = {0x7E, 0x7E};
    TEST_ASSERT_EQUAL_INT(-1, dws_spinel_frame_decode(short_frame, sizeof(short_frame), fpay, sizeof(fpay), &fl));
}

void test_thread_more_guards()
{
    uint8_t out[64];

    // pack/unpack null-pointer guards.
    TEST_ASSERT_EQUAL_UINT8(0, dws_spinel_pack_uint(5, nullptr, 8));
    uint32_t v = 0;
    TEST_ASSERT_EQUAL_INT(0, dws_spinel_unpack_uint(nullptr, 4, &v));

    // command_build guards: null out, cap < 1, value==null with a positive length.
    uint8_t val[2] = {0xAA, 0xBB};
    TEST_ASSERT_EQUAL_UINT16(0, dws_spinel_command_build(0x81, 1, 1, val, 2, nullptr, 8));
    TEST_ASSERT_EQUAL_UINT16(0, dws_spinel_command_build(0x81, 1, 1, val, 2, out, 0));
    TEST_ASSERT_EQUAL_UINT16(0, dws_spinel_command_build(0x81, 1, 1, nullptr, 2, out, 8));
    // command_build overflow at each stage: cmd pack, prop pack, value copy.
    TEST_ASSERT_EQUAL_UINT16(0, dws_spinel_command_build(0x81, 1, 1, nullptr, 0, out, 1));    // cmd pack (cap 0)
    TEST_ASSERT_EQUAL_UINT16(0, dws_spinel_command_build(0x81, 1337, 1, nullptr, 0, out, 3)); // prop pack
    TEST_ASSERT_EQUAL_UINT16(0, dws_spinel_command_build(0x81, 1, 1, val, 10, out, 5));       // value copy

    // command_parse guards: null payload, then a truncated cmd, then a truncated prop.
    uint8_t hdr = 0;
    uint32_t cmd = 0, prop = 0;
    const uint8_t *pv = nullptr;
    uint16_t pvl = 0;
    TEST_ASSERT_EQUAL_INT(-1, dws_spinel_command_parse(nullptr, 4, &hdr, &cmd, &prop, &pv, &pvl));
    const uint8_t tc[2] = {0x81, 0x80}; // header + cmd byte with continuation, no terminator
    TEST_ASSERT_EQUAL_INT(-1, dws_spinel_command_parse(tc, 2, &hdr, &cmd, &prop, &pv, &pvl));
    const uint8_t tp[3] = {0x81, 0x01, 0x80}; // header + valid cmd + truncated prop
    TEST_ASSERT_EQUAL_INT(-1, dws_spinel_command_parse(tp, 3, &hdr, &cmd, &prop, &pv, &pvl));

    // frame_encode: an escaped byte that overflows, an FCS byte that overflows, a flag that overflows.
    uint8_t resv[1] = {0x7E};
    TEST_ASSERT_EQUAL_UINT16(0, dws_spinel_frame_encode(resv, 1, out, 1)); // escape needs 2, cap 1
    uint8_t one[1] = {0x01};
    TEST_ASSERT_EQUAL_UINT16(0, dws_spinel_frame_encode(one, 1, out, 1)); // payload fits, FCS byte overflows
    uint8_t p4[4] = {0x01, 0x02, 0x03, 0x04};
    uint8_t full[32];
    uint16_t fulln = dws_spinel_frame_encode(p4, 4, full, sizeof(full));
    TEST_ASSERT_TRUE(fulln > 1);
    TEST_ASSERT_EQUAL_UINT16(0, dws_spinel_frame_encode(p4, 4, out, (uint16_t)(fulln - 1))); // only the flag can't fit

    // frame_decode: null raw, an over-long unstuffed run, and a real FCS mismatch.
    uint8_t pay[16];
    uint16_t pl = 0;
    TEST_ASSERT_EQUAL_INT(0, dws_spinel_frame_decode(nullptr, 4, pay, sizeof(pay), &pl));
    uint8_t big[80];
    for (int i = 0; i < 70; i++)
        big[i] = 0x00;
    big[70] = ThreadHdlc::HDLC_FLAG;
    TEST_ASSERT_EQUAL_INT(-1, dws_spinel_frame_decode(big, 71, pay, sizeof(pay), &pl)); // unstuffed > scratch
    uint8_t p3[3] = {0x01, 0x02, 0x03};
    uint8_t fr[32];
    uint16_t frn = dws_spinel_frame_encode(p3, 3, fr, sizeof(fr));
    fr[0] ^= 0x01; // 0x01 -> 0x00 (not flag/escape): the payload changes so the stored FCS no longer matches
    uint8_t got[16];
    uint16_t gl = 0;
    TEST_ASSERT_EQUAL_INT(-1, dws_spinel_frame_decode(fr, frn, got, sizeof(got), &gl));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_fcs_x25_check_value);
    RUN_TEST(test_encode_decode_round_trip);
    RUN_TEST(test_byte_stuffing_round_trip);
    RUN_TEST(test_decode_needs_more_without_flag);
    RUN_TEST(test_decode_rejects_bad_fcs);
    RUN_TEST(test_decode_rejects_dangling_escape);
    RUN_TEST(test_decode_rejects_small_payload_buffer);
    RUN_TEST(test_encode_bounds);
    RUN_TEST(test_spinel_pack_uint_kats);
    RUN_TEST(test_spinel_pack_unpack_round_trip);
    RUN_TEST(test_spinel_unpack_needs_more_and_overflow);
    RUN_TEST(test_spinel_command_build_parse_round_trip);
    RUN_TEST(test_spinel_command_through_hdlc);
    RUN_TEST(test_spinel_guards);
    RUN_TEST(test_thread_more_guards);
    return UNITY_END();
}
