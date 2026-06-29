# Feature reference

Every optional feature is a compile-time flag (default off unless noted); enable it in `platformio.ini` `build_flags` or in `DetWebServerConfig.h`. The feature table in the [README](../README.md#features) links to each entry here, and the description shows on hover. Core HTTP/1.1 parsing, routing, the middleware pipeline, JSON, templating, and chunked responses are always on.

## Accept Throttle

`DETWS_ENABLE_ACCEPT_THROTTLE`

Opt-in global accept-rate throttle (connection-flood defense). Default off (zero cost / no behavior change). When set to 1 the accept callback rejects new connections once more than DETWS_ACCEPT_THROTTLE_MAX have been accepted within a DETWS_ACCEPT_THROTTLE_WINDOW_MS fixed window (global across all listeners, two static counters - no per-IP table). This bounds connection churn (e.g. reconnect brute-force) on top of the bounded connection pool and the per-connection auth limits. mitigate finer-grained / per-IP attacks at the network layer.

## AMQP

`DETWS_ENABLE_AMQP`

AMQP 0-9-1 frame codec - the RabbitMQ wire protocol. Default off. services/amqp lets a device be an AMQP client over the outbound client transport: `amqp_protocol_header` writes the `"AMQP" 0 0 9 1` preamble, `amqp_build_frame` / `amqp_parse_frame` build and validate a frame (type + channel + 4-octet size + payload + the mandatory 0xCE frame-end), `amqp_build_method` / `amqp_parse_method` handle a METHOD frame's class-id / method-id / arguments, and `amqp_build_heartbeat` emits a keep-alive. Pure and host-tested; the method-argument field encoding and the connection state are the application's. See src/services/amqp/amqp.h.

## Audit Log

`DETWS_ENABLE_AUDIT_LOG`

Tamper-evident audit log. Default off. services/audit_log keeps an append-only, hash-chained security log: each record carries SHA-256(prev_hash || fields), so altering, deleting, or reordering any retained record breaks the chain (detws_audit_verify() detects it). Storage is a fixed RAM ring of DETWS_AUDIT_LOG_ENTRIES records (no heap); when it wraps, a moving anchor keeps the retained window verifiable. Install a sink (detws_audit_set_sink) to forward every record at creation time to a durable / remote store - SD-card file, syslog or HTTP log service, serial console - preserving the same chain off-device. Pure and host-tested.

## Auth

`DETWS_ENABLE_AUTH`

HTTP Basic Authentication per-route.

## Auth Lockout

`DETWS_ENABLE_AUTH_LOCKOUT`

Opt-in per-IP brute-force lockout for HTTP auth (requires AUTH). Default off (zero cost / no behavior change). When set, the auth gate counts consecutive failed authentications per source IPv4 in a fixed BSS table; after DETWS_AUTH_LOCKOUT_THRESHOLD failures the address is locked out for DETWS_AUTH_LOCKOUT_BASE_MS, doubling on each further failure up to DETWS_AUTH_LOCKOUT_MAX_MS. A locked address gets 429 (Retry-After) with no credential check; a successful auth clears it. Bounded memory (no heap); the table evicts idle, then least-recently-used, addresses when full.

## BACnet

`DETWS_ENABLE_BACNET`

BACnet/IP BVLC + NPDU codec - the ASHRAE 135 building-automation network framing over UDP (47808). Default off. services/bacnet provides `bvlc_build` / `bvlc_parse` for the BACnet/IP virtual-link envelope (type 0x81, function such as Original-Unicast-NPDU 0x0A / Original-Broadcast-NPDU 0x0B, 2-octet length) and `npdu_build` / `npdu_parse` for the network layer (version 0x01 + the NPCI control octet + optional DNET/DLEN/DADR destination addressing + hop count), slicing out the APDU. Layout verified against ASHRAE 135 Annex J / Clause 6; pure and host-tested. The APDU application layer (objects / properties / services) layers on top. See src/services/bacnet/bacnet.h.

## C37.118

`DETWS_ENABLE_C37118`

IEEE C37.118.2 synchrophasor frame codec. Default off. services/c37118 is a zero-heap builder + CRC-validating parser for the PMU / PDC wide-area measurement wire protocol: `c37118_build_frame` emits a `SYNC FRAMESIZE IDCODE SOC FRACSEC DATA CHK` frame (CHK = CRC-CCITT) for any payload, `c37118_build_command` handles the fixed Command frame, and `c37118_parse_frame` validates the CRC and reports the frame type / id / timestamp / payload slice (with `c37118_parse_command`). CRC verified against the canonical CRC-CCITT-FALSE check value; pure and host-tested. The fixed phasor configuration / data model layered on the framing is a future addition. See src/services/c37118/c37118.h.

## CANopen

`DETWS_ENABLE_CANOPEN`

CANopen (CiA 301) message codec. Default off. services/canopen is a zero-heap builder + parser for the CANopen messaging set over classic CAN frames (`shared_primitives/det_can.h`): NMT node control, SYNC, TIME, the heartbeat / boot-up (NMT error control), EMCY, PDO process data, and expedited SDO read / write / abort. The 11-bit COB-ID is a 4-bit function code plus a 7-bit node id, so `canopen_build_*` compute it and `canopen_parse` classifies a received frame back to its function + node; `canopen_parse_sdo_response` decodes a server's expedited upload value, download acknowledge, or abort code. The object dictionary itself is the application's, and SDO is expedited only (segmented / block transfer is a future addition). Identifier allocation verified against CiA 301; pure and host-tested. Drive it from the ESP32 TWAI peripheral or an MCP2515 over SPI to bridge a CANopen field bus onto Wi-Fi. See src/services/canopen/canopen.h.

## CBOR

`DETWS_ENABLE_CBOR`

Zero-heap CBOR (RFC 8949) encoder for compact binary payloads. Default off. When set, network_drivers/presentation/cbor/cbor.h provides a writer that serializes ints, strings, byte strings, arrays, maps, booleans, null, and float32 into a caller-provided buffer - a compact binary alternative to the JSON writer for telemetry. Pure, no heap, host-tested against the RFC 8949 vectors.

## Chunked Responses

Streaming / chunked responses of unbounded length in constant memory via send_chunked(). Always on.

## CIP

`DETWS_ENABLE_CIP`

CIP (Common Industrial Protocol) message codec - the message that rides inside an EtherNet/IP Unconnected Data item (DETWS_ENABLE_ENIP). Default off. services/cip builds the request - `cip_build_epath` encodes a class/instance/attribute EPATH from logical segments (`0x20 | type | format`, 8- or 16-bit ids), `cip_build_get_attr_single` / `cip_build_request` prepend the service code + path size - and `cip_parse_response` splits a reply into service / general status / additional status / data. Service codes and the segment encoding are verified against the Wireshark CIP dissector. Pure and host-tested; wrap the request with `eip_build_send_rr_data` for a working CIP read path. See src/services/cip/cip.h.

## CloudEvents

`DETWS_ENABLE_CLOUDEVENTS`

CloudEvents v1.0 (CNCF) event envelope. Default off. services/cloudevents makes a device's events interoperable with serverless / event-mesh consumers: `cloudevents_build_json()` emits a structured `application/cloudevents+json` envelope (the required `id` / `source` / `type` plus optional `subject` / `datacontenttype` / `data`) over the JSON writer, and `cloudevents_from_headers()` reads an inbound binary-mode event's `ce-*` headers. Pure and host-tested. See src/services/cloudevents.h.

## CoAP

`DETWS_ENABLE_COAP`

CoAP server (RFC 7252) over UDP/5683. A zero-heap Constrained Application Protocol endpoint: a fixed resource table dispatched against the request's Uri-Path, with a pure host-testable message codec (parse/build) and an ESP32 UDP binding via the transport-layer UDP service. Default off; the codec is otherwise unit-tested standalone (env:native_coap).

## CoAP Block

`DETWS_ENABLE_COAP_BLOCK`

CoAP block-wise transfer - RFC 7959 (requires COAP). Default off. When set, the server understands the Block2 (descriptive, responses) and Block1 (control, request uploads) options: - Block2: a representation larger than one block, or any GET that carries a Block2 option, is served one block at a time. A constrained client requests a small block size (SZX) and pages through with ascending block numbers; the server re-renders the (idempotent) resource and slices out the asked-for block, setting the More bit until the last. - Block1: a POST/PUT payload larger than one block is reassembled into a single BSS buffer. Each non-final block is acknowledged 2.31 Continue; the final block dispatches the handler with the whole reassembled payload. One block-wise transfer is reassembled at a time (deterministic, single buffer); an out-of-order or oversized block yields 4.08 / 4.13. Size1/Size2 options and the /.well-known/core listing are out of scope.

## CoAP Observe

`DETWS_ENABLE_COAP_OBSERVE`

CoAP resource observation - RFC 7641 (requires COAP). Default off. When set, a client GET with the Observe option registers as an observer of a resource; the application calls coap_notify(path) to push the resource's current representation to every observer (a CoAP notification from the server port with an increasing Observe sequence). Observers are dropped on a deregister GET, a client RST, or send failure.

## Config IO

`DETWS_ENABLE_CONFIG_IO`

Opt-in schema-driven config export / restore. Default off. Requires CONFIG_STORE. The app declares a fixed schema (key + type); services/config_io serializes the current values to a portable `key=value` text blob (backup / migrate) and parses one back into the store (restore / bulk template). Schema-driven rather than enumerating NVS, so it stays deterministic and zero-heap; the serialize / parse is host-tested.

## Config Store

`DETWS_ENABLE_CONFIG_STORE`

Typed NVS configuration store (WiFi creds, IP config,... as blobs). When set, src/services/config_store/config_store.h provides a typed key/value API (string / u32 / blob) that routes core settings into the ESP32's native NVS partition (via `Preferences`) instead of a JSON file on the filesystem - which survives FS corruption and is the corruption-resistant home for credentials. On host builds it is backed by a fixed in-memory table so the typed contract is unit-testable. Default off.

## CORS

Cross-origin resource sharing with automatic preflight handling. Always on.

## COTP

`DETWS_ENABLE_COTP`

TPKT (RFC 1006) + COTP (ISO 8073 / X.224 class 0) frame codec - the "ISO transport on TCP" foundation under S7comm and IEC 61850 MMS. Default off. services/cotp provides `tpkt_build` / `tpkt_parse` for the 4-octet TPKT envelope (version 3 + 16-bit length), `cotp_build_dt` for a Data TPDU (`LI 0xF0 EOT|NR` + user data), `cotp_build_cr` for a Connection Request (destination/source refs, class 0, the TPDU-size parameter, plus caller TSAP parameters), and `cotp_parse` which reports the TPDU type and the DT user data or the CR/CC refs. Layout verified against RFC 1006 / ISO 8073; pure and host-tested. See src/services/cotp/cotp.h.

## CSRF

`DETWS_ENABLE_CSRF`

Opt-in CSRF protection for state-changing HTTP requests. Default off (zero cost / no behavior change). When set, every POST / PUT / PATCH / DELETE must carry a valid `X-CSRF-Token` header (a stateless, HMAC-signed token); requests without one get 403 Forbidden. GET / HEAD / OPTIONS are exempt (they are not state-changing). Clients fetch a token from the built-in `GET /csrf` endpoint, which also sets it as the `csrf` cookie. No server-side session storage - the token self-validates against an HMAC secret seeded from the hardware RNG at begin(); it is independent of AUTH.

## Dashboard

`DETWS_ENABLE_DASHBOARD`

Real-time SVG dashboard (DASHBOARD; requires SSE). Default off. Serves a self-contained, hand-rolled SVG dashboard page whose widgets are declared in a fixed compile-time DetwsWidget table (zero-heap, deterministic). The page fetches the widget layout as JSON and subscribes to an SSE stream of live values; detws_dashboard_set() + detws_dashboard_publish() push the current readings. The widget-table -> JSON serializers are host-testable; WebSocket controls are a follow-up.

## Device ID

`DETWS_ENABLE_DEVICE_ID`

Stable device UUID derived from the chip MAC (RFC 4122 v5). When set, src/services/device_id/device_id.h derives a deterministic v5 UUID from a MAC (via the library's SHA-1) - a storage-free, stable identity for mDNS hostnames, MQTT client IDs, etc. The MAC->UUID core is host-testable; detws_device_uuid() reads the ESP32 factory MAC. Default off.

## DeviceNet

`DETWS_ENABLE_DEVICENET`

DeviceNet link-adaptation codec. Default off. services/devicenet is the CAN-specific layer of "CIP over CAN": `devicenet_encode_id` / `devicenet_decode_id` pack and unpack the 11-bit DeviceNet identifier as a Message Group (1..4) + Message ID + MAC ID (the four identifier ranges 0x000-0x3FF / 0x400-0x5FF / 0x600-0x7BF / 0x7C0-0x7EF), `devicenet_msg_header` / `devicenet_frag_octet` build the explicit-message header and fragmentation octets, `devicenet_build_explicit` emits a single-frame explicit message, and `devicenet_frag_feed` reassembles a fragmented body (first / middle / last, modulo-64 count) up to `DETWS_DEVICENET_MSG_MAX` octets. The CIP application layer (services / EPATH / data) is the same one EtherNet/IP uses, so build the message body with the existing cip codec (`DETWS_ENABLE_CIP`). Identifier allocation + fragmentation verified against the ODVA DeviceNet spec; pure and host-tested. Drive it from the ESP32 TWAI peripheral or an MCP2515 over SPI to bridge a DeviceNet segment onto Wi-Fi. See src/services/devicenet/devicenet.h.

## DF1

`DETWS_ENABLE_DF1`

Allen-Bradley DF1 full-duplex frame codec. Default off. services/df1 is a zero-heap framing + DLE byte-stuffing + BCC/CRC codec for the Rockwell serial PLC data-link layer (pub. 1770-6.5.16): `df1_build_frame` wraps application data in `DLE STX ... DLE ETX` (a data byte equal to DLE/0x10 is doubled) with either a BCC (the 2's complement of the modulo-256 data sum) or a CRC-16/ARC (poly 0x8005, init 0, over the data plus the ETX, sent low byte first), and `df1_parse_frame` validates the check and un-stuffs the data. Vectors verified against the manual (BCC 0x20->0xE0, CRC "123456789"->0xBB3D). Pure and host-tested; the PCCC application header lives inside the data. See src/services/df1/df1.h.

## Diag

`DETWS_ENABLE_DIAG`

Expose a diagnostic JSON endpoint via server.diag(). Disabled by default - enabling it exposes compile-time configuration (buffer sizes, feature flags) which could aid an attacker. Only enable in development or behind an authenticated route. When enabled, DETWS_DIAG_JSON is a compile-time string constant you can serve from any route handler:

## DNP3

`DETWS_ENABLE_DNP3`

DNP3 (IEEE 1815) data-link frame codec. Default off. services/dnp3 is a zero-heap builder + CRC-validating parser for the SCADA / utility outstation data-link layer: `dnp3_build_frame` emits the `0x0564 LEN CTRL DEST SRC CRC` header block plus the CRC'd 16-octet user-data blocks, and `dnp3_parse_frame` validates the header and every block CRC (CRC-16/DNP, verified against the canonical 0xEA82 check value) and de-blocks the user data. Addresses are little-endian. Pure codec, host-tested; the transport-function reassembly and the application layer (objects / function codes) layer on the de-blocked user data. See src/services/dnp3/dnp3.h.

## Dns Resolver

`DETWS_ENABLE_DNS_RESOLVER`

Opt-in DNS resolver with answer verification. Default off. services/dns_resolver resolves a hostname to an IPv4 address (lwIP dns_gethostbyname, marshalled to tcpip_thread like the http_client) and can reject suspicious answers - 0.0.0.0, broadcast, loopback, multicast - which are spoofing / DNS-rebinding indicators for a remote host. The address classifier / verifier is pure and host-tested; the resolve is ESP32-only (blocking, so call it off the request hot path).

## ESP-NOW

`DETWS_ENABLE_ESPNOW`

ESP-NOW peer messaging. Default off. services/espnow wraps ESP-NOW connectionless peer-to-peer radio messaging in a 3-byte typed envelope (magic + type + length) so a receiver can demux by message type and reject a truncated frame, plus a bounded peer registry (DETWS_ESPNOW_MAX_PEERS, no heap). The envelope codec + registry are pure and host-tested; the radio path (begin / add_peer / send / broadcast over esp_now, decoded frames to a callback) is ESP32-only and can bridge to WebSocket/SSE. No stdlib.

## ETag

`DETWS_ENABLE_ETAG`

Conditional GET via ETag for served files. When set, serve_file()/serve_static() emit a strong `ETag` (derived from the file size + last-modified time) and answer a matching `If-None-Match` with `304 Not Modified`, saving bandwidth on repeat fetches of static assets.

## EtherNet/IP

`DETWS_ENABLE_ENIP`

EtherNet/IP encapsulation codec - the ODVA EtherNet/IP transport (TCP/UDP 44818) that carries CIP. Default off. services/enip provides `eip_build` / `eip_parse` for the 24-octet encapsulation header (little-endian command / length / session handle / status / sender context / options), `eip_build_register_session` to open a session, and `eip_build_send_rr_data` / `eip_parse_send_rr_data` to wrap + unwrap a CIP message as an unconnected message via the Common Packet Format (a Null Address item plus an Unconnected Data item). Commands and CPF item types verified against the Wireshark ENIP dissector; pure and host-tested. The CIP object model inside the Unconnected Data item layers on top. See src/services/enip/enip.h.

## File Serving

`DETWS_ENABLE_FILE_SERVING`

Static file serving via Arduino FS (LittleFS, SPIFFS, SD).

## FINS

`DETWS_ENABLE_FINS`

Omron FINS frame codec. Default off. services/fins is a zero-heap command/response builder + parser for the Factory Interface Network Service (FINS/UDP): `fins_build_command` and `fins_build_memory_area_read` emit the 10-octet routing header (ICF/RSV/GCT, destination + source net/node/unit, SID) plus the MRC/SRC command code and parameters, and `fins_parse_command` / `fins_parse_response` read them back (the response MRES/SRES end code included). Talks to an Omron PLC over the shipped UDP transport (det_udp_sendto). Header layout verified against the FINS spec; pure and host-tested. See src/services/fins/fins.h.

## Flow Export

`DETWS_ENABLE_FLOW_EXPORT`

Flow-record export codec. Default off. services/flow_export is a zero-heap exporter-side codec for on-device flow accounting in three formats: NetFlow v5 (the fixed 24-octet header + 48-octet record, via `flow_v5_write_header` / `flow_v5_write_record`), NetFlow v9 (RFC 3954), and IPFIX (RFC 7011). The v9 / IPFIX side is a small cursor (`FlowWriter`): begin the message (`flow_ipfix_begin` / `flow_v9_begin`), emit a Template (`flow_export_template`), open a matching Data Set and append records (`flow_export_data_begin` / `flow_export_data_record` / `flow_export_data_end`), then `flow_export_finish()` patches the IPFIX message length or the NetFlow v9 record count (and pads each v9 FlowSet to a 4-octet boundary). Field offsets verified against RFC 7011 / RFC 3954 / the published v5 layout; pure and host-tested. The flow cache (5-tuple + counters) and the UDP send (det_udp_sendto) are the application's. See src/services/flow_export/flow_export.h.

## GPIO Map

`DETWS_ENABLE_GPIO_MAP`

Opt-in browser GPIO pin-mapper / diagnostics endpoint. Default off. When set, services/gpio_map serves a compile-time table of GPIO pins (number, label, direction, live level) as JSON for a browser diag panel, and accepts a control POST (`pin`, `level`) to drive an output. The live read / write uses the Arduino digital API on ESP32; the JSON serializer and the control parser are pure and host-testable.

## GraphQL

`DETWS_ENABLE_GRAPHQL`

GraphQL query subset. Default off. services/graphql parses a GraphQL query into a fixed AST node pool (no heap) and emits a `{"data":{...}}` response shaped exactly by the requested selection. Schema-free: a field with a sub-selection is an object (the engine recurses), a leaf field calls your single resolver, and arguments collected along the path are handed to it. Supports nested selections, field arguments, and the anonymous / `query` forms; mutations, subscriptions, fragments, and variables are out of scope. Pure and host-tested; bounds are compile-time (DETWS_GQL_* in graphql.h). Serve it from a POST /graphql route.

## gRPC-Web

`DETWS_ENABLE_GRPC_WEB`

gRPC-Web message framing. Default off. services/grpcweb is a zero-heap length-prefixed frame builder + parser for gRPC-Web, the HTTP/1.1-reachable subset of gRPC (gRPC proper needs HTTP/2): `grpcweb_frame_message` wraps a Protobuf message in the 5-octet `[flags][length BE32]` prefix, `grpcweb_frame_trailer` emits the 0x80 trailers frame (`grpc-status` / `grpc-message`), and `grpcweb_parse` reads one frame back (with `grpcweb_trailer_status` to pull the status out of a trailers frame). Wraps the Protobuf codec (DETWS_ENABLE_PROTOBUF) and rides the shipped HTTP/1.1 server/client. Pure and host-tested. See src/services/grpcweb/grpcweb.h.

## Guardrails

`DETWS_ENABLE_GUARDRAILS`

Opt-in runtime heap/stack guardrails. Default off. When set, services/guardrails samples free heap, the heap low-water mark, the largest free block (fragmentation), and the calling task's remaining stack, and fires a callback when any crosses its threshold - a proactive fail-safe hook beyond the passive numbers in /metrics. The threshold evaluator and the JSON serializer are pure and host-tested; the sample reads esp_* / the FreeRTOS stack high-water on ESP32.

## Host Link

`DETWS_ENABLE_HOSTLINK`

Omron Host Link (C-mode) frame codec. Default off. services/hostlink is a zero-heap ASCII command/response codec for Omron's serial host-link protocol (the RS-232/485 sibling of FINS): `hostlink_build` emits `@UU` + 2-char header code + text + FCS + `*`CR, and `hostlink_parse` FCS-validates and splits a frame (`hostlink_end_code` reads a response's end code). The FCS is the 8-bit XOR from `@` through the text, rendered as two hex digits (verified against the `@00RD00000010` -> `57` example). Pure and host-tested; the UART transport is the application's. See src/services/hostlink/hostlink.h.

## HTTP Client

`DETWS_ENABLE_HTTP_CLIENT`

Outbound HTTP(S) client (raw lwIP, optional client-side mbedTLS). Default off. When set, src/services/http_client/http_client.h can issue a blocking GET/POST to a remote server: it resolves the host (DNS), opens a raw lwIP TCP connection (https:// goes through client-side mbedTLS over the same static arena as the server TLS), sends the request, and returns the status + body in caller buffers. For webhooks, telemetry push, REST calls from the device. The request builder + response parser are host-testable; the transport is ESP32-only.

## HTTP Client TLS

`DETWS_ENABLE_HTTP_CLIENT_TLS`

HTTPS client support inside the HTTP client (needs TLS).

## HTTP/1.1 Parser

RFC 7230 request parser - validates method, path, header names and values byte-by-byte before storing anything. Always on.

## IP Allowlist

`DETWS_ENABLE_IP_ALLOWLIST`

Opt-in source-IP allowlist (accept-time firewall, keyed by source IPv4). Default off (zero cost / no behavior change). When set, the accept callback drops any connection whose source address does not match a configured CIDR rule (see listener_ip_allow_add()). An empty allowlist allows everything, so enabling the feature before adding rules never locks the device out. Rules live in a fixed BSS table of DETWS_IP_ALLOWLIST_SLOTS entries (no heap). This is a coarse first-line filter - a spoofed source address can still pass it - so combine it with the accept throttles and network-layer filtering.

## J1939

`DETWS_ENABLE_J1939`

SAE J1939 codec. Default off. services/j1939 is a zero-heap codec for the heavy-duty-vehicle / agriculture / marine / genset CAN higher-layer protocol over 29-bit extended frames (`shared_primitives/det_can.h`): `j1939_encode_id` / `j1939_decode_id` pack and unpack the priority / PGN / source / destination identifier (both the PDU1 peer-to-peer and PDU2 broadcast forms), `j1939_build_message` emits single frames, `j1939_build_request` and `j1939_build_address_claim` (with `j1939_build_name` for the 64-bit NAME) handle the Request PGN and Address Claimed messages, and the Transport Protocol (BAM announce + TP.DT data packets) reassembles multi-packet messages up to `DETWS_J1939_TP_MAX` octets via `j1939_tp_feed`. Identifier layout, NAME bit fields, and the TP control bytes verified against SAE J1939-21 / -81; pure and host-tested. Drive it from the ESP32 TWAI peripheral or an MCP2515 over SPI to bridge a J1939 bus onto Wi-Fi. See src/services/j1939/j1939.h.

## JSON

Zero-heap JSON writer/reader (json.h) for request bodies and responses. Always on.

## JWT

`DETWS_ENABLE_JWT`

JWT bearer-token authentication (HS256). Default off. When set, src/services/jwt/jwt.h verifies `Authorization: Bearer <jwt>` tokens signed with HMAC-SHA-256 (reusing the SSH crypto layer) and can read integer claims (e.g. `exp`) so a handler/middleware can gate routes on a stateless token. Signature verification is constant-time.

## Keep-Alive

`DETWS_ENABLE_KEEPALIVE`

HTTP/1.1 persistent connections (keep-alive). Default off (every response carries `Connection: close` and the connection is closed after one request - the long-standing behavior). When set to 1, a cleanly-parsed request is answered with `Connection: keep-alive` and the slot is recycled for the next request on the same socket: HTTP/1.1 keeps the connection open unless the client sends `Connection: close`; HTTP/1.0 closes unless the client sends `Connection: keep-alive`. Error responses (400/413/414 and any non-PARSE_COMPLETE path) always close, since the next request boundary is unknown. Idle keep-alive connections are still reclaimed by the existing conn_timeout sweep, and each connection serves at most DETWS_KEEPALIVE_MAX_REQUESTS requests before a deliberate close.

## Log-Buffer

`DETWS_ENABLE_LOGBUF`

Opt-in fixed-RAM rotating log buffer with severity traps. Default off. When set, services/logbuf keeps the last DETWS_LOG_LINES log lines in a fixed ring (oldest pruned on overflow - no heap, bounded), dumps them oldest-first for a `/logs` endpoint, and fires a trap callback when a line is logged at/above a severity threshold (forward criticals as an SNMP trap / webhook). The ring + trap logic is pure and host-tested.

## LwM2M

`DETWS_ENABLE_LWM2M`

OMA LwM2M TLV codec. Default off. services/lwm2m is a zero-heap writer + cursor reader for the LwM2M `application/vnd.oma.lwm2m+tlv` resource encoding carried over the shipped CoAP service for device management: `lwm2m_tlv_write` (with typed `lwm2m_tlv_write_int` shortest-form / `lwm2m_tlv_write_bool` / `lwm2m_tlv_write_string` / `lwm2m_tlv_write_float` helpers), `lwm2m_tlv_read`, and `lwm2m_tlv_value_int`. Handles 8-/16-bit identifiers, inline / 8- / 16- / 24-bit lengths, and the Object-Instance / Resource / Multiple-Resource / Resource-Instance kinds; type-byte layout verified against the LwM2M spec. Pure and host-tested. The registration interface and the standard object model layer on top. See src/services/lwm2m/lwm2m_tlv.h.

## M-Bus

`DETWS_ENABLE_MBUS`

Wired M-Bus (Meter-Bus, EN 13757) frame codec. Default off. services/mbus is a zero-heap builder + parser for the M-Bus link-layer frames used by utility meters (water / gas / heat / electricity): `mbus_build_ack` (the single-character 0xE5), `mbus_build_short` (`10 C A CS 16`), and `mbus_build_long` (`68 L L 68 C A CI ... CS 16`, with convenience `mbus_build_snd_nke` / `mbus_build_req_ud2`), plus `mbus_parse` which validates the start / stop octets, the doubled length, and the 8-bit sum checksum. `mbus_record_next` walks the EN 13757-3 variable-data records, skipping the DIFE / VIFE extension chains and decoding the data length from the DIF coding (incl. the LVAR variable form) so the app can read each value. Frame formats + checksum verified against EN 13757-2; pure and host-tested. Talk to the powered two-wire bus over a UART through an M-Bus level converter (a TSS721-based master) and bridge meter readings onto Wi-Fi. See src/services/mbus/mbus.h.

## MDNS

`DETWS_ENABLE_MDNS`

mDNS / DNS-SD advertisement (`name.local` + `_http._tcp`) via ESPmDNS.

## MELSEC

`DETWS_ENABLE_MELSEC`

Mitsubishi MELSEC MC protocol (binary 3E frame) codec. Default off. services/melsec builds + parses the QnA-compatible binary 3E frames for MELSEC PLCs over TCP/UDP: `melsec_build_read` emits a batch-read (word units) request (little-endian fields, request subheader 0x5000, command 0x0401, a device code - D 0xA8 / M 0x90 / X / Y / R / ... - plus a 24-bit head device number and a point count), and `melsec_parse_response` validates the 0xD000 response and reports the end code (0x0000 = success) and the read data. Frame layout + device codes verified against a third-party MC implementation; pure and host-tested. Completes the major-vendor PLC read set alongside FINS / Host Link (Omron), DF1 (Allen-Bradley), and S7comm (Siemens). See src/services/melsec/melsec.h.

## MessagePack

`DETWS_ENABLE_MSGPACK`

Zero-heap MessagePack encoder and decoder for compact binary payloads. Default off. When set, network_drivers/presentation/msgpack/msgpack.h provides a writer that serializes ints, strings, byte strings, arrays, maps, booleans, nil, and float32 into a caller-provided buffer, plus a cursor decoder (msgpack_peek / msgpack_read_\*, no-copy strings) over a caller buffer - the MessagePack-format sibling of the CBOR / JSON readers and writers. Pure, no heap, host-tested against the spec encodings and round-trip.

## Metrics

`DETWS_ENABLE_METRICS`

Prometheus `/metrics` endpoint (text exposition format 0.0.4). Default off (requires STATS for the underlying counters). When set, DetWebServer::metrics() emits the runtime stats as Prometheus metrics (`detws_uptime_seconds`, `detws_http_requests_total`, `detws_http_responses_total{class=...}`, `detws_active_connections`, `detws_free_heap_bytes`,...) so a Prometheus server can scrape the device.

## Middleware

Composable use() pipeline with a fixed-window rate limiter. Always on.

## Modbus

`DETWS_ENABLE_MODBUS`

Modbus TCP slave/server (Modbus Application Protocol v1.1b3) on TCP/502. Default off. When set, listen(502, PROTO_MODBUS) serves a fixed data model (coils, discrete inputs, holding + input registers, all in BSS) over Modbus TCP: Read/Write Coils (FC 1/5/15), Read Discrete Inputs (FC 2), Read/Write Holding Registers (FC 3/6/16), and Read Input Registers (FC 4). The codec (MBAP framing + PDU dispatch) is pure and host-tested; the TCP transport is ESP32-only. The application reads/writes the model with the accessor functions and is notified of client writes via modbus_on_write(). Modbus has no authentication or encryption - run it only on a trusted control network.

## Modbus Master

`DETWS_ENABLE_MODBUS_MASTER`

Opt-in Modbus master codec + register scanner. Default off. services/modbus/modbus_master builds Modbus TCP read-request ADUs and parses the responses (register values or exception), so an app can poll / auto-discover a slave's registers. Pure and host-tested as a full round-trip against the slave codec (modbus_process_adu); the actual send is the app's TCP.

## Modbus RTU

`DETWS_ENABLE_MODBUS_RTU`

Modbus RTU framing (serial / RS-485). Default off; implies MODBUS. Adds the RTU ADU codec `modbus_rtu_process_adu()` - a `[slave addr][PDU][CRC16]` frame (CRC16-Modbus, little-endian) wrapped around the existing host-tested PDU dispatch: a CRC mismatch or a non-matching unit address is dropped silently (no reply, per the spec), and a broadcast (address 0) is processed with no reply. Pure and host-tested; feed it from a UART/RS-485 driver (the serial transport, framed by the 3.5-character inter-frame idle, is the application's). See src/services/modbus/modbus.h.

## MQTT

`DETWS_ENABLE_MQTT`

MQTT 3.1.1 publish/subscribe client (raw lwIP, optional MQTTS over TLS). Default off. When set, src/services/mqtt/mqtt.h provides a persistent outbound client: connect to a broker, PUBLISH (QoS 0/1/2) and SUBSCRIBE to topics, receive incoming messages via a callback, with keep-alive pings - the dominant IoT messaging pattern, for telemetry push and remote command. The packet codec is host-testable; the transport (DNS + raw lwIP TCP, MQTTS via client-side mbedTLS) is ESP32-only. Full QoS 0/1/2 (outbound DUP retransmit, inbound QoS-2 de-duplication by packet id) and Last-Will are supported.

## MQTT SN

`DETWS_ENABLE_MQTT_SN`

MQTT-SN v1.2 wire codec. Default off. services/mqtt/mqtt_sn is a zero-heap codec for MQTT for Sensor Networks - the UDP / non-TCP MQTT variant for constrained, lossy links (numeric topic IDs instead of strings, gateway discovery, sleeping-client keep-alive). Builders for CONNECT / REGISTER / PUBLISH / SUBSCRIBE (by name or pre-defined id) / PINGREQ / DISCONNECT / SEARCHGW, plus `mqttsn_parse_header()` (the 1- and 3-octet Length forms, big-endian fields) and typed parsers for CONNACK / REGACK / PUBACK / SUBACK / PUBLISH / REGISTER, with a `mqttsn_make_flags()` helper (DUP / QoS / retain / will / clean / TopicIdType). Wire bytes verified against the spec and the Eclipse Paho reference; pure and host-tested. The datagram send (det_udp_sendto), topic-ID registry, and sleep / retransmit state are the application's. See src/services/mqtt/mqtt_sn.h.

## MQTT TLS

`DETWS_ENABLE_MQTT_TLS`

MQTTS: run the MQTT client over client-side TLS (needs TLS).

## MTLS

`DETWS_ENABLE_MTLS`

Mutual TLS - require and verify a client certificate (mTLS). Default off. When set (requires TLS), the server can be given a trust-anchor CA via DetWebServer::tls_require_client_cert(): the TLS handshake then demands a client certificate chaining to that CA (MBEDTLS_SSL_VERIFY_REQUIRED) and aborts the connection if the client presents none or an untrusted one. The verified peer's subject DN is available to handlers via DetWebServer::tls_client_subject(). Strong transport-level client authentication with no passwords.

## Multipart

`DETWS_ENABLE_MULTIPART`

multipart/form-data body parser.

## NATS

`DETWS_ENABLE_NATS`

NATS client protocol codec - the text-based NATS pub/sub messaging protocol. Default off. services/nats lets a device be a NATS client over the outbound client transport: builders for `CONNECT`, `PUB` (with optional reply-to), `SUB` (with optional queue group), `UNSUB`, `PING`, and `PONG`, plus `nats_parse` which decodes an inbound `MSG` / `INFO` / `PING` / `PONG` / `+OK` / `-ERR` (a MSG yields subject / sid / reply-to / payload). Line-oriented (CRLF, space-delimited); only PUB and MSG carry a payload. Pure and host-tested; the connection and subscription state are the application's. See src/services/nats/nats.h.

## NMEA 2000

`DETWS_ENABLE_NMEA2000`

NMEA 2000 codec. Default off; implies J1939 (NMEA 2000 is J1939 at the transport layer). services/nmea2000 is a zero-heap codec for the marine instrumentation network over CAN: it reuses the J1939 29-bit identifier codec and adds the NMEA-specific Fast Packet transport. `n2k_fastpacket_num_frames` sizes a transfer, `n2k_fastpacket_build_frame` emits frame N of a 9..223-octet message (a control octet of sequence counter + frame counter, the first frame carrying the total length and 6 data octets, continuations carrying 7), and `n2k_fastpacket_feed` reassembles a sequence (matching source / PGN / sequence counter, rejecting out-of-order frames and ignoring interleaved sequences); `n2k_build_single` wraps a single-frame message. Fast Packet framing verified against the NMEA 2000 / J1939 layout; pure and host-tested. Drive it from the ESP32 TWAI peripheral or an MCP2515 over SPI to bridge an NMEA 2000 backbone (GPS, wind, depth, engine PGNs) onto Wi-Fi. See src/services/nmea2000/nmea2000.h.

## NTP

`DETWS_ENABLE_NTP`

SNTP wall-clock time sync via the ESP-IDF SNTP client.

## OAuth2

`DETWS_ENABLE_OAUTH2`

OAuth2 token-endpoint client. Default off. services/oauth2 obtains tokens - the counterpart to the OIDC ID-token verifier. It builds the percent-encoded form body for the authorization_code and refresh_token grants (RFC 6749), supporting a confidential client (client_secret) or a public client with PKCE (code_verifier, RFC 7636), and parses the JSON token response (reusing the zero-heap JSON reader). The build + parse core is pure and host-tested; the POST to the token endpoint uses the HTTP(S) client (needs HTTP_CLIENT). No heap, no stdlib.

## Observability

`DETWS_ENABLE_OBSERVABILITY`

Transport-layer observability: connection-event hook + counters. Default off (zero cost when unset - the notify points compile to nothing). When set, the transport (L4) fires an application callback on every connection state transition - `det_conn_on_event(slot, old_state, new_state, reason)` - and maintains lock-free counters (accepts, closes by reason, idle timeouts, RX backpressure events, dropped deferred events, and a live CONN_CLOSING gauge) readable via `det_conn_counters()`. The only state-transition trace the L4/L5 core exposes; pair it with STATS for request-level metrics.

## OIDC

`DETWS_ENABLE_OIDC`

OpenID Connect ID-token verification, RS256. Default off. services/oidc verifies an OIDC ID token (JWT) as a relying party: requires alg RS256, selects the issuer key by kid from a JWKS, verifies the RSASSA-PKCS1-v1.5 SHA-256 signature (real RSA modexp via ssh_rsa, mbedTLS- accelerated on ESP32), and checks iss / aud / exp / nbf, extracting sub / email. Pure and host-tested; the caller fetches + caches the JWKS over HTTPS (off the request hot path) and passes the JSON in. Builds on the SSH RSA primitive, not the HS256 JWT module (services/jwt), so the two are independent.

## OPC-UA

`DETWS_ENABLE_OPCUA`

OPC UA Binary server. Default off. services/opcua provides an OPC UA (IEC 62541) Binary server: the little-endian built-in-type codec (incl. NodeId / ExtensionObject / DateTime / Variant / DataValue / ReferenceDescription), UA-TCP (UACP) message framing, the Hello/Acknowledge handshake, the SecureChannel (OpenSecureChannel, SecurityPolicy None), the Session (CreateSession + ActivateSession), GetEndpoints, the Read, Write and Browse services (registered resolvers map a NodeId to a value / accept a written value / list child references), plus CloseSession + CloseSecureChannel and a ServiceFault for unsupported services, served on TCP via PROTO_OPCUA (`listen(4840, PROTO_OPCUA)`). The MSG framing is spec-faithful (incl. SecureChannelId), so standard clients interoperate (verified with python asyncua: connect + browse + read + write/read-back). All pure and host-tested. No heap, no stdlib.

## OPC-UA Client

`DETWS_ENABLE_OPCUA_CLIENT`

OPC UA Binary client. Default off (requires OPC-UA, shares the codec). services/opcua_client builds the client-side requests (Hello, OpenSecureChannel, CreateSession, ActivateSession, Read, Browse, CloseSession, CloseSecureChannel) and parses the server responses, reusing the opcua.h codec. Transport-agnostic - the app supplies the outbound socket (e.g. an Arduino WiFiClient). No heap, no stdlib.

## OTA

`DETWS_ENABLE_OTA`

Authenticated OTA firmware update (streaming POST to the ESP32 Update API).

## OTA Rollback

`DETWS_ENABLE_OTA_ROLLBACK`

Opt-in OTA rollback protection / soft-brick safeguard. Default off. After an OTA update the new image boots in PENDING_VERIFY; this service confirms it (esp_ota_mark_app_valid) once a self-test passes, or rolls back to the previous image if the self-test fails or the confirm window elapses without success - so a bad update self-heals instead of soft-bricking. The decision logic is pure and host-tested; the commit / rollback use esp_ota_ops. Requires the bootloader's app-rollback support (CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE).

## Partition Monitor

`DETWS_ENABLE_PARTITION_MONITOR`

Opt-in flash partition-map monitor endpoint. Default off. When set, services/partition_monitor reports the device's flash partition table (label, kind, type / subtype, offset, size, and which app slot is running) as JSON, for diagnostics and OTA dashboards. The partition walk uses esp_partition / esp_ota_ops; the JSON serializer and the kind classifier are pure and host-testable.

## Per IP Throttle

`DETWS_ENABLE_PER_IP_THROTTLE`

Opt-in per-IP accept-rate throttle (connection-flood defense, keyed by source IPv4). Default off (zero cost / no behavior change). Complements the global accept throttle: the accept callback rejects a new connection once one source IPv4 address has opened more than DETWS_PER_IP_THROTTLE_MAX connections within a DETWS_PER_IP_THROTTLE_WINDOW_MS fixed window. A fixed BSS table of DETWS_PER_IP_THROTTLE_SLOTS buckets tracks the most-recently-seen source addresses; when a new address arrives and the table is full, an expired or least-recently-started bucket is reused, so memory stays bounded (no heap). This bounds reconnect/brute-force churn from a single host (the gap left by the global throttle, which cannot tell one noisy client from many). It is best-effort: an attacker spreading across many source addresses can still churn the bounded connection pool, so combine it with the global throttle and network-layer filtering.

## Protobuf

`DETWS_ENABLE_PROTOBUF`

Protocol Buffers wire codec. Default off. services/protobuf is a zero-heap streaming Protobuf encoder + cursor reader over caller buffers (the same shape as the CBOR / MessagePack codecs): a writer for varint / ZigZag / fixed32 / fixed64 / length-delimited fields (`pb_uint64`, `pb_sint64`, `pb_fixed32`, `pb_fixed64`, `pb_float`, `pb_double`, `pb_bytes`, `pb_string`; embedded messages are built into a sub-buffer and added with `pb_bytes`), and a reader (`pb_read_field`) that decodes one field at the buffer head and reports bytes consumed, with ZigZag / float / double value decoders. Host-tested against the spec vectors. This is the standalone Protobuf deliverable; gRPC (framed Protobuf over HTTP/2) is gated on the HTTP/2 roadmap item. See src/services/protobuf/protobuf.h.

## Provisioning

`DETWS_ENABLE_PROVISIONING`

First-boot WiFi provisioning: softAP + captive-portal credentials form.

## Proxy Protocol

`DETWS_ENABLE_PROXY_PROTOCOL`

HAProxy PROXY protocol codec - recover the real client IPv4 when the server sits behind a load balancer / reverse proxy that prepends a PROXY header. Default off. services/proxy_protocol provides `proxy_parse` (detects + decodes a v1 text `PROXY TCP4 ...\r\n` header or a v2 binary header - 12-octet signature + ver_cmd / family / address block - and reports the bytes to skip before the real stream) plus `proxy_v1_build` / `proxy_v2_build` for a TCP/IPv4 header. Handles the library's IPv4 family; IPv6 / UNIX / LOCAL headers parse to their length but yield no addresses. Format per the HAProxy PROXY protocol spec; pure and host-tested. See src/services/proxy_protocol/proxy_protocol.h.

## Radio Power

`DETWS_ENABLE_RADIO_POWER`

Opt-in radio power controls. Default off. services/radio_power applies the WiFi modem-sleep mode and an optional max-TX-power cap in one call (esp_wifi_set_ps / esp_wifi_set_max_tx_power) - trade throughput/latency for lower average power on a battery device. The mode names are host-tested; the apply is ESP32-only.

## Range

`DETWS_ENABLE_RANGE`

HTTP Range requests / 206 Partial Content for served files. Default off. When set (requires FILE_SERVING), serve_file() / serve_static() honor a single-range `Range: bytes=...` request header: they answer `206 Partial Content` with a `Content-Range` header and stream only the requested bytes (seeking the file to the start offset), advertise `Accept-Ranges: bytes` on full responses, and answer an unsatisfiable range with `416 Range Not Satisfiable`. This enables resumable downloads and media seeking. Multi-range (multipart/byteranges) requests are not supported - the server falls back to a full 200 response, which is RFC 7233 §3.1 compliant.

## Redis

`DETWS_ENABLE_REDIS`

Redis RESP2 wire codec. Default off. services/redis_resp lets a device drive a Redis server over the shipped outbound client transport: `resp_encode_command` builds a command (an array of bulk strings, binary-safe via explicit arg lengths) and `resp_parse` is a cursor reply decoder (simple / error / integer / bulk / array / nil). Pure and host-tested; the connection is the application's. See src/services/redis_resp.h.

## Routing

Exact, wildcard (/*), :param path parameters, bounded allocation-free regex routes, and per-interface STA/softAP route filters. Always on.

## S7comm

`DETWS_ENABLE_S7COMM`

Siemens S7comm PDU codec. Default off. services/s7comm builds + parses the S7-300/400 communication PDUs carried inside a COTP Data TPDU (DETWS_ENABLE_COTP) over ISO-on-TCP (port 102): `s7_build_setup` (Setup Communication), `s7_build_read_request` (Read Var with S7-ANY items over the DB / I / Q / M areas, encoding the byte address as a 24-bit bit-address), `s7_parse_header`, and `s7_read_next_item` which walks the response data items honoring the length-in-bits transport sizes (BIT/BYTE/INT) and the even-item padding. All constants (protocol id 0x32, ROSCTR, function/area/transport codes) are verified against the Wireshark S7comm dissector. Pure and host-tested; wrap the PDU with `cotp_build_dt` + `tpkt_build`. See src/services/s7comm/s7comm.h.

## SenML

`DETWS_ENABLE_SENML`

SenML (RFC 8428) measurement-pack builder. Default off; implies CBOR. services/senml is a zero-heap SenML-JSON + SenML-CBOR encoder over the shipped JSON / CBOR writers: the caller fills a `SenmlRecord` array (optional base name / base time, name, unit, one value - number / string / boolean, optional time) and `senml_json_build` / `senml_cbor_build` emit the whole pack. SenML-JSON uses the text labels (bn/n/u/v/vs/vb/t); SenML-CBOR uses the integer labels (n=0, u=1, v=2, ..., bn=-2, bt=-3). Numbers that are integral are emitted as integers so timestamps keep full precision. The standard measurement format for CoAP / LwM2M / HTTP telemetry; verified against the RFC example, host-tested. See src/services/senml/senml.h.

## SNMP

`DETWS_ENABLE_SNMP`

SNMP agent (v1/v2c, + v3 USM when SNMP_V3) over lwIP UDP. Zero-heap ASN.1 BER codec + a fixed MIB table on UDP/161. Default off. The BER codec itself is gated by this flag and is otherwise unit-tested standalone (env:native_snmp).

## SNMP Trap

`DETWS_ENABLE_SNMP_TRAP`

Outbound SNMP notifications - traps and informs (requires SNMP). Default off. When set, src/services/snmp/snmp_notify.h sends SNMPv2c (and, with SNMP_V3, SNMPv3 USM) Trap / InformRequest PDUs to a manager over UDP - so the agent can push alerts instead of only answering polls. Reuses the BER codec and the transport-layer UDP service; the PDU builder is host-testable.

## SNMP V3

`DETWS_ENABLE_SNMP_V3`

Add SNMPv3 USM (auth via HMAC-SHA, privacy via AES-128-CFB). Default off.

## Sparkplug

`DETWS_ENABLE_SPARKPLUG`

Sparkplug B payload + topic codec - the Eclipse Sparkplug B industrial-IoT format over MQTT. Default off; implies PROTOBUF (the payload is a Protobuf message). services/sparkplug builds the topic (`spb_build_topic` -> `spBv1.0/group/type/node[/device]`) and serializes the payload over the protobuf codec: `spb_build_metric` emits one Tahu Metric (name / alias / timestamp / datatype + a value - int / long / float / double / boolean / string), and `spb_build_payload` wraps the timestamp + metrics + seq. Field numbers and datatype codes are verified against the Eclipse Tahu sparkplug_b.proto. Pure and host-tested; publish the payload with the MQTT client. See src/services/sparkplug/sparkplug.h.

## SSE

`DETWS_ENABLE_SSE`

Server-Sent Events push support.

## SSH

`DETWS_ENABLE_SSH`

SSH server support (RFC 4253/4252/4254).

## Stats

`DETWS_ENABLE_STATS`

Runtime stats endpoint (uptime, request/error counts, pool usage, heap).

## Stomp

`DETWS_ENABLE_STOMP`

STOMP 1.2 frame codec. Default off. services/stomp is a zero-heap codec for the Simple/Streaming Text Oriented Messaging Protocol: `stomp_build_frame()` writes a frame (command + escaped `key:value` headers + blank line + NUL-terminated body) and `stomp_parse_frame()` is a non-mutating cursor reporting the command, header key/value slices, and body (honoring `content-length`, tolerating `\r\n` line endings, and skipping broker heart-beats), with `stomp_header()` lookup and `stomp_unescape()` for the header escapes (`\r` `\n` `\c` `\\`). Drives CONNECT / SEND / SUBSCRIBE / MESSAGE / ACK against a broker (ActiveMQ / RabbitMQ / Artemis) over the shipped outbound client transport, or STOMP-over-WebSocket via the WS client. Pure and host-tested; the connection and subscription state are the application's. See src/services/stomp.h.

## SunSpec

`DETWS_ENABLE_SUNSPEC`

SunSpec Modbus device-information-model codec. Default off. services/sunspec is a zero-heap codec for the SunSpec Alliance register maps layered on the holding-register model: a model-chain walker (`sunspec_check_marker` / `sunspec_begin` / `sunspec_next_model` - verify the `SunS` 0x53756E53 marker, then iterate each model's id / length / body to the 0xFFFF end model) plus typed point readers (`sunspec_u16`, `sunspec_i16`, `sunspec_u32`, `sunspec_i32`, `sunspec_string`), and a map writer (`sunspec_write_marker`, `sunspec_write_model_header`, the point writers, `sunspec_write_end_model`) for a device exposing its own map. Makes a solar inverter / meter / battery interoperable. Marker and header format verified against the SunSpec spec; pure and host-tested. Pairs with the Modbus service. See src/services/sunspec/sunspec.h.

## Syslog

`DETWS_ENABLE_SYSLOG`

Syslog client (RFC 5424 over UDP). Default off. When set, the device can ship log lines to a remote syslog server (e.g. rsyslog / journald / a SIEM) as RFC 5424 UDP datagrams via the transport-layer UDP service - a zero-heap structured-logging sink for fleets of constrained devices. See src/services/syslog/syslog.h.

## Telemetry

`DETWS_ENABLE_TELEMETRY`

Telemetry math helpers (moving-window stats, rate-of-change, totalizer). Default off. When set, src/services/telemetry/telemetry.h provides zero-heap pure-computation helpers over caller-supplied storage: a moving-window stats accumulator (mean / variance / stddev / min / max), a derivative / rate-of- change tracker, and a trapezoidal run-time totalizer. No ESP32 dependency, so the whole cluster is host-testable; it feeds dashboards, alert triggers, and odometer-style counters.

## Telnet

`DETWS_ENABLE_TELNET`

Telnet server support (RFC 854 / IAC option negotiation).

## Templating

{{var}} response templating via send_template(). Always on.

## Time Source

`DETWS_ENABLE_TIME_SOURCE`

Multi-source time fallback (NTP / RTC / GPS /... by priority). When set, src/services/time_source/time_source.h provides a small registry of user-defined time sources, each a callback returning Unix epoch seconds (0 when that source has no valid time). detws_time_now() queries them in priority order (lowest value first) and returns the first valid result, so the device falls back automatically when its preferred clock is unavailable. Pure and zero-heap (a fixed source table); host-testable. Default off.

## TLS

`DETWS_ENABLE_TLS`

TLS (HTTPS/WSS) via mbedTLS with a static memory pool (ESP32-only). When set, the server can accept TLS connections using mbedTLS configured with MBEDTLS_MEMORY_BUFFER_ALLOC_C over a fixed BSS arena (DETWS_TLS_ARENA_SIZE) - no system heap, so the determinism guarantee is preserved. The TLS engine is compiled only on Arduino/ESP32 (mbedTLS is not part of the native build). Default off.

## TLS Resumption

`DETWS_ENABLE_TLS_RESUMPTION`

TLS session resumption via RFC 5077 session tickets (requires TLS). Default off. When set, the TLS 1.2 server issues encrypted session tickets and accepts them on reconnect, so a returning client completes an abbreviated handshake (no certificate or full key exchange) - much faster and far less CPU than the ~RSA/ECDHE full handshake. Resumption is stateless: the session state lives in the client's ticket, sealed with a server-held key, so there is no growing per-session cache (the determinism / zero-heap-growth guarantee holds; only a small fixed ticket key and a little arena headroom are added). The ticket key rotates automatically on the DETWS_TLS_TICKET_LIFETIME_S schedule. Needs the mbedTLS build to provide MBEDTLS_SSL_TICKET_C (stock arduino-esp32 does).

## TOTP

`DETWS_ENABLE_TOTP`

Opt-in TOTP two-factor auth (RFC 6238). Default off. services/totp computes and verifies time-based one-time passwords (HMAC-SHA1 over the existing SHA-1, Google Authenticator compatible) and decodes base32 shared secrets, for a second factor on top of password / JWT auth. Pure and host-tested against the RFC 6238 vectors; the verifier checks a +/- step window for clock skew.

## UDP Telemetry

`DETWS_ENABLE_UDP_TELEMETRY`

Opt-in fire-and-forget UDP telemetry cast. Default off. When set, services/udp_telemetry casts metric lines (InfluxDB line protocol: `measurement field=val,field2=val2`) to a configured collector over UDP via det_udp_sendto - zero-heap, fire-and-forget (no ACK, no retry), ideal for shipping device metrics to Telegraf/InfluxDB/a log sink. The line builder is pure and host-tested; only the send touches the network.

## Upload

`DETWS_ENABLE_UPLOAD`

Streaming file upload: POST a body straight to a file on the filesystem. Default off. When set, src/services/upload_service.h registers a POST route that streams the request body directly into an Arduino FS file (LittleFS / SPIFFS / SD) - the upload never has to fit in RAM. Reuses the same parser streaming-body hook as OTA. For reliable uploads set RX_BUF_SIZE above the largest inbound TCP segment (TCP_MSS, ~1460): the transport refuses-and-redelivers a segment that will not fit the receive ring (lossless backpressure), but a ring smaller than one segment would stall. The 1024 default suits ordinary requests, not uploads.

## VFS

`DETWS_ENABLE_VFS`

Unified virtual filesystem wrapper. Default off. services/vfs exposes one small file API (open/read/write/close, exists/size/remove/rename, whole-file helpers) over a pluggable backend, so a feature can target storage without knowing the medium. A built-in zero-heap RAM backend (fixed BSS pool - deterministic, host-identical) ships for scratch / tests; an Arduino-FS backend (ESP32) wraps a real fs::FS (LittleFS / SD / SPIFFS) for persistence. Mount one at startup; the API fails closed otherwise. Pool dimensions are tunable in vfs.h (DETWS_VFS_RAM_FILES, _RAM_FILE_SIZE, _MAX_OPEN, _NAME_MAX).

## WAMP

`DETWS_ENABLE_WAMP`

WAMP messaging codec. Default off. services/wamp is a zero-heap codec for the Web Application Messaging Protocol (unified RPC + PubSub over WebSocket, subprotocol `wamp.2.json`): builders for HELLO / SUBSCRIBE / UNSUBSCRIBE / PUBLISH / CALL / REGISTER / YIELD / GOODBYE (JSON arrays emitted via the shared JsonWriter - Options/Details default to `{}`, Arguments / ArgumentsKw are passed as JSON literals), plus a nesting-aware positional parser (`wamp_get_type`, `wamp_get_uint`, `wamp_get_uri`, `wamp_element`) that pulls the message type, ids, and URIs out of an inbound WELCOME / SUBSCRIBED / EVENT / RESULT / INVOCATION / ERROR. Message codes verified against the WAMP spec; pure and host-tested. It rides the shipped WebSocket layer; the session / subscription / registration tables are the application's. See src/services/wamp/wamp.h.

## Web Terminal

`DETWS_ENABLE_WEB_TERMINAL`

Browser "web serial" terminal over WebSocket (src/services/web_terminal). Serves a self-contained terminal page and a WebSocket endpoint: device output is broadcast to all connected browsers, browser input is delivered to a command callback. Requires WEBSOCKET. Default off.

## WebDAV

`DETWS_ENABLE_WEBDAV`

WebDAV server (RFC 4918, class 1 + advisory locks) over the file system. Default off. When set (requires FILE_SERVING), dav() mounts an FS subtree that answers the WebDAV methods - OPTIONS, PROPFIND (Depth 0/1), PROPPATCH, GET, HEAD, PUT, DELETE, MKCOL, COPY, MOVE, and advisory LOCK/UNLOCK - so a client (rclone, cadaver, curl, or a mounted network drive) can browse and edit files. PROPFIND returns a 207 Multi-Status document built into a fixed buffer (DETWS_WEBDAV_BUF_SIZE); a Depth-1 listing is capped at DETWS_WEBDAV_MAX_ENTRIES children. PROPPATCH returns a 207 with each requested property refused 403 Forbidden (read-only live properties, no dead-property store) so Explorer/Finder - which PROPPATCH a timestamp right after a PUT - do not error on a 405. PUT streams the request body straight to the file (via the shared streaming-body sink), so an upload is not bounded by BODY_BUF_SIZE. Locks are advisory (a synthetic token is issued but not enforced). See docs/SECURITY.md before exposing it.

## Webhook

`DETWS_ENABLE_WEBHOOK`

Opt-in outbound webhooks / IFTTT. Default off. Requires HTTP_CLIENT. services/webhook builds an IFTTT Maker URL and a value1/value2/value3 JSON payload (pure, host-tested) and fires them - or any JSON to any URL - via the outbound http_client (POST). Use it to push an event from the device to IFTTT, a Slack/Discord hook, or your own API.

## WebSocket

`DETWS_ENABLE_WEBSOCKET`

WebSocket support (RFC 6455 framing + SHA-1/base64 handshake).

## WS Client

`DETWS_ENABLE_WS_CLIENT`

Outbound WebSocket client (RFC 6455 over raw lwIP, optional wss:// TLS). Default off. When set, src/services/ws_client/ws_client.h connects to a remote WebSocket endpoint (ws://, or wss:// over client-side mbedTLS), performs the RFC 6455 client handshake (Sec-WebSocket-Key/Accept), and sends masked text / binary frames + receives server frames via a callback - for streaming to cloud dashboards or bidirectional control. The frame/handshake codec is host-testable.

## WS Client TLS

`DETWS_ENABLE_WS_CLIENT_TLS`

wss://: run the WebSocket client over client-side TLS (needs TLS).

## WS Deflate

`DETWS_ENABLE_WS_DEFLATE`

WebSocket permessage-deflate (RFC 7692) - bidirectional compression. When set (and WEBSOCKET is on), the server negotiates the `permessage-deflate` extension and both decompresses inbound compressed (RSV1) messages via a bounded INFLATE (network_drivers/presentation/inflate._) and compresses outbound data frames via a bounded DEFLATE (network_drivers/presentation/deflate._); both borrow their table scratch from the shared per-dispatch arena. The extension is negotiated with `{client,server}_no_context_takeover` so every message (de)compresses independently - no window is carried between messages. An outbound frame that would not shrink is sent uncompressed (the per-message RSV1 flag permits this). Default off.
