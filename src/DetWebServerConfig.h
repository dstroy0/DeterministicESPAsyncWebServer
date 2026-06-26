// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file DetWebServerConfig.h
 * @brief User-facing configuration for DeterministicESPAsyncWebServer.
 *
 * **Compile-time sizing constants**
 * These govern static array dimensions and must be set before the first
 * library header is included.  Define any of them in your sketch or in a
 * build flag before including this file to override the defaults:
 * @code
 *   // platformio.ini
 *   build_flags = -DMAX_CONNS=8 -DBODY_BUF_SIZE=512
 * @endcode
 *
 * **Runtime parameters - flash or RAM, your choice**
 * `WebServerConfig` holds values that can be changed without a rebuild.
 * On ESP32, `PROGMEM` is a no-op (const data lands in DROM automatically).
 * On AVR it places data in flash and requires `pgm_read_*` accessors - this
 * library targets ESP32 only, so both forms read identically via pointer:
 * @code
 *   // Flash (PROGMEM, no RAM cost at runtime):
 *   const WebServerConfig my_cfg PROGMEM = { .conn_timeout_ms = 10000 };
 *
 *   // RAM (can be changed at runtime):
 *   WebServerConfig my_cfg = { .conn_timeout_ms = 10000 };
 *
 *   server.begin(80, &my_cfg);
 * @endcode
 * Pass `nullptr` (or omit the argument) to use `DET_DEFAULT_CONFIG`.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_CONFIG_H
#define DETERMINISTICESPASYNCWEBSERVER_CONFIG_H

#include <stdint.h>

// ---------------------------------------------------------------------------
// Compile-time capacity constants (affect static array sizes)
// ---------------------------------------------------------------------------

/** @brief Maximum simultaneous TCP connections. */
#ifndef MAX_CONNS
#define MAX_CONNS 4
#endif

/** @brief Ring-buffer capacity in bytes per connection slot. */
#ifndef RX_BUF_SIZE
#define RX_BUF_SIZE 1024
#endif

/**
 * @brief Compile-time default for connection idle timeout in milliseconds.
 *
 * The actual runtime value is stored in `WebServerConfig::conn_timeout_ms`
 * and loaded into `DeterministicAsyncTCP::conn_timeout_ms` by init().
 */
#ifndef CONN_TIMEOUT_MS
#define CONN_TIMEOUT_MS 5000
#endif

/** @brief Maximum HTTP headers stored per request. */
#ifndef MAX_HEADERS
#define MAX_HEADERS 8
#endif

/** @brief Maximum URL path length (including leading `/`). */
#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN 64
#endif

/** @brief Maximum header field-name length (e.g. `"Content-Type"`). */
#ifndef MAX_KEY_LEN
#define MAX_KEY_LEN 24
#endif

/** @brief Maximum header field-value length. */
#ifndef MAX_VAL_LEN
#define MAX_VAL_LEN 48
#endif

/** @brief Maximum raw query-string length (everything after `?`). */
#ifndef MAX_QUERY_LEN
#define MAX_QUERY_LEN 128
#endif

/** @brief Maximum number of parsed query-string parameters. */
#ifndef MAX_QUERY_PARAMS
#define MAX_QUERY_PARAMS 8
#endif

/** @brief Maximum number of `:name` path parameters captured per route match. */
#ifndef MAX_PATH_PARAMS
#define MAX_PATH_PARAMS 4
#endif

/**
 * @brief Capacity for the full `Authorization` header value (Digest auth).
 *
 * A Digest `Authorization` header (username, realm, nonce, uri, response,
 * qop, nc, cnonce) is far longer than MAX_VAL_LEN, so when DETWS_ENABLE_AUTH
 * is set the parser captures it whole into a dedicated per-request buffer.
 */
#ifndef DIGEST_AUTH_HDR_MAX
#define DIGEST_AUTH_HDR_MAX 384
#endif

/** @brief Maximum query-parameter key length. */
#ifndef QUERY_KEY_LEN
#define QUERY_KEY_LEN 24
#endif

/** @brief Maximum query-parameter value length. */
#ifndef QUERY_VAL_LEN
#define QUERY_VAL_LEN 48
#endif

/**
 * @brief Maximum request body bytes stored in `HttpReq::body`.
 *
 * Bodies larger than this trigger a 413 Payload Too Large response -
 * the parser detects the overflow via `Content-Length` before any body
 * bytes arrive, so no data is read or stored for oversized requests.
 */
#ifndef BODY_BUF_SIZE
#define BODY_BUF_SIZE 256
#endif

/** @brief Maximum simultaneously registered routes. */
#ifndef MAX_ROUTES
#define MAX_ROUTES 16
#endif

/**
 * @brief Maximum globally-registered middleware functions.
 *
 * The middleware chain is a fixed array of function pointers run in
 * registration order before a request reaches its route handler (see
 * DetWebServer::use()). Costs MAX_MIDDLEWARE pointers of BSS; an empty chain
 * adds no per-request work.
 */
#ifndef MAX_MIDDLEWARE
#define MAX_MIDDLEWARE 4
#endif

/**
 * @brief Stack scratch buffer for ChunkedResponse::printf() (see send_chunked()).
 *
 * One formatted chunk must fit in this many bytes (longer output is truncated).
 * Allocated on the stack only while printf() runs - no persistent RAM cost.
 */
#ifndef CHUNK_BUF_SIZE
#define CHUNK_BUF_SIZE 256
#endif

/**
 * @brief Maximum object/array nesting depth for the JsonWriter (see json.h).
 *
 * Bounds the writer's per-level comma-tracking stack (one bool per level);
 * begin_object()/begin_array() beyond this fail the writer instead of
 * overflowing. No heap; ~JSON_MAX_DEPTH bytes of stack inside the writer object.
 */
#ifndef JSON_MAX_DEPTH
#define JSON_MAX_DEPTH 8
#endif

/**
 * @brief Step budget for the regex route matcher (see on_regex()).
 *
 * The matcher is a bounded backtracker: it counts match steps and fails closed
 * (no match) once this budget is exhausted, so a pathological pattern can never
 * backtrack unboundedly. Keeps regex routing deterministic. Routing patterns hit
 * only a handful of steps; the default leaves wide margin.
 */
#ifndef RE_MAX_STEPS
#define RE_MAX_STEPS 2000
#endif

// ---------------------------------------------------------------------------
// WebSocket sizing constants
// ---------------------------------------------------------------------------

/**
 * @brief Maximum simultaneous WebSocket connections.
 *
 * Each connection occupies one TCP slot from MAX_CONNS and one entry in
 * ws_pool[].  MAX_WS_CONNS + MAX_SSE_CONNS must not exceed MAX_CONNS.
 */
#ifndef MAX_WS_CONNS
#define MAX_WS_CONNS 2
#endif

/**
 * @brief Maximum WebSocket frame payload in bytes.
 *
 * Frames larger than this are rejected with Close code 1009 (Message Too Big).
 * Fragmented messages are not supported; each message must fit in one frame.
 */
#ifndef WS_FRAME_SIZE
#define WS_FRAME_SIZE 512
#endif

// ---------------------------------------------------------------------------
// Server-Sent Events sizing constants
// ---------------------------------------------------------------------------

/**
 * @brief Maximum simultaneous SSE connections.
 *
 * Each connection occupies one TCP slot from MAX_CONNS and one entry in
 * sse_pool[].  MAX_WS_CONNS + MAX_SSE_CONNS must not exceed MAX_CONNS.
 */
#ifndef MAX_SSE_CONNS
#define MAX_SSE_CONNS 2
#endif

/**
 * @brief Output buffer size in bytes for a single SSE event.
 *
 * An event larger than this is silently truncated.  The buffer holds the
 * formatted `data: ...\n\n` line before it is handed to tcp_write().
 */
#ifndef SSE_BUF_SIZE
#define SSE_BUF_SIZE 256
#endif

// ---------------------------------------------------------------------------
// Static file serving sizing constants
// ---------------------------------------------------------------------------

/**
 * @brief Bytes read from the filesystem and passed to tcp_write() per loop().
 *
 * Smaller values reduce peak stack use; larger values improve throughput.
 * Must be <= RX_BUF_SIZE to avoid stalling the TCP send window.
 */
#ifndef FILE_CHUNK_SIZE
#define FILE_CHUNK_SIZE 512
#endif

// ---------------------------------------------------------------------------
// Basic Auth sizing constants
// ---------------------------------------------------------------------------

/**
 * @brief Maximum username or password length for HTTP Basic Authentication.
 *
 * Both username and password must fit in this many bytes including the
 * null terminator.  Longer credentials are silently rejected with 401.
 */
#ifndef MAX_AUTH_LEN
#define MAX_AUTH_LEN 32
#endif

// ---------------------------------------------------------------------------
// Multipart form-data sizing constants
// ---------------------------------------------------------------------------

/**
 * @brief Maximum simultaneously parsed multipart parts per request.
 *
 * Parts beyond this limit are silently ignored.  A typical upload form
 * has 1-4 fields; increase this for forms with more.
 */
#ifndef MAX_MULTIPART_PARTS
#define MAX_MULTIPART_PARTS 4
#endif

/**
 * @brief Maximum MIME boundary length (RFC 2046 allows up to 70 characters).
 */
#ifndef MAX_BOUNDARY_LEN
#define MAX_BOUNDARY_LEN 72
#endif

// ---------------------------------------------------------------------------
// Event queue depth
// ---------------------------------------------------------------------------

/**
 * @brief Depth of the FreeRTOS event queue shared between lwIP callbacks and
 *        the main-loop task.
 *
 * Each slot holds one TcpEvt (8 bytes).  The queue is the only heap
 * allocation the library makes at begin() time:
 *
 *   heap = sizeof(StaticQueue_t) + EVT_QUEUE_DEPTH * sizeof(TcpEvt)
 *
 * Must be large enough to absorb a burst of MAX_CONNS * 4 events without
 * blocking the lwIP thread.  DeterministicAsyncTCP::heap_needed() returns
 * the exact byte count; call DetWebServer::heap_available() before begin()
 * to verify a contiguous block exists.
 */
