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

#if DWS_ENABLE_PARTITION_MONITOR

#include "dwserver.h"
#include "shared_primitives/mime.h"

// All partition-monitor-routes state, owned by one instance (internal linkage): the server
// handle. (The route handler is a fixed-signature callback, so it reaches this owner directly.)
struct PartitionRoutesCtx
{
    DWS *srv = nullptr;
};
static PartitionRoutesCtx s_partr;

static void partition_handler(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    DWSPartitionInfo parts[DWS_PARTITION_MAX];
    uint8_t n = dws_partition_collect(parts, DWS_PARTITION_MAX);
    char buf[DWS_PARTITION_JSON_BUF];
    dws_partition_json(parts, n, buf, sizeof(buf));
    if (s_partr.srv)
        s_partr.srv->send(slot_id, 200, DWS_MIME_JSON, buf);
}

void dws_partition_monitor_begin(DWS &server, const char *path)
{
    s_partr.srv = &server;
    server.on((path && path[0]) ? path : "/partitions", HttpMethod::HTTP_GET, partition_handler);
}

#endif // DWS_ENABLE_PARTITION_MONITOR
