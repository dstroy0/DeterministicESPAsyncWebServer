// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dma.cpp
 * @brief DMA peripheral ingest / egress - implementation.
 *
 * DETWS_DMA_SIMULATE (default) runs an in-memory model of the peripheral: an ingress
 * staging ring feeds the ping-pong RX buffers, egress DMA drains the TX buffer into a
 * capture ring, and a loopback channel routes its own TX back into its RX. det_dma_poll()
 * advances that engine and fires the completion callbacks - so the whole pipeline is
 * host- and device-testable with no physical loopback. When the flag is 0, the front end
 * dispatches to the weak det_dma_hw_* hooks a real silicon driver overrides.
 */

#include "services/dma/dma.h"

#if DETWS_ENABLE_DMA

#include <string.h> // memcpy

#ifdef ARDUINO
#include "services/clock.h" // detws_millis()
#endif

namespace
{
uint32_t dma_now()
{
#ifdef ARDUINO
    return detws_millis();
#else
    return 0; // host builds have no clock dependency; t_ms is informational
#endif
}
} // namespace

#if DETWS_DMA_SIMULATE

// Ingress/egress staging holds a few buffers' worth so a single feed can span more than
// one RX transfer (exercising the ping-pong flip) and several TX submits can accumulate
// before capture.
#define DMA_STAGE_CAP (DETWS_DMA_BUF_SIZE * 3)

namespace
{
// Fixed-capacity byte FIFO (no heap): the simulator's ingress and egress staging.
struct byte_ring
{
    uint8_t buf[DMA_STAGE_CAP];
    uint16_t head; // read cursor
    uint16_t len;  // bytes queued

    void reset()
    {
        head = 0;
        len = 0;
    }
    uint16_t space() const
    {
        return (uint16_t)(DMA_STAGE_CAP - len);
    }
    // Append n bytes; fail-closed (append nothing) if they would not all fit.
    bool push(const uint8_t *p, uint16_t n)
    {
        if (n > space())
            return false;
        for (uint16_t i = 0; i < n; i++)
        {
            buf[(head + len) % DMA_STAGE_CAP] = p[i];
            len++;
        }
        return true;
    }
    // Pop up to max bytes into out; returns how many.
    uint16_t pop(uint8_t *out, uint16_t max)
    {
        uint16_t n = (len < max) ? len : max;
        for (uint16_t i = 0; i < n; i++)
        {
            out[i] = buf[head];
            head = (head + 1) % DMA_STAGE_CAP;
            len--;
        }
        return n;
    }
};

struct dma_channel
{
    uint8_t rx_buf[2][DETWS_DMA_BUF_SIZE]; // ping-pong RX
    uint8_t tx_buf[DETWS_DMA_BUF_SIZE];    // egress staging
    byte_ring ingress;                     // sim: bytes arriving on the RX line
    byte_ring egress;                      // sim: bytes transmitted via egress DMA
    det_dma_cb cb;
    void *ctx;
    uint16_t rx_fill;  // bytes in the active RX buffer since the last completion
    uint16_t tx_len;   // bytes pending egress (0 = idle)
    uint16_t seq;      // completion sequence
    uint8_t rx_active; // which ping-pong buffer the engine is filling
    det_dma_periph periph;
    bool loopback;
    bool tx_busy;
    bool open;
};

// All DMA simulator state, owned by one instance (internal linkage): the channel table,
// so it is one named owner, unreachable from any other translation unit.
struct DmaCtx
{
    dma_channel ch[DETWS_DMA_CHANNELS];
};
DmaCtx s_dma;

void emit(dma_channel &c, uint8_t id, det_dma_dir dir, const uint8_t *data, uint16_t len)
{
    det_dma_event ev;
    ev.data = data;
    ev.t_ms = dma_now();
    ev.len = len;
    ev.seq = c.seq++;
    ev.channel = id;
    ev.periph = c.periph;
    ev.dir = dir;
    ev._pad = 0;
    if (c.cb)
        c.cb(&ev, c.ctx);
}

// Complete whatever is queued for the channel: drain egress (TX), route loopback back
// into ingress, then feed ingress into the ping-pong RX buffers, emitting one event per
// full buffer and a final partial event (models the UART idle-line flush) so every poll
// delivers all fed bytes.
void pump(dma_channel &c, uint8_t id)
{
    if (c.tx_busy)
    {
        if (c.loopback)
            c.ingress.push(c.tx_buf, c.tx_len); // internal TX->RX jumper
        c.egress.push(c.tx_buf, c.tx_len);      // capture (best-effort)
        uint16_t sent = c.tx_len;
        c.tx_busy = false;
        c.tx_len = 0;
        emit(c, id, det_dma_dir::DET_DMA_TX, nullptr, sent);
    }

    while (c.ingress.len > 0)
    {
        uint16_t room = (uint16_t)(DETWS_DMA_BUF_SIZE - c.rx_fill);
        uint16_t got = c.ingress.pop(c.rx_buf[c.rx_active] + c.rx_fill, room);
        c.rx_fill += got;
        if (c.rx_fill == DETWS_DMA_BUF_SIZE) // buffer full -> complete + ping-pong flip
        {
            emit(c, id, det_dma_dir::DET_DMA_RX, c.rx_buf[c.rx_active], DETWS_DMA_BUF_SIZE);
            c.rx_active ^= 1;
            c.rx_fill = 0;
        }
    }
    if (c.rx_fill > 0) // idle-line flush of the trailing partial buffer
    {
        emit(c, id, det_dma_dir::DET_DMA_RX, c.rx_buf[c.rx_active], c.rx_fill);
        c.rx_active ^= 1;
        c.rx_fill = 0;
    }
}
} // namespace

