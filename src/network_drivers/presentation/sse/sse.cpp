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

SseConn dws_sse_pool[MAX_SSE_CONNS];

void dws_sse_init()
{
    for (int i = 0; i < MAX_SSE_CONNS; i++)
    {
        dws_sse_pool[i] = {};
        dws_sse_pool[i].dws_sse_id = (uint8_t)i;
    }
}

SseConn *dws_sse_alloc(uint8_t slot_id, const char *path)
{
    for (int i = 0; i < MAX_SSE_CONNS; i++)
    {
        if (!dws_sse_pool[i].active)
        {
            dws_sse_pool[i] = {};
            dws_sse_pool[i].dws_sse_id = (uint8_t)i;
            dws_sse_pool[i].slot_id = slot_id;
            dws_sse_pool[i].active = true;
            strncpy(dws_sse_pool[i].path, path, MAX_PATH_LEN - 1);
            dws_sse_pool[i].path[MAX_PATH_LEN - 1] = '\0';
            return &dws_sse_pool[i];
        }
    }
    return nullptr;
}

SseConn *dws_sse_find(uint8_t slot_id)
{
    for (int i = 0; i < MAX_SSE_CONNS; i++)
    {
        if (dws_sse_pool[i].active && dws_sse_pool[i].slot_id == slot_id)
            return &dws_sse_pool[i];
    }
    return nullptr;
}

void dws_sse_free(uint8_t slot_id)
{
    for (int i = 0; i < MAX_SSE_CONNS; i++)
    {
        if (dws_sse_pool[i].active && dws_sse_pool[i].slot_id == slot_id)
        {
            dws_sse_pool[i] = {};
            dws_sse_pool[i].dws_sse_id = (uint8_t)i;
            return;
        }
    }
}

int dws_sse_format(char *buf, size_t n, const char *data, const char *event, const char *id)
{
    if (!data || n == 0)
        return 0;

    int pos = 0;
    int rem = (int)n;

    // pos starts at 0 here, so the first field needs no remaining-space guard; the
    // later fields do, since a truncated snprintf advances pos toward (or past) rem.
    if (event)
        pos += snprintf(buf + pos, (size_t)(rem - pos), "event: %s\n", event);
    if (id && pos < rem)
        pos += snprintf(buf + pos, (size_t)(rem - pos), "id: %s\n", id);
    if (pos < rem)
        pos += snprintf(buf + pos, (size_t)(rem - pos), "data: %s\n\n", data);

    if (pos <= 0 || pos >= rem)
        return 0;
    return pos;
}

bool dws_sse_write(SseConn *sse, const char *data, const char *event, const char *id)
{
    if (!dws_conn_active(sse->slot_id))
        return false;

    char buf[SSE_BUF_SIZE];
    int pos = dws_sse_format(buf, sizeof(buf), data, event, id);
    if (pos <= 0)
        return false;

    dws_conn_send(sse->slot_id, buf, (u16_t)pos);
    return true;
}
