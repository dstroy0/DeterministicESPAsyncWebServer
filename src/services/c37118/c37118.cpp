// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file c37118.cpp
 * @brief IEEE C37.118.2 synchrophasor frame builder + parser (pure, host-tested).
 */

#include "services/c37118/c37118.h"

#if DWS_ENABLE_C37118

#include <string.h>

#include "shared_primitives/endian.h"

uint16_t c37118_crc(const uint8_t *data, size_t len)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++)
    {
        crc ^= (uint16_t)data[i] << 8;
        for (int b = 0; b < 8; b++)
            crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
    }
    return crc;
}

size_t c37118_build_frame(uint8_t *buf, size_t cap, uint8_t type, uint8_t version, uint16_t idcode, uint32_t soc,
                          uint32_t fracsec, const uint8_t *payload, size_t payload_len)
{
    if (!buf || (payload_len && !payload))
        return 0;
    size_t total = C37118_MIN_FRAME + payload_len; // 14 header + payload + 2 CHK
    if (total > 0xFFFF || total > cap)
        return 0;
    size_t p = 0;
    buf[p++] = C37118_SYNC_LEADER;
    buf[p++] = (uint8_t)(((type & C37118_TYPE_MASK) << C37118_TYPE_SHIFT) | (version & C37118_VERSION_MASK));
    p += dws_wr16be(buf + p, (uint16_t)total);
    p += dws_wr16be(buf + p, idcode);
    p += dws_wr32be(buf + p, soc);
    p += dws_wr32be(buf + p, fracsec);
    if (payload_len)
    {
        memcpy(buf + p, payload, payload_len);
        p += payload_len;
    }
    uint16_t crc = c37118_crc(buf, p); // over everything before CHK
    p += dws_wr16be(buf + p, crc);
    return p;
}

size_t c37118_build_command(uint8_t *buf, size_t cap, uint16_t idcode, uint32_t soc, uint32_t fracsec, uint16_t cmd)
{
    uint8_t payload[2];
    dws_wr16be(payload, cmd);
    return c37118_build_frame(buf, cap, C37118_TYPE_CMD, C37118_VERSION_2011, idcode, soc, fracsec, payload, 2);
}

bool c37118_parse_frame(const uint8_t *buf, size_t len, C37118Frame *out)
{
    if (!buf || !out || len < C37118_MIN_FRAME)
        return false;
    if (buf[0] != C37118_SYNC_LEADER)
        return false;
    uint16_t framesize = dws_rd16be(buf + 2);
    if (framesize < C37118_MIN_FRAME || framesize > len)
        return false; // out of range / not fully buffered
    uint16_t want = c37118_crc(buf, (size_t)framesize - 2);
    uint16_t got = dws_rd16be(buf + framesize - 2);
    if (want != got)
        return false; // CHK mismatch
    out->type = (uint8_t)((buf[1] >> C37118_TYPE_SHIFT) & C37118_TYPE_MASK);
    out->version = (uint8_t)(buf[1] & C37118_VERSION_MASK);
    out->framesize = framesize;
    out->idcode = dws_rd16be(buf + 4);
    out->soc = dws_rd32be(buf + 6);
    out->fracsec = dws_rd32be(buf + 10);
    out->data = buf + 14;
    out->data_len = (size_t)framesize - C37118_MIN_FRAME;
    return true;
}

bool c37118_parse_command(const C37118Frame *f, uint16_t *cmd)
{
    if (!f || f->type != C37118_TYPE_CMD || f->data_len < 2)
        return false;
    if (cmd)
        *cmd = dws_rd16be(f->data);
    return true;
}

#endif // DWS_ENABLE_C37118
