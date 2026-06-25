// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 06.ChunkedResponse.ino
 * @brief Streaming a response of unknown length with chunked transfer.
 *
 * send_chunked() writes the status + headers (Transfer-Encoding: chunked), then
 * invokes a ChunkFiller callback that emits body pieces through a ChunkedResponse
 * writer (write() / write(buf,len) / printf()), then writes the terminating
 * chunk and closes. Each call emits one chunk straight to the socket - the body
 * is never buffered whole, so output size is unbounded with constant memory.
 *
 * Flash, open Serial @ 115200 for the IP, then:
 *   curl -N http://<ip>/stream     # watch 50 lines arrive incrementally
 *   curl -N http://<ip>/count      # printf-built chunks
 */

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

// Filler: emit 50 lines, one chunk each, with no large intermediate buffer.
static void stream_lines(ChunkedResponse &res, HttpReq *req)
{
    (void)req;
    for (int i = 1; i <= 50; i++)
        res.printf("line %d of 50\n", i);
}

// Filler: mix literal writes and formatted writes.
static void stream_count(ChunkedResponse &res, HttpReq *req)
{
    (void)req;
    res.write("counting up:\n");
    for (int i = 0; i <= 10; i++)
        res.printf("  %d\n", i);
    res.write("done\n");
}

void handle_stream(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    server.send_chunked(slot_id, 200, "text/plain", stream_lines);
}

void handle_count(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    server.send_chunked(slot_id, 200, "text/plain", stream_count);
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

    server.on("/stream", HTTP_GET, handle_stream);
    server.on("/count", HTTP_GET, handle_count);

    int32_t result = server.begin(80);
    if (result < 0)
    {
        Serial.printf("begin() failed (error %d)\n", result);
        return;
    }
    Serial.println("Server started on port 80");
}

void loop()
{
    server.handle();
}
