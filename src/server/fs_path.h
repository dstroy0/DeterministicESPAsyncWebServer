// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file fs_path.h
 * @brief Shared filesystem path helpers for the file-transfer servers (SFTP / SCP, and the pattern the
 *        static file server + WebDAV also use): join a mount root with a request subpath and reject `..`
 *        traversal - the single choke point that keeps a request from escaping its mount.
 *
 * Pure string logic (no fs::FS, no heap, no stdlib) so it is host-testable and identical on device + host.
 * The guard is a substring reject (not realpath/symlink resolution) - the on-flash filesystems (FAT /
 * LittleFS) have no symlinks, so a `..`-free joined path cannot escape the root.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_FS_PATH_H
#define DETERMINISTICESPASYNCWEBSERVER_FS_PATH_H

#include "ServerConfig.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/** @brief Join a filesystem @p root and a @p sub path into @p out, normalizing the separator. @return false on
 * overflow. */
inline bool fs_path_join(const char *root, const char *sub, char *out, size_t cap)
{
    size_t rlen = strnlen(root, cap);
    bool root_slash = (rlen > 0 && root[rlen - 1] == '/');
    if (root_slash && sub[0] == '/')
        sub++;
    bool sub_slash = (sub[0] == '/');
    const char *sep = (root_slash || sub_slash) ? "" : "/";
    int wn = snprintf(out, cap, "%s%s%s", root, sep, sub);
    return wn > 0 && wn < (int)cap;
}

/**
 * @brief Resolve a mount @p root + a request @p sub path to an on-disk path in @p out: reject any `..`
 *        traversal, join onto the root, and drop a trailing '/'.
 * @return 0 on success, -1 on a traversal attempt (`..` present), -2 if the joined path would overflow @p out.
 */
inline int fs_path_resolve(const char *root, const char *sub, char *out, size_t cap)
{
    if (strstr(sub, ".."))
        return -1; // path traversal - refuse before touching the filesystem
    if (!fs_path_join(root, sub, out, cap))
        return -2;
    size_t fpl = strnlen(out, cap);
    if (fpl > 1 && out[fpl - 1] == '/')
        out[fpl - 1] = '\0';
    return 0;
}

#endif // DETERMINISTICESPASYNCWEBSERVER_FS_PATH_H
