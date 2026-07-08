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

#include "dwserver.h"
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
    conn_pool[slot].proto = PROTO_HTTP; // dispatch requires an explicit protocol
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
    DeterministicAsyncTCP::pool_init();
    for (int i = 0; i < MAX_CONNS; i++)
    {
        conn_pool[i] = {};
        conn_pool[i].id = i;
        conn_pool[i].state = CONN_ACTIVE;
        conn_pool[i].proto = PROTO_HTTP; // dispatch requires an explicit protocol
        http_reset(i);
    }
    handler_called = false;
    handler_slot = 255;
#if DETWS_ENABLE_WEBSOCKET
    ws_init(); // isolate ws_pool[] between tests (a leftover WS slot makes http_parse skip it)
#endif
#if DETWS_ENABLE_SSE
    sse_init(); // isolate sse_pool[] between tests
#endif
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

// ====================================================================
// REDIRECT + MIME
// ====================================================================

void test_redirect_emits_location_and_status()
{
    conn_pool[0].state = CONN_ACTIVE;
    conn_pool[0].proto = PROTO_HTTP; // dispatch requires an explicit protocol
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->redirect(0, 301, "/index.html");
    const char *out = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(out, "HTTP/1.1 301 Moved Permanently"));
    TEST_ASSERT_NOT_NULL(strstr(out, "Location: /index.html\r\n"));
    TEST_ASSERT_NOT_NULL(strstr(out, "Content-Length: 0\r\n"));
    tcp_capture_disable();
    TEST_ASSERT_EQUAL(CONN_FREE, conn_pool[0].state); // slot released
}

void test_redirect_invalid_code_defaults_to_302()
{
    conn_pool[0].state = CONN_ACTIVE;
    conn_pool[0].proto = PROTO_HTTP; // dispatch requires an explicit protocol
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->redirect(0, 200, "/elsewhere"); // 200 is not a redirect code
    const char *out = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(out, "HTTP/1.1 302 Found"));
    tcp_capture_disable();
}

void test_mime_type_detection()
{
    TEST_ASSERT_EQUAL_STRING("text/html", DetWebServer::mime_type("/index.html"));
    TEST_ASSERT_EQUAL_STRING("text/css", DetWebServer::mime_type("/css/site.css"));
    TEST_ASSERT_EQUAL_STRING("application/javascript", DetWebServer::mime_type("/app.JS")); // case-insensitive
    TEST_ASSERT_EQUAL_STRING("application/json", DetWebServer::mime_type("/api/data.json"));
    TEST_ASSERT_EQUAL_STRING("image/svg+xml", DetWebServer::mime_type("logo.svg"));
    TEST_ASSERT_EQUAL_STRING("image/png", DetWebServer::mime_type("a.b.c.png")); // last extension wins
    // Unknown / missing extension and dotfiles fall back.
    TEST_ASSERT_EQUAL_STRING("application/octet-stream", DetWebServer::mime_type("/file.unknownext"));
    TEST_ASSERT_EQUAL_STRING("application/octet-stream", DetWebServer::mime_type("/noext"));
    TEST_ASSERT_EQUAL_STRING("application/octet-stream", DetWebServer::mime_type("/dir.with.dot/file"));
    TEST_ASSERT_EQUAL_STRING("application/octet-stream", DetWebServer::mime_type(nullptr));
}

// ====================================================================
// SERVE_STATIC (mount a filesystem subtree)
// ====================================================================

static fs::FS g_static_fs; // mock FS instance (state lives in the global registry)

void test_serve_static_file_and_mime()
{
    fs::mock_fs_reset();
    static const char css[] = "body{color:red}";
    fs::mock_fs_add("/www/style.css", css);
    g_server->serve_static("/", g_static_fs, "/www");
    arm_slot(0, "GET /style.css HTTP/1.1\r\nHost: x\r\n\r\n");
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->handle();
    const char *out = tcp_captured();
    tcp_capture_disable();
    TEST_ASSERT_NOT_NULL(strstr(out, "HTTP/1.1 200 OK"));
    TEST_ASSERT_NOT_NULL(strstr(out, "Content-Type: text/css"));
    TEST_ASSERT_NOT_NULL(strstr(out, "body{color:red}"));
}

