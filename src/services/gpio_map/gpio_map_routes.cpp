// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file gpio_map_routes.cpp
 * @brief GPIO pin-mapper routes (GET serves the JSON, POST drives an output).
 *
 * Separated from the host-testable core (gpio_map.cpp) so the serializer + control
 * parser unit-test without pulling in the server. The pin table is caller-owned.
 */

#include "services/gpio_map/gpio_map.h"

#if DETWS_ENABLE_GPIO_MAP

#include "dwserver.h"
#include "shared_primitives/mime.h"

// All gpio-map-routes state, owned by one instance (internal linkage): the server handle plus
// the pin table pointer and count, grouped so it is one named owner, unreachable cross-TU.
// (The route handlers are fixed-signature callbacks, so they reach this single owner directly.)
struct GpioRoutesCtx
{
    DetWebServer *srv = nullptr;
    DetwsGpioPin *pins = nullptr;
    uint8_t count = 0;
};
static GpioRoutesCtx s_gpior;

static void gpio_get_handler(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    detws_gpio_read(s_gpior.pins, s_gpior.count);
    char buf[DETWS_GPIO_JSON_BUF];
    detws_gpio_json(s_gpior.pins, s_gpior.count, buf, sizeof(buf));
    if (s_gpior.srv)
        s_gpior.srv->send(slot_id, 200, DET_MIME_JSON, buf);
}

static void gpio_post_handler(uint8_t slot_id, HttpReq *req)
{
    if (!s_gpior.srv)
        return;
    uint8_t pin;
    uint8_t level;
    if (!detws_gpio_parse_set((const char *)req->body, req->body_len, &pin, &level))
    {
        s_gpior.srv->send(slot_id, 400, DET_MIME_TEXT_PLAIN, "bad request");
        return;
    }
    if (!detws_gpio_is_output(s_gpior.pins, s_gpior.count, pin))
    {
        s_gpior.srv->send(slot_id, 403, DET_MIME_TEXT_PLAIN, "pin not a mapped output");
        return;
    }
    detws_gpio_write(pin, level);
    detws_gpio_read(s_gpior.pins, s_gpior.count);
    char buf[DETWS_GPIO_JSON_BUF];
    detws_gpio_json(s_gpior.pins, s_gpior.count, buf, sizeof(buf));
    s_gpior.srv->send(slot_id, 200, DET_MIME_JSON, buf);
}

void detws_gpio_map_begin(DetWebServer &server, const char *path, DetwsGpioPin *pins, uint8_t count)
{
    s_gpior.srv = &server;
    s_gpior.pins = pins;
    s_gpior.count = count;
    detws_gpio_begin_pins(pins, count);
    const char *p = (path && path[0]) ? path : "/gpio";
    server.on(p, HttpMethod::HTTP_GET, gpio_get_handler);
    server.on(p, HttpMethod::HTTP_POST, gpio_post_handler);
}

#endif // DETWS_ENABLE_GPIO_MAP
