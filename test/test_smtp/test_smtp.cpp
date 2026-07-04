// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the SMTP client dialogue engine (services/smtp/smtp_run). A scripted
// mock server returns one reply per recv (non-pipelined SMTP) and captures the client's
// commands, so the full RFC 5321 exchange - greeting, EHLO, AUTH LOGIN, MAIL/RCPT/DATA,
// dot-stuffing, the terminating "." - is verified without any network or TLS.

#include "services/smtp/smtp.h"
#include <string.h>
#include <string>
#include <unity.h>
#include <vector>

namespace
{
struct Mock
{
    std::vector<std::string> replies; // server -> client, one per recv turn
    size_t idx = 0;
    std::string sent;     // everything the client wrote
    bool dribble = false; // return replies one byte at a time (exercise the accumulate loop)
    size_t dribble_pos = 0;
};

int mock_send(void *c, const uint8_t *d, size_t n)
{
    ((Mock *)c)->sent.append((const char *)d, n);
    return (int)n;
}

int mock_recv(void *c, uint8_t *b, size_t cap)
{
    Mock *m = (Mock *)c;
    if (m->idx >= m->replies.size())
        return -1; // no more scripted data -> I/O error
    const std::string &r = m->replies[m->idx];
    if (m->dribble)
    {
        if (m->dribble_pos >= r.size())
        {
            m->idx++;
            m->dribble_pos = 0;
            if (m->idx >= m->replies.size())
                return -1;
        }
        b[0] = (uint8_t)m->replies[m->idx][m->dribble_pos++];
        if (m->dribble_pos >= m->replies[m->idx].size())
        {
            m->idx++;
            m->dribble_pos = 0;
        }
        return 1;
    }
    size_t n = r.size() < cap ? r.size() : cap;
    memcpy(b, r.data(), n);
    m->idx++;
    return (int)n;
}

// A standard successful conversation, with the message-acceptance reply configurable.
std::vector<std::string> happy_replies()
{
    return {"220 mail.example.net ESMTP\r\n",
            "250-mail.example.net\r\n250 OK\r\n",
            "250 2.1.0 Ok\r\n",
            "250 2.1.5 Ok\r\n",
            "354 End data with <CR><LF>.<CR><LF>\r\n",
            "250 2.0.0 Ok: queued\r\n",
            "221 2.0.0 Bye\r\n"};
}
SmtpConfig base_cfg()
{
    SmtpConfig c;
    c.host = "mail.example.net";
    c.port = 25;
    c.tls = false;
    c.user = nullptr;
    c.pass = nullptr;
    c.from = "device@example.net";
    c.helo = "esp32";
    return c;
}
SmtpMessage base_msg()
{
    SmtpMessage m;
    m.to = "ops@example.net";
    m.subject = "Alert";
    m.body = "sensor tripped";
    return m;
}
} // namespace

void setUp()
{
}
void tearDown()
{
}

void test_happy_path_no_auth()
{
    Mock m;
    m.replies = happy_replies();
    SmtpConfig c = base_cfg();
    SmtpMessage msg = base_msg();
    TEST_ASSERT_EQUAL_INT(SMTP_OK, smtp_run(&c, &msg, mock_send, mock_recv, &m));
    // Commands, in order.
    TEST_ASSERT_TRUE(m.sent.find("EHLO esp32\r\n") != std::string::npos);
    TEST_ASSERT_TRUE(m.sent.find("MAIL FROM:<device@example.net>\r\n") != std::string::npos);
    TEST_ASSERT_TRUE(m.sent.find("RCPT TO:<ops@example.net>\r\n") != std::string::npos);
    TEST_ASSERT_TRUE(m.sent.find("DATA\r\n") != std::string::npos);
    TEST_ASSERT_TRUE(m.sent.find("QUIT\r\n") != std::string::npos);
    // Message headers + body + terminator.
    TEST_ASSERT_TRUE(m.sent.find("Subject: Alert\r\n") != std::string::npos);
    TEST_ASSERT_TRUE(m.sent.find("To: <ops@example.net>\r\n") != std::string::npos);
    TEST_ASSERT_TRUE(m.sent.find("sensor tripped\r\n") != std::string::npos);
    TEST_ASSERT_TRUE(m.sent.find("\r\n.\r\n") != std::string::npos); // end-of-DATA
    // No AUTH when no user configured.
    TEST_ASSERT_TRUE(m.sent.find("AUTH") == std::string::npos);
}

void test_auth_login()
{
    Mock m;
    m.replies = {"220 ESMTP\r\n", "250 OK\r\n", "334 VXNlcm5hbWU6\r\n", "334 UGFzc3dvcmQ6\r\n", "235 2.7.0 Ok\r\n",
                 "250 Ok\r\n",    "250 Ok\r\n", "354 go\r\n",           "250 queued\r\n",       "221 Bye\r\n"};
    SmtpConfig c = base_cfg();
    c.user = "user";
    c.pass = "pass";
    SmtpMessage msg = base_msg();
    TEST_ASSERT_EQUAL_INT(SMTP_OK, smtp_run(&c, &msg, mock_send, mock_recv, &m));
    TEST_ASSERT_TRUE(m.sent.find("AUTH LOGIN\r\n") != std::string::npos);
    TEST_ASSERT_TRUE(m.sent.find("dXNlcg==\r\n") != std::string::npos); // base64("user")
    TEST_ASSERT_TRUE(m.sent.find("cGFzcw==\r\n") != std::string::npos); // base64("pass")
}

