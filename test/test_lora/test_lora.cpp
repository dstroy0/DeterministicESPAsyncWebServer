// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the LoRa codec + SX127x driver (services/lora). The codec (RadioHead
// 4-byte header parse/build) is pure. The driver's register protocol is exercised against
// a mock SX127x: a register file plus a FIFO with the chip's auto-incrementing address
// pointer, so init / send / tx-done / set-rx / recv are verified without a radio (the RF
// link itself needs the module).

#include "services/lora/lora.h"
#include <string.h>
#include <unity.h>

// --- Mock SX127x -----------------------------------------------------------------------

struct MockChip
{
    uint8_t reg[128];
    uint8_t fifo[256];
    uint16_t fifo_ptr;
};
static MockChip g_chip;

static uint8_t mock_read(uint8_t reg, void *ctx)
{
    MockChip *c = (MockChip *)ctx;
    if (reg == 0x00) // RegFifo: read at the pointer, then advance
        return c->fifo[c->fifo_ptr++ & 0xFF];
    return c->reg[reg & 0x7F];
}
static void mock_write(uint8_t reg, uint8_t val, void *ctx)
{
    MockChip *c = (MockChip *)ctx;
    if (reg == 0x0D) // RegFifoAddrPtr: sets the internal FIFO pointer
    {
        c->fifo_ptr = val;
        c->reg[0x0D] = val;
        return;
    }
    if (reg == 0x00) // RegFifo: write at the pointer, then advance
    {
        c->fifo[c->fifo_ptr++ & 0xFF] = val;
        return;
    }
    c->reg[reg & 0x7F] = val;
}

static dws_lora_bus g_bus = {mock_read, mock_write, &g_chip};

static dws_lora_config default_cfg()
{
    dws_lora_config c = {};
    c.freq_hz = 915000000UL;
    c.spreading = 7;
    c.bandwidth = 7; // 125 kHz
    c.coding_rate = 1;
    c.sync_word = 0x12;
    c.tx_power = 17;
    return c;
}

void setUp()
{
    memset(&g_chip, 0, sizeof(g_chip));
    g_chip.reg[0x42] = 0x12; // RegVersion = SX127x id by default
}
void tearDown()
{
}

// --- Codec ------------------------------------------------------------------------------

void test_frame_build_then_parse()
{
    dws_lora_header h = {0xAA, 0x02, 0x03, 0x00};
    const uint8_t pay[3] = {'h', 'i', '!'};
    uint8_t frame[16];
    uint16_t n = dws_lora_frame_build(&h, pay, 3, frame, sizeof(frame));
    TEST_ASSERT_EQUAL_UINT16(7, n); // 4 header + 3 payload

    dws_lora_header out = {};
    const uint8_t *p = nullptr;
    uint16_t pl = 0;
    TEST_ASSERT_TRUE(dws_lora_frame_parse(frame, n, &out, &p, &pl));
    TEST_ASSERT_EQUAL_UINT8(0xAA, out.to);
    TEST_ASSERT_EQUAL_UINT8(0x02, out.from);
    TEST_ASSERT_EQUAL_UINT8(0x03, out.id);
    TEST_ASSERT_EQUAL_UINT16(3, pl);
    TEST_ASSERT_EQUAL_MEMORY(pay, p, 3);
}

void test_frame_parse_rejects_short()
{
    dws_lora_header h = {};
    const uint8_t raw[3] = {1, 2, 3};
    TEST_ASSERT_FALSE(dws_lora_frame_parse(raw, 3, &h, nullptr, nullptr)); // shorter than the header
}

void test_frame_build_bounds()
{
    dws_lora_header h = {};
    uint8_t pay[8] = {0};
    uint8_t small[5];
    TEST_ASSERT_EQUAL_UINT16(0, dws_lora_frame_build(&h, pay, 8, small, sizeof(small))); // 12 > cap 5
}

// --- Driver -----------------------------------------------------------------------------

void test_init_verifies_chip_and_lands_in_standby()
{
    dws_lora_config c = default_cfg();
    TEST_ASSERT_TRUE(dws_lora_init(&g_bus, &c));
    TEST_ASSERT_EQUAL_HEX8(0x81, g_chip.reg[0x01]); // RegOpMode = LoRa | STANDBY
    TEST_ASSERT_EQUAL_HEX8(0x12, g_chip.reg[0x39]); // RegSyncWord
}

void test_init_fails_on_wrong_version()
{
    g_chip.reg[0x42] = 0x00; // not an SX127x
    dws_lora_config c = default_cfg();
    TEST_ASSERT_FALSE(dws_lora_init(&g_bus, &c));
}

void test_init_programs_frequency()
{
    dws_lora_config c = default_cfg(); // 915 MHz
    dws_lora_init(&g_bus, &c);
    uint32_t frf = ((uint32_t)g_chip.reg[0x06] << 16) | ((uint32_t)g_chip.reg[0x07] << 8) | g_chip.reg[0x08];
    // freq = frf * FSTEP, FSTEP = 32e6 / 2^19; reconstruct and check it is ~915 MHz.
    uint32_t freq = (uint32_t)(((uint64_t)frf * 32000000UL) >> 19);
    TEST_ASSERT_UINT32_WITHIN(100, 915000000UL, freq);
}

