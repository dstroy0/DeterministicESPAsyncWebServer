// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file smb_client.cpp
 * @brief SMB2 client dialogue engine (see smb_client.h). Drives the wire codecs through the real
 *        NEGOTIATE / NTLMv2 SESSION_SETUP / TREE_CONNECT / CREATE exchange over a send/recv seam.
 */

#include "smb_client.h"

#if DETWS_ENABLE_SMB

#include "ntlm.h"
#include "ntlmssp.h"
#include "smb2.h"
#include "spnego.h"
#include <Arduino.h> // esp_fill_random() (real on device, mocked on native)
#include <string.h>

// ASCII/Latin-1 -> UTF-16LE (SMB paths are ASCII); returns byte length (2 * chars), 0 on null/overflow.
static size_t utf16le(const char *s, uint8_t *out, size_t cap)
{
    if (!s)
        return 0;
    size_t n = 0;
    for (; s[n]; n++)
    {
        if ((n * 2 + 2) > cap)
            return 0;
        out[n * 2] = (uint8_t)s[n];
        out[n * 2 + 1] = 0;
    }
    return n * 2;
}

// Find the MsvAvTimestamp (AvId 7) FILETIME in a CHALLENGE target-info blob; copy 8 bytes, else 0-fill.
static void find_av_timestamp(const uint8_t *ti, size_t ti_len, uint8_t out[8])
{
    memset(out, 0, 8);
    size_t p = 0;
    while (p + 4 <= ti_len)
    {
        uint16_t id = (uint16_t)(ti[p] | (ti[p + 1] << 8));
        uint16_t len = (uint16_t)(ti[p + 2] | (ti[p + 3] << 8));
        p += 4;
        if (id == 0) // MsvAvEOL
            break;
        if (id == 7 && len == 8 && p + 8 <= ti_len)
        {
            memcpy(out, ti + p, 8);
            return;
        }
        p += len;
    }
}

static bool read_exact(SmbRecvFn recv, void *ctx, uint8_t *buf, size_t n)
{
    size_t got = 0;
    while (got < n)
    {
        int r = recv(ctx, buf + got, n - got);
        if (r <= 0)
            return false;
        got += (size_t)r;
    }
    return true;
}

// Frame the SMB2 message that sits at frame+4 (msg_len bytes) with the Direct-TCP prefix and send it.
static bool send_msg(SmbSendFn send, void *ctx, uint8_t *frame, size_t msg_len)
{
    frame[0] = 0x00;
    frame[1] = (uint8_t)(msg_len >> 16);
    frame[2] = (uint8_t)(msg_len >> 8);
    frame[3] = (uint8_t)msg_len;
    size_t total = 4 + msg_len;
    return send(ctx, frame, total) == (int)total;
}

// Receive one Direct-TCP-framed SMB2 message into rx; return its length, -2 on overflow, -1 on IO.
static int recv_msg(SmbRecvFn recv, void *ctx, uint8_t *rx, size_t cap)
{
    uint8_t pre[4];
    if (!read_exact(recv, ctx, pre, 4) || pre[0] != 0x00)
        return -1;
    size_t len = ((size_t)pre[1] << 16) | ((size_t)pre[2] << 8) | pre[3];
    if (len == 0 || len > cap)
        return len ? -2 : -1;
    if (!read_exact(recv, ctx, rx, len))
        return -1;
    return (int)len;
}

// SMB dialogue working buffers, kept off the caller's stack: smb_open alone needs ~4 KB, which
// overflows the default 8 KB Arduino loopTask (seen on HW as "Stack canary watchpoint triggered").
// The client drives one sequential dialogue at a time (open -> read/write -> close), so a single
// owned working set is correct; it is not reentrant across two concurrent SMB connections.
struct SmbClientCtx
{
    uint8_t tx[DETWS_SMB_BUF];
    uint8_t rx[DETWS_SMB_BUF];
    uint8_t nt_resp[DETWS_SMB_BUF / 2];
    uint8_t ntauth[DETWS_SMB_BUF / 2];
    uint8_t sp2[DETWS_SMB_BUF / 2];
    uint8_t utf16[DETWS_SMB_BUF / 2];
};
static SmbClientCtx s_smb;

