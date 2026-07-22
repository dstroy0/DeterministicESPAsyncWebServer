// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file fanuc_j519.cpp
 * @brief FANUC Stream Motion (J519) UDP codec - builders + parsers (see fanuc_j519.h).
 *
 * Straight-line field packing at fixed offsets: every packet is a constant size, so each builder
 * checks @p cap once up front and each parser requires the exact length. All integers little-endian
 * through shared_primitives/endian.h; floats are IEEE-754 binary32 moved through a uint32_t with
 * memcpy (no type punning, no strict-aliasing UB) - the same approach as the OPC UA Variant codec.
 */

#include "services/fanuc_j519/fanuc_j519.h"

#if DWS_ENABLE_FANUC_J519

#include "shared_primitives/endian.h"
#include <string.h> // memcpy (string.h is allowed; the no-stdlib rule is about stdlib.h/malloc)

namespace
{
// --- float <-> little-endian binary32 --------------------------------------------------------
size_t wr_f32le(uint8_t *p, float v)
{
    uint32_t u;
    memcpy(&u, &v, 4);
    return dws_wr32le(p, u);
}
float rd_f32le(const uint8_t *p)
{
    uint32_t u = dws_rd32le(p);
    float v;
    memcpy(&v, &u, 4);
    return v;
}

// Pack/unpack a run of @p n consecutive binary32 at @p p.
void wr_f32_block(uint8_t *p, const float *src, size_t n)
{
    for (size_t k = 0; k < n; k++)
        wr_f32le(p + 4 * k, src[k]);
}
void rd_f32_block(const uint8_t *p, float *dst, size_t n)
{
    for (size_t k = 0; k < n; k++)
        dst[k] = rd_f32le(p + 4 * k);
}

// Every packet opens with { type, version } - written once here so no builder repeats it.
void wr_header(uint8_t *buf, J519Type type, uint32_t version_no)
{
    dws_wr32le(buf + 0, (uint32_t)type);
    dws_wr32le(buf + 4, version_no);
}

// A parser accepts only its own exact length and type code (the type space is shared per direction,
// so length is what actually separates Start from Status and Request from Ack).
bool hdr_ok(const uint8_t *buf, size_t len, size_t want_len, J519Type want_type)
{
    return buf && len == want_len && dws_rd32le(buf + 0) == (uint32_t)want_type;
}
} // namespace

// --- header ---------------------------------------------------------------------------------------

bool dws_j519_peek(const uint8_t *buf, size_t len, uint32_t *type, uint32_t *version_no)
{
    if (!buf || len < 8)
        return false;
    if (type)
        *type = dws_rd32le(buf + 0);
    if (version_no)
        *version_no = dws_rd32le(buf + 4);
    return true;
}

// --- PC -> robot: build ---------------------------------------------------------------------------

size_t dws_j519_build_start(uint8_t *buf, size_t cap, uint32_t version_no)
{
    if (!buf || cap < DWS_J519_LEN_START)
        return 0;
    wr_header(buf, J519Type::J519_START_OR_STATUS, version_no);
    return DWS_J519_LEN_START;
}

size_t dws_j519_build_stop(uint8_t *buf, size_t cap, uint32_t version_no)
{
    if (!buf || cap < DWS_J519_LEN_STOP)
        return 0;
    wr_header(buf, J519Type::J519_STOP, version_no);
    return DWS_J519_LEN_STOP;
}

size_t dws_j519_build_motion(uint8_t *buf, size_t cap, const J519MotionCommand *cmd)
{
    if (!buf || !cmd || cap < DWS_J519_LEN_MOTION)
        return 0;
    wr_header(buf, J519Type::J519_MOTION, cmd->version_no);
    dws_wr32le(buf + 8, cmd->sequence_no);
    buf[12] = cmd->last_data;
    buf[13] = cmd->read_io_type;
    dws_wr16le(buf + 14, cmd->read_io_index);
    dws_wr16le(buf + 16, cmd->read_io_mask);
    buf[18] = cmd->data_style;
    buf[19] = cmd->write_io_type;
    dws_wr16le(buf + 20, cmd->write_io_index);
    dws_wr16le(buf + 22, cmd->write_io_mask);
    dws_wr16le(buf + 24, cmd->write_io_value);
    dws_wr16le(buf + 26, 0); // unused
    wr_f32_block(buf + 28, cmd->joint_data, DWS_J519_AXES);
    return DWS_J519_LEN_MOTION;
}

size_t dws_j519_build_request(uint8_t *buf, size_t cap, const J519Request *req)
{
    if (!buf || !req || cap < DWS_J519_LEN_REQUEST)
        return 0;
    wr_header(buf, J519Type::J519_REQUEST_OR_ACK, req->version_no);
    dws_wr32le(buf + 8, req->axis_no);
    dws_wr32le(buf + 12, req->threshold_type);
    return DWS_J519_LEN_REQUEST;
}

// --- PC -> robot: parse ---------------------------------------------------------------------------