void test_serve_static_cache_control()
{
    fs::mock_fs_reset();
    static const char css[] = "body{color:red}";
    fs::mock_fs_add("/www/style.css", css);
    g_server->serve_static("/", g_static_fs, "/www");

    g_server->set_cache_control("max-age=3600");
    arm_slot(0, "GET /style.css HTTP/1.1\r\nHost: x\r\n\r\n");
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->handle();
    const char *out = tcp_captured();
    tcp_capture_disable();
    TEST_ASSERT_NOT_NULL(strstr(out, "HTTP/1.1 200 OK"));
    TEST_ASSERT_NOT_NULL(strstr(out, "Cache-Control: max-age=3600"));

    // Clearing it removes the header (and restores the default for later tests).
    g_server->set_cache_control("");
    arm_slot(0, "GET /style.css HTTP/1.1\r\nHost: x\r\n\r\n");
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->handle();
    out = tcp_captured();
    tcp_capture_disable();
    TEST_ASSERT_NULL(strstr(out, "Cache-Control:"));
}

void test_serve_static_index_fallback()
{
    fs::mock_fs_reset();
    static const char html[] = "<h1>home</h1>";
    fs::mock_fs_add("/www/index.html", html);
    g_server->serve_static("/", g_static_fs, "/www");
    arm_slot(0, "GET / HTTP/1.1\r\nHost: x\r\n\r\n");
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->handle();
    const char *out = tcp_captured();
    tcp_capture_disable();
    TEST_ASSERT_NOT_NULL(strstr(out, "HTTP/1.1 200 OK"));
    TEST_ASSERT_NOT_NULL(strstr(out, "Content-Type: text/html"));
    TEST_ASSERT_NOT_NULL(strstr(out, "<h1>home</h1>"));
}

void test_serve_static_gzip_when_accepted()
{
    fs::mock_fs_reset();
    static const char gzbody[] = "\x1f\x8b"
                                 "FAKEGZIP"; // split avoids \x8bF hex-escape merge
    fs::mock_fs_add("/www/app.js.gz", (const uint8_t *)gzbody, sizeof(gzbody) - 1);
    g_server->serve_static("/", g_static_fs, "/www");
    arm_slot(0, "GET /app.js HTTP/1.1\r\nHost: x\r\nAccept-Encoding: gzip, deflate\r\n\r\n");
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->handle();
    const char *out = tcp_captured();
    tcp_capture_disable();
    TEST_ASSERT_NOT_NULL(strstr(out, "HTTP/1.1 200 OK"));
    TEST_ASSERT_NOT_NULL(strstr(out, "Content-Type: application/javascript")); // original type
    TEST_ASSERT_NOT_NULL(strstr(out, "Content-Encoding: gzip"));
}

void test_serve_static_no_gzip_when_not_accepted()
{
    fs::mock_fs_reset();
    static const char js[] = "console.log(1)";
    fs::mock_fs_add("/www/app.js", js);
    fs::mock_fs_add("/www/app.js.gz", "GZIPPED");
    g_server->serve_static("/", g_static_fs, "/www");
    arm_slot(0, "GET /app.js HTTP/1.1\r\nHost: x\r\n\r\n"); // no Accept-Encoding
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->handle();
    const char *out = tcp_captured();
    tcp_capture_disable();
    TEST_ASSERT_NULL(strstr(out, "Content-Encoding: gzip"));
    TEST_ASSERT_NOT_NULL(strstr(out, "console.log(1)"));
}

