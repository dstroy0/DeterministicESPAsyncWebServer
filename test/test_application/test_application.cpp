// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit, stress, and race-condition tests for Layer 7 (Application).
//
// Sections:
//   FUNCTION I/O  - one test per DetWebServer method behavior
//   UNIT          - routing, wildcard, not-found, CORS dispatch
//   STRESS        - route-table full scan, sequential requests, all-slots
//   RACE SIM      - slot state hazards visible to handle()

#include "DeterministicESPAsyncWebServer.h"
#include <unity.h>

// All source layers compiled via native_app env - no stubs needed.

// ---- Shared helpers ------------------------------------------------

static bool handler_called;
static uint8_t handler_slot;

static void record_handler(uint8_t slot_id, HttpReq *)
{
    handler_called = true;
    handler_slot = slot_id;
}

// Push a raw HTTP request into a slot's ring buffer, reset the parser,
// and run http_parse so the slot is ready for handle().
static void arm_slot(uint8_t slot, const char *raw)
{
    conn_pool[slot] = {};
    conn_pool[slot].id = slot;
    conn_pool[slot].state = CONN_ACTIVE;
    conn_pool[slot].pcb = nullptr;

    TcpConn *s = &conn_pool[slot];
    for (size_t i = 0; raw[i]; i++)
    {
        size_t next = (s->rx_head + 1) % RX_BUF_SIZE;
        s->rx_buffer[s->rx_head] = (uint8_t)raw[i];
        s->rx_head = next;
    }
    http_reset(slot);
    http_parse(slot);
}

// Push raw bytes into a ring buffer without resetting or parsing.
static void push_bytes(uint8_t slot, const char *data)
{
    TcpConn *s = &conn_pool[slot];
    for (size_t i = 0; data[i]; i++)
    {
        size_t next = (s->rx_head + 1) % RX_BUF_SIZE;
        s->rx_buffer[s->rx_head] = (uint8_t)data[i];
        s->rx_head = next;
    }
}

static DetWebServer *g_server;

void setUp()
{
    set_millis(0);
    DeterministicAsyncTCP::init(80);
    for (int i = 0; i < MAX_CONNS; i++)
    {
        conn_pool[i] = {};
        conn_pool[i].id = i;
        conn_pool[i].state = CONN_ACTIVE;
        http_reset(i);
    }
    handler_called = false;
    handler_slot = 255;
    g_server = new DetWebServer();
}

void tearDown()
{
    delete g_server;
    g_server = nullptr;
}

// ====================================================================
// FUNCTION I/O TESTS - DetWebServer::on()
// ====================================================================

void test_fn_on_registers_and_dispatches()
{
    g_server->on("/ping", HTTP_GET, record_handler);
    arm_slot(0, "GET /ping HTTP/1.1\r\n\r\n");
    g_server->handle();
    TEST_ASSERT_TRUE(handler_called);
}

void test_fn_on_path_copied_null_terminated()
{
    // A path of exactly MAX_PATH_LEN-1 chars must not overflow the route buffer.
    char path[MAX_PATH_LEN + 4];
    path[0] = '/';
    for (int i = 1; i < MAX_PATH_LEN - 1; i++)
        path[i] = 'a';
    path[MAX_PATH_LEN - 1] = '\0';
    g_server->on(path, HTTP_GET, record_handler);
    arm_slot(0, "GET /a HTTP/1.1\r\n\r\n"); // won't match long path - just must not crash
    g_server->handle();
    TEST_PASS();
}

void test_fn_on_table_full_extra_routes_dropped()
{
    // Fill the table; on() beyond MAX_ROUTES must silently drop
    for (int i = 0; i < MAX_ROUTES + 5; i++)
        g_server->on("/x", HTTP_GET, record_handler);
    arm_slot(0, "GET /x HTTP/1.1\r\n\r\n");
    g_server->handle();
    TEST_ASSERT_TRUE(handler_called);
}

void test_fn_on_same_path_different_methods_are_distinct()
{
    static bool get_called = false;
    static bool post_called = false;
    g_server->on("/r", HTTP_GET, [](uint8_t, HttpReq *) { get_called = true; });
    g_server->on("/r", HTTP_POST, [](uint8_t, HttpReq *) { post_called = true; });

    arm_slot(0, "GET /r HTTP/1.1\r\n\r\n");
    g_server->handle();
    TEST_ASSERT_TRUE(get_called);
    TEST_ASSERT_FALSE(post_called);

    arm_slot(0, "POST /r HTTP/1.1\r\nContent-Length: 0\r\n\r\n");
    g_server->handle();
    TEST_ASSERT_TRUE(post_called);
}