bool dws_j519_parse_motion(const uint8_t *buf, size_t len, J519MotionCommand *out)
{
    if (!out || !hdr_ok(buf, len, DWS_J519_LEN_MOTION, J519Type::J519_MOTION))
        return false;
    out->version_no = dws_rd32le(buf + 4);
    out->sequence_no = dws_rd32le(buf + 8);
    out->last_data = buf[12];
    out->read_io_type = buf[13];
    out->read_io_index = dws_rd16le(buf + 14);
    out->read_io_mask = dws_rd16le(buf + 16);
    out->data_style = buf[18];
    out->write_io_type = buf[19];
    out->write_io_index = dws_rd16le(buf + 20);
    out->write_io_mask = dws_rd16le(buf + 22);
    out->write_io_value = dws_rd16le(buf + 24);
    // octets 26..27 are unused padding - not surfaced
    rd_f32_block(buf + 28, out->joint_data, DWS_J519_AXES);
    return true;
}

bool dws_j519_parse_request(const uint8_t *buf, size_t len, J519Request *out)
{
    if (!out || !hdr_ok(buf, len, DWS_J519_LEN_REQUEST, J519Type::J519_REQUEST_OR_ACK))
        return false;
    out->version_no = dws_rd32le(buf + 4);
    out->axis_no = dws_rd32le(buf + 8);
    out->threshold_type = dws_rd32le(buf + 12);
    return true;
}

// --- robot -> PC: build ---------------------------------------------------------------------------

size_t dws_j519_build_status(uint8_t *buf, size_t cap, const J519RobotStatus *st)
{
    if (!buf || !st || cap < DWS_J519_LEN_STATUS)
        return 0;
    wr_header(buf, J519Type::J519_START_OR_STATUS, st->version_no);
    dws_wr32le(buf + 8, st->sequence_no);
    buf[12] = st->status;
    buf[13] = st->read_io_type;
    dws_wr16le(buf + 14, st->read_io_index);
    dws_wr16le(buf + 16, st->read_io_mask);
    dws_wr16le(buf + 18, st->read_io_value);
    dws_wr32le(buf + 20, st->time_stamp);
    wr_f32_block(buf + 24, st->cartesian_pose, DWS_J519_AXES);
    wr_f32_block(buf + 60, st->joint_pose, DWS_J519_AXES);
    wr_f32_block(buf + 96, st->motor_current, DWS_J519_AXES);
    return DWS_J519_LEN_STATUS;
}

size_t dws_j519_build_ack(uint8_t *buf, size_t cap, const J519Ack *ack)
{
    if (!buf || !ack || cap < DWS_J519_LEN_ACK)
        return 0;
    wr_header(buf, J519Type::J519_REQUEST_OR_ACK, ack->version_no);
    dws_wr32le(buf + 8, ack->axis_no);
    dws_wr32le(buf + 12, ack->threshold_type);
    dws_wr32le(buf + 16, ack->max_cart_speed);
    dws_wr32le(buf + 20, ack->unknown0);
    wr_f32_block(buf + 24, ack->threshold_no_load, DWS_J519_THRESHOLDS);
    wr_f32_block(buf + 104, ack->threshold_max_load, DWS_J519_THRESHOLDS);
    return DWS_J519_LEN_ACK;
}

// --- robot -> PC: parse ---------------------------------------------------------------------------

bool dws_j519_parse_status(const uint8_t *buf, size_t len, J519RobotStatus *out)
{
    if (!out || !hdr_ok(buf, len, DWS_J519_LEN_STATUS, J519Type::J519_START_OR_STATUS))
        return false;
    out->version_no = dws_rd32le(buf + 4);
    out->sequence_no = dws_rd32le(buf + 8);
    out->status = buf[12];
    out->read_io_type = buf[13];
    out->read_io_index = dws_rd16le(buf + 14);
    out->read_io_mask = dws_rd16le(buf + 16);
    out->read_io_value = dws_rd16le(buf + 18);
    out->time_stamp = dws_rd32le(buf + 20);
    rd_f32_block(buf + 24, out->cartesian_pose, DWS_J519_AXES);
    rd_f32_block(buf + 60, out->joint_pose, DWS_J519_AXES);
    rd_f32_block(buf + 96, out->motor_current, DWS_J519_AXES);
    return true;
}

bool dws_j519_parse_ack(const uint8_t *buf, size_t len, J519Ack *out)
{
    if (!out || !hdr_ok(buf, len, DWS_J519_LEN_ACK, J519Type::J519_REQUEST_OR_ACK))
        return false;
    out->version_no = dws_rd32le(buf + 4);
    out->axis_no = dws_rd32le(buf + 8);
    out->threshold_type = dws_rd32le(buf + 12);
    out->max_cart_speed = dws_rd32le(buf + 16);
    out->unknown0 = dws_rd32le(buf + 20);
    rd_f32_block(buf + 24, out->threshold_no_load, DWS_J519_THRESHOLDS);
    rd_f32_block(buf + 104, out->threshold_max_load, DWS_J519_THRESHOLDS);
    return true;
}

#endif // DWS_ENABLE_FANUC_J519
