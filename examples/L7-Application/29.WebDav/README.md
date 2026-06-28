# 29.WebDav - a WebDAV file share backed by LittleFS

**Layer:** L7 Application · **Build flags:** `DETWS_ENABLE_WEBDAV`

## What this example teaches

WebDAV (RFC 4918) extends HTTP with file-management methods, so a remote client
can mount and edit the device's filesystem like a network drive.
`server.dav(url_prefix, fs, fs_root)` mounts a LittleFS subtree as a WebDAV share
in one call: here `/dav` on disk is exposed at the URL `/dav`.

**One call mounts the share:**

```cpp
server.dav("/dav", LittleFS, "/dav"); // URL "/dav" -> LittleFS "/dav"
```

Supported methods: `OPTIONS`, `PROPFIND` (Depth 0/1), `GET`, `HEAD`, `PUT`,
`DELETE`, `MKCOL`, `COPY` (files), `MOVE`, and advisory `LOCK`/`UNLOCK`. `PUT`
buffers the body (bounded by `BODY_BUF_SIZE`) and the locks are advisory only.

**Security note.** A writable share is dangerous on an open network: add per-route
auth ([Foundation/04.Sysadmin](../../Foundation/04.Sysadmin)), HTTPS
([L4-Transport/03.HTTPS](../../L4-Transport/03.HTTPS)), and the per-IP throttle
([L4-Transport/05.PerIpThrottle](../../L4-Transport/05.PerIpThrottle)) before
exposing it. The sketch seeds one file so a fresh share is not empty.

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_WEBDAV=1" \
  --lib="." examples/L7-Application/29.WebDav/29.WebDav.ino
```

```sh
curl -X PROPFIND -H "Depth: 1" http://<ip>/dav/        # list
curl -T file.txt http://<ip>/dav/file.txt              # upload (PUT)
curl -X MKCOL http://<ip>/dav/sub                      # make a collection
curl -X MOVE -H "Destination: /dav/b.txt" http://<ip>/dav/file.txt
curl -X DELETE http://<ip>/dav/b.txt
rclone lsd :webdav: --webdav-url http://<ip>/dav --webdav-vendor other
```

## Annotated source

The complete sketch ([29.WebDav.ino](29.WebDav.ino)), reproduced verbatim with
added explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#define DETWS_ENABLE_WEBDAV 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include <LittleFS.h>
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

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

    if (!LittleFS.begin(true)) // format on first run
    {
        Serial.println("LittleFS mount failed");
        return;
    }
    LittleFS.mkdir("/dav");
    // Seed one file so a fresh share is not empty.
    File f = LittleFS.open("/dav/hello.txt", "w");
    if (f)
    {
        f.print("hello from DeterministicESPAsyncWebServer\n");
        f.close();
    }

    // Mount the LittleFS subtree "/dav" as a WebDAV share at URL "/dav".
    server.dav("/dav", LittleFS, "/dav");
    server.begin(80);
    Serial.println("WebDAV share at http://<ip>/dav");
}

void loop()
{
    server.handle();
}
```
