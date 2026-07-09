// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the DMA ingest / egress simulator (services/dma) host core: an ingress
// feed surfaces as RX completion events, a full buffer ping-pong flips and re-arms, egress
// DMA is captured, TX is one-in-flight fail-closed, and a loopback channel round-trips its
// own TX into RX. The ARDUINO ISR-post + preempt-queue hand-off is HW-verified separately
// (the host has no ISRs); the same simulator drives that device self-test.
//
// The env sizes DETWS_DMA_BUF_SIZE = 8, DETWS_DMA_CHANNELS = 2 (staging = 24).

#include "services/dma/dma.h"
#include <string.h>
#include <unity.h>
#include <vector>

struct Ev
{
    uint8_t dir;
    uint8_t channel;
    uint16_t len;
    uint16_t seq;
    const void *ptr; // RX buffer address (to prove the ping-pong flip)
    std::vector<uint8_t> data;
};

static std::vector<Ev> g_ev;

static void on_ev(const det_dma_event *ev, void *ctx)
{
    (void)ctx;
    Ev e;
    e.dir = ev->dir;
    e.channel = ev->channel;
    e.len = ev->len;
    e.seq = ev->seq;
    e.ptr = ev->data;
    if (ev->dir == DET_DMA_RX && ev->data)
        e.data.assign(ev->data, ev->data + ev->len);
    g_ev.push_back(e);
}

static bool open_ch(uint8_t ch, bool loop)
{
    det_dma_config c = {};
    c.channel = ch;
    c.periph = DET_DMA_UART;
    c.loopback = loop;
    c.on_complete = on_ev;
    c.ctx = nullptr;
    return det_dma_open(&c);
}

static std::vector<uint8_t> rx_concat()
{
    std::vector<uint8_t> v;
    for (size_t i = 0; i < g_ev.size(); i++)
        if (g_ev[i].dir == DET_DMA_RX)
            v.insert(v.end(), g_ev[i].data.begin(), g_ev[i].data.end());
    return v;
}

static size_t count_dir(uint8_t dir)
{
    size_t n = 0;
    for (size_t i = 0; i < g_ev.size(); i++)
        if (g_ev[i].dir == dir)
            n++;
    return n;
}

void setUp()
{
    g_ev.clear();
    det_dma_close(0);
    det_dma_close(1);
}
void tearDown()
{
    det_dma_close(0);
    det_dma_close(1);
}

void test_open_validates()
{
    TEST_ASSERT_FALSE(det_dma_open(nullptr));
    det_dma_config c = {};
    c.channel = 0;
    c.on_complete = nullptr; // null callback
    TEST_ASSERT_FALSE(det_dma_open(&c));
    c.on_complete = on_ev;
    c.channel = DETWS_DMA_CHANNELS; // out of range
    TEST_ASSERT_FALSE(det_dma_open(&c));
    TEST_ASSERT_TRUE(open_ch(0, false));
    TEST_ASSERT_FALSE(open_ch(0, false)); // double open is a no-op
}

void test_ingress_emits_rx_event()
{
    TEST_ASSERT_TRUE(open_ch(0, false));
    const uint8_t msg[] = {'h', 'e', 'l', 'l', 'o'};
    TEST_ASSERT_TRUE(det_dma_sim_feed(0, msg, sizeof(msg)));
    TEST_ASSERT_EQUAL_size_t(0, g_ev.size()); // nothing until we pump the engine
    det_dma_poll();
    TEST_ASSERT_EQUAL_size_t(1, count_dir(DET_DMA_RX));
    TEST_ASSERT_EQUAL_UINT16(5, g_ev[0].len);
    TEST_ASSERT_EQUAL_UINT8(0, g_ev[0].channel);
    TEST_ASSERT_EQUAL_MEMORY(msg, g_ev[0].data.data(), sizeof(msg));
}

