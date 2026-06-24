// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file det_tls.h
 * @brief Deterministic TLS engine: mbedTLS over a static memory pool (DETWS_ENABLE_TLS).
 *
 * Wraps mbedTLS as a server-side TLS layer that keeps the library's zero-heap
 * guarantee: mbedTLS is pointed at a fixed BSS arena via
 * MBEDTLS_MEMORY_BUFFER_ALLOC_C (no system heap), per-connection ssl_context
 * lives in BSS, the RNG is the ESP32 hardware CSPRNG, and the transport BIO is
 * bridged directly to the existing lwIP `tcp_pcb` + per-connection rx ring - so
 * there is no socket layer and no extra task. The handshake is pumped from the
 * single `handle()` loop.
 *
 * ESP32/Arduino only - mbedTLS is not part of the native build. The header
 * compiles everywhere (the functions are no-op stubs unless DETWS_ENABLE_TLS and
 * ARDUINO are both set) so call sites need no extra guards.
 *
 * Lifecycle per connection:
 * @code
 *   det_tls_conn_begin(slot);                 // at accept on the TLS port
 *   // each EVT_DATA, until established:
 *   int h = det_tls_handshake(slot);          // 1 done, 0 pending, <0 fatal
 *   // once established, app data:
 *   int n = det_tls_read(slot, buf, sizeof buf);   // >0 plaintext, 0 again, <0 closed
 *   det_tls_write(slot, data, len);                // encrypts -> tcp_write
 *   det_tls_conn_end(slot);                    // close_notify + free slot ctx
 * @endcode
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_DET_TLS_H
#define DETERMINISTICESPASYNCWEBSERVER_DET_TLS_H

#include "DetWebServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DETWS_ENABLE_TLS && defined(ARDUINO)

/**
 * @brief Initialize the global TLS engine: static pool, RNG, server cert/key.
 *
 * Call once before begin(). Parses the server certificate chain and private key
 * (PEM - NUL-terminated incl. the terminator in the length - or DER) and builds
 * the shared mbedTLS server config. All allocations come from the static arena.
 *
 * @param cert      Certificate (chain) buffer.
 * @param cert_len  Length incl. the trailing NUL for PEM.
 * @param key       Private key buffer.
 * @param key_len   Length incl. the trailing NUL for PEM.
 * @return true on success; false if the pool/cert/key setup failed.
 */
bool det_tls_global_init(const uint8_t *cert, size_t cert_len, const uint8_t *key, size_t key_len);

/** @brief True once det_tls_global_init() has succeeded. */
bool det_tls_ready();

/** @brief Begin a TLS session on connection @p slot (sets up ssl_context + BIO). */
bool det_tls_conn_begin(uint8_t slot);

/**
 * @brief Advance the TLS handshake for @p slot.
 * @return 1 when established, 0 while still in progress (need more data),
 *         negative on a fatal error (caller should drop the connection).
 */
int det_tls_handshake(uint8_t slot);

/** @brief True once the handshake on @p slot has completed. */
bool det_tls_established(uint8_t slot);

/**
 * @brief Read decrypted application data from @p slot.
 * @return >0 plaintext bytes, 0 if none are available yet, <0 on close/error.
 */
int det_tls_read(uint8_t slot, uint8_t *buf, size_t len);

/**
 * @brief Encrypt and send @p len bytes on @p slot (loops over partial writes).
 * @return bytes written, or <0 on error.
 */
int det_tls_write(uint8_t slot, const void *data, size_t len);

/** @brief Send close_notify and tear down the per-connection TLS context. */
void det_tls_conn_end(uint8_t slot);

/** @brief Tear down the TLS context without close_notify (abrupt disconnect/timeout). */
void det_tls_conn_free(uint8_t slot);

/** @brief Peak bytes ever used from the static arena (for sizing DETWS_TLS_ARENA_SIZE). */
size_t det_tls_arena_peak();

#else // stubs (TLS disabled or native build)

static inline bool det_tls_global_init(const uint8_t *, size_t, const uint8_t *, size_t)
{
    return false;
}
static inline bool det_tls_ready()
{
    return false;
}
static inline bool det_tls_conn_begin(uint8_t)
{
    return false;
}
static inline int det_tls_handshake(uint8_t)
{
    return -1;
}
static inline bool det_tls_established(uint8_t)
{
    return false;
}
static inline int det_tls_read(uint8_t, uint8_t *, size_t)
{
    return -1;
}
static inline int det_tls_write(uint8_t, const void *, size_t)
{
    return -1;
}
static inline void det_tls_conn_end(uint8_t)
{
}
static inline void det_tls_conn_free(uint8_t)
{
}
static inline size_t det_tls_arena_peak()
{
    return 0;
}

#endif // DETWS_ENABLE_TLS && ARDUINO

#endif // DETERMINISTICESPASYNCWEBSERVER_DET_TLS_H
