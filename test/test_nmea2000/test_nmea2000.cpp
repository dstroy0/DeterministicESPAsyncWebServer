// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the NMEA 2000 codec (services/nmea2000): single-frame messages (J1939-based)
// and the Fast Packet transport (frame count, build, reassembly round-trip, out-of-order +
// interleaved-sequence handling). Pure host tests.

#include "services/nmea2000/nmea2000.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_num_frames()
{
    TEST_ASSERT_EQUAL_UINT8(1, dws_n2k_fastpacket_num_frames(6));    // fits frame 0
    TEST_ASSERT_EQUAL_UINT8(2, dws_n2k_fastpacket_num_frames(7));    // 6 + 1
    TEST_ASSERT_EQUAL_UINT8(2, dws_n2k_fastpacket_num_frames(13));   // 6 + 7
    TEST_ASSERT_EQUAL_UINT8(3, dws_n2k_fastpacket_num_frames(14));   // 6 + 7 + 1
    TEST_ASSERT_EQUAL_UINT8(32, dws_n2k_fastpacket_num_frames(223)); // 6 + 31*7
}

void test_single_frame()
{
    const uint8_t payload[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    CanFrame f;
    TEST_ASSERT_TRUE(dws_n2k_build_single(&f, 2, 0x01F200, 0x15, 0xFF, payload, 8)); // a PDU2 PGN
    TEST_ASSERT_TRUE(f.extended);
    TEST_ASSERT_EQUAL_UINT8(8, f.dlc);
    TEST_ASSERT_EQUAL_MEMORY(payload, f.data, 8);
    J1939Id d;
    TEST_ASSERT_TRUE(dws_j1939_decode_id(f.id, &d));
    TEST_ASSERT_EQUAL_HEX32(0x01F200, d.pgn);
}

// Build all Fast Packet frames of a 20-octet message, feed them back, reassemble.
void test_fastpacket_roundtrip()
{
    uint8_t msg[20];
    for (int i = 0; i < 20; i++)
        msg[i] = (uint8_t)(0x40 + i);
    const uint32_t pgn = 0x01F801; // e.g. position rapid update, a Fast Packet PGN
    const uint8_t sa = 0x15;
    const uint8_t seq = 3;

    uint8_t frames = dws_n2k_fastpacket_num_frames(20); // 6 + 7 + 7 = 3
    TEST_ASSERT_EQUAL_UINT8(3, frames);

    N2kFastPacketRx rx;
    dws_n2k_fastpacket_reset(&rx);
    for (uint8_t i = 0; i < frames; i++)
    {
        CanFrame f;
        TEST_ASSERT_TRUE(dws_n2k_fastpacket_build_frame(&f, seq, i, 6, pgn, sa, 0xFF, msg, 20));
        N2kFpResult r = dws_n2k_fastpacket_feed(&rx, &f);
        if (i == 0)
            TEST_ASSERT_EQUAL_INT(N2kFpResult::N2K_FP_STARTED, r);
        else if (i + 1 < frames)
            TEST_ASSERT_EQUAL_INT(N2kFpResult::N2K_FP_PROGRESS, r);
        else
            TEST_ASSERT_EQUAL_INT(N2kFpResult::N2K_FP_COMPLETE, r);
    }
    TEST_ASSERT_EQUAL_UINT16(20, rx.total_len);
    TEST_ASSERT_EQUAL_HEX32(pgn, rx.pgn);
    TEST_ASSERT_EQUAL_MEMORY(msg, rx.buf, 20);
}

// A message that fits entirely in frame 0 completes immediately.
void test_fastpacket_single_frame_completes()
{
    uint8_t msg[5] = {9, 8, 7, 6, 5};
    CanFrame f;
    TEST_ASSERT_TRUE(dws_n2k_fastpacket_build_frame(&f, 0, 0, 6, 0x01F200, 0x15, 0xFF, msg, 5));
    N2kFastPacketRx rx;
    dws_n2k_fastpacket_reset(&rx);
    TEST_ASSERT_EQUAL_INT(N2kFpResult::N2K_FP_COMPLETE, dws_n2k_fastpacket_feed(&rx, &f));
    TEST_ASSERT_EQUAL_UINT16(5, rx.total_len);
    TEST_ASSERT_EQUAL_MEMORY(msg, rx.buf, 5);
}

// A continuation frame from a different sequence is ignored, not merged.
void test_fastpacket_interleaved_sequence_ignored()
{
    uint8_t msg[20];
    for (int i = 0; i < 20; i++)
        msg[i] = (uint8_t)i;
    N2kFastPacketRx rx;
    dws_n2k_fastpacket_reset(&rx);
    CanFrame f0;
    dws_n2k_fastpacket_build_frame(&f0, 3, 0, 6, 0x01F801, 0x15, 0xFF, msg, 20);
    TEST_ASSERT_EQUAL_INT(N2kFpResult::N2K_FP_STARTED, dws_n2k_fastpacket_feed(&rx, &f0));

    // A frame from sequence 4 (different message) must not be accepted into seq 3.
    CanFrame other;
    dws_n2k_fastpacket_build_frame(&other, 4, 1, 6, 0x01F801, 0x15, 0xFF, msg, 20);
    TEST_ASSERT_EQUAL_INT(N2kFpResult::N2K_FP_IGNORED, dws_n2k_fastpacket_feed(&rx, &other));
    TEST_ASSERT_TRUE(rx.active); // seq 3 still in progress
}

void test_fastpacket_out_of_order_errors()
{
    uint8_t msg[20] = {0};
    N2kFastPacketRx rx;
    dws_n2k_fastpacket_reset(&rx);
    CanFrame f0;
    dws_n2k_fastpacket_build_frame(&f0, 3, 0, 6, 0x01F801, 0x15, 0xFF, msg, 20);
    TEST_ASSERT_EQUAL_INT(N2kFpResult::N2K_FP_STARTED, dws_n2k_fastpacket_feed(&rx, &f0));
    CanFrame f2;
    dws_n2k_fastpacket_build_frame(&f2, 3, 2, 6, 0x01F801, 0x15, 0xFF, msg, 20); // skip frame 1
    TEST_ASSERT_EQUAL_INT(N2kFpResult::N2K_FP_ERR, dws_n2k_fastpacket_feed(&rx, &f2));
    TEST_ASSERT_FALSE(rx.active);
}

// Fast Packet builder guards, an out-of-range frame index, an encode failure (priority>7),
// and the feed reject branches (null/non-extended/short frame + a bad first-frame length).
void test_nmea2000_error_paths()
{
    CanFrame f;
    const uint8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};

    TEST_ASSERT_FALSE(dws_n2k_fastpacket_build_frame(nullptr, 0, 0, 6, 0x01F801, 0x15, 0xFF, data, 8)); // null out
    TEST_ASSERT_FALSE(dws_n2k_fastpacket_build_frame(&f, 0, 0, 6, 0x01F801, 0x15, 0xFF, nullptr, 8));   // null data
    TEST_ASSERT_FALSE(dws_n2k_fastpacket_build_frame(&f, 8, 0, 6, 0x01F801, 0x15, 0xFF, data, 8));      // seq > 7
    TEST_ASSERT_FALSE(dws_n2k_fastpacket_build_frame(&f, 0, 0, 6, 0x01F801, 0x15, 0xFF, data, 0));      // total_len 0
    TEST_ASSERT_FALSE(
        dws_n2k_fastpacket_build_frame(&f, 0, 5, 6, 0x01F801, 0x15, 0xFF, data, 8)); // frame_idx past the count
    TEST_ASSERT_FALSE(
        dws_n2k_fastpacket_build_frame(&f, 0, 0, 8, 0x01F801, 0x15, 0xFF, data, 8)); // priority>7 -> encode fails

    N2kFastPacketRx rx;
    dws_n2k_fastpacket_reset(&rx);
    dws_n2k_fastpacket_build_frame(&f, 0, 0, 6, 0x01F801, 0x15, 0xFF, data, 8);
    TEST_ASSERT_EQUAL_INT(N2kFpResult::N2K_FP_IGNORED, dws_n2k_fastpacket_feed(nullptr, &f));  // null rx
    TEST_ASSERT_EQUAL_INT(N2kFpResult::N2K_FP_IGNORED, dws_n2k_fastpacket_feed(&rx, nullptr)); // null frame
    CanFrame notext = f;
    notext.extended = false;
    TEST_ASSERT_EQUAL_INT(N2kFpResult::N2K_FP_IGNORED, dws_n2k_fastpacket_feed(&rx, &notext)); // not a 29-bit frame
    CanFrame shortdlc = f;
    shortdlc.dlc = 1;
    TEST_ASSERT_EQUAL_INT(N2kFpResult::N2K_FP_IGNORED, dws_n2k_fastpacket_feed(&rx, &shortdlc)); // dlc < 2

    CanFrame bad_total = f;
    bad_total.data[1] = 0; // first frame declaring a zero total length
    TEST_ASSERT_EQUAL_INT(N2kFpResult::N2K_FP_ERR, dws_n2k_fastpacket_feed(&rx, &bad_total));
}

