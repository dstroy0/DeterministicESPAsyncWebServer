// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file hostlink.cpp
 * @brief Omron Host Link (C-mode) frame builder + parser (pure, host-tested).
 */

#include "services/hostlink/hostlink.h"

#if DWS_ENABLE_HOSTLINK

#include <string.h>

uint8_t hostlink_fcs(const char *data, size_t len)
{
    uint8_t f = 0;
    for (size_t i = 0; i < len; i++)
        f ^= (uint8_t)data[i];
    return f;
}

static char hex_digit(uint8_t v)
{
    return (char)(v < 10 ? '0' + v : 'A' + (v - 10));
}

// Parse one hex digit (0-9 A-F a-f) into 0..15, or -1 if invalid.
static int hex_val(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

size_t hostlink_build(char *buf, size_t cap, uint8_t node, const char *header_code, const char *text, size_t text_len)
{
    if (!buf || !header_code || node > 99 || (text_len && !text))
        return 0;
    if (header_code[0] == '\0' || header_code[1] == '\0') // need exactly 2 header characters
        return 0;
    size_t total = 1 + 2 + 2 + text_len + 2 + 2; // @ + UU + XX + text + FCS + *CR
    if (total >= cap)                            // need room for the frame + a NUL terminator
        return 0;

    size_t p = 0;
    buf[p++] = '@';
    buf[p++] = (char)('0' + node / 10);
    buf[p++] = (char)('0' + node % 10);
    buf[p++] = header_code[0];
    buf[p++] = header_code[1];
    if (text_len)
    {
        memcpy(buf + p, text, text_len);
        p += text_len;
    }
    uint8_t f = hostlink_fcs(buf, p); // XOR over '@' .. end of text
    buf[p++] = hex_digit((uint8_t)(f >> 4));
    buf[p++] = hex_digit((uint8_t)(f & 0x0F));
    buf[p++] = '*';
    buf[p++] = '\r';
    buf[p] = '\0'; // NUL-terminate so callers may treat the ASCII frame as a string (matches sdi12_build)
    return p;
}

bool hostlink_parse(const char *buf, size_t len, HostlinkFrame *out)
{
    // minimum: @ UU XX FF * CR = 1 + 2 + 2 + 2 + 1 + 1 = 9
    if (!buf || !out || len < 9)
        return false;
    if (buf[0] != '@' || buf[len - 1] != '\r' || buf[len - 2] != '*')
        return false;
    if (buf[1] < '0' || buf[1] > '9' || buf[2] < '0' || buf[2] > '9')
        return false;

    size_t fcs_pos = len - 4; // the two FCS chars precede '*' CR
    int hi = hex_val(buf[fcs_pos]);
    int lo = hex_val(buf[fcs_pos + 1]);
    if (hi < 0 || lo < 0)
        return false;
    uint8_t got = (uint8_t)((hi << 4) | lo);
    if (hostlink_fcs(buf, fcs_pos) != got) // XOR over '@' .. last text char
        return false;

    out->node = (uint8_t)((buf[1] - '0') * 10 + (buf[2] - '0'));
    out->header_code[0] = buf[3];
    out->header_code[1] = buf[4];
    out->header_code[2] = '\0';
    out->text = buf + 5;
    out->text_len = fcs_pos - 5;
    return true;
}

bool hostlink_end_code(const HostlinkFrame *f, uint8_t *code)
{
    if (!f || f->text_len < 2)
        return false;
    int hi = hex_val(f->text[0]);
    int lo = hex_val(f->text[1]);
    if (hi < 0 || lo < 0)
        return false;
    if (code)
        *code = (uint8_t)((hi << 4) | lo);
    return true;
}

#endif // DWS_ENABLE_HOSTLINK
