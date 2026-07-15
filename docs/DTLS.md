# DTLS 1.3 (RFC 9147)

`DETWS_ENABLE_DTLS`

DTLS is TLS for **datagrams**: it gives UDP the same confidentiality, integrity, and
authentication that TLS gives TCP, without imposing a reliable, ordered byte stream. That
matters for constrained devices, where the natural transport is a single UDP exchange -
**CoAP** sensor reads, telemetry, LwM2M - and a TCP + TLS session would be pure overhead.

DTLS 1.3 (RFC 9147) reuses the TLS 1.3 handshake and key schedule wholesale and changes only
what datagrams require: a compact record header, per-record sequence numbers that survive
reordering and loss, encryption of those sequence numbers, and its own reliability (records can
be dropped, duplicated, or reordered, so the handshake carries acknowledgements and
retransmits). This server builds DTLS 1.3 on the same hand-rolled TLS 1.3 crypto that backs
HTTP/3 - no second TLS stack.

> **Status.** Two layers are implemented: the **record layer** (RFC 9147 §4), which protects and
> unprotects individual datagrams once keys exist, and the datagram **handshake framing and
> reliability** (RFC 9147 §5, §7) - the 12-byte handshake header, overlap-tolerant message
> reassembly, the ACK message, and the stateless return-routability cookie. The **handshake state
> machine** that drives these (flights, epoch transitions, PTO retransmission) and a
> **CoAP-over-DTLS** front-end are the following phases. Each layer is a complete, independently
> tested unit that the next builds on.

## The record layer

A DTLS 1.3 record comes in two shapes (RFC 9147 §4):

- **DTLSPlaintext** - the classic 13-byte header (`type · legacy_version · epoch ·
48-bit sequence_number · length · fragment`), used unencrypted for the very first handshake
  flight and for alerts in epoch 0.
- **DTLSCiphertext** - the compact **unified header** plus an AEAD-sealed body, used once record
  keys exist. The first byte packs three fixed bits (`001`) and four flags (connection-id
  present, 8- vs 16-bit sequence number, length present, and the low two bits of the epoch), so
  a protected record can be as small as its payload plus a few bytes of header and a 16-byte tag.

Protecting a record:

1. The **inner plaintext** is the payload followed by its true content type (and optional zero
   padding), exactly like TLS 1.3 (RFC 8446 §5.2).
2. It is sealed with **AEAD_AES_128_GCM**. The nonce is the 96-bit write IV XOR the 64-bit
   record sequence number (RFC 9147 §4.2.2 - the epoch is _not_ mixed in), and the associated
   data is the unified header carrying the _plaintext_ sequence number.
3. The **sequence number is then encrypted** (RFC 9147 §4.2.3): a mask is taken from
   `AES-ECB(sn_key, ciphertext[0..15])` and XORed into the on-wire sequence-number bytes. This
   is the same construction as QUIC header protection, so it shares that code. A receiver
   reverses it, then reconstructs the full sequence number as the value nearest the one it
   expects next (RFC 9000 Appendix A.3).

The record keys - the AEAD `key`, the write `iv`, and the sequence-number `sn` key - are each an
`HKDF-Expand-Label` of a TLS 1.3 traffic secret (RFC 8446 §7.3, RFC 9147 §4.2.3).

An **anti-replay sliding window** (RFC 9147 §4.5.1) tracks the highest sequence number accepted
in an epoch and a 64-record bitmap behind it, so a replayed or too-old record is rejected before
it reaches the application.

## The handshake framing

TLS 1.3 assumes a reliable, ordered byte stream; DTLS carries the same handshake messages over
datagrams that can be lost, reordered, or duplicated. The framing layer bridges that gap so the
reused TLS 1.3 message builders never have to know they are on UDP.

- **The 12-byte handshake header** (RFC 9147 §5.2) wraps each message body with a `message_seq`
  and a `fragment_offset` / `fragment_length`, so one message can be split across records and
  each fragment placed independently of the datagram that carried it.