// Fast Packet builder rejects a total_len beyond DWS_N2K_FP_MAX (the last guard clause of
// dws_n2k_fastpacket_build_frame, distinct from the total_len == 0 case already covered above).
void test_fastpacket_build_frame_total_too_large()
{
    CanFrame f;
    const uint8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    TEST_ASSERT_FALSE(dws_n2k_fastpacket_build_frame(&f, 0, 0, 6, 0x01F801, 0x15, 0xFF, data, DWS_N2K_FP_MAX + 1));
}

// Resetting a null context must be a safe no-op, not a crash.
void test_fastpacket_reset_null_is_safe()
{
    dws_n2k_fastpacket_reset(nullptr);
    TEST_ASSERT_TRUE(true);
}

// A first frame declaring a total length beyond DWS_N2K_FP_MAX is rejected by the reassembler
// (distinct from the total == 0 case already covered by test_nmea2000_error_paths).
void test_fastpacket_feed_total_too_large_errors()
{
    uint8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    CanFrame f;
    TEST_ASSERT_TRUE(dws_n2k_fastpacket_build_frame(&f, 0, 0, 6, 0x01F801, 0x15, 0xFF, data, 8));
    f.data[1] = 250; // > DWS_N2K_FP_MAX (223)
    N2kFastPacketRx rx;
    dws_n2k_fastpacket_reset(&rx);
    TEST_ASSERT_EQUAL_INT(N2kFpResult::N2K_FP_ERR, dws_n2k_fastpacket_feed(&rx, &f));
    TEST_ASSERT_FALSE(rx.active);
}

