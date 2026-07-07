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
    std::string fail_send_prefix; // a write beginning with this returns short (I/O failure)
};

int mock_send(void *c, const uint8_t *d, size_t n)
{
    Mock *m = (Mock *)c;
    if (!m->fail_send_prefix.empty() && n >= m->fail_send_prefix.size() &&
        memcmp(d, m->fail_send_prefix.data(), m->fail_send_prefix.size()) == 0)
        return (int)n - 1; // short write -> send_str() / the body send sees != n
    m->sent.append((const char *)d, n);
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

// Run a dialogue with the given scripted replies and return smtp_run's result.
static int dialogue(std::vector<std::string> replies, SmtpConfig c, SmtpMessage msg)
{
    Mock m;
    m.replies = std::move(replies);
    return smtp_run(&c, &msg, mock_send, mock_recv, &m);
}

// An overlong reply that never completes (all continuation lines) overflows the reply
// buffer; smtp_run maps that to an I/O error on the greeting read.
void test_reply_buffer_overflow()
{
    std::string huge;
    while (huge.size() < 600)
        huge += "250-continuation\r\n"; // > DETWS_SMTP_REPLY_MAX, no final line
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_IO, dialogue({huge}, base_cfg(), base_msg()));
}

// A short write on a command line (here EHLO) is an I/O error.
void test_command_send_fails()
{
    Mock m;
    m.replies = {"220 ESMTP\r\n"};
    m.fail_send_prefix = "EHLO";
    SmtpConfig c = base_cfg();
    SmtpMessage msg = base_msg();
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_IO, smtp_run(&c, &msg, mock_send, mock_recv, &m));
}

// The DATA payload send failing (short write) is an I/O error.
void test_body_send_fails()
{
    Mock m;
    m.replies = {"220 ESMTP\r\n", "250 OK\r\n", "250 Ok\r\n", "250 Ok\r\n", "354 go\r\n"};
    m.fail_send_prefix = "From:"; // only the DATA payload begins "From: <...>"
    SmtpConfig c = base_cfg();
    SmtpMessage msg = base_msg();
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_IO, smtp_run(&c, &msg, mock_send, mock_recv, &m));
}

// An AUTH secret too long to base64-encode into the line buffer overflows.
void test_auth_secret_too_long()
{
    SmtpConfig c = base_cfg();
    std::string longuser(400, 'u'); // base64 grows it past DETWS_SMTP_LINE_MAX
    c.user = longuser.c_str();
    c.pass = "pw";
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_OVERFLOW, dialogue({"220 ESMTP\r\n", "250 OK\r\n", "334 x\r\n"}, c, base_msg()));
}

// I/O failure (server hangs up) at each step of the dialogue -> SMTP_ERR_IO.
void test_io_error_at_each_step()
{
    SmtpConfig c = base_cfg();
    SmtpConfig cu = base_cfg();
    cu.user = "user";
    cu.pass = "pass";
    SmtpMessage msg = base_msg();
    // greeting ok, then hang before: EHLO / MAIL(no auth) / AUTH(user) / pass-leg / RCPT / DATA / final.
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_IO, dialogue({"220 x\r\n"}, c, msg));                // EHLO
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_IO, dialogue({"220 x\r\n", "250 OK\r\n"}, c, msg));  // MAIL FROM
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_IO, dialogue({"220 x\r\n", "250 OK\r\n"}, cu, msg)); // AUTH LOGIN
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_IO,
                          dialogue({"220 x\r\n", "250 OK\r\n", "334 a\r\n", "334 b\r\n"}, cu, msg)); // pass leg
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_IO, dialogue({"220 x\r\n", "250 OK\r\n", "250 Ok\r\n"}, c, msg)); // RCPT
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_IO,
                          dialogue({"220 x\r\n", "250 OK\r\n", "250 Ok\r\n", "250 Ok\r\n"}, c, msg)); // DATA
    TEST_ASSERT_EQUAL_INT( // final acceptance read
        SMTP_ERR_IO, dialogue({"220 x\r\n", "250 OK\r\n", "250 Ok\r\n", "250 Ok\r\n", "354 go\r\n"}, c, msg));
}

