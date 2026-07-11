// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file webdav.cpp
 * @brief WebDAV (RFC 4918) filesystem-backed request handler for DetWebServer.
 *
 * Split out of dwserver.cpp (single-purpose server files). The pure WebDAV core - method
 * classification, the 207 Multi-Status XML builder, header parsing - lives in
 * services/webdav/webdav.{h,cpp} and is host-tested; this file is the DetWebServer glue that
 * needs a real filesystem (PROPFIND/PUT/COPY/MOVE/... over an fs::FS subtree). WEBDAV requires
 * FILE_SERVING (enforced in ServerConfig.h), so the file-serving helpers it borrows are always
 * present. Behaviour is identical to the pre-split code - a pure move.
 */

#include "services/webdav/webdav.h" // the pure WebDAV core
#include "dwserver.h"
#include "network_drivers/transport/tcp.h" // conn_pool
#include "server/dwserver_internal.h"
#include "shared_primitives/mime.h"
#include <string.h>

#if DETWS_ENABLE_WEBDAV
// ---------------------------------------------------------------------------
// WebDAV (RFC 4918) - filesystem-backed request handling. The pure core
// (method classification + 207 XML builder + header parsing) lives in
// services/webdav/webdav.{h,cpp} and is host-tested; this part needs a real FS.
// ---------------------------------------------------------------------------

// WebDAV response scratch, owned by one instance (internal linkage): the 207 Multi-Status build
// buffer (BSS). One named owner, unreachable from any other translation unit.
struct DavBufCtx
{
    char buf[DETWS_WEBDAV_BUF_SIZE];
};
static DavBufCtx s_dav;

// http_rfc1123() lives in the FILE_SERVING section above (WEBDAV requires
// FILE_SERVING), shared by both; used here for getlastmodified / creationdate.

// Join an FS root and a subpath into @p out (mirrors serve_static_request's
// separator handling). Returns false on overflow.
static bool dav_join(const char *root, const char *sub, char *out, size_t cap)
{
    size_t rlen = strlen(root);
    bool root_slash = (rlen > 0 && root[rlen - 1] == '/');
    if (root_slash && sub[0] == '/')
        sub++;
    bool sub_slash = (sub[0] == '/');
    const char *sep = (root_slash || sub_slash) ? "" : "/";
    int wn = snprintf(out, cap, "%s%s%s", root, sep, sub);
    return wn > 0 && wn < (int)cap;
}

// The basename of an FS entry name (cores differ: name() may be a full path or a
// bare name).
static const char *dav_basename(const char *name)
{
    const char *slash = strrchr(name, '/');
    return slash ? slash + 1 : name;
}

// Recursively delete a file or directory tree (bounded depth). Re-opens the
// directory after each child removal so iteration is never mutated underneath us.
static bool dav_rm_recursive(fs::FS &fsys, const char *path, int depth)
{
    if (depth > 8)
        return false; // refuse pathologically deep trees rather than overflow the stack
    fs::File d = fsys.open(path, "r");
    if (!d)
        return false;
    if (!d.isDirectory())
    {
        d.close();
        return fsys.remove(path);
    }
    for (;;)
    {
        fs::File c = d.openNextFile();
        if (!c)
            break;
        char cp[256];
        int wn = snprintf(cp, sizeof(cp), "%s/%s", path, dav_basename(c.name()));
        c.close();
        if (wn <= 0 || wn >= (int)sizeof(cp))
        {
            d.close();
            return false;
        }
        if (!dav_rm_recursive(fsys, cp, depth + 1))
        {
            d.close();
            return false;
        }
        d.close();
        d = fsys.open(path, "r"); // reset the directory cursor after the deletion
        if (!d)
            return false;
    }
    d.close();
    return fsys.rmdir(path);
}

