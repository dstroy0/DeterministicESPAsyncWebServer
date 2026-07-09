// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the QUIC frame codec (network_drivers/presentation/http3/quic_frame, RFC 9000
// sec 19): builder/parser round-trips for PADDING/PING/HANDSHAKE_DONE, ACK (single-range built +
// a hand-built multi-range-with-ECN cursor advance), CRYPTO, STREAM (with/without Offset, and the
// LEN-absent "runs to packet end" case), MAX_DATA, and CONNECTION_CLOSE (transport + app), plus a
// multi-frame sequential parse and truncation rejection. Pure host codec.

#include "network_drivers/presentation/http3/quic_frame.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_simple_frames()
{
    uint8_t b[4];
    QuicFrame f;
    TEST_ASSERT_EQUAL_INT(1, (int)quic_build_ping(b, sizeof b));
    TEST_ASSERT_EQUAL_INT(1, (int)quic_frame_parse(b, 1, &f));
    TEST_ASSERT_EQUAL_UINT(QUIC_FT_PING, (unsigned)f.type);

    TEST_ASSERT_EQUAL_INT(1, (int)quic_build_handshake_done(b, sizeof b));
    TEST_ASSERT_EQUAL_INT(1, (int)quic_frame_parse(b, 1, &f));
    TEST_ASSERT_EQUAL_UINT(QUIC_FT_HANDSHAKE_DONE, (unsigned)f.type);

    // A PADDING byte (0x00) parses as one PADDING frame consuming one byte.
    const uint8_t pad[1] = {0x00};
    TEST_ASSERT_EQUAL_INT(1, (int)quic_frame_parse(pad, 1, &f));
    TEST_ASSERT_EQUAL_UINT(QUIC_FT_PADDING, (unsigned)f.type);
}

void test_ack()
{
    uint8_t b[16];
    size_t n = quic_build_ack(b, sizeof b, 1000, 42, 3);
    TEST_ASSERT_TRUE(n > 0);
    QuicFrame f;
    TEST_ASSERT_EQUAL_INT((int)n, (int)quic_frame_parse(b, n, &f));
    TEST_ASSERT_EQUAL_UINT(QUIC_FT_ACK, (unsigned)f.type);
    TEST_ASSERT_TRUE(f.ack.largest == 1000 && f.ack.delay == 42);
    TEST_ASSERT_TRUE(f.ack.range_count == 0 && f.ack.first_range == 3);

    // Hand-built ACK w/ ECN (0x03): largest 60, delay 5, 1 range (gap 2, len 4), ECN 1/2/0.
    const uint8_t ecn[10] = {0x03, 60, 5, 1, 3, 2, 4, 1, 2, 0};
    TEST_ASSERT_EQUAL_INT(10, (int)quic_frame_parse(ecn, sizeof ecn, &f));
    TEST_ASSERT_TRUE(f.ack.largest == 60 && f.ack.range_count == 1 && f.ack.first_range == 3);
}

void test_crypto()
{
    uint8_t b[32];
    const uint8_t data[5] = {'h', 'e', 'l', 'l', 'o'};
    size_t n = quic_build_crypto(b, sizeof b, 7, data, 5);
    TEST_ASSERT_TRUE(n > 0);
    QuicFrame f;
    TEST_ASSERT_EQUAL_INT((int)n, (int)quic_frame_parse(b, n, &f));
    TEST_ASSERT_EQUAL_UINT(QUIC_FT_CRYPTO, (unsigned)f.type);
    TEST_ASSERT_TRUE(f.crypto.offset == 7 && f.crypto.length == 5);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(data, f.crypto.data, 5);
}