// A continuation frame arriving with no sequence in progress is ignored (rx->active is false).
void test_fastpacket_continuation_without_active_sequence_ignored()
{
    uint8_t msg[20] = {0};
    N2kFastPacketRx rx;
    dws_n2k_fastpacket_reset(&rx);
    CanFrame f1;
    TEST_ASSERT_TRUE(dws_n2k_fastpacket_build_frame(&f1, 3, 1, 6, 0x01F801, 0x15, 0xFF, msg, 20));
    TEST_ASSERT_EQUAL_INT(N2kFpResult::N2K_FP_IGNORED, dws_n2k_fastpacket_feed(&rx, &f1));
    TEST_ASSERT_FALSE(rx.active);
}

// A continuation frame with the matching sequence counter but a different source address must
// not be merged into the in-progress message.
void test_fastpacket_continuation_wrong_source_ignored()
{
    uint8_t msg[20];
    for (int i = 0; i < 20; i++)
        msg[i] = (uint8_t)i;
    N2kFastPacketRx rx;
    dws_n2k_fastpacket_reset(&rx);
    CanFrame f0;
    dws_n2k_fastpacket_build_frame(&f0, 3, 0, 6, 0x01F801, 0x15, 0xFF, msg, 20);
    TEST_ASSERT_EQUAL_INT(N2kFpResult::N2K_FP_STARTED, dws_n2k_fastpacket_feed(&rx, &f0));

    CanFrame other_sa;
    dws_n2k_fastpacket_build_frame(&other_sa, 3, 1, 6, 0x01F801, 0x22, 0xFF, msg, 20); // different sa
    TEST_ASSERT_EQUAL_INT(N2kFpResult::N2K_FP_IGNORED, dws_n2k_fastpacket_feed(&rx, &other_sa));
    TEST_ASSERT_TRUE(rx.active); // sequence 3 still in progress
}

