// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file json.cpp
 * @brief Implementation of the zero-heap JSON writer and top-level reader.
 */

#include "json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// JsonWriter
// ---------------------------------------------------------------------------

JsonWriter::JsonWriter(char *buf, size_t cap)
    : _buf(buf), _cap(cap), _len(0), _ok(buf != nullptr && cap >= 1), _after_key(false), _depth(0)
{
    if (_ok)
        _buf[0] = '\0';
}

void JsonWriter::put(char c)
{
    if (!_ok)
        return;
    if (_len + 1 >= _cap) // leave room for the NUL
    {
        _ok = false;
        return;
    }
    _buf[_len++] = c;
    _buf[_len] = '\0';
}

void JsonWriter::put_raw(const char *s)
{
    if (!s)
        return;
    for (; *s; s++)
        put(*s);
}

void JsonWriter::put_escaped(const char *s)
{
    if (!s)
        return;
    for (; *s; s++)
    {
        unsigned char c = (unsigned char)*s;
        switch (c)
        {
        case '"':
            put('\\');
            put('"');
            break;
        case '\\':
            put('\\');
            put('\\');
            break;
        case '\n':
            put('\\');
            put('n');
            break;
        case '\r':
            put('\\');
            put('r');
            break;
        case '\t':
            put('\\');
            put('t');
            break;
        case '\b':
            put('\\');
            put('b');
            break;
        case '\f':
            put('\\');
            put('f');
            break;
        default:
            if (c < 0x20)
            {
                // Control char -> \u00XX
                static const char hexd[] = "0123456789abcdef";
                put('\\');
                put('u');
                put('0');
                put('0');
                put(hexd[(c >> 4) & 0x0f]);
                put(hexd[c & 0x0f]);
            }
            else
            {
                put((char)c);
            }
            break;
        }
    }
}

void JsonWriter::value_prefix()
{
    if (_after_key)
    {
        _after_key = false; // a value right after key(): the comma was its own
        return;
    }
    if (_depth > 0)
    {
        size_t lvl = (size_t)(_depth - 1);
        if (_need_comma[lvl])
            put(',');
        _need_comma[lvl] = true;
    }
}

void JsonWriter::push(char open)
{
    value_prefix();
    put(open);
    if (_depth < JSON_MAX_DEPTH)
    {
        _need_comma[_depth] = false;
        _depth++;
    }
    else
    {
        _ok = false; // nesting too deep
    }
}

void JsonWriter::pop(char close)
{
    put(close);
    if (_depth > 0)
        _depth--;
    else
        _ok = false; // unbalanced close
}

void JsonWriter::begin_object()
{
    push('{');
}
void JsonWriter::end_object()
{
    pop('}');
}
void JsonWriter::begin_array()
{
    push('[');
}
void JsonWriter::end_array()
{
    pop(']');
}

void JsonWriter::key(const char *k)
{
    value_prefix();
    put('"');
    put_escaped(k);
    put('"');
    put(':');
    _after_key = true; // suppress the following value's own comma
}

void JsonWriter::str(const char *v)
{
    value_prefix();
    put('"');
    put_escaped(v);
    put('"');
}

void JsonWriter::integer(long v)
{
    char tmp[24];
    snprintf(tmp, sizeof(tmp), "%ld", v);
    value_prefix();
    put_raw(tmp);
}

void JsonWriter::uinteger(unsigned long v)
{
    char tmp[24];
    snprintf(tmp, sizeof(tmp), "%lu", v);
    value_prefix();
    put_raw(tmp);
}

void JsonWriter::boolean(bool v)
{
    value_prefix();
    put_raw(v ? "true" : "false");
}

void JsonWriter::null_value()
{
    value_prefix();
    put_raw("null");
}

void JsonWriter::raw(const char *literal)
{
    value_prefix();
    put_raw(literal);
}

void JsonWriter::kv_str(const char *k, const char *v)
{
    key(k);
    str(v);
}
void JsonWriter::kv_int(const char *k, long v)
{
    key(k);
    integer(v);
}
void JsonWriter::kv_uint(const char *k, unsigned long v)
{
    key(k);
    uinteger(v);
}
void JsonWriter::kv_bool(const char *k, bool v)
{
    key(k);
    boolean(v);
}
void JsonWriter::kv_null(const char *k)
{
    key(k);
    null_value();
}
void JsonWriter::kv_raw(const char *k, const char *literal)
{
    key(k);
    raw(literal);
}

// ---------------------------------------------------------------------------
// Reader (top-level object members)
// ---------------------------------------------------------------------------

