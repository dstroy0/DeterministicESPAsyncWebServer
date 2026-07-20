// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file spa_router.cpp
 * @brief Single-page-app micro-routing decision (see spa_router.h).
 */

#include "services/spa_router/spa_router.h"

#if DWS_ENABLE_SPA_ROUTER

#include <string.h>

bool dws_spa_has_extension(const char *path)
{
    if (!path)
        return false;
    // Look at the last path segment; an extension is a '.' after the last '/', not at the segment start.
    const char *slash = strrchr(path, '/');
    const char *seg = slash ? slash + 1 : path;
    const char *dot = strrchr(seg, '.');
    return dot && dot != seg && dot[1] != '\0';
}

DWSSpaAction dws_spa_route(const char *path, const char *api_prefix)
{
    if (!path || path[0] == '\0' || (path[0] == '/' && path[1] == '\0'))
        return DWSSpaAction::DWS_SPA_SERVE_SHELL; // "" or "/" -> the app shell

    if (api_prefix && api_prefix[0])
    {
        size_t pl = strnlen(api_prefix, MAX_PATH_LEN + 1);
        if (strncmp(path, api_prefix, pl) == 0)
            return DWSSpaAction::DWS_SPA_PASSTHROUGH; // "/api/..." -> handlers
    }

    return dws_spa_has_extension(path) ? DWSSpaAction::DWS_SPA_SERVE_FILE : DWSSpaAction::DWS_SPA_SERVE_SHELL;
}

DWSSpaAction dws_spa_route_ex(const char *path, const DWSSpaCtx *ctx)
{
    if (!ctx)
        return dws_spa_route(path, nullptr);

    DWSSpaAction a = dws_spa_route(path, ctx->api_prefix);
    // Only the shell decision can degrade. An asset request still resolves to the file (the caller
    // reports a real 404 if it is missing), and the API must keep passing through - the fallback
    // page posts to those same endpoints, so cutting them off would make it useless.
    if (a != DWSSpaAction::DWS_SPA_SERVE_SHELL)
        return a;
    if (!ctx->shell_available || !ctx->client_scripting || ctx->degraded)
        return DWSSpaAction::DWS_SPA_SERVE_FALLBACK;
    return a;
}

// ---------------------------------------------------------------------------
// Conditional UI streaming
// ---------------------------------------------------------------------------

void dws_ui_stream_begin(DWSUiStream *s, const DWSUiFragment *frags, size_t count, void *ctx)
{
    if (!s)
        return;
    s->frags = frags;
    s->count = frags ? count : 0;
    s->ctx = ctx;
    s->idx = 0;
    s->off = 0;
    s->done = (s->count == 0);
}

size_t dws_ui_stream_next(DWSUiStream *s, char *out, size_t cap)
{
    if (!s || !out || cap == 0 || s->done)
        return 0;

    size_t written = 0;
    while (written < cap && s->idx < s->count)
    {
        const DWSUiFragment *f = &s->frags[s->idx];
        // Evaluated here rather than at begin(), so a fragment reflects the state that holds when
        // the stream reaches it. Skipping costs nothing - no bytes are emitted for it at all.
        if (!f->html || (f->when && !f->when(s->ctx)))
        {
            s->idx++;
            s->off = 0;
            continue;
        }
        const char *src = f->html + s->off;
        size_t room = cap - written;
        size_t n = 0;
        while (n < room && src[n] != '\0')
            n++;
        memcpy(out + written, src, n);
        written += n;
        s->off += n;
        if (src[n] == '\0') // this fragment is fully emitted
        {
            s->idx++;
            s->off = 0;
        }
    }
    if (s->idx >= s->count)
        s->done = true;
    return written;
}

bool dws_ui_stream_done(const DWSUiStream *s)
{
    return !s || s->done;
}

#endif // DWS_ENABLE_SPA_ROUTER
