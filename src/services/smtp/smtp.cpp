// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file smtp.cpp
 * @brief Outbound SMTP client (RFC 5321) - implementation. See smtp.h for the model.
 *
 * smtp_run() is the pure dialogue engine (host-testable via the send/recv seam);
 * smtp_send() binds it to the real transport on Arduino (det_client, +det_tls csess).
 */

#include "services/smtp/smtp.h"
#include "ServerConfig.h"
#include "services/clock.h" // dwsdelay

#if DETWS_ENABLE_SMTP

#include "network_drivers/presentation/base64/base64.h"
#include <stdio.h>  // snprintf
#include <string.h> // strlen, memcmp

namespace
{
// Send an entire C string; returns true only if every byte went out.
bool send_str(SmtpSendFn send, void *ctx, const char *s)
{
    size_t n = strnlen(s, DETWS_SMTP_LINE_MAX + 1);
    return n == 0 || send(ctx, (const uint8_t *)s, n) == (int)n;
}

// Is buf[0..len) a complete SMTP reply? A reply is one or more CRLF lines that share a
// 3-digit code; the FINAL line has a space (or nothing) after the code, continuation
// lines have '-'. On a complete reply, set *code to the 3-digit value and return true.
bool reply_complete(const char *buf, size_t len, int *code)
{
    size_t start = 0;
    for (size_t i = 0; i + 1 < len; i++)
    {
        if (buf[i] != '\r' || buf[i + 1] != '\n')
            continue;
        size_t line_len = i - start; // excludes the CRLF
        if (line_len >= 3 && buf[start] >= '0' && buf[start] <= '9' && buf[start + 1] >= '0' && buf[start + 1] <= '9' &&
            buf[start + 2] >= '0' && buf[start + 2] <= '9')
        {
            bool final_line = (line_len == 3) || buf[start + 3] == ' ';
            if (final_line)
            {
                *code = (buf[start] - '0') * 100 + (buf[start + 1] - '0') * 10 + (buf[start + 2] - '0');
                return true;
            }
        }
        start = i + 2; // next line begins after the CRLF
    }
    return false; // no final line yet - need more bytes
}

// Read one (possibly multi-line) reply into a stack buffer and return its code.
SmtpResult read_reply(SmtpRecvFn recv, void *ctx, int *code)
{
    char buf[DETWS_SMTP_REPLY_MAX];
    size_t len = 0;
    for (;;)
    {
        if (reply_complete(buf, len, code))
            return SmtpResult::SMTP_OK;
        if (len >= sizeof(buf))
            return SmtpResult::SMTP_ERR_OVERFLOW;
        int n = recv(ctx, (uint8_t *)buf + len, sizeof(buf) - len);
        if (n <= 0)
            return SmtpResult::SMTP_ERR_IO;
        len += (size_t)n;
    }
}

// Send one command line (already CRLF-terminated) and return the reply code, or a
// negative ::SmtpResult on an I/O failure.
int command(SmtpSendFn send, SmtpRecvFn recv, void *ctx, const char *line)
{
    if (!send_str(send, ctx, line))
        return (int)SmtpResult::SMTP_ERR_IO;
    int code = 0;
    SmtpResult r = read_reply(recv, ctx, &code);
    return (r == SmtpResult::SMTP_OK) ? code : (int)r;
}

// AUTH LOGIN leg: send @p secret base64-encoded + CRLF, return the reply code.
int auth_send_b64(SmtpSendFn send, SmtpRecvFn recv, void *ctx, const char *secret)
{
    char line[DETWS_SMTP_LINE_MAX];
    char b64[DETWS_SMTP_LINE_MAX];
    size_t slen = strnlen(secret, sizeof(b64));
    if (((slen + 2) / 3) * 4 + 3 >= sizeof(b64)) // b64 + CRLF must fit
        return (int)SmtpResult::SMTP_ERR_OVERFLOW;
    base64_encode((const uint8_t *)secret, slen, b64);
    int n = snprintf(line, sizeof(line), "%s\r\n", b64);
    if (n < 0 || (size_t)n >= sizeof(line))
        return (int)SmtpResult::SMTP_ERR_OVERFLOW; // GCOVR_EXCL_LINE  b64+CRLF was just checked to fit
                                                   // sizeof(b64)==sizeof(line); can't overflow
    return command(send, recv, ctx, line);
}

// Assemble the DATA payload (headers + body + terminating dot) into @p out, applying
// CRLF normalization and RFC 5321 sec 4.5.2 dot-stuffing. Returns the length, or <0.
int build_message(char *out, size_t cap, const SmtpConfig *cfg, const SmtpMessage *msg)
{
    int hn = snprintf(out, cap,
                      "From: <%s>\r\n"
                      "To: <%s>\r\n"
                      "Subject: %s\r\n"
                      "MIME-Version: 1.0\r\n"
                      "Content-Type: text/plain; charset=UTF-8\r\n"
                      "\r\n",
                      cfg->from, msg->to, msg->subject ? msg->subject : "");
    if (hn < 0 || (size_t)hn >= cap)
        return (int)SmtpResult::SMTP_ERR_OVERFLOW;
    size_t n = (size_t)hn;

    const char *b = msg->body ? msg->body : "";
    bool at_line_start = true;
    for (size_t i = 0; b[i]; i++)
    {
        char c = b[i];
        if (c == '\r')
            continue; // normalize: CR is dropped, LF becomes CRLF
        if (c == '\n')
        {
            if (n + 2 > cap)
                return (int)SmtpResult::SMTP_ERR_OVERFLOW;
            out[n++] = '\r';
            out[n++] = '\n';
            at_line_start = true;
            continue;
        }
        if (at_line_start && c == '.')
        {
            if (n + 1 > cap) // dot-stuff: a body line starting with '.' gets an extra '.'
                return (int)SmtpResult::SMTP_ERR_OVERFLOW;
            out[n++] = '.';
        }
        if (n + 1 > cap)
            return (int)SmtpResult::SMTP_ERR_OVERFLOW;
        out[n++] = c;
        at_line_start = false;
    }
    // Body must end with CRLF before the terminator.
    if (!(n >= 2 && out[n - 2] == '\r' && out[n - 1] == '\n'))
    {
        if (n + 2 > cap)
            return (int)SmtpResult::SMTP_ERR_OVERFLOW;
        out[n++] = '\r';
        out[n++] = '\n';
    }
    if (n + 3 > cap) // terminating "."CRLF
        return (int)SmtpResult::SMTP_ERR_OVERFLOW;
    out[n++] = '.';
    out[n++] = '\r';
    out[n++] = '\n';
    return (int)n;
}
} // namespace

