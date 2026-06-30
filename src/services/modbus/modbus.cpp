// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file modbus.cpp
 * @brief Modbus TCP slave: data model, MBAP/PDU codec, and the TCP transport.
 */

#include "services/modbus/modbus.h"

#if DETWS_ENABLE_MODBUS

#include "shared_primitives/shim.h"

// ---------------------------------------------------------------------------
// Data model (all BSS - no heap)
// ---------------------------------------------------------------------------

static uint8_t g_coils[(DETWS_MODBUS_COILS + 7) / 8];
static uint8_t g_discrete[(DETWS_MODBUS_DISCRETE_INPUTS + 7) / 8];
static uint16_t g_holding[DETWS_MODBUS_HOLDING_REGS];
static uint16_t g_input[DETWS_MODBUS_INPUT_REGS];
static ModbusWriteCb g_write_cb = nullptr;

static bool bit_get(const uint8_t *a, uint16_t i)
{
    return (a[i >> 3] >> (i & 7)) & 1u;
}
static void bit_set(uint8_t *a, uint16_t i, bool v)
{
    if (v)
        a[i >> 3] |= (uint8_t)(1u << (i & 7));
    else
        a[i >> 3] &= (uint8_t)~(1u << (i & 7));
}

void modbus_server_init()
{
    memset(g_coils, 0, sizeof(g_coils));
    memset(g_discrete, 0, sizeof(g_discrete));
    memset(g_holding, 0, sizeof(g_holding));
    memset(g_input, 0, sizeof(g_input));
    g_write_cb = nullptr;
}

void modbus_on_write(ModbusWriteCb cb)
{
    g_write_cb = cb;
}

bool modbus_get_coil(uint16_t addr)
{
    return (addr < DETWS_MODBUS_COILS) ? bit_get(g_coils, addr) : false;
}
void modbus_set_coil(uint16_t addr, bool on)
{
    if (addr < DETWS_MODBUS_COILS)
        bit_set(g_coils, addr, on);
}
bool modbus_get_discrete_input(uint16_t addr)
{
    return (addr < DETWS_MODBUS_DISCRETE_INPUTS) ? bit_get(g_discrete, addr) : false;
}
void modbus_set_discrete_input(uint16_t addr, bool on)
{
    if (addr < DETWS_MODBUS_DISCRETE_INPUTS)
        bit_set(g_discrete, addr, on);
}
uint16_t modbus_get_holding_reg(uint16_t addr)
{
    return (addr < DETWS_MODBUS_HOLDING_REGS) ? g_holding[addr] : 0;
}
void modbus_set_holding_reg(uint16_t addr, uint16_t value)
{
    if (addr < DETWS_MODBUS_HOLDING_REGS)
        g_holding[addr] = value;
}
uint16_t modbus_get_input_reg(uint16_t addr)
{
    return (addr < DETWS_MODBUS_INPUT_REGS) ? g_input[addr] : 0;
}
void modbus_set_input_reg(uint16_t addr, uint16_t value)
{
    if (addr < DETWS_MODBUS_INPUT_REGS)
        g_input[addr] = value;
}

// ---------------------------------------------------------------------------
// PDU codec (host-testable)
// ---------------------------------------------------------------------------

static uint16_t rd16(const uint8_t *p)
{
    return (uint16_t)((p[0] << 8) | p[1]);
}
static void wr16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)(v & 0xFF);
}

// Build an exception PDU (fc|0x80, code). Always 2 bytes.
static size_t pdu_exception(uint8_t fc, uint8_t code, uint8_t *out)
{
    out[0] = (uint8_t)(fc | 0x80);
    out[1] = code;
    return 2;
}

