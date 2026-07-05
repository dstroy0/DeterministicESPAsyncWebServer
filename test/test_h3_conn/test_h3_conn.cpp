// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the HTTP/3 application engine (network_drivers/presentation/http3/h3_conn; RFC
// 9114). Drives h3_conn through the quic_conn callback seam (qc->cb.on_stream_data /
// on_handshake_done) without a live QUIC handshake: it feeds a QPACK-encoded request on a request
// stream and checks the dispatched method/path, checks the SETTINGS the control stream sends on
// handshake completion, and round-trips a response (HEADERS + DATA) back through the QUIC stream
// send buffer.

#include "network_drivers/presentation/http3/h3_conn.h"
#include "network_drivers/presentation/http3/h3_frame.h"
#include "network_drivers/presentation/http3/qpack.h"
#include "network_drivers/presentation/http3/quic_conn.h"
#include "network_drivers/presentation/http3/quic_varint.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

static char g_method[16], g_path[64], g_auth[64];
static uint8_t g_body[64];
static size_t g_body_len;
static uint64_t g_sid;
static int g_requests;

static void on_request(void *, H3Conn *, uint64_t sid, const char *method, const char *path, const char *authority,
                       const uint8_t *body, size_t body_len)
{
    g_sid = sid;
    strncpy(g_method, method, sizeof(g_method) - 1);
    strncpy(g_path, path, sizeof(g_path) - 1);
    strncpy(g_auth, authority, sizeof(g_auth) - 1);
    g_body_len = body_len < sizeof(g_body) ? body_len : sizeof(g_body);
    memcpy(g_body, body, g_body_len);
    g_requests++;
}

static void minimal_qc(QuicConn *qc)
{
    memset(qc, 0, sizeof(*qc));
    for (int i = 0; i < 3; i++)
        qc->space[i].largest_acked = -1;
    for (size_t i = 0; i < DETWS_QUIC_MAX_STREAMS; i++)
        qc->streams[i].id = UINT64_MAX;
}

static QuicStream *find_stream(QuicConn *qc, uint64_t id)
{
    for (size_t i = 0; i < DETWS_QUIC_MAX_STREAMS; i++)
        if (qc->streams[i].id == id)
            return &qc->streams[i];
    return nullptr;
}

// Emit target for decoding a response field section.
static char e_status[8], e_ctype[32];
static bool resp_emit(void *, const char *name, size_t nlen, const char *value, size_t vlen)
{
    if (nlen == 7 && memcmp(name, ":status", 7) == 0)
    {
        memcpy(e_status, value, vlen);
        e_status[vlen] = '\0';
    }
    else if (nlen == 12 && memcmp(name, "content-type", 12) == 0)
    {
        memcpy(e_ctype, value, vlen);
        e_ctype[vlen] = '\0';
    }
    return true;
}

