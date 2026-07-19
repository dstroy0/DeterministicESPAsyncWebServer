// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file FileUpload.ino
 * @brief Streaming file upload: POST a body straight into a LittleFS file.
 *
 * The request body is streamed into a file in FILE_CHUNK_SIZE pieces via the
 * parser's streaming-body hook (the same mechanism OTA uses), so an upload never
 * has to fit in RAM. A GET route serves the stored file back so you can verify.
 *
 * Flash, open Serial @ 115200 for the IP, then:
 *   curl --data-binary @somefile.bin http://<ip>/upload     # 200 OK <n> bytes
 *   curl http://<ip>/file > roundtrip.bin                   # read it back
 *
 * NOTE: optional services are gated by a compile flag the *library* sources must
 * also see; for PlatformIO enable it for the whole build, e.g.:
 *     build_flags = -DDWS_ENABLE_UPLOAD=1 -DRX_BUF_SIZE=2048
 * (Arduino IDE: they are already set for you in the build_opt.h beside this sketch, so it builds as-is.) The upload
 * sink shares the parser streaming hook with OTA - enable one or the other, not both.
 *
 * RX_BUF_SIZE must exceed the largest inbound TCP segment (TCP_MSS, ~1460) so a
 * full segment fits the receive ring; the transport refuses+redelivers a segment
 * that will not fit (no data loss) but a ring smaller than one segment would
 * stall. The default 1024 is fine for ordinary requests but too small here.
 */

#define DWS_ENABLE_UPLOAD 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/upload_service/upload_service.h"
#include <LittleFS.h>
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

static const char *DEST = "/uploaded.bin";

DWS server;

void setup()
{
    Serial.begin(115200);

    if (!LittleFS.begin(true)) // format on first use
        Serial.println("LittleFS mount failed");

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

    // POST /upload -> stream the body into DEST on LittleFS.
    dws_upload_begin(server, "/upload", LittleFS, DEST);

    // GET /file -> serve the stored file back.
    server.on("/file", HttpMethod::HTTP_GET,
              [](uint8_t id, HttpReq *) { server.serve_file(id, LittleFS, DEST, "application/octet-stream"); });

    int32_t result = server.begin(80);
    if (result < 0)
        Serial.printf("begin() failed (error %d)\n", result);
    else
        Serial.println("Upload server on :80 (curl --data-binary @file http://<ip>/upload)");
}

void loop()
{
    server.handle();
}