#ifndef EVT_QUEUE_DEPTH
#define EVT_QUEUE_DEPTH 16
#endif

// ---------------------------------------------------------------------------
// Internal response buffer sizing constants
// ---------------------------------------------------------------------------

/**
 * @brief Stack buffer for HTTP response header lines in send() / send_empty() /
 *        send_unauth() / serve_file().
 *
 * Must be large enough to hold the status line, Content-Type, Content-Length,
 * Connection, and any CORS headers.  The CORS block alone can reach
 * CORS_HDR_BUF_SIZE bytes, so this value should be at least
 * CORS_HDR_BUF_SIZE + 96.
 */
#ifndef RESP_HDR_BUF_SIZE
#define RESP_HDR_BUF_SIZE 768
#endif

/**
 * @brief Per-connection buffer for app-supplied custom response headers and
 *        cookies.
 *
 * Filled by add_response_header() / set_cookie() and injected into send() /
 * send_empty() / redirect() the same way the CORS block is. RESP_HDR_BUF_SIZE
 * must be large enough to hold the status line plus the CORS block plus this
 * block (see the assert below).
 */
#ifndef EXTRA_HDR_BUF_SIZE
#define EXTRA_HDR_BUF_SIZE 256
#endif

/**
 * @brief Stack buffer for the HTTP 101 Switching Protocols response sent during
 *        the WebSocket handshake.
 *
 * Must hold: status line + Upgrade + Connection + Sec-WebSocket-Accept (28
 * base64 chars) + CRLF pairs.  Minimum is ~120 bytes; default leaves margin.
 */
#ifndef WS_HDR_BUF_SIZE
#define WS_HDR_BUF_SIZE 256
#endif

/**
 * @brief Size of the pre-built CORS header block stored in DetWebServer.
 *
 * Built once by set_cors() and injected into every response.  Must hold
 * Access-Control-Allow-Origin, Access-Control-Allow-Methods, and
 * Access-Control-Allow-Headers lines for the configured origin.
 */
#ifndef CORS_HDR_BUF_SIZE
#define CORS_HDR_BUF_SIZE 192
#endif

// ---------------------------------------------------------------------------
// Feature flags
// ---------------------------------------------------------------------------
// Set any of these to 0 in your sketch BEFORE including this library to strip
// the feature from the build entirely (no code, no RAM, no flash cost).
//
//   #define DETWS_ENABLE_WEBSOCKET 0
//   #include <DeterministicESPAsyncWebServer.h>

/** @brief WebSocket support (RFC 6455 framing + SHA-1/base64 handshake). */
#ifndef DETWS_ENABLE_WEBSOCKET
#define DETWS_ENABLE_WEBSOCKET 1
#endif

/** @brief Server-Sent Events push support. */
#ifndef DETWS_ENABLE_SSE
#define DETWS_ENABLE_SSE 1
#endif

/** @brief multipart/form-data body parser. */
#ifndef DETWS_ENABLE_MULTIPART
#define DETWS_ENABLE_MULTIPART 1
#endif

/** @brief Static file serving via Arduino FS (LittleFS, SPIFFS, SD). */
#ifndef DETWS_ENABLE_FILE_SERVING
#define DETWS_ENABLE_FILE_SERVING 1
#endif

/**
 * @brief WebDAV server (RFC 4918, class 1 + advisory locks) over the file system.
 *
 * Default off. When set (requires DETWS_ENABLE_FILE_SERVING), dav() mounts an FS
 * subtree that answers the WebDAV methods - OPTIONS, PROPFIND (Depth 0/1), GET,
 * HEAD, PUT, DELETE, MKCOL, COPY, MOVE, and advisory LOCK/UNLOCK - so a client
 * (rclone, cadaver, curl, or a mounted network drive) can browse and edit files.
 * PROPFIND returns a 207 Multi-Status document built into a fixed buffer
 * (DETWS_WEBDAV_BUF_SIZE); a Depth-1 listing is capped at
 * DETWS_WEBDAV_MAX_ENTRIES children. PUT buffers the body (bounded by
 * BODY_BUF_SIZE - large uploads need the streaming-body sink). Locks are advisory
 * (a synthetic token is issued but not enforced). PROPPATCH is not supported (the
 * properties are read-only). See docs/SECURITY.md before exposing it.
 */
#ifndef DETWS_ENABLE_WEBDAV
#define DETWS_ENABLE_WEBDAV 0
#endif

/** @brief Buffer (BSS) for a WebDAV 207 Multi-Status response, in bytes (see DETWS_ENABLE_WEBDAV). */
#ifndef DETWS_WEBDAV_BUF_SIZE
#define DETWS_WEBDAV_BUF_SIZE 2048
#endif

/** @brief Maximum children listed in a WebDAV Depth-1 PROPFIND (bounds the response). */
#ifndef DETWS_WEBDAV_MAX_ENTRIES
#define DETWS_WEBDAV_MAX_ENTRIES 32
#endif

/**
 * @brief HTTP method-token buffer size (bytes, including the NUL).
 *
 * Sized for the longest method the server must recognize: 8 normally (OPTIONS),
 * grown to fit the WebDAV methods (PROPPATCH is 9 chars) when WebDAV is enabled.
 */
#ifndef DETWS_METHOD_BUF_SIZE
#if DETWS_ENABLE_WEBDAV
#define DETWS_METHOD_BUF_SIZE 12
#else
#define DETWS_METHOD_BUF_SIZE 8
#endif
#endif

/** @brief HTTP Basic Authentication per-route. */
#ifndef DETWS_ENABLE_AUTH
#define DETWS_ENABLE_AUTH 1
#endif

/** @brief Telnet server support (RFC 854 / IAC option negotiation). */
#ifndef DETWS_ENABLE_TELNET
#define DETWS_ENABLE_TELNET 0
#endif

/** @brief SSH server support (RFC 4253/4252/4254). */
#ifndef DETWS_ENABLE_SSH
#define DETWS_ENABLE_SSH 0
#endif

/**
 * @brief TLS (HTTPS/WSS) via mbedTLS with a static memory pool (ESP32-only).
 *
 * When set, the server can accept TLS connections using mbedTLS configured with
 * MBEDTLS_MEMORY_BUFFER_ALLOC_C over a fixed BSS arena (DETWS_TLS_ARENA_SIZE) -
 * no system heap, so the determinism guarantee is preserved. The TLS engine is
 * compiled only on Arduino/ESP32 (mbedTLS is not part of the native build).
 * Default off.
 */
#ifndef DETWS_ENABLE_TLS
#define DETWS_ENABLE_TLS 0
#endif

/** @brief Maximum simultaneous TLS connections (each holds mbedTLS record buffers). */
#ifndef MAX_TLS_CONNS
#define MAX_TLS_CONNS 1
#endif

/**
 * @brief Mutual TLS - require and verify a client certificate (mTLS).
 *
 * Default off. When set (requires DETWS_ENABLE_TLS), the server can be given a
 * trust-anchor CA via DetWebServer::tls_require_client_cert(): the TLS handshake
 * then demands a client certificate chaining to that CA
 * (MBEDTLS_SSL_VERIFY_REQUIRED) and aborts the connection if the client presents
 * none or an untrusted one. The verified peer's subject DN is available to
 * handlers via DetWebServer::tls_client_subject(). Strong transport-level client
 * authentication with no passwords.
 */
#ifndef DETWS_ENABLE_MTLS
#define DETWS_ENABLE_MTLS 0
#endif

/** @brief Maximum length of a verified mTLS peer subject DN string (incl. NUL). */
#ifndef DETWS_MTLS_SUBJECT_MAX
#define DETWS_MTLS_SUBJECT_MAX 128
#endif

/**
 * @brief SNMP agent (v1/v2c, + v3 USM when DETWS_ENABLE_SNMP_V3) over lwIP UDP.
 *
 * Zero-heap ASN.1 BER codec + a fixed MIB table on UDP/161. Default off. The BER
 * codec itself is gated by this flag and is otherwise unit-tested standalone
 * (env:native_snmp).
 */
#ifndef DETWS_ENABLE_SNMP
#define DETWS_ENABLE_SNMP 0
#endif

/** @brief Add SNMPv3 USM (auth via HMAC-SHA, privacy via AES-128-CFB). Default off. */
#ifndef DETWS_ENABLE_SNMP_V3
#define DETWS_ENABLE_SNMP_V3 0
#endif

/**
 * @brief Outbound SNMP notifications - traps and informs (requires DETWS_ENABLE_SNMP).
 *
 * Default off. When set, src/services/snmp/snmp_notify.h sends SNMPv2c (and, with
 * DETWS_ENABLE_SNMP_V3, SNMPv3 USM) Trap / InformRequest PDUs to a manager over
 * UDP - so the agent can push alerts instead of only answering polls. Reuses the
 * BER codec and the transport-layer UDP service; the PDU builder is host-testable.
 */
#ifndef DETWS_ENABLE_SNMP_TRAP
#define DETWS_ENABLE_SNMP_TRAP 0
#endif

/** @brief Maximum extra variable-bindings (beyond sysUpTime/snmpTrapOID) in one notification. */
#ifndef DETWS_SNMP_TRAP_MAX_VARBINDS
#define DETWS_SNMP_TRAP_MAX_VARBINDS 8
#endif

/** @brief Static datagram buffer for an outbound SNMP notification, bytes. */
#ifndef DETWS_SNMP_TRAP_BUF_SIZE
#define DETWS_SNMP_TRAP_BUF_SIZE 1024
#endif

