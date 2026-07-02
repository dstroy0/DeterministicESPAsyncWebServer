// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 49.AuditLog.ino
 * @brief Tamper-evident, hash-chained audit log (DETWS_ENABLE_AUDIT_LOG).
 *
 * Records security-relevant events in an append-only log where each record
 * chains SHA-256(prev_hash || fields). Any tampering with a retained record
 * breaks the chain, which /audit reports via "intact":false + the first broken
 * sequence number.
 *
 *   GET /login?user=alice&pass=secret  -> logs auth / auth_fail
 *   GET /config?http_port=8080         -> logs a config change
 *   GET /audit                         -> JSON chain dump + integrity status
 *
 * A sink forwards every record - at the moment it is created, before the RAM
 * ring can ever evict it - to a durable / remote store. Here it just prints to
 * Serial; the commented lines show writing to an SD-card file or POSTing to a
 * log service. Because the sink gets the full record (including its chain hash),
 * the external copy keeps the same tamper-evident chain.
 *
 * NOTE: enable it for the whole build. In platformio.ini:
 *     build_flags = -DDETWS_ENABLE_AUDIT_LOG=1
 * (Arduino IDE: it is already set for you in the build_opt.h beside this sketch, so it builds as-is.)
 */

#define DETWS_ENABLE_AUDIT_LOG 1

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/physical/physical.h"
#include "services/audit_log/audit_log.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

// Durable forwarding: runs once per record at append time. Point it wherever you
// keep authoritative logs.
static void audit_sink(const DetwsAuditEntry *e)
{
    char line[256];
    if (detws_audit_format(e, line, sizeof(line)) > 0)
    {
        Serial.print("[AUDIT] ");
        Serial.println(line);
        // SD card:   File f = SD.open("/audit.log", FILE_APPEND); f.println(line); f.close();
        // Log svc:   detws_webhook_post("http://logs.example/ingest", line);  // DETWS_ENABLE_WEBHOOK
    }
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

    detws_audit_reset();
    detws_audit_set_sink(audit_sink);
    detws_audit_append(DETWS_AUDIT_SYSTEM, "boot");

    server.on("/login", HTTP_GET, [](uint8_t id, HttpReq *req) {
        const char *user = http_get_query(req, "user");
        const char *pass = http_get_query(req, "pass");
        char msg[DETWS_AUDIT_MSG_LEN];
        bool ok = pass && strcmp(pass, "secret") == 0;
        snprintf(msg, sizeof(msg), "login %s", user ? user : "?");
        detws_audit_append(ok ? DETWS_AUDIT_AUTH : DETWS_AUDIT_AUTH_FAIL, msg);
        server.send(id, ok ? 200 : 401, "application/json", ok ? "{\"ok\":true}" : "{\"ok\":false}");
    });

    server.on("/config", HTTP_GET, [](uint8_t id, HttpReq *req) {
        const char *port = http_get_query(req, "http_port");
        char msg[DETWS_AUDIT_MSG_LEN];
        snprintf(msg, sizeof(msg), "set http_port=%s", port ? port : "?");
        detws_audit_append(DETWS_AUDIT_CONFIG, msg);
        server.send(id, 200, "application/json", "{\"ok\":true}");
    });

    server.on("/audit", HTTP_GET, [](uint8_t id, HttpReq *) {
        char doc[2048];
        if (detws_audit_dump_json(doc, sizeof(doc)) > 0)
            server.send(id, 200, "application/json", doc);
        else
            server.send(id, 500, "application/json", "{\"error\":\"buffer\"}");
    });

    server.begin(80);
}

void loop()
{
    server.handle();
}
