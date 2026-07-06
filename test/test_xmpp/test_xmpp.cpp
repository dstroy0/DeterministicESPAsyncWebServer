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
    size_t n = detws_xmpp_escape(in, strlen(in), out, sizeof(out));
    TEST_ASSERT_EQUAL_STRING("a&lt;b&amp;c&gt;d&apos;e&quot;f", out);
    TEST_ASSERT_EQUAL_size_t(strlen("a&lt;b&amp;c&gt;d&apos;e&quot;f"), n);
    // Overflow -> 0.
    TEST_ASSERT_EQUAL_size_t(0, detws_xmpp_escape(in, strlen(in), out, 5));
}

void test_message(void)
{
    char out[128];
    size_t n = detws_xmpp_message("juliet@example.com", nullptr, "chat", "Hello & <world>", out, sizeof(out));
    TEST_ASSERT_EQUAL_STRING(
        "<message to=\"juliet@example.com\" type=\"chat\"><body>Hello &amp; &lt;world&gt;</body></message>", out);
    TEST_ASSERT_TRUE(n > 0);
}

void test_presence(void)
{
    char out[64];
    detws_xmpp_presence(nullptr, out, sizeof(out));
    TEST_ASSERT_EQUAL_STRING("<presence/>", out);
    detws_xmpp_presence("unavailable", out, sizeof(out));
    TEST_ASSERT_EQUAL_STRING("<presence type=\"unavailable\"/>", out);
}

void test_iq(void)
{
    char out[64];
    detws_xmpp_iq("get", "1", "<query xmlns='jabber:iq:roster'/>", out, sizeof(out));
    TEST_ASSERT_EQUAL_STRING("<iq type=\"get\" id=\"1\"><query xmlns='jabber:iq:roster'/></iq>", out);
}

void test_stanza_name(void)
{
    char name[32];
    const char *m = "<message to='x'><body>hi</body></message>";
    TEST_ASSERT_EQUAL_size_t(7, detws_xmpp_stanza_name(m, strlen(m), name, sizeof(name)));
    TEST_ASSERT_EQUAL_STRING("message", name);
    const char *s = "<?xml version='1.0'?><stream:stream to='a'>";
    detws_xmpp_stanza_name(s, strlen(s), name, sizeof(name));
    TEST_ASSERT_EQUAL_STRING("stream:stream", name);
}

void test_attr(void)
{
    char val[64];
    const char *m = "<message to=\"a@b\" from=\"c@d\" type=\"chat\">body</message>";
    TEST_ASSERT_EQUAL_size_t(3, detws_xmpp_attr(m, strlen(m), "to", val, sizeof(val)));
    TEST_ASSERT_EQUAL_STRING("a@b", val);
    detws_xmpp_attr(m, strlen(m), "from", val, sizeof(val));
    TEST_ASSERT_EQUAL_STRING("c@d", val);
    detws_xmpp_attr(m, strlen(m), "type", val, sizeof(val));
    TEST_ASSERT_EQUAL_STRING("chat", val);
    // Absent attribute -> 0.
    TEST_ASSERT_EQUAL_size_t(0, detws_xmpp_attr(m, strlen(m), "id", val, sizeof(val)));
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
    return UNITY_END();
}
