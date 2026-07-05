// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the HPACK codec (network_drivers/presentation/http2/hpack) against the RFC 7541
// worked examples: prefix-integer coding (Appendix C.1), the Huffman string code (C.4.1), the
// first request decode with dynamic-table insertion (C.3.1), plus dynamic-table indexing +
// FIFO eviction + size updates, and the server encoder (static indexing + literal round-trip).

#include "network_drivers/presentation/hpack_prim/hpack_prim.h" // prefix-int + Huffman primitives
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

struct Collected
{
    std::vector<std::pair<std::string, std::string>> h;
};
static bool collect(void *ctx, const char *n, size_t nl, const char *v, size_t vl)
{
    ((Collected *)ctx)->h.emplace_back(std::string(n, nl), std::string(v, vl));
    return true;
}

void test_int_coding()
{
    uint8_t b[8];
    size_t c;
    uint32_t v;
    // C.1.1: 10, prefix 5 -> 0x0a
    TEST_ASSERT_EQUAL_INT(1, (int)hpack_encode_int(b, sizeof b, 5, 0, 10));
    TEST_ASSERT_EQUAL_HEX8(0x0a, b[0]);
    TEST_ASSERT_TRUE(hpack_decode_int(b, 1, 5, &c, &v));
    TEST_ASSERT_EQUAL_UINT32(10, v);
    // C.1.2: 1337, prefix 5 -> 1f 9a 0a
    TEST_ASSERT_EQUAL_INT(3, (int)hpack_encode_int(b, sizeof b, 5, 0, 1337));
    const uint8_t exp[3] = {0x1f, 0x9a, 0x0a};
    TEST_ASSERT_EQUAL_UINT8_ARRAY(exp, b, 3);
    TEST_ASSERT_TRUE(hpack_decode_int(b, 3, 5, &c, &v));
    TEST_ASSERT_EQUAL_UINT32(1337, v);
    TEST_ASSERT_EQUAL_INT(3, (int)c);
    // C.1.3: 42, prefix 8 -> 0x2a
    TEST_ASSERT_EQUAL_INT(1, (int)hpack_encode_int(b, sizeof b, 8, 0, 42));
    TEST_ASSERT_EQUAL_HEX8(0x2a, b[0]);
}

void test_huffman()
{
    const char *s = "www.example.com";
    size_t n = strlen(s);
    const uint8_t exp[12] = {0xf1, 0xe3, 0xc2, 0xe5, 0xf2, 0x3a, 0x6b, 0xa0, 0xab, 0x90, 0xf4, 0xff};
    TEST_ASSERT_EQUAL_INT(12, (int)hpack_huff_len(s, n));
    uint8_t out[32];
    TEST_ASSERT_EQUAL_INT(12, (int)hpack_huff_encode(out, sizeof out, s, n));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(exp, out, 12);
    char dec[32];
    size_t dl;
    TEST_ASSERT_TRUE(hpack_huff_decode(exp, 12, dec, sizeof dec, &dl));
    TEST_ASSERT_EQUAL_INT((int)n, (int)dl);
    TEST_ASSERT_EQUAL_MEMORY(s, dec, n);
}

void test_decode_c31_and_index()
{
    // RFC 7541 C.3.1: GET / with :authority www.example.com (no Huffman).
    const uint8_t block[] = {0x82, 0x86, 0x84, 0x41, 0x0f, 0x77, 0x77, 0x77, 0x2e, 0x65,
                             0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d};
    HpackDynTable t;
    hpack_dyn_init(&t, 4096);
    Collected c;
    char scratch[512];
    TEST_ASSERT_TRUE(hpack_decode(&t, block, sizeof block, scratch, sizeof scratch, collect, &c));
    TEST_ASSERT_EQUAL_INT(4, (int)c.h.size());
    TEST_ASSERT_EQUAL_STRING(":method", c.h[0].first.c_str());
    TEST_ASSERT_EQUAL_STRING("GET", c.h[0].second.c_str());
    TEST_ASSERT_EQUAL_STRING(":scheme", c.h[1].first.c_str());
    TEST_ASSERT_EQUAL_STRING("http", c.h[1].second.c_str());
    TEST_ASSERT_EQUAL_STRING(":path", c.h[2].first.c_str());
    TEST_ASSERT_EQUAL_STRING("/", c.h[2].second.c_str());
    TEST_ASSERT_EQUAL_STRING(":authority", c.h[3].first.c_str());
    TEST_ASSERT_EQUAL_STRING("www.example.com", c.h[3].second.c_str());
    // RFC: the dynamic table now holds one entry of size 57.
    TEST_ASSERT_EQUAL_UINT32(57, t.used);
    TEST_ASSERT_EQUAL_INT(1, (int)t.ecount);

    // An indexed reference to entry 62 (0x80|62 = 0xbe) resolves it from the dynamic table.
    const uint8_t idx62[] = {0xbe};
    Collected c2;
    TEST_ASSERT_TRUE(hpack_decode(&t, idx62, 1, scratch, sizeof scratch, collect, &c2));
    TEST_ASSERT_EQUAL_INT(1, (int)c2.h.size());
    TEST_ASSERT_EQUAL_STRING(":authority", c2.h[0].first.c_str());
    TEST_ASSERT_EQUAL_STRING("www.example.com", c2.h[0].second.c_str());
}

