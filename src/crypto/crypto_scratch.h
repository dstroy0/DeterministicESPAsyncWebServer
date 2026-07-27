// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file crypto_scratch.h
 * @brief The shared preallocated crypto scratch buffer + the canonical secure wipe - one home for every
 *        service.
 *
 * A single fixed-size BSS buffer that crypto operations borrow transient working memory from (the big-number
 * Montgomery modexp temporaries, and the opaque per-operation contexts the crypto modules carve), instead of
 * each op owning a dedicated buffer or touching the heap - zero heap, deterministic size. dws_crypto_wipe()
 * is the one secure-zeroing primitive (a volatile store the compiler may not elide); callers wipe the region
 * after any op that touched secrets so key material never lingers.
 *
 * History: this buffer (`crypto_work`) and the wipe lived in bignum.cpp / ssh_keymat.h because SSH was the
 * first heavy crypto user. Hoisted here so every crypto consumer (SMB/NTLM, SNMP, TLS, ...) shares one
 * buffer and one wipe instead of copying the idiom. Single-accessor per task (the synchronous, one-op-at-a-
 * time crypto model), so no lock.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_CRYPTO_SCRATCH_H
#define DETERMINISTICESPASYNCWEBSERVER_CRYPTO_SCRATCH_H

#include "ServerConfig.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief The shared crypto scratch buffer (DWS_CRYPTO_WORK_SIZE bytes, defined in crypto_scratch.cpp).
 *
 * Borrowed by the big-number modexp (Montgomery temporaries at fixed offsets) and the crypto modules' opaque
 * per-operation contexts. Only one crypto op runs at a time per task, so it has a single accessor. Wipe with
 * dws_crypto_wipe(crypto_work, DWS_CRYPTO_WORK_SIZE) after any op that touched secrets.
 */
extern uint8_t crypto_work[DWS_CRYPTO_WORK_SIZE];

/**
 * @brief Securely zero @p len bytes at @p ptr with a volatile store the compiler cannot elide.
 *
 * Use this (not memset) for any buffer that held key material: a plain memset() whose result is never observed
 * (the buffer dies at return) may be elided as a dead store, leaving secrets in memory. The volatile write
 * forces the store even if the memory is never read again.
 *
 * @param ptr  Buffer to wipe.
 * @param len  Number of bytes to zero.
 */
static inline void dws_crypto_wipe(void *ptr, size_t len)
{
    volatile uint8_t *p = (volatile uint8_t *)ptr;
    for (size_t i = 0; i < len; i++)
        p[i] = 0;
}

/**
 * @brief Constant-time equality of two @p n-byte buffers: returns true iff every byte matches, in time
 *        independent of where (or whether) they first differ.
 *
 * Use this for every secret-dependent comparison - AEAD tags, MACs, digests, signature check values - so a
 * timing side channel cannot reveal how many leading bytes matched. Never use memcmp() for those (it returns
 * early on the first mismatch). The XOR-accumulate has no data-dependent branch; only the final all-zero test
 * (the intended result) is a comparison.
 */
static inline bool dws_ct_eq(const void *a, const void *b, size_t n)
{
    const uint8_t *pa = (const uint8_t *)a;
    const uint8_t *pb = (const uint8_t *)b;
    uint8_t diff = 0;
    for (size_t i = 0; i < n; i++)
        diff |= (uint8_t)(pa[i] ^ pb[i]);
    return diff == 0;
}

#endif // DETERMINISTICESPASYNCWEBSERVER_CRYPTO_SCRATCH_H
