// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file det_tls.cpp
 * @brief Deterministic TLS engine implementation (mbedTLS + static pool).
 *
 * ESP32/Arduino only. All mbedTLS allocations are served from a fixed BSS arena
 * (MBEDTLS_MEMORY_BUFFER_ALLOC_C) so no system heap is touched; the RNG is the
 * ESP32 hardware CSPRNG; the transport BIO reads ciphertext straight from the
 * connection's rx ring and writes via tcp_write. v2/v3 mbedTLS differences are
 * bridged with MBEDTLS_VERSION_MAJOR guards (same approach as the SSH layer).
 */

#include "network_drivers/tls/det_tls.h"

#if DETWS_ENABLE_TLS && defined(ARDUINO)

#include "lwip/tcp.h"
#include "network_drivers/transport/transport.h"
#include <Arduino.h>    // millis(), delay() for the blocking client loop
#include <esp_system.h> // esp_fill_random (HW CSPRNG)
#include <string.h>

#include <mbedtls/error.h>
#include <mbedtls/pk.h>
#include <mbedtls/platform.h> // mbedtls_platform_set_calloc_free
#include <mbedtls/sha256.h>   // peer-cert pin hashing (client verification)
#include <mbedtls/ssl.h>
#include <mbedtls/version.h>
#include <mbedtls/x509_crt.h>

// ---------------------------------------------------------------------------
// Static memory pool - a minimal first-fit allocator over a fixed BSS arena.
//
// The precompiled Arduino mbedTLS does not ship MBEDTLS_MEMORY_BUFFER_ALLOC_C,
// so instead we install our own calloc/free via mbedtls_platform_set_calloc_free
// (MBEDTLS_PLATFORM_MEMORY). Every mbedTLS allocation (record buffers, handshake
// temporaries, cert/key) is then served from s_arena - no system heap, so the
// determinism guarantee holds. Bounded by the arena: exhaustion fails the
// handshake cleanly (calloc returns NULL) rather than corrupting anything.
// ---------------------------------------------------------------------------
static uint8_t s_arena[DETWS_TLS_ARENA_SIZE];
static bool s_ready = false;
static bool s_pool_init = false;
static size_t s_pool_used = 0;
static size_t s_pool_peak = 0;

#define TLS_ALIGN 8u
struct PoolBlk
{
    size_t size;  // payload bytes
    uint8_t used; // 0 = free, 1 = in use
};
static const size_t POOL_HDR = (sizeof(PoolBlk) + (TLS_ALIGN - 1)) & ~(size_t)(TLS_ALIGN - 1);

static void pool_init()
{
    PoolBlk *b = (PoolBlk *)s_arena;
    b->size = sizeof(s_arena) - POOL_HDR;
    b->used = 0;
    s_pool_used = 0;
    s_pool_peak = 0;
    s_pool_init = true;
}

static void pool_coalesce()
{
    uint8_t *p = s_arena;
    uint8_t *end = s_arena + sizeof(s_arena);
    while (p < end)
    {
        PoolBlk *b = (PoolBlk *)p;
        uint8_t *next = p + POOL_HDR + b->size;
        if (!b->used && next < end)
        {
            PoolBlk *nb = (PoolBlk *)next;
            if (!nb->used)
            {
                b->size += POOL_HDR + nb->size; // merge
                continue;                       // try merging further
            }
        }
        p = next;
    }
}

static void *pool_calloc(size_t n, size_t size)
{
    if (!s_pool_init)
        pool_init();
    if (n != 0 && size > (size_t)-1 / n)
        return nullptr; // overflow
    size_t want = n * size;
    want = (want + (TLS_ALIGN - 1)) & ~(size_t)(TLS_ALIGN - 1);
    if (want == 0)
        want = TLS_ALIGN;

    uint8_t *p = s_arena;
    uint8_t *end = s_arena + sizeof(s_arena);
    while (p < end)
    {
        PoolBlk *b = (PoolBlk *)p;
        if (!b->used && b->size >= want)
        {
            // Split if the remainder can hold another header + a min payload.
            if (b->size >= want + POOL_HDR + TLS_ALIGN)
            {
                PoolBlk *nb = (PoolBlk *)(p + POOL_HDR + want);
                nb->size = b->size - want - POOL_HDR;
                nb->used = 0;
                b->size = want;
            }
            b->used = 1;
            s_pool_used += b->size;
            if (s_pool_used > s_pool_peak)
                s_pool_peak = s_pool_used;
            void *payload = p + POOL_HDR;
            memset(payload, 0, b->size);
            return payload;
        }
        p += POOL_HDR + b->size;
    }
    return nullptr; // arena exhausted
}

