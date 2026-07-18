// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the HTTP/3 application engine (network_drivers/presentation/http3/dws_h3_conn; RFC
// 9114). Drives dws_h3_conn through the dws_quic_conn callback seam (qc->cb.on_stream_data /
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
    for (size_t i = 0; i < DWS_QUIC_MAX_STREAMS; i++)
        qc->streams[i].id = UINT64_MAX;
}

static QuicStream *find_stream(QuicConn *qc, uint64_t id)
{
    for (size_t i = 0; i < DWS_QUIC_MAX_STREAMS; i++)
        if (qc->streams[i].id == id)
            return &qc->streams[i];
    return nullptr;
}

// Find an HTTP/3 stream (with its classified role) by id.
static H3Stream *find_h3(H3Conn *h3, uint64_t id)
{
    for (size_t i = 0; i < DWS_H3_MAX_STREAMS; i++)
        if (h3->streams[i].role != H3StreamRole::H3_ROLE_FREE && h3->streams[i].id == id)
            return &h3->streams[i];
    return nullptr;
}

// Emit target for decoding a response field section.
static char e_status[8], e_ctype[32];
static bool dws_resp_emit(void *, const char *name, size_t nlen, const char *value, size_t vlen)
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
    dws_h3_conn_init(&h3, &qc, on_request, nullptr);
    TEST_ASSERT_NOT_NULL(qc.cb.on_stream_data);

    // Build a GET request: HEADERS(:method GET, :path /index.html, :authority example.org).
    uint8_t block[128];
    size_t bp = dws_qpack_encode_prefix(block, sizeof(block));
    bp += dws_qpack_encode_header(block + bp, sizeof(block) - bp, ":method", 7, "GET", 3);
    bp += dws_qpack_encode_header(block + bp, sizeof(block) - bp, ":path", 5, "/index.html", 11);
    bp += dws_qpack_encode_header(block + bp, sizeof(block) - bp, ":authority", 10, "example.org", 11);
    uint8_t req[256];
    size_t rp = dws_h3_build_headers(req, sizeof(req), block, bp);

    // Deliver it on request stream 0 with FIN through the QUIC callback seam.
    qc.cb.on_stream_data(qc.cb.app, &qc, 0, req, rp, true);
    TEST_ASSERT_EQUAL_INT(1, g_requests);
    TEST_ASSERT_EQUAL_STRING("GET", g_method);
    TEST_ASSERT_EQUAL_STRING("/index.html", g_path);
    TEST_ASSERT_EQUAL_STRING("example.org", g_auth);
    TEST_ASSERT_EQUAL_UINT(0, g_body_len);

    // Respond and decode what was queued onto the QUIC stream.
    TEST_ASSERT_TRUE(dws_h3_conn_respond(&h3, 0, 200, "text/plain", (const uint8_t *)"hello", 5));
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
        TEST_ASSERT_TRUE(dws_h3_frame_parse(st->tx + off, st->tx_have - off, &fr));
        const uint8_t *fp = st->tx + off + fr.header_len;
        if (fr.type == H3FrameType::H3_HEADERS)
        {
            dws_qpack_decode(fp, (size_t)fr.length, scratch, sizeof(scratch), dws_resp_emit, nullptr);
            saw_headers = true;
        }
        else if (fr.type == H3FrameType::H3_DATA)
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
    dws_h3_conn_init(&h3, &qc, on_request, nullptr);

    uint8_t block[128];
    size_t bp = dws_qpack_encode_prefix(block, sizeof(block));
    bp += dws_qpack_encode_header(block + bp, sizeof(block) - bp, ":method", 7, "POST", 4);
    bp += dws_qpack_encode_header(block + bp, sizeof(block) - bp, ":path", 5, "/submit", 7);
    uint8_t req[256];
    size_t rp = dws_h3_build_headers(req, sizeof(req), block, bp);
    rp += dws_h3_build_data(req + rp, sizeof(req) - rp, (const uint8_t *)"name=x", 6);

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
    dws_h3_conn_init(&h3, &qc, on_request, nullptr);

    // Simulate handshake completion: the control + QPACK streams should be opened.
    qc.cb.on_handshake_done(qc.cb.app, &qc);
    QuicStream *ctrl = find_stream(&qc, 3);
    TEST_ASSERT_NOT_NULL(ctrl);
    // Stream type 0x00, then a SETTINGS frame.
    uint64_t type = 0;
    size_t c = 0;
    TEST_ASSERT_TRUE(dws_quic_varint_decode(ctrl->tx, ctrl->tx_have, &type, &c));
    TEST_ASSERT_EQUAL_UINT64(0x00, type);
    H3Frame fr;
    TEST_ASSERT_TRUE(dws_h3_frame_parse(ctrl->tx + c, ctrl->tx_have - c, &fr));
    TEST_ASSERT_EQUAL_UINT64(H3FrameType::H3_SETTINGS, fr.type);
    // QPACK encoder (7) and decoder (11) streams exist with their type bytes.
    TEST_ASSERT_NOT_NULL(find_stream(&qc, 7));
    TEST_ASSERT_NOT_NULL(find_stream(&qc, 11));
}

