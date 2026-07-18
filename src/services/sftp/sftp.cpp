// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file sftp.cpp
 * @brief SFTP protocol v3 wire codec - implementation. See sftp.h.
 */

#include "services/sftp/sftp.h"

#if DWS_ENABLE_SSH_SFTP

#include <stdio.h>
#include <string.h>
#include <time.h>

// --- reader (big-endian, bounds-checked) ---------------------------------------------------------

void dws_sftp_rd_init(SftpReader *r, const uint8_t *payload, size_t len)
{
    r->p = payload;
    r->len = len;
    r->off = 0;
    r->ok = true;
}

uint8_t dws_sftp_rd_u8(SftpReader *r)
{
    if (!r->ok || r->off + 1 > r->len)
    {
        r->ok = false;
        return 0;
    }
    return r->p[r->off++];
}

uint32_t dws_sftp_rd_u32(SftpReader *r)
{
    if (!r->ok || r->off + 4 > r->len)
    {
        r->ok = false;
        return 0;
    }
    uint32_t v = ((uint32_t)r->p[r->off] << 24) | ((uint32_t)r->p[r->off + 1] << 16) |
                 ((uint32_t)r->p[r->off + 2] << 8) | (uint32_t)r->p[r->off + 3];
    r->off += 4;
    return v;
}

uint64_t dws_sftp_rd_u64(SftpReader *r)
{
    if (!r->ok || r->off + 8 > r->len)
    {
        r->ok = false;
        return 0;
    }
    uint64_t v = 0;
    for (int i = 0; i < 8; i++)
        v = (v << 8) | r->p[r->off + i];
    r->off += 8;
    return v;
}

bool dws_sftp_rd_string(SftpReader *r, const uint8_t **out, uint32_t *out_len)
{
    uint32_t n = dws_sftp_rd_u32(r);
    if (!r->ok || r->off + n > r->len)
    {
        r->ok = false;
        return false;
    }
    if (out)
        *out = r->p + r->off;
    if (out_len)
        *out_len = n;
    r->off += n;
    return true;
}

bool dws_sftp_rd_attrs(SftpReader *r, SftpAttrs *a)
{
    a->flags = dws_sftp_rd_u32(r);
    a->size = 0;
    a->permissions = 0;
    a->atime = 0;
    a->mtime = 0;
    if (a->flags & SSH_FILEXFER_ATTR_SIZE)
        a->size = dws_sftp_rd_u64(r);
    if (a->flags & SSH_FILEXFER_ATTR_UIDGID)
    {
        dws_sftp_rd_u32(r); // uid (ignored)
        dws_sftp_rd_u32(r); // gid (ignored)
    }
    if (a->flags & SSH_FILEXFER_ATTR_PERMISSIONS)
        a->permissions = dws_sftp_rd_u32(r);
    if (a->flags & SSH_FILEXFER_ATTR_ACMODTIME)
    {
        a->atime = dws_sftp_rd_u32(r);
        a->mtime = dws_sftp_rd_u32(r);
    }
    if (a->flags & SSH_FILEXFER_ATTR_EXTENDED)
    {
        uint32_t ec = dws_sftp_rd_u32(r);
        for (uint32_t i = 0; i < ec && r->ok; i++)
        {
            dws_sftp_rd_string(r, nullptr, nullptr); // extended type
            dws_sftp_rd_string(r, nullptr, nullptr); // extended data
        }
    }
    return r->ok;
}

// --- writer --------------------------------------------------------------------------------------

void dws_sftp_wr_init(SftpWriter *w, uint8_t *out, size_t cap)
{
    w->p = out;
    w->cap = cap;
    w->off = 4; // reserve the length prefix
    w->ovf = (cap < 4);
}

void dws_sftp_wr_u8(SftpWriter *w, uint8_t v)
{
    if (w->ovf || w->off + 1 > w->cap)
    {
        w->ovf = true;
        return;
    }
    w->p[w->off++] = v;
}

