// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the hash-chained audit log (services/audit_log). Verify the
// chain detects any tampering (message or hash), that the moving anchor keeps the
// retained window verifiable after the ring wraps, and that the sink + JSON
// rendering behave. SHA-256 is exercised through ssh_sha256.

#include "services/audit_log/audit_log.h"
#include <string.h>
#include <unity.h>

void setUp()
{
    detws_audit_reset();
    detws_audit_set_sink(nullptr);
}
void tearDown()
{
}

// Tamper helper: the test owns the storage, so cast away const to corrupt it.
static DetwsAuditEntry *mutable_at(uint16_t i)
{
    return const_cast<DetwsAuditEntry *>(detws_audit_at(i));
}

void test_append_assigns_monotonic_seq()
{
    TEST_ASSERT_EQUAL_UINT32(1, detws_audit_append(DetwsAuditCat::DETWS_AUDIT_AUTH, "login alice"));
    TEST_ASSERT_EQUAL_UINT32(2, detws_audit_append(DetwsAuditCat::DETWS_AUDIT_AUTH_FAIL, "bad password bob"));
    TEST_ASSERT_EQUAL_UINT32(3, detws_audit_append(DetwsAuditCat::DETWS_AUDIT_CONFIG, "set http_port=80"));
    TEST_ASSERT_EQUAL_UINT16(3, detws_audit_count());
    TEST_ASSERT_EQUAL_STRING("login alice", detws_audit_at(0)->msg);
    TEST_ASSERT_EQUAL_STRING("set http_port=80", detws_audit_at(2)->msg);
    TEST_ASSERT_NULL(detws_audit_at(3));
}

void test_chain_verifies_when_untouched()
{
    for (int i = 0; i < 10; i++)
        detws_audit_append(DetwsAuditCat::DETWS_AUDIT_ACCESS, "GET /resource");
    uint32_t broken = 999;
    TEST_ASSERT_TRUE(detws_audit_verify(&broken));
    TEST_ASSERT_EQUAL_UINT32(999, broken); // untouched on success
}

void test_tampered_message_breaks_chain()
{
    for (int i = 0; i < 6; i++)
        detws_audit_append(DetwsAuditCat::DETWS_AUDIT_SYSTEM, "tick");
    // Corrupt record #4's message in place (hash now mismatches its fields).
    DetwsAuditEntry *e = mutable_at(3);
    strcpy(e->msg, "EVIL");
    uint32_t broken = 0;
    TEST_ASSERT_FALSE(detws_audit_verify(&broken));
    TEST_ASSERT_EQUAL_UINT32(4, broken); // seq of the first failing record
}

void test_tampered_hash_breaks_chain()
{
    for (int i = 0; i < 5; i++)
        detws_audit_append(DetwsAuditCat::DETWS_AUDIT_SYSTEM, "tick");
    mutable_at(2)->hash[0] ^= 0xFF; // flip a hash bit
    uint32_t broken = 0;
    TEST_ASSERT_FALSE(detws_audit_verify(&broken));
    TEST_ASSERT_EQUAL_UINT32(3, broken);
}

// Re-pointing a record to a different category must also break the chain.
void test_tampered_category_breaks_chain()
{
    for (int i = 0; i < 4; i++)
        detws_audit_append(DetwsAuditCat::DETWS_AUDIT_ACCESS, "ok");
    mutable_at(1)->category = DetwsAuditCat::DETWS_AUDIT_ADMIN;
    TEST_ASSERT_FALSE(detws_audit_verify(nullptr));
}

void test_ring_evicts_oldest_and_still_verifies()
{
    const int extra = 8;
    for (int i = 0; i < DETWS_AUDIT_LOG_ENTRIES + extra; i++)
        detws_audit_append(DetwsAuditCat::DETWS_AUDIT_SYSTEM, "msg");
    // Ring is capped; oldest seq advanced past the evicted records.
    TEST_ASSERT_EQUAL_UINT16(DETWS_AUDIT_LOG_ENTRIES, detws_audit_count());
    TEST_ASSERT_EQUAL_UINT32((uint32_t)(extra + 1), detws_audit_at(0)->seq);
    TEST_ASSERT_EQUAL_UINT32((uint32_t)(DETWS_AUDIT_LOG_ENTRIES + extra),
                             detws_audit_at((uint16_t)(detws_audit_count() - 1))->seq);
    // The moving anchor keeps the retained window a complete, verifiable chain.
    TEST_ASSERT_TRUE(detws_audit_verify(nullptr));
}

