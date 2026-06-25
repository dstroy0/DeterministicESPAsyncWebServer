# RFC Compliance

This document details the standards conformance of DeterministicESPAsyncWebServer's
HTTP/1.1 (request parsing, response generation, keep-alive, Range), authentication
(Basic / Digest / JWT), WebSocket, TLS / mTLS, CoAP, SNMP, syslog, the outbound
HTTP client, and error-handling behavior. (SSH conformance lives in [SSH.md](SSH.md);
the TLS security posture is in [SECURITY.md](SECURITY.md).)

## HTTP/1.1 request parsing (RFC 7230)

The parser enforces these rules byte-by-byte during parsing:

<details>
<summary><b>HTTP/1.1 Parsing Conformance Table</b></summary>

| Field              | Allowed characters                                                   | RFC reference | Violation response                  |
| ------------------ | -------------------------------------------------------------------- | ------------- | ----------------------------------- |
| Method             | `tchar` (`ALPHA DIGIT ! # $ % & ' * + - . ^ _ \` \| ~`)              | §3.1.1        | 400                                 |
| Path / Query       | `VCHAR` (%x21–7E)                                                    | RFC 3986 §3.3 | 400                                 |
| Header field-name  | `tchar`                                                              | §3.2          | 400                                 |
| Header field-value | `VCHAR`, SP, HTAB, obs-text (%x80–FF)                                | §3.2          | 400                                 |
| Path length        | ≤ `MAX_PATH_LEN − 1` bytes                                           | §3.1.1        | 414                                 |
| Body size          | ≤ [`BODY_BUF_SIZE`](@ref BODY_BUF_SIZE) bytes (via `Content-Length`) | §3.3.2        | 413                                 |
| Content-Length     | Must be `1*DIGIT`; conflicting duplicates rejected                   | §3.3.2        | 400                                 |
| Host header        | Required for HTTP/1.1; more than one always rejected                 | §5.4          | 400                                 |
| Transfer-Encoding  | Not supported - rejected at dispatch                                 | §3.3.1        | 501                                 |
| HTTP version       | FNV-1a hash match; sets [`HttpReq::version`](@ref HttpReq::version)  | §2.6          | [`HTTP_UNKNOWN`](@ref HTTP_UNKNOWN) |

</details>

Additional behaviors:

- CR mid header field-name → 400
- Leading SP/HTAB in header values stripped per OWS rules (§3.2.3)
- Excess headers beyond [`MAX_HEADERS`](@ref MAX_HEADERS) are consumed and discarded, not rejected
- Query string overflow silently truncates (capacity limit, not a protocol error)
- Host enforcement is governed by [`DETWS_ENFORCE_HOST_HEADER`](@ref DETWS_ENFORCE_HOST_HEADER) (default `1`); set to
  `0` to accept HTTP/1.1 requests without a Host header. The "more than one Host"
  and Content-Length rules are always active. `Host` detection is independent of
  the `MAX_HEADERS` storage cap.

## HTTP/1.1 response generation (RFC 7230 §3.3, §4.1)

By default every response closes the connection (`Connection: close`,
HTTP/1.0-style). With optional **HTTP keep-alive**
([`DETWS_ENABLE_KEEPALIVE`](@ref DETWS_ENABLE_KEEPALIVE), default off) a
cleanly-parsed request is instead answered `Connection: keep-alive` and the
connection is reused for the next request (persistent connections, RFC 7230
§6.3): HTTP/1.1 persists unless the client sends `Connection: close`, HTTP/1.0
closes unless it sends `Connection: keep-alive`, and any error or non-complete
parse (400/413/414) always closes since the next request boundary is unknown.
Each connection serves at most [`DETWS_KEEPALIVE_MAX_REQUESTS`](@ref DETWS_KEEPALIVE_MAX_REQUESTS)
requests (a fairness bound) and idle ones are still reclaimed by the timeout
sweep; pipelined requests already in the buffer are drained in order.

<details>
<summary><b>Response Conformance Table</b></summary>

| Behavior                                  | RFC reference | Notes                                                                                                                                                                                       |
| ----------------------------------------- | ------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `Content-Length` on fixed-size responses  | 7230 §3.3.2   | [`send()`](@ref DetWebServer::send) / [`send_empty()`](@ref DetWebServer::send_empty) / [`redirect()`](@ref DetWebServer::redirect) / [`send_template()`](@ref DetWebServer::send_template) |
| Chunked transfer-encoding (streamed body) | 7230 §4.1     | [`send_chunked()`](@ref DetWebServer::send_chunked): `Transfer-Encoding: chunked`, `<hexlen>\r\n<data>\r\n` chunks, `0\r\n\r\n` terminator                                                  |
| `HEAD` suppresses body, keeps headers     | 7231 §4.3.2   | applies to chunked too (the `Transfer-Encoding` header is sent, but no chunks)                                                                                                              |