void test_stream()
{
    uint8_t b[32];
    const uint8_t data[3] = {1, 2, 3};
    // With offset + FIN.
    size_t n = quic_build_stream(b, sizeof b, 4, 100, data, 3, true);
    QuicFrame f;
    TEST_ASSERT_EQUAL_INT((int)n, (int)quic_frame_parse(b, n, &f));
    TEST_ASSERT_TRUE((f.type & 0xf8) == QUIC_FT_STREAM);
    TEST_ASSERT_TRUE(f.stream.id == 4 && f.stream.offset == 100 && f.stream.length == 3 && f.stream.fin == 1);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(data, f.stream.data, 3);

    // No offset, no FIN.
    n = quic_build_stream(b, sizeof b, 0, 0, data, 3, false);
    TEST_ASSERT_FALSE(b[0] & QUIC_STREAM_OFF);
    TEST_ASSERT_EQUAL_INT((int)n, (int)quic_frame_parse(b, n, &f));
    TEST_ASSERT_TRUE(f.stream.id == 0 && f.stream.offset == 0 && f.stream.fin == 0);

    // LEN bit clear -> Stream Data runs to the end of the packet. Type 0x08 (STREAM), id 0.
    const uint8_t to_end[5] = {0x08, 0x00, 0xAA, 0xBB, 0xCC};
    TEST_ASSERT_EQUAL_INT(5, (int)quic_frame_parse(to_end, sizeof to_end, &f));
    TEST_ASSERT_TRUE(f.stream.id == 0 && f.stream.length == 3);
    TEST_ASSERT_EQUAL_HEX8(0xAA, f.stream.data[0]);
    TEST_ASSERT_EQUAL_HEX8(0xCC, f.stream.data[2]);
}

void test_max_data_and_close()
{
    uint8_t b[64];
    size_t n = quic_build_max_data(b, sizeof b, 65536);
    QuicFrame f;
    TEST_ASSERT_EQUAL_INT((int)n, (int)quic_frame_parse(b, n, &f));
    TEST_ASSERT_TRUE(f.type == QUIC_FT_MAX_DATA && f.max_data.max == 65536);

    n = quic_build_connection_close(b, sizeof b, 0x0a, QUIC_FT_STREAM, "bad", 3);
    TEST_ASSERT_EQUAL_INT((int)n, (int)quic_frame_parse(b, n, &f));
    TEST_ASSERT_TRUE(f.type == QUIC_FT_CONNECTION_CLOSE && f.close.error_code == 0x0a);
    TEST_ASSERT_TRUE(f.close.frame_type == QUIC_FT_STREAM && f.close.reason_len == 3 && f.close.app == 0);
    TEST_ASSERT_EQUAL_UINT8_ARRAY("bad", f.close.reason, 3);

    // Application-level close (0x1d) has no triggering frame type.
    const uint8_t appclose[4] = {0x1d, 0x05, 0x01, 'x'};
    TEST_ASSERT_EQUAL_INT(4, (int)quic_frame_parse(appclose, sizeof appclose, &f));
    TEST_ASSERT_TRUE(f.close.app == 1 && f.close.error_code == 5 && f.close.frame_type == 0 && f.close.reason_len == 1);
}

void test_sequence_and_truncation()
{
    // A packet payload: PADDING, PING, then a CRYPTO frame - parse them in order.
    uint8_t buf[32];
    size_t o = 0;
    buf[o++] = QUIC_FT_PADDING;
    buf[o++] = QUIC_FT_PING;
    const uint8_t data[2] = {0xDE, 0xAD};
    o += quic_build_crypto(buf + o, sizeof buf - o, 0, data, 2);

    size_t pos = 0;
    QuicFrame f;
    size_t c = quic_frame_parse(buf + pos, o - pos, &f);
    TEST_ASSERT_TRUE(c == 1 && f.type == QUIC_FT_PADDING);
    pos += c;
    c = quic_frame_parse(buf + pos, o - pos, &f);
    TEST_ASSERT_TRUE(c == 1 && f.type == QUIC_FT_PING);
    pos += c;
    c = quic_frame_parse(buf + pos, o - pos, &f);
    TEST_ASSERT_TRUE(c > 0 && f.type == QUIC_FT_CRYPTO && f.crypto.length == 2);
    pos += c;
    TEST_ASSERT_EQUAL_UINT((unsigned)o, (unsigned)pos);

    // A CRYPTO frame whose Length exceeds the buffer must be rejected.
    const uint8_t bad[3] = {QUIC_FT_CRYPTO, 0x00, 0x10}; // offset 0, length 16, but no data
    TEST_ASSERT_EQUAL_INT(0, (int)quic_frame_parse(bad, sizeof bad, &f));
}