// The client's control stream (a uni stream, type 0x00) carries SETTINGS; the server classifies the
// stream by its type varint and parses the SETTINGS into peer_settings.
void test_client_control_stream_settings()
{
    QuicConn qc;
    minimal_qc(&qc);
    H3Conn h3;
    dws_h3_conn_init(&h3, &qc, on_request, nullptr);

    uint8_t s[64];
    size_t sp = dws_quic_varint_encode(s, sizeof(s), 0x00); // control stream type
    const uint64_t ids[] = {H3Setting::H3_SETTINGS_MAX_FIELD_SECTION_SIZE};
    const uint64_t vals[] = {12345};
    sp += dws_h3_build_settings(s + sp, sizeof(s) - sp, ids, vals, 1);
    qc.cb.on_stream_data(qc.cb.app, &qc, 2, s, sp, false); // client-initiated uni stream (id 2)

    H3Stream *st = find_h3(&h3, 2);
    TEST_ASSERT_NOT_NULL(st);
    TEST_ASSERT_EQUAL_UINT8(H3StreamRole::H3_ROLE_CONTROL, st->role);
    TEST_ASSERT_EQUAL_UINT64(12345, h3.peer_settings.max_field_section_size);
}

// The QPACK encoder / decoder streams (type 0x02 / 0x03) and an unknown uni stream are classified
// and then drained (static-table only, nothing to process).
void test_client_uni_stream_types()
{
    QuicConn qc;
    minimal_qc(&qc);
    H3Conn h3;
    dws_h3_conn_init(&h3, &qc, on_request, nullptr);

    uint8_t t;
    size_t n = dws_quic_varint_encode(&t, 1, 0x02);
    qc.cb.on_stream_data(qc.cb.app, &qc, 6, &t, n, false);
    n = dws_quic_varint_encode(&t, 1, 0x03);
    qc.cb.on_stream_data(qc.cb.app, &qc, 10, &t, n, false);
    n = dws_quic_varint_encode(&t, 1, 0x1f); // an unknown stream type
    qc.cb.on_stream_data(qc.cb.app, &qc, 14, &t, n, false);

    TEST_ASSERT_EQUAL_UINT8(H3StreamRole::H3_ROLE_QPACK_ENC, find_h3(&h3, 6)->role);
    TEST_ASSERT_EQUAL_UINT8(H3StreamRole::H3_ROLE_QPACK_DEC, find_h3(&h3, 10)->role);
    TEST_ASSERT_EQUAL_UINT8(H3StreamRole::H3_ROLE_OTHER_UNI, find_h3(&h3, 14)->role);
}

// on_handshake_done opens the control + QPACK streams exactly once; a repeat call is a no-op.
void test_handshake_done_idempotent()
{
    QuicConn qc;
    minimal_qc(&qc);
    H3Conn h3;
    dws_h3_conn_init(&h3, &qc, on_request, nullptr);

    qc.cb.on_handshake_done(qc.cb.app, &qc);
    QuicStream *ctrl = find_stream(&qc, 3);
    TEST_ASSERT_NOT_NULL(ctrl);
    size_t first = ctrl->tx_have;
    qc.cb.on_handshake_done(qc.cb.app, &qc); // control_opened -> no-op
    TEST_ASSERT_EQUAL_UINT(first, ctrl->tx_have);
}

// Malformed request frames (an incomplete varint header, or a length past the buffer) do not dispatch.
void test_malformed_request_frame()
{
    g_requests = 0;
    QuicConn qc;
    minimal_qc(&qc);
    H3Conn h3;
    dws_h3_conn_init(&h3, &qc, on_request, nullptr);

    // A HEADERS frame that declares length 9999 but has no payload -> incomplete -> not dispatched.
    uint8_t hdr[8];
    size_t hp = dws_h3_frame_write_header(hdr, sizeof(hdr), H3FrameType::H3_HEADERS, 9999);
    qc.cb.on_stream_data(qc.cb.app, &qc, 0, hdr, hp, true);
    TEST_ASSERT_EQUAL_INT(0, g_requests);

    // A truncated frame-header varint -> parse fails -> not dispatched.
    uint8_t junk[1] = {0xC0}; // first byte of an 8-byte varint, nothing after it
    qc.cb.on_stream_data(qc.cb.app, &qc, 4, junk, sizeof(junk), true);
    TEST_ASSERT_EQUAL_INT(0, g_requests);
}

