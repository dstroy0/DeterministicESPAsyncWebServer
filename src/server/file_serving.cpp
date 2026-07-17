// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file file_serving.cpp
 * @brief Filesystem-backed static file serving for DetWebServer (GET/HEAD of an fs::FS path).
 *
 * Split out of dwserver.cpp (single-purpose server files). Covers the conditional-GET validators
 * (ETag / Last-Modified / If-None-Match / If-Modified-Since), byte-range requests (RFC 7233),
 * pre-compressed .gz variants, and the cross-loop file-send pump that pages a large body out
 * without truncating or blocking the worker. The shared RFC 1123 date helper (http_rfc1123)
 * lives here because WEBDAV requires FILE_SERVING, so this TU is its single always-present home.
 * Behaviour is identical to the pre-split code - a pure move.
 */

#include "dwserver.h"
#include "network_drivers/transport/tcp.h" // conn_pool, det_conn_*, TcpConn/ConnState
#include "server/dwserver_internal.h"
#include "server/http_range.h"      // http_parse_byte_range (shared with the edge cache)
#include "shared_primitives/mime.h" // mime_type, DET_MIME_*
#include <stdio.h>                  // snprintf, sscanf
#include <string.h>                 // strncasecmp, strchr, strstr, strncmp, strnlen
#include <time.h>                   // gmtime_r, strftime (RFC 1123 / conditional-GET dates)

// ---------------------------------------------------------------------------
// File serving
// ---------------------------------------------------------------------------

#if DETWS_ENABLE_FILE_SERVING
// HTTP-date helpers (shared by file serving's Last-Modified / If-Modified-Since and
// WebDAV's getlastmodified / creationdate). WEBDAV requires FILE_SERVING, so this is
// the single home for both. Format a time_t as an RFC 1123 GMT date; leaves @p out
// empty when the timestamp is zero/unavailable.
void http_rfc1123(time_t t, char *out, size_t cap)
{
    out[0] = '\0';
    if (t <= 0)
        return;
    struct tm tmv;
    if (!gmtime_r(&t, &tmv)) // reentrant: never the shared static buffer (worker-safe)
        return;
    strftime(out, cap, "%a, %d %b %Y %H:%M:%S GMT", &tmv);
}