static void pool_free(void *ptr)
{
    if (!ptr)
        return;
    PoolBlk *b = (PoolBlk *)((uint8_t *)ptr - POOL_HDR);
    if (b->used)
    {
        b->used = 0;
        if (s_pool_used >= b->size)
            s_pool_used -= b->size;
    }
    pool_coalesce();
}

static mbedtls_ssl_config s_conf;
static mbedtls_x509_crt s_cert;
static mbedtls_pk_context s_key;
#if DETWS_ENABLE_MTLS
static mbedtls_x509_crt s_ca; // client-cert trust anchor (mTLS)
#endif

struct TlsConn
{
    mbedtls_ssl_context ssl;
    struct tcp_pcb *pcb; // captured at begin: the senders null conn->pcb mid-write
    uint8_t slot;
    bool active;
    bool established;
};
static TlsConn s_tls[MAX_TLS_CONNS];

static TlsConn *find(uint8_t slot)
{
    for (uint8_t i = 0; i < MAX_TLS_CONNS; i++)
        if (s_tls[i].active && s_tls[i].slot == slot)
            return &s_tls[i];
    return nullptr;
}

// ---------------------------------------------------------------------------
// RNG + transport BIO
// ---------------------------------------------------------------------------
static int tls_rng(void *ctx, unsigned char *out, size_t len)
{
    (void)ctx;
    esp_fill_random(out, len); // ESP32 hardware CSPRNG
    return 0;
}

// Server BIO recv (a det_tls_bio_recv_fn): pull ciphertext from the connection's
// rx ring (filled by the lwIP recv callback). No bytes -> WANT_READ so mbedTLS
// yields to the loop. Reads only the ring, so it is safe from either context.
static int server_bio_recv(void *ctx, unsigned char *buf, size_t len)
{
    TlsConn *e = (TlsConn *)ctx;
    TcpConn *c = &conn_pool[e->slot];
    size_t n = 0;
    while (n < len && c->rx_tail != c->rx_head)
    {
        buf[n++] = c->rx_buffer[c->rx_tail];
        c->rx_tail = (c->rx_tail + 1) % RX_BUF_SIZE;
    }
    if (n == 0)
        return MBEDTLS_ERR_SSL_WANT_READ;
    return (int)n;
}