void test_auth_rejected()
{
    Mock m;
    m.replies = {"220 ESMTP\r\n", "250 OK\r\n", "334 x\r\n", "334 y\r\n", "535 5.7.8 auth failed\r\n"};
    SmtpConfig c = base_cfg();
    c.user = "user";
    c.pass = "wrong";
    SmtpMessage msg = base_msg();
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_AUTH, smtp_run(&c, &msg, mock_send, mock_recv, &m));
}

void test_greeting_not_ready()
{
    Mock m;
    m.replies = {"554 no service\r\n"};
    SmtpConfig c = base_cfg();
    SmtpMessage msg = base_msg();
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_PROTOCOL, smtp_run(&c, &msg, mock_send, mock_recv, &m));
}

void test_rcpt_rejected()
{
    Mock m;
    m.replies = {"220 ESMTP\r\n", "250 OK\r\n", "250 Ok\r\n", "550 5.1.1 no such user\r\n"};
    SmtpConfig c = base_cfg();
    SmtpMessage msg = base_msg();
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_PROTOCOL, smtp_run(&c, &msg, mock_send, mock_recv, &m));
}

void test_data_refused()
{
    Mock m;
    m.replies = {"220 ESMTP\r\n", "250 OK\r\n", "250 Ok\r\n", "250 Ok\r\n", "451 try later\r\n"};
    SmtpConfig c = base_cfg();
    SmtpMessage msg = base_msg();
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_PROTOCOL, smtp_run(&c, &msg, mock_send, mock_recv, &m));
}

void test_dot_stuffing()
{
    Mock m;
    m.replies = happy_replies();
    SmtpConfig c = base_cfg();
    SmtpMessage msg = base_msg();
    msg.body = "line1\n.hidden\n..two dots\nlast"; // lines starting with '.' must be stuffed
    TEST_ASSERT_EQUAL_INT(SMTP_OK, smtp_run(&c, &msg, mock_send, mock_recv, &m));
    TEST_ASSERT_TRUE(m.sent.find("..hidden\r\n") != std::string::npos);    // '.' -> '..'
    TEST_ASSERT_TRUE(m.sent.find("...two dots\r\n") != std::string::npos); // '..' -> '...'
    TEST_ASSERT_TRUE(m.sent.find("last\r\n.\r\n") != std::string::npos);   // real terminator intact
}

void test_multiline_reply_and_lf_body()
{
    Mock m;
    m.replies = happy_replies(); // EHLO reply is multi-line ("250-...\r\n250 OK\r\n")
    SmtpConfig c = base_cfg();
    SmtpMessage msg = base_msg();
    msg.body = "a\nb"; // bare LF must be normalized to CRLF
    TEST_ASSERT_EQUAL_INT(SMTP_OK, smtp_run(&c, &msg, mock_send, mock_recv, &m));
    TEST_ASSERT_TRUE(m.sent.find("a\r\nb\r\n") != std::string::npos);
}

void test_partial_reads_dribble()
{
    Mock m;
    m.replies = happy_replies();
    m.dribble = true; // deliver every reply one byte at a time
    SmtpConfig c = base_cfg();
    SmtpMessage msg = base_msg();
    TEST_ASSERT_EQUAL_INT(SMTP_OK, smtp_run(&c, &msg, mock_send, mock_recv, &m));
}

void test_missing_required_arg()
{
    Mock m;
    m.replies = happy_replies();
    SmtpConfig c = base_cfg();
    c.from = ""; // empty sender
    SmtpMessage msg = base_msg();
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_ARG, smtp_run(&c, &msg, mock_send, mock_recv, &m));
}

void test_io_error_when_server_hangs()
{
    Mock m; // no replies scripted -> recv returns -1 on the greeting read
    SmtpConfig c = base_cfg();
    SmtpMessage msg = base_msg();
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_IO, smtp_run(&c, &msg, mock_send, mock_recv, &m));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_happy_path_no_auth);
    RUN_TEST(test_auth_login);
    RUN_TEST(test_auth_rejected);
    RUN_TEST(test_greeting_not_ready);
    RUN_TEST(test_rcpt_rejected);
    RUN_TEST(test_data_refused);
    RUN_TEST(test_dot_stuffing);
    RUN_TEST(test_multiline_reply_and_lf_body);
    RUN_TEST(test_partial_reads_dribble);
    RUN_TEST(test_missing_required_arg);
    RUN_TEST(test_io_error_when_server_hangs);
    return UNITY_END();
}
