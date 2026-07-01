// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file vfs.cpp
 * @brief Unified VFS - dispatch + RAM backend (+ Arduino FS backend on ESP32).
 *
 * The RAM backend is a fixed pool of named in-BSS files and a fixed handle table;
 * everything is bounded and host-identical. The Arduino backend maps handles to a
 * pool of fs::File objects over a real filesystem. The public detws_vfs_* calls
 * just forward to whichever backend is mounted.
 */

#include "services/vfs/vfs.h"

#if DETWS_ENABLE_VFS

#include <string.h>

namespace
{
const DetwsVfsBackend *s_backend = nullptr;

// ---- RAM backend ----------------------------------------------------------
struct RamFile
{
    bool used;
    char name[DETWS_VFS_NAME_MAX];
    size_t len;
    uint8_t data[DETWS_VFS_RAM_FILE_SIZE];
};
struct RamHandle
{
    bool open;
    int file;
    size_t pos;
    int mode;
};
RamFile s_rf[DETWS_VFS_RAM_FILES];
RamHandle s_rh[DETWS_VFS_MAX_OPEN];

int ram_find(const char *name)
{
    for (int i = 0; i < DETWS_VFS_RAM_FILES; i++)
        if (s_rf[i].used && strncmp(s_rf[i].name, name, DETWS_VFS_NAME_MAX) == 0)
            return i;
    return -1;
}

int ram_create(const char *name)
{
    if (strlen(name) >= DETWS_VFS_NAME_MAX)
        return -1;
    for (int i = 0; i < DETWS_VFS_RAM_FILES; i++)
        if (!s_rf[i].used)
        {
            s_rf[i].used = true;
            strncpy(s_rf[i].name, name, DETWS_VFS_NAME_MAX - 1);
            s_rf[i].name[DETWS_VFS_NAME_MAX - 1] = '\0';
            s_rf[i].len = 0;
            return i;
        }
    return -1;
}

bool ram_handle_ok(int h)
{
    return h >= 0 && h < DETWS_VFS_MAX_OPEN && s_rh[h].open;
}

int ram_open(const char *path, int mode)
{
    if (!path)
        return -1;
    int f = ram_find(path);
    if (mode == DETWS_VFS_READ)
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
        if (mode == DETWS_VFS_WRITE)
            s_rf[f].len = 0;
    }
    for (int h = 0; h < DETWS_VFS_MAX_OPEN; h++)
        if (!s_rh[h].open)
        {
            s_rh[h].open = true;
            s_rh[h].file = f;
            s_rh[h].mode = mode;
            s_rh[h].pos = (mode == DETWS_VFS_APPEND) ? s_rf[f].len : 0;
            return h;
        }
    return -1; // handle pool exhausted
}

int ram_read(int h, void *buf, size_t n)
{
    if (!ram_handle_ok(h))
        return -1;
    RamFile *f = &s_rf[s_rh[h].file];
    size_t avail = (s_rh[h].pos < f->len) ? (f->len - s_rh[h].pos) : 0;
    size_t k = n < avail ? n : avail;
    memcpy(buf, f->data + s_rh[h].pos, k);
    s_rh[h].pos += k;
    return (int)k;
}

int ram_write(int h, const void *buf, size_t n)
{
    if (!ram_handle_ok(h) || s_rh[h].mode == DETWS_VFS_READ)
        return -1;
    RamFile *f = &s_rf[s_rh[h].file];
    size_t cap = (s_rh[h].pos < DETWS_VFS_RAM_FILE_SIZE) ? (DETWS_VFS_RAM_FILE_SIZE - s_rh[h].pos) : 0;
    size_t k = n < cap ? n : cap;
    memcpy(f->data + s_rh[h].pos, buf, k);
    s_rh[h].pos += k;
    if (s_rh[h].pos > f->len)
        f->len = s_rh[h].pos;
    return (int)k;
}

void ram_close(int h)
{
    if (h >= 0 && h < DETWS_VFS_MAX_OPEN)
        s_rh[h].open = false;
}

long ram_size(const char *path)
{
    int f = ram_find(path);
    return f < 0 ? -1 : (long)s_rf[f].len;
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
    s_rf[f].used = false;
    return true;
}

bool ram_rename(const char *from, const char *to)
{
    if (!from || !to || strlen(to) >= DETWS_VFS_NAME_MAX)
        return false;
    int f = ram_find(from);
    if (f < 0)
        return false;
    int dst = ram_find(to);
    if (dst >= 0)
        s_rf[dst].used = false; // overwrite an existing destination
    strncpy(s_rf[f].name, to, DETWS_VFS_NAME_MAX - 1);
    s_rf[f].name[DETWS_VFS_NAME_MAX - 1] = '\0';
    return true;
}

const DetwsVfsBackend s_ram_backend = {ram_open, ram_read,   ram_write,  ram_close,
                                       ram_size, ram_exists, ram_remove, ram_rename};
} // namespace

void detws_vfs_mount(const DetwsVfsBackend *backend)
{
    s_backend = backend;
}

const DetwsVfsBackend *detws_vfs_ram(void)
{
    return &s_ram_backend;
}

