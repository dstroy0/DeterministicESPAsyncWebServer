// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the ESP-NOW host-testable core (services/espnow): the typed
// envelope codec (round-trip + rejection of corrupt/mismatched frames) and the
// bounded peer registry. The radio binding is ESP32-only and HW-verified.

#include "services/espnow/espnow.h"
#include <string.h>
#include <unity.h>

void setUp()
{
    dws_espnow_peers_reset();
}
void tearDown()
{
}

void test_encode_decode_roundtrip()
{
    const uint8_t in[] = {1, 2, 3, 4, 5};
    uint8_t frame[64];
    size_t n = dws_espnow_encode(42, in, sizeof(in), frame, sizeof(frame));
    TEST_ASSERT_EQUAL_size_t(sizeof(in) + DWS_ESPNOW_HDR, n);

    uint8_t type = 0;
    const uint8_t *payload = nullptr;
    size_t plen = 0;
    TEST_ASSERT_TRUE(dws_espnow_decode(frame, n, &type, &payload, &plen));
    TEST_ASSERT_EQUAL_UINT8(42, type);
    TEST_ASSERT_EQUAL_size_t(sizeof(in), plen);
    TEST_ASSERT_EQUAL_MEMORY(in, payload, sizeof(in));
}

void test_encode_zero_length()
{
    uint8_t frame[8];
    size_t n = dws_espnow_encode(7, nullptr, 0, frame, sizeof(frame));
    TEST_ASSERT_EQUAL_size_t(DWS_ESPNOW_HDR, n);
    uint8_t type = 0;
    const uint8_t *p = nullptr;
    size_t plen = 9;
    TEST_ASSERT_TRUE(dws_espnow_decode(frame, n, &type, &p, &plen));
    TEST_ASSERT_EQUAL_UINT8(7, type);
    TEST_ASSERT_EQUAL_size_t(0, plen);
}

void test_encode_rejects_oversize_and_small_buffer()
{
    uint8_t big[DWS_ESPNOW_MAX_PAYLOAD + 1] = {0};
    uint8_t frame[300];
    TEST_ASSERT_EQUAL_size_t(0, dws_espnow_encode(1, big, sizeof(big), frame, sizeof(frame)));
    // valid payload but buffer too small
    uint8_t ok[10] = {0};
    TEST_ASSERT_EQUAL_size_t(0, dws_espnow_encode(1, ok, sizeof(ok), frame, 5));
}

void test_decode_rejects_corrupt()
{
    const uint8_t payload[] = {9, 9, 9};
    uint8_t frame[16];
    size_t n = dws_espnow_encode(3, payload, sizeof(payload), frame, sizeof(frame));

    uint8_t type;
    const uint8_t *p;
    size_t plen;
    // bad magic
    uint8_t bad = frame[0];
    frame[0] = 0x00;
    TEST_ASSERT_FALSE(dws_espnow_decode(frame, n, &type, &p, &plen));
    frame[0] = bad;
    // length mismatch (claim more than present)
    frame[2] = 200;
    TEST_ASSERT_FALSE(dws_espnow_decode(frame, n, &type, &p, &plen));
    // too short for a header
    TEST_ASSERT_FALSE(dws_espnow_decode(frame, 2, &type, &p, &plen));
    // trailing garbage (len shorter than buffer)
    frame[2] = (uint8_t)sizeof(payload);
    TEST_ASSERT_FALSE(dws_espnow_decode(frame, n + 1, &type, &p, &plen));
}

void test_peer_registry()
{
    uint8_t a[6] = {0x01, 0, 0, 0, 0, 0xAA};
    uint8_t b[6] = {0x02, 0, 0, 0, 0, 0xBB};
    TEST_ASSERT_EQUAL_INT(0, dws_espnow_peer_count());
    TEST_ASSERT_TRUE(dws_espnow_peer_add(a));
    TEST_ASSERT_TRUE(dws_espnow_peer_add(b));
    TEST_ASSERT_TRUE(dws_espnow_peer_add(a)); // idempotent
    TEST_ASSERT_EQUAL_INT(2, dws_espnow_peer_count());
    TEST_ASSERT_TRUE(dws_espnow_peer_has(a));
    TEST_ASSERT_TRUE(dws_espnow_peer_remove(a));
    TEST_ASSERT_FALSE(dws_espnow_peer_has(a));
    TEST_ASSERT_FALSE(dws_espnow_peer_remove(a)); // already gone
    TEST_ASSERT_EQUAL_INT(1, dws_espnow_peer_count());
}

void test_peer_table_full_fails_closed()
{
    uint8_t mac[6] = {0, 0, 0, 0, 0, 0};
    for (int i = 0; i < DWS_ESPNOW_MAX_PEERS; i++)
    {
        mac[5] = (uint8_t)i;
        TEST_ASSERT_TRUE(dws_espnow_peer_add(mac));
    }
    mac[5] = 0xFF; // one too many
    TEST_ASSERT_FALSE(dws_espnow_peer_add(mac));
    TEST_ASSERT_EQUAL_INT(DWS_ESPNOW_MAX_PEERS, dws_espnow_peer_count());
}

void test_broadcast_address()
{
    for (int i = 0; i < 6; i++)
        TEST_ASSERT_EQUAL_UINT8(0xFF, DWS_ESPNOW_BROADCAST[i]);
}

void test_peer_guard_and_host_stubs()
{
    dws_espnow_peers_reset();
    TEST_ASSERT_FALSE(dws_espnow_peer_add(nullptr)); // null mac fails closed
    // Host build: the ESP-NOW bind functions are unavailable.
    TEST_ASSERT_FALSE(dws_espnow_begin(1, nullptr));
    uint8_t mac[6] = {1, 2, 3, 4, 5, 6};
    dws_espnow_add_peer(mac); // delegates to the pure peer registry
    uint8_t payload[2] = {0xAA, 0xBB};
    TEST_ASSERT_FALSE(dws_espnow_send(mac, 1, payload, sizeof(payload)));
    TEST_ASSERT_FALSE(dws_espnow_broadcast(1, payload, sizeof(payload)));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_encode_decode_roundtrip);
    RUN_TEST(test_encode_zero_length);
    RUN_TEST(test_encode_rejects_oversize_and_small_buffer);
    RUN_TEST(test_decode_rejects_corrupt);
    RUN_TEST(test_peer_registry);
    RUN_TEST(test_peer_table_full_fails_closed);
    RUN_TEST(test_broadcast_address);
    RUN_TEST(test_peer_guard_and_host_stubs);
    return UNITY_END();
}
