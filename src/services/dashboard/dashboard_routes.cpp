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

// Dependency (DASHBOARD requires SSE) is enforced centrally in ServerConfig.h.

#include "dwserver.h"
#include "network_drivers/application/web_assets.h" // DETWS_DASHBOARD_PAGE
#include "shared_primitives/mime.h"
#include <stdio.h>
#if DETWS_ENABLE_WEBSOCKET
#include "network_drivers/presentation/websocket/websocket.h" // ws_pool for inbound control messages
#endif

// All dashboard-routes state, owned by one instance (internal linkage): the server handle and
// the SSE / WebSocket paths, grouped so it is one named owner, unreachable cross-TU. (The route
// handlers are fixed-signature callbacks, so they reach this single owner directly.)
struct DashRoutesCtx
{
    DetWebServer *srv = nullptr;
    char stream_path[MAX_PATH_LEN] = {0};
#if DETWS_ENABLE_WEBSOCKET
    char ws_path[MAX_PATH_LEN] = {0};
#endif
};
static DashRoutesCtx s_dashr;

static void dash_page_handler(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    if (s_dashr.srv)
        s_dashr.srv->send(slot_id, 200, DET_MIME_TEXT_HTML, DETWS_DASHBOARD_PAGE);
}

static void dash_layout_handler(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    char buf[DETWS_DASHBOARD_JSON_BUF];
    detws_dashboard_layout_json(buf, sizeof(buf));
    if (s_dashr.srv)
        s_dashr.srv->send(slot_id, 200, DET_MIME_JSON, buf);
}

static void dash_sse_connect(uint8_t sse_id)
{
    char buf[DETWS_DASHBOARD_JSON_BUF];
    if (s_dashr.srv && detws_dashboard_values_json(buf, sizeof(buf)) > 0)
        s_dashr.srv->sse_send(sse_id, buf); // seed the new client with the latest values
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
    s_dashr.srv = &server;
    detws_dashboard_configure(widgets, count);

    if (!path || !path[0])
        path = "/dashboard";

    char layout_path[MAX_PATH_LEN];
    snprintf(layout_path, sizeof(layout_path), "%s/layout", path);
    snprintf(s_dashr.stream_path, sizeof(s_dashr.stream_path), "%s/stream", path);

    server.on(path, HttpMethod::HTTP_GET, dash_page_handler);
    server.on(layout_path, HttpMethod::HTTP_GET, dash_layout_handler);
    server.on_sse(s_dashr.stream_path, dash_sse_connect);
#if DETWS_ENABLE_WEBSOCKET
    snprintf(s_dashr.ws_path, sizeof(s_dashr.ws_path), "%s/ws", path);
    server.on_ws(s_dashr.ws_path, dash_ws_connect, dash_ws_message, dash_ws_close);
#endif
}

void detws_dashboard_publish()
{
    if (!s_dashr.srv)
        return;
    char buf[DETWS_DASHBOARD_JSON_BUF];
    if (detws_dashboard_values_json(buf, sizeof(buf)) > 0)
        s_dashr.srv->sse_broadcast(s_dashr.stream_path, buf);
}

#endif // DETWS_ENABLE_DASHBOARD
