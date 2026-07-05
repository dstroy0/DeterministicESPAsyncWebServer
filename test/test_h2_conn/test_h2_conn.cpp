// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the HTTP/2 connection engine (network_drivers/presentation/http2/h2_conn,
// RFC 9113): initial SETTINGS on init, the preface + client SETTINGS -> SETTINGS ACK, decoding a
// real HPACK-encoded request into the header callbacks, PING -> PING ACK, split-across-reads
// frame reassembly, and h2_conn_respond serializing a HEADERS + DATA response we can decode back.

#include "network_drivers/presentation/http2/h2_conn.h"
#include "network_drivers/presentation/http2/h2_frame.h"
#include "network_drivers/presentation/http2/hpack.h"
#include <string.h>
#include <string>
#include <unity.h>
#include <utility>
#include <vector>

void setUp()
{
}
void tearDown()
{
}

struct Cap
{
    std::vector<uint8_t> out;
    std::vector<std::pair<std::string, std::string>> req_headers;
    std::vector<uint32_t> headers_end;
    bool last_end_stream = false;
    std::string body;
    bool data_end = false;
};
static void cap_write(void *io, const uint8_t *d, size_t n)
{
    Cap *c = (Cap *)io;
    c->out.insert(c->out.end(), d, d + n);
}
static void cap_hdr(void *app, uint32_t, const char *n, size_t nl, const char *v, size_t vl)
{
    ((Cap *)app)->req_headers.emplace_back(std::string(n, nl), std::string(v, vl));
}
static void cap_hend(void *app, uint32_t sid, bool es)
{
    Cap *c = (Cap *)app;
    c->headers_end.push_back(sid);
    c->last_end_stream = es;
}
static void cap_data(void *app, uint32_t, const uint8_t *d, size_t n, bool es)
{
    Cap *c = (Cap *)app;
    c->body.append((const char *)d, n);
    c->data_end = es;
}

static H2Callbacks mk_cb(Cap *c)
{
    H2Callbacks cb;
    memset(&cb, 0, sizeof cb);
    cb.write = cap_write;
    cb.on_header = cap_hdr;
    cb.on_headers_end = cap_hend;
    cb.on_data = cap_data;
    cb.io = c;
    cb.app = c;
    return cb;
}

// Count frames of a given type in a captured byte stream (walking the 9-byte headers).
static int count_frames(const std::vector<uint8_t> &b, uint8_t type, int *ack_out = nullptr)
{
    int n = 0, ack = 0;
    size_t i = 0;
    while (i + 9 <= b.size())
    {
        H2FrameHeader h;
        h2_parse_header(&b[i], 9, &h);
        if (h.type == type)
        {
            n++;
            if (h.flags & H2_FLAG_ACK)
                ack++;
        }
        i += 9 + h.length;
    }
    if (ack_out)
        *ack_out = ack;
    return n;
}

// Build a client GET request header block via the HPACK encoder.
static size_t build_request(uint8_t *block, size_t cap)
{
    size_t bo = 0;
    bo += hpack_encode_header(block + bo, cap - bo, ":method", 7, "GET", 3);
    bo += hpack_encode_header(block + bo, cap - bo, ":scheme", 7, "https", 5);
    bo += hpack_encode_header(block + bo, cap - bo, ":path", 5, "/", 1);
    bo += hpack_encode_header(block + bo, cap - bo, ":authority", 10, "example.com", 11);
    return bo;
}

void test_init_and_request()
{
    Cap cap;
    H2Callbacks cb = mk_cb(&cap);
    H2Conn c;
    h2_conn_init(&c, &cb); // must emit our SETTINGS
    int acks = 0;
    TEST_ASSERT_EQUAL_INT(1, count_frames(cap.out, H2_SETTINGS, &acks));
    TEST_ASSERT_EQUAL_INT(0, acks); // our SETTINGS is not an ACK

    // Assemble: preface + empty client SETTINGS + HEADERS(stream 1, END_HEADERS|END_STREAM).
    std::vector<uint8_t> in(H2_PREFACE, H2_PREFACE + H2_PREFACE_LEN);
    uint8_t sf[9];
    size_t sn = h2_build_settings(sf, sizeof sf, nullptr, nullptr, 0);
    in.insert(in.end(), sf, sf + sn);
    uint8_t block[128];
    size_t blen = build_request(block, sizeof block);
    uint8_t hf[160];
    size_t hn = h2_build_headers(hf, sizeof hf, 1, block, blen, true);
    in.insert(in.end(), hf, hf + hn);

    cap.out.clear();
    TEST_ASSERT_TRUE(h2_conn_recv(&c, in.data(), in.size()));

    // The request headers were decoded and delivered.
    TEST_ASSERT_EQUAL_INT(4, (int)cap.req_headers.size());
    TEST_ASSERT_EQUAL_STRING(":method", cap.req_headers[0].first.c_str());
    TEST_ASSERT_EQUAL_STRING("GET", cap.req_headers[0].second.c_str());
    TEST_ASSERT_EQUAL_STRING(":path", cap.req_headers[2].first.c_str());
    TEST_ASSERT_EQUAL_STRING("/", cap.req_headers[2].second.c_str());
    TEST_ASSERT_EQUAL_STRING(":authority", cap.req_headers[3].first.c_str());
    TEST_ASSERT_EQUAL_STRING("example.com", cap.req_headers[3].second.c_str());
    TEST_ASSERT_EQUAL_INT(1, (int)cap.headers_end.size());
    TEST_ASSERT_EQUAL_UINT32(1, cap.headers_end[0]);
    TEST_ASSERT_TRUE(cap.last_end_stream);
    // We ACKed the client's SETTINGS.
    int acks2 = 0;
    count_frames(cap.out, H2_SETTINGS, &acks2);
    TEST_ASSERT_EQUAL_INT(1, acks2);
}

