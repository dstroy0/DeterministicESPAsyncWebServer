// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file smtp.cpp
 * @brief Outbound SMTP client (RFC 5321) - implementation. See smtp.h for the model.
 *
 * smtp_run() is the pure dialogue engine (host-testable via the send/recv seam);
 * smtp_send() binds it to the real transport on Arduino (dws_client, +dws_tls csess).
 */

#include "services/smtp/smtp.h"
#include "ServerConfig.h"
#include "services/clock.h" // dwsdelay

#if DWS_ENABLE_SMTP

#include "network_drivers/presentation/base64/base64.h"
#include <stdio.h>  // snprintf
#include <string.h> // strlen, memcmp

namespace
{
// Send an entire C string; returns true only if every byte went out.
bool send_str(SmtpSendFn send, void *ctx, const char *s)
{
    size_t n = strnlen(s, DWS_SMTP_LINE_MAX + 1);
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

// Case-insensitive compare of @p n bytes. EHLO keywords are case-insensitive (RFC 5321 sec 2.4)
// and strncasecmp is not portable across every toolchain this builds under.
bool ieq(const char *a, const char *b, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        char ca = a[i], cb = b[i];
        if (ca >= 'A' && ca <= 'Z')
            ca = (char)(ca - 'A' + 'a');
        if (cb >= 'A' && cb <= 'Z')
            cb = (char)(cb - 'A' + 'a');
        if (ca != cb)
            return false;
    }
    return true;
}

// Does @p want appear as its own EHLO capability line? Each line is "NNN<sep>KEYWORD[ params]",
// so the keyword starts at offset 4 and is matched whole - a server advertising "STARTTLSX" must
// not read as one advertising STARTTLS, since that decides whether credentials go out in clear.
bool reply_has_cap(const char *buf, size_t len, const char *want)
{
    size_t wlen = strlen(want);
    size_t start = 0;
    for (size_t i = 0; i + 1 < len; i++)
    {
        if (buf[i] != '\r' || buf[i + 1] != '\n')
            continue;
        size_t line_len = i - start; // excludes the CRLF
        if (line_len > 4)            // "NNN" + separator + at least one keyword character
        {
            const char *kw = buf + start + 4;
            size_t klen = line_len - 4;
            if (klen >= wlen && ieq(kw, want, wlen) && (klen == wlen || kw[wlen] == ' '))
                return true;
        }
        start = i + 2;
    }
    return false;
}

// Read one (possibly multi-line) reply and return its code. When @p want is given, @p found
// reports whether that capability appeared in the reply.
SmtpResult read_reply_cap(SmtpRecvFn recv, void *ctx, int *code, const char *want, bool *found)
{
    char buf[DWS_SMTP_REPLY_MAX];
    size_t len = 0;
    for (;;)
    {
        if (reply_complete(buf, len, code))
        {
            if (want && found)
                *found = reply_has_cap(buf, len, want);
            return SmtpResult::SMTP_OK;
        }
        if (len >= sizeof(buf))
            return SmtpResult::SMTP_ERR_OVERFLOW;
        int n = recv(ctx, (uint8_t *)buf + len, sizeof(buf) - len);
        if (n <= 0)
            return SmtpResult::SMTP_ERR_IO;
        len += (size_t)n;
    }
}

SmtpResult read_reply(SmtpRecvFn recv, void *ctx, int *code)
{
    return read_reply_cap(recv, ctx, code, nullptr, nullptr);
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
    char line[DWS_SMTP_LINE_MAX];
    char b64[DWS_SMTP_LINE_MAX];
    size_t slen = strnlen(secret, sizeof(b64));
    if (((slen + 2) / 3) * 4 + 3 >= sizeof(b64)) // b64 + CRLF must fit
        return (int)SmtpResult::SMTP_ERR_OVERFLOW;
    dws_base64_encode((const uint8_t *)secret, slen, b64);
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

SmtpResult smtp_run(const SmtpConfig *cfg, const SmtpMessage *msg, SmtpSendFn send, SmtpRecvFn recv,
                    SmtpStartTlsFn starttls, void *ctx)
{
    if (!cfg || !msg || !send || !recv || !cfg->host || !cfg->from || !cfg->from[0] || !msg->to || !msg->to[0])
        return SmtpResult::SMTP_ERR_ARG;

    char line[DWS_SMTP_LINE_MAX];
    int code;

    // Greeting.
    if (read_reply(recv, ctx, &code) != SmtpResult::SMTP_OK)
        return SmtpResult::SMTP_ERR_IO;
    if (code != 220)
        return SmtpResult::SMTP_ERR_PROTOCOL;

    // EHLO. The capability list is only trustworthy once the channel is secure, which is why the
    // STARTTLS path below reissues this command after the upgrade.
    int n = snprintf(line, sizeof(line), "EHLO %s\r\n", (cfg->helo && cfg->helo[0]) ? cfg->helo : "esp32");
    if (n < 0 || (size_t)n >= sizeof(line))
        return SmtpResult::SMTP_ERR_OVERFLOW;
    bool has_starttls = false;
    if (!send_str(send, ctx, line))
        return SmtpResult::SMTP_ERR_IO;
    if (read_reply_cap(recv, ctx, &code, "STARTTLS", &has_starttls) != SmtpResult::SMTP_OK)
        return SmtpResult::SMTP_ERR_IO;
    if (code != 250)
        return SmtpResult::SMTP_ERR_PROTOCOL;

    // STARTTLS (RFC 3207): upgrade in band, then start the session over.
    if (cfg->security == SmtpSecurity::SMTP_STARTTLS)
    {
        // Fail closed on a stripped advertisement. An attacker who can delete the capability line
        // would otherwise get the whole exchange - AUTH credentials included - in the clear.
        if (!has_starttls)
            return SmtpResult::SMTP_ERR_NO_STARTTLS;
        if (!starttls)
            return SmtpResult::SMTP_ERR_ARG; // asked to upgrade with no way to do it
        code = command(send, recv, ctx, "STARTTLS\r\n");
        if (code < 0)
            return (SmtpResult)code;
        if (code != 220)
            return SmtpResult::SMTP_ERR_TLS;
        if (!starttls(ctx))
            return SmtpResult::SMTP_ERR_TLS;
        // RFC 3207 sec 4.2: discard everything learned in the clear and reissue EHLO - the real
        // capability list (AUTH mechanisms especially) is the one the server sends encrypted.
        code = command(send, recv, ctx, line);
        if (code < 0)
            return (SmtpResult)code;
        if (code != 250)
            return SmtpResult::SMTP_ERR_PROTOCOL;
    }

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
    char body[DWS_SMTP_MSG_MAX];
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
// Real-transport binding (Arduino): dws_client, plus a dws_tls csess for SMTPS.
// ---------------------------------------------------------------------------

#if defined(ARDUINO)

#include "network_drivers/transport/client.h"
#include <Arduino.h> // millis, delay
#if DWS_ENABLE_SMTP_TLS
#include "network_drivers/tls/tls.h"
#include <mbedtls/ssl.h> // MBEDTLS_ERR_SSL_WANT_* for the BIO callbacks
#endif

namespace
{
struct SmtpXport;

/** @brief Owned state: which transport the TLS BIO callbacks act on.
 *
 * dws_tls_client_session_begin() carries no context pointer, so mbedtls calls the BIO with a ctx
 * that is not ours. The active transport is parked here for the life of the session instead. */
struct SmtpTlsCtx
{
    SmtpXport *xport;
};

struct SmtpXport
{
    int cid;
    uint32_t deadline;
    const char *host; ///< TLS SNI name, needed when the upgrade happens mid-dialogue
    bool tls_active;  ///< set once a STARTTLS upgrade has completed on this connection
};

static SmtpTlsCtx s_smtp_tls = {nullptr};

// Plaintext seam over dws_client.
int cl_send(void *ctx, const uint8_t *data, size_t len)
{
    SmtpXport *x = (SmtpXport *)ctx;
    size_t sent = 0;
    while (sent < len)
    {
        size_t chunk = len - sent;
        if (chunk > 0xFFFF)
            chunk = 0xFFFF;
        if (!dws_client_send(x->cid, data + sent, chunk))
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
        size_t n = dws_client_read(x->cid, buf, cap);
        if (n > 0)
            return (int)n;
        if (dws_client_is_closed(x->cid) && dws_client_available(x->cid) == 0)
            return -1;
        dwsdelay(5);
    }
    return -1; // timeout
}

#if DWS_ENABLE_SMTP_TLS
// TLS ciphertext BIO: the csess handshake/records read/write the wire via dws_client.
int tls_bio_send(void *ctx, const unsigned char *buf, size_t len)
{
    (void)ctx; // not ours - see SmtpTlsCtx
    SmtpXport *x = s_smtp_tls.xport;
    if (!x)
        return MBEDTLS_ERR_SSL_WANT_WRITE;
    return dws_client_send(x->cid, buf, len) ? (int)len : MBEDTLS_ERR_SSL_WANT_WRITE;
}
int tls_bio_recv(void *ctx, unsigned char *buf, size_t len)
{
    (void)ctx; // not ours - see SmtpTlsCtx
    SmtpXport *x = s_smtp_tls.xport;
    if (!x)
        return MBEDTLS_ERR_SSL_WANT_READ;
    size_t n = dws_client_read(x->cid, buf, len);
    if (n == 0)
        return dws_client_is_closed(x->cid) ? 0 : MBEDTLS_ERR_SSL_WANT_READ;
    return (int)n;
}
// Application seam over the established TLS session.
int tls_send(void *ctx, const uint8_t *data, size_t len)
{
    (void)ctx;
    return dws_tls_client_session_write(data, len) == (int)len ? (int)len : -1;
}
int tls_recv(void *ctx, uint8_t *buf, size_t cap)
{
    SmtpXport *x = (SmtpXport *)ctx;
    while ((int32_t)(x->deadline - millis()) > 0)
    {
        int n = dws_tls_client_session_read(buf, cap);
        if (n > 0)
            return n;
        if (n < 0 && n != MBEDTLS_ERR_SSL_WANT_READ && n != MBEDTLS_ERR_SSL_WANT_WRITE)
            return -1;
        dwsdelay(5);
    }
    return -1; // timeout
}
#endif // DWS_ENABLE_SMTP_TLS

// Switching seam. The dialogue engine gets exactly one send/recv pair for the whole exchange; a
// STARTTLS upgrade flips these underneath it, so the engine never swaps transports mid-conversation
// and cannot accidentally keep writing plaintext after the upgrade.
int xp_send(void *ctx, const uint8_t *data, size_t len)
{
#if DWS_ENABLE_SMTP_TLS
    if (((SmtpXport *)ctx)->tls_active)
        return tls_send(ctx, data, len);
#endif
    return cl_send(ctx, data, len);
}
int xp_recv(void *ctx, uint8_t *buf, size_t cap)
{
#if DWS_ENABLE_SMTP_TLS
    if (((SmtpXport *)ctx)->tls_active)
        return tls_recv(ctx, buf, cap);
#endif
    return cl_recv(ctx, buf, cap);
}

// Upgrade the live connection in place, after the server's 220 to STARTTLS.
bool xp_starttls(void *ctx)
{
    SmtpXport *x = (SmtpXport *)ctx;
#if DWS_ENABLE_SMTP_TLS
    if (!dws_tls_client_session_begin(x->host, tls_bio_send, tls_bio_recv))
        return false;
    // Fresh budget: the deadline carried here was set at connect time and has already funded the
    // greeting, EHLO and STARTTLS round trips. Reusing whatever is left of it can abandon the
    // handshake before the ClientHello even goes out, which the server sees as a silent hang.
    x->deadline = millis() + DWS_SMTP_TIMEOUT_MS;
    int h;
    while ((h = dws_tls_client_session_handshake()) == 0 && (int32_t)(x->deadline - millis()) > 0)
        dwsdelay(5);
    if (h != 1) // 1 = established; 0 = still pending at timeout; <0 = fatal
    {
        dws_tls_client_session_end();
        return false;
    }
    x->tls_active = true; // every later xp_send/xp_recv now goes through the session
    return true;
#else
    (void)x;
    return false; // STARTTLS requested but TLS not built in
#endif
}
} // namespace

SmtpResult smtp_send(const SmtpConfig *cfg, const SmtpMessage *msg)
{
    if (!cfg || !cfg->host)
        return SmtpResult::SMTP_ERR_ARG;

    SmtpXport x;
    x.cid = dws_client_open(cfg->host, cfg->port, DWS_SMTP_TIMEOUT_MS);
    if (x.cid < 0)
        return SmtpResult::SMTP_ERR_CONNECT;
    x.deadline = millis() + DWS_SMTP_TIMEOUT_MS;
    x.host = cfg->host;
    x.tls_active = false;
#if DWS_ENABLE_SMTP_TLS
    s_smtp_tls.xport = &x; // the BIO callbacks read this, not their ctx argument
#endif

    SmtpResult rc;
    if (cfg->security == SmtpSecurity::SMTP_TLS)
    {
#if DWS_ENABLE_SMTP_TLS
        if (!dws_tls_client_session_begin(cfg->host, tls_bio_send, tls_bio_recv))
        {
            dws_client_close(x.cid);
            return SmtpResult::SMTP_ERR_TLS;
        }
        int h;
        while ((h = dws_tls_client_session_handshake()) == 0 && (int32_t)(x.deadline - millis()) > 0)
            dwsdelay(5);
        if (h != 1) // 1 = established; 0 = still pending at timeout; <0 = fatal
        {
            dws_tls_client_session_end();
            dws_client_close(x.cid);
            return SmtpResult::SMTP_ERR_TLS;
        }
        rc = smtp_run(cfg, msg, tls_send, tls_recv, nullptr, &x);
        dws_tls_client_session_end();
#else
        dws_client_close(x.cid);
        return SmtpResult::SMTP_ERR_TLS; // SMTPS requested but TLS not built in
#endif
    }
    else
    {
        rc = smtp_run(cfg, msg, xp_send, xp_recv, xp_starttls, &x);
    }

    dws_client_close(x.cid);
#if DWS_ENABLE_SMTP_TLS
    s_smtp_tls.xport = nullptr; // x is about to go out of scope
#endif
    return rc;
}

#else // host build: no lwIP. smtp_run() above is host-testable; smtp_send() is a stub.

SmtpResult smtp_send(const SmtpConfig *, const SmtpMessage *)
{
    return SmtpResult::SMTP_ERR_CONNECT;
}

#endif // ARDUINO

#endif // DWS_ENABLE_SMTP