// ====================================================================
// FUNCTION I/O TESTS - DetWebServer::on_not_found()
// ====================================================================

void test_fn_on_not_found_called_when_no_match()
{
    g_server->on_not_found(record_handler);
    arm_slot(0, "GET /nowhere HTTP/1.1\r\n\r\n");
    g_server->handle();
    TEST_ASSERT_TRUE(handler_called);
}

void test_fn_on_not_found_not_called_when_match_exists()
{
    static bool nf = false;
    g_server->on("/here", HTTP_GET, record_handler);
    g_server->on_not_found([](uint8_t, HttpReq *) { nf = true; });
    arm_slot(0, "GET /here HTTP/1.1\r\n\r\n");
    g_server->handle();
    TEST_ASSERT_TRUE(handler_called);
    TEST_ASSERT_FALSE(nf);
}

// ====================================================================
// FUNCTION I/O TESTS - DetWebServer::set_cors()
// ====================================================================

void test_fn_set_cors_options_preflight_clears_slot()
{
    g_server->set_cors("*");
    arm_slot(0, "OPTIONS /x HTTP/1.1\r\n\r\n");
    g_server->handle();
    TEST_ASSERT_NOT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
}

void test_fn_set_cors_empty_string_disables()
{
    g_server->set_cors("*");
    g_server->set_cors(""); // disable
    g_server->on("/x", HTTP_OPTIONS, record_handler);
    arm_slot(0, "OPTIONS /x HTTP/1.1\r\n\r\n");
    g_server->handle();
    TEST_ASSERT_TRUE(handler_called); // routed normally, not intercepted as preflight
}

// ====================================================================
// UNIT TESTS - routing
// ====================================================================

void test_wrong_method_does_not_match()
{
    g_server->on("/r", HTTP_POST, record_handler);
    arm_slot(0, "GET /r HTTP/1.1\r\n\r\n");
    g_server->handle();
    TEST_ASSERT_FALSE(handler_called);
}

void test_wrong_path_does_not_match()
{
    g_server->on("/right", HTTP_GET, record_handler);
    arm_slot(0, "GET /wrong HTTP/1.1\r\n\r\n");
    g_server->handle();
    TEST_ASSERT_FALSE(handler_called);
}

void test_all_http_methods_dispatched()
{
    static int counts[7] = {};
    g_server->on("/get", HTTP_GET, [](uint8_t, HttpReq *) { counts[0]++; });
    g_server->on("/post", HTTP_POST, [](uint8_t, HttpReq *) { counts[1]++; });
    g_server->on("/put", HTTP_PUT, [](uint8_t, HttpReq *) { counts[2]++; });
    g_server->on("/delete", HTTP_DELETE, [](uint8_t, HttpReq *) { counts[3]++; });
    g_server->on("/patch", HTTP_PATCH, [](uint8_t, HttpReq *) { counts[4]++; });
    g_server->on("/head", HTTP_HEAD, [](uint8_t, HttpReq *) { counts[5]++; });
    g_server->on("/options", HTTP_OPTIONS, [](uint8_t, HttpReq *) { counts[6]++; });

    arm_slot(0, "GET /get HTTP/1.1\r\n\r\n");
    g_server->handle();
    arm_slot(0, "POST /post HTTP/1.1\r\nContent-Length: 0\r\n\r\n");
    g_server->handle();
    arm_slot(0, "PUT /put HTTP/1.1\r\nContent-Length: 0\r\n\r\n");
    g_server->handle();
    arm_slot(0, "DELETE /delete HTTP/1.1\r\n\r\n");
    g_server->handle();
    arm_slot(0, "PATCH /patch HTTP/1.1\r\nContent-Length: 0\r\n\r\n");
    g_server->handle();
    arm_slot(0, "HEAD /head HTTP/1.1\r\n\r\n");
    g_server->handle();
    arm_slot(0, "OPTIONS /options HTTP/1.1\r\n\r\n");
    g_server->handle();

    for (int i = 0; i < 7; i++)
        TEST_ASSERT_EQUAL_MESSAGE(1, counts[i], "method not dispatched");
}