void test_serve_static_traversal_not_leaked()
{
    fs::mock_fs_reset();
    fs::mock_fs_add("/secret", "topsecret");
    g_server->serve_static("/", g_static_fs, "/www");
    arm_slot(0, "GET /../secret HTTP/1.1\r\nHost: x\r\n\r\n");
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->handle();
    const char *out = tcp_captured();
    tcp_capture_disable();
    TEST_ASSERT_NULL(strstr(out, "topsecret")); // traversal must not leak the file
}

void test_serve_static_missing_is_404()
{
    fs::mock_fs_reset();
    fs::mock_fs_add("/www/exists.txt", "hi");
    g_server->serve_static("/", g_static_fs, "/www");
    arm_slot(0, "GET /nope.txt HTTP/1.1\r\nHost: x\r\n\r\n");
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->handle();
    const char *out = tcp_captured();
    tcp_capture_disable();
    TEST_ASSERT_NOT_NULL(strstr(out, "404"));
}

// A served file carries an ETag; a matching If-None-Match yields 304 (no body).
void test_serve_static_etag_conditional_get()
{
    fs::mock_fs_reset();
    fs::mock_fs_add("/www/page.html", "<html>hi</html>", (time_t)1000);
    g_server->serve_static("/", g_static_fs, "/www");

    // First GET: 200 with an ETag header.
    arm_slot(0, "GET /page.html HTTP/1.1\r\nHost: x\r\n\r\n");
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->handle();
    const char *out1 = tcp_captured();
    tcp_capture_disable();
    TEST_ASSERT_NOT_NULL(strstr(out1, "HTTP/1.1 200 OK"));
    const char *etp = strstr(out1, "ETag: ");
    TEST_ASSERT_NOT_NULL(etp);
    char etag[40];
    etp += 6;
    size_t i = 0;
    while (etp[i] && etp[i] != '\r' && i < sizeof(etag) - 1)
    {
        etag[i] = etp[i];
        i++;
    }
    etag[i] = '\0';

    // Second GET with that ETag in If-None-Match: 304, no body.
    char req[160];
    snprintf(req, sizeof(req), "GET /page.html HTTP/1.1\r\nHost: x\r\nIf-None-Match: %s\r\n\r\n", etag);
    arm_slot(0, req);
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->handle();
    const char *out2 = tcp_captured();
    tcp_capture_disable();
    TEST_ASSERT_NOT_NULL(strstr(out2, "304 Not Modified"));
    TEST_ASSERT_NOT_NULL(strstr(out2, etag));          // ETag echoed
    TEST_ASSERT_NULL(strstr(out2, "<html>hi</html>")); // body suppressed
}

// RFC 9110 13.1.2: If-None-Match supports "*", a comma-separated list, and weak
// comparison (W/"x" matches our strong "x"). All three must yield 304.
void test_serve_static_inm_star_list_weak()
{
    fs::mock_fs_reset();
    fs::mock_fs_add("/www/page.html", "<html>hi</html>", (time_t)1000);
    g_server->serve_static("/", g_static_fs, "/www");

    // First GET to capture the strong ETag (with quotes).
    arm_slot(0, "GET /page.html HTTP/1.1\r\nHost: x\r\n\r\n");
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->handle();
    const char *out1 = tcp_captured();
    tcp_capture_disable();
    const char *etp = strstr(out1, "ETag: ");
    TEST_ASSERT_NOT_NULL(etp);
    char etag[40];
    etp += 6;
    size_t i = 0;
    while (etp[i] && etp[i] != '\r' && i < sizeof(etag) - 1)
    {
        etag[i] = etp[i];
        i++;
    }
    etag[i] = '\0';

    char req[200];
    // (a) "*" matches any current representation -> 304.
    arm_slot(0, "GET /page.html HTTP/1.1\r\nHost: x\r\nIf-None-Match: *\r\n\r\n");
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->handle();
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "304 Not Modified"));
    tcp_capture_disable();

    // (b) weak validator W/"x" matches our strong "x" -> 304.
    snprintf(req, sizeof(req), "GET /page.html HTTP/1.1\r\nHost: x\r\nIf-None-Match: W/%s\r\n\r\n", etag);
    arm_slot(0, req);
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->handle();
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "304 Not Modified"));
    tcp_capture_disable();

    // (c) a list containing the tag (after a non-matching one) -> 304.
    snprintf(req, sizeof(req), "GET /page.html HTTP/1.1\r\nHost: x\r\nIf-None-Match: \"nope\", %s\r\n\r\n", etag);
    arm_slot(0, req);
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->handle();
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "304 Not Modified"));
    tcp_capture_disable();

    // (d) a list with only non-matching tags -> 200 (full response).
    arm_slot(0, "GET /page.html HTTP/1.1\r\nHost: x\r\nIf-None-Match: \"a\", \"b\"\r\n\r\n");
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->handle();
    TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), "HTTP/1.1 200 OK"));
    tcp_capture_disable();
}

