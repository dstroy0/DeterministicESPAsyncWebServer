// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file lora.cpp
 * @brief LoRa codec + SX127x driver - implementation.
 *
 * The codec is the RadioHead 4-byte header. The driver speaks the SX1276/77/78/79 LoRa
 * register protocol (datasheet register map below) through the caller's register-access
 * bus, so the sequence is host-testable with a mock register file and portable across SPI
 * peripherals. The RF link itself needs the module.
 */

#include "services/lora/lora.h"

#if DETWS_ENABLE_LORA

namespace
{
// SX127x LoRa register map (SX1276 datasheet, Table 41).
enum
{
    REG_FIFO = 0x00,
    REG_OP_MODE = 0x01,
    REG_FRF_MSB = 0x06,
    REG_FRF_MID = 0x07,
    REG_FRF_LSB = 0x08,
    REG_PA_CONFIG = 0x09,
    REG_FIFO_ADDR_PTR = 0x0D,
    REG_FIFO_TX_BASE = 0x0E,
    REG_FIFO_RX_BASE = 0x0F,
    REG_FIFO_RX_CURRENT = 0x10,
    REG_IRQ_FLAGS = 0x12,
    REG_RX_NB_BYTES = 0x13,
    REG_PKT_RSSI = 0x1A,
    REG_MODEM_CONFIG1 = 0x1D,
    REG_MODEM_CONFIG2 = 0x1E,
    REG_PREAMBLE_MSB = 0x20,
    REG_PREAMBLE_LSB = 0x21,
    REG_PAYLOAD_LENGTH = 0x22,
    REG_MODEM_CONFIG3 = 0x26,
    REG_SYNC_WORD = 0x39,
    REG_VERSION = 0x42,
};

// RegOpMode: LongRangeMode bit + transceiver mode.
enum
{
    MODE_LORA = 0x80,
    MODE_SLEEP = 0x00,
    MODE_STDBY = 0x01,
    MODE_TX = 0x03,
    MODE_RX_CONT = 0x05,
};

// RegIrqFlags.
enum
{
    IRQ_TX_DONE = 0x08,
    IRQ_PAYLOAD_CRC_ERROR = 0x20,
    IRQ_RX_DONE = 0x40,
};

const uint8_t SX127X_VERSION = 0x12;

inline uint8_t rd(const lora_bus *b, uint8_t reg)
{
    return b->read(reg, b->ctx);
}
inline void wr(const lora_bus *b, uint8_t reg, uint8_t val)
{
    b->write(reg, val, b->ctx);
}
} // namespace

bool lora_frame_parse(const uint8_t *raw, uint16_t len, lora_header *hdr, const uint8_t **payload,
                      uint16_t *payload_len)
{
    if (!raw || !hdr || len < 4)
        return false;
    hdr->to = raw[0];
    hdr->from = raw[1];
    hdr->id = raw[2];
    hdr->flags = raw[3];
    if (payload)
        *payload = raw + 4;
    if (payload_len)
        *payload_len = (uint16_t)(len - 4);
    return true;
}

uint16_t lora_frame_build(const lora_header *hdr, const uint8_t *payload, uint16_t len, uint8_t *out, uint16_t cap)
{
    if (!hdr || !out || len > DETWS_LORA_MAX_PAYLOAD || (uint32_t)len + 4 > cap)
        return 0;
    out[0] = hdr->to;
    out[1] = hdr->from;
    out[2] = hdr->id;
    out[3] = hdr->flags;
    for (uint16_t i = 0; i < len; i++)
        out[4 + i] = payload[i];
    return (uint16_t)(len + 4);
}