void test_root_path_matches_exactly()
{
    g_server->on("/", HTTP_GET, record_handler);
    arm_slot(0, "GET / HTTP/1.1\r\n\r\n");
    g_server->handle();
    TEST_ASSERT_TRUE(handler_called);
}

void test_root_path_does_not_match_subpath()
{
    g_server->on("/", HTTP_GET, record_handler);
    arm_slot(0, "GET /other HTTP/1.1\r\n\r\n");
    g_server->handle();
    TEST_ASSERT_FALSE(handler_called);
}

void test_wildcard_matches_any_suffix()
{
    g_server->on("/api/*", HTTP_GET, record_handler);
    arm_slot(0, "GET /api/users/42 HTTP/1.1\r\n\r\n");
    g_server->handle();
    TEST_ASSERT_TRUE(handler_called);
}

void test_wildcard_does_not_match_unrelated_prefix()
{
    g_server->on("/api/*", HTTP_GET, record_handler);
    arm_slot(0, "GET /other/path HTTP/1.1\r\n\r\n");
    g_server->handle();
    TEST_ASSERT_FALSE(handler_called);
}

void test_exact_route_wins_when_registered_first()
{
    static bool exact_called = false;
    g_server->on("/api/status", HTTP_GET, [](uint8_t, HttpReq *) { exact_called = true; });
    g_server->on("/api/*", HTTP_GET, record_handler);
    arm_slot(0, "GET /api/status HTTP/1.1\r\n\r\n");
    g_server->handle();
    TEST_ASSERT_TRUE(exact_called);
    TEST_ASSERT_FALSE(handler_called);
}

void test_slot_not_stuck_in_complete_after_handle()
{
    g_server->on("/free", HTTP_GET, record_handler);
    arm_slot(0, "GET /free HTTP/1.1\r\n\r\n");
    g_server->handle();
    TEST_ASSERT_NOT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
}

void test_parse_error_slot_auto_reset()
{
    push_bytes(0, "TOOLONGMETHODNAME /path HTTP/1.1\r\n\r\n");
    http_reset(0);
    http_parse(0);
    TEST_ASSERT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);
    g_server->handle();
    TEST_ASSERT_NOT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);
}

// Handler reads req->body from a POST request
void test_handler_reads_body()
{
    static char body_seen[32] = {};
    g_server->on("/body", HTTP_POST,
                 [](uint8_t, HttpReq *req) { strncpy(body_seen, (const char *)req->body, sizeof(body_seen) - 1); });
    arm_slot(0, "POST /body HTTP/1.1\r\nContent-Length: 5\r\n\r\nhello");
    g_server->handle();
    TEST_ASSERT_EQUAL_STRING("hello", body_seen);
}

// Handler calls http_get_query() to read a URL query parameter.
// Value is copied inside the handler because http_reset() clears
// http_pool[] before handle() returns to the test.
void test_handler_reads_query_param()
{
    static char q_seen[48] = {};
    g_server->on("/q", HTTP_GET, [](uint8_t, HttpReq *req) {
        const char *v = http_get_query(req, "id");
        if (v)
            strncpy(q_seen, v, sizeof(q_seen) - 1);
    });
    arm_slot(0, "GET /q?id=42 HTTP/1.1\r\n\r\n");
    g_server->handle();
    TEST_ASSERT_EQUAL_STRING("42", q_seen);
}

// Handler calls http_get_header() to read a custom request header.
// Same copy-inside-handler pattern as test_handler_reads_query_param.
void test_handler_reads_header()
{
    static char h_seen[48] = {};
    g_server->on("/h", HTTP_GET, [](uint8_t, HttpReq *req) {
        const char *v = http_get_header(req, "X-Token");
        if (v)
            strncpy(h_seen, v, sizeof(h_seen) - 1);
    });
    arm_slot(0, "GET /h HTTP/1.1\r\nX-Token: secret\r\n\r\n");
    g_server->handle();
    TEST_ASSERT_EQUAL_STRING("secret", h_seen);
}

