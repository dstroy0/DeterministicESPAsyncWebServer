// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file vfs.cpp
 * @brief Unified VFS - dispatch + RAM backend (+ Arduino FS backend on ESP32).
 *
 * The RAM backend is a fixed pool of named in-BSS files and a fixed handle table;
 * everything is bounded and host-identical. The Arduino backend maps handles to a
 * pool of fs::File objects over a real filesystem. The public dws_vfs_* calls
 * just forward to whichever backend is mounted.
 */

#include "services/vfs/vfs.h"

#if DWS_ENABLE_VFS

#include <string.h>

namespace
{
// ---- RAM backend ----------------------------------------------------------
struct RamFile
{
    bool used;
    char name[DWS_VFS_NAME_MAX];
    size_t len;
    uint8_t data[DWS_VFS_RAM_FILE_SIZE];
};
struct RamHandle
{
    bool open;
    int file;
    size_t pos;
    DetwsVfsMode mode;
};

// All VFS dispatch + RAM-backend state, owned by one instance (internal linkage): the
// mounted backend vtable, the RAM file pool, and the handle table, grouped so it is one
// named owner, unreachable from any other translation unit. (The RAM ops are fixed-signature
// vtable entries, so they reach this single owner directly.)
struct VfsCtx
{
    const DetwsVfsBackend *backend = nullptr;
    RamFile rf[DWS_VFS_RAM_FILES];
    RamHandle rh[DWS_VFS_MAX_OPEN];
};
VfsCtx s_vfs;

int ram_find(const char *name)
{
    for (int i = 0; i < DWS_VFS_RAM_FILES; i++)
        if (s_vfs.rf[i].used && strncmp(s_vfs.rf[i].name, name, DWS_VFS_NAME_MAX) == 0)
            return i;
    return -1;
}

int ram_create(const char *name)
{
    if (strnlen(name, DWS_VFS_NAME_MAX + 1) >= DWS_VFS_NAME_MAX)
        return -1;
    for (int i = 0; i < DWS_VFS_RAM_FILES; i++)
        if (!s_vfs.rf[i].used)
        {
            s_vfs.rf[i].used = true;
            strncpy(s_vfs.rf[i].name, name, DWS_VFS_NAME_MAX - 1);
            s_vfs.rf[i].name[DWS_VFS_NAME_MAX - 1] = '\0';
            s_vfs.rf[i].len = 0;
            return i;
        }
    return -1;
}

bool ram_handle_ok(int h)
{
    return h >= 0 && h < DWS_VFS_MAX_OPEN && s_vfs.rh[h].open;
}

int ram_open(const char *path, int mode)
{
    if (!path)
        return -1;
    const DetwsVfsMode m = (DetwsVfsMode)mode; // the backend ABI carries mode as int; read it back as the enum
    int f = ram_find(path);
    if (m == DetwsVfsMode::DWS_VFS_READ)
    {
        if (f < 0)
            return -1;
    }
    else
    {
        if (f < 0)
            f = ram_create(path);
        if (f < 0)
            return -1;
        if (m == DetwsVfsMode::DWS_VFS_WRITE)
            s_vfs.rf[f].len = 0;
    }
    for (int h = 0; h < DWS_VFS_MAX_OPEN; h++)
        if (!s_vfs.rh[h].open)
        {
            s_vfs.rh[h].open = true;
            s_vfs.rh[h].file = f;
            s_vfs.rh[h].mode = m;
            s_vfs.rh[h].pos = (m == DetwsVfsMode::DWS_VFS_APPEND) ? s_vfs.rf[f].len : 0;
            return h;
        }
    return -1; // handle pool exhausted
}

int ram_read(int h, void *buf, size_t n)
{
    if (!ram_handle_ok(h))
        return -1;
    RamFile *f = &s_vfs.rf[s_vfs.rh[h].file];
    size_t avail = (s_vfs.rh[h].pos < f->len) ? (f->len - s_vfs.rh[h].pos) : 0;
    size_t k = n < avail ? n : avail;
    memcpy(buf, f->data + s_vfs.rh[h].pos, k);
    s_vfs.rh[h].pos += k;
    return (int)k;
}

int ram_write(int h, const void *buf, size_t n)
{
    if (!ram_handle_ok(h) || s_vfs.rh[h].mode == DetwsVfsMode::DWS_VFS_READ)
        return -1;
    RamFile *f = &s_vfs.rf[s_vfs.rh[h].file];
    size_t cap = (s_vfs.rh[h].pos < DWS_VFS_RAM_FILE_SIZE) ? (DWS_VFS_RAM_FILE_SIZE - s_vfs.rh[h].pos) : 0;
    size_t k = n < cap ? n : cap;
    memcpy(f->data + s_vfs.rh[h].pos, buf, k);
    s_vfs.rh[h].pos += k;
    if (s_vfs.rh[h].pos > f->len)
        f->len = s_vfs.rh[h].pos;
    return (int)k;
}

void ram_close(int h)
{
    if (h >= 0 && h < DWS_VFS_MAX_OPEN)
        s_vfs.rh[h].open = false;
}

long ram_size(const char *path)
{
    int f = ram_find(path);
    return f < 0 ? -1 : (long)s_vfs.rf[f].len;
}

bool ram_exists(const char *path)
{
    return ram_find(path) >= 0;
}

bool ram_remove(const char *path)
{
    int f = ram_find(path);
    if (f < 0)
        return false;
    s_vfs.rf[f].used = false;
    return true;
}

bool ram_rename(const char *from, const char *to)
{
    if (!from || !to || strnlen(to, DWS_VFS_NAME_MAX + 1) >= DWS_VFS_NAME_MAX)
        return false;
    int f = ram_find(from);
    if (f < 0)
        return false;
    int dst = ram_find(to);
    if (dst >= 0)
        s_vfs.rf[dst].used = false; // overwrite an existing destination
    strncpy(s_vfs.rf[f].name, to, DWS_VFS_NAME_MAX - 1);
    s_vfs.rf[f].name[DWS_VFS_NAME_MAX - 1] = '\0';
    return true;
}

const DetwsVfsBackend s_ram_backend = {ram_open, ram_read,   ram_write,  ram_close,
                                       ram_size, ram_exists, ram_remove, ram_rename};
} // namespace