</details>

Note the deliberate asymmetry: an inbound **request** carrying `Transfer-Encoding`
is **rejected** (501, §3.3.1 - the server does not de-chunk request bodies),
whereas an outbound **response** may use chunked transfer via
[`send_chunked()`](@ref DetWebServer::send_chunked).

## HTTP authentication (RFC 7235)

A route registered with credentials challenges unauthenticated requests with
`401 Unauthorized` + `WWW-Authenticate` and `Connection: close` (so a client gets
exactly one guess per TCP connection - a built-in brute-force bound).

<details>
<summary><b>Authentication Conformance Table</b></summary>

| Scheme | RFC  | Challenge / verification                                                                                                                                                                                                                 |
| ------ | ---- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Basic  | 7617 | `WWW-Authenticate: Basic realm="…"`; the base64 `user:pass` is decoded with an output-capacity guard before comparison.                                                                                                                  |
| Digest | 7616 | `Digest` with `algorithm=SHA-256`, `qop="auth"`. `HA1=SHA256(user:realm:pass)`, `HA2=SHA256(method:uri)`, `response=SHA256(HA1:nonce:nc:cnonce:qop:HA2)`. The server nonce is regenerated each `begin()` from the ESP32 hardware CSPRNG. |

</details>

Digest nonce-count (`nc`) replay tracking is not implemented (it needs per-client
state that conflicts with the single shared server nonce on a 1-2 client device);
the per-`begin()` nonce rotation bounds the replay window.

## WebSocket framing (RFC 6455)

<details>
<summary><b>WebSocket Framing Conformance Table</b></summary>

| Rule                                            | Section | Behavior                                    |
| ----------------------------------------------- | ------- | ------------------------------------------- |
| Client→server frames must be masked             | §5.1    | Unmasked frame → Close 1002, fail           |
| Reserved opcodes rejected                       | §5.2    | Opcode ∉ {0,1,2,8,9,A} → Close 1002         |
| RSV1–3 must be zero                             | §5.2    | Any RSV bit set → Close 1002                |
| Control frames ≤ 125 bytes                      | §5.5    | Oversized control frame → Close 1002        |
| Control frames must not be fragmented           | §5.5    | Control frame with FIN=0 → Close 1002       |
| Payload ≤ [`WS_FRAME_SIZE`](@ref WS_FRAME_SIZE) | §5.2    | Oversized / 64-bit length → Close 1009      |
| Handshake version negotiation                   | §4.2.1  | Missing/≠ `13` → 426 with supported version |

</details>

Fragmented data messages (continuation frames, §5.4) are reassembled into the
per-connection buffer and delivered once the FIN frame arrives; control frames
may be interleaved between fragments. The reassembled message must fit in
`WS_FRAME_SIZE` (else Close 1009).

## Transport security - TLS (RFC 5246 / 8446)

Optional HTTPS ([`DETWS_ENABLE_TLS`](@ref DETWS_ENABLE_TLS), default off) via
mbedTLS on a static memory pool (no heap). TLS 1.2 (RFC 5246) is the negotiated
minimum; TLS 1.3 (RFC 8446) is used when the client offers it. The verified
cipher suite is `ECDHE-ECDSA-AES256-GCM-SHA384` (forward-secret ECDHE, ECDSA
authentication, AEAD AES-256-GCM). Server certificate/key are loaded via
[`begin_tls()`](@ref DetWebServer::begin_tls) / [`tls_cert()`](@ref DetWebServer::tls_cert).
`wss://` and TLS Server-Sent Events run over the same TLS record layer when TLS is
enabled: the WebSocket upgrade and every subsequent frame/event are encrypted,
transparent to handler code. Optional **mutual TLS**
([`DETWS_ENABLE_MTLS`](@ref DETWS_ENABLE_MTLS)) requires and verifies a client
certificate chaining to a configured CA (RFC 5246 §7.4.6 / RFC 8446 §4.4.2) and
exposes the verified peer subject DN to handlers. Full properties and caveats:
[SECURITY.md](SECURITY.md) §6.

## SNMP agent (RFC 1157 / 1901 / 3416 / 2578 / 1213)