// A served file carries Last-Modified; If-Modified-Since drives a conditional GET
// (304 when not newer than the client's date), with If-None-Match taking precedence.
void test_serve_static_last_modified_conditional_get()
{
    fs::mock_fs_reset();
    fs::mock_fs_add("/www/page.html", "<html>hi</html>", (time_t)1000); // 1970-01-01 00:16:40 GMT
    g_server->serve_static("/", g_static_fs, "/www");
    const char *LM = "Thu, 01 Jan 1970 00:16:40 GMT";
    char req[200];
    const char *o;

    // (1) plain GET: 200 carries the Last-Modified header.
    arm_slot(0, "GET /page.html HTTP/1.1\r\nHost: x\r\n\r\n");
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->handle();
    o = tcp_captured();
    tcp_capture_disable();
    TEST_ASSERT_NOT_NULL(strstr(o, "HTTP/1.1 200 OK"));
    TEST_ASSERT_NOT_NULL(strstr(o, "Last-Modified: Thu, 01 Jan 1970 00:16:40 GMT\r\n"));

    // (2) If-Modified-Since == mtime -> not modified -> 304, no body.
    snprintf(req, sizeof(req), "GET /page.html HTTP/1.1\r\nHost: x\r\nIf-Modified-Since: %s\r\n\r\n", LM);
    arm_slot(0, req);
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->handle();
    o = tcp_captured();
    tcp_capture_disable();
    TEST_ASSERT_NOT_NULL(strstr(o, "304 Not Modified"));
    TEST_ASSERT_NULL(strstr(o, "<html>hi</html>"));

    // (3) If-Modified-Since one second older than mtime -> file IS newer -> 200 + body.
    arm_slot(0, "GET /page.html HTTP/1.1\r\nHost: x\r\nIf-Modified-Since: Thu, 01 Jan 1970 00:16:39 GMT\r\n\r\n");
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->handle();
    o = tcp_captured();
    tcp_capture_disable();
    TEST_ASSERT_NOT_NULL(strstr(o, "HTTP/1.1 200 OK"));
    TEST_ASSERT_NOT_NULL(strstr(o, "<html>hi</html>"));

    // (4) If-Modified-Since newer than mtime -> 304.
    arm_slot(0, "GET /page.html HTTP/1.1\r\nHost: x\r\nIf-Modified-Since: Fri, 02 Jan 1970 00:00:00 GMT\r\n\r\n");
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->handle();
    o = tcp_captured();
    tcp_capture_disable();
    TEST_ASSERT_NOT_NULL(strstr(o, "304 Not Modified"));

    // (5) If-None-Match (non-matching) takes precedence: If-Modified-Since ignored -> 200.
    snprintf(req, sizeof(req),
             "GET /page.html HTTP/1.1\r\nHost: x\r\nIf-None-Match: \"deadbeef\"\r\nIf-Modified-Since: %s\r\n\r\n", LM);
    arm_slot(0, req);
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->handle();
    o = tcp_captured();
    tcp_capture_disable();
    TEST_ASSERT_NOT_NULL(strstr(o, "HTTP/1.1 200 OK"));
    TEST_ASSERT_NOT_NULL(strstr(o, "<html>hi</html>"));
}