void dws_vfs_mount(const DetwsVfsBackend *backend)
{
    s_vfs.backend = backend;
}

const DetwsVfsBackend *dws_vfs_ram(void)
{
    return &s_ram_backend;
}

void dws_vfs_ram_format(void)
{
    for (int i = 0; i < DWS_VFS_RAM_FILES; i++)
        s_vfs.rf[i].used = false;
    for (int h = 0; h < DWS_VFS_MAX_OPEN; h++)
        s_vfs.rh[h].open = false;
}

int dws_vfs_open(const char *path, DetwsVfsMode mode)
{
    return s_vfs.backend ? s_vfs.backend->open(path, (int)mode) : -1; // cross the int backend ABI
}
int dws_vfs_read(int handle, void *buf, size_t n)
{
    return s_vfs.backend ? s_vfs.backend->read(handle, buf, n) : -1;
}
int dws_vfs_write(int handle, const void *buf, size_t n)
{
    return s_vfs.backend ? s_vfs.backend->write(handle, buf, n) : -1;
}
void dws_vfs_close(int handle)
{
    if (s_vfs.backend)
        s_vfs.backend->close(handle);
}
long dws_vfs_size(const char *path)
{
    return s_vfs.backend ? s_vfs.backend->size(path) : -1;
}
bool dws_vfs_exists(const char *path)
{
    return s_vfs.backend ? s_vfs.backend->exists(path) : false;
}
bool dws_vfs_remove(const char *path)
{
    return s_vfs.backend ? s_vfs.backend->remove(path) : false;
}
bool dws_vfs_rename(const char *from, const char *to)
{
    return s_vfs.backend ? s_vfs.backend->rename(from, to) : false;
}

long dws_vfs_read_file(const char *path, void *buf, size_t cap)
{
    long sz = dws_vfs_size(path);
    if (sz < 0 || (size_t)sz > cap)
        return -1;
    int h = dws_vfs_open(path, DetwsVfsMode::DWS_VFS_READ);
    if (h < 0)
        return -1;
    size_t total = 0;
    uint8_t *out = (uint8_t *)buf;
    while (total < (size_t)sz)
    {
        int r = dws_vfs_read(h, out + total, (size_t)sz - total);
        if (r <= 0)
            break;
        total += (size_t)r;
    }
    dws_vfs_close(h);
    return (long)total;
}

