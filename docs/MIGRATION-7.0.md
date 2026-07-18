# Migrating to 7.0.0 — the `dws_` house prefix

7.0.0 is a **breaking, mechanical rename**: the whole public API moves onto one house prefix, `dws`
(DeterministicWebServer). There are **no behavioural changes** — every function does exactly what it did
before under its 6.x name. The rename is a pure `sed`-able substitution, so migrating a sketch is fast.

## The rules

| 6.x                                      | 7.0.0            | example                                                                                                        |
| ---------------------------------------- | ---------------- | -------------------------------------------------------------------------------------------------------------- |
| `det_*`, `detws_*` (functions/vars)      | `dws_*`          | `detws_millis()` → `dws_millis()`, `det_conn_send()` → `dws_conn_send()`                                       |
| `Det*` (PascalCase types)                | `DWS*`           | `DetIp` → `DWSIp`, `DetSb` → `DWSSb`, `DetRelay` → `DWSRelay`                                                  |
| **the server class** `DetWebServer`      | **`DWS`**        | `DetWebServer server;` → `DWS server;` (and `DetWebServerResult` → `DWSResult`)                                |
| `DETWS_*`, `DET_*` (macros)              | `DWS_*`          | `DETWS_ENABLE_SSH` → `DWS_ENABLE_SSH`, `DET_AES_SBOX` → `DWS_AES_SBOX`                                         |
| build flags `-DDETWS_ENABLE_*`           | `-DDWS_ENABLE_*` | in your `build_opt.h` / `platformio.ini`                                                                       |
| bare-prefixed protocol/sensor/codec APIs | `dws_<name>_*`   | `modbus_frame_parse()` → `dws_modbus_frame_parse()`, `snmp_*`, `opcua_*`, `ina219_*`, `hpack_*`, `base64_*`, … |

**Unchanged on purpose:** file names (headers are still `snmp_agent.h`, `sqlite_format.h`, …), include
guards, the standard `intN_t`/`uintN_t` types, and a handful of generic-internal prefixes
(`http_ ws_ net_ server_ auth_ cache_ …`) that were never part of the public surface.

## Migrating a sketch

1. **Build flags** — in your `build_opt.h`, replace `DETWS_ENABLE_` with `DWS_ENABLE_`:
    ```
    sed -i 's/DETWS_ENABLE_/DWS_ENABLE_/g' build_opt.h
    ```
2. **The class + API** — the two substitutions that cover almost everything:
    ```
    sed -i -E 's/\bDetWebServer\b/DWS/g; s/\bdet_/dws_/g; s/\bdetws_/dws_/g; s/\bDet([A-Z])/DWS\1/g' your_sketch.ino
    ```
3. Rebuild. Any remaining unresolved name is a bare-prefixed peripheral call — prefix it with `dws_`
   (e.g. `modbus_…` → `dws_modbus_…`). The configurator (`docs/configurator.html`) and the README feature
   tables list every current flag/symbol under its 7.0 name.

No other changes are required — the behaviour, wire protocols, and defaults are identical to 6.30.
