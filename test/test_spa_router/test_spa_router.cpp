// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/spa_router: the single-page-app routing decision.

#include "services/spa_router/spa_router.h"
#include <unity.h>

void setUp(void)
{
}
void tearDown(void)
{
}

void test_has_extension(void)
{
    TEST_ASSERT_TRUE(dws_spa_has_extension("/app.js"));
    TEST_ASSERT_TRUE(dws_spa_has_extension("/assets/style.css"));
    TEST_ASSERT_TRUE(dws_spa_has_extension("/x/y.min.js"));
    TEST_ASSERT_FALSE(dws_spa_has_extension("/dashboard"));
    TEST_ASSERT_FALSE(dws_spa_has_extension("/devices/42"));
    TEST_ASSERT_FALSE(dws_spa_has_extension("/")); // no segment
    // A dotfile directory in the path but an extensionless final segment is still a route.
    TEST_ASSERT_FALSE(dws_spa_has_extension("/a.b/c"));
    // Trailing dot is not an extension.
    TEST_ASSERT_FALSE(dws_spa_has_extension("/weird."));
}

void test_route(void)
{
    TEST_ASSERT_EQUAL_INT(DWSSpaAction::DWS_SPA_SERVE_SHELL, dws_spa_route("/", "/api/"));
    TEST_ASSERT_EQUAL_INT(DWSSpaAction::DWS_SPA_SERVE_SHELL, dws_spa_route("", "/api/"));
    TEST_ASSERT_EQUAL_INT(DWSSpaAction::DWS_SPA_SERVE_SHELL, dws_spa_route("/dashboard", "/api/"));
    TEST_ASSERT_EQUAL_INT(DWSSpaAction::DWS_SPA_SERVE_SHELL, dws_spa_route("/devices/42", "/api/"));
    TEST_ASSERT_EQUAL_INT(DWSSpaAction::DWS_SPA_SERVE_FILE, dws_spa_route("/app.js", "/api/"));
    TEST_ASSERT_EQUAL_INT(DWSSpaAction::DWS_SPA_SERVE_FILE, dws_spa_route("/assets/logo.svg", "/api/"));
    TEST_ASSERT_EQUAL_INT(DWSSpaAction::DWS_SPA_PASSTHROUGH, dws_spa_route("/api/state", "/api/"));
    TEST_ASSERT_EQUAL_INT(DWSSpaAction::DWS_SPA_PASSTHROUGH, dws_spa_route("/api/devices/42", "/api/"));
    // No API prefix configured: an /api path is just a route.
    TEST_ASSERT_EQUAL_INT(DWSSpaAction::DWS_SPA_SERVE_SHELL, dws_spa_route("/api/state", nullptr));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_has_extension);
    RUN_TEST(test_route);
    return UNITY_END();
}
