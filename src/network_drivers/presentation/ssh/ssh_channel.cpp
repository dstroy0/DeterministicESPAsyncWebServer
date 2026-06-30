// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_channel.cpp
 * @brief SSH connection protocol - multiplexed session channels (RFC 4254).
 *
 * The channel table is owned here; inbound messages are routed to a channel by the
 * recipient channel id they carry, and the local channel id is the channel's slot
 * index in its connection's pool (unique per connection, which is all RFC 4254
 * requires). Other layers go through these functions, never the table.
 */

#include "network_drivers/presentation/ssh/ssh_channel.h"

SshChannel ssh_chan[MAX_SSH_CONNS][DETWS_SSH_MAX_CHANNELS];

static SshChannelDataCb g_data_cb = nullptr;
static SshForwardOpenCb g_forward_open_cb = nullptr;
static SshForwardDataCb g_forward_data_cb = nullptr;

void ssh_channel_set_data_cb(SshChannelDataCb cb)
{
    g_data_cb = cb;
}

void ssh_channel_set_forward_open_cb(SshForwardOpenCb cb)
{
    g_forward_open_cb = cb;
}

void ssh_channel_set_forward_data_cb(SshForwardDataCb cb)
{
    g_forward_data_cb = cb;
}

void ssh_channel_init(uint8_t i)
{
    if (i >= MAX_SSH_CONNS)
        return;
    memset(ssh_chan[i], 0, sizeof(ssh_chan[i])); // reset every channel for this connection
}

// ---------------------------------------------------------------------------
// Wire helpers
// ---------------------------------------------------------------------------

static uint32_t rd_u32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static void wr_u32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}

// Read an SSH string: out points at the data, *slen its length. Advances *off.
static bool rd_string(const uint8_t *p, size_t len, size_t *off, const uint8_t **out, uint32_t *slen)
{
    if (*off + 4 > len)
        return false;
    uint32_t n = rd_u32(p + *off);
    *off += 4;
    if (*off + n > len)
        return false;
    *out = p + *off;
    *slen = n;
    *off += n;
    return true;
}

// ---------------------------------------------------------------------------
// Channel table (owned here)
// ---------------------------------------------------------------------------

// The open channel @p id on connection @p i, or nullptr. local id == slot index.
static SshChannel *chan_by_id(uint8_t i, uint32_t id)
{
    if (i >= MAX_SSH_CONNS || id >= DETWS_SSH_MAX_CHANNELS || !ssh_chan[i][id].open)
        return nullptr;
    return &ssh_chan[i][id];
}

// First free channel slot on connection @p i, or -1 if the pool is full.
static int chan_alloc(uint8_t i)
{
    for (int c = 0; c < DETWS_SSH_MAX_CHANNELS; c++)
        if (!ssh_chan[i][c].open)
            return c;
    return -1;
}

// ---------------------------------------------------------------------------
// CHANNEL_OPEN → CONFIRMATION / FAILURE
// ---------------------------------------------------------------------------

// CHANNEL_OPEN_FAILURE: byte || recipient(sender) || reason || desc || lang.
// reason: 1 admin-prohibited, 2 connect-failed, 3 unknown-type, 4 resource-shortage.
static int build_open_failure(uint8_t *out, size_t cap, uint32_t sender, uint32_t reason, size_t *out_len)
{
    if (cap < 17)
        return -1;
    out[0] = SSH_MSG_CHANNEL_OPEN_FAILURE;
    wr_u32(out + 1, sender);
    wr_u32(out + 5, reason);
    wr_u32(out + 9, 0);  // empty description
    wr_u32(out + 13, 0); // empty language
    *out_len = 17;
    return 0;
}

// CHANNEL_OPEN_CONFIRMATION: byte || recipient(peer) || sender(local) || window || max.
static int build_open_confirm(const SshChannel *c, uint8_t *out, size_t cap, size_t *out_len)
{
    if (cap < 17)
        return -1;
    out[0] = SSH_MSG_CHANNEL_OPEN_CONFIRM;
    wr_u32(out + 1, c->peer_id);
    wr_u32(out + 5, c->local_id);
    wr_u32(out + 9, c->local_window);
    wr_u32(out + 13, SSH_CHAN_MAX_PACKET);
    *out_len = 17;
    return 0;
}

