// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_conn.cpp
 * @brief TCP-transport ↔ SSH-protocol glue.
 */

#include "ssh_conn.h"
#include "../../transport/transport.h"
#include "lwip/tcp.h"
#include "ssh_channel.h"
#include "ssh_keymat.h"
#include "ssh_packet.h"
#include "ssh_server.h"
#include "ssh_transport.h"
#include <string.h>

// SSH session slot ↔ TCP conn slot mapping. conn_for_ssh[j] == 0xFF means the
// SSH slot j is free.
static uint8_t conn_for_ssh[MAX_SSH_CONNS];
static bool g_init_done = false;
static volatile bool g_close[MAX_SSH_CONNS];

static void ensure_init()
{
    if (g_init_done)
        return;
    for (uint8_t j = 0; j < MAX_SSH_CONNS; j++)
        conn_for_ssh[j] = 0xFF;
    g_init_done = true;
}

// ---------------------------------------------------------------------------
// Outbound: frame + (encrypt/MAC) one SSH message and write it to the socket.
// ---------------------------------------------------------------------------

static void ssh_emit(uint8_t i, const uint8_t *payload, size_t len)
{
    if (i >= MAX_SSH_CONNS || conn_for_ssh[i] == 0xFF)
        return;
    TcpConn *conn = &conn_pool[conn_for_ssh[i]];
    if (conn->state != CONN_ACTIVE || !conn->pcb)
        return;

    static uint8_t wire[SSH_PKT_BUF_SIZE + SSH_HMAC_SHA256_LEN];
    size_t wlen = 0;
    if (ssh_pkt_send(i, payload, len, wire, &wlen, sizeof(wire)) != 0)
        return;
    tcp_write(conn->pcb, wire, (u16_t)wlen, TCP_WRITE_FLAG_COPY);
    tcp_output(conn->pcb);
}

// ssh_pkt_recv handler: dispatch one decrypted message, remember fatal results.
static void ssh_msg_handler(uint8_t i, uint8_t msg_type, const uint8_t *payload, size_t len)
{
    if (ssh_server_dispatch(i, msg_type, payload, len) < 0)
        g_close[i] = true;
}

void ssh_conn_setup()
{
    ensure_init();
    ssh_server_set_emit_cb(ssh_emit);
}

// ---------------------------------------------------------------------------
// Connection lifecycle
// ---------------------------------------------------------------------------

void ssh_conn_accept(uint8_t conn_slot)
{
    ensure_init();
    TcpConn *conn = &conn_pool[conn_slot];

    // Allocate a free SSH session slot.
    uint8_t j = 0xFF;
    for (uint8_t k = 0; k < MAX_SSH_CONNS; k++)
        if (conn_for_ssh[k] == 0xFF)
        {
            j = k;
            break;
        }
    if (j == 0xFF)
    {
        // No SSH capacity: drop the connection.
        if (conn->pcb)
        {
            tcp_arg(conn->pcb, nullptr);
            if (tcp_close(conn->pcb) != ERR_OK)
                tcp_abort(conn->pcb);
        }
        conn->state = CONN_FREE;
        conn->pcb = nullptr;
        return;
    }

    conn_for_ssh[j] = conn_slot;
    conn->ssh_id = j;
    g_close[j] = false;

    ssh_transport_init(j);
    ssh_pkt_init(j);
    ssh_channel_init(j);

    // Send the server identification banner (raw, before any binary packet).
    uint8_t banner[64];
    size_t blen = 0;
    if (ssh_transport_server_banner(banner, &blen, sizeof(banner)) == 0 && conn->pcb)
    {
        tcp_write(conn->pcb, banner, (u16_t)blen, TCP_WRITE_FLAG_COPY);
        tcp_output(conn->pcb);
    }
}

static void close_conn(uint8_t conn_slot)
{
    TcpConn *conn = &conn_pool[conn_slot];
    if (conn->pcb)
    {
        tcp_arg(conn->pcb, nullptr);
        if (tcp_close(conn->pcb) != ERR_OK)
            tcp_abort(conn->pcb);
    }
    conn->state = CONN_FREE;
    conn->pcb = nullptr;
    ssh_conn_close(conn_slot);
}

void ssh_conn_rx(uint8_t conn_slot)
{
    TcpConn *conn = &conn_pool[conn_slot];
    uint8_t j = conn->ssh_id;
    if (j >= MAX_SSH_CONNS || conn_for_ssh[j] != conn_slot)
        return;

    // Drain the ring buffer into a linear scratch buffer.
    static uint8_t buf[RX_BUF_SIZE];
    size_t n = 0;
    while (conn->rx_tail != conn->rx_head && n < sizeof(buf))
    {
        buf[n++] = conn->rx_buffer[conn->rx_tail];
        conn->rx_tail = (conn->rx_tail + 1) % RX_BUF_SIZE;
    }
    if (n == 0)
        return;

    size_t off = 0;
    if (ssh_sess[j].phase == SSH_PHASE_BANNER)
    {
        size_t consumed = 0;
        int rc = ssh_transport_recv_banner(j, buf, n, &consumed);
        if (rc < 0)
        {
            close_conn(conn_slot);
            return;
        }
        if (rc == 0)
            return; // need more banner bytes
        off = consumed;
        ssh_sess[j].phase = SSH_PHASE_KEXINIT;
    }

    if (off < n)
        ssh_pkt_recv(j, buf + off, n - off, ssh_msg_handler);

    ssh_wipe(buf, n);

    if (g_close[j])
        close_conn(conn_slot);
}

void ssh_conn_close(uint8_t conn_slot)
{
    TcpConn *conn = &conn_pool[conn_slot];
    uint8_t j = conn->ssh_id;
    if (j < MAX_SSH_CONNS)
    {
        // Zero all key material and session state for this slot.
        ssh_keymat_wipe(j);
        ssh_dh_wipe(j);
        ssh_wipe(&ssh_sess[j], sizeof(SshSession));
        conn_for_ssh[j] = 0xFF;
    }
    conn->ssh_id = SSH_ID_NONE;
}