// After the ring wraps, tampering the OLDEST retained record is still caught -
// the moving anchor makes even the first retained record verifiable.
void test_tamper_after_wrap_detected_at_oldest()
{
    for (int i = 0; i < DETWS_AUDIT_LOG_ENTRIES + 5; i++)
        detws_audit_append(DetwsAuditCat::DETWS_AUDIT_SYSTEM, "x");
    DetwsAuditEntry *oldest = mutable_at(0);
    oldest->msg[0] = (oldest->msg[0] == 'x') ? 'y' : 'x';
    uint32_t broken = 0;
    TEST_ASSERT_FALSE(detws_audit_verify(&broken));
    TEST_ASSERT_EQUAL_UINT32(oldest->seq, broken);
}

void test_reset_clears_everything()
{
    detws_audit_append(DetwsAuditCat::DETWS_AUDIT_AUTH, "a");
    detws_audit_append(DetwsAuditCat::DETWS_AUDIT_AUTH, "b");
    detws_audit_reset();
    TEST_ASSERT_EQUAL_UINT16(0, detws_audit_count());
    TEST_ASSERT_TRUE(detws_audit_verify(nullptr)); // empty chain is trivially intact
    // Sequence restarts at 1 after reset.
    TEST_ASSERT_EQUAL_UINT32(1, detws_audit_append(DetwsAuditCat::DETWS_AUDIT_SYSTEM, "fresh"));
}

// Sink receives every record at append time (the durable-forwarding path).
static int s_sink_calls = 0;
static uint32_t s_sink_last_seq = 0;
static char s_sink_last_msg[DETWS_AUDIT_MSG_LEN];
static void test_sink(const DetwsAuditEntry *e)
{
    s_sink_calls++;
    s_sink_last_seq = e->seq;
    strncpy(s_sink_last_msg, e->msg, sizeof(s_sink_last_msg) - 1);
}

void test_sink_receives_each_record()
{
    s_sink_calls = 0;
    detws_audit_set_sink(test_sink);
    detws_audit_append(DetwsAuditCat::DETWS_AUDIT_AUTH, "one");
    detws_audit_append(DetwsAuditCat::DETWS_AUDIT_AUTH, "two");
    detws_audit_append(DetwsAuditCat::DETWS_AUDIT_AUTH, "three");
    TEST_ASSERT_EQUAL_INT(3, s_sink_calls);
    TEST_ASSERT_EQUAL_UINT32(3, s_sink_last_seq);
    TEST_ASSERT_EQUAL_STRING("three", s_sink_last_msg);
}

void test_format_and_dump_json()
{
    detws_audit_append(DetwsAuditCat::DETWS_AUDIT_AUTH, "login \"alice\"\n"); // forces JSON escaping
    char one[256];
    int n = detws_audit_format(detws_audit_at(0), one, sizeof(one));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_NOT_NULL(strstr(one, "\"seq\":1"));
    TEST_ASSERT_NOT_NULL(strstr(one, "\"cat\":\"auth\""));
    TEST_ASSERT_NOT_NULL(strstr(one, "\\\"alice\\\"")); // quote escaped
    TEST_ASSERT_NOT_NULL(strstr(one, "\\n"));           // newline escaped

    char doc[512];
    int dn = detws_audit_dump_json(doc, sizeof(doc));
    TEST_ASSERT_TRUE(dn > 0);
    TEST_ASSERT_NOT_NULL(strstr(doc, "\"intact\":true"));
    TEST_ASSERT_NOT_NULL(strstr(doc, "\"count\":1"));
}

void test_dump_json_reports_broken_chain()
{
    for (int i = 0; i < 4; i++)
        detws_audit_append(DetwsAuditCat::DETWS_AUDIT_SYSTEM, "ok");
    mutable_at(2)->msg[0] ^= 0xFF;
    char doc[1024]; // 4 records * (~64 hex hash + fields)
    TEST_ASSERT_TRUE(detws_audit_dump_json(doc, sizeof(doc)) > 0);
    TEST_ASSERT_NOT_NULL(strstr(doc, "\"intact\":false"));
    TEST_ASSERT_NOT_NULL(strstr(doc, "\"first_broken\":3"));
}

void test_format_fails_closed_on_small_buffer()
{
    detws_audit_append(DetwsAuditCat::DETWS_AUDIT_AUTH, "some message here");
    char tiny[8];
    TEST_ASSERT_EQUAL_INT(0, detws_audit_format(detws_audit_at(0), tiny, sizeof(tiny)));
}