// A continuation frame with the matching sequence counter and source but a different PGN must
// not be merged into the in-progress message.
void test_fastpacket_continuation_wrong_pgn_ignored()
{
    uint8_t msg[20];
    for (int i = 0; i < 20; i++)
        msg[i] = (uint8_t)i;
    N2kFastPacketRx rx;
    dws_n2k_fastpacket_reset(&rx);
    CanFrame f0;
    dws_n2k_fastpacket_build_frame(&f0, 3, 0, 6, 0x01F801, 0x15, 0xFF, msg, 20);
    TEST_ASSERT_EQUAL_INT(N2kFpResult::N2K_FP_STARTED, dws_n2k_fastpacket_feed(&rx, &f0));

    CanFrame other_pgn;
    dws_n2k_fastpacket_build_frame(&other_pgn, 3, 1, 6, 0x01FD00, 0x15, 0xFF, msg, 20); // different pgn
    TEST_ASSERT_EQUAL_INT(N2kFpResult::N2K_FP_IGNORED, dws_n2k_fastpacket_feed(&rx, &other_pgn));
    TEST_ASSERT_TRUE(rx.active); // sequence 3 still in progress
}

// A 19-octet message's final (3rd) continuation frame carries only 6 remaining octets, fewer
// than a full N2K_FP_FN_DATA continuation payload; exercises the "short last frame" branch in
// both the builder and the reassembler.
void test_fastpacket_roundtrip_short_last_frame()
{
    uint8_t msg[19];
    for (int i = 0; i < 19; i++)
        msg[i] = (uint8_t)(0x60 + i);
    const uint32_t pgn = 0x01F801;
    const uint8_t sa = 0x15;
    const uint8_t seq = 5;

    uint8_t frames = dws_n2k_fastpacket_num_frames(19);
    TEST_ASSERT_EQUAL_UINT8(3, frames);

    N2kFastPacketRx rx;
    dws_n2k_fastpacket_reset(&rx);
    for (uint8_t i = 0; i < frames; i++)
    {
        CanFrame f;
        TEST_ASSERT_TRUE(dws_n2k_fastpacket_build_frame(&f, seq, i, 6, pgn, sa, 0xFF, msg, 19));
        N2kFpResult r = dws_n2k_fastpacket_feed(&rx, &f);
        if (i == 0)
            TEST_ASSERT_EQUAL_INT(N2kFpResult::N2K_FP_STARTED, r);
        else if (i + 1 < frames)
            TEST_ASSERT_EQUAL_INT(N2kFpResult::N2K_FP_PROGRESS, r);
        else
            TEST_ASSERT_EQUAL_INT(N2kFpResult::N2K_FP_COMPLETE, r);
    }
    TEST_ASSERT_EQUAL_UINT16(19, rx.total_len);
    TEST_ASSERT_EQUAL_HEX32(pgn, rx.pgn);
    TEST_ASSERT_EQUAL_MEMORY(msg, rx.buf, 19);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_num_frames);
    RUN_TEST(test_single_frame);
    RUN_TEST(test_fastpacket_roundtrip);
    RUN_TEST(test_fastpacket_single_frame_completes);
    RUN_TEST(test_fastpacket_interleaved_sequence_ignored);
    RUN_TEST(test_fastpacket_out_of_order_errors);
    RUN_TEST(test_nmea2000_error_paths);
    RUN_TEST(test_fastpacket_build_frame_total_too_large);
    RUN_TEST(test_fastpacket_reset_null_is_safe);
    RUN_TEST(test_fastpacket_feed_total_too_large_errors);
    RUN_TEST(test_fastpacket_continuation_without_active_sequence_ignored);
    RUN_TEST(test_fastpacket_continuation_wrong_source_ignored);
    RUN_TEST(test_fastpacket_continuation_wrong_pgn_ignored);
    RUN_TEST(test_fastpacket_roundtrip_short_last_frame);
    return UNITY_END();
}
