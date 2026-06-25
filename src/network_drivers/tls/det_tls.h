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

/**
 * @brief TLS BIO send/recv callbacks (mbedTLS signatures) - the transport
 *        abstraction the engine reads/writes ciphertext through.
 *
 * Both sides conform to this: the server registers BIO functions that read the
 * connection's rx ring and write via the transport (det_conn_raw_send), and the
 * outbound client passes its own pair to det_tls_client_run(). The engine itself
 * never touches lwIP directly.
 */
typedef int (*det_tls_bio_send_fn)(void *ctx, const unsigned char *buf, size_t len);
typedef int (*det_tls_bio_recv_fn)(void *ctx, unsigned char *buf, size_t len);

#if DETWS_ENABLE_MTLS
/**
 * @brief Require a verified client certificate (mTLS): install the trust-anchor CA.
 *
 * Call after det_tls_global_init(). Parses @p ca (PEM - length incl. the trailing
 * NUL - or DER) as the CA chain and switches the server to
 * MBEDTLS_SSL_VERIFY_REQUIRED, so the handshake demands a client certificate that
 * chains to @p ca and aborts the connection otherwise.
 *
 * @return true on success; false if the engine is not initialized or the CA
 *         failed to parse.
 */
bool det_tls_set_client_ca(const uint8_t *ca, size_t ca_len);

/**
 * @brief Copy the established peer's certificate subject DN into @p out.
 *
 * Valid once the handshake on @p slot has completed with a verified client cert.
 * @return the subject string length written (excl. NUL), or <0 if there is no
 *         verified peer certificate.
 */
int det_tls_peer_subject(uint8_t slot, char *out, size_t out_len);
#endif // DETWS_ENABLE_MTLS

#if DETWS_ENABLE_HTTP_CLIENT_TLS
/**
 * @brief Run a blocking client-side TLS exchange over caller-supplied BIO callbacks.
 *
 * Performs a TLS 1.2+ client handshake (SNI = @p host, server cert not verified -
 * see note), writes @p req, then reads the decrypted response into @p out until
 * the peer closes or @p out fills. Uses the shared static arena (installs the
 * allocator if the server side has not). Yields with delay() while waiting, up to
 * @p deadline_ms (millis() timestamp).
 *
 * NOTE: server authentication is OFF by default (no trust store on the device);
 * the transport is encrypted but unauthenticated unless a CA and/or a cert pin is
 * installed via det_tls_client_set_ca() / det_tls_client_set_pin().
 *
 * @return 0 on success (@p out_len set), <0 on handshake/verification/IO failure.
 */
int det_tls_client_run(const char *host, const uint8_t *req, size_t reqlen, uint8_t *out, size_t out_cap,
                       size_t *out_len, det_tls_bio_send_fn send_fn, det_tls_bio_recv_fn recv_fn, uint32_t deadline_ms);
#endif // DETWS_ENABLE_HTTP_CLIENT_TLS

#if DETWS_ENABLE_CLIENT_TLS
/**
 * @brief Install a CA trust anchor for outbound TLS (HTTPS/MQTTS) verification.
 *
 * Pass PEM (length incl. the trailing NUL) or DER; nullptr/0 clears it. With a CA
 * installed, the client handshake verifies the server's certificate chain and its
 * hostname (SNI) and aborts the connection on failure.
 */
void det_tls_client_set_ca(const uint8_t *ca, size_t ca_len);

/**
 * @brief Pin the outbound server's certificate by SHA-256 (32 bytes of the DER).
 *
 * After a successful handshake the peer certificate is hashed and constant-time
 * compared to @p sha256; a mismatch (or no peer cert) fails the connection. Pass
 * nullptr to clear. Can be combined with det_tls_client_set_ca().
 */
void det_tls_client_set_pin(const uint8_t sha256[32]);

/** @brief Clear any installed client CA and cert pin (back to encrypt-only). */
void det_tls_client_clear_verify();

// --- Persistent client TLS session (one outbound connection at a time) ---
// For a long-lived encrypted client (MQTTS): handshake once, then read/write
// application data over the caller's BIO until det_tls_csess_end(). Honors the
// CA/pin installed above. The BIO callbacks read ciphertext from the caller's
// receive ring and write it to the socket.

/** @brief Begin a client TLS session to @p host over the given BIO. @return false on setup failure. */
bool det_tls_csess_begin(const char *host, det_tls_bio_send_fn send_fn, det_tls_bio_recv_fn recv_fn);

/** @brief Advance the handshake. @return 1 established (CA/pin checked), 0 pending, <0 fatal. */
int det_tls_csess_handshake();

/** @brief Read decrypted application data. @return >0 bytes, 0 none yet, <0 closed/error. */
int det_tls_csess_read(uint8_t *buf, size_t len);

/** @brief Encrypt and send @p len bytes. @return bytes written, or <0 on error. */
int det_tls_csess_write(const uint8_t *data, size_t len);

/** @brief Send close_notify and tear down the session. */
void det_tls_csess_end();
#endif // DETWS_ENABLE_CLIENT_TLS

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

#if DETWS_ENABLE_MTLS
static inline bool det_tls_set_client_ca(const uint8_t *, size_t)
{
    return false;
}
static inline int det_tls_peer_subject(uint8_t, char *, size_t)
{
    return -1;
}
#endif // DETWS_ENABLE_MTLS

#if DETWS_ENABLE_CLIENT_TLS
typedef int (*det_tls_bio_send_fn)(void *ctx, const unsigned char *buf, size_t len);
typedef int (*det_tls_bio_recv_fn)(void *ctx, unsigned char *buf, size_t len);
static inline void det_tls_client_set_ca(const uint8_t *, size_t)
{
}
static inline void det_tls_client_set_pin(const uint8_t *)
{
}
static inline void det_tls_client_clear_verify()
{
}
static inline bool det_tls_csess_begin(const char *, det_tls_bio_send_fn, det_tls_bio_recv_fn)
{
    return false;
}
static inline int det_tls_csess_handshake()
{
    return -1;
}
static inline int det_tls_csess_read(uint8_t *, size_t)
{
    return -1;
}
static inline int det_tls_csess_write(const uint8_t *, size_t)
{
    return -1;
}
static inline void det_tls_csess_end()
{
}
#endif // DETWS_ENABLE_CLIENT_TLS

#if DETWS_ENABLE_HTTP_CLIENT_TLS
static inline int det_tls_client_run(const char *, const uint8_t *, size_t, uint8_t *, size_t, size_t *,
                                     det_tls_bio_send_fn, det_tls_bio_recv_fn, uint32_t)
{
    return -1;
}
#endif // DETWS_ENABLE_HTTP_CLIENT_TLS

#endif // DETWS_ENABLE_TLS && ARDUINO

#endif // DETERMINISTICESPASYNCWEBSERVER_DET_TLS_H
