// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_forward.cpp
 * @brief SSH direct-tcpip port-forwarding owner - outbound TCP + byte bridge.
 */

#include "network_drivers/presentation/ssh/ssh_forward.h"

#if DETWS_SSH_PORT_FORWARD

// One forwarded TCP connection: an SSH channel bridged to a client-transport slot.
struct SshFwd
{
    bool active;
    uint8_t ssh_slot;
    uint32_t channel; // local channel id (== ssh_chan row index)
    int cid;          // det_client connection id
};

static SshFwd g_fwd[DETWS_SSH_FWD_MAX];
static SshForwardPolicyCb g_policy = nullptr;

// Target -> client iterations per channel per poll: bounds the work each loop so
// one busy tunnel cannot starve the others (DETWS_SSH_FWD_CHUNK bytes each).
static const int kFwdBurst = 4;

static int fwd_find_free()
{
    for (int i = 0; i < DETWS_SSH_FWD_MAX; i++)
        if (!g_fwd[i].active)
            return i;
    return -1;
}

static SshFwd *fwd_lookup(uint8_t ssh_slot, uint32_t channel)
{
    for (int i = 0; i < DETWS_SSH_FWD_MAX; i++)
        if (g_fwd[i].active && g_fwd[i].ssh_slot == ssh_slot && g_fwd[i].channel == channel)
            return &g_fwd[i];
    return nullptr;
}

// direct-tcpip open: check policy, connect to host:port (blocking), bind a slot.
// Returns 0 to confirm the channel, < 0 to refuse (denied / connect failed / full).
static int on_forward_open(uint8_t ssh_slot, uint32_t channel, const char *host, size_t host_len, uint16_t port)
{
    int idx = fwd_find_free();
    if (idx < 0)
        return -1; // no forward capacity
    char hbuf[DETWS_SSH_FWD_HOST_MAX];
    if (host_len == 0 || host_len >= sizeof(hbuf))
        return -1;
    memcpy(hbuf, host, host_len);
    hbuf[host_len] = 0;
    if (g_policy && !g_policy(hbuf, port))
        return -1;                                                   // target administratively denied
    int cid = det_client_open(hbuf, port, DETWS_SSH_FWD_CONNECT_MS); // blocks on DNS + connect
    if (cid < 0)
        return -1; // -> CHANNEL_OPEN_FAILURE (connect failed)
    g_fwd[idx].active = true;
    g_fwd[idx].ssh_slot = ssh_slot;
    g_fwd[idx].channel = channel;
    g_fwd[idx].cid = cid;
    return 0;
}

// Inbound channel bytes (client -> target).
static void on_forward_data(uint8_t ssh_slot, uint32_t channel, const uint8_t *data, size_t len)
{
    SshFwd *f = fwd_lookup(ssh_slot, channel);
    if (f)
        det_client_send(f->cid, data, len);
}

void ssh_forward_set_policy_cb(SshForwardPolicyCb cb)
{
    g_policy = cb;
}

void ssh_forward_begin()
{
    for (int i = 0; i < DETWS_SSH_FWD_MAX; i++)
        g_fwd[i].active = false;
    ssh_channel_set_forward_open_cb(on_forward_open);
    ssh_channel_set_forward_data_cb(on_forward_data);
}

void ssh_forward_pump(uint8_t ssh_slot)
{
    uint8_t buf[DETWS_SSH_FWD_CHUNK];
    for (int i = 0; i < DETWS_SSH_FWD_MAX; i++)
    {
        SshFwd *f = &g_fwd[i];
        if (!f->active || f->ssh_slot != ssh_slot)
            continue;
        if (f->channel >= DETWS_SSH_MAX_CHANNELS) // defensive: stale binding
        {
            det_client_close(f->cid);
            f->active = false;
            continue;
        }
        SshChannel *c = &ssh_chan[ssh_slot][f->channel];

        // Client closed its side of the channel: drop the target socket.
        if (!c->open)
        {
            det_client_close(f->cid);
            f->active = false;
            continue;
        }

        // Target -> client: forward what the peer window allows, bounded per poll.
        for (int burst = 0; burst < kFwdBurst; burst++)
        {
            size_t avail = det_client_available(f->cid);
            uint32_t win = c->peer_window;
            if (avail == 0 || win == 0)
                break;
            size_t budget = avail;
            if (budget > win)
                budget = win;
            if (c->peer_max_pkt && budget > c->peer_max_pkt)
                budget = c->peer_max_pkt;
            if (budget > sizeof(buf))
                budget = sizeof(buf);
            size_t n = det_client_read(f->cid, buf, budget);
            if (n == 0)
                break;
            if (ssh_conn_send(ssh_slot, f->channel, buf, n) < 0)
                break; // sized to the window, so this should send; retry next poll
        }

        // Target closed (FIN) and fully drained: EOF + CLOSE to the client, free.
        if (det_client_is_closed(f->cid) && det_client_available(f->cid) == 0)
        {
            ssh_conn_close_channel(ssh_slot, f->channel);
            det_client_close(f->cid);
            f->active = false;
        }
    }
}

void ssh_forward_reset(uint8_t ssh_slot)
{
    for (int i = 0; i < DETWS_SSH_FWD_MAX; i++)
        if (g_fwd[i].active && g_fwd[i].ssh_slot == ssh_slot)
        {
            det_client_close(g_fwd[i].cid);
            g_fwd[i].active = false;
        }
}

#endif // DETWS_SSH_PORT_FORWARD
