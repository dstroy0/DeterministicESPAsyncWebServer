// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/xmpp: the XMPP stanza builder + minimal reader.

#include "services/xmpp/xmpp.h"
#include <string.h>
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

void test_escape(void)
{
    const char *in = "a<b&c>d'e\"f";
    char out[64];
    size_t n = dws_xmpp_escape(in, strlen(in), out, sizeof(out));
    TEST_ASSERT_EQUAL_STRING("a&lt;b&amp;c&gt;d&apos;e&quot;f", out);
    TEST_ASSERT_EQUAL_size_t(strlen("a&lt;b&amp;c&gt;d&apos;e&quot;f"), n);
    // Overflow -> 0.
    TEST_ASSERT_EQUAL_size_t(0, dws_xmpp_escape(in, strlen(in), out, 5));
}

void test_message(void)
{
    char out[128];
    size_t n = dws_xmpp_message("juliet@example.com", nullptr, "chat", "Hello & <world>", out, sizeof(out));
    TEST_ASSERT_EQUAL_STRING(
        "<message to=\"juliet@example.com\" type=\"chat\"><body>Hello &amp; &lt;world&gt;</body></message>", out);
    TEST_ASSERT_TRUE(n > 0);
}

void test_presence(void)
{
    char out[64];
    dws_xmpp_presence(nullptr, out, sizeof(out));
    TEST_ASSERT_EQUAL_STRING("<presence/>", out);
    dws_xmpp_presence("unavailable", out, sizeof(out));
    TEST_ASSERT_EQUAL_STRING("<presence type=\"unavailable\"/>", out);
}

void test_iq(void)
{
    char out[64];
    dws_xmpp_iq("get", "1", "<query xmlns='jabber:iq:roster'/>", out, sizeof(out));
    TEST_ASSERT_EQUAL_STRING("<iq type=\"get\" id=\"1\"><query xmlns='jabber:iq:roster'/></iq>", out);
}

void test_stanza_name(void)
{
    char name[32];
    const char *m = "<message to='x'><body>hi</body></message>";
    TEST_ASSERT_EQUAL_size_t(7, dws_xmpp_stanza_name(m, strlen(m), name, sizeof(name)));
    TEST_ASSERT_EQUAL_STRING("message", name);
    const char *s = "<?xml version='1.0'?><stream:stream to='a'>";
    dws_xmpp_stanza_name(s, strlen(s), name, sizeof(name));
    TEST_ASSERT_EQUAL_STRING("stream:stream", name);
}

void test_attr(void)
{
    char val[64];
    const char *m = "<message to=\"a@b\" from=\"c@d\" type=\"chat\">body</message>";
    TEST_ASSERT_EQUAL_size_t(3, dws_xmpp_attr(m, strlen(m), "to", val, sizeof(val)));
    TEST_ASSERT_EQUAL_STRING("a@b", val);
    dws_xmpp_attr(m, strlen(m), "from", val, sizeof(val));
    TEST_ASSERT_EQUAL_STRING("c@d", val);
    dws_xmpp_attr(m, strlen(m), "type", val, sizeof(val));
    TEST_ASSERT_EQUAL_STRING("chat", val);
    // Absent attribute -> 0.
    TEST_ASSERT_EQUAL_size_t(0, dws_xmpp_attr(m, strlen(m), "id", val, sizeof(val)));
}