// Server BIO send (a det_tls_bio_send_fn): emit ciphertext through the transport's
// context-safe raw write (det_conn_raw_send), so the handshake - pumped from the
// main loop - never does an unsynchronized tcp_write racing the lwIP thread, while
// app-data writes (already in the lwIP thread) still go out directly. Uses the pcb
// captured at begin() because the response senders null conn->pcb before writing.
static int server_bio_send(void *ctx, const unsigned char *buf, size_t len)
{
    TlsConn *e = (TlsConn *)ctx;
    if (!e->pcb)
        return MBEDTLS_ERR_SSL_INTERNAL_ERROR;

    size_t avail = tcp_sndbuf(e->pcb);
    if (avail == 0)
        return MBEDTLS_ERR_SSL_WANT_WRITE; // backpressure; retry next pump
    size_t to = len;
    if (to > avail)
        to = avail;
    if (to > 0xFFFF)
        to = 0xFFFF;

    if (det_conn_raw_send(e->pcb, buf, (u16_t)to))
        return (int)to;
    return MBEDTLS_ERR_SSL_WANT_WRITE; // send buffer full -> mbedTLS retries
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
bool det_tls_global_init(const uint8_t *cert, size_t cert_len, const uint8_t *key, size_t key_len)
{
    if (s_ready)
        return true;
    if (!cert || !key)
        return false;

    // Route ALL mbedTLS allocations through our static arena before any mbedTLS
    // object is initialized.
    pool_init();
    mbedtls_platform_set_calloc_free(pool_calloc, pool_free);

    mbedtls_x509_crt_init(&s_cert);
    mbedtls_pk_init(&s_key);
    mbedtls_ssl_config_init(&s_conf);

    if (mbedtls_x509_crt_parse(&s_cert, cert, cert_len) != 0)
        return false;

#if MBEDTLS_VERSION_MAJOR >= 3
    if (mbedtls_pk_parse_key(&s_key, key, key_len, nullptr, 0, tls_rng, nullptr) != 0)
        return false;
#else
    if (mbedtls_pk_parse_key(&s_key, key, key_len, nullptr, 0) != 0)
        return false;
#endif

    if (mbedtls_ssl_config_defaults(&s_conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM,
                                    MBEDTLS_SSL_PRESET_DEFAULT) != 0)
        return false;

    mbedtls_ssl_conf_rng(&s_conf, tls_rng, nullptr);
#if MBEDTLS_VERSION_MAJOR >= 3
    mbedtls_ssl_conf_min_tls_version(&s_conf, MBEDTLS_SSL_VERSION_TLS1_2);
#else
    mbedtls_ssl_conf_min_version(&s_conf, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3);
#endif

    if (mbedtls_ssl_conf_own_cert(&s_conf, &s_cert, &s_key) != 0)
        return false;

    for (uint8_t i = 0; i < MAX_TLS_CONNS; i++)
        s_tls[i].active = false;

    s_ready = true;
    return true;
}

bool det_tls_ready()
{
    return s_ready;
}

bool det_tls_conn_begin(uint8_t slot)
{
    if (!s_ready)
        return false;
    TlsConn *e = nullptr;
    for (uint8_t i = 0; i < MAX_TLS_CONNS; i++)
    {
        if (!s_tls[i].active)
        {
            e = &s_tls[i];
            break;
        }
    }
    if (!e)
        return false; // TLS connection pool full

    e->slot = slot;
    e->pcb = conn_pool[slot].pcb;
    e->active = true;
    e->established = false;
    mbedtls_ssl_init(&e->ssl);
    if (mbedtls_ssl_setup(&e->ssl, &s_conf) != 0)
    {
        mbedtls_ssl_free(&e->ssl);
        e->active = false;
        return false;
    }
    mbedtls_ssl_set_bio(&e->ssl, e, server_bio_send, server_bio_recv, nullptr);
    return true;
}

int det_tls_handshake(uint8_t slot)
{
    TlsConn *e = find(slot);
    if (!e)
        return -1;
    int ret = mbedtls_ssl_handshake(&e->ssl);
    if (ret == 0)
    {
        e->established = true;
        return 1;
    }
    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
        return 0;
    return -1;
}

bool det_tls_established(uint8_t slot)
{
    TlsConn *e = find(slot);
    return e && e->established;
}

int det_tls_read(uint8_t slot, uint8_t *buf, size_t len)
{
    TlsConn *e = find(slot);
    if (!e)
        return -1;
    int ret = mbedtls_ssl_read(&e->ssl, buf, len);
    if (ret > 0)
        return ret;
    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
        return 0; // no plaintext available yet
    return -1;    // close_notify, peer close, or fatal
}

int det_tls_write(uint8_t slot, const void *data, size_t len)
{
    TlsConn *e = find(slot);
    if (!e)
        return -1;
    const unsigned char *p = (const unsigned char *)data;
    size_t sent = 0;
    uint16_t guard = 0; // bound retries so a stuck send buffer cannot spin forever
    while (sent < len)
    {
        int ret = mbedtls_ssl_write(&e->ssl, p + sent, len - sent);
        if (ret > 0)
        {
            sent += (size_t)ret;
            guard = 0;
            continue;
        }
        if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            if (++guard > 64)
                break; // give up this flush; backpressure
            continue;
        }
        return -1;
    }
    return (int)sent;
}

void det_tls_conn_end(uint8_t slot)
{
    TlsConn *e = find(slot);
    if (!e)
        return;
    mbedtls_ssl_close_notify(&e->ssl);
    mbedtls_ssl_free(&e->ssl);
    e->active = false;
    e->established = false;
    e->pcb = nullptr;
}

