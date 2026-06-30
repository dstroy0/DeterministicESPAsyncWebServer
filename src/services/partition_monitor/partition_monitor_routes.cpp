// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file partition_monitor_routes.cpp
 * @brief Partition-map monitor route (GET endpoint serving the JSON).
 *
 * Separated from the host-testable core (partition_monitor.cpp) so the classifier
 * + serializer unit-test without pulling in the server.
 */

#include "services/partition_monitor/partition_monitor.h"

#if DETWS_ENABLE_PARTITION_MONITOR

static DetWebServer *s_srv = nullptr;

static void partition_handler(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    DetwsPartitionInfo parts[DETWS_PARTITION_MAX];
    uint8_t n = detws_partition_collect(parts, DETWS_PARTITION_MAX);
    char buf[DETWS_PARTITION_JSON_BUF];
    detws_partition_json(parts, n, buf, sizeof(buf));
    if (s_srv)
        s_srv->send(slot_id, 200, DET_MIME_JSON, buf);
}

void detws_partition_monitor_begin(DetWebServer &server, const char *path)
{
    s_srv = &server;
    server.on((path && path[0]) ? path : "/partitions", HTTP_GET, partition_handler);
}

#endif // DETWS_ENABLE_PARTITION_MONITOR
