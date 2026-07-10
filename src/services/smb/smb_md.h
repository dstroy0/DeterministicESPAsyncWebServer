// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file smb_md.h
 * @brief MD4 (RFC 1320), MD5 (RFC 1321), and HMAC-MD5 (RFC 2104) - the legacy digests NTLM
 *        needs, for the SMB2 client (DETWS_ENABLE_SMB). Not used anywhere else in the library.
 *
 * NTLMv2 (MS-NLMP) builds on these: the NT hash is MD4(UTF-16LE(password)); the NTLMv2 response
 * and the session key are HMAC-MD5 chains. MD4/MD5 are cryptographically broken and are included
 * ONLY because SMB/NTLM requires them on the wire - do not use them for anything security-new.
 * Zero heap, streaming; verified against the RFC test vectors (see test_smb_crypto).
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SMB_MD_H
#define DETERMINISTICESPASYNCWEBSERVER_SMB_MD_H

#include "ServerConfig.h"

#if DETWS_ENABLE_SMB

#include <stddef.h>
#include <stdint.h>

/** @brief Streaming digest context (shared by MD4 and MD5; both are 16-byte, 64-byte-block). */
struct MdCtx
{
    uint32_t state[4];
    uint64_t bits;    ///< total message length in bits
    uint8_t buf[64];  ///< partial block
    uint32_t buf_len; ///< bytes currently in @ref buf
};

void md5_init(MdCtx *c);
void md5_update(MdCtx *c, const uint8_t *data, size_t len);
void md5_final(MdCtx *c, uint8_t out[16]);
/** @brief One-shot MD5. */
void md5(const uint8_t *data, size_t len, uint8_t out[16]);

void md4_init(MdCtx *c);
void md4_update(MdCtx *c, const uint8_t *data, size_t len);
void md4_final(MdCtx *c, uint8_t out[16]);
/** @brief One-shot MD4 (the NT-hash primitive). */
void md4(const uint8_t *data, size_t len, uint8_t out[16]);

/** @brief HMAC-MD5 (RFC 2104): the NTLMv2 MAC primitive. */
void hmac_md5(const uint8_t *key, size_t key_len, const uint8_t *msg, size_t msg_len, uint8_t out[16]);

#endif // DETWS_ENABLE_SMB

#endif // DETERMINISTICESPASYNCWEBSERVER_SMB_MD_H