void det_tls_conn_free(uint8_t slot)
{
    TlsConn *e = find(slot);
    if (!e)
        return;
    mbedtls_ssl_free(&e->ssl); // abrupt teardown: no close_notify (peer is gone)
    e->active = false;
    e->established = false;
    e->pcb = nullptr;
}

size_t det_tls_arena_peak()
{
    return s_pool_peak;
}

#if DETWS_ENABLE_MTLS
bool det_tls_set_client_ca(const uint8_t *ca, size_t ca_len)
{
    if (!s_ready || !ca)
        return false;
    mbedtls_x509_crt_init(&s_ca);
    if (mbedtls_x509_crt_parse(&s_ca, ca, ca_len) != 0)
        return false;
    // Trust anchor for the client chain + demand a (valid) client cert: an absent
    // or untrusted client certificate now fails the handshake.
    mbedtls_ssl_conf_ca_chain(&s_conf, &s_ca, nullptr);
    mbedtls_ssl_conf_authmode(&s_conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    return true;
}

int det_tls_peer_subject(uint8_t slot, char *out, size_t out_len)
{
    if (!out || out_len == 0)
        return -1;
    out[0] = '\0';
    TlsConn *e = find(slot);
    if (!e || !e->established)
        return -1;
    const mbedtls_x509_crt *peer = mbedtls_ssl_get_peer_cert(&e->ssl);
    if (!peer)
        return -1;
    int n = mbedtls_x509_dn_gets(out, out_len, &peer->subject);
    return n; // bytes written (excl. NUL), or <0 on error
}
#endif // DETWS_ENABLE_MTLS

#if DETWS_ENABLE_CLIENT_TLS
// Optional client-side server authentication (default off = encrypt-only):
//  - a CA trust anchor -> mbedTLS verifies the chain + hostname during handshake;
//  - a 32-byte SHA-256 cert pin -> the peer's certificate DER is hashed and
//    constant-time compared after the handshake.
// Either, both, or neither may be set; both must pass when both are set. Shared by
// the one-shot HTTP client (det_tls_client_run) and the persistent session (csess).
static mbedtls_x509_crt s_client_ca;
static bool s_client_ca_set = false;
static uint8_t s_client_pin[32];
static bool s_client_pin_set = false;

// Route mbedTLS allocations through the static arena (the client may run before
// any server-side TLS init has installed the allocator).
static void client_arena_ensure()
{
    if (!s_pool_init)
    {
        pool_init();
        mbedtls_platform_set_calloc_free(pool_calloc, pool_free);
    }
}

void det_tls_client_set_ca(const uint8_t *ca, size_t ca_len)
{
    client_arena_ensure();
    if (s_client_ca_set)
        mbedtls_x509_crt_free(&s_client_ca);
    s_client_ca_set = false;
    if (!ca || ca_len == 0)
        return;
    mbedtls_x509_crt_init(&s_client_ca);
    s_client_ca_set = (mbedtls_x509_crt_parse(&s_client_ca, ca, ca_len) == 0);
    if (!s_client_ca_set)
        mbedtls_x509_crt_free(&s_client_ca);
}

void det_tls_client_set_pin(const uint8_t sha256[32])
{
    if (!sha256)
    {
        s_client_pin_set = false;
        return;
    }
    memcpy(s_client_pin, sha256, 32);
    s_client_pin_set = true;
}

void det_tls_client_clear_verify()
{
    if (s_client_ca_set)
        mbedtls_x509_crt_free(&s_client_ca);
    s_client_ca_set = false;
    s_client_pin_set = false;
}

// Constant-time 32-byte compare (no early-out on the first differing byte).
static bool ct_eq32(const uint8_t *a, const uint8_t *b)
{
    uint8_t d = 0;
    for (int i = 0; i < 32; i++)
        d |= (uint8_t)(a[i] ^ b[i]);
    return d == 0;
}

// Apply the shared client config: RNG, server-auth mode (CA verify or
// encrypt-only), TLS >= 1.2, and the CA chain when one is installed.
static int client_conf_apply(mbedtls_ssl_config *conf)
{
    if (mbedtls_ssl_config_defaults(conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM,
                                    MBEDTLS_SSL_PRESET_DEFAULT) != 0)
        return -1;
    mbedtls_ssl_conf_rng(conf, tls_rng, nullptr);
    if (s_client_ca_set)
    {
        mbedtls_ssl_conf_ca_chain(conf, &s_client_ca, nullptr);
        mbedtls_ssl_conf_authmode(conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    }
    else
        mbedtls_ssl_conf_authmode(conf, MBEDTLS_SSL_VERIFY_NONE); // encrypt-only (no trust store)
#if MBEDTLS_VERSION_MAJOR >= 3
    mbedtls_ssl_conf_min_tls_version(conf, MBEDTLS_SSL_VERSION_TLS1_2);
#else
    mbedtls_ssl_conf_min_version(conf, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3);
#endif
    return 0;
}

// Post-handshake certificate pin check: true if no pin is set, or the peer's
// certificate DER hashes to the installed pin (constant-time compared).
static bool client_pin_ok(mbedtls_ssl_context *ssl)
{
    if (!s_client_pin_set)
        return true;
    const mbedtls_x509_crt *peer = mbedtls_ssl_get_peer_cert(ssl);
    if (!peer)
        return false;
    uint8_t hash[32];
#if MBEDTLS_VERSION_MAJOR >= 3
    int hret = mbedtls_sha256(peer->raw.p, peer->raw.len, hash, 0);
#else
    int hret = mbedtls_sha256_ret(peer->raw.p, peer->raw.len, hash, 0);
#endif
    return (hret == 0) && ct_eq32(hash, s_client_pin);
}

#if DETWS_ENABLE_HTTP_CLIENT_TLS
// Blocking client-side TLS exchange over caller-supplied BIO callbacks. Used by
// the outbound HTTP client for https://. The transport (raw lwIP tcp_write + the
// receive ring) lives in http_client.cpp; here we own only the mbedTLS client
// session, served from the same static arena as the server side.
//
// Server authentication is OFF by default (the device has no trust store): the
// transport is encrypted but the peer is unauthenticated unless a CA
// (det_tls_client_set_ca) and/or a cert pin (det_tls_client_set_pin) is installed.
int det_tls_client_run(const char *host, const uint8_t *req, size_t reqlen, uint8_t *out, size_t out_cap,
                       size_t *out_len, det_tls_bio_send_fn send_fn, det_tls_bio_recv_fn recv_fn, uint32_t deadline_ms)
{
    if (out_len)
        *out_len = 0;
    if (!req || !out || out_cap == 0 || !send_fn || !recv_fn)
        return -1;

    client_arena_ensure();

    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);

    int rc = -1;
    int ret;
    do
    {
        if (client_conf_apply(&conf) != 0)
            break;
        if (mbedtls_ssl_setup(&ssl, &conf) != 0)
            break;
        if (host)
            mbedtls_ssl_set_hostname(&ssl, host); // SNI (ignored if unsupported)
        mbedtls_ssl_set_bio(&ssl, nullptr, send_fn, recv_fn, nullptr);

        // Handshake (BIO callbacks yield WANT_READ/WANT_WRITE while data is pending).
        while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
        {
            if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
                break;
            if ((int32_t)(deadline_ms - millis()) <= 0)
            {
                ret = -1;
                break;
            }
            delay(5);
        }
        if (ret != 0)
        {
#ifdef DETWS_HTTP_CLIENT_DEBUG
            printf("[tls] handshake ret=-0x%04x arena_peak=%u\n", (unsigned)(-ret), (unsigned)det_tls_arena_peak());
#endif
            break;
        }

        // Certificate pinning (mismatch or no peer cert aborts).
        if (!client_pin_ok(&ssl))
        {
#ifdef DETWS_HTTP_CLIENT_DEBUG
            printf("[tls] cert pin mismatch\n");
#endif
            ret = -1;
            break;
        }

        // Send the (plaintext) request; mbedTLS encrypts and pushes via send_fn.
        size_t sent = 0;
        while (sent < reqlen)
        {
            ret = mbedtls_ssl_write(&ssl, req + sent, reqlen - sent);
            if (ret > 0)
            {
                sent += (size_t)ret;
                continue;
            }
            if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
                break;
            if ((int32_t)(deadline_ms - millis()) <= 0)
                break;
            delay(5);
        }
        if (sent < reqlen)
            break;

        // Read the decrypted response into out until the peer closes / buffer fills.
        size_t total = 0;
        while (total < out_cap)
        {
            ret = mbedtls_ssl_read(&ssl, out + total, out_cap - total);
            if (ret > 0)
            {
                total += (size_t)ret;
                continue;
            }
            if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                if ((int32_t)(deadline_ms - millis()) <= 0)
                    break;
                delay(5);
                continue;
            }
            break; // 0 / PEER_CLOSE_NOTIFY / fatal -> response complete
        }
        if (out_len)
            *out_len = total;
        rc = (total > 0) ? 0 : -1;
    } while (0);

    mbedtls_ssl_close_notify(&ssl);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    return rc;
}
#endif // DETWS_ENABLE_HTTP_CLIENT_TLS

