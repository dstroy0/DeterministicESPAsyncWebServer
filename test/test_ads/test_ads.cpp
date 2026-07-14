// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the Beckhoff ADS / AMS codec (services/ads): the request builders and the
// response parsers. Little-endian, target-before-source AMS addressing; layout per the Beckhoff
// InfoSys AMS/ADS specification. Pure host tests.

#include "services/ads/ads.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// A representative AMS route: PLC at 5.18.30.40.1.1 port 851 (first TC3 runtime), local client
// at 192.168.1.100.1.1 port 32768, invoke id 42.
static AdsRequest make_req()
{
    AdsRequest r;
    const uint8_t tgt[ADS_NET_ID_LEN] = {5, 18, 30, 40, 1, 1};
    const uint8_t src[ADS_NET_ID_LEN] = {192, 168, 1, 100, 1, 1};
    memcpy(r.target.net_id, tgt, ADS_NET_ID_LEN);
    r.target.port = 851;
    memcpy(r.source.net_id, src, ADS_NET_ID_LEN);
    r.source.port = 32768;
    r.invoke_id = 42;
    return r;
}

// ADS Read of 4 octets from index group 0x4020 (%M) offset 0 - byte-exact against the spec.
void test_build_read_bytes()
{
    AdsRequest r = make_req();
    uint8_t buf[64];
    size_t n = ads_build_read(buf, sizeof(buf), &r, AdsIndexGroup::plc_rw_m, 0, 4);
    const uint8_t expect[] = {
        0x00, 0x00,                         // AMS/TCP reserved
        0x2C, 0x00, 0x00, 0x00,             // AMS/TCP length = 32 + 12 = 44
        0x05, 0x12, 0x1E, 0x28, 0x01, 0x01, // target net id 5.18.30.40.1.1
        0x53, 0x03,                         // target port 851
        0xC0, 0xA8, 0x01, 0x64, 0x01, 0x01, // source net id 192.168.1.100.1.1
        0x00, 0x80,                         // source port 32768
        0x02, 0x00,                         // cmd id 2 (Read)
        0x04, 0x00,                         // state flags 0x0004 (request)
        0x0C, 0x00, 0x00, 0x00,             // cbData = 12
        0x00, 0x00, 0x00, 0x00,             // error code
        0x2A, 0x00, 0x00, 0x00,             // invoke id 42
        0x20, 0x40, 0x00, 0x00,             // index group 0x4020
        0x00, 0x00, 0x00, 0x00,             // index offset 0
        0x04, 0x00, 0x00, 0x00              // read length 4
    };
    TEST_ASSERT_EQUAL_size_t(sizeof(expect), n);
    TEST_ASSERT_EQUAL_size_t(50, n); // 6 + 32 + 12
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expect, buf, n);
}

// Parse an ADS Read response: header dispatch + payload (Result + Length + Data).
void test_parse_read_response()
{
    const uint8_t resp[] = {
        0x00, 0x00, 0x2C, 0x00, 0x00, 0x00,             // AMS/TCP: len 44
        0xC0, 0xA8, 0x01, 0x64, 0x01, 0x01, 0x00, 0x80, // target (now the client)
        0x05, 0x12, 0x1E, 0x28, 0x01, 0x01, 0x53, 0x03, // source (the PLC)
        0x02, 0x00,                                     // cmd 2 (Read)
        0x05, 0x00,                                     // state flags 0x0005 (reply)
        0x0C, 0x00, 0x00, 0x00,                         // cbData 12
        0x00, 0x00, 0x00, 0x00,                         // AMS error 0
        0x2A, 0x00, 0x00, 0x00,                         // invoke id 42 (echoed)
        0x00, 0x00, 0x00, 0x00,                         // result 0
        0x04, 0x00, 0x00, 0x00,                         // length 4
        0xDE, 0xAD, 0xBE, 0xEF                          // data
    };
    AdsAmsHeader h;
    TEST_ASSERT_TRUE(ads_parse_ams_header(resp, sizeof(resp), &h));
    TEST_ASSERT_TRUE(h.cmd == AdsCommand::read);
    TEST_ASSERT_EQUAL_HEX16(AdsStateFlags::reply, h.state_flags);
    TEST_ASSERT_EQUAL_UINT32(42, h.invoke_id);
    TEST_ASSERT_EQUAL_UINT32(0, h.error_code);
    TEST_ASSERT_EQUAL_UINT32(12, h.data_len);

    AdsReadResult rr;
    TEST_ASSERT_TRUE(ads_parse_read(h.data, h.data_len, &rr));
    TEST_ASSERT_EQUAL_UINT32(0, rr.result);
    TEST_ASSERT_EQUAL_UINT32(4, rr.len);
    TEST_ASSERT_EQUAL_HEX8(0xDE, rr.data[0]);
    TEST_ASSERT_EQUAL_HEX8(0xEF, rr.data[3]);
}

