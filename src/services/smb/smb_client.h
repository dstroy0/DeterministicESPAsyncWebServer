// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file smb_client.h
 * @brief SMB2 client dialogue engine (DETWS_ENABLE_SMB) - drives the smb2 / ntlm / spnego wire
 *        codecs through a real session to open a file on a Windows share.
 *
 * The wire codecs (smb2.h, ntlm.h, ntlmssp.h, spnego.h) are pure builders/parsers; this ties them
 * into the actual exchange: NEGOTIATE, the two-round NTLMv2 SESSION_SETUP (SPNEGO-wrapped),
 * TREE_CONNECT to `\\server\share`, and CREATE to open the file - handing back a handle that
 * smb_read / smb_write / smb_close use. Like the SMTP engine it is written against a send/recv seam,
 * so the whole exchange is host-tested with a scripted mock SMB2 server (no lwIP / real share).
 *
 * Direct-TCP framing (the 4-byte length prefix) is handled here: each request is framed before
 * `send`, each response is de-framed after `recv` (accumulating until a full message arrives).
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SMB_CLIENT_H
#define DETERMINISTICESPASYNCWEBSERVER_SMB_CLIENT_H

#include "ServerConfig.h"

#if DETWS_ENABLE_SMB

#include <stddef.h>
#include <stdint.h>

/** @brief Result of an SMB client operation. 0 is success; each failure is a distinct code. */
enum class SmbResult : int32_t
{
    SMB_OK = 0,
    SMB_ERR_ARG = -1,      ///< a required field was null/empty
    SMB_ERR_IO = -2,       ///< a send/recv failed, timed out, or the peer closed mid-message
    SMB_ERR_PROTOCOL = -3, ///< a malformed response, or an unexpected NT status
    SMB_ERR_AUTH = -4,     ///< SESSION_SETUP was rejected (bad user/password/domain)
    SMB_ERR_OVERFLOW = -5, ///< a message did not fit the work buffer (DETWS_SMB_BUF)
};

/**
 * @brief Transport seam: the engine moves raw bytes only through these, so it runs against a real
 *        socket (det_client) or a test mock.
 * @return send: bytes written (must equal @p len), else < 0. recv: bytes read (> 0), else <= 0 on
 *         close / error / timeout.
 */
typedef int (*SmbSendFn)(void *ctx, const uint8_t *data, size_t len);
typedef int (*SmbRecvFn)(void *ctx, uint8_t *buf, size_t cap);

/** @brief Server credentials + the file to open. Strings are ASCII/UTF-8 (encoded UTF-16LE for you). */
struct SmbConfig
{
    const char *user;        ///< account name
    const char *pass;        ///< password
    const char *domain;      ///< NTLM domain (null/empty for a local account)
    const char *workstation; ///< client name to announce (null => none)
    const char *share;       ///< the tree path, UNC `\\server\share`
    const char *path;        ///< file name relative to the share root (e.g. `PROGRAMS\A.NC`)
    uint32_t desired_access; ///< SMB2_FILE_GENERIC_READ and/or _WRITE
    uint32_t disposition;    ///< SMB2_FILE_OPEN / _OPEN_IF / _OVERWRITE_IF / _CREATE
};

/** @brief An open file on an authenticated session; the ids thread the follow-up requests. */
struct SmbHandle
{
    uint64_t session_id;
    uint32_t tree_id;
    uint8_t file_id[16];
    uint64_t file_size;       ///< EndofFile from CREATE (the current size)
    uint64_t next_message_id; ///< the MessageId for the next request on this handle
};

/**
 * @brief Run NEGOTIATE -> NTLMv2 SESSION_SETUP -> TREE_CONNECT -> CREATE and fill @p h.
 * @return SmbResult::SMB_OK with @p h populated, or an ::SmbResult error.
 */
SmbResult smb_open(const SmbConfig *cfg, SmbHandle *h, SmbSendFn send, SmbRecvFn recv, void *ctx);

/**
 * @brief CLOSE the open handle (releases the server-side FileId).
 * @return SmbResult::SMB_OK, or an ::SmbResult error.
 */
SmbResult smb_close(SmbHandle *h, SmbSendFn send, SmbRecvFn recv, void *ctx);

/**
 * @brief Read up to @p cap bytes from @p offset of the open handle, looping READ requests until the
 *        buffer is full or the server signals end of file.
 * @param out_len receives the number of bytes actually read (may be < @p cap at EOF).
 * @return SmbResult::SMB_OK, or an ::SmbResult error. Reads at most DETWS_SMB_BUF-sized chunks per round trip.
 */
SmbResult smb_read(SmbHandle *h, uint64_t offset, uint8_t *out, size_t cap, size_t *out_len, SmbSendFn send,
                   SmbRecvFn recv, void *ctx);

/**
 * @brief Write @p len bytes at @p offset of the open handle, looping WRITE requests until all bytes
 *        are acknowledged. Grows the handle's cached file_size if the write extends the file.
 * @param written receives the number of bytes written (equals @p len on success).
 * @return SmbResult::SMB_OK, or an ::SmbResult error.
 */
SmbResult smb_write(SmbHandle *h, uint64_t offset, const uint8_t *data, size_t len, size_t *written, SmbSendFn send,
                    SmbRecvFn recv, void *ctx);

#endif // DETWS_ENABLE_SMB

#endif // DETERMINISTICESPASYNCWEBSERVER_SMB_CLIENT_H
