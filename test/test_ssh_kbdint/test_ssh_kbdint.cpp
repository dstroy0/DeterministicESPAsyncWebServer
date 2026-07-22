// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// SSH keyboard-interactive authentication tests (RFC 4256): the server sends one INFO_REQUEST with a
// single non-echoed "Password:" prompt and verifies the INFO_RESPONSE through the password callback.

#include "network_drivers/presentation/ssh/auth/ssh_auth.h"
#include "network_drivers/presentation/ssh/connection/ssh_server.h" // the dispatcher's INFO_RESPONSE case
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

// A keyboard-interactive request needs a verifier to challenge against, and room for the prompt.
void test_kbdint_request_without_verifier_or_room()
{
    uint8_t pkt[128], out[128];
    size_t n = build_kbdint_request(pkt, "alice");
    size_t olen = 0;

    dws_ssh_auth_set_password_cb(nullptr); // nothing could ever verify an answer -> do not prompt
    TEST_ASSERT_EQUAL_INT(0, dws_ssh_auth_handle_request(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT8(SSH_MSG_USERAUTH_FAILURE, out[0]);

    dws_ssh_auth_set_password_cb(check_uv); // verifier present, but the prompt does not fit
    TEST_ASSERT_EQUAL_INT(-1, dws_ssh_auth_handle_request(0, pkt, n, out, &olen, 8));
}

// The INFO_RESPONSE reader rejects an out-of-range slot, an empty or mistyped payload, a payload
// with no room for the num-responses field, a truncated response string, and an exchange whose
// verifier was removed while it was in flight.
void test_kbdint_info_response_wire_guards()
{
    uint8_t pkt[128], req[128], out[128];
    size_t olen = 0;
    dws_ssh_auth_set_password_cb(check_uv);
    size_t rq = build_kbdint_request(req, "alice");

    size_t n = build_info_response(pkt, 1, "s3cret");
    TEST_ASSERT_EQUAL_INT(-1, dws_ssh_auth_handle_info_response(MAX_SSH_CONNS, pkt, n, out, &olen, sizeof(out)));

    TEST_ASSERT_EQUAL_INT(0, dws_ssh_auth_handle_request(0, req, rq, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL_INT(-1, dws_ssh_auth_handle_info_response(0, pkt, 0, out, &olen, sizeof(out))); // empty

    TEST_ASSERT_EQUAL_INT(0, dws_ssh_auth_handle_request(0, req, rq, out, &olen, sizeof(out)));
    n = build_info_response(pkt, 1, "s3cret");
    pkt[0] = SSH_MSG_USERAUTH_REQUEST; // some other message number
    TEST_ASSERT_EQUAL_INT(-1, dws_ssh_auth_handle_info_response(0, pkt, n, out, &olen, sizeof(out)));

    TEST_ASSERT_EQUAL_INT(0, dws_ssh_auth_handle_request(0, req, rq, out, &olen, sizeof(out)));
    uint8_t stub[3] = {SSH_MSG_USERAUTH_INFO_RESPONSE, 0, 0}; // no room for num-responses
    TEST_ASSERT_EQUAL_INT(-1, dws_ssh_auth_handle_info_response(0, stub, sizeof(stub), out, &olen, sizeof(out)));

    TEST_ASSERT_EQUAL_INT(0, dws_ssh_auth_handle_request(0, req, rq, out, &olen, sizeof(out)));
    uint8_t trunc[9] = {SSH_MSG_USERAUTH_INFO_RESPONSE, 0, 0, 0, 1, 0, 0, 0, 40}; // declares 40 bytes
    TEST_ASSERT_EQUAL_INT(0, dws_ssh_auth_handle_info_response(0, trunc, sizeof(trunc), out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT8(SSH_MSG_USERAUTH_FAILURE, out[0]);

    TEST_ASSERT_EQUAL_INT(0, dws_ssh_auth_handle_request(0, req, rq, out, &olen, sizeof(out)));
    dws_ssh_auth_set_password_cb(nullptr); // verifier removed mid-exchange
    n = build_info_response(pkt, 1, "s3cret");
    TEST_ASSERT_EQUAL_INT(0, dws_ssh_auth_handle_info_response(0, pkt, n, out, &olen, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT8(SSH_MSG_USERAUTH_FAILURE, out[0]);
    TEST_ASSERT_FALSE(ssh_sess[0].authed);
}

// ---- dispatcher integration ----------------------------------------------

static uint8_t emt_type[16];
static int emt_n;
static void rec_emit(uint8_t slot, const uint8_t *p, size_t n)
{
    (void)slot;
    if (emt_n < 16 && n > 0)
        emt_type[emt_n++] = p[0];
}

// Arm a keyboard-interactive exchange through the message dispatcher and answer it.
static int dispatch_kbdint_round(const char *answer)
{
    uint8_t pkt[128];
    size_t n = build_kbdint_request(pkt, "alice");
    int rc = dws_ssh_server_dispatch(0, SSH_MSG_USERAUTH_REQUEST, pkt, n);
    if (rc != 0)
        return rc;
    n = build_info_response(pkt, 1, answer);
    return dws_ssh_server_dispatch(0, SSH_MSG_USERAUTH_INFO_RESPONSE, pkt, n);
}

// The dispatcher only accepts an INFO_RESPONSE mid-userauth and only for an armed exchange; an
// armed round with the right answer emits the prompt then USERAUTH_SUCCESS and authenticates.
void test_kbdint_dispatch_guards_and_success()
{
    dws_ssh_auth_set_password_cb(check_uv);
    dws_ssh_server_set_emit_cb(rec_emit);
    uint8_t pkt[128];
    size_t n = build_info_response(pkt, 1, "s3cret");

    ssh_sess[0].phase = SshPhase::SSH_PHASE_SERVICE; // userauth has not started
    TEST_ASSERT_EQUAL_INT(-1, dws_ssh_server_dispatch(0, SSH_MSG_USERAUTH_INFO_RESPONSE, pkt, n));

    ssh_sess[0].phase = SshPhase::SSH_PHASE_AUTH; // right phase, nothing armed
    TEST_ASSERT_EQUAL_INT(-1, dws_ssh_server_dispatch(0, SSH_MSG_USERAUTH_INFO_RESPONSE, pkt, n));

    emt_n = 0;
    ssh_sess[0].phase = SshPhase::SSH_PHASE_AUTH;
    TEST_ASSERT_EQUAL_INT(0, dispatch_kbdint_round("s3cret"));
    TEST_ASSERT_EQUAL_INT(2, emt_n);
    TEST_ASSERT_EQUAL_UINT8(SSH_MSG_USERAUTH_INFO_REQUEST, emt_type[0]);
    TEST_ASSERT_EQUAL_UINT8(SSH_MSG_USERAUTH_SUCCESS, emt_type[1]);
    TEST_ASSERT_TRUE(ssh_sess[0].authed);
    TEST_ASSERT_EQUAL(SshPhase::SSH_PHASE_OPEN, ssh_sess[0].phase);
}

// A failed INFO_RESPONSE counts toward the per-connection brute-force limit exactly as a failed
// USERAUTH_REQUEST does: the last attempt emits SSH_MSG_DISCONNECT and closes (RFC 4252 §4).
void test_kbdint_dispatch_failures_hit_the_limit()
{
    dws_ssh_auth_set_password_cb(check_uv);
    dws_ssh_server_set_emit_cb(rec_emit);
    for (int k = 1; k < SSH_MAX_AUTH_ATTEMPTS; k++)
    {
        ssh_sess[0].phase = SshPhase::SSH_PHASE_AUTH;
        emt_n = 0;
        TEST_ASSERT_EQUAL_INT(0, dispatch_kbdint_round("wrong"));
        TEST_ASSERT_EQUAL_INT(2, emt_n);
        TEST_ASSERT_EQUAL_UINT8(SSH_MSG_USERAUTH_FAILURE, emt_type[1]);
        TEST_ASSERT_EQUAL_UINT8((uint8_t)k, ssh_sess[0].auth_failures);
    }
    ssh_sess[0].phase = SshPhase::SSH_PHASE_AUTH;
    emt_n = 0;
    TEST_ASSERT_EQUAL_INT(-1, dispatch_kbdint_round("wrong"));
    TEST_ASSERT_EQUAL_INT(3, emt_n);
    TEST_ASSERT_EQUAL_UINT8(SSH_MSG_DISCONNECT, emt_type[2]);
    TEST_ASSERT_FALSE(ssh_sess[0].authed);
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
    RUN_TEST(test_kbdint_request_without_verifier_or_room);
    RUN_TEST(test_kbdint_info_response_wire_guards);
    RUN_TEST(test_kbdint_dispatch_guards_and_success);
    RUN_TEST(test_kbdint_dispatch_failures_hit_the_limit);
    return UNITY_END();
}
