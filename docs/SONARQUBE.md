# SonarQube / SonarCloud analysis

This repository is analyzed by SonarCloud. Because the codebase is C/C++, the
analysis is **CI-based** (a GitHub Actions workflow), not SonarCloud's "Automatic
Analysis": automatic analysis cannot analyze C/C++ because it has no build to
observe. The C/C++ analyzer needs a **compilation database**
(`compile_commands.json`) describing how each source file is compiled.

## One-time setup

1. **Disable Automatic Analysis.** In the SonarCloud project go to
   _Administration -> Analysis Method_ and turn **off** Automatic Analysis. If it
   stays on, the CI scan is rejected with a "you already have automatic analysis
   running" error.
2. **Add the token.** Create a SonarCloud _user/project token_ and add it as the
   repository secret **`SONAR_TOKEN`** (Settings -> Secrets and variables ->
   Actions). The [`SonarCloud` workflow](../.github/workflows/sonarcloud.yml) is a
   no-op until this secret exists.
3. **Check the keys.** Confirm `sonar.projectKey` and `sonar.organization` in
   [`sonar-project.properties`](../sonar-project.properties) match your SonarCloud
   project (SonarCloud -> _Information_). The defaults follow SonarCloud's
   GitHub-import convention (`<owner>_<repo>` and `<owner>`).

## How the compilation database is built

No single build enables all ~111 `DETWS_ENABLE_*` features, so a feature-gated
source file is only compiled in the env whose flag turns it on. To give Sonar a
command for **every** file, [`tools/sonar/gen_compiledb.sh`](../tools/sonar/gen_compiledb.sh)
runs `pio run -t compiledb` for each native env, and
[`tools/sonar/merge_compiledb.py`](../tools/sonar/merge_compiledb.py) merges the
per-env fragments into one `compile_commands.json` (first env to compile a file
wins). The CI workflow runs this so the database's absolute `directory` matches
the runner's checkout; `compile_commands.json` itself is git-ignored.

Run it locally the same way (with PlatformIO on `PATH`):

```bash
bash tools/sonar/gen_compiledb.sh   # writes ./compile_commands.json
```

> Cost note: merging every env means one `compiledb` build per native env, which
> is the slow part of the CI job (the PlatformIO cache mitigates re-runs). If that
> is too slow, a single all-features build is a faster, slightly less complete
> alternative.

## Coverage

The merged database covers every source file the host (native) toolchain compiles
(115 of the library's `.cpp` files as of this writing). The handful it does not
cover are ESP32/platform-only and never built natively: the TLS engine
(`det_tls.cpp`), the lwIP datalink/network shims, the ESP-IDF mDNS/OTA services,
the TCP client, and the dashboard/gpio/partition route registrars. To analyze
those too, add an ESP32 `compiledb` (cross-toolchain) fragment to the merge; it
needs the `espressif32` toolchain in the CI job.