// Recursively copy a file or directory tree from @p src to @p dst (bounded depth).
// Unlike dav_rm_recursive we cannot re-open + take the first child each step (the
// source is not consumed, so that would loop forever); instead we re-open and skip
// to child #idx, which is safe even if a core invalidates an open dir handle across
// the writes the copy makes to the destination tree.
static bool dav_copy_recursive(fs::FS &fsys, const char *src, const char *dst, int depth)
{
    if (depth > 8)
        return false; // refuse pathologically deep trees rather than overflow the stack

    fs::File s = fsys.open(src, "r");
    if (!s)
        return false;
    if (!s.isDirectory())
    {
        fs::File d = fsys.open(dst, "w");
        if (!d)
        {
            s.close();
            return false;
        }
        uint8_t cbuf[FILE_CHUNK_SIZE];
        size_t cn;
        while ((cn = s.read(cbuf, sizeof(cbuf))) > 0)
            d.write(cbuf, cn);
        s.close();
        d.close();
        return true;
    }
    s.close();

    if (!fsys.mkdir(dst)) // create the destination collection (caller cleared any existing dst)
        return false;

    for (int idx = 0;; idx++)
    {
        fs::File d = fsys.open(src, "r");
        if (!d)
            return false;
        fs::File c;
        for (int i = 0; i <= idx; i++)
        {
            c = d.openNextFile();
            if (!c)
                break;
        }
        if (!c)
        {
            d.close();
            break; // no child at this index - done
        }
        char base[128];
        snprintf(base, sizeof(base), "%s", dav_basename(c.name()));
        c.close();
        d.close();

        char sp[256];
        char dp[256];
        int wn1 = snprintf(sp, sizeof(sp), "%s/%s", src, base);
        int wn2 = snprintf(dp, sizeof(dp), "%s/%s", dst, base);
        if (wn1 <= 0 || wn1 >= (int)sizeof(sp) || wn2 <= 0 || wn2 >= (int)sizeof(dp))
            return false;
        if (!dav_copy_recursive(fsys, sp, dp, depth + 1))
            return false;
    }
    return true;
}

// Map a WebDAV request path to its on-disk path under the mount @p r. Strips the
// mount prefix, rejects traversal, joins onto the FS root, and drops a trailing
// '/'. Returns 0 on success, else the HTTP error code (403 traversal, 414 too
// long) - the single source of truth for the path check, shared by the request
// handler and the streaming-PUT begin hook.
static int dav_resolve_path(const Route *r, const char *reqpath, char *out, size_t cap)
{
    size_t plen = strlen(r->path);
    if (plen > 0 && r->path[plen - 1] == '*')
        plen--;
    const char *sub = (strlen(reqpath) >= plen) ? reqpath + plen : "";
    if (strstr(sub, ".."))
        return 403;
    const char *root = r->static_root ? r->static_root : "";
    if (!dav_join(root, sub, out, cap))
        return 414;
    size_t fpl = strlen(out);
    if (fpl > 1 && out[fpl - 1] == '/')
        out[fpl - 1] = '\0';
    return 0;
}

#if DETWS_ENABLE_STREAM_BODY
// Per-connection streaming-PUT state for WebDAV: each slot streams its body to its
// own file, so concurrent PUTs never clobber one another, and a transfer is never
// bounded by BODY_BUF_SIZE. Indexed by the request's slot (req - http_pool).
struct DavPut
{
    fs::File file;  ///< destination file for this slot's PUT.
    bool active;    ///< file opened for the current PUT.
    bool error;     ///< a write (or the open) failed.
    bool existed;   ///< target existed before this PUT (204 vs 201).
    size_t written; ///< bytes written so far.
};

// WebDAV streaming-PUT state, owned by one instance (internal linkage): the serving instance
// and the per-slot destination-file state (each slot streams to its own file, so concurrent
// PUTs never clobber one another). One named owner, unreachable from any other TU.
struct DavPutCtx
{
    DetWebServer *stream_srv = nullptr;
    DavPut put[MAX_CONNS];
};
static DavPutCtx s_davput;

bool DetWebServer::dav_put_begin_tramp(HttpReq *req)
{
    return s_davput.stream_srv && s_davput.stream_srv->dav_stream_put_begin(req);
}
void DetWebServer::dav_put_data_tramp(HttpReq *req, const uint8_t *data, size_t len)
{
    if (s_davput.stream_srv)
        s_davput.stream_srv->dav_stream_put_data(req, data, len);
}
void DetWebServer::dav_put_abort_tramp(HttpReq *req)
{
    // The PUT was torn down before the handler ran: close the half-written file so
    // the handle is not leaked (a leak eventually exhausts LittleFS's open slots).
    uint8_t slot = (uint8_t)(req - http_pool);
    if (slot < MAX_CONNS && s_davput.put[slot].active)
    {
        s_davput.put[slot].file.close();
        s_davput.put[slot].active = false;
    }
}