- **Reassembly** collects a message's fragments into a contiguous body. Fragments may arrive out
  of order, duplicated, or **overlapping** (§5.4 requires handling overlap - a sender that
  lowers its estimate of the path MTU may retransmit with smaller, differently-cut fragments);
  received byte ranges are merged into a bounded interval list, and the message is complete once
  a single interval covers it. The bound caps the work a maximally-fragmented flight can force.
- **The ACK message** (RFC 9147 §7, content type 26) is a list of `(epoch, sequence_number)`
  record numbers. DTLS acknowledges records rather than relying on TCP, so a receiver can tell
  the sender which fragments of a flight arrived and the sender can drop them from its next
  retransmission.
- **The HelloRetryRequest cookie** (RFC 9147 §5.1) is the return-routability / anti-amplification
  defence. Before committing handshake state, the server can answer the first ClientHello with a
  cookie the client must echo. The cookie is **stateless**: it carries an
  `HMAC-SHA256(server_secret, version · timestamp · client_address · payload)`, so the client's
  address is authenticated (a cookie minted for one peer is worthless to another) without the
  server storing anything, and a timestamp bounds its lifetime. The opaque payload is where the
  state machine parks what it needs to resume the handshake after the retry.

## What is verified

The whole record - header layout, AEAD nonce, associated-data selection, and sequence-number
encryption - is pinned **byte-for-byte** to an _independent_ reconstruction: HKDF-Expand-Label
from stdlib HMAC-SHA256, and AES-128-GCM + AES-ECB from a separate crypto library. A byte-exact
match proves every field is assembled the way the RFC specifies. Round-trips across payload
sizes and content types, sequence-number reconstruction across a 16-bit rollover, tampered-tag
and wrong-epoch rejection, and the replay window are all host-tested (`native_dtls`).

The framing layer is host-tested separately (`native_dtls_hs`): the handshake header round-trips
and rejects malformed fragments; reassembly is exercised in order, out of order, and with
overlapping and duplicate fragments, and its bounds are checked; the ACK message round-trips and
rejects malformed input. The cookie's wire format is pinned **byte-for-byte** to an independent
Python-stdlib HMAC-SHA256, and verification is shown to reject a wrong client address, a tampered
byte, a truncated cookie, and a stale or future-dated timestamp.

## Standards

| Area                           | Standard                  | Status                                                                          |
| ------------------------------ | ------------------------- | ------------------------------------------------------------------------------- |
| DTLSPlaintext / DTLSCiphertext | RFC 9147 §4               | Implemented - unified header build/parse, DTLSPlaintext build/parse             |
| Record AEAD (AEAD_AES_128_GCM) | RFC 9147 §4.2, SP 800-38D | Implemented - TLS 1.3 nonce (§4.2.2), header-as-AAD; reuses `quic_aead`         |
| Sequence-number encryption     | RFC 9147 §4.2.3           | Implemented - `AES-ECB(sn_key, ct[0..15])` mask; `sn` key via HKDF-Expand-Label |
| Sequence-number reconstruction | RFC 9147 §4.2.2           | Implemented - closest-to-expected (RFC 9000 App. A.3)                           |
| Anti-replay window             | RFC 9147 §4.5.1           | Implemented - 64-record sliding window                                          |
| Record-key derivation          | RFC 8446 §7.3, RFC 5869   | Implemented - `key` / `iv` / `sn` = HKDF-Expand-Label(traffic secret)           |
| Handshake header + reassembly  | RFC 9147 §5.2, §5.4       | Implemented - 12-byte header build/parse, overlap-tolerant reassembly           |
| ACK message                    | RFC 9147 §7               | Implemented - content type 26, record-number list build/parse                   |
| HelloRetryRequest cookie       | RFC 9147 §5.1             | Implemented - stateless HMAC-SHA256 cookie binding the client address           |
| Handshake state machine        | RFC 9147 §5-6             | **Roadmap** - flights, epoch transitions, PTO retransmission                    |
| Connection ID                  | RFC 9147 §9               | **Roadmap** - negotiated via extension; CID records are rejected for now        |

The handshake DTLS carries is the TLS 1.3 already documented for HTTP/3
(`TLS_AES_128_GCM_SHA256` + X25519 + Ed25519); see [SSH.md](SSH.md) for the shared crypto
primitives and [STANDARDS.md](STANDARDS.md) for the full RFC map.