/** @brief Maximum sub-identifiers (arcs) in an SNMP object identifier. */
#ifndef SNMP_MAX_OID_LEN
#define SNMP_MAX_OID_LEN 32
#endif

/**
 * @brief Maximum registered MIB objects (the agent's fixed OID table).
 *
 * Each entry holds its OID, a value descriptor, and optional get/set callbacks
 * (see src/services/snmp/snmp_agent.h). The table lives in BSS; entries are
 * scanned linearly (small table) and need not be registered in OID order.
 */
#ifndef SNMP_MAX_MIB_ENTRIES
#define SNMP_MAX_MIB_ENTRIES 16
#endif

/**
 * @brief Maximum variable bindings the agent will emit in one response.
 *
 * Bounds GetBulk expansion (max-repetitions is clamped so the total response
 * varbind count never exceeds this) and the per-request decode scratch.
 */
#ifndef SNMP_MAX_VARBINDS
#define SNMP_MAX_VARBINDS 16
#endif

/**
 * @brief Static request/response datagram buffers for the SNMP UDP agent.
 *
 * Two buffers of this size live in BSS (one in, one out) - no heap. 484 is the
 * RFC 1157 minimum maximum message size; the default holds a one-frame UDP
 * payload so GetBulk walks fit without IP fragmentation.
 */
#ifndef SNMP_MSG_BUF_SIZE
#define SNMP_MSG_BUF_SIZE 1472
#endif

/** @brief Maximum SNMP community-string length (including null terminator). */
#ifndef SNMP_COMMUNITY_MAX
#define SNMP_COMMUNITY_MAX 32
#endif

/** @brief Maximum SNMPv3 USM user-name length (including null terminator). */
#ifndef SNMP_V3_USER_MAX
#define SNMP_V3_USER_MAX 32
#endif

/** @brief Maximum SNMPv3 authoritative engine-ID length in bytes (RFC 3411 allows 5..32). */
#ifndef SNMP_V3_ENGINEID_MAX
#define SNMP_V3_ENGINEID_MAX 32
#endif

// ---------------------------------------------------------------------------
// CoAP server sizing constants  (DETWS_ENABLE_COAP must be 1)
// ---------------------------------------------------------------------------

/**
 * @brief CoAP server (RFC 7252) over UDP/5683.
 *
 * A zero-heap Constrained Application Protocol endpoint: a fixed resource table
 * dispatched against the request's Uri-Path, with a pure host-testable message
 * codec (parse/build) and an ESP32 UDP binding via the transport-layer UDP
 * service. Default off; the codec is otherwise unit-tested standalone
 * (env:native_coap).
 */
#ifndef DETWS_ENABLE_COAP
#define DETWS_ENABLE_COAP 0
#endif

/**
 * @brief CoAP resource observation - RFC 7641 (requires DETWS_ENABLE_COAP).
 *
 * Default off. When set, a client GET with the Observe option registers as an
 * observer of a resource; the application calls coap_notify(path) to push the
 * resource's current representation to every observer (a CoAP notification from
 * the server port with an increasing Observe sequence). Observers are dropped on
 * a deregister GET, a client RST, or send failure.
 */
#ifndef DETWS_ENABLE_COAP_OBSERVE
#define DETWS_ENABLE_COAP_OBSERVE 0
#endif

/** @brief Maximum simultaneous CoAP observers (one slot per observed resource per client). */
#ifndef DETWS_COAP_MAX_OBSERVERS
#define DETWS_COAP_MAX_OBSERVERS 4
#endif

/**
 * @brief CoAP block-wise transfer - RFC 7959 (requires DETWS_ENABLE_COAP).
 *
 * Default off. When set, the server understands the Block2 (descriptive,
 * responses) and Block1 (control, request uploads) options:
 *  - Block2: a representation larger than one block, or any GET that carries a
 *    Block2 option, is served one block at a time. A constrained client requests
 *    a small block size (SZX) and pages through with ascending block numbers; the
 *    server re-renders the (idempotent) resource and slices out the asked-for
 *    block, setting the More bit until the last.
 *  - Block1: a POST/PUT payload larger than one block is reassembled into a
 *    single BSS buffer. Each non-final block is acknowledged 2.31 Continue; the
 *    final block dispatches the handler with the whole reassembled payload.
 *
 * One block-wise transfer is reassembled at a time (deterministic, single
 * buffer); an out-of-order or oversized block yields 4.08 / 4.13. Size1/Size2
 * options and the /.well-known/core listing are out of scope.
 */
#ifndef DETWS_ENABLE_COAP_BLOCK
#define DETWS_ENABLE_COAP_BLOCK 0
#endif

/** @brief Largest block-size exponent (SZX) the server will use: block size = 2^(SZX+4) bytes, SZX 0..6 (16..1024). */
#ifndef DETWS_COAP_BLOCK_SZX_MAX
#define DETWS_COAP_BLOCK_SZX_MAX 6
#endif

/**
 * @brief Reassembly buffer for a block-wise (Block1) request upload, in bytes.
 *
 * One buffer of this size lives in BSS only when DETWS_ENABLE_COAP_BLOCK is set.
 * It bounds the largest payload a chunked POST/PUT can deliver to a handler.
 */
#ifndef DETWS_COAP_BLOCK1_MAX
#define DETWS_COAP_BLOCK1_MAX 1024
#endif

/**
 * @brief Maximum registered CoAP resources (the server's fixed routing table).
 *
 * Each entry holds a path pointer, an allowed-methods bitmask, and a handler.
 * The table lives in BSS and is scanned linearly (small table).
 */
#ifndef DETWS_COAP_MAX_RESOURCES
#define DETWS_COAP_MAX_RESOURCES 8
#endif

/** @brief Maximum reconstructed Uri-Path length, including separators and the leading '/'. */
#ifndef DETWS_COAP_MAX_PATH
#define DETWS_COAP_MAX_PATH 64
#endif

/** @brief Maximum reconstructed Uri-Query length (segments joined by '&'). */
#ifndef DETWS_COAP_MAX_QUERY
#define DETWS_COAP_MAX_QUERY 64
#endif

/**
 * @brief Maximum CoAP request/response payload in bytes.
 *
 * Sizes the static scratch a handler writes its response body into and bounds
 * the request payload handed to it. One buffer of this size lives in BSS.
 */
#ifndef DETWS_COAP_MAX_PAYLOAD
#define DETWS_COAP_MAX_PAYLOAD 256
#endif

/**
 * @brief Static response-datagram buffer for the CoAP UDP server.
 *
 * One buffer of this size lives in BSS (the request is transport-owned). Must
 * hold a 4-byte header + token (<=8) + the Content-Format option + a 0xFF marker
 * + DETWS_COAP_MAX_PAYLOAD bytes. When block-wise transfer is enabled it must
 * also hold one full block (2^(DETWS_COAP_BLOCK_SZX_MAX+4) bytes) + option
 * overhead, so the default grows accordingly.
 */
#ifndef DETWS_COAP_MSG_BUF_SIZE
#if DETWS_ENABLE_COAP_BLOCK
#define DETWS_COAP_MSG_BUF_SIZE 1152
#else
#define DETWS_COAP_MSG_BUF_SIZE 512
#endif
#endif

/**
 * @brief Bytes of the static BSS arena mbedTLS allocates from (DETWS_ENABLE_TLS).
 *
 * All mbedTLS allocations (per-connection record buffers, handshake temporaries,
 * cert/key parsing) are served from this fixed arena via a custom allocator
 * installed with mbedtls_platform_set_calloc_free() - never the system heap. Must
 * cover the worst-case handshake peak for MAX_TLS_CONNS; if undersized the
 * handshake fails cleanly (no corruption). Measured peak for ONE ECDSA P-256
 * connection on Arduino-esp32 (16 KB IN + 16 KB OUT records) is ~41.5 KB, so the
 * default leaves a small margin. An RSA cert/larger chain needs more; query the
 * live peak via det_tls_arena_peak(). NOTE: a second concurrent TLS connection
 * roughly doubles the record-buffer cost (~32 KB more), which overflows the
 * static DRAM budget - keep MAX_TLS_CONNS at 1 unless you shrink the IDF record
 * sizes (CONFIG_MBEDTLS_SSL_IN/OUT_CONTENT_LEN, needs an ESP-IDF build).
 */
#ifndef DETWS_TLS_ARENA_SIZE
#define DETWS_TLS_ARENA_SIZE 49152
#endif

// ---------------------------------------------------------------------------
// Optional network services (ESP32-only thin wrappers; each default-off so it
// costs no code/RAM/flash unless explicitly enabled).
// ---------------------------------------------------------------------------

/** @brief mDNS / DNS-SD advertisement (`name.local` + `_http._tcp`) via ESPmDNS. */
#ifndef DETWS_ENABLE_MDNS
#define DETWS_ENABLE_MDNS 0
#endif

/** @brief SNTP wall-clock time sync via the ESP-IDF SNTP client. */
#ifndef DETWS_ENABLE_NTP
#define DETWS_ENABLE_NTP 0
#endif

/** @brief Authenticated OTA firmware update (streaming POST to the ESP32 Update API). */
#ifndef DETWS_ENABLE_OTA
#define DETWS_ENABLE_OTA 0
#endif

/**
 * @brief Streaming file upload: POST a body straight to a file on the filesystem.
 *
 * Default off. When set, src/services/upload_service.h registers a POST route
 * that streams the request body directly into an Arduino FS file (LittleFS /
 * SPIFFS / SD) - the upload never has to fit in RAM. Reuses the same parser
 * streaming-body hook as OTA.
 *
 * For reliable uploads set RX_BUF_SIZE above the largest inbound TCP segment
 * (TCP_MSS, ~1460): the transport refuses-and-redelivers a segment that will not
 * fit the receive ring (lossless backpressure), but a ring smaller than one
 * segment would stall. The 1024 default suits ordinary requests, not uploads.
 */
