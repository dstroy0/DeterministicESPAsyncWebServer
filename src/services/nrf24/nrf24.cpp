// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file nrf24.cpp
 * @brief nRF24L01+ driver - implementation.
 *
 * The nRF24L01+ command protocol (Nordic datasheet): every SPI transaction is a command
 * byte followed by data, and the STATUS register is shifted out on the first byte. Register
 * access is R_REGISTER (0x00 | reg) / W_REGISTER (0x20 | reg); payloads use R_RX_PAYLOAD /
 * W_TX_PAYLOAD. Host-testable with a mock; the RF link needs the module.
 */

#include "services/nrf24/nrf24.h"

#if DETWS_ENABLE_NRF24

namespace
{
// Commands.
enum
{
    CMD_R_REGISTER = 0x00,
    CMD_W_REGISTER = 0x20,
    CMD_R_RX_PAYLOAD = 0x61,
    CMD_W_TX_PAYLOAD = 0xA0,
    CMD_FLUSH_TX = 0xE1,
    CMD_FLUSH_RX = 0xE2,
    CMD_NOP = 0xFF,
};

// Registers.
enum
{
    REG_CONFIG = 0x00,
    REG_EN_AA = 0x01,
    REG_EN_RXADDR = 0x02,
    REG_SETUP_AW = 0x03,
    REG_SETUP_RETR = 0x04,
    REG_RF_CH = 0x05,
    REG_RF_SETUP = 0x06,
    REG_STATUS = 0x07,
    REG_RX_ADDR_P0 = 0x0A,
    REG_TX_ADDR = 0x10,
    REG_RX_PW_P0 = 0x11,
};

// CONFIG bits.
enum
{
    CFG_EN_CRC = 0x08,
    CFG_CRCO = 0x04,
    CFG_PWR_UP = 0x02,
    CFG_PRIM_RX = 0x01,
};

// STATUS bits.
enum
{
    ST_RX_DR = 0x40,
    ST_TX_DS = 0x20,
    ST_RX_P_NO = 0x0E, // bits 3:1 = pipe of the payload at the RX FIFO head
};

void reg_write(const nrf_bus *b, uint8_t reg, uint8_t val)
{
    uint8_t tx[2] = {(uint8_t)(CMD_W_REGISTER | reg), val};
    uint8_t rx[2];
    b->spi(tx, rx, 2, b->ctx);
}

uint8_t reg_read(const nrf_bus *b, uint8_t reg)
{
    uint8_t tx[2] = {(uint8_t)(CMD_R_REGISTER | reg), 0xFF};
    uint8_t rx[2];
    b->spi(tx, rx, 2, b->ctx);
    return rx[1];
}

void reg_write_buf(const nrf_bus *b, uint8_t reg, const uint8_t *buf, uint8_t n)
{
    uint8_t tx[6];
    uint8_t rx[6];
    tx[0] = (uint8_t)(CMD_W_REGISTER | reg);
    for (uint8_t i = 0; i < n; i++)
        tx[1 + i] = buf[i];
    b->spi(tx, rx, (uint8_t)(n + 1), b->ctx);
}

uint8_t status(const nrf_bus *b)
{
    uint8_t tx[1] = {CMD_NOP};
    uint8_t rx[1];
    b->spi(tx, rx, 1, b->ctx);
    return rx[0];
}

void cmd(const nrf_bus *b, uint8_t c)
{
    uint8_t tx[1] = {c};
    uint8_t rx[1];
    b->spi(tx, rx, 1, b->ctx);
}
} // namespace