void test_respond_roundtrip()
{
    Cap cap;
    H2Callbacks cb = mk_cb(&cap);
    H2Conn c;
    h2_conn_init(&c, &cb);
    std::vector<uint8_t> in(H2_PREFACE, H2_PREFACE + H2_PREFACE_LEN);
    uint8_t sf[9];
    in.insert(in.end(), sf, sf + h2_build_settings(sf, sizeof sf, nullptr, nullptr, 0));
    uint8_t block[128];
    size_t blen = build_request(block, sizeof block);
    uint8_t hf[160];
    in.insert(in.end(), hf, hf + h2_build_headers(hf, sizeof hf, 1, block, blen, true));
    TEST_ASSERT_TRUE(h2_conn_recv(&c, in.data(), in.size()));

    cap.out.clear();
    TEST_ASSERT_TRUE(h2_conn_respond(&c, 1, 200, "text/plain", "hi", 2));
    // Output holds a HEADERS frame + a DATA frame on stream 1.
    TEST_ASSERT_EQUAL_INT(1, count_frames(cap.out, H2_HEADERS));
    TEST_ASSERT_EQUAL_INT(1, count_frames(cap.out, H2_DATA));

    // Walk the frames; decode the response HEADERS block and check the DATA.
    HpackDynTable dt;
    hpack_dyn_init(&dt, 4096);
    std::vector<std::pair<std::string, std::string>> rh;
    std::string data;
    bool data_end = false;
    size_t i = 0;
    while (i + 9 <= cap.out.size())
    {
        H2FrameHeader h;
        h2_parse_header(&cap.out[i], 9, &h);
        const uint8_t *pl = &cap.out[i + 9];
        if (h.type == H2_HEADERS && h.stream_id == 1)
        {
            char scratch[256];
            struct RC
            {
                std::vector<std::pair<std::string, std::string>> *v;
            } rc{&rh};
            hpack_decode(
                &dt, pl, h.length, scratch, sizeof scratch,
                [](void *ctx, const char *n, size_t nl, const char *v, size_t vl) {
                    ((RC *)ctx)->v->emplace_back(std::string(n, nl), std::string(v, vl));
                    return true;
                },
                &rc);
        }
        else if (h.type == H2_DATA && h.stream_id == 1)
        {
            data.append((const char *)pl, h.length);
            data_end = (h.flags & H2_FLAG_END_STREAM) != 0;
        }
        i += 9 + h.length;
    }
    TEST_ASSERT_EQUAL_INT(3, (int)rh.size());
    TEST_ASSERT_EQUAL_STRING(":status", rh[0].first.c_str());
    TEST_ASSERT_EQUAL_STRING("200", rh[0].second.c_str());
    TEST_ASSERT_EQUAL_STRING("content-type", rh[1].first.c_str());
    TEST_ASSERT_EQUAL_STRING("text/plain", rh[1].second.c_str());
    TEST_ASSERT_EQUAL_STRING("content-length", rh[2].first.c_str());
    TEST_ASSERT_EQUAL_STRING("2", rh[2].second.c_str());
    TEST_ASSERT_EQUAL_STRING("hi", data.c_str());
    TEST_ASSERT_TRUE(data_end);
}

void test_ping_and_split_recv()
{
    Cap cap;
    H2Callbacks cb = mk_cb(&cap);
    H2Conn c;
    h2_conn_init(&c, &cb);
    // Preface, then a PING frame, fed one byte at a time (exercises reassembly).
    std::vector<uint8_t> in(H2_PREFACE, H2_PREFACE + H2_PREFACE_LEN);
    const uint8_t op[8] = {9, 8, 7, 6, 5, 4, 3, 2};
    uint8_t ping[9 + 8];
    h2_write_header(ping, sizeof ping, 8, H2_PING, 0, 0);
    memcpy(ping + 9, op, 8);
    in.insert(in.end(), ping, ping + sizeof ping);

    cap.out.clear();
    for (size_t k = 0; k < in.size(); k++)
        TEST_ASSERT_TRUE(h2_conn_recv(&c, &in[k], 1));
    // A PING ACK echoing the opaque data was sent.
    int acks = 0;
    TEST_ASSERT_EQUAL_INT(1, count_frames(cap.out, H2_PING, &acks));
    TEST_ASSERT_EQUAL_INT(1, acks);
    // Locate the PING ACK payload and confirm it echoes the opaque bytes.
    size_t i = 0;
    bool found = false;
    while (i + 9 <= cap.out.size())
    {
        H2FrameHeader h;
        h2_parse_header(&cap.out[i], 9, &h);
        if (h.type == H2_PING && (h.flags & H2_FLAG_ACK))
        {
            TEST_ASSERT_EQUAL_MEMORY(op, &cap.out[i + 9], 8);
            found = true;
        }
        i += 9 + h.length;
    }
    TEST_ASSERT_TRUE(found);
}

void test_bad_preface()
{
    Cap cap;
    H2Callbacks cb = mk_cb(&cap);
    H2Conn c;
    h2_conn_init(&c, &cb);
    const uint8_t junk[] = {'G', 'E', 'T', ' ', '/', ' ', 'H'};
    TEST_ASSERT_FALSE(h2_conn_recv(&c, junk, sizeof junk));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_init_and_request);
    RUN_TEST(test_respond_roundtrip);
    RUN_TEST(test_ping_and_split_recv);
    RUN_TEST(test_bad_preface);
    return UNITY_END();
}
