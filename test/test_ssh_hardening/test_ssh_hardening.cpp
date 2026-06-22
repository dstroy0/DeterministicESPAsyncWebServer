// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Built with DETWS_SSH_ALLOW_PASSWORD=0: verifies password authentication is
// refused and not advertised, while publickey auth remains available.

#include "DetWebServerConfig.h"
#include "network_drivers/presentation/ssh/ssh_auth.h"
#include "network_drivers/presentation/ssh/ssh_packet.h"
#include "network_drivers/presentation/ssh/ssh_transport.h"
#include <stdint.h>
#include <string.h>
#include <unity.h>

#if DETWS_SSH_ALLOW_PASSWORD != 0
#error "test_ssh_hardening must be built with DETWS_SSH_ALLOW_PASSWORD=0"
#endif

void setUp()
{
    ssh_transport_init(0);
    ssh_auth_set_password_cb(nullptr);
}
void tearDown()
{
}

static size_t put_string(uint8_t *p, const char *s)
{
    uint32_t n = (uint32_t)strlen(s);
    p[0] = (uint8_t)(n >> 24);
    p[1] = (uint8_t)(n >> 16);
    p[2] = (uint8_t)(n >> 8);
    p[3] = (uint8_t)n;
    memcpy(p + 4, s, n);
    return 4 + n;
}

static bool always_ok(const char *u, const char *p)
{
    (void)u;
    (void)p;
    return true;
}

void test_password_refused_even_with_correct_callback()
{
    // Even a callback that accepts everything must not authenticate, because
    // the password method is compiled out.
    ssh_auth_set_password_cb(always_ok);
    uint8_t pkt[128];
    size_t n = 0;
    pkt[n++] = SSH_MSG_USERAUTH_REQUEST;
    n += put_string(pkt + n, "alice");
    n += put_string(pkt + n, "ssh-connection");
    n += put_string(pkt + n, "password");
    pkt[n++] = 0;
    n += put_string(pkt + n, "whatever");

    uint8_t out[64];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL(SSH_MSG_USERAUTH_FAILURE, out[0]);
    TEST_ASSERT_FALSE(ssh_sess[0].authed);
}

void test_failure_advertises_publickey_only()
{
    uint8_t out[64];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, ssh_auth_build_failure(out, &olen, sizeof(out), false));
    // name-list at out[1..]: must contain "publickey" and not "password".
    bool has_pk = false, has_pw = false;
    for (size_t k = 0; k + 9 <= olen; k++)
        if (memcmp(out + k, "publickey", 9) == 0)
            has_pk = true;
    for (size_t k = 0; k + 8 <= olen; k++)
        if (memcmp(out + k, "password", 8) == 0)
            has_pw = true;
    TEST_ASSERT_TRUE(has_pk);
    TEST_ASSERT_FALSE(has_pw);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_password_refused_even_with_correct_callback);
    RUN_TEST(test_failure_advertises_publickey_only);
    return UNITY_END();
}