// True if a resource last modified at @p mtime is NOT newer than the client's
// If-Modified-Since date @p ims (RFC 1123 form), i.e. a conditional GET should
// answer 304. Parses the date by hand (sscanf, no stdlib) and compares the two
// broken-down times field by field, so no timegm()/epoch round-trip is needed.
// Returns false (serve 200) when there is no usable date - mtime is 0 (no clock),
// @p ims is absent, or it does not parse.
static bool http_not_modified_since(time_t mtime, const char *ims)
{
    if (mtime <= 0 || !ims)
        return false;
    char mon[4] = {0};
    int day = 0;
    int year = 0;
    int hh = 0;
    int mm = 0;
    int ss = 0;
    // "Sun, 06 Nov 1994 08:49:37 GMT" - skip the weekday, read the rest.
    if (sscanf(ims, "%*3s, %d %3s %d %d:%d:%d", &day, mon, &year, &hh, &mm, &ss) != 6)
        return false;
    static const char MONTHS[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    const char *mp = strstr(MONTHS, mon);
    // Must align to a 3-char month boundary: a malformed token like "ebM" appears in
    // the table at a non-multiple-of-3 offset and would otherwise mis-parse as a month.
    if (!mp || ((mp - MONTHS) % 3) != 0)
        return false;
    int imon = (int)(mp - MONTHS) / 3; // 0-based, matches struct tm tm_mon

    struct tm tf;
    if (!gmtime_r(&mtime, &tf)) // reentrant: never the shared static buffer (worker-safe)
        return false;
    // Compare file (tf) vs If-Modified-Since fields, most significant first.
    int fy = tf.tm_year + 1900;
    if (fy != year)
        return fy < year;
    if (tf.tm_mon != imon)
        return tf.tm_mon < imon;
    if (tf.tm_mday != day)
        return tf.tm_mday < day;
    if (tf.tm_hour != hh)
        return tf.tm_hour < hh;
    if (tf.tm_min != mm)
        return tf.tm_min < mm;
    return tf.tm_sec <= ss;
}

// RFC 9110 13.1.2: If-None-Match comparison. Supports "*" (matches any current
// representation), a comma-separated list of entity-tags, and weak comparison
// (an inbound W/"x" matches our strong "x"). @p etag is our tag, quotes included.
static bool inm_matches(const char *inm, const char *etag)
{
    while (*inm == ' ' || *inm == '\t')
        inm++; // GCOVR_EXCL_LINE http_parser strips leading OWS from header values, so inm never starts with WS
    if (inm[0] == '*')
        return true; // "*" matches the existing representation
    size_t etlen = strnlen(etag, 40);
    const char *p = inm;
    while (*p)
    {
        while (*p == ' ' || *p == '\t' || *p == ',')
            p++;
        if (!*p)
            break;
        const char *tag = p;
        if (tag[0] == 'W' && tag[1] == '/') // weak validator: ignore the W/ prefix
            tag += 2;
        if (tag[0] == '"')
        {
            const char *end = strchr(tag + 1, '"');
            if (end && (size_t)(end - tag + 1) == etlen && strncmp(tag, etag, etlen) == 0)
                return true;
        }
        const char *comma = strchr(p, ',');
        if (!comma)
            break;
        p = comma + 1;
    }
    return false;
}

void DetWebServer::serve_file_internal(uint8_t slot_id, bool head, fs::FS &file_sys, const char *fs_path,
                                       const char *content_type, const char *content_encoding)
{
    fs::File f = file_sys.open(fs_path, "r");
    if (!f)
    {
        send(slot_id, 404, DET_MIME_TEXT_PLAIN, "Not Found");
        return;
    }

    if (!det_conn_active(slot_id))
    {
        f.close();
        http_reset(slot_id);
        return;
    }

    size_t file_size = f.size();

    bool keep;
    const char *cl = resp_conn_hdr(slot_id, &keep);

    // Optional Content-Encoding line (e.g. gzip for pre-compressed assets).
    char enc_line[40];
    enc_line[0] = '\0';
    if (content_encoding)
        snprintf(enc_line, sizeof(enc_line), "Content-Encoding: %s\r\n", content_encoding);

#if DETWS_ENABLE_ETAG
    // Conditional GET. Strong validator (ETag) from size + mtime; plus a
    // Last-Modified date validator. A conditional request answers 304 when either
    // the client's If-None-Match matches the ETag, or - per RFC 9110, only when no
    // If-None-Match is present - its If-Modified-Since is not older than the file.
    time_t mtime = f.getLastWrite();
    char etag[40];
    snprintf(etag, sizeof(etag), "\"%x-%lx\"", (unsigned)file_size, (unsigned long)mtime);

    char lm_date[40];
    char lastmod_line[17 + sizeof(lm_date)]; // "Last-Modified: " + date + "\r\n" + NUL
    lastmod_line[0] = '\0';
    http_rfc1123(mtime, lm_date, sizeof(lm_date));
    if (lm_date[0])
        snprintf(lastmod_line, sizeof(lastmod_line), "Last-Modified: %s\r\n", lm_date);

    const char *inm = http_get_header(&http_pool[slot_id], "If-None-Match");
    bool not_modified = inm ? inm_matches(inm, etag)
                            : http_not_modified_since(mtime, http_get_header(&http_pool[slot_id], "If-Modified-Since"));
    if (not_modified)
    {
        f.close();
        char h304[RESP_HDR_BUF_SIZE];
        int n304 = snprintf(h304, sizeof(h304), "HTTP/1.1 304 Not Modified\r\nETag: %s\r\n%s%s%s%s\r\n", etag,
                            lastmod_line, _cache_control_buf, _cors_enabled ? _cors_header_buf : "", cl);
        det_conn_send_flush(slot_id, h304, (u16_t)n304); // 304s are frequent (cache revalidation): one marshal
        resp_end(slot_id, 304, 0, keep, /*pre_flushed=*/true);
        return;
    }
    char etag_line[48];
    snprintf(etag_line, sizeof(etag_line), "ETag: %s\r\n", etag);
#else
    const char *etag_line = "";
    const char *lastmod_line = "";
#endif

    // Default: full 200 response covering the whole file.
    int status = 200;
    size_t body_len = file_size;
    size_t body_off = 0; // file offset the body starts at (nonzero for a Range)
    const char *accept_ranges = "";
    char range_line[64];
    range_line[0] = '\0';

#if DETWS_ENABLE_RANGE
    accept_ranges = "Accept-Ranges: bytes\r\n"; // advertise range support on every file response
    size_t r_start = 0;
    size_t r_end = 0;
    int rr = http_parse_byte_range(http_get_header(&http_pool[slot_id], "Range"), file_size, &r_start, &r_end);
    if (rr < 0)
    {
        // Unsatisfiable range -> 416 with Content-Range: bytes */<size>.
        f.close();
        char h416[RESP_HDR_BUF_SIZE];
        int n416 = snprintf(h416, sizeof(h416),
                            "HTTP/1.1 416 Range Not Satisfiable\r\n"
                            "Content-Range: bytes */%u\r\n"
                            "Content-Length: 0\r\n"
                            "%s%s\r\n",
                            (unsigned)file_size, _cors_enabled ? _cors_header_buf : "", cl);
        det_conn_send_flush(slot_id, h416, (u16_t)n416);
        resp_end(slot_id, 416, 0, keep, /*pre_flushed=*/true);
        return;
    }
    if (rr > 0)
    {
        status = 206;
        body_len = r_end - r_start + 1;
        snprintf(range_line, sizeof(range_line), "Content-Range: bytes %u-%u/%u\r\n", (unsigned)r_start,
                 (unsigned)r_end, (unsigned)file_size);
        f.seek((uint32_t)r_start);
        body_off = r_start;
    }
#endif

    char header[RESP_HDR_BUF_SIZE];
    int hlen =
        snprintf(header, sizeof(header),
                 "HTTP/1.1 %d %s\r\n"
                 "Content-Type: %s\r\n"
                 "Content-Length: %u\r\n"
                 "%s%s%s%s%s%s%s"
                 "%s\r\n",
                 status, status_text(status), content_type, (unsigned)body_len, accept_ranges, range_line, enc_line,
                 etag_line, lastmod_line, _cache_control_buf, _cors_enabled ? _cors_header_buf : "", cl);

    det_conn_send(slot_id, header, (u16_t)hlen);

    // HEAD or empty body: headers only, finish now.
    if (head || body_len == 0)
    {
        f.close();
        resp_end(slot_id, status, 0, keep);
        return;
    }

    // Hand the body to the cross-loop pump: it pages out at most one send-buffer
    // window now and resumes on later loops as the window drains, so a file larger
    // than TCP_SND_BUF is never truncated. The pump owns the file and calls
    // resp_end() at completion - do not close f or end the response here.
    FileSend &s = s_send.file[slot_id];
    s.file = f; // shared handle on ARDUINO; the local f going out of scope keeps it open
    s.off = body_off;
    s.remaining = body_len;
    s.status = status;
    s.total = (int)body_len;
    s.keep = keep;
    s.active = true;
    file_send_pump(slot_id);
}

// Page out a pending file response across worker loops: send up to det_conn_sndbuf()
// bytes now and return; the next loop resumes (woken by the sent callback) until the
// whole body has been queued, then finish the response. Bounded per loop, never
// truncates, never blocks the worker.
void DetWebServer::file_send_pump(uint8_t slot_id)
{
    FileSend &s = s_send.file[slot_id];
    if (!s.active)
        return;

    if (!det_conn_active(slot_id))
    {
        // Connection went away mid-transfer: drop the source and the continuation.
        s.file.close();
        s.active = false;
        return;
    }

    // A file body still being paged out is active, not idle: keep the CONN_TIMEOUT_MS idle sweep
    // off it so a transient send stall on a large file cannot reap the slot mid-transfer.
    det_conn_touch_active(slot_id);

    uint8_t chunk[FILE_CHUNK_SIZE];
    while (s.remaining > 0)
    {
        u16_t avail = det_conn_sndbuf(slot_id);
        if (avail == 0)
        {
            det_conn_flush(slot_id); // push what is queued; resume on a later loop
            return;
        }
        size_t want = s.remaining < sizeof(chunk) ? s.remaining : sizeof(chunk);
        if (want > avail)
            want = avail;
        size_t n = s.file.read(chunk, want);
        if (n == 0)
        {
            s.remaining = 0; // read error / short file: stop (response will be short)
            break;
        }
        if (!det_conn_send(slot_id, chunk, (u16_t)n))
        {
            s.file.seek((uint32_t)s.off); // un-read the bytes that did not go out; retry next loop
            det_conn_flush(slot_id);
            return;
        }
        s.off += n;
        s.remaining -= n;
    }

    // Whole body queued: finish the response (flush, keep-alive/close, log, reset).
    s.file.close();
    s.active = false;
    det_conn_flush(slot_id);
    resp_end(slot_id, s.status, s.total, s.keep);
}

void DetWebServer::serve_file(uint8_t slot_id, fs::FS &file_sys, const char *fs_path, const char *content_type)
{
    serve_file_internal(slot_id, req_is_head(slot_id), file_sys, fs_path, content_type, nullptr);
}

void DetWebServer::serve_static(const char *url_prefix, fs::FS &file_sys, const char *fs_root)
{
    if (_route_count >= MAX_ROUTES)
        return;
    Route *r = &_routes[_route_count++];

    // Store the pattern as a wildcard so path_matches() does a prefix match.
    char pat[MAX_PATH_LEN];
    size_t n = strnlen(url_prefix, MAX_PATH_LEN);
    if (n > 0 && url_prefix[n - 1] == '*')
        snprintf(pat, sizeof(pat), "%s", url_prefix); // already a wildcard
    else
        snprintf(pat, sizeof(pat), "%s*", url_prefix); // append the wildcard
    fill_route_base(r, pat);
    r->type = RouteType::ROUTE_STATIC;
    r->method = HttpMethod::HTTP_GET;
    r->static_fs = &file_sys;
    r->static_root = fs_root;
}

void DetWebServer::serve_static_request(uint8_t slot_id, HttpReq *req, const Route *r)
{
    if (!r->static_fs)
    {
        send(slot_id, 404, DET_MIME_TEXT_PLAIN, "Not Found");
        return;
    }

    // Request path beyond the mount prefix (route path minus its trailing '*').
    size_t plen = strnlen(r->path, MAX_PATH_LEN);
    if (plen > 0 && r->path[plen - 1] == '*')
        plen--;
    const char *sub = (strnlen(req->path, MAX_PATH_LEN) >= plen) ? req->path + plen : "";

    // Reject path traversal before touching the filesystem.
    if (strstr(sub, ".."))
    {
        send(slot_id, 404, DET_MIME_TEXT_PLAIN, "Not Found");
        return;
    }

    const char *root = r->static_root ? r->static_root : "";
    size_t rlen = strnlen(root, MAX_PATH_LEN);
    bool root_slash = (rlen > 0 && root[rlen - 1] == '/');
    if (root_slash && sub[0] == '/') // avoid a doubled separator
        sub++;
    bool sub_slash = (sub[0] == '/');
    const char *sep = (root_slash || sub_slash) ? "" : "/";

    // Directory or bare-prefix request → index.html.
    size_t slen = strnlen(sub, MAX_PATH_LEN);
    bool dir = (slen == 0) || (sub[slen - 1] == '/');

    char fs_path[256];
    int wn = dir ? snprintf(fs_path, sizeof(fs_path), "%s%s%sindex.html", root, sep, sub)
                 : snprintf(fs_path, sizeof(fs_path), "%s%s%s", root, sep, sub);
    if (wn <= 0 || wn >= (int)sizeof(fs_path))
    {
        send(slot_id, 404, DET_MIME_TEXT_PLAIN, "Not Found");
        return;
    }

    const char *ctype = mime_type(fs_path);
    bool head = req_is_head(slot_id);

    // Pre-compressed variant: serve <path>.gz if the client accepts gzip and it
    // exists. Content-Type stays that of the original (uncompressed) resource.
    const char *ae = http_get_header(req, "Accept-Encoding");
    if (ae && strstr(ae, "gzip"))
    {
        char gz[260];
        int gn = snprintf(gz, sizeof(gz), "%s.gz", fs_path);
        if (gn > 0 && gn < (int)sizeof(gz) && r->static_fs->exists(gz))
        {
            serve_file_internal(slot_id, head, *r->static_fs, gz, ctype, "gzip");
            return;
        }
    }

    serve_file_internal(slot_id, head, *r->static_fs, fs_path, ctype, nullptr);
}
#endif // DETWS_ENABLE_FILE_SERVING
