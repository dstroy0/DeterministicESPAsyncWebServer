// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file webdav.cpp
 * @brief WebDAV server core (RFC 4918): method classification, header parsing,
 *        and the 207 Multi-Status XML builder. Pure - no sockets, no filesystem.
 */

#include "services/webdav/webdav.h"

#if DETWS_ENABLE_WEBDAV

#include <string.h>

WebDavMethod webdav_method(const char *m)
{
    if (!m)
        return DAV_M_UNSUPPORTED;
    if (!strcmp(m, "OPTIONS"))
        return DAV_M_OPTIONS;
    if (!strcmp(m, "GET"))
        return DAV_M_GET;
    if (!strcmp(m, "HEAD"))
        return DAV_M_HEAD;
    if (!strcmp(m, "PUT"))
        return DAV_M_PUT;
    if (!strcmp(m, "DELETE"))
        return DAV_M_DELETE;
    if (!strcmp(m, "PROPFIND"))
        return DAV_M_PROPFIND;
    if (!strcmp(m, "PROPPATCH"))
        return DAV_M_PROPPATCH;
    if (!strcmp(m, "MKCOL"))
        return DAV_M_MKCOL;
    if (!strcmp(m, "COPY"))
        return DAV_M_COPY;
    if (!strcmp(m, "MOVE"))
        return DAV_M_MOVE;
    if (!strcmp(m, "LOCK"))
        return DAV_M_LOCK;
    if (!strcmp(m, "UNLOCK"))
        return DAV_M_UNLOCK;
    return DAV_M_UNSUPPORTED;
}

int webdav_depth(const char *depth_hdr, int dflt)
{
    if (!depth_hdr || !depth_hdr[0])
        return dflt;
    if (!strcmp(depth_hdr, "0"))
        return 0;
    if (!strcmp(depth_hdr, "1"))
        return 1;
    if (!strcmp(depth_hdr, "infinity"))
        return DAV_DEPTH_INFINITY;
    return dflt;
}

// Append a NUL-terminated string if it fits; returns false (leaving *len and the
// NUL terminator intact) when it would overflow.
static bool app(char *buf, size_t cap, size_t *len, const char *s)
{
    size_t n = strlen(s);
    if (*len + n + 1 > cap)
        return false;
    memcpy(buf + *len, s, n);
    *len += n;
    buf[*len] = '\0';
    return true;
}

size_t webdav_xml_escape(char *dst, size_t cap, const char *src)
{
    size_t o = 0;
    if (cap == 0)
        return 0;
    for (const char *p = src; *p; p++)
    {
        const char *rep = nullptr;
        switch (*p)
        {
        case '&':
            rep = "&amp;";
            break;
        case '<':
            rep = "&lt;";
            break;
        case '>':
            rep = "&gt;";
            break;
        case '"':
            rep = "&quot;";
            break;
        case '\'':
            rep = "&apos;";
            break;
        default:
            break;
        }
        if (rep)
        {
            size_t rn = strlen(rep);
            if (o + rn + 1 > cap)
                break;
            memcpy(dst + o, rep, rn);
            o += rn;
        }
        else
        {
            if (o + 1 + 1 > cap)
                break;
            dst[o++] = *p;
        }
    }
    dst[o] = '\0';
    return o;
}