void dws_sftp_wr_u32(SftpWriter *w, uint32_t v)
{
    if (w->ovf || w->off + 4 > w->cap)
    {
        w->ovf = true;
        return;
    }
    w->p[w->off++] = (uint8_t)(v >> 24);
    w->p[w->off++] = (uint8_t)(v >> 16);
    w->p[w->off++] = (uint8_t)(v >> 8);
    w->p[w->off++] = (uint8_t)v;
}

void dws_sftp_wr_u64(SftpWriter *w, uint64_t v)
{
    if (w->ovf || w->off + 8 > w->cap)
    {
        w->ovf = true;
        return;
    }
    for (int i = 7; i >= 0; i--)
        w->p[w->off++] = (uint8_t)(v >> (8 * i));
}

void dws_sftp_wr_bytes(SftpWriter *w, const void *b, size_t n)
{
    if (w->ovf || w->off + n > w->cap)
    {
        w->ovf = true;
        return;
    }
    memcpy(w->p + w->off, b, n);
    w->off += n;
}

void dws_sftp_wr_string(SftpWriter *w, const void *s, uint32_t n)
{
    dws_sftp_wr_u32(w, n);
    dws_sftp_wr_bytes(w, s, n);
}

void dws_sftp_wr_attrs(SftpWriter *w, const SftpAttrs *a)
{
    dws_sftp_wr_u32(w, a->flags);
    if (a->flags & SSH_FILEXFER_ATTR_SIZE)
        dws_sftp_wr_u64(w, a->size);
    if (a->flags & SSH_FILEXFER_ATTR_UIDGID)
    {
        dws_sftp_wr_u32(w, 0);
        dws_sftp_wr_u32(w, 0);
    }
    if (a->flags & SSH_FILEXFER_ATTR_PERMISSIONS)
        dws_sftp_wr_u32(w, a->permissions);
    if (a->flags & SSH_FILEXFER_ATTR_ACMODTIME)
    {
        dws_sftp_wr_u32(w, a->atime);
        dws_sftp_wr_u32(w, a->mtime);
    }
}

size_t dws_sftp_wr_finish(SftpWriter *w)
{
    if (w->ovf)
        return 0;
    uint32_t plen = (uint32_t)(w->off - 4);
    w->p[0] = (uint8_t)(plen >> 24);
    w->p[1] = (uint8_t)(plen >> 16);
    w->p[2] = (uint8_t)(plen >> 8);
    w->p[3] = (uint8_t)plen;
    return w->off;
}

size_t dws_sftp_wr_pos(const SftpWriter *w)
{
    return w->off;
}

void dws_sftp_wr_patch_u32(SftpWriter *w, size_t at, uint32_t v)
{
    if (at + 4 > w->cap)
        return;
    w->p[at] = (uint8_t)(v >> 24);
    w->p[at + 1] = (uint8_t)(v >> 16);
    w->p[at + 2] = (uint8_t)(v >> 8);
    w->p[at + 3] = (uint8_t)v;
}

// --- framing -------------------------------------------------------------------------------------

size_t dws_sftp_frame_len(const uint8_t *buf, size_t have, size_t max)
{
    if (have < 4)
        return 0; // need at least the length prefix
    uint32_t plen = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];
    size_t total = (size_t)plen + 4;
    if (plen == 0 || total > max)
        return (size_t)-1; // malformed (0-length) or larger than the caller can hold -> drop
    return total;
}

// --- response builders ---------------------------------------------------------------------------

size_t dws_sftp_build_version(uint8_t *out, size_t cap)
{
    SftpWriter w;
    dws_sftp_wr_init(&w, out, cap);
    dws_sftp_wr_u8(&w, SSH_FXP_VERSION);
    dws_sftp_wr_u32(&w, SFTP_VERSION);
    return dws_sftp_wr_finish(&w);
}