void test_send_loads_fifo_and_starts_tx()
{
    dws_lora_config c = default_cfg();
    dws_lora_init(&g_bus, &c);
    const uint8_t frame[6] = {0x01, 0x02, 0x03, 0x00, 0xDE, 0xAD};
    TEST_ASSERT_TRUE(dws_lora_send(&g_bus, frame, 6));
    TEST_ASSERT_EQUAL_HEX8(0x83, g_chip.reg[0x01]); // RegOpMode = LoRa | TX
    TEST_ASSERT_EQUAL_UINT8(6, g_chip.reg[0x22]);   // RegPayloadLength
    TEST_ASSERT_EQUAL_MEMORY(frame, g_chip.fifo, 6);
}

void test_tx_done_flag()
{
    TEST_ASSERT_FALSE(dws_lora_tx_done(&g_bus)); // no flag
    g_chip.reg[0x12] = 0x08;                     // RegIrqFlags TxDone
    TEST_ASSERT_TRUE(dws_lora_tx_done(&g_bus));
    TEST_ASSERT_EQUAL_HEX8(0xFF, g_chip.reg[0x12]); // flags cleared
}

void test_set_rx_enters_continuous()
{
    dws_lora_set_rx(&g_bus);
    TEST_ASSERT_EQUAL_HEX8(0x85, g_chip.reg[0x01]); // RegOpMode = LoRa | RX_CONTINUOUS
}

void test_recv_reads_frame_and_rssi()
{
    const uint8_t frame[5] = {0x02, 0x01, 0x07, 0x00, 0x5A};
    memcpy(g_chip.fifo, frame, 5);
    g_chip.reg[0x12] = 0x40; // RegIrqFlags RxDone
    g_chip.reg[0x13] = 5;    // RegRxNbBytes
    g_chip.reg[0x10] = 0;    // RegFifoRxCurrentAddr
    g_chip.reg[0x1A] = 120;  // RegPktRssiValue -> -157 + 120 = -37 dBm

    uint8_t buf[16];
    int16_t rssi = 0;
    int n = dws_lora_recv(&g_bus, buf, sizeof(buf), &rssi);
    TEST_ASSERT_EQUAL_INT(5, n);
    TEST_ASSERT_EQUAL_MEMORY(frame, buf, 5);
    TEST_ASSERT_EQUAL_INT16(-37, rssi);
    TEST_ASSERT_EQUAL_HEX8(0xFF, g_chip.reg[0x12]); // IRQ cleared
}

void test_recv_no_packet()
{
    uint8_t buf[16];
    TEST_ASSERT_EQUAL_INT(-1, dws_lora_recv(&g_bus, buf, sizeof(buf), nullptr)); // no RxDone
}

void test_recv_crc_error_dropped()
{
    g_chip.reg[0x12] = 0x40 | 0x20; // RxDone | PayloadCrcError
    g_chip.reg[0x13] = 4;
    uint8_t buf[16];
    TEST_ASSERT_EQUAL_INT(-1, dws_lora_recv(&g_bus, buf, sizeof(buf), nullptr));
    TEST_ASSERT_EQUAL_HEX8(0xFF, g_chip.reg[0x12]); // IRQ still cleared
}

void test_recv_truncates_to_cap()
{
    uint8_t frame[10];
    for (int i = 0; i < 10; i++)
        frame[i] = (uint8_t)(0x10 + i);
    memcpy(g_chip.fifo, frame, 10);
    g_chip.reg[0x12] = 0x40;
    g_chip.reg[0x13] = 10;
    g_chip.reg[0x10] = 0;
    uint8_t buf[4];
    int n = dws_lora_recv(&g_bus, buf, sizeof(buf), nullptr);
    TEST_ASSERT_EQUAL_INT(4, n); // capped
    TEST_ASSERT_EQUAL_MEMORY(frame, buf, 4);
}

void test_frame_parse_build_guards()
{
    dws_lora_header hdr = {};
    const uint8_t *payload = nullptr;
    uint16_t payload_len = 0;
    uint8_t too_short[1] = {0};
    TEST_ASSERT_FALSE(dws_lora_frame_parse(too_short, sizeof(too_short), &hdr, &payload, &payload_len)); // too short
    uint8_t out[4];
    uint8_t pay[8] = {0};
    TEST_ASSERT_EQUAL_UINT16(0, dws_lora_frame_build(&hdr, pay, sizeof(pay), out, 2)); // cap too small
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_frame_build_then_parse);
    RUN_TEST(test_frame_parse_rejects_short);
    RUN_TEST(test_frame_build_bounds);
    RUN_TEST(test_init_verifies_chip_and_lands_in_standby);
    RUN_TEST(test_init_fails_on_wrong_version);
    RUN_TEST(test_init_programs_frequency);
    RUN_TEST(test_send_loads_fifo_and_starts_tx);
    RUN_TEST(test_tx_done_flag);
    RUN_TEST(test_set_rx_enters_continuous);
    RUN_TEST(test_recv_reads_frame_and_rssi);
    RUN_TEST(test_recv_no_packet);
    RUN_TEST(test_recv_crc_error_dropped);
    RUN_TEST(test_recv_truncates_to_cap);
    RUN_TEST(test_frame_parse_build_guards);
    return UNITY_END();
}
