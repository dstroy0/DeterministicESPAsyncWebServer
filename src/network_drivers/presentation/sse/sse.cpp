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

// Append `len` bytes of `src` at *pos if the whole record still leaves room for a trailing NUL (content must
// fit in n-1). Returns false the moment it would not fit, so a record that overflows reports 0 rather than a
// truncated frame. memcpy of a known-length span, no format parsing.
static inline bool sse_append(char *buf, size_t n, size_t *pos, const char *src, size_t len)
{
    if (*pos + len > n - 1)
        return false;
    memcpy(buf + *pos, src, len);
    *pos += len;
    return true;
}

int dws_sse_format(char *buf, size_t n, const char *data, const char *event, const char *id)
{
    if (!data || n == 0)
        return 0;

    // WHATWG event-stream field order: event, then id, then data (blank line terminates the record). A
    // branchless memcpy framer - fixed prefixes + strlen/memcpy of each value + the terminators - instead of
    // three snprintf("%s") calls; ~an order of magnitude cheaper on the Xtensa vsnprintf path, which matters
    // for a high-rate broadcast fan-out (many subscribers). Byte-identical output (test_sse_format).
    size_t pos = 0;
    if (event)
        if (!sse_append(buf, n, &pos, "event: ", 7) || !sse_append(buf, n, &pos, event, strlen(event)) ||
            !sse_append(buf, n, &pos, "\n", 1))
            return 0;
    if (id)
        if (!sse_append(buf, n, &pos, "id: ", 4) || !sse_append(buf, n, &pos, id, strlen(id)) ||
            !sse_append(buf, n, &pos, "\n", 1))
            return 0;
    if (!sse_append(buf, n, &pos, "data: ", 6) || !sse_append(buf, n, &pos, data, strlen(data)) ||
        !sse_append(buf, n, &pos, "\n\n", 2))
        return 0;

    buf[pos] = '\0'; // pos <= n-1 by construction, so the NUL always fits
    return (int)pos;
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
