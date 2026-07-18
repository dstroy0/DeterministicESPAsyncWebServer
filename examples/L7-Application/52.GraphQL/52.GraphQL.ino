// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file 52.GraphQL.ino
 * @brief GraphQL query endpoint (DWS_ENABLE_GRAPHQL).
 *
 * POST a GraphQL query to /graphql; the device resolves the selected fields and
 * returns a `{"data":{...}}` response shaped exactly by the query - one endpoint,
 * the client decides what it needs.
 *
 *   curl -s --data '{ heap uptime net { rssi } }' http://<ip>/graphql
 *     -> {"data":{"heap":210000,"uptime":42,"net":{"rssi":-50}}}
 *   curl -s --data '{ greet(name: "world") }' http://<ip>/graphql
 *     -> {"data":{"greet":"hi world"}}
 *
 * The resolver answers one scalar per dotted path; a field with a sub-selection
 * (like `net`) is an object the engine builds by recursing. Arguments on a field
 * are visible to the resolver for that field and its descendants.
 *
 * NOTE: enable it for the whole build. In platformio.ini:
 *     build_flags = -DDWS_ENABLE_GRAPHQL=1
 * (Arduino IDE: it is already set for you in the build_opt.h beside this sketch, so it builds as-is.)
 */

#define DWS_ENABLE_GRAPHQL 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/graphql/graphql.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DWS server;

static bool resolver(const char *path, const DetwsGqlArgs *args, DetwsGqlValue *out)
{
    if (!strcmp(path, "heap"))
    {
        out->type = DetwsGqlType::DWS_GQL_INT;
        out->i = ESP.getFreeHeap();
        return true;
    }
    if (!strcmp(path, "uptime"))
    {
        out->type = DetwsGqlType::DWS_GQL_INT;
        out->i = millis() / 1000;
        return true;
    }
    if (!strcmp(path, "net.rssi"))
    {
        out->type = DetwsGqlType::DWS_GQL_INT;
        out->i = WiFi.RSSI();
        return true;
    }
    if (!strcmp(path, "net.ip"))
    {
        static char ip[20];
        WiFi.localIP().toString().toCharArray(ip, sizeof(ip));
        out->type = DetwsGqlType::DWS_GQL_STR;
        out->s = ip;
        return true;
    }
    if (!strcmp(path, "greet"))
    {
        const char *who = "?";
        dws_gql_arg_str(args, "name", &who);
        static char b[64];
        snprintf(b, sizeof(b), "hi %s", who);
        out->type = DetwsGqlType::DWS_GQL_STR;
        out->s = b;
        return true;
    }
    return false; // -> null
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

    server.on("/graphql", HttpMethod::HTTP_POST, [](uint8_t id, HttpReq *req) {
        char body[512];
        DetwsGqlResult rc = dws_graphql_execute((const char *)req->body, req->body_len, resolver, body, sizeof(body));
        // The engine writes {"data":...} on success or {"errors":...} on a parse
        // error; 200 with the GraphQL error envelope is the conventional reply.
        server.send(id, rc == DetwsGqlResult::DWS_GQL_OK ? 200 : 400, "application/json", body);
    });

    server.begin(80);
}

void loop()
{
    server.handle();
}
