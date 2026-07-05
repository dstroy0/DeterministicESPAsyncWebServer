// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Integration test for SSH server-to-client compression WIRING (network_drivers/presentation/ssh):
// the compression owner (ssh_comp) + its activation + the packet-layer compress path in
// ssh_pkt_send. It drives the packet layer directly (before NEWKEYS, so no cipher) with compression
// activated, then reconstructs the continuous zlib stream from the framed packets and decodes it via
// inflate_raw (validated vs Python zlib in test_inflate) - proving payloads are compressed on the
// wire and the whole session forms one valid, context-takeover zlib stream.

#include "network_drivers/presentation/inflate/inflate.h"
#include "network_drivers/presentation/ssh/transport/ssh_comp.h"
#include "network_drivers/presentation/ssh/transport/ssh_packet.h"
#include <string.h>
#include <unity.h>

static uint8_t g_stream[64 * 1024]; // every packet's compressed payload, concatenated
static size_t g_stream_len;
static uint8_t g_orig[64 * 1024]; // every original payload, concatenated
static size_t g_orig_len;
static uint8_t g_decoded[64 * 1024];
static uint8_t g_iscratch[INFLATE_SCRATCH_SIZE];

// Send one payload through ssh_pkt_send (unencrypted framing so the payload is visible), then pull
// the on-wire (compressed) payload out of the packet and accumulate it + the original.
static void send_and_capture(const uint8_t *payload, size_t n)
{
    uint8_t wire[SSH_WIRE_CAP];
    size_t wlen = 0;
    int rc = ssh_pkt_send(0, payload, n, wire, &wlen, sizeof(wire));
    TEST_ASSERT_EQUAL_INT(0, rc);

    // Unencrypted packet: [packet_length(4)] [padding_length(1)] [payload...] [padding].
    size_t pkt_len = ((size_t)wire[0] << 24) | ((size_t)wire[1] << 16) | ((size_t)wire[2] << 8) | wire[3];
    uint8_t pad_len = wire[4];
    TEST_ASSERT_TRUE(pkt_len >= (size_t)(1 + pad_len));
    size_t cpayload_len = pkt_len - 1 - pad_len;

    TEST_ASSERT_TRUE(g_stream_len + cpayload_len <= sizeof(g_stream));
    memcpy(g_stream + g_stream_len, wire + 5, cpayload_len);
    g_stream_len += cpayload_len;
    TEST_ASSERT_TRUE(g_orig_len + n <= sizeof(g_orig));
    memcpy(g_orig + g_orig_len, payload, n);
    g_orig_len += n;
}

// Decode the whole captured stream (minus the 2-byte zlib header) and compare to the originals.
static void verify()
{
    TEST_ASSERT_TRUE(g_stream_len >= 2);
    TEST_ASSERT_EQUAL_HEX8(0x78, g_stream[0]);
    TEST_ASSERT_EQUAL_HEX8(0x9C, g_stream[1]);
    size_t dlen = 0;
    int rc = inflate_raw(g_stream + 2, g_stream_len - 2, g_decoded, sizeof(g_decoded), &dlen, g_iscratch,
                         sizeof(g_iscratch));
    TEST_ASSERT_EQUAL_INT(INFLATE_OK, rc);
    TEST_ASSERT_EQUAL_size_t(g_orig_len, dlen);
    TEST_ASSERT_EQUAL_MEMORY(g_orig, g_decoded, g_orig_len);
}

void setUp()
{
    ssh_pkt_init(0);
    ssh_comp_reset(0);
    g_stream_len = 0;
    g_orig_len = 0;
}
void tearDown()
{
}

// zlib@openssh.com is delayed: not active until USERAUTH_SUCCESS, then active for every packet.
void test_delayed_activation()
{
    ssh_comp_set_s2c(0, SSH_COMP_ZLIB_DELAYED);
    TEST_ASSERT_FALSE(ssh_comp_s2c_active(0)); // negotiated but not started
    ssh_comp_on_newkeys(0);                    // must NOT start a delayed stream
    TEST_ASSERT_FALSE(ssh_comp_s2c_active(0));
    ssh_comp_on_auth_success(0); // now it starts
    TEST_ASSERT_TRUE(ssh_comp_s2c_active(0));
}

// zlib (non-delayed) starts right at NEWKEYS.
void test_immediate_activation()
{
    ssh_comp_set_s2c(0, SSH_COMP_ZLIB);
    TEST_ASSERT_FALSE(ssh_comp_s2c_active(0));
    ssh_comp_on_newkeys(0);
    TEST_ASSERT_TRUE(ssh_comp_s2c_active(0));
}

// "none": never activates, whatever the events.
void test_none_never_activates()
{
    ssh_comp_set_s2c(0, SSH_COMP_NONE);
    ssh_comp_on_newkeys(0);
    ssh_comp_on_auth_success(0);
    TEST_ASSERT_FALSE(ssh_comp_s2c_active(0));
}

// The full path: activate, push a realistic terminal session, and prove the framed packets form one
// valid context-takeover zlib stream that decodes back to the originals.
void test_packet_layer_stream_roundtrip()
{
    ssh_comp_set_s2c(0, SSH_COMP_ZLIB_DELAYED);
    ssh_comp_on_auth_success(0);
    TEST_ASSERT_TRUE(ssh_comp_s2c_active(0));

    const char *prompt = "user@esp32:~$ ";
    send_and_capture((const uint8_t *)prompt, strlen(prompt));
    const char *cmd = "uname -a\r\n";
    send_and_capture((const uint8_t *)cmd, strlen(cmd));
    send_and_capture((const uint8_t *)prompt, strlen(prompt)); // repeat -> cross-packet match
    const char *banner = "DeterministicESPAsyncWebServer 1.0 (xtensa)\r\n";
    send_and_capture((const uint8_t *)banner, strlen(banner));
    send_and_capture((const uint8_t *)"", 0); // empty write
    verify();
}

// A longer session that slides the window, driven through the packet layer.
void test_packet_layer_window_slide()
{
    ssh_comp_set_s2c(0, SSH_COMP_ZLIB_DELAYED);
    ssh_comp_on_auth_success(0);
    uint8_t buf[1000];
    for (int k = 0; k < 30; k++)
    {
        for (int j = 0; j < (int)sizeof(buf); j++)
            buf[j] = (uint8_t)("log: event fired at t="[j % 22]) ^ (uint8_t)((j & 3) ? 0 : k);
        send_and_capture(buf, sizeof(buf));
    }
    verify();
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_delayed_activation);
    RUN_TEST(test_immediate_activation);
    RUN_TEST(test_none_never_activates);
    RUN_TEST(test_packet_layer_stream_roundtrip);
    RUN_TEST(test_packet_layer_window_slide);
    return UNITY_END();
}
