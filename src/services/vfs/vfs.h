// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file vfs.h
 * @brief Unified virtual filesystem wrapper (DWS_ENABLE_VFS).
 *
 * One small file API the rest of the library and the application can target,
 * backed by a pluggable store. Two backends ship:
 *
 *  - **RAM** (built-in, host-testable, zero-heap): a fixed pool of
 *    DWS_VFS_RAM_FILES files of up to DWS_VFS_RAM_FILE_SIZE bytes each, all in
 *    BSS - deterministic, bounded, and the same on host and ESP32. Ideal as a
 *    scratch / test / RAM-disk store.
 *
 *  - **Arduino FS** (ESP32 only): wraps a real `fs::FS` (LittleFS / SD / SPIFFS)
 *    for persistent storage. Mount it with
 *    `dws_vfs_mount(dws_vfs_fs(&LittleFS))`.
 *
 * The point is a single seam: a feature (a config store, the audit-log sink, a
 * template loader, an upload target) calls dws_vfs_* and works against whichever
 * backend the application mounted - RAM in tests, flash or SD in production -
 * without knowing which. Handles are small ints the backend assigns; the API
 * fails closed (-1 / false) when nothing is mounted or a bound is hit.
 *
 * Single-accessor like the other services: use it from one context (a worker /
 * loop), not concurrently.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_VFS_H
#define DETERMINISTICESPASYNCWEBSERVER_VFS_H

#include "ServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DWS_ENABLE_VFS

/** @brief Open modes. */
enum class DetwsVfsMode : uint8_t
{
    DWS_VFS_READ = 0,   ///< Read existing file (fails if absent).
    DWS_VFS_WRITE = 1,  ///< Create/truncate for writing.
    DWS_VFS_APPEND = 2, ///< Create/open for appending at end.
};

/**
 * @brief A storage backend. Each call returns a small handle (>= 0) or -1.
 *
 * Implement this to add a backend; the built-in RAM and Arduino-FS backends are
 * returned by dws_vfs_ram() / dws_vfs_fs().
 */
struct DetwsVfsBackend
{
    int (*open)(const char *path, int mode);             ///< -> handle (>=0) or -1.
    int (*read)(int handle, void *buf, size_t n);        ///< bytes read, or -1.
    int (*write)(int handle, const void *buf, size_t n); ///< bytes written, or -1.
    void (*close)(int handle);                           ///< release a handle.
    long (*size)(const char *path);                      ///< file size, or -1 if absent.
    bool (*exists)(const char *path);                    ///< true if the path exists.
    bool (*remove)(const char *path);                    ///< delete; true on success.
    bool (*rename)(const char *from, const char *to);    ///< rename; true on success.
};

/** @brief Mount the active backend (call once at setup; nullptr unmounts). */
void dws_vfs_mount(const DetwsVfsBackend *backend);

/** @brief The built-in deterministic RAM backend (fixed BSS pool, no heap). */
const DetwsVfsBackend *dws_vfs_ram(void);

/** @brief Clear the RAM backend (all files + open handles). */
void dws_vfs_ram_format(void);

// ---------------------------------------------------------------------------
// Unified API - dispatches to the mounted backend (fails closed if unmounted).
// ---------------------------------------------------------------------------

/** @brief Open @p path with @p mode (DetwsVfsMode). @return a handle (>= 0), or -1 on error. */
int dws_vfs_open(const char *path, DetwsVfsMode mode);
/** @brief Read up to @p n bytes from @p handle into @p buf. @return bytes read, or -1. */
int dws_vfs_read(int handle, void *buf, size_t n);
/** @brief Write @p n bytes from @p buf to @p handle. @return bytes written, or -1. */
int dws_vfs_write(int handle, const void *buf, size_t n);
/** @brief Close the open @p handle. */
void dws_vfs_close(int handle);
/** @brief Size of the file at @p path. @return the size in bytes, or -1 if absent. */
long dws_vfs_size(const char *path);
/** @brief @return true if @p path exists. */
bool dws_vfs_exists(const char *path);
/** @brief Delete @p path. @return true on success. */
bool dws_vfs_remove(const char *path);
/** @brief Rename @p from to @p to. @return true on success. */
bool dws_vfs_rename(const char *from, const char *to);

/**
 * @brief Read a whole file into @p buf.
 * @return bytes read (0..cap), or -1 if absent / would exceed @p cap.
 */
long dws_vfs_read_file(const char *path, void *buf, size_t cap);

/** @brief Create/truncate @p path and write @p n bytes. @return true on success. */
bool dws_vfs_write_file(const char *path, const void *buf, size_t n);

#ifdef ARDUINO
namespace fs
{
class FS;
}
/**
 * @brief Arduino FS backend over a real filesystem (LittleFS / SD / SPIFFS).
 *
 * Pass the mounted FS object; returns a backend to hand to dws_vfs_mount().
 * Example: `dws_vfs_mount(dws_vfs_fs(&LittleFS));`
 */
const DetwsVfsBackend *dws_vfs_fs(fs::FS *filesystem);
#endif

#endif // DWS_ENABLE_VFS
#endif // DETERMINISTICESPASYNCWEBSERVER_VFS_H