void test_request_dispatch_and_response()
{
    g_requests = 0;
    QuicConn qc;
    minimal_qc(&qc);
    H3Conn h3;
    h3_conn_init(&h3, &qc, on_request, nullptr);
    TEST_ASSERT_NOT_NULL(qc.cb.on_stream_data);

    // Build a GET request: HEADERS(:method GET, :path /index.html, :authority example.org).
    uint8_t block[128];
    size_t bp = qpack_encode_prefix(block, sizeof(block));
    bp += qpack_encode_header(block + bp, sizeof(block) - bp, ":method", 7, "GET", 3);
    bp += qpack_encode_header(block + bp, sizeof(block) - bp, ":path", 5, "/index.html", 11);
    bp += qpack_encode_header(block + bp, sizeof(block) - bp, ":authority", 10, "example.org", 11);
    uint8_t req[256];
    size_t rp = h3_build_headers(req, sizeof(req), block, bp);

    // Deliver it on request stream 0 with FIN through the QUIC callback seam.
    qc.cb.on_stream_data(qc.cb.app, &qc, 0, req, rp, true);
    TEST_ASSERT_EQUAL_INT(1, g_requests);
    TEST_ASSERT_EQUAL_STRING("GET", g_method);
    TEST_ASSERT_EQUAL_STRING("/index.html", g_path);
    TEST_ASSERT_EQUAL_STRING("example.org", g_auth);
    TEST_ASSERT_EQUAL_UINT(0, g_body_len);

    // Respond and decode what was queued onto the QUIC stream.
    TEST_ASSERT_TRUE(h3_conn_respond(&h3, 0, 200, "text/plain", (const uint8_t *)"hello", 5));
    QuicStream *st = find_stream(&qc, 0);
    TEST_ASSERT_NOT_NULL(st);
    // Walk the response frames: HEADERS then DATA.
    size_t off = 0;
    bool saw_headers = false, saw_data = false;
    char scratch[128];
    e_status[0] = e_ctype[0] = '\0';
    while (off < st->tx_have)
    {
        H3Frame fr;
        TEST_ASSERT_TRUE(h3_frame_parse(st->tx + off, st->tx_have - off, &fr));
        const uint8_t *fp = st->tx + off + fr.header_len;
        if (fr.type == H3_HEADERS)
        {
            qpack_decode(fp, (size_t)fr.length, scratch, sizeof(scratch), resp_emit, nullptr);
            saw_headers = true;
        }
        else if (fr.type == H3_DATA)
        {
            TEST_ASSERT_EQUAL_UINT(5, (size_t)fr.length);
            TEST_ASSERT_EQUAL_UINT8_ARRAY("hello", fp, 5);
            saw_data = true;
        }
        off += fr.header_len + (size_t)fr.length;
    }
    TEST_ASSERT_TRUE(saw_headers);
    TEST_ASSERT_TRUE(saw_data);
    TEST_ASSERT_EQUAL_STRING("200", e_status);
    TEST_ASSERT_EQUAL_STRING("text/plain", e_ctype);
}

void test_post_with_body()
{
    g_requests = 0;
    QuicConn qc;
    minimal_qc(&qc);
    H3Conn h3;
    h3_conn_init(&h3, &qc, on_request, nullptr);

    uint8_t block[128];
    size_t bp = qpack_encode_prefix(block, sizeof(block));
    bp += qpack_encode_header(block + bp, sizeof(block) - bp, ":method", 7, "POST", 4);
    bp += qpack_encode_header(block + bp, sizeof(block) - bp, ":path", 5, "/submit", 7);
    uint8_t req[256];
    size_t rp = h3_build_headers(req, sizeof(req), block, bp);
    rp += h3_build_data(req + rp, sizeof(req) - rp, (const uint8_t *)"name=x", 6);

    qc.cb.on_stream_data(qc.cb.app, &qc, 4, req, rp, true);
    TEST_ASSERT_EQUAL_INT(1, g_requests);
    TEST_ASSERT_EQUAL_STRING("POST", g_method);
    TEST_ASSERT_EQUAL_STRING("/submit", g_path);
    TEST_ASSERT_EQUAL_UINT(6, g_body_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY("name=x", g_body, 6);
}

void test_control_stream_settings_sent()
{
    QuicConn qc;
    minimal_qc(&qc);
    H3Conn h3;
    h3_conn_init(&h3, &qc, on_request, nullptr);

    // Simulate handshake completion: the control + QPACK streams should be opened.
    qc.cb.on_handshake_done(qc.cb.app, &qc);
    QuicStream *ctrl = find_stream(&qc, 3);
    TEST_ASSERT_NOT_NULL(ctrl);
    // Stream type 0x00, then a SETTINGS frame.
    uint64_t type = 0;
    size_t c = 0;
    TEST_ASSERT_TRUE(quic_varint_decode(ctrl->tx, ctrl->tx_have, &type, &c));
    TEST_ASSERT_EQUAL_UINT64(0x00, type);
    H3Frame fr;
    TEST_ASSERT_TRUE(h3_frame_parse(ctrl->tx + c, ctrl->tx_have - c, &fr));
    TEST_ASSERT_EQUAL_UINT64(H3_SETTINGS, fr.type);
    // QPACK encoder (7) and decoder (11) streams exist with their type bytes.
    TEST_ASSERT_NOT_NULL(find_stream(&qc, 7));
    TEST_ASSERT_NOT_NULL(find_stream(&qc, 11));
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_request_dispatch_and_response);
    RUN_TEST(test_post_with_body);
    RUN_TEST(test_control_stream_settings_sent);
    return UNITY_END();
}
