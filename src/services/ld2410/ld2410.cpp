// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ld2410.cpp
 * @brief HLK-LD2410 mmWave radar codec - implementation. See ld2410.h.
 *
 * Frame layout (little-endian lengths / distances), per the Hi-Link serial protocol:
 *   report:  F4 F3 F2 F1 | len(2) | type | AA | state | mv_cm(2) mv_e | st_cm(2) st_e |
 *            det_cm(2) | [engineering: maxMvGate maxStGate mvE[9] stE[9] light out] |
 *            55 | check | F8 F7 F6 F5     (len = 0x0D basic, 0x23 engineering)
 *   command: FD FC FB FA | len(2) | word(2) | [value] | 04 03 02 01
 */

#include "services/ld2410/ld2410.h"
#include "DetWebServerConfig.h"

#if DETWS_ENABLE_LD2410

#include <string.h>

namespace
{
const uint8_t HDR[4] = {0xF4, 0xF3, 0xF2, 0xF1};
const uint8_t FTR[4] = {0xF8, 0xF7, 0xF6, 0xF5};
const uint8_t CMD_HDR[4] = {0xFD, 0xFC, 0xFB, 0xFA};
const uint8_t CMD_FTR[4] = {0x04, 0x03, 0x02, 0x01};

const uint16_t LEN_BASIC = 13;       // payload length for a basic target frame
const uint16_t LEN_ENGINEERING = 35; // ... and for an engineering-mode frame

uint16_t rd16(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

// Build one command frame; returns its length or 0 if @p cap is too small.
size_t cmd_frame(uint8_t *buf, size_t cap, uint16_t word, const uint8_t *val, size_t vlen)
{
    size_t need = 4 + 2 + 2 + vlen + 4;
    if (!buf || cap < need)
        return 0;
    size_t i = 0;
    for (int k = 0; k < 4; k++)
        buf[i++] = CMD_HDR[k];
    uint16_t dl = (uint16_t)(2 + vlen);
    buf[i++] = (uint8_t)(dl & 0xFF);
    buf[i++] = (uint8_t)(dl >> 8);
    buf[i++] = (uint8_t)(word & 0xFF);
    buf[i++] = (uint8_t)(word >> 8);
    for (size_t k = 0; k < vlen; k++)
        buf[i++] = val[k];
    for (int k = 0; k < 4; k++)
        buf[i++] = CMD_FTR[k];
    return i;
}
} // namespace

bool ld2410_parse_report(const uint8_t *f, size_t len, Ld2410Report *out)
{
    if (!f || !out || len < (size_t)(6 + LEN_BASIC + 4))
        return false;
    if (memcmp(f, HDR, 4) != 0)
        return false;
    uint16_t dl = rd16(f + 4);
    if ((size_t)(6 + dl + 4) != len)
        return false; // length field must frame the buffer exactly
    if (memcmp(f + 6 + dl, FTR, 4) != 0)
        return false;

    const uint8_t *p = f + 6;
    Ld2410Report r;
    memset(&r, 0, sizeof(r));
    if (p[0] == 0x02)
    {
        if (dl != LEN_BASIC)
            return false;
        r.engineering = 0;
    }
    else if (p[0] == 0x01)
    {
        if (dl != LEN_ENGINEERING)
            return false;
        r.engineering = 1;
    }
    else
    {
        return false; // unknown data type
    }
    if (p[1] != 0xAA)
        return false; // intra-frame head marker

    r.state = p[2];
    r.moving_cm = rd16(p + 3);
    r.moving_energy = p[5];
    r.static_cm = rd16(p + 6);
    r.static_energy = p[8];
    r.detect_cm = rd16(p + 9);

    if (r.engineering)
    {
        r.max_moving_gate = p[11];
        r.max_static_gate = p[12];
        for (int i = 0; i < LD2410_MAX_GATES; i++)
            r.moving_gate_energy[i] = p[13 + i];
        for (int i = 0; i < LD2410_MAX_GATES; i++)
            r.static_gate_energy[i] = p[22 + i];
        r.light = p[31];
        r.out_pin = p[32];
        if (p[33] != 0x55)
            return false; // tail
    }
    else if (p[11] != 0x55)
    {
        return false; // tail
    }
    *out = r;
    return true;
}

void ld2410_stream_reset(Ld2410Stream *s)
{
    s->pos = 0;
    s->total = 0;
    s->hdr_match = 0;
    s->phase = 0;
}

bool ld2410_stream_push(Ld2410Stream *s, uint8_t b, Ld2410Report *out)
{
    switch (s->phase)
    {
    case 0: // sync on the 4-byte header (bytes are distinct, so resync restarts at most at [0])
        if (b == HDR[s->hdr_match])
        {
            s->buf[s->hdr_match++] = b;
            if (s->hdr_match == 4)
            {
                s->pos = 4;
                s->phase = 1;
            }
        }
        else
        {
            s->hdr_match = (b == HDR[0]) ? 1 : 0;
            if (s->hdr_match)
                s->buf[0] = b;
        }
        return false;
    case 1: // little-endian length field
        s->buf[s->pos++] = b;
        if (s->pos == 6)
        {
            // Widen before comparing: 6 + 0xFFFF + 4 wraps a uint16_t, defeating the guard.
            uint32_t total = 6u + (uint32_t)rd16(s->buf + 4) + 4u;
            if (total > LD2410_FRAME_MAX)
            {
                ld2410_stream_reset(s); // absurd length: drop and resync
                return false;
            }
            s->total = (uint16_t)total;
            s->phase = 2;
        }
        return false;
    default: // body + footer
        s->buf[s->pos++] = b;
        if (s->pos >= s->total)
        {
            bool ok = ld2410_parse_report(s->buf, s->total, out);
            ld2410_stream_reset(s);
            return ok;
        }
        return false;
    }
}

bool ld2410_present(const Ld2410Report *r)
{
    return r && r->state != LD2410_STATE_NONE;
}

uint16_t ld2410_distance_cm(const Ld2410Report *r)
{
    if (!r)
        return 0;
    if (r->state == LD2410_STATE_MOVING || r->state == LD2410_STATE_BOTH)
        return r->moving_cm;
    if (r->state == LD2410_STATE_STATIC)
        return r->static_cm;
    return 0;
}

size_t ld2410_cmd_config_enable(uint8_t *buf, size_t cap)
{
    const uint8_t v[2] = {0x01, 0x00}; // value 0x0001
    return cmd_frame(buf, cap, 0x00FF, v, 2);
}
size_t ld2410_cmd_config_end(uint8_t *buf, size_t cap)
{
    return cmd_frame(buf, cap, 0x00FE, nullptr, 0);
}
size_t ld2410_cmd_engineering(uint8_t *buf, size_t cap, bool on)
{
    return cmd_frame(buf, cap, on ? 0x0062 : 0x0063, nullptr, 0);
}
size_t ld2410_cmd_restart(uint8_t *buf, size_t cap)
{
    return cmd_frame(buf, cap, 0x00A3, nullptr, 0);
}

// ---------------------------------------------------------------------------
// UART binding
// ---------------------------------------------------------------------------

#if defined(ARDUINO)

#include <Arduino.h>

namespace
{
Ld2410Stream s_stream;
Ld2410Report s_last;
bool s_have = false;
} // namespace

bool ld2410_begin(int rx_pin, int tx_pin)
{
    ld2410_stream_reset(&s_stream);
    s_have = false;
    Serial2.begin(DETWS_LD2410_BAUD, SERIAL_8N1, rx_pin, tx_pin);
    return true;
}

bool ld2410_poll()
{
    bool fresh = false;
    while (Serial2.available())
    {
        Ld2410Report r;
        if (ld2410_stream_push(&s_stream, (uint8_t)Serial2.read(), &r))
        {
            s_last = r;
            s_have = true;
            fresh = true;
        }
    }
    return fresh;
}

const Ld2410Report *ld2410_last()
{
    return s_have ? &s_last : nullptr;
}

bool ld2410_set_engineering(bool on)
{
    uint8_t f[16];
    size_t n;
    n = ld2410_cmd_config_enable(f, sizeof(f));
    Serial2.write(f, n);
    n = ld2410_cmd_engineering(f, sizeof(f), on);
    Serial2.write(f, n);
    n = ld2410_cmd_config_end(f, sizeof(f));
    Serial2.write(f, n);
    Serial2.flush();
    return true;
}

bool ld2410_restart()
{
    uint8_t f[16];
    size_t n;
    n = ld2410_cmd_config_enable(f, sizeof(f));
    Serial2.write(f, n);
    n = ld2410_cmd_restart(f, sizeof(f));
    Serial2.write(f, n);
    n = ld2410_cmd_config_end(f, sizeof(f));
    Serial2.write(f, n);
    Serial2.flush();
    return true;
}

#else // host build: no UART. The codec above is host-tested.

bool ld2410_begin(int, int)
{
    return false;
}
bool ld2410_poll()
{
    return false;
}
const Ld2410Report *ld2410_last()
{
    return nullptr;
}
bool ld2410_set_engineering(bool)
{
    return false;
}
bool ld2410_restart()
{
    return false;
}

#endif // ARDUINO

#endif // DETWS_ENABLE_LD2410