// A response body too large to frame into the output buffer fails cleanly.
void test_respond_body_too_large()
{
    QuicConn qc;
    minimal_qc(&qc);
    H3Conn h3;
    dws_h3_conn_init(&h3, &qc, on_request, nullptr);
    static uint8_t big[DWS_H3_STREAM_BUF + 200];
    memset(big, 'x', sizeof(big));
    TEST_ASSERT_FALSE(dws_h3_conn_respond(&h3, 0, 200, "text/plain", big, sizeof(big)));
}

// When every stream slot is occupied, a further new stream is dropped rather than overrunning.
void test_stream_pool_full()
{
    g_requests = 0;
    QuicConn qc;
    minimal_qc(&qc);
    H3Conn h3;
    dws_h3_conn_init(&h3, &qc, on_request, nullptr);

    uint8_t b = 0x00;
    for (uint64_t i = 0; i < DWS_H3_MAX_STREAMS; i++)
        qc.cb.on_stream_data(qc.cb.app, &qc, i * 4, &b, 1, false); // partial request streams, no FIN

    // One more distinct request stream cannot allocate a slot -> silently ignored.
    uint8_t block[64];
    size_t bp = dws_qpack_encode_prefix(block, sizeof(block));
    bp += dws_qpack_encode_header(block + bp, sizeof(block) - bp, ":method", 7, "GET", 3);
    bp += dws_qpack_encode_header(block + bp, sizeof(block) - bp, ":path", 5, "/x", 2);
    uint8_t req[128];
    size_t rp = dws_h3_build_headers(req, sizeof(req), block, bp);
    qc.cb.on_stream_data(qc.cb.app, &qc, (uint64_t)DWS_H3_MAX_STREAMS * 4, req, rp, true);
    TEST_ASSERT_EQUAL_INT(0, g_requests);
}

// A uni-stream type varint split across two deliveries: the first (incomplete) is buffered until the
// rest arrives, then the stream is classified.
void test_uni_stream_partial_type()
{
    QuicConn qc;
    minimal_qc(&qc);
    H3Conn h3;
    dws_h3_conn_init(&h3, &qc, on_request, nullptr);

    uint8_t b0 = 0x40; // first byte of a 2-byte varint - incomplete on its own
    qc.cb.on_stream_data(qc.cb.app, &qc, 2, &b0, 1, false);
    TEST_ASSERT_FALSE(find_h3(&h3, 2)->type_read); // needs more bytes; not yet classified
    uint8_t b1 = 0x00;                             // completes the varint 0x4000 -> value 0 -> control stream
    qc.cb.on_stream_data(qc.cb.app, &qc, 2, &b1, 1, false);
    TEST_ASSERT_TRUE(find_h3(&h3, 2)->type_read);
    TEST_ASSERT_EQUAL_UINT8(H3StreamRole::H3_ROLE_CONTROL, find_h3(&h3, 2)->role);
}

// A pseudo-header longer than its capture buffer is truncated, not overrun.
void test_overlong_field_truncated()
{
    g_requests = 0;
    QuicConn qc;
    minimal_qc(&qc);
    H3Conn h3;
    dws_h3_conn_init(&h3, &qc, on_request, nullptr);

    uint8_t block[128];
    size_t bp = dws_qpack_encode_prefix(block, sizeof(block));
    const char *m = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; // 26 chars > DWS_H3_METHOD_LEN (16)
    bp += dws_qpack_encode_header(block + bp, sizeof(block) - bp, ":method", 7, m, 26);
    bp += dws_qpack_encode_header(block + bp, sizeof(block) - bp, ":path", 5, "/", 1);
    uint8_t req[256];
    size_t rp = dws_h3_build_headers(req, sizeof(req), block, bp);
    qc.cb.on_stream_data(qc.cb.app, &qc, 0, req, rp, true);

    TEST_ASSERT_EQUAL_INT(1, g_requests);
    TEST_ASSERT_EQUAL_UINT(DWS_H3_METHOD_LEN - 1, strlen(g_method)); // truncated to fit
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_request_dispatch_and_response);
    RUN_TEST(test_post_with_body);
    RUN_TEST(test_control_stream_settings_sent);
    RUN_TEST(test_client_control_stream_settings);
    RUN_TEST(test_client_uni_stream_types);
    RUN_TEST(test_handshake_done_idempotent);
    RUN_TEST(test_malformed_request_frame);
    RUN_TEST(test_respond_body_too_large);
    RUN_TEST(test_stream_pool_full);
    RUN_TEST(test_uni_stream_partial_type);
    RUN_TEST(test_overlong_field_truncated);
    return UNITY_END();
}