bool DetWebServer::dav_stream_put_begin(HttpReq *req)
{
    if (strcmp(req->method, "PUT") != 0)
        return false;
    uint8_t slot = (uint8_t)(req - http_pool);
    for (uint8_t i = 0; i < _route_count; i++)
    {
        Route *r = &_routes[i];
        if (!r->is_active || r->type != RouteType::ROUTE_DAV)
            continue;
        if (!path_matches(r->path, r->is_wildcard, req->path))
            continue;
        if (r->iface_filter != DetIface::DETIFACE_ANY && r->iface_filter != conn_pool[slot].iface)
            continue;
        // GCOVR_EXCL_START  a RouteType::ROUTE_DAV route always carries static_fs (set in dav()); this null-guard
        // cannot fire
        if (!r->static_fs)
            return false;
        // GCOVR_EXCL_STOP
        char fs_path[256];
        if (dav_resolve_path(r, req->path, fs_path, sizeof(fs_path)) != 0)
            return false; // traversal / too long - let it buffer; the handler answers 403/414
        DavPut *d = &s_davput.put[slot];
        d->active = false;
        d->error = false;
        d->written = 0;
        d->existed = r->static_fs->exists(fs_path);
        d->file = r->static_fs->open(fs_path, "w");
        if (d->file)
            d->active = true;
        else
            d->error = true;
        return true; // stream regardless so the body is consumed and the handler replies
    }
    return false;
}

void DetWebServer::dav_stream_put_data(HttpReq *req, const uint8_t *data, size_t len)
{
    uint8_t slot = (uint8_t)(req - http_pool);
    // GCOVR_EXCL_START  req is always one of the http_pool slots, so slot < MAX_CONNS; the bound cannot fire
    if (slot >= MAX_CONNS)
        return;
    // GCOVR_EXCL_STOP
    DavPut *d = &s_davput.put[slot];
    if (d->active && !d->error)
    {
        if (d->file.write(data, len) != len)
            d->error = true;
        else
            d->written += len;
    }
}
#endif // DETWS_ENABLE_STREAM_BODY

void DetWebServer::dav(const char *url_prefix, fs::FS &file_sys, const char *fs_root)
{
    if (_route_count >= MAX_ROUTES)
        return;
    Route *r = &_routes[_route_count++];

    char pat[MAX_PATH_LEN];
    size_t n = strlen(url_prefix);
    if (n > 0 && url_prefix[n - 1] == '*')
        snprintf(pat, sizeof(pat), "%s", url_prefix);
    else
        snprintf(pat, sizeof(pat), "%s*", url_prefix);
    fill_route_base(r, pat);
    r->type = RouteType::ROUTE_DAV;
    r->method = HttpMethod::HTTP_GET; // unused: WebDAV dispatch keys off the raw method token
    r->static_fs = &file_sys;
    r->static_root = fs_root;

#if DETWS_ENABLE_STREAM_BODY
    // Stream PUT bodies straight to the file (one global sink; see DETWS_ENABLE_STREAM_BODY).
    s_davput.stream_srv = this;
    http_parser_set_stream_hooks(dav_put_begin_tramp, dav_put_data_tramp, dav_put_abort_tramp);
#endif
}

void DetWebServer::dav_send_status(uint8_t slot_id, int code, const char *extra_headers)
{
    TcpConn *conn = &conn_pool[slot_id];
    if (conn->state != ConnState::CONN_ACTIVE || conn->pcb == nullptr)
    {
        http_reset(slot_id);
        return;
    }
    bool keep;
    const char *cl = resp_conn_hdr(slot_id, &keep);
    char header[RESP_HDR_BUF_SIZE];
    int hlen = snprintf(header, sizeof(header), "HTTP/1.1 %d %s\r\n%sContent-Length: 0\r\n%s\r\n", code,
                        status_text(code), extra_headers ? extra_headers : "", cl);
    det_conn_send(slot_id, header, (u16_t)hlen);
    resp_end(slot_id, code, 0, keep);
}

bool DetWebServer::try_serve_dav(uint8_t slot_id, HttpReq *req)
{
    for (uint8_t i = 0; i < _route_count; i++)
    {
        Route *r = &_routes[i];
        if (!r->is_active || r->type != RouteType::ROUTE_DAV)
            continue;
        if (!path_matches(r->path, r->is_wildcard, req->path))
            continue;
        if (r->iface_filter != DetIface::DETIFACE_ANY && r->iface_filter != conn_pool[slot_id].iface)
            continue;
        serve_dav_request(slot_id, req, r);
        return true;
    }
    return false;
}