bool lora_init(const lora_bus *bus, const lora_config *cfg)
{
    if (!bus || !bus->read || !bus->write || !cfg)
        return false;
    if (rd(bus, REG_VERSION) != SX127X_VERSION)
        return false; // the bus is not talking to an SX127x

    // Switch to LoRa mode (only settable from sleep), then standby.
    wr(bus, REG_OP_MODE, MODE_SLEEP);
    wr(bus, REG_OP_MODE, MODE_LORA | MODE_SLEEP);
    wr(bus, REG_OP_MODE, MODE_LORA | MODE_STDBY);

    // Carrier frequency: Frf = freq / FSTEP, FSTEP = 32 MHz / 2^19.
    uint32_t frf = (uint32_t)(((uint64_t)cfg->freq_hz << 19) / 32000000UL);
    wr(bus, REG_FRF_MSB, (uint8_t)(frf >> 16));
    wr(bus, REG_FRF_MID, (uint8_t)(frf >> 8));
    wr(bus, REG_FRF_LSB, (uint8_t)frf);

    wr(bus, REG_FIFO_TX_BASE, 0x00);
    wr(bus, REG_FIFO_RX_BASE, 0x00);

    // Modem config: explicit header, CRC on, AGC auto; low-data-rate optimize at SF11/12.
    wr(bus, REG_MODEM_CONFIG1, (uint8_t)((cfg->bandwidth << 4) | (cfg->coding_rate << 1)));
    wr(bus, REG_MODEM_CONFIG2, (uint8_t)((cfg->spreading << 4) | 0x04));
    wr(bus, REG_MODEM_CONFIG3, (uint8_t)((cfg->spreading >= 11 ? 0x08 : 0x00) | 0x04));

    wr(bus, REG_PREAMBLE_MSB, 0x00);
    wr(bus, REG_PREAMBLE_LSB, 0x08);
    wr(bus, REG_SYNC_WORD, cfg->sync_word);
    wr(bus, REG_PA_CONFIG, (uint8_t)(0x80 | ((cfg->tx_power - 2) & 0x0F))); // PA_BOOST pin

    wr(bus, REG_OP_MODE, MODE_LORA | MODE_STDBY);
    return true;
}

bool lora_send(const lora_bus *bus, const uint8_t *frame, uint8_t len)
{
    if (!bus || !frame || len == 0 || len > DETWS_LORA_MAX_PAYLOAD + 4)
        return false;
    wr(bus, REG_OP_MODE, MODE_LORA | MODE_STDBY);
    wr(bus, REG_FIFO_ADDR_PTR, 0x00);
    for (uint8_t i = 0; i < len; i++)
        wr(bus, REG_FIFO, frame[i]);
    wr(bus, REG_PAYLOAD_LENGTH, len);
    wr(bus, REG_OP_MODE, MODE_LORA | MODE_TX);
    return true;
}

bool lora_tx_done(const lora_bus *bus)
{
    if (!bus)
        return false;
    if (rd(bus, REG_IRQ_FLAGS) & IRQ_TX_DONE)
    {
        wr(bus, REG_IRQ_FLAGS, 0xFF); // clear all IRQ flags
        return true;
    }
    return false;
}

void lora_set_rx(const lora_bus *bus)
{
    if (!bus)
        return;
    wr(bus, REG_FIFO_ADDR_PTR, 0x00);
    wr(bus, REG_OP_MODE, MODE_LORA | MODE_RX_CONT);
}

int lora_recv(const lora_bus *bus, uint8_t *buf, uint8_t cap, int16_t *rssi)
{
    if (!bus || !buf)
        return -1;
    uint8_t flags = rd(bus, REG_IRQ_FLAGS);
    if (!(flags & IRQ_RX_DONE))
        return -1; // nothing received
    if (flags & IRQ_PAYLOAD_CRC_ERROR)
    {
        wr(bus, REG_IRQ_FLAGS, 0xFF);
        return -1; // corrupt frame, dropped
    }
    uint8_t len = rd(bus, REG_RX_NB_BYTES);
    wr(bus, REG_FIFO_ADDR_PTR, rd(bus, REG_FIFO_RX_CURRENT));
    uint8_t n = 0;
    for (uint8_t i = 0; i < len; i++)
    {
        uint8_t b = rd(bus, REG_FIFO); // advances the FIFO pointer
        if (n < cap)
            buf[n++] = b;
    }
    if (rssi)
        *rssi = (int16_t)(-157 + rd(bus, REG_PKT_RSSI)); // HF port (868/915 MHz)
    wr(bus, REG_IRQ_FLAGS, 0xFF);
    return (int)n;
}

#endif // DETWS_ENABLE_LORA