int smb_open(const SmbConfig *cfg, SmbHandle *h, SmbSendFn send, SmbRecvFn recv, void *ctx)
{
    if (!cfg || !h || !send || !recv || !cfg->user || !cfg->pass || !cfg->share || !cfg->path)
        return SMB_ERR_ARG;

    const char *domain = cfg->domain ? cfg->domain : "";
    size_t mlen;
    int rl;

    // 1. NEGOTIATE
    uint8_t guid[16];
    esp_fill_random(guid, 16);
    mlen = smb2_build_negotiate(s_smb.tx + 4, sizeof(s_smb.tx) - 4, guid, SMB2_NEGOTIATE_SIGNING_ENABLED);
    if (!mlen)
        return SMB_ERR_OVERFLOW;
    if (!send_msg(send, ctx, s_smb.tx, mlen))
        return SMB_ERR_IO;
    rl = recv_msg(recv, ctx, s_smb.rx, sizeof(s_smb.rx));
    if (rl < 0)
        return rl == -2 ? SMB_ERR_OVERFLOW : SMB_ERR_IO;
    Smb2NegotiateResp neg;
    if (!smb2_parse_negotiate_response(s_smb.rx, (size_t)rl, &neg))
        return SMB_ERR_PROTOCOL;

    // 2. SESSION_SETUP round 1: NTLMSSP NEGOTIATE wrapped in SPNEGO
    uint8_t ntneg[64], sp1[128];
    size_t ntneg_n = ntlmssp_build_negotiate(ntneg, sizeof(ntneg), NTLMSSP_CLIENT_DEFAULT_FLAGS);
    size_t sp1_n = spnego_wrap_negotiate(ntneg, ntneg_n, sp1, sizeof(sp1));
    mlen =
        smb2_build_session_setup(s_smb.tx + 4, sizeof(s_smb.tx) - 4, 1, 0, SMB2_NEGOTIATE_SIGNING_ENABLED, sp1, sp1_n);
    if (!mlen)
        return SMB_ERR_OVERFLOW;
    if (!send_msg(send, ctx, s_smb.tx, mlen))
        return SMB_ERR_IO;
    rl = recv_msg(recv, ctx, s_smb.rx, sizeof(s_smb.rx));
    if (rl < 0)
        return rl == -2 ? SMB_ERR_OVERFLOW : SMB_ERR_IO;
    Smb2Header h1;
    if (!smb2_parse_header(s_smb.rx, (size_t)rl, &h1) || h1.status != SMB2_STATUS_MORE_PROCESSING_REQUIRED)
        return SMB_ERR_AUTH;
    uint64_t session_id = h1.session_id;
    Smb2SessionSetupResp ss1;
    if (!smb2_parse_session_setup_response(s_smb.rx, (size_t)rl, &ss1) || !ss1.sec_buf)
        return SMB_ERR_PROTOCOL;
    const uint8_t *chal_tok = nullptr;
    size_t chal_len = 0;
    if (!spnego_parse_response(ss1.sec_buf, ss1.sec_buf_len, &chal_tok, &chal_len))
        return SMB_ERR_PROTOCOL;
    NtlmChallenge ch;
    if (!ntlmssp_parse_challenge(chal_tok, chal_len, &ch))
        return SMB_ERR_PROTOCOL;

    // 3. Compute the NTLMv2 response, wrap the AUTHENTICATE in SPNEGO
    uint8_t nt_hash[16], owf[16];
    ntlm_nt_hash(cfg->pass, nt_hash);
    if (!ntlm_ntowfv2(nt_hash, cfg->user, domain, owf))
        return SMB_ERR_OVERFLOW;
    uint8_t cli_chal[8], ts[8], skey[16];
    esp_fill_random(cli_chal, 8);
    find_av_timestamp(ch.target_info, ch.target_info_len, ts);
    size_t nt_len = ntlm_v2_response(owf, ch.server_challenge, cli_chal, ts, ch.target_info, ch.target_info_len,
                                     s_smb.nt_resp, sizeof(s_smb.nt_resp), skey);
    if (!nt_len)
        return SMB_ERR_OVERFLOW;
    size_t ntauth_n = ntlmssp_build_authenticate(s_smb.ntauth, sizeof(s_smb.ntauth), nullptr, 0, s_smb.nt_resp, nt_len,
                                                 domain, cfg->user, cfg->workstation, ch.flags);
    if (!ntauth_n)
        return SMB_ERR_OVERFLOW;
    size_t sp2_n = spnego_wrap_authenticate(s_smb.ntauth, ntauth_n, s_smb.sp2, sizeof(s_smb.sp2));
    if (!sp2_n)
        return SMB_ERR_OVERFLOW;

    // 4. SESSION_SETUP round 2 (echo the server SessionId)
    mlen = smb2_build_session_setup(s_smb.tx + 4, sizeof(s_smb.tx) - 4, 2, session_id, SMB2_NEGOTIATE_SIGNING_ENABLED,
                                    s_smb.sp2, sp2_n);
    if (!mlen)
        return SMB_ERR_OVERFLOW;
    if (!send_msg(send, ctx, s_smb.tx, mlen))
        return SMB_ERR_IO;
    rl = recv_msg(recv, ctx, s_smb.rx, sizeof(s_smb.rx));
    if (rl < 0)
        return rl == -2 ? SMB_ERR_OVERFLOW : SMB_ERR_IO;
    Smb2Header h2;
    if (!smb2_parse_header(s_smb.rx, (size_t)rl, &h2))
        return SMB_ERR_PROTOCOL;
    if (h2.status != SMB2_STATUS_SUCCESS)
        return SMB_ERR_AUTH;

    // 5. TREE_CONNECT to \\server\share
    size_t utf16_n = utf16le(cfg->share, s_smb.utf16, sizeof(s_smb.utf16));
    if (!utf16_n)
        return SMB_ERR_OVERFLOW;
    mlen = smb2_build_tree_connect(s_smb.tx + 4, sizeof(s_smb.tx) - 4, 3, session_id, s_smb.utf16, utf16_n);
    if (!mlen)
        return SMB_ERR_OVERFLOW;
    if (!send_msg(send, ctx, s_smb.tx, mlen))
        return SMB_ERR_IO;
    rl = recv_msg(recv, ctx, s_smb.rx, sizeof(s_smb.rx));
    if (rl < 0)
        return rl == -2 ? SMB_ERR_OVERFLOW : SMB_ERR_IO;
    Smb2Header h3;
    Smb2TreeConnectResp tc;
    if (!smb2_parse_header(s_smb.rx, (size_t)rl, &h3) || h3.status != SMB2_STATUS_SUCCESS)
        return SMB_ERR_PROTOCOL;
    if (!smb2_parse_tree_connect_response(s_smb.rx, (size_t)rl, &tc))
        return SMB_ERR_PROTOCOL;
    uint32_t tree_id = h3.tree_id;

    // 6. CREATE (open) the file
    utf16_n = utf16le(cfg->path, s_smb.utf16, sizeof(s_smb.utf16));
    if (!utf16_n)
        return SMB_ERR_OVERFLOW;
    mlen = smb2_build_create(s_smb.tx + 4, sizeof(s_smb.tx) - 4, 4, session_id, tree_id, cfg->desired_access,
                             SMB2_FILE_SHARE_READ | SMB2_FILE_SHARE_WRITE, cfg->disposition,
                             SMB2_FILE_NON_DIRECTORY_FILE, s_smb.utf16, utf16_n);
    if (!mlen)
        return SMB_ERR_OVERFLOW;
    if (!send_msg(send, ctx, s_smb.tx, mlen))
        return SMB_ERR_IO;
    rl = recv_msg(recv, ctx, s_smb.rx, sizeof(s_smb.rx));
    if (rl < 0)
        return rl == -2 ? SMB_ERR_OVERFLOW : SMB_ERR_IO;
    Smb2Header h4;
    Smb2CreateResp cr;
    if (!smb2_parse_header(s_smb.rx, (size_t)rl, &h4) || h4.status != SMB2_STATUS_SUCCESS)
        return SMB_ERR_PROTOCOL;
    if (!smb2_parse_create_response(s_smb.rx, (size_t)rl, &cr))
        return SMB_ERR_PROTOCOL;

    h->session_id = session_id;
    h->tree_id = tree_id;
    memcpy(h->file_id, cr.file_id, 16);
    h->file_size = cr.end_of_file;
    h->next_message_id = 5;
    return SMB_OK;
}

