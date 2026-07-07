# 18.SNTP - wall-clock time via SNTP

**Layer:** L7 Application · **Build flags:** `DETWS_ENABLE_NTP`

## What this example teaches

An ESP32 boots with no idea what time it is. `detws_ntp_begin(tz)` starts the
ESP-IDF SNTP client (the first sync lands a few seconds later), and
`detws_ntp_http_date()` formats the current time as an RFC 7231 date string -
the same format HTTP `Date`/`Last-Modified` headers use. `GET /time` returns it,
or `503` until the first sync completes.

**Start the client, then format on demand.**

```cpp
detws_ntp_begin("UTC0"); // POSIX TZ string; set your zone for local time
```

```cpp
char date[40];
if (detws_ntp_http_date(date, sizeof(date)) == 0) {  // 0 = not synced yet
    server.send(slot_id, 503, "text/plain", "Time not synced yet");
    return;
}
server.send(slot_id, 200, "text/plain", date);
```

`detws_ntp_http_date()` returns 0 until the clock is set, so the handler can
distinguish "no time yet" from a real value and answer `503` in the meantime. The
TZ argument is a POSIX TZ string ("UTC0", "EST5EDT", "CET-1CEST", ...) so the
formatted time can be local.

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_NTP=1" \
  --lib="." examples/L7-Application/18.SNTP/18.SNTP.ino
```

```sh
curl http://<ip>/time   # 503 for the first few seconds, then an RFC 7231 date
```

## Annotated source

The complete sketch ([18.SNTP.ino](18.SNTP.ino)), reproduced verbatim with added
explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#define DETWS_ENABLE_NTP 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/ntp_service.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

void handle_time(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    char date[40];
    if (detws_ntp_http_date(date, sizeof(date)) == 0) // 0 -> clock not set yet
    {
        server.send(slot_id, 503, "text/plain", "Time not synced yet");
        return;
    }
    server.send(slot_id, 200, "text/plain", date);
}

void setup()
{
    Serial.begin(115200);
    init_wifi_physical(SSID, PASSWORD);
    Serial.print("Connecting to WiFi");
    while (!wifi_ready())
    {
        delay(250);
        Serial.print('.');
    }
    Serial.print("\nIP: ");
    Serial.println(WiFi.localIP());
    WiFi.setSleep(false);

    server.on("/time", HTTP_GET, handle_time);
    server.begin(80);

    detws_ntp_begin("UTC0"); // POSIX TZ string; set your zone for local time
}

void loop()
{
    server.handle();
}
```
