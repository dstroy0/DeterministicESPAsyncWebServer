# RFC Compliance

This document details the standards conformance of DeterministicESPAsyncWebServer's
HTTP/1.1, WebSocket, and error-handling behavior. (SSH conformance lives in
[SSH.md](SSH.md).)

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