int smb_close(SmbHandle *h, SmbSendFn send, SmbRecvFn recv, void *ctx)
{
    if (!h || !send || !recv)
        return SMB_ERR_ARG;
    uint8_t tx[128], rx[128];
    size_t mlen = smb2_build_close(tx + 4, sizeof(tx) - 4, h->next_message_id, h->session_id, h->tree_id, h->file_id);
    if (!mlen)
        return SMB_ERR_OVERFLOW;
    if (!send_msg(send, ctx, tx, mlen))
        return SMB_ERR_IO;
    int rl = recv_msg(recv, ctx, rx, sizeof(rx));
    if (rl < 0)
        return rl == -2 ? SMB_ERR_OVERFLOW : SMB_ERR_IO;
    Smb2Header hd;
    Smb2CloseResp cl;
    if (!smb2_parse_header(rx, (size_t)rl, &hd) || hd.status != SMB2_STATUS_SUCCESS)
        return SMB_ERR_PROTOCOL;
    if (!smb2_parse_close_response(rx, (size_t)rl, &cl))
        return SMB_ERR_PROTOCOL;
    h->next_message_id++;
    return SMB_OK;
}

int smb_read(SmbHandle *h, uint64_t offset, uint8_t *out, size_t cap, size_t *out_len, SmbSendFn send, SmbRecvFn recv,
             void *ctx)
{
    if (!h || !out || !out_len || !send || !recv)
        return SMB_ERR_ARG;
    *out_len = 0;
    const size_t chunk_max = DETWS_SMB_BUF - 96; // room for the header + READ response body
    size_t total = 0;
    while (total < cap)
    {
        size_t want = cap - total;
        if (want > chunk_max)
            want = chunk_max;
        size_t mlen = smb2_build_read(s_smb.tx + 4, sizeof(s_smb.tx) - 4, h->next_message_id, h->session_id, h->tree_id,
                                      h->file_id, (uint32_t)want, offset + total);
        if (!mlen)
            return SMB_ERR_OVERFLOW;
        if (!send_msg(send, ctx, s_smb.tx, mlen))
            return SMB_ERR_IO;
        int rl = recv_msg(recv, ctx, s_smb.rx, sizeof(s_smb.rx));
        if (rl < 0)
            return rl == -2 ? SMB_ERR_OVERFLOW : SMB_ERR_IO;
        Smb2Header hd;
        if (!smb2_parse_header(s_smb.rx, (size_t)rl, &hd))
            return SMB_ERR_PROTOCOL;
        h->next_message_id++;
        if (hd.status == SMB2_STATUS_END_OF_FILE)
            break;
        if (hd.status != SMB2_STATUS_SUCCESS)
            return SMB_ERR_PROTOCOL;
        Smb2ReadResp r;
        if (!smb2_parse_read_response(s_smb.rx, (size_t)rl, &r) || r.data_len > want)
            return SMB_ERR_PROTOCOL;
        if (r.data_len == 0)
            break;
        memcpy(out + total, r.data, r.data_len);
        total += r.data_len;
        if (r.data_len < want)
            break; // a short read means we reached the end of the file
    }
    *out_len = total;
    return SMB_OK;
}

