# 42.ConfigExport - schema-driven config export / restore

**Layer:** L7 Application · **Build flags:** `DETWS_ENABLE_CONFIG_STORE`, `DETWS_ENABLE_CONFIG_IO`

## What this example teaches

Backing up or bulk-provisioning a fleet needs the device's persisted settings in a
portable form. This declares a schema of persisted fields and exposes them as a
text blob: `GET /config` dumps `key=value` lines for backup or migration, and
`POST /config` with that body restores them into NVS. It is schema-driven over the
typed NVS config store - deterministic and zero-heap.

**Declare the schema once, with field types:**

```cpp
static const DetwsCfgField SCHEMA[] = {
    {"hostname",  DETWS_CFG_STR},
    {"http_port", DETWS_CFG_U32},
    {"location",  DETWS_CFG_STR},
};
```

**Export and import drive off that schema.** The config store is opened under a
namespace ("app") and seeded; export serializes exactly the schema fields, import
parses them back:

```cpp
detws_config_export("app", SCHEMA, SCHEMA_N, buf, sizeof(buf));            // GET -> key=value lines
int n = detws_config_import("app", SCHEMA, SCHEMA_N, req->body, req->body_len); // POST -> n fields restored
```

Because both sides share the schema, an unknown or mistyped key in the import body
is rejected rather than silently written.

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_CONFIG_STORE=1 -DDETWS_ENABLE_CONFIG_IO=1" \
  --lib="." examples/L7-Application/42.ConfigExport/42.ConfigExport.ino
```

```sh
curl http://<ip>/config                       # back up: hostname=sensor-01 ...
curl -X POST http://<ip>/config --data-binary @config.txt   # restore into NVS
```

## Annotated source

The complete sketch ([42.ConfigExport.ino](42.ConfigExport.ino)), reproduced
verbatim with added explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#define DETWS_ENABLE_CONFIG_STORE 1
#define DETWS_ENABLE_CONFIG_IO 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include "services/config_io/config_io.h"
#include "services/config_store/config_store.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

// The persisted fields to back up / restore (shared by export and import).
static const DetwsCfgField SCHEMA[] = {
    {"hostname", DETWS_CFG_STR},
    {"http_port", DETWS_CFG_U32},
    {"location", DETWS_CFG_STR},
};
static const size_t SCHEMA_N = sizeof(SCHEMA) / sizeof(SCHEMA[0]);

void setup()
{
    Serial.begin(115200);
    init_wifi_physical(SSID, PASSWORD);
    while (!wifi_ready())
        delay(250);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    WiFi.setSleep(false);

    // Seed a couple of values (normally set at provisioning).
    detws_config_begin("app");
    detws_config_set_str("hostname", "sensor-01");
    detws_config_set_u32("http_port", 80);
    detws_config_set_str("location", "lab");

    server.on("/config", HTTP_GET, [](uint8_t id, HttpReq *) {
        char buf[512];
        detws_config_export("app", SCHEMA, SCHEMA_N, buf, sizeof(buf));
        server.send(id, 200, "text/plain", buf);
    });
    server.on("/config", HTTP_POST, [](uint8_t id, HttpReq *req) {
        int n = detws_config_import("app", SCHEMA, SCHEMA_N, (const char *)req->body, req->body_len);
        char msg[48];
        snprintf(msg, sizeof(msg), "imported %d field(s)\n", n);
        server.send(id, 200, "text/plain", msg);
    });
    server.begin(80);
}

void loop()
{
    server.handle();
}
```