Optional SNMP agent ([`DETWS_ENABLE_SNMP`](@ref DETWS_ENABLE_SNMP), default off)
on UDP port 161 (via the transport-layer UDP service), with a zero-heap ASN.1 BER codec and a
fixed MIB table. Conformance:

- **Message format (RFC 1157 §3 / RFC 1901):** `SNMPv1` (version 0) and
  `SNMPv2c` (version 1) community-based messages: `SEQUENCE { version, community,
  PDU }`. A request whose community matches neither the read-only nor the
  read-write community is silently discarded.
- **SNMPv3 / USM (RFC 3412 / 3414, optional `DETWS_ENABLE_SNMP_V3`):** the v3
  message format (msgGlobalData + msgSecurityParameters + scopedPDU), engine
  discovery (Report `usmStatsUnknownEngineIDs`), the timeliness window
  (engineBoots / engineTime), authentication `usmHMAC192SHA256` (HMAC-SHA-256,
  RFC 7860) and privacy `usmAesCfb128` (AES-128-CFB, RFC 3826), with key
  localization per RFC 3414 §2.6. The authenticated, decrypted inner PDU is
  dispatched through the same MIB core as v1/v2c.
- **ASN.1 BER (X.690 / RFC 2578 types):** definite-length TLV encoding of
  `INTEGER`, `OCTET STRING`, `NULL`, `OBJECT IDENTIFIER` (first two arcs packed as
  `40·a+b`, sub-identifiers in base-128), `SEQUENCE`, and the SMI application types
  `Counter32` / `Gauge32` / `TimeTicks` / `IpAddress` / `Opaque`. Integers use the
  minimal two's-complement form; unsigned application types add a leading `0x00`
  when the high bit is set.
- **PDU operations (RFC 1157 §4 / RFC 3416 §4):** `GetRequest`, `GetNextRequest`,
  `GetBulkRequest` (v2c), and `SetRequest`, each answered with a `GetResponse`
  echoing the request id. v2c retrieval reports per-varbind exceptions
  (`noSuchObject` / `endOfMibView`); v1 reports `error-status` / `error-index`
  (`noSuchName`). `Set` is authorized by the read-write community and uses
  `error-status` (`noAccess` / `notWritable` / `wrongType`) in both versions.
  `GetBulk` honors `non-repeaters` / `max-repetitions`, clamped so the response
  never exceeds `SNMP_MAX_VARBINDS`; an over-large response degrades to `tooBig`.
- **MIB (RFC 1213):** the standard `system` group (`1.3.6.1.2.1.1`):
  `sysDescr`, `sysObjectID`, `sysUpTime` (`TimeTicks`), `sysContact`, `sysName`,
  `sysLocation`, `sysServices`: plus any application objects registered under a
  private enterprise subtree. `GetNext` / `GetBulk` walk objects in lexicographic
  OID order.

The decode/dispatch/encode core ([`snmp_agent_process()`](@ref snmp_agent_process))
is transport-independent and host-tested; only the UDP socket is ESP32-specific.
SNMP security properties (cleartext community strings, no v1/v2c authentication):
[SECURITY.md](SECURITY.md).

## HTTP Range requests (RFC 7233)

Optional ([`DETWS_ENABLE_RANGE`](@ref DETWS_ENABLE_RANGE), requires file serving,
default off). Served files advertise `Accept-Ranges: bytes`. A single-range
`Range: bytes=A-B`, `bytes=A-`, or `bytes=-N` request is answered `206 Partial
Content` with `Content-Range: bytes A-B/size`, seeking the file and streaming only
the requested bytes (resumable downloads, media seeking). An unsatisfiable range
yields `416 Range Not Satisfiable` with `Content-Range: bytes */size`. A
multi-range (comma-separated) request degrades to a full `200` response, which
RFC 7233 §3.1 explicitly permits, as does a malformed or absent `Range`.

## CoAP server (RFC 7252)

Optional ([`DETWS_ENABLE_COAP`](@ref DETWS_ENABLE_COAP), default off) zero-heap
Constrained Application Protocol endpoint on UDP/5683. Message layer: a
Confirmable (CON) request is answered with a piggybacked ACK, a Non-confirmable
(NON) request with a NON response, and a malformed or empty CON with a Reset
(RST). The parser handles the 4-byte header + token (≤ 8 bytes), delta-encoded
options (Uri-Path, Uri-Query, Content-Format), and the `0xFF` payload marker, then
dispatches GET/POST/PUT/DELETE against a fixed resource table by reconstructed
Uri-Path. The codec ([`coap_server_process()`](@ref coap_server_process)) is
transport-independent and host-tested; only the UDP socket is ESP32-specific.

