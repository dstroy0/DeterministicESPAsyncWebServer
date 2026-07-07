# 52.GraphQL - a GraphQL query endpoint

**Layer:** L7 Application · **Build flags:** `DETWS_ENABLE_GRAPHQL`

## What this example teaches

GraphQL lets the client say exactly which fields it wants in one request. POST a
query to `/graphql` and the device resolves the selected fields and returns a
`{"data":{...}}` response shaped by the query - so the same endpoint serves a
minimal phone client and a rich dashboard without new routes.

**A resolver answers one scalar per dotted path.** Nested selections become dotted
paths (`net.rssi`); arguments are visible to the resolver for that field:

```cpp
static bool resolver(const char *path, const DetwsGqlArgs *args, DetwsGqlValue *out) {
    if (!strcmp(path, "heap"))     { out->type = DETWS_GQL_INT; out->i = ESP.getFreeHeap(); return true; }
    if (!strcmp(path, "net.rssi")) { out->type = DETWS_GQL_INT; out->i = WiFi.RSSI();       return true; }
    if (!strcmp(path, "greet")) {
        const char *who = "?";
        detws_gql_arg_str(args, "name", &who);   // read the field's argument
        static char b[64]; snprintf(b, sizeof(b), "hi %s", who);
        out->type = DETWS_GQL_STR; out->s = b; return true;
    }
    return false; // -> null
}
```

A field with a sub-selection (like `net`) is an object the engine builds by
recursing into the resolver for each child; returning false yields `null`.

**One call executes a query.** The engine parses the body, walks the selection set,
and writes the response envelope:

```cpp
int rc = detws_graphql_execute(req->body, req->body_len, resolver, body, sizeof(body));
server.send(id, rc == DETWS_GQL_OK ? 200 : 400, "application/json", body);
```

It writes `{"data":...}` on success or `{"errors":...}` on a parse error;
answering `200` with the GraphQL error envelope is the conventional reply.

## Build and run

```sh
pio ci --board=esp32dev --project-option="framework=arduino" \
  --project-option="build_flags=-DDETWS_ENABLE_GRAPHQL=1" \
  --lib="." examples/L7-Application/52.GraphQL/52.GraphQL.ino
```

```sh
curl -s --data '{ heap uptime net { rssi } }' http://<ip>/graphql
# {"data":{"heap":210000,"uptime":42,"net":{"rssi":-50}}}
curl -s --data '{ greet(name: "world") }' http://<ip>/graphql
# {"data":{"greet":"hi world"}}
```

## Annotated source

The complete sketch ([52.GraphQL.ino](52.GraphQL.ino)), reproduced verbatim with
added explanatory comments:

```cpp
// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

#define DETWS_ENABLE_GRAPHQL 1

#include "dwserver.h"
#include "network_drivers/physical/physical.h"
#include "services/graphql/graphql.h"
#include <WiFi.h>

static const char *SSID = "YOUR_SSID";
static const char *PASSWORD = "YOUR_PASSWORD";

DetWebServer server;

// One scalar per dotted path; nested objects recurse, arguments are per-field.
static bool resolver(const char *path, const DetwsGqlArgs *args, DetwsGqlValue *out)
{
    if (!strcmp(path, "heap"))
    {
        out->type = DETWS_GQL_INT;
        out->i = ESP.getFreeHeap();
        return true;
    }
    if (!strcmp(path, "uptime"))
    {
        out->type = DETWS_GQL_INT;
        out->i = millis() / 1000;
        return true;
    }
    if (!strcmp(path, "net.rssi"))
    {
        out->type = DETWS_GQL_INT;
        out->i = WiFi.RSSI();
        return true;
    }
    if (!strcmp(path, "net.ip"))
    {
        static char ip[20];
        WiFi.localIP().toString().toCharArray(ip, sizeof(ip));
        out->type = DETWS_GQL_STR;
        out->s = ip;
        return true;
    }
    if (!strcmp(path, "greet"))
    {
        const char *who = "?";
        detws_gql_arg_str(args, "name", &who);
        static char b[64];
        snprintf(b, sizeof(b), "hi %s", who);
        out->type = DETWS_GQL_STR;
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

    server.on("/graphql", HTTP_POST, [](uint8_t id, HttpReq *req) {
        char body[512];
        int rc = detws_graphql_execute((const char *)req->body, req->body_len, resolver, body, sizeof(body));
        // The engine writes {"data":...} on success or {"errors":...} on a parse
        // error; 200 with the GraphQL error envelope is the conventional reply.
        server.send(id, rc == DETWS_GQL_OK ? 200 : 400, "application/json", body);
    });

    server.begin(80);
}

void loop()
{
    server.handle();
}
```