static bool is_ws(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static const char *skip_ws(const char *p)
{
    while (*p && is_ws(*p))
        p++;
    return p;
}

// p points at the opening quote; returns the pointer just past the closing quote
// (or at the terminating NUL if unterminated). Honors backslash escapes.
static const char *skip_string(const char *p)
{
    p++; // opening quote
    while (*p)
    {
        if (*p == '\\' && p[1])
        {
            p += 2;
            continue;
        }
        if (*p == '"')
            return p + 1;
        p++;
    }
    return p;
}

// Skip one JSON value starting at p (ws already consumed). Returns the pointer
// just past the value.
static const char *skip_value(const char *p)
{
    if (*p == '"')
        return skip_string(p);
    if (*p == '{' || *p == '[')
    {
        char open = *p;
        char close = (open == '{') ? '}' : ']';
        int depth = 0;
        while (*p)
        {
            if (*p == '"')
            {
                p = skip_string(p);
                continue;
            }
            if (*p == open)
                depth++;
            else if (*p == close)
            {
                depth--;
                if (depth == 0)
                    return p + 1;
            }
            p++;
        }
        return p;
    }
    // primitive: number / true / false / null
    while (*p && *p != ',' && *p != '}' && *p != ']' && !is_ws(*p))
        p++;
    return p;
}

// Locate the value of a top-level @p key in object @p json. Returns a pointer to
// the first character of the value (ws-skipped), or nullptr if not found.
static const char *json_find_value(const char *json, const char *key)
{
    if (!json || !key)
        return nullptr;
    const char *p = skip_ws(json);
    if (*p != '{')
        return nullptr;
    p++; // into the object

    size_t keylen = strlen(key);
    while (true)
    {
        p = skip_ws(p);
        if (*p == '}' || *p == '\0')
            return nullptr;
        if (*p != '"')
            return nullptr; // expected a member name

        const char *kstart = p + 1;
        const char *kend = skip_string(p); // just past closing quote
        size_t klen = (kend > kstart) ? (size_t)((kend - 1) - kstart) : 0;
        bool match = (klen == keylen) && (strncmp(kstart, key, klen) == 0);

        p = skip_ws(kend);
        if (*p != ':')
            return nullptr;
        p = skip_ws(p + 1);

        if (match)
            return p;

        p = skip_value(p);
        p = skip_ws(p);
        if (*p == ',')
        {
            p++;
            continue;
        }
        return nullptr; // '}' or malformed
    }
}

static int hex_val(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

bool json_get_str(const char *json, const char *key, char *out, size_t out_cap)
{
    if (!out || out_cap == 0)
        return false;
    const char *v = json_find_value(json, key);
    if (!v || *v != '"')
        return false;

    const char *p = v + 1;
    size_t i = 0;
    while (*p && *p != '"')
    {
        char c = *p;
        if (c == '\\' && p[1])
        {
            p++;
            switch (*p)
            {
            case 'n':
                c = '\n';
                break;
            case 't':
                c = '\t';
                break;
            case 'r':
                c = '\r';
                break;
            case 'b':
                c = '\b';
                break;
            case 'f':
                c = '\f';
                break;
            case '"':
                c = '"';
                break;
            case '\\':
                c = '\\';
                break;
            case '/':
                c = '/';
                break;
            case 'u': {
                // \uXXXX: decode to a single byte when <= 0xFF, else '?'.
                int h1 = p[1] ? hex_val(p[1]) : -1;
                int h2 = (h1 >= 0 && p[2]) ? hex_val(p[2]) : -1;
                int h3 = (h2 >= 0 && p[3]) ? hex_val(p[3]) : -1;
                int h4 = (h3 >= 0 && p[4]) ? hex_val(p[4]) : -1;
                if (h4 >= 0)
                {
                    unsigned cp = (unsigned)((h1 << 12) | (h2 << 8) | (h3 << 4) | h4);
                    c = (cp <= 0xFF) ? (char)cp : '?';
                    p += 4;
                }
                else
                {
                    c = '?';
                }
                break;
            }
            default:
                c = *p;
                break;
            }
        }
        if (i + 1 < out_cap)
        {
            out[i++] = c;
        }
        else
        {
            out[i] = '\0'; // truncate to capacity
            return true;
        }
        p++;
    }
    out[i] = '\0';
    return true;
}

bool json_get_int(const char *json, const char *key, long *out)
{
    if (!out)
        return false;
    const char *v = json_find_value(json, key);
    if (!v || *v == '"') // must be a bare number, not a string
        return false;
    char *end = nullptr;
    long val = strtol(v, &end, 10);
    if (end == v)
        return false; // no digits parsed
    *out = val;
    return true;
}

bool json_get_bool(const char *json, const char *key, bool *out)
{
    if (!out)
        return false;
    const char *v = json_find_value(json, key);
    if (!v)
        return false;
    if (strncmp(v, "true", 4) == 0 && (v[4] == '\0' || v[4] == ',' || v[4] == '}' || v[4] == ']' || is_ws(v[4])))
    {
        *out = true;
        return true;
    }
    if (strncmp(v, "false", 5) == 0 && (v[5] == '\0' || v[5] == ',' || v[5] == '}' || v[5] == ']' || is_ws(v[5])))
    {
        *out = false;
        return true;
    }
    return false;
}
