// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file dashboard.cpp
 * @brief Dashboard widget table + JSON serializers (DETWS_ENABLE_DASHBOARD).
 *
 * The host-testable core: it owns the widget table and value array and turns them
 * into the layout / values JSON the page consumes. No server or web_assets
 * dependency lives here, so it compiles and unit-tests standalone; the route /
 * SSE wiring is in dashboard_routes.cpp.
 */

#include "services/dashboard/dashboard.h"

#if DETWS_ENABLE_DASHBOARD

#include "shared_primitives/numparse.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// All dashboard state, owned by one instance (internal linkage): the widget table, the
// per-widget value array, and the inbound-control callback, grouped so it is one named owner,
// unreachable from any other translation unit.
struct DashboardCtx
{
    const DetwsWidget *widgets = nullptr;
    uint8_t count = 0;
    float values[DETWS_DASHBOARD_MAX_WIDGETS] = {};
    DetwsControlCb control_cb = nullptr;
};
static DashboardCtx s_dash;

static const char *widget_type_name(DetwsWidgetType t)
{
    switch (t)
    {
    case DETWS_WIDGET_GAUGE:
        return "gauge";
    case DETWS_WIDGET_BAR:
        return "bar";
    case DETWS_WIDGET_SPARKLINE:
        return "sparkline";
    case DETWS_WIDGET_CHART:
        return "chart";
    case DETWS_WIDGET_BUTTON:
        return "button";
    case DETWS_WIDGET_TOGGLE:
        return "toggle";
    case DETWS_WIDGET_SLIDER:
        return "slider";
    default:
        return "value";
    }
}

// Append a formatted fragment at *pos; return -1 (leaving the buffer truncated) if
// it would not fit, so callers fail closed rather than overflow.
static int json_append(char *out, size_t cap, size_t *pos, const char *fmt, ...)
{
    if (*pos >= cap)
        return -1;
    va_list ap;
    va_start(ap, fmt);
    int w = vsnprintf(out + *pos, cap - *pos, fmt, ap);
    va_end(ap);
    if (w < 0 || (size_t)w >= cap - *pos)
        return -1;
    *pos += (size_t)w;
    return 0;
}

void detws_dashboard_configure(const DetwsWidget *widgets, uint8_t count)
{
    s_dash.widgets = widgets;
    s_dash.count = count > DETWS_DASHBOARD_MAX_WIDGETS ? DETWS_DASHBOARD_MAX_WIDGETS : count;
    for (uint8_t i = 0; i < DETWS_DASHBOARD_MAX_WIDGETS; i++)
        s_dash.values[i] = 0.0f;
}

bool detws_dashboard_set(const char *key, float value)
{
    if (!key || !s_dash.widgets)
        return false;
    for (uint8_t i = 0; i < s_dash.count; i++)
    {
        if (s_dash.widgets[i].key && strcmp(s_dash.widgets[i].key, key) == 0)
        {
            s_dash.values[i] = value;
            return true;
        }
    }
    return false;
}

int detws_dashboard_layout_json(char *out, size_t cap)
{
    if (!out || cap == 0)
        return 0;
    out[0] = '\0';
    if (!s_dash.widgets)
        return 0;
    size_t pos = 0;
    if (json_append(out, cap, &pos, "[") != 0)
        return 0;
    for (uint8_t i = 0; i < s_dash.count; i++)
    {
        const DetwsWidget *w = &s_dash.widgets[i];
        if (json_append(out, cap, &pos,
                        "%s{\"type\":\"%s\",\"label\":\"%s\",\"key\":\"%s\",\"min\":%g,\"max\":%g,\"unit\":\"%s\"}",
                        i ? "," : "", widget_type_name(w->type), w->label ? w->label : "", w->key ? w->key : "",
                        (double)w->min, (double)w->max, w->unit ? w->unit : "") != 0)
            return 0;
    }
    if (json_append(out, cap, &pos, "]") != 0)
        return 0;
    return (int)pos;
}

int detws_dashboard_values_json(char *out, size_t cap)
{
    if (!out || cap == 0)
        return 0;
    out[0] = '\0';
    if (!s_dash.widgets)
        return 0;
    size_t pos = 0;
    if (json_append(out, cap, &pos, "{") != 0)
        return 0;
    for (uint8_t i = 0; i < s_dash.count; i++)
    {
        if (json_append(out, cap, &pos, "%s\"%s\":%g", i ? "," : "", s_dash.widgets[i].key ? s_dash.widgets[i].key : "",
                        (double)s_dash.values[i]) != 0)
            return 0;
    }
    if (json_append(out, cap, &pos, "}") != 0)
        return 0;
    return (int)pos;
}

// ---------------------------------------------------------------------------
// Controls (inbound WebSocket messages)
// ---------------------------------------------------------------------------

void detws_dashboard_on_control(DetwsControlCb cb)
{
    s_dash.control_cb = cb;
}

// Locate the value of "key" in a {"k":...,"v":...} object: a pointer just past
// the ':' (whitespace skipped), or nullptr. The quoted pattern ("k" / "v") only
// matches the message's own keys, not a widget key that happens to contain k/v.
static const char *control_value_ptr(const char *s, const char *key)
{
    char pat[8];
    snprintf(pat, sizeof(pat), "\"%s\"", key);
    const char *p = strstr(s, pat);
    if (!p)
        return nullptr;
    p += strlen(pat);
    while (*p == ' ' || *p == '\t')
        p++;
    if (*p != ':')
        return nullptr;
    p++;
    while (*p == ' ' || *p == '\t')
        p++;
    return p;
}

bool detws_dashboard_parse_control(const char *msg, char *key_out, size_t key_cap, float *value_out)
{
    if (!msg || !key_out || key_cap == 0 || !value_out)
        return false;
    key_out[0] = '\0';
    const char *kp = control_value_ptr(msg, "k");
    const char *vp = control_value_ptr(msg, "v");
    if (!kp || !vp || *kp != '"')
        return false;
    kp++;
    size_t i = 0;
    while (*kp && *kp != '"' && i + 1 < key_cap)
        key_out[i++] = *kp++;
    if (*kp != '"')
    {
        key_out[0] = '\0';
        return false; // unterminated or key too long
    }
    key_out[i] = '\0';
    const char *end = nullptr;
    float v = det_strtof(vp, &end);
    if (end == vp)
        return false; // no numeric value
    *value_out = v;
    return true;
}

bool detws_dashboard_dispatch_control(const char *msg)
{
    char key[32];
    float value;
    if (!detws_dashboard_parse_control(msg, key, sizeof(key), &value))
        return false;
    if (s_dash.control_cb)
        s_dash.control_cb(key, value);
    return s_dash.control_cb != nullptr;
}

#endif // DETWS_ENABLE_DASHBOARD
