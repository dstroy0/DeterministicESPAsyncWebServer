// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file sse.cpp
 * @brief Server-Sent Events connection pool implementation.
 */

#include "sse.h"
#include "network_drivers/transport/tcp.h"
#include <stdio.h>
#include <string.h>

SseConn sse_pool[MAX_SSE_CONNS];

void sse_init()
{
    for (int i = 0; i < MAX_SSE_CONNS; i++)
    {
        sse_pool[i] = {};
        sse_pool[i].sse_id = (uint8_t)i;
    }
}

SseConn *sse_alloc(uint8_t slot_id, const char *path)
{
    for (int i = 0; i < MAX_SSE_CONNS; i++)
    {
        if (!sse_pool[i].active)
        {
            sse_pool[i] = {};
            sse_pool[i].sse_id = (uint8_t)i;
            sse_pool[i].slot_id = slot_id;
            sse_pool[i].active = true;
            strncpy(sse_pool[i].path, path, MAX_PATH_LEN - 1);
            sse_pool[i].path[MAX_PATH_LEN - 1] = '\0';
            return &sse_pool[i];
        }
    }
    return nullptr;
}

SseConn *sse_find(uint8_t slot_id)
{
    for (int i = 0; i < MAX_SSE_CONNS; i++)
    {
        if (sse_pool[i].active && sse_pool[i].slot_id == slot_id)
            return &sse_pool[i];
    }
    return nullptr;
}

void sse_free(uint8_t slot_id)
{
    for (int i = 0; i < MAX_SSE_CONNS; i++)
    {
        if (sse_pool[i].active && sse_pool[i].slot_id == slot_id)
        {
            sse_pool[i] = {};
            sse_pool[i].sse_id = (uint8_t)i;
            return;
        }
    }
}

bool sse_write(SseConn *sse, const char *data, const char *event, const char *id)
{
    if (!data)
        return false;

    TcpConn *conn = &conn_pool[sse->slot_id];
    if (conn->state != ConnState::CONN_ACTIVE || !conn->pcb)
        return false;

    char buf[SSE_BUF_SIZE];
    int pos = 0;
    int rem = (int)sizeof(buf);

    // pos starts at 0 here, so the first field needs no remaining-space guard; the
    // later fields do, since a truncated snprintf advances pos toward (or past) rem.
    if (event)
        pos += snprintf(buf + pos, (size_t)(rem - pos), "event: %s\n", event);
    if (id && pos < rem)
        pos += snprintf(buf + pos, (size_t)(rem - pos), "id: %s\n", id);
    if (pos < rem)
        pos += snprintf(buf + pos, (size_t)(rem - pos), "data: %s\n\n", data);

    if (pos <= 0 || pos >= rem)
        return false;

    det_conn_send(conn->id, buf, (u16_t)pos);
    return true;
}
