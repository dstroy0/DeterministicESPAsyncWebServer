// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the Siemens SIMATIC serial codec (services/simatic): 3964R block framing
// (DLE-doubling + XOR BCC), the 3964R link state machine (STX/DLE handshake, retries, QVZ/ZVZ
// timeouts, priority arbitration), and RK512 SEND/FETCH/reaction telegrams (big-endian words).

#include "services/simatic/simatic.h"
#include <string.h>
#include <unity.h>
#include <vector>

// ---- tx / rx capture for the state machine --------------------------------

static std::vector<uint8_t> g_tx;
static std::vector<uint8_t> g_rx;

static void cap_tx(void *user, uint8_t b)
{
    (void)user;
    g_tx.push_back(b);
}
static void cap_rx(void *user, const uint8_t *d, size_t n)
{
    (void)user;
    g_rx.assign(d, d + n);
}

void setUp()
{
    g_tx.clear();
    g_rx.clear();
}
void tearDown()
{
}

// ---- 3964R framing --------------------------------------------------------

void test_bcc_is_xor()
{
    const uint8_t d[] = {0x11, 0x22, 0x33};
    TEST_ASSERT_EQUAL_HEX8(0x11 ^ 0x22 ^ 0x33, dws_3964r_bcc(d, sizeof(d)));
    // a doubled DLE pair cancels in the XOR (0x10 ^ 0x10 = 0)
    const uint8_t pair[] = {SIMATIC_DLE, SIMATIC_DLE};
    TEST_ASSERT_EQUAL_HEX8(0, dws_3964r_bcc(pair, sizeof(pair)));
}

void test_build_block_stuffs_dle_and_terminates()
{
    const uint8_t data[] = {0x41, SIMATIC_DLE, 0x42}; // 'A', DLE, 'B'
    uint8_t buf[32];
    size_t n = dws_3964r_build_block(buf, sizeof(buf), data, sizeof(data), true);
    // 0x41, DLE, DLE (doubled), 0x42, DLE, ETX, BCC
    TEST_ASSERT_EQUAL_size_t(7, n);
    const uint8_t want[] = {0x41, SIMATIC_DLE, SIMATIC_DLE, 0x42, SIMATIC_DLE, SIMATIC_ETX};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(want, buf, 6);
    TEST_ASSERT_EQUAL_HEX8(dws_3964r_bcc(buf, 6), buf[6]); // BCC over stuffed data + DLE ETX
}

void test_block_round_trip_with_embedded_dle()
{
    const uint8_t data[] = {0x01, SIMATIC_DLE, SIMATIC_ETX, SIMATIC_DLE, 0xFF}; // DLE and ETX in payload
    uint8_t blk[64];
    size_t n = dws_3964r_build_block(blk, sizeof(blk), data, sizeof(data), true);
    TEST_ASSERT_GREATER_THAN_size_t(0, n);
    uint8_t out[64];
    size_t olen = 0;
    TEST_ASSERT_TRUE(dws_3964r_parse_block(blk, n, true, out, sizeof(out), &olen));
    TEST_ASSERT_EQUAL_size_t(sizeof(data), olen);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(data, out, olen);
}

void test_block_round_trip_no_bcc()
{
    const uint8_t data[] = {0xAA, 0xBB, 0xCC};
    uint8_t blk[32];
    size_t n = dws_3964r_build_block(blk, sizeof(blk), data, sizeof(data), false);
    uint8_t out[32];
    size_t olen = 0;
    TEST_ASSERT_TRUE(dws_3964r_parse_block(blk, n, false, out, sizeof(out), &olen));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(data, out, olen);
}