void test_dynamic_eviction()
{
    HpackDynTable t;
    hpack_dyn_init(&t, 50); // room for one 36-byte entry, not two
    // Two literal-with-incremental-indexing inserts (name idx 0 + inline name/value), each size
    // 2+2+32 = 36. The second must evict the first.
    const uint8_t block[] = {0x40, 0x02, 'a', 'a', 0x02, 'b', 'b',  // insert (aa: bb)
                             0x40, 0x02, 'c', 'c', 0x02, 'd', 'd'}; // insert (cc: dd) -> evicts
    Collected c;
    char scratch[128];
    TEST_ASSERT_TRUE(hpack_decode(&t, block, sizeof block, scratch, sizeof scratch, collect, &c));
    TEST_ASSERT_EQUAL_INT(2, (int)c.h.size());
    TEST_ASSERT_EQUAL_INT(1, (int)t.ecount); // only the newest survives
    TEST_ASSERT_EQUAL_UINT32(36, t.used);
    // Index 62 is now (cc: dd); the evicted (aa: bb) would have been 63 (invalid now).
    const uint8_t idx62[] = {0xbe};
    Collected c2;
    TEST_ASSERT_TRUE(hpack_decode(&t, idx62, 1, scratch, sizeof scratch, collect, &c2));
    TEST_ASSERT_EQUAL_STRING("cc", c2.h[0].first.c_str());
    TEST_ASSERT_EQUAL_STRING("dd", c2.h[0].second.c_str());
}

void test_encode_static()
{
    uint8_t out[64];
    TEST_ASSERT_EQUAL_INT(1, (int)hpack_encode_header(out, sizeof out, ":method", 7, "GET", 3));
    TEST_ASSERT_EQUAL_HEX8(0x82, out[0]); // static index 2
    TEST_ASSERT_EQUAL_INT(1, (int)hpack_encode_header(out, sizeof out, ":path", 5, "/", 1));
    TEST_ASSERT_EQUAL_HEX8(0x84, out[0]); // static index 4
    TEST_ASSERT_EQUAL_INT(1, (int)hpack_encode_header(out, sizeof out, ":status", 7, "200", 3));
    TEST_ASSERT_EQUAL_HEX8(0x88, out[0]); // static index 8
}

void test_encode_decode_roundtrip()
{
    struct KV
    {
        const char *n;
        const char *v;
    };
    KV hs[] = {{":status", "200"},
               {"content-type", "text/html"},      // static name, literal value
               {"x-custom-header", "hello world"}, // fully literal
               {"server", "det/1"}};
    uint8_t block[512];
    size_t bo = 0;
    for (auto &kv : hs)
    {
        size_t w = hpack_encode_header(block + bo, sizeof block - bo, kv.n, strlen(kv.n), kv.v, strlen(kv.v));
        TEST_ASSERT_TRUE(w > 0);
        bo += w;
    }
    HpackDynTable t;
    hpack_dyn_init(&t, 4096);
    Collected c;
    char scratch[512];
    TEST_ASSERT_TRUE(hpack_decode(&t, block, bo, scratch, sizeof scratch, collect, &c));
    TEST_ASSERT_EQUAL_INT(4, (int)c.h.size());
    for (int i = 0; i < 4; i++)
    {
        TEST_ASSERT_EQUAL_STRING(hs[i].n, c.h[i].first.c_str());
        TEST_ASSERT_EQUAL_STRING(hs[i].v, c.h[i].second.c_str());
    }
    // The encoder never uses incremental indexing, so the decoder's table stays empty.
    TEST_ASSERT_EQUAL_INT(0, (int)t.ecount);
}

void test_reject_malformed()
{
    HpackDynTable t;
    hpack_dyn_init(&t, 4096);
    Collected c;
    char scratch[128];
    const uint8_t idx0[] = {0x80}; // indexed field, index 0 -> error
    TEST_ASSERT_FALSE(hpack_decode(&t, idx0, 1, scratch, sizeof scratch, collect, &c));
    const uint8_t trunc[] = {0x41, 0x0f, 'w', 'w'}; // literal len 15 but only 2 bytes present
    TEST_ASSERT_FALSE(hpack_decode(&t, trunc, sizeof trunc, scratch, sizeof scratch, collect, &c));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_int_coding);
    RUN_TEST(test_huffman);
    RUN_TEST(test_decode_c31_and_index);
    RUN_TEST(test_dynamic_eviction);
    RUN_TEST(test_encode_static);
    RUN_TEST(test_encode_decode_roundtrip);
    RUN_TEST(test_reject_malformed);
    return UNITY_END();
}
