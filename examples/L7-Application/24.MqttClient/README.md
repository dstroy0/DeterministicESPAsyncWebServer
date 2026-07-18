# 24.MqttClient - the device publishes/subscribes to an MQTT broker

**Layer:** L7 Application · **Build flags:** `DWS_ENABLE_MQTT` (optional `DWS_ENABLE_TLS` + `DWS_ENABLE_MQTT_TLS` for `mqtts://`)

## What this example teaches

A full MQTT 3.1.1 client: the device connects to a broker, SUBSCRIBEs to a topic,
and PUBLISHEs to the same topic once a second at QoS 1 - so it receives its own
messages back through the `on_message` callback, a self-contained round trip you
can watch on Serial. QoS 0/1/2, keep-alive, and DUP retransmit are handled by
`dws_mqtt_loop()`.

**Register the receive callback, then connect with options:**

```cpp
dws_mqtt_set_message_cb(on_message);

MqttConnectOpts opts;
memset(&opts, 0, sizeof(opts));
opts.client_id = "detws-esp32-demo";
opts.keepalive_s = 30;
opts.clean_session = true;
if (dws_mqtt_connect(BROKER, PORT, false, &opts)) // 3rd arg: use TLS?
    dws_mqtt_subscribe(TOPIC, 1);                  // QoS 1
```

**Pump the protocol every loop.** `dws_mqtt_loop()` drives the state machine
(keep-alive PINGs, QoS handshakes, retransmits); skipping it stalls the session:

```cpp
void loop() {
    dws_mqtt_loop();
    if (dws_mqtt_connected() && /* once a second */) {
        dws_mqtt_publish(TOPIC, (const uint8_t *)msg, len, 1, false); // QoS 1, not retained
    }
}
```

`BROKER`/`TOPIC` default to a public test broker; point them at your own broker
for real telemetry or command. For `mqtts://`, pass `true` as the third argument
to `dws_mqtt_connect()` and build with the TLS flags.

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDWS_ENABLE_MQTT=1" \
  --lib="." examples/L7-Application/24.MqttClient/24.MqttClient.ino
```

```sh
# watch the same topic from a host while the device runs:
mosquitto_sub -h broker.hivemq.com -t detws/demo
```

## Annotated source

The complete sketch ([24.MqttClient.ino](24.MqttClient.ino)), reproduced verbatim
with added explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#define DWS_ENABLE_MQTT 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/mqtt/mqtt.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

static const char *BROKER = "broker.hivemq.com"; // public test broker
static const uint16_t PORT = 1883;
static const char *TOPIC = "detws/demo";

// Delivered for every message on a subscribed topic.
void on_message(const char *topic, const uint8_t *payload, size_t len)
{
    Serial.printf("RX [%s]: %.*s\n", topic, (int)len, (const char *)payload);
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

    dws_mqtt_set_message_cb(on_message);

    MqttConnectOpts opts;
    memset(&opts, 0, sizeof(opts));
    opts.client_id = "detws-esp32-demo";
    opts.keepalive_s = 30;
    opts.clean_session = true;

    if (dws_mqtt_connect(BROKER, PORT, false, &opts)) // false = plaintext (true for mqtts://)
    {
        Serial.println("MQTT connected");
        dws_mqtt_subscribe(TOPIC, 1); // QoS 1
    }
    else
    {
        Serial.println("MQTT connect failed");
    }
}

void loop()
{
    dws_mqtt_loop(); // drive keep-alive, QoS handshakes, retransmits

    static uint32_t last = 0;
    static uint32_t n = 0;
    if (dws_mqtt_connected() && millis() - last >= 1000)
    {
        last = millis();
        char msg[48];
        int len = snprintf(msg, sizeof(msg), "hello from esp32 #%lu", (unsigned long)n++);
        dws_mqtt_publish(TOPIC, (const uint8_t *)msg, (size_t)len, 1, false); // QoS 1, not retained
    }
}
```