// Wildcard registered BEFORE exact: first-match means wildcard wins.
// (Complement of test_exact_route_wins_when_registered_first.)
void test_wildcard_before_exact_wildcard_wins()
{
    static bool wildcard_called = false;
    static bool exact_called = false;
    g_server->on("/api/*", HTTP_GET, [](uint8_t, HttpReq *) { wildcard_called = true; });
    g_server->on("/api/status", HTTP_GET, [](uint8_t, HttpReq *) { exact_called = true; });
    arm_slot(0, "GET /api/status HTTP/1.1\r\n\r\n");
    g_server->handle();
    TEST_ASSERT_TRUE(wildcard_called);
    TEST_ASSERT_FALSE(exact_called);
}

// ====================================================================
// STRESS TESTS
// ====================================================================

// Route table full (MAX_ROUTES entries); request matches the LAST route -
// worst-case O(N) linear scan must not corrupt any route or crash.
void stress_last_route_dispatched_in_full_table()
{
    static int last_count = 0;
    for (int i = 0; i < MAX_ROUTES - 1; i++)
    {
        char path[16];
        snprintf(path, sizeof(path), "/r%d", i);
        g_server->on(path, HTTP_GET, [](uint8_t, HttpReq *) {});
    }
    g_server->on("/last", HTTP_GET, [](uint8_t, HttpReq *) { last_count++; });

    arm_slot(0, "GET /last HTTP/1.1\r\n\r\n");
    g_server->handle();
    TEST_ASSERT_EQUAL(1, last_count);
}

// 50 sequential requests on slot 0; handler records each dispatch.
// Verifies zero state leakage between requests.
void stress_sequential_requests_no_state_leak()
{
    static int seq_count = 0;
    g_server->on("/seq", HTTP_GET, [](uint8_t, HttpReq *) { seq_count++; });

    for (int i = 0; i < 50; i++)
    {
        arm_slot(0, "GET /seq HTTP/1.1\r\n\r\n");
        g_server->handle();
    }
    TEST_ASSERT_EQUAL(50, seq_count);
}

// All four slots serve different routes simultaneously in a single handle() call.
void stress_all_slots_dispatched_simultaneously()
{
    static int counts[4] = {};
    g_server->on("/s0", HTTP_GET, [](uint8_t, HttpReq *) { counts[0]++; });
    g_server->on("/s1", HTTP_GET, [](uint8_t, HttpReq *) { counts[1]++; });
    g_server->on("/s2", HTTP_GET, [](uint8_t, HttpReq *) { counts[2]++; });
    g_server->on("/s3", HTTP_GET, [](uint8_t, HttpReq *) { counts[3]++; });

    arm_slot(0, "GET /s0 HTTP/1.1\r\n\r\n");
    arm_slot(1, "GET /s1 HTTP/1.1\r\n\r\n");
    arm_slot(2, "GET /s2 HTTP/1.1\r\n\r\n");
    arm_slot(3, "GET /s3 HTTP/1.1\r\n\r\n");

    g_server->handle();

    for (int i = 0; i < 4; i++)
        TEST_ASSERT_EQUAL_MESSAGE(1, counts[i], "slot not dispatched");
}

// Single wildcard route matches 10 different path suffixes; each dispatches exactly once.
void stress_wildcard_matches_many_paths()
{
    static int wc_count = 0;
    g_server->on("/api/*", HTTP_GET, [](uint8_t, HttpReq *) { wc_count++; });

    const char *paths[] = {
        "GET /api/users HTTP/1.1\r\n\r\n",
        "GET /api/devices HTTP/1.1\r\n\r\n",
        "GET /api/status/health HTTP/1.1\r\n\r\n",
        "GET /api/ HTTP/1.1\r\n\r\n",
        "GET /api/a HTTP/1.1\r\n\r\n",
        "GET /api/b/c/d HTTP/1.1\r\n\r\n",
        "GET /api/1 HTTP/1.1\r\n\r\n",
        "GET /api/2 HTTP/1.1\r\n\r\n",
        "GET /api/3 HTTP/1.1\r\n\r\n",
        "GET /api/long/nested/path HTTP/1.1\r\n\r\n",
    };
    for (int i = 0; i < 10; i++)
    {
        arm_slot(0, paths[i]);
        g_server->handle();
    }
    TEST_ASSERT_EQUAL(10, wc_count);
}