int ssh_channel_handle_open(uint8_t i, const uint8_t *payload, size_t len, uint8_t *out, size_t *out_len, size_t cap)
{
    if (i >= MAX_SSH_CONNS || len < 1 || payload[0] != SSH_MSG_CHANNEL_OPEN)
        return -1;

    size_t off = 1;
    const uint8_t *type;
    uint32_t type_len;
    if (!rd_string(payload, len, &off, &type, &type_len))
        return -1;
    if (off + 12 > len)
        return -1;
    uint32_t sender = rd_u32(payload + off);
    uint32_t init_window = rd_u32(payload + off + 4);
    uint32_t max_pkt = rd_u32(payload + off + 8);
    off += 12;

    bool is_session = (type_len == 7 && memcmp(type, "session", 7) == 0);
    bool is_dtcpip = (type_len == 12 && memcmp(type, "direct-tcpip", 12) == 0);
    if (!is_session && !is_dtcpip)
        return build_open_failure(out, cap, sender, 3u, out_len); // unknown channel type

    // direct-tcpip data: host(string) port(u32) orig_host(string) orig_port(u32).
    const uint8_t *fhost = nullptr;
    uint32_t fhost_len = 0;
    uint16_t fport = 0;
    if (is_dtcpip)
    {
        if (!g_forward_open_cb)
            return build_open_failure(out, cap, sender, 1u, out_len); // forwarding off: prohibited
        if (!rd_string(payload, len, &off, &fhost, &fhost_len) || off + 4 > len)
            return -1;
        fport = (uint16_t)rd_u32(payload + off); // orig host/port follow but are advisory
    }

    int slot = chan_alloc(i);
    if (slot < 0)
        return build_open_failure(out, cap, sender, 4u, out_len); // pool full

    SshChannel *c = &ssh_chan[i][slot];
    c->open = true;
    c->type = is_dtcpip ? (uint8_t)SSH_CHAN_DIRECT_TCPIP : (uint8_t)SSH_CHAN_SESSION;
    c->local_id = (uint32_t)slot;
    c->peer_id = sender;
    c->local_window = SSH_CHAN_WINDOW;
    c->peer_window = init_window;
    c->peer_max_pkt = max_pkt;

    if (is_dtcpip)
    {
        // The owner does the actual TCP connect (no I/O in this codec); on refusal
        // free the channel and fail closed.
        if (g_forward_open_cb(i, c->local_id, (const char *)fhost, fhost_len, fport) < 0)
        {
            c->open = false;
            return build_open_failure(out, cap, sender, 2u, out_len); // connect failed
        }
    }
    return build_open_confirm(c, out, cap, out_len);
}

// ---------------------------------------------------------------------------
// CHANNEL_REQUEST → SUCCESS / FAILURE
// ---------------------------------------------------------------------------

int ssh_channel_handle_request(uint8_t i, const uint8_t *payload, size_t len, uint8_t *out, size_t *out_len, size_t cap)
{
    *out_len = 0;
    if (i >= MAX_SSH_CONNS || len < 1 || payload[0] != SSH_MSG_CHANNEL_REQUEST)
        return -1;

    size_t off = 1;
    if (off + 4 > len)
        return -1;
    uint32_t recipient = rd_u32(payload + off); // our channel id
    off += 4;
    const uint8_t *rtype;
    uint32_t rtype_len;
    if (!rd_string(payload, len, &off, &rtype, &rtype_len))
        return -1;
    if (off >= len)
        return -1;
    bool want_reply = payload[off++] != 0;

    SshChannel *c = chan_by_id(i, recipient);
    if (!c)
        return -1;

    bool accept =
        (rtype_len == 5 && memcmp(rtype, "shell", 5) == 0) || (rtype_len == 4 && memcmp(rtype, "exec", 4) == 0) ||
        (rtype_len == 7 && memcmp(rtype, "pty-req", 7) == 0) || (rtype_len == 3 && memcmp(rtype, "env", 3) == 0);

    if (!want_reply)
        return 0;

    if (cap < 5)
        return -1;
    out[0] = accept ? SSH_MSG_CHANNEL_SUCCESS : SSH_MSG_CHANNEL_FAILURE;
    wr_u32(out + 1, c->peer_id);
    *out_len = 5;
    return 0;
}

// ---------------------------------------------------------------------------
// CHANNEL_DATA (inbound) + flow control
// ---------------------------------------------------------------------------