// Process one PDU (function code + data) against the data model. Returns the
// response PDU length, or 0 if it cannot fit (caller treats 0 as "send nothing").
static size_t modbus_process_pdu(const uint8_t *pdu, size_t pdu_len, uint8_t *out, size_t out_cap)
{
    if (pdu_len < 1)
        return 0;
    uint8_t fc = pdu[0];

    // Dispatch on the function code; each case validates its own request length and
    // address/quantity range, replying with the data or a Modbus exception PDU.
    switch (fc)
    {
    // FC1/FC2: read up to 2000 single-bit coils / discrete inputs, packed 8 per byte.
    case MODBUS_FC_READ_COILS:
    case MODBUS_FC_READ_DISCRETE_INPUTS: {
        if (pdu_len < 5)
            return pdu_exception(fc, MODBUS_EX_ILLEGAL_DATA_VALUE, out);
        uint16_t start = rd16(pdu + 1), qty = rd16(pdu + 3);
        uint16_t limit = (fc == MODBUS_FC_READ_COILS) ? DETWS_MODBUS_COILS : DETWS_MODBUS_DISCRETE_INPUTS;
        const uint8_t *src = (fc == MODBUS_FC_READ_COILS) ? g_coils : g_discrete;
        if (qty < 1 || qty > 2000)
            return pdu_exception(fc, MODBUS_EX_ILLEGAL_DATA_VALUE, out);
        if ((uint32_t)start + qty > limit)
            return pdu_exception(fc, MODBUS_EX_ILLEGAL_DATA_ADDRESS, out);
        uint8_t bytes = (uint8_t)((qty + 7) / 8);
        if ((size_t)2 + bytes > out_cap)
            return 0;
        out[0] = fc;
        out[1] = bytes;
        memset(out + 2, 0, bytes);
        for (uint16_t i = 0; i < qty; i++)
            if (bit_get(src, (uint16_t)(start + i)))
                out[2 + (i >> 3)] |= (uint8_t)(1u << (i & 7));
        return (size_t)2 + bytes;
    }

    // FC3/FC4: read up to 125 16-bit holding / input registers, big-endian.
    case MODBUS_FC_READ_HOLDING_REGS:
    case MODBUS_FC_READ_INPUT_REGS: {
        if (pdu_len < 5)
            return pdu_exception(fc, MODBUS_EX_ILLEGAL_DATA_VALUE, out);
        uint16_t start = rd16(pdu + 1), qty = rd16(pdu + 3);
        uint16_t limit = (fc == MODBUS_FC_READ_HOLDING_REGS) ? DETWS_MODBUS_HOLDING_REGS : DETWS_MODBUS_INPUT_REGS;
        const uint16_t *src = (fc == MODBUS_FC_READ_HOLDING_REGS) ? g_holding : g_input;
        if (qty < 1 || qty > 125)
            return pdu_exception(fc, MODBUS_EX_ILLEGAL_DATA_VALUE, out);
        if ((uint32_t)start + qty > limit)
            return pdu_exception(fc, MODBUS_EX_ILLEGAL_DATA_ADDRESS, out);
        uint8_t bytes = (uint8_t)(qty * 2);
        if ((size_t)2 + bytes > out_cap)
            return 0;
        out[0] = fc;
        out[1] = bytes;
        for (uint16_t i = 0; i < qty; i++)
            wr16(out + 2 + i * 2, src[start + i]);
        return (size_t)2 + bytes;
    }

    // FC5: write one coil (value 0xFF00 = on, 0x0000 = off); echo the request back.
    case MODBUS_FC_WRITE_SINGLE_COIL: {
        if (pdu_len < 5)
            return pdu_exception(fc, MODBUS_EX_ILLEGAL_DATA_VALUE, out);
        uint16_t addr = rd16(pdu + 1), value = rd16(pdu + 3);
        if (value != 0x0000 && value != 0xFF00)
            return pdu_exception(fc, MODBUS_EX_ILLEGAL_DATA_VALUE, out);
        if (addr >= DETWS_MODBUS_COILS)
            return pdu_exception(fc, MODBUS_EX_ILLEGAL_DATA_ADDRESS, out);
        bit_set(g_coils, addr, value == 0xFF00);
        if (g_write_cb)
            g_write_cb(fc, addr, 1);
        if (out_cap < 5)
            return 0;
        memcpy(out, pdu, 5); // echo request
        return 5;
    }

    // FC6: write one holding register; echo the request back.
    case MODBUS_FC_WRITE_SINGLE_REG: {
        if (pdu_len < 5)
            return pdu_exception(fc, MODBUS_EX_ILLEGAL_DATA_VALUE, out);
        uint16_t addr = rd16(pdu + 1), value = rd16(pdu + 3);
        if (addr >= DETWS_MODBUS_HOLDING_REGS)
            return pdu_exception(fc, MODBUS_EX_ILLEGAL_DATA_ADDRESS, out);
        g_holding[addr] = value;
        if (g_write_cb)
            g_write_cb(fc, addr, 1);
        if (out_cap < 5)
            return 0;
        memcpy(out, pdu, 5);
        return 5;
    }

    // FC15: write up to 1968 coils from a packed bitfield; reply start + quantity.
    case MODBUS_FC_WRITE_MULTIPLE_COILS: {
        if (pdu_len < 6)
            return pdu_exception(fc, MODBUS_EX_ILLEGAL_DATA_VALUE, out);
        uint16_t start = rd16(pdu + 1), qty = rd16(pdu + 3);
        uint8_t bc = pdu[5];
        if (qty < 1 || qty > 1968 || bc != (uint8_t)((qty + 7) / 8) || pdu_len < (size_t)6 + bc)
            return pdu_exception(fc, MODBUS_EX_ILLEGAL_DATA_VALUE, out);
        if ((uint32_t)start + qty > DETWS_MODBUS_COILS)
            return pdu_exception(fc, MODBUS_EX_ILLEGAL_DATA_ADDRESS, out);
        for (uint16_t i = 0; i < qty; i++)
        {
            bool v = (pdu[6 + (i >> 3)] >> (i & 7)) & 1u;
            bit_set(g_coils, (uint16_t)(start + i), v);
        }
        if (g_write_cb)
            g_write_cb(fc, start, qty);
        if (out_cap < 5)
            return 0;
        out[0] = fc;
        wr16(out + 1, start);
        wr16(out + 3, qty);
        return 5;
    }

    // FC16: write up to 123 holding registers; reply start + quantity.
    case MODBUS_FC_WRITE_MULTIPLE_REGS: {
        if (pdu_len < 6)
            return pdu_exception(fc, MODBUS_EX_ILLEGAL_DATA_VALUE, out);
        uint16_t start = rd16(pdu + 1), qty = rd16(pdu + 3);
        uint8_t bc = pdu[5];
        if (qty < 1 || qty > 123 || bc != (uint8_t)(qty * 2) || pdu_len < (size_t)6 + bc)
            return pdu_exception(fc, MODBUS_EX_ILLEGAL_DATA_VALUE, out);
        if ((uint32_t)start + qty > DETWS_MODBUS_HOLDING_REGS)
            return pdu_exception(fc, MODBUS_EX_ILLEGAL_DATA_ADDRESS, out);
        for (uint16_t i = 0; i < qty; i++)
            g_holding[start + i] = rd16(pdu + 6 + i * 2);
        if (g_write_cb)
            g_write_cb(fc, start, qty);
        if (out_cap < 5)
            return 0;
        out[0] = fc;
        wr16(out + 1, start);
        wr16(out + 3, qty);
        return 5;
    }

    // Any unsupported function code: reply with the ILLEGAL FUNCTION exception.
    default:
        return pdu_exception(fc, MODBUS_EX_ILLEGAL_FUNCTION, out);
    }
}

