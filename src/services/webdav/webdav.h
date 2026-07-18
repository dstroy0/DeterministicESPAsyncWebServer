// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file webdav.h
 * @brief WebDAV server core (RFC 4918): method classification, header parsing,
 *        and the 207 Multi-Status XML builder.
 *
 * Mirrors the CoAP/SNMP split: this header declares the pure, host-testable core
 * (no sockets, no filesystem - unit-tested in env:native_webdav). The
 * filesystem-backed request handling (PROPFIND directory walk, PUT/MKCOL/DELETE/
 * COPY/MOVE, GET via the file-serving path) lives in DWS::serve_dav_*
 * and runs only on a build with a real Arduino FS.
 *
 * Scope: class 1 (PROPFIND Depth 0/1, PROPPATCH, PUT, DELETE, MKCOL, COPY, MOVE)
 * plus OPTIONS and advisory LOCK/UNLOCK (a synthetic token is issued but not
 * enforced). PROPPATCH is answered 207 with every requested property refused 403
 * (read-only live properties, no dead-property store). The filesystem-backed
 * handler streams a PUT body straight to the file (DWS's stream-body
 * hook), so uploads are not bounded by BODY_BUF_SIZE.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_WEBDAV_H
#define DETERMINISTICESPASYNCWEBSERVER_WEBDAV_H

#include "ServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DWS_ENABLE_WEBDAV

/** @brief WebDAV request methods recognized by the server. */
enum class WebDavMethod : uint8_t
{
    DAV_M_OPTIONS,
    DAV_M_GET,
    DAV_M_HEAD,
    DAV_M_PUT,
    DAV_M_DELETE,
    DAV_M_PROPFIND,
    DAV_M_PROPPATCH,
    DAV_M_MKCOL,
    DAV_M_COPY,
    DAV_M_MOVE,
    DAV_M_LOCK,
    DAV_M_UNLOCK,
    DAV_M_UNSUPPORTED ///< Anything else - answered 405 Method Not Allowed.
};

/** @brief "infinity" Depth value (RFC 4918 §10.2). */
/** @brief Depth: infinity sentinel (a lone constant). */
static constexpr int32_t DAV_DEPTH_INFINITY = 0x7fffffff;

/** @brief Classify an HTTP method token (e.g. "PROPFIND") into a WebDavMethod. */
WebDavMethod dws_webdav_method(const char *m);

/**
 * @brief Parse a Depth header value ("0", "1", or "infinity").
 * @return 0, 1, or DAV_DEPTH_INFINITY; @p dflt when @p depth_hdr is null/empty.
 */
int dws_webdav_depth(const char *depth_hdr, int dflt);

/**
 * @brief XML-escape @p src into @p dst (`&`, `<`, `>`, `"`, `'`).
 * @return length written (NUL-terminated; truncated to fit @p cap).
 */
size_t dws_webdav_xml_escape(char *dst, size_t cap, const char *src);

/**
 * @brief Extract and percent-decode the path of a Destination header.
 *
 * Accepts an absolute URI ("http://host/p/q") or an absolute path ("/p/q"); the
 * scheme + authority are skipped and `%xx` escapes are decoded into @p out.
 * @return false on overflow or a malformed value.
 */
bool dws_webdav_dest_path(const char *destination, char *out, size_t cap);

// 207 Multi-Status incremental builder. Each call appends to a buffer already
// holding @p len bytes and returns the new length, never exceeding @p cap. A
// return value equal to @p len means the fragment did not fit (the caller should
// stop adding entries and close the document).

/** @brief Write the XML prolog and the open <multistatus> element. */
size_t dws_webdav_ms_begin(char *buf, size_t cap, size_t len);

/**
 * @brief Append one <response> describing a resource.
 *
 * @param href           the resource's URL path (XML-escaped here).
 * @param is_collection  true for a directory (emits <collection/>).
 * @param size           content length (files only).
 * @param rfc1123_mtime  Last-Modified string, or "" to omit.
 * @param content_type   MIME type (files only), or "" to omit.
 */
size_t dws_webdav_ms_entry(char *buf, size_t cap, size_t len, const char *href, bool is_collection, uint32_t size,
                           const char *rfc1123_mtime, const char *content_type);

/** @brief Close the <multistatus> element. */
size_t dws_webdav_ms_end(char *buf, size_t cap, size_t len);

/**
 * @brief Build a complete 207 Multi-Status body answering a PROPPATCH.
 *
 * The server has no dead-property store and its live properties are read-only, so
 * every requested property is refused with 403 Forbidden. The property elements
 * are echoed verbatim (self-closed) from the request @p body so the client sees
 * exactly which properties it asked for, namespace declarations intact; a property
 * whose tag would contain a stray '<' is skipped (no XML injection). Up to
 * DWS_WEBDAV_MAX_PROPS properties are echoed.
 *
 * @param buf       destination buffer (whole document, NUL-terminated).
 * @param cap       buffer capacity.
 * @param href      the resource path (XML-escaped here).
 * @param body      the PROPPATCH request body (not required to be NUL-terminated).
 * @param body_len  length of @p body.
 * @return bytes written, or 0 if the document did not fit @p cap.
 */
size_t dws_webdav_proppatch_ms(char *buf, size_t cap, const char *href, const char *body, size_t body_len);

#endif // DWS_ENABLE_WEBDAV

#endif // DETERMINISTICESPASYNCWEBSERVER_WEBDAV_H