int ssh_channel_handle_data(uint8_t i, const uint8_t *payload, size_t len, uint8_t *out, size_t *out_len, size_t cap)
{
    *out_len = 0;
    if (i >= MAX_SSH_CONNS || len < 1 || payload[0] != SSH_MSG_CHANNEL_DATA)
        return -1;

    size_t off = 1;
    if (off + 4 > len)
        return -1;
    uint32_t recipient = rd_u32(payload + off);
    off += 4;
    const uint8_t *data;
    uint32_t dlen;
    if (!rd_string(payload, len, &off, &data, &dlen))
        return -1;

    SshChannel *c = chan_by_id(i, recipient);
    if (!c)
        return -1;
    if (dlen > c->local_window)
        return -1; // peer overran the advertised window (RFC 4254 §5.2)
    c->local_window -= dlen;

    if (dlen > 0)
    {
        if (c->type == SSH_CHAN_DIRECT_TCPIP) // forwarded TCP bytes -> the forward owner
        {
            if (g_forward_data_cb)
                g_forward_data_cb(i, c->local_id, data, dlen);
        }
        else if (g_data_cb) // session bytes -> the application
        {
            g_data_cb(i, c->local_id, data, dlen);
        }
    }

    // Replenish the window once it drops below half.
    if (c->local_window < SSH_CHAN_WINDOW / 2)
    {
        uint32_t add = SSH_CHAN_WINDOW - c->local_window;
        if (cap >= 9)
        {
            out[0] = SSH_MSG_CHANNEL_WINDOW_ADJUST;
            wr_u32(out + 1, c->peer_id);
            wr_u32(out + 5, add);
            *out_len = 9;
            c->local_window += add;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------
// CHANNEL_DATA (outbound)
// ---------------------------------------------------------------------------

int ssh_channel_build_data(uint8_t i, uint32_t channel, const uint8_t *data, size_t len, uint8_t *out, size_t *out_len,
                           size_t cap)
{
    SshChannel *c = (i < MAX_SSH_CONNS) ? chan_by_id(i, channel) : nullptr;
    if (!c)
        return -1;
    if (len > c->peer_window || len > c->peer_max_pkt)
        return -1; // would exceed the client's window / packet size
    if (cap < 1 + 4 + 4 + len)
        return -1;

    out[0] = SSH_MSG_CHANNEL_DATA;
    wr_u32(out + 1, c->peer_id);
    wr_u32(out + 5, (uint32_t)len);
    memcpy(out + 9, data, len);
    *out_len = 9 + len;
    c->peer_window -= (uint32_t)len;
    return 0;
}

// ---------------------------------------------------------------------------
// WINDOW_ADJUST (inbound)
// ---------------------------------------------------------------------------

int ssh_channel_handle_window_adjust(uint8_t i, const uint8_t *payload, size_t len)
{
    if (i >= MAX_SSH_CONNS || len < 9 || payload[0] != SSH_MSG_CHANNEL_WINDOW_ADJUST)
        return -1;
    SshChannel *c = chan_by_id(i, rd_u32(payload + 1));
    if (!c)
        return -1;
    uint32_t add = rd_u32(payload + 5);
    // Saturate rather than overflow the 32-bit window.
    uint32_t w = c->peer_window;
    c->peer_window = (w + add < w) ? 0xFFFFFFFFu : (w + add);
    return 0;
}

// ---------------------------------------------------------------------------
// EOF + CLOSE
// ---------------------------------------------------------------------------

// Frame EOF + CLOSE for an open channel and mark it closed (shared by the inbound
// handler and the app/teardown path).
static int build_close_chan(SshChannel *c, uint8_t *out, size_t *out_len, size_t cap)
{
    if (!c || cap < 10)
        return -1;
    uint32_t peer = c->peer_id;
    out[0] = SSH_MSG_CHANNEL_EOF;
    wr_u32(out + 1, peer);
    out[5] = SSH_MSG_CHANNEL_CLOSE;
    wr_u32(out + 6, peer);
    *out_len = 10;
    c->open = false;
    return 0;
}

int ssh_channel_build_close(uint8_t i, uint32_t channel, uint8_t *out, size_t *out_len, size_t cap)
{
    if (i >= MAX_SSH_CONNS)
        return -1;
    return build_close_chan(chan_by_id(i, channel), out, out_len, cap);
}

int ssh_channel_handle_close(uint8_t i, const uint8_t *payload, size_t len, uint8_t *out, size_t *out_len, size_t cap)
{
    *out_len = 0;
    if (i >= MAX_SSH_CONNS || len < 5 || payload[0] != SSH_MSG_CHANNEL_CLOSE)
        return -1;
    return build_close_chan(chan_by_id(i, rd_u32(payload + 1)), out, out_len, cap);
}