// Every builder returns 0 when the output buffer is too small (each guard exercised).
void test_builder_overflow()
{
    uint8_t b[1];
    const uint8_t d[4] = {1, 2, 3, 4};
    TEST_ASSERT_EQUAL_INT(0, (int)quic_build_ping(b, 0));
    TEST_ASSERT_EQUAL_INT(0, (int)quic_build_handshake_done(b, 0));
    TEST_ASSERT_EQUAL_INT(0, (int)quic_build_padding(b, 1, 3)); // n > cap
    TEST_ASSERT_EQUAL_INT(0, (int)quic_build_ack(b, 1, 1000, 42, 3));
    TEST_ASSERT_EQUAL_INT(0, (int)quic_build_crypto(b, 1, 7, d, 4)); // header fits, data does not
    TEST_ASSERT_EQUAL_INT(0, (int)quic_build_crypto(b, 0, 0, d, 4)); // type varint does not fit
    TEST_ASSERT_EQUAL_INT(0, (int)quic_build_stream(b, 1, 4, 100, d, 4, true));
    TEST_ASSERT_EQUAL_INT(0, (int)quic_build_stream(b, 0, 0, 0, d, 4, false));
    TEST_ASSERT_EQUAL_INT(0, (int)quic_build_max_data(b, 1, 1u << 30));
    TEST_ASSERT_EQUAL_INT(0, (int)quic_build_connection_close(b, 1, 0x0a, 0, "x", 1));
    TEST_ASSERT_EQUAL_INT(0, (int)quic_build_connection_close(b, 4, 0x0a, 0, "hello", 5)); // reason overflows
}

// Every parse guard: empty input, per-frame truncation, and an unhandled frame type.
void test_parse_errors()
{
    QuicFrame f;
    TEST_ASSERT_EQUAL_INT(0, (int)quic_frame_parse((const uint8_t *)"", 0, &f)); // no type byte

    const uint8_t ack_trunc[1] = {QUIC_FT_ACK}; // no Largest/Delay/...
    TEST_ASSERT_EQUAL_INT(0, (int)quic_frame_parse(ack_trunc, 1, &f));
    const uint8_t ack_ranges_trunc[5] = {0x02, 60, 5, 1, 3}; // range_count 1 but no Gap/Length
    TEST_ASSERT_EQUAL_INT(0, (int)quic_frame_parse(ack_ranges_trunc, 5, &f));
    const uint8_t ack_ecn_trunc[7] = {0x03, 60, 5, 0, 3, 1, 2}; // ECN needs 3 counts, only 2
    TEST_ASSERT_EQUAL_INT(0, (int)quic_frame_parse(ack_ecn_trunc, 7, &f));

    const uint8_t crypto_trunc[2] = {QUIC_FT_CRYPTO, 0x00}; // offset ok, no length
    TEST_ASSERT_EQUAL_INT(0, (int)quic_frame_parse(crypto_trunc, 2, &f));

    const uint8_t stream_noid[1] = {0x08}; // STREAM, no Stream ID
    TEST_ASSERT_EQUAL_INT(0, (int)quic_frame_parse(stream_noid, 1, &f));
    const uint8_t stream_off_trunc[2] = {0x0c, 0x00}; // OFF set, id ok, no Offset
    TEST_ASSERT_EQUAL_INT(0, (int)quic_frame_parse(stream_off_trunc, 2, &f));
    const uint8_t stream_len_over[3] = {0x0a, 0x00, 0x08}; // LEN set, length 8 > remaining
    TEST_ASSERT_EQUAL_INT(0, (int)quic_frame_parse(stream_len_over, 3, &f));

    const uint8_t max_trunc[1] = {QUIC_FT_MAX_DATA};
    TEST_ASSERT_EQUAL_INT(0, (int)quic_frame_parse(max_trunc, 1, &f));

    const uint8_t close_trunc[1] = {QUIC_FT_CONNECTION_CLOSE};
    TEST_ASSERT_EQUAL_INT(0, (int)quic_frame_parse(close_trunc, 1, &f));
    const uint8_t close_reason_over[4] = {0x1c, 0x00, 0x00, 0x08}; // reason len 8 > remaining
    TEST_ASSERT_EQUAL_INT(0, (int)quic_frame_parse(close_reason_over, 4, &f));
    const uint8_t appclose_trunc[2] = {0x1d, 0x00}; // error code ok, no reason length
    TEST_ASSERT_EQUAL_INT(0, (int)quic_frame_parse(appclose_trunc, 2, &f));

    const uint8_t unhandled[1] = {0x18}; // NEW_CONNECTION_ID - not handled by this minimal server
    TEST_ASSERT_EQUAL_INT(0, (int)quic_frame_parse(unhandled, 1, &f));
}