#ifndef DETWS_ENABLE_UPLOAD
#define DETWS_ENABLE_UPLOAD 0
#endif

/**
 * @brief Internal: the parser's streaming-body machinery (OTA or file upload).
 *
 * Both stream the request body to a sink instead of buffering it into body[];
 * the parser support is shared and compiled when either feature is enabled.
 */
#if DETWS_ENABLE_OTA || DETWS_ENABLE_UPLOAD
#define DETWS_ENABLE_STREAM_BODY 1
#else
#define DETWS_ENABLE_STREAM_BODY 0
#endif

/** @brief First-boot WiFi provisioning: softAP + captive-portal credentials form. */
#ifndef DETWS_ENABLE_PROVISIONING
#define DETWS_ENABLE_PROVISIONING 0
#endif

/**
 * @brief Syslog client (RFC 5424 over UDP).
 *
 * Default off. When set, the device can ship log lines to a remote syslog server
 * (e.g. rsyslog / journald / a SIEM) as RFC 5424 UDP datagrams via the
 * transport-layer UDP service - a zero-heap structured-logging sink for fleets
 * of constrained devices. See src/services/syslog/syslog.h.
 */
#ifndef DETWS_ENABLE_SYSLOG
#define DETWS_ENABLE_SYSLOG 0
#endif

/** @brief Maximum formatted syslog datagram length in bytes (RFC 5424 line). */
#ifndef DETWS_SYSLOG_MSG_MAX
#define DETWS_SYSLOG_MSG_MAX 256
#endif

/** @brief Maximum syslog HOSTNAME / APP-NAME field length (including NUL). */
#ifndef DETWS_SYSLOG_FIELD_MAX
#define DETWS_SYSLOG_FIELD_MAX 32
#endif

/**
 * @brief JWT bearer-token authentication (HS256).
 *
 * Default off. When set, src/services/jwt/jwt.h verifies `Authorization: Bearer
 * <jwt>` tokens signed with HMAC-SHA-256 (reusing the SSH crypto layer) and can
 * read integer claims (e.g. `exp`) so a handler/middleware can gate routes on a
 * stateless token. Signature verification is constant-time.
 */
#ifndef DETWS_ENABLE_JWT
#define DETWS_ENABLE_JWT 0
#endif

/** @brief Maximum accepted JWT length in bytes (header.payload.signature). */
#ifndef DETWS_JWT_MAX_LEN
#define DETWS_JWT_MAX_LEN 512
#endif

/**
 * @brief Outbound HTTP(S) client (raw lwIP, optional client-side mbedTLS).
 *
 * Default off. When set, src/services/http_client/http_client.h can issue a
 * blocking GET/POST to a remote server: it resolves the host (DNS), opens a raw
 * lwIP TCP connection (https:// goes through client-side mbedTLS over the same
 * static arena as the server TLS), sends the request, and returns the status +
 * body in caller buffers. For webhooks, telemetry push, REST calls from the
 * device. The request builder + response parser are host-testable; the transport
 * is ESP32-only.
 */
#ifndef DETWS_ENABLE_HTTP_CLIENT
#define DETWS_ENABLE_HTTP_CLIENT 0
#endif

/** @brief HTTPS client support inside the HTTP client (needs DETWS_ENABLE_TLS). */
#ifndef DETWS_ENABLE_HTTP_CLIENT_TLS
#define DETWS_ENABLE_HTTP_CLIENT_TLS 0
#endif

/** @brief Receive buffer (and max response size) for the outbound HTTP client, bytes. */
#ifndef DETWS_HTTP_CLIENT_BUF_SIZE
#define DETWS_HTTP_CLIENT_BUF_SIZE 2048
#endif

/**
 * @brief Ciphertext receive-ring size for the https:// client, bytes.
 *
 * The lwIP recv callback feeds TLS wire bytes into this draining ring while the
 * TLS engine pulls and decrypts them, so it holds only the in-flight (not yet
 * decrypted) ciphertext: a multi-KB handshake flight fits without loss thanks to
 * the refuse-and-redeliver backpressure. Must exceed one TCP segment (TCP_MSS,
 * ~1460) or a full segment could never fit. Only used when
 * DETWS_ENABLE_HTTP_CLIENT_TLS is set.
 */
#ifndef DETWS_HTTP_CLIENT_CT_BUF_SIZE
#define DETWS_HTTP_CLIENT_CT_BUF_SIZE 4096
#endif

/** @brief Outbound HTTP client connect/response timeout in milliseconds. */
#ifndef DETWS_HTTP_CLIENT_TIMEOUT_MS
#define DETWS_HTTP_CLIENT_TIMEOUT_MS 8000
#endif

/**
 * @brief MQTT 3.1.1 publish/subscribe client (raw lwIP, optional MQTTS over TLS).
 *
 * Default off. When set, src/services/mqtt/mqtt.h provides a persistent outbound
 * client: connect to a broker, PUBLISH (QoS 0/1) and SUBSCRIBE to topics, receive
 * incoming messages via a callback, with keep-alive pings - the dominant IoT
 * messaging pattern, for telemetry push and remote command. The packet codec is
 * host-testable; the transport (DNS + raw lwIP TCP, MQTTS via client-side mbedTLS)
 * is ESP32-only. QoS 2 and Last-Will are not implemented.
 */
#ifndef DETWS_ENABLE_MQTT
#define DETWS_ENABLE_MQTT 0
#endif

/** @brief MQTTS: run the MQTT client over client-side TLS (needs DETWS_ENABLE_TLS). */
#ifndef DETWS_ENABLE_MQTT_TLS
#define DETWS_ENABLE_MQTT_TLS 0
#endif

/**
 * @brief MQTT packet buffer size in bytes (bounds one outgoing/incoming packet).
 *
 * Two buffers of this size live in BSS (one tx, one rx). Must hold the largest
 * CONNECT/PUBLISH the client sends and the largest incoming PUBLISH it accepts
 * (topic + payload + a few header bytes); larger incoming packets are dropped.
 */
#ifndef DETWS_MQTT_BUF_SIZE
#define DETWS_MQTT_BUF_SIZE 1024
#endif

/** @brief Default MQTT keep-alive interval in seconds (PINGREQ cadence / CONNECT field). */
#ifndef DETWS_MQTT_KEEPALIVE_S
#define DETWS_MQTT_KEEPALIVE_S 30
#endif

/** @brief Ciphertext receive-ring size for MQTTS (draining ring; must exceed one TCP_MSS). */
#ifndef DETWS_MQTT_CT_BUF_SIZE
#define DETWS_MQTT_CT_BUF_SIZE 4096
#endif

/** @brief Maximum inbound MQTT topic length (including NUL) delivered to the callback. */
#ifndef DETWS_MQTT_MAX_TOPIC
#define DETWS_MQTT_MAX_TOPIC 128
#endif

/**
 * @brief Outbound QoS 1/2 in-flight slots (unacknowledged messages held for DUP retransmit).
 *
 * Each slot stores its serialized packet (up to DETWS_MQTT_INFLIGHT_BUF bytes) until
 * the broker acknowledges it; a publish is refused when all slots are busy. The pool
 * costs DETWS_MQTT_MAX_INFLIGHT * (DETWS_MQTT_INFLIGHT_BUF + a few bytes) of BSS.
 */
#ifndef DETWS_MQTT_MAX_INFLIGHT
#define DETWS_MQTT_MAX_INFLIGHT 4
#endif

/** @brief Stored-packet size per in-flight QoS 1/2 slot (caps a retransmittable PUBLISH). */
#ifndef DETWS_MQTT_INFLIGHT_BUF
#define DETWS_MQTT_INFLIGHT_BUF 256
#endif

/** @brief Retransmit timeout (ms) for an unacknowledged in-flight QoS 1/2 message. */
#ifndef DETWS_MQTT_RETRANSMIT_MS
#define DETWS_MQTT_RETRANSMIT_MS 5000
#endif

/** @brief Inbound QoS 2 packet-id de-duplication ring depth (PUBREC-acknowledged, awaiting PUBREL). */
#ifndef DETWS_MQTT_RX_QOS2_SLOTS
#define DETWS_MQTT_RX_QOS2_SLOTS 8
#endif

/**
 * @brief Outbound WebSocket client (RFC 6455 over raw lwIP, optional wss:// TLS).
 *
 * Default off. When set, src/services/ws_client/ws_client.h connects to a remote
 * WebSocket endpoint (ws://, or wss:// over client-side mbedTLS), performs the
 * RFC 6455 client handshake (Sec-WebSocket-Key/Accept), and sends masked text /
 * binary frames + receives server frames via a callback - for streaming to cloud
 * dashboards or bidirectional control. The frame/handshake codec is host-testable.
 */
#ifndef DETWS_ENABLE_WS_CLIENT
#define DETWS_ENABLE_WS_CLIENT 0
#endif

/** @brief wss://: run the WebSocket client over client-side TLS (needs DETWS_ENABLE_TLS). */
#ifndef DETWS_ENABLE_WS_CLIENT_TLS
#define DETWS_ENABLE_WS_CLIENT_TLS 0
#endif

/** @brief WebSocket client send/receive buffer size in bytes (bounds one frame). */
#ifndef DETWS_WS_CLIENT_BUF_SIZE
#define DETWS_WS_CLIENT_BUF_SIZE 1024
#endif

/** @brief Ciphertext receive-ring size for wss:// (draining ring; must exceed one TCP_MSS). */
#ifndef DETWS_WS_CLIENT_CT_BUF_SIZE
#define DETWS_WS_CLIENT_CT_BUF_SIZE 4096
#endif

/**
 * @brief Internal: client-side TLS engine is compiled (HTTPS client, MQTTS, and/or wss client).
 *
 * The outbound HTTP client (one-shot exchange) and the MQTT / WebSocket clients
 * (persistent sessions) share the same client mbedTLS code in det_tls - the
 * CA/pin trust config, the BIO typedefs, and the session API - gated by this.
 */
