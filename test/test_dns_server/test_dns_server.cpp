// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the authoritative DNS server (services/dns_server): the pure response
// builder (A-record answer, NXDOMAIN, non-A query, malformed guards, header flags) and the
// built-in name->IP table (add / case-insensitive lookup / clear).

#include "services/dns_server/dns_server.h"
#include <stdint.h>
#include <string.h>
#include <unity.h>

// Build a DNS query: header + one question (QNAME / QTYPE / QCLASS=IN).
static size_t make_query(uint8_t *buf, uint16_t id, const char *name, uint16_t qtype, bool rd)
{
    size_t n = 0;
    buf[n++] = (uint8_t)(id >> 8);
    buf[n++] = (uint8_t)id;
    buf[n++] = rd ? 0x01 : 0x00; // flags hi: standard query, RD bit
    buf[n++] = 0x00;             // flags lo
    buf[n++] = 0x00;
    buf[n++] = 0x01; // QDCOUNT = 1
    for (int i = 0; i < 6; i++)
        buf[n++] = 0x00; // AN/NS/AR = 0
    const char *p = name;
    while (*p)
    {
        const char *dot = strchr(p, '.');
        size_t label = dot ? (size_t)(dot - p) : strlen(p);
        buf[n++] = (uint8_t)label;
        memcpy(buf + n, p, label);
        n += label;
        p += label;
        if (*p == '.')
            p++;
    }
    buf[n++] = 0x00; // end of QNAME
    buf[n++] = (uint8_t)(qtype >> 8);
    buf[n++] = (uint8_t)qtype;
    buf[n++] = 0x00;
    buf[n++] = 0x01; // QCLASS = IN
    return n;
}

static uint32_t resolve_foo(const char *name)
{
    return strcmp(name, "foo.lan") == 0 ? 0xC0A80105u : 0; // 192.168.1.5
}
static uint32_t resolve_none(const char *name)
{
    (void)name;
    return 0;
}

void setUp()
{
    dns_server_clear();
}
void tearDown()
{
}

void test_a_record_answer()
{
    uint8_t q[128], out[256];
    size_t qlen = make_query(q, 0x1234, "foo.lan", 1, true);
    size_t n = dns_server_build_response(q, qlen, 60, resolve_foo, out, sizeof(out));

    TEST_ASSERT_EQUAL_UINT(qlen + 16, n);  // header+question copied + 16-byte A answer
    TEST_ASSERT_EQUAL_UINT8(0x12, out[0]); // id preserved
    TEST_ASSERT_EQUAL_UINT8(0x34, out[1]);
    TEST_ASSERT_TRUE(out[2] & 0x80);              // QR = 1 (response)
    TEST_ASSERT_TRUE(out[2] & 0x04);              // AA = 1
    TEST_ASSERT_TRUE(out[2] & 0x01);              // RD copied from the query
    TEST_ASSERT_EQUAL_UINT8(0x00, out[3] & 0x0F); // RCODE = 0
    TEST_ASSERT_EQUAL_UINT8(0x01, out[5]);        // QDCOUNT = 1
    TEST_ASSERT_EQUAL_UINT8(0x01, out[7]);        // ANCOUNT = 1
    // Answer record (appended at qlen): 0xC00C, A, IN, TTL, rdlen 4, 192.168.1.5
    const uint8_t *a = out + qlen;
    TEST_ASSERT_EQUAL_UINT8(0xC0, a[0]);
    TEST_ASSERT_EQUAL_UINT8(0x0C, a[1]); // name pointer to the question
    TEST_ASSERT_EQUAL_UINT8(0x00, a[2]);
    TEST_ASSERT_EQUAL_UINT8(0x01, a[3]); // TYPE A
    TEST_ASSERT_EQUAL_UINT8(0x00, a[4]);
    TEST_ASSERT_EQUAL_UINT8(0x01, a[5]);  // CLASS IN
    TEST_ASSERT_EQUAL_UINT8(60, a[9]);    // TTL low byte
    TEST_ASSERT_EQUAL_UINT8(0x04, a[11]); // RDLENGTH 4
    TEST_ASSERT_EQUAL_UINT8(192, a[12]);
    TEST_ASSERT_EQUAL_UINT8(168, a[13]);
    TEST_ASSERT_EQUAL_UINT8(1, a[14]);
    TEST_ASSERT_EQUAL_UINT8(5, a[15]);
}

