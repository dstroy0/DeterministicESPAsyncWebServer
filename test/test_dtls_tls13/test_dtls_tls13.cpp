// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// TLS 1.3 messages the DTLS 1.3 handshake adds to tls13_msg (RFC 8446 §4.1.4 / §4.4.1): the
// HelloRetryRequest builder, the cookie extension parse, the empty EncryptedExtensions, and the
// message_hash transcript wrapper. Compiled with DETWS_ENABLE_DTLS (not HTTP/3) to prove the module
// builds and works for the DTLS path. The HelloRetryRequest is pinned byte-for-byte, anchored on the
// RFC 8446 §4.1.3 magic random.

#include "network_drivers/presentation/http3/tls13_msg.h"
#include <stdint.h>
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// The HelloRetryRequest magic random is SHA-256("HelloRetryRequest") (RFC 8446 §4.1.3).
static const uint8_t HRR_MAGIC[32] = {0xCF, 0x21, 0xAD, 0x74, 0xE5, 0x9A, 0x61, 0x11, 0xBE, 0x1D, 0x8C,
                                      0x02, 0x1E, 0x65, 0xB8, 0x91, 0xC2, 0xA2, 0x11, 0x16, 0x7A, 0xBB,
                                      0x8C, 0x5E, 0x07, 0x9E, 0x09, 0xE2, 0xC8, 0xA8, 0x33, 0x9C};

// The whole DTLS 1.3 HelloRetryRequest for (session_id="", group=X25519, cookie=AA BB CC DD),
// byte-for-byte. DTLS carries the 0xFEFD / 0xFEFC version codepoints (RFC 9147 §5.3), not 0x0303 / 0x0304.
static const uint8_t HRR_WIRE[66] = {0x02, 0x00, 0x00, 0x3e,                         // server_hello, length 62
                                     0xFE, 0xFD,                                     // legacy_version (DTLS 1.2)
                                     0xCF, 0x21, 0xAD, 0x74, 0xE5, 0x9A, 0x61, 0x11, // \_ HRR magic random ...
                                     0xBE, 0x1D, 0x8C, 0x02, 0x1E, 0x65, 0xB8, 0x91, //
                                     0xC2, 0xA2, 0x11, 0x16, 0x7A, 0xBB, 0x8C, 0x5E, //
                                     0x07, 0x9E, 0x09, 0xE2, 0xC8, 0xA8, 0x33, 0x9C, // ... /
                                     0x00,                                           // legacy_session_id_echo: empty
                                     0x13, 0x01,                         // cipher_suite TLS_AES_128_GCM_SHA256
                                     0x00,                               // legacy_compression_method
                                     0x00, 0x16,                         // extensions length 22
                                     0x00, 0x2b, 0x00, 0x02, 0xFE, 0xFC, // supported_versions -> DTLS 1.3 (0xFEFC)
                                     0x00, 0x33, 0x00, 0x02, 0x00, 0x1d, // key_share (HRR) -> selected_group X25519
                                     0x00, 0x2c, 0x00, 0x06, 0x00, 0x04, 0xAA, 0xBB,
                                     0xCC, 0xDD}; // cookie -> AA BB CC DD

static void test_hrr_magic_symbol(void)
{
    // The builder and the RFC constant agree.
    TEST_ASSERT_EQUAL_MEMORY(HRR_MAGIC, tls13_hrr_random, 32);
}

