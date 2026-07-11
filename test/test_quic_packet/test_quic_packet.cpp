// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the QUIC packet header + packet-number codec (network_drivers/presentation/http3/
// quic_packet, RFC 9000 sec 17): the long-header build/parse round-trip, a Version Negotiation
// packet (Version 0 + supported-version list), the short (1-RTT) header parse, and the
// packet-number truncation coding against the Appendix A.2 (encode) and A.3 (decode) worked
// examples. Pure host codec.

#include "network_drivers/presentation/http3/quic_packet.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

void test_long_header_roundtrip()
{
    const uint8_t dcid[5] = {0x01, 0x02, 0x03, 0x04, 0x05};
    const uint8_t scid[2] = {0xAA, 0xBB};
    uint8_t out[32];
    size_t n =
        quic_build_long_header(out, sizeof out, QuicLongPacket::QUIC_LP_INITIAL, QUIC_VERSION_1, dcid, 5, scid, 2, 1);
    const uint8_t exp[14] = {0xC0, 0x00, 0x00, 0x00, 0x01, 0x05, 0x01, 0x02, 0x03, 0x04, 0x05, 0x02, 0xAA, 0xBB};
    TEST_ASSERT_EQUAL_INT(14, (int)n);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(exp, out, 14);

    TEST_ASSERT_TRUE(quic_is_long_header(out[0]));
    QuicLongHeader h;
    TEST_ASSERT_TRUE(quic_parse_long_header(out, n, &h));
    TEST_ASSERT_EQUAL_UINT32(QUIC_VERSION_1, h.version);
    TEST_ASSERT_EQUAL_UINT8(QuicLongPacket::QUIC_LP_INITIAL, h.type);
    TEST_ASSERT_EQUAL_UINT8(5, h.dcid_len);
    TEST_ASSERT_EQUAL_UINT8(2, h.scid_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(dcid, h.dcid, 5);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(scid, h.scid, 2);
    TEST_ASSERT_EQUAL_UINT(14, (unsigned)h.hdr_len);
}

void test_version_negotiation()
{
    const uint8_t dcid[2] = {0xDE, 0xAD};
    const uint8_t scid[2] = {0xBE, 0xEF};
    const uint32_t vers[2] = {QUIC_VERSION_1, 0xFF000000u};
    uint8_t out[32];
    size_t n = quic_build_version_negotiation(out, sizeof out, dcid, 2, scid, 2, vers, 2);
    TEST_ASSERT_EQUAL_INT(1 + 4 + 1 + 2 + 1 + 2 + 8, (int)n);

    QuicLongHeader h;
    TEST_ASSERT_TRUE(quic_parse_long_header(out, n, &h));
    TEST_ASSERT_EQUAL_UINT32(0, h.version); // Version 0 -> Version Negotiation
    TEST_ASSERT_EQUAL_UINT(11, (unsigned)h.hdr_len);
    // Supported-version list follows the header.
    const uint8_t *v = out + h.hdr_len;
    TEST_ASSERT_EQUAL_UINT32(QUIC_VERSION_1, ((uint32_t)v[0] << 24) | (v[1] << 16) | (v[2] << 8) | v[3]);
    TEST_ASSERT_EQUAL_UINT32(0xFF000000u, ((uint32_t)v[4] << 24) | (v[5] << 16) | (v[6] << 8) | v[7]);
}

void test_short_header_parse()
{
    const uint8_t buf[7] = {0x41, 0x11, 0x22, 0x33, 0x44, 0x00, 0x00}; // fixed bit, pn_len bits = 1
    QuicShortHeader h;
    TEST_ASSERT_FALSE(quic_is_long_header(buf[0]));
    TEST_ASSERT_TRUE(quic_parse_short_header(buf, sizeof buf, 4, &h));
    TEST_ASSERT_EQUAL_UINT8(0, h.spin);
    TEST_ASSERT_EQUAL_UINT8(0, h.key_phase);
    TEST_ASSERT_EQUAL_UINT8(2, h.pn_len);
    TEST_ASSERT_EQUAL_UINT8(4, h.dcid_len);
    const uint8_t dcid[4] = {0x11, 0x22, 0x33, 0x44};
    TEST_ASSERT_EQUAL_UINT8_ARRAY(dcid, h.dcid, 4);
    TEST_ASSERT_EQUAL_UINT(5, (unsigned)h.hdr_len);
}

void test_pn_encode()
{
    uint8_t out[4];
    // RFC 9000 A.2: acked 0xabe8b3, sending 0xac5c02 -> 16-bit encoding.
    size_t n = quic_pn_encode(out, sizeof out, 0xac5c02, 0xabe8b3);
    TEST_ASSERT_EQUAL_INT(2, (int)n);
    const uint8_t e1[2] = {0x5c, 0x02};
    TEST_ASSERT_EQUAL_UINT8_ARRAY(e1, out, 2);
    // Same state, sending 0xace8fe -> 24-bit encoding.
    n = quic_pn_encode(out, sizeof out, 0xace8fe, 0xabe8b3);
    TEST_ASSERT_EQUAL_INT(3, (int)n);
    const uint8_t e2[3] = {0xac, 0xe8, 0xfe};
    TEST_ASSERT_EQUAL_UINT8_ARRAY(e2, out, 3);
    // Nothing acked yet, first packet -> single byte.
    TEST_ASSERT_EQUAL_INT(1, (int)quic_pn_length(0, -1));
}

void test_pn_decode()
{
    // RFC 9000 A.3: largest 0xa82f30ea, 16-bit truncated 0x9b32 -> 0xa82f9b32.
    TEST_ASSERT_TRUE(quic_pn_decode(0xa82f30ea, 0x9b32, 16) == 0xa82f9b32ull);

    // Round-trip: encode then decode recovers the full number.
    uint8_t out[4];
    uint64_t full = 0x112233;
    int64_t acked = 0x112200;
    size_t n = quic_pn_encode(out, sizeof out, full, acked);
    uint64_t trunc = 0;
    for (size_t i = 0; i < n; i++)
        trunc = (trunc << 8) | out[i];
    TEST_ASSERT_TRUE(quic_pn_decode(full - 1, trunc, (uint8_t)(n * 8)) == full);
}

void test_reject()
{
    QuicLongHeader h;
    // Destination Connection ID length 21 (> 20) must be dropped.
    const uint8_t bad[8] = {0xC0, 0x00, 0x00, 0x00, 0x01, 21, 0x00, 0x00};
    TEST_ASSERT_FALSE(quic_parse_long_header(bad, sizeof bad, &h));
    // Truncated long header.
    const uint8_t trunc[3] = {0xC0, 0x00, 0x00};
    TEST_ASSERT_FALSE(quic_parse_long_header(trunc, sizeof trunc, &h));
    // A short header parse needs at least 1 + dcid_len bytes.
    QuicShortHeader s;
    const uint8_t sbuf[2] = {0x41, 0x11};
    TEST_ASSERT_FALSE(quic_parse_short_header(sbuf, sizeof sbuf, 4, &s));
}

// Builder guards: bad packet-number length, oversize connection IDs, and buffer overflow.
void test_build_guards()
{
    const uint8_t cid[2] = {0x01, 0x02};
    uint8_t out[32];
    TEST_ASSERT_EQUAL_INT(
        0, (int)quic_build_long_header(out, sizeof out, QuicLongPacket::QUIC_LP_INITIAL, 1, cid, 2, cid, 2, 0));
    TEST_ASSERT_EQUAL_INT(
        0, (int)quic_build_long_header(out, sizeof out, QuicLongPacket::QUIC_LP_INITIAL, 1, cid, 2, cid, 2, 5));
    TEST_ASSERT_EQUAL_INT(
        0, (int)quic_build_long_header(out, sizeof out, QuicLongPacket::QUIC_LP_INITIAL, 1, cid, 21, cid, 2, 1));
    TEST_ASSERT_EQUAL_INT(
        0, (int)quic_build_long_header(out, 6, QuicLongPacket::QUIC_LP_INITIAL, 1, cid, 2, cid, 2, 1)); // too small
    TEST_ASSERT_EQUAL_INT(0, (int)quic_build_version_negotiation(out, 4, cid, 2, cid, 2, 0, 0));        // too small
    TEST_ASSERT_EQUAL_INT(0,
                          (int)quic_build_version_negotiation(out, sizeof out, cid, 21, cid, 2, 0, 0)); // oversize CID

    // Packet-number encode into a buffer too small, and the 4-byte length ceiling.
    uint8_t pn[1];
    TEST_ASSERT_EQUAL_INT(0, (int)quic_pn_encode(pn, 1, 0x123456, -1)); // needs 3 bytes
    TEST_ASSERT_EQUAL_INT(4, (int)quic_pn_length(0xFFFFFFFFull, -1));   // huge unacked range -> 4 bytes
}

// Short-header parse guards: oversize DCID length and a long-header first byte.
void test_short_header_guards()
{
    QuicShortHeader h;
    const uint8_t buf[8] = {0x40, 1, 2, 3, 4, 5, 6, 7};
    TEST_ASSERT_FALSE(quic_parse_short_header(buf, sizeof buf, 21, &h)); // DCID length > 20
    const uint8_t longform[8] = {0xC0, 1, 2, 3, 4, 5, 6, 7};
    TEST_ASSERT_FALSE(quic_parse_short_header(longform, sizeof longform, 4, &h)); // 0x80 set = long header

    // Long-header parse with the Source Connection ID truncated.
    QuicLongHeader lh;
    const uint8_t scid_trunc[8] = {0xC0, 0, 0, 0, 1, 0x00, 0x05, 0xAA}; // dcid_len 0, scid_len 5 but 1 byte
    TEST_ASSERT_FALSE(quic_parse_long_header(scid_trunc, sizeof scid_trunc, &lh));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_long_header_roundtrip);
    RUN_TEST(test_version_negotiation);
    RUN_TEST(test_short_header_parse);
    RUN_TEST(test_pn_encode);
    RUN_TEST(test_pn_decode);
    RUN_TEST(test_reject);
    RUN_TEST(test_build_guards);
    RUN_TEST(test_short_header_guards);
    return UNITY_END();
}
