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

DetwsSpaAction dws_spa_route(const char *path, const char *api_prefix)
{
    if (!path || path[0] == '\0' || (path[0] == '/' && path[1] == '\0'))
        return DetwsSpaAction::DWS_SPA_SERVE_SHELL; // "" or "/" -> the app shell

    if (api_prefix && api_prefix[0])
    {
        size_t pl = strnlen(api_prefix, MAX_PATH_LEN + 1);
        if (strncmp(path, api_prefix, pl) == 0)
            return DetwsSpaAction::DWS_SPA_PASSTHROUGH; // "/api/..." -> handlers
    }

    return dws_spa_has_extension(path) ? DetwsSpaAction::DWS_SPA_SERVE_FILE : DetwsSpaAction::DWS_SPA_SERVE_SHELL;
}

#endif // DWS_ENABLE_SPA_ROUTER
