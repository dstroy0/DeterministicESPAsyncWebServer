// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_channel.h
 * @brief SSH connection protocol - single "session" channel (RFC 4254).
 *
 * After authentication the client opens a "session" channel; this layer
 * confirms it, accepts a shell/exec/pty request, and then exchanges
 * SSH_MSG_CHANNEL_DATA. Inbound channel data is surfaced to the application as
 * a raw byte stream (no PTY emulation); outbound data is framed back to the
 * client. Flow control follows RFC 4254 §5.2 (window tracking + WINDOW_ADJUST).
 *
 * One channel per SSH connection is supported (sufficient for a control/console
 * link on an embedded target).
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SSH_CHANNEL_H
#define DETERMINISTICESPASYNCWEBSERVER_SSH_CHANNEL_H

#include "DetWebServerConfig.h"
#include <stddef.h>
#include <stdint.h>

/** @brief Initial receive window the server advertises (RFC 4254 §5.1). */
#ifndef SSH_CHAN_WINDOW
#define SSH_CHAN_WINDOW 32768u
#endif

/** @brief Maximum SSH channel data payload the server accepts per message. */
#ifndef SSH_CHAN_MAX_PACKET
#define SSH_CHAN_MAX_PACKET 32768u
#endif

/** @brief Per-connection channel state. */
struct SshChannel
{
    bool open;             ///< True once CHANNEL_OPEN_CONFIRMATION is sent.
    uint32_t local_id;     ///< Our channel id (== slot index).
    uint32_t peer_id;      ///< Client's channel id.
    uint32_t local_window; ///< Bytes we may still receive before WINDOW_ADJUST.
    uint32_t peer_window;  ///< Bytes we may still send to the client.
    uint32_t peer_max_pkt; ///< Client's maximum packet size.
};

/** @brief Channel pool, one per SSH connection (BSS). */
extern SshChannel ssh_chan[MAX_SSH_CONNS];

/** @brief Application callback for inbound channel data (raw bytes). */
typedef void (*SshChannelDataCb)(uint8_t slot, const uint8_t *data, size_t len);

/** @brief Install the inbound-data callback. */
void ssh_channel_set_data_cb(SshChannelDataCb cb);

/** @brief Reset channel state for slot @p i. */
void ssh_channel_init(uint8_t i);

/**
 * @brief Handle SSH_MSG_CHANNEL_OPEN and emit CHANNEL_OPEN_CONFIRMATION.
 *
 * Accepts a "session" channel; any other type yields CHANNEL_OPEN_FAILURE.
 * @return 0 if a response was produced, -1 if malformed.
 */
int ssh_channel_handle_open(uint8_t i, const uint8_t *payload, size_t len, uint8_t *out, size_t *out_len, size_t cap);

/**
 * @brief Handle SSH_MSG_CHANNEL_REQUEST.
 *
 * "shell", "exec", and "pty-req" are accepted; anything else is refused. When
 * want_reply is set, CHANNEL_SUCCESS / CHANNEL_FAILURE is written to @p out and
 * *@p out_len > 0; otherwise *@p out_len is 0.
 * @return 0 on success, -1 if malformed.
 */
int ssh_channel_handle_request(uint8_t i, const uint8_t *payload, size_t len, uint8_t *out, size_t *out_len,
                               size_t cap);

/**
 * @brief Handle SSH_MSG_CHANNEL_DATA: bounds-check, update the window, and
 *        invoke the data callback. If the local window is exhausted a
 *        CHANNEL_WINDOW_ADJUST is written to @p out (*@p out_len > 0).
 * @return 0 on success, -1 if malformed or channel not open.
 */
int ssh_channel_handle_data(uint8_t i, const uint8_t *payload, size_t len, uint8_t *out, size_t *out_len, size_t cap);

/**
 * @brief Build an SSH_MSG_CHANNEL_DATA message carrying @p data to the client.
 * @return 0 on success, -1 if the channel is closed, the peer window is too
 *         small, or @p out is too small.
 */
int ssh_channel_build_data(uint8_t i, const uint8_t *data, size_t len, uint8_t *out, size_t *out_len, size_t cap);

/**
 * @brief Handle SSH_MSG_CHANNEL_WINDOW_ADJUST (grows the peer window).
 * @return 0 on success, -1 if malformed.
 */
int ssh_channel_handle_window_adjust(uint8_t i, const uint8_t *payload, size_t len);

/**
 * @brief Build SSH_MSG_CHANNEL_EOF followed by SSH_MSG_CHANNEL_CLOSE.
 * @return 0 on success, -1 if the channel is closed or @p out is too small.
 */
int ssh_channel_build_close(uint8_t i, uint8_t *out, size_t *out_len, size_t cap);

#endif // DETERMINISTICESPASYNCWEBSERVER_SSH_CHANNEL_H