// 20 sequential handle() calls with NO complete parse slots - must be idle no-ops.
void stress_handle_with_no_complete_slots_is_nop()
{
    g_server->on("/x", HTTP_GET, record_handler);
    // All slots in PARSE_METHOD (setUp resets them) - nothing to dispatch
    for (int i = 0; i < 20; i++)
        g_server->handle();
    TEST_ASSERT_FALSE(handler_called);
}

// ====================================================================
// RACE CONDITION SIMULATIONS
// ====================================================================

// Slot transitions to PARSE_COMPLETE between tick and handle() slot scan -
// already covered by the normal flow; here we verify handle() dispatches
// a slot that became complete since the last call.
void race_slot_complete_between_handle_calls()
{
    static bool dispatched = false;
    g_server->on("/late", HTTP_GET, [](uint8_t, HttpReq *) { dispatched = true; });

    g_server->handle(); // no complete slots yet
    TEST_ASSERT_FALSE(dispatched);

    arm_slot(0, "GET /late HTTP/1.1\r\n\r\n"); // becomes complete NOW
    g_server->handle();
    TEST_ASSERT_TRUE(dispatched);
}

// A slot is in PARSE_COMPLETE but its conn state is CONN_FREE (connection
// already dropped by a timeout between parse completion and handle()).
// send() must detect pcb==nullptr/CONN_FREE and call http_reset() cleanly.
void race_conn_freed_after_parse_complete()
{
    g_server->on("/r", HTTP_GET, record_handler);

    arm_slot(0, "GET /r HTTP/1.1\r\n\r\n");
    TEST_ASSERT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);

    // Simulate connection drop between parse and dispatch
    conn_pool[0].state = CONN_FREE;
    conn_pool[0].pcb = nullptr;

    g_server->handle(); // must not crash; slot must be cleaned up
    TEST_ASSERT_NOT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
}

// handle() is called twice without any new input - the second call must
// see no PARSE_COMPLETE slots and dispatch nothing.
void race_double_handle_no_double_dispatch()
{
    static int dispatch_count = 0;
    g_server->on("/dd", HTTP_GET, [](uint8_t, HttpReq *) { dispatch_count++; });

    arm_slot(0, "GET /dd HTTP/1.1\r\n\r\n");
    g_server->handle(); // dispatches once, resets slot
    g_server->handle(); // slot is PARSE_METHOD - must dispatch 0 times

    TEST_ASSERT_EQUAL(1, dispatch_count);
}

// A PARSE_ERROR slot is followed immediately by a valid slot; handle() must
// process the error slot (send 400) and also dispatch the valid slot.
void race_error_and_valid_slot_in_same_handle()
{
    static bool valid_dispatched = false;
    g_server->on("/ok", HTTP_GET, [](uint8_t, HttpReq *) { valid_dispatched = true; });

    // Slot 0: inject a parse error
    push_bytes(0, "TOOLONGMETHODNAME /path HTTP/1.1\r\n\r\n");
    http_reset(0);
    http_parse(0);
    TEST_ASSERT_EQUAL(PARSE_ERROR, http_pool[0].parse_state);

    // Slot 1: valid request
    arm_slot(1, "GET /ok HTTP/1.1\r\n\r\n");

    g_server->handle();

    TEST_ASSERT_NOT_EQUAL(PARSE_ERROR, http_pool[0].parse_state); // 400 sent, reset
    TEST_ASSERT_TRUE(valid_dispatched);                           // slot 1 dispatched
}

// A callback that calls http_reset() directly (instead of via send()) must
// not confuse handle()'s post-dispatch guard.
void race_callback_manually_resets_slot()
{
    static bool manual_reset_called = false;
    g_server->on("/mr", HTTP_GET, [](uint8_t slot_id, HttpReq *) {
        manual_reset_called = true;
        http_reset(slot_id); // reset without sending a response
    });

    arm_slot(0, "GET /mr HTTP/1.1\r\n\r\n");
    g_server->handle(); // must not double-reset or crash

    TEST_ASSERT_TRUE(manual_reset_called);
    TEST_ASSERT_EQUAL(PARSE_METHOD, http_pool[0].parse_state);
}

// ====================================================================
// 414 URI TOO LONG
// ====================================================================