## Remote logging - syslog (RFC 5424)

Optional ([`DETWS_ENABLE_SYSLOG`](@ref DETWS_ENABLE_SYSLOG), default off) syslog
client over UDP. Each line is `<PRI>1 TIMESTAMP HOSTNAME APP-NAME PROCID MSGID
STRUCTURED-DATA MSG`, where `PRI = facility*8 + severity` and TIMESTAMP / PROCID /
MSGID / STRUCTURED-DATA are the NILVALUE `-` (the device has no wall clock or PID).
Fire-and-forget; the formatter ([`syslog_format()`](@ref syslog_format)) is
host-tested and the datagram is sent via the transport-layer UDP service.

## JWT bearer tokens (RFC 7519 / 7515 / 7518)

Optional ([`DETWS_ENABLE_JWT`](@ref DETWS_ENABLE_JWT), default off). Verifies a
compact-serialization JWS (RFC 7515) JSON Web Token (RFC 7519):
`base64url(header).base64url(payload).base64url(signature)` with `alg=HS256`
(HMAC-SHA-256, RFC 7518). The signature is recomputed over `header.payload` and
compared in constant time ([`jwt_bearer_valid()`](@ref jwt_bearer_valid)); integer
claims such as `exp` are readable via [`jwt_claim_int()`](@ref jwt_claim_int) for
the handler to enforce. The full `Authorization` header is captured (a bearer
token exceeds the normal header-value cap). Shared-secret caveat:
[SECURITY.md](SECURITY.md).

## Outbound HTTP(S) client (RFC 7230)

Optional ([`DETWS_ENABLE_HTTP_CLIENT`](@ref DETWS_ENABLE_HTTP_CLIENT), default
off). Builds RFC 7230 request messages ([`http_get()`](@ref http_get) /
[`http_post()`](@ref http_post)) and parses responses framed by `Content-Length`
or `Transfer-Encoding: chunked` (decoded in place) or by connection close.
`https://` runs over client-side mbedTLS
([`DETWS_ENABLE_HTTP_CLIENT_TLS`](@ref DETWS_ENABLE_HTTP_CLIENT_TLS)); the server
certificate is not verified (see [SECURITY.md](SECURITY.md)).

## Automatic error responses

[`handle()`](@ref DetWebServer::handle) sends these before dispatching to any route handler:

<details>
<summary><b>Parser State Errors Table</b></summary>

| Parser state                                            | Response              | Trigger                                            |
| ------------------------------------------------------- | --------------------- | -------------------------------------------------- |
| [`PARSE_ERROR`](@ref PARSE_ERROR)                       | 400 Bad Request       | Any RFC 7230 character violation or malformed CRLF |
| [`PARSE_ENTITY_TOO_LARGE`](@ref PARSE_ENTITY_TOO_LARGE) | 413 Payload Too Large | `Content-Length` > `BODY_BUF_SIZE`                 |
| [`PARSE_URI_TOO_LONG`](@ref PARSE_URI_TOO_LONG)         | 414 URI Too Long      | Path exceeds `MAX_PATH_LEN − 1` bytes              |

</details>

`handle()` also sends these during dispatch:

<details>
<summary><b>Dispatch Condition Errors Table</b></summary>

| Condition                                    | Response                                     | RFC reference |
| -------------------------------------------- | -------------------------------------------- | ------------- |
| `Transfer-Encoding` header present           | 501 Not Implemented                          | 7230 §3.3.1   |
| Unrecognized request method                  | 501 Not Implemented                          | 7231 §6.5.2   |
| Path matches a route but the method does not | 405 Method Not Allowed (with `Allow` header) | 7231 §6.5.5   |
| No matching route, no `on_not_found` handler | 404 Not Found                                | 7231 §6.5.4   |
| WebSocket upgrade on a non-WS route          | 400 Bad Request                              | 6455 §4.2.1   |
| Unsupported `Sec-WebSocket-Version`          | 426 Upgrade Required                         | 6455 §4.2.1   |
| WebSocket or SSE pool full                   | 503 Service Unavailable                      | -             |

</details>

A `HEAD` request is served by the matching `GET` route with the body suppressed
(RFC 7231 §4.3.2); `GET` routes advertise `HEAD` in the `Allow` header.
