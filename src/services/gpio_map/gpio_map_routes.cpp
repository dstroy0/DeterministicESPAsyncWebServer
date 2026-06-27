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

#include "DeterministicESPAsyncWebServer.h"

static DetWebServer *s_srv = nullptr;
static DetwsGpioPin *s_pins = nullptr;
static uint8_t s_count = 0;

static void gpio_get_handler(uint8_t slot_id, HttpReq *req)
{
    (void)req;
    detws_gpio_read(s_pins, s_count);
    char buf[DETWS_GPIO_JSON_BUF];
    detws_gpio_json(s_pins, s_count, buf, sizeof(buf));
    if (s_srv)
        s_srv->send(slot_id, 200, "application/json", buf);
}

static void gpio_post_handler(uint8_t slot_id, HttpReq *req)
{
    if (!s_srv)
        return;
    uint8_t pin, level;
    if (!detws_gpio_parse_set((const char *)req->body, req->body_len, &pin, &level))
    {
        s_srv->send(slot_id, 400, "text/plain", "bad request");
        return;
    }
    if (!detws_gpio_is_output(s_pins, s_count, pin))
    {
        s_srv->send(slot_id, 403, "text/plain", "pin not a mapped output");
        return;
    }
    detws_gpio_write(pin, level);
    detws_gpio_read(s_pins, s_count);
    char buf[DETWS_GPIO_JSON_BUF];
    detws_gpio_json(s_pins, s_count, buf, sizeof(buf));
    s_srv->send(slot_id, 200, "application/json", buf);
}

void detws_gpio_map_begin(DetWebServer &server, const char *path, DetwsGpioPin *pins, uint8_t count)
{
    s_srv = &server;
    s_pins = pins;
    s_count = count;
    detws_gpio_begin_pins(pins, count);
    const char *p = (path && path[0]) ? path : "/gpio";
    server.on(p, HTTP_GET, gpio_get_handler);
    server.on(p, HTTP_POST, gpio_post_handler);
}

#endif // DETWS_ENABLE_GPIO_MAP