#if DETWS_ENABLE_HTTP_CLIENT_TLS || DETWS_ENABLE_MQTT_TLS || DETWS_ENABLE_WS_CLIENT_TLS
#define DETWS_ENABLE_CLIENT_TLS 1
#else
#define DETWS_ENABLE_CLIENT_TLS 0
#endif

// ---------------------------------------------------------------------------
// Full Authorization-header capture (internal)
// ---------------------------------------------------------------------------
// Digest auth and JWT bearer tokens both carry an Authorization value far longer
// than MAX_VAL_LEN, so the parser captures the whole header into a dedicated
// per-request buffer (HttpReq::authorization) when either feature is enabled.

/** @brief True when the parser must capture the full Authorization header value. */
#if DETWS_ENABLE_AUTH || DETWS_ENABLE_JWT
#define DETWS_CAPTURE_AUTH_HEADER 1
#else
#define DETWS_CAPTURE_AUTH_HEADER 0
#endif

/**
 * @brief Capacity of HttpReq::authorization (full Authorization header value).
 *
 * Sized to the larger consumer: a Digest header (DIGEST_AUTH_HDR_MAX) or a
 * `Bearer <jwt>` token (DETWS_JWT_MAX_LEN + the scheme).
 */
#if DETWS_ENABLE_JWT && (DETWS_JWT_MAX_LEN + 16 > DIGEST_AUTH_HDR_MAX)
#define DETWS_AUTH_HDR_CAP (DETWS_JWT_MAX_LEN + 16)
#else
#define DETWS_AUTH_HDR_CAP DIGEST_AUTH_HDR_MAX
#endif

/** @brief Runtime stats endpoint (uptime, request/error counts, pool usage, heap). */
#ifndef DETWS_ENABLE_STATS
#define DETWS_ENABLE_STATS 0
#endif

/**
 * @brief Prometheus `/metrics` endpoint (text exposition format 0.0.4).
 *
 * Default off (requires DETWS_ENABLE_STATS for the underlying counters). When
 * set, DetWebServer::metrics() emits the runtime stats as Prometheus metrics
 * (`detws_uptime_seconds`, `detws_http_requests_total`,
 * `detws_http_responses_total{class=...}`, `detws_active_connections`,
 * `detws_free_heap_bytes`, ...) so a Prometheus server can scrape the device.
 */
#ifndef DETWS_ENABLE_METRICS
#define DETWS_ENABLE_METRICS 0
#endif

/**
 * @brief Browser "web serial" terminal over WebSocket (src/services/web_terminal).
 *
 * Serves a self-contained terminal page and a WebSocket endpoint: device output
 * is broadcast to all connected browsers, browser input is delivered to a
 * command callback. Requires DETWS_ENABLE_WEBSOCKET. Default off.
 */
#ifndef DETWS_ENABLE_WEB_TERMINAL
#define DETWS_ENABLE_WEB_TERMINAL 0
#endif

/**
 * @brief Stack scratch for detws_web_terminal_printf()/println() formatting.
 *
 * One formatted terminal line must fit in this many bytes (longer is truncated).
 * Allocated on the stack only during the call - no persistent RAM cost.
 */
#ifndef TERM_TX_BUF_SIZE
#define TERM_TX_BUF_SIZE 256
#endif

/**
 * @brief Conditional GET via ETag for served files.
 *
 * When set, serve_file()/serve_static() emit a strong `ETag` (derived from the
 * file size + last-modified time) and answer a matching `If-None-Match` with
 * `304 Not Modified`, saving bandwidth on repeat fetches of static assets.
 */
#ifndef DETWS_ENABLE_ETAG
#define DETWS_ENABLE_ETAG 0
#endif

/**
 * @brief Expose a diagnostic JSON endpoint via server.diag().
 *
 * Disabled by default - enabling it exposes compile-time configuration
 * (buffer sizes, feature flags) which could aid an attacker.  Only
 * enable in development or behind an authenticated route.
 *
 * When enabled, DETWS_DIAG_JSON is a compile-time string constant you can
 * serve from any route handler:
 * @code
 *   server.on("/diag", HTTP_GET, [](uint8_t id, HttpReq *) {
 *       server.diag(id);        // convenience wrapper
 *       // or:
 *       server.send(id, 200, "application/json", DETWS_DIAG_JSON);
 *   });
 * @endcode
 */
#ifndef DETWS_ENABLE_DIAG
#define DETWS_ENABLE_DIAG 0
#endif

/**
 * @brief HTTP/1.1 persistent connections (keep-alive).
 *
 * Default off (every response carries `Connection: close` and the connection is
 * closed after one request - the long-standing behavior). When set to 1, a
 * cleanly-parsed request is answered with `Connection: keep-alive` and the slot
 * is recycled for the next request on the same socket: HTTP/1.1 keeps the
 * connection open unless the client sends `Connection: close`; HTTP/1.0 closes
 * unless the client sends `Connection: keep-alive`. Error responses (400/413/414
 * and any non-PARSE_COMPLETE path) always close, since the next request boundary
 * is unknown. Idle keep-alive connections are still reclaimed by the existing
 * conn_timeout sweep, and each connection serves at most
 * DETWS_KEEPALIVE_MAX_REQUESTS requests before a deliberate close.
 */
#ifndef DETWS_ENABLE_KEEPALIVE
#define DETWS_ENABLE_KEEPALIVE 0
#endif

/**
 * @brief Maximum requests served on one keep-alive connection before it is closed.
 *
 * A fairness bound so a single client cannot hold a connection slot
 * indefinitely with a steady request stream. After this many responses the
 * server emits `Connection: close` and drops the link; the client simply
 * reconnects. Only meaningful when DETWS_ENABLE_KEEPALIVE is set.
 */
#ifndef DETWS_KEEPALIVE_MAX_REQUESTS
#define DETWS_KEEPALIVE_MAX_REQUESTS 100
#endif

/**
 * @brief HTTP Range requests / 206 Partial Content for served files.
 *
 * Default off. When set (requires DETWS_ENABLE_FILE_SERVING), serve_file() /
 * serve_static() honor a single-range `Range: bytes=...` request header: they
 * answer `206 Partial Content` with a `Content-Range` header and stream only the
 * requested bytes (seeking the file to the start offset), advertise
 * `Accept-Ranges: bytes` on full responses, and answer an unsatisfiable range
 * with `416 Range Not Satisfiable`. This enables resumable downloads and media
 * seeking. Multi-range (multipart/byteranges) requests are not supported - the
 * server falls back to a full 200 response, which is RFC 7233 §3.1 compliant.
 */
#ifndef DETWS_ENABLE_RANGE
#define DETWS_ENABLE_RANGE 0
#endif

/**
 * @brief Enforce the RFC 7230 §5.4 Host-header requirement (default on).
 *
 * When 1, an HTTP/1.1 request that lacks a Host header - or carries more than
 * one - is rejected with 400 Bad Request. When 0, the Host header is not
 * required (useful for constrained clients or test harnesses that feed bare
 * request lines). The multiple-Host rule and Content-Length validation are
 * always active regardless of this flag.
 */
#ifndef DETWS_ENFORCE_HOST_HEADER
#define DETWS_ENFORCE_HOST_HEADER 1
#endif

/**
 * @brief Allow SSH password authentication (default on).
 *
 * Set to 0 to harden the SSH server to publickey-only authentication
 * (RFC 4252 §7): the "password" method is then refused outright and is not
 * advertised in the USERAUTH_FAILURE method list. Publickey auth is always
 * available regardless of this flag.
 */
#ifndef DETWS_SSH_ALLOW_PASSWORD
#define DETWS_SSH_ALLOW_PASSWORD 1
#endif

/**
 * @brief Maximum failed SSH authentication attempts per connection.
 *
 * RFC 4252 §4 permits the server to disconnect after a small bounded number of
 * failed USERAUTH_REQUESTs. After this many SSH_MSG_USERAUTH_FAILURE responses
 * on one connection the server sends SSH_MSG_DISCONNECT and drops the link.
 * (The publickey "would-be-accepted" probe and a SUCCESS do not count.)
 */
#ifndef SSH_MAX_AUTH_ATTEMPTS
#define SSH_MAX_AUTH_ATTEMPTS 6
#endif

// ---------------------------------------------------------------------------
// Listener pool
// ---------------------------------------------------------------------------

/** @brief Maximum number of simultaneously active listener ports. */
#ifndef MAX_LISTENERS
#define MAX_LISTENERS 3
#endif

/**
 * @brief Maximum simultaneously bound UDP ports (transport-layer UDP service).
 *
 * Sizes the fixed pool in udp_transport.cpp. One slot per bound port, e.g. SNMP
 * (:161) and the captive-portal DNS responder (:53). Costs only a few pointers
 * of BSS each.
 */
#ifndef DETWS_MAX_UDP_LISTENERS
#define DETWS_MAX_UDP_LISTENERS 2
#endif

/**
 * @brief Shared receive-scratch size for the transport-layer UDP service.
 *
 * One static buffer (lwIP delivers a single datagram at a time) into which each
 * incoming datagram is copied before the handler runs. Must hold the largest
 * datagram any UDP service expects (SNMP messages are the largest user).
 */
#ifndef DETWS_UDP_RX_BUF_SIZE
#define DETWS_UDP_RX_BUF_SIZE 1472
#endif

/**
 * @brief Opt-in global accept-rate throttle (connection-flood defense).
 *
 * Default off (zero cost / no behavior change). When set to 1 the accept
 * callback rejects new connections once more than DETWS_ACCEPT_THROTTLE_MAX
 * have been accepted within a DETWS_ACCEPT_THROTTLE_WINDOW_MS fixed window
 * (global across all listeners, two static counters - no per-IP table). This
 * bounds connection churn (e.g. reconnect brute-force) on top of the bounded
 * connection pool and the per-connection auth limits. mitigate finer-grained /
 * per-IP attacks at the network layer.
 */
