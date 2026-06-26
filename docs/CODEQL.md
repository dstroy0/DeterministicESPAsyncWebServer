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
(2026-06-26)._

<!-- FINDINGS -->