bool nrf24_init(const nrf_bus *bus, const nrf_config *cfg)
{
    if (!bus || !bus->spi || !bus->ce || !cfg || !cfg->address)
        return false;
    bus->ce(false, bus->ctx);

    reg_write(bus, REG_CONFIG, CFG_EN_CRC | CFG_CRCO); // power down, 16-bit CRC
    reg_write(bus, REG_RF_CH, cfg->channel);
    if (reg_read(bus, REG_RF_CH) != cfg->channel)
        return false; // written value did not read back -> no chip on the bus

    reg_write(bus, REG_SETUP_AW, 0x03);   // 5-byte addresses
    reg_write(bus, REG_EN_RXADDR, 0x01);  // enable pipe 0
    reg_write(bus, REG_EN_AA, 0x00);      // raw mode (no auto-ack)
    reg_write(bus, REG_SETUP_RETR, 0x00); // no auto-retransmit
    uint8_t dr = (cfg->data_rate == 1) ? 0x08 : (cfg->data_rate == 2) ? 0x20 : 0x00;
    reg_write(bus, REG_RF_SETUP, (uint8_t)(dr | ((cfg->tx_power & 0x03) << 1)));
    reg_write(bus, REG_RX_PW_P0, DETWS_NRF24_PAYLOAD);
    reg_write_buf(bus, REG_RX_ADDR_P0, cfg->address, 5);
    reg_write_buf(bus, REG_TX_ADDR, cfg->address, 5);

    cmd(bus, CMD_FLUSH_RX);
    cmd(bus, CMD_FLUSH_TX);
    reg_write(bus, REG_STATUS, ST_RX_DR | ST_TX_DS | 0x10); // clear all flags

    reg_write(bus, REG_CONFIG, CFG_EN_CRC | CFG_CRCO | CFG_PWR_UP); // power up (standby)
    return true;
}

bool nrf24_send(const nrf_bus *bus, const uint8_t *data, uint8_t len)
{
    if (!bus || !data || len == 0 || len > DETWS_NRF24_PAYLOAD)
        return false;
    bus->ce(false, bus->ctx);
    reg_write(bus, REG_CONFIG, CFG_EN_CRC | CFG_CRCO | CFG_PWR_UP); // PRIM_RX = 0 -> PTX

    uint8_t tx[1 + DETWS_NRF24_PAYLOAD];
    uint8_t rx[1 + DETWS_NRF24_PAYLOAD];
    tx[0] = CMD_W_TX_PAYLOAD;
    for (uint8_t i = 0; i < DETWS_NRF24_PAYLOAD; i++)
        tx[1 + i] = (i < len) ? data[i] : 0x00; // zero-pad to the static width
    bus->spi(tx, rx, (uint8_t)(DETWS_NRF24_PAYLOAD + 1), bus->ctx);

    bus->ce(true, bus->ctx); // key the transmit
    return true;
}

bool nrf24_tx_done(const nrf_bus *bus)
{
    if (!bus)
        return false;
    if (status(bus) & ST_TX_DS)
    {
        reg_write(bus, REG_STATUS, ST_TX_DS); // write-1-to-clear
        return true;
    }
    return false;
}

void nrf24_set_rx(const nrf_bus *bus)
{
    if (!bus)
        return;
    reg_write(bus, REG_CONFIG, CFG_EN_CRC | CFG_CRCO | CFG_PWR_UP | CFG_PRIM_RX); // PRX
    bus->ce(true, bus->ctx);
}

int nrf24_recv(const nrf_bus *bus, uint8_t *buf, uint8_t cap, uint8_t *pipe)
{
    if (!bus || !buf)
        return -1;
    uint8_t st = status(bus);
    if (!(st & ST_RX_DR))
        return -1; // nothing received
    uint8_t p = (uint8_t)((st & ST_RX_P_NO) >> 1);
    if (p > 5) // 0x07 = RX FIFO empty
    {
        reg_write(bus, REG_STATUS, ST_RX_DR);
        return -1;
    }
    uint8_t tx[1 + DETWS_NRF24_PAYLOAD];
    uint8_t rx[1 + DETWS_NRF24_PAYLOAD];
    tx[0] = CMD_R_RX_PAYLOAD;
    for (uint8_t i = 0; i < DETWS_NRF24_PAYLOAD; i++)
        tx[1 + i] = 0xFF;
    bus->spi(tx, rx, (uint8_t)(DETWS_NRF24_PAYLOAD + 1), bus->ctx);

    uint8_t n = (DETWS_NRF24_PAYLOAD < cap) ? DETWS_NRF24_PAYLOAD : cap;
    for (uint8_t i = 0; i < n; i++)
        buf[i] = rx[1 + i];
    if (pipe)
        *pipe = p;
    reg_write(bus, REG_STATUS, ST_RX_DR); // clear
    return (int)n;
}

#endif // DETWS_ENABLE_NRF24