#ifndef DETWS_ENABLE_ACCEPT_THROTTLE
#define DETWS_ENABLE_ACCEPT_THROTTLE 0
#endif

/** @brief Max accepted connections per throttle window (see DETWS_ENABLE_ACCEPT_THROTTLE). */
#ifndef DETWS_ACCEPT_THROTTLE_MAX
#define DETWS_ACCEPT_THROTTLE_MAX 20
#endif

/** @brief Throttle window length in milliseconds (see DETWS_ENABLE_ACCEPT_THROTTLE). */
#ifndef DETWS_ACCEPT_THROTTLE_WINDOW_MS
#define DETWS_ACCEPT_THROTTLE_WINDOW_MS 1000
#endif

/**
 * @brief Opt-in per-IP accept-rate throttle (connection-flood defense, keyed by source IPv4).
 *
 * Default off (zero cost / no behavior change). Complements the global accept
 * throttle: the accept callback rejects a new connection once one source IPv4
 * address has opened more than DETWS_PER_IP_THROTTLE_MAX connections within a
 * DETWS_PER_IP_THROTTLE_WINDOW_MS fixed window. A fixed BSS table of
 * DETWS_PER_IP_THROTTLE_SLOTS buckets tracks the most-recently-seen source
 * addresses; when a new address arrives and the table is full, an expired or
 * least-recently-started bucket is reused, so memory stays bounded (no heap).
 *
 * This bounds reconnect/brute-force churn from a single host (the gap left by the
 * global throttle, which cannot tell one noisy client from many). It is
 * best-effort: an attacker spreading across many source addresses can still churn
 * the bounded connection pool, so combine it with the global throttle and
 * network-layer filtering.
 */
#ifndef DETWS_ENABLE_PER_IP_THROTTLE
#define DETWS_ENABLE_PER_IP_THROTTLE 0
#endif

/** @brief Number of source IPv4 addresses tracked by the per-IP throttle (BSS bucket table). */
#ifndef DETWS_PER_IP_THROTTLE_SLOTS
#define DETWS_PER_IP_THROTTLE_SLOTS 16
#endif

/** @brief Max accepted connections per window from one source IP (see DETWS_ENABLE_PER_IP_THROTTLE). */
#ifndef DETWS_PER_IP_THROTTLE_MAX
#define DETWS_PER_IP_THROTTLE_MAX 10
#endif

/** @brief Per-IP throttle window length in milliseconds (see DETWS_ENABLE_PER_IP_THROTTLE). */
#ifndef DETWS_PER_IP_THROTTLE_WINDOW_MS
#define DETWS_PER_IP_THROTTLE_WINDOW_MS 10000
#endif

// ---------------------------------------------------------------------------
// Telnet sizing constants  (DETWS_ENABLE_TELNET must be 1)
// ---------------------------------------------------------------------------

/** @brief Maximum simultaneous Telnet connections. */
#ifndef MAX_TELNET_CONNS
#define MAX_TELNET_CONNS 2
#endif

/** @brief Stack buffer for one Telnet I/O chunk. */
#ifndef TELNET_BUF_SIZE
#define TELNET_BUF_SIZE 256
#endif

// ---------------------------------------------------------------------------
// SSH sizing constants  (DETWS_ENABLE_SSH must be 1)
// ---------------------------------------------------------------------------

/** @brief Maximum simultaneous SSH connections. */
#ifndef MAX_SSH_CONNS
#define MAX_SSH_CONNS 1
#endif

/** @brief Packet assembly buffer per SSH connection (bytes). */
#ifndef SSH_PKT_BUF_SIZE
#define SSH_PKT_BUF_SIZE 2048
#endif

/** @brief Maximum SSH username length including null terminator. */
#ifndef SSH_MAX_USERNAME_LEN
#define SSH_MAX_USERNAME_LEN 32
#endif

/** @brief Maximum SSH password length including null terminator. */
#ifndef SSH_MAX_PASSWORD_LEN
#define SSH_MAX_PASSWORD_LEN 64
#endif

/**
 * @brief Shared scratch buffer for SSH big-number operations.
 *
 * Holds Montgomery multiplication temporaries and RSA padding workspace.
 * Must be large enough for the biggest single crypto operation:
 *
 *   DH expmod:
 *     base_mont  (SshBigNum = 256 B)
 *     result     (SshBigNum = 256 B)
 *     tmp        (SshBigNum = 256 B)
 *     mont_t     (uint32_t[129] = 516 B)
 *                                ─────
 *                                1284 B  → round up with margin → 1536 B
 *
 * The buffer is zeroed via volatile memset immediately after each operation.
 * Only one SSH KEX may run at a time (guaranteed by the single Arduino loop
 * task and MAX_SSH_CONNS synchronous handshake model).
 */
#ifndef SSH_CRYPTO_WORK_SIZE
#define SSH_CRYPTO_WORK_SIZE 1536
#endif

// ---------------------------------------------------------------------------
// Static RAM (BSS) usage table
// ---------------------------------------------------------------------------
//
// All library memory is in BSS - allocated at link time, zero-initialized by
// the C runtime, never heap-allocated after begin().  The table below shows
// the contribution of every feature at its default constant values.
//
// Sizes are for ESP32 (32-bit pointers, int = 4 B).  Where a size depends on
// a macro the formula is given so you can compute the impact of any change.
//
// ┌──────────────────────────────┬──────────────────────────────────────────────────────────────┬──────────┐
// │ Symbol / pool                │ Size formula                                                 │ Default  │
// ├──────────────────────────────┼──────────────────────────────────────────────────────────────┼──────────┤
// │ TRANSPORT LAYER (always on)  │                                                              │          │
// │  conn_pool[MAX_CONNS]        │ MAX_CONNS × (RX_BUF_SIZE + 22)                              │  4 168 B │
// │  listener_pool[MAX_LISTENERS]│ MAX_LISTENERS × (StaticQueue_t≈48 + EVT_QUEUE_DEPTH×12 + 18)│    654 B │
// │  conn_timeout_ms             │ 4 B                                                          │      4 B │
// │  TRANSPORT SUBTOTAL          │                                                              │  4 826 B │
// ├──────────────────────────────┼──────────────────────────────────────────────────────────────┼──────────┤
// │ HTTP PRESENTATION (always on)│                                                              │          │
// │  http_pool[MAX_CONNS]        │ MAX_CONNS × (MAX_PATH_LEN + MAX_QUERY_LEN                   │          │
// │                              │   + MAX_HEADERS×(MAX_KEY_LEN+MAX_VAL_LEN)                   │          │
// │                              │   + MAX_QUERY_PARAMS×(QUERY_KEY_LEN+QUERY_VAL_LEN)          │          │
// │                              │   + BODY_BUF_SIZE + 50)                                     │  6 668 B │
// │  HTTP SUBTOTAL               │                                                              │  6 668 B │
// ├──────────────────────────────┼──────────────────────────────────────────────────────────────┼──────────┤
// │ WEBSOCKET (DETWS_ENABLE_WEBSOCKET=1)                                                        │          │
// │  ws_pool[MAX_WS_CONNS]       │ MAX_WS_CONNS × (WS_FRAME_SIZE + 29)                         │  1 082 B │
// ├──────────────────────────────┼──────────────────────────────────────────────────────────────┼──────────┤
// │ SSE (DETWS_ENABLE_SSE=1)     │                                                              │          │
// │  sse_pool[MAX_SSE_CONNS]     │ MAX_SSE_CONNS × (MAX_PATH_LEN + 3)                          │    134 B │
// ├──────────────────────────────┼──────────────────────────────────────────────────────────────┼──────────┤
// │ SSH (DETWS_ENABLE_SSH=1)     │                                                              │          │
// │  ssh_pool[MAX_SSH_CONNS]     │ MAX_SSH_CONNS × (SSH_PKT_BUF_SIZE + 22)                     │  2 070 B │
// │  ssh_keys[MAX_SSH_CONNS]     │ MAX_SSH_CONNS × (2×SshAesCtrCtx + 64)                       │          │
// │   └─ SshAesCtrCtx (native)   │   rk[60]=240 + counter[16] + keystream[16] + pos[1] = 273 B │    610 B │
// │   └─ SshAesCtrCtx (ARDUINO)  │   mbedtls_aes_context (≈284 B) + 33 B = ≈317 B per ctx     │    698 B │
// │  ssh_dh[MAX_SSH_CONNS]       │ MAX_SSH_CONNS × (3×SshBigNum[256] + H[32] + 1)              │    801 B │
// │  crypto_work[]               │ SSH_CRYPTO_WORK_SIZE (scratch, wiped after each use)         │  1 536 B │
// │  SSH SUBTOTAL                │                                                              │  5 017 B │
// ├──────────────────────────────┼──────────────────────────────────────────────────────────────┼──────────┤
// │ GRAND TOTAL (all features)   │                                                              │ ≈18 KB   │
// └──────────────────────────────┴──────────────────────────────────────────────────────────────┴──────────┘
//
// ESP32 has 320 KB of SRAM; the library uses ~5–18 KB depending on features.
// Stack usage is separate; the largest frame is during SSH DH key exchange
// (~256 B for the SshBigNum private scalar on the call stack before it is
// zeroed by ssh_dh_finish()).
//
// SSH KEY MATERIAL IS NOT IN THE TABLE ABOVE intentionally:
//   - The RSA host private key is NEVER stored in any static array.  It is
//     loaded from NVS into a local stack frame at sign time, used once, then
//     explicitly zeroed (volatile memset) before the function returns.
//   - AES session keys and HMAC keys live in ssh_keys[] (above), which is a
//     separate BSS symbol from ssh_pool[].  Physical separation means a
//     buffer overflow in the packet receive path (ssh_pool[].pkt_buf) cannot
//     reach the key material without crossing a distinct linker symbol - a
//     significant barrier against heap/BSS spray attacks.
//   - The DH ephemeral private scalar y lives in ssh_dh[].y and is zeroed
//     immediately after the shared secret K is derived.
//   - crypto_work[] is zeroed via volatile memset after every use so that
//     bignum intermediates (including partial products that contain key
//     material) do not persist in memory.

