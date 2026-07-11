// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the SAE J1939 codec (services/j1939): 29-bit id encode/decode (PDU1 + PDU2),
// single-frame messages, Request PGN, Address Claimed + NAME, and the Transport Protocol
// (BAM announce + TP.DT packets) round-tripped through the reassembler. Pure host tests.

#include "services/j1939/j1939.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// PDU2 (broadcast, PF >= 240): PS is part of the PGN, DA is global.
void test_id_pdu2_roundtrip()
{
    uint32_t id = 0;
    TEST_ASSERT_TRUE(j1939_encode_id(&id, 6, 0x00FEEE, 0x21, 0xFF)); // engine temperature-ish PGN
    J1939Id d;
    TEST_ASSERT_TRUE(j1939_decode_id(id, &d));
    TEST_ASSERT_EQUAL_UINT8(6, d.priority);
    TEST_ASSERT_EQUAL_HEX32(0x00FEEE, d.pgn);
    TEST_ASSERT_EQUAL_HEX8(0x21, d.sa);
    TEST_ASSERT_FALSE(d.pdu1);
    TEST_ASSERT_EQUAL_HEX8(0xFF, d.da);
}

// PDU1 (peer-to-peer, PF < 240): PS carries the destination address; the PGN low octet is 0.
void test_id_pdu1_roundtrip()
{
    uint32_t id = 0;
    TEST_ASSERT_TRUE(j1939_encode_id(&id, 3, 0x00EF00, 0x21, 0x05)); // PF 0xEF = 239 -> PDU1
    J1939Id d;
    TEST_ASSERT_TRUE(j1939_decode_id(id, &d));
    TEST_ASSERT_TRUE(d.pdu1);
    TEST_ASSERT_EQUAL_HEX8(0x05, d.da);
    TEST_ASSERT_EQUAL_HEX32(0x00EF00, d.pgn);
    TEST_ASSERT_EQUAL_HEX8(0x21, d.sa);
}

void test_encode_rejects_bad_args()
{
    uint32_t id = 0;
    TEST_ASSERT_FALSE(j1939_encode_id(&id, 8, 0x00FEEE, 0x21, 0xFF)); // priority > 7
    TEST_ASSERT_FALSE(j1939_encode_id(&id, 6, 0x40000, 0x21, 0xFF));  // pgn > 18 bits
}

void test_build_single_frame()
{
    const uint8_t payload[3] = {0x11, 0x22, 0x33};
    CanFrame f;
    TEST_ASSERT_TRUE(j1939_build_message(&f, 6, 0x00FEEE, 0x21, 0xFF, payload, 3));
    TEST_ASSERT_TRUE(f.extended);
    TEST_ASSERT_EQUAL_UINT8(3, f.dlc);
    TEST_ASSERT_EQUAL_MEMORY(payload, f.data, 3);
    J1939Id d;
    TEST_ASSERT_TRUE(j1939_decode_id(f.id, &d));
    TEST_ASSERT_EQUAL_HEX32(0x00FEEE, d.pgn);
}

void test_request_pgn()
{
    CanFrame f;
    TEST_ASSERT_TRUE(j1939_build_request(&f, 0x21, 0x00, 0x00FEEC)); // request the component ID PGN
    J1939Id d;
    TEST_ASSERT_TRUE(j1939_decode_id(f.id, &d));
    TEST_ASSERT_EQUAL_HEX32(J1939_PGN_REQUEST, d.pgn);
    TEST_ASSERT_EQUAL_HEX8(0x00, d.da); // PDU1 to address 0
    TEST_ASSERT_EQUAL_UINT8(3, f.dlc);
    TEST_ASSERT_EQUAL_HEX8(0xEC, f.data[0]); // requested PGN, little-endian
    TEST_ASSERT_EQUAL_HEX8(0xFE, f.data[1]);
    TEST_ASSERT_EQUAL_HEX8(0x00, f.data[2]);
}