// Write appends the data after the 12-octet index-group/offset/length header.
void test_build_write()
{
    AdsRequest r = make_req();
    uint8_t buf[64];
    const uint8_t val[] = {0x01, 0x00, 0x00, 0x00};
    size_t n = ads_build_write(buf, sizeof(buf), &r, AdsIndexGroup::plc_rw_m, 8, val, sizeof(val));
    TEST_ASSERT_EQUAL_size_t(6 + 32 + 12 + 4, n);
    // cbData at AMS-header offset 20 (buf offset 26) = 16.
    TEST_ASSERT_EQUAL_HEX8(0x10, buf[26]);
    // cmd id = 3 (Write).
    TEST_ASSERT_EQUAL_HEX8(0x03, buf[22]);
    // payload: index group / offset / length / data.
    TEST_ASSERT_EQUAL_HEX8(0x20, buf[ADS_HDR_LEN]);      // 0x4020 LE
    TEST_ASSERT_EQUAL_HEX8(0x08, buf[ADS_HDR_LEN + 4]);  // offset 8
    TEST_ASSERT_EQUAL_HEX8(0x04, buf[ADS_HDR_LEN + 8]);  // length 4
    TEST_ASSERT_EQUAL_HEX8(0x01, buf[ADS_HDR_LEN + 12]); // first data octet
}

// ReadWrite carries a read length AND a write length; used for symbol-by-name (0xF003).
void test_build_read_write_symbol()
{
    AdsRequest r = make_req();
    uint8_t buf[80];
    const char *name = "MAIN.counter"; // 12 chars
    size_t nl = strlen(name);
    size_t n = ads_build_read_write(buf, sizeof(buf), &r, AdsIndexGroup::sym_hnd_by_name, 0, 4, (const uint8_t *)name,
                                    (uint32_t)nl);
    TEST_ASSERT_EQUAL_size_t(6 + 32 + 16 + 12, n);
    // cmd id = 9 (ReadWrite).
    TEST_ASSERT_EQUAL_HEX8(0x09, buf[22]);
    // payload: index group 0xF003, offset 0, read len 4, write len 12, then the name.
    TEST_ASSERT_EQUAL_HEX8(0x03, buf[ADS_HDR_LEN]); // 0xF003 LE low
    TEST_ASSERT_EQUAL_HEX8(0xF0, buf[ADS_HDR_LEN + 1]);
    TEST_ASSERT_EQUAL_HEX8(0x04, buf[ADS_HDR_LEN + 8]);  // read length 4
    TEST_ASSERT_EQUAL_HEX8(0x0C, buf[ADS_HDR_LEN + 12]); // write length 12
    TEST_ASSERT_EQUAL_MEMORY(name, buf + ADS_HDR_LEN + 16, nl);
}

void test_read_state_roundtrip()
{
    AdsRequest r = make_req();
    uint8_t buf[64];
    size_t n = ads_build_read_state(buf, sizeof(buf), &r);
    TEST_ASSERT_EQUAL_size_t(ADS_HDR_LEN, n); // no payload
    TEST_ASSERT_EQUAL_HEX8(0x04, buf[22]);    // cmd 4 (ReadState)
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[26]);    // cbData = 0

    const uint8_t resp[] = {
        0x00, 0x00, 0x28, 0x00, 0x00, 0x00,       // len = 32 + 8 = 40
        0,    0,    0,    0,    0,    0,    0, 0, // target net id + port
        0,    0,    0,    0,    0,    0,    0, 0, // source net id + port
        0x04, 0x00,                               // cmd 4
        0x05, 0x00,                               // reply
        0x08, 0x00, 0x00, 0x00,                   // cbData 8
        0x00, 0x00, 0x00, 0x00,                   // error
        0x2A, 0x00, 0x00, 0x00,                   // invoke
        0x00, 0x00, 0x00, 0x00,                   // result 0
        0x05, 0x00,                               // ADS state = run (5)
        0x00, 0x00                                // device state 0
    };
    AdsAmsHeader h;
    TEST_ASSERT_TRUE(ads_parse_ams_header(resp, sizeof(resp), &h));
    AdsReadStateResult st;
    TEST_ASSERT_TRUE(ads_parse_read_state(h.data, h.data_len, &st));
    TEST_ASSERT_EQUAL_UINT32(0, st.result);
    TEST_ASSERT_EQUAL_UINT16((uint16_t)AdsState::run, st.ads_state);
    TEST_ASSERT_EQUAL_UINT16(0, st.device_state);
}

