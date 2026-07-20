// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// SSH keyboard-interactive authentication tests (RFC 4256): the server sends one INFO_REQUEST with a
// single non-echoed "Password:" prompt and verifies the INFO_RESPONSE through the password callback.

#include "network_drivers/presentation/ssh/auth/ssh_auth.h"
#include "network_drivers/presentation/ssh/transport/ssh_packet.h"
#include "network_drivers/presentation/ssh/transport/ssh_transport.h"
#include <stdint.h>
#include <string.h>
#include <unity.h>

void setUp()
{
    ssh_transport_init(0);
    dws_ssh_auth_set_password_cb(nullptr);
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

static bool check_uv(const char *u, const char *p)
{
    return strcmp(u, "alice") == 0 && strcmp(p, "s3cret") == 0;
}

// USERAUTH_REQUEST(50) || user || "ssh-connection" || "keyboard-interactive" || lang || submethods.
static size_t build_kbdint_request(uint8_t *pkt, const char *user)
{
    size_t n = 0;
    pkt[n++] = SSH_MSG_USERAUTH_REQUEST;
    n += put_string(pkt + n, user);
    n += put_string(pkt + n, "ssh-connection");
    n += put_string(pkt + n, "keyboard-interactive");
    n += put_string(pkt + n, ""); // language tag (deprecated)
    n += put_string(pkt + n, ""); // submethods
    return n;
}

// INFO_RESPONSE(61) || uint32 num-responses || response strings.
static size_t build_info_response(uint8_t *pkt, uint32_t nr, const char *resp)
{
    size_t n = 0;
    pkt[n++] = SSH_MSG_USERAUTH_INFO_RESPONSE;
    pkt[n++] = (uint8_t)(nr >> 24);
    pkt[n++] = (uint8_t)(nr >> 16);
    pkt[n++] = (uint8_t)(nr >> 8);
    pkt[n++] = (uint8_t)nr;
    if (resp)
        n += put_string(pkt + n, resp);
    return n;
}

// A keyboard-interactive request produces exactly one non-echoed "Password: " prompt.
void test_kbdint_request_prompts()
{
    dws_ssh_auth_set_password_cb(check_uv);
    uint8_t pkt[128];
    size_t n = build_kbdint_request(pkt, "alice");
    uint8_t out[128];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, dws_ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT8(SSH_MSG_USERAUTH_INFO_REQUEST, out[0]); // 60
    // name(4)=0 || instruction(4)=0 || lang(4)=0 || num-prompts(4)=1 || string("Password: ") || echo=0
    size_t o = 1;
    for (int f = 0; f < 3; f++) // name, instruction, language are all empty
    {
        TEST_ASSERT_EQUAL_UINT32(0, ((uint32_t)out[o] << 24) | ((uint32_t)out[o + 1] << 16) |
                                        ((uint32_t)out[o + 2] << 8) | out[o + 3]);
        o += 4;
    }
    uint32_t np = ((uint32_t)out[o] << 24) | ((uint32_t)out[o + 1] << 16) | ((uint32_t)out[o + 2] << 8) | out[o + 3];
    o += 4;
    TEST_ASSERT_EQUAL_UINT32(1, np); // one prompt
    uint32_t pl = ((uint32_t)out[o] << 24) | ((uint32_t)out[o + 1] << 16) | ((uint32_t)out[o + 2] << 8) | out[o + 3];
    o += 4;
    TEST_ASSERT_EQUAL_UINT32(10, pl); // strlen("Password: ")
    TEST_ASSERT_EQUAL_MEMORY("Password: ", out + o, 10);
    o += pl;
    TEST_ASSERT_EQUAL_UINT8(0, out[o]); // echo = FALSE
}

// The full exchange with the right password authenticates the session.
void test_kbdint_correct_password_succeeds()
{
    dws_ssh_auth_set_password_cb(check_uv);
    uint8_t pkt[128], out[128];
    size_t n, olen = 0;
    n = build_kbdint_request(pkt, "alice");
    TEST_ASSERT_EQUAL_INT(0, dws_ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT8(SSH_MSG_USERAUTH_INFO_REQUEST, out[0]);

    n = build_info_response(pkt, 1, "s3cret");
    TEST_ASSERT_EQUAL_INT(0, dws_ssh_auth_handle_info_response(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT8(SSH_MSG_USERAUTH_SUCCESS, out[0]); // 52
    TEST_ASSERT_TRUE(ssh_sess[0].authed);
}

// A wrong response is rejected and does not authenticate.
void test_kbdint_wrong_password_fails()
{
    dws_ssh_auth_set_password_cb(check_uv);
    uint8_t pkt[128], out[128];
    size_t n, olen = 0;
    n = build_kbdint_request(pkt, "alice");
    dws_ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out));
    n = build_info_response(pkt, 1, "wrong");
    TEST_ASSERT_EQUAL_INT(0, dws_ssh_auth_handle_info_response(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT8(SSH_MSG_USERAUTH_FAILURE, out[0]); // 51
    TEST_ASSERT_FALSE(ssh_sess[0].authed);
}

// An INFO_RESPONSE with no exchange armed is rejected (cannot skip the request).
void test_kbdint_response_without_request_fails()
{
    dws_ssh_auth_set_password_cb(check_uv);
    uint8_t pkt[128], out[128];
    size_t olen = 0;
    size_t n = build_info_response(pkt, 1, "s3cret");
    TEST_ASSERT_EQUAL_INT(-1, dws_ssh_auth_handle_info_response(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_FALSE(ssh_sess[0].authed);
}

// num-responses must match the single prompt; 0 responses fails cleanly.
void test_kbdint_zero_responses_fails()
{
    dws_ssh_auth_set_password_cb(check_uv);
    uint8_t pkt[128], out[128];
    size_t n, olen = 0;
    n = build_kbdint_request(pkt, "alice");
    dws_ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out));
    n = build_info_response(pkt, 0, nullptr);
    TEST_ASSERT_EQUAL_INT(0, dws_ssh_auth_handle_info_response(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT8(SSH_MSG_USERAUTH_FAILURE, out[0]);
    TEST_ASSERT_FALSE(ssh_sess[0].authed);
}

// Replay: a second INFO_RESPONSE after the exchange was consumed is rejected.
void test_kbdint_response_replay_fails()
{
    dws_ssh_auth_set_password_cb(check_uv);
    uint8_t pkt[128], out[128];
    size_t n, olen = 0;
    n = build_kbdint_request(pkt, "alice");
    dws_ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out));
    n = build_info_response(pkt, 1, "s3cret");
    dws_ssh_auth_handle_info_response(0, pkt, n, out, &olen, sizeof(out)); // consumes the exchange
    TEST_ASSERT_EQUAL_INT(-1, dws_ssh_auth_handle_info_response(0, pkt, n, out, &olen, sizeof(out)));
}

// The USERAUTH_FAILURE method list advertises keyboard-interactive.
void test_methods_list_advertises_kbdint()
{
    uint8_t out[128];
    size_t olen = 0;
    TEST_ASSERT_EQUAL_INT(0, dws_ssh_auth_build_failure(out, &olen, sizeof(out), false));
    uint32_t ml = ((uint32_t)out[1] << 24) | ((uint32_t)out[2] << 16) | ((uint32_t)out[3] << 8) | out[4];
    char list[128];
    memcpy(list, out + 5, ml);
    list[ml] = '\0';
    TEST_ASSERT_NOT_NULL(strstr(list, "keyboard-interactive"));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_kbdint_request_prompts);
    RUN_TEST(test_kbdint_correct_password_succeeds);
    RUN_TEST(test_kbdint_wrong_password_fails);
    RUN_TEST(test_kbdint_response_without_request_fails);
    RUN_TEST(test_kbdint_zero_responses_fails);
    RUN_TEST(test_kbdint_response_replay_fails);
    RUN_TEST(test_methods_list_advertises_kbdint);
    return UNITY_END();
}