void test_parse_rejects_bad()
{
    uint8_t out[32];
    size_t olen = 0;
    // bad BCC
    uint8_t blk[16];
    const uint8_t data[] = {0x10, 0x20};
    size_t n = dws_3964r_build_block(blk, sizeof(blk), data, sizeof(data), true);
    blk[n - 1] ^= 0xFF; // corrupt BCC
    TEST_ASSERT_FALSE(dws_3964r_parse_block(blk, n, true, out, sizeof(out), &olen));
    // missing DLE ETX terminator
    const uint8_t noterm[] = {0x41, 0x42, 0x43};
    TEST_ASSERT_FALSE(dws_3964r_parse_block(noterm, sizeof(noterm), false, out, sizeof(out), &olen));
    // dangling DLE (truncated)
    const uint8_t dangle[] = {0x41, SIMATIC_DLE};
    TEST_ASSERT_FALSE(dws_3964r_parse_block(dangle, sizeof(dangle), false, out, sizeof(out), &olen));
    // DLE + illegal control byte
    const uint8_t badctl[] = {SIMATIC_DLE, 0x55, SIMATIC_DLE, SIMATIC_ETX};
    TEST_ASSERT_FALSE(dws_3964r_parse_block(badctl, sizeof(badctl), false, out, sizeof(out), &olen));
    // out-buffer overflow (fail-closed)
    const uint8_t big[] = {1, 2, 3, 4, 5};
    uint8_t bigblk[32];
    size_t bn = dws_3964r_build_block(bigblk, sizeof(bigblk), big, sizeof(big), false);
    uint8_t tiny[2];
    TEST_ASSERT_FALSE(dws_3964r_parse_block(bigblk, bn, false, tiny, sizeof(tiny), &olen));
}

// ---- 3964R link state machine ---------------------------------------------

void test_sm_send_happy_path()
{
    Simatic3964Ctx c;
    dws_3964r_init(&c, true, true, cap_tx, cap_rx, nullptr);
    const uint8_t msg[] = {0x31, 0x32};
    TEST_ASSERT_TRUE(dws_3964r_send(&c, msg, sizeof(msg), 0));
    // first tx is STX; awaiting connect
    TEST_ASSERT_EQUAL_size_t(1, g_tx.size());
    TEST_ASSERT_EQUAL_HEX8(SIMATIC_STX, g_tx[0]);
    // partner acks connect with DLE -> we send the block
    dws_3964r_rx_byte(&c, SIMATIC_DLE, 1);
    TEST_ASSERT_GREATER_THAN_size_t(1, g_tx.size());            // block bytes emitted
    TEST_ASSERT_EQUAL_HEX8(SIMATIC_ETX, g_tx[g_tx.size() - 2]); // ... DLE ETX BCC tail
    // partner acks the block with DLE -> done
    dws_3964r_rx_byte(&c, SIMATIC_DLE, 2);
    TEST_ASSERT_TRUE(dws_3964r_idle(&c));
}

void test_sm_receive_path_delivers()
{
    Simatic3964Ctx c;
    dws_3964r_init(&c, true, true, cap_tx, cap_rx, nullptr);
    // build a block the "partner" sends us
    const uint8_t payload[] = {0x53, 0x10, 0x54}; // 'S', DLE, 'T'
    uint8_t blk[32];
    size_t n = dws_3964r_build_block(blk, sizeof(blk), payload, sizeof(payload), true);
    // partner opens with STX -> we reply DLE (ready)
    dws_3964r_rx_byte(&c, SIMATIC_STX, 0);
    TEST_ASSERT_EQUAL_size_t(1, g_tx.size());
    TEST_ASSERT_EQUAL_HEX8(SIMATIC_DLE, g_tx[0]);
    // feed the block body byte by byte
    for (size_t i = 0; i < n; i++)
        dws_3964r_rx_byte(&c, blk[i], (uint32_t)(1 + i));
    // we ack with a final DLE and deliver the un-stuffed payload
    TEST_ASSERT_EQUAL_HEX8(SIMATIC_DLE, g_tx.back());
    TEST_ASSERT_EQUAL_size_t(sizeof(payload), g_rx.size());
    TEST_ASSERT_EQUAL_HEX8_ARRAY(payload, g_rx.data(), g_rx.size());
    TEST_ASSERT_TRUE(dws_3964r_idle(&c));
}