// A malformed If-Modified-Since must fail safe: serve 200 (the body), never a stale
// 304. Includes the off-by-alignment month token ("ebM" lives inside "FebMar") that
// must NOT mis-parse as a real month.
void test_serve_static_if_modified_since_malformed()
{
    fs::mock_fs_reset();
    fs::mock_fs_add("/www/page.html", "<html>hi</html>", (time_t)1000); // Jan 1970
    g_server->serve_static("/", g_static_fs, "/www");
    const char *bad[] = {
        "not a date",                    // sscanf field count != 6
        "Thu, 01",                       // truncated
        "Thu, 01 ebM 1970 00:00:00 GMT", // bogus month token at a non-3 offset
        "Thu, 01 Xyz 1970 00:00:00 GMT", // unknown month
    };
    for (size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); i++)
    {
        char req[200];
        snprintf(req, sizeof(req), "GET /page.html HTTP/1.1\r\nHost: x\r\nIf-Modified-Since: %s\r\n\r\n", bad[i]);
        arm_slot(0, req);
        conn_pool[0].pcb = &_mock_pcb;
        tcp_capture_reset();
        g_server->handle();
        const char *o = tcp_captured();
        tcp_capture_disable();
        TEST_ASSERT_NOT_NULL(strstr(o, "HTTP/1.1 200 OK")); // not 304
        TEST_ASSERT_NOT_NULL(strstr(o, "<html>hi</html>")); // body served
    }
}

// ====================================================================
// ACCESS-LOG HOOK + RUNTIME STATS
// ====================================================================

static char g_log_method[8];
static char g_log_path[64];
static int g_log_status;
static int g_log_bytes;
static int g_log_calls;
static void capture_log(const char *m, const char *p, int s, int b)
{
    strncpy(g_log_method, m, sizeof(g_log_method) - 1);
    g_log_method[sizeof(g_log_method) - 1] = '\0';
    strncpy(g_log_path, p, sizeof(g_log_path) - 1);
    g_log_path[sizeof(g_log_path) - 1] = '\0';
    g_log_status = s;
    g_log_bytes = b;
    g_log_calls++;
}

void test_request_log_hook_fires()
{
    g_log_calls = 0;
    g_server->on_request_log(capture_log);
    g_server->on("/hi", HTTP_GET, [](uint8_t id, HttpReq *) { g_server->send(id, 200, "text/plain", "hello"); });
    arm_slot(0, "GET /hi HTTP/1.1\r\nHost: x\r\n\r\n");
    conn_pool[0].pcb = &_mock_pcb;
    g_server->handle();
    TEST_ASSERT_EQUAL_INT(1, g_log_calls);
    TEST_ASSERT_EQUAL_STRING("GET", g_log_method);
    TEST_ASSERT_EQUAL_STRING("/hi", g_log_path);
    TEST_ASSERT_EQUAL_INT(200, g_log_status);
    TEST_ASSERT_EQUAL_INT(5, g_log_bytes); // "hello"
    g_server->on_request_log(nullptr);
}

void test_stats_endpoint_emits_json()
{
    g_server->on("/stats", HTTP_GET, [](uint8_t id, HttpReq *) { g_server->stats(id); });
    arm_slot(0, "GET /stats HTTP/1.1\r\nHost: x\r\n\r\n");
    conn_pool[0].pcb = &_mock_pcb;
    tcp_capture_reset();
    g_server->handle();
    const char *out = tcp_captured();
    tcp_capture_disable();
    TEST_ASSERT_NOT_NULL(strstr(out, "application/json"));
    TEST_ASSERT_NOT_NULL(strstr(out, "\"uptime_ms\""));
    TEST_ASSERT_NOT_NULL(strstr(out, "\"requests\""));
    TEST_ASSERT_NOT_NULL(strstr(out, "\"http_2xx\""));
    TEST_ASSERT_NOT_NULL(strstr(out, "\"http_4xx\""));
    TEST_ASSERT_NOT_NULL(strstr(out, "\"active_conns\""));
}