void detws_vfs_ram_format(void)
{
    for (int i = 0; i < DETWS_VFS_RAM_FILES; i++)
        s_rf[i].used = false;
    for (int h = 0; h < DETWS_VFS_MAX_OPEN; h++)
        s_rh[h].open = false;
}

int detws_vfs_open(const char *path, int mode)
{
    return s_backend ? s_backend->open(path, mode) : -1;
}
int detws_vfs_read(int handle, void *buf, size_t n)
{
    return s_backend ? s_backend->read(handle, buf, n) : -1;
}
int detws_vfs_write(int handle, const void *buf, size_t n)
{
    return s_backend ? s_backend->write(handle, buf, n) : -1;
}
void detws_vfs_close(int handle)
{
    if (s_backend)
        s_backend->close(handle);
}
long detws_vfs_size(const char *path)
{
    return s_backend ? s_backend->size(path) : -1;
}
bool detws_vfs_exists(const char *path)
{
    return s_backend ? s_backend->exists(path) : false;
}
bool detws_vfs_remove(const char *path)
{
    return s_backend ? s_backend->remove(path) : false;
}
bool detws_vfs_rename(const char *from, const char *to)
{
    return s_backend ? s_backend->rename(from, to) : false;
}

long detws_vfs_read_file(const char *path, void *buf, size_t cap)
{
    long sz = detws_vfs_size(path);
    if (sz < 0 || (size_t)sz > cap)
        return -1;
    int h = detws_vfs_open(path, DETWS_VFS_READ);
    if (h < 0)
        return -1;
    size_t total = 0;
    uint8_t *out = (uint8_t *)buf;
    while (total < (size_t)sz)
    {
        int r = detws_vfs_read(h, out + total, (size_t)sz - total);
        if (r <= 0)
            break;
        total += (size_t)r;
    }
    detws_vfs_close(h);
    return (long)total;
}

bool detws_vfs_write_file(const char *path, const void *buf, size_t n)
{
    int h = detws_vfs_open(path, DETWS_VFS_WRITE);
    if (h < 0)
        return false;
    size_t total = 0;
    const uint8_t *in = (const uint8_t *)buf;
    while (total < n)
    {
        int w = detws_vfs_write(h, in + total, n - total);
        if (w <= 0)
            break;
        total += (size_t)w;
    }
    detws_vfs_close(h);
    return total == n;
}

// ---------------------------------------------------------------------------
// Arduino FS backend (ESP32): wraps a real fs::FS over LittleFS / SD / SPIFFS.
// ---------------------------------------------------------------------------
#ifdef ARDUINO

#include <FS.h>

namespace
{
fs::FS *s_fs = nullptr;
fs::File s_fsfile[DETWS_VFS_MAX_OPEN];
bool s_fsopen[DETWS_VFS_MAX_OPEN];

int fs_open(const char *path, int mode)
{
    if (!s_fs || !path)
        return -1;
    const char *m = (mode == DETWS_VFS_WRITE) ? FILE_WRITE : (mode == DETWS_VFS_APPEND) ? FILE_APPEND : FILE_READ;
    for (int h = 0; h < DETWS_VFS_MAX_OPEN; h++)
        if (!s_fsopen[h])
        {
            s_fsfile[h] = s_fs->open(path, m);
            if (!s_fsfile[h])
                return -1;
            s_fsopen[h] = true;
            return h;
        }
    return -1;
}
int fs_read(int h, void *buf, size_t n)
{
    if (h < 0 || h >= DETWS_VFS_MAX_OPEN || !s_fsopen[h])
        return -1;
    return (int)s_fsfile[h].read((uint8_t *)buf, n);
}
int fs_write(int h, const void *buf, size_t n)
{
    if (h < 0 || h >= DETWS_VFS_MAX_OPEN || !s_fsopen[h])
        return -1;
    return (int)s_fsfile[h].write((const uint8_t *)buf, n);
}
void fs_close(int h)
{
    if (h >= 0 && h < DETWS_VFS_MAX_OPEN && s_fsopen[h])
    {
        s_fsfile[h].close();
        s_fsopen[h] = false;
    }
}
long fs_size(const char *path)
{
    if (!s_fs)
        return -1;
    fs::File f = s_fs->open(path, FILE_READ);
    if (!f)
        return -1;
    long sz = (long)f.size();
    f.close();
    return sz;
}
bool fs_exists(const char *path)
{
    return s_fs && s_fs->exists(path);
}
bool fs_remove(const char *path)
{
    return s_fs && s_fs->remove(path);
}
bool fs_rename(const char *from, const char *to)
{
    return s_fs && s_fs->rename(from, to);
}

const DetwsVfsBackend s_fs_backend = {fs_open, fs_read, fs_write, fs_close, fs_size, fs_exists, fs_remove, fs_rename};
} // namespace

const DetwsVfsBackend *detws_vfs_fs(fs::FS *filesystem)
{
    s_fs = filesystem;
    for (int h = 0; h < DETWS_VFS_MAX_OPEN; h++)
        s_fsopen[h] = false;
    return &s_fs_backend;
}

#endif // ARDUINO

#endif // DETWS_ENABLE_VFS