void test_parse_device_info()
{
    const uint8_t resp[] = {
        0x00, 0x00, 0x38, 0x00, 0x00, 0x00,                                           // len = 32 + 24 = 56
        0,    0,    0,    0,    0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x01, 0x00, // cmd 1 (ReadDeviceInfo)
        0x05, 0x00,                                                                   // reply
        0x18, 0x00, 0x00, 0x00,                                                       // cbData 24
        0x00, 0x00, 0x00, 0x00,                                                       // error
        0x2A, 0x00, 0x00, 0x00,                                                       // invoke
        0x00, 0x00, 0x00, 0x00,                                                       // result 0
        0x03,                                                                         // major 3
        0x01,                                                                         // minor 1
        0xD4, 0x0F,                                                                   // build 4052
        'P',  'l',  'c',  '3',  '0',  0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0              // 16-octet device name "Plc30"
    };
    AdsAmsHeader h;
    TEST_ASSERT_TRUE(ads_parse_ams_header(resp, sizeof(resp), &h));
    AdsDeviceInfo di;
    TEST_ASSERT_TRUE(ads_parse_read_device_info(h.data, h.data_len, &di));
    TEST_ASSERT_EQUAL_UINT32(0, di.result);
    TEST_ASSERT_EQUAL_UINT8(3, di.version_major);
    TEST_ASSERT_EQUAL_UINT8(1, di.version_minor);
    TEST_ASSERT_EQUAL_UINT16(4052, di.version_build);
    TEST_ASSERT_EQUAL_STRING("Plc30", di.device_name);
}

void test_write_control_and_result()
{
    AdsRequest r = make_req();
    uint8_t buf[64];
    size_t n = ads_build_write_control(buf, sizeof(buf), &r, (uint16_t)AdsState::run, 0, nullptr, 0);
    TEST_ASSERT_EQUAL_size_t(ADS_HDR_LEN + 8, n);
    TEST_ASSERT_EQUAL_HEX8(0x05, buf[22]);              // cmd 5 (WriteControl)
    TEST_ASSERT_EQUAL_HEX8(0x08, buf[26]);              // cbData 8
    TEST_ASSERT_EQUAL_HEX8(0x05, buf[ADS_HDR_LEN]);     // ADS state = run
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[ADS_HDR_LEN + 2]); // device state

    const uint8_t data[] = {0x00, 0x00, 0x00, 0x00}; // result 0
    uint32_t res = 0xFFFFFFFF;
    TEST_ASSERT_TRUE(ads_parse_result(data, sizeof(data), &res));
    TEST_ASSERT_EQUAL_UINT32(0, res);
}

void test_add_notification()
{
    AdsRequest r = make_req();
    uint8_t buf[80];
    size_t n = ads_build_add_notification(buf, sizeof(buf), &r, AdsIndexGroup::sym_val_by_handle, 0x1234, 2,
                                          AdsTransMode::server_on_change, 0, 10000);
    TEST_ASSERT_EQUAL_size_t(ADS_HDR_LEN + 40, n);
    TEST_ASSERT_EQUAL_HEX8(0x06, buf[22]);               // cmd 6 (AddNotification)
    TEST_ASSERT_EQUAL_HEX8(0x28, buf[26]);               // cbData 40
    TEST_ASSERT_EQUAL_HEX8(0x02, buf[ADS_HDR_LEN + 8]);  // length 2
    TEST_ASSERT_EQUAL_HEX8(0x04, buf[ADS_HDR_LEN + 12]); // trans mode 4 (on change)

    const uint8_t resp_data[] = {0x00, 0x00, 0x00, 0x00, 0x11, 0x22, 0x33, 0x44}; // result 0 + handle
    uint32_t result = 0xFF, handle = 0;
    TEST_ASSERT_TRUE(ads_parse_add_notification(resp_data, sizeof(resp_data), &result, &handle));
    TEST_ASSERT_EQUAL_UINT32(0, result);
    TEST_ASSERT_EQUAL_HEX32(0x44332211, handle);
}