#if DETWS_ENABLE_METRICS
// Prometheus /metrics emits the stats counters in text exposition format.
void test_metrics_emits_prometheus()
{
    conn_pool[0] = {};
    conn_pool[0].id = 0;
    conn_pool[0].state = CONN_ACTIVE;
    conn_pool[0].proto = PROTO_HTTP; // dispatch requires an explicit protocol
    conn_pool[0].pcb = &_mock_pcb;
    http_reset(0);
    tcp_capture_reset();
    g_server->metrics(0);
    const char *out = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(out, "text/plain; version=0.0.4"));
    TEST_ASSERT_NOT_NULL(strstr(out, "# TYPE detws_http_requests_total counter"));
    TEST_ASSERT_NOT_NULL(strstr(out, "detws_http_responses_total{class=\"2xx\"}"));
    TEST_ASSERT_NOT_NULL(strstr(out, "detws_free_heap_bytes"));
    TEST_ASSERT_NOT_NULL(strstr(out, "detws_uptime_seconds"));
    tcp_capture_disable();
}
#endif

#if DETWS_ENABLE_SSE
// Regression: sse_do_upgrade() must store the request path by VALUE before
// http_reset() zeroes the parser buffer, so a later path-matched sse_broadcast()
// reaches the client. (A dangling path pointer made broadcasts silently miss.)
void test_sse_broadcast_after_upgrade_matches_path()
{
    g_server->on_sse("/events", nullptr);

    conn_pool[0] = {};
    conn_pool[0].id = 0;
    conn_pool[0].state = CONN_ACTIVE;
    conn_pool[0].proto = PROTO_HTTP; // dispatch requires an explicit protocol
    conn_pool[0].pcb = &_mock_pcb;
    push_bytes(0, "GET /events HTTP/1.1\r\n\r\n");
    http_reset(0);
    http_parse(0);

    tcp_capture_reset();
    g_server->handle(); // dispatch -> sse_do_upgrade (200 text/event-stream)
    g_server->sse_broadcast("/events", "hello", "msg");
    const char *out = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(out, "text/event-stream")); // upgrade happened
    TEST_ASSERT_NOT_NULL(strstr(out, "data: hello"));       // broadcast matched the stored path
    tcp_capture_disable();
}
#endif

