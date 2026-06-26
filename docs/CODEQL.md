# CodeQL static analysis

The C/C++ sources are scanned with [CodeQL](https://codeql.github.com/) for
security and quality issues.

## In CI

The [CodeQL workflow](../.github/workflows/codeql.yml) runs on every push and
pull request to `main` (and weekly). It is an **advanced** setup: the database is
built by tracing a host/native PlatformIO compile of the security-critical cores
(HTTP parser, WebSocket, multipart, base64 / JSON, the SSH crypto stack, SNMP,
and the protocol / codec services), then analyzed with the `security-and-quality`
query suite. Results appear under the repository's **Security -> Code scanning**
tab.

The workflow publishes a code-scanning check named **`CodeQL`** - require that
context in the branch-protection rule / ruleset for `main`. If GitHub "default"
code scanning is enabled for C/C++, disable it (Settings -> Code security -> Code
scanning) so this advanced workflow runs instead.

## Running locally

Create a database by tracing a native build, then analyze it:

```sh
codeql database create cpp-db --language=cpp \
  --command="pio test --without-testing -e native_app -e native_ssh"

codeql database analyze cpp-db \
  codeql/cpp-queries:codeql-suites/cpp-security-and-quality.qls \
  --format=sarif-latest --output=cpp.sarif --download
```

The native environments compile the host-portable code (parsers, codecs, the
software crypto paths). The ESP32-only wrappers - NVS, the mbedTLS / lwIP glue,
mDNS - are thin SDK adapters; CI scans a broader env set (see the workflow).

## Findings

_Latest local scan: `security-and-quality` suite over `native_app` + `native_ssh`
(2026-06-26); 84 of 227 C/C++ files analyzed._

**No security (CWE) findings.** The memory-safety queries (buffer overflow,
integer overflow, use-after-free, tainted path, invalid pointer deref, etc.) all
ran and reported nothing in `src/`. The 18 total results are low-severity
quality / maintainability items - 8 in `src/`, the rest in tests, mocks, and the
vendored Unity headers.

### `src/` results and disposition

| Rule                         | Location                                                            | Disposition                                                                                                                                             |
| ---------------------------- | ------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `cpp/useless-expression`     | `transport/listener.cpp:144`, `:300`                                | **False positive** - `xQueueSend` / `tcp_close` are no-op mocks in the host build (no side effects); on ESP32 they do real work.                        |
| `cpp/unused-static-function` | `presentation/http_parser.cpp:40`                                   | **False positive** - `fnv1a` is consumed at compile time by the `constexpr HASH_HTTP10` / `HASH_HTTP11` initializers, so no runtime call exists to see. |
| `cpp/constant-comparison`    | `presentation/sse.cpp:79`                                           | **Benign** - the first `pos < rem` guard is trivially true (`pos == 0`, buffer non-empty); kept for defensive consistency with the later, real checks.  |
| `cpp/long-switch`            | `http_parser.cpp:187`, `ssh/ssh_server.cpp:39`, `websocket.cpp:279` | **Won't fix (style)** - protocol state machines / message dispatchers; a long `switch` is the clearest form.                                            |
| `cpp/unused-static-variable` | `DetWebServerConfig.h:1519`                                         | **Cleanup candidate** - `DET_DEFAULT_CONFIG` is vestigial (no reader); left for a deliberate config-struct cleanup.                                     |

### Tests / mocks / vendored (not production code)

`cpp/irregular-enum-init` (vendored `Unity/unity_internals.h`),
`cpp/stack-address-escape` (`test/mocks/FS.h`, a test double),
`cpp/offset-use-before-range-check` and the remaining `cpp/unused-*` (under
`test/`). No action - test / third-party code.