int smb_write(SmbHandle *h, uint64_t offset, const uint8_t *data, size_t len, size_t *written, SmbSendFn send,
              SmbRecvFn recv, void *ctx)
{
    if (!h || !data || !written || !send || !recv)
        return SMB_ERR_ARG;
    *written = 0;
    const size_t chunk_max = DETWS_SMB_BUF - 128; // room for the header + WRITE request body
    size_t total = 0;
    while (total < len)
    {
        size_t want = len - total;
        if (want > chunk_max)
            want = chunk_max;
        size_t mlen = smb2_build_write(s_smb.tx + 4, sizeof(s_smb.tx) - 4, h->next_message_id, h->session_id,
                                       h->tree_id, h->file_id, data + total, want, offset + total);
        if (!mlen)
            return SMB_ERR_OVERFLOW;
        if (!send_msg(send, ctx, s_smb.tx, mlen))
            return SMB_ERR_IO;
        int rl = recv_msg(recv, ctx, s_smb.rx, sizeof(s_smb.rx));
        if (rl < 0)
            return rl == -2 ? SMB_ERR_OVERFLOW : SMB_ERR_IO;
        Smb2Header hd;
        if (!smb2_parse_header(s_smb.rx, (size_t)rl, &hd))
            return SMB_ERR_PROTOCOL;
        h->next_message_id++;
        if (hd.status != SMB2_STATUS_SUCCESS)
            return SMB_ERR_PROTOCOL;
        Smb2WriteResp w;
        if (!smb2_parse_write_response(s_smb.rx, (size_t)rl, &w) || w.count == 0 || w.count > want)
            return SMB_ERR_PROTOCOL; // no progress or a bogus count
        total += w.count;
    }
    if (offset + total > h->file_size)
        h->file_size = offset + total;
    *written = total;
    return SMB_OK;
}

#endif // DETWS_ENABLE_SMB
