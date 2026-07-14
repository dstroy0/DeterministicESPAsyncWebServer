// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ads.cpp
 * @brief Beckhoff ADS / AMS builder + parser (pure, host-tested). All fields little-endian.
 */

#include "services/ads/ads.h"

#if DETWS_ENABLE_ADS

#include <string.h>

// AMS is little-endian throughout.
static size_t put16le(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)(v >> 8);
    return 2;
}

static size_t put32le(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
    p[2] = (uint8_t)((v >> 16) & 0xFF);
    p[3] = (uint8_t)((v >> 24) & 0xFF);
    return 4;
}

static uint16_t get16le(const uint8_t *p)
{
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t get32le(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static uint64_t get64le(const uint8_t *p)
{
    return (uint64_t)get32le(p) | ((uint64_t)get32le(p + 4) << 32);
}

// Write the AMS/TCP header (with the final total length) + the 32-octet AMS header. The payload
// is appended by the caller; `payload_len` is cbData. Returns ADS_HDR_LEN, or 0 if too small.
static size_t write_header(uint8_t *buf, size_t cap, const AdsRequest *r, AdsCommand cmd, uint32_t payload_len)
{
    if (!buf || !r || cap < (size_t)ADS_HDR_LEN + payload_len)
        return 0;
    size_t p = 0;
    // AMS/TCP header: reserved(2) + length(4). length covers the AMS header + payload.
    buf[p++] = 0x00;
    buf[p++] = 0x00;
    p += put32le(buf + p, (uint32_t)ADS_AMS_HDR_LEN + payload_len);
    // AMS header.
    memcpy(buf + p, r->target.net_id, ADS_NET_ID_LEN);
    p += ADS_NET_ID_LEN;
    p += put16le(buf + p, r->target.port);
    memcpy(buf + p, r->source.net_id, ADS_NET_ID_LEN);
    p += ADS_NET_ID_LEN;
    p += put16le(buf + p, r->source.port);
    p += put16le(buf + p, (uint16_t)cmd); // wire byte in
    p += put16le(buf + p, AdsStateFlags::request);
    p += put32le(buf + p, payload_len); // cbData
    p += put32le(buf + p, 0);           // error code (0 on a request)
    p += put32le(buf + p, r->invoke_id);
    return p; // == ADS_HDR_LEN
}

size_t ads_build_read_device_info(uint8_t *buf, size_t cap, const AdsRequest *r)
{
    return write_header(buf, cap, r, AdsCommand::read_device_info, 0);
}

size_t ads_build_read_state(uint8_t *buf, size_t cap, const AdsRequest *r)
{
    return write_header(buf, cap, r, AdsCommand::read_state, 0);
}

size_t ads_build_read(uint8_t *buf, size_t cap, const AdsRequest *r, uint32_t index_group, uint32_t index_offset,
                      uint32_t read_len)
{
    size_t p = write_header(buf, cap, r, AdsCommand::read, 12);
    if (!p)
        return 0;
    p += put32le(buf + p, index_group);
    p += put32le(buf + p, index_offset);
    p += put32le(buf + p, read_len);
    return p;
}

size_t ads_build_write(uint8_t *buf, size_t cap, const AdsRequest *r, uint32_t index_group, uint32_t index_offset,
                       const uint8_t *data, uint32_t len)
{
    if (len && !data)
        return 0;
    size_t p = write_header(buf, cap, r, AdsCommand::write, 12 + len);
    if (!p)
        return 0;
    p += put32le(buf + p, index_group);
    p += put32le(buf + p, index_offset);
    p += put32le(buf + p, len);
    if (len)
    {
        memcpy(buf + p, data, len);
        p += len;
    }
    return p;
}

size_t ads_build_read_write(uint8_t *buf, size_t cap, const AdsRequest *r, uint32_t index_group, uint32_t index_offset,
                            uint32_t read_len, const uint8_t *write_data, uint32_t write_len)
{
    if (write_len && !write_data)
        return 0;
    size_t p = write_header(buf, cap, r, AdsCommand::read_write, 16 + write_len);
    if (!p)
        return 0;
    p += put32le(buf + p, index_group);
    p += put32le(buf + p, index_offset);
    p += put32le(buf + p, read_len);
    p += put32le(buf + p, write_len);
    if (write_len)
    {
        memcpy(buf + p, write_data, write_len);
        p += write_len;
    }
    return p;
}

size_t ads_build_write_control(uint8_t *buf, size_t cap, const AdsRequest *r, uint16_t ads_state, uint16_t device_state,
                               const uint8_t *data, uint32_t len)
{
    if (len && !data)
        return 0;
    size_t p = write_header(buf, cap, r, AdsCommand::write_control, 8 + len);
    if (!p)
        return 0;
    p += put16le(buf + p, ads_state);
    p += put16le(buf + p, device_state);
    p += put32le(buf + p, len);
    if (len)
    {
        memcpy(buf + p, data, len);
        p += len;
    }
    return p;
}

size_t ads_build_add_notification(uint8_t *buf, size_t cap, const AdsRequest *r, uint32_t index_group,
                                  uint32_t index_offset, uint32_t length, AdsTransMode mode, uint32_t max_delay,
                                  uint32_t cycle_time)
{
    // IndexGroup + IndexOffset + Length + TransMode + MaxDelay + CycleTime + Reserved(16) = 40.
    size_t p = write_header(buf, cap, r, AdsCommand::add_notification, 40);
    if (!p)
        return 0;
    p += put32le(buf + p, index_group);
    p += put32le(buf + p, index_offset);
    p += put32le(buf + p, length);
    p += put32le(buf + p, (uint32_t)mode); // wire byte in
    p += put32le(buf + p, max_delay);
    p += put32le(buf + p, cycle_time);
    memset(buf + p, 0, 16); // reserved
    p += 16;
    return p;
}

size_t ads_build_del_notification(uint8_t *buf, size_t cap, const AdsRequest *r, uint32_t notification_handle)
{
    size_t p = write_header(buf, cap, r, AdsCommand::del_notification, 4);
    if (!p)
        return 0;
    p += put32le(buf + p, notification_handle);
    return p;
}

bool ads_parse_ams_header(const uint8_t *buf, size_t len, AdsAmsHeader *out)
{
    if (!buf || !out || len < (size_t)ADS_HDR_LEN)
        return false;
    if (buf[0] != 0x00 || buf[1] != 0x00) // AMS/TCP reserved
        return false;
    uint32_t frame_len = get32le(buf + 2); // AMS header + payload
    if (frame_len < (uint32_t)ADS_AMS_HDR_LEN)
        return false;
    if ((size_t)ADS_AMSTCP_HDR_LEN + frame_len > len)
        return false;
    const uint8_t *a = buf + ADS_AMSTCP_HDR_LEN;
    memcpy(out->target.net_id, a, ADS_NET_ID_LEN);
    out->target.port = get16le(a + 6);
    memcpy(out->source.net_id, a + 8, ADS_NET_ID_LEN);
    out->source.port = get16le(a + 14);
    out->cmd = (AdsCommand)get16le(a + 16); // wire byte out
    out->state_flags = get16le(a + 18);
    out->data_len = get32le(a + 20);
    out->error_code = get32le(a + 24);
    out->invoke_id = get32le(a + 28);
    // cbData must fit inside the frame the AMS/TCP length promised.
    if ((uint32_t)ADS_AMS_HDR_LEN + out->data_len > frame_len)
        return false;
    out->data = a + ADS_AMS_HDR_LEN;
    return true;
}

bool ads_parse_read(const uint8_t *data, size_t data_len, AdsReadResult *out)
{
    if (!data || !out || data_len < 8)
        return false;
    out->result = get32le(data);
    out->len = get32le(data + 4);
    if (8 + (size_t)out->len > data_len)
        return false;
    out->data = data + 8;
    return true;
}

bool ads_parse_result(const uint8_t *data, size_t data_len, uint32_t *result)
{
    if (!data || !result || data_len < 4)
        return false;
    *result = get32le(data);
    return true;
}

bool ads_parse_read_state(const uint8_t *data, size_t data_len, AdsReadStateResult *out)
{
    if (!data || !out || data_len < 8)
        return false;
    out->result = get32le(data);
    out->ads_state = get16le(data + 4);
    out->device_state = get16le(data + 6);
    return true;
}

bool ads_parse_read_device_info(const uint8_t *data, size_t data_len, AdsDeviceInfo *out)
{
    if (!data || !out || data_len < 4 + 4 + ADS_DEVICE_NAME_LEN)
        return false;
    out->result = get32le(data);
    out->version_major = data[4];
    out->version_minor = data[5];
    out->version_build = get16le(data + 6);
    memcpy(out->device_name, data + 8, ADS_DEVICE_NAME_LEN);
    out->device_name[ADS_DEVICE_NAME_LEN] = '\0'; // the field is not guaranteed NUL-terminated
    return true;
}

bool ads_parse_add_notification(const uint8_t *data, size_t data_len, uint32_t *result, uint32_t *handle)
{
    if (!data || !result || !handle || data_len < 8)
        return false;
    *result = get32le(data);
    *handle = get32le(data + 4);
    return true;
}

bool ads_parse_notification(const uint8_t *data, size_t data_len, AdsNotificationSampleFn on_sample, void *user)
{
    // Length(4) + Stamps(4), then per stamp: Timestamp(8) + Samples(4) + samples.
    if (!data || !on_sample || data_len < 8)
        return false;
    uint32_t length = get32le(data); // octets after this field
    uint32_t stamps = get32le(data + 4);
    if (4 + (size_t)length > data_len)
        return false;
    size_t p = 8;
    for (uint32_t s = 0; s < stamps; s++)
    {
        if (p + 12 > data_len) // timestamp(8) + samples(4)
            return false;
        uint64_t timestamp = get64le(data + p);
        uint32_t samples = get32le(data + p + 8);
        p += 12;
        for (uint32_t i = 0; i < samples; i++)
        {
            if (p + 8 > data_len) // handle(4) + size(4)
                return false;
            uint32_t handle = get32le(data + p);
            uint32_t size = get32le(data + p + 4);
            p += 8;
            if (p + (size_t)size > data_len)
                return false;
            on_sample(handle, data + p, size, timestamp, user);
            p += size;
        }
    }
    return true;
}

#endif // DETWS_ENABLE_ADS
