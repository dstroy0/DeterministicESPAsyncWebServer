# 03.Telnet - a line-oriented Telnet console

**Layer:** L5 Session · **Build flags:** `DETWS_ENABLE_TELNET`

## What this example teaches

This opens a Telnet console (RFC 854) on port 23 alongside the HTTP server. The
library negotiates echo + character mode, edits the input line for you
(backspace works), and hands each completed line to a command callback - so you
write a tiny command interpreter, not a byte pump.

> **Plaintext warning.** Telnet has no auth and no encryption. Use it only on a
> trusted LAN; prefer [SSH](../01.SSH) or the WebSocket terminal for anything
> exposed.

**Opening the port.** As with SSH, you add a non-HTTP listener and then
`begin()` (which activates all listeners, here HTTP on 80 too):

```cpp
server.listen(23, PROTO_TELNET); // open the Telnet port
telnet_on_command(on_command);   // register the line handler
server.begin(80);                // also start HTTP; begin() activates every listener
```

**A line-at-a-time command handler.** The callback receives a fully-edited line
and the connection id; reply with `telnet_print` / `telnet_println` /
`telnet_printf`. This example is a minimal shell with `help`, `heap`, `uptime`,
and an echo fallback:

```cpp
void on_command(const char *line, uint8_t conn_id) {
    if (strcmp(line, "heap") == 0)
        telnet_printf("free heap: %u bytes\r\n", ESP.getFreeHeap());
    else if (line[0])
        telnet_printf("echo: %s\r\n", line);
}
```

**Build-flag note.** `DETWS_ENABLE_TELNET` must be set for the whole build; an
in-sketch `#define` does not reach the separately compiled library, so pass it as
a `build_flag` (see the [build_flags gotcha](../../../docs/EXAMPLES.md)).

`WiFi.setSleep(false)` is called so the radio stays responsive for an interactive
session.

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_TELNET=1" \
  --lib="." examples/L5-Session/03.Telnet/03.Telnet.ino
```

Flash, read the IP on Serial, then:

```sh
telnet <ip> 23        # type "help"
```

## Annotated source

The complete sketch ([03.Telnet.ino](03.Telnet.ino)), reproduced verbatim with
added explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#define DETWS_ENABLE_TELNET 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "network_drivers/presentation/telnet/telnet.h" // telnet_on_command / telnet_printf
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

// Called once per completed line (the library handles echo + line editing).
void on_command(const char *line, uint8_t conn_id)
{
    (void)conn_id;
    if (strcmp(line, "help") == 0)
        telnet_println("commands: help, heap, uptime, <echo>");
    else if (strcmp(line, "heap") == 0)
        telnet_printf("free heap: %u bytes\r\n", ESP.getFreeHeap());
    else if (strcmp(line, "uptime") == 0)
        telnet_printf("uptime: %lu ms\r\n", millis());
    else if (line[0])                       // non-empty -> echo it
        telnet_printf("echo: %s\r\n", line);
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
    WiFi.setSleep(false);                    // keep the radio responsive for the session

    server.listen(23, PROTO_TELNET);         // open the Telnet port
    telnet_on_command(on_command);

    server.begin(80);                        // also start HTTP (begin() activates all listeners)
    Serial.println("Telnet on port 23 (try: telnet <ip>)");
}

void loop()
{
    server.handle();
}
```