void test_nxdomain()
{
    uint8_t q[128], out[256];
    size_t qlen = make_query(q, 1, "unknown.lan", 1, false);
    size_t n = dns_server_build_response(q, qlen, 60, resolve_none, out, sizeof(out));
    TEST_ASSERT_EQUAL_UINT(qlen, n);              // no answer appended
    TEST_ASSERT_EQUAL_UINT8(0x00, out[7]);        // ANCOUNT = 0
    TEST_ASSERT_EQUAL_UINT8(0x03, out[3] & 0x0F); // RCODE = 3 (NXDOMAIN)
}

void test_non_a_query_no_error()
{
    uint8_t q[128], out[256];
    size_t qlen = make_query(q, 1, "foo.lan", 28, false); // AAAA
    size_t n = dns_server_build_response(q, qlen, 60, resolve_foo, out, sizeof(out));
    TEST_ASSERT_EQUAL_UINT(qlen, n);
    TEST_ASSERT_EQUAL_UINT8(0x00, out[7]);        // ANCOUNT = 0
    TEST_ASSERT_EQUAL_UINT8(0x00, out[3] & 0x0F); // RCODE 0 (not NXDOMAIN - we just don't serve AAAA)
}

void test_multilabel_name_reaches_resolver()
{
    static char seen[128];
    struct L
    {
        static uint32_t cap(const char *name)
        {
            strncpy(seen, name, sizeof(seen) - 1);
            seen[sizeof(seen) - 1] = 0;
            return 0;
        }
    };
    uint8_t q[128], out[256];
    size_t qlen = make_query(q, 1, "a.b.c.example", 1, false);
    dns_server_build_response(q, qlen, 60, L::cap, out, sizeof(out));
    TEST_ASSERT_EQUAL_STRING("a.b.c.example", seen);
}

void test_malformed_guards()
{
    uint8_t q[128], out[256];
    size_t qlen = make_query(q, 1, "foo.lan", 1, false);
    TEST_ASSERT_EQUAL_UINT(0, dns_server_build_response(q, 11, 60, resolve_foo, out, sizeof(out))); // < header
    TEST_ASSERT_EQUAL_UINT(0, dns_server_build_response(nullptr, qlen, 60, resolve_foo, out, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT(0, dns_server_build_response(q, qlen, 60, nullptr, out, sizeof(out)));
    // A compression pointer inside the question is illegal.
    uint8_t bad[16] = {0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0xC0, 0x0C, 0, 1};
    TEST_ASSERT_EQUAL_UINT(0, dns_server_build_response(bad, sizeof(bad), 60, resolve_foo, out, sizeof(out)));
    // Output too small for the answer.
    TEST_ASSERT_EQUAL_UINT(0, dns_server_build_response(q, qlen, 60, resolve_foo, out, qlen + 8));
}

void test_table_add_lookup_case_insensitive()
{
    TEST_ASSERT_TRUE(dns_server_add("Printer.LAN", 192, 168, 1, 10));
    TEST_ASSERT_TRUE(dns_server_add("clock.lan", 192, 168, 1, 11));
    TEST_ASSERT_EQUAL_HEX32(0xC0A8010Au, dns_server_lookup("printer.lan")); // case-insensitive hit
    TEST_ASSERT_EQUAL_HEX32(0xC0A8010Au, dns_server_lookup("PRINTER.LAN"));
    TEST_ASSERT_EQUAL_HEX32(0xC0A8010Bu, dns_server_lookup("clock.lan"));
    TEST_ASSERT_EQUAL_HEX32(0u, dns_server_lookup("absent.lan"));
    dns_server_clear();
    TEST_ASSERT_EQUAL_HEX32(0u, dns_server_lookup("printer.lan"));
}

void test_end_to_end_with_table()
{
    dns_server_add("gw.lan", 10, 0, 0, 1);
    uint8_t q[128], out[256];
    size_t qlen = make_query(q, 0xABCD, "gw.lan", 1, false);
    size_t n = dns_server_build_response(q, qlen, 60, dns_server_lookup, out, sizeof(out));
    TEST_ASSERT_EQUAL_UINT(qlen + 16, n);
    const uint8_t *a = out + qlen;
    TEST_ASSERT_EQUAL_UINT8(10, a[12]);
    TEST_ASSERT_EQUAL_UINT8(0, a[13]);
    TEST_ASSERT_EQUAL_UINT8(0, a[14]);
    TEST_ASSERT_EQUAL_UINT8(1, a[15]);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_a_record_answer);
    RUN_TEST(test_nxdomain);
    RUN_TEST(test_non_a_query_no_error);
    RUN_TEST(test_multilabel_name_reaches_resolver);
    RUN_TEST(test_malformed_guards);
    RUN_TEST(test_table_add_lookup_case_insensitive);
    RUN_TEST(test_end_to_end_with_table);
    return UNITY_END();
}