void test_address_claim_name()
{
    uint64_t name = j1939_build_name(true, 2, 0, 5, 130, 0, 1, 0x1F2, 0x12345);
    CanFrame f;
    TEST_ASSERT_TRUE(j1939_build_address_claim(&f, 0x80, name));
    J1939Id d;
    TEST_ASSERT_TRUE(j1939_decode_id(f.id, &d));
    TEST_ASSERT_EQUAL_HEX32(J1939_PGN_ADDRESS_CLAIM, d.pgn);
    TEST_ASSERT_EQUAL_HEX8(0x80, d.sa);
    TEST_ASSERT_EQUAL_UINT8(8, f.dlc);
    // NAME transmitted little-endian; reconstruct and compare.
    uint64_t back = 0;
    for (int i = 0; i < 8; i++)
        back |= (uint64_t)f.data[i] << (8 * i);
    TEST_ASSERT_EQUAL_HEX64(name, back);
    // The high bit (arbitrary-address-capable) and identity number survive the round trip.
    TEST_ASSERT_EQUAL_HEX32(0x12345u, (uint32_t)(back & 0x1FFFFFu));
    TEST_ASSERT_TRUE((back >> 63) & 1u);
}

void test_tp_num_packets()
{
    TEST_ASSERT_EQUAL_UINT8(2, j1939_tp_num_packets(9));  // ceil(9/7)
    TEST_ASSERT_EQUAL_UINT8(3, j1939_tp_num_packets(16)); // ceil(16/7)
    TEST_ASSERT_EQUAL_UINT8(1, j1939_tp_num_packets(7));
}

// Build a BAM announce + its TP.DT packets, feed them to the reassembler, get the message back.
void test_tp_bam_roundtrip()
{
    uint8_t msg[16];
    for (int i = 0; i < 16; i++)
        msg[i] = (uint8_t)(0xA0 + i);
    const uint32_t pgn = 0x00FECA; // DM1-style broadcast PGN
    const uint8_t sa = 0x21;

    J1939TpRx rx;
    j1939_tp_reset(&rx);

    CanFrame cm;
    TEST_ASSERT_TRUE(j1939_build_bam_cm(&cm, sa, pgn, 16));
    TEST_ASSERT_EQUAL_INT(J1939TpResult::J1939_TP_STARTED, j1939_tp_feed(&rx, &cm));

    uint8_t packets = j1939_tp_num_packets(16); // 3
    for (uint8_t seq = 1; seq <= packets; seq++)
    {
        uint16_t off = (uint16_t)((seq - 1) * J1939_TP_DT_LEN);
        uint8_t len = (uint8_t)((16 - off) < J1939_TP_DT_LEN ? (16 - off) : J1939_TP_DT_LEN);
        CanFrame dt;
        TEST_ASSERT_TRUE(j1939_build_tp_dt(&dt, sa, J1939_ADDR_GLOBAL, seq, msg + off, len));
        J1939TpResult r = j1939_tp_feed(&rx, &dt);
        if (seq < packets)
            TEST_ASSERT_EQUAL_INT(J1939TpResult::J1939_TP_PROGRESS, r);
        else
            TEST_ASSERT_EQUAL_INT(J1939TpResult::J1939_TP_COMPLETE, r);
    }
    TEST_ASSERT_EQUAL_UINT16(16, rx.total_size);
    TEST_ASSERT_EQUAL_HEX32(pgn, rx.pgn);
    TEST_ASSERT_EQUAL_MEMORY(msg, rx.buf, 16);
}

// An out-of-sequence data packet aborts the session.
void test_tp_out_of_sequence_errors()
{
    J1939TpRx rx;
    j1939_tp_reset(&rx);
    CanFrame cm;
    j1939_build_bam_cm(&cm, 0x21, 0x00FECA, 16);
    TEST_ASSERT_EQUAL_INT(J1939TpResult::J1939_TP_STARTED, j1939_tp_feed(&rx, &cm));

    uint8_t chunk[7] = {1, 2, 3, 4, 5, 6, 7};
    CanFrame dt;
    j1939_build_tp_dt(&dt, 0x21, J1939_ADDR_GLOBAL, 2, chunk, 7); // skips seq 1
    TEST_ASSERT_EQUAL_INT(J1939TpResult::J1939_TP_ERROR, j1939_tp_feed(&rx, &dt));
    TEST_ASSERT_FALSE(rx.active);
}

