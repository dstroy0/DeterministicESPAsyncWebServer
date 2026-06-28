# Migration guide

Breaking changes between major versions, with the minimal edits needed to move
forward.

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