// --- Persistent client session (csess): one long-lived outbound TLS connection
// (e.g. MQTTS). Handshake once, then read/write application data over the
// caller's BIO until det_tls_csess_end(). Honors the CA/pin trust config above. ---
static mbedtls_ssl_context s_csess_ssl;
static mbedtls_ssl_config s_csess_conf;
static bool s_csess_active = false;

bool det_tls_csess_begin(const char *host, det_tls_bio_send_fn send_fn, det_tls_bio_recv_fn recv_fn)
{
    if (!send_fn || !recv_fn)
        return false;
    if (s_csess_active)
        det_tls_csess_end();
    client_arena_ensure();
    mbedtls_ssl_init(&s_csess_ssl);
    mbedtls_ssl_config_init(&s_csess_conf);
    if (client_conf_apply(&s_csess_conf) != 0 || mbedtls_ssl_setup(&s_csess_ssl, &s_csess_conf) != 0)
    {
        mbedtls_ssl_free(&s_csess_ssl);
        mbedtls_ssl_config_free(&s_csess_conf);
        return false;
    }
    if (host)
        mbedtls_ssl_set_hostname(&s_csess_ssl, host);
    mbedtls_ssl_set_bio(&s_csess_ssl, nullptr, send_fn, recv_fn, nullptr);
    s_csess_active = true;
    return true;
}

