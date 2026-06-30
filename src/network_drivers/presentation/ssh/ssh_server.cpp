// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_server.cpp
 * @brief SSH message dispatcher implementation.
 */

#include "ssh_server.h"
#include "shared_primitives/shim.h"
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
        // RFC 8308: with encryption now active, advertise the signature algorithms
        // we accept for pubkey userauth (server-sig-algs) so a modern client will
        // sign an RSA key - it otherwise reports "no mutual signature algorithm".
        // First encrypted message, before the client's SERVICE_REQUEST.
        if (s->ext_info_c && ssh_extinfo_build(buf, &n, sizeof(buf)) == 0)
            emit(i, buf, n);
        return 0;

    case SSH_MSG_EXT_INFO:
        return 0; // RFC 8308: a client may send its own EXT_INFO; we ignore it

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
        emit(i, buf, n); // SUCCESS (→ phase OPEN), PK_OK probe, or FAILURE
        // Brute-force defense (RFC 4252 §4): bound failed attempts per
        // connection. Only an actual USERAUTH_FAILURE counts - a SUCCESS or the
        // publickey "would-be-accepted" probe (PK_OK) does not.
        if (n > 0 && buf[0] == SSH_MSG_USERAUTH_FAILURE)
        {
            if (++s->auth_failures >= SSH_MAX_AUTH_ATTEMPTS)
            {
                // SSH_MSG_DISCONNECT payload: byte || uint32(reason) ||
                // string(desc) || string(lang="").  Emitted through the normal
                // packet path, then -1 tells the caller to close the socket.
                static const char desc[] = "too many authentication failures";
                const uint32_t dl = (uint32_t)(sizeof(desc) - 1);
                size_t o = 0;
                buf[o++] = SSH_MSG_DISCONNECT;
                buf[o++] = (uint8_t)(SSH_DISCONNECT_NO_MORE_AUTH_METHODS_AVAILABLE >> 24);
                buf[o++] = (uint8_t)(SSH_DISCONNECT_NO_MORE_AUTH_METHODS_AVAILABLE >> 16);
                buf[o++] = (uint8_t)(SSH_DISCONNECT_NO_MORE_AUTH_METHODS_AVAILABLE >> 8);
                buf[o++] = (uint8_t)(SSH_DISCONNECT_NO_MORE_AUTH_METHODS_AVAILABLE);
                buf[o++] = (uint8_t)(dl >> 24);
                buf[o++] = (uint8_t)(dl >> 16);
                buf[o++] = (uint8_t)(dl >> 8);
                buf[o++] = (uint8_t)(dl);
                memcpy(buf + o, desc, dl);
                o += dl;
                buf[o++] = 0; // language tag: empty string (4-byte length 0)
                buf[o++] = 0;
                buf[o++] = 0;
                buf[o++] = 0;
                emit(i, buf, o);
                return -1; // close the connection
            }
        }
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
        // handle_close frames CHANNEL_EOF + CHANNEL_CLOSE back to back, but each SSH
        // message must travel in its own binary packet (RFC 4253 6): a strict peer
        // runs packet_check_eom() after every message and rejects two in one. Emit
        // the two halves as separate packets.
        if (ssh_channel_handle_close(i, payload, len, buf, &n, sizeof(buf)) == 0 && n == 10)
        {
            emit(i, buf, 5);     // CHANNEL_EOF
            emit(i, buf + 5, 5); // CHANNEL_CLOSE
        }
        return 0;

    default: {
        // RFC 4253 §11.4: reply to an unrecognized message with
        // SSH_MSG_UNIMPLEMENTED carrying the rejected packet's sequence number.
        // ssh_pkt_recv() has already incremented seq_no_recv past this packet,
        // so the rejected packet's number is seq_no_recv - 1.
        uint32_t rej = ssh_pkt[i].seq_no_recv - 1u;
        buf[0] = SSH_MSG_UNIMPLEMENTED;
        buf[1] = (uint8_t)(rej >> 24);
        buf[2] = (uint8_t)(rej >> 16);
        buf[3] = (uint8_t)(rej >> 8);
        buf[4] = (uint8_t)(rej);
        emit(i, buf, 5);
        return 0;
    }
    }
}
