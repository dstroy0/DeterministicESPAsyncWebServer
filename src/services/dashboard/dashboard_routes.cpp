// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dashboard_routes.cpp
 * @brief Dashboard server wiring: page, layout JSON, and SSE value stream.
 *
 * Separated from the host-testable core (dashboard.cpp) so the serializers can be
 * unit-tested without pulling in the server. Requires DETWS_ENABLE_SSE.
 */

#include "services/dashboard/dashboard.h"

#if DETWS_ENABLE_DASHBOARD

// Dependency (DASHBOARD requires SSE) is enforced centrally in DetWebServerConfig.h.

#include "DeterministicESPAsyncWebServer.h"
#include "network_drivers/application/web_assets.h" // DETWS_DASHBOARD_PAGE
#include <stdio.h>
#if DETWS_ENABLE_WEBSOCKET
#include "network_drivers/presentation/websocket.h" // ws_pool for inbound control messages
#endif

static DetWebServer *s_srv = nullptr;
static char s_stream_path[MAX_PATH_LEN];
#if DETWS_ENABLE_WEBSOCKET
static char s_ws_path[MAX_PATH_LEN];
#endif

static void dash_page_handler(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    if (s_srv)
        s_srv->send(slot_id, 200, "text/html", DETWS_DASHBOARD_PAGE);
}

static void dash_layout_handler(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    char buf[DETWS_DASHBOARD_JSON_BUF];
    detws_dashboard_layout_json(buf, sizeof(buf));
    if (s_srv)
        s_srv->send(slot_id, 200, "application/json", buf);
}

static void dash_sse_connect(uint8_t sse_id)
{
    char buf[DETWS_DASHBOARD_JSON_BUF];
    if (s_srv && detws_dashboard_values_json(buf, sizeof(buf)) > 0)
        s_srv->sse_send(sse_id, buf); // seed the new client with the latest values
}

#if DETWS_ENABLE_WEBSOCKET
static void dash_ws_connect(uint8_t ws_id)
{
    (void)ws_id;
}
static void dash_ws_message(uint8_t ws_id)
{
    // Control widgets send {"k":"<key>","v":<num>}; parse + dispatch to the callback.
    if (ws_id < MAX_WS_CONNS)
        detws_dashboard_dispatch_control((const char *)ws_pool[ws_id].buf);
}
static void dash_ws_close(uint8_t ws_id)
{
    (void)ws_id;
}
#endif

void detws_dashboard_begin(DetWebServer &server, const char *path, const DetwsWidget *widgets, uint8_t count)
{
    s_srv = &server;
    detws_dashboard_configure(widgets, count);

    if (!path || !path[0])
        path = "/dashboard";

    char layout_path[MAX_PATH_LEN];
    snprintf(layout_path, sizeof(layout_path), "%s/layout", path);
    snprintf(s_stream_path, sizeof(s_stream_path), "%s/stream", path);

    server.on(path, HTTP_GET, dash_page_handler);
    server.on(layout_path, HTTP_GET, dash_layout_handler);
    server.on_sse(s_stream_path, dash_sse_connect);
#if DETWS_ENABLE_WEBSOCKET
    snprintf(s_ws_path, sizeof(s_ws_path), "%s/ws", path);
    server.on_ws(s_ws_path, dash_ws_connect, dash_ws_message, dash_ws_close);
#endif
}

void detws_dashboard_publish()
{
    if (!s_srv)
        return;
    char buf[DETWS_DASHBOARD_JSON_BUF];
    if (detws_dashboard_values_json(buf, sizeof(buf)) > 0)
        s_srv->sse_broadcast(s_stream_path, buf);
}

#endif // DETWS_ENABLE_DASHBOARD
