# Architecture & internal data piping

How bytes and control flow move between the OSI-style layers, who owns each
cross-layer concern, and where the piping is still being straightened. This is the
map referenced by the internal-piping cleanup: the rule we are converging on is
**one owner per cross-layer concern, behind a clean API** - no layer reaches into
another's internals.

## Layers

```
src/network_drivers/
  physical/     WiFi / Ethernet bring-up, link state
  datalink/     L2 framing concerns
  network/      IPv4 / interface tagging (DetIface: STA vs AP)
  transport/    lwIP raw-API callbacks, the per-connection RX ring, the TcpEvt
                event queue, and the det_conn_* I/O API. OWNS all socket I/O.
  tls/          mbedTLS record layer (static-pool BIO) - plaintext <-> ciphertext
  session/      worker task(s), event dispatch (server_tick), scratch arena,
                deferred-callback queues, the ProtoHandler dispatch table
  presentation/ http_parser, websocket, telnet, ssh, sse - turn a byte stream
                into requests/frames
  application/  DetWebServer: routes, handlers, serve_file / send_chunked pumps,
                WebDAV
src/services/   mqtt, modbus, opcua, snmp, coap, ... (protocol features)
```

## Two threads, two boundaries

The server is a 2-thread system and every cross-layer hazard lives on one of two
boundaries:

1. **tcpip_thread (Core 0, producer)** runs the lwIP callbacks: `lowlevel_recv_cb`
   copies an inbound segment into the connection's RX ring and posts a `TcpEvt`;
   `lowlevel_sent_cb` nudges the owning worker; `listener_accept_cb` assigns a new
   slot its owner worker.
2. **worker task(s) (Core 1, consumer)** run `service_once()` -> `server_tick()`
   (drain the event queue -> `dispatch_event` -> `ProtoHandler::on_data`) then a
   per-slot pump (`on_poll`, the HTTP/WS/SSE inline pump, the file/chunk send pumps).

Cross-thread synchronization primitives (no hot-path locks):

- `DetAtomic<>` (acquire/release) on `TcpConn::state`, `rx_head`, `rx_tail`.
- The FreeRTOS `TcpEvt` queue (producer -> owner worker).
- `tcpip_api_call` marshaling for the app->lwIP direction (see TX below).

## TX path - clean, single owner (transport)

Every layer that sends bytes calls the transport API; nobody calls lwIP `tcp_*`
directly:

```
app / presentation  -->  det_conn_send / det_conn_flush / det_conn_sndbuf
                         (TLS slots route through det_tls_write)
                              |
                         det_tcp_marshal (DET_OP_*)  -->  tcpip_thread: tcp_write / tcp_output
```

This is the model the RX side is being moved toward. See [[transport-io-api]].

## RX path - the concern being straightened

Inbound:

```
tcpip_thread: recv_cb  -->  copy into TcpConn.rx_buffer (advance rx_head)  -->  post EVT_DATA
worker:       server_tick -> dispatch_event -> on_data  -->  drain the ring (advance rx_tail)
```

**Receive-window flow control: now single-owner (transport).** `recv_cb` no longer
ACKs on copy. The worker calls `det_conn_ack_consumed(slot)` once per slot per loop
and transport reopens the TCP window by exactly the bytes drained since the last ACK
(ack-on-consume; `tcp_recved` marshaled). The window therefore tracks ring occupancy
and a slow consumer cannot overflow the ring. **TCP-level requirement:**
`RX_BUF_SIZE >= TCP_WND` (you cannot advertise a window larger than your buffer).
See docs/BUGS.md "RX flow-control deadlock".

**RX ring read API: now single-owner (transport).** Consumers no longer index
`rx_buffer` or advance `rx_tail`. They drain through the transport read API -
`det_conn_available` / `det_conn_read_byte` / `det_conn_read` / `det_conn_peek` /
`det_conn_consume` (inline in transport.h, single-consumer per slot). Migrated:
`presentation.cpp` (HTTP), `websocket.cpp`, `telnet.cpp`, `ssh/ssh_conn.cpp`,
`tls/det_tls.cpp`, and the conn_pool-ring services `modbus.cpp` / `opcua.cpp` (their
duplicated `ring_peek/consume/avail` are now thin adapters over the API). The read
functions only consume; the window is reopened by the worker's single
`det_conn_ack_consumed()` per loop - so there is exactly one place that touches the
ring indices for draining and one that ACKs.

The MQTT and WebSocket **client** modules (`mqtt.cpp`, `ws_client.cpp`) keep their
own private `s_rx` ring - this is correct, not the same spaghetti. They are
single-instance outbound clients (the device connecting out), not server slots in
`conn_pool`; the buffer is filled by the module's own recv/TLS path and is owned
solely by that module, so nothing cross-layer reaches into it. A unified client
transport (so clients and the server share one I/O API) is separate future work,
not part of this server-ring straightening.

## Streaming-body hooks - slot-aware

`http_parser_set_stream_hooks(begin, data, abort)` are global singletons
(last-registered-wins, so OTA / upload / WebDAV streaming are still mutually
exclusive per build). All three now take `HttpReq*`, so a sink can keep
per-connection state: WebDAV holds per-slot PUT state (`g_dav_put[MAX_CONNS]`) and
each connection streams to its own file. This fixed the concurrent-PUT clobber
(docs/BUGS.md) - HW: 4 parallel PUTs with distinct payloads, all byte-exact.

## Ownership: current vs target

| Concern              | Owner (target)         | Status                                          |
| -------------------- | ---------------------- | ----------------------------------------------- |
| Socket TX            | transport `det_conn_*` | DONE                                            |
| RX receive window    | transport              | DONE (`det_conn_ack_consumed`, ack-on-consume)  |
| RX ring read/drain   | transport (read API)   | DONE (`det_conn_read*`; consumers off the ring) |
| Streaming sink state | per-slot, slot-aware   | DONE (`g_dav_put[MAX_CONNS]`, slot-aware hooks) |
| Event routing        | session (owner queue)  | DONE                                            |
| Scratch memory       | session (per-worker)   | DONE                                            |

## Straightening plan (phased; each phase host + HW regresses every consumer)

1. **DONE - RX read API in transport** - `det_conn_available` / `det_conn_read_byte`
   / `det_conn_read` / `det_conn_peek` / `det_conn_consume` (inline, transport.h).
2. **DONE - migrate the consumers** - HTTP / websocket / telnet / ssh / tls + the
   conn_pool-ring services (modbus / opcua) all drain through the API; no external
   `rx_tail` modulo remains. The read functions consume only; `det_conn_ack_consumed`
   stays the one place that reopens the window (per loop), so draining and ACKing
   each have exactly one owner. HW: 10/10 50 KB byte-exact, backpressure 0.
3. **DONE - slot-aware streaming hooks** - `HttpStreamDataCb(HttpReq*, ...)` +
   per-slot WebDAV PUT state `g_dav_put[MAX_CONNS]`; fixed the concurrent-PUT bug
   (HW: 4 parallel PUTs, distinct payloads, all byte-exact).
4. **DONE - reconcile private-buffer services** - mqtt / ws_client `s_rx_*` are
   single-instance outbound clients (not `conn_pool` server slots); each buffer is
   module-owned and nothing cross-layer touches it, so it is correct as-is. A unified
   client transport is tracked as separate future work, not this refactor.

All four phases are complete: every cross-layer concern (TX, RX window, RX read,
streaming sink state, events, scratch) now has exactly one owner behind a clean API.