void test_sm_block_nak_retries()
{
    Simatic3964Ctx c;
    dws_3964r_init(&c, true, false, cap_tx, cap_rx, nullptr);
    const uint8_t msg[] = {0x01};
    dws_3964r_send(&c, msg, sizeof(msg), 0);
    dws_3964r_rx_byte(&c, SIMATIC_DLE, 1); // connect
    size_t before = g_tx.size();
    dws_3964r_rx_byte(&c, SIMATIC_NAK, 2); // reject the block -> resend STX
    TEST_ASSERT_EQUAL_HEX8(SIMATIC_STX, g_tx.back());
    TEST_ASSERT_GREATER_THAN_size_t(before, g_tx.size());
    TEST_ASSERT_FALSE(dws_3964r_idle(&c)); // retrying
}

void test_sm_qvz_timeout_then_abort()
{
    Simatic3964Ctx c;
    dws_3964r_init(&c, true, false, cap_tx, cap_rx, nullptr);
    const uint8_t msg[] = {0x01};
    dws_3964r_send(&c, msg, sizeof(msg), 0); // STX at t=0, deadline t=QVZ
    // never ack; tick past the deadline repeatedly -> connection retries then abort
    uint32_t t = DWS_SIMATIC_QVZ_MS;
    for (int i = 0; i < SIMATIC_MAX_CONN_RETRY + 1; i++)
    {
        dws_3964r_tick(&c, t);
        t += DWS_SIMATIC_QVZ_MS;
    }
    TEST_ASSERT_TRUE(dws_3964r_idle(&c)); // gave up after MAX retries
}

void test_sm_priority_arbitration()
{
    // Low-priority station, mid-send, sees a partner STX -> yields to receive.
    Simatic3964Ctx lo;
    dws_3964r_init(&lo, false, true, cap_tx, cap_rx, nullptr);
    const uint8_t msg[] = {0x01};
    dws_3964r_send(&lo, msg, sizeof(msg), 0); // sent our STX, awaiting connect
    g_tx.clear();
    dws_3964r_rx_byte(&lo, SIMATIC_STX, 1);       // collision
    TEST_ASSERT_EQUAL_HEX8(SIMATIC_DLE, g_tx[0]); // yielded: replied DLE (now receiving)

    // High-priority station in the same collision keeps its send (ignores the partner STX).
    Simatic3964Ctx hi;
    dws_3964r_init(&hi, true, true, cap_tx, cap_rx, nullptr);
    dws_3964r_send(&hi, msg, sizeof(msg), 0);
    g_tx.clear();
    dws_3964r_rx_byte(&hi, SIMATIC_STX, 1);
    TEST_ASSERT_EQUAL_size_t(0, g_tx.size()); // ignored; still awaiting its own connect DLE
    TEST_ASSERT_FALSE(dws_3964r_idle(&hi));
}

// Regression (found on the ESP32-P4 HW run): a request/response peer replies to a received block from
// inside the rx callback (e.g. an RK512 FETCH -> a reaction). deliver_or_nak must be IDLE before the
// callback so that reply-send is accepted, else the responder never answers.
static Simatic3964Ctx *g_reply_ctx = nullptr;
static void rx_then_reply(void *user, const uint8_t *d, size_t n)
{
    (void)user;
    (void)d;
    (void)n;
    if (g_reply_ctx)
    {
        const uint8_t reply[] = {0xAA, 0xBB};
        dws_3964r_send(g_reply_ctx, reply, sizeof(reply), 100);
    }
}

void test_sm_reply_from_rx_callback()
{
    Simatic3964Ctx c;
    dws_3964r_init(&c, true, true, cap_tx, rx_then_reply, nullptr);
    g_reply_ctx = &c;
    const uint8_t payload[] = {0x01};
    uint8_t blk[16];
    size_t n = dws_3964r_build_block(blk, sizeof(blk), payload, sizeof(payload), true);
    dws_3964r_rx_byte(&c, SIMATIC_STX, 0);
    for (size_t i = 0; i < n; i++)
        dws_3964r_rx_byte(&c, blk[i], (uint32_t)(1 + i));
    g_reply_ctx = nullptr;
    // the block was acked (DLE) then the reply's STX was emitted -> the link is now sending the reply
    TEST_ASSERT_EQUAL_HEX8(SIMATIC_STX, g_tx.back());
    TEST_ASSERT_FALSE(dws_3964r_idle(&c));
}

