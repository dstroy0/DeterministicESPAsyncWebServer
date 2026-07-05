// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the QUIC transport-parameters codec (network_drivers/presentation/http3/quic_tp;
// RFC 9000 sec 18). Covers the spec defaults, an encode/parse round-trip (connection IDs + every
// varint parameter + the migration flag), parsing a hand-built byte string, skipping unknown/GREASE
// parameters, and rejection of malformed or out-of-range input.

#include "network_drivers/presentation/http3/quic_tp.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_defaults()
{
    QuicTransportParams tp;
    quic_tp_defaults(&tp);
    TEST_ASSERT_EQUAL_UINT64(65527, tp.max_udp_payload_size);
    TEST_ASSERT_EQUAL_UINT64(3, tp.ack_delay_exponent);
    TEST_ASSERT_EQUAL_UINT64(25, tp.max_ack_delay);
    TEST_ASSERT_EQUAL_UINT64(2, tp.active_connection_id_limit);
    TEST_ASSERT_EQUAL_UINT64(0, tp.initial_max_data);
    TEST_ASSERT_FALSE(tp.has_original_dcid);
    TEST_ASSERT_FALSE(tp.disable_active_migration);
}

void test_roundtrip()
{
    QuicTransportParams tp;
    quic_tp_defaults(&tp);
    tp.has_original_dcid = true;
    tp.original_dcid_len = 8;
    memcpy(tp.original_dcid, "\x00\x01\x02\x03\x04\x05\x06\x07", 8);
    tp.has_initial_scid = true;
    tp.initial_scid_len = 4;
    memcpy(tp.initial_scid, "\xaa\xbb\xcc\xdd", 4);
    tp.initial_max_data = 1048576;
    tp.initial_max_sd_bidi_local = 262144;
    tp.initial_max_sd_bidi_remote = 262144;
    tp.initial_max_sd_uni = 65536;
    tp.initial_max_streams_bidi = 100;
    tp.initial_max_streams_uni = 3;
    tp.max_idle_timeout = 30000;
    tp.disable_active_migration = true;

    uint8_t buf[256];
    size_t n = quic_tp_encode(&tp, buf, sizeof(buf));
    TEST_ASSERT_TRUE(n > 0);

    QuicTransportParams out;
    TEST_ASSERT_TRUE(quic_tp_parse(buf, n, &out));
    TEST_ASSERT_TRUE(out.has_original_dcid);
    TEST_ASSERT_EQUAL_UINT8(8, out.original_dcid_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(tp.original_dcid, out.original_dcid, 8);
    TEST_ASSERT_TRUE(out.has_initial_scid);
    TEST_ASSERT_EQUAL_UINT8(4, out.initial_scid_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(tp.initial_scid, out.initial_scid, 4);
    TEST_ASSERT_EQUAL_UINT64(1048576, out.initial_max_data);
    TEST_ASSERT_EQUAL_UINT64(262144, out.initial_max_sd_bidi_local);
    TEST_ASSERT_EQUAL_UINT64(262144, out.initial_max_sd_bidi_remote);
    TEST_ASSERT_EQUAL_UINT64(65536, out.initial_max_sd_uni);
    TEST_ASSERT_EQUAL_UINT64(100, out.initial_max_streams_bidi);
    TEST_ASSERT_EQUAL_UINT64(3, out.initial_max_streams_uni);
    TEST_ASSERT_EQUAL_UINT64(30000, out.max_idle_timeout);
    TEST_ASSERT_TRUE(out.disable_active_migration);
}

// Parse a hand-built byte string independent of our own encoder:
//   0f 04 aabbccdd        initial_source_connection_id = aabbccdd
//   04 04 80100000        initial_max_data = 0x100000 (1048576)
//   01 02 6710            max_idle_timeout = 0x2710 (10000)
void test_parse_bytes()
{
    static const uint8_t enc[] = {0x0f, 0x04, 0xaa, 0xbb, 0xcc, 0xdd, 0x04, 0x04,
                                  0x80, 0x10, 0x00, 0x00, 0x01, 0x02, 0x67, 0x10};
    QuicTransportParams tp;
    TEST_ASSERT_TRUE(quic_tp_parse(enc, sizeof(enc), &tp));
    TEST_ASSERT_TRUE(tp.has_initial_scid);
    TEST_ASSERT_EQUAL_UINT8(4, tp.initial_scid_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY("\xaa\xbb\xcc\xdd", tp.initial_scid, 4);
    TEST_ASSERT_EQUAL_UINT64(1048576, tp.initial_max_data);
    TEST_ASSERT_EQUAL_UINT64(10000, tp.max_idle_timeout);
}

// An unknown (GREASE) parameter is skipped; a following known one still parses.
void test_skip_unknown()
{
    // id 0x1a (unknown), len 3, value 01 02 03; then 04 01 20 (initial_max_data = 0x20 = 32).
    static const uint8_t enc[] = {0x1a, 0x03, 0x01, 0x02, 0x03, 0x04, 0x01, 0x20};
    QuicTransportParams tp;
    TEST_ASSERT_TRUE(quic_tp_parse(enc, sizeof(enc), &tp));
    TEST_ASSERT_EQUAL_UINT64(32, tp.initial_max_data);
}

void test_reject_duplicate()
{
    // initial_max_data twice.
    static const uint8_t enc[] = {0x04, 0x01, 0x20, 0x04, 0x01, 0x21};
    QuicTransportParams tp;
    TEST_ASSERT_FALSE(quic_tp_parse(enc, sizeof(enc), &tp));
}

void test_reject_oversized_cid()
{
    // original_destination_connection_id with a 21-byte value (max is 20).
    uint8_t enc[2 + 21];
    enc[0] = QUIC_TP_ORIGINAL_DCID;
    enc[1] = 21;
    memset(enc + 2, 0xff, 21);
    QuicTransportParams tp;
    TEST_ASSERT_FALSE(quic_tp_parse(enc, sizeof(enc), &tp));
}

void test_reject_bad_values()
{
    QuicTransportParams tp;
    // active_connection_id_limit = 1 (must be >= 2).
    static const uint8_t a[] = {0x0e, 0x01, 0x01};
    TEST_ASSERT_FALSE(quic_tp_parse(a, sizeof(a), &tp));
    // max_udp_payload_size = 1000 (< 1200).
    static const uint8_t b[] = {0x03, 0x02, 0x43, 0xe8};
    TEST_ASSERT_FALSE(quic_tp_parse(b, sizeof(b), &tp));
    // ack_delay_exponent = 21 (> 20).
    static const uint8_t c[] = {0x0a, 0x01, 0x15};
    TEST_ASSERT_FALSE(quic_tp_parse(c, sizeof(c), &tp));
    // disable_active_migration with a non-zero length.
    static const uint8_t d[] = {0x0c, 0x01, 0x00};
    TEST_ASSERT_FALSE(quic_tp_parse(d, sizeof(d), &tp));
    // truncated: id present, length says 4 but only 1 byte follows.
    static const uint8_t e[] = {0x04, 0x04, 0x00};
    TEST_ASSERT_FALSE(quic_tp_parse(e, sizeof(e), &tp));
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_defaults);
    RUN_TEST(test_roundtrip);
    RUN_TEST(test_parse_bytes);
    RUN_TEST(test_skip_unknown);
    RUN_TEST(test_reject_duplicate);
    RUN_TEST(test_reject_oversized_cid);
    RUN_TEST(test_reject_bad_values);
    return UNITY_END();
}