bool dws_vfs_write_file(const char *path, const void *buf, size_t n)
{
    int h = dws_vfs_open(path, DetwsVfsMode::DWS_VFS_WRITE);
    if (h < 0)
        return false;
    size_t total = 0;
    const uint8_t *in = (const uint8_t *)buf;
    while (total < n)
    {
        int w = dws_vfs_write(h, in + total, n - total);
        if (w <= 0)
            break;
        total += (size_t)w;
    }
    dws_vfs_close(h);
    return total == n;
}

// ---------------------------------------------------------------------------
// Arduino FS backend (ESP32): wraps a real fs::FS over LittleFS / SD / SPIFFS.
// ---------------------------------------------------------------------------
#ifdef ARDUINO

#include <FS.h>

namespace
{
// All Arduino FS-backend state, owned by one instance (internal linkage): the mounted
// filesystem and the open-file handle pool, grouped so it is one named owner. (The FS ops
// are fixed-signature vtable entries, so they reach this single owner directly.)
struct FsCtx
{
    fs::FS *fs = nullptr;
    fs::File fsfile[DWS_VFS_MAX_OPEN];
    bool fsopen[DWS_VFS_MAX_OPEN];
};
FsCtx s_vfs_fs;

int fs_open(const char *path, int mode)
{
    if (!s_vfs_fs.fs || !path)
        return -1;
    const DetwsVfsMode md = (DetwsVfsMode)mode; // ABI int -> enum
    const char *m = (md == DetwsVfsMode::DWS_VFS_WRITE)    ? FILE_WRITE
                    : (md == DetwsVfsMode::DWS_VFS_APPEND) ? FILE_APPEND
                                                           : FILE_READ;
    for (int h = 0; h < DWS_VFS_MAX_OPEN; h++)
        if (!s_vfs_fs.fsopen[h])
        {
            s_vfs_fs.fsfile[h] = s_vfs_fs.fs->open(path, m);
            if (!s_vfs_fs.fsfile[h])
                return -1;
            s_vfs_fs.fsopen[h] = true;
            return h;
        }
    return -1;
}
int fs_read(int h, void *buf, size_t n)
{
    if (h < 0 || h >= DWS_VFS_MAX_OPEN || !s_vfs_fs.fsopen[h])
        return -1;
    return (int)s_vfs_fs.fsfile[h].read((uint8_t *)buf, n);
}
int fs_write(int h, const void *buf, size_t n)
{
    if (h < 0 || h >= DWS_VFS_MAX_OPEN || !s_vfs_fs.fsopen[h])
        return -1;
    return (int)s_vfs_fs.fsfile[h].write((const uint8_t *)buf, n);
}
void fs_close(int h)
{
    if (h >= 0 && h < DWS_VFS_MAX_OPEN && s_vfs_fs.fsopen[h])
    {
        s_vfs_fs.fsfile[h].close();
        s_vfs_fs.fsopen[h] = false;
    }
}
long fs_size(const char *path)
{
    if (!s_vfs_fs.fs)
        return -1;
    fs::File f = s_vfs_fs.fs->open(path, FILE_READ);
    if (!f)
        return -1;
    long sz = (long)f.size();
    f.close();
    return sz;
}
bool fs_exists(const char *path)
{
    return s_vfs_fs.fs && s_vfs_fs.fs->exists(path);
}
bool fs_remove(const char *path)
{
    return s_vfs_fs.fs && s_vfs_fs.fs->remove(path);
}
bool fs_rename(const char *from, const char *to)
{
    return s_vfs_fs.fs && s_vfs_fs.fs->rename(from, to);
}

const DetwsVfsBackend s_fs_backend = {fs_open, fs_read, fs_write, fs_close, fs_size, fs_exists, fs_remove, fs_rename};
} // namespace

const DetwsVfsBackend *dws_vfs_fs(fs::FS *filesystem)
{
    s_vfs_fs.fs = filesystem;
    for (int h = 0; h < DWS_VFS_MAX_OPEN; h++)
        s_vfs_fs.fsopen[h] = false;
    return &s_fs_backend;
}

#endif // ARDUINO

#endif // DWS_ENABLE_VFS
