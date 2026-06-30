// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file config_store.cpp
 * @brief Typed NVS configuration store - implementation.
 *
 * ESP32: the Arduino `Preferences` NVS wrapper. Host: a fixed in-memory table so
 * the typed contract is unit-testable. See config_store.h.
 */

#include "config_store.h"

#if DETWS_ENABLE_CONFIG_STORE

#ifdef ARDUINO

namespace
{
Preferences s_prefs;
bool s_open = false;
} // namespace

bool detws_config_begin(const char *ns)
{
    if (s_open)
        s_prefs.end();
    s_open = s_prefs.begin(ns, false); // read-write
    return s_open;
}

bool detws_config_set_str(const char *key, const char *val)
{
    if (!val)
        return false;
    return s_prefs.putString(key, val) == strlen(val);
}

size_t detws_config_get_str(const char *key, char *out, size_t out_cap, const char *def)
{
    if (!out || out_cap == 0)
        return 0;
    if (!s_prefs.isKey(key))
    {
        size_t n = def ? strlen(def) : 0;
        if (n > out_cap - 1)
            n = out_cap - 1;
        if (n)
            memcpy(out, def, n);
        out[n] = '\0';
        return n;
    }
    return s_prefs.getString(key, out, out_cap);
}

bool detws_config_set_u32(const char *key, uint32_t val)
{
    return s_prefs.putUInt(key, val) == sizeof(uint32_t);
}

uint32_t detws_config_get_u32(const char *key, uint32_t def)
{
    return s_prefs.getUInt(key, def);
}

bool detws_config_set_blob(const char *key, const void *data, size_t len)
{
    if (!data)
        return false;
    return s_prefs.putBytes(key, data, len) == len;
}

size_t detws_config_get_blob(const char *key, void *out, size_t out_cap)
{
    if (!out || !s_prefs.isKey(key))
        return 0;
    return s_prefs.getBytes(key, out, out_cap);
}

bool detws_config_erase(const char *key)
{
    return s_prefs.remove(key);
}

bool detws_config_clear(void)
{
    return s_prefs.clear();
}

#else // host: in-memory backend (test double for NVS)

namespace
{
struct Entry
{
    char key[DETWS_CONFIG_KEY_MAX];
    uint8_t val[DETWS_CONFIG_VAL_MAX];
    size_t len;
    bool used;
};
Entry s_tbl[DETWS_CONFIG_MAX_ENTRIES];

bool key_ok(const char *key)
{
    return key && key[0] && strlen(key) < DETWS_CONFIG_KEY_MAX;
}

Entry *find(const char *key)
{
    for (int i = 0; i < DETWS_CONFIG_MAX_ENTRIES; i++)
        if (s_tbl[i].used && strcmp(s_tbl[i].key, key) == 0)
            return &s_tbl[i];
    return nullptr;
}

Entry *find_or_alloc(const char *key)
{
    Entry *e = find(key);
    if (e)
        return e;
    for (int i = 0; i < DETWS_CONFIG_MAX_ENTRIES; i++)
        if (!s_tbl[i].used)
        {
            s_tbl[i].used = true;
            strncpy(s_tbl[i].key, key, DETWS_CONFIG_KEY_MAX - 1);
            s_tbl[i].key[DETWS_CONFIG_KEY_MAX - 1] = '\0';
            s_tbl[i].len = 0;
            return &s_tbl[i];
        }
    return nullptr; // table full
}

bool store(const char *key, const void *data, size_t len)
{
    if (!key_ok(key) || len > DETWS_CONFIG_VAL_MAX)
        return false;
    Entry *e = find_or_alloc(key);
    if (!e)
        return false;
    memcpy(e->val, data, len);
    e->len = len;
    return true;
}
} // namespace

bool detws_config_begin(const char *ns)
{
    (void)ns; // single in-memory namespace on host
    return true;
}

bool detws_config_set_str(const char *key, const char *val)
{
    if (!val)
        return false;
    return store(key, val, strlen(val) + 1); // include the null terminator
}

size_t detws_config_get_str(const char *key, char *out, size_t out_cap, const char *def)
{
    if (!out || out_cap == 0)
        return 0;
    Entry *e = key_ok(key) ? find(key) : nullptr;
    const char *src = e ? (const char *)e->val : (def ? def : "");
    size_t n = strlen(src);
    if (n > out_cap - 1)
        n = out_cap - 1;
    memcpy(out, src, n);
    out[n] = '\0';
    return n;
}

bool detws_config_set_u32(const char *key, uint32_t val)
{
    uint8_t b[4] = {(uint8_t)val, (uint8_t)(val >> 8), (uint8_t)(val >> 16), (uint8_t)(val >> 24)};
    return store(key, b, sizeof(b));
}

uint32_t detws_config_get_u32(const char *key, uint32_t def)
{
    Entry *e = key_ok(key) ? find(key) : nullptr;
    if (!e || e->len < 4)
        return def;
    return (uint32_t)e->val[0] | ((uint32_t)e->val[1] << 8) | ((uint32_t)e->val[2] << 16) | ((uint32_t)e->val[3] << 24);
}

bool detws_config_set_blob(const char *key, const void *data, size_t len)
{
    if (!data)
        return false;
    return store(key, data, len);
}

size_t detws_config_get_blob(const char *key, void *out, size_t out_cap)
{
    Entry *e = key_ok(key) ? find(key) : nullptr;
    if (!e || !out)
        return 0;
    size_t n = e->len < out_cap ? e->len : out_cap;
    memcpy(out, e->val, n);
    return n;
}

bool detws_config_erase(const char *key)
{
    Entry *e = key_ok(key) ? find(key) : nullptr;
    if (!e)
        return false;
    e->used = false;
    e->len = 0;
    return true;
}

bool detws_config_clear(void)
{
    for (int i = 0; i < DETWS_CONFIG_MAX_ENTRIES; i++)
        s_tbl[i] = Entry{};
    return true;
}

#endif // ARDUINO
#endif // DETWS_ENABLE_CONFIG_STORE