int det_tls_csess_handshake()
{
    if (!s_csess_active)
        return -1;
    int ret = mbedtls_ssl_handshake(&s_csess_ssl);
    if (ret == 0)
        return client_pin_ok(&s_csess_ssl) ? 1 : -1; // verify the pin once established
    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
        return 0;
    return -1;
}

int det_tls_csess_read(uint8_t *buf, size_t len)
{
    if (!s_csess_active)
        return -1;
    int ret = mbedtls_ssl_read(&s_csess_ssl, buf, len);
    if (ret > 0)
        return ret;
    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
        return 0; // no plaintext available yet
    return -1;    // close_notify / peer close / fatal
}

int det_tls_csess_write(const uint8_t *data, size_t len)
{
    if (!s_csess_active)
        return -1;
    size_t sent = 0;
    uint16_t guard = 0;
    while (sent < len)
    {
        int ret = mbedtls_ssl_write(&s_csess_ssl, data + sent, len - sent);
        if (ret > 0)
        {
            sent += (size_t)ret;
            guard = 0;
            continue;
        }
        if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            if (++guard > 64)
                break; // send buffer stuck; report a short write
            delay(2);
            continue;
        }
        return -1;
    }
    return (int)sent;
}

void det_tls_csess_end()
{
    if (!s_csess_active)
        return;
    mbedtls_ssl_close_notify(&s_csess_ssl);
    mbedtls_ssl_free(&s_csess_ssl);
    mbedtls_ssl_config_free(&s_csess_conf);
    s_csess_active = false;
}

#endif // DETWS_ENABLE_CLIENT_TLS

#endif // DETWS_ENABLE_TLS && ARDUINO