// DeviceNotification stamp/sample walk: one stamp, one sample (handle 0x1234, 2 octets).
static uint32_t g_notif_count = 0;
static uint32_t g_notif_handle = 0;
static uint32_t g_notif_len = 0;
static uint64_t g_notif_ts = 0;
static uint8_t g_notif_val[8] = {0};
static void on_sample(uint32_t handle, const uint8_t *sample, uint32_t sample_len, uint64_t ts, void *user)
{
    (void)user;
    g_notif_count++;
    g_notif_handle = handle;
    g_notif_len = sample_len;
    g_notif_ts = ts;
    if (sample_len <= sizeof(g_notif_val))
        memcpy(g_notif_val, sample, sample_len);
}

void test_parse_notification_stream()
{
    g_notif_count = 0;
    const uint8_t payload[] = {
        0x1A, 0x00, 0x00, 0x00,                         // Length = 26 (octets after this field)
        0x01, 0x00, 0x00, 0x00,                         // Stamps = 1
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Timestamp = 1
        0x01, 0x00, 0x00, 0x00,                         // Samples = 1
        0x34, 0x12, 0x00, 0x00,                         // NotificationHandle = 0x1234
        0x02, 0x00, 0x00, 0x00,                         // SampleSize = 2
        0x11, 0x22                                      // data
    };
    TEST_ASSERT_TRUE(ads_parse_notification(payload, sizeof(payload), on_sample, nullptr));
    TEST_ASSERT_EQUAL_UINT32(1, g_notif_count);
    TEST_ASSERT_EQUAL_HEX32(0x1234, g_notif_handle);
    TEST_ASSERT_EQUAL_UINT32(2, g_notif_len);
    TEST_ASSERT_EQUAL_UINT64(1, g_notif_ts);
    TEST_ASSERT_EQUAL_HEX8(0x11, g_notif_val[0]);
    TEST_ASSERT_EQUAL_HEX8(0x22, g_notif_val[1]);
}

void test_build_overflow_fails_closed()
{
    AdsRequest r = make_req();
    uint8_t small[40]; // header alone is 38; Read needs 50
    TEST_ASSERT_EQUAL_size_t(0, ads_build_read(small, sizeof(small), &r, 0, 0, 4));
    uint8_t tiny[8];
    TEST_ASSERT_EQUAL_size_t(0, ads_build_read_state(tiny, sizeof(tiny), &r)); // needs 38
}

void test_parse_guards()
{
    AdsAmsHeader h;
    TEST_ASSERT_FALSE(ads_parse_ams_header(nullptr, 64, &h));
    const uint8_t short_buf[10] = {0};
    TEST_ASSERT_FALSE(ads_parse_ams_header(short_buf, sizeof(short_buf), &h)); // < 38

    // AMS/TCP reserved not zero.
    uint8_t badres[ADS_HDR_LEN] = {0};
    badres[0] = 0x01;
    // give it a plausible length so only the reserved check trips
    badres[2] = ADS_AMS_HDR_LEN;
    TEST_ASSERT_FALSE(ads_parse_ams_header(badres, sizeof(badres), &h));

    // AMS/TCP length promises more than the buffer holds.
    uint8_t liar[ADS_HDR_LEN] = {0};
    liar[2] = 0xFF; // frame_len 0x00FF but only 32 octets follow
    TEST_ASSERT_FALSE(ads_parse_ams_header(liar, sizeof(liar), &h));

    // Read payload shorter than its declared data length.
    const uint8_t bad_read[] = {0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0xAA}; // says 16, 1 follows
    AdsReadResult rr;
    TEST_ASSERT_FALSE(ads_parse_read(bad_read, sizeof(bad_read), &rr));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_build_read_bytes);
    RUN_TEST(test_parse_read_response);
    RUN_TEST(test_build_write);
    RUN_TEST(test_build_read_write_symbol);
    RUN_TEST(test_read_state_roundtrip);
    RUN_TEST(test_parse_device_info);
    RUN_TEST(test_write_control_and_result);
    RUN_TEST(test_add_notification);
    RUN_TEST(test_parse_notification_stream);
    RUN_TEST(test_build_overflow_fails_closed);
    RUN_TEST(test_parse_guards);
    return UNITY_END();
}
