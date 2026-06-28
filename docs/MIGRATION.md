# Migration guide

Breaking changes, newest first, with the minimal edits needed to move forward.
This library breaks public APIs between releases when correctness or a cleaner
design calls for it (see the development-status notice in the README), so check
here on every upgrade.

## 4.6.x to 4.7.0 - streaming-body data hook is slot-aware

Only affects code that installs the low-level parser streaming hooks directly via
`http_parser_set_stream_hooks()` (OTA / upload / WebDAV do this internally; most
applications never touch it). The `data` callback now receives the `HttpReq *` for
the connection so a sink can keep per-connection state - this is what makes
concurrent streamed uploads (e.g. parallel WebDAV PUTs) write to their own files
instead of clobbering one global transfer.

- `HttpStreamDataCb` changed from `void (*)(const uint8_t *data, size_t len)` to
  `void (*)(HttpReq *req, const uint8_t *data, size_t len)`.
- Update your data hook's signature; derive the slot with `req - http_pool` if you
  need per-connection state (ignore `req` if you only ever stream one at a time).
- `begin` and `abort` hooks are unchanged (they already took `HttpReq *`).

Before: `void on_data(const uint8_t *d, size_t n)`
After: `void on_data(HttpReq *req, const uint8_t *d, size_t n)`

## 4.4.x to 4.5.0 - chunked responses are a pull generator

`send_chunked()` no longer takes a one-shot `ChunkFiller(ChunkedResponse&, HttpReq*)`
that pushes pieces with `res.write()` / `res.printf()`. That model truncated any
body larger than the TCP send buffer (it ignored backpressure). It is replaced by
a pull generator that the server pages across loops, so a body of any size streams
in constant memory without truncation.

- `ChunkedResponse` (and its `write` / `printf` / `total`) is **removed**.
- The new signature is
  `send_chunked(slot, code, content_type, ChunkSource source, void *ctx = nullptr)`.
- `ChunkSource` is `size_t (*)(uint8_t *buf, size_t cap, void *ctx)`: write up to
  `cap` bytes of the next body piece into `buf` and return the count, or `0` to
  end. Track your position in `ctx`.
- `ctx` **must outlive the response** (a large body finishes on a later loop), so
  it must be static / global, not on the handler's stack.

Before:

```cpp
void fill(ChunkedResponse &res, HttpReq *) {
    for (int i = 0; i < 100; i++) res.printf("line %d\n", i);
}
server.send_chunked(slot, 200, "text/plain", fill);
```

After:

```cpp
struct Ctx { int i; };
size_t src(uint8_t *buf, size_t cap, void *vctx) {
    Ctx *c = (Ctx *)vctx;
    if (c->i >= 100) return 0;
    return (size_t)snprintf((char *)buf, cap, "line %d\n", c->i++);
}
static Ctx ctx; ctx.i = 0;            // static: outlives the response
server.send_chunked(slot, 200, "text/plain", src, &ctx);
```

## 3.x to 4.0.0

v4.0.0 is a structural and API-cleanup release. There are no behavioral changes
to the wire protocols; the breaks are include paths, two removed methods, and one
dispatch-default change.

### 1. Presentation codec include paths

Each presentation codec moved into its own subfolder (mirroring `services/*` and
`presentation/ssh/`). The layer dispatcher (`presentation.h`/`presentation.cpp`)
stays at the `presentation/` root and is unchanged. If you include a codec header
directly, update the path:

| Old (3.x)                                    | New (4.0.0)                                              |
| -------------------------------------------- | -------------------------------------------------------- |
| `network_drivers/presentation/base64.h`      | `network_drivers/presentation/base64/base64.h`           |
| `network_drivers/presentation/cbor.h`        | `network_drivers/presentation/cbor/cbor.h`               |
| `network_drivers/presentation/deflate.h`     | `network_drivers/presentation/deflate/deflate.h`         |
| `network_drivers/presentation/http_parser.h` | `network_drivers/presentation/http_parser/http_parser.h` |
| `network_drivers/presentation/inflate.h`     | `network_drivers/presentation/inflate/inflate.h`         |
| `network_drivers/presentation/json.h`        | `network_drivers/presentation/json/json.h`               |
| `network_drivers/presentation/msgpack.h`     | `network_drivers/presentation/msgpack/msgpack.h`         |
| `network_drivers/presentation/multipart.h`   | `network_drivers/presentation/multipart/multipart.h`     |
| `network_drivers/presentation/sha1.h`        | `network_drivers/presentation/sha1/sha1.h`               |
| `network_drivers/presentation/sse.h`         | `network_drivers/presentation/sse/sse.h`                 |
| `network_drivers/presentation/telnet.h`      | `network_drivers/presentation/telnet/telnet.h`           |
| `network_drivers/presentation/websocket.h`   | `network_drivers/presentation/websocket/websocket.h`     |

Including the umbrella header `DeterministicESPAsyncWebServer.h` requires no
change; it pulls in the codecs it needs through the new paths.

### 2. Removed methods: `heap_needed()` / `heap_available()`

`DetWebServer::heap_needed()`, `DetWebServer::heap_available()`, and the
`DeterministicAsyncTCP` methods of the same name were no-op shims (they always
returned `0` and `true`): the library makes no heap allocations, so there was
nothing to pre-flight. They are removed.

If you called them, delete the call. The pre-flight pattern is no longer needed:

```cpp
// 3.x
if (!DetWebServer::heap_available()) { /* bail */ }
server.begin(80);

// 4.0.0
server.begin(80);
```

### 3. `PROTO_NONE` no longer falls back to HTTP

A connection slot is assigned its protocol from its listener on accept, and
`listen()` defaults to `PROTO_HTTP`:

```cpp
int32_t listen(uint16_t port, ConnProto proto = PROTO_HTTP);
```

In 3.x a slot left at `PROTO_NONE` (or carrying an unregistered protocol) was
dispatched as HTTP. That implicit fallback is removed: such a slot now resolves
to no handler and its events are dropped. Normal servers are unaffected because
every accepted slot already carries an explicit protocol. The only code that
needs updating is test or harness code that builds a `conn_pool` slot by hand and
relied on the fallback; set the protocol explicitly:

```cpp
conn_pool[i].state = CONN_ACTIVE;
conn_pool[i].proto = PROTO_HTTP; // required in 4.0.0
```
