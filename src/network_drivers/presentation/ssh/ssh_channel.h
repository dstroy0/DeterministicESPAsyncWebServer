// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_channel.h
 * @brief SSH connection protocol - multiplexed "session" channels (RFC 4254).
 *
 * After authentication the client opens one or more channels; this layer confirms
 * each, accepts a shell/exec/pty request, and exchanges SSH_MSG_CHANNEL_DATA.
 * Inbound channel data is surfaced to the application as a raw byte stream (no PTY
 * emulation), tagged with its channel id; outbound data is framed back to the
 * client on a given channel. Flow control follows RFC 4254 §5.2 (per-channel
 * window tracking + WINDOW_ADJUST).
 *
 * Up to DETWS_SSH_MAX_CHANNELS channels per connection are multiplexed, each with
 * its own id, peer id, and windows. The default (1) is the original single-channel
 * control/console link. Every inbound message is routed to its channel by the
 * recipient channel id it carries.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SSH_CHANNEL_H
#define DETERMINISTICESPASYNCWEBSERVER_SSH_CHANNEL_H

#include "shared_primitives/shim.h"

/** @brief Initial receive window the server advertises (RFC 4254 §5.1). */
#ifndef SSH_CHAN_WINDOW
#define SSH_CHAN_WINDOW 32768u
#endif

/** @brief Maximum SSH channel data payload the server accepts per message. */
#ifndef SSH_CHAN_MAX_PACKET
#define SSH_CHAN_MAX_PACKET 32768u
#endif

/** @brief Channel type (RFC 4254). */
enum SshChanType
{
    SSH_CHAN_SESSION = 0,     ///< "session" - shell / exec / data
    SSH_CHAN_DIRECT_TCPIP = 1 ///< "direct-tcpip" - client-initiated TCP forward (ssh -L)
};

/** @brief Per-connection channel state. */
struct SshChannel
{
    bool open;             ///< True once CHANNEL_OPEN_CONFIRMATION is sent.
    uint8_t type;          ///< SshChanType: session or direct-tcpip forward.
    uint32_t local_id;     ///< Our channel id (== slot index).
    uint32_t peer_id;      ///< Client's channel id.
    uint32_t local_window; ///< Bytes we may still receive before WINDOW_ADJUST.
    uint32_t peer_window;  ///< Bytes we may still send to the client.
    uint32_t peer_max_pkt; ///< Client's maximum packet size.
};

/** @brief Channel pool: DETWS_SSH_MAX_CHANNELS channels per SSH connection (BSS).
 *  Owned by this layer; src/ code routes through the functions below, never the
 *  array (tests inspect it white-box). Index: [connection slot][channel slot]. */
extern SshChannel ssh_chan[MAX_SSH_CONNS][DETWS_SSH_MAX_CHANNELS];

/** @brief Application callback for inbound channel data (raw bytes), tagged with
 *  the channel id it arrived on. */
typedef void (*SshChannelDataCb)(uint8_t slot, uint32_t channel, const uint8_t *data, size_t len);

/** @brief Install the inbound-data callback (session channels). */
void ssh_channel_set_data_cb(SshChannelDataCb cb);

/**
 * @brief "direct-tcpip" forward request: a client asked the server to open a TCP
 *        connection to @p host : @p port (ssh -L). The forwarding owner (which
 *        does the actual TCP I/O - this codec does not) decides whether to allow
 *        it; @p host is not NUL-terminated (@p host_len bytes).
 * @return 0 to accept (the channel is opened and confirmed), < 0 to refuse
 *         (CHANNEL_OPEN_FAILURE, administratively prohibited / connect failed).
 *
 * If no callback is installed, all forward requests are refused - so forwarding is
 * opt-in (no open relay by default).
 */
typedef int (*SshForwardOpenCb)(uint8_t slot, uint32_t channel, const char *host, size_t host_len, uint16_t port);

/** @brief Inbound data on a direct-tcpip channel (the owner writes it to the
 *  forwarded TCP socket). Kept separate from the session data callback. */
typedef void (*SshForwardDataCb)(uint8_t slot, uint32_t channel, const uint8_t *data, size_t len);

/** @brief Install the direct-tcpip forward open-policy callback (opt-in). */
void ssh_channel_set_forward_open_cb(SshForwardOpenCb cb);

/** @brief Install the direct-tcpip forward inbound-data callback. */
void ssh_channel_set_forward_data_cb(SshForwardDataCb cb);

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
 * @brief Build an SSH_MSG_CHANNEL_DATA message carrying @p data to the client on
 *        channel @p channel (a local channel id from a prior open).
 * @return 0 on success, -1 if the channel is closed/unknown, the peer window is
 *         too small, or @p out is too small.
 */
int ssh_channel_build_data(uint8_t i, uint32_t channel, const uint8_t *data, size_t len, uint8_t *out, size_t *out_len,
                           size_t cap);

/**
 * @brief Handle SSH_MSG_CHANNEL_WINDOW_ADJUST (grows the peer window).
 * @return 0 on success, -1 if malformed.
 */
int ssh_channel_handle_window_adjust(uint8_t i, const uint8_t *payload, size_t len);

/**
 * @brief Build SSH_MSG_CHANNEL_EOF + SSH_MSG_CHANNEL_CLOSE for channel @p channel
 *        and mark it closed.
 * @return 0 on success, -1 if the channel is closed/unknown or @p out is too small.
 */
int ssh_channel_build_close(uint8_t i, uint32_t channel, uint8_t *out, size_t *out_len, size_t cap);

/**
 * @brief Handle an inbound SSH_MSG_CHANNEL_CLOSE: route to the recipient channel,
 *        reply with EOF + CLOSE, and mark it closed.
 * @return 0 if a response was produced, -1 if malformed or the channel is unknown.
 */
int ssh_channel_handle_close(uint8_t i, const uint8_t *payload, size_t len, uint8_t *out, size_t *out_len, size_t cap);

#endif // DETERMINISTICESPASYNCWEBSERVER_SSH_CHANNEL_H