SmtpResult smtp_run(const SmtpConfig *cfg, const SmtpMessage *msg, SmtpSendFn send, SmtpRecvFn recv, void *ctx)
{
    if (!cfg || !msg || !send || !recv || !cfg->host || !cfg->from || !cfg->from[0] || !msg->to || !msg->to[0])
        return SmtpResult::SMTP_ERR_ARG;

    char line[DETWS_SMTP_LINE_MAX];
    int code;

    // Greeting.
    if (read_reply(recv, ctx, &code) != SmtpResult::SMTP_OK)
        return SmtpResult::SMTP_ERR_IO;
    if (code != 220)
        return SmtpResult::SMTP_ERR_PROTOCOL;

    // EHLO.
    int n = snprintf(line, sizeof(line), "EHLO %s\r\n", (cfg->helo && cfg->helo[0]) ? cfg->helo : "esp32");
    if (n < 0 || (size_t)n >= sizeof(line))
        return SmtpResult::SMTP_ERR_OVERFLOW;
    code = command(send, recv, ctx, line);
    if (code < 0)
        return (SmtpResult)code;
    if (code != 250)
        return SmtpResult::SMTP_ERR_PROTOCOL;

    // AUTH LOGIN (only when a username is configured).
    if (cfg->user && cfg->user[0])
    {
        code = command(send, recv, ctx, "AUTH LOGIN\r\n");
        if (code < 0)
            return (SmtpResult)code;
        if (code != 334)
            return SmtpResult::SMTP_ERR_AUTH;
        code = auth_send_b64(send, recv, ctx, cfg->user);
        if (code < 0)
            return (SmtpResult)code;
        if (code != 334)
            return SmtpResult::SMTP_ERR_AUTH;
        code = auth_send_b64(send, recv, ctx, cfg->pass ? cfg->pass : "");
        if (code < 0)
            return (SmtpResult)code;
        if (code != 235)
            return SmtpResult::SMTP_ERR_AUTH;
    }

    // MAIL FROM.
    n = snprintf(line, sizeof(line), "MAIL FROM:<%s>\r\n", cfg->from);
    if (n < 0 || (size_t)n >= sizeof(line))
        return SmtpResult::SMTP_ERR_OVERFLOW;
    code = command(send, recv, ctx, line);
    if (code < 0)
        return (SmtpResult)code;
    if (code != 250)
        return SmtpResult::SMTP_ERR_PROTOCOL;

    // RCPT TO.
    n = snprintf(line, sizeof(line), "RCPT TO:<%s>\r\n", msg->to);
    if (n < 0 || (size_t)n >= sizeof(line))
        return SmtpResult::SMTP_ERR_OVERFLOW;
    code = command(send, recv, ctx, line);
    if (code < 0)
        return (SmtpResult)code;
    if (code != 250 && code != 251) // 251 = user not local; will forward
        return SmtpResult::SMTP_ERR_PROTOCOL;

    // DATA.
    code = command(send, recv, ctx, "DATA\r\n");
    if (code < 0)
        return (SmtpResult)code;
    if (code != 354)
        return SmtpResult::SMTP_ERR_PROTOCOL;

    // The message itself, then wait for acceptance.
    char body[DETWS_SMTP_MSG_MAX];
    int mlen = build_message(body, sizeof(body), cfg, msg);
    if (mlen < 0)
        return (SmtpResult)mlen;
    if (send(ctx, (const uint8_t *)body, (size_t)mlen) != mlen)
        return SmtpResult::SMTP_ERR_IO;
    if (read_reply(recv, ctx, &code) != SmtpResult::SMTP_OK)
        return SmtpResult::SMTP_ERR_IO;
    if (code != 250)
        return SmtpResult::SMTP_ERR_PROTOCOL;

    // QUIT is best-effort - the message is already accepted.
    (void)command(send, recv, ctx, "QUIT\r\n");
    return SmtpResult::SMTP_OK;
}