// A NULL message stores an empty string; each category renders its name.
void test_null_msg_and_categories()
{
    detws_audit_append(DetwsAuditCat::DETWS_AUDIT_SYSTEM, nullptr);
    TEST_ASSERT_EQUAL_STRING("", detws_audit_at(0)->msg);

    detws_audit_reset();
    detws_audit_append(DetwsAuditCat::DETWS_AUDIT_AUTH_FAIL, "a");
    detws_audit_append(DetwsAuditCat::DETWS_AUDIT_ACCESS, "b");
    detws_audit_append(DetwsAuditCat::DETWS_AUDIT_CONFIG, "c");
    detws_audit_append(DetwsAuditCat::DETWS_AUDIT_ADMIN, "d");
    char doc[1024];
    TEST_ASSERT_TRUE(detws_audit_dump_json(doc, sizeof(doc)) > 0);
    TEST_ASSERT_NOT_NULL(strstr(doc, "\"cat\":\"auth_fail\""));
    TEST_ASSERT_NOT_NULL(strstr(doc, "\"cat\":\"access\""));
    TEST_ASSERT_NOT_NULL(strstr(doc, "\"cat\":\"config\""));
    TEST_ASSERT_NOT_NULL(strstr(doc, "\"cat\":\"admin\""));
}

// Every JSON escape branch: backslash, tab, CR, and a \u00XX control char.
void test_json_escape_all_chars()
{
    detws_audit_append(DetwsAuditCat::DETWS_AUDIT_SYSTEM, "a\\b\tc\r\x01");
    char buf[256];
    TEST_ASSERT_TRUE(detws_audit_format(detws_audit_at(0), buf, sizeof(buf)) > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "\\\\"));    // backslash
    TEST_ASSERT_NOT_NULL(strstr(buf, "\\t"));     // tab
    TEST_ASSERT_NOT_NULL(strstr(buf, "\\r"));     // carriage return
    TEST_ASSERT_NOT_NULL(strstr(buf, "\\u0001")); // control char
}

// detws_audit_format fails closed for a null arg and for every buffer size below
// the full length (walking cap across each stage: head, escape, mid, hash, close).
void test_format_fails_closed_all_stages()
{
    detws_audit_append(DetwsAuditCat::DETWS_AUDIT_AUTH, "hi\tthere");
    const DetwsAuditEntry *e = detws_audit_at(0);
    char full[256];
    int flen = detws_audit_format(e, full, sizeof(full));
    TEST_ASSERT_TRUE(flen > 0);
    TEST_ASSERT_EQUAL_INT(0, detws_audit_format(e, full, 0));
    TEST_ASSERT_EQUAL_INT(0, detws_audit_format(nullptr, full, sizeof(full)));
    for (int cap = 1; cap < flen; cap++)
    {
        char small[256];
        TEST_ASSERT_EQUAL_INT(0, detws_audit_format(e, small, (size_t)cap));
    }
}

// detws_audit_dump_json fails closed for a null arg and every under-sized buffer
// (head, the inter-record comma, a truncated entry, and the closing bytes).
void test_dump_fails_closed_all_stages()
{
    for (int i = 0; i < 3; i++)
        detws_audit_append(DetwsAuditCat::DETWS_AUDIT_SYSTEM, "rec");
    char full[1024];
    int flen = detws_audit_dump_json(full, sizeof(full));
    TEST_ASSERT_TRUE(flen > 0);
    TEST_ASSERT_EQUAL_INT(0, detws_audit_dump_json(full, 0));
    TEST_ASSERT_EQUAL_INT(0, detws_audit_dump_json(nullptr, sizeof(full)));
    for (int cap = 1; cap < flen; cap++)
    {
        char small[1024];
        TEST_ASSERT_EQUAL_INT(0, detws_audit_dump_json(small, (size_t)cap));
    }
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_append_assigns_monotonic_seq);
    RUN_TEST(test_chain_verifies_when_untouched);
    RUN_TEST(test_tampered_message_breaks_chain);
    RUN_TEST(test_tampered_hash_breaks_chain);
    RUN_TEST(test_tampered_category_breaks_chain);
    RUN_TEST(test_ring_evicts_oldest_and_still_verifies);
    RUN_TEST(test_tamper_after_wrap_detected_at_oldest);
    RUN_TEST(test_reset_clears_everything);
    RUN_TEST(test_sink_receives_each_record);
    RUN_TEST(test_format_and_dump_json);
    RUN_TEST(test_dump_json_reports_broken_chain);
    RUN_TEST(test_format_fails_closed_on_small_buffer);
    RUN_TEST(test_null_msg_and_categories);
    RUN_TEST(test_json_escape_all_chars);
    RUN_TEST(test_format_fails_closed_all_stages);
    RUN_TEST(test_dump_fails_closed_all_stages);
    return UNITY_END();
}