// Remaining per-field truncations (STREAM Length varint, transport CONNECTION_CLOSE
// frame-type varint) and builder mid-write overflow guards (padding success; crypto/stream
// header fits but the body/offset/length does not).
void test_frame_edge_guards()
{
    QuicFrame f;
    // STREAM with LEN set but the Length varint is absent -> rejected at the length read.
    const uint8_t stream_len_trunc[2] = {0x0a, 0x00}; // STREAM|LEN, id 0, no Length
    TEST_ASSERT_EQUAL_INT(0, (int)quic_frame_parse(stream_len_trunc, 2, &f));
    // Transport CONNECTION_CLOSE with an error code but no triggering-frame-type varint.
    const uint8_t close_ft_trunc[2] = {0x1c, 0x00};
    TEST_ASSERT_EQUAL_INT(0, (int)quic_frame_parse(close_ft_trunc, 2, &f));

    uint8_t b[8];
    const uint8_t d[3] = {1, 2, 3};
    // quic_build_padding success: n <= cap zeroes n bytes and returns n.
    TEST_ASSERT_EQUAL_INT(3, (int)quic_build_padding(b, sizeof b, 3));
    TEST_ASSERT_EQUAL_HEX8(0, b[0]);
    TEST_ASSERT_EQUAL_HEX8(0, b[2]);
    // CRYPTO: type+offset+length varints fit but the data does not.
    TEST_ASSERT_EQUAL_INT(0, (int)quic_build_crypto(b, 4, 7, d, 3));
    // STREAM: type+id fit but the Offset varint does not.
    TEST_ASSERT_EQUAL_INT(0, (int)quic_build_stream(b, 2, 4, 100, d, 3, true));
    // STREAM (no offset): type+id fit but the Length varint does not.
    TEST_ASSERT_EQUAL_INT(0, (int)quic_build_stream(b, 2, 0, 0, d, 3, false));
    // STREAM: the header fits but the stream data does not.
    TEST_ASSERT_EQUAL_INT(0, (int)quic_build_stream(b, 3, 0, 0, d, 3, false));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_frame_edge_guards);
    RUN_TEST(test_simple_frames);
    RUN_TEST(test_ack);
    RUN_TEST(test_crypto);
    RUN_TEST(test_stream);
    RUN_TEST(test_max_data_and_close);
    RUN_TEST(test_sequence_and_truncation);
    RUN_TEST(test_builder_overflow);
    RUN_TEST(test_parse_errors);
    return UNITY_END();
}