bool det_dma_open(const det_dma_config *cfg)
{
    if (!cfg || !cfg->on_complete || cfg->channel >= DETWS_DMA_CHANNELS)
        return false;
    dma_channel &c = s_dma.ch[cfg->channel];
    if (c.open)
        return false;
    c.ingress.reset();
    c.egress.reset();
    c.cb = cfg->on_complete;
    c.ctx = cfg->ctx;
    c.rx_fill = 0;
    c.tx_len = 0;
    c.seq = 0;
    c.rx_active = 0;
    c.periph = cfg->periph;
    c.loopback = cfg->loopback;
    c.tx_busy = false;
    c.open = true;
    return true;
}

bool det_dma_tx_submit(uint8_t ch, const uint8_t *buf, uint16_t len)
{
    if (ch >= DETWS_DMA_CHANNELS || !buf || len == 0 || len > DETWS_DMA_BUF_SIZE)
        return false;
    dma_channel &c = s_dma.ch[ch];
    if (!c.open || c.tx_busy) // one transfer in flight at a time (fail-closed)
        return false;
    memcpy(c.tx_buf, buf, len);
    c.tx_len = len;
    c.tx_busy = true;
    return true;
}

void det_dma_close(uint8_t ch)
{
    if (ch >= DETWS_DMA_CHANNELS)
        return;
    s_dma.ch[ch].open = false;
}

void det_dma_poll(void)
{
    for (uint8_t i = 0; i < DETWS_DMA_CHANNELS; i++)
        if (s_dma.ch[i].open)
            pump(s_dma.ch[i], i);
}

bool det_dma_sim_feed(uint8_t ch, const uint8_t *bytes, uint16_t len)
{
    if (ch >= DETWS_DMA_CHANNELS || !bytes)
        return false;
    dma_channel &c = s_dma.ch[ch];
    if (!c.open)
        return false;
    return c.ingress.push(bytes, len);
}

uint16_t det_dma_sim_capture(uint8_t ch, uint8_t *out, uint16_t max)
{
    if (ch >= DETWS_DMA_CHANNELS || !out)
        return 0;
    dma_channel &c = s_dma.ch[ch];
    if (!c.open)
        return 0;
    return c.egress.pop(out, max);
}

#else // real silicon backend: dispatch to weak hooks a driver overrides (fail-closed).

extern "C"
{
    __attribute__((weak)) bool det_dma_hw_open(const det_dma_config *cfg)
    {
        (void)cfg;
        return false;
    }
    __attribute__((weak)) bool det_dma_hw_tx_submit(uint8_t ch, const uint8_t *buf, uint16_t len)
    {
        (void)ch;
        (void)buf;
        (void)len;
        return false;
    }
    __attribute__((weak)) void det_dma_hw_close(uint8_t ch)
    {
        (void)ch;
    }
    __attribute__((weak)) void det_dma_hw_poll(void)
    {
    }
}

bool det_dma_open(const det_dma_config *cfg)
{
    if (!cfg || !cfg->on_complete || cfg->channel >= DETWS_DMA_CHANNELS)
        return false;
    return det_dma_hw_open(cfg);
}

bool det_dma_tx_submit(uint8_t ch, const uint8_t *buf, uint16_t len)
{
    if (ch >= DETWS_DMA_CHANNELS || !buf || len == 0 || len > DETWS_DMA_BUF_SIZE)
        return false;
    return det_dma_hw_tx_submit(ch, buf, len);
}

void det_dma_close(uint8_t ch)
{
    if (ch < DETWS_DMA_CHANNELS)
        det_dma_hw_close(ch);
}

void det_dma_poll(void)
{
    det_dma_hw_poll();
}

#endif // DETWS_DMA_SIMULATE

#endif // DETWS_ENABLE_DMA