// ---------------------------------------------------------------------------
// Real-transport binding (Arduino): det_client, plus a det_tls csess for SMTPS.
// ---------------------------------------------------------------------------

#if defined(ARDUINO)

#include "network_drivers/transport/client.h"
#include <Arduino.h> // millis, delay
#if DETWS_ENABLE_TLS
#include "network_drivers/tls/tls.h"
#endif

namespace
{
struct SmtpXport
{
    int cid;
    uint32_t deadline;
};

// Plaintext seam over det_client.
int cl_send(void *ctx, const uint8_t *data, size_t len)
{
    SmtpXport *x = (SmtpXport *)ctx;
    size_t sent = 0;
    while (sent < len)
    {
        size_t chunk = len - sent;
        if (chunk > 0xFFFF)
            chunk = 0xFFFF;
        if (!det_client_send(x->cid, data + sent, chunk))
            return -1;
        sent += chunk;
    }
    return (int)len;
}
int cl_recv(void *ctx, uint8_t *buf, size_t cap)
{
    SmtpXport *x = (SmtpXport *)ctx;
    while ((int32_t)(x->deadline - millis()) > 0)
    {
        size_t n = det_client_read(x->cid, buf, cap);
        if (n > 0)
            return (int)n;
        if (det_client_is_closed(x->cid) && det_client_available(x->cid) == 0)
            return -1;
        dwsdelay(5);
    }
    return -1; // timeout
}

#if DETWS_ENABLE_TLS
// TLS ciphertext BIO: the csess handshake/records read/write the wire via det_client.
int tls_bio_send(void *ctx, const unsigned char *buf, size_t len)
{
    SmtpXport *x = (SmtpXport *)ctx;
    return det_client_send(x->cid, buf, len) ? (int)len : MBEDTLS_ERR_SSL_WANT_WRITE;
}
int tls_bio_recv(void *ctx, unsigned char *buf, size_t len)
{
    SmtpXport *x = (SmtpXport *)ctx;
    size_t n = det_client_read(x->cid, buf, len);
    if (n == 0)
        return det_client_is_closed(x->cid) ? 0 : MBEDTLS_ERR_SSL_WANT_READ;
    return (int)n;
}
// Application seam over the established TLS session.
int tls_send(void *ctx, const uint8_t *data, size_t len)
{
    (void)ctx;
    return det_tls_csess_write(data, len) == (int)len ? (int)len : -1;
}
int tls_recv(void *ctx, uint8_t *buf, size_t cap)
{
    SmtpXport *x = (SmtpXport *)ctx;
    while ((int32_t)(x->deadline - millis()) > 0)
    {
        int n = det_tls_csess_read(buf, cap);
        if (n > 0)
            return n;
        if (n < 0 && n != MBEDTLS_ERR_SSL_WANT_READ && n != MBEDTLS_ERR_SSL_WANT_WRITE)
            return -1;
        dwsdelay(5);
    }
    return -1; // timeout
}
#endif // DETWS_ENABLE_TLS
} // namespace

