# 35.Dashboard - a real-time SVG dashboard with live telemetry and controls

**Layer:** L7 Application · **Build flags:** `DETWS_ENABLE_DASHBOARD` (SSE + WebSocket, both on by default)

## What this example teaches

This ties several layers together into a finished feature: a fixed compile-time
widget table served at `/dashboard`. Display widgets (gauge / value / chart) update
live over SSE; control widgets (toggle / slider / button) send values back to the
device over WebSocket, delivered to a control callback. The page itself is rendered
by the library - you only declare widgets and feed data.

**Widgets are a compile-time table - the deterministic source of truth.** Each
entry is type, label, data key, min, max, and unit:

```cpp
static const DetwsWidget WIDGETS[] = {
    {DETWS_WIDGET_GAUGE,  "Free heap",  "heap",   0, 320000, "B"},
    {DETWS_WIDGET_VALUE,  "Uptime",     "uptime", 0, 0,      "s"},
    {DETWS_WIDGET_CHART,  "WiFi RSSI",  "rssi",  -100, 0,    "dBm"},
    {DETWS_WIDGET_TOGGLE, "Onboard LED","led",    0, 1,      ""},
    {DETWS_WIDGET_SLIDER, "Brightness", "bright", 0, 255,    ""},
    {DETWS_WIDGET_BUTTON, "Identify",   "ident",  0, 0,      ""},
};
```

**Feed display widgets; receive control widgets.** A control callback fires when
the browser moves a control:

```cpp
static void on_control(const char *key, float value) {
    if (strcmp(key, "led") == 0)    digitalWrite(LED_PIN, value >= 0.5f ? HIGH : LOW);
    else if (strcmp(key, "bright") == 0) analogWrite(LED_PIN, (int)value);
}

detws_dashboard_on_control(on_control);
detws_dashboard_begin(server, "/dashboard", WIDGETS, sizeof(WIDGETS) / sizeof(WIDGETS[0]));
```

Then push a frame periodically; `set()` stages a value by key, `publish()` flushes
the frame to every connected browser over SSE:

```cpp
detws_dashboard_set("heap", (float)ESP.getFreeHeap());
detws_dashboard_set("uptime", (float)(now / 1000));
detws_dashboard_set("rssi", (float)WiFi.RSSI());
detws_dashboard_publish();
```

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_DASHBOARD=1" \
  --lib="." examples/L7-Application/35.Dashboard/35.Dashboard.ino
```

Flash, then open `http://<ip>/dashboard` in a browser: the gauges and chart update
live and the toggle/slider/button drive the LED.

## Annotated source

The complete sketch ([35.Dashboard.ino](35.Dashboard.ino)), reproduced verbatim
with added explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#define DETWS_ENABLE_DASHBOARD 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/dashboard/dashboard.h"
#include <WiFi.h>
#include <math.h>
#include <string.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

static const int LED_PIN = 2; // onboard LED on many ESP32 dev boards

// Compile-time widget table - the dashboard's deterministic source of truth.
// Display widgets are fed over SSE; control widgets send values back over WS.
static const DetwsWidget WIDGETS[] = {
    {DETWS_WIDGET_GAUGE, "Free heap", "heap", 0, 320000, "B"}, {DETWS_WIDGET_VALUE, "Uptime", "uptime", 0, 0, "s"},
    {DETWS_WIDGET_CHART, "WiFi RSSI", "rssi", -100, 0, "dBm"}, {DETWS_WIDGET_TOGGLE, "Onboard LED", "led", 0, 1, ""},
    {DETWS_WIDGET_SLIDER, "Brightness", "bright", 0, 255, ""}, {DETWS_WIDGET_BUTTON, "Identify", "ident", 0, 0, ""},
};

// Invoked when a control widget sends a value from the browser.
static void on_control(const char *key, float value)
{
    Serial.printf("control %s = %.1f\n", key, (double)value);
    if (strcmp(key, "led") == 0)
        digitalWrite(LED_PIN, value >= 0.5f ? HIGH : LOW);
    else if (strcmp(key, "bright") == 0)
        analogWrite(LED_PIN, (int)value);
    // "ident" is momentary - just log it above.
}

void setup()
{
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
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

    detws_dashboard_on_control(on_control);
    detws_dashboard_begin(server, "/dashboard", WIDGETS, sizeof(WIDGETS) / sizeof(WIDGETS[0]));
    server.begin(80);
}

void loop()
{
    server.handle();

    // Push a telemetry frame once a second.
    static uint32_t last_ms = 0;
    uint32_t now = millis();
    if (now - last_ms >= 1000)
    {
        last_ms = now;
        detws_dashboard_set("heap", (float)ESP.getFreeHeap());
        detws_dashboard_set("uptime", (float)(now / 1000));
        detws_dashboard_set("rssi", (float)WiFi.RSSI());
        detws_dashboard_publish();
    }
}
```