size_t modbus_process_adu(const uint8_t *req, size_t req_len, uint8_t *resp, size_t resp_cap)
{
    if (req_len < 8 || resp_cap < 8)
        return 0; // need MBAP (7) + at least a function code

    uint16_t tid = rd16(req);
    uint16_t pid = rd16(req + 2);
    uint16_t len = rd16(req + 4);
    uint8_t uid = req[6];

    if (pid != 0)
        return 0; // not Modbus
    if (len < 2 || (size_t)6 + len > req_len)
        return 0; // length field disagrees with the frame

    const uint8_t *pdu = req + 7;
    size_t pdu_len = (size_t)len - 1; // len counts the unit id + the PDU

    size_t rlen = modbus_process_pdu(pdu, pdu_len, resp + 7, resp_cap - 7);
    if (rlen == 0)
        return 0;

    wr16(resp, tid);                      // echo transaction id
    wr16(resp + 2, 0);                    // protocol id = 0
    wr16(resp + 4, (uint16_t)(1 + rlen)); // length = unit id + response PDU
    resp[6] = uid;                        // echo unit id
    return 7 + rlen;
}

#if DETWS_ENABLE_MODBUS_RTU
// CRC16-Modbus (init 0xFFFF, reflected poly 0xA001); transmitted low byte first.
static uint16_t modbus_crc16(const uint8_t *data, size_t len)
{
    uint16_t crc = 0xFFFFu;
    for (size_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (int b = 0; b < 8; b++)
            crc = (crc & 1u) ? (uint16_t)((crc >> 1) ^ 0xA001u) : (uint16_t)(crc >> 1);
    }
    return crc;
}