void test_buffer_fills_then_partial_flush()
{
    TEST_ASSERT_TRUE(open_ch(0, false));
    uint8_t msg[DETWS_DMA_BUF_SIZE + 3];
    for (size_t i = 0; i < sizeof(msg); i++)
        msg[i] = (uint8_t)i;
    TEST_ASSERT_TRUE(det_dma_sim_feed(0, msg, sizeof(msg)));
    det_dma_poll();
    // one full-buffer completion + one partial idle-line flush
    TEST_ASSERT_EQUAL_size_t(2, count_dir(DET_DMA_RX));
    TEST_ASSERT_EQUAL_UINT16(DETWS_DMA_BUF_SIZE, g_ev[0].len);
    TEST_ASSERT_EQUAL_UINT16(3, g_ev[1].len);
    std::vector<uint8_t> got = rx_concat();
    TEST_ASSERT_EQUAL_size_t(sizeof(msg), got.size());
    TEST_ASSERT_EQUAL_MEMORY(msg, got.data(), sizeof(msg));
}

void test_ping_pong_flips_buffer()
{
    TEST_ASSERT_TRUE(open_ch(0, false));
    uint8_t msg[DETWS_DMA_BUF_SIZE * 2]; // exactly two full buffers
    for (size_t i = 0; i < sizeof(msg); i++)
        msg[i] = (uint8_t)(0x40 + i);
    TEST_ASSERT_TRUE(det_dma_sim_feed(0, msg, sizeof(msg)));
    det_dma_poll();
    TEST_ASSERT_EQUAL_size_t(2, count_dir(DET_DMA_RX));
    // consecutive completions use different buffers (the engine flipped, not reused)
    TEST_ASSERT_NOT_EQUAL(g_ev[0].ptr, g_ev[1].ptr);
    TEST_ASSERT_EQUAL_UINT16(0, g_ev[0].seq);
    TEST_ASSERT_EQUAL_UINT16(1, g_ev[1].seq); // per-channel sequence increments
    std::vector<uint8_t> got = rx_concat();
    TEST_ASSERT_EQUAL_MEMORY(msg, got.data(), sizeof(msg));
}

void test_egress_captures_tx()
{
    TEST_ASSERT_TRUE(open_ch(0, false));
    const uint8_t out[] = {'a', 'b', 'c', 'd'};
    TEST_ASSERT_TRUE(det_dma_tx_submit(0, out, sizeof(out)));
    det_dma_poll();
    TEST_ASSERT_EQUAL_size_t(1, count_dir(DET_DMA_TX));
    TEST_ASSERT_EQUAL_size_t(0, count_dir(DET_DMA_RX)); // no loopback -> no RX
    TEST_ASSERT_EQUAL_UINT16(4, g_ev[0].len);
    TEST_ASSERT_NULL(g_ev[0].ptr); // TX events carry no buffer

    uint8_t cap[16];
    uint16_t n = det_dma_sim_capture(0, cap, sizeof(cap));
    TEST_ASSERT_EQUAL_UINT16(4, n);
    TEST_ASSERT_EQUAL_MEMORY(out, cap, 4);
}

void test_tx_one_in_flight_fail_closed()
{
    TEST_ASSERT_TRUE(open_ch(0, false));
    const uint8_t a[] = {1, 2, 3};
    const uint8_t b[] = {4, 5};
    TEST_ASSERT_TRUE(det_dma_tx_submit(0, a, sizeof(a)));
    TEST_ASSERT_FALSE(det_dma_tx_submit(0, b, sizeof(b))); // busy -> fail-closed
    det_dma_poll();                                        // TX completes, frees the channel
    TEST_ASSERT_TRUE(det_dma_tx_submit(0, b, sizeof(b)));
    det_dma_poll();
    TEST_ASSERT_EQUAL_size_t(2, count_dir(DET_DMA_TX));
}

void test_tx_rejects_bad_len()
{
    TEST_ASSERT_TRUE(open_ch(0, false));
    const uint8_t x[1] = {9};
    TEST_ASSERT_FALSE(det_dma_tx_submit(0, x, 0)); // zero length
    uint8_t big[DETWS_DMA_BUF_SIZE + 1] = {0};
    TEST_ASSERT_FALSE(det_dma_tx_submit(0, big, DETWS_DMA_BUF_SIZE + 1)); // oversize
    TEST_ASSERT_FALSE(det_dma_tx_submit(0, nullptr, 4));                  // null buffer
}