#if DETWS_ENABLE_WEBSOCKET
// The WebSocket send API: bad-id / inactive / terminal-state guards send
// nothing; a live connection frames text (0x81) and binary (0x82) payloads and
// flushes, and ws_disconnect queues a Close frame (0x88).
void test_ws_send_api()
{
    ws_init();
    conn_pool[0] = {};
    conn_pool[0].id = 0;
    conn_pool[0].state = CONN_ACTIVE;
    conn_pool[0].proto = PROTO_HTTP;
    conn_pool[0].pcb = &_mock_pcb;
    WsConn *ws = ws_alloc(0);
    TEST_ASSERT_NOT_NULL(ws);

    // Guards: out-of-range id and an id that is in range but inactive.
    tcp_capture_reset();
    g_server->ws_send_text(MAX_WS_CONNS, "x");                       // id >= MAX
    g_server->ws_send_text(1, "x");                                  // in range, inactive
    g_server->ws_send_binary(MAX_WS_CONNS, (const uint8_t *)"x", 1); // id >= MAX
    TEST_ASSERT_EQUAL_size_t(0, tcp_captured_len());

    // Text frame -> FIN|TEXT opcode.
    tcp_capture_reset();
    g_server->ws_send_text(0, "hello");
    TEST_ASSERT_TRUE(tcp_captured_len() >= 2);
    TEST_ASSERT_EQUAL_HEX8(0x81, (uint8_t)tcp_captured()[0]);

    // Binary frame -> FIN|BINARY opcode.
    tcp_capture_reset();
    const uint8_t payload[3] = {1, 2, 3};
    g_server->ws_send_binary(0, payload, sizeof(payload));
    TEST_ASSERT_TRUE(tcp_captured_len() >= 2);
    TEST_ASSERT_EQUAL_HEX8(0x82, (uint8_t)tcp_captured()[0]);

    // A terminal parse state suppresses further sends.
    ws->parse_state = WS_CLOSED;
    tcp_capture_reset();
    g_server->ws_send_text(0, "nope");
    g_server->ws_send_binary(0, payload, sizeof(payload));
    TEST_ASSERT_EQUAL_size_t(0, tcp_captured_len());
    ws->parse_state = WS_HEADER1; // reopen for disconnect

    // Disconnect: Close frame (opcode 0x88); the out-of-range id is a no-op.
    tcp_capture_reset();
    g_server->ws_disconnect(MAX_WS_CONNS);
    g_server->ws_disconnect(0);
    TEST_ASSERT_TRUE(tcp_captured_len() >= 2);
    TEST_ASSERT_EQUAL_HEX8(0x88, (uint8_t)tcp_captured()[0]);
    tcp_capture_disable();
}
#endif

#if DETWS_ENABLE_SSE
// The SSE send API: sse_send writes an event/id/data block to the bound slot;
// bad-id / inactive guards send nothing; sse_broadcast skips connections whose
// stored path does not match.
void test_sse_send_api()
{
    sse_init();
    conn_pool[0] = {};
    conn_pool[0].id = 0;
    conn_pool[0].state = CONN_ACTIVE;
    conn_pool[0].proto = PROTO_HTTP;
    conn_pool[0].pcb = &_mock_pcb;
    SseConn *sse = sse_alloc(0, "/events");
    TEST_ASSERT_NOT_NULL(sse);

    // Guards send nothing.
    tcp_capture_reset();
    g_server->sse_send(MAX_SSE_CONNS, "x"); // id >= MAX
    g_server->sse_send(1, "x");             // in range, inactive
    TEST_ASSERT_EQUAL_size_t(0, tcp_captured_len());

    // A live send emits the event, id, and data fields (RFC-style SSE block).
    tcp_capture_reset();
    g_server->sse_send(0, "hi", "msg", "42");
    const char *out = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(out, "event: msg"));
    TEST_ASSERT_NOT_NULL(strstr(out, "id: 42"));
    TEST_ASSERT_NOT_NULL(strstr(out, "data: hi"));

    // Broadcast to a non-matching path skips the connection (no output).
    tcp_capture_reset();
    g_server->sse_broadcast("/other", "skip");
    TEST_ASSERT_EQUAL_size_t(0, tcp_captured_len());
    tcp_capture_disable();
}
#endif

