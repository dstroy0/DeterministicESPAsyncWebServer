// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file multipart.cpp
 * @brief In-place multipart/form-data parser implementation.
 */

#include "multipart.h"
#include <string.h>

// Skip past a CRLF pair; returns p+2 if CRLF found, else p unchanged.
static char *skip_crlf(char *p)
{
    if (p[0] == '\r' && p[1] == '\n')
        return p + 2;
    return p;
}

// Extract parameter value: search for `key="<value>"` inside `src`.
// If found, null-terminates in-place and returns pointer to the value.
// Returns nullptr if not found.
static char *extract_quoted_param(char *src, const char *key)
{
    char *p = strstr(src, key);
    if (!p)
        return nullptr;
    p += strlen(key);
    if (*p != '"')
        return nullptr;
    p++; // skip opening quote
    char *end = strchr(p, '"');
    if (!end)
        return nullptr;
    *end = '\0';
    return p;
}

bool multipart_parse(HttpReq *req, Multipart *mp)
{
    mp->part_count = 0;

    const char *ct = http_get_header(req, "Content-Type");
    if (!ct)
        return false;

    // Extract boundary value (may be quoted or unquoted)
    const char *bsearch = strstr(ct, "boundary=");
    if (!bsearch)
        return false;
    bsearch += 9;
    if (*bsearch == '"')
        bsearch++;

    char bval[MAX_BOUNDARY_LEN + 1];
    size_t blen = 0;
    while (*bsearch && *bsearch != '"' && *bsearch != ';' && *bsearch != ' '
           && blen < MAX_BOUNDARY_LEN)
        bval[blen++] = *bsearch++;
    bval[blen] = '\0';

    if (blen == 0)
        return false;

    // Delimiter is "--" + boundary
    char delim[MAX_BOUNDARY_LEN + 3];
    delim[0] = '-';
    delim[1] = '-';
    memcpy(delim + 2, bval, blen + 1); // includes null
    size_t dlen = blen + 2;

    char *body = (char *)req->body;

    // Find the first delimiter
    char *pos = strstr(body, delim);
    if (!pos)
        return false;

    pos += (int)dlen;
    pos = skip_crlf(pos);

    while (mp->part_count < MAX_MULTIPART_PARTS)
    {
        // End boundary is "--" immediately after delimiter
        if (pos[0] == '-' && pos[1] == '-')
            break;

        MultipartPart *part = &mp->parts[mp->part_count];
        part->name     = nullptr;
        part->filename = nullptr;
        part->type     = nullptr;
        part->data     = nullptr;
        part->data_len = 0;

        // Parse per-part headers until the blank line
        for (;;)
        {
            if (pos[0] == '\r' && pos[1] == '\n')
            {
                pos += 2; // blank line → start of data
                break;
            }

            char *line_end = strstr(pos, "\r\n");
            if (!line_end)
                return false;

            *line_end = '\0'; // null-terminate header line

            if (strncasecmp(pos, "Content-Disposition:", 20) == 0)
            {
                char *v = pos + 20;
                while (*v == ' ')
                    v++;
                // Extract name and filename (in-place, with null termination)
                // Extract filename before name: filename= appears after name= in the
                // header, so extracting it first avoids corrupting name='s search
                // when extract_quoted_param null-terminates the value in-place.
                part->filename = extract_quoted_param(v, "filename=");
                part->name     = extract_quoted_param(v, "name=");
            }
            else if (strncasecmp(pos, "Content-Type:", 13) == 0)
            {
                char *v = pos + 13;
                while (*v == ' ')
                    v++;
                part->type = v;
            }

            pos = line_end + 2; // next line (skip '\0' + '\n')
        }

        // Data runs from pos until the next delimiter (preceded by \r\n)
        part->data   = pos;
        char *next   = strstr(pos, delim);
        if (!next)
            return false;

        char *data_end  = next - 2; // \r\n before the delimiter
        part->data_len  = (size_t)(data_end - pos);
        *data_end       = '\0'; // null-terminate data

        mp->part_count++;

        pos = next + (int)dlen;
        pos = skip_crlf(pos);
    }

    return mp->part_count > 0;
}

const char *multipart_get_field(const Multipart *mp, const char *field)
{
    for (int i = 0; i < mp->part_count; i++)
    {
        if (mp->parts[i].name && strcmp(mp->parts[i].name, field) == 0)
            return mp->parts[i].data;
    }
    return nullptr;
}