// A wrong reply code at each step -> the step-specific SmtpResult.
void test_protocol_error_at_each_step()
{
    SmtpConfig c = base_cfg();
    SmtpConfig cu = base_cfg();
    cu.user = "user";
    cu.pass = "pass";
    SmtpMessage msg = base_msg();
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_PROTOCOL, dialogue({"220 x\r\n", "500 no ehlo\r\n"}, c, msg)); // EHLO != 250
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_AUTH,
                          dialogue({"220 x\r\n", "250 OK\r\n", "500 no auth\r\n"}, cu, msg)); // AUTH != 334
    TEST_ASSERT_EQUAL_INT(
        SMTP_ERR_AUTH, dialogue({"220 x\r\n", "250 OK\r\n", "334 a\r\n", "500 bad user\r\n"}, cu, msg)); // user != 334
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_PROTOCOL,
                          dialogue({"220 x\r\n", "250 OK\r\n", "550 denied\r\n"}, c, msg)); // MAIL != 250
    TEST_ASSERT_EQUAL_INT(                                                                  // final acceptance != 250
        SMTP_ERR_PROTOCOL,
        dialogue({"220 x\r\n", "250 OK\r\n", "250 Ok\r\n", "250 Ok\r\n", "354 go\r\n", "451 rejected\r\n"}, c, msg));
}

// Each outgoing command line that is built with snprintf overflows when its variable
// field (helo / from / to) is longer than DETWS_SMTP_LINE_MAX.
void test_command_line_overflows()
{
    std::string big(300, 'z');
    SmtpConfig ch = base_cfg();
    ch.helo = big.c_str();
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_OVERFLOW, dialogue({"220 x\r\n"}, ch, base_msg())); // EHLO line

    SmtpConfig cf = base_cfg();
    cf.from = big.c_str();
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_OVERFLOW, dialogue({"220 x\r\n", "250 OK\r\n"}, cf, base_msg())); // MAIL FROM line

    SmtpMessage mt = base_msg();
    mt.to = big.c_str();
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_OVERFLOW,
                          dialogue({"220 x\r\n", "250 OK\r\n", "250 Ok\r\n"}, base_cfg(), mt)); // RCPT TO line
}

// A header field so long that the message headers do not fit -> build_message overflow.
void test_message_header_overflow()
{
    std::string bigsub(2100, 'S');
    SmtpMessage msg = base_msg();
    msg.subject = bigsub.c_str();
    TEST_ASSERT_EQUAL_INT(
        SMTP_ERR_OVERFLOW,
        dialogue({"220 x\r\n", "250 OK\r\n", "250 Ok\r\n", "250 Ok\r\n", "354 go\r\n"}, base_cfg(), msg));
}

// A CR in the body is dropped (CRLF normalization).
void test_cr_in_body_dropped()
{
    Mock m;
    m.replies = happy_replies();
    SmtpConfig c = base_cfg();
    SmtpMessage msg = base_msg();
    msg.body = "x\r\ny"; // the bare CR is stripped, the LF becomes CRLF
    TEST_ASSERT_EQUAL_INT(SMTP_OK, smtp_run(&c, &msg, mock_send, mock_recv, &m));
    TEST_ASSERT_TRUE(m.sent.find("x\r\ny\r\n") != std::string::npos);
}

// Sweep the body length across the DATA-buffer boundary so every build_message overflow
// guard (regular char, LF->CRLF, dot-stuff, trailing CRLF, terminating dot) fires at its
// own boundary length, without hard-coding exact byte counts.
void test_build_message_boundary_overflows()
{
    const std::vector<std::string> to_data = {"220 x\r\n", "250 OK\r\n", "250 Ok\r\n", "250 Ok\r\n", "354 go\r\n"};
    bool saw_overflow = false;
    for (size_t L = 1850; L <= 2060; L++)
    {
        std::string base(L, 'x');
        for (const std::string &suffix : {std::string(""), std::string("\n"), std::string("\n.")})
        {
            std::string body = base + suffix;
            SmtpMessage msg = base_msg();
            msg.body = body.c_str();
            int r = dialogue(to_data, base_cfg(), msg);
            TEST_ASSERT_NOT_EQUAL(SMTP_OK, r); // no final 250 scripted -> never succeeds
            if (r == SMTP_ERR_OVERFLOW)
                saw_overflow = true;
        }
    }
    TEST_ASSERT_TRUE(saw_overflow); // the sweep crossed the buffer boundary
}

// The host build's smtp_send() is a stub (no lwIP) that reports a connect failure.
void test_host_smtp_send_stub()
{
    SmtpConfig c = base_cfg();
    SmtpMessage msg = base_msg();
    TEST_ASSERT_EQUAL_INT(SMTP_ERR_CONNECT, smtp_send(&c, &msg));
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
    RUN_TEST(test_reply_buffer_overflow);
    RUN_TEST(test_command_send_fails);
    RUN_TEST(test_body_send_fails);
    RUN_TEST(test_auth_secret_too_long);
    RUN_TEST(test_io_error_at_each_step);
    RUN_TEST(test_protocol_error_at_each_step);
    RUN_TEST(test_command_line_overflows);
    RUN_TEST(test_message_header_overflow);
    RUN_TEST(test_cr_in_body_dropped);
    RUN_TEST(test_build_message_boundary_overflows);
    RUN_TEST(test_host_smtp_send_stub);
    return UNITY_END();
}