SmtpResult smtp_send(const SmtpConfig *cfg, const SmtpMessage *msg)
{
    if (!cfg || !cfg->host)
        return SmtpResult::SMTP_ERR_ARG;

    SmtpXport x;
    x.cid = det_client_open(cfg->host, cfg->port, DETWS_SMTP_TIMEOUT_MS);
    if (x.cid < 0)
        return SmtpResult::SMTP_ERR_CONNECT;
    x.deadline = millis() + DETWS_SMTP_TIMEOUT_MS;

    SmtpResult rc;
    if (cfg->tls)
    {
#if DETWS_ENABLE_TLS
        if (!det_tls_csess_begin(cfg->host, tls_bio_send, tls_bio_recv))
        {
            det_client_close(x.cid);
            return SmtpResult::SMTP_ERR_TLS;
        }
        int h;
        while ((h = det_tls_csess_handshake()) == 0 && (int32_t)(x.deadline - millis()) > 0)
            dwsdelay(5);
        if (h != 1) // 1 = established; 0 = still pending at timeout; <0 = fatal
        {
            det_tls_csess_end();
            det_client_close(x.cid);
            return SmtpResult::SMTP_ERR_TLS;
        }
        rc = smtp_run(cfg, msg, tls_send, tls_recv, &x);
        det_tls_csess_end();
#else
        det_client_close(x.cid);
        return SmtpResult::SMTP_ERR_TLS; // SMTPS requested but TLS not built in
#endif
    }
    else
    {
        rc = smtp_run(cfg, msg, cl_send, cl_recv, &x);
    }

    det_client_close(x.cid);
    return rc;
}

#else // host build: no lwIP. smtp_run() above is host-testable; smtp_send() is a stub.

SmtpResult smtp_send(const SmtpConfig *, const SmtpMessage *)
{
    return SmtpResult::SMTP_ERR_CONNECT;
}

#endif // ARDUINO

#endif // DETWS_ENABLE_SMTP
