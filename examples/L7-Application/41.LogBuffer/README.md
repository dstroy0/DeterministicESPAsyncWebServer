# 41.LogBuffer - a fixed-RAM rotating log buffer with severity traps

**Layer:** L7 Application · **Build flags:** `DETWS_ENABLE_LOGBUF`

## What this example teaches

When there is no serial cable attached, you still want the last few log lines. This
keeps the most recent `DETWS_LOG_LINES` lines in a fixed RAM ring (oldest pruned on
overflow), serves them at `GET /logs`, and fires a trap callback on WARN+ lines -
here it just prints, but a real app could forward an SNMP trap
([26.SnmpTrap](../26.SnmpTrap)) or a webhook ([46.Webhook](../46.Webhook)).

**Set a severity trap, then log by level:**

```cpp
detws_log_set_trap(DETWS_LOG_WARN, on_trap); // trap on WARN and ERROR
detws_log(DETWS_LOG_INFO, "boot complete");
```

The trap fires only for lines at or above the threshold, so criticals get pushed
while routine INFO lines just accumulate in the ring:

```cpp
static void on_trap(uint8_t level, const char *line) {
    Serial.printf("[trap] %s\n", line); // forward criticals here (SNMP trap / webhook)
}
```

**Dump the ring on demand.** `detws_log_dump()` writes the whole buffer into a
caller-owned array sized `DETWS_LOG_LINES * DETWS_LOG_LINE_LEN`:

```cpp
char buf[DETWS_LOG_LINES * DETWS_LOG_LINE_LEN];
detws_log_dump(buf, sizeof(buf));
server.send(id, 200, "text/plain", buf);
```

The loop logs a heap/uptime line every five seconds, escalating to WARN (which
trips the trap) when heap runs low.

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_LOGBUF=1" \
  --lib="." examples/L7-Application/41.LogBuffer/41.LogBuffer.ino
```

```sh
curl http://<ip>/logs   # the last DETWS_LOG_LINES lines, oldest first
```

## Annotated source

The complete sketch ([41.LogBuffer.ino](41.LogBuffer.ino)), reproduced verbatim
with added explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#define DETWS_ENABLE_LOGBUF 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/logbuf/logbuf.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

// Fired for every line at or above the trap threshold.
static void on_trap(uint8_t level, const char *line)
{
    Serial.printf("[trap] %s\n", line); // forward criticals here (SNMP trap / webhook)
    (void)level;
}

void setup()
{
    Serial.begin(115200);
    init_wifi_physical(SSID, PASSWORD);
    while (!wifi_ready())
        delay(250);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    WiFi.setSleep(false);

    detws_log_set_trap(DETWS_LOG_WARN, on_trap); // trap on WARN and ERROR
    detws_log(DETWS_LOG_INFO, "boot complete");

    server.on("/logs", HTTP_GET, [](uint8_t id, HttpReq *) {
        char buf[DETWS_LOG_LINES * DETWS_LOG_LINE_LEN];
        detws_log_dump(buf, sizeof(buf));
        server.send(id, 200, "text/plain", buf);
    });
    server.begin(80);
}

void loop()
{
    static uint32_t last = 0;
    if (millis() - last >= 5000)
    {
        last = millis();
        char msg[64];
        uint32_t heap = ESP.getFreeHeap();
        snprintf(msg, sizeof(msg), "heap=%u uptime=%lus", (unsigned)heap, millis() / 1000);
        detws_log(heap < 20000 ? DETWS_LOG_WARN : DETWS_LOG_INFO, msg); // WARN trips the trap
    }
    server.handle();
}
```