void test_loopback_round_trip()
{
    TEST_ASSERT_TRUE(open_ch(0, true)); // internal TX -> RX jumper
    const uint8_t ping[] = {'P', 'I', 'N', 'G'};
    TEST_ASSERT_TRUE(det_dma_tx_submit(0, ping, sizeof(ping)));
    det_dma_poll(); // one poll: TX drains, loops into ingress, RX completes
    TEST_ASSERT_EQUAL_size_t(1, count_dir(DET_DMA_TX));
    TEST_ASSERT_EQUAL_size_t(1, count_dir(DET_DMA_RX));
    std::vector<uint8_t> got = rx_concat();
    TEST_ASSERT_EQUAL_size_t(sizeof(ping), got.size());
    TEST_ASSERT_EQUAL_MEMORY(ping, got.data(), sizeof(ping)); // byte-exact round trip
}

void test_feed_fail_closed_when_full()
{
    TEST_ASSERT_TRUE(open_ch(0, false));
    uint8_t big[DETWS_DMA_BUF_SIZE * 3 + 1] = {0};
    TEST_ASSERT_FALSE(det_dma_sim_feed(0, big, sizeof(big))); // past staging -> reject whole
    uint8_t ok[DETWS_DMA_BUF_SIZE * 3] = {0};
    TEST_ASSERT_TRUE(det_dma_sim_feed(0, ok, sizeof(ok))); // exactly fits
}

void test_closed_channel_is_inert()
{
    const uint8_t x[] = {1, 2, 3};
    TEST_ASSERT_FALSE(det_dma_sim_feed(0, x, sizeof(x))); // never opened
    TEST_ASSERT_FALSE(det_dma_tx_submit(0, x, sizeof(x)));
    det_dma_poll();
    TEST_ASSERT_EQUAL_size_t(0, g_ev.size());
    TEST_ASSERT_TRUE(open_ch(0, false));
    det_dma_close(0);
    TEST_ASSERT_FALSE(det_dma_sim_feed(0, x, sizeof(x))); // closed again
}

void test_two_channels_independent()
{
    TEST_ASSERT_TRUE(open_ch(0, false));
    TEST_ASSERT_TRUE(open_ch(1, false));
    const uint8_t a[] = {0xA0, 0xA1};
    const uint8_t b[] = {0xB0, 0xB1, 0xB2};
    TEST_ASSERT_TRUE(det_dma_sim_feed(0, a, sizeof(a)));
    TEST_ASSERT_TRUE(det_dma_sim_feed(1, b, sizeof(b)));
    det_dma_poll();
    size_t ch0 = 0, ch1 = 0;
    for (size_t i = 0; i < g_ev.size(); i++)
    {
        if (g_ev[i].dir != DET_DMA_RX)
            continue;
        if (g_ev[i].channel == 0)
        {
            ch0++;
            TEST_ASSERT_EQUAL_MEMORY(a, g_ev[i].data.data(), sizeof(a));
        }
        else if (g_ev[i].channel == 1)
        {
            ch1++;
            TEST_ASSERT_EQUAL_MEMORY(b, g_ev[i].data.data(), sizeof(b));
        }
    }
    TEST_ASSERT_EQUAL_size_t(1, ch0);
    TEST_ASSERT_EQUAL_size_t(1, ch1);
}

void test_channel_guard_subconditions()
{
    det_dma_close(255); // out-of-range close is a no-op
    det_dma_close(0);   // ensure channel 0 is closed
    uint8_t b[4] = {0};
    TEST_ASSERT_FALSE(det_dma_sim_feed(0, b, sizeof(b)));        // channel not open
    TEST_ASSERT_EQUAL_UINT16(0, det_dma_sim_capture(0, b, 4));   // channel not open
    TEST_ASSERT_EQUAL_UINT16(0, det_dma_sim_capture(255, b, 4)); // bad channel
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_open_validates);
    RUN_TEST(test_ingress_emits_rx_event);
    RUN_TEST(test_buffer_fills_then_partial_flush);
    RUN_TEST(test_ping_pong_flips_buffer);
    RUN_TEST(test_egress_captures_tx);
    RUN_TEST(test_tx_one_in_flight_fail_closed);
    RUN_TEST(test_tx_rejects_bad_len);
    RUN_TEST(test_loopback_round_trip);
    RUN_TEST(test_feed_fail_closed_when_full);
    RUN_TEST(test_closed_channel_is_inert);
    RUN_TEST(test_two_channels_independent);
    RUN_TEST(test_channel_guard_subconditions);
    return UNITY_END();
}
