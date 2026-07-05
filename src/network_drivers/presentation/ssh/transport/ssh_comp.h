// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file ssh_comp.h
 * @brief SSH per-connection compression owner (server-to-client `zlib` / `zlib@openssh.com`).
 *
 * One owner for the whole compression concern (per the "one owner per cross-layer concern" rule):
 * it holds the per-connection streaming compressor + its PSRAM-resident buffers, records the
 * negotiated s2c algorithm, and starts the stream at the right moment (immediately after NEWKEYS for
 * `zlib`, or after SSH_MSG_USERAUTH_SUCCESS for the delayed `zlib@openssh.com`). The transport packet
 * layer asks it, per outbound packet, whether to compress and hands it the payload; nothing else
 * touches compression state.
 *
 * Only server-to-client is implemented (see ssh_zlib.h for why c2s stays `none`).
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SSH_COMP_H
#define DETERMINISTICESPASYNCWEBSERVER_SSH_COMP_H

#include "DetWebServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DETWS_ENABLE_SSH_ZLIB

/** @brief Negotiated server-to-client compression algorithm. */
enum SshCompAlg
{
    SSH_COMP_NONE = 0,        ///< no compression (also the c2s direction, always)
    SSH_COMP_ZLIB = 1,        ///< "zlib" (RFC 4253) - starts right after NEWKEYS
    SSH_COMP_ZLIB_DELAYED = 2 ///< "zlib@openssh.com" - starts after SSH_MSG_USERAUTH_SUCCESS
};

/** @brief Reset compression state for slot @p i (fresh connection). Does NOT run on a re-key. */
void ssh_comp_reset(uint8_t i);

/** @brief Record the s2c algorithm negotiated in KEXINIT (::SshCompAlg). */
void ssh_comp_set_s2c(uint8_t i, uint8_t alg);

/** @brief NEWKEYS completed: start the stream now if `zlib` was negotiated (idempotent). */
void ssh_comp_on_newkeys(uint8_t i);

/** @brief SSH_MSG_USERAUTH_SUCCESS sent: start the stream if `zlib@openssh.com` was negotiated. */
void ssh_comp_on_auth_success(uint8_t i);

/** @brief True once the s2c stream is active and outbound payloads must be compressed. */
bool ssh_comp_s2c_active(uint8_t i);

/**
 * @brief Compress one outbound payload, continuing the session's zlib stream.
 * @return 0 on success (*out_len set), -1 on overflow / oversized input / inactive slot.
 */
int ssh_comp_s2c(uint8_t i, const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_cap, size_t *out_len);

#endif // DETWS_ENABLE_SSH_ZLIB
#endif // DETERMINISTICESPASYNCWEBSERVER_SSH_COMP_H