// Hex digit value, or -1.
static int hexval(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

bool webdav_dest_path(const char *destination, char *out, size_t cap)
{
    if (!destination || !out || cap == 0)
        return false;

    // Skip an absolute-URI scheme + authority: after "://", advance to the first
    // '/' (the path). An abs-path value ("/p/q") is used as-is.
    const char *p = destination;
    const char *scheme = strstr(destination, "://");
    if (scheme)
    {
        p = scheme + 3;
        while (*p && *p != '/')
            p++;
        if (*p != '/')
            return false; // authority with no path
    }
    else if (*p != '/')
    {
        return false; // not an absolute path
    }

    // Percent-decode into out.
    size_t o = 0;
    for (; *p; p++)
    {
        char c = *p;
        if (c == '%')
        {
            int hi = hexval(p[1]);
            int lo = (hi >= 0) ? hexval(p[2]) : -1;
            if (hi < 0 || lo < 0)
                return false; // malformed escape
            c = (char)((hi << 4) | lo);
            p += 2;
        }
        if (o + 1 >= cap)
            return false; // no room for char + NUL
        out[o++] = c;
    }
    out[o] = '\0';
    return true;
}

size_t webdav_ms_begin(char *buf, size_t cap, size_t len)
{
    app(buf, cap, &len, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<D:multistatus xmlns:D=\"DAV:\">\n");
    return len;
}

size_t webdav_ms_entry(char *buf, size_t cap, size_t len, const char *href, bool is_collection, uint32_t size,
                       const char *rfc1123_mtime, const char *content_type)
{
    // Build the whole <response> in a temp first so the append is atomic: a
    // partial element is never left in the document when the buffer fills.
    char tmp[512];
    size_t t = 0;
    char esc[256];

    webdav_xml_escape(esc, sizeof(esc), href);
    if (!app(tmp, sizeof(tmp), &t, "  <D:response>\n    <D:href>") || !app(tmp, sizeof(tmp), &t, esc) ||
        !app(tmp, sizeof(tmp), &t, "</D:href>\n    <D:propstat>\n      <D:prop>\n        <D:resourcetype>"))
        return len;

    if (is_collection)
    {
        if (!app(tmp, sizeof(tmp), &t, "<D:collection/>"))
            return len;
    }
    if (!app(tmp, sizeof(tmp), &t, "</D:resourcetype>\n"))
        return len;

    if (!is_collection)
    {
        char num[24];
        unsigned long s = (unsigned long)size;
        // minimal itoa to avoid pulling in snprintf in the pure core
        char rev[24];
        int rn = 0;
        do
        {
            rev[rn++] = (char)('0' + (int)(s % 10));
            s /= 10;
        } while (s && rn < (int)sizeof(rev));
        int ni = 0;
        while (rn > 0)
            num[ni++] = rev[--rn];
        num[ni] = '\0';
        if (!app(tmp, sizeof(tmp), &t, "        <D:getcontentlength>") || !app(tmp, sizeof(tmp), &t, num) ||
            !app(tmp, sizeof(tmp), &t, "</D:getcontentlength>\n"))
            return len;
        if (content_type && content_type[0])
        {
            if (!app(tmp, sizeof(tmp), &t, "        <D:getcontenttype>") || !app(tmp, sizeof(tmp), &t, content_type) ||
                !app(tmp, sizeof(tmp), &t, "</D:getcontenttype>\n"))
                return len;
        }
    }

    if (rfc1123_mtime && rfc1123_mtime[0])
    {
        if (!app(tmp, sizeof(tmp), &t, "        <D:getlastmodified>") || !app(tmp, sizeof(tmp), &t, rfc1123_mtime) ||
            !app(tmp, sizeof(tmp), &t, "</D:getlastmodified>\n"))
            return len;
    }

    if (!app(tmp, sizeof(tmp), &t,
             "      </D:prop>\n      <D:status>HTTP/1.1 200 OK</D:status>\n"
             "    </D:propstat>\n  </D:response>\n"))
        return len;

    // Atomic commit: append the finished element only if it fits.
    if (!app(buf, cap, &len, tmp))
        return len; // no room - caller stops adding entries
    return len;
}

size_t webdav_ms_end(char *buf, size_t cap, size_t len)
{
    app(buf, cap, &len, "</D:multistatus>\n");
    return len;
}

// True for a byte that ends an XML element name (whitespace, '/', '>').
static bool name_end_char(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '/' || c == '>';
}

size_t webdav_proppatch_ms(char *buf, size_t cap, const char *href, const char *body, size_t body_len)
{
    size_t len = 0;
    if (cap)
        buf[0] = '\0'; // always a valid C-string, even if nothing below fits
    char esc[256];
    webdav_xml_escape(esc, sizeof(esc), href);
    if (!app(buf, cap, &len,
             "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<D:multistatus xmlns:D=\"DAV:\">\n"
             "  <D:response>\n    <D:href>") ||
        !app(buf, cap, &len, esc) || !app(buf, cap, &len, "</D:href>\n    <D:propstat>\n      <D:prop>\n"))
        return 0;

    // Walk the request and echo every element that sits directly inside a <prop>
    // (across all <set>/<remove> blocks) as a self-closed element. The wrappers
    // (propertyupdate / set / remove / prop) are skipped; only the properties are
    // reflected, each refused 403 below.
    int emitted = 0;
    bool in_prop = false;
    size_t i = 0;
    while (i < body_len && emitted < DETWS_WEBDAV_MAX_PROPS)
    {
        if (body[i] != '<')
        {
            i++;
            continue;
        }
        size_t start = i + 1;
        if (start < body_len && (body[start] == '?' || body[start] == '!'))
        {
            while (i < body_len && body[i] != '>') // skip PI / comment / declaration
                i++;
            i++;
            continue;
        }
        bool closing = (start < body_len && body[start] == '/');
        if (closing)
            start++;
        size_t end = start;
        while (end < body_len && body[end] != '>')
            end++;
        if (end >= body_len)
            break; // unterminated tag
        bool self_closed = (end > start && body[end - 1] == '/');
        size_t name_end = start;
        while (name_end < end && !name_end_char(body[name_end]))
            name_end++;
        size_t local = start; // local name = after the last ':' in the qualified name
        for (size_t k = start; k < name_end; k++)
            if (body[k] == ':')
                local = k + 1;
        bool is_prop = (name_end - local) == 4 && !strncmp(&body[local], "prop", 4);

        if (closing)
        {
            if (is_prop)
                in_prop = false;
            i = end + 1;
            continue;
        }
        if (is_prop)
        {
            if (!self_closed) // <prop> opens a block; <prop/> is an empty block
                in_prop = true;
            i = end + 1;
            continue;
        }
        if (in_prop)
        {
            // Echo this property element self-closed. Copy the open-tag content
            // (name + its own xmlns/attrs), dropping a trailing '/' and trailing
            // whitespace; reject a span containing '<' so nothing is injected.
            size_t copy_end = self_closed ? end - 1 : end;
            while (copy_end > start && (body[copy_end - 1] == ' ' || body[copy_end - 1] == '\t' ||
                                        body[copy_end - 1] == '\r' || body[copy_end - 1] == '\n'))
                copy_end--;
            bool ok = copy_end > start;
            for (size_t k = start; k < copy_end && ok; k++)
                if (body[k] == '<')
                    ok = false;
            char tag[256];
            size_t tl = copy_end - start;
            if (ok && tl < sizeof(tag))
            {
                memcpy(tag, &body[start], tl);
                tag[tl] = '\0';
                if (app(buf, cap, &len, "        <") && app(buf, cap, &len, tag) && app(buf, cap, &len, "/>\n"))
                    emitted++;
            }
            if (!self_closed)
            {
                // Skip the property's value up to its close tag (no nesting expected).
                size_t j = end + 1;
                while (j + 1 < body_len && !(body[j] == '<' && body[j + 1] == '/'))
                    j++;
                while (j < body_len && body[j] != '>')
                    j++;
                i = (j < body_len) ? j + 1 : body_len;
            }
            else
                i = end + 1;
            continue;
        }
        i = end + 1;
    }

    if (!app(buf, cap, &len,
             "      </D:prop>\n      <D:status>HTTP/1.1 403 Forbidden</D:status>\n"
             "    </D:propstat>\n  </D:response>\n</D:multistatus>\n"))
        return 0;
    return len;
}

#endif // DETWS_ENABLE_WEBDAV