// ---------------------------------------------------------------------------
// Runtime configuration struct
// ---------------------------------------------------------------------------

/**
 * @brief Runtime-tunable server parameters.
 *
 * Can be declared as `const PROGMEM` (flash) or as a mutable variable (RAM).
 * Pass a pointer to DetWebServer::begin() or DeterministicAsyncTCP::init().
 */
struct WebServerConfig
{
    /** Milliseconds of inactivity before a connection is force-closed. */
    uint32_t conn_timeout_ms;
};

/** @brief Built-in defaults used when no config is supplied to begin(). */
static const WebServerConfig DET_DEFAULT_CONFIG = {5000u};

// ---------------------------------------------------------------------------
// Protocol identifier
// ---------------------------------------------------------------------------

/**
 * @brief Application protocol spoken on a listener port or connection slot.
 *
 * Stored in both Listener::proto and TcpConn::proto.  The session layer uses
 * this to route events to the correct protocol handler without branching on
 * port numbers.
 *
 * All values are always present regardless of feature flags - the enum is
 * part of the listener API.  Feature flags gate the implementation, not the
 * identifier.
 */
enum ConnProto
{
    PROTO_NONE = 0,   ///< Unassigned slot.
    PROTO_HTTP = 1,   ///< HTTP/1.1 with optional WS and SSE upgrades.
    PROTO_TELNET = 2, ///< Telnet (RFC 854).
    PROTO_SSH = 3,    ///< SSH (RFC 4253/4252/4254).
};

/**
 * @brief Network interface a connection arrived on (for per-route filtering).
 *
 * Stamped onto each TcpConn at accept time by comparing the connection's local
 * IP to the softAP IP (see DetWebServer::set_ap_ip()). Used to gate routes to
 * the station or softAP interface only (DetWebServer::on(..., DetIface)).
 */
enum DetIface : uint8_t
{
    DETIFACE_ANY = 0, ///< Unknown / no filter (matches any interface).
    DETIFACE_STA = 1, ///< Station interface (joined to an AP / your LAN).
    DETIFACE_AP = 2,  ///< softAP interface (clients joined to the device).
};

// ---------------------------------------------------------------------------
// Diagnostic JSON string  (only defined when DETWS_ENABLE_DIAG == 1)
// ---------------------------------------------------------------------------
// DETWS_DIAG_JSON is a compile-time string literal - zero runtime cost.
// Adjacent string literals are concatenated by the compiler; DETWS_STR()
// stringifies an integer macro value without evaluating it twice.

#if DETWS_ENABLE_DIAG

#define _DETWS_STR_(x) #x
#define _DETWS_STR(x) _DETWS_STR_(x)

#if DETWS_ENABLE_WEBSOCKET
#define _DETWS_F_WS "true"
#else
#define _DETWS_F_WS "false"
#endif

#if DETWS_ENABLE_SSE
#define _DETWS_F_SSE "true"
#else
#define _DETWS_F_SSE "false"
#endif

#if DETWS_ENABLE_MULTIPART
#define _DETWS_F_MP "true"
#else
#define _DETWS_F_MP "false"
#endif

#if DETWS_ENABLE_FILE_SERVING
#define _DETWS_F_FS "true"
#else
#define _DETWS_F_FS "false"
#endif

#if DETWS_ENABLE_AUTH
#define _DETWS_F_AUTH "true"
#else
#define _DETWS_F_AUTH "false"
#endif

#define DETWS_DIAG_JSON                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 \
    "{"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 \
    "\"lib\":\"DeterministicESPAsyncWebServer\","                                                                                                                                                                                                                                                                                                                                                                                                                                                                       \
    "\"features\":{"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    \
    "\"websocket\":" _DETWS_F_WS ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    \
    "\"sse\":" _DETWS_F_SSE ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         \
    "\"multipart\":" _DETWS_F_MP ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    \
    "\"file_serving\":" _DETWS_F_FS ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 \
    "\"auth\":" _DETWS_F_AUTH "},"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      \
    "\"config\":{"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      \
    "\"MAX_CONNS\":" _DETWS_STR(                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        \
        MAX_CONNS) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  \
                   "\"RX_BUF_SIZE\":" _DETWS_STR(                                                                                                                                                                                                                                                                                                                                                                                                                                                                       \
                       RX_BUF_SIZE) ","                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 \
                                    "\"BODY_BUF_SIZE\":" _DETWS_STR(BODY_BUF_SIZE) ","                                                                                                                                                                                                                                                                                                                                                                                                                                  \
                                                                                   "\"MAX_ROUTES\":" _DETWS_STR(MAX_ROUTES) ","                                                                                                                                                                                                                                                                                                                                                                                         \
                                                                                                                            "\"MAX_"                                                                                                                                                                                                                                                                                                                                                                                    \
                                                                                                                            "HEADERS\""                                                                                                                                                                                                                                                                                                                                                                                 \
                                                                                                                            ":" _DETWS_STR(                                                                                                                                                                                                                                                                                                                                                                             \
                                                                                                                                MAX_HEADERS) ","                                                                                                                                                                                                                                                                                                                                                                        \
                                                                                                                                             "\"MAX_PATH_"                                                                                                                                                                                                                                                                                                                                                              \
                                                                                                                                             "LEN\""                                                                                                                                                                                                                                                                                                                                                                    \
                                                                                                                                             ":" _DETWS_STR(                                                                                                                                                                                                                                                                                                                                                            \
                                                                                                                                                 MAX_PATH_LEN) ","                                                                                                                                                                                                                                                                                                                                                      \
                                                                                                                                                               "\"MAX_KEY_LEN\":" _DETWS_STR(                                                                                                                                                                                                                                                                                                                           \
                                                                                                                                                                   MAX_KEY_LEN) ","                                                                                                                                                                                                                                                                                                                                     \
                                                                                                                                                                                "\"MAX_VAL_LEN\":" _DETWS_STR(MAX_VAL_LEN) ","                                                                                                                                                                                                                                                                                          \
                                                                                                                                                                                                                           "\"MAX_QUERY_LEN\":" _DETWS_STR(MAX_QUERY_LEN) ","                                                                                                                                                                                                                                           \
                                                                                                                                                                                                                                                                          "\"MAX_QUERY_PARAMS\":" _DETWS_STR(MAX_QUERY_PARAMS) ","                                                                                                                                                                                      \
                                                                                                                                                                                                                                                                                                                               "\"CONN_TIMEOUT_MS\":" _DETWS_STR(CONN_TIMEOUT_MS) ","                                                                                                                                   \
                                                                                                                                                                                                                                                                                                                                                                                  "\"RESP_HDR_BUF_SIZE\":" _DETWS_STR(                                                                                                  \
                                                                                                                                                                                                                                                                                                                                                                                      RESP_HDR_BUF_SIZE) ","                                                                                                            \
                                                                                                                                                                                                                                                                                                                                                                                                         "\"WS_HDR_BUF_SIZE\":" _DETWS_STR(                                                                             \
                                                                                                                                                                                                                                                                                                                                                                                                             WS_HDR_BUF_SIZE) ","                                                                                       \
                                                                                                                                                                                                                                                                                                                                                                                                                              "\"CORS_HDR_BUF_SIZE\":" _DETWS_STR(CORS_HDR_BUF_SIZE) ","                                \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     "\"EVT_QUEUE_DEPTH\":" _DETWS_STR( \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         EVT_QUEUE_DEPTH) "}"           \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          "}"

#endif // DETWS_ENABLE_DIAG

// ---------------------------------------------------------------------------
// Compile-time sanity checks
// ---------------------------------------------------------------------------
// These produce a clear #error message in the compiler output rather than a
// cryptic linker failure or silent misbehavior.

#if EVT_QUEUE_DEPTH < MAX_CONNS * 4
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: EVT_QUEUE_DEPTH must be >= MAX_CONNS * 4 to absorb event bursts without blocking lwIP"
#endif

#if MAX_CONNS < 1
#error "DeterministicESPAsyncWebServer: MAX_CONNS must be >= 1"
#endif

#if MAX_CONNS > 255
#error "DeterministicESPAsyncWebServer: MAX_CONNS must be <= 255 (slot IDs are uint8_t)"
#endif

#if DETWS_ENABLE_WEBSOCKET && DETWS_ENABLE_SSE
#if MAX_WS_CONNS + MAX_SSE_CONNS > MAX_CONNS
#error "DeterministicESPAsyncWebServer: MAX_WS_CONNS + MAX_SSE_CONNS must not exceed MAX_CONNS"
#endif
#elif DETWS_ENABLE_WEBSOCKET
#if MAX_WS_CONNS > MAX_CONNS
#error "DeterministicESPAsyncWebServer: MAX_WS_CONNS must not exceed MAX_CONNS"
#endif
#elif DETWS_ENABLE_SSE
#if MAX_SSE_CONNS > MAX_CONNS
#error "DeterministicESPAsyncWebServer: MAX_SSE_CONNS must not exceed MAX_CONNS"
#endif
#endif

#if BODY_BUF_SIZE < 1
#error "DeterministicESPAsyncWebServer: BODY_BUF_SIZE must be >= 1"
#endif

#if BODY_BUF_SIZE > RX_BUF_SIZE
#error "DeterministicESPAsyncWebServer: BODY_BUF_SIZE must not exceed RX_BUF_SIZE (parser reads from the ring buffer)"
#endif