// status_text() renders the reason phrase for every code the server emits.
// send() formats "HTTP/1.1 <code> <reason>", so drive it across the codes that
// higher-level tests do not already exercise (plus an unknown code -> default).
void test_status_text_reason_phrases()
{
    struct StatusCase
    {
        int code;
        const char *reason;
    };
    static const StatusCase cases[] = {
        {201, "Created"},
        {206, "Partial Content"},
        {303, "See Other"},
        {304, "Not Modified"},
        {307, "Temporary Redirect"},
        {308, "Permanent Redirect"},
        {401, "Unauthorized"},
        {405, "Method Not Allowed"},
        {408, "Request Timeout"},
        {409, "Conflict"},
        {413, "Payload Too Large"},
        {414, "URI Too Long"},
        {416, "Range Not Satisfiable"},
        {429, "Too Many Requests"},
        {500, "Internal Server Error"},
        {501, "Not Implemented"},
        {503, "Service Unavailable"},
        {999, "Unknown"}, // 999 -> default
#if DETWS_ENABLE_WEBDAV
        {207, "Multi-Status"},
        {412, "Precondition Failed"},
        {423, "Locked"},
        {502, "Bad Gateway"},
#endif
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
    {
        conn_pool[0] = {};
        conn_pool[0].id = 0;
        conn_pool[0].state = CONN_ACTIVE;
        conn_pool[0].proto = PROTO_HTTP;
        conn_pool[0].pcb = &_mock_pcb;
        http_reset(0);
        tcp_capture_reset();
        g_server->send(0, cases[i].code, "text/plain", "x");
        char want[48];
        snprintf(want, sizeof(want), "HTTP/1.1 %d %s", cases[i].code, cases[i].reason);
        TEST_ASSERT_NOT_NULL(strstr(tcp_captured(), want));
    }
    tcp_capture_disable();
}

// A 405 lists every method registered for the matched path in the Allow header;
// method_name() renders each token. Register PATCH/HEAD/OPTIONS on one path and
// request it with an unregistered method.
void test_allow_header_lists_methods()
{
    g_server->on("/m", HTTP_PATCH, record_handler);
    g_server->on("/m", HTTP_OPTIONS, record_handler);
    g_server->on("/m", HTTP_HEAD, record_handler);
    g_server->on("/m", HTTP_PUT, record_handler);
    g_server->on("/m", HTTP_METHOD_UNKNOWN, record_handler); // -> method_name() default ""
    arm_slot(0, "DELETE /m HTTP/1.1\r\n\r\n");
    conn_pool[0].pcb = &_mock_pcb; // arm_slot leaves pcb null; the 405 must emit
    tcp_capture_reset();
    g_server->handle();
    const char *out = tcp_captured();
    TEST_ASSERT_NOT_NULL(strstr(out, "405"));
    TEST_ASSERT_NOT_NULL(strstr(out, "PATCH"));
    TEST_ASSERT_NOT_NULL(strstr(out, "OPTIONS"));
    TEST_ASSERT_NOT_NULL(strstr(out, "HEAD"));
    TEST_ASSERT_NOT_NULL(strstr(out, "PUT"));
    tcp_capture_disable();
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

    RUN_TEST(test_redirect_emits_location_and_status);
    RUN_TEST(test_redirect_invalid_code_defaults_to_302);
    RUN_TEST(test_mime_type_detection);

    RUN_TEST(test_serve_static_file_and_mime);
    RUN_TEST(test_serve_static_index_fallback);
    RUN_TEST(test_serve_static_gzip_when_accepted);
    RUN_TEST(test_serve_static_no_gzip_when_not_accepted);
    RUN_TEST(test_serve_static_traversal_not_leaked);
    RUN_TEST(test_serve_static_missing_is_404);
    RUN_TEST(test_serve_static_etag_conditional_get);
    RUN_TEST(test_serve_static_inm_star_list_weak);
    RUN_TEST(test_serve_static_last_modified_conditional_get);
    RUN_TEST(test_serve_static_if_modified_since_malformed);
    RUN_TEST(test_serve_static_cache_control);

    RUN_TEST(test_request_log_hook_fires);
    RUN_TEST(test_stats_endpoint_emits_json);
    RUN_TEST(test_status_text_reason_phrases);
    RUN_TEST(test_allow_header_lists_methods);

#if DETWS_ENABLE_WEBSOCKET
    RUN_TEST(test_ws_send_api);
#endif
#if DETWS_ENABLE_SSE
    RUN_TEST(test_sse_broadcast_after_upgrade_matches_path);
    RUN_TEST(test_sse_send_api);
#endif
#if DETWS_ENABLE_METRICS
    RUN_TEST(test_metrics_emits_prometheus);
#endif

    return UNITY_END();
}