void test_uri_too_long_auto_resets_slot()
{
    // Overflow the path buffer - handle() should send 414 and free the slot
    char req[MAX_PATH_LEN + 64];
    int idx = 0;
    memcpy(req + idx, "GET /", 5);
    idx += 5;
    for (int i = 0; i < MAX_PATH_LEN; i++)
        req[idx++] = 'a';
    memcpy(req + idx, " HTTP/1.1\r\n\r\n", 13);
    idx += 13;
    req[idx] = '\0';

    push_bytes(0, req);
    http_reset(0);
    http_parse(0);
    TEST_ASSERT_EQUAL(PARSE_URI_TOO_LONG, http_pool[0].parse_state);

    g_server->handle(); // must send 414 and reset the slot
    TEST_ASSERT_NOT_EQUAL(PARSE_URI_TOO_LONG, http_pool[0].parse_state);
}

// ====================================================================
// TRANSFER-ENCODING REJECTION
// ====================================================================

void test_transfer_encoding_chunked_is_501()
{
    // A request advertising Transfer-Encoding must be rejected with 501
    arm_slot(0, "POST /data HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n");
    g_server->on("/data", HTTP_POST, [](uint8_t, HttpReq *) {
        TEST_FAIL_MESSAGE("handler must not be called for Transfer-Encoding request");
    });
    g_server->handle(); // must send 501, not dispatch the route
    TEST_ASSERT_NOT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
}

void test_transfer_encoding_identity_is_501()
{
    // Even "identity" is rejected - we advertise no TE support at all
    arm_slot(0, "GET / HTTP/1.1\r\nTransfer-Encoding: identity\r\n\r\n");
    g_server->handle();
    TEST_ASSERT_NOT_EQUAL(PARSE_COMPLETE, http_pool[0].parse_state);
}

int main()
{
    UNITY_BEGIN();

    // Function I/O: handler API access
    RUN_TEST(test_handler_reads_body);
    RUN_TEST(test_handler_reads_query_param);
    RUN_TEST(test_handler_reads_header);
    RUN_TEST(test_wildcard_before_exact_wildcard_wins);

    // Function I/O: on()
    RUN_TEST(test_fn_on_registers_and_dispatches);
    RUN_TEST(test_fn_on_path_copied_null_terminated);
    RUN_TEST(test_fn_on_table_full_extra_routes_dropped);
    RUN_TEST(test_fn_on_same_path_different_methods_are_distinct);

    // Function I/O: on_not_found()
    RUN_TEST(test_fn_on_not_found_called_when_no_match);
    RUN_TEST(test_fn_on_not_found_not_called_when_match_exists);

    // Function I/O: set_cors()
    RUN_TEST(test_fn_set_cors_options_preflight_clears_slot);
    RUN_TEST(test_fn_set_cors_empty_string_disables);

    // Unit tests
    RUN_TEST(test_wrong_method_does_not_match);
    RUN_TEST(test_wrong_path_does_not_match);
    RUN_TEST(test_all_http_methods_dispatched);
    RUN_TEST(test_root_path_matches_exactly);
    RUN_TEST(test_root_path_does_not_match_subpath);
    RUN_TEST(test_wildcard_matches_any_suffix);
    RUN_TEST(test_wildcard_does_not_match_unrelated_prefix);
    RUN_TEST(test_exact_route_wins_when_registered_first);
    RUN_TEST(test_slot_not_stuck_in_complete_after_handle);
    RUN_TEST(test_parse_error_slot_auto_reset);

    // Stress tests
    RUN_TEST(stress_last_route_dispatched_in_full_table);
    RUN_TEST(stress_sequential_requests_no_state_leak);
    RUN_TEST(stress_all_slots_dispatched_simultaneously);
    RUN_TEST(stress_wildcard_matches_many_paths);
    RUN_TEST(stress_handle_with_no_complete_slots_is_nop);

    // Race condition simulations
    RUN_TEST(race_slot_complete_between_handle_calls);
    RUN_TEST(race_conn_freed_after_parse_complete);
    RUN_TEST(race_double_handle_no_double_dispatch);
    RUN_TEST(race_error_and_valid_slot_in_same_handle);
    RUN_TEST(race_callback_manually_resets_slot);

    // 414
    RUN_TEST(test_uri_too_long_auto_resets_slot);

    // Transfer-Encoding rejection
    RUN_TEST(test_transfer_encoding_chunked_is_501);
    RUN_TEST(test_transfer_encoding_identity_is_501);

    return UNITY_END();
}
