// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file smtp.h
 * @brief Outbound SMTP client (RFC 5321) - send a device email alert.
 *
 * A blocking one-shot: connect, greet, optional AUTH LOGIN, then MAIL FROM / RCPT TO /
 * DATA a plain-text message and QUIT. It rides the shared outbound client transport
 * (`dws_client`), with implicit TLS (SMTPS, typically port 465) when the config sets
 * `tls` and DWS_ENABLE_TLS is on. Zero heap; every buffer is a compile-time size
 * (`DWS_SMTP_*`). Gated by DWS_ENABLE_SMTP.
 *
 * The dialogue itself (smtp_run) is written against a send/recv seam, so the whole
 * protocol exchange - greeting codes, AUTH, dot-stuffing, the terminating `.` - is
 * unit-tested on the host with a scripted mock server, no lwIP or TLS required.
 *
 * "SMS fallback" needs no extra code: most mobile carriers accept an email-to-SMS
 * gateway address (e.g. `5551234567@txt.example.net`) as the recipient.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SMTP_H
#define DETERMINISTICESPASYNCWEBSERVER_SMTP_H

#include <stddef.h>
#include <stdint.h>

/** @brief Result of an SMTP send. 0 is success; every failure is a distinct negative code. */
enum class SmtpResult : int32_t
{
    SMTP_OK = 0,
    SMTP_ERR_ARG = -1,      ///< a required field (host / from / to) was null or empty
    SMTP_ERR_CONNECT = -2,  ///< could not open the transport (DNS / connect)
    SMTP_ERR_TLS = -3,      ///< the TLS handshake failed (SMTPS)
    SMTP_ERR_IO = -4,       ///< a send/recv failed or the reply timed out
    SMTP_ERR_PROTOCOL = -5, ///< the server returned an unexpected reply code
    SMTP_ERR_AUTH = -6,     ///< AUTH was rejected (bad user/password)
    SMTP_ERR_OVERFLOW = -7, ///< a command line or the message exceeded its fixed buffer
};

/**
 * @brief Transport seam for smtp_run(): the engine sends and receives raw bytes only
 * through these, so it can run against a real socket or a test mock.
 *
 * @return send: number of bytes written (must equal @p len), or <0 on error.
 * @return recv: number of bytes read (>0), or <=0 on close / error / timeout.
 */
typedef int (*SmtpSendFn)(void *ctx, const uint8_t *data, size_t len);
typedef int (*SmtpRecvFn)(void *ctx, uint8_t *buf, size_t cap);

/** @brief Server address + credentials for one send. Addresses are bare (no angle brackets). */
struct SmtpConfig
{
    const char *host; ///< server hostname (also the TLS SNI name)
    uint16_t port;    ///< 25 / 587 / 465
    bool tls;         ///< true = implicit TLS on connect (SMTPS); false = plaintext
    const char *user; ///< AUTH LOGIN username (null or empty => skip AUTH)
    const char *pass; ///< AUTH LOGIN password
    const char *from; ///< envelope sender + From: header address
    const char *helo; ///< EHLO domain to announce (null => "esp32")
};

/** @brief One plain-text message. */
struct SmtpMessage
{
    const char *to;      ///< single recipient address (envelope + To: header)
    const char *subject; ///< Subject: header (null => empty)
    const char *body;    ///< plain-text UTF-8 body; LF or CRLF line ends, dot-stuffed for you
};

/**
 * @brief Drive the full SMTP exchange over @p send / @p recv. Pure - no lwIP or TLS -
 * so it is host-testable with a scripted transport.
 * @return SmtpResult::SMTP_OK on a delivered message, else an ::SmtpResult error.
 */
SmtpResult smtp_run(const SmtpConfig *cfg, const SmtpMessage *msg, SmtpSendFn send, SmtpRecvFn recv, void *ctx);

/**
 * @brief Blocking one-shot send over the real transport (dws_client, plus TLS when
 * `cfg->tls`). Opens the connection, runs smtp_run(), and closes.
 * @return SmtpResult::SMTP_OK or an ::SmtpResult error. On non-Arduino (host) builds there is no
 *         lwIP, so this returns SmtpResult::SMTP_ERR_CONNECT; use smtp_run() directly in tests.
 */
SmtpResult smtp_send(const SmtpConfig *cfg, const SmtpMessage *msg);

#endif // DETERMINISTICESPASYNCWEBSERVER_SMTP_H
