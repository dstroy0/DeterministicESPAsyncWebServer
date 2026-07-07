# Contributing

Thanks for your interest in DeterministicESPAsyncWebServer. This is a zero-heap,
OSI-layered, RFC-faithful network server for the ESP32. The bar for changes is
high because the whole point of the library is predictability: fixed buffers, no
runtime allocation, bounded latency, and behavior that matches the relevant
specification. The notes below keep contributions aligned with that.

## Ground rules

- The project is licensed **AGPL-3.0-or-later**. By contributing you agree your
  contribution is licensed under the same terms.
- Every new source file starts with the standard header:

    ```cpp
    // Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
    // SPDX-License-Identifier: AGPL-3.0-or-later
    ```

- Be excellent to each other. See the [Code of Conduct](CODE_OF_CONDUCT.md).

## Development environment

- [PlatformIO](https://platformio.org/) is the build system (CLI or the VS Code
  extension). Most logic is host-testable, so you do not need an ESP32 to start.
- Node is only needed for the docs/spelling tooling. After `npm install` you get
  `npm run format`, `npm run format:check`, and `npm run spell`.

## Build and test

The architecture is deliberately split so the logic compiles and runs on your
host machine, separate from the `#ifdef ARDUINO` hardware wrappers.

- **Native tests** (fast, no hardware): every feature has a `native_*` test
  environment. Run one with:

    ```sh
    pio test -e <native_env>      # pick the env for the area you touched
    ```

    The `native_*` env blocks in [platformio.ini](../platformio.ini) are
    **generated** from a single table, [test/test_matrix.json](../test/test_matrix.json).
    To add or change a test env, edit that table (each entry keeps its own flags,
    src filter, and test dirs - per-feature isolation is the point) and regenerate:

    ```sh
    python3 test/gen_test_envs.py            # rewrite the env blocks in platformio.ini
    python3 test/gen_test_envs.py --check    # CI: fail if the ini is out of date
    ```

    Do not hand-edit the generated region of `platformio.ini`. The full suite
    (`test/run_tests.sh`) auto-discovers every native env, so a new entry is run
    and reported automatically.

- **Compile for hardware:**

    ```sh
    pio run -e esp32dev
    ```

- **Build an example.** Examples are gated behind feature flags, and `pio ci`
  only applies a sketch's `#define` to the sketch translation unit, not to the
  separately compiled library. Pass the feature flags as `build_flags` so the
  library compiles with them too:

    ```sh
    pio ci --board=esp32dev \
      --project-option="framework=arduino" \
      --project-option="build_flags=-DDETWS_ENABLE_WEBSOCKET=1" \
      --lib="." examples/L6-Presentation/09.WebSocket/09.WebSocket.ino
    ```

    This is the single most common "it builds for me but CI fails" gotcha. See
    also [docs/EXAMPLES.md](../docs/EXAMPLES.md) when present.

New logic must ship with a native test. Make it work, then optimize, then re-run
the tests to confirm no regression.

## Code style

- **Formatting is enforced.** C/C++/`.ino` follow
  [.clang-format](../.clang-format) (4-space indent, 120 columns). Markdown is
  formatted with Prettier. Run both before opening a PR:

    ```sh
    clang-format -i <files-you-changed>
    npm run format
    ```

- `snake_case` for identifiers; short names where context is obvious.
- **American English** in code, identifiers, comments, and docs.
- **No `<stdlib.h>` / `<cstdlib>` in library code.** Parse by hand; do not pull
  in `malloc`, `atoi`, `strtoll`, `strtod`, and friends. `stdio`/`string` are
  fine.
- No dynamic allocation in the request path. Every buffer is sized at compile
  time and bounds-checked.
- **Follow the relevant RFC.** Implement correct framing, headers, status codes,
  and field syntax, and cite the RFC and section in code and docs (the codebase
  already does this, for example RFC 7230, 6455, 7233, 5424, 7252). If you must
  deviate for a constraint, document it as an explicit deviation.

## Build flags

Features are opt-in via `DETWS_ENABLE_*` flags (default off) in
[src/ServerConfig.h](../src/ServerConfig.h). Some features depend on
others; illegal combinations fail at compile time with a clear `#error`. If you
add a feature that builds on another, add the matching dependency guard and
update the build-flag tree in the README.

## Commits and pull requests

- **Conventional Commits.** Use `feat:`, `fix:`, `docs:`, `ci:`, `chore:`,
  `refactor:`, `test:`, etc. The changelog is generated from commit history, so
  the prefix matters.
- Keep PRs focused. One logical change per PR.
- Fill out the pull request template. CI runs clang-format, Prettier, cspell,
  CodeQL, the native test suites, and the pentest suite; all must pass.
- New features need: implementation, a native test, a compile check, and an
  example. Hardware verification is appreciated but the maintainer will confirm
  on real hardware before release.

## Reporting bugs and security issues

- Functional bugs: open an issue using the bug report form.
- Security vulnerabilities: do **not** open a public issue. See
  [SECURITY.md](SECURITY.md).
