// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_channel.cpp
 * @brief SSH connection protocol - single session channel (RFC 4254).
 */

#include "ssh_channel.h"
#include "ssh_packet.h" // SSH_MSG_CHANNEL_*
#include <string.h>

SshChannel ssh_chan[MAX_SSH_CONNS];

static SshChannelDataCb g_data_cb = nullptr;

void ssh_channel_set_data_cb(SshChannelDataCb cb)
{
    g_data_cb = cb;
}

void ssh_channel_init(uint8_t i)
{
    if (i >= MAX_SSH_CONNS)
        return;
    memset(&ssh_chan[i], 0, sizeof(SshChannel));
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
// CHANNEL_OPEN → CONFIRMATION / FAILURE
// ---------------------------------------------------------------------------

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

    bool is_session = (type_len == 7 && memcmp(type, "session", 7) == 0);

    if (!is_session || ssh_chan[i].open)
    {
        // CHANNEL_OPEN_FAILURE: byte || recipient || reason || desc || lang
        uint32_t reason = is_session ? 4u /* resource shortage */ : 3u /* unknown type */;
        if (cap < 1 + 4 + 4 + 4 + 4)
            return -1;
        out[0] = SSH_MSG_CHANNEL_OPEN_FAILURE;
        wr_u32(out + 1, sender);
        wr_u32(out + 5, reason);
        wr_u32(out + 9, 0);  // empty description
        wr_u32(out + 13, 0); // empty language
        *out_len = 17;
        return 0;
    }

    SshChannel *c = &ssh_chan[i];
    c->open = true;
    c->local_id = i;
    c->peer_id = sender;
    c->local_window = SSH_CHAN_WINDOW;
    c->peer_window = init_window;
    c->peer_max_pkt = max_pkt;

    // CONFIRMATION: byte || recipient(sender) || sender(local) || window || max
    if (cap < 1 + 16)
        return -1;
    out[0] = SSH_MSG_CHANNEL_OPEN_CONFIRM;
    wr_u32(out + 1, c->peer_id);
    wr_u32(out + 5, c->local_id);
    wr_u32(out + 9, c->local_window);
    wr_u32(out + 13, SSH_CHAN_MAX_PACKET);
    *out_len = 17;
    return 0;
}

// ---------------------------------------------------------------------------
// CHANNEL_REQUEST → SUCCESS / FAILURE
// ---------------------------------------------------------------------------

int ssh_channel_handle_request(uint8_t i, const uint8_t *payload, size_t len, uint8_t *out, size_t *out_len, size_t cap)
{
    if (i >= MAX_SSH_CONNS || len < 1 || payload[0] != SSH_MSG_CHANNEL_REQUEST)
        return -1;

    size_t off = 1;
    if (off + 4 > len)
        return -1;
    off += 4; // recipient channel (ours)
    const uint8_t *rtype;
    uint32_t rtype_len;
    if (!rd_string(payload, len, &off, &rtype, &rtype_len))
        return -1;
    if (off >= len)
        return -1;
    bool want_reply = payload[off++] != 0;

    bool accept =
        (rtype_len == 5 && memcmp(rtype, "shell", 5) == 0) || (rtype_len == 4 && memcmp(rtype, "exec", 4) == 0) ||
        (rtype_len == 7 && memcmp(rtype, "pty-req", 7) == 0) || (rtype_len == 3 && memcmp(rtype, "env", 3) == 0);

    *out_len = 0;
    if (!want_reply)
        return 0;

    if (cap < 5)
        return -1;
    out[0] = accept ? SSH_MSG_CHANNEL_SUCCESS : SSH_MSG_CHANNEL_FAILURE;
    wr_u32(out + 1, ssh_chan[i].peer_id);
    *out_len = 5;
    return 0;
}

// ---------------------------------------------------------------------------
// CHANNEL_DATA (inbound) + flow control
// ---------------------------------------------------------------------------

int ssh_channel_handle_data(uint8_t i, const uint8_t *payload, size_t len, uint8_t *out, size_t *out_len, size_t cap)
{
    *out_len = 0;
    if (i >= MAX_SSH_CONNS || !ssh_chan[i].open || len < 1 || payload[0] != SSH_MSG_CHANNEL_DATA)
        return -1;

    size_t off = 1;
    if (off + 4 > len)
        return -1;
    off += 4; // recipient channel
    const uint8_t *data;
    uint32_t dlen;
    if (!rd_string(payload, len, &off, &data, &dlen))
        return -1;

    SshChannel *c = &ssh_chan[i];
    if (dlen > c->local_window)
        return -1; // peer overran the advertised window (RFC 4254 §5.2)
    c->local_window -= dlen;

    if (g_data_cb && dlen > 0)
        g_data_cb(i, data, dlen);

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

int ssh_channel_build_data(uint8_t i, const uint8_t *data, size_t len, uint8_t *out, size_t *out_len, size_t cap)
{
    if (i >= MAX_SSH_CONNS || !ssh_chan[i].open)
        return -1;
    SshChannel *c = &ssh_chan[i];
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
    if (i >= MAX_SSH_CONNS || !ssh_chan[i].open || len < 9 || payload[0] != SSH_MSG_CHANNEL_WINDOW_ADJUST)
        return -1;
    uint32_t add = rd_u32(payload + 5);
    // Saturate rather than overflow the 32-bit window.
    uint32_t w = ssh_chan[i].peer_window;
    ssh_chan[i].peer_window = (w + add < w) ? 0xFFFFFFFFu : (w + add);
    return 0;
}

// ---------------------------------------------------------------------------
// EOF + CLOSE (outbound)
// ---------------------------------------------------------------------------

int ssh_channel_build_close(uint8_t i, uint8_t *out, size_t *out_len, size_t cap)
{
    if (i >= MAX_SSH_CONNS || !ssh_chan[i].open)
        return -1;
    if (cap < 10)
        return -1;
    uint32_t peer = ssh_chan[i].peer_id;
    out[0] = SSH_MSG_CHANNEL_EOF;
    wr_u32(out + 1, peer);
    out[5] = SSH_MSG_CHANNEL_CLOSE;
    wr_u32(out + 6, peer);
    *out_len = 10;
    ssh_chan[i].open = false;
    return 0;
}
