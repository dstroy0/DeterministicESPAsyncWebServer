// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_server.cpp
 * @brief SSH message dispatcher implementation.
 */

#include "ssh_server.h"
#include "ssh_auth.h"
#include "ssh_channel.h"
#include "ssh_dh.h"
#include "ssh_packet.h"
#include "ssh_transport.h"

static SshEmitCb g_emit = nullptr;

void ssh_server_set_emit_cb(SshEmitCb cb)
{
    g_emit = cb;
}

static inline void emit(uint8_t i, const uint8_t *p, size_t n)
{
    if (g_emit && n > 0)
        g_emit(i, p, n);
}

int ssh_server_dispatch(uint8_t i, uint8_t msg_type, const uint8_t *payload, size_t len)
{
    if (i >= MAX_SSH_CONNS)
        return -1;
    SshSession *s = &ssh_sess[i];

    uint8_t buf[SSH_PKT_BUF_SIZE];
    size_t n = 0;

    switch (msg_type)
    {
    case SSH_MSG_IGNORE:
        return 0;

    case SSH_MSG_DISCONNECT:
        return -1; // peer is closing

    case SSH_MSG_KEXINIT:
        // Initial KEX or an in-session re-key request. Negotiate, reply with our
        // own KEXINIT, generate a fresh ephemeral, and await KEXDH_INIT.
        if (ssh_kexinit_parse(i, payload, len) != 0)
            return -1;
        if (ssh_kexinit_build(i, buf, &n, sizeof(buf)) != 0)
            return -1;
        emit(i, buf, n);
        if (ssh_dh_generate(i) != 0)
            return -1;
        s->phase = SSH_PHASE_DH_INIT;
        return 0;

    case SSH_MSG_KEXDH_INIT:
        if (s->phase != SSH_PHASE_DH_INIT)
            return -1;
        if (ssh_kexdh_handle(i, payload, len, buf, &n, sizeof(buf)) != 0)
            return -1;
        emit(i, buf, n); // KEXDH_REPLY
        {
            uint8_t newkeys = SSH_MSG_NEWKEYS;
            emit(i, &newkeys, 1); // server NEWKEYS (still unencrypted)
        }
        return 0; // ssh_kexdh_handle advanced phase to NEWKEYS

    case SSH_MSG_NEWKEYS:
        ssh_newkeys_complete(i); // activate encryption; → SERVICE or OPEN
        return 0;

    case SSH_MSG_SERVICE_REQUEST:
        if (ssh_auth_handle_service_request(payload, len, buf, &n, sizeof(buf)) != 0)
            return -1;
        emit(i, buf, n);
        s->phase = SSH_PHASE_AUTH;
        return 0;

    case SSH_MSG_USERAUTH_REQUEST:
        if (s->phase != SSH_PHASE_AUTH)
            return -1;
        if (ssh_auth_handle_request(i, payload, len, buf, &n, sizeof(buf)) != 0)
            return -1;
        emit(i, buf, n); // SUCCESS (→ phase OPEN) or FAILURE
        return 0;

    case SSH_MSG_CHANNEL_OPEN:
        if (!s->authed)
            return -1;
        if (ssh_channel_handle_open(i, payload, len, buf, &n, sizeof(buf)) != 0)
            return -1;
        emit(i, buf, n);
        return 0;

    case SSH_MSG_CHANNEL_REQUEST:
        if (!s->authed)
            return -1;
        if (ssh_channel_handle_request(i, payload, len, buf, &n, sizeof(buf)) != 0)
            return -1;
        emit(i, buf, n); // SUCCESS/FAILURE only when want_reply was set
        return 0;

    case SSH_MSG_CHANNEL_DATA:
        if (!s->authed)
            return -1;
        if (ssh_channel_handle_data(i, payload, len, buf, &n, sizeof(buf)) != 0)
            return -1;
        emit(i, buf, n); // WINDOW_ADJUST when the receive window is replenished
        return 0;

    case SSH_MSG_CHANNEL_WINDOW_ADJUST:
        ssh_channel_handle_window_adjust(i, payload, len);
        return 0;

    case SSH_MSG_CHANNEL_EOF:
        return 0;

    case SSH_MSG_CHANNEL_CLOSE:
        if (ssh_channel_build_close(i, buf, &n, sizeof(buf)) == 0)
            emit(i, buf, n);
        return 0;

    default:
        // Unrecognized message: ignore (a strict SSH_MSG_UNIMPLEMENTED reply
        // requires the rejected packet's sequence number, which is not exposed
        // to the dispatcher).
        return 0;
    }
}