void DetWebServer::serve_dav_request(uint8_t slot_id, HttpReq *req, const Route *r)
{
    // GCOVR_EXCL_START  a RouteType::ROUTE_DAV route always carries static_fs (set in dav()); this null-guard cannot
    // fire
    if (!r->static_fs)
    {
        dav_send_status(slot_id, 404, "");
        return;
    }
    // GCOVR_EXCL_STOP
    fs::FS &fsys = *r->static_fs;

    char fs_path[256];
    int rc = dav_resolve_path(r, req->path, fs_path, sizeof(fs_path));
    if (rc != 0)
    {
        dav_send_status(slot_id, rc, ""); // 403 traversal / 414 too long
        return;
    }

    // Mount-prefix length and FS root, used by COPY/MOVE to resolve the Destination.
    size_t plen = strlen(r->path);
    if (plen > 0 && r->path[plen - 1] == '*')
        plen--;
    const char *root = r->static_root ? r->static_root : "";

    switch (webdav_method(req->method))
    {
    case WebDavMethod::DAV_M_OPTIONS:
        add_response_header(slot_id, "DAV", "1, 2");
        add_response_header(slot_id, "Allow",
                            "OPTIONS, GET, HEAD, PUT, DELETE, PROPFIND, PROPPATCH, MKCOL, COPY, MOVE, LOCK, UNLOCK");
        add_response_header(slot_id, "MS-Author-Via", "DAV");
        send_empty(slot_id, 200);
        return;

    case WebDavMethod::DAV_M_GET:
    case WebDavMethod::DAV_M_HEAD: {
        fs::File f = fsys.open(fs_path, "r");
        if (!f)
        {
            dav_send_status(slot_id, 404, "");
            return;
        }
        bool isdir = f.isDirectory();
        f.close();
        if (isdir)
        {
            dav_send_status(slot_id, 405, ""); // GET on a collection is not a download
            return;
        }
        serve_file_internal(slot_id, webdav_method(req->method) == WebDavMethod::DAV_M_HEAD, fsys, fs_path,
                            mime_type(fs_path), nullptr);
        return;
    }

    case WebDavMethod::DAV_M_PUT: {
#if DETWS_ENABLE_STREAM_BODY
        if (req->body_streaming)
        {
            // The body was written to this slot's file as it arrived (dav_stream_put_*).
            DavPut *d = &s_davput.put[slot_id];
            if (d->active)
            {
                d->file.close();
                d->active = false; // closed here: the abort hook must not double-close
            }
            else
            {
                dav_send_status(slot_id, 409, ""); // parent missing / not writable
                return;
            }
            if (d->error)
            {
                dav_send_status(slot_id, 507, ""); // a write failed (e.g. disk full)
                return;
            }
            dav_send_status(slot_id, d->existed ? 204 : 201, "");
            return;
        }
#endif
        // Buffered fallback (streaming disabled): body bounded by BODY_BUF_SIZE.
        bool existed = fsys.exists(fs_path);
        fs::File f = fsys.open(fs_path, "w");
        if (!f)
        {
            dav_send_status(slot_id, 409, ""); // parent missing / not writable
            return;
        }
        // Only an empty CL:0 PUT reaches this buffered path (a bodied PUT to a DAV route streams, or
        // bails before this switch since stream_begin's decline reasons also fail the top-level
        // resolve), so body_len is always 0 here and the write never runs.
        if (req->body_len)
            f.write(req->body, req->body_len); // GCOVR_EXCL_LINE unreachable: body_len always 0 (see above)
        f.close();
        dav_send_status(slot_id, existed ? 204 : 201, "");
        return;
    }

    case WebDavMethod::DAV_M_DELETE: {
        if (!fsys.exists(fs_path))
        {
            dav_send_status(slot_id, 404, "");
            return;
        }
        dav_send_status(slot_id, dav_rm_recursive(fsys, fs_path, 0) ? 204 : 403, "");
        return;
    }

    case WebDavMethod::DAV_M_MKCOL:
        if (fsys.exists(fs_path))
        {
            dav_send_status(slot_id, 405, ""); // already exists
            return;
        }
        dav_send_status(slot_id, fsys.mkdir(fs_path) ? 201 : 409, "");
        return;

    case WebDavMethod::DAV_M_COPY:
    case WebDavMethod::DAV_M_MOVE: {
        const char *dest_hdr = http_get_header(req, "Destination");
        char dest_url[256];
        if (!dest_hdr || !webdav_dest_path(dest_hdr, dest_url, sizeof(dest_url)))
        {
            dav_send_status(slot_id, 400, "");
            return;
        }
        // The destination must live under this same mount.
        if (strncmp(dest_url, r->path, plen) != 0)
        {
            dav_send_status(slot_id, 502, "");
            return;
        }
        const char *dest_sub = dest_url + plen;
        if (strstr(dest_sub, ".."))
        {
            dav_send_status(slot_id, 403, "");
            return;
        }
        char dest_fs[256];
        if (!dav_join(root, dest_sub, dest_fs, sizeof(dest_fs)))
        {
            dav_send_status(slot_id, 414, "");
            return;
        }
        size_t dpl = strlen(dest_fs);
        if (dpl > 1 && dest_fs[dpl - 1] == '/')
            dest_fs[dpl - 1] = '\0';

        const char *ow = http_get_header(req, "Overwrite");
        bool overwrite = !(ow && (ow[0] == 'F' || ow[0] == 'f'));
        bool dest_exists = fsys.exists(dest_fs);
        if (dest_exists && !overwrite)
        {
            dav_send_status(slot_id, 412, "");
            return;
        }

        if (webdav_method(req->method) == WebDavMethod::DAV_M_MOVE)
        {
            if (dest_exists)
                dav_rm_recursive(fsys, dest_fs, 0); // replace
            bool ok = fsys.rename(fs_path, dest_fs);
            dav_send_status(slot_id, ok ? (dest_exists ? 204 : 201) : 409, "");
            return;
        }

        // COPY: a file or a whole collection (RFC 4918 9.8). Depth applies to a
        // collection source: "0" copies just the collection itself, "infinity"
        // (the default, also when absent) copies the entire tree.
        fs::File src = fsys.open(fs_path, "r");
        if (!src)
        {
            dav_send_status(slot_id, 404, "");
            return;
        }
        bool src_is_dir = src.isDirectory();
        src.close();

        const char *depth_h = http_get_header(req, "Depth");
        bool shallow = depth_h && depth_h[0] == '0'; // Depth: 0

        if (dest_exists)
            dav_rm_recursive(fsys, dest_fs, 0); // overwrite: clear the target first

        bool ok;
        if (src_is_dir && shallow)
            ok = fsys.mkdir(dest_fs); // collection, Depth:0 - just the collection, no members
        else
            ok = dav_copy_recursive(fsys, fs_path, dest_fs, 0);
        dav_send_status(slot_id, ok ? (dest_exists ? 204 : 201) : 409, "");
        return;
    }

    case WebDavMethod::DAV_M_LOCK: {
        // Advisory lock: issue a synthetic exclusive-write token (NOT enforced).
        unsigned long tok = (unsigned long)millis();
#ifdef ARDUINO
        tok ^= (unsigned long)esp_random();
#endif
        char token[48];
        snprintf(token, sizeof(token), "opaquelocktoken:%08lx-detws", tok);
        snprintf(s_dav.buf, sizeof(s_dav.buf),
                 "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                 "<D:prop xmlns:D=\"DAV:\"><D:lockdiscovery><D:activelock>"
                 "<D:locktype><D:write/></D:locktype>"
                 "<D:lockscope><D:exclusive/></D:lockscope>"
                 "<D:depth>infinity</D:depth><D:timeout>Second-3600</D:timeout>"
                 "<D:locktoken><D:href>%s</D:href></D:locktoken>"
                 "</D:activelock></D:lockdiscovery></D:prop>\n",
                 token);
        // RFC 4918 §10.5: Lock-Token uses a Coded-URL (angle-bracketed).
        char lt[64];
        snprintf(lt, sizeof(lt), "<%s>", token);
        add_response_header(slot_id, "Lock-Token", lt);
        send(slot_id, 200, "application/xml; charset=utf-8", s_dav.buf);
        return;
    }

    case WebDavMethod::DAV_M_UNLOCK:
        dav_send_status(slot_id, 204, ""); // advisory: nothing to release
        return;

    case WebDavMethod::DAV_M_PROPFIND: {
        fs::File f = fsys.open(fs_path, "r");
        if (!f)
        {
            dav_send_status(slot_id, 404, "");
            return;
        }
        bool isdir = f.isDirectory();
        uint32_t fsize = (uint32_t)f.size();
        time_t mtime = f.getLastWrite();

        int depth = webdav_depth(http_get_header(req, "Depth"), 1);

        // RFC 4918 9.1.1: this server lists at most one level, so a Depth: infinity
        // PROPFIND is rejected with 403 + the propfind-finite-depth precondition rather
        // than silently returning a partial (one-level) 207 the client would read as
        // complete. Clients wanting a listing use Depth: 0 or 1.
        if (depth == DAV_DEPTH_INFINITY)
        {
            f.close();
            static const char body[] = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
                                       "<D:error xmlns:D=\"DAV:\"><D:propfind-finite-depth/></D:error>\r\n";
            send(slot_id, 403, "application/xml", body);
            return;
        }

        // Self href: the request path, with a trailing '/' for a collection.
        char self_href[MAX_PATH_LEN + 2];
        snprintf(self_href, sizeof(self_href), "%s", req->path);
        size_t sl = strlen(self_href);
        if (isdir && (sl == 0 || self_href[sl - 1] != '/'))
        {
            if (sl + 1 < sizeof(self_href))
            {
                self_href[sl++] = '/';
                self_href[sl] = '\0';
            }
        }

        size_t cap = sizeof(s_dav.buf), len = 0;
        len = webdav_ms_begin(s_dav.buf, cap, len);
        char mt[40];
        http_rfc1123(mtime, mt, sizeof(mt));
        len = webdav_ms_entry(s_dav.buf, cap, len, self_href, isdir, fsize, mt, isdir ? "" : mime_type(fs_path));

        if (isdir && depth >= 1)
        {
            int count = 0;
            for (;;)
            {
                fs::File c = f.openNextFile();
                if (!c)
                    break;
                if (count >= DETWS_WEBDAV_MAX_ENTRIES)
                {
                    c.close(); // GCOVR_EXCL_LINE  a 2048B DETWS_WEBDAV_BUF_SIZE fills at ~8 entries, well before
                    break;     // GCOVR_EXCL_LINE  MAX_ENTRIES(32), so the buffer-full break preempts this cap
                }
                const char *base = dav_basename(c.name());
                bool cdir = c.isDirectory();
                uint32_t csize = (uint32_t)c.size();
                time_t cmt = c.getLastWrite();
                char chref[MAX_PATH_LEN + 80];
                snprintf(chref, sizeof(chref), "%s%s%s", self_href, base, cdir ? "/" : "");
                char cmtbuf[40];
                http_rfc1123(cmt, cmtbuf, sizeof(cmtbuf));
                c.close();
                size_t before = len;
                len = webdav_ms_entry(s_dav.buf, cap, len, chref, cdir, csize, cmtbuf, cdir ? "" : mime_type(base));
                if (len == before)
                    break; // buffer full - stop listing
                count++;
            }
        }
        f.close();
        len = webdav_ms_end(s_dav.buf, cap, len);
        send(slot_id, 207, "application/xml; charset=utf-8", s_dav.buf);
        return;
    }

    case WebDavMethod::DAV_M_PROPPATCH: {
        // Read-only properties (no dead-property store): answer 207 with each
        // requested property refused 403, rather than 405 - keeps Explorer/Finder,
        // which PROPPATCH a timestamp right after a PUT, from erroring.
        if (!fsys.exists(fs_path))
        {
            dav_send_status(slot_id, 404, "");
            return;
        }
        size_t n = webdav_proppatch_ms(s_dav.buf, sizeof(s_dav.buf), req->path, (const char *)req->body, req->body_len);
        if (!n)
        {
            dav_send_status(slot_id, 507, ""); // Insufficient Storage: response did not fit the buffer
            return;
        }
        send(slot_id, 207, "application/xml; charset=utf-8", s_dav.buf);
        return;
    }

    case WebDavMethod::DAV_M_UNSUPPORTED:
    default:
        dav_send_status(
            slot_id, 405,
            "Allow: OPTIONS, GET, HEAD, PUT, DELETE, PROPFIND, PROPPATCH, MKCOL, COPY, MOVE, LOCK, UNLOCK\r\n");
        return;
    }
}
#endif // DETWS_ENABLE_WEBDAV