// ---- RK512 telegrams ------------------------------------------------------

void test_rk512_build_send_field_order()
{
    const uint16_t words[] = {0x1234, 0xABCD};
    uint8_t buf[32];
    size_t n = dws_rk512_build_send(buf, sizeof(buf), Rk512Area::DB, 5, 0x0010, words, 2);
    TEST_ASSERT_EQUAL_size_t(8 + 4, n);
    // [SEND, coord=0, area=DB, dbnr=5, addr BE, count BE, words BE]
    const uint8_t want[] = {0x00, 0x00, (uint8_t)Rk512Area::DB, 0x05, 0x00, 0x10, 0x00, 0x02, 0x12, 0x34, 0xAB, 0xCD};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(want, buf, n);
}

void test_rk512_build_fetch_and_parse()
{
    uint8_t buf[16];
    size_t n = dws_rk512_build_fetch(buf, sizeof(buf), Rk512Area::MB, 0, 0x0100, 4);
    TEST_ASSERT_EQUAL_size_t(8, n);
    Rk512Header h;
    TEST_ASSERT_TRUE(dws_rk512_parse_header(buf, n, &h));
    TEST_ASSERT_EQUAL(Rk512Cmd::FETCH, h.cmd);
    TEST_ASSERT_EQUAL(Rk512Area::MB, h.area);
    TEST_ASSERT_EQUAL_UINT16(0x0100, h.addr);
    TEST_ASSERT_EQUAL_UINT16(4, h.count);
}

void test_rk512_reaction_round_trip()
{
    uint8_t buf[8];
    size_t n = dws_rk512_build_reaction(buf, sizeof(buf), 0x0000);
    uint16_t status = 0xFFFF;
    const uint8_t *data = (const uint8_t *)1;
    size_t dlen = 99;
    TEST_ASSERT_TRUE(dws_rk512_parse_reaction(buf, n, &status, &data, &dlen));
    TEST_ASSERT_EQUAL_UINT16(0, status);
    TEST_ASSERT_NULL(data);
    TEST_ASSERT_EQUAL_size_t(0, dlen);
    // a non-zero error status
    dws_rk512_build_reaction(buf, sizeof(buf), 0x8001);
    dws_rk512_parse_reaction(buf, 3, &status, nullptr, nullptr);
    TEST_ASSERT_EQUAL_UINT16(0x8001, status);
}

void test_rk512_parse_rejects()
{
    Rk512Header h;
    const uint8_t shortbuf[] = {0x00, 0x00};
    TEST_ASSERT_FALSE(dws_rk512_parse_header(shortbuf, sizeof(shortbuf), &h)); // too short
    const uint8_t badarea[] = {0x00, 0x00, 0xEE, 0x00, 0x00, 0x00, 0x00, 0x01};
    TEST_ASSERT_FALSE(dws_rk512_parse_header(badarea, sizeof(badarea), &h)); // invalid area
    // overflow guards
    uint8_t tiny[4];
    const uint16_t w[] = {1};
    TEST_ASSERT_EQUAL_size_t(0, dws_rk512_build_send(tiny, sizeof(tiny), Rk512Area::DB, 0, 0, w, 1));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_bcc_is_xor);
    RUN_TEST(test_build_block_stuffs_dle_and_terminates);
    RUN_TEST(test_block_round_trip_with_embedded_dle);
    RUN_TEST(test_block_round_trip_no_bcc);
    RUN_TEST(test_parse_rejects_bad);
    RUN_TEST(test_sm_send_happy_path);
    RUN_TEST(test_sm_receive_path_delivers);
    RUN_TEST(test_sm_block_nak_retries);
    RUN_TEST(test_sm_qvz_timeout_then_abort);
    RUN_TEST(test_sm_priority_arbitration);
    RUN_TEST(test_sm_reply_from_rx_callback);
    RUN_TEST(test_rk512_build_send_field_order);
    RUN_TEST(test_rk512_build_fetch_and_parse);
    RUN_TEST(test_rk512_reaction_round_trip);
    RUN_TEST(test_rk512_parse_rejects);
    return UNITY_END();
}