size_t dws_sftp_build_status(uint32_t id, uint32_t code, const char *msg, uint8_t *out, size_t cap)
{
    SftpWriter w;
    dws_sftp_wr_init(&w, out, cap);
    dws_sftp_wr_u8(&w, SSH_FXP_STATUS);
    dws_sftp_wr_u32(&w, id);
    dws_sftp_wr_u32(&w, code);
    size_t ml = msg ? strnlen(msg, cap) : 0;
    dws_sftp_wr_string(&w, msg ? msg : "", (uint32_t)ml);
    dws_sftp_wr_string(&w, "", 0); // language tag
    return dws_sftp_wr_finish(&w);
}

size_t dws_sftp_build_handle(uint32_t id, const void *handle, uint32_t hlen, uint8_t *out, size_t cap)
{
    SftpWriter w;
    dws_sftp_wr_init(&w, out, cap);
    dws_sftp_wr_u8(&w, SSH_FXP_HANDLE);
    dws_sftp_wr_u32(&w, id);
    dws_sftp_wr_string(&w, handle, hlen);
    return dws_sftp_wr_finish(&w);
}

size_t dws_sftp_build_attrs(uint32_t id, const SftpAttrs *a, uint8_t *out, size_t cap)
{
    SftpWriter w;
    dws_sftp_wr_init(&w, out, cap);
    dws_sftp_wr_u8(&w, SSH_FXP_ATTRS);
    dws_sftp_wr_u32(&w, id);
    dws_sftp_wr_attrs(&w, a);
    return dws_sftp_wr_finish(&w);
}

size_t dws_sftp_build_data(uint32_t id, const void *data, uint32_t dlen, uint8_t *out, size_t cap)
{
    SftpWriter w;
    dws_sftp_wr_init(&w, out, cap);
    dws_sftp_wr_u8(&w, SSH_FXP_DATA);
    dws_sftp_wr_u32(&w, id);
    dws_sftp_wr_string(&w, data, dlen);
    return dws_sftp_wr_finish(&w);
}

size_t dws_sftp_build_name1(uint32_t id, const char *name, const char *longname, const SftpAttrs *a, uint8_t *out,
                            size_t cap)
{
    SftpWriter w;
    dws_sftp_wr_init(&w, out, cap);
    dws_sftp_wr_u8(&w, SSH_FXP_NAME);
    dws_sftp_wr_u32(&w, id);
    dws_sftp_wr_u32(&w, 1); // one entry
    dws_sftp_wr_string(&w, name, (uint32_t)strnlen(name, cap));
    dws_sftp_wr_string(&w, longname, (uint32_t)strnlen(longname, cap));
    dws_sftp_wr_attrs(&w, a);
    return dws_sftp_wr_finish(&w);
}

size_t dws_sftp_format_longname(bool is_dir, uint32_t perms, uint64_t size, uint32_t mtime, const char *name, char *out,
                                size_t cap)
{
    static const char *kMonths[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    char mode[11];
    mode[0] = is_dir ? 'd' : '-';
    static const char rwx[9] = {'r', 'w', 'x', 'r', 'w', 'x', 'r', 'w', 'x'};
    for (int i = 0; i < 9; i++)
        mode[1 + i] = (perms & (1u << (8 - i))) ? rwx[i] : '-';
    mode[10] = '\0';

    char date[16];
    time_t t = (time_t)mtime;
    struct tm tmv;
    memset(&tmv, 0, sizeof(tmv));
    gmtime_r(&t, &tmv); // reentrant; mtime==0 -> epoch, a harmless placeholder date
    int mon = (tmv.tm_mon >= 0 && tmv.tm_mon < 12) ? tmv.tm_mon : 0;
    snprintf(date, sizeof(date), "%s %2d %5d", kMonths[mon], tmv.tm_mday, tmv.tm_year + 1900);

    int n = snprintf(out, cap, "%s 1 0 0 %llu %s %s", mode, (unsigned long long)size, date, name);
    if (n < 0)
    {
        if (cap)
            out[0] = '\0';
        return 0;
    }
    if ((size_t)n < cap)
        return (size_t)n;
    return cap ? cap - 1 : 0;
}

#endif // DWS_ENABLE_SSH_SFTP