size_t modbus_rtu_process_adu(const uint8_t *req, size_t req_len, uint8_t *resp, size_t resp_cap, uint8_t my_addr)
{
    if (req_len < 4 || resp_cap < 4) // addr(1) + min PDU(1) + CRC(2)
        return 0;

    // Validate the trailing CRC over [addr .. last PDU byte] (low byte first).
    uint16_t want = modbus_crc16(req, req_len - 2);
    uint16_t got = (uint16_t)(req[req_len - 2] | (req[req_len - 1] << 8));
    if (want != got)
        return 0; // corrupt frame - drop silently (no response), per Modbus RTU

    uint8_t addr = req[0];
    bool broadcast = (addr == 0);
    if (!broadcast && addr != my_addr)
        return 0; // not addressed to this slave

    const uint8_t *pdu = req + 1;
    size_t pdu_len = req_len - 3; // strip addr + 2 CRC bytes

    size_t rlen = modbus_process_pdu(pdu, pdu_len, resp + 1, resp_cap - 3); // leave addr + CRC room
    if (rlen == 0)
        return 0;
    if (broadcast)
        return 0; // executed, but a broadcast gets no reply

    resp[0] = my_addr;
    uint16_t crc = modbus_crc16(resp, 1 + rlen);
    resp[1 + rlen] = (uint8_t)(crc & 0xFFu);
    resp[2 + rlen] = (uint8_t)(crc >> 8);
    return 1 + rlen + 2;
}
#endif // DETWS_ENABLE_MODBUS_RTU

// ---------------------------------------------------------------------------
// TCP transport (ESP32-only; the core above is host-tested standalone)
// ---------------------------------------------------------------------------

#if defined(ARDUINO)

#include "network_drivers/transport/transport.h"

// Bytes available in the slot's rx ring.
// Thin adapters over the transport RX read API - the ring is owned by transport;
// this service never indexes rx_buffer or advances rx_tail itself.
static size_t ring_avail(const TcpConn *c)
{
    return det_conn_available(c->id);
}
static void ring_peek(const TcpConn *c, size_t off, uint8_t *dst, size_t n)
{
    det_conn_peek(c->id, off, dst, n);
}
static void ring_consume(TcpConn *c, size_t n)
{
    det_conn_consume(c->id, n);
}

static void raw_send(uint8_t slot, const void *data, size_t n)
{
    TcpConn *c = &conn_pool[slot];
    if (c->state != CONN_ACTIVE || !c->pcb || n == 0)
        return;
    det_conn_send(c->id, data, (u16_t)n);
    det_conn_flush(c->id);
}

static void close_conn(uint8_t slot)
{
    det_conn_close(slot); // transport owns detach + slot reset + close
}

void modbus_rx(uint8_t slot)
{
    TcpConn *conn = &conn_pool[slot];

    // Frame complete ADUs out of the rx ring; a partial frame stays buffered.
    for (;;)
    {
        size_t avail = ring_avail(conn);
        if (avail < 8) // MBAP (7) + at least a function code
            break;

        uint8_t hdr[6];
        ring_peek(conn, 0, hdr, 6);
        uint16_t pid = (uint16_t)((hdr[2] << 8) | hdr[3]);
        uint16_t len = (uint16_t)((hdr[4] << 8) | hdr[5]);
        if (pid != 0 || len < 2 || len > (MODBUS_ADU_MAX - 6))
        {
            close_conn(slot); // not a Modbus TCP frame - drop the connection
            return;
        }
        size_t frame_total = (size_t)6 + len;
        if (avail < frame_total)
            break; // wait for the rest of the frame

        uint8_t adu[MODBUS_ADU_MAX];
        ring_peek(conn, 0, adu, frame_total);
        ring_consume(conn, frame_total);

        uint8_t resp[MODBUS_ADU_MAX];
        size_t rl = modbus_process_adu(adu, frame_total, resp, sizeof(resp));
        if (rl)
            raw_send(slot, resp, rl);
    }
}

#endif // ARDUINO

#endif // DETWS_ENABLE_MODBUS
