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
// All southbound-registry state, owned by one instance (internal linkage): the bounded
// driver table and its count, grouped so it is one named owner, unreachable cross-TU.
struct SouthboundCtx
{
    const SouthboundDriver *drivers[DETWS_SOUTHBOUND_MAX_DRIVERS] = {};
    size_t count = 0;
};
SouthboundCtx s_sb;
} // namespace

int detws_southbound_register(const SouthboundDriver *drv)
{
    if (!drv || !drv->name)
        return SB_ERR_ARG;
    if (detws_southbound_find(drv->name))
        return SB_ERR_DUP;
    if (s_sb.count >= DETWS_SOUTHBOUND_MAX_DRIVERS)
        return SB_ERR_FULL;
    s_sb.drivers[s_sb.count++] = drv;
    return SB_OK;
}

void detws_southbound_clear(void)
{
    for (size_t i = 0; i < DETWS_SOUTHBOUND_MAX_DRIVERS; i++)
        s_sb.drivers[i] = nullptr;
    s_sb.count = 0;
}

size_t detws_southbound_count(void)
{
    return s_sb.count;
}

const SouthboundDriver *detws_southbound_find(const char *name)
{
    if (!name)
        return nullptr;
    for (size_t i = 0; i < s_sb.count; i++)
        if (s_sb.drivers[i] && s_sb.drivers[i]->name && strcmp(s_sb.drivers[i]->name, name) == 0)
            return s_sb.drivers[i];
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