// Builder argument rejections, including the priority-out-of-range path that makes
// ext_frame's id encode fail.
void test_build_error_paths()
{
    CanFrame f;
    const uint8_t data[3] = {1, 2, 3};
    TEST_ASSERT_FALSE(j1939_decode_id(0x18FEEE21u, nullptr)); // null out

    TEST_ASSERT_FALSE(j1939_build_message(nullptr, 6, 0x00FEEE, 0x21, 0xFF, data, 3)); // null out
    TEST_ASSERT_FALSE(j1939_build_message(&f, 6, 0x00FEEE, 0x21, 0xFF, nullptr, 3));   // len but null data
    TEST_ASSERT_FALSE(j1939_build_message(&f, 8, 0x00FEEE, 0x21, 0xFF, data, 3));      // priority>7 -> encode fails

    TEST_ASSERT_FALSE(j1939_build_request(nullptr, 0x21, 0, 0x00FEEC)); // null out
    TEST_ASSERT_FALSE(j1939_build_request(&f, 0x21, 0, 0x40000));       // pgn > 18 bits

    TEST_ASSERT_FALSE(j1939_build_bam_cm(nullptr, 0x21, 0x00FECA, 16)); // null out
    TEST_ASSERT_FALSE(j1939_build_bam_cm(&f, 0x21, 0x00FECA, 8));       // total_size < 9
    TEST_ASSERT_FALSE(j1939_build_bam_cm(&f, 0x21, 0x40000, 16));       // pgn > 18 bits

    const uint8_t chunk[7] = {1, 2, 3, 4, 5, 6, 7};
    TEST_ASSERT_FALSE(j1939_build_tp_dt(&f, 0x21, 0xFF, 0, chunk, 7));   // seq 0
    TEST_ASSERT_FALSE(j1939_build_tp_dt(&f, 0x21, 0xFF, 1, chunk, 0));   // chunk_len 0
    TEST_ASSERT_FALSE(j1939_build_tp_dt(&f, 0x21, 0xFF, 1, nullptr, 7)); // null chunk
    TEST_ASSERT_FALSE(j1939_build_tp_dt(&f, 0x21, 0xFF, 1, chunk, 9));   // chunk_len > 7
}

// The transport-protocol reassembler's ignore/error branches.
void test_tp_feed_error_paths()
{
    J1939TpRx rx;
    j1939_tp_reset(&rx);
    CanFrame cm;
    j1939_build_bam_cm(&cm, 0x21, 0x00FECA, 16);

    TEST_ASSERT_EQUAL_INT(J1939TpResult::J1939_TP_IGNORED, j1939_tp_feed(nullptr, &cm)); // null rx
    TEST_ASSERT_EQUAL_INT(J1939TpResult::J1939_TP_IGNORED, j1939_tp_feed(&rx, nullptr)); // null frame
    CanFrame notext = cm;
    notext.extended = false;
    TEST_ASSERT_EQUAL_INT(J1939TpResult::J1939_TP_IGNORED, j1939_tp_feed(&rx, &notext)); // not a 29-bit frame

    CanFrame cm_ctrl = cm;
    cm_ctrl.data[0] = 0xFF; // not BAM/RTS -> not a receiver-side session start
    TEST_ASSERT_EQUAL_INT(J1939TpResult::J1939_TP_IGNORED, j1939_tp_feed(&rx, &cm_ctrl));

    CanFrame cm_bad = cm;
    cm_bad.data[3] = 99; // packet count != ceil(total/7)
    TEST_ASSERT_EQUAL_INT(J1939TpResult::J1939_TP_ERROR, j1939_tp_feed(&rx, &cm_bad));

    // A TP.DT with no active session is ignored.
    j1939_tp_reset(&rx);
    const uint8_t chunk[7] = {1, 2, 3, 4, 5, 6, 7};
    CanFrame dt;
    j1939_build_tp_dt(&dt, 0x21, J1939_ADDR_GLOBAL, 1, chunk, 7);
    TEST_ASSERT_EQUAL_INT(J1939TpResult::J1939_TP_IGNORED, j1939_tp_feed(&rx, &dt));

    // An extended frame whose PGN is neither TP.CM nor TP.DT is ignored.
    CanFrame other;
    j1939_build_message(&other, 6, 0x00FEEE, 0x21, 0xFF, chunk, 3);
    TEST_ASSERT_EQUAL_INT(J1939TpResult::J1939_TP_IGNORED, j1939_tp_feed(&rx, &other));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_id_pdu2_roundtrip);
    RUN_TEST(test_id_pdu1_roundtrip);
    RUN_TEST(test_encode_rejects_bad_args);
    RUN_TEST(test_build_single_frame);
    RUN_TEST(test_request_pgn);
    RUN_TEST(test_address_claim_name);
    RUN_TEST(test_tp_num_packets);
    RUN_TEST(test_tp_bam_roundtrip);
    RUN_TEST(test_tp_out_of_sequence_errors);
    RUN_TEST(test_build_error_paths);
    RUN_TEST(test_tp_feed_error_paths);
    return UNITY_END();
}