#if DETWS_ENABLE_FILE_SERVING && FILE_CHUNK_SIZE > RX_BUF_SIZE
#error "DeterministicESPAsyncWebServer: FILE_CHUNK_SIZE must not exceed RX_BUF_SIZE"
#endif

#if MAX_KEY_LEN < 4
#error "DeterministicESPAsyncWebServer: MAX_KEY_LEN must be >= 4 (minimum valid HTTP header name length)"
#endif

#if MAX_VAL_LEN < 1
#error "DeterministicESPAsyncWebServer: MAX_VAL_LEN must be >= 1"
#endif

#if MAX_PATH_LEN < 2
#error "DeterministicESPAsyncWebServer: MAX_PATH_LEN must be >= 2 (minimum: \"/\")"
#endif

#if MAX_ROUTES < 1
#error "DeterministicESPAsyncWebServer: MAX_ROUTES must be >= 1"
#endif

#if MAX_MIDDLEWARE < 1
#error "DeterministicESPAsyncWebServer: MAX_MIDDLEWARE must be >= 1"
#endif

#if CHUNK_BUF_SIZE < 16
#error "DeterministicESPAsyncWebServer: CHUNK_BUF_SIZE must be >= 16"
#endif

#if JSON_MAX_DEPTH < 1
#error "DeterministicESPAsyncWebServer: JSON_MAX_DEPTH must be >= 1"
#endif

#if RE_MAX_STEPS < 64
#error "DeterministicESPAsyncWebServer: RE_MAX_STEPS must be >= 64"
#endif

#if DETWS_ENABLE_TLS
#if MAX_TLS_CONNS < 1 || MAX_TLS_CONNS > MAX_CONNS
#error "DeterministicESPAsyncWebServer: MAX_TLS_CONNS must be between 1 and MAX_CONNS"
#endif
#if DETWS_TLS_ARENA_SIZE < 8192
#error "DeterministicESPAsyncWebServer: DETWS_TLS_ARENA_SIZE is far too small for a TLS handshake"
#endif
#endif

#if DETWS_ENABLE_SNMP
#if SNMP_MAX_OID_LEN < 4
#error "DeterministicESPAsyncWebServer: SNMP_MAX_OID_LEN must be >= 4"
#endif
#if SNMP_MAX_MIB_ENTRIES < 1
#error "DeterministicESPAsyncWebServer: SNMP_MAX_MIB_ENTRIES must be >= 1"
#endif
#if SNMP_MAX_VARBINDS < 1
#error "DeterministicESPAsyncWebServer: SNMP_MAX_VARBINDS must be >= 1"
#endif
#if SNMP_MSG_BUF_SIZE < 484
#error "DeterministicESPAsyncWebServer: SNMP_MSG_BUF_SIZE must be >= 484 (RFC 1157 minimum)"
#endif
#endif

#if DETWS_ENABLE_COAP
#if DETWS_COAP_MAX_RESOURCES < 1
#error "DeterministicESPAsyncWebServer: DETWS_COAP_MAX_RESOURCES must be >= 1"
#endif
#if DETWS_COAP_MAX_PATH < 2
#error "DeterministicESPAsyncWebServer: DETWS_COAP_MAX_PATH must be >= 2 (minimum: \"/\")"
#endif
#if DETWS_COAP_MAX_PAYLOAD < 1
#error "DeterministicESPAsyncWebServer: DETWS_COAP_MAX_PAYLOAD must be >= 1"
#endif
#if DETWS_COAP_MSG_BUF_SIZE < (DETWS_COAP_MAX_PAYLOAD + 16)
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: DETWS_COAP_MSG_BUF_SIZE must be >= DETWS_COAP_MAX_PAYLOAD + 16 (header + token + Content-Format option + payload marker)"
#endif
#endif

#if DETWS_ENABLE_AUTH && MAX_AUTH_LEN < 2
#error "DeterministicESPAsyncWebServer: MAX_AUTH_LEN must be >= 2 when DETWS_ENABLE_AUTH is set"
#endif

#if DETWS_ENABLE_PER_IP_THROTTLE
#if DETWS_PER_IP_THROTTLE_SLOTS < 1
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: DETWS_PER_IP_THROTTLE_SLOTS must be >= 1 when DETWS_ENABLE_PER_IP_THROTTLE is set"
#endif
#if DETWS_PER_IP_THROTTLE_MAX < 1
#error "DeterministicESPAsyncWebServer: DETWS_PER_IP_THROTTLE_MAX must be >= 1 when DETWS_ENABLE_PER_IP_THROTTLE is set"
#endif
#endif

#if DETWS_ENABLE_WEBDAV
#if !DETWS_ENABLE_FILE_SERVING
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_WEBDAV requires DETWS_ENABLE_FILE_SERVING"
#endif
#if DETWS_WEBDAV_BUF_SIZE < 256
#error "DeterministicESPAsyncWebServer: DETWS_WEBDAV_BUF_SIZE must be >= 256"
#endif
#if DETWS_METHOD_BUF_SIZE < 10
#error "DeterministicESPAsyncWebServer: DETWS_METHOD_BUF_SIZE must be >= 10 when DETWS_ENABLE_WEBDAV is set (PROPPATCH)"
#endif
#endif

#if DETWS_ENABLE_KEEPALIVE && DETWS_KEEPALIVE_MAX_REQUESTS < 1
#error "DeterministicESPAsyncWebServer: DETWS_KEEPALIVE_MAX_REQUESTS must be >= 1 when DETWS_ENABLE_KEEPALIVE is set"
#endif

#if DETWS_ENABLE_RANGE && !DETWS_ENABLE_FILE_SERVING
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_RANGE requires DETWS_ENABLE_FILE_SERVING"
#endif

#if DETWS_ENABLE_MTLS && !DETWS_ENABLE_TLS
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_MTLS requires DETWS_ENABLE_TLS"
#endif

#if DETWS_ENABLE_METRICS && !DETWS_ENABLE_STATS
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_METRICS requires DETWS_ENABLE_STATS"
#endif

#if DETWS_ENABLE_HTTP_CLIENT_TLS && !DETWS_ENABLE_TLS
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_HTTP_CLIENT_TLS requires DETWS_ENABLE_TLS"
#endif

#if DETWS_ENABLE_MQTT_TLS && !DETWS_ENABLE_TLS
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_MQTT_TLS requires DETWS_ENABLE_TLS"
#endif

#if DETWS_ENABLE_WS_CLIENT_TLS && !DETWS_ENABLE_TLS
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_WS_CLIENT_TLS requires DETWS_ENABLE_TLS"
#endif

#if DETWS_ENABLE_SNMP_TRAP && !DETWS_ENABLE_SNMP
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_SNMP_TRAP requires DETWS_ENABLE_SNMP"
#endif

#if DETWS_ENABLE_COAP_OBSERVE && !DETWS_ENABLE_COAP
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_COAP_OBSERVE requires DETWS_ENABLE_COAP"
#endif

#if DETWS_ENABLE_COAP_BLOCK
#if !DETWS_ENABLE_COAP
#error "DeterministicESPAsyncWebServer: DETWS_ENABLE_COAP_BLOCK requires DETWS_ENABLE_COAP"
#endif
#if DETWS_COAP_BLOCK_SZX_MAX > 6
#error "DeterministicESPAsyncWebServer: DETWS_COAP_BLOCK_SZX_MAX must be <= 6 (block size 2^(SZX+4); SZX 7 is reserved)"
#endif
#if DETWS_COAP_MSG_BUF_SIZE < ((1 << (DETWS_COAP_BLOCK_SZX_MAX + 4)) + 16)
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: DETWS_COAP_MSG_BUF_SIZE must hold one full block (2^(DETWS_COAP_BLOCK_SZX_MAX+4)) + 16 header/option bytes"
#endif
#if DETWS_COAP_BLOCK1_MAX < (1 << (DETWS_COAP_BLOCK_SZX_MAX + 4))
#error "DeterministicESPAsyncWebServer: DETWS_COAP_BLOCK1_MAX must be >= one block (2^(DETWS_COAP_BLOCK_SZX_MAX+4))"
#endif
#endif

#if DETWS_ENABLE_WEBSOCKET && WS_FRAME_SIZE < 2
#error "DeterministicESPAsyncWebServer: WS_FRAME_SIZE must be >= 2 when DETWS_ENABLE_WEBSOCKET is set"
#endif

#if DETWS_ENABLE_SSE && SSE_BUF_SIZE < 8
#error "DeterministicESPAsyncWebServer: SSE_BUF_SIZE must be >= 8 when DETWS_ENABLE_SSE is set"
#endif

#if DETWS_ENABLE_MULTIPART && MAX_MULTIPART_PARTS < 1
#error "DeterministicESPAsyncWebServer: MAX_MULTIPART_PARTS must be >= 1 when DETWS_ENABLE_MULTIPART is set"
#endif

#if RESP_HDR_BUF_SIZE < 128
#error "DeterministicESPAsyncWebServer: RESP_HDR_BUF_SIZE must be >= 128 (status line + headers + CORS block)"
#endif

#if DETWS_ENABLE_WEBSOCKET && WS_HDR_BUF_SIZE < 128
#error "DeterministicESPAsyncWebServer: WS_HDR_BUF_SIZE must be >= 128 when DETWS_ENABLE_WEBSOCKET is set"
#endif

#if CORS_HDR_BUF_SIZE < 64
#error "DeterministicESPAsyncWebServer: CORS_HDR_BUF_SIZE must be >= 64"
#endif

#if RESP_HDR_BUF_SIZE < (CORS_HDR_BUF_SIZE + EXTRA_HDR_BUF_SIZE + 96)
#error                                                                                                                 \
    "DeterministicESPAsyncWebServer: RESP_HDR_BUF_SIZE must be >= CORS_HDR_BUF_SIZE + EXTRA_HDR_BUF_SIZE + 96 (status line + CORS block + custom-header block are injected into response headers)"
#endif

#endif