void test_escape_all_entities_and_overflow(void)
{
    char buf[64];
    // Every escapable character plus a normal one exercises each switch case in put_escaped.
    TEST_ASSERT_TRUE(dws_xmpp_escape("a&<>'\"b", 7, buf, sizeof(buf)) > 0);
    TEST_ASSERT_EQUAL_STRING("a&amp;&lt;&gt;&apos;&quot;b", buf);
    // Null in / out fail closed.
    TEST_ASSERT_EQUAL_size_t(0, dws_xmpp_escape(nullptr, 3, buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_size_t(0, dws_xmpp_escape("x", 1, nullptr, sizeof(buf)));
    // Overflow on an escaped char (needs 5 bytes) and on a plain char (both put paths' false side).
    TEST_ASSERT_EQUAL_size_t(0, dws_xmpp_escape("&", 1, buf, 3));
    TEST_ASSERT_EQUAL_size_t(0, dws_xmpp_escape("abcd", 4, buf, 3));
}

void test_builders_overflow_fail_closed(void)
{
    char tiny[8]; // every builder's put-chain fails partway -> finish(!ok) returns 0
    TEST_ASSERT_EQUAL_size_t(0, dws_xmpp_stream_open("a", "b", tiny, sizeof(tiny)));
    TEST_ASSERT_EQUAL_size_t(0, dws_xmpp_message("to", "from", "chat", "hi", tiny, sizeof(tiny)));
    TEST_ASSERT_EQUAL_size_t(0, dws_xmpp_presence("available", tiny, sizeof(tiny)));
    TEST_ASSERT_EQUAL_size_t(0, dws_xmpp_iq("get", "id1", "<query/>", tiny, sizeof(tiny)));
    // Header fits but the body/child does not (the optional-block put fails).
    char mid[20];
    TEST_ASSERT_EQUAL_size_t(0, dws_xmpp_message("t", nullptr, nullptr, "aBodyThatWillNotFit", mid, sizeof(mid)));
    TEST_ASSERT_EQUAL_size_t(0, dws_xmpp_iq("t", nullptr, "<aChildThatWillNotFit/>", mid, sizeof(mid)));
}

void test_builders_omit_optional_and_null_attrs(void)
{
    // body/child null skip the optional block; null attr values skip put_attr (its `!value` true side).
    char buf[128];
    TEST_ASSERT_TRUE(dws_xmpp_message("to", nullptr, nullptr, nullptr, buf, sizeof(buf)) > 0);
    TEST_ASSERT_NOT_NULL(strstr(buf, "<message"));
    TEST_ASSERT_NULL(strstr(buf, "<body>"));
    TEST_ASSERT_TRUE(dws_xmpp_iq("set", nullptr, nullptr, buf, sizeof(buf)) > 0);
    TEST_ASSERT_NULL(strstr(buf, "<child"));
    TEST_ASSERT_TRUE(dws_xmpp_presence(nullptr, buf, sizeof(buf)) > 0); // null type -> bare <presence/>
    TEST_ASSERT_TRUE(dws_xmpp_stream_open(nullptr, nullptr, buf, sizeof(buf)) > 0);
}

void test_stanza_name_edges(void)
{
    char out[32];
    // Each terminator: '>', '/', space, tab, newline.
    TEST_ASSERT_TRUE(dws_xmpp_stanza_name("<iq>", 4, out, sizeof(out)) > 0);
    TEST_ASSERT_EQUAL_STRING("iq", out);
    TEST_ASSERT_TRUE(dws_xmpp_stanza_name("<presence/>", 11, out, sizeof(out)) > 0);
    TEST_ASSERT_EQUAL_STRING("presence", out);
    TEST_ASSERT_TRUE(dws_xmpp_stanza_name("<message x>", 11, out, sizeof(out)) > 0);
    TEST_ASSERT_EQUAL_STRING("message", out);
    dws_xmpp_stanza_name("<tag\tx>", 7, out, sizeof(out)); // tab terminator
    dws_xmpp_stanza_name("<tag\nx>", 7, out, sizeof(out)); // newline terminator
    // Skips declaration / comment / close tag; no start tag; null; overflow.
    TEST_ASSERT_EQUAL_size_t(0, dws_xmpp_stanza_name("<?xml?>", 7, out, sizeof(out)));
    TEST_ASSERT_EQUAL_size_t(0, dws_xmpp_stanza_name("<!--c-->", 8, out, sizeof(out)));
    TEST_ASSERT_EQUAL_size_t(0, dws_xmpp_stanza_name("</close>", 8, out, sizeof(out)));
    TEST_ASSERT_EQUAL_size_t(0, dws_xmpp_stanza_name("no tag", 6, out, sizeof(out)));
    TEST_ASSERT_EQUAL_size_t(0, dws_xmpp_stanza_name(nullptr, 5, out, sizeof(out)));
    TEST_ASSERT_EQUAL_size_t(0, dws_xmpp_stanza_name("<message>", 9, out, 3)); // cap too small
}

void test_attr_edges(void)
{
    char out[32];
    // Single-quoted value + the leading-space substring guard (must not match 'to' inside 'xto').
    TEST_ASSERT_TRUE(dws_xmpp_attr("<m xto='no' to='yes'>", 21, "to", out, sizeof(out)) > 0);
    TEST_ASSERT_EQUAL_STRING("yes", out);
    // Unquoted value, null args, and value overflow all fail closed.
    TEST_ASSERT_EQUAL_size_t(0, dws_xmpp_attr("<m to=x>", 8, "to", out, sizeof(out)));
    TEST_ASSERT_EQUAL_size_t(0, dws_xmpp_attr(nullptr, 5, "to", out, sizeof(out)));
    TEST_ASSERT_EQUAL_size_t(0, dws_xmpp_attr("<m to='x'>", 10, nullptr, out, sizeof(out)));
    TEST_ASSERT_EQUAL_size_t(0, dws_xmpp_attr("<m to='alice'>", 14, "to", out, 3)); // cap too small
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_escape);
    RUN_TEST(test_message);
    RUN_TEST(test_presence);
    RUN_TEST(test_iq);
    RUN_TEST(test_stanza_name);
    RUN_TEST(test_attr);
    RUN_TEST(test_escape_all_entities_and_overflow);
    RUN_TEST(test_builders_overflow_fail_closed);
    RUN_TEST(test_builders_omit_optional_and_null_attrs);
    RUN_TEST(test_stanza_name_edges);
    RUN_TEST(test_attr_edges);
    return UNITY_END();
}
