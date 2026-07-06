// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file southbound.cpp
 * @brief Southbound protocol-driver framework registry + dispatch (see southbound.h).
 */

#include "services/southbound/southbound.h"

#if DETWS_ENABLE_SOUTHBOUND

#include <string.h>

#ifndef DETWS_SOUTHBOUND_MAX_DRIVERS
#define DETWS_SOUTHBOUND_MAX_DRIVERS 8 ///< bounded registry; no heap.
#endif

namespace
{
const SouthboundDriver *g_drivers[DETWS_SOUTHBOUND_MAX_DRIVERS] = {};
size_t g_count = 0;
} // namespace

int detws_southbound_register(const SouthboundDriver *drv)
{
    if (!drv || !drv->name)
        return SB_ERR_ARG;
    if (detws_southbound_find(drv->name))
        return SB_ERR_DUP;
    if (g_count >= DETWS_SOUTHBOUND_MAX_DRIVERS)
        return SB_ERR_FULL;
    g_drivers[g_count++] = drv;
    return SB_OK;
}

void detws_southbound_clear(void)
{
    for (size_t i = 0; i < DETWS_SOUTHBOUND_MAX_DRIVERS; i++)
        g_drivers[i] = nullptr;
    g_count = 0;
}

size_t detws_southbound_count(void)
{
    return g_count;
}

const SouthboundDriver *detws_southbound_find(const char *name)
{
    if (!name)
        return nullptr;
    for (size_t i = 0; i < g_count; i++)
        if (g_drivers[i] && g_drivers[i]->name && strcmp(g_drivers[i]->name, name) == 0)
            return g_drivers[i];
    return nullptr;
}

int detws_southbound_read(const char *name, uint32_t point, int32_t *value_out)
{
    if (!value_out)
        return SB_ERR_ARG;
    const SouthboundDriver *d = detws_southbound_find(name);
    if (!d)
        return SB_ERR_NOT_FOUND;
    if (!d->read)
        return SB_ERR_UNSUPPORTED;
    return d->read(d->ctx, point, value_out);
}

int detws_southbound_write(const char *name, uint32_t point, int32_t value)
{
    const SouthboundDriver *d = detws_southbound_find(name);
    if (!d)
        return SB_ERR_NOT_FOUND;
    if (!d->write)
        return SB_ERR_UNSUPPORTED;
    return d->write(d->ctx, point, value);
}

int detws_southbound_read_block(const char *name, uint32_t first, int32_t *out, size_t n)
{
    if (!out || n == 0)
        return SB_ERR_ARG;
    const SouthboundDriver *d = detws_southbound_find(name);
    if (!d)
        return SB_ERR_NOT_FOUND;
    if (!d->read_block)
        return SB_ERR_UNSUPPORTED;
    return d->read_block(d->ctx, first, out, n);
}

int detws_southbound_write_block(const char *name, uint32_t first, const int32_t *in, size_t n)
{
    if (!in || n == 0)
        return SB_ERR_ARG;
    const SouthboundDriver *d = detws_southbound_find(name);
    if (!d)
        return SB_ERR_NOT_FOUND;
    if (!d->write_block)
        return SB_ERR_UNSUPPORTED;
    return d->write_block(d->ctx, first, in, n);
}

#endif // DETWS_ENABLE_SOUTHBOUND
