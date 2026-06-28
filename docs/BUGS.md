# Bug log

A running record of every bug found in this library: what broke, the root cause,
the fix, and status. Newest first. A bug is logged here the moment it is found
(even before it is fixed) so nothing slips.

Status key: **OPEN** (found, not fixed) - **FIXED** (fixed, validated) - **SHIPPED** (released).

---

## RX flow-control deadlock on streamed uploads (WebDAV PUT)

- **Status:** FIXED (v4.6.0; host + HW validated)
- **Found:** 2026-06-28, stress-testing WebDAV streaming PUT on hardware.
- **Symptom:** large PUTs intermittently hung ~20 s (curl timeout), stored 0 bytes,
  and repeated hangs eventually wedged every slot (device pings but HTTP is dead).
- **Root cause:** `recv_cb` ACKed received data on **copy** (`tcp_recved` at copy
  time), decoupling the advertised TCP window from how full the ring actually was.
  A slow consumer (flash writes) let the ring fill to `RX_BUF_SIZE`; the next
  segment was refused (`ERR_MEM` -> lwIP `refused_data`). When `RX_BUF_SIZE <
TCP_WND` the ring can never hold a full receive window, so refusals were constant
  and lwIP's refused-data redelivery raced fatally. Serial trace nailed it: failing
  PUTs stalled at exactly `bytes_written == RX_BUF_SIZE`. This was an **interlayer**
  bug: the receive-window invariant was smeared across transport (ACK on copy),
  presentation (drains the ring) and session (worker loop), with no single owner.
- **Fix:** ack-on-consume, owned entirely by transport. `recv_cb` no longer ACKs;
  the worker calls `det_conn_ack_consumed(slot)` once per loop and transport
  reopens the window by exactly the bytes drained since the last ACK
  (`tcp_recved` marshaled to tcpip_thread). The window now tracks ring occupancy,
  so a slow sink cannot overflow the ring. **TCP-level requirement:** the ring must
  hold at least one receive window (`RX_BUF_SIZE >= TCP_WND`); a smaller ring is a
  configuration error (you cannot advertise a window larger than your buffer).
- **HW proof:** RX=8192 (>= TCP_WND) + ack-on-consume -> 10/10 50 KB byte-exact,
  `backpressure=0`. Pre-fix: RX=2048 -> ~40% hang + permanent wedge.

## Boot stack-overflow when RX_BUF_SIZE is large (pool_init)

- **Status:** FIXED (v4.6.0; host validated)
- **Found:** 2026-06-28, while testing large rings for the deadlock above.
- **Symptom:** "Stack canary watchpoint triggered (loopTask)" at boot
  (`begin()` -> `pool_init` -> `memset`) once `RX_BUF_SIZE` was set large (e.g. 8192).
- **Root cause:** `conn_pool[i] = {}` materializes a full `sizeof(TcpConn)`
  temporary - the entire `rx_buffer[RX_BUF_SIZE]` - on the loopTask stack.
- **Fix:** reset from a single `static const TcpConn blank = {}` in BSS via
  copy-assign (uses `DetAtomic::operator=`, no atomic-memset UB); no large stack
  temporary.

## WebDAV streamed PUT leaks the file handle on abort

- **Status:** FIXED (v4.6.0)
- **Found:** 2026-06-28, investigating the deadlock cascade to a permanent wedge.
- **Symptom:** a PUT torn down before completion (peer reset / timeout / the
  deadlock above) never closed `g_dav_put_file`; after a few, LittleFS ran out of
  open-file slots and `open()` failed ("no permits for creation").
- **Root cause:** the file was closed only on the PARSE_COMPLETE handler path; no
  cleanup hook for an aborted stream.
- **Fix:** added `HttpStreamAbortCb` to the parser stream hooks, fired from
  `http_parser_reset` when `body_streaming && parse_state != PARSE_COMPLETE`.
  WebDAV registers it and closes the half-written file. The completion path now
  also clears `g_dav_put_active` so the abort hook cannot double-close.

## WebDAV concurrent streamed PUTs clobber each other

- **Status:** OPEN
- **Found:** 2026-06-28, concurrent (4x) 50 KB PUT stress test.
- **Symptom:** 4 simultaneous PUTs -> one 201, the rest 409/timeout (the file was
  not actually written for the losers).
- **Root cause:** the WebDAV streaming-PUT state (`g_dav_put_file/active/error/...`)
  is a single global, and `HttpStreamDataCb` carries no slot/connection, so the
  data hook cannot route bytes to per-connection state. Overlapping PUTs share and
  clobber the one global transfer.
- **Planned fix:** make the streaming-body data/abort hooks slot-aware (pass the
  `HttpReq*`/slot) and hold per-slot WebDAV PUT state (`g_dav_put[MAX_CONNS]`), so
  each connection streams to its own file. Aligns with the no-spaghetti directive
  (the data hook lacking connection context is the interlayer gap).

## TX truncation on large responses (serve_file, send_chunked)

- **Status:** SHIPPED (serve_file v4.4.1; send_chunked v4.5.0)
- **Found:** 2026-06-28, stress-testing file/chunked GET on hardware.
- **Symptom:** any file/WebDAV GET or chunked/SSE body larger than ~`TCP_SND_BUF`
  (~5.7 KB) was truncated.
- **Root cause:** the senders called `det_conn_send()` and ignored the return; once
  the TCP send buffer filled, the remainder was silently dropped. Hidden on host
  because the mock `tcp_sndbuf` is constant and `tcp_write` never returns `ERR_MEM`.
- **Fix:** per-slot send continuations (`file_send_pump` / `chunk_send_pump`) that
  page out one send-window per worker loop and resume on the sent callback; the
  chunked API became a pull generator (`ChunkSource`) so it can resume across loops.