static void test_hrr_build_kat(void)
{
    const uint8_t cookie[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t out[128];
    size_t n = tls13_build_hello_retry_request(out, sizeof(out), nullptr, 0, TLS_GROUP_X25519, cookie, sizeof(cookie),
                                               /*dtls=*/true);
    TEST_ASSERT_EQUAL_size_t(sizeof(HRR_WIRE), n);
    TEST_ASSERT_EQUAL_MEMORY(HRR_WIRE, out, sizeof(HRR_WIRE));
    // The random field carries the magic, i.e. this ServerHello is a HelloRetryRequest.
    TEST_ASSERT_EQUAL_MEMORY(HRR_MAGIC, out + 6, 32);
}

static void test_hrr_echoes_session_id(void)
{
    const uint8_t sid[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    const uint8_t cookie[2] = {0x01, 0x02};
    uint8_t out[128];
    size_t n = tls13_build_hello_retry_request(out, sizeof(out), sid, sizeof(sid), TLS_GROUP_X25519, cookie,
                                               sizeof(cookie), /*dtls=*/true);
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_UINT8(sizeof(sid), out[38]);        // legacy_session_id_echo length
    TEST_ASSERT_EQUAL_MEMORY(sid, out + 39, sizeof(sid)); // echoed verbatim
}

static void test_message_hash(void)
{
    uint8_t h[32];
    for (unsigned i = 0; i < 32; i++)
        h[i] = (uint8_t)(0x80 + i);
    uint8_t out[40];
    size_t n = tls13_build_message_hash(out, sizeof(out), h);
    TEST_ASSERT_EQUAL_size_t(36, n);
    // message_hash(254) || uint24(32) || Hash(CH1)
    TEST_ASSERT_EQUAL_UINT8(254, out[0]);
    TEST_ASSERT_EQUAL_UINT8(0x00, out[1]);
    TEST_ASSERT_EQUAL_UINT8(0x00, out[2]);
    TEST_ASSERT_EQUAL_UINT8(0x20, out[3]);
    TEST_ASSERT_EQUAL_MEMORY(h, out + 4, 32);
}

static void test_empty_encrypted_extensions(void)
{
    uint8_t out[16];
    size_t n = tls13_build_encrypted_extensions_empty(out, sizeof(out));
    const uint8_t want[6] = {0x08, 0x00, 0x00, 0x02, 0x00, 0x00}; // EE, len 2, extensions length 0
    TEST_ASSERT_EQUAL_size_t(sizeof(want), n);
    TEST_ASSERT_EQUAL_MEMORY(want, out, sizeof(want));
}

// A ClientHello carrying a cookie extension (the client's retry) is parsed and the cookie recovered.
static void test_client_hello_cookie_parse(void)
{
    const uint8_t cookie[3] = {0x11, 0x22, 0x33};
    // Assemble a minimal but well-formed ClientHello with exactly one extension (cookie).
    uint8_t ch[128];
    size_t p = 0;
    ch[p++] = 0x01; // client_hello
    size_t body_len_at = p;
    p += 3; // 24-bit body length (patched below)
    ch[p++] = 0x03;
    ch[p++] = 0x03; // legacy_version
    for (int i = 0; i < 32; i++)
        ch[p++] = (uint8_t)i; // random
    ch[p++] = 0x00;           // session_id length 0
    ch[p++] = 0x00;
    ch[p++] = 0x02;
    ch[p++] = 0x13;
    ch[p++] = 0x01; // cipher_suites: TLS_AES_128_GCM_SHA256
    ch[p++] = 0x01;
    ch[p++] = 0x00; // compression_methods: [null]
    // extensions: total length, then cookie extension.
    uint16_t ext_body = (uint16_t)(2 + sizeof(cookie)); // cookie<2> length prefix + cookie
    uint16_t ext_total = (uint16_t)(4 + ext_body);      // ext type(2) + ext len(2) + body
    ch[p++] = (uint8_t)(ext_total >> 8);
    ch[p++] = (uint8_t)ext_total;
    ch[p++] = 0x00;
    ch[p++] = 0x2c; // cookie extension type
    ch[p++] = (uint8_t)(ext_body >> 8);
    ch[p++] = (uint8_t)ext_body;
    ch[p++] = (uint8_t)(sizeof(cookie) >> 8);
    ch[p++] = (uint8_t)sizeof(cookie); // cookie length
    for (unsigned i = 0; i < sizeof(cookie); i++)
        ch[p++] = cookie[i];
    uint32_t body_len = (uint32_t)(p - body_len_at - 3);
    ch[body_len_at] = (uint8_t)(body_len >> 16);
    ch[body_len_at + 1] = (uint8_t)(body_len >> 8);
    ch[body_len_at + 2] = (uint8_t)body_len;

    Tls13ClientHello hello;
    TEST_ASSERT_TRUE(tls13_parse_client_hello(ch, p, &hello));
    TEST_ASSERT_NOT_NULL(hello.cookie);
    TEST_ASSERT_EQUAL_size_t(sizeof(cookie), hello.cookie_len);
    TEST_ASSERT_EQUAL_MEMORY(cookie, hello.cookie, sizeof(cookie));
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_hrr_magic_symbol);
    RUN_TEST(test_hrr_build_kat);
    RUN_TEST(test_hrr_echoes_session_id);
    RUN_TEST(test_message_hash);
    RUN_TEST(test_empty_encrypted_extensions);
    RUN_TEST(test_client_hello_cookie_parse);
    return UNITY_END();
}
