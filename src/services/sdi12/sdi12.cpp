// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file sdi12.cpp
 * @brief SDI-12 command / response codec (pure, host-tested).
 */

#include "services/sdi12/sdi12.h"

#if DETWS_ENABLE_SDI12

size_t sdi12_build(char *buf, size_t cap, char addr, const char *body)
{
    if (!buf || !body)
        return 0;
    size_t blen = strlen(body);
    size_t n = 1 + blen + 1; // addr + body + '!'
    if (cap < n + 1)         // + room for the NUL terminator
        return 0;
    buf[0] = addr;
    memcpy(buf + 1, body, blen);
    buf[1 + blen] = '!';
    buf[n] = '\0';
    return n;
}

size_t sdi12_build_ack(char *buf, size_t cap, char addr)
{
    return sdi12_build(buf, cap, addr, "");
}

size_t sdi12_build_identify(char *buf, size_t cap, char addr)
{
    return sdi12_build(buf, cap, addr, "I");
}

size_t sdi12_build_measure(char *buf, size_t cap, char addr, bool with_crc)
{
    return sdi12_build(buf, cap, addr, with_crc ? "MC" : "M");
}

size_t sdi12_build_concurrent(char *buf, size_t cap, char addr, bool with_crc)
{
    return sdi12_build(buf, cap, addr, with_crc ? "CC" : "C");
}

size_t sdi12_build_data(char *buf, size_t cap, char addr, uint8_t d_index)
{
    if (d_index > 9)
        return 0;
    char body[3] = {'D', (char)('0' + d_index), '\0'};
    return sdi12_build(buf, cap, addr, body);
}

size_t sdi12_build_change_address(char *buf, size_t cap, char addr, char new_addr)
{
    char body[3] = {'A', new_addr, '\0'};
    return sdi12_build(buf, cap, addr, body);
}

size_t sdi12_build_query_address(char *buf, size_t cap)
{
    return sdi12_build(buf, cap, '?', "");
}

bool sdi12_parse_measure(const char *resp, size_t len, char *addr, uint16_t *ready_sec, uint8_t *num_values)
{
    if (!resp || len < 5) // a<ttt><n> is at least 5 octets
        return false;
    for (int i = 1; i <= 3; i++)
        if (!det_np_digit(resp[i]))
            return false;
    if (!det_np_digit(resp[4]))
        return false;
    if (addr)
        *addr = resp[0];
    if (ready_sec)
        *ready_sec = (uint16_t)((resp[1] - '0') * 100 + (resp[2] - '0') * 10 + (resp[3] - '0'));
    // The value count is the remaining digits (1 digit for aM!, 2 for aC!).
    uint16_t count = 0;
    for (size_t i = 4; i < len && det_np_digit(resp[i]); i++)
        count = (uint16_t)(count * 10 + (resp[i] - '0'));
    if (num_values)
        *num_values = (uint8_t)count;
    return true;
}

bool sdi12_parse_values(const char *resp, size_t len, float *out, size_t max, size_t *n)
{
    if (!resp || !out || !n)
        return false;
    size_t cnt = 0;
    size_t i = 1; // skip the leading address
    while (i < len && cnt < max)
    {
        char c = resp[i];
        if (c == '\r' || c == '\n')
            break;
        if (c == '+' || c == '-')
        {
            const char *start = resp + i;
            const char *end = start;
            // det_strtof handles a leading '-'; for '+' parse the magnitude after the sign.
            float v = (c == '+') ? det_strtof(start + 1, &end) : det_strtof(start, &end);
            if (end == start || (c == '+' && end == start + 1)) // no digits consumed
            {
                i++;
                continue;
            }
            out[cnt++] = v;
            i = (size_t)(end - resp);
        }
        else
        {
            i++; // CRC octets / separators are skipped (they never begin with +/-)
        }
    }
    *n = cnt;
    return true;
}

uint16_t sdi12_crc16(const uint8_t *data, size_t len)
{
    uint16_t crc = 0;
    for (size_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (int b = 0; b < 8; b++)
            crc = (crc & 1u) ? (uint16_t)((crc >> 1) ^ SDI12_CRC_POLY) : (uint16_t)(crc >> 1);
    }
    return crc;
}

void sdi12_crc_encode(uint16_t crc, char out[SDI12_CRC_CHARS])
{
    out[0] = (char)(0x40u | (crc >> 12)); // top bits
    out[1] = (char)(0x40u | ((crc >> 6) & 0x3Fu));
    out[2] = (char)(0x40u | (crc & 0x3Fu));
}

bool sdi12_check_crc(const char *resp, size_t len)
{
    if (!resp)
        return false;
    // Trim a trailing <CR><LF> if present.
    while (len > 0 && (resp[len - 1] == '\n' || resp[len - 1] == '\r'))
        len--;
    if (len < SDI12_CRC_CHARS + 1) // need at least 1 data octet + the 3 CRC octets
        return false;
    size_t data_len = len - SDI12_CRC_CHARS;
    char enc[SDI12_CRC_CHARS];
    sdi12_crc_encode(sdi12_crc16((const uint8_t *)resp, data_len), enc);
    return memcmp(enc, resp + data_len, SDI12_CRC_CHARS) == 0;
}

#endif // DETWS_ENABLE_SDI12
